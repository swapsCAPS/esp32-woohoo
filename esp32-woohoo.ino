#include <FastLED.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <AsyncMqttClient.h>

#include "secret.h"
#include "config.h"

extern "C" {
	#include "freertos/FreeRTOS.h"
	#include "freertos/timers.h"
}

#define NUM_LEDS 1
#define DATA_PIN 5
#define CLOCK_PIN 13
#define uS_TO_S_FACTOR 1000000
#define I2C_SDA 21
#define I2C_SCL 22

TimerHandle_t deadmanSwitch;

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

AsyncMqttClient mqttClient;

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

void onDeadmanSwitch() {
  Serial.println("-- onDeadmanSwitch() triggered!");
  goToSleep();
}

void connectToWifi() {
  Serial.println("-- connectToWifi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt() {
  Serial.println("-- connectToMqtt");
  mqttClient.connect();
}

void onWiFiEvent(WiFiEvent_t event) {
    Serial.printf("-- onWiFiEvent() event: %d\n", event);
    switch(event) {
      case SYSTEM_EVENT_STA_GOT_IP:
          Serial.print("-- onWiFiEvent() connected to ");
          Serial.print(ssid);
          Serial.print(" with ");
          Serial.print(pass);
          Serial.print(", own ip: [");
          Serial.print(WiFi.localIP());
          Serial.println("]");

          WiFi.macAddress(macAddress);
          setMacAddressStr();
          setTopicBuffie();

          connectToMqtt();

          break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
          Serial.println("WiFi lost connection");
          break;
    }
}

void initWiFi() {
  Serial.println("-- initWiFi() start");

  Serial.print("-- initWiFi() configured for: \"");
  Serial.print(ssid);
  Serial.print("\" with \"");
  Serial.print(pass);
  Serial.println("\"");

  WiFi.mode(WIFI_STA);
  WiFi.onEvent(onWiFiEvent);

  Serial.println("-- initWiFi() finish");
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("-- onMqttConnect() connected");

  measure();

  mqttClient.disconnect();
  WiFi.disconnect();

  goToSleep();
}

void goToSleep() {
  Serial.println("-- goToSleep()");

  esp_deep_sleep_start();
}

void initMQTT() {
  Serial.println("-- initMQTT() start");

  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setClientId(macAddressStrArr);
  mqttClient.onConnect(onMqttConnect);

  Serial.println("-- initMQTT() finish");
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

  mqttClient.publish(topicBuffie, 0, true, pubBuffie);

  Serial.println("-- measure() finish");
}

void setup() {
  deadmanSwitch = xTimerCreate("deadmanSwitch", pdMS_TO_TICKS(DEADMAN_SWITCH_MS), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(onDeadmanSwitch));
  xTimerStart(deadmanSwitch, 0);

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
  FastLED.setBrightness(4); // Devices with overly bright LEDs suck!

  Wire.begin(I2C_SDA, I2C_SCL);

  strcpy(ssid, WIFI_SSID);
  strcpy(pass, WIFI_PASSWORD);

  print_wakeup_reason();

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  Serial.begin(115200);

  Serial.println("* setup() start");

  initWiFi();
  initMQTT();

  connectToWifi();

  Serial.println("* setup() finished");
}

void loop() {
  // TODO finish config setting
  // TODO fix publish not always working
  // TODO implement MQTT ack / callback?
}
