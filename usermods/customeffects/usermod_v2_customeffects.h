/*
   @title   Usermod Custom Effects (CE)
   @file    usermod_v2_customeffects.h
   @date    20220818
   @author  Ewoud Wijma
   @Copyright (c) 2023 Ewoud Wijma
   @repo    https://github.com/ewoudwijma/ARTI
 */

#pragma once

#include "wled.h"

#include "arti_wled.h"

//declare global variables
ARTI * arti;

//effect function
uint16_t mode_customEffect(void) { 
  //tbd: move statics to SEGMENT.data
  static bool succesful;
  static bool notEnoughHeap;

  static char previousEffect[charLength];
  if (SEGENV.call == 0) {
    strcpy(previousEffect, ""); //force init
    SEGMENT.fill(BLACK); //in case not all leds used e.g. when using expand 1d Circle. Tbd: fill black should never be used to allow for blends/transitions
  }

  char currentEffect[charLength];
  strcpy(currentEffect, (SEGMENT.name != nullptr)?SEGMENT.name:"default"); //note: switching preset with segment name to preset without does not clear the SEGMENT.name variable, but not gonna solve here ;-)

  if (strcmp(previousEffect, currentEffect) != 0) 
  {
    strcpy(previousEffect, currentEffect);

    // if (artiWrapper != nullptr && artiWrapper->arti != nullptr) {
    if (arti != nullptr) 
    {
      arti->close();
      delete arti; arti = nullptr;
    }

    // if (!SEGENV.allocateData(sizeof(ArtiWrapper))) return mode_static();  // We use this method for allocating memory for static variables.
    // artiWrapper = reinterpret_cast<ArtiWrapper*>(SEGENV.data);
    arti = new ARTI();

    char programFileName[fileNameLength];
    strcpy(programFileName, "/");
    strcat(programFileName, currentEffect);
    strcat(programFileName, ".wled");

    succesful = arti->setup("/wledv033.json", programFileName);

    if (!succesful)
      ERROR_ARTI("Setup not succesful\n");
  }
  else 
  {
    if (succesful) // && SEGENV.call < 250 for each frame
    {
      if (esp_get_free_heap_size() <= 20000) 
      {
        ERROR_ARTI("Not enough free heap (%u <= 30000)\n", esp_get_free_heap_size());
        notEnoughHeap = true;
        succesful = false;
      }
      else
      {
        // static int previousMillis;
        // static int previousCall;
        // if (millis() - previousMillis > 5000) { //tried SEGENV.aux0 but that looks to be overwritten!!! (dangling pointer???)
        //   previousMillis = millis();
        //   MEMORY_ARTI("Heap renderFrame %u %u fps\n", esp_get_free_heap_size(), (SEGENV.call - previousCall)/5);
        //   previousCall = SEGENV.call;
        // }
        
        succesful = arti->loop();
      }
    }
    else 
    {
      arti->closeLog();
      if (notEnoughHeap && esp_get_free_heap_size() > 20000) {
        ERROR_ARTI("Again enough free heap, restart effect (%u > 30000)\n", esp_get_free_heap_size());
        succesful = true;
        notEnoughHeap = false;
        strcpy(previousEffect, ""); // force new create
      }
      else {
        //mode_static
        SEGMENT.fill(SEGCOLOR(0));
        return 350;
      }
    }
  }

  return FRAMETIME;
}

static const char _data_FX_MODE_CUSTOMEFFECT[] PROGMEM = "⚙️ Custom Effect@Speed,Intensity,Custom 1, Custom 2, Custom 3;!;!;1;mp12=0";

class CustomEffectsUserMod : public Usermod {
  private:
    // strings to reduce flash memory usage (used more than twice)
    static const char _name[]; //usermod name

    char errorMessage[100] = "";

    bool     enabled = false;
    bool     initDone = false;

  public:

    void setup() {
      if (!initDone)
        strip.addEffect(FX_MODE_CUSTOMEFFECT, &mode_customEffect, _data_FX_MODE_CUSTOMEFFECT);
      initDone = true;
      enabled = true;
    }

    void connected() {
    }

    void loop() {
    }

    /*
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
     * Below it is shown how this could be used for e.g. a light sensor
     */
    void addToJsonInfo(JsonObject& root)
    {
    }


    /*
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void addToJsonState(JsonObject& root)
    {
      //root["user0"] = userVar0;
      if (!initDone) return;  // prevent crash on boot applyPreset()
      JsonObject usermod = root[FPSTR(_name)];
      if (usermod.isNull()) {
        usermod = root.createNestedObject(FPSTR(_name));
      }
      usermod["on"] = enabled;
    }


    /*
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void readFromJsonState(JsonObject& root)
    {
      // userVar0 = root["user0"] | userVar0; //if "user0" key exists in JSON, update, else keep old value
      //if (root["bri"] == 255) Serial.println(F("Don't burn down your garage!"));
    }


    void addToConfig(JsonObject& root)
    {
    }


    bool readFromConfig(JsonObject& root)
    {
      JsonObject top = root[FPSTR(_name)];

      bool configComplete = !top.isNull();


      //  * Return true in case the config values returned from Usermod Settings were complete, or false if you'd like WLED to save your defaults to disk (so any missing values are editable in Usermod Settings)
      return configComplete;
    }

    void appendConfigData()
    {
    }

    /*
     * handleOverlayDraw() is called just before every show() (LED strip update frame) after effects have set the colors.
     * Use this to blank out some LEDs or set them to a different color regardless of the set effect mode.
     * Commonly used for custom clocks (Cronixie, 7 segment)
     */
    void handleOverlayDraw()
    {
      //strip.setPixelColor(0, RGBW32(0,0,0,0)) // set the first pixel to black
    }

   
    /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
    uint16_t getId()
    {
      return USERMOD_ID_CUSTOMEFFECTS;
    }
};

// strings to reduce flash memory usage (used more than twice)
const char CustomEffectsUserMod::_name[]       PROGMEM = "CustomEffects";
