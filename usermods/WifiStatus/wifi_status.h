#pragma once

#include "wled.h"
#include <deque>

#ifndef USERMOD_WIFI_STATUS_LED_PIN
  #ifdef ARDUINO_ARCH_ESP32
    #define USERMOD_WIFI_STATUS_LED_PIN 2 
  #else //ESP8266 boards
    #define USERMOD_WIFI_STATUS_LED_PIN 16
  #endif
#endif

class WifiStatusUsermod : public Usermod {

  private:
    bool initDone = false;
    bool enabled = true;
    int8_t ledPin       = USERMOD_WIFI_STATUS_LED_PIN; // LED PIN
    std::deque<std::pair<uint8_t, unsigned int>> blinkSequence {};

    int ledState = LOW;
    unsigned long previousMillis = 0;
    unsigned int interval = 1000;

    // string that are used multiple time (this will save some flash memory)
    static const char _name[];
    static const char _enabled[];

    void enable(bool enable) {
      enabled = enable;
    }

    void reconfigure(int8_t oldPin, bool oldEnabled){
      if(oldEnabled != enabled || oldPin != ledPin){
        DEBUG_PRINT(FPSTR(_name));
        DEBUG_PRINTLN(F(" enabled or ledPin changed"));
        if(isAllocatedPin(oldPin)){
          deallocatePin(oldPin);
          digitalWrite(oldPin, LOW);
        }
      }
      configure();
    }

    void configure(){
      if(enabled && pinManager.isPinOk(ledPin) && ledPin >= 0){
        if(!isAllocatedPin(ledPin)){
          if(allocatePin(ledPin)){
            pinMode(ledPin, OUTPUT);
          }else{
            DEBUG_PRINT(FPSTR(_name));
            DEBUG_PRINTLN(F(" Can't allocate ledPin"));
            enabled = false;
            return;
          }
        } else pinMode(ledPin, OUTPUT);
      }
    }

    bool isAllocatedPin(int8_t pin){
      return pinManager.getPinOwner(pin) == PinOwner::UM_WIFI_STATUS;
    }

    void deallocatePin(int8_t pin){
      pinManager.deallocatePin(pin, PinOwner::UM_WIFI_STATUS);
    }

    bool allocatePin(int8_t pin){
      return pinManager.allocatePin(pin, true, PinOwner::UM_WIFI_STATUS);
    }

    void handleBlink(){
      unsigned long currentMillis = millis();
      
      if(blinkSequence.size() < 1) //nothing to blink
        return;

      if (currentMillis - previousMillis >= interval){
        auto event = blinkSequence.front();
        DEBUG_PRINTF("handleBlink() STATE=%d INTERVAL=%d \n", event.first, event.second);
        blinkSequence.pop_front();
        digitalWrite(ledPin, event.first);
        interval = event.second;
        previousMillis = millis();
      }
    }

    void addSequenceConnected(){
      blinkSequence.push_back({HIGH, 2000});
      blinkSequence.push_back({LOW, 10});
    }

    void addSequenceDisconnected(){
      blinkSequence.push_back({HIGH, 200});
      blinkSequence.push_back({LOW, 100});
    }

    void addSequenceAPmode(){
      // blinkSequence.push_back({LOW, 200});
      blinkSequence.push_back({HIGH, 200});
      blinkSequence.push_back({LOW, 200});
      blinkSequence.push_back({HIGH, 200});
      blinkSequence.push_back({LOW, 1000});
    }

  public:

    // gets called once at boot. Do all initialization that doesn't depend on network here
    void setup() {
      DEBUG_PRINTLN(F("---Wifi Status Usermod: setup() start"));
      initDone = true;
      configure();
      DEBUG_PRINTLN(F("---Wifi Status Usermod: setup() end"));
    }

    // gets called every time WiFi is (re-)connected. Initialize own network interfaces here
    void connected() {
      DEBUG_PRINTLN(F("---Wifi Status Usermod: connected()"));
    }

    /*
     * Main Wifi Status loop()
     */
    void loop() {
      if (!enabled || strip.isUpdating()) 
        return; 

      if(blinkSequence.size() <= 1){
        if(WiFi.status() == WL_CONNECTED){
          addSequenceConnected();
        }else if(apActive){
          addSequenceAPmode();
        }else {
          addSequenceDisconnected();
        }
      }

      handleBlink();
    }

    /*
     * addToConfig() can be used to add custom persistent settings to the cfg.json file in the "um" (usermod) object.
     * It will be called by WLED when settings are actually saved (for example, LED settings are saved)
     * If you want to force saving the current state, use serializeConfig() in your loop().
     * 
     * CAUTION: serializeConfig() will initiate a filesystem write operation.
     * It might cause the LEDs to stutter and will cause flash wear if called too often.
     * Use it sparingly and always in the loop, never in network callbacks!
     * 
     * addToConfig() will also not yet add your setting to one of the settings pages automatically.
     * To make that work you still have to add the setting to the HTML, xml.cpp and set.cpp manually.
     * 
     * I highly recommend checking out the basics of ArduinoJson serialization and deserialization in order to use custom settings!
     */
    void addToConfig(JsonObject& root) {
      // we add JSON object: {"WifiStatus": {"enabled": true, "pin": 2}}
      JsonObject top = root.createNestedObject(FPSTR(_name)); // usermodname
      top[FPSTR(_enabled)]     = enabled;
      top["pin"] = ledPin;

      DEBUG_PRINTLN(F("Autosave config saved."));
    }

    /*
     * readFromConfig() can be used to read back the custom settings you added with addToConfig().
     * This is called by WLED when settings are loaded (currently this only happens once immediately after boot)
     * 
     * readFromConfig() is called BEFORE setup(). This means you can use your persistent values in setup() (e.g. pin assignments, buffer sizes),
     * but also that if you want to write persistent values to a dynamic buffer, you'd need to allocate it here instead of in setup.
     * If you don't know what that is, don't fret. It most likely doesn't affect your use case :)
     * 
     * The function should return true if configuration was successfully loaded or false if there was no configuration.
     */
    bool readFromConfig(JsonObject& root) {
      // Looking for JSON object: {"WifiStatus": {"enabled": true, "pin": 2}}
      JsonObject top = root[FPSTR(_name)];
      if (top.isNull()) {
        DEBUG_PRINT(FPSTR(_name));
        DEBUG_PRINTLN(F(": No config found. (Using defaults.)"));
        return false;
      }

      bool oldEnabled = enabled;
      int8_t oldLedPin = ledPin; 

      enabled             = top[FPSTR(_enabled)] | enabled;
      ledPin              = top[FPSTR("pin")] | ledPin;

      reconfigure(oldLedPin, oldEnabled);

      DEBUG_PRINT(FPSTR(_name));
      DEBUG_PRINTLN(F(" config (re)loaded."));

      // use "return !top["newestParameter"].isNull();" when updating Usermod with new features
      return true;
    }

    /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
    uint16_t getId() {
      return USERMOD_ID_WIFI_STATUS;
    }
};

// strings to reduce flash memory usage (used more than twice)
const char WifiStatusUsermod::_name[]                PROGMEM = "WifiStatus";
const char WifiStatusUsermod::_enabled[]             PROGMEM = "enabled";
