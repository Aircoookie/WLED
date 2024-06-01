/*
 * Class implementation for addressing various light types
 */

#include <Arduino.h>
#include <IPAddress.h>
#include "const.h"
#include "pin_manager.h"
#include "bus_wrapper.h"
#include "bus_manager.h"

extern bool cctICused;

//colors.cpp
uint32_t colorBalanceFromKelvin(uint16_t kelvin, uint32_t rgb);

//udp.cpp
uint8_t realtimeBroadcast(uint8_t type, IPAddress client, uint16_t length, byte *buffer, uint8_t bri=255, bool isRGBW=false);

// enable additional debug output
#if defined(WLED_DEBUG_HOST)
  #include "net_debug.h"
  #define DEBUGOUT NetDebug
#else
  #define DEBUGOUT Serial
#endif

#ifdef WLED_DEBUG
  #ifndef ESP8266
  #include <rom/rtc.h>
  #endif
  #define DEBUG_PRINT(x) DEBUGOUT.print(x)
  #define DEBUG_PRINTLN(x) DEBUGOUT.println(x)
  #define DEBUG_PRINTF(x...) DEBUGOUT.printf(x)
  #define DEBUG_PRINTF_P(x...) DEBUGOUT.printf_P(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(x...)
  #define DEBUG_PRINTF_P(x...)
#endif

//color mangling macros
#define RGBW32(r,g,b,w) (uint32_t((byte(w) << 24) | (byte(r) << 16) | (byte(g) << 8) | (byte(b))))
#define R(c) (byte((c) >> 16))
#define G(c) (byte((c) >> 8))
#define B(c) (byte(c))
#define W(c) (byte((c) >> 24))


void ColorOrderMap::add(uint16_t start, uint16_t len, uint8_t colorOrder) {
  if (_count >= WLED_MAX_COLOR_ORDER_MAPPINGS) {
    return;
  }
  if (len == 0) {
    return;
  }
  // upper nibble contains W swap information
  if ((colorOrder & 0x0F) > COL_ORDER_MAX) {
    return;
  }
  _mappings[_count].start = start;
  _mappings[_count].len = len;
  _mappings[_count].colorOrder = colorOrder;
  _count++;
}

uint8_t IRAM_ATTR ColorOrderMap::getPixelColorOrder(uint16_t pix, uint8_t defaultColorOrder) const {
  if (_count > 0) {
    // upper nibble contains W swap information
    // when ColorOrderMap's upper nibble contains value >0 then swap information is used from it, otherwise global swap is used
    for (unsigned i = 0; i < _count; i++) {
      if (pix >= _mappings[i].start && pix < (_mappings[i].start + _mappings[i].len)) {
        return _mappings[i].colorOrder | ((_mappings[i].colorOrder >> 4) ? 0 : (defaultColorOrder & 0xF0));
      }
    }
  }
  return defaultColorOrder;
}


uint32_t Bus::autoWhiteCalc(uint32_t c) {
  uint8_t aWM = _autoWhiteMode;
  if (_gAWM < AW_GLOBAL_DISABLED) aWM = _gAWM;
  if (aWM == RGBW_MODE_MANUAL_ONLY) return c;
  uint8_t w = W(c);
  //ignore auto-white calculation if w>0 and mode DUAL (DUAL behaves as BRIGHTER if w==0)
  if (w > 0 && aWM == RGBW_MODE_DUAL) return c;
  uint8_t r = R(c);
  uint8_t g = G(c);
  uint8_t b = B(c);
  if (aWM == RGBW_MODE_MAX) return RGBW32(r, g, b, r > g ? (r > b ? r : b) : (g > b ? g : b)); // brightest RGB channel
  w = r < g ? (r < b ? r : b) : (g < b ? g : b);
  if (aWM == RGBW_MODE_AUTO_ACCURATE) { r -= w; g -= w; b -= w; } //subtract w in ACCURATE mode
  return RGBW32(r, g, b, w);
}

uint8_t *Bus::allocData(size_t size) {
  if (_data) free(_data); // should not happen, but for safety
  return _data = (uint8_t *)(size>0 ? calloc(size, sizeof(uint8_t)) : nullptr);
}


