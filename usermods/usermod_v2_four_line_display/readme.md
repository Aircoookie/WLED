# Rotary Encoder UI Usermod

First, thanks to the authors of the ssd11306_i2c_oled_u8g2 mod.

This usermod provides a four line display using either
128x32 or 128x64 OLED displays.
It's can operate independently, but starts to provide
a relatively complete on-device UI when paired with the 
Rotary Encoder UI usermod. I strongly encourage you to use 
them together.

[See the pair of usermods in action](https://www.youtube.com/watch?v=tITQY80rIOA)

## Installation

Copy and update the example `platformio_override.ini.sample` 
from the Rotary Encoder UI usermode folder to the root directory of your particular build.
This file should be placed in the same directory as `platformio.ini`.

### Define Your Options

* `USERMOD_FOUR_LINE_DISLAY`   - define this to have this the Four Line Display mod included wled00\usermods_list.cpp - also tells Rotary Encoder usermod, if installed, that the display is available
* `FLD_PIN_SCL`                - The display SCL pin, defaults to 5
* `FLD_PIN_SDA`                - The display SDA pin, defaults to 4
* `FLIP_MODE`                  - Set to 0 or 1
* `LINE_HEIGHT`                - Set to 1 or 2

There are other `#define` values in the Usermod that might be of interest.

### PlatformIO requirements

This usermod requires the `U8g2` and `Wire` libraries. See the 
`platformio_override.ini.sample` found in the Rotary Encoder
UI usermod folder for how to include these using `platformio_override.ini`.

## Change Log

2021-02
* First public release
