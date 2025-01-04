# Sleep Manager Module for WLED

## Overview

The **SleepManager** module for WLED is designed to manage power consumption by enabling the ESP32 to enter deep sleep based on idle time or battery voltage levels. This module allows for more efficient battery use by automatically putting the device to sleep and waking it up based on predefined conditions. It is especially useful for battery-powered setups where minimizing power consumption is essential.

## Features

- **Idle-based Sleep**: Automatically puts the device to sleep after a specified period of inactivity.
- **Battery Voltage Monitoring**: Monitors the battery voltage and enters deep sleep when the voltage falls below a set threshold.
- **Custom GPIO Configuration**: Allows configuration of GPIO pins for controlling power actions (e.g., pull-up or pull-down).
- **Touchpad Wakeup**: Supports waking up the device using a touch sensor (configurable via `WAKEUP_TOUCH_PIN`).
- **Voltage Protection**: Prevents deep sleep if the voltage is too low, protecting the device from shutting down unintentionally.
- **Preset Wakeup**: Optionally allows setting a timer to wake the device up after a set period.

## Configuration

To configure the **SleepManager** module, you will need to add and adjust the following settings in your WLED setup:

### Define GPIO Pins (`CONFIGPINS`)

You can specify a set of GPIO pins for controlling the sleep behavior. The syntax for defining this configuration is:

```cpp
// GPIO NUM,start pull up(1)down(0),end pull up(1)down(0)dis(-1)...
#define CONFIGPINS 0,1,1, 25,0,1, 26,1,0
```


Example `plantformio.ini`:

```
[env:esp32dev_debug]
extends = esp32_idf_V4
board = esp32dev
build_unflags = ${common.build_unflags}
build_flags = ${common.build_flags_esp32_V4} 
  -D USERMOD_SLEEP 
  ; GPIO NUM,start pull up(1)down(0),end pull up(1)down(0)dis(-1)...
  -D CONFIGPINS="25, 0, 1, 26, 0, 1, 27, 1, 0, 0, -1, 1, 4, -1, 0" 
```