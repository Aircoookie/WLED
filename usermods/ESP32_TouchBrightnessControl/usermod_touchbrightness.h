//
//  usermod_touchbrightness.h
//  github.com/aircoookie/WLED
//
//  Created by Justin Kühner on 14.09.2020.
//  Copyright © 2020 NeariX. All rights reserved.
//  https://github.com/NeariX67/
//  Discord: @NeariX#4799


#pragma once

#include "wled.h"

#define threshold 40                    //Increase value if touches falsely accur. Decrease value if actual touches are not recognized
#define touchPin T0                     //T0 = D4 / GPIO4

//Define the 5 brightness levels
//Long press to turn off / on
#define brightness1 51
#define brightness2 102
#define brightness3 153
#define brightness4 204
#define brightness5 255


#ifdef ESP32


class TouchBrightnessControl : public Usermod {
  private:
    unsigned long lastTime = 0;         //Interval
    unsigned long lastTouch = 0;        //Timestamp of last Touch
    unsigned long lastRelease = 0;      //Timestamp of last Touch release
    boolean released = true;            //current Touch state (touched/released)
    uint16_t touchReading = 0;          //sensor reading, maybe use uint8_t???
    uint16_t touchDuration = 0;         //duration of last touch
  public:
  
    void setup() {
      lastTouch = millis();
      lastRelease = millis();
      lastTime = millis();
    }

    void loop() {
      if (millis() - lastTime >= 50) {                           //Check every 50ms if a touch occurs
        lastTime = millis();
        touchReading = touchRead(touchPin);                      //Read touch sensor on pin T0 (GPIO4 / D4)
        
        if(touchReading < threshold && released) {               //Touch started
          released = false;
          lastTouch = millis();
        }
        else if(touchReading >= threshold && !released) {        //Touch released
          released = true;
          lastRelease = millis();
          touchDuration = lastRelease - lastTouch;               //Calculate duration
        }
        
        //Serial.println(touchDuration);

        if(touchDuration >= 800 && released) {                   //Toggle power if button press is longer than 800ms
          touchDuration = 0;                                     //Reset touch duration to avoid multiple actions on same touch
          toggleOnOff();
          colorUpdated(2);                                       //Refresh values
        }
        else if(touchDuration >= 100 && released) {              //Switch to next brightness if touch is between 100 and 800ms
          touchDuration = 0;                                     //Reset touch duration to avoid multiple actions on same touch
          if(bri < brightness1) {
            bri = brightness1;
          } else if(bri >= brightness1 && bri < brightness2) {
            bri = brightness2;
          } else if(bri >= brightness2 && bri < brightness3) {
            bri = brightness3;
          } else if(bri >= brightness3 && bri < brightness4) {
            bri = brightness4;
          } else if(bri >= brightness4 && bri < brightness5) {
            bri = brightness5;
          } else if(bri >= brightness5) {
            bri = brightness1;
          }
          colorUpdated(2);                                       //Refresh values
        }
        
      }
    }
};
#endif
