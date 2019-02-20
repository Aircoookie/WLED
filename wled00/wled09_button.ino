/*
 * Physical IO
 */

void handleButton()
{
  if (buttonEnabled && millis() - buttonReleasedTime > 20) //debounce
  {
    if (digitalRead(BTNPIN) == LOW && !buttonPressedBefore) //pressed
    {
      buttonPressedTime = millis();
      buttonPressedBefore = true;
    }
     else if (digitalRead(BTNPIN) == HIGH && buttonPressedBefore) //released
    {
      if (buttonReleasedTime == 0) {
        buttonReleasedTime = millis();
      } else {
        if (digitalRead(BTNPIN) == HIGH)
        {
          if (buttonReleasedTime - buttonPressedTime > 7000) {initAP();}
          else if (buttonReleasedTime  - buttonPressedTime > 700) 
          {
            if (macroLongPress != 0) {applyMacro(macroLongPress);}
            else _setRandomColor(false,true);
          }
          else {
            if (macroButton == 0)
            {
              toggleOnOff();
              colorUpdated(2);
            } else {
              applyMacro(macroButton);
            }
          }
          buttonPressedBefore = false;
        }
        buttonReleasedTime = 0;
      }
    }
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
