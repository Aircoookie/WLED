/*
  WS2812FX.cpp - Library for WS2812 LED effects.
  Harm Aldick - 2016
  www.aldick.org
  FEATURES
    * A lot of blinken modes and counting
    * WS2812FX can be used as drop-in replacement for Adafruit NeoPixel Library
  NOTES
    * Uses the Adafruit NeoPixel library. Get it here:
      https://github.com/adafruit/Adafruit_NeoPixel
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
  CHANGELOG
  2016-05-28   Initial beta release
  2016-06-03   Code cleanup, minor improvements, new modes
  2016-06-04   2 new fx, fixed setColor (now also resets _mode_color)
  2017-02-02   added external trigger functionality (e.g. for sound-to-light)
  2017-02-02   removed "blackout" on mode, speed or color-change
  2017-09-26   implemented segment and reverse features
  2017-11-16   changed speed calc, reduced memory footprint
  2018-02-24   added hooks for user created custom effects
  Modified for WLED
*/

#include "WS2812FX.h"
#include "FastLED.h"
#include "palettes.h";

void WS2812FX::init(bool supportWhite, uint16_t countPixels, bool skipFirst)
{
  if (supportWhite == _rgbwMode && countPixels == _length && _locked != NULL) return;
  RESET_RUNTIME;
  _rgbwMode = supportWhite;
  _skipFirstMode = skipFirst;
  _length = countPixels;
  if (_skipFirstMode) _length++;
  uint8_t ty = 1;
  if (supportWhite) ty =2;
  bus->Begin((NeoPixelType)ty, _length);
  if (_locked != NULL) delete _locked;
  _locked = new byte[_length];
  _segments[0].start = 0;
  _segments[0].stop = _length -1;
  unlockAll();
  setBrightness(_brightness);
  show();
  _running = true;
}

void WS2812FX::service() {
  if(_running || _triggered) {
    unsigned long now = millis(); // Be aware, millis() rolls over every 49 days
    bool doShow = false;
    for(uint8_t i=0; i < _num_segments; i++) {
      _segment_index = i;
      if(now > SEGMENT_RUNTIME.next_time || _triggered) {
        doShow = true;
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
}

void WS2812FX::clear()
{
  bus->ClearTo(RgbColor(0));
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
  if (_reverseMode) i = _length - 1 -i;
  if (_locked[i] && SEGMENT.mode != FX_MODE_FIRE_2012) return;
  if (IS_REVERSE)   i = SEGMENT.stop - (i - SEGMENT.start); //reverse just individual segment
  if (!_cronixieMode)
  {
    if (_skipFirstMode) {i++;if(i==1)bus->SetPixelColor(i, RgbwColor(0,0,0,0));}
    bus->SetPixelColor(i, RgbwColor(r,g,b,w));
  } else {
    if(i>6)return;
    byte o = 10*i;
    if (_cronixieBacklightEnabled && _cronixieDigits[i] <11)
    {
      byte rCorr = (int)(((double)((_segments[0].colors[1]>>16) & 0xFF))*_cronixieSecMultiplier);
      byte gCorr = (int)(((double)((_segments[0].colors[1]>>8) & 0xFF))*_cronixieSecMultiplier);
      byte bCorr = (int)(((double)((_segments[0].colors[1]) & 0xFF))*_cronixieSecMultiplier);
      byte wCorr = (int)(((double)((_segments[0].colors[1]>>24) & 0xFF))*_cronixieSecMultiplier);
      for (int j=o; j< o+19; j++)
      {
        bus->SetPixelColor((_skipFirstMode)?j+1:j,RgbwColor(rCorr,gCorr,bCorr,wCorr));
      }
    } else
    {
      for (int j=o; j< o+19; j++)
      {
        bus->SetPixelColor((_skipFirstMode)?j+1:j,RgbwColor(0,0,0,0));
      }
    }
    switch(_cronixieDigits[i])
    {
      case 0: bus->SetPixelColor((_skipFirstMode)?o+6:o+5,RgbwColor(r,g,b,w)); break;
      case 1: bus->SetPixelColor((_skipFirstMode)?o+1:o+0,RgbwColor(r,g,b,w)); break;
      case 2: bus->SetPixelColor((_skipFirstMode)?o+7:o+6,RgbwColor(r,g,b,w)); break;
      case 3: bus->SetPixelColor((_skipFirstMode)?o+2:o+1,RgbwColor(r,g,b,w)); break;
      case 4: bus->SetPixelColor((_skipFirstMode)?o+8:o+7,RgbwColor(r,g,b,w)); break;
      case 5: bus->SetPixelColor((_skipFirstMode)?o+3:o+2,RgbwColor(r,g,b,w)); break;
      case 6: bus->SetPixelColor((_skipFirstMode)?o+9:o+8,RgbwColor(r,g,b,w)); break;
      case 7: bus->SetPixelColor((_skipFirstMode)?o+4:o+3,RgbwColor(r,g,b,w)); break;
      case 8: bus->SetPixelColor((_skipFirstMode)?o+10:o+9,RgbwColor(r,g,b,w)); break;
      case 9: bus->SetPixelColor((_skipFirstMode)?o+5:o+4,RgbwColor(r,g,b,w)); break;
      default: break;
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
  if (b) _segments[0].stop = 5;
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

void WS2812FX::show(void) {
  bus->Show();
}

void WS2812FX::trigger() {
  _triggered = true;
}

void WS2812FX::setMode(uint8_t m) {
  RESET_RUNTIME;
  bool ua = _segments[0].mode == FX_MODE_FIRE_2012 && m != FX_MODE_FIRE_2012;
  _segments[0].mode = constrain(m, 0, MODE_COUNT - 1);
  if (ua) unlockAll();
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
  if (_cronixieMode) _cronixieSecMultiplier = getSafePowerMultiplier(900, 100, c, _brightness);
}

void WS2812FX::setBrightness(uint8_t b) {
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
  if (_skipFirstMode) i++;
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
  return lColor.W*16777216 + lColor.R*65536 + lColor.G*256 + lColor.B;
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
  if (SEGMENT.mode == FX_MODE_FIRE_2012) return;
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
  if (SEGMENT.mode == FX_MODE_FIRE_2012) return;
  if (i >= 0 && i < _length) _locked[i] = true;
}

void WS2812FX::lockRange(uint16_t i, uint16_t i2)
{
  if (SEGMENT.mode == FX_MODE_FIRE_2012) return;
  for (uint16_t x = i; x <= i2; x++)
  {
    if (i >= 0 && i < _length) _locked[i] = true;
  }
}

void WS2812FX::unlock(uint16_t i)
{
  if (SEGMENT.mode == FX_MODE_FIRE_2012) return;
  if (i >= 0 && i < _length) _locked[i] = false;
}

void WS2812FX::unlockRange(uint16_t i, uint16_t i2)
{
  if (SEGMENT.mode == FX_MODE_FIRE_2012) return;
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
  unsigned long waitMax = millis() + 20; //refresh after 20 seconds if transition enabled
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

  uint32_t w3 = ((w2 * blend) + (w1 * (255 - blend))) / 256;
  uint32_t r3 = ((r2 * blend) + (r1 * (255 - blend))) / 256;
  uint32_t g3 = ((g2 * blend) + (g1 * (255 - blend))) / 256;
  uint32_t b3 = ((b2 * blend) + (b1 * (255 - blend))) / 256;

  return ((w3 << 24) | (r3 << 16) | (g3 << 8) | (b3));
}


double WS2812FX::getPowerEstimate(uint16_t leds, uint32_t c, byte b)
{
  double _mARequired = 100; //ESP power
  double _mul = (double)b/255;
  double _sum = ((c & 0xFF000000) >> 24) + ((c & 0x00FF0000) >> 16) + ((c & 0x0000FF00) >>  8) + ((c & 0x000000FF) >>  0);
  _sum /= (_rgbwMode)?1024:768;
  double _mAPerLed = 50*(_mul*_sum);
  _mARequired += leds*_mAPerLed;
  return _mARequired;
}

//DISCLAIMER
//This is just a helper function for huge amounts of LEDs.
//It is NOT guaranteed to stay within the safeAmps margin.
//Stay safe with high amperage and have a reasonable safety margin!
//I am NOT to be held liable for burned down garages!
double WS2812FX::getSafePowerMultiplier(double safeMilliAmps, uint16_t leds, uint32_t c, byte b)
{
  double _mARequired = getPowerEstimate(leds,c,b);
  if (_mARequired > safeMilliAmps)
  {
    return safeMilliAmps/_mARequired;
  }
  return 1.0;
}


/* #####################################################
#
#  Color and Blinken Functions
#
##################################################### */

/*
 * Turns everything off. Doh.
 */
void WS2812FX::strip_off() {
  clear();
  show();
}


/*
 * Put a value 0 to 255 in to get a color value.
 * The colours are a transition r -> g -> b -> back to r
 * Inspired by the Adafruit examples.
 */
uint32_t WS2812FX::color_wheel(uint8_t pos) {
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
  uint8_t r = 0;
  uint8_t x = 0;
  uint8_t y = 0;
  uint8_t d = 0;

  while(d < 42) {
    r = random8();
    x = abs(pos - r);
    y = 255 - x;
    d = min(x, y);
  }

  return r;
}


/*
 * No blinking. Just plain old static light.
 */
uint16_t WS2812FX::mode_static(void) {
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, SEGMENT.colors[0]);
  }
  return (SEGMENT_RUNTIME.trans_act == 1) ? 20 : 500;
}


/*
 * Blink/strobe function
 * Alternate between color1 and color2
 * if(strobe == true) then create a strobe effect
 */
uint16_t WS2812FX::blink(uint32_t color1, uint32_t color2, bool strobe) {
  uint32_t color = ((SEGMENT_RUNTIME.counter_mode_call & 1) == 0) ? color1 : color2;
  if(IS_REVERSE) color = (color == color1) ? color2 : color1;
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, color);
  }

  if((SEGMENT_RUNTIME.counter_mode_call & 1) == 0) {
    return strobe ? 20 : (100 + ((1986 * (uint32_t)(255 - SEGMENT.speed)) / 255))*(float)(SEGMENT.intensity/128.0);
  } else {
    return strobe ? 50 + ((1986 * (uint32_t)(255 - SEGMENT.speed)) / 255) : (100 + ((1986 * (uint32_t)(255 - SEGMENT.speed)) / 255))*(float)(2.0-(SEGMENT.intensity/128.0));
  }
}


/*
 * Normal blinking. 50% on/off time.
 */
uint16_t WS2812FX::mode_blink(void) {
  return blink(SEGMENT.colors[0], SEGMENT.colors[1], false);
}


/*
 * Classic Blink effect. Cycling through the rainbow.
 */
uint16_t WS2812FX::mode_blink_rainbow(void) {
  return blink(color_wheel(SEGMENT_RUNTIME.counter_mode_call & 0xFF), SEGMENT.colors[1], false);
}


/*
 * Classic Strobe effect.
 */
uint16_t WS2812FX::mode_strobe(void) {
  return blink(SEGMENT.colors[0], SEGMENT.colors[1], true);
}


/*
 * Classic Strobe effect. Cycling through the rainbow.
 */
uint16_t WS2812FX::mode_strobe_rainbow(void) {
  return blink(color_wheel(SEGMENT_RUNTIME.counter_mode_call & 0xFF), SEGMENT.colors[1], true);
}


/*
 * Color wipe function
 * LEDs are turned on (color1) in sequence, then turned off (color2) in sequence.
 * if (bool rev == true) then LEDs are turned off in reverse order
 */
uint16_t WS2812FX::color_wipe(uint32_t color1, uint32_t color2, bool rev) {
  if(SEGMENT_RUNTIME.counter_mode_step < SEGMENT_LENGTH) {
    uint32_t led_offset = SEGMENT_RUNTIME.counter_mode_step;
    setPixelColor(SEGMENT.start + led_offset, color1);
  } else {
    uint32_t led_offset = SEGMENT_RUNTIME.counter_mode_step - SEGMENT_LENGTH;
    if(rev) {
      setPixelColor(SEGMENT.stop - led_offset, color2);
    } else {
      setPixelColor(SEGMENT.start + led_offset, color2);
    }
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % (SEGMENT_LENGTH * 2);
  return SPEED_FORMULA_L;
}


/*
 * Lights all LEDs one after another.
 */
uint16_t WS2812FX::mode_color_wipe(void) {
  return color_wipe(SEGMENT.colors[0], SEGMENT.colors[1], false);
}

/*
 * Lights all LEDs one after another. Turns off opposite
 */
uint16_t WS2812FX::mode_color_sweep(void) {
  return color_wipe(SEGMENT.colors[0], SEGMENT.colors[1], true);
}


/*
 * Turns all LEDs after each other to a random color.
 * Then starts over with another color.
 */
uint16_t WS2812FX::mode_color_wipe_random(void) {
  if(SEGMENT_RUNTIME.counter_mode_step % SEGMENT_LENGTH == 0) { // aux_param will store our random color wheel index
    SEGMENT_RUNTIME.aux_param = get_random_wheel_index(SEGMENT_RUNTIME.aux_param);
  }
  uint32_t color = color_wheel(SEGMENT_RUNTIME.aux_param);
  return color_wipe(color, color, false);
}


/*
 * Random color introduced alternating from start and end of strip.
 */
uint16_t WS2812FX::mode_color_sweep_random(void) {
  if(SEGMENT_RUNTIME.counter_mode_step % SEGMENT_LENGTH == 0) { // aux_param will store our random color wheel index
    SEGMENT_RUNTIME.aux_param = get_random_wheel_index(SEGMENT_RUNTIME.aux_param);
  }
  uint32_t color = color_wheel(SEGMENT_RUNTIME.aux_param);
  return color_wipe(color, color, true);
}


/*
 * Lights all LEDs in one random color up. Then switches them
 * to the next random color.
 */
uint16_t WS2812FX::mode_random_color(void) {
  SEGMENT_RUNTIME.aux_param = get_random_wheel_index(SEGMENT_RUNTIME.aux_param); // aux_param will store our random color wheel index
  uint32_t color = color_wheel(SEGMENT_RUNTIME.aux_param);

  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, color);
  }
  return 50 + (20 * (uint32_t)(255 - SEGMENT.speed));
}


/*
 * Lights every LED in a random color. Changes all LED at the same time
 * to new random colors.
 */
uint16_t WS2812FX::mode_dynamic(void) {
  if(SEGMENT.intensity > 127 || SEGMENT_RUNTIME.counter_mode_call == 0) {
    for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
      setPixelColor(i, color_wheel(random8()));
    }
  }
  setPixelColor(SEGMENT.start + random(SEGMENT_LENGTH), color_wheel(random8()));
  return 50 + (15 * (uint32_t)(255 - SEGMENT.speed));
}


