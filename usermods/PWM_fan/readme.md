# PWM fan

v2 Usermod to to control PWM fan with RPM feedback and temperature control

This usermod requires Dallas Temperature usermod to obtain temperature information. If this is not available the fan will always run at 100% speed.
If the fan does not have _tacho_ (RPM) output you can set the _tacho-pin_ to -1 to not use that feature.

You can also set the thershold temperature at which fan runs at lowest speed. If the actual temperature measured will be 3°C greater than threshold temperature the fan will run at 100%.

If the _tacho_ is supported the current speed (in RPM) will be repored in WLED Info page.

## Installation

Add the compile-time option `-D USERMOD_PWM_FAN` to your `platformio.ini` (or `platformio_override.ini`) or use `#define USERMOD_PWM_FAN` in `myconfig.h`.
You will also need `-D USERMOD_DALLASTEMPERATURE`.

### Define Your Options

All of the parameters are configured during run-time using Usermods settings page.
This includes:

* PWM output pin (can be configured at compile time `-D PWM_PIN=xx`)
* tacho input pin (can be configured at compile time `-D TACHO_PIN=xx`)
* sampling frequency in seconds
* threshold temperature in degees C

_NOTE:_ You may also need to tweak Dallas Temperature usermod sampling frequency to match PWM fan sampling frequency.

### PlatformIO requirements

No special requirements.

## Control PWM fan speed using JSON API

You can use e.g. `{"PWM-fan":{"speed":30,"lock":true}}` to set fan speed to 30 percent of maximum speed (replace 30 with arbitrary value between 0 and 100) and lock the speed.
If you include `speed` property you can set fan speed in percent (%) of maximum speed.
If you include `lock` property you can lock (_true_) or unlock (_false_) fan speed.
If the fan speed is unlocked it will revert to temperature controlled speed on next update cycle. Once fan speed is locked it will remain so until it is unlocked by next API call.

## Change Log

2021-10
* First public release
2022-05
* Added JSON API call to allow changing of speed