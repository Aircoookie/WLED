/*
 * JSON API (De)serialization
 */

void deserializeSegment(JsonObject elem, byte it)
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
    uint16_t grp = elem["grp"] | seg.grouping;
    uint16_t spc = elem["spc"] | seg.spacing;
    strip.setSegment(id, start, stop, grp, spc);
    
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
          if (id == strip.getMainSegmentId()) //temporary
          { 
            if (i == 0) {col[0] = rgbw[0]; col[1] = rgbw[1]; col[2] = rgbw[2]; col[3] = rgbw[3];}
            if (i == 1) {colSec[0] = rgbw[0]; colSec[1] = rgbw[1]; colSec[2] = rgbw[2]; colSec[3] = rgbw[3];}
          }
        }
      }
    }
    
    //if (pal != seg.palette && pal < strip.getPaletteCount()) strip.setPalette(pal);
    seg.setOption(0, elem["sel"] | seg.getOption(0)); //selected
    seg.setOption(1, elem["rev"] | seg.getOption(1)); //reverse
    //int cln = seg_0["cln"];
    //temporary, strip object gets updated via colorUpdated()
    if (id == strip.getMainSegmentId()) {
      effectCurrent = elem["fx"] | effectCurrent;
      effectSpeed = elem["sx"] | effectSpeed;
      effectIntensity = elem["ix"] | effectIntensity;
      effectPalette = elem["pal"] | effectPalette;
    } else { //permanent
      byte fx = elem["fx"] | seg.mode;
      if (fx != seg.mode && fx < strip.getModeCount()) strip.setMode(id, fx);
      seg.speed = elem["sx"] | seg.speed;
      seg.intensity = elem["ix"] | seg.intensity;
      seg.palette = elem["pal"] | seg.palette;
    }
  }
}

bool deserializeState(JsonObject root)
{
  strip.applyToAllSelected = false;
  bool stateResponse = root["v"] | false;

  int ps = root["ps"] | -1;
  if (ps >= 0) applyPreset(ps);
  
  bri = root["bri"] | bri;
  
  bool on = root["on"] | (bri > 0);
  if (!on != !bri) toggleOnOff();

  int tr = root["transition"] | -1;
  if (tr >= 0)
  {
    transitionDelay = tr;
    transitionDelay *= 100;
  }

  tr = root["tt"] | -1;
  if (tr >= 0)
  {
    transitionDelayTemp = tr;
    transitionDelayTemp *= 100;
    jsonTransitionOnce = true;
  }
  
  int cy = root["pl"] | -2;
  if (cy > -2) presetCyclingEnabled = (cy >= 0);
  JsonObject ccnf = root["ccnf"];
  presetCycleMin = ccnf["min"] | presetCycleMin;
  presetCycleMax = ccnf["max"] | presetCycleMax;
  tr = ccnf["time"] | -1;
  if (tr >= 2)
  {
    presetCycleTime = tr;
    presetCycleTime *= 100;
  }

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

  byte prevMain = strip.getMainSegmentId();
  strip.mainSegment = root["mainseg"] | prevMain;
  if (strip.getMainSegmentId() != prevMain) setValuesFromMainSeg();

  int it = 0;
  JsonVariant segVar = root["seg"];
  if (segVar.is<JsonObject>())
  {
    int id = segVar["id"] | -1;
    
    if (id < 0) { //set all selected segments
      bool didSet = false;
      byte lowestActive = 99;
      for (byte s = 0; s < strip.getMaxSegments(); s++)
      {
        WS2812FX::Segment sg = strip.getSegment(s);
        if (sg.isActive())
        {
          if (lowestActive == 99) lowestActive = s;
          if (sg.isSelected()) {
            deserializeSegment(segVar, s);
            didSet = true;
          }
        }
      }
      if (!didSet && lowestActive < strip.getMaxSegments()) deserializeSegment(segVar, lowestActive);
    } else { //set only the segment with the specified ID
      deserializeSegment(segVar, it);
    }
  } else {
    JsonArray segs = segVar.as<JsonArray>();
    for (JsonObject elem : segs)
    {
      deserializeSegment(elem, it);
      it++;
    }
  }

  colorUpdated(noNotification ? NOTIFIER_CALL_MODE_NO_NOTIFY : NOTIFIER_CALL_MODE_DIRECT_CHANGE);

  //write presets to flash directly?
  bool persistSaves = !(root["np"] | false);

  ps = root["psave"] | -1;
  if (ps >= 0) savePreset(ps, persistSaves);

  return stateResponse;
}

void serializeSegment(JsonObject& root, WS2812FX::Segment& seg, byte id)
{
	root["id"] = id;
	root["start"] = seg.start;
	root["stop"] = seg.stop;
	root["len"] = seg.stop - seg.start;
  root["grp"] = seg.grouping;
  root["spc"] = seg.spacing;

	JsonArray colarr = root.createNestedArray("col");

	for (uint8_t i = 0; i < 3; i++)
	{
		JsonArray colX = colarr.createNestedArray();
		colX.add((seg.colors[i] >> 16) & 0xFF);
		colX.add((seg.colors[i] >> 8) & 0xFF);
		colX.add((seg.colors[i]) & 0xFF);
		if (useRGBW)
			colX.add((seg.colors[i] >> 24) & 0xFF);
	}

	root["fx"] = seg.mode;
	root["sx"] = seg.speed;
	root["ix"] = seg.intensity;
	root["pal"] = seg.palette;
	root["sel"] = seg.isSelected();
	root["rev"] = seg.getOption(1);
}


