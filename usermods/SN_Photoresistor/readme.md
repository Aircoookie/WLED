# SN_Photoresistor usermod

This usermod will read from an attached photoresistor sensor like the KY-018 sensor.
The luminance is displayed both in the Info section of the web UI as well as published to the `/luminance` MQTT topic if enabled.

## Installation

Copy the example `platformio_override.ini` to the root directory.  This file should be placed in the same directory as `platformio.ini`.

### Define Your Options

* `USERMOD_SN_PHOTORESISTOR`                      - define this to have this user mod included wled00\usermods_list.cpp
* `USERMOD_SN_PHOTORESISTOR_MEASUREMENT_INTERVAL` - the number of milliseconds between measurements, defaults to 60 seconds
* `USERMOD_SN_PHOTORESISTOR_FIRST_MEASUREMENT_AT` - the number of milliseconds after boot to take first measurement, defaults to 20 seconds
* `USERMOD_SN_PHOTORESISTOR_REFERENCE_VOLTAGE`    - the voltage supplied to the sensor, defaults to 5v
* `USERMOD_SN_PHOTORESISTOR_ADC_PRECISION`        - the ADC precision is the number of distinguishable ADC inputs, defaults to 1024.0 (10 bits)
* `USERMOD_SN_PHOTORESISTOR_RESISTOR_VALUE`       - the resistor size, defaults to 10000.0 (10K hms)
* `USERMOD_SN_PHOTORESISTOR_OFFSET_VALUE`         - the offset value to report on, defaults to 25

All parameters can be configured at runtime using Usermods settings page.

## Project link

* [QuinLED-Dig-Uno](https://quinled.info/2018/09/15/quinled-dig-uno/) - Project link

### PlatformIO requirements

If you are using `platformio_override.ini`, you should be able to refresh the task list and see your custom task, for example `env:usermod_sn_photoresistor_d1_mini`.

## Change Log
