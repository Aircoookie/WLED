/*
 * JSON API (De)serialization
 */

bool deserializeState(JsonObject root)
{
  bool stateResponse = root["v"] | false;
  
  bri = root["bri"] | bri;
  
  bool on = root["on"] | (bri > 0);
  if (!on != !bri) toggleOnOff();
  
  if (root.containsKey("transition"))
  {
    transitionDelay = root["transition"];
    transitionDelay *= 100;
  }

  int ps = root["ps"] | -1;
  if (ps >= 0) applyPreset(ps);
  
  int cy = root["pl"] | -1;
  presetCyclingEnabled = (cy >= 0);

  JsonObject nl = root["nl"];
  nightlightActive    = nl["on"]   | nightlightActive;
  nightlightDelayMins = nl["dur"]  | nightlightDelayMins;
  nightlightFade      = nl["fade"] | nightlightFade;
  nightlightTargetBri = nl["tbri"] | nightlightTargetBri;

  JsonObject udpn = root["udpn"];
  notifyDirect         = udpn["send"] | notifyDirect;
  receiveNotifications = udpn["recv"] | receiveNotifications;
  bool noNotification  = udpn["nn"]; //send no notification just for this request

  int timein = root["time"] | -1;
  if (timein != -1) setTime(timein);

  int it = 0;
  JsonArray segs = root["seg"];
  for (JsonObject elem : segs)
  {
    byte id = elem["id"] | it;
    if (id < strip.getMaxSegments())
    {
      WS2812FX::Segment& seg = strip.getSegment(id);
      uint16_t start = elem["start"] | seg.start;
      int stop = elem["stop"] | -1;

      if (stop < 0) {
        uint16_t len = elem["len"];
        stop = (len > 0) ? start + len : seg.stop;
      }
      strip.setSegment(id, start, stop);
      
      JsonArray colarr = elem["col"];
      if (!colarr.isNull())
      {
        for (uint8_t i = 0; i < 3; i++)
        {
          JsonArray colX = colarr[i];
          if (colX.isNull()) break;
          byte sz = colX.size();
          if (sz > 0 && sz < 5)
          {
            int rgbw[] = {0,0,0,0};
            byte cp = copyArray(colX, rgbw);
            seg.colors[i] = ((rgbw[3] << 24) | ((rgbw[0]&0xFF) << 16) | ((rgbw[1]&0xFF) << 8) | ((rgbw[2]&0xFF)));
            if (cp == 1 && rgbw[0] == 0) seg.colors[i] = 0;
            if (id == 0) //temporary
            { 
              if (i == 0) {col[0] = rgbw[0]; col[1] = rgbw[1]; col[2] = rgbw[2]; col[3] = rgbw[3];}
              if (i == 1) {colSec[0] = rgbw[0]; colSec[1] = rgbw[1]; colSec[2] = rgbw[2]; colSec[3] = rgbw[3];}
            }
          }
        }
      }
      
      byte fx = elem["fx"] | seg.mode;
      if (fx != seg.mode && fx < strip.getModeCount()) strip.setMode(id, fx);
      seg.speed = elem["sx"] | seg.speed;
      seg.intensity = elem["ix"] | seg.intensity;
      seg.palette = elem["pal"] | seg.palette;
      //if (pal != seg.palette && pal < strip.getPaletteCount()) strip.setPalette(pal);
      seg.setOption(0, elem["sel"] | seg.getOption(0)); //selected
      seg.setOption(1, elem["rev"] | seg.getOption(1)); //reverse
      //int cln = seg_0["cln"];
      //temporary
      if (id == 0) {
        effectCurrent = seg.mode;
        effectSpeed = seg.speed;
        effectIntensity = seg.intensity;
        effectPalette = seg.palette;
      }
    }
    it++;
  }
  colorUpdated(noNotification ? 5:1);

  return stateResponse;
}

void serializeState(JsonObject root)
{
  root["on"] = (bri > 0);
  root["bri"] = briLast;
  root["transition"] = transitionDelay/100; //in 100ms

  root["ps"] = -1; //
  root["pl"] = (presetCyclingEnabled) ? 0: -1;
  
  JsonObject nl = root.createNestedObject("nl");
  nl["on"] = nightlightActive;
  nl["dur"] = nightlightDelayMins;
  nl["fade"] = nightlightFade;
  nl["tbri"] = nightlightTargetBri;
  
  JsonObject udpn = root.createNestedObject("udpn");
  udpn["send"] = notifyDirect;
  udpn["recv"] = receiveNotifications;
  
  JsonArray seg = root.createNestedArray("seg");
  for (byte s = 0; s < strip.getMaxSegments(); s++)
  {
    WS2812FX::Segment sg = strip.getSegment(s);
    if (sg.isActive())
    {
      JsonObject seg0 = seg.createNestedObject();
      serializeSegment(seg0, sg, s);
    }
  }
}

