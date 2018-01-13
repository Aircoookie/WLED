/*
 * The Overlay function is over a year old, largely untested and not configurable during runtime. Consider it as deprecated for now, it might get either removed/simplified/reworked.
 */
#ifdef USEOVERLAYS
void _nixieDisplay(int num[], int dur[], int pausedur[], int cnt)
{
  strip.setRange(overlayMin, overlayMax, 0);
  if (num[nixieClockI] >= 0 && !nixiePause)
  {
    strip.setIndividual(num[nixieClockI],((uint32_t)white << 24)| ((uint32_t)col_t[0] << 16) | ((uint32_t)col_t[1] << 8) | col_t[2]);
    strip.unlock(num[nixieClockI]);
  }
  if (!nixiePause)
  {
    overlayRefreshMs = dur[nixieClockI];
  } else
  {
    overlayRefreshMs = pausedur[nixieClockI];
  }
  if (pausedur[nixieClockI] > 0 && !nixiePause)
  {
    nixiePause = true;
  } else {
    if (nixieClockI < cnt -1)
    {
      nixieClockI++;
    } else
    {
      nixieClockI = -1;
    }
    nixiePause = false;
  }
}

void _nixieNumber(int number, int dur) 
{
  if (nixieClockI < 0)
  {
    DEBUG_PRINT(number);
    int digitCnt = -1;
    int digits[4];
    digits[3] = number/1000;
    digits[2] = (number/100)%10;
    digits[1] = (number/10)%10;
    digits[0] = number%10;
    if (number > 999) //four digits
    {
      digitCnt = 4;
    } else if (number > 99) //three digits
    {
      digitCnt = 3;
    } else if (number > 9) //two digits
    {
      digitCnt = 2;
    } else { //single digit
      digitCnt = 1;
    }
    DEBUG_PRINT(" ");
    for (int i = 0; i < digitCnt; i++)
    {
      DEBUG_PRINT(digits[i]);
      overlayArr[digitCnt-1-i] = digits[i];
      overlayDur[digitCnt-1-i] = ((dur/4)*3)/digitCnt;
      overlayPauseDur[digitCnt-1-i] = 0;
    }
    DEBUG_PRINTLN(" ");
    for (int i = 1; i < digitCnt; i++)
    {
      if (overlayArr[i] == overlayArr[i-1])
      {
        overlayPauseDur[i-1] = dur/12;
        overlayDur[i-1] = overlayDur[i-1]-dur/12;
      }
    }
    for (int i = digitCnt; i < 6; i++)
    {
      overlayArr[i] = -1;
      overlayDur[i] = 0;
      overlayPauseDur[i] = 0;
    }
    overlayPauseDur[5] = dur/4;
    for (int i = 0; i < 6; i++)
    {
      if (overlayArr[i] != -1)
      {
        overlayArr[i] = overlayArr[i] + overlayMin;
        if (overlayReverse)
          overlayArr[i] = overlayMax - overlayArr[i];
      }
    }
    for (int i = 0; i <6; i++)
    {
      DEBUG_PRINT(overlayArr[i]);
      DEBUG_PRINT(" ");
      DEBUG_PRINT(overlayDur[i]);
      DEBUG_PRINT(" ");
      DEBUG_PRINT(overlayPauseDur[i]);
      DEBUG_PRINT(" ");
    }
    DEBUG_PRINTLN(" ");
    nixieClockI = 0;
  } else {
    _nixieDisplay(overlayArr, overlayDur, overlayPauseDur, 6);
  }
}

void handleOverlays()
{
  //properties: range, (color)
  //0 no overlay
  //1 solid color (NI)
  //2 analog clock
  //3 digital nixie-style clock one digit
  //4 just static hour (NI)
  //5 analog countdown
  //6 digital one digit countdown
  if (millis() - overlayRefreshedTime > overlayRefreshMs)
  {
    overlayRefreshedTime = millis();
    switch (overlayCurrent)
    {
      case 2: _overlayAnalogClock(); break;//2 analog clock
      case 3: _overlayNixieClock(); break;//nixie 1-digit
      case 5: _overlayAnalogCountdown(); break;//a.countdown 
      case 6: _overlayNixieCountdown(); break;//d.
    }
  }
}

