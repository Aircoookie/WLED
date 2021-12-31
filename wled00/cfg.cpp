#include "wled.h"
#include "wled_ethernet.h"

/*
 * Serializes and parses the cfg.json and wsec.json settings files, stored in internal FS.
 * The structure of the JSON is not to be considered an official API and may change without notice.
 */

//simple macro for ArduinoJSON's or syntax
#define CJSON(a,b) a = b | a

void getStringFromJson(char* dest, const char* src, size_t len) {
  if (src != nullptr) strlcpy(dest, src, len);
}

bool deserializeConfig(JsonObject doc, bool fromFS) {
  bool needsSave = false;
  //int rev_major = doc["rev"][0]; // 1
  //int rev_minor = doc["rev"][1]; // 0

  //long vid = doc[F("vid")]; // 2010020

  #ifdef WLED_USE_ETHERNET
  JsonObject ethernet = doc[F("eth")];
  CJSON(ethernetType, ethernet["type"]);
  // NOTE: Ethernet configuration takes priority over other use of pins
  WLED::instance().initEthernet();
  #endif

  JsonObject id = doc["id"];
  getStringFromJson(cmDNS, id[F("mdns")], 33);
  getStringFromJson(serverDescription, id[F("name")], 33);
  getStringFromJson(alexaInvocationName, id[F("inv")], 33);

  JsonObject nw_ins_0 = doc["nw"]["ins"][0];
  getStringFromJson(clientSSID, nw_ins_0[F("ssid")], 33);
  //int nw_ins_0_pskl = nw_ins_0[F("pskl")];
  //The WiFi PSK is normally not contained in the regular file for security reasons.
  //If it is present however, we will use it
  getStringFromJson(clientPass, nw_ins_0["psk"], 65);

  JsonArray nw_ins_0_ip = nw_ins_0["ip"];
  JsonArray nw_ins_0_gw = nw_ins_0["gw"];
  JsonArray nw_ins_0_sn = nw_ins_0["sn"];

  for (byte i = 0; i < 4; i++) {
    CJSON(staticIP[i], nw_ins_0_ip[i]);
    CJSON(staticGateway[i], nw_ins_0_gw[i]);
    CJSON(staticSubnet[i], nw_ins_0_sn[i]);
  }

  JsonObject ap = doc["ap"];
  getStringFromJson(apSSID, ap[F("ssid")], 33);
  getStringFromJson(apPass, ap["psk"] , 65); //normally not present due to security
  //int ap_pskl = ap[F("pskl")];

  CJSON(apChannel, ap[F("chan")]);
  if (apChannel > 13 || apChannel < 1) apChannel = 1;

  CJSON(apHide, ap[F("hide")]);
  if (apHide > 1) apHide = 1;

  CJSON(apBehavior, ap[F("behav")]);
  
  /*
  JsonArray ap_ip = ap["ip"];
  for (byte i = 0; i < 4; i++) {
    apIP[i] = ap_ip;
  }
  */

  noWifiSleep = doc[F("wifi")][F("sleep")] | !noWifiSleep; // inverted
  noWifiSleep = !noWifiSleep;
  //int wifi_phy = doc[F("wifi")][F("phy")]; //force phy mode n?

  JsonObject hw = doc[F("hw")];

  // initialize LED pins and lengths prior to other HW (except for ethernet)
  JsonObject hw_led = hw[F("led")];

  CJSON(strip.ablMilliampsMax, hw_led[F("maxpwr")]);
  CJSON(strip.milliampsPerLed, hw_led[F("ledma")]);
  Bus::setAutoWhiteMode(hw_led[F("rgbwm")] | Bus::getAutoWhiteMode());
  CJSON(correctWB, hw_led["cct"]);
  CJSON(cctFromRgb, hw_led[F("cr")]);
	CJSON(strip.cctBlending, hw_led[F("cb")]);
	Bus::setCCTBlend(strip.cctBlending);
	strip.setTargetFps(hw_led["fps"]); //NOP if 0, default 42 FPS

  JsonArray ins = hw_led["ins"];
  
  if (fromFS || !ins.isNull()) {
    uint8_t s = 0;  // bus iterator
    busses.removeAll();
    uint32_t mem = 0;
    for (JsonObject elm : ins) {
      if (s >= WLED_MAX_BUSSES) break;
      uint8_t pins[5] = {255, 255, 255, 255, 255};
      JsonArray pinArr = elm["pin"];
      if (pinArr.size() == 0) continue;
      pins[0] = pinArr[0];
      uint8_t i = 0;
      for (int p : pinArr) {
        pins[i++] = p;
        if (i>4) break;
      }

      uint16_t length = elm[F("len")] | 1;
      uint8_t colorOrder = (int)elm[F("order")];
      uint8_t skipFirst = elm[F("skip")];
      uint16_t start = elm["start"] | 0;
      if (length==0 || start + length > MAX_LEDS) continue; // zero length or we reached max. number of LEDs, just stop
      uint8_t ledType = elm["type"] | TYPE_WS2812_RGB;
      bool reversed = elm["rev"];
      bool refresh = elm["ref"] | false;
      ledType |= refresh << 7;  // hack bit 7 to indicate strip requires off refresh
      s++;
      BusConfig bc = BusConfig(ledType, pins, start, length, colorOrder, reversed, skipFirst);
      mem += BusManager::memUsage(bc);
      if (mem <= MAX_LED_MEMORY && busses.getNumBusses() <= WLED_MAX_BUSSES) busses.add(bc);  // finalization will be done in WLED::beginStrip()
    }
    // finalization done in beginStrip()
  }
  if (hw_led["rev"]) busses.getBus(0)->reversed = true; //set 0.11 global reversed setting for first bus

  // read multiple button configuration
  JsonObject btn_obj = hw["btn"];
  JsonArray hw_btn_ins = btn_obj[F("ins")];
  if (!hw_btn_ins.isNull()) {
    uint8_t s = 0;
    for (JsonObject btn : hw_btn_ins) {
      CJSON(buttonType[s], btn["type"]);
      int8_t pin = btn["pin"][0] | -1;
      if (pin > -1 && pinManager.allocatePin(pin, false, PinOwner::Button)) {
        btnPin[s] = pin;
        pinMode(btnPin[s], INPUT_PULLUP);
      } else {
        btnPin[s] = -1;
      }
      JsonArray hw_btn_ins_0_macros = btn["macros"];
      CJSON(macroButton[s], hw_btn_ins_0_macros[0]);
      CJSON(macroLongPress[s],hw_btn_ins_0_macros[1]);
      CJSON(macroDoublePress[s], hw_btn_ins_0_macros[2]);
      if (++s >= WLED_MAX_BUTTONS) break; // max buttons reached
    }
    // clear remaining buttons
    for (; s<WLED_MAX_BUTTONS; s++) {
      btnPin[s]           = -1;
      buttonType[s]       = BTN_TYPE_NONE;
      macroButton[s]      = 0;
      macroLongPress[s]   = 0;
      macroDoublePress[s] = 0;
    }
  } else {
    // new install/missing configuration (button 0 has defaults)
    if (fromFS) {
      // relies upon only being called once with fromFS == true, which is currently true.
      uint8_t s = 0;
      if (pinManager.allocatePin(btnPin[0], false, PinOwner::Button)) { // initialized to #define value BTNPIN, or zero if not defined(!)
        ++s; // do not clear default button if allocated successfully 
      }
      for (; s<WLED_MAX_BUTTONS; s++) {
        btnPin[s]           = -1;
        buttonType[s]       = BTN_TYPE_NONE;
        macroButton[s]      = 0;
        macroLongPress[s]   = 0;
        macroDoublePress[s] = 0;
      }
    }
  }
  CJSON(touchThreshold,btn_obj[F("tt")]);
  CJSON(buttonPublishMqtt,btn_obj["mqtt"]);

  int hw_ir_pin = hw["ir"]["pin"] | -2; // 4
  if (hw_ir_pin > -2) {
    if (pinManager.allocatePin(hw_ir_pin, false, PinOwner::IR)) {
      irPin = hw_ir_pin;
    } else {
      irPin = -1;
    }
  }
  CJSON(irEnabled, hw["ir"]["type"]);

  JsonObject relay = hw[F("relay")];
  int hw_relay_pin = relay["pin"] | -2;
  if (hw_relay_pin > -2) {
    if (pinManager.allocatePin(hw_relay_pin,true, PinOwner::Relay)) {
      rlyPin = hw_relay_pin;
      pinMode(rlyPin, OUTPUT);
    } else {
      rlyPin = -1;
    }
  }
  if (relay.containsKey("rev")) {
    rlyMde = !relay["rev"];
  }

  //int hw_status_pin = hw[F("status")]["pin"]; // -1

  JsonObject light = doc[F("light")];
  CJSON(briMultiplier, light[F("scale-bri")]);
  CJSON(strip.paletteBlend, light[F("pal-mode")]);
  CJSON(autoSegments, light[F("aseg")]);

  float light_gc_bri = light["gc"]["bri"];
  float light_gc_col = light["gc"]["col"]; // 2.8
  if (light_gc_bri > 1.5) strip.gammaCorrectBri = true;
  else if (light_gc_bri > 0.5) strip.gammaCorrectBri = false;
  if (light_gc_col > 1.5) strip.gammaCorrectCol = true;
  else if (light_gc_col > 0.5) strip.gammaCorrectCol = false;

  JsonObject light_tr = light[F("tr")];
  CJSON(fadeTransition, light_tr[F("mode")]);
  int tdd = light_tr["dur"] | -1;
  if (tdd >= 0) transitionDelayDefault = tdd * 100;
  CJSON(strip.paletteFade, light_tr["pal"]);

  JsonObject light_nl = light["nl"];
  CJSON(nightlightMode, light_nl[F("mode")]);
  byte prev = nightlightDelayMinsDefault;
  CJSON(nightlightDelayMinsDefault, light_nl[F("dur")]);
  if (nightlightDelayMinsDefault != prev) nightlightDelayMins = nightlightDelayMinsDefault;

  CJSON(nightlightTargetBri, light_nl[F("tbri")]);
  CJSON(macroNl, light_nl["macro"]);

  JsonObject def = doc[F("def")];
  CJSON(bootPreset, def["ps"]);
  CJSON(turnOnAtBoot, def["on"]); // true
  CJSON(briS, def["bri"]); // 128

  JsonObject interfaces = doc["if"];

  JsonObject if_sync = interfaces[F("sync")];
  CJSON(udpPort, if_sync[F("port0")]); // 21324
  CJSON(udpPort2, if_sync[F("port1")]); // 65506

  JsonObject if_sync_recv = if_sync["recv"];
  CJSON(receiveNotificationBrightness, if_sync_recv["bri"]);
  CJSON(receiveNotificationColor, if_sync_recv["col"]);
  CJSON(receiveNotificationEffects, if_sync_recv["fx"]);
  CJSON(receiveGroups, if_sync_recv["grp"]);
  //! following line might be a problem if called after boot
  receiveNotifications = (receiveNotificationBrightness || receiveNotificationColor || receiveNotificationEffects);

  JsonObject if_sync_send = if_sync["send"];
  prev = notifyDirectDefault;
  CJSON(notifyDirectDefault, if_sync_send[F("dir")]);
  if (notifyDirectDefault != prev) notifyDirect = notifyDirectDefault;
  CJSON(notifyButton, if_sync_send["btn"]);
  CJSON(notifyAlexa, if_sync_send["va"]);
  CJSON(notifyHue, if_sync_send["hue"]);
  CJSON(notifyMacro, if_sync_send["macro"]);
  CJSON(notifyTwice, if_sync_send[F("twice")]);
  CJSON(syncGroups, if_sync_send["grp"]);

  JsonObject if_nodes = interfaces["nodes"];
  CJSON(nodeListEnabled, if_nodes[F("list")]);
  CJSON(nodeBroadcastEnabled, if_nodes[F("bcast")]);

  JsonObject if_live = interfaces["live"];
  CJSON(receiveDirect, if_live["en"]);
  CJSON(e131Port, if_live["port"]); // 5568
  if (e131Port == DDP_DEFAULT_PORT) e131Port = E131_DEFAULT_PORT; // prevent double DDP port allocation
  CJSON(e131Multicast, if_live[F("mc")]);

  JsonObject if_live_dmx = if_live[F("dmx")];
  CJSON(e131Universe, if_live_dmx[F("uni")]);
  CJSON(e131SkipOutOfSequence, if_live_dmx[F("seqskip")]);
  CJSON(DMXAddress, if_live_dmx[F("addr")]);
  CJSON(DMXMode, if_live_dmx[F("mode")]);

  tdd = if_live[F("timeout")] | -1;
  if (tdd >= 0) realtimeTimeoutMs = tdd * 100;
  CJSON(arlsForceMaxBri, if_live[F("maxbri")]);
  CJSON(arlsDisableGammaCorrection, if_live[F("no-gc")]); // false
  CJSON(arlsOffset, if_live[F("offset")]); // 0

  CJSON(alexaEnabled, interfaces["va"][F("alexa")]); // false

  CJSON(macroAlexaOn, interfaces["va"]["macros"][0]);
  CJSON(macroAlexaOff, interfaces["va"]["macros"][1]);

#ifndef WLED_DISABLE_BLYNK
  const char* apikey = interfaces["blynk"][F("token")] | "Hidden";
  tdd = strnlen(apikey, 36);
  if (tdd > 20 || tdd == 0)
    getStringFromJson(blynkApiKey, apikey, 36); //normally not present due to security

  JsonObject if_blynk = interfaces["blynk"];
  getStringFromJson(blynkHost, if_blynk[F("host")], 33);
  CJSON(blynkPort, if_blynk["port"]);
#endif

#ifdef WLED_ENABLE_MQTT
  JsonObject if_mqtt = interfaces["mqtt"];
  CJSON(mqttEnabled, if_mqtt["en"]);
  getStringFromJson(mqttServer, if_mqtt[F("broker")], 33);
  CJSON(mqttPort, if_mqtt["port"]); // 1883
  getStringFromJson(mqttUser, if_mqtt[F("user")], 41);
  getStringFromJson(mqttPass, if_mqtt["psk"], 65); //normally not present due to security
  getStringFromJson(mqttClientID, if_mqtt[F("cid")], 41);

  getStringFromJson(mqttDeviceTopic, if_mqtt[F("topics")][F("device")], 33); // "wled/test"
  getStringFromJson(mqttGroupTopic, if_mqtt[F("topics")][F("group")], 33); // ""
#endif

#ifndef WLED_DISABLE_HUESYNC
  JsonObject if_hue = interfaces["hue"];
  CJSON(huePollingEnabled, if_hue["en"]);
  CJSON(huePollLightId, if_hue["id"]);
  tdd = if_hue[F("iv")] | -1;
  if (tdd >= 2) huePollIntervalMs = tdd * 100;

  JsonObject if_hue_recv = if_hue["recv"];
  CJSON(hueApplyOnOff, if_hue_recv["on"]);
  CJSON(hueApplyBri, if_hue_recv["bri"]);
  CJSON(hueApplyColor, if_hue_recv["col"]);

  JsonArray if_hue_ip = if_hue["ip"];

  for (byte i = 0; i < 4; i++)
    CJSON(hueIP[i], if_hue_ip[i]);
#endif

  JsonObject if_ntp = interfaces[F("ntp")];
  CJSON(ntpEnabled, if_ntp["en"]);
  getStringFromJson(ntpServerName, if_ntp[F("host")], 33); // "1.wled.pool.ntp.org"
  CJSON(currentTimezone, if_ntp[F("tz")]);
  CJSON(utcOffsetSecs, if_ntp[F("offset")]);
  CJSON(useAMPM, if_ntp[F("ampm")]);
  CJSON(longitude, if_ntp[F("ln")]);
  CJSON(latitude, if_ntp[F("lt")]);

  JsonObject ol = doc[F("ol")];
  prev = overlayDefault;
  CJSON(overlayDefault ,ol[F("clock")]); // 0
  CJSON(countdownMode, ol[F("cntdwn")]);
  if (prev != overlayDefault) overlayCurrent = overlayDefault;

  CJSON(overlayMin, ol["min"]);
  CJSON(overlayMax, ol[F("max")]);
  CJSON(analogClock12pixel, ol[F("o12pix")]);
  CJSON(analogClock5MinuteMarks, ol[F("o5m")]);
  CJSON(analogClockSecondsTrail, ol[F("osec")]);

  //timed macro rules
  JsonObject tm = doc[F("timers")];
  JsonObject cntdwn = tm[F("cntdwn")];
  JsonArray cntdwn_goal = cntdwn[F("goal")];
  CJSON(countdownYear,  cntdwn_goal[0]);
  CJSON(countdownMonth, cntdwn_goal[1]);
  CJSON(countdownDay,   cntdwn_goal[2]);
  CJSON(countdownHour,  cntdwn_goal[3]);
  CJSON(countdownMin,   cntdwn_goal[4]);
  CJSON(countdownSec,   cntdwn_goal[5]);
  CJSON(macroCountdown, cntdwn["macro"]);
  setCountdown();

  JsonArray timers = tm["ins"];
  uint8_t it = 0;
  for (JsonObject timer : timers) {
    if (it > 9) break;
    if (it<8 && timer[F("hour")]==255) it=8;  // hour==255 -> sunrise/sunset 
    CJSON(timerHours[it], timer[F("hour")]);
    CJSON(timerMinutes[it], timer["min"]);
    CJSON(timerMacro[it], timer["macro"]);

    byte dowPrev = timerWeekday[it];
    //note: act is currently only 0 or 1.
    //the reason we are not using bool is that the on-disk type in 0.11.0 was already int
    int actPrev = timerWeekday[it] & 0x01;
    CJSON(timerWeekday[it], timer[F("dow")]);
    if (timerWeekday[it] != dowPrev) { //present in JSON
      timerWeekday[it] <<= 1; //add active bit
      int act = timer["en"] | actPrev;
      if (act) timerWeekday[it]++;
    }
    if (it<8) {
			JsonObject start = timer["start"];
			byte startm = start["mon"];
			if (startm) timerMonth[it] = (startm << 4);
      CJSON(timerDay[it], start["day"]);
			JsonObject end = timer["end"];
			CJSON(timerDayEnd[it], end["day"]);
			byte endm = end["mon"];
			if (startm) timerMonth[it] += endm & 0x0F;
			if (!(timerMonth[it] & 0x0F)) timerMonth[it] += 12; //default end month to 12
    }
    it++;
  }

  JsonObject ota = doc["ota"];
  const char* pwd = ota["psk"]; //normally not present due to security

  bool pwdCorrect = !otaLock; //always allow access if ota not locked
  if (pwd != nullptr && strncmp(otaPass, pwd, 33) == 0) pwdCorrect = true;

  if (pwdCorrect) { //only accept these values from cfg.json if ota is unlocked (else from wsec.json)
    CJSON(otaLock, ota[F("lock")]);
    CJSON(wifiLock, ota[F("lock-wifi")]);
    CJSON(aOtaEnabled, ota[F("aota")]);
    getStringFromJson(otaPass, pwd, 33); //normally not present due to security
  }

  #ifdef WLED_ENABLE_DMX
  JsonObject dmx = doc["dmx"];
  CJSON(DMXChannels, dmx[F("chan")]);
  CJSON(DMXGap,dmx[F("gap")]);
  CJSON(DMXStart, dmx["start"]);
  CJSON(DMXStartLED,dmx[F("start-led")]);

  JsonArray dmx_fixmap = dmx[F("fixmap")];
  for (int i = 0; i < dmx_fixmap.size(); i++) {
    if (i > 14) break;
    CJSON(DMXFixtureMap[i],dmx_fixmap[i]);
  }

  CJSON(e131ProxyUniverse, dmx[F("e131proxy")]);
  #endif

  DEBUG_PRINTLN(F("Starting usermod config."));
  JsonObject usermods_settings = doc["um"];
  if (!usermods_settings.isNull()) {
    needsSave = !usermods.readFromConfig(usermods_settings);
  }

  if (fromFS) return needsSave;
  doReboot = doc[F("rb")] | doReboot;
  return (doc["sv"] | true);
}

