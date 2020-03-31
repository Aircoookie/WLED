/*
 * Arduino IDE compatibility file.
 */
#include "wled.h"

void setup() {
  WLED::instance().setup();
}

void loop() {
  WLED::instance().loop();
}
