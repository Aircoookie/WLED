# Temperature usermod

Based on the excellent `QuinLED_Dig_Uno_Temp_MQTT` by srg74 and 400killer!  
This usermod will read from an attached DS18B20 temperature sensor (as available on the QuinLED Dig-Uno)  
The temperature is displayed both in the Info section of the web UI as well as published to the `/temperature` MQTT topic if enabled.  
This usermod will be expanded with support for different sensor types in the future.

## Installation

Copy `usermod_temperature.h` to the wled00 directory.  
Uncomment the corresponding lines in `usermods_list.cpp` and compile!  
If this is the only v2 usermod you plan to use, you can alternatively replace `usermods_list.h` in wled00 with the one in this folder.

## Project link

* [QuinLED-Dig-Uno](https://quinled.info/2018/09/15/quinled-dig-uno/) - Project link

### PlatformIO requirements

You might have to uncomment `DallasTemperature@~3.8.0`,`OneWire@~2.3.5 under` `[common]` section in `platformio.ini`:

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
