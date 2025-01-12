# Update Brightness Follow Sun


## Installation

define `USERMOD_BRIGHTNESS_FOLLOW_SUN` e.g.

`#define USERMOD_BRIGHTNESS_FOLLOW_SUN` in my_config.h

or add `-D USERMOD_BRIGHTNESS_FOLLOW_SUN` to `build_flags` in platformio_override.ini

### Define Your Options

Open Usermod Settings in WLED to change settings:

`Update Interval(sec)` - update interval for change brightness
`Min Brightness` - set brightness by map of min-max-min : sunrise-suntop-sunset
`Max Brightness` - as as
`Relax Hour` - before sunrise and after sunset, maintain the min brightness for several hours


### PlatformIO requirements

No special requirements.

## Change Log

2025-01-02
* init
