# ESP32 + BME280 IoT Project

### Devices
Battery powered ESP32 + BME280 modules.  
They take measurements roughly every minute, send it to an mqtt broker and go back to sleep.  
Code can be found in `./esp32-woohoo.ino`

### Server
Simple rust mqtt scraper to expose prometheus metrics.
Code can be found in `./mqtt-to-prom`

### MQTT
If using mosquitto and running in a safe environment set
```
# /etc/mosquitto/mosquitto.conf
listener 1883 0.0.0.0 # Allow external connections
allow_anonymous true  # We have not set up auth
```

### Build

#### esp32-woohoo.ino
Dependencies:
- FastLED
- Adafruit_Sensor
- Adafruit_BME280
- PubSubClient by Nick O'Leary
- In Arduino IDE -> File -> Preferences -> Additional Boards Manager URLs, add: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`

#### mqtt-to-prom
Make sure [cross](https://github.com/rust-embedded/cross) is installed
```
cargo install cross
```
If running on raspberry pi 4 with aarch64 run:
```bash
make build-pi
```

## TODO
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
[Aint they cute?!](/images/3.jpg)

[](/images/model-animation.gif)

Deep sleep, wake, connect wifi, connect mqtt, take measurement, send measurement, deep sleep...
[Oooh look at the pretty lights!](/images/blinkenlights.gif)
