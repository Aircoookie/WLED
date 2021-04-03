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

/*
  Custom per-LED mapping has moved!

  Create a file "ledmap.json" using the edit page.

  this is just an example (30 LEDs). It will first set all even, then all uneven LEDs.
  {"map":[
  0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28,
  1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29]}

  another example. Switches direction every 5 LEDs.
  {"map":[
  0, 1, 2, 3, 4, 9, 8, 7, 6, 5, 10, 11, 12, 13, 14,
  19, 18, 17, 16, 15, 20, 21, 22, 23, 24, 29, 28, 27, 26, 25]
*/

//do not call this method from system context (network callback)
void WS2812FX::finalizeInit(uint16_t countPixels, bool skipFirst)
{
  RESET_RUNTIME;
  _length = countPixels;
  _skipFirstMode = skipFirst;

  _lengthRaw = _length;
  if (_skipFirstMode) {
    _lengthRaw += LED_SKIP_AMOUNT;
  }

  //if busses failed to load, add default (FS issue...)
  if (busses.getNumBusses() == 0) {
    uint8_t defPin[] = {LEDPIN};
    BusConfig defCfg = BusConfig(TYPE_WS2812_RGB, defPin, 0, _lengthRaw, COL_ORDER_GRB);
    busses.add(defCfg);
  }
  
  deserializeMap();

  //make segment 0 cover the entire strip
  _segments[0].start = 0;
  _segments[0].stop = _length;

  setBrightness(_brightness);

  #ifdef ESP8266
  for (uint8_t i = 0; i < busses.getNumBusses(); i++) {
    Bus* b = busses.getBus(i);
    if ((!IS_DIGITAL(b->getType()) || IS_2PIN(b->getType()))) continue;
    uint8_t pins[5];
    b->getPins(pins);
    BusDigital* bd = static_cast<BusDigital*>(b);
    if (pins[0] == 3) bd->reinit();
  }
  #endif
}

