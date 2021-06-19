/*
 * That usermod implements support of simple hand gestures with VL53L0X sensor: on/off and brightness correction.
 * It can be useful for kitchen strips to avoid any touches.
 * - on/off - just swipe a hand below your sensor ("shortPressAction" is called and can be customized through WLED macros)
 * - brightness correction - keep your hand below sensor for 1 second to switch to "brightness" mode.
 *                           Configure brightness by changing distance to the sensor (see parameters below for customization).
 *                           "macroLongPress" is also called here.
 *
 * Enabling this mod usermod:
 * 1. Attach VL53L0X sensor to i2c pins according to default pins for your board.
 * 2. Add "-D USERMOD_VL53L0X_GESTURES" to your build flags at platformio.ini (plaformio_override.ini) for needed environment.
 * In my case, for example: build_flags = ${common.build_flags_esp8266} -D RLYPIN=12 -D USERMOD_VL53L0X_GESTURES
 * 3. Add "pololu/VL53L0X" dependency to lib_deps like this:
 * lib_deps = ${env.lib_deps}
 *     pololu/VL53L0X @ ^1.3.0
 */
#pragma once

#include "wled.h"

#include <Wire.h>
#include <VL53L0X.h>

#ifndef VL53L0X_MAX_RANGE_MM
#define VL53L0X_MAX_RANGE_MM 230 // max height in millimiters to react for motions
#endif

#ifndef VL53L0X_MIN_RANGE_OFFSET
#define VL53L0X_MIN_RANGE_OFFSET 60 // minimal range in millimiters that sensor can detect. Used in long motions to correct brightnes calculation.
#endif

#ifndef VL53L0X_DELAY_MS
#define VL53L0X_DELAY_MS 100 // how often to get data from sensor 
#endif

#ifndef VL53L0X_LONG_MOTION_DELAY_MS
#define VL53L0X_LONG_MOTION_DELAY_MS 1000 // how often to get data from sensor 
#endif

class UsermodVL53L0XGestures : public Usermod {
  private:
    //Private class members. You can declare variables and functions only accessible to your usermod here
    unsigned long lastTime = 0;
    VL53L0X sensor;

    bool wasMotionBefore = false;
    bool isLongMotion = false;
    unsigned long motionStartTime = 0;
    
  public:

    void setup() {
      Wire.begin();

      sensor.setTimeout(150);
      if (!sensor.init())
      {
        DEBUG_PRINTLN(F("Failed to detect and initialize VL53L0X sensor!"));
      } else {
        sensor.setMeasurementTimingBudget(20000); // set high speed mode
      }
    }


    void loop() {
      if (millis() - lastTime > VL53L0X_DELAY_MS)
      {
        lastTime = millis();

        int range = sensor.readRangeSingleMillimeters();
        DEBUG_PRINTF(F("range: %d, brightness: %d"), range, bri);

        if (range < VL53L0X_MAX_RANGE_MM)
        {
          if (!wasMotionBefore)
          {
            motionStartTime = millis();
            DEBUG_PRINTF(F("motionStartTime: %d"), motionStartTime);
          }
          wasMotionBefore = true;

          if (millis() - motionStartTime > VL53L0X_LONG_MOTION_DELAY_MS) //long motion
          {
            DEBUG_PRINTF(F("long motion: %d"), motionStartTime);
            if (!isLongMotion)
            {
              if (macroLongPress)
              {
                applyMacro(macroLongPress);
              }
              isLongMotion = true;
            }

            // set brightness according to range
            bri = (VL53L0X_MAX_RANGE_MM - max(range, VL53L0X_MIN_RANGE_OFFSET)) * 255 / (VL53L0X_MAX_RANGE_MM - VL53L0X_MIN_RANGE_OFFSET);
            DEBUG_PRINTF(F("new brightness: %d"), bri);
            colorUpdated(1);
          }
        } else if (wasMotionBefore) { //released
          long dur = millis() - motionStartTime;

          if (!isLongMotion)
          { //short press
            DEBUG_PRINTF(F("shortPressAction..."));
            shortPressAction();
          }
          wasMotionBefore = false;
          isLongMotion = false;
        }
      }
    }

    /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
    uint16_t getId()
    {
      return USERMOD_ID_VL53L0X;
    }
};