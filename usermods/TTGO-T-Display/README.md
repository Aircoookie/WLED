# TTGO T-Display ESP32 with 240x135 TFT via SPI with TFT_eSPI
This usermod allows use of the TTGO T-Display ESP32 module with integrated 240x135 display
for controlling WLED and showing the following information: 
- Current SSID
- IP address if obtained
  * in AP mode and turned off lightning AP password is shown
- Current effect
- Current palette

Usermod based on a rework of the ssd1306_i2c_oled_u8g2 usermod in WLED repo.

## Hardware
![Hardware](assets/ttgo_hardware1.png)

## Github reference for TTGO-Tdisplay

* [TTGO T-Display](https://github.com/Xinyuan-LilyGO/TTGO-T-Display)

## Requirements
Functionality checked with:
- TTGO T-Display
- PlatformIO
- Group of 4 individual Neopixels from Adafruit

## Platformio Requirements
### platformio.ini changes
Add `TFT_eSPI` dependency to `lib_deps` under `[common]` section in `platformio.ini`:
```ini
# platformio.ini
...
[common]
...
lib_deps =
  ...
  TFT_eSPI
...
```
In order to build for just this board, in `platformio.ini`, also comment out the `default_envs` line in the release binaries section:
```ini
# Release binaries
; default_envs = nodemcuv2, esp01, esp01_1m_ota, esp01_1m_full, esp32dev, custom_WS2801, custom_APA102, custom_LEDPIN_16, custom_LEDPIN_4, custom32_LEDPIN_16 
```
...and uncomment the `default_envs = esp32dev` line in the `Single Boards` section:
```ini
# Single binaries (uncomment your board)
; default_envs = nodemcuv2
; default_envs = esp01
; default_envs = esp01_1m_ota
; default_envs = esp01_1m_full
; default_envs = esp07
; default_envs = d1_mini
; default_envs = heltec_wifi_kit_8
; default_envs = h803wf
; default_envs = d1_mini_debug
; default_envs = d1_mini_ota
default_envs = esp32dev
; default_envs = esp8285_4CH_MagicHome
; default_envs = esp8285_4CH_H801
; default_envs = esp8285_5CH_H801
; default_envs = d1_mini_5CH_Shojo_PCB
; default_envs = wemos_shield_esp32
; default_envs = m5atom
```

### Platformio_overrides.ini (added)
Create a `platformio_overrides.ini` file in the root of your project folder to remap the button pin to the on-board
button to the right of the USB-C connector (with the TTGO marking on top - see hardware photo).  and include the following:
```ini
[env:esp32dev]
build_flags = ${common.build_flags_esp32} 
; PIN defines - uncomment and change, if needed:
;  -D LEDPIN=2
   -D BTNPIN=35
;  -D IR_PIN=4
;  -D RLYPIN=12
;  -D RLYMDE=1
```
### TFT_eSPI Library Adjustments (board selection)
NOTE:  I am relatively new to Platformio and VS Code, but I find that in order to get the project populated with the TFT_eSPI library (so the following changes can be made), I need to attempt an initial build that I know will fail.  There is probably a better way to accomplish this, but it worked for me, so this is why I am documenting it this way.  Once the first build fails, the `User_Setup_Select.h` file can be found in the `/.pio/libdeps/esp32dev/TFT_eSPI_ID1559` folder.

Modify the  `User_Setup_Select.h` file in the `TFT_eSPI` library, as follows:
- Comment out the line:
```ini
//#include <User_Setup.h>           // Default setup is root library folder
```
- Uncomment the line:
```ini
#include <User_Setups/Setup25_TTGO_T_Display.h>    // Setup file for ESP32 and TTGO T-Display ST7789V SPI bus TFT
```

## Arduino IDE
- UNTESTED