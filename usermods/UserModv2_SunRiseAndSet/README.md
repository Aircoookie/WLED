WLED v2 UserMod for running macros at sunrise and sunset.

At the time of this text, this user mod requires code to be changed to set certain variables:
    1. To reflect the user's graphical location (latitude/longitude) used for calculating apparent sunrise/sunset
    2. To specify which macros will be run at sunrise and/or sunset. (defaults to 15 at sunrise and 16 at sunset)
    3. To optionally provide an offset from sunrise/sunset, in minutes (max of +/- 2 hours), when the macro will be run.

In addition, WLED must be configured to get time from NTP (and the time must be retrieved via NTP.)

Please open the UserMod_SunRiseAndSet.h file for instructions on what needs to be changed, where to copy files, etc.

If this usermod proves useful enough, the code might eventually be updated to allow prompting for the required information
via the web interface and to store settings in EEPROM instead of hard-coding in the .h file.

This usermod has only been tested on the esp32dev platform, but there's no reason it wouldn't work on other platforms.
