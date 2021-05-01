#pragma once

#include "wled.h"

#ifndef MULTI_RELAY_MAX_RELAYS
  #define MULTI_RELAY_MAX_RELAYS 4
#endif

/*
 * This usermod handles multiple relay outputs.
 * These outputs complement built-in relay output in a way that the activation can be delayed.
 * They can also activate/deactivate in reverse logic independently.
 */

class MultiRelay : public Usermod {

  private:
    // pins
    int8_t _relayPin[MULTI_RELAY_MAX_RELAYS];
    // delay (in seconds) before relay state changes
    uint16_t _relayDelay[MULTI_RELAY_MAX_RELAYS];
    // activation mode (high/low)
    bool _relayMode[MULTI_RELAY_MAX_RELAYS];
    // relay active state
    bool _relayActive[MULTI_RELAY_MAX_RELAYS];

    // switch timer start time
    uint32_t _switchTimerStart = 0;
    // old brightness
    uint8_t _oldBrightness = 0;

    // usermod enabled
    bool enabled = false;  // needs to be configured (no default config)
    // status of initialisation
    bool initDone = false;

    // strings to reduce flash memory usage (used more than twice)
    static const char _name[];
    static const char _enabled[];
    static const char _relay[];
    static const char _delay[];
    static const char _activeHigh[];

    /**
     * switch relay on/off
     */
    void switchRelay(uint8_t relay) {
      if (relay>=MULTI_RELAY_MAX_RELAYS || _relayPin[relay]<0) return;
      pinMode(_relayPin[relay], OUTPUT);
      bool mode = bri ? _relayMode[relay] : !_relayMode[relay];
      digitalWrite(_relayPin[relay], mode);
      publishMqtt(mode ? "on" : "off", relay);
    }


    void publishMqtt(const char* state, int relay) {
      //Check if MQTT Connected, otherwise it will crash the 8266
      if (WLED_MQTT_CONNECTED){
        char subuf[64];
        sprintf_P(subuf, PSTR("%s/relay/%d"), mqttDeviceTopic, relay);
        mqtt->publish(subuf, 0, true, state);
      }
    }

    /**
     * switch off the strip if the delay has elapsed 
     */
    void handleOffTimer() {
      bool activeRelays = false;
      for (uint8_t i=0; i<MULTI_RELAY_MAX_RELAYS; i++) {
        if (_relayActive[i] && _switchTimerStart > 0 && millis() - _switchTimerStart > (_relayDelay[i]*1000)) {
          switchRelay(i);  // toggle relay
          _relayActive[i] = false;
        }
        activeRelays = activeRelays || _relayActive[i];
      }
      if (!activeRelays) _switchTimerStart = 0;
    }

  public:
    /**
     * constructor
     */
    MultiRelay() {
      for (uint8_t i=0; i<MULTI_RELAY_MAX_RELAYS; i++) {
        _relayPin[i] = -1;
        _relayMode[i] = false;
        _relayDelay[i] = 0;
        _relayActive[i] = false;
      }
    }
    /**
     * desctructor
     */
    ~MultiRelay() {}

    /**
     * Enable/Disable the usermod
     */
    void enable(bool enable) { enabled = enable; }
    /**
     * Get usermod enabled/disabled state
     */
    bool isEnabled() { return enabled; }

    //Functions called by WLED

    /**
     * setup() is called once at boot. WiFi is not yet connected at this point.
     * You can use it to initialize variables, sensors or similar.
     */
    void setup() {
      // pins retrieved from cfg.json (readFromConfig()) prior to running setup()
      for (uint8_t i=0; i<MULTI_RELAY_MAX_RELAYS; i++) {
        if (_relayPin[i]<0) continue;
        if (!pinManager.allocatePin(_relayPin[i],true)) {
          _relayPin[i] = -1;  // allocation failed
        } else {
          switchRelay(i);
          _relayActive[i] = false;
        }
      }
      _oldBrightness = bri;
      initDone = true;
    }

    /**
     * connected() is called every time the WiFi is (re)connected
     * Use it to initialize network interfaces
     */
    void connected() {}

    /**
     * loop() is called continuously. Here you can check for events, read sensors, etc.
     */
    void loop() {
      if (!enabled) return;

      static unsigned long lastUpdate = 0;
      if (millis() - lastUpdate < 200) return;  // update only 5 times/s
      lastUpdate = millis();

      //set relay when LEDs turn on
      if (_oldBrightness != bri) {
        _oldBrightness = bri;
        _switchTimerStart = millis();
        for (uint8_t i=0; i<MULTI_RELAY_MAX_RELAYS; i++) {
          if (_relayPin[i]>=0) _relayActive[i] = true;
        }
      }

      handleOffTimer();
    }

