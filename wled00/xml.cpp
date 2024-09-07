#include "wled.h"
#include "wled_ethernet.h"

/*
 * Sending XML status files to client
 */

//build XML response to HTTP /win API request
void XML_response(Print& dest)
{
  dest.printf_P(PSTR("<?xml version=\"1.0\" ?><vs><ac>%d</ac>"), (nightlightActive && nightlightMode > NL_MODE_SET) ? briT : bri);
  for (int i = 0; i < 3; i++)
  {
   dest.printf_P(PSTR("<cl>%d</cl>"), col[i]);
  }
  for (int i = 0; i < 3; i++)
  {
    dest.printf_P(PSTR("<cs>%d</cs>"), colSec[i]);
  }
  dest.printf_P(PSTR("<ns>%d</ns><nr>%d</nr><nl>%d</nl><nf>%d</nf><nd>%d</nd><nt>%d</nt><fx>%d</fx><sx>%d</sx><ix>%d</ix><fp>%d</fp><wv>%d</wv><ws>%d</ws><ps>%d</ps><cy>%d</cy><ds>%s%s</ds><ss>%d</ss></vs>"),
    notifyDirect, receiveGroups!=0, nightlightActive, nightlightMode > NL_MODE_SET, nightlightDelayMins,
    nightlightTargetBri, effectCurrent, effectSpeed, effectIntensity, effectPalette,
    strip.hasWhiteChannel() ? col[3] : -1, colSec[3], currentPreset, currentPlaylist >= 0,
    serverDescription, realtimeMode ? PSTR(" (live)") : "",
    strip.getFirstSelectedSegId()
  );
}

static void extractPin(Print& dest, JsonObject &obj, const char *key) {
  if (obj[key].is<JsonArray>()) {
    JsonArray pins = obj[key].as<JsonArray>();
    for (JsonVariant pv : pins) {
      if (pv.as<int>() > -1) { dest.print(","); dest.print(pv.as<int>()); }
    }
  } else {
    if (obj[key].as<int>() > -1) { dest.print(","); dest.print(obj[key].as<int>()); }
  }
}

// dest.print used pins by scanning JsonObject (1 level deep)
void fillUMPins(Print& dest, JsonObject &mods)
{
  for (JsonPair kv : mods) {
    // kv.key() is usermod name or subobject key
    // kv.value() is object itself
    JsonObject obj = kv.value();
    if (!obj.isNull()) {
      // element is an JsonObject
      if (!obj["pin"].isNull()) {
        extractPin(dest, obj, "pin");
      } else {
        // scan keys (just one level deep as is possible with usermods)
        for (JsonPair so : obj) {
          const char *key = so.key().c_str();
          if (strstr(key, "pin")) {
            // we found a key containing "pin" substring
            if (strlen(strstr(key, "pin")) == 3) {
              // and it is at the end, we found another pin
              extractPin(dest, obj, key);
              continue;
            }
          }
          if (!obj[so.key()].is<JsonObject>()) continue;
          JsonObject subObj = obj[so.key()];
          if (!subObj["pin"].isNull()) {
            // get pins from subobject
            extractPin(dest, subObj, "pin");
          }
        }
      }
    }
  }
}

