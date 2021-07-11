# Stream data via MQTT
This mod uses the MQTT topic /stream to receive binary color
values in RGBW format. The image is shown for realtimeTimeoutMs
and than replaced with the default values.

## Usermod installation
Activate usermod by adding `-D MQTT_STREAM` to the build flags

## MQTT topics
This usermod listens on `[mqttDeviceTopic]/stream` and 
`[mqttGroupTopic]/stream` for binary data that is shown
on the LED strip itself. The length of the binary data
must be 4*LedCount otherwise the data will be ignored!

## MQTT stream generator
A demo stream generator can be found here: [WLED-MQTTStreamer](https://github.com/DasBasti/WLED-MQTTStreamer)