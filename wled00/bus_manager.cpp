/*
 * Class implementation for addressing various light types
 */

#include <Arduino.h>
#include <IPAddress.h>
#include "const.h"
#include "pin_manager.h"
#include "bus_wrapper.h"
#include "bus_manager.h"

//colors.cpp
uint32_t colorBalanceFromKelvin(uint16_t kelvin, uint32_t rgb);
uint16_t approximateKelvinFromRGB(uint32_t rgb);
void colorRGBtoRGBW(byte* rgb);

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
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(x...)
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
  if (colorOrder > COL_ORDER_MAX) {
    return;
  }
  _mappings[_count].start = start;
  _mappings[_count].len = len;
  _mappings[_count].colorOrder = colorOrder;
  _count++;
}

uint8_t IRAM_ATTR ColorOrderMap::getPixelColorOrder(uint16_t pix, uint8_t defaultColorOrder) const {
  if (_count == 0) return defaultColorOrder;
  // upper nibble containd W swap information
  uint8_t swapW = defaultColorOrder >> 4;
  for (uint8_t i = 0; i < _count; i++) {
    if (pix >= _mappings[i].start && pix < (_mappings[i].start + _mappings[i].len)) {
      return _mappings[i].colorOrder | (swapW << 4);
    }
  }
  return defaultColorOrder;
}


