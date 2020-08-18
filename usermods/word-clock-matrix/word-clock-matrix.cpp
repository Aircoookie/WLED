#include "wled.h"
/*
 * This v1 usermod file allows you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * EEPROM bytes 2750+ are reserved for your custom use case. (if you extend #define EEPSIZE in const.h)
 * If you just need 8 bytes, use 2551-2559 (you do not need to increase EEPSIZE)
 * 
 * Consider the v2 usermod API if you need a more advanced feature set!
 */


uint8_t minuteLast = 99;
int dayBrightness = 128;
int nightBrightness = 16;

//Use userVar0 and userVar1 (API calls &U0=,&U1=, uint16_t)

//gets called once at boot. Do all initialization that doesn't depend on network here
void userSetup()
{
  //select first two segments (background color + FX settable)
  WS2812FX::Segment &seg = strip.getSegment(0);
  seg.colors[0] = ((0 << 24) | ((0 & 0xFF) << 16) | ((0 & 0xFF) << 8) | ((0 & 0xFF)));
  strip.getSegment(0).setOption(0, false);
  strip.getSegment(0).setOption(2, false);
  //other segments are text
  for (int i = 1; i < 10; i++)
  {
    WS2812FX::Segment &seg = strip.getSegment(i);
    seg.colors[0] = ((0 << 24) | ((0 & 0xFF) << 16) | ((190 & 0xFF) << 8) | ((180 & 0xFF)));
    strip.getSegment(i).setOption(0, true);
    strip.setBrightness(128);
  }
}

//gets called every time WiFi is (re-)connected. Initialize own network interfaces here
void userConnected()
{
}


void hourChime()
{
  strip.resetSegments();

  WS2812FX::Segment &seg = strip.getSegment(0);
  seg.colors[0] = ((0 << 24) | ((0 & 0xFF) << 16) | ((0 & 0xFF) << 8) | ((0 & 0xFF)));
  for (int i = 1; i < 10; i++)
  {
    WS2812FX::Segment &seg = strip.getSegment(i);
    strip.getSegment(i).setOption(0, true);
    seg.mode = 12;
    seg.palette = 1;
    strip.setBrightness(255);
  }
}

