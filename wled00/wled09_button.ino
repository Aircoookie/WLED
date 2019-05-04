/*
 * Physical IO
 */

void shortPressAction()
{
  if (!macroButton)
  {
    toggleOnOff();
    colorUpdated(2);
  } else {
    applyMacro(macroButton);
  }
}


void handleButton()
{
#ifdef BTNPIN
  if (!buttonEnabled) return;
  
  if (digitalRead(BTNPIN) == LOW && !buttonPressedBefore) //pressed
  {
    buttonPressedTime = millis();
    buttonPressedBefore = true;
  }
  else if (digitalRead(BTNPIN) == HIGH && buttonPressedBefore) //released
  {
    long dur = millis() - buttonPressedTime;
    if (dur < 50) {buttonPressedBefore = false; return;} //too short "press", debounce
    bool doublePress = buttonWaitTime;
    buttonWaitTime = 0;

    if (dur > 6000) {initAP();}
    else if (dur > 600) //long press
    {
      if (macroLongPress) {applyMacro(macroLongPress);}
      else _setRandomColor(false,true);
    }
    else { //short press
      if (macroDoublePress)
      {
        if (doublePress) applyMacro(macroDoublePress);
        else buttonWaitTime = millis();
      } else shortPressAction();
    }
    buttonPressedBefore = false;
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
    #if RLYPIN >= 0
     if (!offMode) digitalWrite(RLYPIN, !RLYMDE);
    #endif
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
