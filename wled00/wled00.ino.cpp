# 1 "C:\\Users\\Frank\\AppData\\Local\\Temp\\tmpr9svp7zf"
#include <Arduino.h>
# 1 "D:/ARDUINO_Work/WORK/#ESP32/WLED_soundreactive-GitHub_DEV/WLED-AC_main/WLED_main/wled00/wled00.ino"
# 13 "D:/ARDUINO_Work/WORK/#ESP32/WLED_soundreactive-GitHub_DEV/WLED-AC_main/WLED_main/wled00/wled00.ino"
#include "wled.h"
void setup();
void loop();
#line 15 "D:/ARDUINO_Work/WORK/#ESP32/WLED_soundreactive-GitHub_DEV/WLED-AC_main/WLED_main/wled00/wled00.ino"
void setup() {
  WLED::instance().setup();
}

void loop() {
  WLED::instance().loop();
}