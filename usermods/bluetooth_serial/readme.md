Experimental Bluetooth Control for WLED.

* Allows access to JSON API over Bluetooth Serial Connection.
* Normal WiFi functionality is disabled while Bluetooth is in use
* Bluetooth device shows up as whatever is set in "Server description", "WLED" by default.
* Normal WiFi functionality can be accessed for general WLED setup under the following conditions
  * Normal WiFi functionality until "Apply preset at boot" is set.
  * If active preset changes by any means during first 5 seconds, normal WiFi functionality will resume. (simple button press would suffice)
  * If 'e' is sent over the bluetooth serial connection, normal WiFi functionality will resume.
* Only tested on D1 Mini ESP32 with the following PlatformIO example
* [Serial Bluetooth Terminal](https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal) Android app for testing
* [Python to WLED bluetooth serial reference](https://medium.com/@18218004/devlog-6-bluetooth-and-esp32-ba076a8e207d)








```
[platformio]
default_envs = esp32dev
[env:esp32dev]
board = esp32dev
platform = ${esp32.platform}

platform_packages = ${esp32.platform_packages}
build_unflags = ${common.build_unflags}
build_flags = ${common.build_flags_esp32} 
  -D WLED_RELEASE_NAME=ESP32
  -D WLED_DISABLE_OTA
  -D USERMOD_BLUETOOTH_SERIAL
  #-D WLED_DEBUG
  
lib_deps = ${esp32.lib_deps}
           BluetoothSerial
           
monitor_filters = esp32_exception_decoder
board_build.partitions = tools/WLED_ESP32_NO_OTA.csv
```
