/*
    Arduino Studio support file.
*/
#include "wled.h"

WLED wled;
void setup() {
  wled.instance();  // Force creation of static instance
}

void loop() {
  wled.loop();
}