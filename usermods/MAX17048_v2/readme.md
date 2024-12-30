# Adafruit MAX17048 Usermod (LiPo & LiIon Battery Monitor & Fuel Gauge)
This usermod reads information from an Adafruit MAX17048  and outputs the following:
- Battery Voltage
- Battery Level Percentage


## Dependencies
Libraries:
- `Adafruit_BusIO@~1.14.5` (by [adafruit](https://github.com/adafruit/Adafruit_BusIO))
- `Adafruit_MAX1704X@~1.0.2` (by [adafruit](https://github.com/adafruit/Adafruit_MAX1704X))

These must be added under `lib_deps` in your `platform.ini` (or `platform_override.ini`).
Data is published over MQTT - make sure you've enabled the MQTT sync interface.

## Compilation

To enable, compile with `USERMOD_MAX17048` define in the build_flags (e.g. in `platformio.ini` or `platformio_override.ini`) such as in the example below:
```ini
[env:usermod_max17048_d1_mini]
extends = env:d1_mini
build_flags =
  ${common.build_flags_esp8266}
  -D USERMOD_MAX17048
lib_deps = 
  ${esp8266.lib_deps}
  https://github.com/adafruit/Adafruit_BusIO @ 1.14.5
  https://github.com/adafruit/Adafruit_MAX1704X @ 1.0.2
```

### Configuration Options
The following settings can be set at compile-time but are configurable on the usermod menu (except First Monitor time):
- USERMOD_MAX17048_MIN_MONITOR_INTERVAL (the min number of milliseconds between checks, defaults to 10,000 ms)
- USERMOD_MAX17048_MAX_MONITOR_INTERVAL (the max number of milliseconds between checks, defaults to 10,000 ms)
- USERMOD_MAX17048_FIRST_MONITOR_AT


Additionally, the Usermod Menu allows you to:
- Enable or Disable the usermod
- Enable or Disable Home Assistant Discovery (turn on/off to sent MQTT Discovery entries for Home Assistant)
- Configure SCL/SDA GPIO Pins

## API
The following method is available to interact with the usermod from other code modules:
- `getBatteryVoltageV` read the last battery voltage (in Volt) obtained from the sensor
- `getBatteryPercent` reads the last battery percentage obtained from the sensor

## MQTT
MQTT topics are as follows (`<deviceTopic>` is set in MQTT section of Sync Setup menu):
Measurement type | MQTT topic
--- | ---
Battery Voltage | `<deviceTopic>/batteryVoltage`
Battery Percent | `<deviceTopic>/batteryPercent`

## Authors
Carlos Cruz [@ccruz09](https://github.com/ccruz09)


## Revision History
Jan 2024
- Added Home Assistant Discovery
- Implemented PinManager to register pins
- Added API call for other modules to read battery voltage and percentage
- Added info-screen outputs
- Updated `readme.md`