void displayTimeHSHL(byte hour, byte minute)
{
  bool isToHour = false;      //true if minute > 30
  strip.setSegment(0, 0, 64); // background
  strip.setSegment(1, 0, 2);  //It is

  strip.setSegment(2, 0, 0);
  strip.setSegment(3, 0, 0); //disable minutes
  strip.setSegment(4, 0, 0); //past
  strip.setSegment(6, 0, 0); //to
  strip.setSegment(8, 0, 0); //disable o'clock

  if (hour < 24) //valid time, display
  {
    if (minute == 30)
    {
      strip.setSegment(2, 3, 6); //half
      strip.setSegment(3, 0, 0); //minutes
    }
    else if (minute == 15 || minute == 45)
    {
      strip.setSegment(3, 0, 0); //minutes
    }
    else if (minute == 10)
    {
      //strip.setSegment(5, 6, 8); //ten
    }
    else if (minute == 5)
    {
      //strip.setSegment(5, 16, 18); //five
    }
    else if (minute == 0)
    {
      strip.setSegment(3, 0, 0); //minutes
      //hourChime();
    }
    else
    {
      strip.setSegment(3, 18, 22); //minutes
    }

    //past or to?
    if (minute == 0)
    {                              //full hour
      strip.setSegment(3, 0, 0);   //disable minutes
      strip.setSegment(4, 0, 0);   //disable past
      strip.setSegment(6, 0, 0);   //disable to
      strip.setSegment(8, 60, 64); //o'clock
    }
    else if (minute > 34)
    {
      //strip.setSegment(6, 22, 24); //to
      //minute = 60 - minute;
      isToHour = true;
    }
    else
    {
      //strip.setSegment(4, 24, 27); //past
      //isToHour = false;
    }
  }
  else
  { //temperature display
  }

  //byte minuteRem = minute %10;

  if (minute <= 4)
  {
    strip.setSegment(3, 0, 0);   //nothing
    strip.setSegment(5, 0, 0);   //nothing
    strip.setSegment(6, 0, 0);   //nothing
    strip.setSegment(8, 60, 64); //o'clock
  }
  else if (minute <= 9)
  {
    strip.setSegment(5, 16, 18); // five past
    strip.setSegment(4, 24, 27); //past
  }
  else if (minute <= 14)
  {
    strip.setSegment(5, 6, 8);   // ten past
    strip.setSegment(4, 24, 27); //past
  }
  else if (minute <= 19)
  {
    strip.setSegment(5, 8, 12);  // quarter past
    strip.setSegment(3, 0, 0);   //minutes
    strip.setSegment(4, 24, 27); //past
  }
  else if (minute <= 24)
  {
    strip.setSegment(5, 12, 16); // twenty past
    strip.setSegment(4, 24, 27); //past
  }
  else if (minute <= 29)
  {
    strip.setSegment(5, 12, 18); // twenty-five past
    strip.setSegment(4, 24, 27); //past
  }
  else if (minute <= 34)
  {
    strip.setSegment(5, 3, 6);   // half past
    strip.setSegment(3, 0, 0);   //minutes
    strip.setSegment(4, 24, 27); //past
  }
  else if (minute <= 39)
  {
    strip.setSegment(5, 12, 18); // twenty-five to
    strip.setSegment(6, 22, 24); //to
  }
  else if (minute <= 44)
  {
    strip.setSegment(5, 12, 16); // twenty to
    strip.setSegment(6, 22, 24); //to
  }
  else if (minute <= 49)
  {
    strip.setSegment(5, 8, 12);  // quarter to
    strip.setSegment(3, 0, 0);   //minutes
    strip.setSegment(6, 22, 24); //to
  }
  else if (minute <= 54)
  {
    strip.setSegment(5, 6, 8);   // ten to
    strip.setSegment(6, 22, 24); //to
  }
  else if (minute <= 59)
  {
    strip.setSegment(5, 16, 18); // five to
    strip.setSegment(6, 22, 24); //to
  }

  //hours
  if (hour > 23)
    return;
  if (isToHour)
    hour++;
  if (hour > 12)
    hour -= 12;
  if (hour == 0)
    hour = 12;

  switch (hour)
  {
  case 1:
    strip.setSegment(7, 27, 29);
    break; //one
  case 2:
    strip.setSegment(7, 35, 37);
    break; //two
  case 3:
    strip.setSegment(7, 29, 32);
    break; //three
  case 4:
    strip.setSegment(7, 32, 35);
    break; //four
  case 5:
    strip.setSegment(7, 37, 40);
    break; //five
  case 6:
    strip.setSegment(7, 43, 45);
    break; //six
  case 7:
    strip.setSegment(7, 40, 43);
    break; //seven
  case 8:
    strip.setSegment(7, 45, 48);
    break; //eight
  case 9:
    strip.setSegment(7, 48, 50);
    break; //nine
  case 10:
    strip.setSegment(7, 54, 56);
    break; //ten
  case 11:
    strip.setSegment(7, 50, 54);
    break; //eleven
  case 12:
    strip.setSegment(7, 56, 60);
    break; //twelve
  }
}

void timeOfDay() {
  //Used to set brightness dependant of time of day - lights dimmed at night

  //monday to thursday and sunday

  if ((weekday(local) == 6) | (weekday(local) == 7)) {
    if (hour(local) > 0 | hour(local) < 8) {
      strip.setBrightness(nightBrightness);
    }
    else {
      strip.setBrightness(dayBrightness);
    }
  }
  else {
    if (hour(local) < 6 | hour(local) >= 22) {
      strip.setBrightness(nightBrightness);
    }
    else {
      strip.setBrightness(dayBrightness);
    }
  }
}

//loop. You can use "if (WLED_CONNECTED)" to check for successful connection
void userLoop()
{
  if (minute(local) != minuteLast)
  {
    updateLocalTime();
    timeOfDay();
    minuteLast = minute(local);
    displayTimeHSHL(hour(local), minute(local));
    if (minute(local) == 0){
      //hourChime();
    }
    if (minute(local) == 1){
      //userSetup();
    }
  }
}
