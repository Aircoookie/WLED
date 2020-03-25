#ifndef WLED_JSON_H
#define WLED_JSON_H
/*
 * JSON API (De)serialization
 */
#include <Arduino.h>
#include "ESPAsyncWebServer.h"
#include "src/dependencies/json/ArduinoJson-v6.h"
#include "src/dependencies/json/AsyncJson-v6.h"
#include "fx.h"
// TODO: AsynicWebServerRequest conflict?

void deserializeSegment(JsonObject elem, byte it);
bool deserializeState(JsonObject root);
void serializeSegment(JsonObject& root, WS2812FX::Segment& seg, byte id);
void serializeState(JsonObject root);
void serializeInfo(JsonObject root);
void serveJson(AsyncWebServerRequest* request);
void serveLiveLeds(AsyncWebServerRequest* request);

#endif //WLED_JSON_H