BusDigital::BusDigital(BusConfig &bc, uint8_t nr, const ColorOrderMap &com)
: Bus(bc.type, bc.start, bc.autoWhite, bc.count, bc.reversed, (bc.refreshReq || bc.type == TYPE_TM1814))
, _skip(bc.skipAmount) //sacrificial pixels
, _colorOrder(bc.colorOrder)
, _milliAmpsPerLed(bc.milliAmpsPerLed)
, _milliAmpsMax(bc.milliAmpsMax)
, _colorOrderMap(com)
{
  if (!IS_DIGITAL(bc.type) || !bc.count) return;
  if (!pinManager.allocatePin(bc.pins[0], true, PinOwner::BusDigital)) return;
  _frequencykHz = 0U;
  _pins[0] = bc.pins[0];
  if (IS_2PIN(bc.type)) {
    if (!pinManager.allocatePin(bc.pins[1], true, PinOwner::BusDigital)) {
      cleanup();
      return;
    }
    _pins[1] = bc.pins[1];
    _frequencykHz = bc.frequency ? bc.frequency : 2000U; // 2MHz clock if undefined
  }
  _iType = PolyBus::getI(bc.type, _pins, nr);
  if (_iType == I_NONE) return;
  if (bc.doubleBuffer && !allocData(bc.count * Bus::getNumberOfChannels(bc.type))) return;
  //_buffering = bc.doubleBuffer;
  uint16_t lenToCreate = bc.count;
  if (bc.type == TYPE_WS2812_1CH_X3) lenToCreate = NUM_ICS_WS2812_1CH_3X(bc.count); // only needs a third of "RGB" LEDs for NeoPixelBus
  _busPtr = PolyBus::create(_iType, _pins, lenToCreate + _skip, nr, _frequencykHz);
  _valid = (_busPtr != nullptr);
  DEBUG_PRINTF_P(PSTR("%successfully inited strip %u (len %u) with type %u and pins %u,%u (itype %u). mA=%d/%d\n"), _valid?"S":"Uns", nr, bc.count, bc.type, _pins[0], IS_2PIN(bc.type)?_pins[1]:255, _iType, _milliAmpsPerLed, _milliAmpsMax);
}

//fine tune power estimation constants for your setup
//you can set it to 0 if the ESP is powered by USB and the LEDs by external
#ifndef MA_FOR_ESP
  #ifdef ESP8266
    #define MA_FOR_ESP         80 //how much mA does the ESP use (Wemos D1 about 80mA)
  #else
    #define MA_FOR_ESP        120 //how much mA does the ESP use (ESP32 about 120mA)
  #endif
#endif

//DISCLAIMER
//The following function attemps to calculate the current LED power usage,
//and will limit the brightness to stay below a set amperage threshold.
//It is NOT a measurement and NOT guaranteed to stay within the ablMilliampsMax margin.
//Stay safe with high amperage and have a reasonable safety margin!
//I am NOT to be held liable for burned down garages or houses!

// To disable brightness limiter we either set output max current to 0 or single LED current to 0
uint8_t BusDigital::estimateCurrentAndLimitBri() {
  bool useWackyWS2815PowerModel = false;
  byte actualMilliampsPerLed = _milliAmpsPerLed;

  if (_milliAmpsMax < MA_FOR_ESP/BusManager::getNumBusses() || actualMilliampsPerLed == 0) { //0 mA per LED and too low numbers turn off calculation
    return _bri;
  }

  if (_milliAmpsPerLed == 255) {
    useWackyWS2815PowerModel = true;
    actualMilliampsPerLed = 12; // from testing an actual strip
  }

  size_t powerBudget = (_milliAmpsMax - MA_FOR_ESP/BusManager::getNumBusses()); //80/120mA for ESP power
  if (powerBudget > getLength()) { //each LED uses about 1mA in standby, exclude that from power budget
    powerBudget -= getLength();
  } else {
    powerBudget = 0;
  }

  uint32_t busPowerSum = 0;
  for (unsigned i = 0; i < getLength(); i++) {  //sum up the usage of each LED
    uint32_t c = getPixelColor(i); // always returns original or restored color without brightness scaling
    byte r = R(c), g = G(c), b = B(c), w = W(c);

    if (useWackyWS2815PowerModel) { //ignore white component on WS2815 power calculation
      busPowerSum += (max(max(r,g),b)) * 3;
    } else {
      busPowerSum += (r + g + b + w);
    }
  }

  if (hasWhite()) { //RGBW led total output with white LEDs enabled is still 50mA, so each channel uses less
    busPowerSum *= 3;
    busPowerSum >>= 2; //same as /= 4
  }

  // powerSum has all the values of channels summed (max would be getLength()*765 as white is excluded) so convert to milliAmps
  busPowerSum = (busPowerSum * actualMilliampsPerLed) / 765;
  _milliAmpsTotal = busPowerSum * _bri / 255;

  uint8_t newBri = _bri;
  if (busPowerSum * _bri / 255 > powerBudget) { //scale brightness down to stay in current limit
    float scale = (float)(powerBudget * 255) / (float)(busPowerSum * _bri);
    if (scale >= 1.0f) return _bri;
    _milliAmpsTotal = ceilf((float)_milliAmpsTotal * scale);
    uint8_t scaleB = min((int)(scale * 255), 255);
    newBri = unsigned(_bri * scaleB) / 256 + 1;
  }
  return newBri;
}

