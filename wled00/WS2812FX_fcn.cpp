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

#include "WS2812FX.h"
#include "palettes.h"

#define LED_SKIP_AMOUNT 1

void WS2812FX::init(bool supportWhite, uint16_t countPixels, bool skipFirst)
{
  if (supportWhite == _rgbwMode && countPixels == _length && _locked != NULL) return;
  RESET_RUNTIME;
  _rgbwMode = supportWhite;
  _skipFirstMode = skipFirst;
  _length = countPixels;
  
  uint8_t ty = 1;
  if (supportWhite) ty =2;
  uint16_t lengthRaw = _length;
  if (_skipFirstMode) lengthRaw += LED_SKIP_AMOUNT;
  bus->Begin((NeoPixelType)ty, lengthRaw);
  
  if (_locked != NULL) delete _locked;
  _locked = new byte[_length];
  
  _segments[0].start = 0;
  _segments[0].stop = _length -1;
  
  unlockAll();
  setBrightness(_brightness);
}

void WS2812FX::service() {
  unsigned long now = millis(); // Be aware, millis() rolls over every 49 days
  bool doShow = false;
  for(uint8_t i=0; i < _num_segments; i++)
  {
    _segment_index = i;
    if(now > SEGMENT_RUNTIME.next_time || _triggered)
    {
      doShow = true;
      handle_palette();
      uint16_t delay = (this->*_mode[SEGMENT.mode])();
      SEGMENT_RUNTIME.next_time = now + max(delay, 5);
      SEGMENT_RUNTIME.counter_mode_call++;
    }
  }
  if(doShow) {
    show();
  }
  _triggered = false;
}

bool WS2812FX::modeUsesLock(uint8_t m)
{
  if (m == FX_MODE_FIRE_2012 || m == FX_MODE_COLORTWINKLE  ||
      m == FX_MODE_METEOR    || m == FX_MODE_METEOR_SMOOTH || 
      m == FX_MODE_RIPPLE) return true;
  return false;
}

void WS2812FX::setPixelColor(uint16_t n, uint32_t c) {
  uint8_t w = (c >> 24) & 0xFF;
  uint8_t r = (c >> 16) & 0xFF;
  uint8_t g = (c >>  8) & 0xFF;
  uint8_t b =  c        & 0xFF;
  setPixelColor(n, r, g, b, w);
}