void deserializeConfigFromFS() {
  bool success = deserializeConfigSec();
  if (!success) { //if file does not exist, try reading from EEPROM
    deEEPSettings();
    return;
  }

  #ifdef WLED_USE_DYNAMIC_JSON
  DynamicJsonDocument doc(JSON_BUFFER_SIZE);
  #else
  if (!requestJSONBufferLock(1)) return;
  #endif

  DEBUG_PRINTLN(F("Reading settings from /cfg.json..."));

  success = readObjectFromFile("/cfg.json", nullptr, &doc);
  if (!success) { //if file does not exist, try reading from EEPROM
    deEEPSettings();
    releaseJSONBufferLock();
    return;
  }

  // NOTE: This routine deserializes *and* applies the configuration
  //       Therefore, must also initialize ethernet from this function
  bool needsSave = deserializeConfig(doc.as<JsonObject>(), true);
  releaseJSONBufferLock();

  if (needsSave) serializeConfig(); // usermods required new prameters
}

void serializeConfig() {
  serializeConfigSec();

  DEBUG_PRINTLN(F("Writing settings to /cfg.json..."));

  #ifdef WLED_USE_DYNAMIC_JSON
  DynamicJsonDocument doc(JSON_BUFFER_SIZE);
  #else
  if (!requestJSONBufferLock(2)) return;
  #endif

  JsonArray rev = doc.createNestedArray("rev");
  rev.add(1); //major settings revision
  rev.add(0); //minor settings revision

  doc[F("vid")] = VERSION;

  JsonObject id = doc.createNestedObject("id");
  id[F("mdns")] = cmDNS;
  id[F("name")] = serverDescription;
  id[F("inv")] = alexaInvocationName;

  JsonObject nw = doc.createNestedObject("nw");

  JsonArray nw_ins = nw.createNestedArray("ins");

  JsonObject nw_ins_0 = nw_ins.createNestedObject();
  nw_ins_0[F("ssid")] = clientSSID;
  nw_ins_0[F("pskl")] = strlen(clientPass);

  JsonArray nw_ins_0_ip = nw_ins_0.createNestedArray("ip");
  JsonArray nw_ins_0_gw = nw_ins_0.createNestedArray("gw");
  JsonArray nw_ins_0_sn = nw_ins_0.createNestedArray("sn");

  for (byte i = 0; i < 4; i++) {
    nw_ins_0_ip.add(staticIP[i]);
    nw_ins_0_gw.add(staticGateway[i]);
    nw_ins_0_sn.add(staticSubnet[i]);
  }

  JsonObject ap = doc.createNestedObject("ap");
  ap[F("ssid")] = apSSID;
  ap[F("pskl")] = strlen(apPass);
  ap[F("chan")] = apChannel;
  ap[F("hide")] = apHide;
  ap[F("behav")] = apBehavior;

  JsonArray ap_ip = ap.createNestedArray("ip");
  ap_ip.add(4);
  ap_ip.add(3);
  ap_ip.add(2);
  ap_ip.add(1);

  JsonObject wifi = doc.createNestedObject("wifi");
  wifi[F("sleep")] = !noWifiSleep;
  //wifi[F("phy")] = 1;

  #ifdef WLED_USE_ETHERNET
  JsonObject ethernet = doc.createNestedObject("eth");
  ethernet["type"] = ethernetType;
  if (ethernetType != WLED_ETH_NONE && ethernetType < WLED_NUM_ETH_TYPES) {
    JsonArray pins = ethernet.createNestedArray("pin");
    for (uint8_t p=0; p<WLED_ETH_RSVD_PINS_COUNT; p++) pins.add(esp32_nonconfigurable_ethernet_pins[p].pin);
    if (ethernetBoards[ethernetType].eth_power>=0)     pins.add(ethernetBoards[ethernetType].eth_power);
    if (ethernetBoards[ethernetType].eth_mdc>=0)       pins.add(ethernetBoards[ethernetType].eth_mdc);
    if (ethernetBoards[ethernetType].eth_mdio>=0)      pins.add(ethernetBoards[ethernetType].eth_mdio);
    switch (ethernetBoards[ethernetType].eth_clk_mode) {
      case ETH_CLOCK_GPIO0_IN:
      case ETH_CLOCK_GPIO0_OUT:
        pins.add(0);
        break;
      case ETH_CLOCK_GPIO16_OUT:
        pins.add(16);
        break;
      case ETH_CLOCK_GPIO17_OUT:
        pins.add(17);
        break;
    }
  }
  #endif

  JsonObject hw = doc.createNestedObject("hw");

  JsonObject hw_led = hw.createNestedObject("led");
  hw_led[F("total")] = strip.getLengthTotal(); //no longer read, but provided for compatibility on downgrade
  hw_led[F("maxpwr")] = strip.ablMilliampsMax;
  hw_led[F("ledma")] = strip.milliampsPerLed;
  hw_led["cct"] = correctWB;
  hw_led[F("cr")] = cctFromRgb;
	hw_led[F("cb")] = strip.cctBlending;
	hw_led["fps"] = strip.getTargetFps();
	hw_led[F("rgbwm")] = Bus::getAutoWhiteMode();

  JsonArray hw_led_ins = hw_led.createNestedArray("ins");

  for (uint8_t s = 0; s < busses.getNumBusses(); s++) {
    Bus *bus = busses.getBus(s);
    if (!bus || bus->getLength()==0) break;
    JsonObject ins = hw_led_ins.createNestedObject();
    ins["start"] = bus->getStart();
    ins[F("len")] = bus->getLength();
    JsonArray ins_pin = ins.createNestedArray("pin");
    uint8_t pins[5];
    uint8_t nPins = bus->getPins(pins);
    for (uint8_t i = 0; i < nPins; i++) ins_pin.add(pins[i]);
    ins[F("order")] = bus->getColorOrder();
    ins["rev"] = bus->reversed;
    ins[F("skip")] = bus->skippedLeds();
    ins["type"] = bus->getType() & 0x7F;
    ins["ref"] = bus->isOffRefreshRequired();
    ins[F("rgbw")] = bus->isRgbw();
  }

  // button(s)
  JsonObject hw_btn = hw.createNestedObject("btn");
  hw_btn["max"] = WLED_MAX_BUTTONS; // just information about max number of buttons (not actually used)
  JsonArray hw_btn_ins = hw_btn.createNestedArray("ins");

  // configuration for all buttons
  for (uint8_t i=0; i<WLED_MAX_BUTTONS; i++) {
    JsonObject hw_btn_ins_0 = hw_btn_ins.createNestedObject();
    hw_btn_ins_0["type"] = buttonType[i];
    JsonArray hw_btn_ins_0_pin = hw_btn_ins_0.createNestedArray("pin");
    hw_btn_ins_0_pin.add(btnPin[i]);
    JsonArray hw_btn_ins_0_macros = hw_btn_ins_0.createNestedArray("macros");
    hw_btn_ins_0_macros.add(macroButton[i]);
    hw_btn_ins_0_macros.add(macroLongPress[i]);
    hw_btn_ins_0_macros.add(macroDoublePress[i]);
  }

  hw_btn[F("tt")] = touchThreshold;
  hw_btn["mqtt"] = buttonPublishMqtt;

  JsonObject hw_ir = hw.createNestedObject("ir");
  hw_ir["pin"] = irPin;
  hw_ir["type"] = irEnabled;  // the byte 'irEnabled' does contain the IR-Remote Type ( 0=disabled )

  JsonObject hw_relay = hw.createNestedObject(F("relay"));
  hw_relay["pin"] = rlyPin;
  hw_relay["rev"] = !rlyMde;

  //JsonObject hw_status = hw.createNestedObject("status");
  //hw_status["pin"] = -1;

  JsonObject light = doc.createNestedObject(F("light"));
  light[F("scale-bri")] = briMultiplier;
  light[F("pal-mode")] = strip.paletteBlend;
  light[F("aseg")] = autoSegments;

  JsonObject light_gc = light.createNestedObject("gc");
  light_gc["bri"] = (strip.gammaCorrectBri) ? 2.8 : 1.0;
  light_gc["col"] = (strip.gammaCorrectCol) ? 2.8 : 1.0;

  JsonObject light_tr = light.createNestedObject("tr");
  light_tr[F("mode")] = fadeTransition;
  light_tr["dur"] = transitionDelayDefault / 100;
  light_tr["pal"] = strip.paletteFade;

  JsonObject light_nl = light.createNestedObject("nl");
  light_nl[F("mode")] = nightlightMode;
  light_nl["dur"] = nightlightDelayMinsDefault;
  light_nl[F("tbri")] = nightlightTargetBri;
  light_nl["macro"] = macroNl;

  JsonObject def = doc.createNestedObject("def");
  def["ps"] = bootPreset;
  def["on"] = turnOnAtBoot;
  def["bri"] = briS;

  JsonObject interfaces = doc.createNestedObject("if");

  JsonObject if_sync = interfaces.createNestedObject("sync");
  if_sync[F("port0")] = udpPort;
  if_sync[F("port1")] = udpPort2;

  JsonObject if_sync_recv = if_sync.createNestedObject("recv");
  if_sync_recv["bri"] = receiveNotificationBrightness;
  if_sync_recv["col"] = receiveNotificationColor;
  if_sync_recv["fx"] = receiveNotificationEffects;
  if_sync_recv["grp"] = receiveGroups;

  JsonObject if_sync_send = if_sync.createNestedObject("send");
  if_sync_send[F("dir")] = notifyDirect;
  if_sync_send["btn"] = notifyButton;
  if_sync_send["va"] = notifyAlexa;
  if_sync_send["hue"] = notifyHue;
  if_sync_send["macro"] = notifyMacro;
  if_sync_send[F("twice")] = notifyTwice;
  if_sync_send["grp"] = syncGroups;

  JsonObject if_nodes = interfaces.createNestedObject("nodes");
  if_nodes[F("list")] = nodeListEnabled;
  if_nodes[F("bcast")] = nodeBroadcastEnabled;

  JsonObject if_live = interfaces.createNestedObject("live");
  if_live["en"] = receiveDirect;
  if_live["port"] = e131Port;
  if_live[F("mc")] = e131Multicast;

  JsonObject if_live_dmx = if_live.createNestedObject("dmx");
  if_live_dmx[F("uni")] = e131Universe;
  if_live_dmx[F("seqskip")] = e131SkipOutOfSequence;
  if_live_dmx[F("addr")] = DMXAddress;
  if_live_dmx[F("mode")] = DMXMode;

  if_live[F("timeout")] = realtimeTimeoutMs / 100;
  if_live[F("maxbri")] = arlsForceMaxBri;
  if_live[F("no-gc")] = arlsDisableGammaCorrection;
  if_live[F("offset")] = arlsOffset;

  JsonObject if_va = interfaces.createNestedObject("va");
  if_va[F("alexa")] = alexaEnabled;

  JsonArray if_va_macros = if_va.createNestedArray("macros");
  if_va_macros.add(macroAlexaOn);
  if_va_macros.add(macroAlexaOff);

#ifndef WLED_DISABLE_BLYNK
  JsonObject if_blynk = interfaces.createNestedObject("blynk");
  if_blynk[F("token")] = strlen(blynkApiKey) ? "Hidden":"";
  if_blynk[F("host")] = blynkHost;
  if_blynk["port"] = blynkPort;
#endif

#ifdef WLED_ENABLE_MQTT
  JsonObject if_mqtt = interfaces.createNestedObject("mqtt");
  if_mqtt["en"] = mqttEnabled;
  if_mqtt[F("broker")] = mqttServer;
  if_mqtt["port"] = mqttPort;
  if_mqtt[F("user")] = mqttUser;
  if_mqtt[F("pskl")] = strlen(mqttPass);
  if_mqtt[F("cid")] = mqttClientID;

  JsonObject if_mqtt_topics = if_mqtt.createNestedObject(F("topics"));
  if_mqtt_topics[F("device")] = mqttDeviceTopic;
  if_mqtt_topics[F("group")] = mqttGroupTopic;
#endif

#ifndef WLED_DISABLE_HUESYNC
  JsonObject if_hue = interfaces.createNestedObject("hue");
  if_hue["en"] = huePollingEnabled;
  if_hue["id"] = huePollLightId;
  if_hue[F("iv")] = huePollIntervalMs / 100;

  JsonObject if_hue_recv = if_hue.createNestedObject("recv");
  if_hue_recv["on"] = hueApplyOnOff;
  if_hue_recv["bri"] = hueApplyBri;
  if_hue_recv["col"] = hueApplyColor;

  JsonArray if_hue_ip = if_hue.createNestedArray("ip");
  for (byte i = 0; i < 4; i++) {
    if_hue_ip.add(hueIP[i]);
  }
#endif

  JsonObject if_ntp = interfaces.createNestedObject("ntp");
  if_ntp["en"] = ntpEnabled;
  if_ntp[F("host")] = ntpServerName;
  if_ntp[F("tz")] = currentTimezone;
  if_ntp[F("offset")] = utcOffsetSecs;
  if_ntp[F("ampm")] = useAMPM;
  if_ntp[F("ln")] = longitude;
  if_ntp[F("lt")] = latitude;

  JsonObject ol = doc.createNestedObject("ol");
  ol[F("clock")] = overlayDefault;
  ol[F("cntdwn")] = countdownMode;

  ol["min"] = overlayMin;
  ol[F("max")] = overlayMax;
  ol[F("o12pix")] = analogClock12pixel;
  ol[F("o5m")] = analogClock5MinuteMarks;
  ol[F("osec")] = analogClockSecondsTrail;

  JsonObject timers = doc.createNestedObject(F("timers"));

  JsonObject cntdwn = timers.createNestedObject(F("cntdwn"));
  JsonArray goal = cntdwn.createNestedArray(F("goal"));
  goal.add(countdownYear); goal.add(countdownMonth); goal.add(countdownDay);
  goal.add(countdownHour); goal.add(countdownMin); goal.add(countdownSec);
  cntdwn["macro"] = macroCountdown;

  JsonArray timers_ins = timers.createNestedArray("ins");

  for (byte i = 0; i < 10; i++) {
    if (timerMacro[i] == 0 && timerHours[i] == 0 && timerMinutes[i] == 0) continue; // sunrise/sunset get saved always (timerHours=255)
    JsonObject timers_ins0 = timers_ins.createNestedObject();
    timers_ins0["en"] = (timerWeekday[i] & 0x01);
    timers_ins0[F("hour")] = timerHours[i];
    timers_ins0["min"] = timerMinutes[i];
    timers_ins0["macro"] = timerMacro[i];
    timers_ins0[F("dow")] = timerWeekday[i] >> 1;
    if (i<8) {
			JsonObject start = timers_ins0.createNestedObject("start");
      start["mon"] = (timerMonth[i] >> 4) & 0xF;
      start["day"] = timerDay[i];
			JsonObject end = timers_ins0.createNestedObject("end");
			end["mon"] = timerMonth[i] & 0xF;
      end["day"] = timerDayEnd[i];
    }
  }

  JsonObject ota = doc.createNestedObject("ota");
  ota[F("lock")] = otaLock;
  ota[F("lock-wifi")] = wifiLock;
  ota[F("pskl")] = strlen(otaPass);
  ota[F("aota")] = aOtaEnabled;

  #ifdef WLED_ENABLE_DMX
  JsonObject dmx = doc.createNestedObject("dmx");
  dmx[F("chan")] = DMXChannels;
  dmx[F("gap")] = DMXGap;
  dmx["start"] = DMXStart;
  dmx[F("start-led")] = DMXStartLED;

  JsonArray dmx_fixmap = dmx.createNestedArray(F("fixmap"));
  for (byte i = 0; i < 15; i++) {
    dmx_fixmap.add(DMXFixtureMap[i]);
  }

  dmx[F("e131proxy")] = e131ProxyUniverse;
  #endif

  JsonObject usermods_settings = doc.createNestedObject("um");
  usermods.addToConfig(usermods_settings);

  File f = WLED_FS.open("/cfg.json", "w");
  if (f) serializeJson(doc, f);
  f.close();
  releaseJSONBufferLock();
}

