# QuinLED Dig Uno board

These files allow WLED 0.9.1 to report the temp sensor on the Quinled board to MQTT. I use it to report the board temp to Home Assistant via MQTT, so it will send notifications if something happens and the board start to heat up.
This code uses Aircookie's WLED software. It has a premade file for user modifications. I use it to publish the temperature from the dallas temperature sensor on the Quinled board. The entries for the top of the WLED00 file, initializes the required libraries, and variables for the sensor. The .ino file waits for 60 seconds, and checks to see if the MQTT server is connected (thanks Aircoookie). It then poles the sensor, and published it using the MQTT service already running, using the main topic programmed in the WLED UI.

Installation of file: Copy and replace file in wled00 directory

## Project link

* [QuinLED-Dig-Uno](https://quinled.info/2018/09/15/quinled-dig-uno/) - Project link

### Platformio requirements

Uncomment `DallasTemperature@~3.8.0`,`OneWire@~2.3.5 under` `[common]` section in `platformio.ini`:

```ini
# platformio.ini
...
[platformio]
...
; default_envs = esp07
default_envs = d1_mini
...
[common]
...
lib_deps_external =
  ...
  #For use SSD1306 OLED display uncomment following
  U8g2@~2.27.3
  #For Dallas sensor uncomment following 2 lines
  DallasTemperature@~3.8.0
  OneWire@~2.3.5
...
```
