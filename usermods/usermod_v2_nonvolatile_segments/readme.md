# NonVolatile Segments

v2 usermod to automatically save and restore segment configures on reboot

* brightness
* colors
* palette
* CCT
* name
* power state

If you have multiple channels and use the `create a segment per channel` featuer
then on every reboot you'll loose any modifications you made to the segments.
This is a workaround to the way the WLED initializes that will restore these
settings and prevent flashing on boot. This will only update the `cfg.json` when
a change has been detected in the above attributes.

## Installation 

Add `-D USEMOD_NV_SEGMENTS` in your `platformio_override.ini` file.
To prevent segments flashing on boot please also add `-D DEFAULT_SEGMENT_OPACITY=1`.
To allow each channel to get to the full brightness add `-D DEFAULT_BRIGHTNESS=255`.

## Build time configuration
* `USERMOD_NV_SEGMENTS_AUTOSAVE_SECONDS` - The default time between saves (defaults to every 15 minutes)
* `USERMOD_NV_SEGMENTS_MAX_AUTOSAVE_SECONDS` - The maximum allowed time between saves (defaults to 24 hours)

## Change Log

2022-05
* First public release