void serializeState(JsonObject root)
{
  if (errorFlag) root["error"] = errorFlag;
  
  root["on"] = (bri > 0);
  root["bri"] = briLast;
  root["transition"] = transitionDelay/100; //in 100ms

  root["ps"] = currentPreset;
  root["pss"] = savedPresets;
  root["pl"] = (presetCyclingEnabled) ? 0: -1;

  //temporary for preser cycle
  JsonObject ccnf = root.createNestedObject("ccnf");
  ccnf["min"] = presetCycleMin;
  ccnf["max"] = presetCycleMax;
  ccnf["time"] = presetCycleTime/100;
  
  JsonObject nl = root.createNestedObject("nl");
  nl["on"] = nightlightActive;
  nl["dur"] = nightlightDelayMins;
  nl["fade"] = nightlightFade;
  nl["tbri"] = nightlightTargetBri;
  
  JsonObject udpn = root.createNestedObject("udpn");
  udpn["send"] = notifyDirect;
  udpn["recv"] = receiveNotifications;

  root["mainseg"] = strip.getMainSegmentId();
  
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

void serializeInfo(JsonObject root)
{
  root["ver"] = versionString;
  root["vid"] = VERSION;
  
  JsonObject leds = root.createNestedObject("leds");
  leds["count"] = ledCount;
  leds["rgbw"] = useRGBW;
  leds["wv"] = useRGBW && (strip.rgbwMode == RGBW_MODE_MANUAL_ONLY || strip.rgbwMode == RGBW_MODE_DUAL); //should a white channel slider be displayed?
  JsonArray leds_pin = leds.createNestedArray("pin");
  leds_pin.add(LEDPIN);
  
  leds["pwr"] = strip.currentMilliamps;
  leds["maxpwr"] = (strip.currentMilliamps)? strip.ablMilliampsMax : 0;
  leds["maxseg"] = strip.getMaxSegments();
  leds["seglock"] = false; //will be used in the future to prevent modifications to segment config

  root["str"] = syncToggleReceive;
  
  root["name"] = serverDescription;
  root["udpport"] = udpPort;
  root["live"] = (bool)realtimeMode;
  root["fxcount"] = strip.getModeCount();
  root["palcount"] = strip.getPaletteCount();

  JsonObject wifi_info = root.createNestedObject("wifi");
  wifi_info["bssid"] = WiFi.BSSIDstr();
  int qrssi = WiFi.RSSI();
  wifi_info["rssi"] = qrssi;
  wifi_info["signal"] = getSignalQuality(qrssi);
  wifi_info["channel"] = WiFi.channel();
  
  #ifdef ARDUINO_ARCH_ESP32
  #ifdef WLED_DEBUG
    wifi_info["txPower"] = (int) WiFi.getTxPower();
    wifi_info["sleep"] = (bool) WiFi.getSleep();
  #endif
  root["arch"] = "esp32";
  root["core"] = ESP.getSdkVersion();
  //root["maxalloc"] = ESP.getMaxAllocHeap();
  #ifdef WLED_DEBUG
    root["resetReason0"] = (int)rtc_get_reset_reason(0);
    root["resetReason1"] = (int)rtc_get_reset_reason(1);
  #endif
  root["lwip"] = 0;
  #else
  root["arch"] = "esp8266";
  root["core"] = ESP.getCoreVersion();
  //root["maxalloc"] = ESP.getMaxFreeBlockSize();
  #ifdef WLED_DEBUG
    root["resetReason"] = (int)ESP.getResetInfoPtr()->reason;
  #endif
  root["lwip"] = LWIP_VERSION_MAJOR;
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
  #ifdef WLED_ENABLE_ADALIGHT
  os += 0x02;
  #endif
  #ifndef WLED_DISABLE_OTA 
  os += 0x01;
  #endif
  root["opt"] = os;
  
  root["brand"] = "WLED";
  root["product"] = "DIY light";
  root["mac"] = escapedMac;
}

void serveJson(AsyncWebServerRequest* request)
{
  byte subJson = 0;
  const String& url = request->url();
  if      (url.indexOf("state") > 0) subJson = 1;
  else if (url.indexOf("info")  > 0) subJson = 2;
  else if (url.indexOf("live")  > 0) {
    serveLiveLeds(request);
    return;
  }
  else if (url.indexOf("eff")   > 0) {
    request->send_P(200, "application/json", JSON_mode_names);
    return;
  }
  else if (url.indexOf("pal")   > 0) {
    request->send_P(200, "application/json", JSON_palette_names);
    return;
  }
  else if (url.length() > 6) { //not just /json
    request->send(  501, "application/json", F("{\"error\":\"Not implemented\"}"));
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

#define MAX_LIVE_LEDS 180

void serveLiveLeds(AsyncWebServerRequest* request)
{
  byte used = ledCount;
  byte n = (used -1) /MAX_LIVE_LEDS +1; //only serve every n'th LED if count over MAX_LIVE_LEDS
  char buffer[2000] = "{\"leds\":[";
  olen = 9;
  obuf = buffer;

  for (uint16_t i= 0; i < used; i += n)
  {
    olen += sprintf(buffer + olen, "\"%06X\",", strip.getPixelColor(i));
  }
  olen -= 1;
  oappend("],\"n\":");
  oappendi(n);
  oappend("}");
  request->send(200, "application/json", buffer);
}
