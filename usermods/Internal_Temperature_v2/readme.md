# MCU Temp Usermod
This usermod adds the temperature readout to the Info tab and also publishes that over the topic `mcutemp` topic.


A shown temp of 53,33°C might indicate that the internal temp is not supported.

ESP8266 does not have a internal temp sensor

ESP32S2 seems to crash on reading the sensor -> disabled

Add a build flag `-D USERMOD_INTERNAL_TEMPERATURE` to your platformio environment.