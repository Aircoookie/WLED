#include "wled.h"

/*
 * Used to draw clock overlays over the strip
 */
 
void initCronixie()
{
  if (overlayCurrent == 3 && dP[0] == 255) //if dP[0] is 255, cronixie is not yet init'ed
  {
    setCronixie();
    strip.getSegment(0).grouping = 10; //10 LEDs per digit
  } else if (dP[0] < 255 && overlayCurrent != 3)
  {
    strip.getSegment(0).grouping = 1;
    dP[0] = 255; 
  }
}


//handleOverlays is essentially the equivalent of usermods.loop
void handleOverlays()
{
  initCronixie();
  if (overlayCurrent == 3) {
    _overlayCronixie();//Diamex cronixie clock kit
    strip.trigger();
  }
}

/**
 * Write the encoded number into the led strip at the specified position.
 * In this case we rely on the regular effect funtion to have enabled/colored the leds according to it's own rules.
 * Here we simply disable the leds that are supposed to be dark, leading to a number display that animates according to the selected effect.
 */
void drawNumberDigitalClock(bool i0, bool i1, bool i2, bool i3, bool i4, bool i5, bool i6, int pos)
{
  pos = pos * 7;
  if (pos > 7)
  {
    pos = pos + 2;
  }
  if (!i0)
  {
    strip.setPixelColor(pos, 0x000000);
  }

  if (!i1)
  {
    strip.setPixelColor(pos + 1, 0x000000);
  }

  if (!i2)
  {
    strip.setPixelColor(pos + 2, 0x000000);
  }
  if (!i3)
  {
    strip.setPixelColor(pos + 3, 0x000000);
  }
  if (!i4)
  {
    strip.setPixelColor(pos + 4, 0x000000);
  }
  if (!i5)
  {
    strip.setPixelColor(pos + 5, 0x000000);
  }
  if (!i6)
  {
    strip.setPixelColor(pos + 6, 0x000000);
  }
}
/**
 * Encode a single digit into a 7-segment sequence.
 */
void writeDigitDigitalClock(int num, int pos)
{
  switch (num)
  {
  case 0:
    drawNumberDigitalClock(false, true, true, true, true, true, true, pos);
    break;
  case 1:
    drawNumberDigitalClock(false, true, false, false, false, false, true, pos);
    break;
  case 2:
    drawNumberDigitalClock(true, true, true, false, true, true, false, pos);
    break;
  case 3:
    drawNumberDigitalClock(true, true, true, false, false, true, true, pos);
    break;
  case 4:
    drawNumberDigitalClock(true, true, false, true, false, false, true, pos);
    break;
  case 5:
    drawNumberDigitalClock(true, false, true, true, false, true, true, pos);
    break;
  case 6:
    drawNumberDigitalClock(true, false, true, true, true, true, true, pos);
    break;
  case 7:
    drawNumberDigitalClock(false, true, true, false, false, false, true, pos);
    break;
  case 8:
    drawNumberDigitalClock(true, true, true, true, true, true, true, pos);
    break;
  case 9:
    drawNumberDigitalClock(true, true, true, true, false, true, true, pos);
    break;
  }
}

void _overlayDigitalClock()
{
  // hours
  writeDigitDigitalClock(hour(localTime) / 10, 0);
  writeDigitDigitalClock(hour(localTime) - ((hour(localTime) / 10) * 10), 1);
  // minutes
  writeDigitDigitalClock(minute(localTime) / 10, 2);
  writeDigitDigitalClock(minute(localTime) - ((minute(localTime) / 10) * 10), 3);
}

void _overlayAnalogClock()
{
  int overlaySize = overlayMax - overlayMin +1;
  if (countdownMode)
  {
    _overlayAnalogCountdown(); return;
  }
  double hourP = ((double)(hour(localTime)%12))/12;
  double minuteP = ((double)minute(localTime))/60;
  hourP = hourP + minuteP/12;
  double secondP = ((double)second(localTime))/60;
  int hourPixel = floor(analogClock12pixel + overlaySize*hourP);
  if (hourPixel > overlayMax) hourPixel = overlayMin -1 + hourPixel - overlayMax;
  int minutePixel = floor(analogClock12pixel + overlaySize*minuteP);
  if (minutePixel > overlayMax) minutePixel = overlayMin -1 + minutePixel - overlayMax; 
  int secondPixel = floor(analogClock12pixel + overlaySize*secondP);
  if (secondPixel > overlayMax) secondPixel = overlayMin -1 + secondPixel - overlayMax;
  if (analogClockSecondsTrail)
  {
    if (secondPixel < analogClock12pixel)
    {
      strip.setRange(analogClock12pixel, overlayMax, 0xFF0000);
      strip.setRange(overlayMin, secondPixel, 0xFF0000);
    } else
    {
      strip.setRange(analogClock12pixel, secondPixel, 0xFF0000);
    }
  }
  if (analogClock5MinuteMarks)
  {
    for (byte i = 0; i <= 12; i++)
    {
      int pix = analogClock12pixel + round((overlaySize / 12.0) *i);
      if (pix > overlayMax) pix -= overlaySize;
      strip.setPixelColor(pix, 0x00FFAA);
    }
  }
  if (!analogClockSecondsTrail) strip.setPixelColor(secondPixel, 0xFF0000);
  strip.setPixelColor(minutePixel, 0x00FF00);
  strip.setPixelColor(hourPixel, 0x0000FF);
}


