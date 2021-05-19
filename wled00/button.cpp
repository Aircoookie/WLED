#include "wled.h"

/*
 * Physical IO
 */

#define WLED_DEBOUNCE_THRESHOLD 50 //only consider button input of at least 50ms as valid (debouncing)

void shortPressAction(uint8_t b)
{
  if (!macroButton[b])
  {
    toggleOnOff();
    colorUpdated(NOTIFIER_CALL_MODE_BUTTON);
  } else {
    applyPreset(macroButton[b]);
  }
}

bool isButtonPressed(uint8_t i)
{
  if (btnPin[i]<0) return false;
  //TODO: this may need switch statement (for inverted buttons)
  #ifdef ARDUINO_ARCH_ESP32
  if (buttonType[i]==BTN_TYPE_TOUCH && touchRead(btnPin[i]) <= touchThreshold) return true; else
  #endif
  if (digitalRead(btnPin[i]) == LOW) return true;
  return false;
}


void handleSwitch(uint8_t b)
{
  if (buttonPressedBefore[b] != isButtonPressed(b)) {
    buttonPressedTime[b] = millis();
    buttonPressedBefore[b] = !buttonPressedBefore[b];
  }

  if (buttonLongPressed[b] == buttonPressedBefore[b]) return;
    
  if (millis() - buttonPressedTime[b] > WLED_DEBOUNCE_THRESHOLD) { //fire edge event only after 50ms without change (debounce)
    if (buttonPressedBefore[b]) { //LOW, falling edge, switch closed
      if (macroButton[b]) applyPreset(macroButton[b]);
      else { //turn on
        if (!bri) {toggleOnOff(); colorUpdated(NOTIFIER_CALL_MODE_BUTTON);}
      } 
    } else { //HIGH, rising edge, switch opened
      if (macroLongPress[b]) applyPreset(macroLongPress[b]);
      else { //turn off
        if (bri) {toggleOnOff(); colorUpdated(NOTIFIER_CALL_MODE_BUTTON);}
      } 
    }
    buttonLongPressed[b] = buttonPressedBefore[b]; //save the last "long term" switch state
  }
}


void handleButton()
{
  for (uint8_t b=0; b<WLED_MAX_BUTTONS; b++) {
    if (btnPin[b]<0 || !(buttonType[b] > BTN_TYPE_NONE)) continue;


    if (buttonType[b] == BTN_TYPE_SWITCH) { //button is not momentary, but switch. This is only suitable on pins whose on-boot state does not matter (NOT gpio0)
      handleSwitch(b); continue;
    }

    //momentary button logic
    if (isButtonPressed(b)) //pressed
    {
      if (!buttonPressedBefore[b]) buttonPressedTime[b] = millis();
      buttonPressedBefore[b] = true;

      if (millis() - buttonPressedTime[b] > 600) //long press
      {
        if (!buttonLongPressed[b]) 
        {
          if (macroLongPress[b]) {applyPreset(macroLongPress[b]);}
          else _setRandomColor(false,true);

          buttonLongPressed[b] = true;
        }
      }
    }
    else if (!isButtonPressed(b) && buttonPressedBefore[b]) //released
    {
      long dur = millis() - buttonPressedTime[b];
      if (dur < WLED_DEBOUNCE_THRESHOLD) {buttonPressedBefore[b] = false; continue;} //too short "press", debounce
      bool doublePress = buttonWaitTime[b];
      buttonWaitTime[b] = 0;

      if (dur > 6000 && b==0) //long press on button 0
      {
        WLED::instance().initAP(true);
      }
      else if (!buttonLongPressed[b]) { //short press
        if (macroDoublePress[b])
        {
          if (doublePress) applyPreset(macroDoublePress[b]);
          else buttonWaitTime[b] = millis();
        } else shortPressAction(b);
      }
      buttonPressedBefore[b] = false;
      buttonLongPressed[b] = false;
    }

    if (buttonWaitTime[b] && millis() - buttonWaitTime[b] > 450 && !buttonPressedBefore[b])
    {
      buttonWaitTime[b] = 0;
      shortPressAction(b);
    }
  }
}

void handleIO()
{
  handleButton();
  
  //set relay when LEDs turn on
  if (strip.getBrightness())
  {
    lastOnTime = millis();
    if (offMode)
    {
      if (rlyPin>=0) {
        pinMode(rlyPin, OUTPUT);
        digitalWrite(rlyPin, rlyMde);
      }
      offMode = false;
    }
  } else if (millis() - lastOnTime > 600)
  {
    if (!offMode) {
      #ifdef ESP8266
      // turn off built-in LED if strip is turned off
      // this will break digital bus so will need to be reinitialised on On
      pinMode(LED_BUILTIN, OUTPUT);
      digitalWrite(LED_BUILTIN, HIGH);
      #endif
      if (rlyPin>=0) {
        pinMode(rlyPin, OUTPUT);
        digitalWrite(rlyPin, !rlyMde);
      }
    }
    offMode = true;
  }
}
