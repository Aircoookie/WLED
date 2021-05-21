#pragma once
#include "TFTs.h"
#include "wled.h"

class ElekstubeIPSUsermod : public Usermod {
  private:
    TFTs tfts;
    void updateClockDisplay(TFTs::show_t show=TFTs::yes) {
      uint8_t hr = hour(localTime);
      uint8_t hrTens = hr/10;
      uint8_t mi = minute(localTime);
      uint8_t mittens = mi/10;
      uint8_t s = second(localTime);
      uint8_t sTens = s/10;
      tfts.setDigit(HOURS_TENS, hrTens, show);
      tfts.setDigit(HOURS_ONES, hr - hrTens*10, show);
      tfts.setDigit(MINUTES_TENS, mittens, show);
      tfts.setDigit(MINUTES_ONES, mi - mittens*10, show);
      tfts.setDigit(SECONDS_TENS, sTens, show);
      tfts.setDigit(SECONDS_ONES, s - sTens*10, show);
    }
    unsigned long lastTime = 0;
  public:

    void setup() {
      tfts.begin();
      tfts.fillScreen(TFT_BLACK);
      tfts.setTextColor(TFT_WHITE, TFT_BLACK);
      tfts.setCursor(0, 0, 2);
      tfts.println("<STARTUP>");
    }

    void loop() {
      if (lastTime == 0) {
        tfts.fillScreen(TFT_BLACK);
        updateClockDisplay(TFTs::force);
      }
      if (millis() - lastTime > 100) {
        updateClockDisplay();
        lastTime = millis();
      }
    }

    uint16_t getId()
    {
      return USERMOD_ID_ELEKSTUBE_IPS;
    }
};