void BusDigital::show() {
  _milliAmpsTotal = 0;
  if (!_valid) return;

  uint8_t cctWW = 0, cctCW = 0;
  uint8_t newBri = estimateCurrentAndLimitBri();  // will fill _milliAmpsTotal
  if (newBri < _bri) PolyBus::setBrightness(_busPtr, _iType, newBri); // limit brightness to stay within current limits

  if (_data) {
    size_t channels = getNumberOfChannels();
    int16_t oldCCT = Bus::_cct; // temporarily save bus CCT
    for (size_t i=0; i<_len; i++) {
      size_t offset = i * channels;
      uint8_t co = _colorOrderMap.getPixelColorOrder(i+_start, _colorOrder);
      uint32_t c;
      if (_type == TYPE_WS2812_1CH_X3) { // map to correct IC, each controls 3 LEDs (_len is always a multiple of 3)
        switch (i%3) {
          case 0: c = RGBW32(_data[offset]  , _data[offset+1], _data[offset+2], 0); break;
          case 1: c = RGBW32(_data[offset-1], _data[offset]  , _data[offset+1], 0); break;
          case 2: c = RGBW32(_data[offset-2], _data[offset-1], _data[offset]  , 0); break;
        }
      } else {
        if (hasRGB()) c = RGBW32(_data[offset], _data[offset+1], _data[offset+2], hasWhite() ? _data[offset+3] : 0);
        else          c = RGBW32(0, 0, 0, _data[offset]);
      }
      if (hasCCT()) {
        // unfortunately as a segment may span multiple buses or a bus may contain multiple segments and each segment may have different CCT
        // we need to extract and appy CCT value for each pixel individually even though all buses share the same _cct variable
        // TODO: there is an issue if CCT is calculated from RGB value (_cct==-1), we cannot do that with double buffer
        Bus::_cct = _data[offset+channels-1];
        Bus::calculateCCT(c, cctWW, cctCW);
      }
      uint16_t pix = i;
      if (_reversed) pix = _len - pix -1;
      pix += _skip;
      PolyBus::setPixelColor(_busPtr, _iType, pix, c, co, (cctCW<<8) | cctWW);
    }
    #if !defined(STATUSLED) || STATUSLED>=0
    if (_skip) PolyBus::setPixelColor(_busPtr, _iType, 0, 0, _colorOrderMap.getPixelColorOrder(_start, _colorOrder)); // paint skipped pixels black
    #endif
    for (int i=1; i<_skip; i++) PolyBus::setPixelColor(_busPtr, _iType, i, 0, _colorOrderMap.getPixelColorOrder(_start, _colorOrder)); // paint skipped pixels black
    Bus::_cct = oldCCT;
  } else {
    if (newBri < _bri) {
      uint16_t hwLen = _len;
      if (_type == TYPE_WS2812_1CH_X3) hwLen = NUM_ICS_WS2812_1CH_3X(_len); // only needs a third of "RGB" LEDs for NeoPixelBus
      for (unsigned i = 0; i < hwLen; i++) {
        // use 0 as color order, actual order does not matter here as we just update the channel values as-is
        uint32_t c = restoreColorLossy(PolyBus::getPixelColor(_busPtr, _iType, i, 0), _bri);
        if (hasCCT()) Bus::calculateCCT(c, cctWW, cctCW); // this will unfortunately corrupt (segment) CCT data on every bus
        PolyBus::setPixelColor(_busPtr, _iType, i, c, 0, (cctCW<<8) | cctWW); // repaint all pixels with new brightness
      }
    }
  }
  PolyBus::show(_busPtr, _iType, !_data); // faster if buffer consistency is not important (use !_buffering this causes 20% FPS drop)
  // restore bus brightness to its original value
  // this is done right after show, so this is only OK if LED updates are completed before show() returns
  // or async show has a separate buffer (ESP32 RMT and I2S are ok)
  if (newBri < _bri) PolyBus::setBrightness(_busPtr, _iType, _bri);
}

bool BusDigital::canShow() {
  if (!_valid) return true;
  return PolyBus::canShow(_busPtr, _iType);
}

void BusDigital::setBrightness(uint8_t b) {
  if (_bri == b) return;
  //Fix for turning off onboard LED breaking bus
  #ifdef LED_BUILTIN
  if (_bri == 0) { // && b > 0, covered by guard if above
    if (_pins[0] == LED_BUILTIN || _pins[1] == LED_BUILTIN) reinit();
  }
  #endif
  Bus::setBrightness(b);
  PolyBus::setBrightness(_busPtr, _iType, b);
}