    /**
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     */
    void addToJsonInfo(JsonObject &root) {
      if (enabled) {
        uint8_t count = 0;
        for (uint8_t i=0; i<MULTI_RELAY_MAX_RELAYS; i++) if (_relayPin[i]>=0) count++;

        JsonObject user = root["u"];
        if (user.isNull())
          user = root.createNestedObject("u");

        JsonArray infoArr = user.createNestedArray(F("Number of relays")); //name
        infoArr.add(String(count));
      }
    }

    /**
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void addToJsonState(JsonObject &root) {
    }

    /**
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void readFromJsonState(JsonObject &root) {
    }

    /**
     * provide the changeable values
     */
    void addToConfig(JsonObject &root) {
      JsonObject top = root.createNestedObject(FPSTR(_name));

      top[FPSTR(_enabled)] = enabled;
      for (uint8_t i=0; i<MULTI_RELAY_MAX_RELAYS; i++) {
        String parName = FPSTR(_relay); parName += "-"; parName += i; parName += "-";
        top[parName+"pin"]              = _relayPin[i];
        top[parName+FPSTR(_activeHigh)] = _relayMode[i];
        top[parName+FPSTR(_delay)]      = _relayDelay[i];
      }
      DEBUG_PRINTLN(F("MultiRelay config saved."));
    }

    /**
     * restore the changeable values
     * readFromConfig() is called before setup() to populate properties from values stored in cfg.json
     */
    void readFromConfig(JsonObject &root) {
      int8_t oldPin[MULTI_RELAY_MAX_RELAYS];

      JsonObject top = root[FPSTR(_name)];
      if (top.isNull()) return;

      if (top[FPSTR(_enabled)] != nullptr) {
        if (top[FPSTR(_enabled)].is<bool>()) {
          enabled = top[FPSTR(_enabled)].as<bool>(); // reading from cfg.json
        } else {
          // change from settings page
          String str = top[FPSTR(_enabled)]; // checkbox -> off or on
          enabled = (bool)(str!="off"); // off is guaranteed to be present
        }
      }

      for (uint8_t i=0; i<MULTI_RELAY_MAX_RELAYS; i++) {
        String parName = FPSTR(_relay); parName += "-"; parName += i; parName += "-";

        oldPin[i] = _relayPin[i];
        if (top[parName+"pin"] != nullptr) _relayPin[i] = min(39,max(-1,top[parName+"pin"].as<int>()));

        if (top[parName+FPSTR(_activeHigh)] != nullptr) {
          if (top[parName+FPSTR(_activeHigh)].is<bool>()) {
            _relayMode[i] = top[parName+FPSTR(_activeHigh)].as<bool>(); // reading from cfg.json
          } else {
            // change from settings page
            String str = top[parName+FPSTR(_activeHigh)]; // checkbox -> off or on
            _relayMode[i] = (bool)(str!="off"); // off is guaranteed to be present
          }
        }

        _relayDelay[i] = min(600,max(0,abs(top[parName+FPSTR(_delay)].as<int>())));
      }

      if (!initDone) {
        // reading config prior to setup()
        DEBUG_PRINTLN(F("MultiRelay config loaded."));
      } else {
        // deallocate all pins 1st
        for (uint8_t i=0; i<MULTI_RELAY_MAX_RELAYS; i++)
          if (oldPin[i]>=0) {
            pinManager.deallocatePin(oldPin[i]);
          }
        // allocate new pins
        for (uint8_t i=0; i<MULTI_RELAY_MAX_RELAYS; i++) {
          if (_relayPin[i]>=0 && pinManager.allocatePin(_relayPin[i],true)) {
            switchRelay(i);
          } else {
            _relayPin[i] = -1;
          }
          _relayActive[i] = false;
        }
        DEBUG_PRINTLN(F("MultiRelay config (re)loaded."));
      }
    }

    /**
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
    uint16_t getId()
    {
      return USERMOD_ID_MULTI_RELAY;
    }
};

// strings to reduce flash memory usage (used more than twice)
const char MultiRelay::_name[]       PROGMEM = "MultiRelay";
const char MultiRelay::_enabled[]    PROGMEM = "enabled";
const char MultiRelay::_relay[]      PROGMEM = "relay";
const char MultiRelay::_delay[]      PROGMEM = "delay-s";
const char MultiRelay::_activeHigh[] PROGMEM = "active-high";