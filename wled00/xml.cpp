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

  // add reserved (unusable) pins
  dest.print(SET_F("d.rsvd=["));
  for (unsigned i = 0; i < WLED_NUM_PINS; i++) {
    if (!pinManager.isPinOk(i, false)) {  // include readonly pins
      dest.print(i); dest.print(",");
    }
  }
  #ifdef WLED_ENABLE_DMX
  dest.print(SET_F("2,")); // DMX hardcoded pin
  #endif
  #if defined(WLED_DEBUG) && !defined(WLED_DEBUG_HOST)
  dest.printf_P(PSTR(",%d"),hardwareTX); // debug output (TX) pin
  #endif
  //Note: Using pin 3 (RX) disables Adalight / Serial JSON
  #ifdef WLED_USE_ETHERNET
  if (ethernetType != WLED_ETH_NONE && ethernetType < WLED_NUM_ETH_TYPES) {
    for (unsigned p=0; p<WLED_ETH_RSVD_PINS_COUNT; p++) { dest.printf(",%d", esp32_nonconfigurable_ethernet_pins[p].pin); }
    if (ethernetBoards[ethernetType].eth_power>=0)     { dest.printf(",%d", ethernetBoards[ethernetType].eth_power); }
    if (ethernetBoards[ethernetType].eth_mdc>=0)       { dest.printf(",%d", ethernetBoards[ethernetType].eth_mdc); }
    if (ethernetBoards[ethernetType].eth_mdio>=0)      { dest.printf(",%d", ethernetBoards[ethernetType].eth_mdio); }
    switch (ethernetBoards[ethernetType].eth_clk_mode) {
      case ETH_CLOCK_GPIO0_IN:
      case ETH_CLOCK_GPIO0_OUT:
        dest.print(SET_F("0"));
        break;
      case ETH_CLOCK_GPIO16_OUT:
        dest.print(SET_F("16"));
        break;
      case ETH_CLOCK_GPIO17_OUT:
        dest.print(SET_F("17"));
        break;
    }
  }
  #endif
  dest.print(SET_F("];")); // rsvd

  // add info for read-only GPIO
  dest.print(SET_F("d.ro_gpio=["));
  bool firstPin = true;
  for (unsigned i = 0; i < WLED_NUM_PINS; i++) {
    if (pinManager.isReadOnlyPin(i)) {
      // No comma before the first pin
      if (!firstPin) dest.print(SET_F(","));
      dest.print(i);
      firstPin = false;
    }
  }
  dest.print(SET_F("];"));

  // add info about max. # of pins
  dest.print(SET_F("d.max_gpio="));
  dest.print(WLED_NUM_PINS);
  dest.print(SET_F(";"));
}

