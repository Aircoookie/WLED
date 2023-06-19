# NetCube Systems Universal Light Controller - Battery Management Usermod

This usermod will read status from an IP5306-I2C battery management soc, and monitor the battery voltage via and voltage divider.

## Installation

Copy the example `platformio_override.ini` to the root directory.  This file should be placed in the same directory as `platformio.ini`.

### Define Your Options

* `USERMOD_ULC_BATTERYMANAGEMENT`                      - define this to have this user mod included wled00\usermods_list.cpp