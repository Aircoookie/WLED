# Wifi Status on LED
v2 Usermod to indicate Wifi connection state on LED connected to a GPIO.

Three states are supported:

1. Wifi **disconnected** - indicated by fast blinks of LED
2. Wifi **connected** - LED on with very short off after 2 seconds
3. Wifi in **Access Point mode** - LED blink twice time and pause for 1 second 

## Installation

Copy and update the example `platformio_override.ini.sample` 
from the Rotary Encoder UI usermode folder to the root directory of your particular build.
This file should be placed in the same directory as `platformio.ini`.

### Define Your Options

* `USERMOD_WIFI_STATUS`         - define this to have this usermod included wled00\usermods_list.cpp

Example to add in platformio_override:
  -D USERMOD_WIFI_STATUS

You can also configure plugin parameters (like led pin and enablation) using Usermods settings page.

### PlatformIO requirements

No special requirements.

## Authors
Piotr Jóźwiak [@petrusvr](https://github.com/petrusvr)

## Change Log

2023-11
* First public release
