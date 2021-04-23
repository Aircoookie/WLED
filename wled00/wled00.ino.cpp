# 1 "/var/folders/d4/c5jgllqs6z70lk9chnqcnwkc0000gp/T/tmpwmtez7ox"
#include <Arduino.h>
# 1 "/Users/ahmedshehata/Developer/cpp/WLED-fork/WLED/wled00/wled00.ino"
# 13 "/Users/ahmedshehata/Developer/cpp/WLED-fork/WLED/wled00/wled00.ino"
#include "wled.h"
void setup();
void loop();
#line 15 "/Users/ahmedshehata/Developer/cpp/WLED-fork/WLED/wled00/wled00.ino"
void setup() {
  WLED::instance().setup();
}

void loop() {
  WLED::instance().loop();
}