void _overlayAnalogClock()
{
  int overlaySize = overlayMax - overlayMin +1;
  strip.unlockAll();
  if (overlayDimBg)
  {
    uint32_t ct = (white>>1)*16777216 + (col[0]>>1)*65536 + (col[1]>>1)*256 + (col[2]>>1);
    if (useGammaCorrectionRGB) ct = (gamma8[white]>>1)*16777216 + (gamma8[col[0]]>>1)*65536 + (gamma8[col[1]]>>1)*256 + (gamma8[col[2]]>>1);
    strip.setRange(overlayMin, overlayMax, ct);
  }
  local = TZ.toLocal(now(), &tcr);
  double hourP = ((double)(hour(local)%12))/12;
  double minuteP = ((double)minute(local))/60;
  hourP = hourP + minuteP/12;
  double secondP = ((double)second(local))/60;
  int hourPixel = floor(analogClock12pixel + overlaySize*hourP);
  if (hourPixel > overlayMax) hourPixel = overlayMin -1 + hourPixel - overlayMax;
  int minutePixel = floor(analogClock12pixel + overlaySize*minuteP);
  if (minutePixel > overlayMax) minutePixel = overlayMin -1 + minutePixel - overlayMax; 
  int secondPixel = floor(analogClock12pixel + overlaySize*secondP);
  if (secondPixel > overlayMax) secondPixel = overlayMin -1 + secondPixel - overlayMax;
  if (analogClock5MinuteMarks)
  {
    int pix;
    for (int i = 0; i <= 12; i++)
    {
      pix = overlayMin + analogClock12pixel + (overlaySize/12)*i;
      if (pix > overlayMax) pix = pix - overlayMax;
      strip.setIndividual(pix, 0xAAAAAA);
    }
  }
  if (analogClockSecondsTrail)
  {
    strip.setRange(analogClock12pixel, secondPixel, 0xFF0000);
  } else
  {
    strip.setIndividual(secondPixel, 0xFF0000);
  }
  strip.setIndividual(minutePixel, 0x00FF00);
  strip.setIndividual(hourPixel, 0x0000FF);
  overlayRefreshMs = 998;
}

void _overlayNixieClock()
{
  if (nixieClockI < 0)
  {
      local = TZ.toLocal(now(), &tcr);
      overlayArr[0] = hour(local);
      if (nixieClock12HourFormat && overlayArr[0] > 12)
      {
        overlayArr[0] = overlayArr[0]%12;
      }
      overlayArr[1] = -1;
      if (overlayArr[0] > 9)
      {
        overlayArr[1] = overlayArr[0]%10;
        overlayArr[0] = overlayArr[0]/10;
      }
      overlayArr[2] = minute(local);
      overlayArr[3] = overlayArr[2]%10;
      overlayArr[2] = overlayArr[2]/10;
      overlayArr[4] = -1;
      overlayArr[5] = -1;
      if (nixieClockDisplaySeconds)
      {
        overlayArr[4] = second(local);
        overlayArr[5] = overlayArr[4]%10;
        overlayArr[4] = overlayArr[4]/10;
      }
      for (int i = 0; i < 6; i++)
      {
        if (overlayArr[i] != -1)
        {
          overlayArr[i] = overlayArr[i] + overlayMin;
          if (overlayReverse)
            overlayArr[i] = overlayMax - overlayArr[i];
        }
      }
      overlayDur[0] = 12 + 12*(255 - overlaySpeed);
      if (overlayArr[1] == overlayArr[0])
      {
        overlayPauseDur[0] = 3 + 3*(255 - overlaySpeed);
      } else
      {
        overlayPauseDur[0] = 0;
      }
      if (overlayArr[1] == -1)
      {
        overlayDur[1] = 0;
      } else
      {
        overlayDur[1] = 12 + 12*(255 - overlaySpeed);
      }
      overlayPauseDur[1] = 9 + 9*(255 - overlaySpeed);

      overlayDur[2] = 12 + 12*(255 - overlaySpeed);
      if (overlayArr[2] == overlayArr[3])
      {
        overlayPauseDur[2] = 3 + 3*(255 - overlaySpeed);
      } else
      {
        overlayPauseDur[2] = 0;
      }
      overlayDur[3] = 12 + 12*(255 - overlaySpeed);
      overlayPauseDur[3] = 9 + 9*(255 - overlaySpeed);
      
      if (overlayArr[4] == -1)
      {
        overlayDur[4] = 0;
        overlayPauseDur[4] = 0;
        overlayDur[5] = 0;
      } else
      {
        overlayDur[4] = 12 + 12*(255 - overlaySpeed);
        if (overlayArr[5] == overlayArr[4])
        {
          overlayPauseDur[4] = 3 + 3*(255 - overlaySpeed);
        } else
        {
          overlayPauseDur[4] = 0;
        }
        overlayDur[5] = 12 + 12*(255 - overlaySpeed);
      }
      overlayPauseDur[5] = 22 + 22*(255 - overlaySpeed);
      
      nixieClockI = 0;   
  } else
  {
    _nixieDisplay(overlayArr, overlayDur, overlayPauseDur, 6);
  }
}

