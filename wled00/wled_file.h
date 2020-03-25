#ifndef WLED_FILE_H
#define WLED_FILE_H
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
/*
 * Utility for SPIFFS filesystem & Serial console
 */

void handleSerial();
bool handleFileRead(AsyncWebServerRequest*, String path);

#endif // WLED_FILE_H