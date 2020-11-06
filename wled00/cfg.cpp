#include "wled.h"

/*
 * Serializes and parses the cfg.json and wsec.json settings files, stored in internal FS.
 * The structure of the JSON is not to be considered an official API and may change without notice.
 */

//simple macro for ArduinoJSON's or syntax
#define CJSON(a,b) a = b | a

void getStringFromJson(char* dest, const char* src, size_t len) {
  if (src != nullptr) strlcpy(dest, src, len);
}

void deserializeConfig() {
  bool fromeep = false;
  bool success = deserializeConfigSec();
  if (!success) { //if file does not exist, try reading from EEPROM
    deEEPSettings();
    fromeep = true;
  }

  DynamicJsonDocument doc(JSON_BUFFER_SIZE);

  DEBUG_PRINTLN(F("Reading settings from /cfg.json..."));

  success = readObjectFromFile("/cfg.json", nullptr, &doc);
  if (!success) { //if file does not exist, try reading from EEPROM
    if (!fromeep) deEEPSettings();
    return;
  }

  //deserializeJson(doc, json);

  //int rev_major = doc["rev"][0]; // 1
  //int rev_minor = doc["rev"][1]; // 0

  //long vid = doc["vid"]; // 2010020

  JsonObject id = doc["id"];
  getStringFromJson(cmDNS, id["mdns"], 33);
  getStringFromJson(serverDescription, id["name"], 33);
  getStringFromJson(alexaInvocationName, id["inv"], 33);

  JsonObject nw_ins_0 = doc["nw"]["ins"][0];
  getStringFromJson(clientSSID, nw_ins_0["ssid"], 33);
  //int nw_ins_0_pskl = nw_ins_0["pskl"];
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
  getStringFromJson(apSSID, ap["ssid"], 33);
  getStringFromJson(apPass, ap["psk"] , 65); //normally not present due to security
  //int ap_pskl = ap["pskl"];

  CJSON(apChannel, ap["chan"]);
  if (apChannel > 13 || apChannel < 1) apChannel = 1;

  CJSON(apHide, ap["hide"]);
  if (apHide > 1) apHide = 1;

  CJSON(apBehavior, ap["behav"]);

  /*
  JsonArray ap_ip = ap["ip"];
  for (byte i = 0; i < 4; i++) {
    apIP[i] = ap_ip;
  }*/

  noWifiSleep = doc["wifi"]["sleep"] | !noWifiSleep; // inverted
  noWifiSleep = !noWifiSleep;
  //int wifi_phy = doc["wifi"]["phy"]; //force phy mode n?

  JsonObject hw = doc["hw"];

  JsonObject hw_led = hw["led"];
  CJSON(ledCount, hw_led["total"]);
  if (ledCount > MAX_LEDS) ledCount = MAX_LEDS;

  CJSON(strip.ablMilliampsMax, hw_led["maxpwr"]);
  CJSON(strip.milliampsPerLed, hw_led["ledma"]);
  CJSON(strip.reverseMode, hw_led["rev"]);

  JsonObject hw_led_ins_0 = hw_led["ins"][0];
  //bool hw_led_ins_0_en = hw_led_ins_0["en"]; // true
  //int hw_led_ins_0_start = hw_led_ins_0["start"]; // 0
  //int hw_led_ins_0_len = hw_led_ins_0["len"]; // 1200

  //int hw_led_ins_0_pin_0 = hw_led_ins_0["pin"][0]; // 2

  strip.colorOrder = hw_led_ins_0["order"];
  //bool hw_led_ins_0_rev = hw_led_ins_0["rev"]; // false
  skipFirstLed = hw_led_ins_0["skip"]; // 0
  //int hw_led_ins_0_type = hw_led_ins_0["type"]; // 2*/

  JsonObject hw_btn_ins_0 = hw["btn"]["ins"][0];
  buttonEnabled = hw_btn_ins_0["en"] | buttonEnabled;

  //int hw_btn_ins_0_pin_0 = hw_btn_ins_0["pin"][0]; // 0

  JsonArray hw_btn_ins_0_macros = hw_btn_ins_0["macros"];
  CJSON(macroButton, hw_btn_ins_0_macros[0]);
  CJSON(macroLongPress,hw_btn_ins_0_macros[1]);
  CJSON(macroDoublePress, hw_btn_ins_0_macros[2]);

  //int hw_btn_ins_0_type = hw_btn_ins_0["type"]; // 0

  //int hw_ir_pin = hw["ir"]["pin"]; // 4
  CJSON(irEnabled, hw["ir"]["type"]); // 0

  //int hw_relay_pin = hw["relay"]["pin"]; // 12
  //bool hw_relay_rev = hw["relay"]["rev"]; // false

  //int hw_status_pin = hw["status"]["pin"]; // -1

  JsonObject light = doc["light"];
  CJSON(briMultiplier, light["scale-bri"]);
  CJSON(strip.paletteBlend, light["pal-mode"]);

  float light_gc_bri = light["gc"]["bri"];
  float light_gc_col = light["gc"]["col"]; // 2.8
  if (light_gc_bri > 1.5) strip.gammaCorrectBri = true;
  else if (light_gc_bri > 0.5) strip.gammaCorrectBri = false;
  if (light_gc_col > 1.5) strip.gammaCorrectCol = true;
  else if (light_gc_col > 0.5) strip.gammaCorrectCol = false;

  JsonObject light_tr = light["tr"];
  CJSON(fadeTransition, light_tr["mode"]);
  int tdd = light_tr["dur"] | -1;
  if (tdd >= 0) transitionDelayDefault = tdd * 100;
  CJSON(strip.paletteFade, light_tr["pal"]);

  JsonObject light_nl = light["nl"];
  CJSON(nightlightMode, light_nl["mode"]);
  CJSON(nightlightDelayMinsDefault, light_nl["dur"]);
  nightlightDelayMins = nightlightDelayMinsDefault;

  CJSON(nightlightTargetBri, light_nl["tbri"]);
  CJSON(macroNl, light_nl["macro"]);

  JsonObject def = doc["def"];
  CJSON(bootPreset, def["ps"]);
  CJSON(turnOnAtBoot, def["on"]); // true
  CJSON(briS, def["bri"]); // 128
  if (briS == 0) briS = 255;

  JsonObject def_cy = def["cy"];
  CJSON(presetCyclingEnabled, def_cy["on"]);

  CJSON(presetCycleMin, def_cy["range"][0]);
  CJSON(presetCycleMax, def_cy["range"][1]);

  tdd = def_cy["dur"] | -1;
  if (tdd >= 0) presetCycleTime = tdd * 100;

  JsonObject interfaces = doc["if"];

  JsonObject if_sync = interfaces["sync"];
  CJSON(udpPort, if_sync["port0"]); // 21324
  CJSON(udpPort2, if_sync["port1"]); // 65506

  JsonObject if_sync_recv = if_sync["recv"];
  CJSON(receiveNotificationBrightness, if_sync_recv["bri"]);
  CJSON(receiveNotificationColor, if_sync_recv["col"]);
  CJSON(receiveNotificationEffects, if_sync_recv["fx"]);
  receiveNotifications = (receiveNotificationBrightness || receiveNotificationColor || receiveNotificationEffects);

  JsonObject if_sync_send = if_sync["send"];
  CJSON(notifyDirectDefault, if_sync_send["dir"]);
  notifyDirect = notifyDirectDefault;
  CJSON(notifyButton, if_sync_send["btn"]);
  CJSON(notifyAlexa, if_sync_send["va"]);
  CJSON(notifyHue, if_sync_send["hue"]);
  CJSON(notifyMacro, if_sync_send["macro"]);
  CJSON(notifyTwice, if_sync_send["twice"]);

  JsonObject if_realtime = interfaces["realtime"];
  CJSON(receiveDirect, if_realtime["en"]);
  CJSON(e131Port, if_realtime["port"]); // 5568
  CJSON(e131Multicast, if_realtime["mc"]);

  JsonObject if_realtime_dmx = if_realtime["dmx"];
  CJSON(e131Universe, if_realtime_dmx["uni"]);
  CJSON(e131SkipOutOfSequence, if_realtime_dmx["seqskip"]);
  CJSON(DMXAddress, if_realtime_dmx["addr"]);
  CJSON(DMXMode, if_realtime_dmx["mode"]);

  tdd = if_realtime["timeout"] | -1;
  if (tdd >= 0) realtimeTimeoutMs = tdd * 100;
  CJSON(arlsForceMaxBri, if_realtime["maxbri"]);
  CJSON(arlsDisableGammaCorrection, if_realtime["no-gc"]); // false
  CJSON(arlsOffset, if_realtime["offset"]); // 0

  CJSON(alexaEnabled, interfaces["va"]["alexa"]); // false

  CJSON(macroAlexaOn, interfaces["va"]["macros"][0]);
  CJSON(macroAlexaOff, interfaces["va"]["macros"][1]);

  const char* apikey = interfaces["blynk"]["token"] | "Hidden";
  tdd = strnlen(apikey, 36);
  if (tdd > 20 || tdd == 0)
    getStringFromJson(blynkApiKey, apikey, 36); //normally not present due to security

  JsonObject if_mqtt = interfaces["mqtt"];
  CJSON(mqttEnabled, if_mqtt["en"]);
  getStringFromJson(mqttServer, if_mqtt["broker"], 33);
  CJSON(mqttPort, if_mqtt["port"]); // 1883
  getStringFromJson(mqttUser, if_mqtt["user"], 41);
  getStringFromJson(mqttPass, if_mqtt["psk"], 41); //normally not present due to security
  getStringFromJson(mqttClientID, if_mqtt["cid"], 41);

  getStringFromJson(mqttDeviceTopic, if_mqtt["topics"]["device"], 33); // "wled/test"
  getStringFromJson(mqttGroupTopic, if_mqtt["topics"]["group"], 33); // "" 

  JsonObject if_hue = interfaces["hue"];
  CJSON(huePollingEnabled, if_hue["en"]);
  CJSON(huePollLightId, if_hue["id"]);
  tdd = if_hue["iv"] | -1;
  if (tdd >= 2) huePollIntervalMs = tdd * 100;

  JsonObject if_hue_recv = if_hue["recv"];
  CJSON(hueApplyOnOff, if_hue_recv["on"]);
  CJSON(hueApplyBri, if_hue_recv["bri"]);
  CJSON(hueApplyColor, if_hue_recv["col"]);

  JsonArray if_hue_ip = if_hue["ip"];

  for (byte i = 0; i < 4; i++)
    CJSON(hueIP[i], if_hue_ip[i]);

  JsonObject if_ntp = interfaces["ntp"];
  CJSON(ntpEnabled, if_ntp["en"]);
  getStringFromJson(ntpServerName, if_ntp["host"], 33); // "1.wled.pool.ntp.org"
  CJSON(currentTimezone, if_ntp["tz"]);
  CJSON(utcOffsetSecs, if_ntp["offset"]);
  CJSON(useAMPM, if_ntp["ampm"]);

  JsonObject ol = doc["ol"];
  CJSON(overlayDefault ,ol["clock"]); // 0
  CJSON(countdownMode, ol["cntdwn"]);
  overlayCurrent = overlayDefault;

  JsonArray ol_cntdwn = ol["cntdwn"]; //[20,12,31,23,59,59]

  //timed macro rules
  JsonObject tm = doc["timers"];
  JsonObject cntdwn = tm["cntdwn"];
  JsonArray cntdwn_goal = cntdwn["goal"];
  CJSON(countdownYear,  cntdwn_goal[0]);
  CJSON(countdownMonth, cntdwn_goal[1]);
  CJSON(countdownDay,   cntdwn_goal[2]);
  CJSON(countdownHour,  cntdwn_goal[3]);
  CJSON(countdownMin,   cntdwn_goal[4]);
  CJSON(countdownSec,   cntdwn_goal[5]);
  CJSON(macroCountdown, cntdwn["macro"]);

  JsonArray timers = tm["ins"];
  uint8_t it = 0;
  for (JsonObject timer : timers) {
    if (it > 7) break;
    CJSON(timerHours[it], timer["hour"]);
    CJSON(timerMinutes[it], timer["min"]);
    CJSON(timerMacro[it], timer["macro"]);

    byte dowPrev =  timerWeekday[it];
    bool actPrev = timerWeekday[it] & 0x01;
    CJSON(timerWeekday[it], timer["dow"]);
    if (timerWeekday[it] != dowPrev) { //present in JSON
      timerWeekday[it] <<= 1; //add active bit
      bool act = timer["en"] | actPrev;
      if (act) timerWeekday[it]++;
    }

    it++;
  }

  JsonObject ota = doc["ota"];
  const char* pwd = ota["psk"]; //normally not present due to security

  bool pwdCorrect = !otaLock; //always allow access if ota not locked
  if (pwd != nullptr && strncmp(otaPass, pwd, 33) == 0) pwdCorrect = true;

  if (pwdCorrect) { //only accept these values from cfg.json if ota is unlocked (else from wsec.json)
    CJSON(otaLock, ota["lock"]);
    CJSON(wifiLock, ota["lock-wifi"]);
    CJSON(aOtaEnabled, ota["aota"]);
    getStringFromJson(otaPass, pwd, 33); //normally not present due to security
  }

  //DMX missing!
}

