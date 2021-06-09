#include <FastLED.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <PubSubClient.h>

#include "secret.h"

#define SEALEVELPRESSURE_HPA 1013.00
#define NUM_LEDS 1
#define DATA_PIN 5
#define CLOCK_PIN 13
#define uS_TO_S_FACTOR 1000000
#define TIME_TO_SLEEP  60 // TODO increase me
#define I2C_SDA 21
#define I2C_SCL 22

CRGB leds[NUM_LEDS];
Adafruit_BME280 bme;

// Hooked up using voltage divider
// In this case using 10K and 22K resistors I had laying around
int batteryLevelPin = A0;

// Recommended delay time between reading from Bosch Sensortec
unsigned long delayTime;

byte macAddress[6];

char ssid[32];
char pass[20];

char macAddressStrArr[18];
char topicBuffie[32];
char pubBuffie[27];
char valBuffie[8];

IPAddress broker(192,168,178,20); // IP address of your MQTT broker eg. 192.168.1.50
WiFiClient wclient;

PubSubClient client(wclient); // Setup MQTT client
bool state = 0;

void array_to_string(byte array[], unsigned int len, char buffer[]) {
  for (unsigned int i = 0; i < len; i++) {
    byte nib1 = (array[i] >> 4) & 0x0F;
    byte nib2 = (array[i] >> 0) & 0x0F;

    uint8_t offset = i;

    buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
    buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;

    if (i == len - 1) { break; }

    buffer[i*2+2] = ':';
  }
  buffer[len*2] = '\0';
}

void setLED(uint32_t val) {
  leds[0] = val;
  FastLED.show();
}

void setMacAddressStr() {
  array_to_string(macAddress, 6, macAddressStrArr);
}

void setTopicBuffie() {
  strcat(topicBuffie, "sensors/thp/");
  strcat(topicBuffie, macAddressStrArr);
}

void blinkDelay(uint16_t ival, uint8_t times, uint32_t color) {
  for (uint8_t i = 0; i < times; i++) {
    setLED(color);
    delay(ival / 2);
    setLED(0x000000);
    delay(ival / 2);
  }
}

void initWiFi() {
  Serial.println("-- initWiFi() start");

  Serial.print("-- initWiFi() connecting to: ");
  Serial.print(ssid);
  Serial.print(" with ");
  Serial.println(pass);

  WiFi.mode(WIFI_STA);

  uint8_t connStatus;
  while (connStatus != WL_CONNECTED) {
    WiFi.begin(ssid, pass);

    /* blinkDelay(1000, 2, 0xFFFF00); */
    delay(2000);

    connStatus = WiFi.status();
    if (connStatus == WL_CONNECTED) {
      Serial.println("-- initWiFi() connected");
      break;
    }

    Serial.println("-- initWiFi() connection failed, retrying...");
    WiFi.disconnect();
    /* blinkDelay(1000, 10, 0xFF0000); */
    delay(10000);
  }

  Serial.print("-- Connected to ");
  Serial.print(ssid);
  Serial.print(" with ");
  Serial.print(pass);
  Serial.print(", own ip: [");
  Serial.print(WiFi.localIP());
  Serial.println("]");

  WiFi.macAddress(macAddress);
  setMacAddressStr();
  setTopicBuffie();

  Serial.println("-- initWiFi() finish");
}

void initMQTT() {
  Serial.println("-- initMQTT() start");

  client.setServer(broker, 1883);

  while(!client.connected()) {
    Serial.println("-- initMQTT() connecting...");

    if (client.connect(macAddressStrArr)) {
      Serial.println("-- initMQTT() connected");
      break;
    };

    Serial.println("-- initMQTT() connection failed, retrying...");

    /* blinkDelay(1000, 5, 0x00FFFF); */
    delay(5000);
  }

  Serial.println("-- initMQTT() finish");
}

