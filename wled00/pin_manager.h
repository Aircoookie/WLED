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

#if DEBUG
  // Allocates a single pin.
  #define ALLOCATE_PIN(gpio, output) pinManager._allocatePinDebug(gpio, output, __FILE__, __LINE__)
  // Allocates all pins, or none of the pins, in the array
  // This should simplify error condition handling in clients
  // that need more than one pin to work correctly, such as
  // ethernet, rotary encoders, and the like.
  #define ALLOCATE_MULTIPLE_PINS(mptArray,arrayElementCount) pinManager._allocateMultiplePinsDebug(mptArray, arrayElementCount, __FILE__, __LINE__)
#else
  // Allocates a single pin.
  #define ALLOCATE_PIN(gpio, output) pinManager._allocatePin(gpio, output)
  // Allocates all pins, or none of the pins, in the array
  // This should simplify error condition handling in clients
  // that need more than one pin to work correctly, such as
  // ethernet, rotary encoders, and the like.
  #define ALLOCATE_MULTIPLE_PINS(mptArray,arrayElementCount) pinManager._allocateMultiplePins(mptArray, arrayElementCount)
#endif

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

  bool _allocatePinDebug(byte gpio, bool output, char const * file, int line);
  bool _allocateMultiplePinsDebug(const managed_pin_type * mptArray, byte arrayElementCount, char const * file, int line);
  bool _allocatePin(byte gpio, bool output);
  
  [[deprecated("Replaced by macro ALLOCATE_PIN(gpio, output), for improved debugging")]]
  inline bool allocatePin(byte gpio, bool output = true) { return _allocatePin(gpio, output); }
  bool isPinAllocated(byte gpio);
  bool isPinOk(byte gpio, bool output = true);
  #ifdef ARDUINO_ARCH_ESP32
  byte allocateLedc(byte channels);
  void deallocateLedc(byte pos, byte channels);
  #endif
  bool _allocateMultiplePins(const managed_pin_type * mpt, byte arrayElementCount);
  inline void deallocatePin(managed_pin_type mpt)
  {
    deallocatePin(mpt.pin);
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