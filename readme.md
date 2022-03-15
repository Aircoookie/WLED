# HyperSerialWLED
Fork of the WLED project where the Adalight USB serial protocol @115200 speed is replaced with the AWA protocol with data integrity check at @2000000 speed for use with [HyperHDR](https://github.com/awawa-dev/HyperHDR).<br/>

**:warning: WARNING for ESP8266 users: although WLED allows pin-out redefinition, still the only hardware available SPI pins for APA102/SK9822/HD107 are: GPIO13 (MOSI, usually D7) and GPIO14 (SCLK/CLOCK, usually D5) and no software can change it :warning:**

**In the releases you can also find special version working at only 921600 in case if your serial port chip doesn't support 2Mb speed.**  
  
**The project will not follow WLED releases anymore unless there is a serious reason. It's designed to help you to setup your LED strip. Then you should migrate to HyperSerialEsp8266 or HyperSerialESP32 because when choosing fast USB solution you don't need a Wifi. So in this case those projects offer you lower power usage, less electromagnetic interference and more available resources.**  
  
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
- Configurable analog clock (Cronixie, 7-segment and EleksTube IPS clock support via usermods) 
- Configurable Auto Brightness limit for safer operation  
- Filesystem-based config for easier backup of presets and settings  

## 💡 Supported light control interfaces
- WLED app for [Android](https://play.google.com/store/apps/details?id=com.aircoookie.WLED) and [iOS](https://apps.apple.com/us/app/wled/id1475695033)
- JSON and HTTP request APIs  
- MQTT  
- Blynk IoT  
- E1.31, Art-Net, DDP and TPM2.net
- [diyHue](https://github.com/diyhue/diyHue) (Wled is supported by diyHue, including Hue Sync Entertainment under udp. Thanks to [Gregory Mallios](https://github.com/gmallios))
- UDP realtime  
- Alexa voice control (including dimming and color)  
- Sync to Philips hue lights  
- Adalight (PC ambilight via serial) and TPM2  
- Sync color of multiple WLED devices (UDP notifier)  
- Infrared remotes (24-key RGB, receiver required)  
- Simple timers/schedules (time from NTP, timezones/DST supported)  

## 📲 Quick start guide and documentation

See the [documentation on our official site](https://kno.wled.ge)!

[On this page](https://kno.wled.ge/basics/tutorials/) you can find excellent tutorials made by the community and helpful tools to help you get your new lamp up and running!

## 🖼️ User interface
<img src="/images/macbook-pro-space-gray-on-the-wooden-table.jpg" width="50%"><img src="/images/walking-with-iphone-x.jpg" width="50%">

## 💾 Compatible hardware

See [here](https://kno.wled.ge/basics/compatible-hardware)!

## ✌️ Other

Licensed under the MIT license  
Credits [here](https://kno.wled.ge/about/contributors/)!

Join the Discord server to discuss everything about WLED!

<a href="https://discord.gg/KuqP7NE"><img src="https://discordapp.com/api/guilds/473448917040758787/widget.png?style=banner2" width="25%"></a>

Check out the WLED [Discourse forum](https://wled.discourse.group)!  
You can also send me mails to [dev.aircoookie@gmail.com](mailto:dev.aircoookie@gmail.com), but please only do so if you want to talk to me privately.  
If WLED really brightens up your every day, you can [![](https://img.shields.io/badge/send%20me%20a%20small%20gift-paypal-blue.svg?style=flat-square)](https://paypal.me/aircoookie)


*Disclaimer:*   
If you are sensitive to photosensitive epilepsy it is not recommended that you use this software.  
In case you still want to try, don't use strobe, lighting or noise modes or high effect speed settings.
As per the MIT license, I assume no liability for any damage to you or any other person or equipment.  

