# Usermod Distance Staircase
This usermod is a modification of the animated staircase usermod and makes your staircase look cool by illuminating it with an animation. It uses PIR and ultrasonic sensors. At the Top and bottom are PIR sensors to detect entering the staircase.
The ultrasonic sensor detects how far one is through the staircase.

- Light up the steps in the direction you're walking. At the position you are.
- Switch off the steps after you, in the direction of the last detected movement.

## WLED integration
To include this usermod in your WLED setup, you have to be able to [compile WLED from source](https://kno.wled.ge/advanced/compiling-wled/).

Before compiling, you have to make the following modifications:

Edit `usermods_list.cpp`:
1. Open `wled00/usermods_list.cpp`
2. add `#include "../usermods/Animated_Staircase/Animated_Staircase.h"` to the top of the file
3. add `usermods.add(new Animated_Staircase());` to the end of the `void registerUsermods()` function.

Or just add `-D USERMOD_DISTANCE_STAIRCASE` to the build arguments.

You can configure usermod using the Usermods settings page.
Please enter GPIO pins for PIR and ultrasonic sensors (trigger and echo).

## Hardware installation
1. Attach the LED strip to each step of the stairs.
3. Connect the data-out pin at the end of each strip per step to the data-in pin on the 
   next step, creating one large virtual LED strip.
4. Mount PIR sensors at the bottom and top of the stairs and connect them to the ESP.
5. Mount the US sensor in line with the stairs. E.g. at the top looking down.
5. To make sure all LEDs get enough power and have your staircase lighted evenly, power each
   step from one side, using at least AWG14 or 2.5mm^2 cable. Don't connect them serial as you
   do for the datacable!

You _may_ need to use 10k pull-down resistors on the selected PIR pins, depending on the sensor.

## WLED configuration
1. In the WLED UI, configure a segment for each step. The highest step of the stairs is the 
   lowest segment id. 
2. Save your segments into a preset. 
3. Ideally, add the preset in the config > LED setup menu to the "apply 
   preset **n** at boot" setting.

To read the current settings, open a browser to `http://xxx.xxx.xxx.xxx/json/state` (use your WLED 
device IP address). The device will respond with a json object containing all WLED settings. 
The staircase settings and sensor states are inside the WLED "state" element:

```json
{
    "state": {
        "staircase": {
            "enabled": true,
            "bottom-sensor": false,
            "top-sensor": false
        },
}
```

### Enable/disable the usermod
By disabling the usermod you will be able to keep the LED's on, independent from the sensor
activity. This enables you to play with the lights without the usermod switching them on or off.

To disable the usermod:

```bash
curl -X POST -H "Content-Type: application/json" \
     -d {"staircase":{"enabled":false}} \
     xxx.xxx.xxx.xxx/json/state
```

To enable the usermod again, use `"enabled":true`.

Alternatively you can use _Usermod_ Settings page where you can change other parameters as well.

**Please note:** using an HC-SR04 sensor, particularly when detecting echos at longer
distances creates delays in the WLED software, _might_ introduce timing hiccups in your animation or
a less responsive web interface. It is therefore advised to keep the detection distance as short as possible.

**MQTT**
All sensors can be auto detected by Homeassistant and publish their values.
