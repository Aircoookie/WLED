#include "pin_manager.h"
#include "wled.h"

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

#ifdef ARDUINO_ARCH_ESP32
byte PinManagerClass::allocateLedc(byte channels)
{
  if (channels > 16 || channels == 0) return 255;
  byte ca = 0;
  for (byte i = 0; i < 16; i++) {
    byte by = i >> 3;
    byte bi = i - 8*by;
    if (bitRead(ledcAlloc[by], bi)) { //found occupied pin
      ca = 0;
    } else {
      ca++;
    }
    if (ca >= channels) { //enough free channels
      byte in = (i + 1) - ca;
      for (byte j = 0; j < ca; j++) {
        byte b = in + j;
        byte by = b >> 3;
        byte bi = b - 8*by;
        bitWrite(ledcAlloc[by], bi, true);
      }
      return in;
    }
  }
  return 255; //not enough consecutive free LEDC channels
}

void PinManagerClass::deallocateLedc(byte pos, byte channels)
{
  for (byte j = pos; j < pos + channels; j++) {
    if (j > 16) return;
    byte by = j >> 3;
    byte bi = j - 8*by;
    bitWrite(ledcAlloc[by], bi, false);
  }
}
#endif

PinManagerClass pinManager = PinManagerClass();