void serializeConfig() {
  DEBUG_PRINTLN(F("Writing settings to /cfg.json..."));

  DynamicJsonDocument doc(JSON_BUFFER_SIZE);

  //{ //scope this to reduce stack size
  JsonArray rev = doc.createNestedArray("rev");
  rev.add(1); //major settings revision
  rev.add(0); //minor settings revision

  doc["vid"] = VERSION;

  JsonObject id = doc.createNestedObject("id");
  id["mdns"] = cmDNS;
  id["name"] = serverDescription;
  id["inv"] = alexaInvocationName;

  JsonObject nw = doc.createNestedObject("nw");

  JsonArray nw_ins = nw.createNestedArray("ins");

  JsonObject nw_ins_0 = nw_ins.createNestedObject();
  nw_ins_0["ssid"] = clientSSID;
  nw_ins_0["pskl"] = strlen(clientPass);

  JsonArray nw_ins_0_ip = nw_ins_0.createNestedArray("ip");
  JsonArray nw_ins_0_gw = nw_ins_0.createNestedArray("gw");
  JsonArray nw_ins_0_sn = nw_ins_0.createNestedArray("sn");

  for (byte i = 0; i < 4; i++) {
    nw_ins_0_ip.add(staticIP[i]);
    nw_ins_0_gw.add(staticGateway[i]);
    nw_ins_0_sn.add(staticSubnet[i]);
  }

  JsonObject ap = doc.createNestedObject("ap");
  ap["ssid"] = apSSID;
  ap["pskl"] = strlen(apPass);
  ap["chan"] = apChannel;
  ap["behav"] = apBehavior;

  JsonArray ap_ip = ap.createNestedArray("ip");
  ap_ip.add(4);
  ap_ip.add(3);
  ap_ip.add(2);
  ap_ip.add(1);

  JsonObject wifi = doc.createNestedObject("wifi");
  wifi["sleep"] = !noWifiSleep;
  wifi["phy"] = 1;

  JsonObject hw = doc.createNestedObject("hw");

  JsonObject hw_led = hw.createNestedObject("led");
  hw_led["total"] = ledCount;
  hw_led["maxpwr"] = strip.ablMilliampsMax;
  hw_led["ledma"] = strip.milliampsPerLed;
  hw_led["rev"] = strip.reverseMode;

  JsonArray hw_led_ins = hw_led.createNestedArray("ins");

  JsonObject hw_led_ins_0 = hw_led_ins.createNestedObject();
  hw_led_ins_0["en"] = true;
  hw_led_ins_0["start"] = 0;
  hw_led_ins_0["len"] = ledCount;
  JsonArray hw_led_ins_0_pin = hw_led_ins_0.createNestedArray("pin");
  hw_led_ins_0_pin.add(LEDPIN);
  #ifdef DATAPIN
  hw_led_ins_0_pin.add(DATAPIN);
  #endif
  hw_led_ins_0["order"] = strip.colorOrder; //color order
  hw_led_ins_0["rev"] = false;
  hw_led_ins_0["skip"] = skipFirstLed ? 1 : 0;

  //this is very crude and temporary
  byte ledType = TYPE_WS2812_RGB;
  if (strip.rgbwMode) ledType = TYPE_SK6812_RGBW;
  #ifdef USE_WS2801
    ledType = TYPE_WS2801;
  #endif
  #ifdef USE_APA102
    ledType = TYPE_APA102;
  #endif
  #ifdef USE_LPD8806
    ledType = TYPE_LPD8806;
  #endif
  #ifdef USE_P9813
    ledType = TYPE_P9813;
  #endif
  #ifdef USE_TM1814
    ledType = TYPE_TM1814;
  #endif

  hw_led_ins_0["type"] = ledType;

  JsonObject hw_btn = hw.createNestedObject("btn");

  JsonArray hw_btn_ins = hw_btn.createNestedArray("ins");

  JsonObject hw_btn_ins_0 = hw_btn_ins.createNestedObject();
  hw_btn_ins_0["type"] = (buttonEnabled) ? BTN_TYPE_PUSH : BTN_TYPE_NONE;

  JsonArray hw_btn_ins_0_pin = hw_btn_ins_0.createNestedArray("pin");
  hw_btn_ins_0_pin.add(BTNPIN);

  JsonArray hw_btn_ins_0_macros = hw_btn_ins_0.createNestedArray("macros");
  hw_btn_ins_0_macros.add(macroButton);
  hw_btn_ins_0_macros.add(macroLongPress);
  hw_btn_ins_0_macros.add(macroDoublePress);

  JsonObject hw_ir = hw.createNestedObject("ir");
  hw_ir["pin"] = IR_PIN;
  hw_ir["type"] = 0;

  JsonObject hw_relay = hw.createNestedObject("relay");
  hw_relay["pin"] = RLYPIN;
  hw_relay["rev"] = (RLYMDE) ? false : true;
  JsonObject hw_status = hw.createNestedObject("status");
  hw_status["pin"] = -1;

  JsonObject light = doc.createNestedObject("light");
  light["scale-bri"] = briMultiplier;
  light["pal-mode"] = strip.paletteBlend;

  JsonObject light_gc = light.createNestedObject("gc");
  light_gc["bri"] = (strip.gammaCorrectBri) ? 2.8 : 1.0;
  light_gc["col"] = (strip.gammaCorrectCol) ? 2.8 : 1.0;

  JsonObject light_tr = light.createNestedObject("tr");
  light_tr["mode"] = fadeTransition;
  light_tr["dur"] = transitionDelayDefault / 100;
  light_tr["pal"] = strip.paletteFade;

  JsonObject light_nl = light.createNestedObject("nl");
  light_nl["mode"] = nightlightMode;
  light_nl["dur"] = nightlightDelayMinsDefault;
  light_nl["tbri"] = nightlightTargetBri;
  light_nl["macro"] = macroNl;

  JsonObject def = doc.createNestedObject("def");
  def["ps"] = bootPreset;
  def["on"] = turnOnAtBoot;
  def["bri"] = briS;

  //to be removed once preset cycles are presets
  if (saveCurrPresetCycConf) {
    JsonObject def_cy = def.createNestedObject("cy");
    def_cy["on"] = presetCyclingEnabled;

    JsonArray def_cy_range = def_cy.createNestedArray("range");
    def_cy_range.add(presetCycleMin);
    def_cy_range.add(presetCycleMax);
    def_cy["dur"] = presetCycleTime / 100;
  }

  JsonObject interfaces = doc.createNestedObject("if");

  JsonObject if_sync = interfaces.createNestedObject("sync");
  if_sync["port0"] = udpPort;
  if_sync["port1"] = udpPort2;

  JsonObject if_sync_recv = if_sync.createNestedObject("recv");
  if_sync_recv["bri"] = receiveNotificationBrightness;
  if_sync_recv["col"] = receiveNotificationColor;
  if_sync_recv["fx"] = receiveNotificationEffects;

  JsonObject if_sync_send = if_sync.createNestedObject("send");
  if_sync_send["dir"] = notifyDirect;
  if_sync_send["btn"] = notifyButton;
  if_sync_send["va"] = notifyAlexa;
  if_sync_send["hue"] = notifyHue;
  if_sync_send["macro"] = notifyMacro;
  if_sync_send["twice"] = notifyTwice;

  JsonObject if_realtime = interfaces.createNestedObject("realtime");
  if_realtime["en"] = receiveDirect;
  if_realtime["port"] = e131Port;
  if_realtime["mc"] = e131Multicast;

  JsonObject if_realtime_dmx = if_realtime.createNestedObject("dmx");
  if_realtime_dmx["uni"] = e131Universe;
  if_realtime_dmx["seqskip"] = e131SkipOutOfSequence;
  if_realtime_dmx["addr"] = DMXAddress;
  if_realtime_dmx["mode"] = DMXMode;
  if_realtime["timeout"] = realtimeTimeout / 100;
  if_realtime["maxbri"] = arlsForceMaxBri;
  if_realtime["no-gc"] = arlsDisableGammaCorrection;
  if_realtime["offset"] = arlsOffset;

  JsonObject if_va = interfaces.createNestedObject("va");
  if_va["alexa"] = alexaEnabled;

  JsonArray if_va_macros = if_va.createNestedArray("macros");
  if_va_macros.add(macroAlexaOn);
  if_va_macros.add(macroAlexaOff);
  JsonObject if_blynk = interfaces.createNestedObject("blynk");
  if_blynk["token"] = strlen(blynkApiKey) ? "Hidden":"";

  JsonObject if_mqtt = interfaces.createNestedObject("mqtt");
  if_mqtt["en"] = mqttEnabled;
  if_mqtt["broker"] = mqttServer;
  if_mqtt["port"] = mqttPort;
  if_mqtt["user"] = mqttUser;
  if_mqtt["pskl"] = strlen(mqttPass);
  if_mqtt["cid"] = mqttClientID;

  JsonObject if_mqtt_topics = if_mqtt.createNestedObject("topics");
  if_mqtt_topics["device"] = mqttDeviceTopic;
  if_mqtt_topics["group"] = mqttGroupTopic;

  JsonObject if_hue = interfaces.createNestedObject("hue");
  if_hue["en"] = huePollingEnabled;
  if_hue["id"] = huePollLightId;
  if_hue["iv"] = huePollIntervalMs / 100;

  JsonObject if_hue_recv = if_hue.createNestedObject("recv");
  if_hue_recv["on"] = hueApplyOnOff;
  if_hue_recv["bri"] = hueApplyBri;
  if_hue_recv["col"] = hueApplyColor;

  JsonArray if_hue_ip = if_hue.createNestedArray("ip");
  for (byte i = 0; i < 4; i++) {
    if_hue_ip.add(hueIP[i]);
  }

  JsonObject if_ntp = interfaces.createNestedObject("ntp");
  if_ntp["en"] = ntpEnabled;
  if_ntp["host"] = ntpServerName;
  if_ntp["tz"] = currentTimezone;
  if_ntp["offset"] = utcOffsetSecs;
  if_ntp["ampm"] = useAMPM;

  JsonObject ol = doc.createNestedObject("ol");
  ol["clock"] = overlayDefault;
  ol["cntdwn"] = countdownMode;

  JsonObject timers = doc.createNestedObject("timers");

  JsonObject cntdwn = timers.createNestedObject("cntdwn");
  JsonArray goal = cntdwn.createNestedArray("goal");
  goal.add(countdownYear); goal.add(countdownMonth); goal.add(countdownDay);
  goal.add(countdownHour); goal.add(countdownMin); goal.add(countdownSec);
  cntdwn["macro"] = macroCountdown;

  JsonArray timers_ins = timers.createNestedArray("ins");

  for (byte i = 0; i < 8; i++) {
    if (timerMacro[i] == 0 && !(timerWeekday[i] & 0x01)) continue;
    JsonObject timers_ins0 = timers_ins.createNestedObject();
    timers_ins0["en"] = (timerWeekday[i] & 0x01);
    timers_ins0["hour"] = timerHours[i];
    timers_ins0["min"] = timerMinutes[i];
    timers_ins0["macro"] = timerMacro[i];
    timers_ins0["dow"] = timerWeekday[i] >> 1;
  }

  JsonObject ota = doc.createNestedObject("ota");
  ota["lock"] = otaLock;
  ota["lock-wifi"] = wifiLock;
  ota["pskl"] = strlen(otaPass);
  ota["aota"] = aOtaEnabled;
  //}

  File f = WLED_FS.open("/cfg.json", "w");
  if (f) serializeJson(doc, f);
  f.close();
}