/*
 * Does the "standby-breathing" of well known i-Devices. Fixed Speed.
 * Use mode "fade" if you like to have something similar with a different speed.
 */
uint16_t WS2812FX::mode_breath(void) {
  int lum = SEGMENT_RUNTIME.counter_mode_step;
  if(lum > 255) lum = 511 - lum; // lum = 15 -> 255 -> 15

  uint16_t delay;
  if(lum == 15) delay = 970; // 970 pause before each breath
  else if(lum <=  25) delay = 38; // 19
  else if(lum <=  50) delay = 36; // 18
  else if(lum <=  75) delay = 28; // 14
  else if(lum <= 100) delay = 20; // 10
  else if(lum <= 125) delay = 14; // 7
  else if(lum <= 150) delay = 11; // 5
  else delay = 10; // 4

  uint32_t color = SEGMENT.colors[0];
  uint8_t w = (color >> 24 & 0xFF) * lum / 256;
  uint8_t r = (color >> 16 & 0xFF) * lum / 256;
  uint8_t g = (color >>  8 & 0xFF) * lum / 256;
  uint8_t b = (color       & 0xFF) * lum / 256;
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, r, g, b, w);
  }

  SEGMENT_RUNTIME.counter_mode_step += 2;
  if(SEGMENT_RUNTIME.counter_mode_step > (512-15)) SEGMENT_RUNTIME.counter_mode_step = 15;
  return delay;
}


/*
 * Fades the LEDs between two colors
 */
uint16_t WS2812FX::mode_fade(void) {
  int lum = SEGMENT_RUNTIME.counter_mode_step;
  if(lum > 255) lum = 511 - lum; // lum = 0 -> 255 -> 0

  uint32_t color = color_blend(SEGMENT.colors[0], SEGMENT.colors[1], lum);
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, color);
  }

  SEGMENT_RUNTIME.counter_mode_step += 4;
  if(SEGMENT_RUNTIME.counter_mode_step > 511) SEGMENT_RUNTIME.counter_mode_step = 0;
  return 5 + ((15 * (uint32_t)(255 - SEGMENT.speed)) / 255);
}


//TODO add intensity (more than 1 pixel lit)
/*
 * Runs a single pixel back and forth.
 */
uint16_t WS2812FX::mode_scan(void) {
  if(SEGMENT_RUNTIME.counter_mode_step > (SEGMENT_LENGTH * 2) - 3) {
    SEGMENT_RUNTIME.counter_mode_step = 0;
  }

  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, SEGMENT.colors[1]);
  }

  int led_offset = SEGMENT_RUNTIME.counter_mode_step - (SEGMENT_LENGTH - 1);
  led_offset = abs(led_offset); 
  setPixelColor(SEGMENT.start + led_offset, SEGMENT.colors[0]);

  SEGMENT_RUNTIME.counter_mode_step++;
  return SPEED_FORMULA_L;
}


/*
 * Runs two pixel back and forth in opposite directions.
 */
uint16_t WS2812FX::mode_dual_scan(void) {
  if(SEGMENT_RUNTIME.counter_mode_step > (SEGMENT_LENGTH * 2) - 3) {
    SEGMENT_RUNTIME.counter_mode_step = 0;
  }

  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, BLACK);
  }

  int led_offset = SEGMENT_RUNTIME.counter_mode_step - (SEGMENT_LENGTH - 1);
  led_offset = abs(led_offset);

  setPixelColor(SEGMENT.start + led_offset, SEGMENT.colors[0]);
  setPixelColor(SEGMENT.start + SEGMENT_LENGTH - led_offset - 1, SEGMENT.colors[0]);

  SEGMENT_RUNTIME.counter_mode_step++;
  return SPEED_FORMULA_L;
}


/*
 * Cycles all LEDs at once through a rainbow.
 */
uint16_t WS2812FX::mode_rainbow(void) {
  uint32_t color = color_wheel(SEGMENT_RUNTIME.counter_mode_step);
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, color);
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) & 0xFF;
  return 1 + (((uint32_t)(255 - SEGMENT.speed)) / 5);
}


/*
 * Cycles a rainbow over the entire string of LEDs.
 */
uint16_t WS2812FX::mode_rainbow_cycle(void) {
  for(uint16_t i=0; i < SEGMENT_LENGTH; i++) {
    uint32_t color = color_wheel(((i * 256 / ((uint16_t)(SEGMENT_LENGTH*(float)(SEGMENT.intensity/128.0))+1)) + SEGMENT_RUNTIME.counter_mode_step) & 0xFF);
    setPixelColor(SEGMENT.start + i, color);
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) & 0xFF;
  return 1 + (((uint32_t)(255 - SEGMENT.speed)) / 5);
}


/*
 * theater chase function
 */
uint16_t WS2812FX::theater_chase(uint32_t color1, uint32_t color2) {
  SEGMENT_RUNTIME.counter_mode_call = SEGMENT_RUNTIME.counter_mode_call % 3;
  for(uint16_t i=0; i < SEGMENT_LENGTH; i++) {
    if((i % 3) == SEGMENT_RUNTIME.counter_mode_call) {
      setPixelColor(SEGMENT.start + i, color1);
    } else {
      setPixelColor(SEGMENT.start + i, color2);
    }
  }
  return 50 + (2 * (uint32_t)(255 - SEGMENT.speed));
}


/*
 * Theatre-style crawling lights.
 * Inspired by the Adafruit examples.
 */
uint16_t WS2812FX::mode_theater_chase(void) {
  return theater_chase(SEGMENT.colors[0], SEGMENT.colors[1]);
}


/*
 * Theatre-style crawling lights with rainbow effect.
 * Inspired by the Adafruit examples.
 */
