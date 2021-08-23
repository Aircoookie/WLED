#pragma once

#include "wled.h"

#ifndef MULTI_RELAY_MAX_RELAYS
  #define MULTI_RELAY_MAX_RELAYS 4
#endif

#define ON  true
#define OFF false

/*
 * This usermod handles multiple relay outputs.
 * These outputs complement built-in relay output in a way that the activation can be delayed.
 * They can also activate/deactivate in reverse logic independently.
 */


typedef struct relay_t {
  int8_t pin;
  bool active;
  bool mode;
  bool state;
  bool external;
  uint16_t delay;
} Relay;


class MultiRelay : public Usermod {

  private:
    // array of relays
    Relay _relay[MULTI_RELAY_MAX_RELAYS];

    // switch timer start time
    uint32_t _switchTimerStart = 0;
    // old brightness
    bool _oldBrightness = 0;

    // usermod enabled
    bool enabled = false;  // needs to be configured (no default config)
    // status of initialisation
    bool initDone = false;

    // strings to reduce flash memory usage (used more than twice)
    static const char _name[];
    static const char _enabled[];
    static const char _relay_str[];
    static const char _delay_str[];
    static const char _activeHigh[];
    static const char _external[];


    void publishMqtt(const char* state, int relay) {
      //Check if MQTT Connected, otherwise it will crash the 8266
      if (WLED_MQTT_CONNECTED){
        char subuf[64];
        sprintf_P(subuf, PSTR("%s/relay/%d"), mqttDeviceTopic, relay);
        mqtt->publish(subuf, 0, false, state);
      }
    }

    /**
     * switch off the strip if the delay has elapsed 
     */
    void handleOffTimer() {
      bool activeRelays = false;
      for (uint8_t i=0; i<MULTI_RELAY_MAX_RELAYS; i++) {
        if (_relay[i].active && _switchTimerStart > 0 && millis() - _switchTimerStart > (_relay[i].delay*1000)) {
          if (!_relay[i].external) toggleRelay(i);
          _relay[i].active = false;
        }
        activeRelays = activeRelays || _relay[i].active;
      }
      if (!activeRelays) _switchTimerStart = 0;
    }

    /**
     * HTTP API handler
     * borrowed from:
     * https://github.com/gsieben/WLED/blob/master/usermods/GeoGab-Relays/usermod_GeoGab.h
     */
    #define GEOGABVERSION "0.1.3"
    void InitHtmlAPIHandle() {  // https://github.com/me-no-dev/ESPAsyncWebServer
      DEBUG_PRINTLN(F("Relays: Initialize HTML API"));

      server.on("/relays", HTTP_GET, [this](AsyncWebServerRequest *request) {
        DEBUG_PRINTLN("Relays: HTML API");
        String janswer;
        String error = "";
        //int params = request->params();
        janswer = F("{\"NoOfRelays\":");
        janswer += String(MULTI_RELAY_MAX_RELAYS) + ",";

        if (getActiveRelayCount()) {
          // Commands
          if(request->hasParam("switch")) {
            /**** Switch ****/
            AsyncWebParameter* p = request->getParam("switch");
            // Get Values
            for (int i=0; i<MULTI_RELAY_MAX_RELAYS; i++) {
              int value = getValue(p->value(), ',', i);
              if (value==-1) {
                error = F("There must be as much arugments as relays");
              } else {
                // Switch
                if (_relay[i].external) switchRelay(i, (bool)value);
              }
            }
          } else if(request->hasParam("toggle")) {
            /**** Toggle ****/
            AsyncWebParameter* p = request->getParam("toggle");
            // Get Values
            for (int i=0;i<MULTI_RELAY_MAX_RELAYS;i++) {
              int value = getValue(p->value(), ',', i);
              if (value==-1) {
                error = F("There must be as mutch arugments as relays");
              } else {
                // Toggle
                if (value && _relay[i].external) toggleRelay(i);
              }
            }
          } else {
            error = F("No valid command found");
          }
        } else {
          error = F("No active relays");
        }

        // Status response
        char sbuf[16];
        for (int i=0; i<MULTI_RELAY_MAX_RELAYS; i++) {
          sprintf_P(sbuf, PSTR("\"%d\":%d,"), i, (_relay[i].pin<0 ? -1 : (int)_relay[i].state));
          janswer += sbuf;
        }
        janswer += F("\"error\":\"");
        janswer += error;
        janswer += F("\",");
        janswer += F("\"SW Version\":\"");
        janswer += String(GEOGABVERSION);
        janswer += F("\"}");
        request->send(200, "application/json", janswer);
      });
    }

