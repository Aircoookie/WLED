# Usermod GeoGab

+++ Coded 12/20 by Gabriel Sieben +++

Version 1.0.0

Adds the fuction of four additional relays to WLED. Reacts on MQTT / Alexa / Webinterface / APP 
The code was written for the ESP8266 D1 Mini and may have to be adapted to other devices (e.g. pinout).
I assume no liability for the code.



## PINS

The pins for this usermod are defined in the file usermod_GeoGab.h.

If you want to connect a relay, search for the appropriate circuit on the web. By default, the relays are set to LowActive. To reverse this, there is a corresponding define (GEOGAB_ACTIVEHIGH). Please also note: https://github.com/Aircoookie/WLED/wiki/Control-a-relay-with-WLED

Pinsetting ON a D1 Mini Device: 
    * D0 (GPIO 16): Relay 1
    * D1 (GPIO 5):  Relay 2
    * D2 (GPIO 4):  IR Remote
    * D3 (GPIO 0):  Button / LED Clock (when used)
    * D4 (GPIO 2):  Data / WS2812b Stripe
    * D5 (GPIO 14): DS18B20 / IF Temperature Mod
    * D6 (GPIO 12): Stripe Power Switch Relay / Mosfet circuit (Should be preferred)
    * D7 (GPIO 13): Relay 3
    * D8 (GPIO 15): Relay 4



## Installation

I: Add the corresponding lines in `wled00\usermods_list.cpp` (As found in the example file right in this folder)

II: Add the corresponding Defines in platform_override.ini or platform.ini (As found in the example file right in this folder)

III:
If the Weppage/APP function is to be used:
    Perform the following changes:
    1.      Modify the file: wled00/data/index.hmt
    1.1         ADD New Bottom Line Field: 
                    Search for: `<button class="tablinks" onclick="openTab(3)">"` ...
                    Add line after the found line: `<button class="tablinks" onclick="openTab(4)"><i class="icons">&#xe0bb;</i><p class="tab-label">Relays</p></button>`
                    
                    Search for: `.bot button {`
                    Change value of width to 20%: `width:25%` to `witdth:20%`


    2.      Compile the changes as mentioned in: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality#changing-web-ui
    2.1.    Install the Software: https://nodejs.org/en/download/ (only once)
    2.2.    Run in Command Line: npm install (only once)
>   2.3.    Run in Command Line: npm run build

IV: Change in file "wled00/wled.h" Line "#define ESPALEXA_MAXDEVICES 1" to "#define ESPALEXA_MAXDEVICES 5" (in thes release line 91)



## Define Your Options
Example: build_flags = ${common.build_flags_esp8266} -D USERMOD_GEOGAB -D GEOGAB_HTTP -D GEOGAB_MQTT -D GEOGAB_ALEXA 

define this to ...
* `USERMOD_GEOGAB`                                 - have this user mod included wled00\usermods_list.cpp
* `GEOGAB_ACTIVEHIGH`                              - By default, the relays are set to LowActive. To reverse this, GEOGAB_HIGHACTIVE must be defined. 

Interfaces:
* `GEOGAB_HTTP`                                    - activate GeoGab HTTP Function (For the Webpage please do the webpage/APP change of the install section)
* `GEOGAB_MQTT`                                    - activate GeoGab MQTT Function (comandos via MQTT )
* `GEOGAB_ALEXA`                                   - activate GeoGab AKEXA Function (comandos via ALEXA)   (Minde installation step IV)


## Usage
* Webpage: Use the new page/column `relays`
* MQTT: 
* ALEXA:
* HTTP:


## ToDo's
    * Modify Alexa Names over the Webpage -> read/write into the config file
    * GEOGAB_HTTP
    * GEOGAB_MQTT
    * GEOGAB_ALEXA
    * Maybe: Write a Phyton script which modifies the orginal source accordingly (index.htm + compilation, wled.h (Alexa Max Devices)
