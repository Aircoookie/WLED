#include "wled.h"

/*
 * Physical IO
 */

void shortPressAction()
{
  if (!macroButton)
  {
    toggleOnOff();
    colorUpdated(NOTIFIER_CALL_MODE_BUTTON);
  } else {
    applyPreset(macroButton);
  }
}

bool isButtonPressed()
{
  if (btnPin>=0 && digitalRead(btnPin) == LOW) return true;
  #ifdef TOUCHPIN
    if (touchRead(TOUCHPIN) <= TOUCH_THRESHOLD) return true;
  #endif
  return false;
}


void handleButton()
{
  if (btnPin<0 || !buttonEnabled) return;

  if (isButtonPressed()) //pressed
  {
    if (!buttonPressedBefore) buttonPressedTime = millis();
    buttonPressedBefore = true;

    if (millis() - buttonPressedTime > 600) //long press
    {
      if (!buttonLongPressed) 
      {
        if (macroLongPress) {applyPreset(macroLongPress);}
        else _setRandomColor(false,true);

        buttonLongPressed = true;
      }
    }
  }
  else if (!isButtonPressed() && buttonPressedBefore) //released
  {
    long dur = millis() - buttonPressedTime;
    if (dur < 50) {buttonPressedBefore = false; return;} //too short "press", debounce
    bool doublePress = buttonWaitTime;
    buttonWaitTime = 0;

    if (dur > 6000) //long press
    {
      WLED::instance().initAP(true);
    }
    else if (!buttonLongPressed) { //short press
      if (macroDoublePress)
      {
        if (doublePress) applyPreset(macroDoublePress);
        else buttonWaitTime = millis();
      } else shortPressAction();
    }
    buttonPressedBefore = false;
    buttonLongPressed = false;
  }

  if (buttonWaitTime && millis() - buttonWaitTime > 450 && !buttonPressedBefore)
  {
    buttonWaitTime = 0;
    shortPressAction();
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
      //turn off built-in LED if strip is turned off
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