void _overlayAnalogCountdown()
{
  if ((unsigned long)toki.second() < countdownTime)
  {
    long diff = countdownTime - toki.second();
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
    byte pixelCnt = perc*overlaySize;
    if (analogClock12pixel + pixelCnt > overlayMax)
    {
      strip.setRange(analogClock12pixel, overlayMax, ((uint32_t)colSec[3] << 24)| ((uint32_t)colSec[0] << 16) | ((uint32_t)colSec[1] << 8) | colSec[2]);
      strip.setRange(overlayMin, overlayMin +pixelCnt -(1+ overlayMax -analogClock12pixel), ((uint32_t)colSec[3] << 24)| ((uint32_t)colSec[0] << 16) | ((uint32_t)colSec[1] << 8) | colSec[2]);
    } else
    {
      strip.setRange(analogClock12pixel, analogClock12pixel + pixelCnt, ((uint32_t)colSec[3] << 24)| ((uint32_t)colSec[0] << 16) | ((uint32_t)colSec[1] << 8) | colSec[2]);
    }
  }
}

void handleOverlayDraw() {
  usermods.handleOverlayDraw();
  if (!overlayCurrent) return;
  switch (overlayCurrent)
  {
    case 1: _overlayAnalogClock(); break;
    case 3: _drawOverlayCronixie(); break;
    case 4: _overlayDigitalClock(); break;
  }
}

/*
 * Support for the Cronixie clock
 */
 
#ifndef WLED_DISABLE_CRONIXIE
byte _digitOut[6] = {10,10,10,10,10,10};
 
byte getSameCodeLength(char code, int index, char const cronixieDisplay[])
{
  byte counter = 0;
  
  for (int i = index+1; i < 6; i++)
  {
    if (cronixieDisplay[i] == code)
    {
      counter++;
    } else {
      return counter;
    }
  }
  return counter;
}

void setCronixie()
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
  DEBUG_PRINTLN(cronixieDisplay);
  
  for (int i = 0; i < 6; i++)
  {
    dP[i] = 10;
    switch (cronixieDisplay[i])
    {
      case '_': dP[i] = 10; break; 
      case '-': dP[i] = 11; break; 
      case 'r': dP[i] = random(1,7); break; //random btw. 1-6
      case 'R': dP[i] = random(0,10); break; //random btw. 0-9
      //case 't': break; //Test upw.
      //case 'T': break; //Test dnw.
      case 'b': dP[i] = 14 + getSameCodeLength('b',i,cronixieDisplay); i = i+dP[i]-14; break; 
      case 'B': dP[i] = 14 + getSameCodeLength('B',i,cronixieDisplay); i = i+dP[i]-14; break;
      case 'h': dP[i] = 70 + getSameCodeLength('h',i,cronixieDisplay); i = i+dP[i]-70; break;
      case 'H': dP[i] = 20 + getSameCodeLength('H',i,cronixieDisplay); i = i+dP[i]-20; break;
      case 'A': dP[i] = 108; i++; break;
      case 'a': dP[i] = 58; i++; break;
      case 'm': dP[i] = 74 + getSameCodeLength('m',i,cronixieDisplay); i = i+dP[i]-74; break;
      case 'M': dP[i] = 24 + getSameCodeLength('M',i,cronixieDisplay); i = i+dP[i]-24; break;
      case 's': dP[i] = 80 + getSameCodeLength('s',i,cronixieDisplay); i = i+dP[i]-80; break; //refresh more often bc. of secs
      case 'S': dP[i] = 30 + getSameCodeLength('S',i,cronixieDisplay); i = i+dP[i]-30; break;
      case 'Y': dP[i] = 36 + getSameCodeLength('Y',i,cronixieDisplay); i = i+dP[i]-36; break; 
      case 'y': dP[i] = 86 + getSameCodeLength('y',i,cronixieDisplay); i = i+dP[i]-86; break; 
      case 'I': dP[i] = 39 + getSameCodeLength('I',i,cronixieDisplay); i = i+dP[i]-39; break;  //Month. Don't ask me why month and minute both start with M.
      case 'i': dP[i] = 89 + getSameCodeLength('i',i,cronixieDisplay); i = i+dP[i]-89; break; 
      //case 'W': break;
      //case 'w': break;
      case 'D': dP[i] = 43 + getSameCodeLength('D',i,cronixieDisplay); i = i+dP[i]-43; break;
      case 'd': dP[i] = 93 + getSameCodeLength('d',i,cronixieDisplay); i = i+dP[i]-93; break;
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
      //case 'V': break; //user var0
      //case 'v': break; //user var1
    }
  }
  DEBUG_PRINT("result ");
  for (int i = 0; i < 5; i++)
  {
    DEBUG_PRINT((int)dP[i]);
    DEBUG_PRINT(" ");
  }
  DEBUG_PRINTLN((int)dP[5]);

  _overlayCronixie(); //refresh
}