uint16_t WS2812FX::mode_theater_chase_rainbow(void) {
  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) & 0xFF;
  return theater_chase(color_wheel(SEGMENT_RUNTIME.counter_mode_step), SEGMENT.colors[1]);
}


/*
 * Running lights effect with smooth sine transition.
 */
uint16_t WS2812FX::mode_running_lights(void) {
  uint8_t w = ((SEGMENT.colors[0] >> 24) & 0xFF);
  uint8_t r = ((SEGMENT.colors[0] >> 16) & 0xFF);
  uint8_t g = ((SEGMENT.colors[0] >> 8) & 0xFF);
  uint8_t b = (SEGMENT.colors[0] & 0xFF);

  for(uint16_t i=0; i < SEGMENT_LENGTH; i++) {
    int s = (sin(i+SEGMENT_RUNTIME.counter_mode_call) * 127) + 128;
    setPixelColor(SEGMENT.start + i, (((uint32_t)(r * s)) / 255), (((uint32_t)(g * s)) / 255), (((uint32_t)(b * s)) / 255), (((uint32_t)(w * s)) / 255));
  }
  
  return 10 + (uint16_t)(255 - SEGMENT.speed);
}


/*
 * twinkle function
 */
uint16_t WS2812FX::twinkle(uint32_t color) {
  if(SEGMENT_RUNTIME.counter_mode_step == 0) {
    for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
      setPixelColor(i, SEGMENT.colors[1]);
    }
    uint16_t min_leds = max(1, SEGMENT_LENGTH / 5); // make sure, at least one LED is on
    uint16_t max_leds = max(1, SEGMENT_LENGTH / 2); // make sure, at least one LED is on
    SEGMENT_RUNTIME.counter_mode_step = random(min_leds, max_leds);
  }

  setPixelColor(SEGMENT.start + random(SEGMENT_LENGTH), color);

  SEGMENT_RUNTIME.counter_mode_step--;
  return 50 + (8 * (uint16_t)(255 - SEGMENT.speed));
}

/*
 * Blink several LEDs on, reset, repeat.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
uint16_t WS2812FX::mode_twinkle(void) {
  return twinkle(SEGMENT.colors[0]);
}

/*
 * Blink several LEDs in random colors on, reset, repeat.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
uint16_t WS2812FX::mode_twinkle_random(void) {
  return twinkle(color_wheel(random8()));
}


/*
 * fade out function
 * fades out the current segment by dividing each pixel's intensity by 2
 */
void WS2812FX::fade_out(uint8_t rate) {
  static const float rateMap[] = {1.1, 1.20, 1.5, 2.0, 4.0, 8.0, 16.0, 64.0};
  if (rate > 7) rate = 7;
  float mappedRate = rateMap[rate];

  uint32_t color = SEGMENT.colors[1]; // target color
  int w2 = (color >> 24) & 0xff;
  int r2 = (color >> 16) & 0xff;
  int g2 = (color >>  8) & 0xff;
  int b2 =  color        & 0xff;

  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    color = getPixelColor(i);
    if(rate == 0) { // old fade-to-black algorithm
      setPixelColor(i, (color >> 1) & 0x7F7F7F7F);
    } else { // new fade-to-color algorithm
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
}


/*
 * twinkle_fade function
 */
uint16_t WS2812FX::twinkle_fade(uint32_t color) {
  fade_out((255-SEGMENT.intensity) / 32);

  if(random8(3) == 0) {
    setPixelColor(SEGMENT.start + random(SEGMENT_LENGTH), color);
  }
  return 100 + ((uint32_t)(255 - SEGMENT.speed)) / 3;
}


/*
 * Blink several LEDs on, fading out.
 */
uint16_t WS2812FX::mode_twinkle_fade(void) {
  return twinkle_fade(SEGMENT.colors[0]);
}


/*
 * Blink several LEDs in random colors on, fading out.
 */
uint16_t WS2812FX::mode_twinkle_fade_random(void) {
  return twinkle_fade(color_wheel(random8()));
}


/*
 * Blinks one LED at a time.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
uint16_t WS2812FX::mode_sparkle(void) {
  setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.aux_param, SEGMENT.colors[1]);
  SEGMENT_RUNTIME.aux_param = random(SEGMENT_LENGTH); // aux_param stores the random led index
  setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.aux_param, SEGMENT.colors[0]);
  return 10 + (uint16_t)(255 - SEGMENT.speed);
}


/*
 * Lights all LEDs in the color. Flashes single white pixels randomly.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
uint16_t WS2812FX::mode_flash_sparkle(void) {
  if(SEGMENT_RUNTIME.counter_mode_call == 0) {
    for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
      setPixelColor(i, SEGMENT.colors[0]);
    }
  }

  setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.aux_param, SEGMENT.colors[0]);

  if(random8(5) == 0) {
    SEGMENT_RUNTIME.aux_param = random(SEGMENT_LENGTH); // aux_param stores the random led index
    setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.aux_param, SEGMENT.colors[1]);
    return 20;
  } 
  return 20 + (uint16_t)(255-SEGMENT.speed);
}


/*
 * Like flash sparkle. With more flash.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
uint16_t WS2812FX::mode_hyper_sparkle(void) {
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, SEGMENT.colors[0]);
  }

  if(random8(5) < 2) {
    for(uint16_t i=0; i < max(1, SEGMENT_LENGTH/3); i++) {
      setPixelColor(SEGMENT.start + random(SEGMENT_LENGTH), SEGMENT.colors[1]);
    }
    return 20;
  }
  return 20 + (uint16_t)(255-SEGMENT.speed);
}


/*
 * Strobe effect with different strobe count and pause, controlled by speed.
 */
uint16_t WS2812FX::mode_multi_strobe(void) {
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, SEGMENT.colors[1]);
  }

  uint16_t delay = 50 + 20*(uint16_t)(255-SEGMENT.speed);
  uint16_t count = 2 * ((SEGMENT.speed / 10) + 1);
  if(SEGMENT_RUNTIME.counter_mode_step < count) {
    if((SEGMENT_RUNTIME.counter_mode_step & 1) == 0) {
      for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
        setPixelColor(i, SEGMENT.colors[0]);
      }
      delay = 20;
    } else {
      delay = 50;
    }
  }
  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % (count + 1);
  return delay;
}

/*
 * Android loading circle
 */
uint16_t WS2812FX::mode_android(void) {
  if (SEGMENT_RUNTIME.counter_mode_call == 0)
  {
    SEGMENT_RUNTIME.aux_param = 0;
    SEGMENT_RUNTIME.counter_mode_step = SEGMENT.start;
  }
  
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, SEGMENT.colors[1]);
  }

  if (SEGMENT_RUNTIME.aux_param2 > ((float)SEGMENT.intensity/255.0)*(float)SEGMENT_LENGTH)
  {
    SEGMENT_RUNTIME.aux_param = 1;
  } else
  {
    if (SEGMENT_RUNTIME.aux_param2 < 2) SEGMENT_RUNTIME.aux_param = 0;
  }

  uint16_t a = SEGMENT_RUNTIME.counter_mode_step;
  
  if (SEGMENT_RUNTIME.aux_param == 0)
  {
    if (SEGMENT_RUNTIME.counter_mode_call %3 == 1) {a++;}
    else {SEGMENT_RUNTIME.aux_param2++;}
  } else
  {
    a++;
    if (SEGMENT_RUNTIME.counter_mode_call %3 != 1) SEGMENT_RUNTIME.aux_param2--;
  }
  
  if (a > SEGMENT.stop) a = SEGMENT.start;

  if (a + SEGMENT_RUNTIME.aux_param2 <= SEGMENT.stop)
  {
    for(int i = a; i < a+SEGMENT_RUNTIME.aux_param2; i++) {
      setPixelColor(i, SEGMENT.colors[0]);
    }
  } else
  {
    for(int i = a; i <= SEGMENT.stop; i++) {
      setPixelColor(i, SEGMENT.colors[0]);
    }
    for(int i = SEGMENT.start; i < SEGMENT_RUNTIME.aux_param2 - (SEGMENT.stop +1 -a); i++) {
      setPixelColor(i, SEGMENT.colors[0]);
    }
  }
  SEGMENT_RUNTIME.counter_mode_step = a;

  return 3 + ((8 * (uint32_t)(255 - SEGMENT.speed)) / SEGMENT_LENGTH);
}

/*
 * color chase function.
 * color1 = background color
 * color2 and color3 = colors of two adjacent leds
 */
uint16_t WS2812FX::chase(uint32_t color1, uint32_t color2, uint32_t color3) {
  uint16_t a = SEGMENT_RUNTIME.counter_mode_step;
  uint16_t b = (a + 1) % SEGMENT_LENGTH;
  uint16_t c = (b + 1) % SEGMENT_LENGTH;
  setPixelColor(SEGMENT.start + a, color1);
  setPixelColor(SEGMENT.start + b, color2);
  setPixelColor(SEGMENT.start + c, color3);

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
  return SPEED_FORMULA_L;
}

/*
 * Tricolor chase mode
 */
uint16_t WS2812FX::mode_tricolor_chase(void) {
  return chase(SEGMENT.colors[0], SEGMENT.colors[1], SEGMENT.colors[2]);
}

/*
 * Bicolor chase, more primary color.
 */
uint16_t WS2812FX::mode_chase_color(void) {
  return chase(SEGMENT.colors[1], SEGMENT.colors[0], SEGMENT.colors[0]);
}

/*
 * Primary running followed by random color.
 */
uint16_t WS2812FX::mode_chase_random(void) {
  if(SEGMENT_RUNTIME.counter_mode_step == 0) {
    SEGMENT_RUNTIME.aux_param = get_random_wheel_index(SEGMENT_RUNTIME.aux_param);
  }
  return chase(color_wheel(SEGMENT_RUNTIME.aux_param), SEGMENT.colors[0], SEGMENT.colors[0]);
}


/*
 * Primary running on rainbow.
 */
uint16_t WS2812FX::mode_chase_rainbow_white(void) {
  uint16_t n = SEGMENT_RUNTIME.counter_mode_step;
  uint16_t m = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
  uint32_t color2 = color_wheel(((n * 256 / SEGMENT_LENGTH) + (SEGMENT_RUNTIME.counter_mode_call & 0xFF)) & 0xFF);
  uint32_t color3 = color_wheel(((m * 256 / SEGMENT_LENGTH) + (SEGMENT_RUNTIME.counter_mode_call & 0xFF)) & 0xFF);

  return chase(SEGMENT.colors[0], color2, color3);
}


