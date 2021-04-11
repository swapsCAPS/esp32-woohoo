#include <FastLED.h>
#include <WiFi.h>

#include "secret.h"

#define NUM_LEDS 1
#define DATA_PIN 5
#define CLOCK_PIN 13

CRGB leds[NUM_LEDS];

void setLED(uint32_t val) {
  leds[0] = val;
  FastLED.show();
}

void initWiFi() {
  Serial.println("--- initWiFi() start");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    setLED(0x000000);
    Serial.print("--- initWiFi() trying to connect to ");
    Serial.println(ssid);
    delay(500);
    setLED(0xFFFF00);
    delay(500);
  }

  setLED(0x00FF00);

  Serial.print("--- Connected to ");
  Serial.print(ssid);
  Serial.print(", own ip: [");
  Serial.print(WiFi.localIP());
  Serial.println("]");
  Serial.println("--- initWiFi() finish");
}

void setup() {
  Serial.begin(115200);

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
  FastLED.setBrightness(16);


  setLED(0x00FF00);

  Serial.println();

  Serial.println("--- setup() start");

  initWiFi();

  Serial.println("--- setup() finish");
}

void loop() {
}
