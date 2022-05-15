#pragma once

#include "wled.h"


// TODO
//   clap_1 - clap_10 is terrible, fix this.
//   add invert setting
//   secret tap/knock


class clapControlUsermod : public Usermod {
  private:

    int clapSensorPin;
    int clapDelay;
    int bounceDelay;

    bool clap = false;
    int clapCount = 0;
    int clapReading = 0;
    unsigned long lastClap = 0;

    int clap_1;
    int clap_2;
    int clap_3;
    int clap_4;
    int clap_5;
    int clap_6;
    int clap_7;
    int clap_8;
    int clap_9;
    int clap_10;



  public:

    void loop() {

      // Read from sensor
      clapReading = digitalRead(clapSensorPin);

      // If new clap detected and not already detecting clap
      if ( clapReading == LOW && !clap && millis() - lastClap >= bounceDelay){
        clapCount++;
        lastClap = millis();
        clap = true;
      }

      // No clap detected
      else if (clapReading == HIGH){

        // If previous clap has ended
        if (clap) {
          clap = false;
        }
        else{
          // check if clapping session has ended
          if ( (clapCount > 0) && (millis() - lastClap >= clapDelay) ){

            //Serial.println(clapCount);

            if (clapCount == 1 && clap_1 > 0) applyPreset(clap_1);
            else if (clapCount == 2 && clap_2 > 0) applyPreset(clap_2);
            else if (clapCount == 3 && clap_3 > 0) applyPreset(clap_3);
            else if (clapCount == 4 && clap_4 > 0) applyPreset(clap_4);
            else if (clapCount == 5 && clap_5 > 0) applyPreset(clap_5);
            else if (clapCount == 6 && clap_6 > 0) applyPreset(clap_6);
            else if (clapCount == 7 && clap_7 > 0) applyPreset(clap_7);
            else if (clapCount == 8 && clap_8 > 0) applyPreset(clap_8);
            else if (clapCount == 9 && clap_9 > 0) applyPreset(clap_9);
            else if (clapCount == 10 && clap_10 > 0) applyPreset(clap_10);

            clapCount=0;
          }
        }
      }

    } // end loop()






    void addToConfig(JsonObject& root)
    {
      JsonObject top = root.createNestedObject("Clap Control");

      top["clapDelay"] = clapDelay;
      top["bounceDelay"] = bounceDelay;
      top["pin"] = clapSensorPin;

      top["clap_1"] = clap_1;
      top["clap_2"] = clap_2;
      top["clap_3"] = clap_3;
      top["clap_4"] = clap_4;
      top["clap_5"] = clap_5;
      top["clap_6"] = clap_6;
      top["clap_7"] = clap_7;
      top["clap_8"] = clap_8;
      top["clap_9"] = clap_9;
      top["clap_10"] = clap_10;
    }


    bool readFromConfig(JsonObject& root)
    {
      JsonObject top = root["Clap Control"];

      bool configComplete = !top.isNull();

      configComplete &= getJsonValue(top["clapDelay"], clapDelay, 250);
      configComplete &= getJsonValue(top["bounceDelay"], bounceDelay, 50);
      configComplete &= getJsonValue(top["pin"], clapSensorPin, 2);

      configComplete &= getJsonValue(top["clap_1"], clap_1, 0);
      configComplete &= getJsonValue(top["clap_2"], clap_2, 0);
      configComplete &= getJsonValue(top["clap_3"], clap_3, 0);
      configComplete &= getJsonValue(top["clap_4"], clap_4, 0);
      configComplete &= getJsonValue(top["clap_5"], clap_5, 0);
      configComplete &= getJsonValue(top["clap_6"], clap_6, 0);
      configComplete &= getJsonValue(top["clap_7"], clap_7, 0);
      configComplete &= getJsonValue(top["clap_8"], clap_8, 0);
      configComplete &= getJsonValue(top["clap_9"], clap_9, 0);
      configComplete &= getJsonValue(top["clap_10"], clap_10, 0);

      return configComplete;
    }



    uint16_t getId(){return USERMOD_ID_CLAP_CONTROL;}

    //void setup() {}
    //void connected() {}
    //void addToJsonInfo(JsonObject& root){}
    //void addToJsonState(JsonObject& root){}
    //void readFromJsonState(JsonObject& root){}
    //void handleOverlayDraw(){}
};