//If LEDs are skipped, it is possible to use the first as a status LED.
//TODO only show if no new show due in the next 50ms
void BusDigital::setStatusPixel(uint32_t c) {
  if (_valid && _skip) {
    PolyBus::setPixelColor(_busPtr, _iType, 0, c, _colorOrderMap.getPixelColorOrder(_start, _colorOrder));
    if (canShow()) PolyBus::show(_busPtr, _iType);
  }
}

void IRAM_ATTR BusDigital::setPixelColor(uint16_t pix, uint32_t c) {
  if (!_valid) return;
  uint8_t cctWW = 0, cctCW = 0;
  if (hasWhite()) c = autoWhiteCalc(c);
  if (Bus::_cct >= 1900) c = colorBalanceFromKelvin(Bus::_cct, c); //color correction from CCT
  if (_data) {
    size_t offset = pix * getNumberOfChannels();
    if (hasRGB()) {
      _data[offset++] = R(c);
      _data[offset++] = G(c);
      _data[offset++] = B(c);
    }
    if (hasWhite()) _data[offset++] = W(c);
    // unfortunately as a segment may span multiple buses or a bus may contain multiple segments and each segment may have different CCT
    // we need to store CCT value for each pixel (if there is a color correction in play, convert K in CCT ratio)
    if (hasCCT())   _data[offset]   = Bus::_cct >= 1900 ? (Bus::_cct - 1900) >> 5 : (Bus::_cct < 0 ? 127 : Bus::_cct); // TODO: if _cct == -1 we simply ignore it
  } else {
    if (_reversed) pix = _len - pix -1;
    pix += _skip;
    uint8_t co = _colorOrderMap.getPixelColorOrder(pix+_start, _colorOrder);
    if (_type == TYPE_WS2812_1CH_X3) { // map to correct IC, each controls 3 LEDs
      uint16_t pOld = pix;
      pix = IC_INDEX_WS2812_1CH_3X(pix);
      uint32_t cOld = restoreColorLossy(PolyBus::getPixelColor(_busPtr, _iType, pix, co),_bri);
      switch (pOld % 3) { // change only the single channel (TODO: this can cause loss because of get/set)
        case 0: c = RGBW32(R(cOld), W(c)   , B(cOld), 0); break;
        case 1: c = RGBW32(W(c)   , G(cOld), B(cOld), 0); break;
        case 2: c = RGBW32(R(cOld), G(cOld), W(c)   , 0); break;
      }
    }
    if (hasCCT()) Bus::calculateCCT(c, cctWW, cctCW);
    PolyBus::setPixelColor(_busPtr, _iType, pix, c, co, (cctCW<<8) | cctWW);
  }
}

// returns original color if global buffering is enabled, else returns lossly restored color from bus
uint32_t IRAM_ATTR BusDigital::getPixelColor(uint16_t pix) {
  if (!_valid) return 0;
  if (_data) {
    size_t offset = pix * getNumberOfChannels();
    uint32_t c;
    if (!hasRGB()) {
      c = RGBW32(_data[offset], _data[offset], _data[offset], _data[offset]);
    } else {
      c = RGBW32(_data[offset], _data[offset+1], _data[offset+2], hasWhite() ? _data[offset+3] : 0);
    }
    return c;
  } else {
    if (_reversed) pix = _len - pix -1;
    pix += _skip;
    uint8_t co = _colorOrderMap.getPixelColorOrder(pix+_start, _colorOrder);
    uint32_t c = restoreColorLossy(PolyBus::getPixelColor(_busPtr, _iType, (_type==TYPE_WS2812_1CH_X3) ? IC_INDEX_WS2812_1CH_3X(pix) : pix, co),_bri);
    if (_type == TYPE_WS2812_1CH_X3) { // map to correct IC, each controls 3 LEDs
      uint8_t r = R(c);
      uint8_t g = _reversed ? B(c) : G(c); // should G and B be switched if _reversed?
      uint8_t b = _reversed ? G(c) : B(c);
      switch (pix % 3) { // get only the single channel
        case 0: c = RGBW32(g, g, g, g); break;
        case 1: c = RGBW32(r, r, r, r); break;
        case 2: c = RGBW32(b, b, b, b); break;
      }
    }
    return c;
  }
}

uint8_t BusDigital::getPins(uint8_t* pinArray) {
  uint8_t numPins = IS_2PIN(_type) ? 2 : 1;
  for (unsigned i = 0; i < numPins; i++) pinArray[i] = _pins[i];
  return numPins;
}

void BusDigital::setColorOrder(uint8_t colorOrder) {
  // upper nibble contains W swap information
  if ((colorOrder & 0x0F) > 5) return;
  _colorOrder = colorOrder;
}

