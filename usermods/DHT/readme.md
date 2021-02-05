# DHT Temperature/Humidity sensor usermod

This usermod will read from an attached DHT22 or DHT11 humidity and temperature sensor.
The sensor readings are displayed in the Info section of the web UI.

If sensor is not detected after a while (10 update intervals), this usermod will be disabled.

## Installation

Copy the example `platformio_override.ini` to the root directory.  This file should be placed in the same directory as `platformio.ini`.

### Define Your Options

* `USERMOD_DHT`                      - define this to have this user mod included wled00\usermods_list.cpp
* `USERMOD_DHT_DHTTYPE`              - DHT model: 11, 21, 22 for DHT11, DHT21, or DHT22, defaults to 22/DHT22
* `USERMOD_DHT_PIN`                  - pin to which DTH is connected, defaults to Q2 pin on QuinLed Dig-Uno's board
* `USERMOD_DHT_CELSIUS`              - define this to report temperatures in degrees celsious, otherwise fahrenheit will be reported
* `USERMOD_DHT_MEASUREMENT_INTERVAL` - the number of milliseconds between measurements, defaults to 60 seconds
* `USERMOD_DHT_FIRST_MEASUREMENT_AT` - the number of milliseconds after boot to take first measurement, defaults to 90 seconds
* `USERMOD_DHT_STATS`                - For debug, report delay stats

## Project link

* [QuinLED-Dig-Uno](https://quinled.info/2018/09/15/quinled-dig-uno/) - Project link

### PlatformIO requirements

If you are using `platformio_override.ini`, you should be able to refresh the task list and see your custom task, for example `env:d1_mini_usermod_dht_C`. If not, you can add the libraries and dependencies into `platformio.ini` as you see fit.


## Change Log

2020-02-04
* Change default QuinLed pin to Q2
* Instead of trying to keep updates at constant cadence, space readings out by measurement interval; hope this helps to avoid occasional bursts of readings with errors
* Add some more (optional) stats
2020-02-03
* Due to poor readouts on ESP32 with previous DHT library, rewrote to use https://github.com/alwynallan/DHT_nonblocking
* The new library serializes/delays up to 5ms for the sensor readout  
2020-02-02 
* Created
