#pragma once

#include "wled.h"

class UsermodBackgroundDemo : public Usermod {
  private:
    int backgroundIndex = 0;
    int mainIndex = 0;
    
  public:
    void setup() {
      Serial.println("Setup - UsermodBackgroundDemo");
    }

    void doBackgroundWork(){
      backgroundIndex++;
      Serial.println("B: " + String(backgroundIndex));
    }

    void backgroundLoop(){
      EVERY_N_MILLISECONDS(5000){ doBackgroundWork(); }    
    }

    void doMainWork(){
      mainIndex++;
      Serial.println("M: " + String(mainIndex));    
    }

    void loop() {
      EVERY_N_MILLISECONDS(1000) { doMainWork(); }
    }

    uint16_t getId() {
      return USERMOD_ID_BACKGROUND;
    }
};