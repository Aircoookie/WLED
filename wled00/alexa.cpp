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

void alexaInit()
{
  if (alexaEnabled && WLED_CONNECTED)
  {
    if (espalexa.getDeviceCount() == 0) //only init once
    {
      // the original configured device for keeping old behavior
      espalexaDevice = new EspalexaDevice(alexaInvocationName, onAlexaChange, EspalexaDeviceType::extendedcolor);
      espalexa.addDevice(espalexaDevice);
      // 9 further devices with names Effect 1-9 (in German) for switching presets 1-9 on (off and other commands will work like before)
      // switch on e.g. by saying (in German): "Alexa, Effekt eins anschalten"
      espalexaDevice = new EspalexaDevice("Effekt eins", onAlexaChange, EspalexaDeviceType::extendedcolor);
      espalexa.addDevice(espalexaDevice);
      espalexaDevice = new EspalexaDevice("Effekt zwei", onAlexaChange, EspalexaDeviceType::extendedcolor);
      espalexa.addDevice(espalexaDevice);
      espalexaDevice = new EspalexaDevice("Effekt drei", onAlexaChange, EspalexaDeviceType::extendedcolor);
      espalexa.addDevice(espalexaDevice);
      espalexaDevice = new EspalexaDevice("Effekt vier", onAlexaChange, EspalexaDeviceType::extendedcolor);
      espalexa.addDevice(espalexaDevice);
      espalexaDevice = new EspalexaDevice("Effekt fünf", onAlexaChange, EspalexaDeviceType::extendedcolor);
      espalexa.addDevice(espalexaDevice);
      espalexaDevice = new EspalexaDevice("Effekt sechs", onAlexaChange, EspalexaDeviceType::extendedcolor);
      espalexa.addDevice(espalexaDevice);
      espalexaDevice = new EspalexaDevice("Effekt sieben", onAlexaChange, EspalexaDeviceType::extendedcolor);
      espalexa.addDevice(espalexaDevice);
      espalexaDevice = new EspalexaDevice("Effekt acht", onAlexaChange, EspalexaDeviceType::extendedcolor);
      espalexa.addDevice(espalexaDevice);
      espalexaDevice = new EspalexaDevice("Effekt neun", onAlexaChange, EspalexaDeviceType::extendedcolor);
      espalexa.addDevice(espalexaDevice);
      espalexa.begin(&server);
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
  espalexaDevice = dev;
  EspalexaDeviceProperty m = espalexaDevice->getLastChangedProperty();
  String name = espalexaDevice->getName();
  
  if (m == EspalexaDeviceProperty::on)
  {
    if (name == alexaInvocationName) 
    {
      // keep the old switch-on behavior for the configured name
      if (!macroAlexaOn)
      {
        if (bri == 0)
        {
          bri = briLast;
          colorUpdated(CALL_MODE_ALEXA);
        }
      } else {
        applyPreset(macroAlexaOn, CALL_MODE_ALEXA);
        if (bri == 0) espalexaDevice->setValue(briLast); //stop Alexa from complaining if macroAlexaOn does not actually turn on
      }
    } else {
      // new switch-on behavior for devices 1-9 which switch on presets 1-9
      byte preset = 0;
      if (name == "Effekt eins") preset = 1;
      else if (name == "Effekt zwei") preset = 2;
      else if (name == "Effekt drei") preset = 3;
      else if (name == "Effekt vier") preset = 4;
      else if (name == "Effekt fünf") preset = 5;
      else if (name == "Effekt sechs") preset = 6;
      else if (name == "Effekt sieben") preset = 7;
      else if (name == "Effekt acht") preset = 8;
      else if (name == "Effekt neun") preset = 9;
      applyPreset(preset,CALL_MODE_ALEXA);
    }
  } else if (m == EspalexaDeviceProperty::off)
  {
    if (!macroAlexaOff)
    {
      if (bri > 0)
      {
        briLast = bri;
        bri = 0;
        colorUpdated(CALL_MODE_ALEXA);
      }
    } else {
      applyPreset(macroAlexaOff, CALL_MODE_ALEXA);
      if (bri != 0) espalexaDevice->setValue(0); //stop Alexa from complaining if macroAlexaOff does not actually turn off
    }
  } else if (m == EspalexaDeviceProperty::bri)
  {
    bri = espalexaDevice->getValue();
    colorUpdated(CALL_MODE_ALEXA);
  } else //color
  {
    if (espalexaDevice->getColorMode() == EspalexaColorMode::ct) //shade of white
    {
      uint16_t ct = espalexaDevice->getCt();
			if (!ct) return;
			uint16_t k = 1000000 / ct; //mireds to kelvin
			
			if (strip.hasCCTBus()) {
				uint8_t segid = strip.getMainSegmentId();
				WS2812FX::Segment& seg = strip.getSegment(segid);
				uint8_t cctPrev = seg.cct;
				seg.setCCT(k, segid);
				if (seg.cct != cctPrev) effectChanged = true; //send UDP
				col[0]= 0; col[1]= 0; col[2]= 0; col[3]= 255;
			} else if (strip.isRgbw) {
        switch (ct) { //these values empirically look good on RGBW
          case 199: col[0]=255; col[1]=255; col[2]=255; col[3]=255; break;
          case 234: col[0]=127; col[1]=127; col[2]=127; col[3]=255; break;
          case 284: col[0]=  0; col[1]=  0; col[2]=  0; col[3]=255; break;
          case 350: col[0]=130; col[1]= 90; col[2]=  0; col[3]=255; break;
          case 383: col[0]=255; col[1]=153; col[2]=  0; col[3]=255; break;
					default : colorKtoRGB(k, col);
        }
      } else {
        colorKtoRGB(k, col);
      }
    } else {
      uint32_t color = espalexaDevice->getRGB();
    
      col[0] = ((color >> 16) & 0xFF);
      col[1] = ((color >>  8) & 0xFF);
      col[2] = ( color        & 0xFF);
      col[3] = 0;
    }
    colorUpdated(CALL_MODE_ALEXA);
  }
}


#else
 void alexaInit(){}
 void handleAlexa(){}
#endif
