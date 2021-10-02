#pragma once

#include "wled.h"



class SevenSegmentDisplay : public Usermod {

  #define WLED_SS_BUFFLEN 6
  #define REFRESHTIME 497
  private:
    //Private class members. You can declare variables and functions only accessible to your usermod here
    unsigned long lastRefresh = 0;
    unsigned long lastCharacterStep = 0;
    char ssDisplayBuffer[WLED_SS_BUFFLEN+1]; //Runtime buffer of what should be displayed.
    char ssCharacterMask[36] = {0x77,0x11,0x6B,0x3B,0x1D,0x3E,0x7E,0x13,0x7F,0x1F,0x5F,0x7C,0x66,0x79,0x6E,0x4E,0x76,0x5D,0x44,0x71,0x5E,0x64,0x27,0x58,0x77,0x4F,0x1F,0x48,0x3E,0x6C,0x75,0x25,0x7D,0x2A,0x3D,0x6B};
    int ssDisplayMessageIdx = 0;         //Position of the start of the message to be physically displayed.


    // set your config variables to their boot default value (this can also be done in readFromConfig() or a constructor if you prefer)
    unsigned long resfreshTime = 497;
    byte ssLEDPerSegment = 1;                            //The number of LEDs in each segment of the 7 seg (total per digit is 7 * ssLedPerSegment)
    byte ssLEDPerPeriod = 1;                              //A Period will have 1x and a Colon will have 2x
    int ssStartLED = 0;                                  //The pixel that the display starts at. 
    /*  HH - 0-23. hh - 1-12, kk - 1-24 hours
    //  MM or mm - 0-59 minutes
    //  SS or ss = 0-59 seconds
    //  : for a colon
    //  All others for alpha numeric, (will be blank when displaying time)
    */
    char ssDisplayMask[WLED_SS_BUFFLEN+1] = "HHMMSS";  //Physical Display Mask, this should reflect physical equipment.
    /* ssDisplayConfig
    //           -------
    //         /   A   /          0 - EDCGFAB
    //        / F     / B         1 - EDCBAFG
    //       /       /            2 - GCDEFAB
    //       -------              3 - GBAFEDC
    //     /   G   /              4 - FABGEDC
    //    / E     / C             5 - FABCDEG
    //   /       /
    //   -------
    //      D
    */
    byte ssDisplayConfig = 5;            //Physical configuration of the Seven segment display
    char ssDisplayMessage[50] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";//Message that can scroll across the display
    bool ssDoDisplayMessage = false;         //If not, display time. 
    unsigned long ssScrollSpeed = 1000;             //Time between advancement of extended message scrolling, in milliseconds.



    unsigned long _overlaySevenSegmentProcess()
    {
      //Do time for now.
      if(!ssDoDisplayMessage)
      {
        //Format the ssDisplayBuffer based on ssDisplayMask
        for(int index = 0; index < WLED_SS_BUFFLEN; index++)
        {
          //Only look for time formatting if there are at least 2 characters left in the buffer.
          if((index < WLED_SS_BUFFLEN - 1) && (ssDisplayMask[index] == ssDisplayMask[index + 1]))
          {
            int timeVar = 0;
            switch(ssDisplayMask[index])
            {
              case 'h':
                timeVar = hourFormat12(localTime);
                break;
              case 'H':
                timeVar = hour(localTime);
                break;
              case 'k':
                timeVar = hour(localTime) + 1;
                break;
              case 'M':
              case 'm':
                timeVar = minute(localTime);
                break;
              case 'S':
              case 's':
                timeVar = second(localTime);
                break;

            }

            //Only want to leave a blank in the hour formatting. 
            if((ssDisplayMask[index] == 'h' || ssDisplayMask[index] == 'H' || ssDisplayMask[index] == 'k') && timeVar < 10)
              ssDisplayBuffer[index] = ' ';
            else
              ssDisplayBuffer[index] = 0x30 + (timeVar / 10);
            ssDisplayBuffer[index + 1] = 0x30 + (timeVar % 10);  

            //Need to increment the index because of the second digit.
            index++;
          }
          else 
          {
            ssDisplayBuffer[index] = (ssDisplayMask[index] == ':' ? ':' : ' ');
          }
        }
        return REFRESHTIME;
      }
      else
      {
        /* This will handle displaying a message and the scrolling of the message if its longer than the buffer length */
        //TODO: Progress message starting point depending on display length, message length, display time, etc...

        //Increase the displayed message index to progress it one character.
        ssDisplayMessageIdx++;

        //Check to see if the message has scrolled completely
        size_t len = strlen(ssDisplayMessage); //really should grab this when the message is set. TODO
        if(ssDisplayMessageIdx > len)
        {
          //If it has displayed the whole message and the display time has exceeded, go back to clock.
          ssDisplayMessageIdx = 0;
          ssDoDisplayMessage = false;
          return REFRESHTIME;
        }
        //Display message
        for(int index = 0; index < WLED_SS_BUFFLEN; index++){
          if(ssDisplayMessageIdx + index < len)
            ssDisplayBuffer[index] = ssDisplayMessage[ssDisplayMessageIdx+index];
          else
            ssDisplayBuffer[index] = ' ';
        }
        return ssScrollSpeed;
      }
    }
    
