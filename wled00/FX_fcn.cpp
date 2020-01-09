/*
  WS2812FX_fcn.cpp contains all utility functions
  Harm Aldick - 2016
  www.aldick.org
  LICENSE
  The MIT License (MIT)
  Copyright (c) 2016  Harm Aldick
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.

  Modified heavily for WLED
*/

#include "FX.h"
#include "palettes.h"

#define LED_SKIP_AMOUNT  1
#define MIN_SHOW_DELAY  15

void WS2812FX::init(bool supportWhite, uint16_t countPixels, bool skipFirst, uint8_t disableNLeds)
{
  if (supportWhite == _rgbwMode && countPixels == _length && disableNLeds == _disableNLeds) return;
  RESET_RUNTIME;
  _rgbwMode = supportWhite;
  _skipFirstMode = skipFirst;
  _length = countPixels;

  if (disableNLeds > 0) {
    uint16_t groupCount = disableNLeds +1;
    //since 1st led is lit, even partial group has a led lit, whereas int division truncates decimal.
    bool hasExtraLight = _length % groupCount != 0;
    _usableCount = _length/groupCount;
    _usableCount += hasExtraLight ? 1 : 0;
  } else {
    _usableCount = _length;
  }

  _disableNLeds = disableNLeds;

  uint8_t ty = 1;
  if (supportWhite) ty =2;
  _lengthRaw = _length;
  if (_skipFirstMode) {
    _lengthRaw += LED_SKIP_AMOUNT;
  }

  bus->Begin((NeoPixelType)ty, _lengthRaw);
  
  _segments[0].start = 0;
  _segments[0].stop = _usableCount;

  setBrightness(_brightness);
}

void WS2812FX::service() {
  uint32_t nowUp = millis(); // Be aware, millis() rolls over every 49 days
  now = nowUp + timebase;
  if (nowUp - _lastShow < MIN_SHOW_DELAY) return;
  bool doShow = false;
  for(uint8_t i=0; i < MAX_NUM_SEGMENTS; i++)
  {
    _segment_index = i;
    if (SEGMENT.isActive())
    {
      if(nowUp > SEGENV.next_time || _triggered || (doShow && SEGMENT.mode == 0)) //last is temporary
      {
        doShow = true;
        handle_palette();
        uint16_t delay = (this->*_mode[SEGMENT.mode])();
        SEGENV.next_time = nowUp + delay;
        if (SEGMENT.mode != FX_MODE_HALLOWEEN_EYES) SEGENV.call++;
      }
    }
  }
  if(doShow) {
    yield();
    show();
  }
  _triggered = false;
}

void WS2812FX::setPixelColor(uint16_t n, uint32_t c) {
  uint8_t w = (c >> 24);
  uint8_t r = (c >> 16);
  uint8_t g = (c >>  8);
  uint8_t b =  c       ;
  setPixelColor(n, r, g, b, w);
}