/*
 * Red - Amber - Green - Blue lights running
 */
uint16_t WS2812FX::mode_colorful(void) {
  uint32_t cols[]{0x00FF0000,0x00EEBB00,0x0000EE00,0x000077CC,0x00FF0000,0x00EEBB00,0x0000EE00};
  if (SEGMENT.intensity < 127) //pastel (easter) colors
  {
    cols[0] = 0x00FF8040;
    cols[1] = 0x00E5D241;
    cols[2] = 0x0077FF77;
    cols[3] = 0x0077F0F0;
    for (uint8_t i = 4; i < 7; i++) cols[i] = cols[i-4];
  }
  int i = SEGMENT.start;
  for (i; i <= SEGMENT.stop ; i+=4)
  {
    setPixelColor(i, cols[SEGMENT_RUNTIME.counter_mode_step]);
    setPixelColor(i+1, cols[SEGMENT_RUNTIME.counter_mode_step+1]);
    setPixelColor(i+2, cols[SEGMENT_RUNTIME.counter_mode_step+2]);
    setPixelColor(i+3, cols[SEGMENT_RUNTIME.counter_mode_step+3]);
  }
  i+=4;
  if(i <= SEGMENT.stop)
  {
    setPixelColor(i, cols[SEGMENT_RUNTIME.counter_mode_step]);
    
    if(i+1 <= SEGMENT.stop)
    {
      setPixelColor(i+1, cols[SEGMENT_RUNTIME.counter_mode_step+1]);
      
      if(i+2 <= SEGMENT.stop)
      {
        setPixelColor(i+2, cols[SEGMENT_RUNTIME.counter_mode_step+2]);
      }
    }
  }
  
  if (SEGMENT.speed > 0) SEGMENT_RUNTIME.counter_mode_step++; //static if lowest speed
  if (SEGMENT_RUNTIME.counter_mode_step >3) SEGMENT_RUNTIME.counter_mode_step = 0;
  return 50 + (15 * (uint32_t)(255 - SEGMENT.speed));
}


/*
 * Emulates a traffic light.
 */
uint16_t WS2812FX::mode_traffic_light(void) {
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) setPixelColor(i, SEGMENT.colors[1]);
  uint32_t mdelay = 500;
  for (int i = SEGMENT.start; i < SEGMENT.stop-1 ; i+=3)
  {
    switch (SEGMENT_RUNTIME.counter_mode_step)
    {
      case 0: setPixelColor(i, 0x00FF0000); mdelay = 150 + (100 * (uint32_t)(255 - SEGMENT.speed));break;
      case 1: setPixelColor(i, 0x00FF0000); mdelay = 150 + (20 * (uint32_t)(255 - SEGMENT.speed)); setPixelColor(i+1, 0x00EECC00); break;
      case 2: setPixelColor(i+2, 0x0000FF00); mdelay = 150 + (100 * (uint32_t)(255 - SEGMENT.speed));break;
      case 3: setPixelColor(i+1, 0x00EECC00); mdelay = 150 + (20 * (uint32_t)(255 - SEGMENT.speed));break;
    }
  }

  SEGMENT_RUNTIME.counter_mode_step++;
  if (SEGMENT_RUNTIME.counter_mode_step >3) SEGMENT_RUNTIME.counter_mode_step = 0;
  return mdelay;
}


/*
 * Primary, secondary running on rainbow.
 */
uint16_t WS2812FX::mode_chase_rainbow(void) {
  uint8_t color_sep = 256 / SEGMENT_LENGTH;
  uint8_t color_index = SEGMENT_RUNTIME.counter_mode_call & 0xFF;
  uint32_t color = color_wheel(((SEGMENT_RUNTIME.counter_mode_step * color_sep) + color_index) & 0xFF);

  return chase(color, SEGMENT.colors[0],SEGMENT.colors[1]);
}


/*
 * Sec flashes running on prim.
 */
uint16_t WS2812FX::mode_chase_flash(void) {
  const static uint8_t flash_count = 4;
  uint8_t flash_step = SEGMENT_RUNTIME.counter_mode_call % ((flash_count * 2) + 1);

  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, SEGMENT.colors[0]);
  }

  uint16_t delay = 10 + ((30 * (uint16_t)(255 - SEGMENT.speed)) / SEGMENT_LENGTH);
  if(flash_step < (flash_count * 2)) {
    if(flash_step % 2 == 0) {
      uint16_t n = SEGMENT_RUNTIME.counter_mode_step;
      uint16_t m = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
      setPixelColor(SEGMENT.start + n, SEGMENT.colors[1]);
      setPixelColor(SEGMENT.start + m, SEGMENT.colors[1]);
      delay = 20;
    } else {
      delay = 30;
    }
  } else {
    SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
  }
  return delay;
}


/*
 * Prim flashes running, followed by random color.
 */
uint16_t WS2812FX::mode_chase_flash_random(void) {
  const static uint8_t flash_count = 4;
  uint8_t flash_step = SEGMENT_RUNTIME.counter_mode_call % ((flash_count * 2) + 1);

  for(uint16_t i=0; i < SEGMENT_RUNTIME.counter_mode_step; i++) {
    setPixelColor(SEGMENT.start + i, color_wheel(SEGMENT_RUNTIME.aux_param));
  }

  uint16_t delay = 1 + ((10 * (uint16_t)(255 - SEGMENT.speed)) / SEGMENT_LENGTH);
  if(flash_step < (flash_count * 2)) {
    uint16_t n = SEGMENT_RUNTIME.counter_mode_step;
    uint16_t m = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
    if(flash_step % 2 == 0) {
      setPixelColor(SEGMENT.start + n, SEGMENT.colors[0]);
      setPixelColor(SEGMENT.start + m, SEGMENT.colors[0]);
      delay = 20;
    } else {
      setPixelColor(SEGMENT.start + n, color_wheel(SEGMENT_RUNTIME.aux_param));
      setPixelColor(SEGMENT.start + m, SEGMENT.colors[1]);
      delay = 30;
    }
  } else {
    SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;

    if(SEGMENT_RUNTIME.counter_mode_step == 0) {
      SEGMENT_RUNTIME.aux_param = get_random_wheel_index(SEGMENT_RUNTIME.aux_param);
    }
  }
  return delay;
}


/*
 * Alternating pixels running function.
 */
uint16_t WS2812FX::running(uint32_t color1, uint32_t color2) {
  for(uint16_t i=0; i < SEGMENT_LENGTH; i++) {
    if((i + SEGMENT_RUNTIME.counter_mode_step) % 4 < 2) {
      setPixelColor(SEGMENT.stop - i, color1);
    } else {
      setPixelColor(SEGMENT.stop - i, color2);
    }
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) & 0x3;
  return  35 + ((350 * (uint32_t)(255 - SEGMENT.speed)) / 255);
}

/*
 * Alternating color/white pixels running.
 */
uint16_t WS2812FX::mode_running_color(void) {
  return running(SEGMENT.colors[0], WHITE);
}


/*
 * Alternating red/blue pixels running.
 */
uint16_t WS2812FX::mode_running_red_blue(void) {
  return running(RED, BLUE);
}


/*
 * Alternating red/green pixels running.
 */
uint16_t WS2812FX::mode_merry_christmas(void) {
  return running(RED, GREEN);
}

/*
 * Alternating orange/purple pixels running.
 */
uint16_t WS2812FX::mode_halloween(void) {
  return running(PURPLE, ORANGE);
}


/*
 * Random colored pixels running.
 */
uint16_t WS2812FX::mode_running_random(void) {
  for(uint16_t i=SEGMENT_LENGTH-1; i > 0; i--) {
    setPixelColor(SEGMENT.start + i, getPixelColor(SEGMENT.start + i - 1));
  }

  if(SEGMENT_RUNTIME.counter_mode_step == 0) {
    SEGMENT_RUNTIME.aux_param = get_random_wheel_index(SEGMENT_RUNTIME.aux_param);
    setPixelColor(SEGMENT.start, color_wheel(SEGMENT_RUNTIME.aux_param));
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step == 0) ? 1 : 0;
  return SPEED_FORMULA_L;
}


/*
 * K.I.T.T.
 */
uint16_t WS2812FX::mode_larson_scanner(void) {
  fade_out((255-SEGMENT.intensity) / 32);

  if(SEGMENT_RUNTIME.counter_mode_step < SEGMENT_LENGTH) {
    setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]);
  } else {
    setPixelColor(SEGMENT.start + ((SEGMENT_LENGTH * 2) - SEGMENT_RUNTIME.counter_mode_step) - 2, SEGMENT.colors[0]);
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % ((SEGMENT_LENGTH * 2) - 2);
  return SPEED_FORMULA_L;
}


/*
 * Firing comets from one end.
 */
uint16_t WS2812FX::mode_comet(void) {
  fade_out((255-SEGMENT.intensity) / 32);

  if(IS_REVERSE) {
    setPixelColor(SEGMENT.stop - SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]);
  } else {
    setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]);
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
  return SPEED_FORMULA_L;
}


/*
 * Fireworks function.
 */
uint16_t WS2812FX::fireworks(uint32_t color) {
  uint32_t prevLed, thisLed, nextLed;

  fade_out((255-SEGMENT.intensity) / 32);

  // set brightness(i) = ((brightness(i-1)/4 + brightness(i+1))/4) + brightness(i)
  for(uint16_t i=SEGMENT.start + 1; i <SEGMENT.stop; i++) {
    prevLed = (getPixelColor(i-1) >> 2) & 0x3F3F3F3F;
    thisLed = getPixelColor(i);
    nextLed = (getPixelColor(i+1) >> 2) & 0x3F3F3F3F;
    setPixelColor(i, prevLed + thisLed + nextLed);
  }

  if(!_triggered) {
    for(uint16_t i=0; i<max(1, SEGMENT_LENGTH/20); i++) {
      if(random8(10) == 0) {
        setPixelColor(SEGMENT.start + random(SEGMENT_LENGTH), color);
      }
    }
  } else {
    for(uint16_t i=0; i<max(1, SEGMENT_LENGTH/10); i++) {
      setPixelColor(SEGMENT.start + random(SEGMENT_LENGTH), color);
    }
  }
  return SPEED_FORMULA_L;
}


