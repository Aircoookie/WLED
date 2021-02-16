#pragma once

#include "wled.h"

//This is an empty v2 usermod template. Please see the file usermod_v2_example.h in the EXAMPLE_v2 usermod folder for documentation on the functions you can use!

class mqttPIR : public Usermod {
  private:
   // PIR sensor pin
const int MOTION_PIN = 13;
 // MQTT topic for sensor values
#define MQTT_TOPIC  "/motion"
// notification mode for colorUpdated()
const byte NotifyUpdateMode = NOTIFIER_CALL_MODE_NO_NOTIFY; // NOTIFIER_CALL_MODE_DIRECT_CHANGE
// 1 min delay before switch off after the sensor state goes LOW
uint32_t m_switchOffDelay = 60000;
// off timer start time
uint32_t m_offTimerStart = 0;
// current PIR sensor pin state
//byte m_PIRsensorPinState = LOW;
// PIR sensor enabled - ISR attached
bool m_PIRenabled = true;
// state if serializeConfig() should be called
bool m_updateConfig = false;
// define analog pin
const int LIGHT_PIN = A0; 
// current light value
int m_lightValue = 0;
// there are light enought
bool m_lightEnought = false; 
// light treshold
int m_lightTreshold = 250;
// MQTT Topic
#define MQTT_TOPIC "/motion"
int prevState = LOW;
//light activated by PIR
bool m_pirActivated = false;


public:
  void setup() {
    pinMode(MOTION_PIN, INPUT);
    pinMode(LIGHT_PIN,INPUT);
  }


void publishMqtt(String state)
{
  //Check if MQTT Connected, otherwise it will crash the 8266
  if (mqtt != nullptr){
    char subuf[38];
    strcpy(subuf, mqttDeviceTopic);
    strcat(subuf, MQTT_TOPIC);
    mqtt->publish(subuf, 0, true, state.c_str());
  }
}

void switchStrip()
  {
      m_lightEnought = m_lightValue > m_lightTreshold ? true:false;
      if (m_lightEnought){
        applyPreset(random(2,6)); // preset 2-6 para cuando sea de dia.
        //bri = 140;
        colorUpdated(NotifyUpdateMode);
        
      }
      else{
        applyPreset(1); // El preset 1 sera la luz de noche.
        //bri = 90;
        colorUpdated(NotifyUpdateMode);
      }
  }

  void loop() {
      if(m_PIRenabled){
        if (digitalRead(MOTION_PIN) == HIGH && prevState == LOW) { // Motion detected
          publishMqtt("ON");
          prevState = HIGH;
          m_pirActivated = true;
          m_offTimerStart = millis();

        } 
        if (digitalRead(MOTION_PIN) == LOW && prevState == HIGH) {  // Motion stopped
          publishMqtt("OFF");
          prevState = LOW;
          //bri = 0;
          //colorUpdated(NotifyUpdateMode);
        }
      }
      if(m_pirActivated){
        if (bri <= 0){
          m_lightValue = analogRead(LIGHT_PIN);
          switchStrip();
        }
        if(millis() - m_offTimerStart > m_switchOffDelay){
          m_pirActivated = false;
          bri = 0;
          publishMqtt("timeOFF");
          colorUpdated(NotifyUpdateMode);
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

      JsonArray infoArr = user.createNestedArray("&#x23F2; PIR sensor state"); //name
      String uiDomString = "<button class=\"btn infobtn\" onclick=\"requestJson({PIRenabled:";
      String sensorStateInfo;

      // PIR sensor state
      if (m_PIRenabled)
      {
        uiDomString += "false";
        sensorStateInfo = (prevState != LOW ? "active" : "inactive"); //value
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
    uiDomString = "&#x23F2; switch off timer<span style=\"display:block;padding-left:25px;\">\
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
      //this code adds "u":{"&#x23F2; Ldr sensor data":uiDomString} to the info object
    uiDomString = "&#x23F2; LDR treshold adjust<span style=\"display:block;padding-left:25px;\">\
after <input type=\"number\" min=\"1\" max=\"1024\" value=\"";
    uiDomString += (m_lightTreshold);
    uiDomString += "\" onchange=\"requestJson({LDRadjust:parseInt(this.value)});\"/span>";
    infoArr = user.createNestedArray(uiDomString); //name

    // m_lightValue
      
      infoArr.add(m_lightValue);
      
    }

  /**
   * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
   * Values in the state object may be modified by connected clients
   * Add "PIRenabled" to json state. This can be used to disable/enable the sensor.
   * Add "PIRoffSec" to json state. This can be used to adjust <m_switchOffDelay> milliseconds.
   */
  void addToJsonState(JsonObject &root)
  {
    root["PIRenabled"] = m_PIRenabled;
    root["PIRoffSec"] = (m_switchOffDelay / 1000);
    root["LDRadjust"] = m_lightTreshold;
  }

  /**
   * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
   * Values in the state object may be modified by connected clients
   * Read "PIRenabled" from json state and switch enable/disable the PIR sensor.
   * Read "PIRoffSec" from json state and adjust <m_switchOffDelay> milliseconds.
   */
  void readFromJsonState(JsonObject &root)
  {
    if (root["PIRoffSec"] != nullptr)
    {
      m_switchOffDelay = (1000 * max(60UL, min(43200UL, root["PIRoffSec"].as<unsigned long>())));
      m_updateConfig = true;
    }
    if (root["LDRadjust"] != nullptr)
    {
      m_lightTreshold = max(0, min(1024, root["LDRadjust"].as<int>()));
      m_updateConfig = true;
    }

    if (root["PIRenabled"] != nullptr)
    {
      if (root["PIRenabled"] && !m_PIRenabled)
      {
      //  attachInterrupt(digitalPinToInterrupt(PIRsensorPin), ISR_PIRstateChange, CHANGE);
      //  newPIRsensorState(true, true);
      }
      else if (m_PIRenabled)
      {
        //detachInterrupt(PIRsensorPin);
      }
      m_PIRenabled = root["PIRenabled"];
      m_updateConfig = true;
    }
  }

  /**
   * provide the changeable values
   */
  void addToConfig(JsonObject &root)
  {
    JsonObject top = root.createNestedObject("PIRsensorSwitch");
    top["PIRenabled"] = m_PIRenabled;
    top["PIRoffSec"] = m_switchOffDelay;
    top["LDRadjust"] = m_lightTreshold;
  }

  /**
   * restore the changeable values
   */
  void readFromConfig(JsonObject &root)
  {
    JsonObject top = root["PIRsensorSwitch"];
    m_PIRenabled = (top["PIRenabled"] != nullptr ? top["PIRenabled"] : true);
    m_switchOffDelay = top["PIRoffSec"] | m_switchOffDelay;
    m_lightTreshold = top["LDRadjust"] | m_lightTreshold;
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
