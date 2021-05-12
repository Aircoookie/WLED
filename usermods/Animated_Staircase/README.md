# Usermod Animated Staircase
This usermod makes your staircase look cool by switching it on with an animation. It uses
PIR or ultrasonic sensors at the top and bottom of your stairs to:

- Light up the steps in your walking direction, leading the way.
- Switch off the steps after you, in the direction of the last detected movement.
- Always switch on when one of the sensors detects movement, even if an effect
  is still running. It can therewith handle multiple people on the stairs gracefully.

The Animated Staircase can be controlled by the WLED API. Change settings such as
speed, on/off time and distance settings by sending an HTTP request, see below.

## WLED integration
To include this usermod in your WLED setup, you have to be able to [compile WLED from source](https://github.com/Aircoookie/WLED/wiki/Compiling-WLED).

Before compiling, you have to make the following modifications:

Edit `usermods_list.cpp`:
1. Open `wled00/usermods_list.cpp`
2. add `#include "../usermods/Animated_Staircase/Animated_Staircase.h"` to the top of the file
3. add `usermods.add(new Animated_Staircase());` to the end of the `void registerUsermods()` function.

You can configure usermod using Usermods settings page.
Please enter GPIO pins for PIR sensors or ultrasonic sensor (trigger and echo).
If you use PIR sensor enter -1 for echo pin.
Maximum distance for ultrasonic sensor can be configured as a time needed for echo (see below).

## Hardware installation
1. Stick the LED strip under each step of the stairs.
2. Connect the ESP8266 pin D4 or ESP32 pin D2 to the first LED data pin at the bottom step
   of your stairs.
3. Connect the data-out pin at the end of each strip per step to the data-in pin on the 
   other end of the next step, creating one large virtual LED strip.
4. Mount sensors of choice at the bottom and top of the stairs and connect them to the ESP.
5. To make sure all LEDs get enough power and have your staircase lighted evenly, power each
   step from one side, using at least AWG14 or 2.5mm^2 cable. Don't connect them serial as you
   do for the datacable!

You _may_ need to use 10k pull-down resistors on the selected PIR pins, depending on the sensor.

## WLED configuration
1. In the WLED UI, confgure a segment for each step. The lowest step of the stairs is the 
   lowest segment id. 
2. Save your segments into a preset. 
3. Ideally, add the preset in the config > LED setup menu to the "apply 
   preset **n** at boot" setting.

## Changing behavior through API
The Staircase settings can be changed through the WLED JSON api.

**NOTE:** We are using [curl](https://curl.se/) to send HTTP POSTs to the WLED API.
If you're using Windows and want to use the curl commands, replace the `\` with a `^`
or remove them and put everything on one line.


| Setting          | Description                                                   | Default |
|------------------|---------------------------------------------------------------|---------|
| enabled          | Enable or disable the usermod                                 | true    |
| segment-delay-ms | Delay (milliseconds) between switching on/off each step       | 150     |
| on-time-s        | Time (seconds) the stairs stay lit after last detection       | 5       |
| bottom-echo-us   | Detection range of ultrasonic sensor                          | 1749    |
| bottom-sensor    | Manually trigger a down to up animation via API               | false   | 
| top-sensor       | Manually trigger an up to down animation via API              | false   |


To read the current settings, open a browser to `http://xxx.xxx.xxx.xxx/json/state` (use your WLED 
device IP address). The device will respond with a json object containing all WLED settings. 
The staircase settings and sensor states are inside the WLED status element:

```json
{
    "state": {
        "staircase": {
            "enabled": true,
            "segment-delay-ms": 150,
            "on-time-s": 5,
            "bottom-sensor": false,
            "tops-ensor": false
        },
}
```

### Enable/disable the usermod
By disabling the usermod you will be able to keep the LED's on, independent from the sensor
activity. This enables to play with the lights without the usermod switching them on or off.

To disable the usermod:

```bash
curl -X POST -H "Content-Type: application/json" \
     -d {"staircase":{"enabled":false}} \
     xxx.xxx.xxx.xxx/json/state
```

To enable the usermod again, use `"enabled":true`.

### Changing animation parameters
To change the delay between the steps to (for example) 100 milliseconds and the on-time to
10 seconds:

```bash
curl -X POST -H "Content-Type: application/json" \
     -d '{"staircase":{"segment-delay-ms":100,"on-time-s":10}}' \
     xxx.xxx.xxx.xxx/json/state
```

### Changing detection range of the ultrasonic HC-SR04 sensor
When an ultrasonic sensor is enabled in `Animated_Staircase_config.h`, you'll see a 
`bottom-echo-us` setting appear in the json api:

```json
{
    "state": {
        "staircase": {
            "enabled": true,
            "segment-delay-ms": 150,
            "on-time-s": 5,
            "bottom-echo-us": 1749
        },
}
```

If the HC-SR04 sensor detects an echo within 1749 microseconds (corresponding to ~30 cm 
detection range from the sensor), it will trigger switching on the staircase. This setting 
can be changed through the API with an HTTP POST:

```bash
curl -X POST -H "Content-Type: application/json" \
     -d '{"staircase":{"bottom-echo-us":1166}}' \
     xxx.xxx.xxx.xxx/json/state
```

Calculating the detection range can be performed as follows: The speed of sound is 343m/s at 20 
degrees Centigrade. Since the sound has to travel back and forth, the detection range for the
sensor in cm is (0.0343 * maxTimeUs) / 2. To get you started, please find delays and distances below:

| Distance | Detection time  |
|---------:|----------------:|
|     5 cm |          292 uS |
|    10 cm |          583 uS |
|    20 cm |         1166 uS |
|    30 cm |         1749 uS |
|    50 cm |         2915 uS |
|   100 cm |         5831 uS |

**Please note:** that using an HC-SR04 sensor, particularly when detecting echos at longer
distances creates delays in the WLED software, and _might_ introduce timing hickups in your animations or
a less responsive web interface. It is therefore advised to keep the detection time as short as possible.

### Animation triggering through the API
Instead of stairs activation by one of the sensors, you can also trigger the animation through
the API. To simulate triggering the bottom sensor, use:

```bash
curl -X POST -H "Content-Type: application/json" \
     -d '{"staircase":{"bottom-sensor":true}}' \
     xxx.xxx.xxx.xxx/json/state
```

Likewise, to trigger the top sensor, use:

```bash
curl -X POST -H "Content-Type: application/json" \
     -d '{"staircase":{"top-sensor":true}}' \
     xxx.xxx.xxx.xxx/json/state
```

Have fun with this usermod.<br/>
www.rolfje.com

## Change log
2021-04
* Adaptation for runtime configuration.