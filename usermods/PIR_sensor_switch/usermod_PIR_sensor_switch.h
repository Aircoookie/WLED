#pragma once

#include "wled.h"

#ifndef PIR_SENSOR_PIN
  // compatible with QuinLED-Dig-Uno
  #ifdef ARDUINO_ARCH_ESP32
    #define PIR_SENSOR_PIN 23 // Q4
  #else //ESP8266 boards
    #define PIR_SENSOR_PIN 13 // Q4 (D7 on D1 mini)
  #endif
#endif

/*
 * This usermod handles PIR sensor states.
 * The strip will be switched on and the off timer will be resetted when the sensor goes HIGH. 
 * When the sensor state goes LOW, the off timer is started and when it expires, the strip is switched off. 
 * 
 * 
 * Usermods allow you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * 
 * v2 usermods are class inheritance based and can (but don't have to) implement more functions, each of them is shown in this example.
 * Multiple v2 usermods can be added to one compilation easily.
 * 
 * Creating a usermod:
 * This file serves as an example. If you want to create a usermod, it is recommended to use usermod_v2_empty.h from the usermods folder as a template.
 * Please remember to rename the class and file to a descriptive name.
 * You may also use multiple .h and .cpp files.
 * 
 * Using a usermod:
 * 1. Copy the usermod into the sketch folder (same folder as wled00.ino)
 * 2. Register the usermod by adding #include "usermod_filename.h" in the top and registerUsermod(new MyUsermodClass()) in the bottom of usermods_list.cpp
 */

class PIRsensorSwitch : public Usermod
{
public:
  /**
   * constructor
   */
  PIRsensorSwitch() {}
  /**
   * desctructor
   */
  ~PIRsensorSwitch() {}

  /**
   * Enable/Disable the PIR sensor
   */
  void EnablePIRsensor(bool enable) { m_PIRenabled = enable; }
  /**
   * Get PIR sensor enabled/disabled state
   */
  bool PIRsensorEnabled() { return m_PIRenabled; }

private:
  // PIR sensor pin
  int8_t PIRsensorPin = PIR_SENSOR_PIN;
  // notification mode for colorUpdated()
  const byte NotifyUpdateMode = NOTIFIER_CALL_MODE_NO_NOTIFY; // NOTIFIER_CALL_MODE_DIRECT_CHANGE
  // delay before switch off after the sensor state goes LOW
  uint32_t m_switchOffDelay = 600000; // 10min
  // off timer start time
  uint32_t m_offTimerStart = 0;
  // current PIR sensor pin state
  byte m_PIRsensorPinState = LOW;
  // PIR sensor enabled - ISR attached
  bool m_PIRenabled = true;
  // status of initialisation
  bool initDone = false;
  // on and off presets
  uint8_t m_onPreset = 0;
  uint8_t m_offPreset = 0;

  unsigned long lastLoop = 0;

  // strings to reduce flash memory usage (used more than twice)
  static const char _name[];
  static const char _switchOffDelay[];
  static const char _enabled[];
  static const char _onPreset[];
  static const char _offPreset[];

  /**
   * return or change if new PIR sensor state is available
   */
  static volatile bool newPIRsensorState(bool changeState = false, bool newState = false);

  /**
   * PIR sensor state has changed
   */
  static void IRAM_ATTR ISR_PIRstateChange();

  /**
   * switch strip on/off
   */
  void switchStrip(bool switchOn)
  {
    if (switchOn && m_onPreset) {
      applyPreset(m_onPreset);
    } else if (!switchOn && m_offPreset) {
      applyPreset(m_offPreset);
    } else if (switchOn && bri == 0) {
      bri = briLast;
      colorUpdated(NotifyUpdateMode);
    } else if (!switchOn && bri != 0) {
      briLast = bri;
      bri = 0;
      colorUpdated(NotifyUpdateMode);
    }
  }

  void publishMqtt(const char* state)
  {
    //Check if MQTT Connected, otherwise it will crash the 8266
    if (WLED_MQTT_CONNECTED){
      char subuf[64];
      strcpy(subuf, mqttDeviceTopic);
      strcat_P(subuf, PSTR("/motion"));
      mqtt->publish(subuf, 0, true, state);
    }
  }

