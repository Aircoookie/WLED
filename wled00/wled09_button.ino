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
      if (bri == 0)
      {
        bri = bri_last;
      } else
      {
        bri_last = bri;
        bri = 0;
      }
      colorUpdated(2);
    }
     else if (digitalRead(buttonPin) == HIGH && buttonPressedBefore)
    {
      delay(15);
      if (digitalRead(buttonPin) == HIGH)
      {
        buttonPressedBefore = false;
      }
    }
  }
}
