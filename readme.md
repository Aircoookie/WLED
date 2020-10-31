<p align="center">
  <img src="/images/wled_logo_akemi.png">
  <a href="https://github.com/Aircoookie/WLED/releases"><img src="https://img.shields.io/github/release/Aircoookie/WLED.svg?style=flat-square"></a>
  <a href="https://raw.githubusercontent.com/Aircoookie/WLED/master/LICENSE"><img src="https://img.shields.io/github/license/Aircoookie/wled?color=blue&style=flat-square"></a>
  <a href="https://wled.discourse.group"><img src="https://img.shields.io/discourse/topics?colorB=blue&label=forum&server=https%3A%2F%2Fwled.discourse.group%2F&style=flat-square"></a>
  <a href="https://discord.gg/KuqP7NE"><img src="https://img.shields.io/discord/473448917040758787.svg?colorB=blue&label=discord&style=flat-square"></a>
  <a href="https://github.com/Aircoookie/WLED/wiki"><img src="https://img.shields.io/badge/quick_start-wiki-blue.svg?style=flat-square"></a>
  <a href="https://github.com/Aircoookie/WLED-App"><img src="https://img.shields.io/badge/app-wled-blue.svg?style=flat-square"></a>

  </p>
  
# Welcome to my project WLED! ✨

A fast and feature-rich implementation of an ESP8266/ESP32 webserver to control NeoPixel (WS2812B, WS2811, SK6812, APA102) LEDs or also SPI based chipsets like the WS2801!

## ⚙️ Features
- WS2812FX library integrated for over 100 special effects  
- FastLED noise effects and 50 palettes  
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

## 💡 Supported light control interfaces
- WLED app for [Android](https://play.google.com/store/apps/details?id=com.aircoookie.WLED) and [iOS](https://apps.apple.com/us/app/wled/id1475695033)
- JSON and HTTP request APIs  
- MQTT  
- Blynk IoT  
- E1.31, Art-Net and TPM2.net
- [Hyperion](https://github.com/hyperion-project/hyperion.ng)
- UDP realtime  
- Alexa voice control (including dimming and color)  
- Sync to Philips hue lights  
- Adalight (PC ambilight via serial) and TPM2  
- Sync color of multiple WLED devices (UDP notifier)  
- Infrared remotes (24-key RGB, receiver required)  
- Simple timers/schedules (time from NTP, timezones/DST supported)  

## 📲 Quick start guide and documentation

See the [wiki](https://github.com/Aircoookie/WLED/wiki)!

DrZzs has made some excellent video guides:  
[Introduction, hardware and installation](https://www.youtube.com/watch?v=tXvtxwK3jRk)  
[Settings, tips and tricks](https://www.youtube.com/watch?v=6eCE2BpLaUQ)  

If you'd rather read, here is a very [detailed step-by-step beginner tutorial](https://tynick.com/blog/11-03-2019/getting-started-with-wled-on-esp8266/) by tynick!  

Russian speakers, check out the videos by Room31:
[WLED Firmware Overview: Interface and Settings](https://youtu.be/h7lKsczEI7E)  
[ESP8266 based LED controller for WS2812b strip. WLED Firmware + OpenHAB](https://youtu.be/K4ioTt3XvGc)  

## 🖼️ Images
<img src="/images/macbook-pro-space-gray-on-the-wooden-table.jpg" width="50%"><img src="/images/walking-with-iphone-x.jpg" width="50%">

## 💾 Compatible LED Strips
Type | Voltage | Comments
|---|---|---|
WS2812B | 5v |
WS2813 | 5v | 
SK6812 | 5v | RGBW
APA102 | 5v | C/D
WS2801 | 5v | C/D
LPD8806 | 5v | C/D
TM1814 | 12v | RGBW
WS2811 | 12v | 3-LED segments
WS2815 | 12v | 
GS8208 | 12v |
Analog/non-addressable | any | Requires additional circuitry

## 🧊 Compatible PC RGB Fans and ARGB accessories
Brand | Model | Comments
|---|---|---|
Corsair | HD120 Fan | Uses WS2812B, data-in only
PCCOOLER | Moonlight 5-pack Fans | Uses WS2812B, includes Data-out connector to keep each fan uniquely addressable if wired in series like traditional LED strips
Any | 5v 3-pin ARGB for PC | Any PC RGB device that supports the 5v 3-pin ARGB motherboard header should work fine with WLED. All the major motherboard vendors support the Corsair HD120 and PCCOOLER fans listed, so we can safely assume any device that supports motherboard ARGB 5V 3-Pin standard will work with WLED.


## ✌️ Other

Licensed under the MIT license  
Credits [here](https://github.com/Aircoookie/WLED/wiki/Contributors-&-About)!

Uses Linearicons by Perxis!

Join the Discord server to discuss everything about WLED!

<a href="https://discord.gg/KuqP7NE"><img src="https://discordapp.com/api/guilds/473448917040758787/widget.png?style=banner2" width="25%"></a>

Check out the WLED [Discourse forum](https://wled.discourse.group)!  
You can also send me mails to [dev.aircoookie@gmail.com](mailto:dev.aircoookie@gmail.com), but please only do so if you want to talk to me privately.  
If WLED really brightens up your every day, you can [![](https://img.shields.io/badge/send%20me%20a%20small%20gift-paypal-blue.svg?style=flat-square)](https://paypal.me/aircoookie)


*Disclaimer:*   
If you are sensitive to photosensitive epilepsy it is not recommended that you use this software.  
In case you still want to try, don't use strobe, lighting or noise modes or high effect speed settings.
As per the MIT license, I assume no liability for any damage to you or any other person or equipment.  