//get values for settings form in javascript
void getSettingsJS(byte subPage, Print& dest)
{
  //0: menu 1: wifi 2: leds 3: ui 4: sync 5: time 6: sec
  DEBUG_PRINTF_P(PSTR("settings resp %u\n"), (unsigned)subPage);

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
    dest.printf_P(PSTR("resetWiFi(%d);"), WLED_MAX_WIFI_COUNT);
    for (size_t n = 0; n < multiWiFi.size(); n++) {
      l = strlen(multiWiFi[n].clientPass);
      char fpass[l+1]; //fill password field with ***
      fpass[l] = 0;
      memset(fpass,'*',l);
      dest.printf_P(PSTR("addWiFi(\"%s\",\",%s\",0x%X,0x%X,0x%X);"),
        multiWiFi[n].clientSSID,
        fpass,
        (uint32_t) multiWiFi[n].staticIP, // explicit cast required as this is a struct
        (uint32_t) multiWiFi[n].staticGW,
        (uint32_t) multiWiFi[n].staticSN);
    }

    printSetValue(dest,PSTR("D0"),dnsAddress[0]);
    printSetValue(dest,PSTR("D1"),dnsAddress[1]);
    printSetValue(dest,PSTR("D2"),dnsAddress[2]);
    printSetValue(dest,PSTR("D3"),dnsAddress[3]);

    printSetValue(dest,PSTR("CM"),cmDNS);
    printSetIndex(dest,PSTR("AB"),apBehavior);
    printSetValue(dest,PSTR("AS"),apSSID);
    printSetCheckbox(dest,PSTR("AH"),apHide);

    l = strlen(apPass);
    char fapass[l+1]; //fill password field with ***
    fapass[l] = 0;
    memset(fapass,'*',l);
    printSetValue(dest,PSTR("AP"),fapass);

    printSetValue(dest,PSTR("AC"),apChannel);
    #ifdef ARDUINO_ARCH_ESP32
    printSetValue(dest,PSTR("TX"),txPower);
    #else
    dest.print(F("gId('tx').style.display='none';"));
    #endif
    printSetCheckbox(dest,PSTR("FG"),force802_3g);
    printSetCheckbox(dest,PSTR("WS"),noWifiSleep);

    #ifndef WLED_DISABLE_ESPNOW
    printSetCheckbox(dest,PSTR("RE"),enableESPNow);
    printSetValue(dest,PSTR("RMAC"),linked_remote);
    #else
    //hide remote settings if not compiled
    dest.print(F("toggle('ESPNOW');"));  // hide ESP-NOW setting
    #endif

    #ifdef WLED_USE_ETHERNET
    printSetValue(dest,PSTR("ETH"),ethernetType);
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
      printSetMessage(dest,PSTR("(\"sip\")[0]"),s);
    } else
    {
      printSetMessage(dest,PSTR("(\"sip\")[0]"),(char*)F("Not connected"));
    }

    if (WiFi.softAPIP()[0] != 0) //is active
    {
      char s[16];
      IPAddress apIP = WiFi.softAPIP();
      sprintf(s, "%d.%d.%d.%d", apIP[0], apIP[1], apIP[2], apIP[3]);
      printSetMessage(dest,PSTR("(\"sip\")[1]"),s);
    } else
    {
      printSetMessage(dest,PSTR("(\"sip\")[1]"),(char*)F("Not active"));
    }

    #ifndef WLED_DISABLE_ESPNOW
    if (strlen(last_signal_src) > 0) { //Have seen an ESP-NOW Remote
      printSetMessage(dest,PSTR("(\"rlid\")[0]"),last_signal_src);
    } else if (!enableESPNow) {
      printSetMessage(dest,PSTR("(\"rlid\")[0]"),(char*)F("(Enable ESP-NOW to listen)"));
    } else {
      printSetMessage(dest,PSTR("(\"rlid\")[0]"),(char*)F("None"));
    }
    #endif
  }

  if (subPage == SUBPAGE_LEDS)
  {
    char nS[32];

    appendGPIOinfo(dest);

    dest.print(SET_F("d.ledTypes=")); dest.print(BusManager::getLEDTypesJSONString().c_str()); dest.print(";");

    // set limits
    dest.printf_P(PSTR("bLimits(%d,%d,%d,%d,%d,%d,%d,%d);"),
      WLED_MAX_BUSSES,
      WLED_MIN_VIRTUAL_BUSSES,
      MAX_LEDS_PER_BUS,
      MAX_LED_MEMORY,
      MAX_LEDS,
      WLED_MAX_COLOR_ORDER_MAPPINGS,
      WLED_MAX_DIGITAL_CHANNELS,
      WLED_MAX_ANALOG_CHANNELS
    );

    printSetCheckbox(dest,PSTR("MS"),strip.autoSegments);
    printSetCheckbox(dest,PSTR("CCT"),strip.correctWB);
    printSetCheckbox(dest,PSTR("IC"),cctICused);
    printSetCheckbox(dest,PSTR("CR"),strip.cctFromRgb);
    printSetValue(dest,PSTR("CB"),strip.cctBlending);
    printSetValue(dest,PSTR("FR"),strip.getTargetFps());
    printSetValue(dest,PSTR("AW"),Bus::getGlobalAWMode());
    printSetCheckbox(dest,PSTR("LD"),useGlobalLedBuffer);

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
        if (pinManager.isPinOk(pins[i]) || bus->isVirtual()) printSetValue(dest,lp,pins[i]);
      }
      printSetValue(dest,lc,bus->getLength());
      printSetValue(dest,lt,bus->getType());
      printSetValue(dest,co,bus->getColorOrder() & 0x0F);
      printSetValue(dest,ls,bus->getStart());
      printSetCheckbox(dest,cv,bus->isReversed());
      printSetValue(dest,sl,bus->skippedLeds());
      printSetCheckbox(dest,rf,bus->isOffRefreshRequired());
      printSetValue(dest,aw,bus->getAutoWhiteMode());
      printSetValue(dest,wo,bus->getColorOrder() >> 4);
      unsigned speed = bus->getFrequency();
      if (bus->isPWM()) {
        switch (speed) {
          case WLED_PWM_FREQ/2    : speed = 0; break;
          case WLED_PWM_FREQ*2/3  : speed = 1; break;
          default:
          case WLED_PWM_FREQ      : speed = 2; break;
          case WLED_PWM_FREQ*2    : speed = 3; break;
          case WLED_PWM_FREQ*10/3 : speed = 4; break; // uint16_t max (19531 * 3.333)
        }
      } else if (bus->is2Pin()) {
        switch (speed) {
          case  1000 : speed = 0; break;
          case  2000 : speed = 1; break;
          default:
          case  5000 : speed = 2; break;
          case 10000 : speed = 3; break;
          case 20000 : speed = 4; break;
        }
      }
      printSetValue(dest,sp,speed);
      printSetValue(dest,la,bus->getLEDCurrent());
      printSetValue(dest,ma,bus->getMaxCurrent());
      sumMa += bus->getMaxCurrent();
    }
    printSetValue(dest,PSTR("MA"),BusManager::ablMilliampsMax() ? BusManager::ablMilliampsMax() : sumMa);
    printSetCheckbox(dest,PSTR("ABL"),BusManager::ablMilliampsMax() || sumMa > 0);
    printSetCheckbox(dest,PSTR("PPL"),!BusManager::ablMilliampsMax() && sumMa > 0);

    dest.printf_P(PSTR("resetCOM(%d);"), WLED_MAX_COLOR_ORDER_MAPPINGS);
    const ColorOrderMap& com = BusManager::getColorOrderMap();
    for (int s = 0; s < com.count(); s++) {
      const ColorOrderMapEntry* entry = com.get(s);
      if (entry == nullptr) break;
      dest.printf_P(PSTR("addCOM(%d,%d,%d);"), entry->start, entry->len, entry->colorOrder);
    }

    printSetValue(dest,PSTR("CA"),briS);

    printSetCheckbox(dest,PSTR("BO"),turnOnAtBoot);
    printSetValue(dest,PSTR("BP"),bootPreset);

    printSetCheckbox(dest,PSTR("GB"),gammaCorrectBri);
    printSetCheckbox(dest,PSTR("GC"),gammaCorrectCol);
    dtostrf(gammaCorrectVal,3,1,nS); printSetValue(dest,PSTR("GV"),nS);
    printSetCheckbox(dest,PSTR("TF"),fadeTransition);
    printSetCheckbox(dest,PSTR("EB"),modeBlending);
    printSetValue(dest,PSTR("TD"),transitionDelayDefault);
    printSetCheckbox(dest,PSTR("PF"),strip.paletteFade);
    printSetValue(dest,PSTR("TP"),randomPaletteChangeTime);
    printSetCheckbox(dest,PSTR("TH"),useHarmonicRandomPalette);
    printSetValue(dest,PSTR("BF"),briMultiplier);
    printSetValue(dest,PSTR("TB"),nightlightTargetBri);
    printSetValue(dest,PSTR("TL"),nightlightDelayMinsDefault);
    printSetValue(dest,PSTR("TW"),nightlightMode);
    printSetIndex(dest,PSTR("PB"),strip.paletteBlend);
    printSetValue(dest,PSTR("RL"),rlyPin);
    printSetCheckbox(dest,PSTR("RM"),rlyMde);
    printSetCheckbox(dest,PSTR("RO"),rlyOpenDrain);
    for (int i = 0; i < WLED_MAX_BUTTONS; i++) {
      dest.printf_P(PSTR("addBtn(%d,%d,%d);"), i, btnPin[i], buttonType[i]);
    }
    printSetCheckbox(dest,PSTR("IP"),disablePullUp);
    printSetValue(dest,PSTR("TT"),touchThreshold);
