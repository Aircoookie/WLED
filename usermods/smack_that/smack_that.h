#pragma once

#include "wled.h"

#define MAX_SMACKS 10

class smackThatUsermod : public Usermod {
  private:

    int8_t smackSensorPin; // pin to be used by sensor
    int smackDelay;        // time after last smack to end session and apply presets
    int bounceDelay;       // cooldown time after each new smack is first detected, helps with sensor bounce and timing
    bool invertSensorHL;   // invert HIGH/LOW for sensor
    bool enabled;          // enable / disable Smack That Usermod
    int serialOutputLevel; // 0: disabled, 1: data when preset applied, 2: data on smacks with preset if applied
    
    bool smack              = false; // Is there a smack being detected right now
    int smackCount          = 0;     // The number of smacks in this session, resets after smackDelay timeout
    int smackReading        = 0;     // Holds the reading from the sensor
    unsigned long lastSmack = 0;     // millis time of last smack detected
    int sensorHL           = LOW;    // default trigger setting for sensor, can be inverted with "Invert" in usermod setting page    
    int loadPreset         = 0;      // holds preset loaded last
    int smacksToPreset[MAX_SMACKS];  // Stores smacks to preset



  public:

    void loop() {
      if (!enabled) return;

      // Read from sensor
      smackReading = digitalRead(smackSensorPin);

      // If new smack detected and not already detecting an ongoing smack
      if (smackReading == (invertSensorHL?!sensorHL:sensorHL) && !smack && millis() - lastSmack >= bounceDelay){

        smackCount++;
        lastSmack = millis();
        smack = true;
      }

      // No smack detected
      else if (smackReading == (invertSensorHL?sensorHL:!sensorHL)){

        // If a previous smack has ended
        if (smack) {
          smack = false;
        }

        else{
       
          // Check if smack session has ended
          if (smackCount > 0 && millis() - lastSmack >= smackDelay){
            for (int i=1; i<=MAX_SMACKS; i++){        
              if (smackCount == i && smacksToPreset[i] > 0){
                applyPreset(smacksToPreset[i]);
                loadPreset = smacksToPreset[i];
                break;
              }
            }
           
           if (serialOutputLevel > 0){
            serialOutput(smackCount, loadPreset);
           }
           
           smackCount = 0;
           loadPreset = 0;
          }
        }
      }     
    } // end loop()



    void addToConfig(JsonObject& root)
    {
      JsonObject top = root.createNestedObject("Smack That");

      top["Enable"]                    = enabled;
      top["Smack Timeout (ms)"]        = smackDelay;
      top["Bounce Delay (ms)"]         = bounceDelay;
      top["Serial Output Level (0-2)"] = serialOutputLevel;
      top["Pin"]                       = smackSensorPin;
      top["Invert"]                    = invertSensorHL;

      for (int i = 1; i <= MAX_SMACKS; i++) {
        top[getKey(i)] = smacksToPreset[i];
      }
    }



    bool readFromConfig(JsonObject& root)
    {
      JsonObject top = root["Smack That"];

      bool configComplete = !top.isNull();

      configComplete &= getJsonValue(top["Enable"],                    enabled,           true);
      configComplete &= getJsonValue(top["Smack Timeout (ms)"],        smackDelay,         250);
      configComplete &= getJsonValue(top["Bounce Delay (ms)"],         bounceDelay,       150);
      configComplete &= getJsonValue(top["Serial Output Level (0-2)"], serialOutputLevel, 0);
      configComplete &= getJsonValue(top["Pin"],                       smackSensorPin,     -1);
      configComplete &= getJsonValue(top["Invert"],                    invertSensorHL,    false);

      for (int i = 1; i <= MAX_SMACKS; i++) {
        configComplete &= getJsonValue(top[getKey(i)], smacksToPreset[i], 0);
        }

      return configComplete;
    }



    // Generate JSON Key
    String getKey(uint8_t i) {
      if (i == 1) return "1 Smack";
      else return String(i) + " Smacks";
    }



    // Send JSON blobs with smack and preset data out over serial if enabled
    void serialOutput(int smacks, int preset){
      if (serialOutputLevel>=2 || (serialOutputLevel==1 && preset>0)){
        Serial.write("{\"smacks\":");
        Serial.print(smacks);
        
        if (preset>0){
          Serial.write(",\"preset\":");
          Serial.print(preset);
        }
        
        Serial.println("}");
      }
    }


    
    uint16_t getId(){return USERMOD_ID_SMACK_THAT;}

    //void setup() {}
    //void connected() {}
    //void addToJsonInfo(JsonObject& root){}
    //void addToJsonState(JsonObject& root){}
    //void readFromJsonState(JsonObject& root){}
    //void handleOverlayDraw(){}
};