void BusDigital::reinit() {
  if (!_valid) return;
  PolyBus::begin(_busPtr, _iType, _pins);
}

void BusDigital::cleanup() {
  DEBUG_PRINTLN(F("Digital Cleanup."));
  PolyBus::cleanup(_busPtr, _iType);
  _iType = I_NONE;
  _valid = false;
  _busPtr = nullptr;
  if (_data != nullptr) freeData();
  pinManager.deallocatePin(_pins[1], PinOwner::BusDigital);
  pinManager.deallocatePin(_pins[0], PinOwner::BusDigital);
}


BusPwm::BusPwm(BusConfig &bc)
: Bus(bc.type, bc.start, bc.autoWhite, 1, bc.reversed)
{
  if (!IS_PWM(bc.type)) return;
  uint8_t numPins = NUM_PWM_PINS(bc.type);
  _frequency = bc.frequency ? bc.frequency : WLED_PWM_FREQ;

#ifdef ESP8266
  // duty cycle resolution (_depth) can be extracted from this formula: 1MHz > _frequency * 2^_depth
  if      (_frequency > 1760) _depth =  8;
  else if (_frequency >  880) _depth =  9;
  else                        _depth = 10; // WLED_PWM_FREQ <= 880Hz
  analogWriteRange((1<<_depth)-1);
  analogWriteFreq(_frequency);
#else
  _ledcStart = pinManager.allocateLedc(numPins);
  if (_ledcStart == 255) { //no more free LEDC channels
    deallocatePins(); return;
  }
  // duty cycle resolution (_depth) can be extracted from this formula: 80MHz > _frequency * 2^_depth
  if      (_frequency > 78124) _depth =  9;
  else if (_frequency > 39062) _depth = 10;
  else if (_frequency > 19531) _depth = 11;
  else                         _depth = 12; // WLED_PWM_FREQ <= 19531Hz
#endif

  for (unsigned i = 0; i < numPins; i++) {
    uint8_t currentPin = bc.pins[i];
    if (!pinManager.allocatePin(currentPin, true, PinOwner::BusPwm)) {
      deallocatePins(); return;
    }
    _pins[i] = currentPin; //store only after allocatePin() succeeds
    #ifdef ESP8266
    pinMode(_pins[i], OUTPUT);
    #else
    ledcSetup(_ledcStart + i, _frequency, _depth);
    ledcAttachPin(_pins[i], _ledcStart + i);
    #endif
  }
  _data = _pwmdata; // avoid malloc() and use stack
  _valid = true;
  DEBUG_PRINTF_P(PSTR("%successfully inited PWM strip with type %u and pins %u,%u,%u,%u,%u\n"), _valid?"S":"Uns", bc.type, _pins[0], _pins[1], _pins[2], _pins[3], _pins[4]);
}

void BusPwm::setPixelColor(uint16_t pix, uint32_t c) {
  if (pix != 0 || !_valid) return; //only react to first pixel
  if (_type != TYPE_ANALOG_3CH) c = autoWhiteCalc(c);
  if (Bus::_cct >= 1900 && (_type == TYPE_ANALOG_3CH || _type == TYPE_ANALOG_4CH)) {
    c = colorBalanceFromKelvin(Bus::_cct, c); //color correction from CCT
  }
  uint8_t r = R(c);
  uint8_t g = G(c);
  uint8_t b = B(c);
  uint8_t w = W(c);

  switch (_type) {
    case TYPE_ANALOG_1CH: //one channel (white), relies on auto white calculation
      _data[0] = w;
      break;
    case TYPE_ANALOG_2CH: //warm white + cold white
      if (cctICused) {
        _data[0] = w;
        _data[1] = Bus::_cct < 0 || Bus::_cct > 255 ? 127 : Bus::_cct;
      } else {
        Bus::calculateCCT(c, _data[0], _data[1]);
      }
      break;
    case TYPE_ANALOG_5CH: //RGB + warm white + cold white
      if (cctICused)
        _data[4] = Bus::_cct < 0 || Bus::_cct > 255 ? 127 : Bus::_cct;
      else
        Bus::calculateCCT(c, w, _data[4]);
    case TYPE_ANALOG_4CH: //RGBW
      _data[3] = w;
    case TYPE_ANALOG_3CH: //standard dumb RGB
      _data[0] = r; _data[1] = g; _data[2] = b;
      break;
  }
}