/*
 * Firework sparks.
 */
uint16_t WS2812FX::mode_fireworks(void) {
  uint32_t color = SEGMENT.colors[0];
  return fireworks(color);
}


/*
 * Random colored firework sparks.
 */
uint16_t WS2812FX::mode_fireworks_random(void) {
  uint32_t color = color_wheel(random8());
  return fireworks(color);
}


/*
 * Fire flicker function
 */
uint16_t WS2812FX::mode_fire_flicker(void) {
  byte w = (SEGMENT.colors[0] >> 24) & 0xFF;
  byte r = (SEGMENT.colors[0] >> 16) & 0xFF;
  byte g = (SEGMENT.colors[0] >>  8) & 0xFF;
  byte b = (SEGMENT.colors[0]        & 0xFF);
  byte lum = max(w, max(r, max(g, b)))/(((256-SEGMENT.intensity)/16)+1);
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    int flicker = random8(lum);
    setPixelColor(i, max(r - flicker, 0), max(g - flicker, 0), max(b - flicker, 0), max(w - flicker, 0));
  }
  return 10 + (2 * (uint16_t)(255 - SEGMENT.speed));
}


/*
 * Gradient run
 */
uint16_t WS2812FX::mode_gradient(void) {
   if (SEGMENT_RUNTIME.counter_mode_call == 0) SEGMENT_RUNTIME.counter_mode_step = 0;
   byte p_w = (SEGMENT.colors[0] & 0xFF000000) >> 24;
   byte p_r = (SEGMENT.colors[0] & 0x00FF0000) >> 16;
   byte p_g = (SEGMENT.colors[0] & 0x0000FF00) >>  8;
   byte p_b = (SEGMENT.colors[0] & 0x000000FF) >>  0;
   byte p_w2 = (SEGMENT.colors[1] & 0xFF000000) >> 24;
   byte p_r2 = (SEGMENT.colors[1] & 0x00FF0000) >> 16;
   byte p_g2 = (SEGMENT.colors[1] & 0x0000FF00) >>  8;
   byte p_b2 = (SEGMENT.colors[1] & 0x000000FF) >>  0;
   byte nw,nr,ng,nb;
   float per,val; //0.0 = sec 1.0 = pri
   float brd = SEGMENT.intensity/2; if (brd <1.0) brd = 1.0;
   int pp = SEGMENT_RUNTIME.counter_mode_step;
   int p1 = pp-SEGMENT_LENGTH;
   int p2 = pp+SEGMENT_LENGTH;

   for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++)
   {
       val = min(abs(pp-i),min(abs(p1-i),abs(p2-i)));
       per = val/brd;
       if (per >1.0) per = 1.0;
       nw = p_w+((p_w2 - p_w)*per);
       nr = p_r+((p_r2 - p_r)*per);
       ng = p_g+((p_g2 - p_g)*per);
       nb = p_b+((p_b2 - p_b)*per);
       setPixelColor(i,nr,ng,nb,nw);
   }

   SEGMENT_RUNTIME.counter_mode_step++;
   if (SEGMENT_RUNTIME.counter_mode_step > SEGMENT.stop) SEGMENT_RUNTIME.counter_mode_step = SEGMENT.start;
   if (SEGMENT.speed == 0) SEGMENT_RUNTIME.counter_mode_step = SEGMENT.start + (SEGMENT_LENGTH >> 1);
   return SPEED_FORMULA_L;
}


/*
 * Gradient run with hard transition
 */
uint16_t WS2812FX::mode_loading(void) {
   if (SEGMENT_RUNTIME.counter_mode_call == 0) SEGMENT_RUNTIME.counter_mode_step = 0;
   byte p_w = (SEGMENT.colors[0] & 0xFF000000) >> 24;
   byte p_r = (SEGMENT.colors[0] & 0x00FF0000) >> 16;
   byte p_g = (SEGMENT.colors[0] & 0x0000FF00) >>  8;
   byte p_b = (SEGMENT.colors[0] & 0x000000FF) >>  0;
   byte p_w2 = (SEGMENT.colors[1] & 0xFF000000) >> 24;
   byte p_r2 = (SEGMENT.colors[1] & 0x00FF0000) >> 16;
   byte p_g2 = (SEGMENT.colors[1] & 0x0000FF00) >>  8;
   byte p_b2 = (SEGMENT.colors[1] & 0x000000FF) >>  0;
   byte nw,nr,ng,nb;
   float per,val; //0.0 = sec 1.0 = pri
   float brd = SEGMENT.intensity; if (brd <1.0) brd = 1.0;
   int pp = SEGMENT_RUNTIME.counter_mode_step;
   int p1 = pp+SEGMENT_LENGTH;

   for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++)
   {
       pp = SEGMENT_RUNTIME.counter_mode_step;
       if (i > pp) pp+=SEGMENT_LENGTH;
       val = abs(pp-i);
       per = val/brd;
       if (per >1.0) per = 1.0;
       nw = p_w+((p_w2 - p_w)*per);
       nr = p_r+((p_r2 - p_r)*per);
       ng = p_g+((p_g2 - p_g)*per);
       nb = p_b+((p_b2 - p_b)*per);
       setPixelColor(i,nr,ng,nb,nw);
   }

   SEGMENT_RUNTIME.counter_mode_step++;
   if (SEGMENT_RUNTIME.counter_mode_step > SEGMENT.stop) SEGMENT_RUNTIME.counter_mode_step = SEGMENT.start;
   if (SEGMENT.speed == 0) SEGMENT_RUNTIME.counter_mode_step = SEGMENT.stop;
   return SPEED_FORMULA_L;
}


/*  
 * Lights all LEDs after each other up starting from the outer edges and  
 * finishing in the middle. Then turns them in reverse order off. Repeat. 
 */ 
uint16_t WS2812FX::mode_dual_color_wipe_in_out(void) {  
  int end = SEGMENT_LENGTH - SEGMENT_RUNTIME.counter_mode_step - 1; 
  bool odd = (SEGMENT_LENGTH % 2) == 1; 
  int mid = odd ? ((SEGMENT_LENGTH / 2) + 1) : (SEGMENT_LENGTH / 2);  
  if (SEGMENT_RUNTIME.counter_mode_step < mid) {  
    setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]); 
    setPixelColor(SEGMENT.start + end, SEGMENT.colors[0]); 
  } else {  
    if (odd) {  
      // If odd, we need to 'double count' the center LED (once to turn it on,  
      // once to turn it off). So trail one behind after the middle LED.  
      setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step - 1, 0); 
      setPixelColor(SEGMENT.start + end + 1, 0); 
    } else {  
      setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step, 0); 
      setPixelColor(SEGMENT.start + end, 0); 
    } 
  } 
   SEGMENT_RUNTIME.counter_mode_step++; 
  if (odd) {  
    if (SEGMENT_RUNTIME.counter_mode_step > SEGMENT_LENGTH) { 
      SEGMENT_RUNTIME.counter_mode_step = 0;  
    } 
  } else {  
    if (SEGMENT_RUNTIME.counter_mode_step >= SEGMENT_LENGTH) {  
      SEGMENT_RUNTIME.counter_mode_step = 0;  
    } 
  } 
  return SPEED_FORMULA_L;
}


 /* 
 * Lights all LEDs after each other up starting from the outer edges and  
 * finishing in the middle. Then turns them in that order off. Repeat.  
 */ 
uint16_t WS2812FX::mode_dual_color_wipe_in_in(void) { 
  bool odd = (SEGMENT_LENGTH % 2) == 1; 
  int mid = SEGMENT_LENGTH / 2; 
  if (odd) {  
    if (SEGMENT_RUNTIME.counter_mode_step <= mid) { 
      setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]); 
      setPixelColor(SEGMENT.start + SEGMENT_LENGTH - SEGMENT_RUNTIME.counter_mode_step - 1, SEGMENT.colors[0]);  
    } else {  
      int i = SEGMENT_RUNTIME.counter_mode_step - mid;  
      setPixelColor(SEGMENT.start + i - 1, 0); 
      setPixelColor(SEGMENT.start + SEGMENT_LENGTH - i, 0);  
    } 
  } else {  
    if (SEGMENT_RUNTIME.counter_mode_step < mid) {  
      setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]); 
      setPixelColor(SEGMENT.start + SEGMENT_LENGTH - SEGMENT_RUNTIME.counter_mode_step - 1, SEGMENT.colors[0]);  
    } else {  
      int i = SEGMENT_RUNTIME.counter_mode_step - mid;  
      setPixelColor(SEGMENT.start + i, 0); 
      setPixelColor(SEGMENT.start + SEGMENT_LENGTH - i - 1, 0);  
    } 
  } 
   SEGMENT_RUNTIME.counter_mode_step++; 
  if (odd) {  
    if (SEGMENT_RUNTIME.counter_mode_step > SEGMENT_LENGTH) { 
      SEGMENT_RUNTIME.counter_mode_step = 0;  
    } 
  } else {  
    if (SEGMENT_RUNTIME.counter_mode_step >= SEGMENT_LENGTH) {  
      SEGMENT_RUNTIME.counter_mode_step = 0;  
    } 
  } 
  return SPEED_FORMULA_L;
}


 /* 
 * Lights all LEDs after each other up starting from the middle and 
 * finishing at the edges. Then turns them off in that order. Repeat. 
 */ 
