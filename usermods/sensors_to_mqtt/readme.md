# Sensors To Home Assistant (or mqtt)

This usermod will publish values of the BMP280, CCS811 and Si7021 sensors to Home Assistant via MQTT.

Its using home assistant automatic device discovery feature.

The use of Home Assistant is not mandatory; it will publish the sensor values via MQTT just fine without it.

Its resusing the mqtt connection set in the WLED web user interface.

## Maintainer

twitter.com/mpronk89

## Features

- Reads BMP280, CCS811 and Si7021 senors
- Publishes via MQTT, configured via webui of wled
- Announces device in Home Assistant for easy setup
- Efficient energy usage
- Updates every 60 seconds

## Example mqtt topics:

`$mqttDeviceTopic` is set in webui of WLED!

```
temperature: $mqttDeviceTopic/temperature
pressure: $mqttDeviceTopic/pressure
humidity: $mqttDeviceTopic/humidity
tvoc: $mqttDeviceTopic/tvoc
eCO2:  $mqttDeviceTopic/eco2
IAQ:  $mqttDeviceTopic/iaq
```

# Installation

## Hardware

### Requirements

1. BMP280/CCS811/Si7021 sensor. E.g. https://aliexpress.com/item/32979998543.html
2. A microcontroller which can talk i2c, e.g. esp32

### installation

Attach the sensor to the i2c interface.

Default PINs esp32:

```
SCL_PIN = 22;
SDA_PIN = 21;
```

Default PINs ESP8266:

```
SCL_PIN = 5;
SDA_PIN = 4;
```

## Enable in WLED

1. Copy `usermod_v2_SensorsToMqtt.h` into the `wled00` directory.
2. Add to `build_flags` in platformio.ini:

```
  -D USERMOD_SENSORSTOMQTT
```

3. And add to `lib_deps` in platformio.ini:

```
  adafruit/Adafruit BMP280 Library @ 2.1.0
  adafruit/Adafruit CCS811 Library @ 1.0.4
  adafruit/Adafruit Si7021 Library @ 1.4.0
```

The #ifdefs in `usermods_list.cpp` should do the rest :)

# Credits

- Aircoookie for making WLED
- Other usermod creators for example code
- Bouke_Regnerus for https://community.home-assistant.io/t/example-indoor-air-quality-text-sensor-using-ccs811-sensor/125854
- You, for reading this
