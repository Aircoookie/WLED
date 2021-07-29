# 1 "/tmp/tmpykvtn0n2"
#include <Arduino.h>
# 1 "/home/thomas/Documents/PlatformIO/Projects/wled2/WLED/wled00/wled00.ino"
# 13 "/home/thomas/Documents/PlatformIO/Projects/wled2/WLED/wled00/wled00.ino"
#include "wled.h"
void setup();
void loop();
#line 15 "/home/thomas/Documents/PlatformIO/Projects/wled2/WLED/wled00/wled00.ino"
void setup() {
  WLED::instance().setup();
}

void loop() {
  WLED::instance().loop();
}