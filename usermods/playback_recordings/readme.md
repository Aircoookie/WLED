# Playback recordings

This usermod can play recorded animations

## ⌨️ Build
- modify `platformio.ini` and add to the `build_flags` of your configuration the following

```
-D USERMOD_PLAYBACK_RECORDINGS
```

_Optionally SD-card:_ Enable the SD card mod (only one at a time)
1. via `-D WLED_USE_SD_MMC` when connected via MMC or
1. via `-D WLED_USE_SD_SPI` when connected via SPI (use usermod page to setup SPI pins)

## ⚙️ Setup a playback of a recording with a preset

- Go to `[WLED-IP]/edit` and upload a recording file to flash memory (LittleFS), e.g. `record.tpm2` or any other file with a supported file format
- The file extension encodes the format used to decode the animation
- Go to `Presets` and create a new preset
- Edit `API Command` and add this to playback the animation on segment 2

```json
{"playback":{"file":"/record.tpm2"}}
```

_Optionally SD-card:_
- ➕ To add an animation to the SD, add it via the SD card on a host computer – there's no WLED interface to do that right now
- 👯‍♀️ If two equally named playbacks are stored on the SD-card and on the flash, the system **will prefer `SD` over `Flash`** stored playback recording paths.

## ⚠️ Mandatory parameters

Every `playback` must contain `file`, so this is a minimal example
 ```json
{"playback":{"file":"/record.tpm2"}}
```

## Optional parameters

### 🔁 Repeats (Bool / Integer)

This example plays `record.tpm2` three times (1 time by default, plus 2 repeatedly)
```json
{"playback":{"file":"/record.tpm2","repeat":2}}
```

| value   | effect               |
| ------- | -------------------- |
| `false` | (default) plays once |
| `true`  | loops forever        |
| `0`     | (default) plays once |
| `1`     | plays two time       |

### 🎬 Framerate / FPS
If the default 25 Fps aren't what you want you can
adjust the framerate to e.g. 30 pictures per second
```json
{"playback":{"file":"/record.tpm2","fps":30}}
```

Adjust the frame to change every 2 seconds to the next picture
```json
{"playback":{"file":"/record.tpm2","fps":0.5}}
```


### 🎯 Segments

This example runs the playback on segment 2

```json
{"playback":{"file":"/record.tpm2","seg":2}}
```
or
```json
{"playback":{"file":"/record.tpm2","seg":{"id":2}}}
```