//does no index check
uint32_t BusPwm::getPixelColor(uint16_t pix) {
  if (!_valid) return 0;
  // TODO getting the reverse from CCT is involved (a quick approximation when CCT blending is ste to 0 implemented)
  switch (_type) {
    case TYPE_ANALOG_1CH: //one channel (white), relies on auto white calculation
      return RGBW32(0, 0, 0, _data[0]);
    case TYPE_ANALOG_2CH: //warm white + cold white
      if (cctICused) return RGBW32(0, 0, 0, _data[0]);
      else           return RGBW32(0, 0, 0, _data[0] + _data[1]);
    case TYPE_ANALOG_5CH: //RGB + warm white + cold white
      if (cctICused) return RGBW32(_data[0], _data[1], _data[2], _data[3]);
      else           return RGBW32(_data[0], _data[1], _data[2], _data[3] + _data[4]);
    case TYPE_ANALOG_4CH: //RGBW
      return RGBW32(_data[0], _data[1], _data[2], _data[3]);
    case TYPE_ANALOG_3CH: //standard dumb RGB
      return RGBW32(_data[0], _data[1], _data[2], 0);
  }
  return RGBW32(_data[0], _data[0], _data[0], _data[0]);
}

#ifndef ESP8266
static const uint16_t cieLUT[256] = {
	0, 2, 4, 5, 7, 9, 11, 13, 15, 16, 
	18, 20, 22, 24, 26, 27, 29, 31, 33, 35, 
	34, 36, 37, 39, 41, 43, 45, 47, 49, 52, 
	54, 56, 59, 61, 64, 67, 69, 72, 75, 78, 
	81, 84, 87, 90, 94, 97, 100, 104, 108, 111, 
	115, 119, 123, 127, 131, 136, 140, 144, 149, 154, 
	158, 163, 168, 173, 178, 183, 189, 194, 200, 205, 
	211, 217, 223, 229, 235, 241, 247, 254, 261, 267, 
	274, 281, 288, 295, 302, 310, 317, 325, 333, 341, 
	349, 357, 365, 373, 382, 391, 399, 408, 417, 426, 
	436, 445, 455, 464, 474, 484, 494, 505, 515, 526, 
	536, 547, 558, 569, 580, 592, 603, 615, 627, 639, 
	651, 663, 676, 689, 701, 714, 727, 741, 754, 768, 
	781, 795, 809, 824, 838, 853, 867, 882, 897, 913, 
	928, 943, 959, 975, 991, 1008, 1024, 1041, 1058, 1075, 
	1092, 1109, 1127, 1144, 1162, 1180, 1199, 1217, 1236, 1255, 
	1274, 1293, 1312, 1332, 1352, 1372, 1392, 1412, 1433, 1454, 
	1475, 1496, 1517, 1539, 1561, 1583, 1605, 1628, 1650, 1673, 
	1696, 1719, 1743, 1767, 1791, 1815, 1839, 1864, 1888, 1913, 
	1939, 1964, 1990, 2016, 2042, 2068, 2095, 2121, 2148, 2176, 
	2203, 2231, 2259, 2287, 2315, 2344, 2373, 2402, 2431, 2461, 
	2491, 2521, 2551, 2581, 2612, 2643, 2675, 2706, 2738, 2770, 
	2802, 2835, 2867, 2900, 2934, 2967, 3001, 3035, 3069, 3104, 
	3138, 3174, 3209, 3244, 3280, 3316, 3353, 3389, 3426, 3463, 
	3501, 3539, 3576, 3615, 3653, 3692, 3731, 3770, 3810, 3850, 
	3890, 3930, 3971, 4012, 4053, 4095
};
#endif

void BusPwm::show() {
  if (!_valid) return;
  uint8_t numPins = NUM_PWM_PINS(_type);
  unsigned maxBri = (1<<_depth) - 1;
  #ifdef ESP8266
  unsigned pwmBri = (unsigned)(roundf(powf((float)_bri / 255.0f, 1.7f) * (float)maxBri)); // using gamma 1.7 to extrapolate PWM duty cycle
  #else
  unsigned pwmBri = cieLUT[_bri] >> (12 - _depth); // use CIE LUT
  #endif
  for (unsigned i = 0; i < numPins; i++) {
    unsigned scaled = (_data[i] * pwmBri) / 255;
    if (_reversed) scaled = maxBri - scaled;
    #ifdef ESP8266
    analogWrite(_pins[i], scaled);
    #else
    ledcWrite(_ledcStart + i, scaled);
    #endif
  }
}

uint8_t BusPwm::getPins(uint8_t* pinArray) {
  if (!_valid) return 0;
  uint8_t numPins = NUM_PWM_PINS(_type);
  for (unsigned i = 0; i < numPins; i++) {
    pinArray[i] = _pins[i];
  }
  return numPins;
}

