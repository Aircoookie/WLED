#include "wled.h"

/*
 * Registers pins so there is no attempt for two interfaces to use the same pin
 */

void PinManagerClass::deallocatePin(byte gpio)
{
  if (!isPinOk(gpio, false)) return;
  
  byte by = gpio >> 3;
  byte bi = gpio - 8*by;
  bitWrite(pinAlloc[by], bi, false);
}

bool PinManagerClass::allocatePin(byte gpio, bool output)
{
  if (!isPinOk(gpio, output)) return false;
  if (isPinAllocated(gpio)) {
    DEBUG_PRINT(F("Attempted duplicate allocation of pin "));
    DEBUG_PRINTLN(gpio);
    return false;
  }

  byte by = gpio >> 3;
  byte bi = gpio - 8*by;
  bitWrite(pinAlloc[by], bi, true);
  
  return true;
}

bool PinManagerClass::isPinAllocated(byte gpio)
{
  if (!isPinOk(gpio, false)) return true;
  
  byte by = gpio >> 3;
  byte bi = gpio - 8*by;
  return bitRead(pinAlloc[by], bi);
}

bool PinManagerClass::isPinOk(byte gpio, bool output)
{
  if (gpio <  6) return  true;
  if (gpio < 12) return false; //SPI flash pins
  
  #ifdef ESP8266
  if (gpio < 17) return true;
  #else //ESP32
  if (gpio < 34) return true;
  if (gpio < 40 && !output) return true; //34-39 input only
  #endif
  
  return false;
}