//settings in /wsec.json, not accessible via webserver, for passwords and tokens
bool deserializeConfigSec() {
  DEBUG_PRINTLN(F("Reading settings from /wsec.json..."));

  #ifdef WLED_USE_DYNAMIC_JSON
  DynamicJsonDocument doc(JSON_BUFFER_SIZE);
  #else
  if (!requestJSONBufferLock(3)) return false;
  #endif

  bool success = readObjectFromFile("/wsec.json", nullptr, &doc);
  if (!success) {
    releaseJSONBufferLock();
    return false;
  }

  JsonObject nw_ins_0 = doc["nw"]["ins"][0];
  getStringFromJson(clientPass, nw_ins_0["psk"], 65);

  JsonObject ap = doc["ap"];
  getStringFromJson(apPass, ap["psk"] , 65);

  JsonObject interfaces = doc["if"];

#ifndef WLED_DISABLE_BLYNK
  const char* apikey = interfaces["blynk"][F("token")] | "Hidden";
  int tdd = strnlen(apikey, 36);
  if (tdd > 20 || tdd == 0)
    getStringFromJson(blynkApiKey, apikey, 36);
#endif

#ifdef WLED_ENABLE_MQTT
  JsonObject if_mqtt = interfaces["mqtt"];
  getStringFromJson(mqttPass, if_mqtt["psk"], 65);
#endif

#ifndef WLED_DISABLE_HUESYNC
  getStringFromJson(hueApiKey, interfaces["hue"][F("key")], 47);
#endif

  JsonObject ota = doc["ota"];
  getStringFromJson(otaPass, ota[F("pwd")], 33);
  CJSON(otaLock, ota[F("lock")]);
  CJSON(wifiLock, ota[F("lock-wifi")]);
  CJSON(aOtaEnabled, ota[F("aota")]);

  releaseJSONBufferLock();
  return true;
}

