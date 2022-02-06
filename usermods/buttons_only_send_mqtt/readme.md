# Usermod only send mqtt message on button and PIR

This user mod will only send mqtt messages on button presses (short, long, double) and
PIR motion. It will not attempt to turn on or off the LEDs or apply any presets. 
To send double button presses it is necessary to set a non-zero value for the `double`
action for the button on the Time & Macros under the Button actions section.

The motivation for this usermod was so that I could use buttons or PIR in Home
Assistant without it changing anything or attempting to change the LED states or
apply a preset. I think this might be possible by setting unconfigured macros for the
button actions but then any presses or motion cause a pop up notification on the WLED
page that a preset failed.

## Installation 

Add `#define USERMOD_BUTTONS_SEND_MQTT` in `my_config.h`

