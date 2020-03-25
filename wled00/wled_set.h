#ifndef WLED_SET_H
#define WLED_SET_H
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
/*
 * Receives client input
 */

void _setRandomColor(bool _sec,bool fromButton=false);
bool isAsterisksOnly(const char* str, byte maxLen);
void handleSettingsSet(AsyncWebServerRequest *request, byte subPage);
bool handleSet(AsyncWebServerRequest *request, const String& req);
int getNumVal(const String* req, uint16_t pos);
bool updateVal(const String* req, const char* key, byte* val, byte minv=0, byte maxv=255);

#endif // WLED_SET_H