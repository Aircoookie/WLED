# Temperature usermod

Based on the excellent `QuinLED_Dig_Uno_Temp_MQTT` by srg74 and 400killer!  
This usermod will read from an attached DS18B20 temperature sensor (as available on the QuinLED Dig-Uno)  
The temperature is displayed both in the Info section of the web UI as well as published to the `/temperature` MQTT topic if enabled.  
This usermod will be expanded with support for different sensor types in the future.

If temperature sensor is not detected during boot, this usermod will be disabled.

## Installation

Copy the example `platformio_override.ini` to the root directory.  This file should be placed in the same directory as `platformio.ini`.

### Define Your Options

* `USERMOD_DALLASTEMPERATURE`                      - define this to have this user mod included wled00\usermods_list.cpp
* `USERMOD_DALLASTEMPERATURE_CELSIUS`              - define this to report temperatures in degrees celsious, otherwise fahrenheit will be reported
* `USERMOD_DALLASTEMPERATURE_MEASUREMENT_INTERVAL` - the number of milliseconds between measurements, defaults to 60 seconds
* `USERMOD_DALLASTEMPERATURE_FIRST_MEASUREMENT_AT` - the number of milliseconds after boot to take first measurement, defaults to 20 seconds

## Project link

* [QuinLED-Dig-Uno](https://quinled.info/2018/09/15/quinled-dig-uno/) - Project link

### PlatformIO requirements

If you are using `platformio_override.ini`, you should be able to refresh the task list and see your custom task, for example `env:d1_mini_usermod_dallas_temperature_C`.


If you are not using `platformio_override.ini`, you might have to uncomment `DallasTemperature@~3.8.0`,`OneWire@~2.3.5 under` `[common]` section in `platformio.ini`:

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

## Change Log

2020-09-12 
* Changed to use async, non-blocking implementation
* Do not report low temperatures that indicate an error to mqtt
* Disable plugin if temperature sensor not detected
* Report the number of seconds until the first read in the info screen instead of sensor error