uint32_t Bus::autoWhiteCalc(uint32_t c) {
  uint8_t aWM = _autoWhiteMode;
  if (_gAWM < 255) aWM = _gAWM;
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


BusDigital::BusDigital(BusConfig &bc, uint8_t nr, const ColorOrderMap &com) : Bus(bc.type, bc.start, bc.autoWhite), _colorOrderMap(com) {
  if (!IS_DIGITAL(bc.type) || !bc.count) return;
  if (!pinManager.allocatePin(bc.pins[0], true, PinOwner::BusDigital)) return;
  _pins[0] = bc.pins[0];
  if (IS_2PIN(bc.type)) {
    if (!pinManager.allocatePin(bc.pins[1], true, PinOwner::BusDigital)) {
    cleanup(); return;
    }
    _pins[1] = bc.pins[1];
  }
  reversed = bc.reversed;
  _needsRefresh = bc.refreshReq || bc.type == TYPE_TM1814;
  _skip = bc.skipAmount;    //sacrificial pixels
  _len = bc.count + _skip;
  _iType = PolyBus::getI(bc.type, _pins, nr);
  if (_iType == I_NONE) return;
  uint16_t lenToCreate = _len;
  if (bc.type == TYPE_WS2812_1CH_X3) lenToCreate = NUM_ICS_WS2812_1CH_3X(_len); // only needs a third of "RGB" LEDs for NeoPixelBus 
  _busPtr = PolyBus::create(_iType, _pins, lenToCreate, nr);
  _valid = (_busPtr != nullptr);
  _colorOrder = bc.colorOrder;
  DEBUG_PRINTF("%successfully inited strip %u (len %u) with type %u and pins %u,%u (itype %u)\n", _valid?"S":"Uns", nr, _len, bc.type, _pins[0],_pins[1],_iType);
}

void BusDigital::show() {
  PolyBus::show(_busPtr, _iType);
}

bool BusDigital::canShow() {
  return PolyBus::canShow(_busPtr, _iType);
}

void BusDigital::setBrightness(uint8_t b) {
  //Fix for turning off onboard LED breaking bus
  #ifdef LED_BUILTIN
  if (_bri == 0 && b > 0) {
    if (_pins[0] == LED_BUILTIN || _pins[1] == LED_BUILTIN) PolyBus::begin(_busPtr, _iType, _pins);
  }
  #endif
  Bus::setBrightness(b);
  PolyBus::setBrightness(_busPtr, _iType, b);
}

//If LEDs are skipped, it is possible to use the first as a status LED.
//TODO only show if no new show due in the next 50ms
void BusDigital::setStatusPixel(uint32_t c) {
  if (_skip && canShow()) {
    PolyBus::setPixelColor(_busPtr, _iType, 0, c, _colorOrderMap.getPixelColorOrder(_start, _colorOrder));
    PolyBus::show(_busPtr, _iType);
  }
}

void IRAM_ATTR BusDigital::setPixelColor(uint16_t pix, uint32_t c) {
  if (_type == TYPE_SK6812_RGBW || _type == TYPE_TM1814 || _type == TYPE_WS2812_1CH_X3) c = autoWhiteCalc(c);
  if (_cct >= 1900) c = colorBalanceFromKelvin(_cct, c); //color correction from CCT
  if (reversed) pix = _len - pix -1;
  else pix += _skip;
  uint8_t co = _colorOrderMap.getPixelColorOrder(pix+_start, _colorOrder);
  if (_type == TYPE_WS2812_1CH_X3) { // map to correct IC, each controls 3 LEDs
    uint16_t pOld = pix;
    pix = IC_INDEX_WS2812_1CH_3X(pix);
    uint32_t cOld = PolyBus::getPixelColor(_busPtr, _iType, pix, co);
    switch (pOld % 3) { // change only the single channel (TODO: this can cause loss because of get/set)
      case 0: c = RGBW32(R(cOld), W(c)   , B(cOld), 0); break;
      case 1: c = RGBW32(W(c)   , G(cOld), B(cOld), 0); break;
      case 2: c = RGBW32(R(cOld), G(cOld), W(c)   , 0); break;
    }
  }
  PolyBus::setPixelColor(_busPtr, _iType, pix, c, co);
}

uint32_t BusDigital::getPixelColor(uint16_t pix) {
  if (reversed) pix = _len - pix -1;
  else pix += _skip;
  uint8_t co = _colorOrderMap.getPixelColorOrder(pix+_start, _colorOrder);
  if (_type == TYPE_WS2812_1CH_X3) { // map to correct IC, each controls 3 LEDs
    uint16_t pOld = pix;
    pix = IC_INDEX_WS2812_1CH_3X(pix);
    uint32_t c = PolyBus::getPixelColor(_busPtr, _iType, pix, co);
    switch (pOld % 3) { // get only the single channel
      case 0: c = RGBW32(G(c), G(c), G(c), G(c)); break;
      case 1: c = RGBW32(R(c), R(c), R(c), R(c)); break;
      case 2: c = RGBW32(B(c), B(c), B(c), B(c)); break;
    }
    return c;
  }
  return PolyBus::getPixelColor(_busPtr, _iType, pix, co);
}

uint8_t BusDigital::getPins(uint8_t* pinArray) {
  uint8_t numPins = IS_2PIN(_type) ? 2 : 1;
  for (uint8_t i = 0; i < numPins; i++) pinArray[i] = _pins[i];
  return numPins;
}

void BusDigital::setColorOrder(uint8_t colorOrder) {
  // upper nibble contains W swap information
  if ((colorOrder & 0x0F) > 5) return;
  _colorOrder = colorOrder;
}

void BusDigital::reinit() {
  PolyBus::begin(_busPtr, _iType, _pins);
}

void BusDigital::cleanup() {
  DEBUG_PRINTLN(F("Digital Cleanup."));
  PolyBus::cleanup(_busPtr, _iType);
  _iType = I_NONE;
  _valid = false;
  _busPtr = nullptr;
  pinManager.deallocatePin(_pins[1], PinOwner::BusDigital);
  pinManager.deallocatePin(_pins[0], PinOwner::BusDigital);
}


BusPwm::BusPwm(BusConfig &bc) : Bus(bc.type, bc.start, bc.autoWhite) {
  _valid = false;
  if (!IS_PWM(bc.type)) return;
  uint8_t numPins = NUM_PWM_PINS(bc.type);

  #ifdef ESP8266
  analogWriteRange(255);  //same range as one RGB channel
  analogWriteFreq(WLED_PWM_FREQ);
  #else
  _ledcStart = pinManager.allocateLedc(numPins);
  if (_ledcStart == 255) { //no more free LEDC channels
    deallocatePins(); return;
  }
  #endif

  for (uint8_t i = 0; i < numPins; i++) {
    uint8_t currentPin = bc.pins[i];
    if (!pinManager.allocatePin(currentPin, true, PinOwner::BusPwm)) {
    deallocatePins(); return;
    }
    _pins[i] = currentPin; //store only after allocatePin() succeeds
    #ifdef ESP8266
    pinMode(_pins[i], OUTPUT);
    #else
    ledcSetup(_ledcStart + i, WLED_PWM_FREQ, 8);
    ledcAttachPin(_pins[i], _ledcStart + i);
    #endif
  }
  reversed = bc.reversed;
  _valid = true;
}

void BusPwm::setPixelColor(uint16_t pix, uint32_t c) {
  if (pix != 0 || !_valid) return; //only react to first pixel
  if (_type != TYPE_ANALOG_3CH) c = autoWhiteCalc(c);
  if (_cct >= 1900 && (_type == TYPE_ANALOG_3CH || _type == TYPE_ANALOG_4CH)) {
    c = colorBalanceFromKelvin(_cct, c); //color correction from CCT
  }
  uint8_t r = R(c);
  uint8_t g = G(c);
  uint8_t b = B(c);
  uint8_t w = W(c);
  uint8_t cct = 0; //0 - full warm white, 255 - full cold white
  if (_cct > -1) {
    if (_cct >= 1900)    cct = (_cct - 1900) >> 5;
    else if (_cct < 256) cct = _cct;
  } else {
    cct = (approximateKelvinFromRGB(c) - 1900) >> 5;
  }

  uint8_t ww, cw;
  #ifdef WLED_USE_IC_CCT
  ww = w;
  cw = cct;
  #else
  //0 - linear (CCT 127 = 50% warm, 50% cold), 127 - additive CCT blending (CCT 127 = 100% warm, 100% cold)
  if (cct       < _cctBlend) ww = 255;
  else ww = ((255-cct) * 255) / (255 - _cctBlend);

  if ((255-cct) < _cctBlend) cw = 255;
  else                       cw = (cct * 255) / (255 - _cctBlend);

  ww = (w * ww) / 255; //brightness scaling
  cw = (w * cw) / 255;
  #endif

  switch (_type) {
    case TYPE_ANALOG_1CH: //one channel (white), relies on auto white calculation
      _data[0] = w;
      break;
    case TYPE_ANALOG_2CH: //warm white + cold white
      _data[1] = cw;
      _data[0] = ww;
      break;
    case TYPE_ANALOG_5CH: //RGB + warm white + cold white
      _data[4] = cw;
      w = ww;
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
  return RGBW32(_data[0], _data[1], _data[2], _data[3]);
}

void BusPwm::show() {
  if (!_valid) return;
  uint8_t numPins = NUM_PWM_PINS(_type);
  for (uint8_t i = 0; i < numPins; i++) {
    uint8_t scaled = (_data[i] * _bri) / 255;
    if (reversed) scaled = 255 - scaled;
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
  for (uint8_t i = 0; i < numPins; i++) {
    pinArray[i] = _pins[i];
  }
  return numPins;
}

void BusPwm::deallocatePins() {
  uint8_t numPins = NUM_PWM_PINS(_type);
  for (uint8_t i = 0; i < numPins; i++) {
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


BusOnOff::BusOnOff(BusConfig &bc) : Bus(bc.type, bc.start, bc.autoWhite) {
  _valid = false;
  if (bc.type != TYPE_ONOFF) return;

  uint8_t currentPin = bc.pins[0];
  if (!pinManager.allocatePin(currentPin, true, PinOwner::BusOnOff)) {
    return;
  }
  _pin = currentPin; //store only after allocatePin() succeeds
  pinMode(_pin, OUTPUT);
  reversed = bc.reversed;
  _valid = true;
}

void BusOnOff::setPixelColor(uint16_t pix, uint32_t c) {
  if (pix != 0 || !_valid) return; //only react to first pixel
  c = autoWhiteCalc(c);
  uint8_t r = R(c);
  uint8_t g = G(c);
  uint8_t b = B(c);
  uint8_t w = W(c);

  _data = bool((r+g+b+w) && _bri) ? 0xFF : 0;
}

uint32_t BusOnOff::getPixelColor(uint16_t pix) {
  if (!_valid) return 0;
  return RGBW32(_data, _data, _data, _data);
}

void BusOnOff::show() {
  if (!_valid) return;
  digitalWrite(_pin, reversed ? !(bool)_data : (bool)_data);
}

uint8_t BusOnOff::getPins(uint8_t* pinArray) {
  if (!_valid) return 0;
  pinArray[0] = _pin;
  return 1;
}


BusNetwork::BusNetwork(BusConfig &bc) : Bus(bc.type, bc.start, bc.autoWhite) {
  _valid = false;
  switch (bc.type) {
    case TYPE_NET_ARTNET_RGB:
      _rgbw = false;
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
  _data = (byte *)malloc(bc.count * _UDPchannels);
  if (_data == nullptr) return;
  memset(_data, 0, bc.count * _UDPchannels);
  _len = bc.count;
  _client = IPAddress(bc.pins[0],bc.pins[1],bc.pins[2],bc.pins[3]);
  _broadcastLock = false;
  _valid = true;
}

void BusNetwork::setPixelColor(uint16_t pix, uint32_t c) {
  if (!_valid || pix >= _len) return;
  if (hasWhite()) c = autoWhiteCalc(c);
  if (_cct >= 1900) c = colorBalanceFromKelvin(_cct, c); //color correction from CCT
  uint16_t offset = pix * _UDPchannels;
  _data[offset]   = R(c);
  _data[offset+1] = G(c);
  _data[offset+2] = B(c);
  if (_rgbw) _data[offset+3] = W(c);
}

uint32_t BusNetwork::getPixelColor(uint16_t pix) {
  if (!_valid || pix >= _len) return 0;
  uint16_t offset = pix * _UDPchannels;
  return RGBW32(_data[offset], _data[offset+1], _data[offset+2], _rgbw ? (_data[offset+3] << 24) : 0);
}

void BusNetwork::show() {
  if (!_valid || !canShow()) return;
  _broadcastLock = true;
  realtimeBroadcast(_UDPtype, _client, _len, _data, _bri, _rgbw);
  _broadcastLock = false;
}

uint8_t BusNetwork::getPins(uint8_t* pinArray) {
  for (uint8_t i = 0; i < 4; i++) {
    pinArray[i] = _client[i];
  }
  return 4;
}

void BusNetwork::cleanup() {
  _type = I_NONE;
  _valid = false;
  if (_data != nullptr) free(_data);
  _data = nullptr;
}


//utility to get the approx. memory usage of a given BusConfig
uint32_t BusManager::memUsage(BusConfig &bc) {
  uint8_t type = bc.type;
  uint16_t len = bc.count + bc.skipAmount;
  if (type > 15 && type < 32) {
    #ifdef ESP8266
      if (bc.pins[0] == 3) { //8266 DMA uses 5x the mem
        if (type > 29) return len*20; //RGBW
        return len*15;
      }
      if (type > 29) return len*4; //RGBW
      return len*3;
    #else //ESP32 RMT uses double buffer?
      if (type > 29) return len*8; //RGBW
      return len*6;
    #endif
  }
  if (type > 31 && type < 48)   return 5;
  if (type == 44 || type == 45) return len*4; //RGBW
  return len*3; //RGB
}

int BusManager::add(BusConfig &bc) {
  if (getNumBusses() - getNumVirtualBusses() >= WLED_MAX_BUSSES) return -1;
  if (bc.type >= TYPE_NET_DDP_RGB && bc.type < 96) {
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
  for (uint8_t i = 0; i < numBusses; i++) delete busses[i];
  numBusses = 0;
}

void BusManager::show() {
  for (uint8_t i = 0; i < numBusses; i++) {
    busses[i]->show();
  }
}

void BusManager::setStatusPixel(uint32_t c) {
  for (uint8_t i = 0; i < numBusses; i++) {
    busses[i]->setStatusPixel(c);
  }
}

void IRAM_ATTR BusManager::setPixelColor(uint16_t pix, uint32_t c, int16_t cct) {
  for (uint8_t i = 0; i < numBusses; i++) {
    Bus* b = busses[i];
    uint16_t bstart = b->getStart();
    if (pix < bstart || pix >= bstart + b->getLength()) continue;
    busses[i]->setPixelColor(pix - bstart, c);
  }
}

void BusManager::setBrightness(uint8_t b) {
  for (uint8_t i = 0; i < numBusses; i++) {
    busses[i]->setBrightness(b);
  }
}

void BusManager::setSegmentCCT(int16_t cct, bool allowWBCorrection) {
  if (cct > 255) cct = 255;
  if (cct >= 0) {
    //if white balance correction allowed, save as kelvin value instead of 0-255
    if (allowWBCorrection) cct = 1900 + (cct << 5);
  } else cct = -1;
  Bus::setCCT(cct);
}

uint32_t BusManager::getPixelColor(uint16_t pix) {
  for (uint8_t i = 0; i < numBusses; i++) {
    Bus* b = busses[i];
    uint16_t bstart = b->getStart();
    if (pix < bstart || pix >= bstart + b->getLength()) continue;
    return b->getPixelColor(pix - bstart);
  }
  return 0;
}

bool BusManager::canAllShow() {
  for (uint8_t i = 0; i < numBusses; i++) {
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
  for (uint8_t i=0; i<numBusses; i++) len += busses[i]->getLength();
  return len;
}

// Bus static member definition
int16_t Bus::_cct = -1;
uint8_t Bus::_cctBlend = 0;
uint8_t Bus::_gAWM = 255;
