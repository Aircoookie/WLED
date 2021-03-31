#define WLED_DEFINE_GLOBAL_VARS //only in one source file, wled.cpp!
#include "wled.h"
#include <Arduino.h>

/*
 * Main WLED class implementation. Mostly initialization and connection logic
 */

WLED::WLED()
{
}

#ifdef WLED_USE_ETHERNET
// settings for various ethernet boards
typedef struct EthernetSettings {
  uint8_t        eth_address;
  int            eth_power;
  int            eth_mdc;
  int            eth_mdio;
  eth_phy_type_t eth_type;
  eth_clock_mode_t eth_clk_mode;
} ethernet_settings;

ethernet_settings ethernetBoards[] = {
  // None
  {
  },
  
  // WT32-EHT01
  // Please note, from my testing only these pins work for LED outputs:
  //   IO2, IO4, IO12, IO14, IO15
  // These pins do not appear to work from my testing:
  //   IO35, IO36, IO39
  {
    1,                 // eth_address, 
    16,                // eth_power, 
    23,                // eth_mdc, 
    18,                // eth_mdio, 
    ETH_PHY_LAN8720,   // eth_type,
    ETH_CLOCK_GPIO0_IN // eth_clk_mode
  },

  // ESP32-POE
  {
     0,                  // eth_address, 
    12,                  // eth_power, 
    23,                  // eth_mdc, 
    18,                  // eth_mdio, 
    ETH_PHY_LAN8720,     // eth_type,
    ETH_CLOCK_GPIO17_OUT // eth_clk_mode
  },

   // WESP32
  {
    0,			              // eth_address,
    -1,			              // eth_power,
    16,			              // eth_mdc,
    17,			              // eth_mdio,
    ETH_PHY_LAN8720,      // eth_type,
    ETH_CLOCK_GPIO0_IN	  // eth_clk_mode
  },

  // QuinLed-ESP32-Ethernet
  {
    0,			              // eth_address,
    5,			              // eth_power,
    23,			              // eth_mdc,
    18,			              // eth_mdio,
    ETH_PHY_LAN8720,      // eth_type,
    ETH_CLOCK_GPIO17_OUT	// eth_clk_mode
  }
};

#endif

// turns all LEDs off and restarts ESP
void WLED::reset()
{
  briT = 0;
  #ifdef WLED_ENABLE_WEBSOCKETS
  ws.closeAll(1012);
  #endif
  long dly = millis();
  while (millis() - dly < 450) {
    yield();        // enough time to send response to client
  }
  setAllLeds();
  DEBUG_PRINTLN("MODULE RESET");
  ESP.restart();
}

bool oappendi(int i)
{
  char s[11];
  sprintf(s, "%d", i);
  return oappend(s);
}

bool oappend(const char* txt)
{
  uint16_t len = strlen(txt);
  if (olen + len >= OMAX)
    return false;        // buffer full
  strcpy(obuf + olen, txt);
  olen += len;
  return true;
}

void prepareHostname(char* hostname)
{
  const char *pC = serverDescription;
  uint8_t pos = 5;

  while (*pC && pos < 24) { // while !null and not over length
    if (isalnum(*pC)) {     // if the current char is alpha-numeric append it to the hostname
      hostname[pos] = *pC;
      pos++;
    } else if (*pC == ' ' || *pC == '_' || *pC == '-' || *pC == '+' || *pC == '!' || *pC == '?' || *pC == '*') {
      hostname[pos] = '-';
      pos++;
    }
      // else do nothing - no leading hyphens and do not include hyphens for all other characters.
      pC++;
    }
    // if the hostname is left blank, use the mac address/default mdns name
    if (pos < 6) {
      sprintf(hostname + 5, "%*s", 6, escapedMac.c_str() + 6);
    } else { //last character must not be hyphen
      while (pos > 0 && hostname[pos -1] == '-') {
        hostname[pos -1] = 0;
        pos--;
      }
    }
}

