#include <FastLED.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include "secret.h"

#define SEALEVELPRESSURE_HPA 1013.00
#define NUM_LEDS 1
#define DATA_PIN 5
#define CLOCK_PIN 13
#define uS_TO_S_FACTOR 1000000
#define TIME_TO_SLEEP  60
#define I2C_SDA 21
#define I2C_SCL 22

CRGB leds[NUM_LEDS];
Adafruit_BME280 bme;

// Recommended delay time between reading from Bosch Sensortec
unsigned long delayTime;

char ssid[32];
char pass[20];

void setLED(uint32_t val) {
  leds[0] = val;
  FastLED.show();
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

    blinkDelay(1000, 2, 0xFFFF00);

    connStatus = WiFi.status();
    if (connStatus == WL_CONNECTED) { break; }

    Serial.println("-- initWiFi() connection failed, retrying...");
    WiFi.disconnect();
    blinkDelay(1000, 10, 0xFF0000);
  }

  setLED(0x00FF00);
  delay(500);

  Serial.print("-- Connected to ");
  Serial.print(ssid);
  Serial.print(" with ");
  Serial.print(pass);
  Serial.print(", own ip: [");
  Serial.print(WiFi.localIP());
  Serial.println("]");
  Serial.println("-- initWiFi() finish");
}

// TODO init server location in same way as wifi.
// TODO persist config!
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

  setLED(0xFF00FF);

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
  Serial.print(bme.readTemperature());
  Serial.print("°C ");
  Serial.print(bme.readHumidity());
  Serial.print("%, ");
  Serial.print(bme.readPressure() / 100.0f, 2);
  Serial.println("hPa");

  FastLED.setBrightness(1); // Devices with overly bright LEDs suck!
  setLED(0xFF0000);

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

  setLED(0xFF0000);

  Serial.begin(115200);
  delay(1000);

  Serial.println("* setup() start");

  /* bool isConnected = initConnection(); */

  //initConfig();

  initWiFi();

  measure();

  Serial.println("* setup() finish");

  esp_deep_sleep_start();
}

void loop() {
  // TODO buffer?
  // TODO send measurement(s?)
  // TODO mqtt?
}
