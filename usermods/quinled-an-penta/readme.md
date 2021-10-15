# QuinLED-An-Penta
The (un)official usermod to get the best out of the QuinLED-An-Penta, like using the OLED and the SHT30 temperature/humidity sensor.

## Requirements
* "U8gs" by olikraus, v2.28 or higher: https://github.com/olikraus/u8g2
* "SHT85" by Rob Tillaart, v0.2 or higher: https://github.com/RobTillaart/SHT85

## Usermod installation
Simply copy the below block (build task) to your `platformio_override.ini` and compile WLED using this new build task. Or use an existing one and add the buildflag `-D QUINLED_AN_PENTA`.

```
[env:custom_esp32dev_usermod_quinled_an_penta]
extends = env:esp32dev
build_flags = ${common.build_flags_esp32} -D WLED_RELEASE_NAME=ESP32 -D QUINLED_AN_PENTA
lib_deps = ${esp32.lib_deps}
    olikraus/U8g2@~2.28.8
    robtillaart/SHT85@~0.2.0
```

## Some words about the (optional) OLED
This mod has been optimized for an SSD1306 driven 128x64 OLED. Using a smaller OLED or an OLED using a different driver will result in unexpected results.
I highly recommend using these "two color monochromatic OLEDs", which have the first 16 pixels in a different color than the other 48, e.g. a yellow/blue OLED.
Also note, you need to have an SPI driven OLED, **not** i2c!

## Configuration
Navigate to the "Config" and then to the "Usermods" section. If you compiled WLED with `-D QUINLED_AN_PENTA`, you will see the config for it there:
* Enable-OLED:
  * What it does: Enabled the optional OLED that can be mounted to the 7-pin female header
  * Possible values: Enabled/Disabled
  * Default: Disabled
* OLED-Use-Progress-Bars:
  * What it does: Toggle between showing percentage numbers or a progress-bar-like visualization for each LED channels brightness level
  * Possible values: Enabled/Disabled
  * Default: Disabled
* Enable-SHT30-Temp-Humidity-Sensor:
  * What it does: Enabled the onboard SHT30 temperature and humidity sensor
  * Possible values: Enabled/Disabled
  * Default: Disabled

## Change log
2021-10
* First implementation.