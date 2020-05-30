# TTGO T-Display ESP32 with 240x135 TFT via SPI with TFT_eSPI
This usermod allows use of the TTGO T-Display ESP32 module with integrated 240x135 display
for controlling WLED and showing the following information: 
* Current SSID
* IP address if obtained
  * in AP mode and turned off lightning AP password is shown
* Current effect
* Current palette

Usermod based on a rework of the ssd1306_i2c_oled_u8g2 usermod from the WLED repo.

## Hardware
![Hardware](assets/ttgo_hardware1.png)

## Github reference for TTGO-Tdisplay

* [TTGO T-Display](https://github.com/Xinyuan-LilyGO/TTGO-T-Display)

## Requirements
Functionality checked with:
* TTGO T-Display
* PlatformIO
* Group of 4 individual Neopixels from Adafruit, and a full string of 68 LEDs.

## Platformio Requirements
### Platformio.ini changes
Under the root folder of the project, in the `platformio.ini` file, uncomment the `TFT_eSPI` line within the [common] section, under `lib_deps`:
```ini
# platformio.ini
...
[common]
...
lib_deps =
    ...
  #For use of the TTGO T-Display ESP32 Module with integrated TFT display uncomment the following line  
    #TFT_eSPI
...
```

### Platformio_overrides.ini (added)
Copy the `platformio_overrides.ini` file which is contained in the `usermods/TTGO-T-Display/` folder into the root of your project folder. This file contains an override that remaps the button pin of WLED to use the on-board button to the right of the USB-C connector (when viewed with the port oriented downward - see hardware photo).

### TFT_eSPI Library Adjustments (board selection)
NOTE:  I am relatively new to Platformio and VS Code, but I find that in order to get the project populated with the TFT_eSPI library (so the following changes can be made), I need to attempt an initial build that I know will fail.  There is probably a better way to accomplish this, but it worked for me.  Once the first build fails, the `User_Setup_Select.h` file can be found in the `/.pio/libdeps/esp32dev/TFT_eSPI_ID1559` folder.

Modify the  `User_Setup_Select.h` file as follows:
* Comment out the following line (which is the 'default' setup file):
```ini
//#include <User_Setup.h>           // Default setup is root library folder
```
* Uncomment the following line (which points to the setup file for the TTGO T-Display):
```ini
#include <User_Setups/Setup25_TTGO_T_Display.h>    // Setup file for ESP32 and TTGO T-Display ST7789V SPI bus TFT
```

Run the build again and it should complete correctly.

## Arduino IDE
- UNTESTED