    void _overlaySevenSegmentDraw()
    {
      
      //Start pixels at ssStartLED, Use ssLEDPerSegment, ssLEDPerPeriod, ssDisplayBuffer
      int indexLED = 0;
      for(int indexBuffer = 0; indexBuffer < WLED_SS_BUFFLEN; indexBuffer++)
      {
        if(ssDisplayBuffer[indexBuffer] == 0) break;
        else if(ssDisplayBuffer[indexBuffer] == '.')
        {
          //Won't ever turn off LED lights for a period. (or will we?)
          indexLED += ssLEDPerPeriod; 
          continue;
        }
        else if(ssDisplayBuffer[indexBuffer] == ':')
        {
          //Turn off colon if odd second?
          indexLED += ssLEDPerPeriod * 2;
        }
        else if(ssDisplayBuffer[indexBuffer] == ' ')
        {
          //Turn off all 7 segments.
          _overlaySevenSegmentLEDOutput(0, indexLED);
          indexLED += ssLEDPerSegment * 7;
        }
        else
        {
          //Turn off correct segments.
          _overlaySevenSegmentLEDOutput(_overlaySevenSegmentGetCharMask(ssDisplayBuffer[indexBuffer]), indexLED);
          indexLED += ssLEDPerSegment * 7;
        }
        
      }

    }

    void _overlaySevenSegmentLEDOutput(char mask, int indexLED)
    {
      for(char index = 0; index < 7; index++)
      {
        if((mask & (0x40 >> index)) != (0x40 >> index))
        {
          for(int numPerSeg = 0; numPerSeg < ssLEDPerSegment;  numPerSeg++)
          {
            strip.setPixelColor(indexLED, 0x000000);
          }
        }
        indexLED += ssLEDPerSegment;
      }
    }

    char _overlaySevenSegmentGetCharMask(char var)
    {
      //ssCharacterMask
      if(var > 0x60) //Essentially a "toLower" call. 
        var -= 0x20;
      if(var > 0x39) //Meaning it is a non-numeric
          var -= 0x07;
      var -= 0x30; //Shift ascii down to start numeric 0 at index 0.
      
      char mask = ssCharacterMask[var];
    /*
      0 - EDCGFAB
      1 - EDCBAFG
      2 - GCDEFAB
      3 - GBAFEDC
      4 - FABGEDC
      5 - FABCDEG
      */
      switch(ssDisplayConfig)
      {
        case 1:
          mask = _overlaySevenSegmentSwapBits(mask, 0, 3, 1);
          mask = _overlaySevenSegmentSwapBits(mask, 1, 2, 1);
          break;
        case 2:
          mask = _overlaySevenSegmentSwapBits(mask, 3, 6, 1);
          mask = _overlaySevenSegmentSwapBits(mask, 4, 5, 1);
          break;
        case 3:
          mask = _overlaySevenSegmentSwapBits(mask, 0, 4, 3);
          mask = _overlaySevenSegmentSwapBits(mask, 3, 6, 1);
          mask = _overlaySevenSegmentSwapBits(mask, 4, 5, 1);
          break;
        case 4:
          mask = _overlaySevenSegmentSwapBits(mask, 0, 4, 3);
          break;
        case 5:
          mask = _overlaySevenSegmentSwapBits(mask, 0, 4, 3);
          mask = _overlaySevenSegmentSwapBits(mask, 0, 3, 1);
          mask = _overlaySevenSegmentSwapBits(mask, 1, 2, 1);
          break;
      }
      return mask;
    }

    char _overlaySevenSegmentSwapBits(char x, char p1, char p2, char n)
    {
        /* Move all bits of first set to rightmost side */
        char set1 = (x >> p1) & ((1U << n) - 1);
    
        /* Move all bits of second set to rightmost side */
        char set2 = (x >> p2) & ((1U << n) - 1);
    
        /* Xor the two sets */
        char Xor = (set1 ^ set2);
    
        /* Put the Xor bits back to their original positions */
        Xor = (Xor << p1) | (Xor << p2);
    
        /* Xor the 'Xor' with the original number so that the 
        two sets are swapped */
        char result = x ^ Xor;
    
        return result;
    }  