void serializeSegment(JsonObject& root, WS2812FX::Segment& seg, byte id)
{
  root["id"] = id;
  root["start"] = seg.start;
  root["stop"] = seg.stop;
  root["len"] = seg.stop - seg.start;
  
  JsonArray colarr = root.createNestedArray("col");

  for (uint8_t i = 0; i < 3; i++)
  {
    JsonArray colX = colarr.createNestedArray();
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
  root["sel"] = seg.isSelected();
  root["rev"] = seg.getOption(1);
  root["cln"] = -1;
}

void serializeInfo(JsonObject root)
{
  root["ver"] = versionString;
  root["vid"] = VERSION;
  
  JsonObject leds = root.createNestedObject("leds");
  leds["count"] = ledCount;
  leds["rgbw"] = useRGBW;
  JsonArray leds_pin = leds.createNestedArray("pin");
  leds_pin.add(LEDPIN);
  
  leds["pwr"] = strip.currentMilliamps;
  leds["maxpwr"] = strip.ablMilliampsMax;
  leds["maxseg"] = strip.getMaxSegments();
  
  root["name"] = serverDescription;
  root["udpport"] = udpPort;
  root["live"] = realtimeActive;
  root["fxcount"] = strip.getModeCount();
  root["palcount"] = strip.getPaletteCount();
  #ifdef ARDUINO_ARCH_ESP32
  root["arch"] = "esp32";
  root["core"] = ESP.getSdkVersion();
  //root["maxalloc"] = ESP.getMaxAllocHeap();
  #else
  root["arch"] = "esp8266";
  root["core"] = ESP.getCoreVersion();
  //root["maxalloc"] = ESP.getMaxFreeBlockSize();
  #endif
  root["freeheap"] = ESP.getFreeHeap();
  root["uptime"] = millis()/1000;
  
  byte os = 0;
  #ifdef WLED_DEBUG
  os  = 0x80;
  #endif
  #ifndef WLED_DISABLE_ALEXA
  os += 0x40;
  #endif
  #ifndef WLED_DISABLE_BLYNK
  os += 0x20;
  #endif
  #ifndef WLED_DISABLE_CRONIXIE
  os += 0x10;
  #endif
  #ifndef WLED_DISABLE_FILESYSTEM
  os += 0x08;
  #endif
  #ifndef WLED_DISABLE_HUESYNC
  os += 0x04;
  #endif
  #ifndef WLED_DISABLE_MOBILE_UI
  os += 0x02;
  #endif
  #ifndef WLED_DISABLE_OTA 
  os += 0x01;
  #endif
  root["opt"] = os;
  
  root["brand"] = "WLED";
  root["product"] = "DIY light";
  root["btype"] = "dev";
  root["mac"] = escapedMac;
}

void serveJson(AsyncWebServerRequest* request)
{
  byte subJson = 0;
  const String& url = request->url();
  if      (url.indexOf("state") > 0) subJson = 1;
  else if (url.indexOf("info")  > 0) subJson = 2;
  else if (url.indexOf("eff")   > 0) {
    request->send_P(200, "application/json", JSON_mode_names);
    return;
  }
  else if (url.indexOf("pal")   > 0) {
    request->send_P(200, "application/json", JSON_palette_names);
    return;
  }
  else if (url.length() > 6) { //not just /json
    request->send(  501, "application/json", "{\"error\":\"Not implemented\"}");
    return;
  }
  
  AsyncJsonResponse* response = new AsyncJsonResponse();
  JsonObject doc = response->getRoot();

  switch (subJson)
  {
    case 1: //state
      serializeState(doc); break;
    case 2: //info
      serializeInfo(doc); break;
    default: //all
      JsonObject state = doc.createNestedObject("state");
      serializeState(state);
      JsonObject info  = doc.createNestedObject("info");
      serializeInfo(info);
      doc["effects"]  = serialized((const __FlashStringHelper*)JSON_mode_names);
      doc["palettes"] = serialized((const __FlashStringHelper*)JSON_palette_names);
  }
  
  response->setLength();
  request->send(response);
}
