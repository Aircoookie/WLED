#ifndef BusManager_h
#define BusManager_h

/*
 * Class for addressing various light types
 */

#include "wled.h"

class BusManager {
  public:
  BusManager() {

  };
  
  int add(uint8_t busType, uint8_t* pins, uint16_t len = 1) {
    if (numBusses >= WLED_MAX_BUSSES) return -1;
    if (IS_DIGITAL(busType)) {
      busses[numBusses] = new BusDigital(busType, pins, len);
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

  virtual void setBrightness(uint8_t b) {};

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

  protected:
  uint8_t _type = TYPE_NONE;
  uint16_t _start;
};

class BusDigital : public Bus {
  public:
  BusDigital(uint8_t type, uint8_t* pins, uint16_t len) : Bus(type) {
    if (!IS_DIGITAL(type)) return;
    _pins[0] = pins[0];
    if (IS_2PIN(type)) _pins[1] = pins[1];
    _len = len;
  };

  uint8_t getColorOrder() {
    return _colorOrder;
  }

  void setColorOrder(uint8_t colorOrder) {
    if (colorOrder > 5) return;
    _colorOrder = colorOrder;
  }

  private: 
  uint8_t _colorOrder = COL_ORDER_GRB;
  uint8_t _pins[2];
  uint16_t _len;
};

class BusPwm : public Bus {
  public:
  BusPwm(uint8_t type, uint8_t* pins) : Bus(type) {
    if (!IS_ANALOG(type)) return;
    uint8_t numPins = NUM_PWM_PINS(type);
    if (numPins == 0) numPins = 1;
    for (uint8_t i = 0; i < numPins; i++) {
      _pins[i] = pins[i];
    }
  };
  private: 
  uint8_t _pins[5];
  uint8_t _data[5];
};

#endif
