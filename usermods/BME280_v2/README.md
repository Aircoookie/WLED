Hello! I have written a v2 usermod for the BME280/BMP280 sensor based on the [existing v1 usermod](https://github.com/Aircoookie/WLED/blob/master/usermods/Wemos_D1_mini%2BWemos32_mini_shield/usermod_bme280.cpp). It is not just a refactor, there are many changes which I made to fit my use case, and I hope they will fit the use cases of others as well! Most notably, this usermod is *just* for the BME280 and does not control a display like in the v1 usermod designed for the WeMos shield. 

- Requires libraries `BME280@~3.0.0` (by [finitespace](https://github.com/finitespace/BME280)) and `Wire`. Please add these under `lib_deps` in your `platform.ini` (or `platform_override.ini`).
- Data is published over MQTT so make sure you've enabled the MQTT sync interface.
- This usermod also writes to serial (GPIO1 on ESP8266). Please make sure nothing else listening on the serial TX pin of your board will get confused by log messages!

To enable, compile with `USERMOD_BME280` defined (i.e. `platformio_override.ini`)
```ini
build_flags =
  ${common.build_flags_esp8266}
  -D USERMOD_BME280
```
or define `USERMOD_BME280` in `my_config.h`
```c++
#define USERMOD_BME280
```

Changes include:
- Adjustable measure intervals
  - Temperature and pressure have separate intervals due to pressure not frequently changing at any constant altitude
- Adjustment of number of decimal places in published sensor values
  - Separate adjustment for temperature, humidity and pressure values
  - Values are rounded to the specified number of decimal places
- Pressure measured in units of hPa instead of Pa
- Calculation of heat index (apparent temperature) and dew point
  - These, along with humidity measurements, are disabled if the sensor is a BMP280
- 16x oversampling of sensor during measurement
- Values are only published if they are different from the previous value
- Values are published on startup (continually until the MQTT broker acknowledges a successful publication)

Adjustments are made through preprocessor definitions at the start of the class definition.

MQTT topics are as follows:
Measurement type | MQTT topic
--- | ---
Temperature | `<deviceTopic>/temperature`
Humidity | `<deviceTopic>/humidity`
Pressure | `<deviceTopic>/pressure`
Heat index | `<deviceTopic>/heat_index`
Dew point | `<deviceTopic>/dew_point`