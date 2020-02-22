![WLED logo](https://raw.githubusercontent.com/Aircoookie/WLED/master/wled_logo.png)   

[![](https://img.shields.io/github/release/Aircoookie/WLED.svg?style=flat-square)](https://github.com/Aircoookie/WLED/releases)
[![](https://img.shields.io/discourse/topics?colorB=blue&label=forum&server=https%3A%2F%2Fwled.discourse.group%2F&style=flat-square)](https://wled.discourse.group)
[![](https://img.shields.io/discord/473448917040758787.svg?colorB=blue&label=discord&style=flat-square)](https://discord.gg/KuqP7NE)
[![](https://img.shields.io/badge/quick_start-wiki-blue.svg?style=flat-square)](https://github.com/Aircoookie/WLED/wiki)
[![](https://img.shields.io/badge/app-wled-blue.svg?style=flat-square)](https://github.com/Aircoookie/WLED-App)

## Welcome to my project WLED!

A fast and feature-rich implementation of an ESP8266/ESP32 webserver to control NeoPixel (WS2812B, WS2811, SK6812, APA102) LEDs!

### Features:
- WS2812FX library integrated for almost 100 special effects  
- FastLED noise effects and palettes  
- Modern UI with color, effect and segment controls  
- Segments to set different effects and colors to parts of the LEDs  
- Settings page - configuration over network  
- Access Point and station mode - automatic failsafe AP  
- Support for RGBW strips  
- 16 user presets to save and load colors/effects easily, supports cycling through them.  
- Macro functions to automatically execute API calls  
- Nightlight function (gradually dims down)  
- Full OTA software updatability (HTTP + ArduinoOTA), password protectable  
- Configurable analog clock + support for the Cronixie kit by Diamex  
- Configurable Auto Brightness limit for safer operation  

### Supported light control interfaces:
- WLED app for Android and iOS  
- JSON and HTTP request APIs  
- MQTT  
- Blynk IoT  
- E1.31  
- Hyperion  
- UDP realtime  
- Alexa voice control (including dimming and color)  
- Sync to Philips hue lights  
- Adalight (PC ambilight via serial)  
- Sync color of multiple WLED devices (UDP notifier)  
- Infrared remotes (24-key RGB, receiver required)  
- Simple timers/schedules (time from NTP, timezones/DST supported)  

### Quick start guide and documentation:

See the [wiki](https://github.com/Aircoookie/WLED/wiki)!

DrZzs has made some excellent video guides:  
[Introduction, hardware and installation](https://www.youtube.com/watch?v=tXvtxwK3jRk)  
[Settings, tips and tricks](https://www.youtube.com/watch?v=6eCE2BpLaUQ)  

If you'd rather read, here is a very [detailed step-by-step beginner tutorial](https://tynick.com/blog/11-03-2019/getting-started-with-wled-on-esp8266/) by tynick!  

### Other

Licensed under the MIT license  
Credits [here](https://github.com/Aircoookie/WLED/wiki/Contributors-&-About)!

Uses Linearicons by Perxis!

Join the Discord [server](https://discord.gg/KuqP7NE) to discuss everything about WLED!  
Check out the WLED [Discourse forum](https://wled.discourse.group)!  
You can also send me mails to [dev.aircoookie@gmail.com](mailto:dev.aircoookie@gmail.com), but please only do so if you want to talk to me privately.  
If WLED really brightens up your every day, you can [![](https://img.shields.io/badge/send%20me%20a%20small%20gift-paypal-blue.svg?style=flat-square)](https://paypal.me/aircoookie)

*Disclaimer:*   
If you are sensitive to photoeleptic seizures it is not recommended that you use this software.  
In case you still want to try, don't use strobe, lighting or noise modes or high effect speed settings.
As per the MIT license, i assume no liability for any damage to you or any other person or equipment.  

