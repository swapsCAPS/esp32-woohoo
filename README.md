# ESP32 + BME280 IoT Project

## BOM
- DFRobot Firebeetle ESP32-E
- 3.7V LiPo battery
- BME280
- Rectangular rocker switch

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
- [ ] Add timers to go back to sleep if connection not made
- [x] Round values to eliminate "noise"
- [x] [bugfix] parsing float goes wrong sometimes, write some tests
  - [x] test this
  - [x] output payload on parse error
  - [ ] check if happens again
- [x] Grafana dashboard
- [ ] NTP server call
- [ ] Send out time stamps
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