void BusPwm::deallocatePins() {
  uint8_t numPins = NUM_PWM_PINS(_type);
  for (unsigned i = 0; i < numPins; i++) {
    pinManager.deallocatePin(_pins[i], PinOwner::BusPwm);
    if (!pinManager.isPinOk(_pins[i])) continue;
    #ifdef ESP8266
    digitalWrite(_pins[i], LOW); //turn off PWM interrupt
    #else
    if (_ledcStart < 16) ledcDetachPin(_pins[i]);
    #endif
  }
  #ifdef ARDUINO_ARCH_ESP32
  pinManager.deallocateLedc(_ledcStart, numPins);
  #endif
}


BusOnOff::BusOnOff(BusConfig &bc)
: Bus(bc.type, bc.start, bc.autoWhite, 1, bc.reversed)
, _onoffdata(0)
{
  if (bc.type != TYPE_ONOFF) return;

  uint8_t currentPin = bc.pins[0];
  if (!pinManager.allocatePin(currentPin, true, PinOwner::BusOnOff)) {
    return;
  }
  _pin = currentPin; //store only after allocatePin() succeeds
  pinMode(_pin, OUTPUT);
  _data = &_onoffdata; // avoid malloc() and use stack
  _valid = true;
  DEBUG_PRINTF_P(PSTR("%successfully inited On/Off strip with pin %u\n"), _valid?"S":"Uns", _pin);
}

void BusOnOff::setPixelColor(uint16_t pix, uint32_t c) {
  if (pix != 0 || !_valid) return; //only react to first pixel
  c = autoWhiteCalc(c);
  uint8_t r = R(c);
  uint8_t g = G(c);
  uint8_t b = B(c);
  uint8_t w = W(c);
  _data[0] = bool(r|g|b|w) && bool(_bri) ? 0xFF : 0;
}

uint32_t BusOnOff::getPixelColor(uint16_t pix) {
  if (!_valid) return 0;
  return RGBW32(_data[0], _data[0], _data[0], _data[0]);
}

void BusOnOff::show() {
  if (!_valid) return;
  digitalWrite(_pin, _reversed ? !(bool)_data[0] : (bool)_data[0]);
}

uint8_t BusOnOff::getPins(uint8_t* pinArray) {
  if (!_valid) return 0;
  pinArray[0] = _pin;
  return 1;
}


BusNetwork::BusNetwork(BusConfig &bc)
: Bus(bc.type, bc.start, bc.autoWhite, bc.count)
, _broadcastLock(false)
{
  switch (bc.type) {
    case TYPE_NET_ARTNET_RGB:
      _rgbw = false;
      _UDPtype = 2;
      break;
    case TYPE_NET_ARTNET_RGBW:
      _rgbw = true;
      _UDPtype = 2;
      break;
    case TYPE_NET_E131_RGB:
      _rgbw = false;
      _UDPtype = 1;
      break;
    default: // TYPE_NET_DDP_RGB / TYPE_NET_DDP_RGBW
      _rgbw = bc.type == TYPE_NET_DDP_RGBW;
      _UDPtype = 0;
      break;
  }
  _UDPchannels = _rgbw ? 4 : 3;
  _client = IPAddress(bc.pins[0],bc.pins[1],bc.pins[2],bc.pins[3]);
  _valid = (allocData(_len * _UDPchannels) != nullptr);
  DEBUG_PRINTF_P(PSTR("%successfully inited virtual strip with type %u and IP %u.%u.%u.%u\n"), _valid?"S":"Uns", bc.type, bc.pins[0], bc.pins[1], bc.pins[2], bc.pins[3]);
}

void BusNetwork::setPixelColor(uint16_t pix, uint32_t c) {
  if (!_valid || pix >= _len) return;
  if (_rgbw) c = autoWhiteCalc(c);
  if (Bus::_cct >= 1900) c = colorBalanceFromKelvin(Bus::_cct, c); //color correction from CCT
  uint16_t offset = pix * _UDPchannels;
  _data[offset]   = R(c);
  _data[offset+1] = G(c);
  _data[offset+2] = B(c);
  if (_rgbw) _data[offset+3] = W(c);
}

uint32_t BusNetwork::getPixelColor(uint16_t pix) {
  if (!_valid || pix >= _len) return 0;
  uint16_t offset = pix * _UDPchannels;
  return RGBW32(_data[offset], _data[offset+1], _data[offset+2], (_rgbw ? _data[offset+3] : 0));
}

void BusNetwork::show() {
  if (!_valid || !canShow()) return;
  _broadcastLock = true;
  realtimeBroadcast(_UDPtype, _client, _len, _data, _bri, _rgbw);
  _broadcastLock = false;
}

uint8_t BusNetwork::getPins(uint8_t* pinArray) {
  for (unsigned i = 0; i < 4; i++) {
    pinArray[i] = _client[i];
  }
  return 4;
}

void BusNetwork::cleanup() {
  _type = I_NONE;
  _valid = false;
  freeData();
}


