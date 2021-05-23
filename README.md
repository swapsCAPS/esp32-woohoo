# ESP32 + BME280 IoT Project

### Devices
Battery powered ESP32 + BME280 modules.  
They take measurements roughly every minute, send it to an mqtt broker and go back to sleep.  
Code can be found in `./esp32-woohoo.ino`

### Server
Simple rust mqtt scraper to expose prometheus metrics.
Code can be found in `./mqtt-to-prom`

## TODO
- [x] [bugfix] parsing float goes wrong sometimes, write some tests
  - [ ] test this
- [x] Grafana dashboard
- [ ] NTP server call
- [ ] Send out time stamps
- [ ] Add last will on connect()
- [ ] Auto config server. Loop over serial ports, check for devices and flash config to them.
  - [ ] Auto config server
  - [ ] Use EEPROM to store wifi credentials, broker location, etc.

## Media
3D printed case, check `./stl` for files.
![Aint they cute?!](/images/3.jpg)

![](/images/model-animation.gif)

Deep sleep, wake, connect wifi, connect mqtt, take measurement, send measurement, deep sleep...
![Oooh look at the pretty lights!](/images/blinkenlights.gif)
