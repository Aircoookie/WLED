/*
 * JSON API (De)serialization
 */

void serveJsonState(AsyncWebServerRequest* request)
{
  AsyncJsonResponse * response = new AsyncJsonResponse();
  JsonObject& doc = response->getRoot();

  doc["on"] = (bri > 0);
  doc["bri"] = briLast;
  doc["transition"] = transitionDelay/100; //in 100ms
  
  JsonObject& nl = doc.createNestedObject("nl");
  nl["on"] = nightlightActive;
  nl["dur"] = nightlightDelayMins;
  nl["fade"] = nightlightFade;
  nl["tbri"] = nightlightTargetBri;
  
  JsonObject& udpn = doc.createNestedObject("udpn");
  udpn["send"] = notifyDirect;
  udpn["recv"] = receiveNotifications;
  
  JsonArray& seg = doc.createNestedArray("seg");
  JsonObject& seg0 = seg.createNestedObject();
  serializeSegment(seg0);

  response->setLength();
  request->send(response);
}

JsonObject& serializeSegment(JsonObject& root)
{
  WS2812FX::Segment seg = strip.getSegment();
  
  //root["i"] = i;
  root["start"] = seg.start;
  root["stop"] = seg.stop;
  root["len"] = seg.stop - seg.start;
  
  JsonArray& colarr = root.createNestedArray("col");

  for (uint8_t i = 0; i < 3; i++)
  {
    JsonArray& colX = colarr.createNestedArray();
    colX.add((seg.colors[i] >> 16) & 0xFF);
    colX.add((seg.colors[i] >>  8) & 0xFF);
    colX.add((seg.colors[i]      ) & 0xFF);
    if (useRGBW)
    colX.add((seg.colors[i] >> 24) & 0xFF);
  }
  
  root["fx"] = seg.mode;
  root["sx"] = seg.speed;
  root["ix"] = seg.intensity;
  root["pal"] = seg.palette;
  root["sel"] = ((seg.options & 0x01) == 0x01);
  root["rev"] = ((seg.options & 0x02) == 0x02);
  root["cln"] = -1;
}

//fill string buffer with build info
void serveJsonInfo(AsyncWebServerRequest* request)
{
  AsyncJsonResponse* response = new AsyncJsonResponse();
  JsonObject& doc = response->getRoot();
  
  doc["ver"] = versionString;
  doc["vid"] = VERSION;
  
  JsonObject& leds = doc.createNestedObject("leds");
  leds["count"] = ledCount;
  leds["rgbw"] = useRGBW;
  JsonArray& leds_pin = leds.createNestedArray("pin");
  leds_pin.add(LEDPIN);
  
  leds["pwr"] = strip.currentMilliamps;
  leds["maxpwr"] = strip.ablMilliampsMax;
  leds["maxseg"] = 1;
  doc["name"] = serverDescription;
  doc["udpport"] = udpPort;
  doc["live"] = realtimeActive;
  doc["fxcount"] = strip.getModeCount();
  doc["palcount"] = strip.getPaletteCount();
  #ifdef ARDUINO_ARCH_ESP32
  doc["arch"] = "esp32";
  doc["core"] = ESP.getSdkVersion();
  //doc["maxalloc"] = ESP.getMaxAllocHeap();
  #else
  doc["arch"] = "esp8266";
  doc["core"] = ESP.getCoreVersion();
  //doc["maxalloc"] = ESP.getMaxFreeBlockSize();
  #endif
  doc["freeheap"] = ESP.getFreeHeap();
  doc["uptime"] = millis()/1000;
  
  JsonArray& opt = doc.createNestedArray("opt");
  #ifndef WLED_DISABLE_ALEXA
  opt.add("alexa");
  #endif
  #ifndef WLED_DISABLE_BLYNK
  opt.add("blynk");
  #endif
  #ifndef WLED_DISABLE_CRONIXIE
  opt.add("cronixie");
  #endif
  #ifdef WLED_DEBUG
  opt.add("debug");
  #endif
  #ifdef USEFS
  opt.add("fs");
  #endif
  #ifndef WLED_DISABLE_HUESYNC
  opt.add("huesync");
  #endif
  #ifndef WLED_DISABLE_MOBILE_UI
  opt.add("mobile-ui");
  #endif
  #ifndef WLED_DISABLE_OTA 
  opt.add("ota");
  #endif
  
  doc["brand"] = "wled";
  doc["product"] = "DIY light";
  doc["btype"] = "dev";
  doc["mac"] = escapedMac;

  response->setLength();
  request->send(response);
}