//handle Ethernet connection event
void WiFiEvent(WiFiEvent_t event)
{
  #ifdef WLED_USE_ETHERNET
  char hostname[25] = "wled-";
  #endif
  
  switch (event) {
#if defined(ARDUINO_ARCH_ESP32) && defined(WLED_USE_ETHERNET)
    case SYSTEM_EVENT_ETH_START:
      DEBUG_PRINT("ETH Started");
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      DEBUG_PRINT("ETH Connected");
      if (!apActive) {
        WiFi.disconnect(true);
      }
      if (staticIP != (uint32_t)0x00000000 && staticGateway != (uint32_t)0x00000000) {
        ETH.config(staticIP, staticGateway, staticSubnet, IPAddress(8, 8, 8, 8));
      } else {
        ETH.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
      }
      // convert the "serverDescription" into a valid DNS hostname (alphanumeric)
      prepareHostname(hostname);
      ETH.setHostname(hostname);
      showWelcomePage = false;
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      DEBUG_PRINT("ETH Disconnected");
      forceReconnect = true;
      break;
#endif
    default:
      break;
  }
}

void WLED::loop()
{
  handleIR();        // 2nd call to function needed for ESP32 to return valid results -- should be good for ESP8266, too
  handleConnection();
  handleSerial();
  handleNotifications();
  handleTransitions();
#ifdef WLED_ENABLE_DMX
  handleDMX();
#endif
  userLoop();
  usermods.loop();

  yield();
  handleIO();
  handleIR();
  handleNetworkTime();
  handleAlexa();

  handleOverlays();
  yield();

  if (doReboot)
    reset();
  if (doCloseFile) {
    closeFile();
    yield();
  }

  if (!realtimeMode || realtimeOverride)  // block stuff if WARLS/Adalight is enabled
  {
    if (apActive)
      dnsServer.processNextRequest();
#ifndef WLED_DISABLE_OTA
    if (WLED_CONNECTED && aOtaEnabled)
      ArduinoOTA.handle();
#endif
    handleNightlight();
    handlePlaylist();
    yield();

    handleHue();
    handleBlynk();

    yield();

    if (!offMode)
      strip.service();
#ifdef ESP8266
    else if (!noWifiSleep)
      delay(1); //required to make sure ESP enters modem sleep (see #1184)
#endif
  }
  yield();
#ifdef ESP8266
  MDNS.update();
#endif
  if (millis() - lastMqttReconnectAttempt > 30000) {
    if (lastMqttReconnectAttempt > millis()) rolloverMillis++; //millis() rolls over every 50 days
    initMqtt();
    refreshNodeList();
    if (nodeBroadcastEnabled) sendSysInfoUDP();
    yield();
  }

  //LED settings have been saved, re-init busses
  //This code block causes severe FPS drop on ESP32 with the original "if (busConfigs[0] != nullptr)" conditional. Investigate! 
  if (doInitBusses) {
    doInitBusses = false;
    busses.removeAll();
    uint32_t mem = 0;
    strip.isRgbw = false;
    for (uint8_t i = 0; i < WLED_MAX_BUSSES; i++) {
      if (busConfigs[i] == nullptr) break;
      mem += busses.memUsage(*busConfigs[i]);
      if (mem <= MAX_LED_MEMORY) busses.add(*busConfigs[i]);
      //if (BusManager::isRgbw(busConfigs[i]->type)) strip.isRgbw = true;
      strip.isRgbw = (strip.isRgbw || BusManager::isRgbw(busConfigs[i]->type));
      delete busConfigs[i]; busConfigs[i] = nullptr;
    }
    strip.finalizeInit(ledCount, skipFirstLed);
    yield();
    serializeConfig();
  }
  
  yield();
  handleWs();
  handleStatusLED();

// DEBUG serial logging
#ifdef WLED_DEBUG
  if (millis() - debugTime > 9999) {
    DEBUG_PRINTLN("---DEBUG INFO---");
    DEBUG_PRINT("Runtime: ");       DEBUG_PRINTLN(millis());
    DEBUG_PRINT("Unix time: ");     DEBUG_PRINTLN(now());
    DEBUG_PRINT("Free heap: ");     DEBUG_PRINTLN(ESP.getFreeHeap());
    DEBUG_PRINT("Wifi state: ");    DEBUG_PRINTLN(WiFi.status());

    if (WiFi.status() != lastWifiState) {
      wifiStateChangedTime = millis();
    }
    lastWifiState = WiFi.status();
    DEBUG_PRINT("State time: ");    DEBUG_PRINTLN(wifiStateChangedTime);
    DEBUG_PRINT("NTP last sync: "); DEBUG_PRINTLN(ntpLastSyncTime);
    DEBUG_PRINT("Client IP: ");     DEBUG_PRINTLN(Network.localIP());
    DEBUG_PRINT("Loops/sec: ");     DEBUG_PRINTLN(loops / 10);
    loops = 0;
    debugTime = millis();
  }
  loops++;
#endif        // WLED_DEBUG
}

