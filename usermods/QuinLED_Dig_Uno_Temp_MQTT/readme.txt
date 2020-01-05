These files allow WLED 0.8.6 to report the temp sensor on the Quinled board to MQTT. I use it to report the board temp to Home Assistant via MQTT, so it will send notifications if something happens and the board start to heat up.

This code uses Aircookie's WLED software. It has a premade file for user modifications. I use it to publish the temperature from the dallas temperature sensor on the Quinled board. The entries for the top of the WLED00 file, initializes the required libraries, and variables for the sensor. The .ino file waits for 60 seconds, and checks to see if the MQTT server is connected (thanks Aircoookie). It then poles the sensor, and published it using the MQTT service already running, using the main topic programmed in the WLED UI.

To install:

Add the entries in the WLED00 file to the top of the same file from Aircoookies WLED.
Replace the WLED06_usermod.ino file in Aircoookies WLED folder.
