#ifndef WLED_ALEXA_H
#define WLED_ALEXA_H
/*
 * Alexa Voice On/Off/Brightness Control. Emulates a Philips Hue bridge to Alexa.
 * 
 * This was put together from these two excellent projects:
 * https://github.com/kakopappa/arduino-esp8266-alexa-wemo-switch
 * https://github.com/probonopd/ESP8266HueEmulator
 */
#include "src/dependencies/espalexa/EspalexaDevice.h"

void onAlexaChange(EspalexaDevice* dev);
void alexaInit();
void handleAlexa();
void onAlexaChange(EspalexaDevice* dev);

#endif // WLED_ALEXA_H