void WLED::setup()
{
  Serial.begin(115200);
  Serial.setTimeout(50);
  DEBUG_PRINTLN();
  DEBUG_PRINT("---WLED ");
  DEBUG_PRINT(versionString);
  DEBUG_PRINT(" ");
  DEBUG_PRINT(VERSION);
  DEBUG_PRINTLN(" INIT---");
#ifdef ARDUINO_ARCH_ESP32
  DEBUG_PRINT("esp32 ");
  DEBUG_PRINTLN(ESP.getSdkVersion());
#else
  DEBUG_PRINT("esp8266 ");
  DEBUG_PRINTLN(ESP.getCoreVersion());
#endif
  DEBUG_PRINT("heap ");
  DEBUG_PRINTLN(ESP.getFreeHeap());
  registerUsermods();

  //DEBUG_PRINT(F("LEDs inited. heap usage ~"));
  //DEBUG_PRINTLN(heapPreAlloc - ESP.getFreeHeap());

#ifdef WLED_USE_DMX //reserve GPIO2 as hardcoded DMX pin
  pinManager.allocatePin(2);
#endif

  bool fsinit = false;
  DEBUGFS_PRINTLN(F("Mount FS"));
#ifdef ARDUINO_ARCH_ESP32
  fsinit = WLED_FS.begin(true);
#else
  fsinit = WLED_FS.begin();
#endif
  if (!fsinit) {
    DEBUGFS_PRINTLN(F("FS failed!"));
    errorFlag = ERR_FS_BEGIN;
  } else deEEP();
  updateFSInfo();
  deserializeConfig();

#if STATUSLED
  bool lStatusLed = false;
  for (uint8_t i=0; i<strip.numStrips; i++) {
    if (strip.getStripPin(i)==STATUSLED) {
      lStatusLed = true;
      break;
    }
  }
  if (!lStatusLed)
    pinMode(STATUSLED, OUTPUT);
#endif

  //DEBUG_PRINTLN(F("Load EEPROM"));
  //loadSettingsFromEEPROM();
  beginStrip();
  userSetup();
  usermods.setup();
  if (strcmp(clientSSID, DEFAULT_CLIENT_SSID) == 0)
    showWelcomePage = true;
  WiFi.persistent(false);
  #ifdef WLED_USE_ETHERNET
  WiFi.onEvent(WiFiEvent);
  #endif

  Serial.println(F("Ada"));

  // generate module IDs
  escapedMac = WiFi.macAddress();
  escapedMac.replace(":", "");
  escapedMac.toLowerCase();
  if (strcmp(cmDNS, "x") == 0)        // fill in unique mdns default
  {
    strcpy_P(cmDNS, PSTR("wled-"));
    sprintf(cmDNS + 5, "%*s", 6, escapedMac.c_str() + 6);
  }
  if (mqttDeviceTopic[0] == 0) {
    strcpy_P(mqttDeviceTopic, PSTR("wled/"));
    sprintf(mqttDeviceTopic + 5, "%*s", 6, escapedMac.c_str() + 6);
  }
  if (mqttClientID[0] == 0) {
    strcpy_P(mqttClientID, PSTR("WLED-"));
    sprintf(mqttClientID + 5, "%*s", 6, escapedMac.c_str() + 6);
  }

  strip.service();

#ifndef WLED_DISABLE_OTA
  if (aOtaEnabled) {
    ArduinoOTA.onStart([]() {
#ifdef ESP8266
      wifi_set_sleep_type(NONE_SLEEP_T);
#endif
      DEBUG_PRINTLN(F("Start ArduinoOTA"));
    });
    if (strlen(cmDNS) > 0)
      ArduinoOTA.setHostname(cmDNS);
  }
#endif
#ifdef WLED_ENABLE_DMX
  initDMX();
#endif
  // HTTP server page init
  initServer();
}

