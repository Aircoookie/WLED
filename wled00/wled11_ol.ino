void nixieDisplay(int num[], int dur[], int pausedur[], int cnt)
{
  strip.setRange(overlayMin, overlayMax, 0);
  if (num[nixieClockI] >= 0 && !nixiePause)
  {
    strip.setIndividual(num[nixieClockI], ((uint32_t)col_t[0] << 16) | ((uint32_t)col_t[1] << 8) | col_t[2]);
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

void nixieNumber(int number, int dur) 
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
    nixieDisplay(overlayArr, overlayDur, overlayPauseDur, 6);
  }
}

void handleOverlays()
{
  //properties: range, (color)
  //0 no overlay
  //1 solid color
  //2 analog clock
  //3 digital nixie-style clock one digit
  //4 just static hour
  //5 analog countdown
  //6 digital one digit countdown
  if (millis() - overlayRefreshedTime > overlayRefreshMs)
  {
    overlayRefreshedTime = millis();
    switch (overlayCurrent)
    {
      case 2: {//2 analog clock
        int overlaySize = overlayMax - overlayMin +1;
        strip.unlockAll();
        local = TZ.toLocal(now(), &tcr);
        double hourP = ((double)(hour(local)%12))/12;
        double minuteP = ((double)minute(local))/60;
        hourP = hourP + minuteP/12;
        double secondP = ((double)second(local))/60;
        int hourPixel = floor(overlayMin + analogClock12pixel + overlaySize*hourP);
        if (hourPixel > overlayMax) hourPixel = hourPixel - overlayMax;
        int minutePixel = floor(overlayMin + analogClock12pixel + overlaySize*minuteP);
        if (minutePixel > overlayMax) minutePixel = minutePixel - overlayMax; 
        int secondPixel = floor(overlayMin + analogClock12pixel + overlaySize*secondP);
        if (secondPixel > overlayMax) secondPixel = secondPixel - overlayMax;
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
          strip.setRange(analogClock12pixel, secondPixel, 0x0000FF);
        } else
        {
          strip.setIndividual(secondPixel, 0x0000FF);
        }
        strip.setIndividual(minutePixel, 0x00FF00);
        strip.setIndividual(hourPixel, 0xFF0000);
        overlayRefreshMs = 998; break;
      }
      case 3: {
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
          nixieDisplay(overlayArr, overlayDur, overlayPauseDur, 6);
        }
      } break;
    case 5: {//countdown
        if (now() >= countdownTime)
        {
          if (effectCurrent != 8){
            effectCurrent = 8;
            strip.setMode(8);
            strip.setSpeed(255);
          }
          
          nixieNumber(2017, 2017);
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
          nixieNumber(diff, 800);
        }
      }
    }
  }
}

