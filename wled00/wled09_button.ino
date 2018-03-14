/*
 * Physical IO
 */

void handleButton()
{
  if (buttonEnabled)
  {
    if (digitalRead(buttonPin) == LOW && !buttonPressedBefore)
    {
      buttonPressedTime = millis();
      buttonPressedBefore = true;
    }
     else if (digitalRead(buttonPin) == HIGH && buttonPressedBefore)
    {
      delay(15); //debounce
      if (digitalRead(buttonPin) == HIGH)
      {
        if (millis() - buttonPressedTime > 7000) {initAP();}
        else if (millis() - buttonPressedTime > 700) 
        {
          if (macroLongPress != 0) {applyMacro(macroLongPress);}
          else _setRandomColor(false);
        }
        else {
          if (macroButton == 0)
          {
            if (bri == 0)
            {
              bri = briLast;
            } else
            {
              briLast = bri;
              bri = 0;
            }
            colorUpdated(2);
          } else {
            applyMacro(macroButton);
          }
        }
        buttonPressedBefore = false;
      }
    }
  }

  //output
  if (auxActive || auxActiveBefore)
  {
    if (!auxActiveBefore)
    {
      auxActiveBefore = true;
      switch (auxTriggeredState)
      {
        case 0: pinMode(auxPin, INPUT); break;
        case 1: pinMode(auxPin, OUTPUT); digitalWrite(auxPin, HIGH); break;
        case 2: pinMode(auxPin, OUTPUT); digitalWrite(auxPin, LOW); break;
      }
      auxStartTime = millis();
    }
    if ((millis() - auxStartTime > auxTime*1000 && auxTime != 255) || !auxActive)
    {
      auxActive = false;
      auxActiveBefore = false;
      switch (auxDefaultState)
      {
        case 0: pinMode(auxPin, INPUT); break;
        case 1: pinMode(auxPin, OUTPUT); digitalWrite(auxPin, HIGH); break;
        case 2: pinMode(auxPin, OUTPUT); digitalWrite(auxPin, LOW); break;
      }
    }
  }
}
