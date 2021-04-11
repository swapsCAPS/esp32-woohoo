#include <FastLED.h>
#include <WiFi.h>

#include "secret.h"

#define NUM_LEDS 1
#define DATA_PIN 5
#define CLOCK_PIN 13

CRGB leds[NUM_LEDS];

char ssid[32];
char pass[20];

void setLED(uint32_t val) {
  leds[0] = val;
  FastLED.show();
}

void initWiFi() {
  Serial.println("--- initWiFi() start");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    setLED(0x000000);
    Serial.print("--- initWiFi() trying to connect to ");
    Serial.print(ssid);
    Serial.print(" with ");
    Serial.println(pass);
    delay(500);
    setLED(0xFFFF00);
    delay(500);
  }

  setLED(0x00FF00);

  Serial.print("--- Connected to ");
  Serial.print(ssid);
  Serial.print(" with ");
  Serial.print(pass);
  Serial.print(", own ip: [");
  Serial.print(WiFi.localIP());
  Serial.println("]");
  Serial.println("--- initWiFi() finish");
}

// TODO init server location in same way as wifi.
// TODO persist config!
void initConfig() {
  Serial.println("--- initConfig() start");

  Serial.println("--- initConfig() waiting for wifi_conf...");

  setLED(0xFF00BB);

  delay(2000);

  strcpy(ssid, DEFAULT_SSID);
  strcpy(pass, DEFAULT_PASSWORD);

  while (Serial.available() > 0) {
    String str = Serial.readString();
    Serial.print("--- initConfig() received: [");
    Serial.print(str);
    Serial.println("]");

    if (str.startsWith("wifi_conf=")) {
      String cut = str.substring(10);
      cut.trim(); // Potential white space
      Serial.println("--- initConfig() setting wifi conf");
      cut.substring(0, cut.indexOf(",")).toCharArray(ssid, 32);
      Serial.print("--- initConfig() ssid: ["); Serial.print(ssid); Serial.println("]");
      Serial.print("--- initConfig() pass: ["); Serial.print(pass); Serial.println("]");
      cut.substring(cut.indexOf(",") + 1).toCharArray(pass, 20);
    } else {
      Serial.print("--- initConfig() unknown input: ");
    }
  }

  if (strcmp(ssid, DEFAULT_SSID) == 0 && strcmp(pass, DEFAULT_PASSWORD) == 0) {
    Serial.println("--- initConfig() using defaults as set in secret.h");
  }

  Serial.println("--- initConfig() done");
}

void setup() {
  Serial.begin(115200);

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
  FastLED.setBrightness(8); // Devices with overly bright LEDs suck!

  setLED(0x00FF00);

  Serial.println();

  Serial.println("--- setup() start");

  initConfig();

  initWiFi();

  Serial.println("--- setup() finish");
}

void loop() {
  // TODO deep sleep
  // TODO wake
  // TODO measure
  // TODO buffer?
  // TODO connect wifi
  // TODO send measurement(s?)
}
