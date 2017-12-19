#ifdef CRONIXIE
uint8_t getSameCodeLength(char code, int index, char const digits[])
{
  uint8_t counter = 0;
  
  for (int i = index+1; i < 6; i++)
  {
    if (digits[i] == code)
    {
      counter++;
    } else {
      return counter;
    }
  }
  return counter;
}

void setCronixie(char const digits[])
{
  /*
   * digit purpose index
   * 0-9 | 0-9 (incl. random)
   * 10 | blank
   * 11 | blank, bg off
   * 12 | test upw.
   * 13 | test dnw.
   * 14 | binary AM/PM
   * 15 | BB upper +50 for no trailing 0
   * 16 | BBB
   * 17 | BBBB
   * 18 | BBBBB
   * 19 | BBBBBB
   * 20 | H
   * 21 | HH
   * 22 | HHH
   * 23 | HHHH
   * 24 | M
   * 25 | MM
   * 26 | MMM
   * 27 | MMMM
   * 28 | MMMMM
   * 29 | MMMMMM
   * 30 | S
   * 31 | SS
   * 32 | SSS
   * 33 | SSSS
   * 34 | SSSSS
   * 35 | SSSSSS
   * 36 | Y
   * 37 | YY
   * 38 | YYYY
   * 39 | I
   * 40 | II
   * 41 | W
   * 42 | WW
   * 43 | D
   * 44 | DD
   * 45 | DDD
   * 46 | V
   * 47 | VV
   * 48 | VVV
   * 49 | VVVV
   * 50 | VVVVV
   * 51 | VVVVVV
   * 52 | v
   * 53 | vv
   * 54 | vvv
   * 55 | vvvv
   * 56 | vvvvv
   * 57 | vvvvvv
   */

  //H HourLower | HH - Hour 24. | AH - Hour 12. | HHH Hour of Month | HHHH Hour of Year
  //M MinuteUpper | MM Minute of Hour | MMM Minute of 12h | MMMM Minute of Day | MMMMM Minute of Month | MMMMMM Minute of Year
  //S SecondUpper | SS Second of Minute | SSS Second of 10 Minute | SSSS Second of Hour | SSSSS Second of Day | SSSSSS Second of Week
  //B AM/PM | BB 0-6/6-12/12-18/18-24 | BBB 0-3... | BBBB 0-1.5... | BBBBB 0-1 | BBBBBB 0-0.5
  
  //Y YearLower | YY - Year LU | YYYY - Std.
  //I MonthLower | II - Month of Year 
  //W Week of Month | WW Week of Year
  //D Day of Week | DD Day Of Month | DDD Day Of Year

  DEBUG_PRINT("cset ");
  DEBUG_PRINTLN(digits);

  cronixieRefreshMs = 1997; //Only refresh every 2secs if no seconds are displayed
  
  for (int i = 0; i < 6; i++)
  {
    dP[i] = 10;
    switch (digits[i])
    {
      case '_': dP[i] = 10;
      case '-': dP[i] = 11;
      case 'r': dP[i] = random(1,7); break; //random btw. 1-6
      case 'R': dP[i] = random(0,10); break; //random btw. 0-9
      case 't': break; //Test upw.
      case 'T': break; //Test dnw.
      case 'b': dP[i] = 14 + getSameCodeLength('b',i,digits); i = i+dP[i]-14; break; 
      case 'B': dP[i] = 14 + getSameCodeLength('B',i,digits); i = i+dP[i]-14; break;
      case 'h': dP[i] = 70 + getSameCodeLength('h',i,digits); i = i+dP[i]-70; break;
      case 'H': dP[i] = 20 + getSameCodeLength('H',i,digits); i = i+dP[i]-20; break;
      case 'A': dP[i] = 108; i++; break;
      case 'a': dP[i] = 58; i++; break;
      case 'm': dP[i] = 74 + getSameCodeLength('m',i,digits); i = i+dP[i]-74; break;
      case 'M': dP[i] = 24 + getSameCodeLength('M',i,digits); i = i+dP[i]-24; break;
      case 's': dP[i] = 80 + getSameCodeLength('s',i,digits); i = i+dP[i]-80; cronixieRefreshMs = 497; break; //refresh more often bc. of secs
      case 'S': dP[i] = 30 + getSameCodeLength('S',i,digits); i = i+dP[i]-30; cronixieRefreshMs = 497; break;
      case 'Y': break;
      case 'y': break;
      case 'I': break; //Month. Don't ask me why month and minute both start with M.
      case 'i': break;
      case 'W': break;
      case 'w': break;
      case 'D': break;
      case 'd': break;
      case '0': dP[i] = 0; break;
      case '1': dP[i] = 1; break;
      case '2': dP[i] = 2; break;
      case '3': dP[i] = 3; break;
      case '4': dP[i] = 4; break;
      case '5': dP[i] = 5; break;
      case '6': dP[i] = 6; break;
      case '7': dP[i] = 7; break;
      case '8': dP[i] = 8; break;
      case '9': dP[i] = 9; break;
      case 'V': break; //user var0
      case 'v': break; //user var1
    }
  }
  DEBUG_PRINT("result ");
  for (int i = 0; i < 5; i++)
  {
    DEBUG_PRINT((int)dP[i]);
    DEBUG_PRINT(" ");
  }
  DEBUG_PRINTLN((int)dP[5]);

  cronixieRefreshedTime = 0; //refresh immediately
}

