#pragma once
#include "TFTs.h"
#include "wled.h"

//Large parts of the code are from https://github.com/SmittyHalibut/EleksTubeHAX

class ElekstubeIPSUsermod : public Usermod {
  private:
    TFTs tfts;
    void updateClockDisplay(TFTs::show_t show=TFTs::yes) {
      bool set[6] = {false}; 
      for (uint8_t i = 0; i<6; i++) {
        char c = cronixieDisplay[i];
        if (c >= '0' && c <= '9') {
          tfts.setDigit(5-i, c - '0', show); set[i] = true;
        } else if (c >= 'A' && c <= 'G') {
          tfts.setDigit(5-i, c - 'A' + 10, show); set[i] = true; //10.bmp to 16.bmp static display
        } else if (c == '-' || c == '_' || c == ' ') {
          tfts.setDigit(5-i, 255, show); set[i] = true; //blank
        } else {
          set[i] = false; //display HHMMSS time
        }
      }
      uint8_t hr = hour(localTime);
      uint8_t hrTens = hr/10;
      uint8_t mi = minute(localTime);
      uint8_t mittens = mi/10;
      uint8_t s = second(localTime);
      uint8_t sTens = s/10;
      if (!set[0]) tfts.setDigit(HOURS_TENS, hrTens, show);
      if (!set[1]) tfts.setDigit(HOURS_ONES, hr - hrTens*10, show);
      if (!set[2]) tfts.setDigit(MINUTES_TENS, mittens, show);
      if (!set[3]) tfts.setDigit(MINUTES_ONES, mi - mittens*10, show);
      if (!set[4]) tfts.setDigit(SECONDS_TENS, sTens, show);
      if (!set[5]) tfts.setDigit(SECONDS_ONES, s - sTens*10, show);
    }
    unsigned long lastTime = 0;
  public:

    void setup() {
      tfts.begin();
      tfts.fillScreen(TFT_BLACK);

      for (int8_t i = 5; i >= 0; i--) {
        tfts.setDigit(i, 255, TFTs::force); //turn all off
      }
    }

    void loop() {
      if (toki.isTick()) {
        updateLocalTime();
        updateClockDisplay();
      }
    }

    uint16_t getId()
    {
      return USERMOD_ID_ELEKSTUBE_IPS;
    }
};