void serializeConfigSec() {
  DEBUG_PRINTLN(F("Writing settings to /wsec.json..."));

  #ifdef WLED_USE_DYNAMIC_JSON
  DynamicJsonDocument doc(JSON_BUFFER_SIZE);
  #else
  if (!requestJSONBufferLock(4)) return;
  #endif

  JsonObject nw = doc.createNestedObject("nw");

  JsonArray nw_ins = nw.createNestedArray("ins");

  JsonObject nw_ins_0 = nw_ins.createNestedObject();
  nw_ins_0["psk"] = clientPass;

  JsonObject ap = doc.createNestedObject("ap");
  ap["psk"] = apPass;

  JsonObject interfaces = doc.createNestedObject("if");
#ifndef WLED_DISABLE_BLYNK
  JsonObject if_blynk = interfaces.createNestedObject("blynk");
  if_blynk[F("token")] = blynkApiKey;
#endif
#ifdef WLED_ENABLE_MQTT
  JsonObject if_mqtt = interfaces.createNestedObject("mqtt");
  if_mqtt["psk"] = mqttPass;
#endif
#ifndef WLED_DISABLE_HUESYNC
  JsonObject if_hue = interfaces.createNestedObject("hue");
  if_hue[F("key")] = hueApiKey;
#endif

  JsonObject ota = doc.createNestedObject("ota");
  ota[F("pwd")] = otaPass;
  ota[F("lock")] = otaLock;
  ota[F("lock-wifi")] = wifiLock;
  ota[F("aota")] = aOtaEnabled;

  File f = WLED_FS.open("/wsec.json", "w");
  if (f) serializeJson(doc, f);
  f.close();
  releaseJSONBufferLock();
}