void WLED::beginStrip()
{
  // Initialize NeoPixel Strip and button

  if (ledCount > MAX_LEDS || ledCount == 0)
    ledCount = 30;

  strip.finalizeInit(ledCount, skipFirstLed);
  strip.setBrightness(0);
  strip.setShowCallback(handleOverlayDraw);

  if (bootPreset > 0) applyPreset(bootPreset);
  if (turnOnAtBoot) {
    if (briS > 0) bri = briS;
    else if (bri == 0) bri = 128;
  } else {
    briLast = briS; bri = 0;
  }
  colorUpdated(NOTIFIER_CALL_MODE_INIT);

  // init relay pin
  if (rlyPin>=0)
    digitalWrite(rlyPin, (rlyMde ? bri : !bri));

  // disable button if it is "pressed" unintentionally
  if (btnPin>=0 && isButtonPressed())
    buttonEnabled = false;
}

void WLED::initAP(bool resetAP)
{
  if (apBehavior == AP_BEHAVIOR_BUTTON_ONLY && !resetAP)
    return;

  if (!apSSID[0] || resetAP)
    strcpy_P(apSSID, PSTR("WLED-AP"));
  if (resetAP)
    strcpy_P(apPass, PSTR(DEFAULT_AP_PASS));
  DEBUG_PRINT(F("Opening access point "));
  DEBUG_PRINTLN(apSSID);
  WiFi.softAPConfig(IPAddress(4, 3, 2, 1), IPAddress(4, 3, 2, 1), IPAddress(255, 255, 255, 0));
  WiFi.softAP(apSSID, apPass, apChannel, apHide);

  if (!apActive) // start captive portal if AP active
  {
    DEBUG_PRINTLN(F("Init AP interfaces"));
    server.begin();
    if (udpPort > 0 && udpPort != ntpLocalPort) {
      udpConnected = notifierUdp.begin(udpPort);
    }
    if (udpRgbPort > 0 && udpRgbPort != ntpLocalPort && udpRgbPort != udpPort) {
      udpRgbConnected = rgbUdp.begin(udpRgbPort);
    }
    if (udpPort2 > 0 && udpPort2 != ntpLocalPort && udpPort2 != udpPort && udpPort2 != udpRgbPort) {
      udp2Connected = notifier2Udp.begin(udpPort2);
    }
    e131.begin(false, e131Port, e131Universe, E131_MAX_UNIVERSE_COUNT);

    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, "*", WiFi.softAPIP());
  }
  apActive = true;
}

