# 1 "C:\\Users\\atuline\\AppData\\Local\\Temp\\tmp3zh3u1hm"
#include <Arduino.h>
# 1 "F:/Arduino/Sketches/FastLED Sketches/_GITSEQUENCES/WLED/wled00/wled00.ino"
# 13 "F:/Arduino/Sketches/FastLED Sketches/_GITSEQUENCES/WLED/wled00/wled00.ino"
#include "wled.h"
void setup();
void loop();
#line 15 "F:/Arduino/Sketches/FastLED Sketches/_GITSEQUENCES/WLED/wled00/wled00.ino"
void setup() {
  WLED::instance().setup();
}

void loop() {
  WLED::instance().loop();
}