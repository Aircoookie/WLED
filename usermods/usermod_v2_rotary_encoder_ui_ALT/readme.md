# Rotary Encoder UI Usermod ALT

Thank you to the authors of the original version of these usermods. It would not have been possible without them!
"usermod_v2_four_line_display" (old but gold, removed since release 0.15.0)
"usermod_v2_rotary_encoder_ui" (old but gold, removed since release 0.15.0)

The core of these usermods are a copy of the originals, the changes made to the RotaryEncoder usermod were made to support the new UI in the display usermod.

## Functionalities

Press the encoder to cycle through the options:
* Brightness
* Speed
* Intensity
* Palette
* Effect
* Main Color (only if display is used)
* Saturation (only if display is used)

Press and hold the encoder to display Network Info. If AP is active, it will display the AP, SSID and Password

Also shows if the timer is enabled.

[See the pair of usermods in action](https://www.youtube.com/watch?v=ulZnBt9z3TI)

## Installation

Copy the example `platformio–override.sample.ini` to the root directory of your particular build and rename it to `platformio_override.ini`.

### Define Your Options

* `USERMOD_ROTARY_ENCODER_UI`       - define this to have this user mod included wled00\usermods_list.cpp
* `USERMOD_FOUR_LINE_DISPLAY`       - define this to have this the Four Line Display mod included wled00\usermods_list.cpp
                                        also tells this usermod that the display is available
                                        (see the Four Line Display usermod `readme.md` for more details)
* `ENCODER_DT_PIN`                  - defaults to 18
* `ENCODER_CLK_PIN`                 - defaults to 5
* `ENCODER_SW_PIN`                  - defaults to 19
* `USERMOD_ROTARY_ENCODER_GPIO`     - GPIO functionality:
                                        `INPUT_PULLUP` to use internal pull-up
                                        `INPUT` to use pull-up on the PCB

### PlatformIO requirements

No special requirements.


## Change Log

2021-10
* First public release