void WLED::initConnection()
{
  #ifdef WLED_ENABLE_WEBSOCKETS
  ws.onEvent(wsEvent);
  #endif

#if defined(ARDUINO_ARCH_ESP32) && defined(WLED_USE_ETHERNET)
  // Only initialize ethernet board if not NONE
  if (ethernetType != WLED_ETH_NONE && ethernetType < WLED_NUM_ETH_TYPES) {
    ethernet_settings es = ethernetBoards[ethernetType];
    ETH.begin(
      (uint8_t) es.eth_address, 
      (int)     es.eth_power, 
      (int)     es.eth_mdc, 
      (int)     es.eth_mdio, 
      (eth_phy_type_t)   es.eth_type,
      (eth_clock_mode_t) es.eth_clk_mode
    );
  }
#endif

  WiFi.disconnect(true);        // close old connections
#ifdef ESP8266
  WiFi.setPhyMode(WIFI_PHY_MODE_11N);
#endif

  if (staticIP[0] != 0 && staticGateway[0] != 0) {
    WiFi.config(staticIP, staticGateway, staticSubnet, IPAddress(8, 8, 8, 8));
  } else {
    WiFi.config(0U, 0U, 0U);
  }

  lastReconnectAttempt = millis();

  if (!WLED_WIFI_CONFIGURED) {
    DEBUG_PRINT(F("No connection configured. "));
    if (!apActive)
      initAP();        // instantly go to ap mode
    return;
  } else if (!apActive) {
    if (apBehavior == AP_BEHAVIOR_ALWAYS) {
      initAP();
    } else {
      DEBUG_PRINTLN(F("Access point disabled."));
      WiFi.softAPdisconnect(true);
      WiFi.mode(WIFI_STA);
    }
  }
  showWelcomePage = false;

  DEBUG_PRINT(F("Connecting to "));
  DEBUG_PRINT(clientSSID);
  DEBUG_PRINTLN("...");

  // convert the "serverDescription" into a valid DNS hostname (alphanumeric)
  char hostname[25] = "wled-";
  prepareHostname(hostname);

#ifdef ESP8266
  WiFi.hostname(hostname);
#endif

  WiFi.begin(clientSSID, clientPass);

#ifdef ARDUINO_ARCH_ESP32
  WiFi.setSleep(!noWifiSleep);
  WiFi.setHostname(hostname);
#else
  wifi_set_sleep_type((noWifiSleep) ? NONE_SLEEP_T : MODEM_SLEEP_T);
#endif
}

void WLED::initInterfaces()
{
  DEBUG_PRINTLN(F("Init STA interfaces"));

  if (hueIP[0] == 0) {
    hueIP[0] = Network.localIP()[0];
    hueIP[1] = Network.localIP()[1];
    hueIP[2] = Network.localIP()[2];
  }

  // init Alexa hue emulation
  if (alexaEnabled)
    alexaInit();

#ifndef WLED_DISABLE_OTA
  if (aOtaEnabled)
    ArduinoOTA.begin();
#endif

  strip.service();
  // Set up mDNS responder:
  if (strlen(cmDNS) > 0) {
  #ifndef WLED_DISABLE_OTA
    if (!aOtaEnabled) //ArduinoOTA begins mDNS for us if enabled
      MDNS.begin(cmDNS);
  #else
    MDNS.begin(cmDNS);
  #endif

    DEBUG_PRINTLN(F("mDNS started"));
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("wled", "tcp", 80);
    MDNS.addServiceTxt("wled", "tcp", "mac", escapedMac.c_str());
  }
  server.begin();

  if (udpPort > 0 && udpPort != ntpLocalPort) {
    udpConnected = notifierUdp.begin(udpPort);
    if (udpConnected && udpRgbPort != udpPort)
      udpRgbConnected = rgbUdp.begin(udpRgbPort);
    if (udpConnected && udpPort2 != udpPort && udpPort2 != udpRgbPort)
      udp2Connected = notifier2Udp.begin(udpPort2);
  }
  if (ntpEnabled)
    ntpConnected = ntpUdp.begin(ntpLocalPort);

  initBlynk(blynkApiKey, blynkHost, blynkPort);
  e131.begin(e131Multicast, e131Port, e131Universe, E131_MAX_UNIVERSE_COUNT);
  reconnectHue();
  initMqtt();
  interfacesInited = true;
  wasConnected = true;
}

