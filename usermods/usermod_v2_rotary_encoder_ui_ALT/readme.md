# Rotary Encoder UI Usermod ALT

Thank you to the authors of the original version of these usermods. It would not have been possible without them!
"usermod_v2_four_line_display"
"usermod_v2_rotary_encoder_ui"

The core of these usermods are a copy of the originals. The main changes are to the FourLineDisplay usermod.
The display usermod UI has been completely changed.


The changes made to the RotaryEncoder usermod were made to support the new UI in the display usermod.
Without the display, it functions identical to the original.
The original "usermod_v2_auto_save" will not work with the display just yet.

Press the encoder to cycle through the options:
* Brightness
* Speed
* Intensity
* Palette
* Effect
* Main Color (only if display is used)
* Saturation (only if display is used)

Press and hold the encoder to display Network Info
    if AP is active, it will display the AP, SSID and Password

Also shows if the timer is enabled.

[See the pair of usermods in action](https://www.youtube.com/watch?v=ulZnBt9z3TI)

## Installation

Copy the example `platformio_override.sample.ini` to the root directory of your particular build and rename it to `platformio_override.ini`.

To activate this alternative usermod, add `#define USE_ALT_DISPlAY` (NOTE: CASE SENSITIVE) to the `usermods_list.cpp` file, or add `-D USE_ALT_DISPlAY` to your `platformio_override.ini` file

### Define Your Options

* `USERMOD_ROTARY_ENCODER_UI`       - define this to have this user mod included wled00\usermods_list.cpp
* `USERMOD_FOUR_LINE_DISPLAY`       - define this to have this the Four Line Display mod included wled00\usermods_list.cpp
                                        also tells this usermod that the display is available
                                        (see the Four Line Display usermod `readme.md` for more details)
* `USE_ALT_DISPlAY`                 - Mandatory to use Four Line Display
* `ENCODER_DT_PIN`                  - defaults to 18
* `ENCODER_CLK_PIN`                 - defaults to 5
* `ENCODER_SW_PIN`                  - defaults to 19
* `USERMOD_ROTARY_ENCODER_GPIO`     - GPIO functionality:
                                        `INPUT_PULLUP` to use internal pull-up
                                        `INPUT` to use pull-up on the PCB

### PlatformIO requirements

Note: the Four Line Display usermod requires the libraries `U8g2` and `Wire`.

## Change Log

2021-10
* First public release