  /**
   * Read and update PIR sensor state.
   * Initilize/reset switch off timer
   */
  bool updatePIRsensorState()
  {
    if (newPIRsensorState()) {
      m_PIRsensorPinState = digitalRead(PIRsensorPin);

      if (m_PIRsensorPinState == HIGH) {
        m_offTimerStart = 0;
        switchStrip(true);
        publishMqtt("on");
      } else /*if (bri != 0)*/ {
        // start switch off timer
        m_offTimerStart = millis();
      }
      newPIRsensorState(true, false);
      return true;
    }
    return false;
  }

  /**
   * switch off the strip if the delay has elapsed 
   */
  bool handleOffTimer()
  {
    if (m_offTimerStart > 0 && millis() - m_offTimerStart > m_switchOffDelay)
    {
      if (m_PIRenabled == true)
      {
        switchStrip(false);
        publishMqtt("off");
      }
      m_offTimerStart = 0;
      return true;
    }
    return false;
  }

public:
  //Functions called by WLED

  /**
   * setup() is called once at boot. WiFi is not yet connected at this point.
   * You can use it to initialize variables, sensors or similar.
   */
  void setup()
  {
    // pin retrieved from cfg.json (readFromConfig()) prior to running setup()
    if (!pinManager.allocatePin(PIRsensorPin,false)) {
      PIRsensorPin = -1;  // allocation failed
      m_PIRenabled = false;
      DEBUG_PRINTLN(F("PIRSensorSwitch pin allocation failed."));
    } else {
      // PIR Sensor mode INPUT_PULLUP
      pinMode(PIRsensorPin, INPUT_PULLUP);
      if (m_PIRenabled) {
        // assign interrupt function and set CHANGE mode
        attachInterrupt(digitalPinToInterrupt(PIRsensorPin), ISR_PIRstateChange, CHANGE);
        newPIRsensorState(true, false);
      }
    }
    initDone = true;
  }

  /**
   * connected() is called every time the WiFi is (re)connected
   * Use it to initialize network interfaces
   */
  void connected()
  {
  }

  /**
   * loop() is called continuously. Here you can check for events, read sensors, etc.
   */
  void loop()
  {
    // only check sensors 10x/s
    unsigned long now = millis();
    if (now - lastLoop < 100) return;
    lastLoop = now;

    if (!updatePIRsensorState()) {
      handleOffTimer();
    }
  }

  /**
   * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
   * 
   * Add PIR sensor state and switch off timer duration to jsoninfo
   */
  void addToJsonInfo(JsonObject &root)
  {
    JsonObject user = root["u"];
    if (user.isNull()) user = root.createNestedObject("u");

    if (m_PIRenabled)
    {
      // off timer
      String uiDomString = F("PIR <i class=\"icons\">&#xe325;</i>");
      JsonArray infoArr = user.createNestedArray(uiDomString); // timer value
      if (m_offTimerStart > 0)
      {
        uiDomString = "";
        unsigned int offSeconds = (m_switchOffDelay - (millis() - m_offTimerStart)) / 1000;
        if (offSeconds >= 3600)
        {
          uiDomString += (offSeconds / 3600);
          uiDomString += F("h ");
          offSeconds %= 3600;
        }
        if (offSeconds >= 60)
        {
          uiDomString += (offSeconds / 60);
          offSeconds %= 60;
        }
        else if (uiDomString.length() > 0)
        {
          uiDomString += 0;
        }
        if (uiDomString.length() > 0)
        {
          uiDomString += F("min ");
        }
        uiDomString += (offSeconds);
        infoArr.add(uiDomString + F("s"));
      } else {
        infoArr.add(F("inactive"));
      }
    } else {
      String uiDomString = F("PIR sensor");
      JsonArray infoArr = user.createNestedArray(uiDomString);
      infoArr.add(F("disabled"));
    }
  }

  /**
   * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
   * Values in the state object may be modified by connected clients
   */
/*
  void addToJsonState(JsonObject &root)
  {
  }
*/

  /**
   * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
   * Values in the state object may be modified by connected clients
   */
/*
  void readFromJsonState(JsonObject &root)
  {
  }
*/