void WS2812FX::service() {
  uint32_t nowUp = millis(); // Be aware, millis() rolls over every 49 days
  now = nowUp + timebase;
  if (nowUp - _lastShow < MIN_SHOW_DELAY) return;
  bool doShow = false;

  for(uint8_t i=0; i < MAX_NUM_SEGMENTS; i++)
  {
    _segment_index = i;

    // reset the segment runtime data if needed, called before isActive to ensure deleted
    // segment's buffers are cleared
    SEGENV.resetIfRequired();

    if (!SEGMENT.isActive()) continue;

    if(nowUp > SEGENV.next_time || _triggered || (doShow && SEGMENT.mode == 0)) //last is temporary
    {
      if (SEGMENT.grouping == 0) SEGMENT.grouping = 1; //sanity check
      doShow = true;
      uint16_t delay = FRAMETIME;

      if (!SEGMENT.getOption(SEG_OPTION_FREEZE)) { //only run effect function if not frozen
        _virtualSegmentLength = SEGMENT.virtualLength();
        _bri_t = SEGMENT.opacity; _colors_t[0] = SEGMENT.colors[0]; _colors_t[1] = SEGMENT.colors[1]; _colors_t[2] = SEGMENT.colors[2];
        if (!IS_SEGMENT_ON) _bri_t = 0;
        for (uint8_t t = 0; t < MAX_NUM_TRANSITIONS; t++) {
          if ((transitions[t].segment & 0x3F) != i) continue;
          uint8_t slot = transitions[t].segment >> 6;
          if (slot == 0) _bri_t = transitions[t].currentBri();
          _colors_t[slot] = transitions[t].currentColor(SEGMENT.colors[slot]);
        }
        for (uint8_t c = 0; c < 3; c++) _colors_t[c] = gamma32(_colors_t[c]);
        handle_palette();
        delay = (this->*_mode[SEGMENT.mode])(); //effect function
        if (SEGMENT.mode != FX_MODE_HALLOWEEN_EYES) SEGENV.call++;
      }

      SEGENV.next_time = nowUp + delay;
    }
  }
  _virtualSegmentLength = 0;
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

//used to map from segment index to physical pixel, taking into account grouping, offsets, reverse and mirroring
uint16_t WS2812FX::realPixelIndex(uint16_t i) {
  int16_t iGroup = i * SEGMENT.groupLength();

  /* reverse just an individual segment */
  int16_t realIndex = iGroup;
  if (IS_REVERSE) {
    if (IS_MIRROR) {
      realIndex = (SEGMENT.length() -1) / 2 - iGroup;  //only need to index half the pixels
    } else {
      realIndex = SEGMENT.length() - iGroup - 1;
    }
  }

  realIndex += SEGMENT.start;

  return realIndex;
}

void WS2812FX::setPixelColor(uint16_t i, byte r, byte g, byte b, byte w)
{
  //auto calculate white channel value if enabled
  if (isRgbw) {
    if (rgbwMode == RGBW_MODE_AUTO_BRIGHTER || (w == 0 && (rgbwMode == RGBW_MODE_DUAL || rgbwMode == RGBW_MODE_LEGACY)))
    {
      //white value is set to lowest RGB channel
      //thank you to @Def3nder!
      w = r < g ? (r < b ? r : b) : (g < b ? g : b);
    } else if (rgbwMode == RGBW_MODE_AUTO_ACCURATE && w == 0)
    {
      w = r < g ? (r < b ? r : b) : (g < b ? g : b);
      r -= w; g -= w; b -= w;
    }
  }
  
  uint16_t skip = _skipFirstMode ? LED_SKIP_AMOUNT : 0;
  if (SEGLEN) {//from segment

    //color_blend(getpixel, col, _bri_t); (pseudocode for future blending of segments)
    if (_bri_t < 255) {  
      r = scale8(r, _bri_t);
      g = scale8(g, _bri_t);
      b = scale8(b, _bri_t);
      w = scale8(w, _bri_t);
    }
    uint32_t col = ((w << 24) | (r << 16) | (g << 8) | (b));

    /* Set all the pixels in the group, ensuring _skipFirstMode is honored */
    bool reversed = IS_REVERSE;
    uint16_t realIndex = realPixelIndex(i);

    for (uint16_t j = 0; j < SEGMENT.grouping; j++) {
      int16_t indexSet = realIndex + (reversed ? -j : j);
      if (indexSet < customMappingSize) indexSet = customMappingTable[indexSet];
      if (indexSet >= SEGMENT.start && indexSet < SEGMENT.stop) {
        busses.setPixelColor(indexSet + skip, col);
        if (IS_MIRROR) { //set the corresponding mirrored pixel
          uint16_t indexMir = SEGMENT.stop - indexSet + SEGMENT.start - 1;
          if (indexMir < customMappingSize) indexMir = customMappingTable[indexMir];
          busses.setPixelColor(indexMir + skip, col);
        }
      }
    }
  } else { //live data, etc.
    if (i < customMappingSize) i = customMappingTable[i];
    
    uint32_t col = ((w << 24) | (r << 16) | (g << 8) | (b));
    busses.setPixelColor(i + skip, col);
  }
  if (skip && i == 0) {
    for (uint16_t j = 0; j < skip; j++) {
      busses.setPixelColor(j, BLACK);
    }
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

  // avoid race condition, caputre _callback value
  show_callback callback = _callback;
  if (callback) callback();

  //power limit calculation
  //each LED can draw up 195075 "power units" (approx. 53mA)
  //one PU is the power it takes to have 1 channel 1 step brighter per brightness step
  //so A=2,R=255,G=0,B=0 would use 510 PU per LED (1mA is about 3700 PU)
  bool useWackyWS2815PowerModel = false;
  byte actualMilliampsPerLed = milliampsPerLed;

  if(milliampsPerLed == 255) {
    useWackyWS2815PowerModel = true;
    actualMilliampsPerLed = 12; // from testing an actual strip
  }

  if (ablMilliampsMax > 149 && actualMilliampsPerLed > 0) //0 mA per LED and too low numbers turn off calculation
  {
    uint32_t puPerMilliamp = 195075 / actualMilliampsPerLed;
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
      uint32_t c = busses.getPixelColor(i);
      byte r = c >> 16, g = c >> 8, b = c, w = c >> 24;

      if(useWackyWS2815PowerModel)
      {
        // ignore white component on WS2815 power calculation
        powerSum += (MAX(MAX(r,g),b)) * 3;
      }
      else 
      {
        powerSum += (r + g + b + w);
      }
    }


    if (isRgbw) //RGBW led total output with white LEDs enabled is still 50mA, so each channel uses less
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
      busses.setBrightness(newBri);
      currentMilliamps = (powerSum0 * newBri) / puPerMilliamp;
    } else
    {
      currentMilliamps = powerSum / puPerMilliamp;
      busses.setBrightness(_brightness);
    }
    currentMilliamps += MA_FOR_ESP; //add power of ESP back to estimate
    currentMilliamps += _length; //add standby power back to estimate
  } else {
    currentMilliamps = 0;
    busses.setBrightness(_brightness);
  }
  
  // some buses send asynchronously and this method will return before
  // all of the data has been sent.
  // See https://github.com/Makuna/NeoPixelBus/wiki/ESP32-NeoMethods#neoesp32rmt-methods
  busses.show();
  unsigned long now = millis();
  unsigned long diff = now - _lastShow;
  uint16_t fpsCurr = 200;
  if (diff > 0) fpsCurr = 1000 / diff;
  _cumulativeFps = (3 * _cumulativeFps + fpsCurr) >> 2;
  _lastShow = now;
}