void WS2812FX::setPixelColor(uint16_t i, byte r, byte g, byte b, byte w)
{
  if (_locked[i] && !_modeUsesLock) return;
  if (_reverseMode) i = _length - 1 -i;
  if (IS_REVERSE)   i = SEGMENT.stop - (i - SEGMENT.start); //reverse just individual segment
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
    if (_skipFirstMode)
    { 
      if (i < LED_SKIP_AMOUNT) bus->SetPixelColor(i, RgbwColor(0,0,0,0));
      i += LED_SKIP_AMOUNT;
    }
  
    bus->SetPixelColor(i, RgbwColor(r,g,b,w));
  } else {
    if(i>6)return;
    byte o = 10*i;
    if (_cronixieBacklightEnabled && _cronixieDigits[i] <11)
    {
      byte r2 = (_segments[0].colors[1] >>16) & 0xFF;
      byte g2 = (_segments[0].colors[1] >> 8) & 0xFF;
      byte b2 = (_segments[0].colors[1]     ) & 0xFF;
      byte w2 = (_segments[0].colors[1] >>24) & 0xFF;
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

void WS2812FX::setReverseMode(bool b)
{
  _reverseMode = b;
}

void WS2812FX::driverModeCronixie(bool b)
{
  _cronixieMode = b;
  _segments[0].stop = (b) ? 5 : _length-1;
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
#define PU_PER_MA        3600 //power units per milliamperere for accurate power estimation 
                              //formula: 195075 divided by mA per fully lit LED, here ~54mA)
                              //lowering the value increases the estimated usage and therefore makes the ABL more aggressive
                              
#define MA_FOR_ESP        100 //how much mA does the ESP use (Wemos D1 about 80mA, ESP32 about 120mA)
                              //you can set it to 0 if the ESP is powered by USB and the LEDs by external

void WS2812FX::show(void) {
  //power limit calculation
  //each LED can draw up 195075 "power units" (approx. 53mA)
  //one PU is the power it takes to have 1 channel 1 step brighter per brightness step
  //so A=2,R=255,G=0,B=0 would use 510 PU per LED (1mA is about 3700 PU)
  
  if (ablMilliampsMax > 149 && ablMilliampsMax < 65000) //lower numbers and 65000 turn off calculation
  {
    uint32_t powerBudget = (ablMilliampsMax - MA_FOR_ESP) * PU_PER_MA; //100mA for ESP power
    if (powerBudget > PU_PER_MA * _length) //each LED uses about 1mA in standby, exclude that from power budget
    {
      powerBudget -= PU_PER_MA * _length;
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
      powerSum >> 2; //same as /= 4
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
      currentMilliamps = (powerSum0 * newBri) / PU_PER_MA;
    } else
    {
      currentMilliamps = powerSum / PU_PER_MA;
      bus->SetBrightness(_brightness);
    }
    currentMilliamps += MA_FOR_ESP; //add power of ESP back to estimate
    currentMilliamps += _length; //add standby power back to estimate
  } else {
    currentMilliamps = 0;
  }
  
  bus->Show();
}

void WS2812FX::trigger() {
  _triggered = true;
}

void WS2812FX::setMode(uint8_t m) {
  RESET_RUNTIME;
  bool ua = modeUsesLock(_segments[0].mode) && !modeUsesLock(m);
  if (m > MODE_COUNT - 1) m = MODE_COUNT - 1;
  _segments[0].mode = m;
  if (ua) unlockAll();
  _modeUsesLock = modeUsesLock(_segments[0].mode);
  setBrightness(_brightness);
}

//TODO transitions

void WS2812FX::setSpeed(uint8_t s) {
  _segments[0].speed = s;
}

void WS2812FX::setIntensity(uint8_t in) {
  _segments[0].intensity = in;
}

void WS2812FX::setPalette(uint8_t p) {
  _segments[0].palette = p;
}

bool WS2812FX::setEffectConfig(uint8_t m, uint8_t s, uint8_t i, uint8_t p) {
  bool changed = false;
  m = constrain(m, 0, MODE_COUNT - 1);
  if (m != _segments[0].mode)      { setMode(m);      changed = true; }
  if (s != _segments[0].speed)     { setSpeed(s);     changed = true; }
  if (i != _segments[0].intensity) { setIntensity(i); changed = true; }
  if (p != _segments[0].palette)   { setPalette(p);   changed = true; }
  return changed;
}

void WS2812FX::setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  setColor(((uint32_t)w << 24) |((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}

void WS2812FX::setSecondaryColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  setSecondaryColor(((uint32_t)w << 24) |((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}

void WS2812FX::setColor(uint32_t c) {
  _segments[0].colors[0] = c;
}

void WS2812FX::setSecondaryColor(uint32_t c) {
  _segments[0].colors[1] = c;
}

void WS2812FX::setBrightness(uint8_t b) {
  if (_brightness == b) return;
  _brightness = b;
  bus->SetBrightness(_brightness);
  show();
}

uint8_t WS2812FX::getMode(void) {
  return _segments[0].mode;
}

uint8_t WS2812FX::getSpeed(void) {
  return _segments[0].speed;
}

uint8_t WS2812FX::getBrightness(void) {
  return _brightness;
}

uint8_t WS2812FX::getNumSegments(void) {
  return _num_segments;
}

void WS2812FX::setNumSegments(uint8_t n) {
  _num_segments = n;
}

uint32_t WS2812FX::getColor(void) {
  return _segments[0].colors[0];
}

uint32_t WS2812FX::getPixelColor(uint16_t i)
{
  if (_reverseMode) i = _length- 1 -i;
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

WS2812FX::Segment WS2812FX::getSegment(void) {
  return SEGMENT;
}

WS2812FX::Segment_runtime WS2812FX::getSegmentRuntime(void) {
  return SEGMENT_RUNTIME;
}

WS2812FX::Segment* WS2812FX::getSegments(void) {
  return _segments;
}

void WS2812FX::setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color, uint8_t speed, uint8_t intensity, bool reverse) {
  uint32_t colors[] = {color, 0, 0};
  setSegment(n, start, stop, mode, colors, speed, intensity, reverse);
}

void WS2812FX::setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint8_t speed, uint8_t intensity, bool reverse) {
  setSegment(n, start, stop, mode, colors, speed, intensity, (uint8_t)(reverse ? REVERSE : NO_OPTIONS));
}

void WS2812FX::setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint8_t speed, uint8_t intensity, uint8_t options) {
  if(n < (sizeof(_segments) / sizeof(_segments[0]))) {
    if(n + 1 > _num_segments) _num_segments = n + 1;
    _segments[n].start = start;
    _segments[n].stop = stop;
    _segments[n].mode = mode;
    _segments[n].speed = speed;
    _segments[n].intensity = intensity;
    _segments[n].options = options;

    for(uint8_t i=0; i<NUM_COLORS; i++) {
      _segments[n].colors[i] = colors[i];
    }
  }
}

void WS2812FX::resetSegments() {
  memset(_segments, 0, sizeof(_segments));
  memset(_segment_runtimes, 0, sizeof(_segment_runtimes));
  _segment_index = 0;
  _num_segments = 1;
  setSegment(0, 0, 7, FX_MODE_STATIC, (const uint32_t[]){DEFAULT_COLOR, 0, 0}, DEFAULT_SPEED, 128, NO_OPTIONS);
}

void WS2812FX::setIndividual(uint16_t i, uint32_t col)
{
  if (modeUsesLock(SEGMENT.mode)) return;
  if (i >= 0 && i < _length)
  {
    _locked[i] = false;
    setPixelColor(i, col);
    _locked[i] = true;
  }
}

void WS2812FX::setRange(uint16_t i, uint16_t i2, uint32_t col)
{
  if (i2 >= i)
  {
    for (uint16_t x = i; x <= i2; x++) setIndividual(x,col);
  } else
  {
    for (uint16_t x = i2; x <= i; x++) setIndividual(x,col);
  }
}

void WS2812FX::lock(uint16_t i)
{
  if (modeUsesLock(SEGMENT.mode)) return;
  if (i >= 0 && i < _length) _locked[i] = true;
}

void WS2812FX::lockRange(uint16_t i, uint16_t i2)
{
  if (modeUsesLock(SEGMENT.mode)) return;
  for (uint16_t x = i; x <= i2; x++)
  {
    if (i >= 0 && i < _length) _locked[i] = true;
  }
}

void WS2812FX::unlock(uint16_t i)
{
  if (modeUsesLock(SEGMENT.mode)) return;
  if (i >= 0 && i < _length) _locked[i] = false;
}

void WS2812FX::unlockRange(uint16_t i, uint16_t i2)
{
  if (modeUsesLock(SEGMENT.mode)) return;
  for (uint16_t x = i; x < i2; x++)
  {
    if (x >= 0 && x < _length) _locked[x] = false;
  }
}

void WS2812FX::unlockAll()
{
  for (int i=0; i < _length; i++) _locked[i] = false;
}

void WS2812FX::setTransitionMode(bool t)
{
  SEGMENT_RUNTIME.trans_act = (t) ? 1:2;
  if (!t) return;
  unsigned long waitMax = millis() + 20; //refresh after 20 ms if transition enabled
  if (SEGMENT.mode == FX_MODE_STATIC && SEGMENT_RUNTIME.next_time > waitMax) SEGMENT_RUNTIME.next_time = waitMax;
}

/*
 * color blend function
 */
uint32_t WS2812FX::color_blend(uint32_t color1, uint32_t color2, uint8_t blend) {
  if(blend == 0)   return color1;
  if(blend == 255) return color2;

  int w1 = (color1 >> 24) & 0xff;
  int r1 = (color1 >> 16) & 0xff;
  int g1 = (color1 >>  8) & 0xff;
  int b1 =  color1        & 0xff;

  int w2 = (color2 >> 24) & 0xff;
  int r2 = (color2 >> 16) & 0xff;
  int g2 = (color2 >>  8) & 0xff;
  int b2 =  color2        & 0xff;

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
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, c);
  }
}

/*
 * fade out function, higher rate = quicker fade
 */
void WS2812FX::fade_out(uint8_t rate) {
  rate = (255-rate) >> 1;
  float mappedRate = float(rate) +1.1;

  uint32_t color = SEGMENT.colors[1]; // target color
  int w2 = (color >> 24) & 0xff;
  int r2 = (color >> 16) & 0xff;
  int g2 = (color >>  8) & 0xff;
  int b2 =  color        & 0xff;

  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
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
  for(uint16_t i = SEGMENT.start; i <= SEGMENT.stop; i++)
  {
    CRGB cur = fastled_from_col(getPixelColor(i));
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


CRGB WS2812FX::fastled_from_col(uint32_t color)
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
  if ((SEGMENT.mode >= FX_MODE_METEOR) && SEGMENT.palette == 0) paletteIndex = 4;
  
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
      CRGB prim = fastled_from_col(SEGMENT.colors[0]);
      targetPalette = CRGBPalette16(prim); break;}
    case 3: {//based on primary
      //considering performance implications
      CRGB prim = fastled_from_col(SEGMENT.colors[0]);
      CHSV prim_hsv = rgb2hsv_approximate(prim);
      targetPalette = CRGBPalette16(
                      CHSV(prim_hsv.h, prim_hsv.s, prim_hsv.v), //color itself
                      CHSV(prim_hsv.h, max(prim_hsv.s - 50,0), prim_hsv.v), //less saturated
                      CHSV(prim_hsv.h, prim_hsv.s, max(prim_hsv.v - 50,0)), //darker
                      CHSV(prim_hsv.h, prim_hsv.s, prim_hsv.v)); //color itself
      break;}
    case 4: {//primary + secondary
      CRGB prim = fastled_from_col(SEGMENT.colors[0]);
      CRGB sec  = fastled_from_col(SEGMENT.colors[1]);
      targetPalette = CRGBPalette16(sec,prim); break;}
    case 5: {//based on primary + secondary
      CRGB prim = fastled_from_col(SEGMENT.colors[0]);
      CRGB sec  = fastled_from_col(SEGMENT.colors[1]);
      targetPalette = CRGBPalette16(sec,prim,CRGB::White); break;}
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
  if (SEGMENT.palette == 0 && mcol < 3) return SEGMENT.colors[mcol]; //WS2812FX default
  uint8_t paletteIndex = i;
  if (mapping) paletteIndex = map(i,SEGMENT.start,SEGMENT.stop,0,255);
  if (!wrap) paletteIndex = scale8(paletteIndex, 240); //cut off blend at palette "end"
  CRGB fastled_col;
  fastled_col = ColorFromPalette( currentPalette, paletteIndex, pbri, (paletteBlend == 3)? NOBLEND:LINEARBLEND);
  return  fastled_col.r*65536 +  fastled_col.g*256 +  fastled_col.b;
}
