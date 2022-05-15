#pragma once

#include "wled.h"


// TODO
//   pin handler
//   clap_1 - clap_10 is terrible, fix this.
//   secret tap/knock


class clapControlUsermod : public Usermod {
  private:

    int clapSensorPin;
    int clapDelay;
    int bounceDelay;
    bool invertSensorHL;

    bool clap              = false; // Is there a clap being detected right now
    int clapCount          = 0;     // The number of claps in this session, resets after clapDelay timeout
    int clapReading        = 0;     // Holds the reading from the sensor
    unsigned long lastClap = 0;     // millis time of last clap detected
    int sensorHL           = LOW;   // default trigger setting for sensor, can be inverted with "Invert" in usermod setting page

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
      if ( clapReading == (invertSensorHL?!sensorHL:sensorHL) && !clap && millis() - lastClap >= bounceDelay){
        clapCount++;
        lastClap = millis();
        clap = true;
      }

      // No clap detected
      else if (clapReading == (invertSensorHL?sensorHL:!sensorHL)){

        // If previous clap has ended
        if (clap) {
          clap = false;
        }
        else{
          // check if clapping session has ended
          if ( (clapCount > 0) && (millis() - lastClap >= clapDelay) ){

            //Serial.println(clapCount);

            if      (clapCount == 1 && clap_1   > 0) applyPreset(clap_1);
            else if (clapCount == 2 && clap_2   > 0) applyPreset(clap_2);
            else if (clapCount == 3 && clap_3   > 0) applyPreset(clap_3);
            else if (clapCount == 4 && clap_4   > 0) applyPreset(clap_4);
            else if (clapCount == 5 && clap_5   > 0) applyPreset(clap_5);
            else if (clapCount == 6 && clap_6   > 0) applyPreset(clap_6);
            else if (clapCount == 7 && clap_7   > 0) applyPreset(clap_7);
            else if (clapCount == 8 && clap_8   > 0) applyPreset(clap_8);
            else if (clapCount == 9 && clap_9   > 0) applyPreset(clap_9);
            else if (clapCount == 10 && clap_10 > 0) applyPreset(clap_10);

            clapCount=0;
          }
        }
      }

    } // end loop()






    void addToConfig(JsonObject& root)
    {
      JsonObject top = root.createNestedObject("Clap Control");

      top["Clap Timeout (ms)"] = clapDelay;
      top["Bounce Delay (ms)"] = bounceDelay;
      top["Pin"] = clapSensorPin;
      top["Invert"] = invertSensorHL;

      top["1 Clap"]   = clap_1;
      top["2 Claps"]  = clap_2;
      top["3 Claps"]  = clap_3;
      top["4 Claps"]  = clap_4;
      top["5 Claps"]  = clap_5;
      top["6 Claps"]  = clap_6;
      top["7 Claps"]  = clap_7;
      top["8 Claps"]  = clap_8;
      top["9 Claps"]  = clap_9;
      top["10 Claps"] = clap_10;
    }


    bool readFromConfig(JsonObject& root)
    {
      JsonObject top = root["Clap Control"];

      bool configComplete = !top.isNull();

      configComplete &= getJsonValue(top["Clap Timeout (ms)"], clapDelay,      250);
      configComplete &= getJsonValue(top["Bounce Delay (ms)"], bounceDelay,    50);
      configComplete &= getJsonValue(top["Pin"],               clapSensorPin,  -1);
      configComplete &= getJsonValue(top["Invert"],            invertSensorHL, false);

      configComplete &= getJsonValue(top["1 Clap"],   clap_1, 0);
      configComplete &= getJsonValue(top["2 Claps"],  clap_2, 0);
      configComplete &= getJsonValue(top["3 Claps"],  clap_3, 0);
      configComplete &= getJsonValue(top["4 Claps"],  clap_4, 0);
      configComplete &= getJsonValue(top["5 Claps"],  clap_5, 0);
      configComplete &= getJsonValue(top["6 Claps"],  clap_6, 0);
      configComplete &= getJsonValue(top["7 Claps"],  clap_7, 0);
      configComplete &= getJsonValue(top["8 Claps"],  clap_8, 0);
      configComplete &= getJsonValue(top["9 Claps"],  clap_9, 0);
      configComplete &= getJsonValue(top["10 Claps"], clap_10, 0);

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
