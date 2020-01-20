# Almost universal controller board for outdoor applications
This usermod is using ideas from @mrVanboy and @400killer
## Features
*   SSD1306 128x32 and 128x64 I2C OLED display
*   On screen IP address, SSID and controller status (e.g. ON or OFF, recent effect)
*   Auto display shutoff for saving display lifetime
*   Dallas temperature sensor
*   Reporting temperature to MQTT broker

## Hardware
![Hardware connection](assets/controller.jpg)

## Functionality checked with
*   ESP-07S
*   PlatformIO
*   SSD1306 128x32 I2C OLED display
*   DS18B20 (temperature sensor)
*   KY-022 (infrared receiver)
*   Push button (N.O. momentary switch)

### Platformio requirements
Uncomment `U8g2@~2.27.3`,`DallasTemperature@~3.8.0`,`OneWire@~2.3.5 under` `[common]` section in `platformio.ini`:
```ini
# platformio.ini
...
[common]
...
lib_deps_external =
  ...
  #For use SSD1306 0.91" OLED display uncomment following
  U8g2@~2.27.3
  #For Dallas sensor uncomment following 2 lines
  DallasTemperature@~3.8.0
  OneWire@~2.3.5
...
```
