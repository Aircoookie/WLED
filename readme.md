WLED is a basic, fast and (relatively) (ok, VERY relatively) secure implementation of a ESP8266 webserver to control Neopixel (WS2812B) leds

Uses ESP8266 Arduino libraries from 15th August 2016! Untested with newer version!
Contents in the /data directory need to be uploaded to SPIFFS.

Features: (V0.2)
- RGB and brightness sliders
- Settings page - configuration over network
- Access Point and station mode - automatic failsafe AP
- Edit page. Change html and other files via OTA.

Additions for V0.3 (nearly complete!)
- WS2812FX library integrated for nearly 50 special effects!
- Nightlight function (gradually dims down)
- Notifier function (multiple ESPs sync color via UDP broadcast)
- Support for power pushbutton
- Full OTA software update capability
- Password protected OTA page for added security (OTA lock)
