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
        setTime(rtcTime);
        updateLocalTime();
      } else {
        if (!RTC.chipPresent()) disabled = true; //don't waste time if H/W error
      }
    }

    void loop() {
      if (!disabled && millis() - lastTime > 500) {
        time_t t = now();
        if (t != RTC.get()) RTC.set(t); //set RTC to NTP/UI-provided value

        lastTime = millis();
      }
    }

    uint16_t getId()
    {
      return USERMOD_ID_RTC;
    }
};