uint16_t WS2812FX::mode_dual_color_wipe_out_out(void) { 
  int end = SEGMENT_LENGTH - SEGMENT_RUNTIME.counter_mode_step - 1; 
  bool odd = (SEGMENT_LENGTH % 2) == 1; 
  int mid = SEGMENT_LENGTH / 2; 
   if (odd) { 
    if (SEGMENT_RUNTIME.counter_mode_step <= mid) { 
      setPixelColor(SEGMENT.start + mid + SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]); 
      setPixelColor(SEGMENT.start + mid - SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]); 
    } else {  
      setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step - 1, 0); 
      setPixelColor(SEGMENT.start + end + 1, 0); 
    } 
  } else {  
    if (SEGMENT_RUNTIME.counter_mode_step < mid) {  
      setPixelColor(SEGMENT.start + mid - SEGMENT_RUNTIME.counter_mode_step - 1, SEGMENT.colors[0]); 
      setPixelColor(SEGMENT.start + mid + SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]); 
    } else {  
      setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step, 0); 
      setPixelColor(SEGMENT.start + end, 0); 
    } 
  } 
   SEGMENT_RUNTIME.counter_mode_step++; 
  if (odd) {  
    if (SEGMENT_RUNTIME.counter_mode_step > SEGMENT_LENGTH) { 
      SEGMENT_RUNTIME.counter_mode_step = 0;  
    } 
  } else {  
    if (SEGMENT_RUNTIME.counter_mode_step >= SEGMENT_LENGTH) {  
      SEGMENT_RUNTIME.counter_mode_step = 0;  
    } 
  } 
  return SPEED_FORMULA_L;
} 


 /* 
 * Lights all LEDs after each other up starting from the middle and 
 * finishing at the edges. Then turns them off in reverse order. Repeat.  
 */ 
uint16_t WS2812FX::mode_dual_color_wipe_out_in(void) {  
  bool odd = (SEGMENT_LENGTH % 2) == 1; 
  int mid = SEGMENT_LENGTH / 2; 
   if (odd) { 
    if (SEGMENT_RUNTIME.counter_mode_step <= mid) { 
      setPixelColor(SEGMENT.start + mid + SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]); 
      setPixelColor(SEGMENT.start + mid - SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]); 
    } else {  
      int i = SEGMENT_RUNTIME.counter_mode_step - mid;  
      setPixelColor(SEGMENT.start + i - 1, 0); 
      setPixelColor(SEGMENT.start + SEGMENT_LENGTH - i, 0);  
    } 
  } else {  
    if (SEGMENT_RUNTIME.counter_mode_step < mid) {  
      setPixelColor(SEGMENT.start + mid - SEGMENT_RUNTIME.counter_mode_step - 1, SEGMENT.colors[0]); 
      setPixelColor(SEGMENT.start + mid + SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]); 
    } else {  
      int i = SEGMENT_RUNTIME.counter_mode_step - mid;  
      setPixelColor(SEGMENT.start + i, 0); 
      setPixelColor(SEGMENT.start + SEGMENT_LENGTH - i - 1, 0);  
    } 
  } 
   SEGMENT_RUNTIME.counter_mode_step++; 
  if (odd) {  
    if (SEGMENT_RUNTIME.counter_mode_step > SEGMENT_LENGTH) { 
      SEGMENT_RUNTIME.counter_mode_step = 0;  
    } 
  } else {  
    if (SEGMENT_RUNTIME.counter_mode_step >= SEGMENT_LENGTH) {  
      SEGMENT_RUNTIME.counter_mode_step = 0;  
    } 
  } 
  return SPEED_FORMULA_L;
}


/*
 * Alternating white/red/black pixels running.
 */
uint16_t WS2812FX::mode_circus_combustus(void) {
  return chase(RED, WHITE, BLACK);
}


/*
 * ICU mode
 */
uint16_t WS2812FX::mode_icu(void) {
  uint16_t dest = SEGMENT_RUNTIME.counter_mode_step & 0xFFFF;

  for (uint16_t i = SEGMENT.start; i <= SEGMENT.stop; i++)
  {
    setPixelColor(i, SEGMENT.colors[1]);
  }
 
  setPixelColor(SEGMENT.start + dest, SEGMENT.colors[0]);
  setPixelColor(SEGMENT.start + dest + SEGMENT_LENGTH/2, SEGMENT.colors[0]);

  if(SEGMENT_RUNTIME.aux_param == dest) { // pause between eye movements
    if(random8(6) == 0) { // blink once in a while
      setPixelColor(SEGMENT.start + dest, BLACK);
      setPixelColor(SEGMENT.start + dest + SEGMENT_LENGTH/2, BLACK);
      return 200;
    }
    SEGMENT_RUNTIME.aux_param = random(SEGMENT_LENGTH/2);
    return 1000 + random(2000);
  }

  setPixelColor(SEGMENT.start + dest, BLACK);
  setPixelColor(SEGMENT.start + dest + SEGMENT_LENGTH/2, BLACK);

  if(SEGMENT_RUNTIME.aux_param > SEGMENT_RUNTIME.counter_mode_step) {
    SEGMENT_RUNTIME.counter_mode_step++;
    dest++;
  } else if (SEGMENT_RUNTIME.aux_param < SEGMENT_RUNTIME.counter_mode_step) {
    SEGMENT_RUNTIME.counter_mode_step--;
    dest--;
  }

  setPixelColor(SEGMENT.start + dest, SEGMENT.colors[0]);
  setPixelColor(SEGMENT.start + dest + SEGMENT_LENGTH/2, SEGMENT.colors[0]);

  return SPEED_FORMULA_L;
}


/*
 * Custom mode by Aircoookie. Color Wipe, but with 3 colors
 */
uint16_t WS2812FX::mode_tricolor_wipe(void)
{
  if(SEGMENT_RUNTIME.counter_mode_step < SEGMENT_LENGTH) {
    uint32_t led_offset = SEGMENT_RUNTIME.counter_mode_step;
    setPixelColor(SEGMENT.start + led_offset, SEGMENT.colors[0]);
  } else if (SEGMENT_RUNTIME.counter_mode_step < SEGMENT_LENGTH*2) {
    uint32_t led_offset = SEGMENT_RUNTIME.counter_mode_step - SEGMENT_LENGTH;
    setPixelColor(SEGMENT.start + led_offset, SEGMENT.colors[1]);
  } else
  {
    uint32_t led_offset = SEGMENT_RUNTIME.counter_mode_step - SEGMENT_LENGTH*2;
    setPixelColor(SEGMENT.start + led_offset, SEGMENT.colors[2]);
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % (SEGMENT_LENGTH * 3);
  return SPEED_FORMULA_L;
}


/*
 * Fades between 3 colors
 * Custom mode by Keith Lord: https://github.com/kitesurfer1404/WS2812FX/blob/master/src/custom/TriFade.h
 * Modified by Aircoookie
 */
uint16_t WS2812FX::mode_tricolor_fade(void)
{
  static uint32_t color1 = 0, color2 = 0;

  if(SEGMENT_RUNTIME.counter_mode_step == 0) {
    color1 = SEGMENT.colors[0];
    color2 = SEGMENT.colors[1];
  } else if(SEGMENT_RUNTIME.counter_mode_step == 256) {
    color1 = SEGMENT.colors[1];
    color2 = SEGMENT.colors[2];
  } else if(SEGMENT_RUNTIME.counter_mode_step == 512) {
    color1 = SEGMENT.colors[2];
    color2 = SEGMENT.colors[0];
  }

  uint32_t color = color_blend(color1, color2, SEGMENT_RUNTIME.counter_mode_step % 256);
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, color);
  }

  SEGMENT_RUNTIME.counter_mode_step += 4;
  if(SEGMENT_RUNTIME.counter_mode_step >= 768) SEGMENT_RUNTIME.counter_mode_step = 0;

  return 5 + ((uint32_t)(255 - SEGMENT.speed) / 10);
}


/*
 * Creates random comets
 * Custom mode by Keith Lord: https://github.com/kitesurfer1404/WS2812FX/blob/master/src/custom/MultiComet.h
 */
uint16_t WS2812FX::mode_multi_comet(void)
{
  fade_out((255-SEGMENT.intensity) / 32);

  static uint16_t comets[] = {UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX};

  for(uint8_t i=0; i < 6; i++) {
    if(comets[i] < SEGMENT_LENGTH) {
      if (SEGMENT.colors[2] != SEGMENT.colors[1])
      {
        setPixelColor(SEGMENT.start + comets[i], i % 2 ? SEGMENT.colors[0] : SEGMENT.colors[2]);
      } else
      {
        setPixelColor(SEGMENT.start + comets[i], SEGMENT.colors[0]);
      }
      comets[i]++;
    } else {
      if(!random(SEGMENT_LENGTH)) {
        comets[i] = 0;
      }
    }
  }
  return SPEED_FORMULA_L;
}


/*
 * Creates two Larson scanners moving in opposite directions
 * Custom mode by Keith Lord: https://github.com/kitesurfer1404/WS2812FX/blob/master/src/custom/DualLarson.h
 */
uint16_t WS2812FX::mode_dual_larson_scanner(void){
  if (SEGMENT_RUNTIME.aux_param)
  {
    SEGMENT_RUNTIME.counter_mode_step--;
  } else
  {
    SEGMENT_RUNTIME.counter_mode_step++;
  }

  fade_out((255-SEGMENT.intensity) / 32);

  setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]);
  if (SEGMENT.colors[2] != SEGMENT.colors[1])
  {
    setPixelColor(SEGMENT.stop - SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[2]);
  } else
  {
    setPixelColor(SEGMENT.stop - SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]);
  }

  if(SEGMENT_RUNTIME.counter_mode_step >= (SEGMENT.stop - SEGMENT.start) || SEGMENT_RUNTIME.counter_mode_step <= 0)
  SEGMENT_RUNTIME.aux_param = !SEGMENT_RUNTIME.aux_param;
  
  return SPEED_FORMULA_L;
}


/*
 * Running random pixels
 * Custom mode by Keith Lord: https://github.com/kitesurfer1404/WS2812FX/blob/master/src/custom/RandomChase.h
 */
uint16_t WS2812FX::mode_random_chase(void)
{
  for(uint16_t i=SEGMENT.stop; i>SEGMENT.start; i--) {
    setPixelColor(i, getPixelColor(i-1));
  }
  uint32_t color = getPixelColor(SEGMENT.start + 1);
  int r = random(6) != 0 ? (color >> 16 & 0xFF) : random(256);
  int g = random(6) != 0 ? (color >> 8  & 0xFF) : random(256);
  int b = random(6) != 0 ? (color       & 0xFF) : random(256);
  setPixelColor(SEGMENT.start, r, g, b);

  return 15 + (15 * (uint32_t)(255 - SEGMENT.speed));
}

typedef struct Oscillator {
  int16_t pos;
  int8_t  size;
  int8_t  dir;
  int8_t  speed;
} oscillator;