/**
 * Returns a true value if any of the strips are still being updated.
 * On some hardware (ESP32), strip updates are done asynchronously.
 */
bool WS2812FX::isUpdating() {
  return !busses.canAllShow();
}

/**
 * Returns the refresh rate of the LED strip. Useful for finding out whether a given setup is fast enough.
 * Only updates on show() or is set to 0 fps if last show is more than 2 secs ago, so accurary varies
 */
uint16_t WS2812FX::getFps() {
  if (millis() - _lastShow > 2000) return 0;
  return _cumulativeFps +1;
}

/**
 * Forces the next frame to be computed on all active segments.
 */
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
  return 13 + GRADIENT_PALETTE_COUNT;
}

//TODO effect transitions


bool WS2812FX::setEffectConfig(uint8_t m, uint8_t s, uint8_t in, uint8_t p) {
  Segment& seg = _segments[getMainSegmentId()];
  uint8_t modePrev = seg.mode, speedPrev = seg.speed, intensityPrev = seg.intensity, palettePrev = seg.palette;

  bool applied = false;
  
  if (applyToAllSelected) {
    for (uint8_t i = 0; i < MAX_NUM_SEGMENTS; i++)
    {
      if (_segments[i].isSelected())
      {
        _segments[i].speed = s;
        _segments[i].intensity = in;
        _segments[i].palette = p;
        setMode(i, m);
        applied = true;
      }
    }
  } 
  
  if (!applyToAllSelected || !applied) {
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

  bool applied = false;
  
  if (applyToAllSelected) {
    for (uint8_t i = 0; i < MAX_NUM_SEGMENTS; i++)
    {
      if (_segments[i].isSelected()) {
        _segments[i].setColor(slot, c, i);
        applied = true;
      }
    }
  }

  if (!applyToAllSelected || !applied) {
    uint8_t mainseg = getMainSegmentId();
    _segments[mainseg].setColor(slot, c, mainseg);
  }
}

void WS2812FX::setBrightness(uint8_t b) {
  if (gammaCorrectBri) b = gamma8(b);
  if (_brightness == b) return;
  _brightness = b;
  _segment_index = 0;
  if (_brightness == 0) { //unfreeze all segments on power off
    for (uint8_t i = 0; i < MAX_NUM_SEGMENTS; i++)
    {
      _segments[i].setOption(SEG_OPTION_FREEZE, false);
    }
  }
  if (SEGENV.next_time > millis() + 22 && millis() - _lastShow > MIN_SHOW_DELAY) show();//apply brightness change immediately if no refresh soon
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
  if (_segments[mainSegment].isActive()) return mainSegment;
  for (uint8_t i = 0; i < MAX_NUM_SEGMENTS; i++) //get first active
  {
    if (_segments[i].isActive()) return i;
  }
  return 0;
}

uint32_t WS2812FX::getColor(void) {
  return _segments[getMainSegmentId()].colors[0];
}

uint32_t WS2812FX::getPixelColor(uint16_t i)
{
  i = realPixelIndex(i);
  
  if (i < customMappingSize) i = customMappingTable[i];

  if (_skipFirstMode) i += LED_SKIP_AMOUNT;
  
  if (i >= _lengthRaw) return 0;
  
  return busses.getPixelColor(i);
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

uint32_t WS2812FX::getLastShow(void) {
  return _lastShow;
}

//TODO these need to be on a per-strip basis
uint8_t WS2812FX::getColorOrder(void) {
  return COL_ORDER_GRB;
}

void WS2812FX::setColorOrder(uint8_t co) {
  //bus->SetColorOrder(co);
}

void WS2812FX::setSegment(uint8_t n, uint16_t i1, uint16_t i2, uint8_t grouping, uint8_t spacing) {
  if (n >= MAX_NUM_SEGMENTS) return;
  Segment& seg = _segments[n];

  //return if neither bounds nor grouping have changed
  if (seg.start == i1 && seg.stop == i2 && (!grouping || (seg.grouping == grouping && seg.spacing == spacing))) return;

  if (seg.stop) setRange(seg.start, seg.stop -1, 0); //turn old segment range off
  if (i2 <= i1) //disable segment
  {
    seg.stop = 0; 
    if (n == mainSegment) //if main segment is deleted, set first active as main segment
    {
      for (uint8_t i = 0; i < MAX_NUM_SEGMENTS; i++)
      {
        if (_segments[i].isActive()) {
          mainSegment = i;
          return;
        }
      }
      mainSegment = 0; //should not happen (always at least one active segment)
    }
    return;
  }
  if (i1 < _length) seg.start = i1;
  seg.stop = i2;
  if (i2 > _length) seg.stop = _length;
  if (grouping) {
    seg.grouping = grouping;
    seg.spacing = spacing;
  }
  _segment_runtimes[n].reset();
}

void WS2812FX::resetSegments() {
  mainSegment = 0;
  memset(_segments, 0, sizeof(_segments));
  //memset(_segment_runtimes, 0, sizeof(_segment_runtimes));
  _segment_index = 0;
  _segments[0].mode = DEFAULT_MODE;
  _segments[0].colors[0] = DEFAULT_COLOR;
  _segments[0].start = 0;
  _segments[0].speed = DEFAULT_SPEED;
  _segments[0].intensity = DEFAULT_INTENSITY;
  _segments[0].stop = _length;
  _segments[0].grouping = 1;
  _segments[0].setOption(SEG_OPTION_SELECTED, 1);
  _segments[0].setOption(SEG_OPTION_ON, 1);
  _segments[0].opacity = 255;

  for (uint16_t i = 1; i < MAX_NUM_SEGMENTS; i++)
  {
    _segments[i].colors[0] = color_wheel(i*51);
    _segments[i].grouping = 1;
    _segments[i].setOption(SEG_OPTION_ON, 1);
    _segments[i].opacity = 255;
    _segments[i].speed = DEFAULT_SPEED;
    _segments[i].intensity = DEFAULT_INTENSITY;
    _segment_runtimes[i].reset();
  }
  _segment_runtimes[0].reset();
}

//After this function is called, setPixelColor() will use that segment (offsets, grouping, ... will apply)
void WS2812FX::setPixelSegment(uint8_t n)
{
  if (n < MAX_NUM_SEGMENTS) {
    _segment_index = n;
    _virtualSegmentLength = SEGMENT.length();
  } else {
    _segment_index = 0;
    _virtualSegmentLength = 0;
  }
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

void WS2812FX::setTransition(uint16_t t)
{
  _transitionDur = t;
}

void WS2812FX::setTransitionMode(bool t)
{
  unsigned long waitMax = millis() + 20; //refresh after 20 ms if transition enabled
  for (uint16_t i = 0; i < MAX_NUM_SEGMENTS; i++)
  {
    _segment_index = i;
    SEGMENT.setOption(SEG_OPTION_TRANSITIONAL, t);

    if (t && SEGMENT.mode == FX_MODE_STATIC && SEGENV.next_time > waitMax) SEGENV.next_time = waitMax;
  }
}

/*
 * color blend function
 */
uint32_t WS2812FX::color_blend(uint32_t color1, uint32_t color2, uint16_t blend, bool b16) {
  if(blend == 0)   return color1;
  uint16_t blendmax = b16 ? 0xFFFF : 0xFF;
  if(blend == blendmax) return color2;
  uint8_t shift = b16 ? 16 : 8;

  uint32_t w1 = (color1 >> 24) & 0xFF;
  uint32_t r1 = (color1 >> 16) & 0xFF;
  uint32_t g1 = (color1 >>  8) & 0xFF;
  uint32_t b1 =  color1        & 0xFF;

  uint32_t w2 = (color2 >> 24) & 0xFF;
  uint32_t r2 = (color2 >> 16) & 0xFF;
  uint32_t g2 = (color2 >>  8) & 0xFF;
  uint32_t b2 =  color2        & 0xFF;

  uint32_t w3 = ((w2 * blend) + (w1 * (blendmax - blend))) >> shift;
  uint32_t r3 = ((r2 * blend) + (r1 * (blendmax - blend))) >> shift;
  uint32_t g3 = ((g2 * blend) + (g1 * (blendmax - blend))) >> shift;
  uint32_t b3 = ((b2 * blend) + (b1 * (blendmax - blend))) >> shift;

  return ((w3 << 24) | (r3 << 16) | (g3 << 8) | (b3));
}

/*
 * Fills segment with color
 */
void WS2812FX::fill(uint32_t c) {
  for(uint16_t i = 0; i < SEGLEN; i++) {
    setPixelColor(i, c);
  }
}

/*
 * Blends the specified color with the existing pixel color.
 */
void WS2812FX::blendPixelColor(uint16_t n, uint32_t color, uint8_t blend)
{
  setPixelColor(n, color_blend(getPixelColor(n), color, blend));
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

  for(uint16_t i = 0; i < SEGLEN; i++) {
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
  for(uint16_t i = 0; i < SEGLEN; i++)
  {
    CRGB cur = col_to_crgb(getPixelColor(i));
    CRGB part = cur;
    part.nscale8(seep);
    cur.nscale8(keep);
    cur += carryover;
    if(i > 0) {
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
 * Generates a tristate square wave w/ attac & decay 
 * @param x input value 0-255
 * @param pulsewidth 0-127 
 * @param attdec attac & decay, max. pulsewidth / 2
 * @returns signed waveform value
 */
int8_t WS2812FX::tristate_square8(uint8_t x, uint8_t pulsewidth, uint8_t attdec) {
  int8_t a = 127;
  if (x > 127) {
    a = -127;
    x -= 127;
  }

  if (x < attdec) { //inc to max
    return (int16_t) x * a / attdec;
  }
  else if (x < pulsewidth - attdec) { //max
    return a;
  }  
  else if (x < pulsewidth) { //dec to 0
    return (int16_t) (pulsewidth - x) * a / attdec;
  }
  return 0;
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
    d = MIN(x, y);
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


void WS2812FX::load_gradient_palette(uint8_t index)
{
  byte i = constrain(index, 0, GRADIENT_PALETTE_COUNT -1);
  byte tcp[72]; //support gradient palettes with up to 18 entries
  memcpy_P(tcp, (byte*)pgm_read_dword(&(gGradientPalettes[i])), 72);
  targetPalette.loadDynamicGradientPalette(tcp);
}


/*
 * FastLED palette modes helper function. Limitation: Due to memory reasons, multiple active segments with FastLED will disable the Palette transitions
 */
void WS2812FX::handle_palette(void)
{
  bool singleSegmentMode = (_segment_index == _segment_index_palette_last);
  _segment_index_palette_last = _segment_index;

  byte paletteIndex = SEGMENT.palette;
  if (paletteIndex == 0) //default palette. Differs depending on effect
  {
    switch (SEGMENT.mode)
    {
      case FX_MODE_FIRE_2012  : paletteIndex = 35; break; //heat palette
      case FX_MODE_COLORWAVES : paletteIndex = 26; break; //landscape 33
      case FX_MODE_FILLNOISE8 : paletteIndex =  9; break; //ocean colors
      case FX_MODE_NOISE16_1  : paletteIndex = 20; break; //Drywet
      case FX_MODE_NOISE16_2  : paletteIndex = 43; break; //Blue cyan yellow
      case FX_MODE_NOISE16_3  : paletteIndex = 35; break; //heat palette
      case FX_MODE_NOISE16_4  : paletteIndex = 26; break; //landscape 33
      case FX_MODE_GLITTER    : paletteIndex = 11; break; //rainbow colors
      case FX_MODE_SUNRISE    : paletteIndex = 35; break; //heat palette
      case FX_MODE_FLOW       : paletteIndex =  6; break; //party
    }
  }
  if (SEGMENT.mode >= FX_MODE_METEOR && paletteIndex == 0) paletteIndex = 4;
  
  switch (paletteIndex)
  {
    case 0: //default palette. Exceptions for specific effects above
      targetPalette = PartyColors_p; break;
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
    case 3: {//primary + secondary
      CRGB prim = col_to_crgb(SEGCOLOR(0));
      CRGB sec  = col_to_crgb(SEGCOLOR(1));
      targetPalette = CRGBPalette16(prim,prim,sec,sec); break;}
    case 4: {//primary + secondary + tertiary
      CRGB prim = col_to_crgb(SEGCOLOR(0));
      CRGB sec  = col_to_crgb(SEGCOLOR(1));
      CRGB ter  = col_to_crgb(SEGCOLOR(2));
      targetPalette = CRGBPalette16(ter,sec,prim); break;}
    case 5: {//primary + secondary (+tert if not off), more distinct
      CRGB prim = col_to_crgb(SEGCOLOR(0));
      CRGB sec  = col_to_crgb(SEGCOLOR(1));
      if (SEGCOLOR(2)) {
        CRGB ter = col_to_crgb(SEGCOLOR(2));
        targetPalette = CRGBPalette16(prim,prim,prim,prim,prim,sec,sec,sec,sec,sec,ter,ter,ter,ter,ter,prim);
      } else {
        targetPalette = CRGBPalette16(prim,prim,prim,prim,prim,prim,prim,prim,sec,sec,sec,sec,sec,sec,sec,sec);
      }
      break;}
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
      load_gradient_palette(paletteIndex -13);
  }
  
  if (singleSegmentMode && paletteFade && SEGENV.call > 0) //only blend if just one segment uses FastLED mode
  {
    nblendPaletteTowardPalette(currentPalette, targetPalette, 48);
  } else
  {
    currentPalette = targetPalette;
  }
}


/*
 * Gets a single color from the currently selected palette.
 * @param i Palette Index (if mapping is true, the full palette will be SEGLEN long, if false, 255). Will wrap around automatically.
 * @param mapping if true, LED position in segment is considered for color
 * @param wrap FastLED palettes will usally wrap back to the start smoothly. Set false to get a hard edge
 * @param mcol If the default palette 0 is selected, return the standard color 0, 1 or 2 instead. If >2, Party palette is used instead
 * @param pbri Value to scale the brightness of the returned color by. Default is 255. (no scaling)
 * @returns Single color from palette
 */
uint32_t WS2812FX::color_from_palette(uint16_t i, bool mapping, bool wrap, uint8_t mcol, uint8_t pbri)
{
  if (SEGMENT.palette == 0 && mcol < 3) {
    uint32_t color = SEGCOLOR(mcol);
    if (pbri != 255) {
      CRGB crgb_color = col_to_crgb(color);
      crgb_color.nscale8_video(pbri);
      return crgb_to_col(crgb_color);
    } else {
      return color;
    }
  }

  uint8_t paletteIndex = i;
  if (mapping) paletteIndex = (i*255)/(SEGLEN -1);
  if (!wrap) paletteIndex = scale8(paletteIndex, 240); //cut off blend at palette "end"
  CRGB fastled_col;
  fastled_col = ColorFromPalette( currentPalette, paletteIndex, pbri, (paletteBlend == 3)? NOBLEND:LINEARBLEND);

  return crgb_to_col(fastled_col);
}

//@returns `true` if color, mode, speed, intensity and palette match
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
  //if (a->getOption(SEG_OPTION_REVERSED) != b->getOption(SEG_OPTION_REVERSED)) return false;
  return true;
}


//load custom mapping table from JSON file
void WS2812FX::deserializeMap(void) {
  if (!WLED_FS.exists("/ledmap.json")) return;
  DynamicJsonDocument doc(JSON_BUFFER_SIZE);  // full sized buffer for larger maps

  DEBUG_PRINTLN(F("Reading LED map from /ledmap.json..."));

  if (!readObjectFromFile("/ledmap.json", nullptr, &doc)) return; //if file does not exist just exit

  if (customMappingTable != nullptr) {
    delete[] customMappingTable;
    customMappingTable = nullptr;
    customMappingSize = 0;
  }

  JsonArray map = doc[F("map")];
  if (!map.isNull() && map.size()) {  // not an empty map
    customMappingSize  = map.size();
    customMappingTable = new uint16_t[customMappingSize];
    for (uint16_t i=0; i<customMappingSize; i++) {
      customMappingTable[i] = (uint16_t) map[i];
    }
  }
}

//gamma 2.8 lookup table used for color correction
byte gammaT[] = {
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

uint8_t WS2812FX::gamma8_cal(uint8_t b, float gamma) {
  return (int)(pow((float)b / 255.0, gamma) * 255 + 0.5);
}

void WS2812FX::calcGammaTable(float gamma)
{
  for (uint16_t i = 0; i < 256; i++) {
    gammaT[i] = gamma8_cal(i, gamma);
  }
}

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

WS2812FX* WS2812FX::instance = nullptr;