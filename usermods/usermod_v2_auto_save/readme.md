# Auto Save

v2 Usermod to automatically save settings 
to preset number AUTOSAVE_PRESET_NUM after a change to any of

* brightness
* effect speed
* effect intensity
* mode (effect)
* palette

but it will wait for AUTOSAVE_SETTLE_MS milliseconds, a "settle" 
period in case there are other changes (any change will 
extend the "settle" window).

It will additionally load preset AUTOSAVE_PRESET_NUM at startup.
during the first `loop()`.  Reasoning below.

AutoSaveUsermod is standalone, but if FourLineDisplayUsermod is installed, it will notify the user of the saved changes.

Note: I don't love that WLED doesn't respect the brightness of the preset being auto loaded, so the AutoSaveUsermod will set the AUTOSAVE_PRESET_NUM preset in the first loop, so brightness IS honored. This means WLED will effectively ignore Default brightness and Apply N preset at boot when the AutoSaveUsermod is installed.

## Installation

Copy and update the example `platformio_override.ini.sample` 
from the Rotary Encoder UI usermode folder to the root directory of your particular build.
This file should be placed in the same directory as `platformio.ini`.

### Define Your Options

* `USERMOD_AUTO_SAVE`   - define this to have this the Auto Save usermod included wled00\usermods_list.cpp
* `USERMOD_FOUR_LINE_DISPLAY`  - define this to have this the Four Line Display mod included wled00\usermods_list.cpp - also tells this usermod that the display is available (see the Four Line Display usermod `readme.md` for more details)

You can configure auto-save parameters using Usermods settings page.

### PlatformIO requirements

No special requirements.

Note: the Four Line Display usermod requires the libraries `U8g2` and `Wire`.

## Change Log

2021-02
* First public release
2021-04
* Adaptation for runtime configuration.