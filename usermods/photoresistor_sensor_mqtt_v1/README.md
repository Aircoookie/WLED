# Photoresister sensor with MQTT

This simple usermod allows attaching a photoresistor sensor like the KY-018 and publish the readings in percentage over MQTT. The frequency of MQTT messages can be modified, and there is a threshold value that can be set so that significant changes in the readings can be published immediately instead of waiting for the next update. This was found to be a good compromise between spamming MQTT messages and delayed updates.

I also found it useful to limit the frequency of analog pin reads because otherwise the board hangs.

This usermod has only been tested with the KY-018 sensor though should work for any other analog pin sensor. Note that this does not control the LED strip directly, it only publishes MQTT readings for use with other integrations like Home Assistant.

## Installation

Copy and replace the file `usermod.cpp` in wled00 directory.
