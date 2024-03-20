# I2C/SPI 4 Line Display Usermod ALT

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

Press and hold the encoder to display Network Info. If AP is active, it will display AP, SSID and password

Also shows if the timer is enabled

[See the pair of usermods in action](https://www.youtube.com/watch?v=ulZnBt9z3TI)

## Installation

Please refer to the original `usermod_v2_rotary_encoder_ui` readme for the main instructions.

Copy the example `platformio_override.sample.ini` from the usermod_v2_rotary_encoder_ui_ALT folder to the root directory of your particular build and rename it to `platformio_override.ini`.

This file should be placed in the same directory as `platformio.ini`.

Then, to activate this alternative usermod, add `#define USE_ALT_DISPlAY` (NOTE: CASE SENSITIVE) to the `usermods_list.cpp` file,
                                        or add `-D USE_ALT_DISPlAY` to the original `platformio_override.ini.sample` file


## Configuration

These options are configurable in Config > Usermods

### Usermod Setup

* Global I2C GPIOs (HW) - Set the SDA and SCL pins

### 4LineDisplay

* `enabled` - enable/disable usermod
* `type` - display type in numeric format
    * 1 = I2C SSD1306 128x32
    * 2 = I2C SH1106 128x32
    * 3 = I2C SSD1306 128x64 (4 double-height lines)
    * 4 = I2C SSD1305 128x32
    * 5 = I2C SSD1305 128x64 (4 double-height lines)
    * 6 = SPI SSD1306 128x32
    * 7 = SPI SSD1306 128x64 (4 double-height lines)
    * 8 = SPI SSD1309 128x64 (4 double-height lines)
    * 9 = I2C SSD1309 128x64 (4 double-height lines)
* `pin` - GPIO pins used for display; SPI displays can use SCK, MOSI, CS, DC & RST
* `flip` - flip/rotate display 180°
* `contrast` - set display contrast (higher contrast may reduce display lifetime)
* `screenTimeOutSec` - screen saver time-out in seconds
* `sleepMode` - enable/disable screen saver
* `clockMode` - enable/disable clock display in screen saver mode
* `showSeconds` - Show seconds on the clock display
* `i2c-freq-kHz` - I2C clock frequency in kHz (may help reduce dropped frames, range: 400-3400)


### PlatformIO requirements

Note: the Four Line Display usermod requires the libraries `U8g2` and `Wire`.

## Change Log

2021-10
* First public release
