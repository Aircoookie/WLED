#pragma once

#include "wled.h"

class PowerAPUsermod : public Usermod {
  private:
    unsigned long lastTime = 0;
    String fname = F("/boot.dat");

  public:
    void setup() {
      if (WLED_FS.exists(fname)) {
        File fl = WLED_FS.open(fname,"r+");
        if (!fl) DEBUG_PRINTLN(F("--- File read failed ---"));
        char data = fl.read();
        if (data == '0') {
          DEBUG_PRINTLN(F("--- 2nd boot ---"));
          fl.seek(0);
          fl.write('1');
        } else if (data == '1') {
          DEBUG_PRINTLN(F("--- 3rd boot ---"));
          apBehavior = AP_BEHAVIOR_ALWAYS;
          WLED::instance().initAP(true);
        }
        fl.close();
      } else {
        DEBUG_PRINTLN(F("--- 1st boot ---"));
        File fl = WLED_FS.open(fname,"w");
        fl.write((uint8_t*)"0 ", 2);  // write('0'); does not work somehow
        fl.close();
      }
    }

    void loop() {
      if (millis() < 10000 && millis() - lastTime > 5000) {
        lastTime = millis();
        if (WLED_FS.exists(fname)) {
          DEBUG_PRINTLN(F("--- Removing boot file ---"));
          WLED_FS.remove(fname);
        }
     }
    }

    uint16_t getId() { return USERMOD_ID_UNSPECIFIED; }

};