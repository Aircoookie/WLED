# SHT
Usermod to support various SHT i2c sensors like the SHT30, SHT31, SHT35 and SHT85

## Requirements
* "SHT85" by Rob Tillaart, v0.2 or higher: https://github.com/RobTillaart/SHT85

## Usermod installation
1. Simply copy the below block (build task) to your `platformio_override.ini` and compile WLED using this new build task. Or use an existing one, add the buildflag `-D USERMOD_SHT` and the below library dependencies.
2. If you're not using thr SHT30 sensor, change the `-D USERMOD_SHT_TYPE_SHT30` build flag to one of these: `USERMOD_SHT_TYPE_SHT31`, `USERMOD_SHT_TYPE_SHT35` or `USERMOD_SHT_TYPE_SHT85`.

ESP32:
```
[env:custom_esp32dev_usermod_sht]
extends = env:esp32dev
build_flags = ${common.build_flags_esp32}
  -D USERMOD_SHT
  -D USERMOD_SHT_TYPE_SHT30
lib_deps = ${esp32.lib_deps}
    robtillaart/SHT85@~0.3.3
```

ESP8266:
```
[env:custom_d1_mini_usermod_quinled_an_penta]
extends = env:d1_mini
build_flags = ${common.build_flags_esp8266}
  -D USERMOD_SHT
  -D USERMOD_SHT_TYPE_SHT30
lib_deps = ${esp8266.lib_deps}
    olikraus/U8g2@~2.28.8
    robtillaart/SHT85@~0.2.0
```

## MQTT Discovery for Home Assistant
If you're using Home Assistant and want to have the temperature and humidity available as entities in HA, you can tick the "Add-To-Home-Assistant-MQTT-Discovery" option in the usermod settings. If you have an MQTT broker configured under "Sync Settings" and it is connected, the mod will publish the auto discovery message to your broker and HA will instantly find it and create an entity each for the temperature and humidity.

### Publishing readings via MQTT
Regardless of having MQTT discovery ticked or not, the mod will always report temperature and humidity to the WLED MQTT topic of that instance, if you have a broker configured and it's connected.

## Change log
2022-12
* First implementation.

## Credits
ezcGman | Andy: Find me on the Intermit.Tech (QuinLED) Discord server: https://discord.gg/WdbAauG