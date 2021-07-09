# RGB Encoder Board

This usermod-v2 adds support for the awesome RGB Rotary Encoder Board by Adam Zeloof / "Isotope Engineering" to control the overall brightness of your WLED instance: https://github.com/isotope-engineering/RGB-Encoder-Board. A great DIY rotary encoder with 20 tiny SK6805 / "NeoPixel Nano" LEDs.

https://user-images.githubusercontent.com/3090131/124680599-0180ab80-dec7-11eb-9065-a6d08ebe0287.mp4

## Credits
The actual / original code that does the different LED modes is from Adam Zeloof. So I don't take credit for these. But I ported it to WLED, which involved replacing the LED library he used (because, guess what, WLED already has one; so no need to add another one, but use whatever WLED uses), plus the rotary encoder library, because that one was not compatible with ESP, only Arduino.
So it was quite more work than I hoped, but I got there eventually :)

## Requirements
* "ESP Rotary" by Lennart Hennigs, v1.5.0 or higher: https://github.com/LennartHennigs/ESPRotary

## Usermod installation
Simply copy the below block (build task) to your `platformio_override.ini` and compile WLED using this new build task. Or use an existing one and add the buildflag `-D RGB_ROTARY_ENCODER`.

ESP32:
```
[env:custom_esp32dev_usermod_rgb_encoder_board]
extends = env:esp32dev
build_flags = ${common.build_flags_esp32} -D WLED_RELEASE_NAME=ESP32 -D RGB_ROTARY_ENCODER
lib_deps = ${esp32.lib_deps}
    lennarthennigs/ESP Rotary@^1.5.0
```

ESP8266 / D1 Mini:
```
[env:custom_d1_mini_usermod_rgb_encoder_board]
extends = env:d1_mini
build_flags = ${common.build_flags_esp8266} -D RGB_ROTARY_ENCODER
lib_deps = ${esp8266.lib_deps}
    lennarthennigs/ESP Rotary@^1.5.0
```

## How to connect the board to your ESP
We gonna need (minimum) three or (maximum) four GPIOs for the board:
* "ea": Basically tells if the encoder goes into one or the other direction
* "eb": Same thing, but the other direction
* "di": LED data in. To actually control the LEDs
* *(optional)* "sw": The integrated switch in the rotary encoder. Can be omitted for the bare functionality of just controlling the brightness

We also gonna need some power, so:

* "vdd": Needs to be connected to **+5V**.
* "gnd": Well, it's GND.

You can freely pick the GPIOs, it doesn't matter. Those will be configured in the "Usermods" section in the WLED web panel:

## Configuration
Navigate to the "Config" and then to the "Usermods" section. If you compiled WLED with `-D RGB_ROTARY_ENCODER`, you will see the config for it there. The settings there are the GPIOs we mentioned before (*Note: The switch pin is not there, as this can just be configured the "normal" button on the "LED Preferences" page*), plus a few more:
* LED pin:
  * Possible values: Any valid and available GPIO
  * Default: 3
  * What it does: Pin to control the LED ring
* ea pin:
  * Possible values: Any valid and available GPIO
  * Default: 15
  * What it does: First of the two rotary encoder pins
* eb pin:
  * Possible values: Any valid and available GPIO
  * Default: 32
  * What it does: Second of the two rotary encoder pins
* LED Mode:
  * Possible values: 1-3
  * Default: 3
  * What it does: The usermod provides three different modes of how the LEDs can look like. Here's an example: https://github.com/isotope-engineering/RGB-Encoder-Board/blob/master/images/rgb-encoder-animations.gif
    * Up left is "1"
    * Up right is not supported / doesn't make sense for brightness control
    * Bottom left is "2"
    * Bottom right is "3"
* LED Brightness:
  * Possible values: 1-255
  * Default: 64
  * What it does: Brightness of the LED ring
* Steps per click:
  * Possible values: Any positive number
  * Default: 4
  * What it does: With each "click", a rotary encoder actually increments it's "steps". Most rotary encoder do four "steps" per "click". I know this sounds super weird, so just leave this the default value, unless your rotary encoder behaves weirdly, like with one click, it makes two LEDs light up, or you sometimes need two click for one LED. Then you should play around with this value or write a small sketch using the same "ESP Rotary" library and read out the steps it does.
* Increment per click:
  * Possible values: Any positive number
  * Default: 5
  * What it does: Most rotary encoder have 20 "clicks", so basically 20 positions. This value should be set to 100 / `number of clicks`

## Change log
2021-07
* First implementation.
