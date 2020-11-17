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
    applyMacro(macroButton);
  }
}

bool isButtonPressed()
{
  #ifdef BTNPIN
    if (digitalRead(BTNPIN) == LOW) return true;
  #endif
  #ifdef TOUCHPIN
    if (touchRead(TOUCHPIN) <= TOUCH_THRESHOLD) return true;
  #endif
  return false;
}


void handleButton()
{
#if defined(BTNPIN) || defined(TOUCHPIN)
  if (!buttonEnabled) return;

  if (isButtonPressed()) //pressed
  {
    if (!buttonPressedBefore) buttonPressedTime = millis();
    buttonPressedBefore = true;

    if (millis() - buttonPressedTime > 600) //long press
    {
      if (!buttonLongPressed) 
      {
        if (macroLongPress) {applyMacro(macroLongPress);}
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
        if (doublePress) applyMacro(macroDoublePress);
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
#endif
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
      #if RLYPIN >= 0
       digitalWrite(RLYPIN, RLYMDE);
      #endif
      offMode = false;
    }
  } else if (millis() - lastOnTime > 600)
  {
     if (!offMode) {
      #if LEDPIN == LED_BUILTIN
        pinMode(LED_BUILTIN, OUTPUT);
        digitalWrite(LED_BUILTIN, HIGH);
      #endif
      #if RLYPIN >= 0
       digitalWrite(RLYPIN, !RLYMDE);
      #endif
     }
    offMode = true;
  }

  #if AUXPIN >= 0
  //output
  if (auxActive || auxActiveBefore)
  {
    if (!auxActiveBefore)
    {
      auxActiveBefore = true;
      switch (auxTriggeredState)
      {
        case 0: pinMode(AUXPIN, INPUT); break;
        case 1: pinMode(AUXPIN, OUTPUT); digitalWrite(AUXPIN, HIGH); break;
        case 2: pinMode(AUXPIN, OUTPUT); digitalWrite(AUXPIN, LOW); break;
      }
      auxStartTime = millis();
    }
    if ((millis() - auxStartTime > auxTime*1000 && auxTime != 255) || !auxActive)
    {
      auxActive = false;
      auxActiveBefore = false;
      switch (auxDefaultState)
      {
        case 0: pinMode(AUXPIN, INPUT); break;
        case 1: pinMode(AUXPIN, OUTPUT); digitalWrite(AUXPIN, HIGH); break;
        case 2: pinMode(AUXPIN, OUTPUT); digitalWrite(AUXPIN, LOW); break;
      }
    }
  }
  #endif
}
