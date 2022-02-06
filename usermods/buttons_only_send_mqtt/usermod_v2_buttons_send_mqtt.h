#pragma once

#include "wled.h"

/*
 * Usermods allow you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * 
 * This is an example for a v2 usermod.
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

//class name. Use something descriptive and leave the ": public Usermod" part :)
class ButtonsSendMqtt: public Usermod
{
private:
  static constexpr const char* _mqtt_topic_button = "%s/button/%d";
  static const long WLED_DEBOUNCE_THRESHOLD = 50;    //only consider button input of at least 50ms as valid (debouncing)
  static const long WLED_LONG_PRESS = 600;           //long press if button is released after held for at least 600ms
  static const long WLED_DOUBLE_PRESS = 350;         //double press if another press within 350ms after a short press
  static const long WLED_LONG_REPEATED_ACTION = 300; //how often a repeated action (e.g. dimming) is fired on long press on button IDs >0
  static const long WLED_LONG_AP = 10000;            //how long the button needs to be held to activate WLED-AP
public:
  //Functions called by WLED

  /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
  uint16_t getId()
  {
    return USERMOD_ID_BUTTONS_SEND_MQTT;
  }

  bool handleButton(uint8_t b)
  {
    if ((buttonType[b] == BTN_TYPE_ANALOG || buttonType[b] == BTN_TYPE_ANALOG_INVERTED))
      return false;

    //button is not momentary, but switch. This is only suitable on pins whose on-boot state does not matter (NOT gpio0)
    if (buttonType[b] == BTN_TYPE_SWITCH || buttonType[b] == BTN_TYPE_PIR_SENSOR)
    {
      handleSwitch(b);
      return true;
    }

    //momentary button logic
    if (isButtonPressed(b))
    { //pressed

      if (!buttonPressedBefore[b])
        buttonPressedTime[b] = millis();
      buttonPressedBefore[b] = true;

      if (millis() - buttonPressedTime[b] > WLED_LONG_PRESS)
      { //long press
        if (!buttonLongPressed[b])
          longPressAction(b);
        else if (b)
        { //repeatable action (~3 times per s) on button > 0
          longPressAction(b);
          buttonPressedTime[b] = millis() - WLED_LONG_REPEATED_ACTION; //300ms
        }
        buttonLongPressed[b] = true;
      }
    }
    else if (!isButtonPressed(b) && buttonPressedBefore[b])
    { //released

      long dur = millis() - buttonPressedTime[b];
      if (dur < WLED_DEBOUNCE_THRESHOLD)
      {
        buttonPressedBefore[b] = false;
        return true;
      }                                     //too short "press", debounce
      bool doublePress = buttonWaitTime[b]; //did we have a short press before?
      buttonWaitTime[b] = 0;

      if (b == 0 && dur > WLED_LONG_AP)
      { //long press on button 0 (when released)
        WLED::instance().initAP(true);
      }
      else if (!buttonLongPressed[b])
      { //short press
        if (b == 0 && !macroDoublePress[b])
        { //don't wait for double press on button 0 if no double press macro set
          shortPressAction(b);
        }
        else
        { //double press if less than 350 ms between current press and previous short press release (buttonWaitTime!=0)
          if (doublePress)
          {
            doublePressAction(b);
          }
          else
          {
            buttonWaitTime[b] = millis();
          }
        }
      }
      buttonPressedBefore[b] = false;
      buttonLongPressed[b] = false;
    }

    //if 350ms elapsed since last short press release it is a short press
    if (buttonWaitTime[b] && millis() - buttonWaitTime[b] > WLED_DOUBLE_PRESS && !buttonPressedBefore[b])
    {
      buttonWaitTime[b] = 0;
      shortPressAction(b);
    }
    return true;
  }

private:
  void handleSwitch(uint8_t b)
  {
    // isButtonPressed() handles inverted/noninverted logic
    if (buttonPressedBefore[b] != isButtonPressed(b))
    {
      buttonPressedTime[b] = millis();
      buttonPressedBefore[b] = !buttonPressedBefore[b];
    }

    if (buttonLongPressed[b] == buttonPressedBefore[b])
      return;

    if (millis() - buttonPressedTime[b] > WLED_DEBOUNCE_THRESHOLD)
    { //fire edge event only after 50ms without change (debounce)
      // publish MQTT message
      if (buttonPublishMqtt && WLED_MQTT_CONNECTED)
      {
        char subuf[64];
        if (buttonType[b] == BTN_TYPE_PIR_SENSOR)
          sprintf_P(subuf, PSTR("%s/motion/%d"), mqttDeviceTopic, (int)b);
        else
          sprintf_P(subuf, _mqtt_topic_button, mqttDeviceTopic, (int)b);
        mqtt->publish(subuf, 0, false, !buttonPressedBefore[b] ? "off" : "on");
      }

      buttonLongPressed[b] = buttonPressedBefore[b]; //save the last "long term" switch state
    }
  }
  void shortPressAction(uint8_t b)
  {
    // publish MQTT message
    if (buttonPublishMqtt && WLED_MQTT_CONNECTED)
    {
      char subuf[64];
      sprintf_P(subuf, _mqtt_topic_button, mqttDeviceTopic, (int)b);
      mqtt->publish(subuf, 0, false, "short");
    }
  }

  void longPressAction(uint8_t b)
  {
    // publish MQTT message
    if (buttonPublishMqtt && WLED_MQTT_CONNECTED)
    {
      char subuf[64];
      sprintf_P(subuf, _mqtt_topic_button, mqttDeviceTopic, (int)b);
      mqtt->publish(subuf, 0, false, "long");
    }
  }

  void doublePressAction(uint8_t b)
  {
    // publish MQTT message
    if (buttonPublishMqtt && WLED_MQTT_CONNECTED)
    {
      char subuf[64];
      sprintf_P(subuf, _mqtt_topic_button, mqttDeviceTopic, (int)b);
      mqtt->publish(subuf, 0, false, "double");
    }
  }
};