byte stacO = 0;
uint32_t lastHeap;
unsigned long heapTime = 0;

void WLED::handleConnection()
{
  if (millis() < 2000 && (!WLED_WIFI_CONFIGURED || apBehavior == AP_BEHAVIOR_ALWAYS))
    return;
  if (lastReconnectAttempt == 0)
    initConnection();

  // reconnect WiFi to clear stale allocations if heap gets too low
  if (millis() - heapTime > 5000) {
    uint32_t heap = ESP.getFreeHeap();
    if (heap < 9000 && lastHeap < 9000) {
      DEBUG_PRINT(F("Heap too low! "));
      DEBUG_PRINTLN(heap);
      forceReconnect = true;
    }
    lastHeap = heap;
    heapTime = millis();
  }

  byte stac = 0;
  if (apActive) {
#ifdef ESP8266
    stac = wifi_softap_get_station_num();
#else
    wifi_sta_list_t stationList;
    esp_wifi_ap_get_sta_list(&stationList);
    stac = stationList.num;
#endif
    if (stac != stacO) {
      stacO = stac;
      DEBUG_PRINT(F("Connected AP clients: "));
      DEBUG_PRINTLN(stac);
      if (!WLED_CONNECTED && WLED_WIFI_CONFIGURED) {        // trying to connect, but not connected
        if (stac)
          WiFi.disconnect();        // disable search so that AP can work
        else
          initConnection();        // restart search
      }
    }
  }
  if (forceReconnect) {
    DEBUG_PRINTLN(F("Forcing reconnect."));
    initConnection();
    interfacesInited = false;
    forceReconnect = false;
    wasConnected = false;
    return;
  }
  if (!Network.isConnected()) {
    if (interfacesInited) {
      DEBUG_PRINTLN(F("Disconnected!"));
      interfacesInited = false;
      initConnection();
    }
    if (millis() - lastReconnectAttempt > ((stac) ? 300000 : 20000) && WLED_WIFI_CONFIGURED)
      initConnection();
    if (!apActive && millis() - lastReconnectAttempt > 12000 && (!wasConnected || apBehavior == AP_BEHAVIOR_NO_CONN))
      initAP();
  } else if (!interfacesInited) {        // newly connected
    DEBUG_PRINTLN("");
    DEBUG_PRINT(F("Connected! IP address: "));
    DEBUG_PRINTLN(Network.localIP());
    initInterfaces();
    userConnected();
    usermods.connected();

    // shut down AP
    if (apBehavior != AP_BEHAVIOR_ALWAYS && apActive) {
      dnsServer.stop();
      WiFi.softAPdisconnect(true);
      apActive = false;
      DEBUG_PRINTLN(F("Access point disabled."));
    }
  }
}

void WLED::handleStatusLED()
{
  #if STATUSLED
  for (uint8_t s=0; s<strip.numStrips; s++) {
    if (strip.getStripPin(s)==STATUSLED) {
      return; // pin used for strip
    }
  }

  ledStatusType = WLED_CONNECTED ? 0 : 2;
  if (mqttEnabled && ledStatusType != 2) // Wi-Fi takes presendence over MQTT
    ledStatusType = WLED_MQTT_CONNECTED ? 0 : 4;
  if (ledStatusType) {
    if (millis() - ledStatusLastMillis >= (1000/ledStatusType)) {
      ledStatusLastMillis = millis();
      ledStatusState = ledStatusState ? 0 : 1;
      digitalWrite(STATUSLED, ledStatusState);
    }
  } else {
    #ifdef STATUSLEDINVERTED
      digitalWrite(STATUSLED, HIGH);
    #else
      digitalWrite(STATUSLED, LOW);
    #endif

  }
  #endif
}
