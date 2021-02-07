# Rotary Encoder UI Usermod

First, thanks to the authors of other Rotary Encoder usermods.

This usermod starts to provide a relatively complete on-device
UI with paired with the Four Line Display usermod. I strongly
encourage you to use them together.

## Installation

Copy and update the example `platformio_override.ini.sample` to the root directory of your particular build.
This file should be placed in the same directory as `platformio.ini`.

### Define Your Options

* `USERMOD_ROTARY_ENCODER_UI`             - define this to have this user mod included wled00\usermods_list.cpp
* `USERMOD_FOUR_LINE_DISLAY`              - define this to have this the Four Line Display mod included wled00\usermods_list.cpp - also tells this usermod that the display is available
* `ENCODER_DT_PIN`                        - The encoders DT pin, defaults to 12
* `ENCODER_CLK_PIN`                       - The encoders CLK pin, defaults to 14
* `ENCODER_SW_PIN`                        - The encoders SW pin, defaults to 13

### PlatformIO requirements

No special requirements.

Four Line Display requires `U8g2` and `Wire`.

## Change Log

2020-02
* First public release
