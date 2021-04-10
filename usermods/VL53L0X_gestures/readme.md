# Description

That usermod implements support of simple hand gestures with VL53L0X sensor: on/off and brightness correction.
It can be useful for kitchen strips to avoid any touches.
 - on/off - just swipe a hand below your sensor ("shortPressAction" is called and can be customized through WLED macros)
 - brightness correction - keep your hand below sensor for 1 second to switch to "brightness" mode.
                           Configure brightness by changing distance to the sensor (see parameters below for customization).
                           "macroLongPress" is also called here.

## Installation 

1. Attach VL53L0X sensor to i2c pins according to default pins for your board.
2. Add `-D USERMOD_VL53L0X_GESTURES` to your build flags at platformio.ini (plaformio_override.ini) for needed environment.
In my case, for example: `build_flags = ${common.build_flags_esp8266} -D RLYPIN=12 -D USERMOD_VL53L0X_GESTURES`
3. Add "pololu/VL53L0X" dependency below to `lib_deps` like this:
```ini
lib_deps = ${env.lib_deps}
      pololu/VL53L0X @ ^1.3.0
```

My entire `platformio_override.ini` for example (for nodemcu board):
```ini
[platformio]
default_envs = nodemcu

[env:nodemcu]
board = nodemcu
platform = ${common.platform_wled_default}
platform_packages = ${common.platform_packages}
board_build.ldscript = ${common.ldscript_4m1m}
build_unflags = ${common.build_unflags}
build_flags = ${common.build_flags_esp8266} -D RLYPIN=12 -D USERMOD_VL53L0X_GESTURES
lib_deps = ${env.lib_deps}
  pololu/VL53L0X @ ^1.3.0
```