void _overlayCronixie()
{
  byte h = hour(localTime);
  byte h0 = h;
  byte m = minute(localTime);
  byte s = second(localTime);
  byte d = day(localTime);
  byte mi = month(localTime);
  int y = year(localTime);
  //this has to be changed in time for 22nd century
  y -= 2000; if (y<0) y += 30; //makes countdown work

  if (useAMPM && !countdownMode)
  {
    if (h>12) h-=12;
    else if (h==0) h+=12;
  }
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
          
          case 43: _digitOut[i] = weekday(localTime); _digitOut[i]--; if (_digitOut[i]<1) _digitOut[i]= 7; break; //D
          case 44: _digitOut[i] = d/10; _digitOut[i+1] = d- _digitOut[i]*10; i++; break; //DD
          case 40: _digitOut[i] = mi/10; _digitOut[i+1] = mi- _digitOut[i]*10; i++; break; //II
          case 37: _digitOut[i] = y/10; _digitOut[i+1] = y- _digitOut[i]*10; i++; break; //YY
          case 39: _digitOut[i] = 2; _digitOut[i+1] = 0; _digitOut[i+2] = y/10; _digitOut[i+3] = y- _digitOut[i+2]*10; i+=3; break; //YYYY
          
          //case 16: _digitOut[i+2] = ((h0/3)&1)?1:0; i++; //BBB (BBBB NI)
          //case 15: _digitOut[i+1] = (h0>17 || (h0>5 && h0<12))?1:0; i++; //BB
          case 14: _digitOut[i] = (h0>11)?1:0; break; //B
        }
      } else
      {
        switch(dP[i])
        {
          case 71: _digitOut[i] = h/10; _digitOut[i+1] = h- _digitOut[i]*10; if(_digitOut[i] == 0) _digitOut[i]=10; i++; break; //hh
          case 75: _digitOut[i] = m/10; _digitOut[i+1] = m- _digitOut[i]*10; if(_digitOut[i] == 0) _digitOut[i]=10; i++; break; //mm
          case 81: _digitOut[i] = s/10; _digitOut[i+1] = s- _digitOut[i]*10; if(_digitOut[i] == 0) _digitOut[i]=10; i++; break; //ss
          //case 66: _digitOut[i+2] = ((h0/3)&1)?1:10; i++; //bbb (bbbb NI)
          //case 65: _digitOut[i+1] = (h0>17 || (h0>5 && h0<12))?1:10; i++; //bb
          case 64: _digitOut[i] = (h0>11)?1:10; break; //b

          case 93: _digitOut[i] = weekday(localTime); _digitOut[i]--; if (_digitOut[i]<1) _digitOut[i]= 7; break; //d
          case 94: _digitOut[i] = d/10; _digitOut[i+1] = d- _digitOut[i]*10; if(_digitOut[i] == 0) _digitOut[i]=10; i++; break; //dd
          case 90: _digitOut[i] = mi/10; _digitOut[i+1] = mi- _digitOut[i]*10; if(_digitOut[i] == 0) _digitOut[i]=10; i++; break; //ii
          case 87: _digitOut[i] = y/10; _digitOut[i+1] = y- _digitOut[i]*10; i++; break; //yy
          case 89: _digitOut[i] = 2; _digitOut[i+1] = 0; _digitOut[i+2] = y/10; _digitOut[i+3] = y- _digitOut[i+2]*10; i+=3; break; //yyyy
        }
      }
    }
  }
}

void _drawOverlayCronixie()
{
  byte offsets[] = {5, 0, 6, 1, 7, 2, 8, 3, 9, 4};
  
  for (uint16_t i = 0; i < 6; i++)
  {
    byte o = 10*i;
    byte excl = 10;
    if(_digitOut[i] < 10) excl = offsets[_digitOut[i]];
    excl += o;
    
    if (cronixieBacklight && _digitOut[i] <11)
    {
      uint32_t col = strip.gamma32(strip.getSegment(0).colors[1]);
      for (uint16_t j=o; j< o+10; j++) {
        if (j != excl) strip.setPixelColor(j, col);
      }
    } else
    {
      for (uint16_t j=o; j< o+10; j++) {
        if (j != excl) strip.setPixelColor(j, 0);
      }
    }
  }
}

#else // WLED_DISABLE_CRONIXIE
byte getSameCodeLength(char code, int index, char const cronixieDisplay[]) { return 0; }
void setCronixie() {}
void _overlayCronixie() {}
void _drawOverlayCronixie() {}
#endif
