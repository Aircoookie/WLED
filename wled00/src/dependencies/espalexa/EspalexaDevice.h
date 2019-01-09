#ifndef EspalexaDevice_h
#define EspalexaDevice_h

#include "Arduino.h"

typedef void (*CallbackBriFunction) (uint8_t br);
typedef void (*CallbackColFunction) (uint8_t br, uint32_t col);

class EspalexaDevice {
private:
  String _deviceName;
  CallbackBriFunction _callback;
  CallbackColFunction _callbackCol;
  uint8_t _val, _val_last, _sat = 0;
  uint16_t _hue = 0, _ct = 0;
  uint8_t _changed = 0;
  
public:
  EspalexaDevice();
  ~EspalexaDevice();
  EspalexaDevice(String deviceName, CallbackBriFunction gnCallback, uint8_t initialValue =0);
  EspalexaDevice(String deviceName, CallbackColFunction gnCallback, uint8_t initialValue =0);
  
  bool isColorDevice();
  bool isColorTemperatureMode();
  String getName();
  uint8_t getLastChangedProperty();
  uint8_t getValue();
  uint16_t getHue();
  uint8_t getSat();
  uint16_t getCt();
  uint32_t getColorRGB();
  
  void setPropertyChanged(uint8_t p);
  void setValue(uint8_t bri);
  void setPercent(uint8_t perc);
  void setName(String name);
  void setColor(uint16_t hue, uint8_t sat);
  void setColor(uint16_t ct);
  
  void doCallback();
  
  uint8_t getLastValue(); //last value that was not off (1-255)
};

#endif