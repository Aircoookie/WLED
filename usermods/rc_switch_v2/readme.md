# RC Switch usermod

This usermod will allow to operate remote radio controlled devices.

## Installation 

Copy the example `platformio_override.ini` to the root directory.  This file should be placed in the same directory as `platformio.ini`.

### Define Your Options

* `USERMOD_RC_SWITCH`                - define this to include this user mod wled00\usermods_list.cpp
* `USERMOD_RC_SWITCH_PIN`            - pin to which RC transmitter is connected
* `USERMOD_RC_SWITCH_PULSE_LENGTH`   - RC switch pulse length
* `USERMOD_RC_SWITCH_PROTOCOL`       - RC switch protocol

### Define library

Add rc-switch library of your choide in `lib_deps` in `platformio_override.ini`

### Control

Send commands with MQTT or HTTP request