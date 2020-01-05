/*
 * Alexa Voice On/Off/Brightness Control. Emulates a Philips Hue bridge to Alexa.
 * 
 * This was put together from these two excellent projects:
 * https://github.com/kakopappa/arduino-esp8266-alexa-wemo-switch
 * https://github.com/probonopd/ESP8266HueEmulator
 */

#ifndef WLED_DISABLE_ALEXA
void onAlexaChange(EspalexaDevice* dev);

void alexaInit()
{
  if (alexaEnabled && WLED_CONNECTED)
  {
    if (espalexaDevice == nullptr) //only init once
    {
      espalexaDevice = new EspalexaDevice(alexaInvocationName, onAlexaChange, EspalexaDeviceType::extendedcolor);
      espalexa.addDevice(espalexaDevice);
      espalexa.begin(&server);
    } else {
      espalexaDevice->setName(alexaInvocationName);
    }
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
        colorUpdated(10);
      }
    } else applyMacro(macroAlexaOn);
  } else if (m == EspalexaDeviceProperty::off)
  {
    if (!macroAlexaOff)
    {
      if (bri > 0)
      {
        briLast = bri;
        bri = 0;
        colorUpdated(10);
      }
    } else applyMacro(macroAlexaOff);
  } else if (m == EspalexaDeviceProperty::bri)
  {
    bri = espalexaDevice->getValue();
    colorUpdated(10);
  } else //color
  {
    uint32_t color = espalexaDevice->getRGB();
    col[3] = ((color >> 24) & 0xFF);  // white color from Alexa is "pure white only" 
    col[0] = ((color >> 16) & 0xFF);
    col[1] = ((color >>  8) & 0xFF);
    col[2] = (color & 0xFF);
    if (useRGBW && col[3] == 0) colorRGBtoRGBW(col);  // do not touch white value if EspalexaDevice.cpp did set the white channel
    colorUpdated(10);
  }
}


#else
 void alexaInit(){}
 void handleAlexa(){}
#endif
