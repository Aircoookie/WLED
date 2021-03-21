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
#define touchPin 12
#define touchPin2 14                     //T0 = D4 / GPIO4 ESP32-CAM 14, 12 

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
    unsigned long lastTouch = 0;        //Timestamp of last Touch1
    unsigned long lastTouch2 = 0;
    unsigned long lastRelease = 0;      //Timestamp of last Touch1 release
    unsigned long lastRelease2 = 0;
    boolean released = true;            //current Touch state (touched/released)
    boolean released2 = true;
    uint16_t touchReading = 0;          //sensor1 reading, maybe use uint8_t???
    uint16_t touchReading2 = 0;
    uint16_t touchDuration = 0;         //duration of last touch
    uint16_t touchDuration2 = 0;

  public:
  
    void setup() {
      lastTouch = millis();
      lastTouch2 = millis();
      lastRelease = millis();
      lastRelease2 = millis();
      lastTime = millis();
    }

    void loop() {
      if (millis() - lastTime >= 50) {                           //Check every 50ms if a touch occurs
        lastTime = millis();
        touchReading = touchRead(touchPin);                      //Read touch sensor on pin T0 (GPIO4 / D4)
        touchReading2 = touchRead(touchPin2);

        if(touchReading < threshold && released) {               //Touch started
          released = false;
          lastTouch = millis();
        }
        else if(touchReading >= threshold && !released) {        //Touch released
          released = true;
          lastRelease = millis();
          touchDuration = lastRelease - lastTouch;               //Calculate duration
        }
        
        if(touchReading2 < threshold && released2) {
          released2 = false;
          lastTouch2 = millis();
        }
        else if(touchReading2 >= threshold && !released2) {
          released2 = true;
          lastRelease2 = millis();
          touchDuration2 = lastRelease2 - lastTouch2;
        }
        //Serial.println(touchDuration);

        if(touchDuration >= 800 && released) {                   //Toggle power if button press is longer than 800ms
          touchDuration = 0;                                     //Reset touch duration to avoid multiple actions on same touch
          toggleOnOff();
          colorUpdated(2);                                       //Refresh values
        }
        else if(touchDuration >= 100 && released) {              //Switch to next brightness if touch is between 100 and 800ms
          touchDuration = 0;                                     //Reset touch duration to avoid multiple actions on same touch
          // if(bri < brightness1) {
          //   bri = brightness1;
          // } else if(bri >= brightness1 && bri < brightness2) {
          //   bri = brightness2;
          // } else if(bri >= brightness2 && bri < brightness3) {
          //   bri = brightness3;
          // } else if(bri >= brightness3 && bri < brightness4) {
          //   bri = brightness4;
          // } else if(bri >= brightness4 && bri < brightness5) {
          //   bri = brightness5;
          // } else if(bri >= brightness5) {
          //   bri = brightness1;
          // }
          if (bri > 0) {
          effectCurrent += 1;
          if (effectCurrent >= MODE_COUNT)
            effectCurrent = 0;
          colorUpdated(2);
          } else {
            toggleOnOff();
          }                                       //Refresh values
        }

        if(touchDuration2 >= 800 && released2) {
          touchDuration2 = 0;
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
            colorUpdated(2);
        }
        else if (touchDuration2 >= 100 && released2)
        {                     //Switch to next brightness if touch is between 100 and 800ms
          touchDuration2 = 0; //Reset touch duration to avoid multiple actions on same touch

          if (bri > 0)
          {
            effectCurrent -= 1;
            if (effectCurrent < 0)
              effectCurrent = (MODE_COUNT - 1);
            // colorUpdated(2);
          }
          else
          {
            toggleOnOff();
          } //Refresh values
        }
      }
    }
};
#endif