//settings in /wsec.json, not accessible via webserver, for passwords and tokens
bool deserializeConfigSec() {
  DEBUG_PRINTLN(F("Reading settings from /wsec.json..."));

  DynamicJsonDocument doc(JSON_BUFFER_SIZE);

  bool success = readObjectFromFile("/wsec.json", nullptr, &doc);
  if (!success) return false;

  JsonObject nw_ins_0 = doc["nw"]["ins"][0];
  getStringFromJson(clientPass, nw_ins_0["psk"], 65);

  JsonObject ap = doc["ap"];
  getStringFromJson(apPass, ap["psk"] , 65);

  JsonObject interfaces = doc["if"];

  const char* apikey = interfaces["blynk"]["token"] | "Hidden";
  int tdd = strnlen(apikey, 36);
  if (tdd > 20 || tdd == 0)
    getStringFromJson(blynkApiKey, apikey, 36);

  JsonObject if_mqtt = interfaces["mqtt"];
  getStringFromJson(mqttPass, if_mqtt["psk"], 41);

  getStringFromJson(hueApiKey, interfaces["hue"]["key"], 47);

  JsonObject ota = doc["ota"];
  getStringFromJson(otaPass, ota["pwd"], 33);
  CJSON(otaLock, ota["lock"]);
  CJSON(wifiLock, ota["lock-wifi"]);
  CJSON(aOtaEnabled, ota["aota"]);

  return true;
}

