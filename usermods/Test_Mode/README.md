# Usermod Test Mode
This usermod allows you to load a specific test pattern (brightness, color, effect, palette) by holding down a designated number of buttons.

It was developed for quality control on pre-assembled QuinLED boards with PWM outputs to control "dumb" LEDs.   
To test a digital output you can simply connect it to an addressable LED, which will either work properly or not.  
This doesn't work with analog outputs, because the MOSFET can fail "closed" and thus still light up the LED. The whole dimming range must be tested.

## Installation

Add -D USERMOD_TEST_MODE to your build_flags and compile WLED.

If you want, you can modify the usermod behavior with these additional flags:

| Flag                    | Default Value | Description                                                                                                  |
|-------------------------|---------------|--------------------------------------------------------------------------------------------------------------|
| -D TEST_MODE_BUTTONS    | 2             | Number of buttons to hold down simultaneously to load the test pattern.                                      |
| -D TEST_MODE_BRIGHTNESS | 60            | Brightness to set the LEDs to. Accepted values are 0-255.                                                    |
| -D TEST_MODE_COLOR      | ULTRAWHITE    | Color to set the LEDs to. Accepted values are WLED color macro names or other types WLED accepts (e.g. hex). |
| -D TEST_MODE_EFFECT     | FX_MODE_FADE  | Effect to set the LEDs to. Accepted values are WLED effect macro names or numbers.                           |
| -D TEST_MODE_PALETTE    | 0             | Palette to set the LEDs to. Accepted values are WLED palette numbers.                                        |

## Usage
The usermod is enabled by default. To use it, first configure a number of buttons in WLED that's equal or higher than what TEST_MODE_BUTTONS has been set to (by default, 2). If you want to use this on a production line, you'll most likely want to define the buttons in your compile flags. Once the device is on, simply hold down the buttons and the test mode pattern will load. The usermod will automatically disable itself, but it will keep handling button presses for the current boot until WLED has been restarted. This was done so that the boards can be tested on the assembly line once without risking that the final user accidentally triggers the test pattern during normal use. If you want to load the test pattern again, you can manually re-enable the usermod in the "Usermods" settings page, but it will keep disabling itself after each use.
## Authors
- PaoloTK [@PaoloTK](https://github.com/PaoloTK)