  /**
   * provide the changeable values
   */
  void addToConfig(JsonObject &root)
  {
    JsonObject top = root.createNestedObject(FPSTR(_name));
    top[FPSTR(_enabled)] = m_PIRenabled;
    top[FPSTR(_switchOffDelay)] = m_switchOffDelay / 1000;
    top["pin"] = PIRsensorPin;
    top[FPSTR(_onPreset)] = m_onPreset;
    top[FPSTR(_offPreset)] = m_offPreset;
    DEBUG_PRINTLN(F("PIR config saved."));
  }

  /**
   * restore the changeable values
   * readFromConfig() is called before setup() to populate properties from values stored in cfg.json
   */
  void readFromConfig(JsonObject &root)
  {
    bool oldEnabled = m_PIRenabled;
    int8_t oldPin = PIRsensorPin;

    JsonObject top = root[FPSTR(_name)];
    if (top.isNull()) return;

    if (top["pin"] != nullptr) {
      PIRsensorPin = min(39,max(-1,top["pin"].as<int>())); // check bounds
    }

    if (top[FPSTR(_enabled)] != nullptr) {
      if (top[FPSTR(_enabled)].is<bool>()) {
        m_PIRenabled = top[FPSTR(_enabled)].as<bool>(); // reading from cfg.json
      } else {
        // change from settings page
        String str = top[FPSTR(_enabled)]; // checkbox -> off or on
        m_PIRenabled = (bool)(str!="off"); // off is guaranteed to be present
      }
    }

    if (top[FPSTR(_switchOffDelay)] != nullptr) {
      m_switchOffDelay = (top[FPSTR(_switchOffDelay)].as<int>() * 1000);
    }

    if (top[FPSTR(_onPreset)] != nullptr) {
      m_onPreset = max(0,min(250,top[FPSTR(_onPreset)].as<int>()));
    }

    if (top[FPSTR(_offPreset)] != nullptr) {
      m_offPreset = max(0,min(250,top[FPSTR(_offPreset)].as<int>()));
    }

    if (!initDone) {
      // reading config prior to setup()
      DEBUG_PRINTLN(F("PIR config loaded."));
    } else {
      if (oldPin != PIRsensorPin || oldEnabled != m_PIRenabled) {
        if (oldEnabled) {
          // remove old ISR if disabling usermod
          detachInterrupt(oldPin);
        }
        // check if pin is OK
        if (oldPin != PIRsensorPin && oldPin >= 0) {
          // if we are changing pin in settings page
          // deallocate old pin
          pinManager.deallocatePin(oldPin);
          if (pinManager.allocatePin(PIRsensorPin,false)) {
            pinMode(PIRsensorPin, INPUT_PULLUP);
          } else {
            // allocation failed
            PIRsensorPin = -1;
            m_PIRenabled = false;
          }
        }
        if (m_PIRenabled) {
          attachInterrupt(digitalPinToInterrupt(PIRsensorPin), ISR_PIRstateChange, CHANGE);
          newPIRsensorState(true, false);
        }
        DEBUG_PRINTLN(F("PIR config (re)loaded."));
      }
    }
  }

  /**
   * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
   * This could be used in the future for the system to determine whether your usermod is installed.
   */
  uint16_t getId()
  {
    return USERMOD_ID_PIRSWITCH;
  }
};

//////////////////////////////////////////////////////
// PIRsensorSwitch static method implementations

volatile bool PIRsensorSwitch::newPIRsensorState(bool changeState, bool newState)
{
  static volatile bool s_PIRsensorState = false;
  if (changeState) s_PIRsensorState = newState;
  return s_PIRsensorState;
}

void IRAM_ATTR PIRsensorSwitch::ISR_PIRstateChange()
{
  newPIRsensorState(true, true);
}

// strings to reduce flash memory usage (used more than twice)
const char PIRsensorSwitch::_name[]           PROGMEM = "PIRsensorSwitch";
const char PIRsensorSwitch::_enabled[]        PROGMEM = "PIRenabled";
const char PIRsensorSwitch::_switchOffDelay[] PROGMEM = "PIRoffSec";
const char PIRsensorSwitch::_onPreset[]       PROGMEM = "on-preset";
const char PIRsensorSwitch::_offPreset[]      PROGMEM = "off-preset";
