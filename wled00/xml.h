#ifndef WLED_XML_H
#define WLED_XML_H
#include <Arduino.h>
#include <ESPAsyncWebServer.h>

/*
 * Sending XML status files to client
 */
char* XML_response(AsyncWebServerRequest *request, char* dest = nullptr);
char* URL_response(AsyncWebServerRequest *request);
void sappend(char stype, const char* key, int val);
void sappends(char stype, const char* key, char* val);
void getSettingsJS(byte subPage, char* dest);

#endif // WLED_XML_H