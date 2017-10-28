WLED is a basic, fast and (relatively) secure implementation of a ESP8266 webserver to control Neopixel (WS2812B) leds

Uses ESP arduino version 2.3.0 (latest as of October 2017).
Contents in the /data directory may be uploaded to SPIFFS.

Features: (V0.4)
- RGB, HSB, and brightness sliders
- Settings page - configuration over network
- Access Point and station mode - automatic failsafe AP
- WS2812FX library integrated for nearly 50 special effects!
- Support for RGBW strips
- Nightlight function (gradually dims down)
- Notifier function (multiple ESPs sync color via UDP broadcast)
- Support for power pushbutton
- Full OTA software update capability
- Password protected OTA page for added security (OTA lock)
- Alexa smart home device server
- (unstable) NTP and experimental analog clock function
- client HTML controlled
- Edit page. Change html and other files via OTA. (needs to be enabled in source)

Compile settings:
Board: WeMos D1 mini (untested with other HW, should work though)
CPU frequency: 80 MHz
Flash size : 4MB (3MB SPIFFS)
Upload speed: 921600


Quick start guide:

-If you do not plan to change the software, you can use the supplied binary files instead-
Just flash a basic HTTP OTA updater sketch and upload the bin!


1. Make sure your ESP module has a min. 4MB SPI flash module. (SHOULD now support 1MB modules, untested)
Connect a  WS2812B RGB led strip to GPIO2. Optionally connect a NO-pushbutton to GPIO0 (internal pull-up) and ground.

2. Follow a guide to setup your Arduino client (I am using version 1.8.1) with the ESP8266 libraries.
For current compiles I use version 2.3.0.

3. In file "wled00.ino", you should change the access point and OTA update passphrases for added security (you can change them later, this is just the "factory default"). Flash the sketch.

5. Connect to automatically started WiFi access point "WLED-AP" using default passwort "wled1234". Go to the IP "192.168.4.1".

6. Click on the cog icon to edit settings like connecting the module to your home WiFi.

7. Have fun with the software!


Advanced module control via HTTP requests:

Base URL scheme: "<moduleip>/win". This will return a XML file with some current values.
Add one or multiple of the following parameters after the base url to change values:
"&A=<0-255>" set LED brightness (yellow slider)
"&R=<0-255>" set LED red value (red slider)
"&G=<0-255>" set LED green value (green slider)
"&B=<0-255>" set LED blue value (blue slider)
"&W=<0-255>" set LED white value (white slider) (only when RGBW enabled)
"&T=<0 or 1 or 2-255>" 0: switch off, on, toggle
"&FX=<0-47>" set LED effect (refer to WS2812FX library)
"&SX=<0-255>" set LED effect speed (refer to WS2812FX library)
"&RN=<0 or 1>" receive notifications on or off
"&SN=<0 or 1>" send (direct) notifications on or off
"&NL=<0 or 1>" turns nightlight function on or off
"&MD=<0 or 1>" sets client color picker mode (RGB/HSB)
("&OL=<0, 1, 3 or 5>" experimental clock overlays)
("&I=<0-255>" experimental individual LED control)
("&I=<0-255>&I2=<0-255>" experimental individual LED range control)


Licensed under the MIT license 
Uses libraries: 
ESP8266 Arduino Core 
WS2812FX by kitesurfer1404 (Aircoookie fork) 
Timezone library by JChristensen
arduino-esp8266-alexa-multiple-wemo-switch by kakopappa


Software update procedure:

Method 1: Reflash the new update source via USB.

Method 2: The software has an integrated OTA software update capability.
First you have to enable it by typing in the correct OTA passphrase (default: "wledota") in the settings menu.
Remove the tick in the checkbox "OTA locked". Then save settings and reboot the ESP.
Now you can go to "<moduleip>/update" to update binary firmware.
To edit flash content (images and HTML), go to "<moduleip>/edit". (only if USEFS was uncommented in source)
After you are done, it is recommended to lock the OTA function again.
To do so, tick the checkbox again (you can change the passphrase by typing in a new one now). Reboot.
If you try to access the update page now, you should see the message "OTA lock active".


The software now supports audio-reactive-led-strip!

1. Download [audio-reactive-led-strip](https://github.com/scottlawsonbc/audio-reactive-led-strip) and follow its installation instruction. Use python 3!
2. Insert the following code in led.py after line 66:
    m.append(1);
    m.append(2);
3. In config.py set your led amount, ESP IP and WLED UDP notifier port. For FPS, a setting between 15-30 is recommended.
4. Run visualization.py! If you have a low amount of LEDS (e.g. 10) try lowering the sigma values in line 129-131.
5. If you have multiple WLED devices, you can sync them all with music.
Use the led count of your largest device and set the IP to X.X.X.255 (UDP broadcast).
You can adjust the position of the amplitude with the WARLS offset setting.
Note that there is currently an issue preventing you from accessing the control web page while the script is running. HTTP requests work.


Uses [WS2812FX](https://github.com/kitesurfer1404/WS2812FX)!






