#ifndef BusManager_h
#define BusManager_h

/*
 * Class for addressing various light types
 */

#include "wled.h"
#include "bus_wrapper.h"

class BusManager {
  public:
  BusManager() {

  };
  
  int add(uint8_t busType, uint8_t* pins, uint16_t len = 1) {
    if (numBusses >= WLED_MAX_BUSSES) return -1;
    if (IS_DIGITAL(busType)) {
      busses[numBusses] = new BusDigital(busType, pins, len, numBusses);
    } else {
      busses[numBusses] = new BusPwm(busType, pins);
    }
    numBusses++;
    return numBusses -1;
  }

  void removeAll() {
    for (uint8_t i = 0; i < numBusses; i++) delete busses[i];
    numBusses = 0;
  }
  //void remove(uint8_t id);

  void show() {
    for (uint8_t i = 0; i < numBusses; i++) {
      busses[i]->show();
    }
  }

  void setPixelColor(uint16_t pix, uint32_t c) {
    for (uint8_t i = 0; i < numBusses; i++) {
      Bus* b = busses[i];
      uint16_t bstart = b->getStart();
      if (pix < bstart) continue;
      busses[i]->setPixelColor(pix - bstart, c);
    }
  }

  void setBrightness(uint8_t b) {
    for (uint8_t i = 0; i < numBusses; i++) {
      busses[i]->setBrightness(b);
    }
  }

  private:
  uint8_t numBusses = 0;
  Bus* busses[WLED_MAX_BUSSES];
};

//parent class of BusDigital and BusPwm
class Bus {
  public:
  Bus(uint8_t type) {
    _type = type;
  };
  
  virtual void show() {}

  virtual void setPixelColor(uint16_t pix, uint32_t c) {};

  virtual void setBrightness(uint8_t b) { _bri = b; };

  virtual uint32_t getPixelColor(uint16_t pix) { return 0; };

  virtual ~Bus() { //throw the bus under the bus
    
  }

  uint16_t getStart() {
    return _start;
  }

  void setStart(uint16_t start) {
    _start = start;
  }

  virtual uint8_t getColorOrder() {
    return COL_ORDER_RGB;
  }

  virtual void setColorOrder() {}

  uint8_t getType() {
    return _type;
  }

  bool isOk() {
    return _valid;
  }

  protected:
  uint8_t _type = TYPE_NONE;
  uint8_t _bri = 255;
  uint16_t _start;
  bool _valid = false;
};


class BusDigital : public Bus {
  public:
  BusDigital(uint8_t type, uint8_t* pins, uint16_t len, uint8_t nr) : Bus(type) {
    if (!IS_DIGITAL(type) || !len) return;
    _pins[0] = pins[0];
    if (IS_2PIN(type)) _pins[1] = pins[1];
    _len = len;
    _iType = PolyBus::getI(type, _pins, nr);
    if (_iType == I_NONE) return;
    _valid = true;
  };

  void show() {
    PolyBus::show(_busPtr, _iType);
  }

  void setPixelColor(uint16_t pix, uint32_t c) {

  }

  uint8_t getColorOrder() {
    return _colorOrder;
  }

  void setColorOrder(uint8_t colorOrder) {
    if (colorOrder > 5) return;
    _colorOrder = colorOrder;
  }

  private: 
  uint8_t _colorOrder = COL_ORDER_GRB;
  uint8_t _pins[2] = {255, 255};
  uint8_t _iType = I_NONE;
  uint16_t _len = 0;
  void * _busPtr = nullptr;
};


class BusPwm : public Bus {
  public:
  BusPwm(uint8_t type, uint8_t* pins) : Bus(type) {
    if (!IS_PWM(type)) return;
    uint8_t numPins = NUM_PWM_PINS(type);

    #ifdef ESP8266
    analogWriteRange(255);  //same range as one RGB channel
    analogWriteFreq(WLED_PWM_FREQ_ESP8266);
    #else
    _ledcStart = pinManager.allocateLedc(numPins);
    if (_ledcStart == 255) { //no more free LEDC channels
      deallocatePins(); return;
    }
    #endif

    for (uint8_t i = 0; i < numPins; i++) {
      _pins[i] = pins[i];
      if (!pinManager.allocatePin(_pins[i])) {
        deallocatePins(); return;
      }
      #ifdef ESP8266
      pinMode(_pins[i], OUTPUT);
      #else
      ledcSetup(_ledcStart + i, WLED_PWM_FREQ_ESP32, 8);
      ledcAttachPin(_pins[i], _ledcStart + i);
      #endif
    }

    _valid = true;
  };

  void setPixelColor(uint16_t pix, uint32_t c) {
    if (pix != 0 || !_valid) return; //only react to first pixel
    uint8_t r = c >> 16;
    uint8_t g = c >>  8;
    uint8_t b = c      ;
    uint8_t w = c >> 24;

    switch (_type) {
      case TYPE_ANALOG_1CH: //one channel (white), use highest RGBW value
        _data[0] = max(r, max(g, max(b, w))); break;
      
      case TYPE_ANALOG_2CH: //warm white + cold white, we'll need some nice handling here, for now just R+G channels
      case TYPE_ANALOG_3CH: //standard dumb RGB
      case TYPE_ANALOG_4CH: //RGBW
      case TYPE_ANALOG_5CH: //we'll want the white handling from 2CH here + RGB
        _data[0] = r; _data[1] = g; _data[2] = b; _data[3] = w; _data[4] = 0; break;

      default: return;
    }
  }

  //does no index check
  uint32_t getPixelColor(uint16_t pix) {
    return ((_data[3] << 24) | (_data[0] << 16) | (_data[1] << 8) | (_data[2]));
  }

  void show() {
    uint8_t numPins = NUM_PWM_PINS(_type);
    for (uint8_t i = 0; i < numPins; i++) {
      uint8_t scaled = (_data[i] * _bri) / 255;
      #ifdef ESP8266
      analogWrite(_pins[i], scaled);
      #else
      ledcWrite(_ledcStart + i, scaled);
      #endif
    }
  }

  ~BusPwm() {
    deallocatePins();
  };

  private: 
  uint8_t _pins[5];
  uint8_t _data[5] = {255, 255, 255, 255, 255};
  #ifdef ARDUINO_ARCH_ESP32
  uint8_t _ledcStart = 255;
  #endif

  void deallocatePins() {
    uint8_t numPins = NUM_PWM_PINS(_type);
    for (uint8_t i = 0; i < numPins; i++) {
      if (!pinManager.isPinOk(_pins[i])) continue;
      #ifdef ESP8266
      digitalWrite(_pins[i], LOW); //turn off PWM interrupt
      #else
      if (_ledcStart < 16) ledcDetachPin(_pins[i], _ledcStart + i);
      #endif
      pinManager.deallocatePin(_pins[i]);
    }
    #ifdef ARDUINO_ARCH_ESP32
    pinManager.deallocateLedc(_ledcStart, numPins);
    #endif
  }
};

#endif
