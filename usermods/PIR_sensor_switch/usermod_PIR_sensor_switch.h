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

// MQTT topic for sensor values
const char MQTT_TOPIC[] = "/motion";

class PIRsensorSwitch : public Usermod
{
public:
  /**
   * constructor
   */
  PIRsensorSwitch()
  {
    // set static instance pointer
    PIRsensorSwitchInstance(this);
  }
  /**
   * desctructor
   */
  ~PIRsensorSwitch()
  {
    PIRsensorSwitchInstance(nullptr, true);
    ;
  }

  /**
   * return the instance pointer of the class
   */
  static PIRsensorSwitch *GetInstance() { return PIRsensorSwitchInstance(); }

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
  uint32_t m_switchOffDelay = 600000;
  // off timer start time
  uint32_t m_offTimerStart = 0;
  // current PIR sensor pin state
  byte m_PIRsensorPinState = LOW;
  // PIR sensor enabled - ISR attached
  bool m_PIRenabled = true;
  // state if serializeConfig() should be called
  bool m_updateConfig = false;

  /**
   * return or change if new PIR sensor state is available
   */
  static volatile bool newPIRsensorState(bool changeState = false, bool newState = false);

  /**
   * PIR sensor state has changed
   */
  static void IRAM_ATTR ISR_PIRstateChange();

  /**
   * Set/get instance pointer
   */
  static PIRsensorSwitch *PIRsensorSwitchInstance(PIRsensorSwitch *pInstance = nullptr, bool bRemoveInstance = false);

  /**
   * switch strip on/off
   */
  void switchStrip(bool switchOn)
  {
    if (switchOn && bri == 0)
    {
      bri = briLast;
      colorUpdated(NotifyUpdateMode);
    }
    else if (!switchOn && bri != 0)
    {
      briLast = bri;
      bri = 0;
      colorUpdated(NotifyUpdateMode);
    }
  }

  void publishMqtt(const char* state)
  {
    //Check if MQTT Connected, otherwise it will crash the 8266
    if (mqtt != nullptr){
      char subuf[64];
      strcpy(subuf, mqttDeviceTopic);
      strcat(subuf, MQTT_TOPIC);
      mqtt->publish(subuf, 0, true, state);
    }
  }

