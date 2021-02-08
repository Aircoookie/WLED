# Auto Save

v2 Usermod automatically to automatically save settings 
to preset AUTOSAVE_PRESET_NUM after a change to any of

* brightness
* effect speed
* effect intensity
* mode (effect)
* palette

happens, but it will wait for SETTINGS_SETTLE_MS 
milliseconds "settle" period in case there are other changes.

It will additionally load preset AUTOSAVE_PRESET_NUM at startup.
The reasoning for this is the brightness isn't set
with the preset that is auto-loaded at startup by WLED
instead it sets to default brightness level. I'd prefer to
use the brightness frmo the preset.

Functionality is enhanced when paired with the
Four Line Display Usermod.

## Installation

Copy and update the example `platformio_override.ini.sample` 
from the Rotary Encoder UI usermode folder to the root directory of your particular build.
This file should be placed in the same directory as `platformio.ini`.

### Define Your Options

* `USERMOD_AUTO_SAVE`   - define this to have this the Auto Save usermod included wled00\usermods_list.cpp
* `USERMOD_FOUR_LINE_DISLAY`   - define this to have this the Four Line Display mod included wled00\usermods_list.cpp - also tells this usermod that the display is available (see the Four Line Display usermod `readme.md` for more details)
* `AUTOSAVE_SETTLE_MS`         - Minimum time to wave before auto saving, defaults to 10000  (10s)
* `AUTOSAVE_PRESET_NUM`        - Preset number to auto-save to, auto-load at startup from, defaults to 99

### PlatformIO requirements

No special requirements.

Note: the Four Line Display usermod requires the libraries `U8g2` and `Wire`.

## Change Log

2021-02
* First public release