uint16_t WS2812FX::mode_oscillate(void)
{
  static oscillator oscillators[NUM_COLORS] = {
    {SEGMENT_LENGTH/4,   SEGMENT_LENGTH/8,  1, 1},
    {SEGMENT_LENGTH/4*2, SEGMENT_LENGTH/8, -1, 1},
    {SEGMENT_LENGTH/4*3, SEGMENT_LENGTH/8,  1, 2}
  };

  for(int8_t i=0; i < sizeof(oscillators)/sizeof(oscillators[0]); i++) {
    oscillators[i].pos += oscillators[i].dir * oscillators[i].speed;
    if((oscillators[i].dir == -1) && (oscillators[i].pos <= 0)) {
      oscillators[i].pos = 0;
      oscillators[i].dir = 1;
      oscillators[i].speed = random(1, 3);
    }
    if((oscillators[i].dir == 1) && (oscillators[i].pos >= (SEGMENT_LENGTH - 1))) {
      oscillators[i].pos = SEGMENT_LENGTH - 1;
      oscillators[i].dir = -1;
      oscillators[i].speed = random(1, 3);
    }
  }

  for(int16_t i=0; i < SEGMENT_LENGTH; i++) {
    uint32_t color = BLACK;
    for(int8_t j=0; j < sizeof(oscillators)/sizeof(oscillators[0]); j++) {
      if(i >= oscillators[j].pos - oscillators[j].size && i <= oscillators[j].pos + oscillators[j].size) {
        color = (color == BLACK) ? SEGMENT.colors[j] : color_blend(color, SEGMENT.colors[j], 128);
      }
    }
    setPixelColor(SEGMENT.start + i, color);
  }
  return 15 + (uint32_t)(255 - SEGMENT.speed);
}


uint16_t WS2812FX::mode_lightning(void)
{
  uint16_t ledstart = SEGMENT.start + random8(SEGMENT_LENGTH);                               // Determine starting location of flash
  uint16_t ledlen = random8(SEGMENT.stop - ledstart);                      // Determine length of flash (not to go beyond NUM_LEDS-1)
  uint8_t bri = 255/random8(1, 3);   

  if (SEGMENT_RUNTIME.counter_mode_step == 0)
  {
    SEGMENT_RUNTIME.aux_param = random8(3, 3 + SEGMENT.intensity/20); //number of flashes
    bri = 52; 
    SEGMENT_RUNTIME.aux_param2 = 1;
  }

  for (int i = SEGMENT.start; i <= SEGMENT.stop; i++)
  {
    setPixelColor(i,SEGMENT.colors[1]);
  }
  
  if (SEGMENT_RUNTIME.aux_param2) {
    for (int i = ledstart; i < ledstart + ledlen; i++)
    {
      setPixelColor(i,bri,bri,bri,bri);
    }
    SEGMENT_RUNTIME.aux_param2 = 0;
    SEGMENT_RUNTIME.counter_mode_step++;
    return random8(4, 10);                                    // each flash only lasts 4-10 milliseconds
  }

  SEGMENT_RUNTIME.aux_param2 = 1;
  if (SEGMENT_RUNTIME.counter_mode_step == 1) return (200);                       // longer delay until next flash after the leader

  if (SEGMENT_RUNTIME.counter_mode_step <= SEGMENT_RUNTIME.aux_param) return (50 + random8(100));  // shorter delay between strokes

  SEGMENT_RUNTIME.counter_mode_step = 0;
  return (random8(255 - SEGMENT.speed) * 100);                            // delay between strikes
}


// WLED limitation: Analog Clock overlay will NOT work when Fire2012 is active
// Fire2012 by Mark Kriegsman, July 2012
// as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
//// 
// This basic one-dimensional 'fire' simulation works roughly as follows:
// There's a underlying array of 'heat' cells, that model the temperature
// at each point along the line.  Every cycle through the simulation, 
// four steps are performed:
//  1) All cells cool down a little bit, losing heat to the air
//  2) The heat from each cell drifts 'up' and diffuses a little
//  3) Sometimes randomly new 'sparks' of heat are added at the bottom
//  4) The heat from each cell is rendered as a color into the leds array
//     The heat-to-color mapping uses a black-body radiation approximation.
//
// Temperature is in arbitrary units from 0 (cold black) to 255 (white hot).
//
// This simulation scales it self a bit depending on NUM_LEDS; it should look
// "OK" on anywhere from 20 to 100 LEDs without too much tweaking. 
//
// I recommend running this simulation at anywhere from 30-100 frames per second,
// meaning an interframe delay of about 10-35 milliseconds.
//
// Looks best on a high-density LED setup (60+ pixels/meter).
//
//
// There are two main parameters you can play with to control the look and
// feel of your fire: COOLING (used in step 1 above), and SPARKING (used
// in step 3 above) (Effect Intensity = Sparking).
//
// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100 
#define COOLING  75

uint16_t WS2812FX::mode_fire_2012(void)
{
  // Step 1.  Cool down every cell a little
  for( int i = SEGMENT.start; i <= SEGMENT.stop; i++) {
    _locked[i] = qsub8(_locked[i],  random8(0, ((COOLING * 10) / SEGMENT_LENGTH) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for( int k= SEGMENT.stop; k >= SEGMENT.start + 2; k--) {
    _locked[k] = (_locked[k - 1] + _locked[k - 2] + _locked[k - 2] ) / 3;
  }
  
  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if( random8() <= SEGMENT.intensity ) {
    int y = SEGMENT.start + random8(7);
    if (y <= SEGMENT.stop) _locked[y] = qadd8(_locked[y], random8(160,255) );
  }

  // Step 4.  Map from heat cells to LED colors
  for( int j = SEGMENT.start; j <= SEGMENT.stop; j++) {
    CRGB color = HeatColor(_locked[j]);
    setPixelColor(j, color.red, color.green, color.blue);
  }
  return 10 + (uint16_t)(255 - SEGMENT.speed)/6;
}


// Pride2015
// Animated, ever-changing rainbows.
// by Mark Kriegsman: https://gist.github.com/kriegsman/964de772d64c502760e5
uint16_t WS2812FX::mode_pride_2015(void)
{
  uint16_t duration = 10 + SEGMENT.speed;
  uint16_t sPseudotime = SEGMENT_RUNTIME.counter_mode_step;
  uint16_t sHue16 = SEGMENT_RUNTIME.aux_param;
 
  uint8_t sat8 = beatsin88( 87, 220, 250);
  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 1, 3000);
  
  sPseudotime += duration * msmultiplier;
  sHue16 += duration * beatsin88( 400, 5,9);
  uint16_t brightnesstheta16 = sPseudotime;
  CRGB fastled_col;
  
  for( uint16_t i = SEGMENT.start ; i <= SEGMENT.stop; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 >> 8;

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);
    
    CRGB newcolor = CHSV( hue8, sat8, bri8);

    uint32_t color = getPixelColor(i);
    fastled_col.red = (color >> 16 & 0xFF);
    fastled_col.green = (color >> 8  & 0xFF);
    fastled_col.blue = (color       & 0xFF);
    
    nblend( fastled_col, newcolor, 64);
    setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
  }
  SEGMENT_RUNTIME.counter_mode_step = sPseudotime;
  SEGMENT_RUNTIME.aux_param = sHue16;
  return 20;
}


// eight colored dots, weaving in and out of sync with each other
uint16_t WS2812FX::mode_juggle(void){
  fade_out((255-SEGMENT.intensity) / 32);
  CRGB fastled_col;
  byte dothue = 0;
  for ( byte i = 0; i < 8; i++) {
    uint16_t index = SEGMENT.start + beatsin16(i + 7, 0, SEGMENT_LENGTH -1);
    uint32_t color = getPixelColor(index);
    fastled_col.red = (color >> 16 & 0xFF);
    fastled_col.green = (color >> 8  & 0xFF);
    fastled_col.blue = (color       & 0xFF);
    fastled_col |= CHSV(dothue, 220, 255);
    setPixelColor(index, fastled_col.red, fastled_col.green, fastled_col.blue);
    dothue += 32;
  }
  return 10 + (uint16_t)(255 - SEGMENT.speed)/4;
}


/*
 * FastLED palette modes helper function. Limitation: Due to memory reasons, multiple active segments with FastLED will disable the Palette transitions
 */
CRGBPalette16 currentPalette(CRGB::Black);
CRGBPalette16 targetPalette(CloudColors_p);

void WS2812FX::handle_palette(void)
{
  bool singleSegmentMode = (_segment_index == _segment_index_palette_last);
  _segment_index_palette_last = _segment_index;
  
  switch (SEGMENT.palette)
  {
    case 0: {//periodically replace palette with a random one. Doesn't work with multiple FastLED segments
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
    case 1: {//primary color only
      CRGB prim;
      prim.red   = (SEGMENT.colors[0] >> 16 & 0xFF);
      prim.green = (SEGMENT.colors[0] >> 8  & 0xFF);
      prim.blue  = (SEGMENT.colors[0]       & 0xFF);
      targetPalette = CRGBPalette16(prim); break;}
    case 2: {//based on primary
      //considering performance implications
      CRGB prim;
      prim.red   = (SEGMENT.colors[0] >> 16 & 0xFF);
      prim.green = (SEGMENT.colors[0] >> 8  & 0xFF);
      prim.blue  = (SEGMENT.colors[0]       & 0xFF);
      CHSV prim_hsv = rgb2hsv_approximate(prim);
      targetPalette = CRGBPalette16(
                      CHSV(prim_hsv.h, prim_hsv.s, prim_hsv.v), //color itself
                      CHSV(prim_hsv.h, max(prim_hsv.s - 50,0), prim_hsv.v), //less saturated
                      CHSV(prim_hsv.h, prim_hsv.s, max(prim_hsv.h - 50,0)), //darker
                      CHSV(prim_hsv.h, prim_hsv.s, prim_hsv.v)); //color itself
      break;}
    case 3: {//primary + secondary
      CRGB prim;
      prim.red   = (SEGMENT.colors[0] >> 16 & 0xFF);
      prim.green = (SEGMENT.colors[0] >> 8  & 0xFF);
      prim.blue  = (SEGMENT.colors[0]       & 0xFF);
      CRGB sec;
      sec.red    = (SEGMENT.colors[1] >> 16 & 0xFF);
      sec.green  = (SEGMENT.colors[1] >> 8  & 0xFF);
      sec.blue   = (SEGMENT.colors[1]       & 0xFF);
      targetPalette = CRGBPalette16(prim,sec,prim); break;}
    case 4: {//based on primary + secondary
      CRGB prim;
      prim.red   = (SEGMENT.colors[0] >> 16 & 0xFF);
      prim.green = (SEGMENT.colors[0] >> 8  & 0xFF);
      prim.blue  = (SEGMENT.colors[0]       & 0xFF);
      CRGB sec;
      sec.red    = (SEGMENT.colors[1] >> 16 & 0xFF);
      sec.green  = (SEGMENT.colors[1] >> 8  & 0xFF);
      sec.blue   = (SEGMENT.colors[1]       & 0xFF);
      CHSV prim_hsv = rgb2hsv_approximate(prim);
      CHSV sec_hsv  = rgb2hsv_approximate(sec );
      targetPalette = CRGBPalette16(
                      CHSV(prim_hsv.h, prim_hsv.s, prim_hsv.v), //color itself
                      CHSV(prim_hsv.h, max(prim_hsv.s - 50,0), prim_hsv.v), //less saturated
                      CHSV(sec_hsv.h, sec_hsv.s, max(sec_hsv.v - 50,0)), //darker
                      CHSV(sec_hsv.h, sec_hsv.s, sec_hsv.v)); //color itself
      break;}
    case 5: //Party colors
      targetPalette = PartyColors_p; break;
    case 6: //Cloud colors
      targetPalette = CloudColors_p; break;
    case 7: //Lava colors
      targetPalette = LavaColors_p; break;
    case 8: //Ocean colors
      targetPalette = OceanColors_p; break;
    case 9: //Forest colors
      targetPalette = ForestColors_p; break;
    case 10: //Rainbow colors
      targetPalette = RainbowColors_p; break;
    case 11: //Rainbow stripe colors
      targetPalette = RainbowStripeColors_p; break;
    default: //progmem palettes
      targetPalette = gGradientPalettes[constrain(SEGMENT.palette -12, 0, gGradientPaletteCount -1)];
  }
  
  if (singleSegmentMode && paletteFade) //only blend if just one segment uses FastLED mode
  {
    nblendPaletteTowardPalette(currentPalette, targetPalette, 42);
  } else
  {
    currentPalette = targetPalette;
  }
}


uint16_t WS2812FX::mode_palette(void)
{
  handle_palette();
  CRGB fastled_col;
  for (uint16_t i = SEGMENT.start; i <= SEGMENT.stop; i++)
  {
    uint8_t colorIndex = map(i,SEGMENT.start,SEGMENT.stop,0,255) + (SEGMENT_RUNTIME.counter_mode_step >> 6 & 0xFF);
    fastled_col = ColorFromPalette( currentPalette, colorIndex, 255, LINEARBLEND);
    setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
  }
  SEGMENT_RUNTIME.counter_mode_step += SEGMENT.speed;
  if (SEGMENT.speed == 0) SEGMENT_RUNTIME.counter_mode_step = 0;
  return 20;
}


// ColorWavesWithPalettes by Mark Kriegsman: https://gist.github.com/kriegsman/8281905786e8b2632aeb
// This function draws color waves with an ever-changing,
// widely-varying set of parameters, using a color palette.
uint16_t WS2812FX::mode_colorwaves(void)
{
  handle_palette();
  uint16_t duration = 10 + SEGMENT.speed;
  uint16_t sPseudotime = SEGMENT_RUNTIME.counter_mode_step;
  uint16_t sHue16 = SEGMENT_RUNTIME.aux_param;

  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 300, 1500);

  sPseudotime += duration * msmultiplier;
  sHue16 += duration * beatsin88( 400, 5, 9);
  uint16_t brightnesstheta16 = sPseudotime;
  CRGB fastled_col;

  for ( uint16_t i = SEGMENT.start ; i <= SEGMENT.stop; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;
    uint16_t h16_128 = hue16 >> 7;
    if ( h16_128 & 0x100) {
      hue8 = 255 - (h16_128 >> 1);
    } else {
      hue8 = h16_128 >> 1;
    }

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);

    uint8_t index = hue8;
    //index = triwave8( index);
    index = scale8( index, 240);

    CRGB newcolor = ColorFromPalette(currentPalette, index, bri8);

    uint32_t color = getPixelColor(i);
    fastled_col.red = (color >> 16 & 0xFF);
    fastled_col.green = (color >> 8  & 0xFF);
    fastled_col.blue = (color       & 0xFF);

    nblend(fastled_col, newcolor, 128);
    setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
  }
  SEGMENT_RUNTIME.counter_mode_step = sPseudotime;
  SEGMENT_RUNTIME.aux_param = sHue16;
  return 20;
}


