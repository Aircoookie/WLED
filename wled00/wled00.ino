/*
    Arduino Studio support file.
*/
#include "wled.h"

WLED& wled;
void setup() {
  wled = WLED::instance();
  wled.setup();
}

void loop() {
  wled.loop();
}