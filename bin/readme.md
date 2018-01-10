### What binary should I choose?

Currently WLED supports the ESP8266 and a very early, experimental version of ESP32 support.
Be sure to choose the correct binary for your platform and LED type.

- Do you have a standard RGB WS2812B NeoPixel strip?
	--> Use wled05dev_XXXXXXX_RGB_PLATFORM.bin
	
- Do you have an RGBW SK6812 strip (half of the LED is white)?
	--> Use wled05dev_XXXXXXX_RGBW_PLATFORM.bin
	
- Do you have a Cronixie clock set by Diamex?
	--> Use wled05dev_XXXXXXX_CRONIXIE_PLATFORM.bin
	
	
### What about wled03 and wled04?

These are legacy releases only for the ESP8266. They don't include the latest features and may have unfixed bugs - only use them if the new wled05dev test builds don't work for you!