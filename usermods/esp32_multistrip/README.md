# esp32_multistrip

This usermod enables up to 8 data pins to be used from an esp32 module to drive separate LED strands. This only works with one-wire LEDs like the WS2812.

The esp32 RMT hardware is used for data output. See here for hardware driver implementation details: https://github.com/Makuna/NeoPixelBus/wiki/ESP32-NeoMethods#neoesp32rmt-methods

Pass the following variables to the compiler as build flags:

 - `ESP32_MULTISTRIP`
   - Define this to use usermod NpbWrapper.h instead of default one in WLED.
 - `NUM_STRIPS`
   - Number of strips in use
 - `PIXEL_COUNTS`
   - List of pixel counts in each strip
 - `DATA_PINS`
   - List of data pins each strip is attached to. There may be board-specific restrictions on which pins can be used for RTM.

From the perspective of WLED software, the LEDs are addressed as one long strand. The modified NbpWrapper.h file addresses the appropriate strand from the overall LED index based on the number of LEDs defined in each strand.

See `platformio_override.ini` for example configuration.

Tested on low cost ESP-WROOM-32 dev boards from Amazon, such as those sold by KeeYees.
