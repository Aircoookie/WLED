//EspalexaDevice Class

#include "EspalexaDevice.h"

EspalexaDevice::EspalexaDevice(){}

EspalexaDevice::EspalexaDevice(String deviceName, CallbackBriFunction gnCallback, uint8_t initialValue) { //constructor
  
  _deviceName = deviceName;
  _callback = gnCallback;
  _val = initialValue;
  _val_last = _val;
}

EspalexaDevice::EspalexaDevice(String deviceName, CallbackColFunction gnCallback, uint8_t initialValue) { //constructor for color device
  
  _deviceName = deviceName;
  _callbackCol = gnCallback;
  _callback = nullptr;
  _val = initialValue;
  _val_last = _val;
}

EspalexaDevice::~EspalexaDevice(){/*nothing to destruct*/}

bool EspalexaDevice::isColorDevice()
{
  //if brightness-only callback is null, we have color device
  return (_callback == nullptr);
}

bool EspalexaDevice::isColorTemperatureMode()
{
  return _ct;
}

String EspalexaDevice::getName()
{
  return _deviceName;
}

uint8_t EspalexaDevice::getLastChangedProperty()
{
  return _changed;
}

uint8_t EspalexaDevice::getValue()
{
  return _val;
}

uint16_t EspalexaDevice::getHue()
{
  return _hue;
}

uint8_t EspalexaDevice::getSat()
{
  return _sat;
}

uint16_t EspalexaDevice::getCt()
{
  if (_ct == 0) return 500;
  return _ct;
}

uint32_t EspalexaDevice::getColorRGB()
{
  uint8_t rgb[3];
  
  if (isColorTemperatureMode())
  {
    //TODO tweak a bit to match hue lamp characteristics
    //based on https://gist.github.com/paulkaplan/5184275
    float temp = 10000/ _ct; //kelvins = 1,000,000/mired (and that /100)
    float r, g, b;

    if( temp <= 66 ){ 
      r = 255; 
      g = temp;
      g = 99.470802 * log(g) - 161.119568;
      if( temp <= 19){
          b = 0;
      } else {
          b = temp-10;
          b = 138.517731 * log(b) - 305.044793;
      }
    } else {
      r = temp - 60;
      r = 329.698727 * pow(r, -0.13320476);
      g = temp - 60;
      g = 288.12217 * pow(g, -0.07551485 );
      b = 255;
    }
    
    rgb[0] = (byte)constrain(r,0.1,255.1);
    rgb[1] = (byte)constrain(g,0.1,255.1);
    rgb[2] = (byte)constrain(b,0.1,255.1);
  } 
  else
  { //hue + sat mode
    float h = ((float)_hue)/65535.0;
    float s = ((float)_sat)/255.0;
    byte i = floor(h*6);
    float f = h * 6-i;
    float p = 255 * (1-s);
    float q = 255 * (1-f*s);
    float t = 255 * (1-(1-f)*s);
    switch (i%6) {
      case 0: rgb[0]=255,rgb[1]=t,rgb[2]=p;break;
      case 1: rgb[0]=q,rgb[1]=255,rgb[2]=p;break;
      case 2: rgb[0]=p,rgb[1]=255,rgb[2]=t;break;
      case 3: rgb[0]=p,rgb[1]=q,rgb[2]=255;break;
      case 4: rgb[0]=t,rgb[1]=p,rgb[2]=255;break;
      case 5: rgb[0]=255,rgb[1]=p,rgb[2]=q;
    }
  }
  return ((rgb[0] << 16) | (rgb[1] << 8) | (rgb[2]));
}

uint8_t EspalexaDevice::getLastValue()
{
  if (_val_last == 0) return 255;
  return _val_last;
}

void EspalexaDevice::setPropertyChanged(uint8_t p)
{
  //0: initial 1: on 2: off 3: bri 4: col 5: ct
  _changed = p;
}

//you need to re-discover the device for the Alexa name to change
void EspalexaDevice::setName(String name)
{
  _deviceName = name;
}

void EspalexaDevice::setValue(uint8_t val)
{
  if (_val != 0)
  {
    _val_last = _val;
  }
  if (val != 0)
  {
    _val_last = val;
  }
  _val = val;
}

void EspalexaDevice::setPercent(uint8_t perc)
{
  uint16_t val = perc * 255;
  val /= 100;
  if (val > 255) val = 255;
  setValue(val);
}

void EspalexaDevice::setColor(uint16_t hue, uint8_t sat)
{
  _hue = hue;
  _sat = sat;
  _ct = 0;
}

void EspalexaDevice::setColor(uint16_t ct)
{
  _ct = ct;
}

void EspalexaDevice::doCallback()
{
  (_callback != nullptr) ? _callback(_val) : _callbackCol(_val, getColorRGB());
}