void handleCronixie()
{
  if (millis() - cronixieRefreshedTime > cronixieRefreshMs)
  {
    cronixieRefreshedTime = millis();
    local = TZ.toLocal(now(), &tcr);
    uint8_t h = hour(local);
    uint8_t h0 = h;
    uint8_t m = minute(local);
    uint8_t s = second(local);
    uint8_t d = day(local);
    uint8_t mi = month(local);
    uint16_t y = year(local);
    if (cronixieUseAMPM)
    {
      if (h>12) h-=12;
      else if (h==0) h+=12;
    }
    byte _digitOut[]{10,10,10,10,10,10};
    for (int i = 0; i < 6; i++)
    {
      if (dP[i] < 12) _digitOut[i] = dP[i];
      else {
        if (dP[i] < 65)
        {
          switch(dP[i])
          {
            case 21: _digitOut[i] = h/10; _digitOut[i+1] = h- _digitOut[i]*10; i++; break; //HH
            case 25: _digitOut[i] = m/10; _digitOut[i+1] = m- _digitOut[i]*10; i++; break; //MM
            case 31: _digitOut[i] = s/10; _digitOut[i+1] = s- _digitOut[i]*10; i++; break; //SS

            case 20: _digitOut[i] = h- (h/10)*10; break; //H
            case 24: _digitOut[i] = m/10; break; //M
            case 30: _digitOut[i] = s/10; break; //S
            case 16: _digitOut[i+2] = ((h0/3)&1)?1:0; i++; //BBB (BBBB NI)
            case 15: _digitOut[i+1] = (h0>17 || (h0>5 && h0<12))?1:0; i++; //BB
            case 14: _digitOut[i] = (h0>11)?1:0; break; //B
          }
        } else
        {
          switch(dP[i])
          {
            case 71: _digitOut[i] = h/10; _digitOut[i+1] = h- _digitOut[i]*10; if(_digitOut[i] == 0) _digitOut[i]=10; i++; break; //hh
            case 75: _digitOut[i] = m/10; _digitOut[i+1] = m- _digitOut[i]*10; if(_digitOut[i] == 0) _digitOut[i]=10; i++; break; //mm
            case 81: _digitOut[i] = s/10; _digitOut[i+1] = s- _digitOut[i]*10; if(_digitOut[i] == 0) _digitOut[i]=10; i++; break; //ss
          }
        }
      }
    }
    DEBUG_PRINT("out ");
    for (int i = 0; i < 5; i++)
    {
      DEBUG_PRINT((int)_digitOut[i]);
      DEBUG_PRINT(" ");
    }
    DEBUG_PRINTLN((int)_digitOut[5]);
    strip.setCronixieDigits(_digitOut);
    //strip.trigger(); //this has a drawback, no effects slower than RefreshMs. advantage: Quick update, not dependant on effect time
  }
}
#endif