void _overlayAnalogCountdown()
{
  strip.unlockAll();
  if (now() >= countdownTime)
  {
    //what to do if countdown finished
  } else
  {
    long diff = countdownTime - now();
    double pval = 60;
    if (diff > 31557600L) //display in years if more than 365 days
    {
      pval = 315576000L; //10 years
    } else if (diff > 2592000L) //display in months if more than a month
    {
      pval = 31557600L; //1 year
    } else if (diff > 604800) //display in weeks if more than a week
    {
      pval = 2592000L; //1 month
    } else if (diff > 86400) //display in days if more than 24 hours
    {
      pval = 604800; //1 week
    } else if (diff > 3600) //display in hours if more than 60 minutes
    {
      pval = 86400; //1 day
    } else if (diff > 60) //display in minutes if more than 60 seconds
    {
      pval = 3600; //1 hour
    }
    int overlaySize = overlayMax - overlayMin +1;
    double perc = (pval-(double)diff)/pval;
    if (perc > 1.0) perc = 1.0;
    uint8_t pixelCnt = perc*overlaySize;
    if (analogClock12pixel + pixelCnt > overlayMax)
    {
      strip.setRange(analogClock12pixel, overlayMax, ((uint32_t)white_sec << 24)| ((uint32_t)col_sec[0] << 16) | ((uint32_t)col_sec[1] << 8) | col_sec[2]);
      strip.setRange(overlayMin, overlayMin +pixelCnt -(1+ overlayMax -analogClock12pixel), ((uint32_t)white_sec << 24)| ((uint32_t)col_sec[0] << 16) | ((uint32_t)col_sec[1] << 8) | col_sec[2]);
    } else
    {
      strip.setRange(analogClock12pixel, analogClock12pixel + pixelCnt, ((uint32_t)white_sec << 24)| ((uint32_t)col_sec[0] << 16) | ((uint32_t)col_sec[1] << 8) | col_sec[2]);
    }
  }
  overlayRefreshMs = 998;
}

void _overlayNixieCountdown()
{
  if (now() >= countdownTime)
  {
    if (effectCurrent != 8){
      effectCurrent = 8;
      strip.setMode(8);
      strip.setSpeed(255);
    }
    
    _nixieNumber(2018, 2018);
  } else
  {
    long diff = countdownTime - now();
    if (diff > 86313600L) //display in years if more than 999 days
    {
      diff = diff/31557600L;
    } else if (diff > 3596400) //display in days if more than 999 hours
    {
      diff = diff/86400;
    } else if (diff > 59940) //display in hours if more than 999 minutes
    {
      diff = diff/1440;
    } else if (diff > 999) //display in minutes if more than 999 seconds
    {
      diff = diff/60;
    }
    _nixieNumber(diff, 800);
  }
}
#endif
