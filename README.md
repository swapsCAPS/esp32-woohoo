# ESP32 + BME280 IoT Project

## BOM
- DFRobot Firebeetle ESP32-E
- 3.7V LiPo battery
- BME280
- Rectangular rocker switch

## ESP32 WiFi woes
My router does not play nice with ESP32's it seems. Using my phone's hotspot, connecting to the internet and getting a time from NTP works fine.
I **am** getting an IP address, so I know the credentials are correct. But it seems I can't reach the internet, and get disconnected fairly quickly after connecting.  
I fiddled with the security settings and changed from WPA2 + WPA3 to WPA + WPA2 and all of a sudden the ESP32 connected fine. Yay! Wanting to be thorough, I tried using WPA2 + WPA3 mode again to see if I could reproduce my issue. But no! For some weird reason it just keeps connecting now. Needs some more testing. I'll be testing a deep sleep script that sleeps for 10 minutes, then when it wakes up, gets the time and logs if it succeeded or failed to `Serial`.
According to some sources you need to disconnect wifi before deep sleep. I have not been able to reproduce consistent failures by _not_ disconnecting though, so I'll test without that first.
Seems that I successfully got ntp time every 10 minutes. Trying with WPA2 + WPA3 for a couple of cycles. Something weird is going on with my router. I should try turning it off and on again and/or doing a hard reset.  
Could it be there is some weird stateful thing going on?  
**Update:** After tweaking WPA settings back and forth, something in the modem appears to have been nudged. I'm getting consistently connected now, and can not reproduce my issue... I have not touched anything else, I should have probably just reset my modem, before diving into the software side of things.  
**Update 2:** WPA setting does not appear to persist properly, and it appears I have been using WPA + WPA2 this whole time... Come h4ck me!

## Battery life
Experimenting with two devices with their jumper wire cut. Leaving one as is. Will turn them on at the same time. and see how long each will last.  
**Update** I've left two devices side by side, in an attempt to minimize the WiFi interference component of the measurement. Weirdly, after 24 hours the one with its jumper cut is reporting a lower battery level than the one without the jumper cut. It's probably still just too early to say, on top of that, there are a lot of other variables that could influence the reading.

## Devices
Battery powered ESP32 + BME280 modules.  
They take a measurement, send it to an mqtt broker and go to sleep for `TIME_TO_SLEEP`, defined in `./config.h`.  
Arduino code can be found in `./esp32-woohoo.ino`

## Server
Simple rust mqtt scraper to expose prometheus metrics.  
Code can be found in `./mqtt-to-prom`  
Grafana dashboard available in `./mqtt-to-prom/meta/grafana-dashboard.json`

## MQTT
If using mosquitto and running in a safe environment set
```
# /etc/mosquitto/mosquitto.conf
listener 1883 0.0.0.0 # Allow external connections
allow_anonymous true  # We have not set up auth
```

## Build

### esp32-woohoo.ino
- Add the FireBeetle ESP32-E board in Arduino IDE Boards Manager
  - In Arduino IDE -> File -> Preferences -> Additional Boards Manager URLs, add: `http://download.dfrobot.top/FireBeetle/package_DFRobot_index.json`
    - This is for the DFRobot ESP32-E version!
    - Their server is slow af, grab coffee
  - Select the board in Arduino IDE -> Tools -> Board -> DFRobot ESP32 Arduino -> DFRobot FireBeetle ESP32-E
- Ensure the following libs are installed
  - WiFi by Arduino `1.2.7`
  - FastLED by Daniel Garcia `3.5.0`
  - Adafruit Unified Sensor `1.15`
  - Adafruit BME280 Library `2.2.2`
  - https://github.com/marvinroger/async-mqtt-client
- Add a `secret.h` file in the root of the project with WiFi credentials
  ```c
  // ./secret.h
  #define WIFI_SSID "SSID"
  #define WIFI_PASSWORD "hunter1" // lol
  ```

### mqtt-to-prom
Make sure [cross](https://github.com/rust-embedded/cross) is installed
```
cargo install cross
```
If running on raspberry pi 4 with aarch64 run:
```bash
make build-pi
```

## TODO
- [x] Add timer to go back to sleep if connection not made
- [x] Round values to eliminate "noise"
- [x] [bugfix] parsing float goes wrong sometimes, write some tests
  - [x] test this
  - [x] output payload on parse error
  - [ ] check if happens again
- [x] Grafana dashboard
- [x] NTP server call
- [x] Send out time stamps
- [ ] Blink on connection failures?
  - [ ] Needs some experimentation, might substantially reduce batter life
- [ ] Improve battery life
  - [x] Read out SoC (on A0?)
    - [x] Using voltage divider (10KΩ + 20KΩ)
  - [x] Reduce/disable LED on time
  - [x] Reduce amount of measurements taken, i.e. increase sleep time
  - [ ] Go back to deep sleep after several connection failures
  - [ ] Get better idea of battery level percentage (0-4096 but cuts out at 2000 or smth)
  - [ ] If possible light up when battery too low
- [x] Add last will on connect()
    - [ ] Decided to skip this for now. Will do server side most likely.
- [ ] Auto config server. Loop over serial ports, check for devices and flash config to them.
  - [ ] Auto config server
  - [ ] Use EEPROM to store wifi credentials, broker location, etc.

## Media
3D printed case, check `./stl` for files.
![Aint they cute?!](/images/3.jpg)

![](/images/model-animation.gif)

Deep sleep, wake, connect wifi, connect mqtt, take measurement, send measurement, deep sleep...  
![Oooh look at the pretty lights!](/images/blinkenlights.gif)
