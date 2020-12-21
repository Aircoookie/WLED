#include "wled.h"

/*
 * Alexa Voice On/Off/Brightness/Color Control. Emulates a Philips Hue bridge to Alexa.
 * 
 * This was put together from these two excellent projects:
 * https://github.com/kakopappa/arduino-esp8266-alexa-wemo-switch
 * https://github.com/probonopd/ESP8266HueEmulator
 */
#include "src/dependencies/espalexa/EspalexaDevice.h"

#ifndef WLED_DISABLE_ALEXA
void onAlexaChange(EspalexaDevice* dev);
void toggleFountain(uint8_t onOff);
void toogleSmoke(uint8_t onOff);
void toogleCrystal(uint8_t onOff);
void toogleSwitch4(uint8_t onOff);

void alexaInit()
{
  if (alexaEnabled && WLED_CONNECTED)
  {
    if (espalexaDevice == nullptr) //only init once
    {
      espalexaDevice = new EspalexaDevice(alexaInvocationName, onAlexaChange, EspalexaDeviceType::extendedcolor);
      espalexa.addDevice(espalexaDevice);  
      device2 = new EspalexaDevice(alexaD2, toggleFountain);
      espalexa.addDevice(device2);
      device3 = new EspalexaDevice(alexaD3, toogleSmoke);
      espalexa.addDevice(device3);
      device4 = new EspalexaDevice(alexaD4, toogleCrystal);
      espalexa.addDevice(device4);
      device5 = new EspalexaDevice(alexaD5, toogleSwitch4);
      espalexa.addDevice(device5);
      
      espalexa.begin(&server);
    } else {
      espalexaDevice->setName(alexaInvocationName);
      espalexaDevice->setName(alexaD2);
      espalexaDevice->setName(alexaD3);
      espalexaDevice->setName(alexaD4);
      espalexaDevice->setName(alexaD5);
    }
  }
}

void toggleFountain(uint8_t onOff)
{
  if (onOff)
  {
    DEBUG_PRINTLN("toogleFountain Received ON");
    digitalWrite(switch1, HIGH);
  } else {
    DEBUG_PRINTLN("toogleFountain Received OFF");
    digitalWrite(switch1, LOW);
  }

}

void toogleSmoke(uint8_t onOff)
{
  if (onOff)
  {
    DEBUG_PRINTLN("toogleSmoke Received ON");
    digitalWrite(switch2, HIGH);
  } else {
    DEBUG_PRINTLN("toogleSmoke Received OFF");
    digitalWrite(switch2, LOW);
  }
}

void toogleCrystal(uint8_t onOff)
{
  if (onOff)
  {
    DEBUG_PRINTLN("toogleCrystal Received ON");
    digitalWrite(switch3, HIGH);
  } else {
    DEBUG_PRINTLN("toogleCrystal Received OFF");
    digitalWrite(switch3, LOW);
  }
}

void toogleSwitch4(uint8_t onOff)
{
  if (onOff)
  {
    DEBUG_PRINTLN("toogleSwitch4 Received ON");
//    digitalWrite(switch4, HIGH);
  } else {
    DEBUG_PRINTLN("toogleSwitch4 Received OFF");
//    digitalWrite(switch4, LOW);
  }
}


void handleAlexa()
{
  if (!alexaEnabled || !WLED_CONNECTED) return;
  espalexa.loop();
}

void onAlexaChange(EspalexaDevice* dev)
{
  EspalexaDeviceProperty m = espalexaDevice->getLastChangedProperty();
  
  if (m == EspalexaDeviceProperty::on)
  {
    if (!macroAlexaOn)
    {
      if (bri == 0)
      {
        bri = briLast;
        colorUpdated(NOTIFIER_CALL_MODE_ALEXA);
      }
    } else applyPreset(macroAlexaOn);
  } else if (m == EspalexaDeviceProperty::off)
  {
    if (!macroAlexaOff)
    {
      if (bri > 0)
      {
        briLast = bri;
        bri = 0;
        colorUpdated(NOTIFIER_CALL_MODE_ALEXA);
      }
    } else applyPreset(macroAlexaOff);
  } else if (m == EspalexaDeviceProperty::bri)
  {
    bri = espalexaDevice->getValue();
    colorUpdated(NOTIFIER_CALL_MODE_ALEXA);
  } else //color
  {
    if (espalexaDevice->getColorMode() == EspalexaColorMode::ct) //shade of white
    {
      uint16_t ct = espalexaDevice->getCt();
      if (useRGBW)
      {
        switch (ct) { //these values empirically look good on RGBW
          case 199: col[0]=255; col[1]=255; col[2]=255; col[3]=255; break;
          case 234: col[0]=127; col[1]=127; col[2]=127; col[3]=255; break;
          case 284: col[0]=  0; col[1]=  0; col[2]=  0; col[3]=255; break;
          case 350: col[0]=130; col[1]= 90; col[2]=  0; col[3]=255; break;
          case 383: col[0]=255; col[1]=153; col[2]=  0; col[3]=255; break;
        }
      } else {
        colorCTtoRGB(ct, col);
      }
    } else {
      uint32_t color = espalexaDevice->getRGB();
    
      col[0] = ((color >> 16) & 0xFF);
      col[1] = ((color >>  8) & 0xFF);
      col[2] = ( color        & 0xFF);
      col[3] = 0;
    }
    colorUpdated(NOTIFIER_CALL_MODE_ALEXA);
  }
}


#else
 void alexaInit(){}
 void handleAlexa(){}
#endif
