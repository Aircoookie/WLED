# DHT Temperature usermod

Based on the `Temperature Usermod` and DHT Sensor Lib examples
This usermod will read from an attached DHT temperature sensor (DHT22, DHT11 or DHT21)  
The temperature is displayed both in the Info section of the web UI as well as published to the `/temperature` MQTT topic if enabled.

## Installation

Copy the example `platformio_override.ini` to the root directory.  This file should be placed in the same directory as `platformio.ini`.

You might need to change the file `wled00\usermods_list.cpp` to add this usermod as shown in the file `usermods\DHT\usermods_list.cpp`

### TIP

I could only get this to work with the flag `-D DHT_DEBUG` enabled, otherwise the DHT module lib would not load with the usermod.

### Define Your Options

* `USERMOD_DHT`                      - define this to have this user mod included wled00\usermods_list.cpp
* `USERMOD_DHT_DHTTYPE`              - define type of sensor to be used: DHT22, DHT11 or DHT21, default to DHT22
* `USERMOD_DHT_FAHRENHEIT`           - define this to report temperatures in fahrenheit, otherwise degrees celsious will be reported
* `USERMOD_DHT_MEASUREMENT_INTERVAL` - the number of milliseconds between measurements, defaults to 60 seconds
* `USERMOD_DHT_FIRST_MEASUREMENT_AT` - the number of milliseconds after boot to take first measurement, defaults to 20 seconds

### PlatformIO requirements

If you are using `platformio_override.ini`, you should be able to refresh the task list and see your custom task, for example `env:custom32_APA102_dht_debug`.

If you are not using `platformio_override.ini`, you might have to add `adafruit/DHT sensor library@^1.4.1` and `adafruit/Adafruit Unified Sensor@^1.1.4` to lib_deps under `[common]` section in `platformio.ini`:

```ini
# platformio.ini
...
[env]
...
lib_deps =
  ...
  #For DHT sensor uncomment following 2 lines
  adafruit/DHT sensor library@^1.4.1
  adafruit/Adafruit Unified Sensor@^1.1.4

...
```