// colored stripes pulsing at a defined Beats-Per-Minute (BPM)
uint16_t WS2812FX::mode_bpm(void)
{
  handle_palette();
  CRGB fastled_col;
  uint8_t beat = beatsin8(SEGMENT.speed, 64, 255);
  for ( int i = SEGMENT.start; i <= SEGMENT.stop; i++) {
    fastled_col = ColorFromPalette(currentPalette, SEGMENT_RUNTIME.counter_mode_step + (i * 2), beat - SEGMENT_RUNTIME.counter_mode_step + (i * 10));
    setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
  }
  SEGMENT_RUNTIME.counter_mode_step++;
  if (SEGMENT_RUNTIME.counter_mode_step >= 255) SEGMENT_RUNTIME.counter_mode_step = 0;
  return 20;
}


uint16_t WS2812FX::mode_fillnoise8(void)
{
  if (SEGMENT_RUNTIME.counter_mode_call == 0) SEGMENT_RUNTIME.counter_mode_step = random(12345);
  handle_palette();
  CRGB fastled_col;
  for (int i = SEGMENT.start; i <= SEGMENT.stop; i++) {
    uint8_t index = inoise8(i * SEGMENT_LENGTH, SEGMENT_RUNTIME.counter_mode_step + i * SEGMENT_LENGTH) % 255;
    fastled_col = ColorFromPalette(currentPalette, index, 255, LINEARBLEND);
    setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
  }
  SEGMENT_RUNTIME.counter_mode_step += beatsin8(SEGMENT.speed, 1, 4); //10,1,4

  return 20;
}

uint16_t WS2812FX::mode_noise16_1(void)
{
  uint16_t scale = 320;                                      // the "zoom factor" for the noise
  handle_palette();
  CRGB fastled_col;
  SEGMENT_RUNTIME.counter_mode_step += (1 + SEGMENT.speed/16);

  for (int i = SEGMENT.start; i <= SEGMENT.stop; i++) {

    uint16_t shift_x = beatsin8(11);                           // the x position of the noise field swings @ 17 bpm
    uint16_t shift_y = SEGMENT_RUNTIME.counter_mode_step/42;             // the y position becomes slowly incremented


    uint16_t real_x = (i + shift_x) * scale;                  // the x position of the noise field swings @ 17 bpm
    uint16_t real_y = (i + shift_y) * scale;                  // the y position becomes slowly incremented
    uint32_t real_z = SEGMENT_RUNTIME.counter_mode_step;                          // the z position becomes quickly incremented

    uint8_t noise = inoise16(real_x, real_y, real_z) >> 8;   // get the noise data and scale it down

    uint8_t index = sin8(noise * 3);                         // map LED color based on noise data

    fastled_col = ColorFromPalette(currentPalette, index, 255, LINEARBLEND);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.
    setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
  }

  return 20;
}


uint16_t WS2812FX::mode_noise16_2(void)
{
  uint16_t scale = 1000;                                       // the "zoom factor" for the noise
  handle_palette();
  CRGB fastled_col;
  SEGMENT_RUNTIME.counter_mode_step += (1 + SEGMENT.speed);

  for (int i = SEGMENT.start; i <= SEGMENT.stop; i++) {

    uint16_t shift_x = SEGMENT_RUNTIME.counter_mode_step >> 6;                         // x as a function of time
    uint16_t shift_y = SEGMENT_RUNTIME.counter_mode_step/42;

    uint32_t real_x = (i + shift_x) * scale;                  // calculate the coordinates within the noise field
    uint32_t real_y = (i + shift_y) * scale;                  // based on the precalculated positions
    uint32_t real_z = 4223;

    uint8_t noise = inoise16(real_x, 0, 4223) >> 8;    // get the noise data and scale it down

    uint8_t index = sin8(noise * 3);                          // map led color based on noise data

    fastled_col = ColorFromPalette(currentPalette, index, noise, LINEARBLEND);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.
    setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
  }

  return 20;
}


uint16_t WS2812FX::mode_noise16_3(void)
{
  uint16_t scale = 800;                                       // the "zoom factor" for the noise
  handle_palette();
  CRGB fastled_col;
  SEGMENT_RUNTIME.counter_mode_step += (1 + SEGMENT.speed);

  for (int i = SEGMENT.start; i <= SEGMENT.stop; i++) {

    uint16_t shift_x = 4223;                                  // no movement along x and y
    uint16_t shift_y = 1234;

    uint32_t real_x = (i + shift_x) * scale;                  // calculate the coordinates within the noise field
    uint32_t real_y = (i + shift_y) * scale;                  // based on the precalculated positions
    uint32_t real_z = SEGMENT_RUNTIME.counter_mode_step*8;  

    uint8_t noise = inoise16(real_x, real_y, real_z) >> 8;    // get the noise data and scale it down

    uint8_t index = sin8(noise * 3);                          // map led color based on noise data

    fastled_col = ColorFromPalette(currentPalette, index, noise, LINEARBLEND);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.
    setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
  }

  return 20;
}


//https://github.com/aykevl/ledstrip-spark/blob/master/ledstrip.ino
uint16_t WS2812FX::mode_noise16_4(void)
{
  handle_palette();
  CRGB fastled_col;
  SEGMENT_RUNTIME.counter_mode_step += SEGMENT.speed;
  for (int i = SEGMENT.start; i <= SEGMENT.stop; i++) {
    int16_t index = inoise16(uint32_t(i - SEGMENT.start) << 12, SEGMENT_RUNTIME.counter_mode_step/8);
    fastled_col = ColorFromPalette(currentPalette, index);
    setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
  }
  return 20;
}


