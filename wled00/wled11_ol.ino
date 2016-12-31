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
        switch (nixieClockI)
        {
          case 0: 
            strip.setFade(99);
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
              Serial.print(overlayArr[i]);
              Serial.print(" ");
            }
            Serial.println(" ");
            if (overlayBackgroundBlack) {
              strip.setRange(overlayMin, overlayMax, 0);
              strip.setIndividual(overlayArr[0], ((uint32_t)col_t[0] << 16) | ((uint32_t)col_t[1] << 8) | col_t[2]);
              strip.unlock(overlayArr[0]);
            } else
            {
              strip.unlockAll();
              strip.setIndividual(overlayArr[0], overlayColor);
            }
            overlayRefreshMs = 10 + 10*(255 - overlaySpeed);
            nixieClockI++;
            break;
          case 1:
            if (overlayBackgroundBlack) {
              if (overlayArr[1] != -1)
              {
                strip.setRange(overlayMin, overlayMax, 0);
                strip.setIndividual(overlayArr[1], ((uint32_t)col_t[0] << 16) | ((uint32_t)col_t[1] << 8) | col_t[2]);
                strip.unlock(overlayArr[1]);
              }
            } else
            { 
              if (overlayArr[1] != -1)
              {
                strip.unlockAll();
                strip.setIndividual(overlayArr[1], overlayColor);
              }
            }
            if (overlayArr[1] == -1)
            {
              overlayRefreshMs = 10 + 10*(255 - overlaySpeed);
            } else
            {
              overlayRefreshMs = 20 + 20*(255 - overlaySpeed);
            }
            nixieClockI++;
            break;
          case 2:
            if (overlayBackgroundBlack) {
                strip.setRange(overlayMin, overlayMax, 0);
                strip.setIndividual(overlayArr[2], ((uint32_t)col_t[0] << 16) | ((uint32_t)col_t[1] << 8) | col_t[2]);
                strip.unlock(overlayArr[2]);
            } else
            { 
                strip.unlockAll();
                strip.setIndividual(overlayArr[2], overlayColor);
            }
            overlayRefreshMs = 10 + 10*(255 - overlaySpeed);
            nixieClockI++;
            break;
          case 3:
            if (overlayBackgroundBlack) {
                strip.setRange(overlayMin, overlayMax, 0);
                strip.setIndividual(overlayArr[3], ((uint32_t)col_t[0] << 16) | ((uint32_t)col_t[1] << 8) | col_t[2]);
                strip.unlock(overlayArr[3]);
            } else
            { 
                strip.unlockAll();
                strip.setIndividual(overlayArr[3], overlayColor);
            }
            overlayRefreshMs = 20 + 20*(255 - overlaySpeed);
            nixieClockI++;
            break;
          case 4:
            if (overlayBackgroundBlack) {
              if (overlayArr[4] != -1)
              {
                strip.setRange(overlayMin, overlayMax, 0);
                strip.setIndividual(overlayArr[4], ((uint32_t)col_t[0] << 16) | ((uint32_t)col_t[1] << 8) | col_t[2]);
                strip.unlock(overlayArr[4]);
              }
            } else
            { 
              if (overlayArr[4] != -1)
              {
                strip.unlockAll();
                strip.setIndividual(overlayArr[4], overlayColor);
              }
            }
            if (overlayArr[4] == -1)
            {
              overlayRefreshMs = 0;
            } else
            {
              overlayRefreshMs = 10 + 10*(255 - overlaySpeed);
            }
            nixieClockI++;
            break;
          case 5:
            if (overlayBackgroundBlack) {
              if (overlayArr[5] != -1)
              {
                strip.setRange(overlayMin, overlayMax, 0);
                strip.setIndividual(overlayArr[5], ((uint32_t)col_t[0] << 16) | ((uint32_t)col_t[1] << 8) | col_t[2]);
                strip.unlock(overlayArr[5]);
              }
            } else
            { 
              if (overlayArr[5] != -1)
              {
                strip.unlockAll();
                strip.setIndividual(overlayArr[5], overlayColor);
              }
            }
            overlayRefreshMs = 30 + 30*(255 - overlaySpeed);
            nixieClockI = 0;
            break;
          }
        }
    }
  }
}

