/*
 * Alexa Voice On/Off/Brightness Control. Emulates a Philips Hue bridge to Alexa.
 * 
 * This was put together from these two excellent projects:
 * https://github.com/kakopappa/arduino-esp8266-alexa-wemo-switch
 * https://github.com/probonopd/ESP8266HueEmulator
 */
void prepareIds() {
  escapedMac = WiFi.macAddress();
  escapedMac.replace(":", "");
  escapedMac.toLowerCase();
}

#ifndef WLED_DISABLE_ALEXA
void onAlexaChange(byte b, uint32_t color);

void alexaInit()
{
  if (alexaEnabled && WiFi.status() == WL_CONNECTED)
  {
    if (espalexaDevice == nullptr) //only init once
    {
      espalexaDevice = new EspalexaDevice(alexaInvocationName, onAlexaChange);
      espalexa.addDevice(espalexaDevice);
      espalexa.begin(&server);
    } else {
      espalexaDevice->setName(alexaInvocationName);
    }
  }
}

void handleAlexa()
{
  if (!alexaEnabled || WiFi.status() != WL_CONNECTED) return;
  espalexa.loop();
}

void onAlexaChange(byte b, uint32_t color)
{
  byte m = espalexaDevice->getLastChangedProperty();
  
  if (m == 1){ //ON
    if (!macroAlexaOn)
    {
      if (bri == 0)
      {
        bri = briLast;
        colorUpdated(10);
      }
    } else applyMacro(macroAlexaOn);
  } else if (m == 2) //OFF
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
  } else if (m == 3) //brightness
  {
    bri = b;
    colorUpdated(10);
  } else //color
  {
    col[0] = ((color >> 16) & 0xFF);
    col[1] = ((color >>  8) & 0xFF);
    col[2] = (color & 0xFF);
    if (useRGBW) colorRGBtoRGBW(col);
    colorUpdated(10);
  }
}


#else
 void alexaInit(){}
 void handleAlexa(){}
#endif