#ifndef WLED_DISABLE_INFRARED
    printSetValue(dest,PSTR("IR"),irPin);
    printSetValue(dest,PSTR("IT"),irEnabled);
#endif    
    printSetCheckbox(dest,PSTR("MSO"),!irApplyToAllSelected);
  }

  if (subPage == SUBPAGE_UI)
  {
    printSetValue(dest,PSTR("DS"),serverDescription);
    printSetCheckbox(dest,PSTR("SU"),simplifiedUI);
  }

  if (subPage == SUBPAGE_SYNC)
  {
    [[maybe_unused]] char nS[32];
    printSetValue(dest,PSTR("UP"),udpPort);
    printSetValue(dest,PSTR("U2"),udpPort2);
  #ifndef WLED_DISABLE_ESPNOW
    if (enableESPNow) printSetCheckbox(dest,PSTR("EN"),useESPNowSync);
    else              dest.print(F("toggle('ESPNOW');"));  // hide ESP-NOW setting
  #else
    dest.print(F("toggle('ESPNOW');"));  // hide ESP-NOW setting
  #endif
    printSetValue(dest,PSTR("GS"),syncGroups);
    printSetValue(dest,PSTR("GR"),receiveGroups);

    printSetCheckbox(dest,PSTR("RB"),receiveNotificationBrightness);
    printSetCheckbox(dest,PSTR("RC"),receiveNotificationColor);
    printSetCheckbox(dest,PSTR("RX"),receiveNotificationEffects);
    printSetCheckbox(dest,PSTR("RP"),receiveNotificationPalette);
    printSetCheckbox(dest,PSTR("SO"),receiveSegmentOptions);
    printSetCheckbox(dest,PSTR("SG"),receiveSegmentBounds);
    printSetCheckbox(dest,PSTR("SS"),sendNotifications);
    printSetCheckbox(dest,PSTR("SD"),notifyDirect);
    printSetCheckbox(dest,PSTR("SB"),notifyButton);
    printSetCheckbox(dest,PSTR("SH"),notifyHue);
    printSetValue(dest,PSTR("UR"),udpNumRetries);

    printSetCheckbox(dest,PSTR("NL"),nodeListEnabled);
    printSetCheckbox(dest,PSTR("NB"),nodeBroadcastEnabled);

    printSetCheckbox(dest,PSTR("RD"),receiveDirect);
    printSetCheckbox(dest,PSTR("MO"),useMainSegmentOnly);
    printSetCheckbox(dest,PSTR("RLM"),realtimeRespectLedMaps);
    printSetValue(dest,PSTR("EP"),e131Port);
    printSetCheckbox(dest,PSTR("ES"),e131SkipOutOfSequence);
    printSetCheckbox(dest,PSTR("EM"),e131Multicast);
    printSetValue(dest,PSTR("EU"),e131Universe);
    printSetValue(dest,PSTR("DA"),DMXAddress);
    printSetValue(dest,PSTR("XX"),DMXSegmentSpacing);
    printSetValue(dest,PSTR("PY"),e131Priority);
    printSetValue(dest,PSTR("DM"),DMXMode);
    printSetValue(dest,PSTR("ET"),realtimeTimeoutMs);
    printSetCheckbox(dest,PSTR("FB"),arlsForceMaxBri);
    printSetCheckbox(dest,PSTR("RG"),arlsDisableGammaCorrection);
    printSetValue(dest,PSTR("WO"),arlsOffset);
    #ifndef WLED_DISABLE_ALEXA
    printSetCheckbox(dest,PSTR("AL"),alexaEnabled);
    printSetValue(dest,PSTR("AI"),alexaInvocationName);
    printSetCheckbox(dest,PSTR("SA"),notifyAlexa);
    printSetValue(dest,PSTR("AP"),alexaNumPresets);
    #else
    dest.print(F("toggle('Alexa');"));  // hide Alexa settings
    #endif

    #ifndef WLED_DISABLE_MQTT
    printSetCheckbox(dest,PSTR("MQ"),mqttEnabled);
    printSetValue(dest,PSTR("MS"),mqttServer);
    printSetValue(dest,PSTR("MQPORT"),mqttPort);
    printSetValue(dest,PSTR("MQUSER"),mqttUser);
    byte l = strlen(mqttPass);
    char fpass[l+1]; //fill password field with ***
    fpass[l] = 0;
    memset(fpass,'*',l);
    printSetValue(dest,PSTR("MQPASS"),fpass);
    printSetValue(dest,PSTR("MQCID"),mqttClientID);
    printSetValue(dest,PSTR("MD"),mqttDeviceTopic);
    printSetValue(dest,PSTR("MG"),mqttGroupTopic);
    printSetCheckbox(dest,PSTR("BM"),buttonPublishMqtt);
    printSetCheckbox(dest,PSTR("RT"),retainMqttMsg);
    dest.printf_P(PSTR("d.Sf.MD.maxlength=%d;d.Sf.MG.maxlength=%d;d.Sf.MS.maxlength=%d;"),
                  MQTT_MAX_TOPIC_LEN, MQTT_MAX_TOPIC_LEN, MQTT_MAX_SERVER_LEN);
    #else
    dest.print(F("toggle('MQTT');"));    // hide MQTT settings
    #endif

    #ifndef WLED_DISABLE_HUESYNC
    printSetValue(dest,PSTR("H0"),hueIP[0]);
    printSetValue(dest,PSTR("H1"),hueIP[1]);
    printSetValue(dest,PSTR("H2"),hueIP[2]);
    printSetValue(dest,PSTR("H3"),hueIP[3]);
    printSetValue(dest,PSTR("HL"),huePollLightId);
    printSetValue(dest,PSTR("HI"),huePollIntervalMs);
    printSetCheckbox(dest,PSTR("HP"),huePollingEnabled);
    printSetCheckbox(dest,PSTR("HO"),hueApplyOnOff);
    printSetCheckbox(dest,PSTR("HB"),hueApplyBri);
    printSetCheckbox(dest,PSTR("HC"),hueApplyColor);
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

    printSetMessage(dest,PSTR("(\"sip\")[0]"),hueErrorString);
    #else
    dest.print(F("toggle('Hue');"));    // hide Hue Sync settings
    #endif
    printSetValue(dest,PSTR("BD"),serialBaud);
    #ifndef WLED_ENABLE_ADALIGHT
    dest.print(SET_F("toggle('Serial);"));
    #endif
  }

  if (subPage == SUBPAGE_TIME)
  {
    printSetCheckbox(dest,PSTR("NT"),ntpEnabled);
    printSetValue(dest,PSTR("NS"),ntpServerName);
    printSetCheckbox(dest,PSTR("CF"),!useAMPM);
    printSetIndex(dest,PSTR("TZ"),currentTimezone);
    printSetValue(dest,PSTR("UO"),utcOffsetSecs);
    char tm[32];
    dtostrf(longitude,4,2,tm);
    printSetValue(dest,PSTR("LN"),tm);
    dtostrf(latitude,4,2,tm);
    printSetValue(dest,PSTR("LT"),tm);
    getTimeString(tm);
    printSetMessage(dest,PSTR("(\"times\")[0]"),tm);
    if ((int)(longitude*10.0f) || (int)(latitude*10.0f)) {
      sprintf_P(tm, PSTR("Sunrise: %02d:%02d Sunset: %02d:%02d"), hour(sunrise), minute(sunrise), hour(sunset), minute(sunset));
      printSetMessage(dest,PSTR("(\"times\")[1]"),tm);
    }
    printSetCheckbox(dest,PSTR("OL"),overlayCurrent);
    printSetValue(dest,PSTR("O1"),overlayMin);
    printSetValue(dest,PSTR("O2"),overlayMax);
    printSetValue(dest,PSTR("OM"),analogClock12pixel);
    printSetCheckbox(dest,PSTR("OS"),analogClockSecondsTrail);
    printSetCheckbox(dest,PSTR("O5"),analogClock5MinuteMarks);
    printSetCheckbox(dest,PSTR("OB"),analogClockSolidBlack);

    printSetCheckbox(dest,PSTR("CE"),countdownMode);
    printSetValue(dest,PSTR("CY"),countdownYear);
    printSetValue(dest,PSTR("CI"),countdownMonth);
    printSetValue(dest,PSTR("CD"),countdownDay);
    printSetValue(dest,PSTR("CH"),countdownHour);
    printSetValue(dest,PSTR("CM"),countdownMin);
    printSetValue(dest,PSTR("CS"),countdownSec);

    printSetValue(dest,PSTR("A0"),macroAlexaOn);
    printSetValue(dest,PSTR("A1"),macroAlexaOff);
    printSetValue(dest,PSTR("MC"),macroCountdown);
    printSetValue(dest,PSTR("MN"),macroNl);
    for (unsigned i=0; i<WLED_MAX_BUTTONS; i++) {
      dest.printf_P(PSTR("addRow(%d,%d,%d,%d);"), i, macroButton[i], macroLongPress[i], macroDoublePress[i]);
    }

    char k[4];
    k[2] = 0; //Time macros
    for (int i = 0; i<10; i++)
    {
      k[1] = 48+i; //ascii 0,1,2,3
      if (i<8) { k[0] = 'H'; printSetValue(dest,k,timerHours[i]); }
      k[0] = 'N'; printSetValue(dest,k,timerMinutes[i]);
      k[0] = 'T'; printSetValue(dest,k,timerMacro[i]);
      k[0] = 'W'; printSetValue(dest,k,timerWeekday[i]);
      if (i<8) {
        k[0] = 'M'; printSetValue(dest,k,(timerMonth[i] >> 4) & 0x0F);
				k[0] = 'P'; printSetValue(dest,k,timerMonth[i] & 0x0F);
        k[0] = 'D'; printSetValue(dest,k,timerDay[i]);
				k[0] = 'E'; printSetValue(dest,k,timerDayEnd[i]);
      }
    }
  }

  if (subPage == SUBPAGE_SEC)
  {
    byte l = strlen(settingsPIN);
    char fpass[l+1]; //fill PIN field with 0000
    fpass[l] = 0;
    memset(fpass,'0',l);
    printSetValue(dest,PSTR("PIN"),fpass);
    printSetCheckbox(dest,PSTR("NO"),otaLock);
    printSetCheckbox(dest,PSTR("OW"),wifiLock);
    printSetCheckbox(dest,PSTR("AO"),aOtaEnabled);
    char tmp_buf[128];
    snprintf_P(tmp_buf,sizeof(tmp_buf),PSTR("WLED %s (build %d)"),versionString,VERSION);
    printSetMessage(dest,PSTR("(\"sip\")[0]"),tmp_buf);
    dest.printf_P(PSTR("sd=\"%s\";"), serverDescription);
  }

  #ifdef WLED_ENABLE_DMX // include only if DMX is enabled
  if (subPage == SUBPAGE_DMX)
  {
    printSetValue(dest,PSTR("PU"),e131ProxyUniverse);

    printSetValue(dest,PSTR("CN"),DMXChannels);
    printSetValue(dest,PSTR("CG"),DMXGap);
    printSetValue(dest,PSTR("CS"),DMXStart);
    printSetValue(dest,PSTR("SL"),DMXStartLED);

    printSetIndex(dest,PSTR("CH1"),DMXFixtureMap[0]);
    printSetIndex(dest,PSTR("CH2"),DMXFixtureMap[1]);
    printSetIndex(dest,PSTR("CH3"),DMXFixtureMap[2]);
    printSetIndex(dest,PSTR("CH4"),DMXFixtureMap[3]);
    printSetIndex(dest,PSTR("CH5"),DMXFixtureMap[4]);
    printSetIndex(dest,PSTR("CH6"),DMXFixtureMap[5]);
    printSetIndex(dest,PSTR("CH7"),DMXFixtureMap[6]);
    printSetIndex(dest,PSTR("CH8"),DMXFixtureMap[7]);
    printSetIndex(dest,PSTR("CH9"),DMXFixtureMap[8]);
    printSetIndex(dest,PSTR("CH10"),DMXFixtureMap[9]);
    printSetIndex(dest,PSTR("CH11"),DMXFixtureMap[10]);
    printSetIndex(dest,PSTR("CH12"),DMXFixtureMap[11]);
    printSetIndex(dest,PSTR("CH13"),DMXFixtureMap[12]);
    printSetIndex(dest,PSTR("CH14"),DMXFixtureMap[13]);
    printSetIndex(dest,PSTR("CH15"),DMXFixtureMap[14]);
  }
  #endif

  if (subPage == SUBPAGE_UM) //usermods
  {
    appendGPIOinfo(dest);
    dest.printf_P(PSTR("numM=%d;"), usermods.getModCount());
    printSetValue(dest,PSTR("SDA"),i2c_sda);
    printSetValue(dest,PSTR("SCL"),i2c_scl);
    printSetValue(dest,PSTR("MOSI"),spi_mosi);
    printSetValue(dest,PSTR("MISO"),spi_miso);
    printSetValue(dest,PSTR("SCLK"),spi_sclk);
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
    char tmp_buf[128];
    snprintf_P(tmp_buf,sizeof(tmp_buf),PSTR("WLED %s<br>%s<br>(%s build %d)"),
      versionString,
      releaseString,
    #if defined(ARDUINO_ARCH_ESP32)
      ESP.getChipModel(),
    #else
      F("esp8266"),
    #endif
      VERSION);

    printSetMessage(dest,PSTR("(\"sip\")[0]"),tmp_buf);
  }

  if (subPage == SUBPAGE_2D) // 2D matrices
  {
    printSetValue(dest,PSTR("SOMP"),strip.isMatrix);
    #ifndef WLED_DISABLE_2D
    dest.print(F("maxPanels=")); dest.print(WLED_MAX_PANELS); dest.print(F(";"));
    dest.print(F("resetPanels();"));
    if (strip.isMatrix) {
      if(strip.panels>0){
        printSetValue(dest,PSTR("PW"),strip.panel[0].width); //Set generator Width and Height to first panel size for convenience
        printSetValue(dest,PSTR("PH"),strip.panel[0].height);
      }
      printSetValue(dest,PSTR("MPC"),strip.panels);
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
        pO[l] = 'B'; printSetValue(dest,pO,strip.panel[i].bottomStart);
        pO[l] = 'R'; printSetValue(dest,pO,strip.panel[i].rightStart);
        pO[l] = 'V'; printSetValue(dest,pO,strip.panel[i].vertical);
        pO[l] = 'S'; printSetCheckbox(dest,pO,strip.panel[i].serpentine);
        pO[l] = 'X'; printSetValue(dest,pO,strip.panel[i].xOffset);
        pO[l] = 'Y'; printSetValue(dest,pO,strip.panel[i].yOffset);
        pO[l] = 'W'; printSetValue(dest,pO,strip.panel[i].width);
        pO[l] = 'H'; printSetValue(dest,pO,strip.panel[i].height);
      }
    }
    #else
    dest.print(F("gId(\"somp\").remove(1);")); // remove 2D option from dropdown
    #endif
  }
}
