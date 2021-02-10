# Usermod PIR Staircase

This usermod makes your staircase look cool:

- When the bottom PIR detects movement, the stairs will light up bottom-to-top
  step by step in the selected color effect.
- When the top PIR detects movement, the stairs will light up top-to-bottom
  step by step in the selected color effect.
- The stairs lights will switch off in the direction of the last PIR detection, and
  will always switch on when one of the PIRs detect movement, even if an effect
  is still running. It can therewith handle multiple people on the stairs gracefully.

## WLED integration

1. Open `wled00/usermods_list.cpp`
2. add `#include "../usermods/PIR_staircase/PIR_staircase.h"` to the top
3. add `usermods.add(new PIR_staircase());` to the end of the `void registerUsermods()` function.
4. Open `usermods/PIR_staircase/PIR_staircase_config.h` 
5. Change the PIR pinnumbers from line 13 and 14 into whatever
   pins are supported by your board.

   Examples:

   Using D7 and D6 pin notation as used on several boards:
   ```
     #define topPIR_PIN    D7
     #define bottomPIR_PIN D6
   ```

   Using GPIO 25 and 26 pins:
   ```
     #define topPIR_PIN    26
     #define bottomPIR_PIN 25
   ```

## Hardware installation
1. Stick the led strip under each step of the stairs.
2. Mount a PIR sensor at the bottom of the stairs and connect it to bottomPIR_PIN.
3. Mount a PIR sensor at the top of the stairs and connect it to topPIR_PIN.

You may need to use 1k pull-down resistors on the selected PIR pins, depending on the sensor.

## WLED configuration
1. In the WLED UI, confgure a segment for each step. The lowest step of the stairs is the 
   lowest segment id. 
2. Save your segments into a preset. 
3. Ideally, add the subsequent preset ìn the config > LED setup menu to the "apply 
   preset `x` at boot" setting.

## Changing settings through API

The PIR Staircase settings can be changed through the api. There are two settings that
can be changed:

| Setting          | Description                                                             |
|------------------|-------------------------------------------------------------------------|
| segment-delay-ms | The delay in milliseconds between turning on steps                      |
| on-time-s        | The number of seconds the stairs stay lit after the last PIR detection. |


To read the settings, open a browser to `http://192.168.0.19/json/state` (where you need to change 
192.168.0.19 into the ip address of your WLED device). The device will respond with a json object
containing all WLED settings. The PIR staircase settings are inside the WLED status element:

```json
{
    "state": {
        "staircase": {
            "enabled": true,
            "segment-delay-ms": 150,
            "on-time-s": 5
        },
}
```


### Changing animation parameters

To change the delay between the steps to 100 milliseconds and the on-time to
10 seconds:

```bash
curl -X POST -H "Content-Type: application/json" \
     -d {"staircase":{"segment-delay-ms":100",on-time-s":10}} \
     192.168.0.19/json/state
```

(where 192.168.0.19 is the address of your WLED device)

**NOTE:** If you're using Windows and want to use the curl commands in this README, 
replace the `\` with a `^` or remove them and put everything on one line.


### Enable/disable the plugin through API

By disabling the plugin you will be able to control the
status of the leds yourself, and play with the WLED settings
without interference of this usermod.

To enable the plugin:

```bash
curl -X POST -H "Content-Type: application/json" \
     -d {"staircase":{"enabled":true}} \
     192.168.0.19/json/state
```

To disable the plugin:

```bash
curl -X POST -H "Content-Type: application/json" \
     -d {"staircase":{"enabled":false}} \
     192.168.0.19/json/state
```

## Replacing a PIR sensor with an ultrasonic HC-SR04 sensor

This usermod can handle both PIR and Ultrasonic sensors for detecting
movement. PIR sensors are the easiest, and the default. Should you want
to use an ultrasonic distance sensor instead of the default PIR sensor
at one or both ends of the stairs, there is a bit more editting involved.
In this example we will replace the _bottom_ PIR sensor with an 
[HC-SR04](https://components101.com/ultrasonic-sensor-working-pinout-datasheet)
ultrasonic sensor.

Comment out the line in `PIR_staircase_config.h` that defines the bottomPIR_PIN, like so:

```
// #define bottomPIR_PIN D6
```

A few lines below that, you'll find two lines defining where the `Trigger`
and `Echo` pins of your HC-SR04 sensor are connected to. Change them to match
your connections:

```
#define bottomTriggerPin D0
#define bottomEchoPin D1
```

Now compile and upload the usermod again (remember that you still need to
add it to `wled00/usermods_list.cpp`, see above).

After uploading it to your ESP8266 or ESP32 board, you'll see a `bottom-echo-us`
setting appear in the json api:

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

If the HC-SR04 sensor detects an echo within 1749 microseconds (within 30 cm from
the sensor), it will trigger switching on the staircase. This setting can be changed
through the API with an HTTP POST:

```bash
curl -X POST -H "Content-Type: application/json" \
     -d {"staircase":{"bottom-echo-us":1166}} \
     192.168.0.19/json/state
```

(where 192.168.0.19 is the ip address of your WLED device)

The speed of sound is 343 meters per second at 20 degress Celcius. Since the sound
has to travel back and forth, the detection distance for the sensor in cm is
(0.0343 * maxTimeUs) / 2. To get you started, please find delays and distances below:

| Distance |	Detection time |
|---------:|----------------:|
|     5 cm |          292 uS |
|    10 cm |          583 uS |
|    20 cm |         1166 uS |
|    30 cm |         1749 uS |
|    50 cm |         2915 uS |
|   100 cm |         5831 uS |

**Please note:** that using an HC-SR04 sensor, particularly when detecting echos at longer
distances creates delays, and might introduce timing hickups in your animations or
a less responsive web interface.

## Triggering through the API

Instead of PIR or Ultrasonic sensors, you can also trigger the animation through
the API. To simulate triggering the bottom sensor, use:

```bash
curl -X POST -H "Content-Type: application/json" \
     -d {"staircase":{"bottomsensor":true}} \
     192.168.0.19/json/state
```

Likewise, to trigger the top sensor, use:

```bash
curl -X POST -H "Content-Type: application/json" \
     -d {"staircase":{"topsensor":true}} \
     192.168.0.19/json/state
```

Have fun with this usermod.<br/>
www.rolfje.com
