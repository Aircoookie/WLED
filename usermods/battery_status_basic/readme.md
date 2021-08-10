# Battery status/level Usermod

This Usermod allows you to monitor the battery level of your battery powered project.

You can see the battery level in the `info modal` right under the `estimated current`. 

For this to work the positive side of the (18650) battery must be connected to pin `A0` of the d1mini/esp8266 with a 100k ohm resistor (see [Useful Links](#useful-links)).

<p align="center">
  <img width="400" src="assets/usermod_battery_level_info_modal.png">
</p>

## Installation

define `USERMOD_BATTERY_STATUS_BASIC` in `my_config.h`

### Define Your Options

* `USERMOD_BATTERY_STATUS_BASIC`                   - define this (in `my_config.h`) to have this user mod included wled00\usermods_list.cpp
* `BATTERY_MEASUREMENT_PIN`                        - defaults to A0 on esp8266 and esp32
* `USERMOD_BATTERY_MEASUREMENT_INTERVAL`           - the frequency to check the battery, defaults to 30 seconds
* `USERMOD_BATTERY_MIN_VOLTAGE`                    - minimum voltage of the Battery used, default is 3.6 (18650 battery standard)
* `USERMOD_BATTERY_MAX_VOLTAGE`                    - maximum voltage of the Battery used, default is 4.2 (18650 battery standard)

Currently the parameters cannot be changed during runtime, this 'feature' will be added soon.

## Useful Links
* https://lazyzero.de/elektronik/esp8266/wemos_d1_mini_a0/start
* https://arduinodiy.wordpress.com/2016/12/25/monitoring-lipo-battery-voltage-with-wemos-d1-minibattery-shield-and-thingspeak/

## Change Log

2021-08-10
* Created
