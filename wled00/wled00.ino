/*
 * WLED Arduino IDE compatibility file.
 * 
 * Where has everything gone?
 * 
 * In April 2020, the project's structure underwent a major change. 
 * Global variables are now found in file "wled.h"
 * Global function declarations are found in "fcn_declare.h"
 * 
 * Usermod compatibility: Existing wled06_usermod.ino mods should continue to work. Delete usermod.cpp.
 * New usermods should use usermod.cpp instead.
 */
#include "wled.h"

unsigned long lastMillis = 0; //WLEDMM
unsigned long loopCounter = 0; //WLEDMM

void setup() {
  WLED::instance().setup();
}

void loop() {
  //WLEDMM show loops per second
  loopCounter++;
  if (millis() - lastMillis >= 10000) {
    USER_PRINTF("%lu lps\n",loopCounter/10);
    lastMillis = millis();
    loopCounter = 0;
  }

  WLED::instance().loop();
}
