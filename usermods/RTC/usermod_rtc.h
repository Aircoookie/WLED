#pragma once

#include "src/dependencies/time/DS1307RTC.h"
#include "wled.h"

//Connect DS1307 to standard I2C pins (ESP32: GPIO 21 (SDA)/GPIO 22 (SCL))

class RTCUsermod : public Usermod {
  private:
    unsigned long lastTime = 0;
    bool disabled = false;
  public:

    void setup() {
      time_t rtcTime = RTC.get();
      if (rtcTime) {
        toki.setTime(rtcTime,TOKI_NO_MS_ACCURACY,TOKI_TS_RTC);
        updateLocalTime();
      } else {
        if (!RTC.chipPresent()) disabled = true; //don't waste time if H/W error
      }
    }

    void loop() {
      if (!disabled && toki.isTick()) {
        time_t t = toki.second();
        if (t != RTC.get()) RTC.set(t); //set RTC to NTP/UI-provided value
      }
    }

    uint16_t getId()
    {
      return USERMOD_ID_RTC;
    }
};