void WS2812FX::setPixelColor(uint16_t i, byte r, byte g, byte b, byte w)
{
  i = i * (_disableNLeds+1);
  if (IS_REVERSE) i = SEGMENT.stop -1 -i + SEGMENT.start; //reverse just individual segment
  byte tmpg = g;
  switch (colorOrder) //0 = Grb, default
  {
    case 0: break;                  //0 = Grb, default
    case 1: g = r; r = tmpg; break; //1 = Rgb, common for WS2811
    case 2: g = b; b = tmpg; break; //2 = Brg
    case 3: g = b; b = r; r = tmpg; //3 = Rbg
  }
  if (!_cronixieMode)
  {
    if (reverseMode) i = _length -1 -i;
    if (_skipFirstMode)
    { 
      if (i < LED_SKIP_AMOUNT) bus->SetPixelColor(i, RgbwColor(0,0,0,0));
      i += LED_SKIP_AMOUNT;
    }
    if (i < _lengthRaw) bus->SetPixelColor(i, RgbwColor(r,g,b,w));
    if (_disableNLeds > 0) {
      for(uint16_t offCount=0; offCount < _disableNLeds; offCount++) {
        if (i < _lengthRaw) bus->SetPixelColor((i + offCount + 1), RgbwColor(0,0,0,0));
      }
    }
  } else {
    if(i>6)return;
    byte o = 10*i;
    if (_cronixieBacklightEnabled && _cronixieDigits[i] <11)
    {
      byte r2 = _segments[0].colors[1] >>16;
      byte g2 = _segments[0].colors[1] >> 8;
      byte b2 = _segments[0].colors[1];
      byte w2 = _segments[0].colors[1] >>24;
      for (int j=o; j< o+19; j++)
      {
        bus->SetPixelColor(j, RgbwColor(r2,g2,b2,w2));
      }
    } else
    {
      for (int j=o; j< o+19; j++)
      {
        bus->SetPixelColor(j, RgbwColor(0,0,0,0));
      }
    }
    if (_skipFirstMode) o += LED_SKIP_AMOUNT;
    switch(_cronixieDigits[i])
    {
      case 0: bus->SetPixelColor(o+5, RgbwColor(r,g,b,w)); break;
      case 1: bus->SetPixelColor(o+0, RgbwColor(r,g,b,w)); break;
      case 2: bus->SetPixelColor(o+6, RgbwColor(r,g,b,w)); break;
      case 3: bus->SetPixelColor(o+1, RgbwColor(r,g,b,w)); break;
      case 4: bus->SetPixelColor(o+7, RgbwColor(r,g,b,w)); break;
      case 5: bus->SetPixelColor(o+2, RgbwColor(r,g,b,w)); break;
      case 6: bus->SetPixelColor(o+8, RgbwColor(r,g,b,w)); break;
      case 7: bus->SetPixelColor(o+3, RgbwColor(r,g,b,w)); break;
      case 8: bus->SetPixelColor(o+9, RgbwColor(r,g,b,w)); break;
      case 9: bus->SetPixelColor(o+4, RgbwColor(r,g,b,w)); break;
    }
  }
}

void WS2812FX::driverModeCronixie(bool b)
{
  _cronixieMode = b;
  _segments[0].stop = (b) ? 6 : _length;
}

void WS2812FX::setCronixieBacklight(bool b)
{
  _cronixieBacklightEnabled = b;
}

void WS2812FX::setCronixieDigits(byte d[])
{
  for (int i = 0; i<6; i++)
  {
    _cronixieDigits[i] = d[i];
  }
}


//DISCLAIMER
//The following function attemps to calculate the current LED power usage,
//and will limit the brightness to stay below a set amperage threshold.
//It is NOT a measurement and NOT guaranteed to stay within the ablMilliampsMax margin.
//Stay safe with high amperage and have a reasonable safety margin!
//I am NOT to be held liable for burned down garages!

//fine tune power estimation constants for your setup                  
#define MA_FOR_ESP        100 //how much mA does the ESP use (Wemos D1 about 80mA, ESP32 about 120mA)
                              //you can set it to 0 if the ESP is powered by USB and the LEDs by external

void WS2812FX::show(void) {
  if (_callback) _callback();
  
  //power limit calculation
  //each LED can draw up 195075 "power units" (approx. 53mA)
  //one PU is the power it takes to have 1 channel 1 step brighter per brightness step
  //so A=2,R=255,G=0,B=0 would use 510 PU per LED (1mA is about 3700 PU)
  
  if (ablMilliampsMax > 149 && milliampsPerLed > 0) //0 mA per LED and too low numbers turn off calculation
  {
    uint32_t puPerMilliamp = 195075 / milliampsPerLed;
    uint32_t powerBudget = (ablMilliampsMax - MA_FOR_ESP) * puPerMilliamp; //100mA for ESP power
    if (powerBudget > puPerMilliamp * _length) //each LED uses about 1mA in standby, exclude that from power budget
    {
      powerBudget -= puPerMilliamp * _length;
    } else
    {
      powerBudget = 0;
    }

    uint32_t powerSum = 0;

    for (uint16_t i = 0; i < _length; i++) //sum up the usage of each LED
    {
      RgbwColor c = bus->GetPixelColorRgbw(i);
      powerSum += (c.R + c.G + c.B + c.W);
    }

    if (_rgbwMode) //RGBW led total output with white LEDs enabled is still 50mA, so each channel uses less
    {
      powerSum *= 3;
      powerSum = powerSum >> 2; //same as /= 4
    }

    uint32_t powerSum0 = powerSum;
    powerSum *= _brightness;
    
    if (powerSum > powerBudget) //scale brightness down to stay in current limit
    {
      float scale = (float)powerBudget / (float)powerSum;
      uint16_t scaleI = scale * 255;
      uint8_t scaleB = (scaleI > 255) ? 255 : scaleI;
      uint8_t newBri = scale8(_brightness, scaleB);
      bus->SetBrightness(newBri);
      currentMilliamps = (powerSum0 * newBri) / puPerMilliamp;
    } else
    {
      currentMilliamps = powerSum / puPerMilliamp;
      bus->SetBrightness(_brightness);
    }
    currentMilliamps += MA_FOR_ESP; //add power of ESP back to estimate
    currentMilliamps += _length; //add standby power back to estimate
  } else {
    currentMilliamps = 0;
    bus->SetBrightness(_brightness);
  }
  
  bus->Show();
  _lastShow = millis();
}

