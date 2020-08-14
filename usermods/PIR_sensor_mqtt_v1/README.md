# PIR sensor with MQTT

This simple usermod allows attaching a PIR sensor like the AM312 and publish the readings over MQTT. A message is sent when motion is detected as well as when motion has stopped.

This usermod has only been tested with the AM312 sensor though should work for any other PIR sensor. Note that this does not control the LED strip directly, it only publishes MQTT readings for use with other integrations like Home Assistant.

## Installation

Copy and replace the file `usermod.cpp` in wled00 directory.
