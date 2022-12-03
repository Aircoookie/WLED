#pragma once

#include "wled.h"

//class name. Use something descriptive and leave the ": public Usermod" part :)
class PowerAPUsermod : public Usermod {
  private:
    unsigned long lastTime = 0;
    String fname = F("/boot.dat");

  public:
    void setup() {
      if (WLED_FS.exists(fname)) {
        File fl = WLED_FS.open(fname,"w");
        fl.seek(0);
        char data = fl.read();
        if (data == '0') {
          fl.seek(0);
          fl.write('1');
        } else if (data == '1') {
          apBehavior = AP_BEHAVIOR_ALWAYS;
          WLED::instance().initAP(true);
        }
        fl.close();
      } else {
        File fl = WLED_FS.open(fname,"w");
        fl.write('0');
        fl.close();
      }
    }

    void loop() {
      if (millis() < 10000 && millis() - lastTime > 5000) {
        lastTime = millis();
        if (WLED_FS.exists(fname)) {
          WLED_FS.remove(fname);
        }
     }
    }

    uint16_t getId() { return USERMOD_ID_UNSPECIFIED; }

};