void WS2812FX::trigger() {
  _triggered = true;
}

void WS2812FX::setMode(uint8_t segid, uint8_t m) {
  if (segid >= MAX_NUM_SEGMENTS) return;
   
  if (m >= MODE_COUNT) m = MODE_COUNT - 1;

  if (_segments[segid].mode != m) 
  {
    _segment_runtimes[segid].reset();
    _segments[segid].mode = m;
  }
}

uint8_t WS2812FX::getModeCount()
{
  return MODE_COUNT;
}

uint8_t WS2812FX::getPaletteCount()
{
  return 13 + gGradientPaletteCount;
}

//TODO transitions


bool WS2812FX::setEffectConfig(uint8_t m, uint8_t s, uint8_t in, uint8_t p) {
  uint8_t mainSeg = getMainSegmentId();
  Segment& seg = _segments[getMainSegmentId()];
  uint8_t modePrev = seg.mode, speedPrev = seg.speed, intensityPrev = seg.intensity, palettePrev = seg.palette;

  if (applyToAllSelected) {
    for (uint8_t i = 0; i < MAX_NUM_SEGMENTS; i++)
    {
      if (_segments[i].isSelected())
      {
        _segments[i].speed = s;
        _segments[i].intensity = in;
        _segments[i].palette = p;
        setMode(i, m);
      }
    }
  } else {
    seg.speed = s;
    seg.intensity = in;
    seg.palette = p;
    setMode(mainSegment, m);
  }
  
  if (seg.mode != modePrev || seg.speed != speedPrev || seg.intensity != intensityPrev || seg.palette != palettePrev) return true;
  return false;
}

