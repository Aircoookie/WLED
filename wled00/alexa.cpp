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

void stripTurnOnOff(bool turnOn, bool revertLastKnownBrightness = true) {
  if (turnOn && bri == 0) {
    bri = revertLastKnownBrightness ? briLast : 255;
  }
  if(!turnOn && bri != 0) {
    briLast = bri;
    bri = 0;
  }
}

void turnOffAllAlexaDevices() {
  for (byte i = 0; i < espalexa.getDeviceCount(); i++) {
    espalexa.getDevice(i)->setValue(0);
  }
}

template<typename T>
void handleCT(EspalexaDevice* dev, T *stripOrSegment, bool hasCCT, bool hasWhite) {
  uint16_t ct = dev->getCt();
  if (!ct) return;

  byte rgbw[4];
  uint16_t k = 1000000 / ct; //mireds to kelvin

  if (hasCCT) {
    stripOrSegment->setCCT(k);
    if (hasWhite) {
      rgbw[0] = 0; rgbw[1] = 0; rgbw[2] = 0; rgbw[3] = 255;
    } else {
      rgbw[0] = 255; rgbw[1] = 255; rgbw[2] = 255; rgbw[3] = 0;
      dev->setValue(255);
    }
  } else if (hasWhite) {
    switch (ct) { //these values empirically look good on RGBW
      case 199: rgbw[0]=255; rgbw[1]=255; rgbw[2]=255; rgbw[3]=255; break;
      case 234: rgbw[0]=127; rgbw[1]=127; rgbw[2]=127; rgbw[3]=255; break;
      case 284: rgbw[0]=  0; rgbw[1]=  0; rgbw[2]=  0; rgbw[3]=255; break;
      case 350: rgbw[0]=130; rgbw[1]= 90; rgbw[2]=  0; rgbw[3]=255; break;
      case 383: rgbw[0]=255; rgbw[1]=153; rgbw[2]=  0; rgbw[3]=255; break;
      default : colorKtoRGB(k, rgbw);
    }
  } else {
    colorKtoRGB(k, rgbw);
  }

  stripOrSegment->setColor(0, RGBW32(rgbw[0], rgbw[1], rgbw[2], rgbw[3]));
}

void onStripChange(EspalexaDevice* dev) {
  EspalexaDeviceProperty m = dev->getLastChangedProperty();

  switch(m) {
    case EspalexaDeviceProperty::on:
      if(!macroAlexaOn) {
        stripTurnOnOff(true);
      } else {
        applyPreset(macroAlexaOn, CALL_MODE_ALEXA);
        if (bri == 0) dev->setValue(briLast); //stop Alexa from complaining if macroAlexaOn does not actually turn on
      }
      break;
    case EspalexaDeviceProperty::off:
      if(!macroAlexaOff) {
        stripTurnOnOff(false);
      } else {
        applyPreset(macroAlexaOff, CALL_MODE_ALEXA);
      }
      turnOffAllAlexaDevices(); // stops Alexa from complaining if macroAlexaOff does not actually turn off
      break;
    case EspalexaDeviceProperty::bri:
      bri = dev->getValue();
    break;
    case EspalexaDeviceProperty::ct: {
      bool hasManualWhite = strip.getActiveSegsLightCapabilities(true) & SEG_CAPABILITY_W;
      handleCT(dev, &strip, strip.hasCCTBus(), hasManualWhite);
      break;
    }
    default:
      strip.setColor(0, dev->getRGB());
      break;
  }
  
  stateUpdated(CALL_MODE_ALEXA);
}

void onSegmentChange(EspalexaDevice* dev, Segment *segment) {
  EspalexaDeviceProperty m = dev->getLastChangedProperty();

  switch(m) {
    case EspalexaDeviceProperty::on:
      segment->setOption(SEG_OPTION_ON, true);
      stripTurnOnOff(true, false);
      break;
    case EspalexaDeviceProperty::off:
      segment->setOption(SEG_OPTION_ON, false);
      break;
    case EspalexaDeviceProperty::bri:
      segment->setOpacity(dev->getValue());
    break;
    case EspalexaDeviceProperty::ct: {
      bool segmentHasCCT = segment->getLightCapabilities() & SEG_CAPABILITY_CCT;
      bool segmentHasWhite = segment->getLightCapabilities() & SEG_CAPABILITY_W;

      handleCT(dev, segment, segmentHasCCT, segmentHasWhite);
      break;
    }
    default:
      segment->setColor(0, dev->getRGB());
      break;
  }
  
  stateUpdated(CALL_MODE_ALEXA);
}

void onPresetChange(EspalexaDevice *dev, byte presetIndex) {
  EspalexaDeviceProperty m = dev->getLastChangedProperty();
  
  switch(m) {
    case EspalexaDeviceProperty::on:
      // turn off other preset devices
      for (byte i = 1; i < espalexa.getDeviceCount(); i++)
      {
        if (i == dev->getId()) continue;
        espalexa.getDevice(i)->setValue(0); // turn off other presets
      }

      applyPreset(dev->getId(), CALL_MODE_ALEXA); // in alexaInit() preset 1 device was added second (index 1), preset 2 third (index 2) etc.
      break;
    default:
      onStripChange(dev);
      break;
  }

  stateUpdated(CALL_MODE_ALEXA);
}

void initAlexaForPresets() {
  if(!alexaNumPresets) return;
  // up to 9 devices (added second, third, ... i.e. index 1 to 9) serve for switching on up to nine presets (preset IDs 1 to 9 in WLED),
  // names are identical as the preset names, switching off can be done by switching off any of them
  String name = "";
  for (byte presetIndex = 1; presetIndex <= alexaNumPresets; presetIndex++) {
    if (!getPresetName(presetIndex, name)) break; // no more presets
    espalexa.addDevice(new EspalexaDevice(name.c_str(), [name, presetIndex](EspalexaDevice* dev) { onPresetChange(dev, presetIndex); }, EspalexaDeviceType::extendedcolor));
  }
}

void initAlexaForStrip() {
  espalexaDevice = new EspalexaDevice(alexaInvocationName, [](EspalexaDevice* dev) { onStripChange(dev); }, EspalexaDeviceType::extendedcolor);
  espalexa.addDevice(espalexaDevice);
}
 
void initAlexaForSegments() {
  if(strip.getSegmentsNum() < 2) return;
  for(uint8_t i = 0; i < strip.getSegmentsNum(); i++) {
    String name = "Segment " + String(i);

    espalexa.addDevice(new EspalexaDevice(name.c_str(), [i](EspalexaDevice* dev) { 
      Segment &segment = strip.getSegment(i);
      onSegmentChange(dev, &segment); 
    }, EspalexaDeviceType::extendedcolor));
  }
}
  
void alexaInit()
{
  if (!alexaEnabled || !WLED_CONNECTED) return;

  espalexa.removeAllDevices();
  
  initAlexaForStrip();
  initAlexaForSegments();
  initAlexaForPresets();

  espalexa.begin(&server);
}

void handleAlexa()
{
  if (!alexaEnabled || !WLED_CONNECTED) return;
  espalexa.loop();
}


#else
 void alexaInit(){}
 void handleAlexa(){}
#endif