void appendGPIOinfo(Print& dest) {
  dest.print(F("d.um_p=[-1")); // has to have 1 element
  if (i2c_sda > -1 && i2c_scl > -1) {
    dest.printf_P(PSTR(",%d,%d"), i2c_sda, i2c_scl);
  }
  if (spi_mosi > -1 && spi_sclk > -1) {
    dest.printf_P(PSTR(",%d,%d"), spi_mosi, spi_sclk);
  }
  // usermod pin reservations will become unnecessary when settings pages will read cfg.json directly
  if (requestJSONBufferLock(6)) {
    // if we can't allocate JSON buffer ignore usermod pins
    JsonObject mods = pDoc->createNestedObject(F("um"));
    usermods.addToConfig(mods);
    if (!mods.isNull()) fillUMPins(dest, mods);
    releaseJSONBufferLock();
  }
  dest.print(F("];"));

  // add reserved and usermod pins as d.um_p array
  #if defined(CONFIG_IDF_TARGET_ESP32S2)
  dest.print(F("d.rsvd=[22,23,24,25,26,27,28,29,30,31,32"));
  #elif defined(CONFIG_IDF_TARGET_ESP32S3)
  dest.print(F("d.rsvd=[19,20,22,23,24,25,26,27,28,29,30,31,32"));  // includes 19+20 for USB OTG (JTAG)
  if (psramFound()) dest.print(F(",33,34,35,36,37")); // in use for "octal" PSRAM or "octal" FLASH -seems that octal PSRAM is very common on S3.
  #elif defined(CONFIG_IDF_TARGET_ESP32C3)
  dest.print(F("d.rsvd=[11,12,13,14,15,16,17"));
  #elif defined(ESP32)
  dest.print(F("d.rsvd=[6,7,8,9,10,11,24,28,29,30,31,37,38"));
  if (!pinManager.isPinOk(16,false)) dest.print(F(",16")); // covers PICO & WROVER
  if (!pinManager.isPinOk(17,false)) dest.print(F(",17")); // covers PICO & WROVER
  #else
  dest.print(F("d.rsvd=[6,7,8,9,10,11"));
  #endif

  #ifdef WLED_ENABLE_DMX
  dest.print(F(",2")); // DMX hardcoded pin
  #endif

  #if defined(WLED_DEBUG) && !defined(WLED_DEBUG_HOST)
  dest.printf_P(PSTR(",%d"),hardwareTX); // debug output (TX) pin
  #endif

  //Note: Using pin 3 (RX) disables Adalight / Serial JSON

  #ifdef WLED_USE_ETHERNET
  if (ethernetType != WLED_ETH_NONE && ethernetType < WLED_NUM_ETH_TYPES) {
    for (unsigned p=0; p<WLED_ETH_RSVD_PINS_COUNT; p++) { dest.printf(",%d",esp32_nonconfigurable_ethernet_pins[p].pin); }
    if (ethernetBoards[ethernetType].eth_power>=0)     { dest.printf(",%d", ethernetBoards[ethernetType].eth_power); }
    if (ethernetBoards[ethernetType].eth_mdc>=0)       { dest.printf(",%d", ethernetBoards[ethernetType].eth_mdc); }
    if (ethernetBoards[ethernetType].eth_mdio>=0)      { dest.printf(",%d", ethernetBoards[ethernetType].eth_mdio); }
    switch (ethernetBoards[ethernetType].eth_clk_mode) {
      case ETH_CLOCK_GPIO0_IN:
      case ETH_CLOCK_GPIO0_OUT:
        dest.print(F(",0"));
        break;
      case ETH_CLOCK_GPIO16_OUT:
        dest.print(F(",16"));
        break;
      case ETH_CLOCK_GPIO17_OUT:
        dest.print(F(",17"));
        break;
    }
  }
  #endif

  dest.print(F("];"));

  // add info for read-only GPIO
  dest.print(F("d.ro_gpio=["));
  #if defined(CONFIG_IDF_TARGET_ESP32S2)
  dest.print(46);
  #elif defined(CONFIG_IDF_TARGET_ESP32S3)
  // none for S3
  #elif defined(CONFIG_IDF_TARGET_ESP32C3)
  // none for C3
  #elif defined(ESP32)
  dest.print(F("34,35,36,37,38,39"));
  #else
  // none for ESP8266
  #endif
  dest.print(F("];"));

  // add info about max. # of pins
  dest.print(F("d.max_gpio="));
  #if defined(CONFIG_IDF_TARGET_ESP32S2)
  dest.print(46);
  #elif defined(CONFIG_IDF_TARGET_ESP32S3)
  dest.print(48);
  #elif defined(CONFIG_IDF_TARGET_ESP32C3)
  dest.print(21);
  #elif defined(ESP32)
  dest.print(39);
  #else
  dest.print(16);
  #endif
  dest.print(F(";"));
}

