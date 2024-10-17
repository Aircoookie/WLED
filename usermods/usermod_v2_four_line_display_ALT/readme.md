# I2C/SPI 4 Line Display Usermod ALT

Thank you to the authors of the original version of these usermods. It would not have been possible without them!
"usermod_v2_four_line_display" (old but gold, removed since release 0.15.0)
"usermod_v2_rotary_encoder_ui" (old but gold, removed since release 0.15.0)

The core of these usermods are a copy of the originals, the display usermod UI has been completely changed.

## Functionalities

Press the encoder to cycle through the options:
* Brightness
* Speed
* Intensity
* Palette
* Effect
* Main Color
* Saturation

Press and hold the encoder to display Network Info. If AP is active, it will display the AP, SSID and Password

Also shows if the timer is enabled.

[See the pair of usermods in action](https://www.youtube.com/watch?v=ulZnBt9z3TI)

## Installation

Copy the example `\usermods\usermod_v2_rotary_encoder_ui_ALT\platformio–override.sample.ini` to the root directory of your particular build and rename it to `platformio_override.ini`.

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

## Compatibility

The original "usermod_v2_auto_save" will not work with the display just yet.

## Change Log

2021-10
* First public release
