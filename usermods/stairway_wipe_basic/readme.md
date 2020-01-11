### Stairway lighting

Quick usermod to accomplish something similar to [this video](https://www.youtube.com/watch?v=NHkju5ncC4A).

This usermod allows you to add a lightstrip alongside or on the steps of a staircase.
When the `userVar0` variable is set, the LEDs will gradually turn on in a Wipe effect.
Both directions are supported by setting userVar0 to 1 and 2, respectively (HTTP API commands `U0=1` and `U0=2`).

After the Wipe is complete, the light will either stay on (Solid effect) indefinitely or after `userVar1` seconds have elapsed.
If userVar0 is updated (e.g. by triggering a second sensor) the light will slowly fade off.
This could be extended to also run a Wipe effect in reverse order to turn the LEDs back off.

This is just a basic version to accomplish this using HTTP API calls `U0` and `U1` and/or macros.
It should be easy to adapt this code however to interface with motion sensors or other input devices.