//get values for settings form in javascript
void getSettingsJS(byte subPage, Print& dest)
{
  //0: menu 1: wifi 2: leds 3: ui 4: sync 5: time 6: sec
  DEBUG_PRINT(F("settings resp"));
  DEBUG_PRINTLN(subPage);

  if (subPage <0 || subPage >10) return;

  if (subPage == SUBPAGE_MENU)
  {
  #ifdef WLED_DISABLE_2D // include only if 2D is not compiled in
    dest.print(F("gId('2dbtn').style.display='none';"));
  #endif
  #ifdef WLED_ENABLE_DMX // include only if DMX is enabled
    dest.print(F("gId('dmxbtn').style.display='';"));
  #endif
  }

  if (subPage == SUBPAGE_WIFI)
  {
    size_t l;
    dest.print(F("resetWiFi(" TOSTRING(WLED_MAX_WIFI_COUNT) ");"));
    for (size_t n = 0; n < multiWiFi.size(); n++) {
      l = strlen(multiWiFi[n].clientPass);
      char fpass[l+1]; //fill password field with ***
      fpass[l] = 0;
      memset(fpass,'*',l);
      dest.printf_P(PSTR("addWiFi(\"%s\",\",%s\",0x%X,0x%X,0x%X);"), multiWiFi[n].clientSSID, fpass, (uint32_t) multiWiFi[n].staticIP, (uint32_t) multiWiFi[n].staticGW, (uint32_t) multiWiFi[n].staticSN);
    }

    sappend(dest,'v',SET_F("D0"),dnsAddress[0]);
    sappend(dest,'v',SET_F("D1"),dnsAddress[1]);
    sappend(dest,'v',SET_F("D2"),dnsAddress[2]);
    sappend(dest,'v',SET_F("D3"),dnsAddress[3]);

    sappends(dest,'s',SET_F("CM"),cmDNS);
    sappend(dest,'i',SET_F("AB"),apBehavior);
    sappends(dest,'s',SET_F("AS"),apSSID);
    sappend(dest,'c',SET_F("AH"),apHide);

    l = strlen(apPass);
    char fapass[l+1]; //fill password field with ***
    fapass[l] = 0;
    memset(fapass,'*',l);
    sappends(dest,'s',SET_F("AP"),fapass);

    sappend(dest,'v',SET_F("AC"),apChannel);
    #ifdef ARDUINO_ARCH_ESP32
    sappend(dest,'v',SET_F("TX"),txPower);
    #else
    dest.print(F("gId('tx').style.display='none';"));
    #endif
    sappend(dest,'c',SET_F("FG"),force802_3g);
    sappend(dest,'c',SET_F("WS"),noWifiSleep);

    #ifndef WLED_DISABLE_ESPNOW
    sappend(dest,'c',SET_F("RE"),enableESPNow);
    sappends(dest,'s',SET_F("RMAC"),linked_remote);
    #else
    //hide remote settings if not compiled
    dest.print(F("toggle('ESPNOW');"));  // hide ESP-NOW setting
    #endif

    #ifdef WLED_USE_ETHERNET
    sappend(dest,'v',SET_F("ETH"),ethernetType);
    #else
    //hide ethernet setting if not compiled in
    dest.print(F("gId('ethd').style.display='none';"));
    #endif

    if (Network.isConnected()) //is connected
    {
      char s[32];
      IPAddress localIP = Network.localIP();
      sprintf(s, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);

      #if defined(ARDUINO_ARCH_ESP32) && defined(WLED_USE_ETHERNET)
      if (Network.isEthernet()) strcat_P(s ,SET_F(" (Ethernet)"));
      #endif
      sappends(dest,'m',SET_F("(\"sip\")[0]"),s);
    } else
    {
      sappends(dest,'m',SET_F("(\"sip\")[0]"),(char*)F("Not connected"));
    }

    if (WiFi.softAPIP()[0] != 0) //is active
    {
      char s[16];
      IPAddress apIP = WiFi.softAPIP();
      sprintf(s, "%d.%d.%d.%d", apIP[0], apIP[1], apIP[2], apIP[3]);
      sappends(dest,'m',SET_F("(\"sip\")[1]"),s);
    } else
    {
      sappends(dest,'m',SET_F("(\"sip\")[1]"),(char*)F("Not active"));
    }

    #ifndef WLED_DISABLE_ESPNOW
    if (strlen(last_signal_src) > 0) { //Have seen an ESP-NOW Remote
      sappends(dest,'m',SET_F("(\"rlid\")[0]"),last_signal_src);
    } else if (!enableESPNow) {
      sappends(dest,'m',SET_F("(\"rlid\")[0]"),(char*)F("(Enable ESP-NOW to listen)"));
    } else {
      sappends(dest,'m',SET_F("(\"rlid\")[0]"),(char*)F("None"));
    }
    #endif
  }

  if (subPage == SUBPAGE_LEDS)
  {
    char nS[32];

    appendGPIOinfo(dest);

    // set limits
    dest.print(F("bLimits("
       TOSTRING(WLED_MAX_BUSSES) ","
       TOSTRING(WLED_MIN_VIRTUAL_BUSSES) ","
       TOSTRING(MAX_LEDS_PER_BUS) ","
       TOSTRING(MAX_LED_MEMORY) ","
       TOSTRING(MAX_LEDS) ","
       TOSTRING(WLED_MAX_COLOR_ORDER_MAPPINGS) ","
       TOSTRING(WLED_MAX_DIGITAL_CHANNELS) ","
       TOSTRING(WLED_MAX_ANALOG_CHANNELS) ");"
    ));

    sappend(dest,'c',SET_F("MS"),strip.autoSegments);
    sappend(dest,'c',SET_F("CCT"),strip.correctWB);
    sappend(dest,'c',SET_F("IC"),cctICused);
    sappend(dest,'c',SET_F("CR"),strip.cctFromRgb);
    sappend(dest,'v',SET_F("CB"),strip.cctBlending);
    sappend(dest,'v',SET_F("FR"),strip.getTargetFps());
    sappend(dest,'v',SET_F("AW"),Bus::getGlobalAWMode());
    sappend(dest,'c',SET_F("LD"),useGlobalLedBuffer);

    unsigned sumMa = 0;
    for (int s = 0; s < BusManager::getNumBusses(); s++) {
      Bus* bus = BusManager::getBus(s);
      if (bus == nullptr) continue;
      int offset = s < 10 ? 48 : 55;
      char lp[4] = "L0"; lp[2] = offset+s; lp[3] = 0; //ascii 0-9 //strip data pin
      char lc[4] = "LC"; lc[2] = offset+s; lc[3] = 0; //strip length
      char co[4] = "CO"; co[2] = offset+s; co[3] = 0; //strip color order
      char lt[4] = "LT"; lt[2] = offset+s; lt[3] = 0; //strip type
      char ls[4] = "LS"; ls[2] = offset+s; ls[3] = 0; //strip start LED
      char cv[4] = "CV"; cv[2] = offset+s; cv[3] = 0; //strip reverse
      char sl[4] = "SL"; sl[2] = offset+s; sl[3] = 0; //skip 1st LED
      char rf[4] = "RF"; rf[2] = offset+s; rf[3] = 0; //off refresh
      char aw[4] = "AW"; aw[2] = offset+s; aw[3] = 0; //auto white mode
      char wo[4] = "WO"; wo[2] = offset+s; wo[3] = 0; //swap channels
      char sp[4] = "SP"; sp[2] = offset+s; sp[3] = 0; //bus clock speed
      char la[4] = "LA"; la[2] = offset+s; la[3] = 0; //LED current
      char ma[4] = "MA"; ma[2] = offset+s; ma[3] = 0; //max per-port PSU current
      dest.print(F("addLEDs(1);"));
      uint8_t pins[5];
      int nPins = bus->getPins(pins);
      for (int i = 0; i < nPins; i++) {
        lp[1] = offset+i;
        if (pinManager.isPinOk(pins[i]) || IS_VIRTUAL(bus->getType())) sappend(dest,'v',lp,pins[i]);
      }
      sappend(dest,'v',lc,bus->getLength());
      sappend(dest,'v',lt,bus->getType());
      sappend(dest,'v',co,bus->getColorOrder() & 0x0F);
      sappend(dest,'v',ls,bus->getStart());
      sappend(dest,'c',cv,bus->isReversed());
      sappend(dest,'v',sl,bus->skippedLeds());
      sappend(dest,'c',rf,bus->isOffRefreshRequired());
      sappend(dest,'v',aw,bus->getAutoWhiteMode());
      sappend(dest,'v',wo,bus->getColorOrder() >> 4);
      unsigned speed = bus->getFrequency();
      if (IS_PWM(bus->getType())) {
        switch (speed) {
          case WLED_PWM_FREQ/2    : speed = 0; break;
          case WLED_PWM_FREQ*2/3  : speed = 1; break;
          default:
          case WLED_PWM_FREQ      : speed = 2; break;
          case WLED_PWM_FREQ*2    : speed = 3; break;
          case WLED_PWM_FREQ*10/3 : speed = 4; break; // uint16_t max (19531 * 3.333)
        }
      } else if (IS_DIGITAL(bus->getType()) && IS_2PIN(bus->getType())) {
        switch (speed) {
          case  1000 : speed = 0; break;
          case  2000 : speed = 1; break;
          default:
          case  5000 : speed = 2; break;
          case 10000 : speed = 3; break;
          case 20000 : speed = 4; break;
        }
      }
      sappend(dest,'v',sp,speed);
      sappend(dest,'v',la,bus->getLEDCurrent());
      sappend(dest,'v',ma,bus->getMaxCurrent());
      sumMa += bus->getMaxCurrent();
    }
    sappend(dest,'v',SET_F("MA"),BusManager::ablMilliampsMax() ? BusManager::ablMilliampsMax() : sumMa);
    sappend(dest,'c',SET_F("ABL"),BusManager::ablMilliampsMax() || sumMa > 0);
    sappend(dest,'c',SET_F("PPL"),!BusManager::ablMilliampsMax() && sumMa > 0);

    dest.print(F("resetCOM(" TOSTRING(WLED_MAX_COLOR_ORDER_MAPPINGS) ");"));
    const ColorOrderMap& com = BusManager::getColorOrderMap();
    for (int s = 0; s < com.count(); s++) {
      const ColorOrderMapEntry* entry = com.get(s);
      if (entry == nullptr) break;
      dest.printf_P(PSTR("addCOM(%d,%d,%d);"), entry->start, entry->len, entry->colorOrder);
    }

    sappend(dest,'v',SET_F("CA"),briS);

    sappend(dest,'c',SET_F("BO"),turnOnAtBoot);
    sappend(dest,'v',SET_F("BP"),bootPreset);

    sappend(dest,'c',SET_F("GB"),gammaCorrectBri);
    sappend(dest,'c',SET_F("GC"),gammaCorrectCol);
    dtostrf(gammaCorrectVal,3,1,nS); sappends(dest,'s',SET_F("GV"),nS);
    sappend(dest,'c',SET_F("TF"),fadeTransition);
    sappend(dest,'c',SET_F("EB"),modeBlending);
    sappend(dest,'v',SET_F("TD"),transitionDelayDefault);
    sappend(dest,'c',SET_F("PF"),strip.paletteFade);
    sappend(dest,'v',SET_F("TP"),randomPaletteChangeTime);
    sappend(dest,'c',SET_F("TH"),useHarmonicRandomPalette);
    sappend(dest,'v',SET_F("BF"),briMultiplier);
    sappend(dest,'v',SET_F("TB"),nightlightTargetBri);
    sappend(dest,'v',SET_F("TL"),nightlightDelayMinsDefault);
    sappend(dest,'v',SET_F("TW"),nightlightMode);
    sappend(dest,'i',SET_F("PB"),strip.paletteBlend);
    sappend(dest,'v',SET_F("RL"),rlyPin);
    sappend(dest,'c',SET_F("RM"),rlyMde);
    sappend(dest,'c',SET_F("RO"),rlyOpenDrain);
    for (int i = 0; i < WLED_MAX_BUTTONS; i++) {
      dest.printf_P(PSTR("addBtn(%d,%d,%d);"), i, btnPin[i], buttonType[i]);
    }
    sappend(dest,'c',SET_F("IP"),disablePullUp);
    sappend(dest,'v',SET_F("TT"),touchThreshold);
#ifndef WLED_DISABLE_INFRARED
    sappend(dest,'v',SET_F("IR"),irPin);
    sappend(dest,'v',SET_F("IT"),irEnabled);
#endif    
    sappend(dest,'c',SET_F("MSO"),!irApplyToAllSelected);
  }

  if (subPage == SUBPAGE_UI)
  {
    sappends(dest,'s',SET_F("DS"),serverDescription);
    sappend(dest,'c',SET_F("SU"),simplifiedUI);
  }

  if (subPage == SUBPAGE_SYNC)
  {
    [[maybe_unused]] char nS[32];
    sappend(dest,'v',SET_F("UP"),udpPort);
    sappend(dest,'v',SET_F("U2"),udpPort2);
  #ifndef WLED_DISABLE_ESPNOW
    if (enableESPNow) sappend(dest,'c',SET_F("EN"),useESPNowSync);
    else              dest.print(F("toggle('ESPNOW');"));  // hide ESP-NOW setting
  #else
    dest.print(F("toggle('ESPNOW');"));  // hide ESP-NOW setting
  #endif
    sappend(dest,'v',SET_F("GS"),syncGroups);
    sappend(dest,'v',SET_F("GR"),receiveGroups);

    sappend(dest,'c',SET_F("RB"),receiveNotificationBrightness);
    sappend(dest,'c',SET_F("RC"),receiveNotificationColor);
    sappend(dest,'c',SET_F("RX"),receiveNotificationEffects);
    sappend(dest,'c',SET_F("SO"),receiveSegmentOptions);
    sappend(dest,'c',SET_F("SG"),receiveSegmentBounds);
    sappend(dest,'c',SET_F("SS"),sendNotifications);
    sappend(dest,'c',SET_F("SD"),notifyDirect);
    sappend(dest,'c',SET_F("SB"),notifyButton);
    sappend(dest,'c',SET_F("SH"),notifyHue);
    sappend(dest,'v',SET_F("UR"),udpNumRetries);

    sappend(dest,'c',SET_F("NL"),nodeListEnabled);
    sappend(dest,'c',SET_F("NB"),nodeBroadcastEnabled);

    sappend(dest,'c',SET_F("RD"),receiveDirect);
    sappend(dest,'c',SET_F("MO"),useMainSegmentOnly);
    sappend(dest,'c',SET_F("RLM"),realtimeRespectLedMaps);
    sappend(dest,'v',SET_F("EP"),e131Port);
    sappend(dest,'c',SET_F("ES"),e131SkipOutOfSequence);
    sappend(dest,'c',SET_F("EM"),e131Multicast);
    sappend(dest,'v',SET_F("EU"),e131Universe);
    sappend(dest,'v',SET_F("DA"),DMXAddress);
    sappend(dest,'v',SET_F("XX"),DMXSegmentSpacing);
    sappend(dest,'v',SET_F("PY"),e131Priority);
    sappend(dest,'v',SET_F("DM"),DMXMode);
    sappend(dest,'v',SET_F("ET"),realtimeTimeoutMs);
    sappend(dest,'c',SET_F("FB"),arlsForceMaxBri);
    sappend(dest,'c',SET_F("RG"),arlsDisableGammaCorrection);
    sappend(dest,'v',SET_F("WO"),arlsOffset);
    #ifndef WLED_DISABLE_ALEXA
    sappend(dest,'c',SET_F("AL"),alexaEnabled);
    sappends(dest,'s',SET_F("AI"),alexaInvocationName);
    sappend(dest,'c',SET_F("SA"),notifyAlexa);
    sappend(dest,'v',SET_F("AP"),alexaNumPresets);
    #else
    dest.print(F("toggle('Alexa');"));  // hide Alexa settings
    #endif

    #ifndef WLED_DISABLE_MQTT
    sappend(dest,'c',SET_F("MQ"),mqttEnabled);
    sappends(dest,'s',SET_F("MS"),mqttServer);
    sappend(dest,'v',SET_F("MQPORT"),mqttPort);
    sappends(dest,'s',SET_F("MQUSER"),mqttUser);
    byte l = strlen(mqttPass);
    char fpass[l+1]; //fill password field with ***
    fpass[l] = 0;
    memset(fpass,'*',l);
    sappends(dest,'s',SET_F("MQPASS"),fpass);
    sappends(dest,'s',SET_F("MQCID"),mqttClientID);
    sappends(dest,'s',"MD",mqttDeviceTopic);
    sappends(dest,'s',SET_F("MG"),mqttGroupTopic);
    sappend(dest,'c',SET_F("BM"),buttonPublishMqtt);
    sappend(dest,'c',SET_F("RT"),retainMqttMsg);
    dest.print(F("d.Sf.MD.maxlength=" TOSTRING(MQTT_MAX_TOPIC_LEN) ";\n"
                 "d.Sf.MG.maxlength=" TOSTRING(MQTT_MAX_TOPIC_LEN) ";\n"
                 "d.Sf.MS.maxlength=" TOSTRING(MQTT_MAX_SERVER_LEN) ";"));
    #else
    dest.print(F("toggle('MQTT');"));    // hide MQTT settings
    #endif

    #ifndef WLED_DISABLE_HUESYNC
    sappend(dest,'v',SET_F("H0"),hueIP[0]);
    sappend(dest,'v',SET_F("H1"),hueIP[1]);
    sappend(dest,'v',SET_F("H2"),hueIP[2]);
    sappend(dest,'v',SET_F("H3"),hueIP[3]);
    sappend(dest,'v',SET_F("HL"),huePollLightId);
    sappend(dest,'v',SET_F("HI"),huePollIntervalMs);
    sappend(dest,'c',SET_F("HP"),huePollingEnabled);
    sappend(dest,'c',SET_F("HO"),hueApplyOnOff);
    sappend(dest,'c',SET_F("HB"),hueApplyBri);
    sappend(dest,'c',SET_F("HC"),hueApplyColor);
    char hueErrorString[25];
    switch (hueError)
    {
      case HUE_ERROR_INACTIVE     : strcpy_P(hueErrorString,PSTR("Inactive"));                break;
      case HUE_ERROR_ACTIVE       : strcpy_P(hueErrorString,PSTR("Active"));                  break;
      case HUE_ERROR_UNAUTHORIZED : strcpy_P(hueErrorString,PSTR("Unauthorized"));            break;
      case HUE_ERROR_LIGHTID      : strcpy_P(hueErrorString,PSTR("Invalid light ID"));        break;
      case HUE_ERROR_PUSHLINK     : strcpy_P(hueErrorString,PSTR("Link button not pressed")); break;
      case HUE_ERROR_JSON_PARSING : strcpy_P(hueErrorString,PSTR("JSON parsing error"));      break;
      case HUE_ERROR_TIMEOUT      : strcpy_P(hueErrorString,PSTR("Timeout"));                 break;
      default: sprintf_P(hueErrorString,PSTR("Bridge Error %i"),hueError);
    }

    sappends(dest,'m',SET_F("(\"sip\")[0]"),hueErrorString);
    #else
    dest.print(F("toggle('Hue');"));    // hide Hue Sync settings
    #endif
    sappend(dest,'v',SET_F("BD"),serialBaud);
  }

  if (subPage == SUBPAGE_TIME)
  {
    sappend(dest,'c',SET_F("NT"),ntpEnabled);
    sappends(dest,'s',SET_F("NS"),ntpServerName);
    sappend(dest,'c',SET_F("CF"),!useAMPM);
    sappend(dest,'i',SET_F("TZ"),currentTimezone);
    sappend(dest,'v',SET_F("UO"),utcOffsetSecs);
    char tm[32];
    dtostrf(longitude,4,2,tm);
    sappends(dest,'s',SET_F("LN"),tm);
    dtostrf(latitude,4,2,tm);
    sappends(dest,'s',SET_F("LT"),tm);
    getTimeString(tm);
    sappends(dest,'m',SET_F("(\"times\")[0]"),tm);
    if ((int)(longitude*10.0f) || (int)(latitude*10.0f)) {
      sprintf_P(tm, PSTR("Sunrise: %02d:%02d Sunset: %02d:%02d"), hour(sunrise), minute(sunrise), hour(sunset), minute(sunset));
      sappends(dest,'m',SET_F("(\"times\")[1]"),tm);
    }
    sappend(dest,'c',SET_F("OL"),overlayCurrent);
    sappend(dest,'v',SET_F("O1"),overlayMin);
    sappend(dest,'v',SET_F("O2"),overlayMax);
    sappend(dest,'v',SET_F("OM"),analogClock12pixel);
    sappend(dest,'c',SET_F("OS"),analogClockSecondsTrail);
    sappend(dest,'c',SET_F("O5"),analogClock5MinuteMarks);
    sappend(dest,'c',SET_F("OB"),analogClockSolidBlack);

    sappend(dest,'c',SET_F("CE"),countdownMode);
    sappend(dest,'v',SET_F("CY"),countdownYear);
    sappend(dest,'v',SET_F("CI"),countdownMonth);
    sappend(dest,'v',SET_F("CD"),countdownDay);
    sappend(dest,'v',SET_F("CH"),countdownHour);
    sappend(dest,'v',SET_F("CM"),countdownMin);
    sappend(dest,'v',SET_F("CS"),countdownSec);

    sappend(dest,'v',SET_F("A0"),macroAlexaOn);
    sappend(dest,'v',SET_F("A1"),macroAlexaOff);
    sappend(dest,'v',SET_F("MC"),macroCountdown);
    sappend(dest,'v',SET_F("MN"),macroNl);
    for (unsigned i=0; i<WLED_MAX_BUTTONS; i++) {
      dest.printf_P(PSTR("addRow(%d,%d,%d,%d);"), i, macroButton[i], macroLongPress[i], macroDoublePress[i]);
    }

    char k[4];
    k[2] = 0; //Time macros
    for (int i = 0; i<10; i++)
    {
      k[1] = 48+i; //ascii 0,1,2,3
      if (i<8) { k[0] = 'H'; sappend(dest,'v',k,timerHours[i]); }
      k[0] = 'N'; sappend(dest,'v',k,timerMinutes[i]);
      k[0] = 'T'; sappend(dest,'v',k,timerMacro[i]);
      k[0] = 'W'; sappend(dest,'v',k,timerWeekday[i]);
      if (i<8) {
        k[0] = 'M'; sappend(dest,'v',k,(timerMonth[i] >> 4) & 0x0F);
				k[0] = 'P'; sappend(dest,'v',k,timerMonth[i] & 0x0F);
        k[0] = 'D'; sappend(dest,'v',k,timerDay[i]);
				k[0] = 'E'; sappend(dest,'v',k,timerDayEnd[i]);
      }
    }
  }

  if (subPage == SUBPAGE_SEC)
  {
    byte l = strlen(settingsPIN);
    char fpass[l+1]; //fill PIN field with 0000
    fpass[l] = 0;
    memset(fpass,'0',l);
    sappends(dest,'s',SET_F("PIN"),fpass);
    sappend(dest,'c',SET_F("NO"),otaLock);
    sappend(dest,'c',SET_F("OW"),wifiLock);
    sappend(dest,'c',SET_F("AO"),aOtaEnabled);
    char msg_buf[256];
    snprintf_P(msg_buf,sizeof(msg_buf), PSTR("WLED %s (build " TOSTRING(VERSION) ")"), versionString);
    sappends(dest,'m',SET_F("(\"sip\")[0]"),msg_buf);
    dest.printf_P(PSTR("sd=\"%s\";"), serverDescription);
  }

  #ifdef WLED_ENABLE_DMX // include only if DMX is enabled
  if (subPage == SUBPAGE_DMX)
  {
    sappend(dest,'v',SET_F("PU"),e131ProxyUniverse);

    sappend(dest,'v',SET_F("CN"),DMXChannels);
    sappend(dest,'v',SET_F("CG"),DMXGap);
    sappend(dest,'v',SET_F("CS"),DMXStart);
    sappend(dest,'v',SET_F("SL"),DMXStartLED);

    sappend(dest,'i',SET_F("CH1"),DMXFixtureMap[0]);
    sappend(dest,'i',SET_F("CH2"),DMXFixtureMap[1]);
    sappend(dest,'i',SET_F("CH3"),DMXFixtureMap[2]);
    sappend(dest,'i',SET_F("CH4"),DMXFixtureMap[3]);
    sappend(dest,'i',SET_F("CH5"),DMXFixtureMap[4]);
    sappend(dest,'i',SET_F("CH6"),DMXFixtureMap[5]);
    sappend(dest,'i',SET_F("CH7"),DMXFixtureMap[6]);
    sappend(dest,'i',SET_F("CH8"),DMXFixtureMap[7]);
    sappend(dest,'i',SET_F("CH9"),DMXFixtureMap[8]);
    sappend(dest,'i',SET_F("CH10"),DMXFixtureMap[9]);
    sappend(dest,'i',SET_F("CH11"),DMXFixtureMap[10]);
    sappend(dest,'i',SET_F("CH12"),DMXFixtureMap[11]);
    sappend(dest,'i',SET_F("CH13"),DMXFixtureMap[12]);
    sappend(dest,'i',SET_F("CH14"),DMXFixtureMap[13]);
    sappend(dest,'i',SET_F("CH15"),DMXFixtureMap[14]);
  }
  #endif

  if (subPage == SUBPAGE_UM) //usermods
  {
    appendGPIOinfo(dest);
    dest.printf_P(PSTR("numM=%d;"), usermods.getModCount());
    sappend(dest,'v',SET_F("SDA"),i2c_sda);
    sappend(dest,'v',SET_F("SCL"),i2c_scl);
    sappend(dest,'v',SET_F("MOSI"),spi_mosi);
    sappend(dest,'v',SET_F("MISO"),spi_miso);
    sappend(dest,'v',SET_F("SCLK"),spi_sclk);
    dest.printf_P(PSTR("addInfo('SDA','%d');"
                 "addInfo('SCL','%d');"
                 "addInfo('MOSI','%d');"
                 "addInfo('MISO','%d');"
                 "addInfo('SCLK','%d');"),
      HW_PIN_SDA, HW_PIN_SCL, HW_PIN_DATASPI, HW_PIN_MISOSPI, HW_PIN_CLOCKSPI
    );
    usermods.appendConfigData(dest);
  }

  if (subPage == SUBPAGE_UPDATE) // update
  {
    char msg[256];
    snprintf_P(msg, sizeof(msg), PSTR("WLED %s<br>%s<br>(%s build %d)"),
      versionString,
      releaseString,
#if defined(ARDUINO_ARCH_ESP32)
      ESP.getChipModel(),
#else
      F("esp8266"),
  #endif
      VERSION
    );

    sappends(dest,'m',SET_F("(\"sip\")[0]"),msg);
  }

  if (subPage == SUBPAGE_2D) // 2D matrices
  {
    sappend(dest,'v',SET_F("SOMP"),strip.isMatrix);
    #ifndef WLED_DISABLE_2D
    dest.print(F("maxPanels=")); dest.print(WLED_MAX_PANELS); dest.print(F(";"));
    dest.print(F("resetPanels();"));
    if (strip.isMatrix) {
      if(strip.panels>0){
        sappend(dest,'v',SET_F("PW"),strip.panel[0].width); //Set generator Width and Height to first panel size for convenience
        sappend(dest,'v',SET_F("PH"),strip.panel[0].height);
      }
      sappend(dest,'v',SET_F("MPC"),strip.panels);
      // panels
      for (unsigned i=0; i<strip.panels; i++) {
        char n[5];
        dest.print(F("addPanel("));
        dest.print(itoa(i,n,10));
        dest.print(F(");"));
        char pO[8] = { '\0' };
        snprintf_P(pO, 7, PSTR("P%d"), i);       // MAX_PANELS is 64 so pO will always only be 4 characters or less
        pO[7] = '\0';
        unsigned l = strlen(pO);
        // create P0B, P1B, ..., P63B, etc for other PxxX
        pO[l] = 'B'; sappend(dest,'v',pO,strip.panel[i].bottomStart);
        pO[l] = 'R'; sappend(dest,'v',pO,strip.panel[i].rightStart);
        pO[l] = 'V'; sappend(dest,'v',pO,strip.panel[i].vertical);
        pO[l] = 'S'; sappend(dest,'c',pO,strip.panel[i].serpentine);
        pO[l] = 'X'; sappend(dest,'v',pO,strip.panel[i].xOffset);
        pO[l] = 'Y'; sappend(dest,'v',pO,strip.panel[i].yOffset);
        pO[l] = 'W'; sappend(dest,'v',pO,strip.panel[i].width);
        pO[l] = 'H'; sappend(dest,'v',pO,strip.panel[i].height);
      }
    }
    #else
    dest.print(F("gId(\"somp\").remove(1);")); // remove 2D option from dropdown
    #endif
  }
}