//utility to get the approx. memory usage of a given BusConfig
uint32_t BusManager::memUsage(BusConfig &bc) {
  if (bc.type == TYPE_ONOFF || IS_PWM(bc.type)) return 5;

  uint16_t len = bc.count + bc.skipAmount;
  uint16_t channels = Bus::getNumberOfChannels(bc.type);
  uint16_t multiplier = 1;
  if (IS_DIGITAL(bc.type)) { // digital types
    if (IS_16BIT(bc.type)) len *= 2; // 16-bit LEDs
    #ifdef ESP8266
      if (bc.pins[0] == 3) { //8266 DMA uses 5x the mem
        multiplier = 5;
      }
    #else //ESP32 RMT uses double buffer, I2S uses 5x buffer
      multiplier = 2;
    #endif
  }
  return len * channels * multiplier; //RGB
}

int BusManager::add(BusConfig &bc) {
  if (getNumBusses() - getNumVirtualBusses() >= WLED_MAX_BUSSES) return -1;
  if (IS_VIRTUAL(bc.type)) {
    busses[numBusses] = new BusNetwork(bc);
  } else if (IS_DIGITAL(bc.type)) {
    busses[numBusses] = new BusDigital(bc, numBusses, colorOrderMap);
  } else if (bc.type == TYPE_ONOFF) {
    busses[numBusses] = new BusOnOff(bc);
  } else {
    busses[numBusses] = new BusPwm(bc);
  }
  return numBusses++;
}

//do not call this method from system context (network callback)
void BusManager::removeAll() {
  DEBUG_PRINTLN(F("Removing all."));
  //prevents crashes due to deleting busses while in use.
  while (!canAllShow()) yield();
  for (unsigned i = 0; i < numBusses; i++) delete busses[i];
  numBusses = 0;
}

void BusManager::show() {
  _milliAmpsUsed = 0;
  for (unsigned i = 0; i < numBusses; i++) {
    busses[i]->show();
    _milliAmpsUsed += busses[i]->getUsedCurrent();
  }
  if (_milliAmpsUsed) _milliAmpsUsed += MA_FOR_ESP;
}

void BusManager::setStatusPixel(uint32_t c) {
  for (unsigned i = 0; i < numBusses; i++) {
    busses[i]->setStatusPixel(c);
  }
}

void IRAM_ATTR BusManager::setPixelColor(uint16_t pix, uint32_t c) {
  for (unsigned i = 0; i < numBusses; i++) {
    unsigned bstart = busses[i]->getStart();
    if (pix < bstart || pix >= bstart + busses[i]->getLength()) continue;
    busses[i]->setPixelColor(pix - bstart, c);
  }
}

void BusManager::setBrightness(uint8_t b) {
  for (unsigned i = 0; i < numBusses; i++) {
    busses[i]->setBrightness(b);
  }
}

void BusManager::setSegmentCCT(int16_t cct, bool allowWBCorrection) {
  if (cct > 255) cct = 255;
  if (cct >= 0) {
    //if white balance correction allowed, save as kelvin value instead of 0-255
    if (allowWBCorrection) cct = 1900 + (cct << 5);
  } else cct = -1; // will use kelvin approximation from RGB
  Bus::setCCT(cct);
}

uint32_t BusManager::getPixelColor(uint16_t pix) {
  for (unsigned i = 0; i < numBusses; i++) {
    unsigned bstart = busses[i]->getStart();
    if (pix < bstart || pix >= bstart + busses[i]->getLength()) continue;
    return busses[i]->getPixelColor(pix - bstart);
  }
  return 0;
}

bool BusManager::canAllShow() {
  for (unsigned i = 0; i < numBusses; i++) {
    if (!busses[i]->canShow()) return false;
  }
  return true;
}

Bus* BusManager::getBus(uint8_t busNr) {
  if (busNr >= numBusses) return nullptr;
  return busses[busNr];
}

//semi-duplicate of strip.getLengthTotal() (though that just returns strip._length, calculated in finalizeInit())
uint16_t BusManager::getTotalLength() {
  uint16_t len = 0;
  for (unsigned i=0; i<numBusses; i++) len += busses[i]->getLength();
  return len;
}

// Bus static member definition
int16_t Bus::_cct = -1;
uint8_t Bus::_cctBlend = 0;
uint8_t Bus::_gAWM = 255;

uint16_t BusDigital::_milliAmpsTotal = 0;

uint8_t       BusManager::numBusses = 0;
Bus*          BusManager::busses[WLED_MAX_BUSSES+WLED_MIN_VIRTUAL_BUSSES];
ColorOrderMap BusManager::colorOrderMap = {};
uint16_t      BusManager::_milliAmpsUsed = 0;
uint16_t      BusManager::_milliAmpsMax = ABL_MILLIAMPS_DEFAULT;