  public:
    //Functions called by WLED

    /*
     * setup() is called once at boot. WiFi is not yet connected at this point.
     * You can use it to initialize variables, sensors or similar.
     */
    void setup() {
      //Serial.println("Hello from my usermod!");
    }

    /*
     * loop() is called continuously. Here you can check for events, read sensors, etc.
     */
    void loop() {
      if (millis() - lastRefresh > resfreshTime) {
        resfreshTime = _overlaySevenSegmentProcess();
        lastRefresh = millis();
      }
    }

    void handleOverlayDraw(){
      _overlaySevenSegmentDraw();
    }

// void onMqttConnect(bool sessionPresent)
// {
//     if (mqttDeviceTopic[0] == 0)
//         return;

//     for (int pinNr = 0; pinNr < NUM_SWITCH_PINS; pinNr++) {
//         char buf[128];
//         StaticJsonDocument<1024> json;
//         sprintf(buf, "%s Switch %d", serverDescription, pinNr + 1);
//         json[F("name")] = buf;

//         sprintf(buf, "%s/switch/%d", mqttDeviceTopic, pinNr);
//         json["~"] = buf;
//         strcat(buf, "/set");
//         mqtt->subscribe(buf, 0);

//         json[F("stat_t")] = "~/state";
//         json[F("cmd_t")] = "~/set";
//         json[F("pl_off")] = F("OFF");
//         json[F("pl_on")] = F("ON");

//         char uid[16];
//         sprintf(uid, "%s_sw%d", escapedMac.c_str(), pinNr);
//         json[F("unique_id")] = uid;

//         strcpy(buf, mqttDeviceTopic);
//         strcat(buf, "/status");
//         json[F("avty_t")] = buf;
//         json[F("pl_avail")] = F("online");
//         json[F("pl_not_avail")] = F("offline");
//         //TODO: dev
//         sprintf(buf, "homeassistant/switch/%s/config", uid);
//         char json_str[1024];
//         size_t payload_size = serializeJson(json, json_str);
//         mqtt->publish(buf, 0, true, json_str, payload_size);
//         updateState(pinNr);
//     }
// }

// bool onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
// {
//     //Note: Payload is not necessarily null terminated. Check "len" instead.
//     for (int pinNr = 0; pinNr < NUM_SWITCH_PINS; pinNr++) {
//         char buf[64];
//         sprintf(buf, "%s/switch/%d/set", mqttDeviceTopic, pinNr);
//         if (strcmp(topic, buf) == 0) {
//             //Any string starting with "ON" is interpreted as ON, everything else as OFF
//             setState(pinNr, len >= 2 && payload[0] == 'O' && payload[1] == 'N');
//             return true;
//         }
//     }
// }

//     /*
//      * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
//      * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
//      * Below it is shown how this could be used for e.g. a light sensor
//      */
//     /*
//     void addToJsonInfo(JsonObject& root)
//     {
//       int reading = 20;
//       //this code adds "u":{"Light":[20," lux"]} to the info object
//       JsonObject user = root["u"];
//       if (user.isNull()) user = root.createNestedObject("u");

//       JsonArray lightArr = user.createNestedArray("Light"); //name
//       lightArr.add(reading); //value
//       lightArr.add(" lux"); //unit
//     }
//     */


//     /*
//      * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
//      * Values in the state object may be modified by connected clients
//      */
//     void addToJsonState(JsonObject& root)
//     {
//       //root["user0"] = userVar0;
//     }


//     /*
//      * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
//      * Values in the state object may be modified by connected clients
//      */
//     void readFromJsonState(JsonObject& root)
//     {
//       userVar0 = root["user0"] | userVar0; //if "user0" key exists in JSON, update, else keep old value
//       //if (root["bri"] == 255) Serial.println(F("Don't burn down your garage!"));
//     }


//     /*
//      * addToConfig() can be used to add custom persistent settings to the cfg.json file in the "um" (usermod) object.
//      * It will be called by WLED when settings are actually saved (for example, LED settings are saved)
//      * If you want to force saving the current state, use serializeConfig() in your loop().
//      * 
//      * CAUTION: serializeConfig() will initiate a filesystem write operation.
//      * It might cause the LEDs to stutter and will cause flash wear if called too often.
//      * Use it sparingly and always in the loop, never in network callbacks!
//      * 
//      * addToConfig() will make your settings editable through the Usermod Settings page automatically.
//      *
//      * Usermod Settings Overview:
//      * - Numeric values are treated as floats in the browser.
//      *   - If the numeric value entered into the browser contains a decimal point, it will be parsed as a C float
//      *     before being returned to the Usermod.  The float data type has only 6-7 decimal digits of precision, and
//      *     doubles are not supported, numbers will be rounded to the nearest float value when being parsed.
//      *     The range accepted by the input field is +/- 1.175494351e-38 to +/- 3.402823466e+38.
//      *   - If the numeric value entered into the browser doesn't contain a decimal point, it will be parsed as a
//      *     C int32_t (range: -2147483648 to 2147483647) before being returned to the usermod.
//      *     Overflows or underflows are truncated to the max/min value for an int32_t, and again truncated to the type
//      *     used in the Usermod when reading the value from ArduinoJson.
//      * - Pin values can be treated differently from an integer value by using the key name "pin"
//      *   - "pin" can contain a single or array of integer values
//      *   - On the Usermod Settings page there is simple checking for pin conflicts and warnings for special pins
//      *     - Red color indicates a conflict.  Yellow color indicates a pin with a warning (e.g. an input-only pin)
//      *   - Tip: use int8_t to store the pin value in the Usermod, so a -1 value (pin not set) can be used
//      *
//      * See usermod_v2_auto_save.h for an example that saves Flash space by reusing ArduinoJson key name strings
//      * 
//      * If you need a dedicated settings page with custom layout for your Usermod, that takes a lot more work.  
//      * You will have to add the setting to the HTML, xml.cpp and set.cpp manually.
//      * See the WLED Soundreactive fork (code and wiki) for reference.  https://github.com/atuline/WLED
//      * 
//      * I highly recommend checking out the basics of ArduinoJson serialization and deserialization in order to use custom settings!
//      */
//     void addToConfig(JsonObject& root)
//     {
//       JsonObject top = root.createNestedObject("exampleUsermod");
//       top["great"] = userVar0; //save these vars persistently whenever settings are saved
//       top["testBool"] = testBool;
//       top["testInt"] = testInt;
//       top["testLong"] = testLong;
//       top["testULong"] = testULong;
//       top["testFloat"] = testFloat;
//       top["testString"] = testString;
//       JsonArray pinArray = top.createNestedArray("pin");
//       pinArray.add(testPins[0]);
//       pinArray.add(testPins[1]); 
//     }


//     /*
//      * readFromConfig() can be used to read back the custom settings you added with addToConfig().
//      * This is called by WLED when settings are loaded (currently this only happens immediately after boot, or after saving on the Usermod Settings page)
//      * 
//      * readFromConfig() is called BEFORE setup(). This means you can use your persistent values in setup() (e.g. pin assignments, buffer sizes),
//      * but also that if you want to write persistent values to a dynamic buffer, you'd need to allocate it here instead of in setup.
//      * If you don't know what that is, don't fret. It most likely doesn't affect your use case :)
//      * 
//      * Return true in case the config values returned from Usermod Settings were complete, or false if you'd like WLED to save your defaults to disk (so any missing values are editable in Usermod Settings)
//      * 
//      * getJsonValue() returns false if the value is missing, or copies the value into the variable provided and returns true if the value is present
//      * The configComplete variable is true only if the "exampleUsermod" object and all values are present.  If any values are missing, WLED will know to call addToConfig() to save them
//      * 
//      * This function is guaranteed to be called on boot, but could also be called every time settings are updated
//      */
//     bool readFromConfig(JsonObject& root)
//     {
//       // default settings values could be set here (or below using the 3-argument getJsonValue()) instead of in the class definition or constructor
//       // setting them inside readFromConfig() is slightly more robust, handling the rare but plausible use case of single value being missing after boot (e.g. if the cfg.json was manually edited and a value was removed)

//       JsonObject top = root["exampleUsermod"];

//       bool configComplete = !top.isNull();

//       configComplete &= getJsonValue(top["great"], userVar0);
//       configComplete &= getJsonValue(top["testBool"], testBool);
//       configComplete &= getJsonValue(top["testULong"], testULong);
//       configComplete &= getJsonValue(top["testFloat"], testFloat);
//       configComplete &= getJsonValue(top["testString"], testString);

//       // A 3-argument getJsonValue() assigns the 3rd argument as a default value if the Json value is missing
//       configComplete &= getJsonValue(top["testInt"], testInt, 42);  
//       configComplete &= getJsonValue(top["testLong"], testLong, -42424242);
//       configComplete &= getJsonValue(top["pin"][0], testPins[0], -1);
//       configComplete &= getJsonValue(top["pin"][1], testPins[1], -1);

//       return configComplete;
//     }

   
//     /*
//      * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
//      * This could be used in the future for the system to determine whether your usermod is installed.
//      */
//     uint16_t getId()
//     {
//       return USERMOD_ID_SEVEN_SEGMENT_DISPLAY;
//     }

  
};