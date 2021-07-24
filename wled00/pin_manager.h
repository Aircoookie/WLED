#ifndef WLED_PIN_MANAGER_H
#define WLED_PIN_MANAGER_H
/*
 * Registers pins so there is no attempt for two interfaces to use the same pin
 */
#include <Arduino.h>

typedef struct PinManagerPinType {
  byte pin;
  byte isOutput;
} managed_pin_type;

class PinManagerClass {
  private:
  #ifdef ESP8266
  uint8_t pinAlloc[3] = {0x00, 0x00, 0x00}; //24bit, 1 bit per pin, we use first 17bits
  #else
  uint8_t pinAlloc[5] = {0x00, 0x00, 0x00, 0x00, 0x00}; //40bit, 1 bit per pin, we use all bits
  uint8_t ledcAlloc[2] = {0x00, 0x00}; //16 LEDC channels
  #endif

  public:
  void deallocatePin(byte gpio);
  bool allocatePin(byte gpio, bool output = true);
  bool isPinAllocated(byte gpio);
  bool isPinOk(byte gpio, bool output = true);
  #ifdef ARDUINO_ARCH_ESP32
  byte allocateLedc(byte channels);
  void deallocateLedc(byte pos, byte channels);
  #endif
  bool allocateMultiplePins(const managed_pin_type * mpt, byte arrayElementCount);
  inline void deallocatePin(managed_pin_type mpt)
  {
    deallocatePin(mpt.pin);
  }
  inline bool allocatePin(managed_pin_type mpt)
  {
    return allocatePin(mpt.pin, mpt.isOutput);
  }
  inline bool isPinAllocated(managed_pin_type mpt)
  {
    return isPinAllocated(mpt.pin);
  }
  inline bool isPinOk(managed_pin_type mpt)
  {
    return isPinOk(mpt.pin);
  }
};

extern PinManagerClass pinManager;
#endif