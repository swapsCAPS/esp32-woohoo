#include <FastLED.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <PubSubClient.h>

#include "secret.h"
#include "config.h"

#define NUM_LEDS 1
#define DATA_PIN 5
#define CLOCK_PIN 13
#define uS_TO_S_FACTOR 1000000
#define I2C_SDA 21
#define I2C_SCL 22

CRGB leds[NUM_LEDS];
Adafruit_BME280 bme;

// Recommended delay time between reading from Bosch Sensortec
unsigned long delayTime;

byte macAddress[6];

char ssid[32];
char pass[20];

char macAddressStrArr[18];
char topicBuffie[32];
char pubBuffie[27];
char valBuffie[8];

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

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

void printWiFiStatus() {
  switch (WiFi.status()) {
    case WL_CONNECTED:
      Serial.println("--- WiFi.status(): WL_CONNECTED");
      break;
    case WL_NO_SHIELD:
      Serial.println("--- WiFi.status(): WL_NO_SHIELD");
      break;
    case WL_CONNECT_FAILED:
      Serial.println("--- WiFi.status(): WL_CONNECT_FAILED");
      break;
    case WL_CONNECTION_LOST:
      Serial.println("--- WiFi.status(): WL_CONNECTION_LOST");
      break;
    case WL_DISCONNECTED:
      Serial.println("--- WiFi.status(): WL_DISCONNECTED");
      break;
    case WL_SCAN_COMPLETED:
      Serial.println("--- WiFi.status(): WL_DISCONNECTED");
      break;
    case WL_NO_SSID_AVAIL:
      Serial.println("--- WiFi.status(): WL_NO_SSID_AVAIL");
      break;
    case WL_IDLE_STATUS:
      Serial.println("--- WiFi.status(): WL_IDLE_STATUS");
      break;
    default:
      Serial.print("--- WiFi.status(): Unknown WiFi status [");
      Serial.print(WiFi.status());
      Serial.println("]");
      break;
  }
}

void initWiFi() {
  Serial.println("-- initWiFi() start");

  Serial.print("-- initWiFi() connecting to: \"");
  Serial.print(ssid);
  Serial.print("\" with \"");
  Serial.print(pass);
  Serial.println("\"");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("-- initWiFi() connection failed, retrying...");
    delay(5000);
  }

  Serial.print("-- initWiFi() connected to ");
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

  mqttClient.setServer(broker, 1883);

  while(!mqttClient.connected()) {
    Serial.print("-- initMQTT() connecting with client id: ");
    Serial.println(macAddressStrArr);

    if (mqttClient.connect("test")) {
      Serial.println("-- i
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

  if (strcmp(ssid, WIFI_SSID) == 0 && strcmp(pass, WIFI_PASSWORD) == 0) {
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

  Serial.print("-- measure() publishing: ");
  Serial.println(pubBuffie);

  mqttClient.publish(topicBuffie, pubBuffie);

  delay(500);

  Serial.println("-- measure() finish");
}

void setup() {
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
  FastLED.setBrightness(4); // Devices with overly bright LEDs suck!

  Wire.begin(I2C_SDA, I2C_SCL);

  strcpy(ssid, WIFI_SSID);
  strcpy(pass, WIFI_PASSWORD);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  print_wakeup_reason();

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  Serial.begin(115200);

  Serial.println("* setup() start");

  initWiFi();

  delay(30000);

  initMQTT();

  delay(1000);

  measure();

  delay(1000);

  Serial.println("* setup() finish");

  mqttClient.disconnect();
  WiFi.disconnect();
  esp_deep_sleep_start();
}

void loop() {
  // TODO finish config setting
  // TODO fix publish not always working
  // TODO implement MQTT ack / callback?
}
