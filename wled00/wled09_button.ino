/*
 * Physical IO
 */

void handleButton()
{
  if (buttonEnabled)
  {
    if (digitalRead(buttonPin) == LOW && !buttonPressedBefore)
    {
      buttonPressedBefore = true;
      if (buttonMacro == 255)
      {
        if (bri == 0)
        {
          bri = bri_last;
        } else
        {
          bri_last = bri;
          bri = 0;
        }
        colorUpdated(2);
      } else {
        applyMacro(buttonMacro);
      }
    }
     else if (digitalRead(buttonPin) == HIGH && buttonPressedBefore)
    {
      delay(15); //debounce
      if (digitalRead(buttonPin) == HIGH)
      {
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
