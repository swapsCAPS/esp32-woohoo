# ESP32 + BME280 IoT Project

### Devices
Battery powered ESP32 + BME280 modules.  
They take measurements roughly every minute, send it to an mqtt broker and go back to sleep.  
Code can be found in `./esp32-woohoo.ino`

### Server
Simple rust mqtt scraper to expose prometheus metrics.
Code can be found in `./mqtt-to-prom`

## TODO
- [x] Round values to eliminate "noise"
- [x] [bugfix] parsing float goes wrong sometimes, write some tests
  - [x] test this
  - [x] output payload on parse error
  - [ ] check if happens again
- [x] Grafana dashboard
- [ ] NTP server call
- [ ] Send out time stamps
- [ ] Improve battery life
    - [x] Read out SoC (on A0?)
      - [x] Using voltage divider (10KΩ + 20KΩ)
    - [x] Reduce/disable LED on time
    - [ ] Reduce amount of measurements taken, i.e. increase sleep time
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
