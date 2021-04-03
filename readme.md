# HyperSerialWLED
Fork of the WLED project where the Adalight USB serial protocol @115200 speed is replaced with the AWA protocol with data integrity check at @2000000 speed for use with [HyperHDR](https://github.com/awawa-dev/HyperHDR).<br/><br/>
Now it's possible to use HyperSerialWLED with any LED strip/pinout supported by the WLED.<br/>
1 For installation and configuration of WLED please refer to the WLED project: [link](https://github.com/Aircoookie/WLED)<br/>
2 For configuration of HyperHDR please refer to the base project of AWA protocol: [link](https://github.com/awawa-dev/HyperSerialEsp8266)<br/><br/>

<p align="center"> <b>ESP8266:</b><br/><img src="https://i.postimg.cc/CdT7hsG6/esp8266-flashing.jpg"/></p><br/>
<p align="center"> <img src="https://i.postimg.cc/C5fJpQqq/esp8266working.jpg"/></p><br/><br/>

<p align="center"> <b>ESP32:</b><br/><img src="https://i.postimg.cc/dQrq3JrZ/esp32.jpg"/></p><br/>
<p align="center"> <img src="https://i.postimg.cc/1XrhH5rW/esp2.jpg"/></p><br/>

<p align="center"> <b>WLED is receiving data from the USB serial port at @2000000 baud:</b><br/><img src="https://i.postimg.cc/76RXckf4/esp-rec.jpg"/></p><br/>

## ⚙️ Features of WLED 0.12
- WS2812FX library integrated for over 100 special effects  
- FastLED noise effects and 50 palettes  
- Modern UI with color, effect and segment controls  
- Segments to set different effects and colors to parts of the LEDs  
- Settings page - configuration over network  
- Access Point and station mode - automatic failsafe AP  
- Up to 10 LED outputs per instance
- Support for RGBW strips  
- Up to 250 user presets to save and load colors/effects easily, supports cycling through them.  
- Presets can be used to automatically execute API calls  
- Nightlight function (gradually dims down)  
- Full OTA software updatability (HTTP + ArduinoOTA), password protectable  
- Configurable analog clock + support for the Cronixie kit by Diamex  
- Configurable Auto Brightness limit for safer operation  
- Filesystem-based config for easier backup of presets and settings  

# Disclaimer
You use it on your own risk. As per the MIT license, I assume no liability for any damage to you or any other person or equipment.
