# usermod_v2_HttpPullLightControl

The `usermod_v2_HttpPullLightControl` is a custom user module for WLED that enables remote control over the lighting state and color through HTTP requests. It periodically polls a specified URL to obtain a JSON response containing instructions for controlling individual lights.

## Features

* Configure the URL endpoint (supports both HTTP and HTTPS) and polling interval via the WLED user interface.
* The ability to control the brightness of all lights and the state (on/off) and color of individual lights remotely.
* The ability to control an effect to run and if the effect is already running, it won't restart.
* The ability to control these settings per segment.
* Unique ID generation based on the device's MAC address and a configurable salt value, appended to the request URL for identification.

## Usage

* Configuration
** Enable the `usermod_v2_HttpPullLightControl` via the WLED user interface.
** Specify the URL endpoint and polling interval.

* Request Format:
** The module sends a GET request to the configured URL, appending a unique identifier as a query parameter:
`https://www.example.com/mycustompage.php?id=xxxxxxxx`
where xxxxxxx is a hash of the MAC address combined with a given salt.

* Response Format:
** The URL endpoint should respond with a JSON object containing brightness and lights array. Each object in the lights array represents an individual light and its desired state and color:
`{
  "segments": [
    {
      "segment_id": 0,
      "brightness": 10, // Optional
      "lights": [
        {"index": 1, "state": true, "color": {"r": 100, "g": 0, "b": 0}},
        {"index": 10, "state": true, "color": {"r": 100, "g": 0, "b": 0}},
        {"index": 20, "state": true, "color": {"r": 0, "g": 100, "b": 0}}
        // Meer lights...
      ]
    }
  ]
}
`

** Or use the example below to start an effect.

`{
  "segments": [
    {
      "segment_id": 0,
      "brightness": 20, // Optional
      "effect_id": 28, // Only use the effect_id when you are NOT setting lights, this can not be both!
      "speed": 200,
      "intensity": 150,
      "palette_id": 20,
      "color_slots": [
        {"index": 0, "color": {"r": 100, "g": 0, "b": 0}},
        {"index": 1, "color": {"r": 100, "g": 0, "b": 0}},
        {"index": 2, "color": {"r": 0, "g": 100, "b": 0}}
        // More color_slots are currently not supported (and also not supported by WLED)
      ]
    }
  ]
}
`


## Installation

1. Add `usermod_v2_HttpPullLightControl` to your WLED project following the instructions provided in the WLED documentation.
2. Compile by setting the build_flag: -D USERMOD_HTTP_PULL_LIGHT_CONTROL and upload to your ESP32/ESP8266!
3. There are several compile options which you can put in your platformio.ini or platformio_override.ini:
- -DUSERMOD_HTTP_PULL_LIGHT_CONTROL   ;To Enable the usermod
- -DHTTP_PULL_LIGHT_CONTROL_URL="\"http://mydomain.com/json-response.php\""   ; The URL which will be requested all the time to set the lights/effects
-  -DHTTP_PULL_LIGHT_CONTROL_SALT="\"my_very-S3cret_C0de\""  ; A secret SALT which will help by making the ID more safe
-  -DHTTP_PULL_LIGHT_CONTROL_INTERVAL=30 ; The interval at which the URL is requested in seconds
-  -DHTTP_PULL_LIGHT_CONTROL_HIDE_SALT ; Do you want to Hide the SALT in the User Interface? If yes, Set this flag. Note that the salt can now only be set via the above -DHTTP_PULL_LIGHT_CONTROL_SALT= setting

-  -DWLED_AP_SSID="\"Christmas Card\"" ; These flags are not just for my Usermod but you probably want to set them
-  -DWLED_AP_PASS="\"christmass\""
-  -DWLED_OTA_PASS="\"otapw-secret\""
-  -DMDNS_NAME="\"christmascard\""
-  -DSERVERNAME="\"CHRISTMASCARD\""
-  -D ABL_MILLIAMPS_DEFAULT=450
-  -D DEFAULT_LED_COUNT=60 ; For a LED Ring of 60 LEDs
-  -D BTNPIN=41  ; The M5Stack Atom S3 Lite has a button on GPIO41
-  -D LEDPIN=2 ; The M5Stack Atom S3 Lite has a Grove connector on the front, we use this GPIO2
-  -D STATUSLED=35 ; The M5Stack Atom S3 Lite has a Multi-Color LED on GPIO35, although I didnt managed to control it
-  -D IRPIN=4  ; The M5Stack Atom S3 Lite has a IR LED on GPIO4

-  -D DEBUG=1  ; Set these DEBUG flags ONLY if you want to debug and read out Serial (using Visual Studio Code - Serial Monitor)
-  -DDEBUG_LEVEL=5
-  -DWLED_DEBUG

## Use Case: Interactive Christmas Cards

Imagine distributing interactive Christmas cards embedded with a tiny ESP32 and a string of 20 LEDs to 20 friends. When a friend powers on their card, it connects to their Wi-Fi network and starts polling your server via the `usermod_v2_HttpPullLightControl`. (Tip: Let them scan a QR code to connect to the WLED WiFi, from there they configure their own WiFi).

Your server keeps track of how many cards are active at any given time. If all 20 cards are active, your server instructs each card to light up all of its LEDs. However, if only 4 cards are active, your server instructs each card to light up only 4 LEDs. This creates a real-time interactive experience, symbolizing the collective spirit of the holiday season. Each lit LED represents a friend who's thinking about the others, and the visual feedback creates a sense of connection among the group, despite the physical distance.

This setup demonstrates a unique way to blend traditional holiday sentiments with modern technology, offering an engaging and memorable experience.