// TODO init server location in same way as wifi.
// TODO init client id
// TODO persist config in EEPROM
void initConfig() {
  Serial.println("-- initConfig() start");

  Serial.println("-- initConfig() waiting for wifi_conf...");

  setLED(0xFF00BB);

  delay(2000);

  while (Serial.available() > 0) {
    String str = Serial.readString();
    Serial.print("-- initConfig() received: [");
    Serial.print(str);
    Serial.println("]");

    if (str.startsWith("wifi_conf=")) {
      String cut = str.substring(10);
      cut.trim(); // Potential white space
      Serial.println("-- initConfig() setting wifi conf");
      cut.substring(0, cut.indexOf(",")).toCharArray(ssid, 32);
      Serial.print("-- initConfig() ssid: ["); Serial.print(ssid); Serial.println("]");
      Serial.print("-- initConfig() pass: ["); Serial.print(pass); Serial.println("]");
      cut.substring(cut.indexOf(",") + 1).toCharArray(pass, 20);
    } else {
      Serial.print("-- initConfig() unknown input: ");
    }
  }

  if (strcmp(ssid, DEFAULT_SSID) == 0 && strcmp(pass, DEFAULT_PASSWORD) == 0) {
    Serial.println("-- initConfig() using defaults as set in secret.h");
  }

  Serial.println("-- initConfig() done");
}

bool initConnection() {
  bool isConnected = false;
  for (int i = 0; i < 5; i++) {
    Serial.println("hello?");
    delay(500);
    while (Serial.available() > 0) {
      String str = Serial.readString();
      char response[3];
      str.toCharArray(response, 3);
      if (strcmp(response, "hi!") == 0) {
        Serial.println("hi there!");
        isConnected = true;
        break;
      }
    }
  }
  return isConnected;
}

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:     Serial.println("-- Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1:     Serial.println("-- Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER:    Serial.println("-- Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: Serial.println("-- Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP:      Serial.println("-- Wakeup caused by ULP program"); break;
    default:                        Serial.printf("-- Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
}

void measure() {
  Serial.println("-- measure() start");

  /* setLED(0xFF00FF); */

  while(!bme.begin(0x76)) {
    Serial.println("-- measure() bme.begin() failed");
    delay(1000);
  }

  bme.setSampling(
      Adafruit_BME280::MODE_FORCED,
      Adafruit_BME280::SAMPLING_X1, // temp
      Adafruit_BME280::SAMPLING_X1, // pressure
      Adafruit_BME280::SAMPLING_X1, // humidity
      Adafruit_BME280::FILTER_OFF
      );

  bme.takeForcedMeasurement();

  dtostrf(bme.readTemperature(), 5, 2, valBuffie);
  strcat(pubBuffie, valBuffie);
  strcat(pubBuffie, ",");
  dtostrf(bme.readHumidity(), 5, 2, valBuffie);
  strcat(pubBuffie, valBuffie);
  strcat(pubBuffie, ",");
  dtostrf(bme.readPressure() / 100.0f, 7, 2, valBuffie);
  strcat(pubBuffie, valBuffie);
  strcat(pubBuffie, ",");
  itoa(analogRead(batteryLevelPin), valBuffie, 10);
  strcat(pubBuffie, valBuffie);

  Serial.print("-- ");
  Serial.println(pubBuffie);

  client.publish(topicBuffie, pubBuffie);

  delay(500);

  Serial.println("-- measure() finish");
}

void setup() {
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
  FastLED.setBrightness(4); // Devices with overly bright LEDs suck!

  Wire.begin(I2C_SDA, I2C_SCL);

  strcpy(ssid, DEFAULT_SSID);
  strcpy(pass, DEFAULT_PASSWORD);

  print_wakeup_reason();

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  /* setLED(0xFF0000); */

  Serial.begin(115200);

  Serial.println("* setup() start");

  /* bool isConnected = initConnection(); */

  //initConfig();

  initWiFi();

  /* blinkDelay(500, 2, 0x00FF00); */
  delay(1000);

  initMQTT();

  /* blinkDelay(250, 4, 0x00FF00); */
  delay(1000);

  measure();

  /* blinkDelay(250, 2, 0x00FF00); */
  delay(1000);

  /* FastLED.setBrightness(2); // Devices with overly bright LEDs suck! */
  /* setLED(0xFF0000); */

  Serial.println("* setup() finish");

  client.disconnect();
  WiFi.disconnect();
  esp_deep_sleep_start();
}

void loop() {
  // TODO finish config setting
  // TODO fix publish not always working
  // TODO implement MQTT ack / callback?
}