    int getValue(String data, char separator, int index) {
      int found = 0;
      int strIndex[] = {0, -1};
      int maxIndex = data.length()-1;

      for(int i=0; i<=maxIndex && found<=index; i++){
        if(data.charAt(i)==separator || i==maxIndex){
            found++;
            strIndex[0] = strIndex[1]+1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
      }
      return found>index ? data.substring(strIndex[0], strIndex[1]).toInt() : -1;
    }

  public:
    /**
     * constructor
     */
    MultiRelay() {
      for (uint8_t i=0; i<MULTI_RELAY_MAX_RELAYS; i++) {
        _relay[i].pin      = -1;
        _relay[i].delay    = 0;
        _relay[i].mode     = false;
        _relay[i].active   = false;
        _relay[i].state    = false;
        _relay[i].external = false;
      }
    }
    /**
     * desctructor
     */
    ~MultiRelay() {}

    /**
     * Enable/Disable the usermod
     */
    inline void enable(bool enable) { enabled = enable; }
    /**
     * Get usermod enabled/disabled state
     */
    inline bool isEnabled() { return enabled; }

    /**
     * switch relay on/off
     */
    void switchRelay(uint8_t relay, bool mode) {
      if (relay>=MULTI_RELAY_MAX_RELAYS || _relay[relay].pin<0) return;
      _relay[relay].state = mode;
      pinMode(_relay[relay].pin, OUTPUT);
      digitalWrite(_relay[relay].pin, mode ? !_relay[relay].mode : _relay[relay].mode);
      publishMqtt(mode ? "on" : "off", relay);
    }

    /**
     * toggle relay
     */
    inline void toggleRelay(uint8_t relay) {
      switchRelay(relay, !_relay[relay].state);
    }

    uint8_t getActiveRelayCount() {
      uint8_t count = 0;
      for (uint8_t i=0; i<MULTI_RELAY_MAX_RELAYS; i++) if (_relay[i].pin>=0) count++;
      return count;
    }

    //Functions called by WLED

    /**
     * handling of MQTT message
     * topic only contains stripped topic (part after /wled/MAC)
     * topic should look like: /relay/X/command; where X is relay number, 0 based
     */
    bool onMqttMessage(char* topic, char* payload) {
      if (strlen(topic) > 8 && strncmp_P(topic, PSTR("/relay/"), 7) == 0 && strncmp_P(topic+8, PSTR("/command"), 8) == 0) {
        uint8_t relay = strtoul(topic+7, NULL, 10);
        if (relay<MULTI_RELAY_MAX_RELAYS) {
          String action = payload;
          if (action == "on") {
            if (_relay[relay].external) switchRelay(relay, true);
            return true;
          } else if (action == "off") {
            if (_relay[relay].external) switchRelay(relay, false);
            return true;
          } else if (action == "toggle") {
            if (_relay[relay].external) toggleRelay(relay);
            return true;
          }
        }
      }
      return false;
    }

    /**
     * subscribe to MQTT topic for controlling relays
     */
    void onMqttConnect(bool sessionPresent) {
      //(re)subscribe to required topics
      char subuf[64];
      if (mqttDeviceTopic[0] != 0) {
        strcpy(subuf, mqttDeviceTopic);
        strcat_P(subuf, PSTR("/relay/#"));
        mqtt->subscribe(subuf, 0);
      }
    }

    /**
     * setup() is called once at boot. WiFi is not yet connected at this point.
     * You can use it to initialize variables, sensors or similar.
     */
    void setup() {
      // pins retrieved from cfg.json (readFromConfig()) prior to running setup()
      for (uint8_t i=0; i<MULTI_RELAY_MAX_RELAYS; i++) {
        if (_relay[i].pin<0) continue;
        if (!pinManager.allocatePin(_relay[i].pin,true, PinOwner::UM_MultiRelay)) {
          _relay[i].pin = -1;  // allocation failed
        } else {
          switchRelay(i, _relay[i].state = (bool)bri);
          _relay[i].active = false;
        }
      }
      _oldBrightness = (bool)bri;
      initDone = true;
    }

    /**
     * connected() is called every time the WiFi is (re)connected
     * Use it to initialize network interfaces
     */
    void connected() {
      InitHtmlAPIHandle();
    }

    /**
     * loop() is called continuously. Here you can check for events, read sensors, etc.
     */
    void loop() {
      if (!enabled || strip.isUpdating()) return;

      static unsigned long lastUpdate = 0;
      if (millis() - lastUpdate < 200) return;  // update only 5 times/s
      lastUpdate = millis();

      //set relay when LEDs turn on
      if (_oldBrightness != (bool)bri) {
        _oldBrightness = (bool)bri;
        _switchTimerStart = millis();
        for (uint8_t i=0; i<MULTI_RELAY_MAX_RELAYS; i++) {
          if (_relay[i].pin>=0) _relay[i].active = true;
        }
      }

      handleOffTimer();
    }

    /**
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     */
    void addToJsonInfo(JsonObject &root) {
      if (enabled) {
        JsonObject user = root["u"];
        if (user.isNull())
          user = root.createNestedObject("u");

        JsonArray infoArr = user.createNestedArray(F("Number of relays")); //name
        infoArr.add(String(getActiveRelayCount()));
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
        String parName = FPSTR(_relay_str); parName += "-"; parName += i; parName += "-";
        top[parName+"pin"]              = _relay[i].pin;
        top[parName+FPSTR(_activeHigh)] = _relay[i].mode;
        top[parName+FPSTR(_delay_str)]  = _relay[i].delay;
        top[parName+FPSTR(_external)]   = _relay[i].external;
      }
      DEBUG_PRINTLN(F("MultiRelay config saved."));
    }

    /**
     * restore the changeable values
     * readFromConfig() is called before setup() to populate properties from values stored in cfg.json
     * 
     * The function should return true if configuration was successfully loaded or false if there was no configuration.
     */
    bool readFromConfig(JsonObject &root) {
      int8_t oldPin[MULTI_RELAY_MAX_RELAYS];

      JsonObject top = root[FPSTR(_name)];
      if (top.isNull()) {
        DEBUG_PRINT(FPSTR(_name));
        DEBUG_PRINTLN(F(": No config found. (Using defaults.)"));
        return false;
      }

      enabled = top[FPSTR(_enabled)] | enabled;

      for (uint8_t i=0; i<MULTI_RELAY_MAX_RELAYS; i++) {
        String parName = FPSTR(_relay_str); parName += "-"; parName += i; parName += "-";
        oldPin[i]          = _relay[i].pin;
        _relay[i].pin      = top[parName+"pin"] | _relay[i].pin;
        _relay[i].mode     = top[parName+FPSTR(_activeHigh)] | _relay[i].mode;
        _relay[i].external = top[parName+FPSTR(_external)]   | _relay[i].external;
        _relay[i].delay    = top[parName+FPSTR(_delay_str)]  | _relay[i].delay;
        _relay[i].delay    = min(600,max(0,abs((int)_relay[i].delay))); // bounds checking max 10min
      }

      DEBUG_PRINT(FPSTR(_name));
      if (!initDone) {
        // reading config prior to setup()
        DEBUG_PRINTLN(F(" config loaded."));
      } else {
        // deallocate all pins 1st
        for (uint8_t i=0; i<MULTI_RELAY_MAX_RELAYS; i++)
          if (oldPin[i]>=0) {
            pinManager.deallocatePin(oldPin[i], PinOwner::UM_MultiRelay);
          }
        // allocate new pins
        for (uint8_t i=0; i<MULTI_RELAY_MAX_RELAYS; i++) {
          if (_relay[i].pin>=0 && pinManager.allocatePin(_relay[i].pin, true, PinOwner::UM_MultiRelay)) {
            if (!_relay[i].external) {
              switchRelay(i, _relay[i].state = (bool)bri);
            }
          } else {
            _relay[i].pin = -1;
          }
          _relay[i].active = false;
        }
        DEBUG_PRINTLN(F(" config (re)loaded."));
      }
      // use "return !top["newestParameter"].isNull();" when updating Usermod with new features
      return true;
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
const char MultiRelay::_relay_str[]  PROGMEM = "relay";
const char MultiRelay::_delay_str[]  PROGMEM = "delay-s";
const char MultiRelay::_activeHigh[] PROGMEM = "active-high";
const char MultiRelay::_external[]   PROGMEM = "external";
