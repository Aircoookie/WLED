#ifndef WLED_SERVER_H
#define WLED_SERVER_H
#include <Arduino.h>
/*
 * Server page declarations
 */
class AsyncWebServerRequest;


bool isIp(String str);
bool captivePortal(AsyncWebServerRequest *request);
void initServer();
void serveIndexOrWelcome(AsyncWebServerRequest *request);
void serveIndex(AsyncWebServerRequest* request);
String msgProcessor(const String& var);
void serveMessage(AsyncWebServerRequest* request, uint16_t code, String headl, String subl="", byte optionT=255);
String settingsProcessor(const String& var);
String dmxProcessor(const String& var);
void serveSettings(AsyncWebServerRequest* request);

#endif //WLED_SERVER_H