void WS2812FX::setColor(uint8_t slot, uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  setColor(slot, ((uint32_t)w << 24) |((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}

void WS2812FX::setColor(uint8_t slot, uint32_t c) {
  if (slot >= NUM_COLORS) return;
  if (applyToAllSelected) {
    for (uint8_t i = 0; i < MAX_NUM_SEGMENTS; i++)
    {
      if (_segments[i].isSelected()) _segments[i].colors[slot] = c;
    }
  } else {
    _segments[getMainSegmentId()].colors[slot] = c;
  }
}

void WS2812FX::setBrightness(uint8_t b) {
  if (_brightness == b) return;
  _brightness = (gammaCorrectBri) ? gamma8(b) : b;
  _segment_index = 0;
  if (SEGENV.next_time > millis() + 22) show();//apply brightness change immediately if no refresh soon
}

uint8_t WS2812FX::getMode(void) {
  return _segments[getMainSegmentId()].mode;
}

uint8_t WS2812FX::getSpeed(void) {
  return _segments[getMainSegmentId()].speed;
}

uint8_t WS2812FX::getBrightness(void) {
  return _brightness;
}

uint8_t WS2812FX::getMaxSegments(void) {
  return MAX_NUM_SEGMENTS;
}

/*uint8_t WS2812FX::getFirstSelectedSegment(void)
{
  for (uint8_t i = 0; i < MAX_NUM_SEGMENTS; i++)
  {
    if (_segments[i].isActive() && _segments[i].isSelected()) return i;
  }
  for (uint8_t i = 0; i < MAX_NUM_SEGMENTS; i++) //if none selected, get first active
  {
    if (_segments[i].isActive()) return i;
  }
  return 0;
}*/

uint8_t WS2812FX::getMainSegmentId(void) {
  if (mainSegment >= MAX_NUM_SEGMENTS) return 0;
  return mainSegment;
}

uint32_t WS2812FX::getColor(void) {
  return _segments[getMainSegmentId()].colors[0];
}

uint32_t WS2812FX::getPixelColor(uint16_t i)
{
  i = i * (_disableNLeds+1);
  if (reverseMode) i = _length- 1 -i;
  if (IS_REVERSE)   i = SEGMENT.stop -1 -i + SEGMENT.start; //reverse just individual segment
  if (_skipFirstMode) i += LED_SKIP_AMOUNT;
  if (_cronixieMode)
  {
    if(i>6)return 0;
    byte o = 10*i;
    switch(_cronixieDigits[i])
    {
      case 0: i=o+5; break;
      case 1: i=o+0; break;
      case 2: i=o+6; break;
      case 3: i=o+1; break;
      case 4: i=o+7; break;
      case 5: i=o+2; break;
      case 6: i=o+8; break;
      case 7: i=o+3; break;
      case 8: i=o+9; break;
      case 9: i=o+4; break;
      default: return 0;
    }
  }
  if (i >= _lengthRaw) return 0;
  RgbwColor lColor = bus->GetPixelColorRgbw(i);
  byte r = lColor.R, g = lColor.G, b = lColor.B;
  switch (colorOrder)
  {
    case 0: break;                                    //0 = Grb
    case 1: r = lColor.G; g = lColor.R; break;        //1 = Rgb, common for WS2811
    case 2: g = lColor.B; b = lColor.G; break;        //2 = Brg
    case 3: r = lColor.B; g = lColor.R; b = lColor.G; //3 = Rbg
  }
  return ( (lColor.W << 24) | (r << 16) | (g << 8) | (b) );
}

WS2812FX::Segment& WS2812FX::getSegment(uint8_t id) {
  if (id >= MAX_NUM_SEGMENTS) return _segments[0];
  return _segments[id];
}

WS2812FX::Segment_runtime WS2812FX::getSegmentRuntime(void) {
  return SEGENV;
}

WS2812FX::Segment* WS2812FX::getSegments(void) {
  return _segments;
}

uint16_t WS2812FX::getUsableCount(void) {
  return _usableCount;
}

uint32_t WS2812FX::getLastShow(void) {
  return _lastShow;
}

void WS2812FX::setSegment(uint8_t n, uint16_t i1, uint16_t i2) {
  if (n >= MAX_NUM_SEGMENTS) return;
  Segment& seg = _segments[n];
  if (seg.start == i1 && seg.stop == i2) return;

  _segment_index = n; fill(0); //turn old segment range off
  if (i2 <= i1) //disable segment
  {
    seg.stop = 0; return;
  }
  if (i1 < _length) seg.start = i1;
  seg.stop = i2;
  if (i2 > _length) seg.stop = _length;
  _segment_runtimes[n].reset();
}

void WS2812FX::resetSegments() {
  memset(_segments, 0, sizeof(_segments));
  //memset(_segment_runtimes, 0, sizeof(_segment_runtimes));
  _segment_index = 0;
  _segments[0].mode = DEFAULT_MODE;
  _segments[0].colors[0] = DEFAULT_COLOR;
  _segments[0].start = 0;
  _segments[0].speed = DEFAULT_SPEED;
  _segments[0].stop = _length;
  _segments[0].setOption(0, 1); //select
  for (uint16_t i = 1; i < MAX_NUM_SEGMENTS; i++)
  {
    _segments[i].colors[0] = color_wheel(i*51);
    _segment_runtimes[i].reset();
  }
  _segment_runtimes[0].reset();
}

void WS2812FX::setRange(uint16_t i, uint16_t i2, uint32_t col)
{
  if (i2 >= i)
  {
    for (uint16_t x = i; x <= i2; x++) setPixelColor(x, col);
  } else
  {
    for (uint16_t x = i2; x <= i; x++) setPixelColor(x, col);
  }
}

void WS2812FX::setShowCallback(show_callback cb)
{
  _callback = cb;
}

void WS2812FX::setTransitionMode(bool t)
{
  _segment_index = getMainSegmentId();
  SEGMENT.setOption(7,t);
  if (!t) return;
  unsigned long waitMax = millis() + 20; //refresh after 20 ms if transition enabled
  if (SEGMENT.mode == FX_MODE_STATIC && SEGENV.next_time > waitMax) SEGENV.next_time = waitMax;
}

/*
 * color blend function
 */
uint32_t WS2812FX::color_blend(uint32_t color1, uint32_t color2, uint8_t blend) {
  if(blend == 0)   return color1;
  if(blend == 255) return color2;

  uint32_t w1 = (color1 >> 24) & 0xff;
  uint32_t r1 = (color1 >> 16) & 0xff;
  uint32_t g1 = (color1 >>  8) & 0xff;
  uint32_t b1 =  color1        & 0xff;

  uint32_t w2 = (color2 >> 24) & 0xff;
  uint32_t r2 = (color2 >> 16) & 0xff;
  uint32_t g2 = (color2 >>  8) & 0xff;
  uint32_t b2 =  color2        & 0xff;

  uint32_t w3 = ((w2 * blend) + (w1 * (255 - blend))) >> 8;
  uint32_t r3 = ((r2 * blend) + (r1 * (255 - blend))) >> 8;
  uint32_t g3 = ((g2 * blend) + (g1 * (255 - blend))) >> 8;
  uint32_t b3 = ((b2 * blend) + (b1 * (255 - blend))) >> 8;

  return ((w3 << 24) | (r3 << 16) | (g3 << 8) | (b3));
}

/*
 * Fills segment with color
 */
void WS2812FX::fill(uint32_t c) {
  for(uint16_t i=SEGMENT.start; i < SEGMENT.stop; i++) {
    setPixelColor(i, c);
  }
}

/*
 * fade out function, higher rate = quicker fade
 */
void WS2812FX::fade_out(uint8_t rate) {
  rate = (255-rate) >> 1;
  float mappedRate = float(rate) +1.1;

  uint32_t color = SEGCOLOR(1); // target color
  int w2 = (color >> 24) & 0xff;
  int r2 = (color >> 16) & 0xff;
  int g2 = (color >>  8) & 0xff;
  int b2 =  color        & 0xff;

  for(uint16_t i=SEGMENT.start; i < SEGMENT.stop; i++) {
    color = getPixelColor(i);
    int w1 = (color >> 24) & 0xff;
    int r1 = (color >> 16) & 0xff;
    int g1 = (color >>  8) & 0xff;
    int b1 =  color        & 0xff;

    int wdelta = (w2 - w1) / mappedRate;
    int rdelta = (r2 - r1) / mappedRate;
    int gdelta = (g2 - g1) / mappedRate;
    int bdelta = (b2 - b1) / mappedRate;

    // if fade isn't complete, make sure delta is at least 1 (fixes rounding issues)
    wdelta += (w2 == w1) ? 0 : (w2 > w1) ? 1 : -1;
    rdelta += (r2 == r1) ? 0 : (r2 > r1) ? 1 : -1;
    gdelta += (g2 == g1) ? 0 : (g2 > g1) ? 1 : -1;
    bdelta += (b2 == b1) ? 0 : (b2 > b1) ? 1 : -1;

    setPixelColor(i, r1 + rdelta, g1 + gdelta, b1 + bdelta, w1 + wdelta);
  }
}

/*
 * blurs segment content, source: FastLED colorutils.cpp
 */
void WS2812FX::blur(uint8_t blur_amount)
{
  uint8_t keep = 255 - blur_amount;
  uint8_t seep = blur_amount >> 1;
  CRGB carryover = CRGB::Black;
  for(uint16_t i = SEGMENT.start; i < SEGMENT.stop; i++)
  {
    CRGB cur = col_to_crgb(getPixelColor(i));
    CRGB part = cur;
    part.nscale8(seep);
    cur.nscale8(keep);
    cur += carryover;
    if(i > SEGMENT.start) {
      uint32_t c = getPixelColor(i-1);
      uint8_t r = (c >> 16 & 0xFF);
      uint8_t g = (c >> 8  & 0xFF);
      uint8_t b = (c       & 0xFF);
      setPixelColor(i-1, qadd8(r, part.red), qadd8(g, part.green), qadd8(b, part.blue));
    }
    setPixelColor(i,cur.red, cur.green, cur.blue);
    carryover = part;
  }
}

uint16_t WS2812FX::triwave16(uint16_t in)
{
  if (in < 0x8000) return in *2;
  return 0xFFFF - (in - 0x8000)*2;
}

/*
 * Put a value 0 to 255 in to get a color value.
 * The colours are a transition r -> g -> b -> back to r
 * Inspired by the Adafruit examples.
 */
uint32_t WS2812FX::color_wheel(uint8_t pos) {
  if (SEGMENT.palette) return color_from_palette(pos, false, true, 0);
  pos = 255 - pos;
  if(pos < 85) {
    return ((uint32_t)(255 - pos * 3) << 16) | ((uint32_t)(0) << 8) | (pos * 3);
  } else if(pos < 170) {
    pos -= 85;
    return ((uint32_t)(0) << 16) | ((uint32_t)(pos * 3) << 8) | (255 - pos * 3);
  } else {
    pos -= 170;
    return ((uint32_t)(pos * 3) << 16) | ((uint32_t)(255 - pos * 3) << 8) | (0);
  }
}

/*
 * Returns a new, random wheel index with a minimum distance of 42 from pos.
 */
uint8_t WS2812FX::get_random_wheel_index(uint8_t pos) {
  uint8_t r = 0, x = 0, y = 0, d = 0;

  while(d < 42) {
    r = random8();
    x = abs(pos - r);
    y = 255 - x;
    d = min(x, y);
  }
  return r;
}


uint32_t WS2812FX::crgb_to_col(CRGB fastled)
{
  return (((uint32_t)fastled.red << 16) | ((uint32_t)fastled.green << 8) | fastled.blue);
}


CRGB WS2812FX::col_to_crgb(uint32_t color)
{
  CRGB fastled_col;
  fastled_col.red =   (color >> 16 & 0xFF);
  fastled_col.green = (color >> 8  & 0xFF);
  fastled_col.blue =  (color       & 0xFF);
  return fastled_col;
}


/*
 * FastLED palette modes helper function. Limitation: Due to memory reasons, multiple active segments with FastLED will disable the Palette transitions
 */
void WS2812FX::handle_palette(void)
{
  bool singleSegmentMode = (_segment_index == _segment_index_palette_last);
  _segment_index_palette_last = _segment_index;

  byte paletteIndex = SEGMENT.palette;
  if (SEGMENT.mode == FX_MODE_GLITTER && paletteIndex == 0) paletteIndex = 11;
  if (SEGMENT.mode >= FX_MODE_METEOR && paletteIndex == 0) paletteIndex = 4;
  
  switch (paletteIndex)
  {
    case 0: {//default palette. Differs depending on effect
      switch (SEGMENT.mode)
      {
        case FX_MODE_FIRE_2012  : targetPalette = gGradientPalettes[22]; break;//heat palette
        case FX_MODE_COLORWAVES : targetPalette = gGradientPalettes[13]; break;//landscape 33
        case FX_MODE_FILLNOISE8 : targetPalette = OceanColors_p;         break;
        case FX_MODE_NOISE16_1  : targetPalette = gGradientPalettes[17]; break;//Drywet
        case FX_MODE_NOISE16_2  : targetPalette = gGradientPalettes[30]; break;//Blue cyan yellow
        case FX_MODE_NOISE16_3  : targetPalette = gGradientPalettes[22]; break;//heat palette
        case FX_MODE_NOISE16_4  : targetPalette = gGradientPalettes[13]; break;//landscape 33
        //case FX_MODE_GLITTER    : targetPalette = RainbowColors_p;       break;
        
        default: targetPalette = PartyColors_p; break;//palette, bpm
      }
      break;}
    case 1: {//periodically replace palette with a random one. Doesn't work with multiple FastLED segments
      if (!singleSegmentMode)
      {
        targetPalette = PartyColors_p; break; //fallback
      }
      if (millis() - _lastPaletteChange > 1000 + ((uint32_t)(255-SEGMENT.intensity))*100)
      {
        targetPalette = CRGBPalette16(
                        CHSV(random8(), 255, random8(128, 255)),
                        CHSV(random8(), 255, random8(128, 255)),
                        CHSV(random8(), 192, random8(128, 255)),
                        CHSV(random8(), 255, random8(128, 255)));
        _lastPaletteChange = millis();
      } break;}
    case 2: {//primary color only
      CRGB prim = col_to_crgb(SEGCOLOR(0));
      targetPalette = CRGBPalette16(prim); break;}
    case 3: {//based on primary
      //considering performance implications
      CRGB prim = col_to_crgb(SEGCOLOR(0));
      CHSV prim_hsv = rgb2hsv_approximate(prim);
      targetPalette = CRGBPalette16(
                      CHSV(prim_hsv.h, prim_hsv.s, prim_hsv.v), //color itself
                      CHSV(prim_hsv.h, max(prim_hsv.s - 50,0), prim_hsv.v), //less saturated
                      CHSV(prim_hsv.h, prim_hsv.s, max(prim_hsv.v - 50,0)), //darker
                      CHSV(prim_hsv.h, prim_hsv.s, prim_hsv.v)); //color itself
      break;}
    case 4: {//primary + secondary
      CRGB prim = col_to_crgb(SEGCOLOR(0));
      CRGB sec  = col_to_crgb(SEGCOLOR(1));
      targetPalette = CRGBPalette16(sec,prim); break;}
    case 5: {//based on primary + secondary
      CRGB prim = col_to_crgb(SEGCOLOR(0));
      CRGB sec  = col_to_crgb(SEGCOLOR(1));
      CRGB ter  = col_to_crgb(SEGCOLOR(2));
      targetPalette = CRGBPalette16(ter,sec,prim); break;}
    case 6: //Party colors
      targetPalette = PartyColors_p; break;
    case 7: //Cloud colors
      targetPalette = CloudColors_p; break;
    case 8: //Lava colors
      targetPalette = LavaColors_p; break;
    case 9: //Ocean colors
      targetPalette = OceanColors_p; break;
    case 10: //Forest colors
      targetPalette = ForestColors_p; break;
    case 11: //Rainbow colors
      targetPalette = RainbowColors_p; break;
    case 12: //Rainbow stripe colors
      targetPalette = RainbowStripeColors_p; break;
    default: //progmem palettes
      targetPalette = gGradientPalettes[constrain(SEGMENT.palette -13, 0, gGradientPaletteCount -1)];
  }
  
  if (singleSegmentMode && paletteFade) //only blend if just one segment uses FastLED mode
  {
    nblendPaletteTowardPalette(currentPalette, targetPalette, 48);
  } else
  {
    currentPalette = targetPalette;
  }
}

uint32_t WS2812FX::color_from_palette(uint16_t i, bool mapping, bool wrap, uint8_t mcol, uint8_t pbri)
{
  if (SEGMENT.palette == 0 && mcol < 3) return SEGCOLOR(mcol); //WS2812FX default
  uint8_t paletteIndex = i;
  if (mapping) paletteIndex = map(i,SEGMENT.start,SEGMENT.stop-1,0,255);
  if (!wrap) paletteIndex = scale8(paletteIndex, 240); //cut off blend at palette "end"
  CRGB fastled_col;
  fastled_col = ColorFromPalette( currentPalette, paletteIndex, pbri, (paletteBlend == 3)? NOBLEND:LINEARBLEND);
  return  fastled_col.r*65536 +  fastled_col.g*256 +  fastled_col.b;
}

bool WS2812FX::segmentsAreIdentical(Segment* a, Segment* b)
{
  //if (a->start != b->start) return false;
  //if (a->stop != b->stop) return false;
  for (uint8_t i = 0; i < NUM_COLORS; i++)
  {
    if (a->colors[i] != b->colors[i]) return false;
  }
  if (a->mode != b->mode) return false;
  if (a->speed != b->speed) return false;
  if (a->intensity != b->intensity) return false;
  if (a->palette != b->palette) return false;
  //if (a->getOption(1) != b->getOption(1)) return false; //reverse
  return true;
}

//gamma 2.4 lookup table used for color correction
const byte gammaT[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

uint8_t WS2812FX::gamma8(uint8_t b)
{
  return gammaT[b];
}

uint32_t WS2812FX::gamma32(uint32_t color)
{
  if (!gammaCorrectCol) return color;
  uint8_t w = (color >> 24);
  uint8_t r = (color >> 16);
  uint8_t g = (color >>  8);
  uint8_t b =  color;
  w = gammaT[w];
  r = gammaT[r];
  g = gammaT[g];
  b = gammaT[b];
  return ((w << 24) | (r << 16) | (g << 8) | (b));
}

uint16_t WS2812FX::_usedSegmentData = 0;