  /**
   * Read and update PIR sensor state.
   * Initilize/reset switch off timer
   */
  bool updatePIRsensorState()
  {
    if (newPIRsensorState())
    {
      m_PIRsensorPinState = digitalRead(PIRsensorPin);

      if (m_PIRsensorPinState == HIGH)
      {
        m_offTimerStart = 0;
        switchStrip(true);
        publishMqtt("on");
      }
      else if (bri != 0)
      {
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
      if (m_PIRenabled)
      {
        // assign interrupt function and set CHANGE mode
        attachInterrupt(digitalPinToInterrupt(PIRsensorPin), ISR_PIRstateChange, CHANGE);
      }
    }
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
    if (!updatePIRsensorState())
    {
      handleOffTimer();
      if (m_updateConfig)
      {
        serializeConfig();
        m_updateConfig = false;
      }
    }
  }

  /**
   * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
   * 
   * Add PIR sensor state and switch off timer duration to jsoninfo
   */
  void addToJsonInfo(JsonObject &root)
  {
    //this code adds "u":{"&#x23F2; PIR sensor state":uiDomString} to the info object
    // the value contains a button to toggle the sensor enabled/disabled
    JsonObject user = root["u"];
    if (user.isNull())
      user = root.createNestedObject("u");

    JsonArray infoArr = user.createNestedArray("<i class=\"icons\">&#xe08f;</i> PIR sensor state"); //name
    String uiDomString = "<button class=\"btn infobtn\" onclick=\"requestJson({PIRenabled:";
    String sensorStateInfo;

    // PIR sensor state
    if (m_PIRenabled)
    {
      uiDomString += "false";
      sensorStateInfo = (m_PIRsensorPinState != LOW ? "active" : "inactive"); //value
    }
    else
    {
      uiDomString += "true";
      sensorStateInfo = "Disabled !";
    }
    uiDomString += "});return false;\">";
    uiDomString += sensorStateInfo;
    uiDomString += "</button>";
    infoArr.add(uiDomString); //value

    //this code adds "u":{"&#x23F2; switch off timer":uiDomString} to the info object
    uiDomString = "<i class=\"icons\">&#xe325;</i> switch off timer<span style=\"display:block;padding-left:25px;\">\
after <input type=\"number\" min=\"1\" max=\"720\" value=\"";
    uiDomString += (m_switchOffDelay / 60000);
    uiDomString += "\" onchange=\"requestJson({PIRoffSec:parseInt(this.value)*60});\">min</span>";
    infoArr = user.createNestedArray(uiDomString); //name

    // off timer
    if (m_offTimerStart > 0)
    {
      uiDomString = "";
      unsigned int offSeconds = (m_switchOffDelay - (millis() - m_offTimerStart)) / 1000;
      if (offSeconds >= 3600)
      {
        uiDomString += (offSeconds / 3600);
        uiDomString += " hours ";
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
        uiDomString += " min ";
      }
      uiDomString += (offSeconds);
      infoArr.add(uiDomString + " sec");
    }
    else
    {
      infoArr.add("inactive");
    }
  }

  /**
   * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
   * Values in the state object may be modified by connected clients
   * Add "PIRenabled" to json state. This can be used to disable/enable the sensor.
   * Add "PIRoffSec" to json state. This can be used to adjust <m_switchOffDelay> milliseconds.
   */
  void addToJsonState(JsonObject &root)
  {
    root[F("PIRenabled")] = m_PIRenabled;
    root[F("PIRoffSec")] = (m_switchOffDelay / 1000);
  }

  /**
   * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
   * Values in the state object may be modified by connected clients
   * Read "PIRenabled" from json state and switch enable/disable the PIR sensor.
   * Read "PIRoffSec" from json state and adjust <m_switchOffDelay> milliseconds.
   */
  void readFromJsonState(JsonObject &root)
  {
    if (root[F("PIRoffSec")] != nullptr)
    {
      m_switchOffDelay = (1000 * max(60UL, min(43200UL, root[F("PIRoffSec")].as<unsigned long>())));
      m_updateConfig = true;
    }

    if (root[F("pin")] != nullptr)
    {
      int8_t pin = (int)root[F("pin")];
      // check if pin is OK
      if (pin != PIRsensorPin && pin>=0 && pinManager.allocatePin(pin,false)) {
        // deallocate old pin
        pinManager.deallocatePin(PIRsensorPin);
        // PIR Sensor mode INPUT_PULLUP
        pinMode(pin, INPUT_PULLUP);
        if (m_PIRenabled)
        {
          // remove old ISR
          detachInterrupt(PIRsensorPin);
          // assign interrupt function and set CHANGE mode
          attachInterrupt(digitalPinToInterrupt(pin), ISR_PIRstateChange, CHANGE);
          newPIRsensorState(true, true);
        }
        PIRsensorPin = pin;
        m_updateConfig = true;
      }
    }

    if (root[F("PIRenabled")] != nullptr)
    {
      if (root[F("PIRenabled")] && !m_PIRenabled)
      {
        attachInterrupt(digitalPinToInterrupt(PIRsensorPin), ISR_PIRstateChange, CHANGE);
        newPIRsensorState(true, true);
      }
      else if (m_PIRenabled)
      {
        detachInterrupt(PIRsensorPin);
      }
      m_PIRenabled = root[F("PIRenabled")];
      m_updateConfig = true;
    }
  }

  /**
   * provide the changeable values
   */
  void addToConfig(JsonObject &root)
  {
    JsonObject top = root.createNestedObject(F("PIRsensorSwitch"));
    top[F("PIRenabled")] = m_PIRenabled;
    top[F("PIRoffSec")] = m_switchOffDelay;
    top[F("pin")] = PIRsensorPin;
  }

  /**
   * restore the changeable values
   * readFromConfig() is called before setup() to populate properties from values stored in cfg.json
   */
  void readFromConfig(JsonObject &root)
  {
    JsonObject top = root[F("PIRsensorSwitch")];
    if (!top.isNull() && top[F("pin")] != nullptr) {
      PIRsensorPin = (int)top[F("pin")];
    }
    m_PIRenabled = (top[F("PIRenabled")] != nullptr ? top[F("PIRenabled")] : true);
    m_switchOffDelay = top[F("PIRoffSec")] | m_switchOffDelay;
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
  if (changeState)
  {
    s_PIRsensorState = newState;
  }
  return s_PIRsensorState;
}

void IRAM_ATTR PIRsensorSwitch::ISR_PIRstateChange()
{
  newPIRsensorState(true, true);
}

PIRsensorSwitch *PIRsensorSwitch::PIRsensorSwitchInstance(PIRsensorSwitch *pInstance, bool bRemoveInstance)
{
  static PIRsensorSwitch *s_pPIRsensorSwitch = nullptr;
  if (pInstance != nullptr || bRemoveInstance)
  {
    s_pPIRsensorSwitch = pInstance;
  }
  return s_pPIRsensorSwitch;
}
