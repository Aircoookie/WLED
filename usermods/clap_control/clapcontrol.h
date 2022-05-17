#pragma once

#include "wled.h"

#define MAX_CLAPS 10

class clapControlUsermod : public Usermod {
  private:

    int8_t clapSensorPin;  // pin to be used by sensor
    int clapDelay;         // time after last clap to end session and apply presets
    int bounceDelay;       // cooldown time after each new clap is first detected, helps with sensor bounce and timing
    bool invertSensorHL;   // invert HIGH/LOW for sensor
    bool enabled;          // enable / disable clap control
    int serialOutputLevel; // 0: disabled, 1: data when preset applied, 2: data on claps with preset if applied
    
    bool clap              = false; // Is there a clap being detected right now
    int clapCount          = 0;     // The number of claps in this session, resets after clapDelay timeout
    int clapReading        = 0;     // Holds the reading from the sensor
    unsigned long lastClap = 0;     // millis time of last clap detected
    int sensorHL           = LOW;   // default trigger setting for sensor, can be inverted with "Invert" in usermod setting page    
    int loadPreset         = 0;     // holds preset loaded last
    int clapsToPreset[MAX_CLAPS];   // Stores claps to preset



  public:

    void loop() {
      if (!enabled) return;

      // Read from sensor
      clapReading = digitalRead(clapSensorPin);

      // If new clap detected and not already detecting an ongoing clap
      if (clapReading == (invertSensorHL?!sensorHL:sensorHL) && !clap && millis() - lastClap >= bounceDelay){

        clapCount++;
        lastClap = millis();
        clap = true;
      }

      // No clap detected
      else if (clapReading == (invertSensorHL?sensorHL:!sensorHL)){

        // If a previous clap has ended
        if (clap) {
          clap = false;
        }

        else{
       
          // Check if clapping session has ended
          if (clapCount > 0 && millis() - lastClap >= clapDelay){
            for (int i=1; i<=MAX_CLAPS; i++){        
              if (clapCount == i && clapsToPreset[i] > 0){
                applyPreset(clapsToPreset[i]);
                loadPreset = clapsToPreset[i];
                break;
              }
            }
           
           if (serialOutputLevel > 0){
            serialOutput(clapCount, loadPreset);
           }
           
           clapCount = 0;
           loadPreset = 0;
          }
        }
      }     
    } // end loop()



    void addToConfig(JsonObject& root)
    {
      JsonObject top = root.createNestedObject("Clap Control");

      top["Enable"]                    = enabled;
      top["Clap Timeout (ms)"]         = clapDelay;
      top["Bounce Delay (ms)"]         = bounceDelay;
      top["Serial Output Level (0-2)"] = serialOutputLevel;
      top["Pin"]                       = clapSensorPin;
      top["Invert"]                    = invertSensorHL;

      for (int i = 1; i <= MAX_CLAPS; i++) {
        top[getKey(i)] = clapsToPreset[i];
      }
    }



    bool readFromConfig(JsonObject& root)
    {
      JsonObject top = root["Clap Control"];

      bool configComplete = !top.isNull();

      configComplete &= getJsonValue(top["Enable"],                    enabled,           true);
      configComplete &= getJsonValue(top["Clap Timeout (ms)"],         clapDelay,         250);
      configComplete &= getJsonValue(top["Bounce Delay (ms)"],         bounceDelay,       150);
      configComplete &= getJsonValue(top["Serial Output Level (0-2)"], serialOutputLevel, 0);
      configComplete &= getJsonValue(top["Pin"],                       clapSensorPin,     -1);
      configComplete &= getJsonValue(top["Invert"],                    invertSensorHL,    false);

      for (int i = 1; i <= MAX_CLAPS; i++) {
        configComplete &= getJsonValue(top[getKey(i)], clapsToPreset[i], 0);
        }

      return configComplete;
    }



    // Generate JSON Key
    String getKey(uint8_t i) {
      if (i == 1) return "1 Clap";
      else return String(i) + " Claps";
    }



    // Send JSON blobs with clap and preset data out over serial if enabled
    void serialOutput(int claps, int preset){
      if (serialOutputLevel>=2 || (serialOutputLevel==1 && preset>0)){
        Serial.write("{\"claps\":");
        Serial.print(claps);
        
        if (preset>0){
          Serial.write(",\"preset\":");
          Serial.print(preset);
        }
        
        Serial.println("}");
      }
    }


    
    uint16_t getId(){return USERMOD_ID_CLAP_CONTROL;}

    //void setup() {}
    //void connected() {}
    //void addToJsonInfo(JsonObject& root){}
    //void addToJsonState(JsonObject& root){}
    //void readFromJsonState(JsonObject& root){}
    //void handleOverlayDraw(){}
};