void serializeConfigSec() {
  DEBUG_PRINTLN(F("Writing settings to /wsec.json..."));

  DynamicJsonDocument doc(JSON_BUFFER_SIZE);

  JsonObject nw = doc.createNestedObject("nw");

  JsonArray nw_ins = nw.createNestedArray("ins");

  JsonObject nw_ins_0 = nw_ins.createNestedObject();
  nw_ins_0["psk"] = clientPass;

  JsonObject ap = doc.createNestedObject("ap");
  ap["psk"] = apPass;

  JsonObject interfaces = doc.createNestedObject("if");
  JsonObject if_blynk = interfaces.createNestedObject("blynk");
  if_blynk["token"] = blynkApiKey;
  JsonObject if_mqtt = interfaces.createNestedObject("mqtt");
  if_mqtt["psk"] = mqttPass;
  JsonObject if_hue = interfaces.createNestedObject("hue");
  if_hue["key"] = hueApiKey;

  JsonObject ota = doc.createNestedObject("ota");
  ota["pwd"] = otaPass;
  ota["lock"] = otaLock;
  ota["lock-wifi"] = wifiLock;
  ota["aota"] = aOtaEnabled;

  File f = WLED_FS.open("/wsec.json", "w");
  if (f) serializeJson(doc, f);
  f.close();
}