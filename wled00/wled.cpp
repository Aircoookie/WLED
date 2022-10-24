#define WLED_DEFINE_GLOBAL_VARS //only in one source file, wled.cpp!
#include "wled.h"
#include "wled_ethernet.h"
#include <Arduino.h>

#if defined(ARDUINO_ARCH_ESP32) && defined(WLED_DISABLE_BROWNOUT_DET)
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#endif

/*
 * Main WLED class implementation. Mostly initialization and connection logic
 */

WLED::WLED()
{
}

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
  applyBri();
  DEBUG_PRINTLN(F("WLED RESET"));
  ESP.restart();
}

void WLED::loop()
{
  #ifdef WLED_DEBUG
  static unsigned long maxUsermodMillis = 0;
  static uint16_t avgUsermodMillis = 0;
  static unsigned long maxStripMillis = 0;
  static uint16_t avgStripMillis = 0;
  #endif

  handleTime();
  handleIR();        // 2nd call to function needed for ESP32 to return valid results -- should be good for ESP8266, too
  handleConnection();
  handleSerial();
  handleNotifications();
  handleTransitions();
#ifdef WLED_ENABLE_DMX
  handleDMX();
#endif
  userLoop();

  #ifdef WLED_DEBUG
  unsigned long usermodMillis = millis();
  #endif
  usermods.loop();
  #ifdef WLED_DEBUG
  usermodMillis = millis() - usermodMillis;
  avgUsermodMillis += usermodMillis;
  if (usermodMillis > maxUsermodMillis) maxUsermodMillis = usermodMillis;
  #endif

  yield();
  handleIO();
  handleIR();
  #ifndef WLED_DISABLE_ALEXA
  handleAlexa();
  #endif

  yield();

  if (doSerializeConfig) serializeConfig();

  if (doReboot && !doInitBusses) // if busses have to be inited & saved, wait until next iteration
    reset();

  if (doCloseFile) {
    closeFile();
    yield();
  }

  if (!realtimeMode || realtimeOverride || (realtimeMode && useMainSegmentOnly))  // block stuff if WARLS/Adalight is enabled
  {
    if (apActive) dnsServer.processNextRequest();
    #ifndef WLED_DISABLE_OTA
    if (WLED_CONNECTED && aOtaEnabled && !otaLock && correctPIN) ArduinoOTA.handle();
    #endif
    handleNightlight();
    handlePlaylist();
    yield();

    #ifndef WLED_DISABLE_HUESYNC
    handleHue();
    yield();
    #endif

    #ifndef WLED_DISABLE_BLYNK
    handleBlynk();
    yield();
    #endif

    handlePresets();
    yield();

    #ifdef WLED_DEBUG
    unsigned long stripMillis = millis();
    #endif
    if (!offMode || strip.isOffRefreshRequired())
      strip.service();
    #ifdef ESP8266
    else if (!noWifiSleep)
      delay(1); //required to make sure ESP enters modem sleep (see #1184)
    #endif
    #ifdef WLED_DEBUG
    stripMillis = millis() - stripMillis;
    if (stripMillis > 50) DEBUG_PRINTLN("Slow strip.");
    avgStripMillis += stripMillis;
    if (stripMillis > maxStripMillis) maxStripMillis = stripMillis;
    #endif
  }

  yield();
#ifdef ESP8266
  MDNS.update();
#endif

  //millis() rolls over every 50 days
  if (lastMqttReconnectAttempt > millis()) {
    rolloverMillis++;
    lastMqttReconnectAttempt = 0;
    ntpLastSyncTime = 0;
    strip.restartRuntime();
  }
  if (millis() - lastMqttReconnectAttempt > 30000) {
    lastMqttReconnectAttempt = millis();
    initMqtt();
    yield();
    // refresh WLED nodes list
    refreshNodeList();
    if (nodeBroadcastEnabled) sendSysInfoUDP();
    yield();
  }

  // 15min PIN time-out
  if (strlen(settingsPIN)>0 && millis() - lastEditTime > 900000) {
    correctPIN = false;
    createEditHandler(false);
  }

  //LED settings have been saved, re-init busses
  //This code block causes severe FPS drop on ESP32 with the original "if (busConfigs[0] != nullptr)" conditional. Investigate! 
  if (doInitBusses) {
    doInitBusses = false;
    DEBUG_PRINTLN(F("Re-init busses."));
    bool aligned = strip.checkSegmentAlignment(); //see if old segments match old bus(ses)
    busses.removeAll();
    uint32_t mem = 0;
    for (uint8_t i = 0; i < WLED_MAX_BUSSES; i++) {
      if (busConfigs[i] == nullptr) break;
      mem += BusManager::memUsage(*busConfigs[i]);
      if (mem <= MAX_LED_MEMORY) {
        busses.add(*busConfigs[i]);
      }
      delete busConfigs[i]; busConfigs[i] = nullptr;
    }
    strip.finalizeInit();
    loadLedmap = 0;
    if (aligned) strip.makeAutoSegments();
    else strip.fixInvalidSegments();
    yield();
    serializeConfig();
  }
  if (loadLedmap >= 0) {
    strip.deserializeMap(loadLedmap);
    loadLedmap = -1;
  }

  yield();
  handleWs();
  handleStatusLED();

// DEBUG serial logging (every 30s)
#ifdef WLED_DEBUG
  if (millis() - debugTime > 29999) {
    DEBUG_PRINTLN(F("---DEBUG INFO---"));
    DEBUG_PRINT(F("Runtime: "));       DEBUG_PRINTLN(millis());
    DEBUG_PRINT(F("Unix time: "));     toki.printTime(toki.getTime());
    DEBUG_PRINT(F("Free heap: "));     DEBUG_PRINTLN(ESP.getFreeHeap());
    #if defined(ARDUINO_ARCH_ESP32) && defined(WLED_USE_PSRAM)
    if (psramFound()) {
      DEBUG_PRINT(F("Total PSRAM: "));    DEBUG_PRINT(ESP.getPsramSize()/1024); DEBUG_PRINTLN("kB");
      DEBUG_PRINT(F("Free PSRAM: "));     DEBUG_PRINT(ESP.getFreePsram()/1024); DEBUG_PRINTLN("kB");
    } else
      DEBUG_PRINTLN(F("No PSRAM"));
    #endif
    DEBUG_PRINT(F("Wifi state: "));      DEBUG_PRINTLN(WiFi.status());

    if (WiFi.status() != lastWifiState) {
      wifiStateChangedTime = millis();
    }
    lastWifiState = WiFi.status();
    DEBUG_PRINT(F("State time: "));      DEBUG_PRINTLN(wifiStateChangedTime);
    DEBUG_PRINT(F("NTP last sync: "));   DEBUG_PRINTLN(ntpLastSyncTime);
    DEBUG_PRINT(F("Client IP: "));       DEBUG_PRINTLN(Network.localIP());
    if (loops > 0) { // avoid division by zero
      DEBUG_PRINT(F("Loops/sec: "));       DEBUG_PRINTLN(loops / 30);
      DEBUG_PRINT(F("UM time[ms]: "));     DEBUG_PRINT(avgUsermodMillis/loops); DEBUG_PRINT("/");DEBUG_PRINTLN(maxUsermodMillis);
      DEBUG_PRINT(F("Strip time[ms]: "));  DEBUG_PRINT(avgStripMillis/loops); DEBUG_PRINT("/"); DEBUG_PRINTLN(maxStripMillis);
    }
    strip.printSize();
    loops = 0;
    maxUsermodMillis = 0;
    maxStripMillis = 0;
    avgUsermodMillis = 0;
    avgStripMillis = 0;
    debugTime = millis();
  }
  loops++;
#endif        // WLED_DEBUG
  toki.resetTick();

#if WLED_WATCHDOG_TIMEOUT > 0
  // we finished our mainloop, reset the watchdog timer
  if (!strip.isUpdating())
  #ifdef ARDUINO_ARCH_ESP32
    esp_task_wdt_reset();
  #else
    ESP.wdtFeed();
  #endif
#endif
}

void WLED::enableWatchdog() {
#if WLED_WATCHDOG_TIMEOUT > 0
#ifdef ARDUINO_ARCH_ESP32
  esp_err_t watchdog = esp_task_wdt_init(WLED_WATCHDOG_TIMEOUT, true);
  DEBUG_PRINT(F("Watchdog enabled: "));
  if (watchdog == ESP_OK) {
    DEBUG_PRINTLN(F("OK"));
  } else {
    DEBUG_PRINTLN(watchdog);
    return;
  }
  esp_task_wdt_add(NULL);
#else
  ESP.wdtEnable(WLED_WATCHDOG_TIMEOUT * 1000);
#endif
#endif
}

void WLED::disableWatchdog() {
#if WLED_WATCHDOG_TIMEOUT > 0
DEBUG_PRINTLN(F("Watchdog: disabled"));
#ifdef ARDUINO_ARCH_ESP32
  esp_task_wdt_delete(NULL);
#else
  ESP.wdtDisable();
#endif
#endif
}

void WLED::setup()
{
  #if defined(ARDUINO_ARCH_ESP32) && defined(WLED_DISABLE_BROWNOUT_DET)
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detection
  #endif

  Serial.begin(115200);
  Serial.setTimeout(50);
  #if defined(WLED_DEBUG) && (defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32C3))
  delay(2500);  // allow CDC USB serial to initialise
  #endif
  DEBUG_PRINTLN();
  DEBUG_PRINT(F("---WLED "));
  DEBUG_PRINT(versionString);
  DEBUG_PRINT(" ");
  DEBUG_PRINT(VERSION);
  DEBUG_PRINTLN(F(" INIT---"));
#ifdef ARDUINO_ARCH_ESP32
  DEBUG_PRINT(F("esp32 "));
  DEBUG_PRINTLN(ESP.getSdkVersion());
  #if defined(ESP_ARDUINO_VERSION)
    //DEBUG_PRINTF(F("arduino-esp32  0x%06x\n"), ESP_ARDUINO_VERSION);
    DEBUG_PRINTF("arduino-esp32 v%d.%d.%d\n", int(ESP_ARDUINO_VERSION_MAJOR), int(ESP_ARDUINO_VERSION_MINOR), int(ESP_ARDUINO_VERSION_PATCH));  // availeable since v2.0.0
  #else
    DEBUG_PRINTLN(F("arduino-esp32 v1.0.x\n"));  // we can't say in more detail.
  #endif

  DEBUG_PRINT(F("CPU:   ")); DEBUG_PRINT(ESP.getChipModel());
  DEBUG_PRINT(F(" rev.")); DEBUG_PRINT(ESP.getChipRevision());
  DEBUG_PRINT(F(", ")); DEBUG_PRINT(ESP.getChipCores()); DEBUG_PRINT(F(" core(s)"));
  DEBUG_PRINT(F(", ")); DEBUG_PRINT(ESP.getCpuFreqMHz()); DEBUG_PRINTLN(F("MHz."));
  DEBUG_PRINT(F("FLASH: ")); DEBUG_PRINT((ESP.getFlashChipSize()/1024)/1024);
  DEBUG_PRINT(F("MB, Mode ")); DEBUG_PRINT(ESP.getFlashChipMode());
  #ifdef WLED_DEBUG
  switch (ESP.getFlashChipMode()) {
    // missing: Octal modes
    case FM_QIO:  DEBUG_PRINT(F(" (QIO)")); break;
    case FM_QOUT: DEBUG_PRINT(F(" (QOUT)"));break;
    case FM_DIO:  DEBUG_PRINT(F(" (DIO)")); break;
    case FM_DOUT: DEBUG_PRINT(F(" (DOUT)"));break;
    default: break;
  }
  #endif
  DEBUG_PRINT(F(", speed ")); DEBUG_PRINT(ESP.getFlashChipSpeed()/1000000);DEBUG_PRINTLN(F("MHz."));

#else
  DEBUG_PRINT(F("esp8266 "));
  DEBUG_PRINTLN(ESP.getCoreVersion());
#endif
  DEBUG_PRINT(F("heap "));
  DEBUG_PRINTLN(ESP.getFreeHeap());

  #if defined(ARDUINO_ARCH_ESP32) && defined(WLED_USE_PSRAM)
  if (psramFound()) {
#if !defined(CONFIG_IDF_TARGET_ESP32C3) && !defined(CONFIG_IDF_TARGET_ESP32S2) && !defined(CONFIG_IDF_TARGET_ESP32S3)
    // GPIO16/GPIO17 reserved for SPI RAM
    managed_pin_type pins[2] = { {16, true}, {17, true} };
    pinManager.allocateMultiplePins(pins, 2, PinOwner::SPI_RAM);
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
    // S3: add GPIO 33-37 for "octal" PSRAM
    managed_pin_type pins[5] = { {33, true}, {34, true}, {35, true}, {36, true}, {37, true} };
    pinManager.allocateMultiplePins(pins, 5, PinOwner::SPI_RAM);
#endif
      DEBUG_PRINT(F("Total PSRAM: "));    DEBUG_PRINT(ESP.getPsramSize()/1024); DEBUG_PRINTLN("kB");
      DEBUG_PRINT(F("Free PSRAM : "));    DEBUG_PRINT(ESP.getFreePsram()/1024); DEBUG_PRINTLN("kB");
    } else
      DEBUG_PRINTLN(F("No PSRAM found."));
  #endif
  #if defined(ARDUINO_ARCH_ESP32) && defined(BOARD_HAS_PSRAM) && !defined(WLED_USE_PSRAM)
      DEBUG_PRINTLN(F("PSRAM not used."));
  #endif

  //DEBUG_PRINT(F("LEDs inited. heap usage ~"));
  //DEBUG_PRINTLN(heapPreAlloc - ESP.getFreeHeap());

#ifdef WLED_DEBUG
  pinManager.allocatePin(hardwareTX, true, PinOwner::DebugOut); // TX (GPIO1 on ESP32) reserved for debug output
#endif
#ifdef WLED_ENABLE_DMX //reserve GPIO2 as hardcoded DMX pin
  pinManager.allocatePin(2, true, PinOwner::DMX);
#endif

  DEBUG_PRINTLN(F("Registering usermods ..."));
  registerUsermods();

  for (uint8_t i=1; i<WLED_MAX_BUTTONS; i++) btnPin[i] = -1;

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
  } 
#ifdef WLED_ADD_EEPROM_SUPPORT
  else deEEP();
#endif
  updateFSInfo();

  strcpy_P(apSSID, PSTR("WLED-AP"));  // otherwise it is empty on first boot until config is saved
  DEBUG_PRINTLN(F("Reading config"));
  deserializeConfigFromFS();

#if defined(STATUSLED) && STATUSLED>=0
  if (!pinManager.isPinAllocated(STATUSLED)) {
    // NOTE: Special case: The status LED should *NOT* be allocated.
    //       See comments in handleStatusLed().
    pinMode(STATUSLED, OUTPUT);
  }
#endif

  DEBUG_PRINTLN(F("Initializing strip"));
  beginStrip();

  DEBUG_PRINTLN(F("Usermods setup"));
  userSetup();
  usermods.setup();

  if (strcmp(clientSSID, DEFAULT_CLIENT_SSID) == 0)
    showWelcomePage = true;
  WiFi.persistent(false);
  #ifdef WLED_USE_ETHERNET
  WiFi.onEvent(WiFiEvent);
  #endif

  #ifdef WLED_ENABLE_ADALIGHT
  //Serial RX (Adalight, Improv, Serial JSON) only possible if GPIO3 unused
  //Serial TX (Debug, Improv, Serial JSON) only possible if GPIO1 unused
  if (!pinManager.isPinAllocated(hardwareRX) && !pinManager.isPinAllocated(hardwareTX)) {
    Serial.println(F("Ada"));
  }
  #endif

  // generate module IDs
  escapedMac = WiFi.macAddress();
  escapedMac.replace(":", "");
  escapedMac.toLowerCase();
  // fill in unique mdns default
  if (strcmp(cmDNS, "x") == 0) sprintf_P(cmDNS, PSTR("wled-%*s"), 6, escapedMac.c_str() + 6);
  if (mqttDeviceTopic[0] == 0) sprintf_P(mqttDeviceTopic, PSTR("wled/%*s"), 6, escapedMac.c_str() + 6);
  if (mqttClientID[0] == 0)    sprintf_P(mqttClientID, PSTR("WLED-%*s"), 6, escapedMac.c_str() + 6);

#ifdef WLED_ENABLE_ADALIGHT
  if (Serial.available() > 0 && Serial.peek() == 'I') handleImprovPacket();
#endif

  strip.service(); // why?

#ifndef WLED_DISABLE_OTA
  if (aOtaEnabled) {
    ArduinoOTA.onStart([]() {
#ifdef ESP8266
      wifi_set_sleep_type(NONE_SLEEP_T);
#endif
      WLED::instance().disableWatchdog();
      DEBUG_PRINTLN(F("Start ArduinoOTA"));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      // reenable watchdog on failed update
      WLED::instance().enableWatchdog();
    });
    if (strlen(cmDNS) > 0)
      ArduinoOTA.setHostname(cmDNS);
  }
#endif
#ifdef WLED_ENABLE_DMX
  initDMX();
#endif

#ifdef WLED_ENABLE_ADALIGHT
  if (Serial.available() > 0 && Serial.peek() == 'I') handleImprovPacket();
#endif

  // HTTP server page init
  initServer();

  enableWatchdog();

  #if defined(ARDUINO_ARCH_ESP32) && defined(WLED_DISABLE_BROWNOUT_DET)
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 1); //enable brownout detector
  #endif
}

void WLED::beginStrip()
{
  // Initialize NeoPixel Strip and button
  strip.finalizeInit(); // busses created during deserializeConfig()
  strip.loadCustomPalettes();
  strip.deserializeMap();
  strip.makeAutoSegments();
  strip.setBrightness(0);
  strip.setShowCallback(handleOverlayDraw);

  if (turnOnAtBoot) {
    if (briS > 0) bri = briS;
    else if (bri == 0) bri = 128;
  } else {
    briLast = briS; bri = 0;
  }
  if (bootPreset > 0) {
    applyPreset(bootPreset, CALL_MODE_INIT);
  }
  colorUpdated(CALL_MODE_INIT);

  // init relay pin
  if (rlyPin>=0)
    digitalWrite(rlyPin, (rlyMde ? bri : !bri));
}

void WLED::initAP(bool resetAP)
{
  if (apBehavior == AP_BEHAVIOR_BUTTON_ONLY && !resetAP)
    return;

  if (resetAP) {
    strcpy_P(apSSID, PSTR("WLED-AP"));
    strcpy_P(apPass, PSTR(DEFAULT_AP_PASS));
  }
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
    ddp.begin(false, DDP_DEFAULT_PORT);

    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, "*", WiFi.softAPIP());
  }
  apActive = true;
}

bool WLED::initEthernet()
{
#if defined(ARDUINO_ARCH_ESP32) && defined(WLED_USE_ETHERNET)

  static bool successfullyConfiguredEthernet = false;

  if (successfullyConfiguredEthernet) {
    // DEBUG_PRINTLN(F("initE: ETH already successfully configured, ignoring"));
    return false;
  }
  if (ethernetType == WLED_ETH_NONE) {
    return false;
  }
  if (ethernetType >= WLED_NUM_ETH_TYPES) {
    DEBUG_PRINT(F("initE: Ignoring attempt for invalid ethernetType ")); DEBUG_PRINTLN(ethernetType);
    return false;
  }

  DEBUG_PRINT(F("initE: Attempting ETH config: ")); DEBUG_PRINTLN(ethernetType);

  // Ethernet initialization should only succeed once -- else reboot required
  ethernet_settings es = ethernetBoards[ethernetType];
  managed_pin_type pinsToAllocate[10] = {
    // first six pins are non-configurable
    esp32_nonconfigurable_ethernet_pins[0],
    esp32_nonconfigurable_ethernet_pins[1],
    esp32_nonconfigurable_ethernet_pins[2],
    esp32_nonconfigurable_ethernet_pins[3],
    esp32_nonconfigurable_ethernet_pins[4],
    esp32_nonconfigurable_ethernet_pins[5],
    { (int8_t)es.eth_mdc,   true },  // [6] = MDC  is output and mandatory
    { (int8_t)es.eth_mdio,  true },  // [7] = MDIO is bidirectional and mandatory
    { (int8_t)es.eth_power, true },  // [8] = optional pin, not all boards use
    { ((int8_t)0xFE),       false }, // [9] = replaced with eth_clk_mode, mandatory
  };
  // update the clock pin....
  if (es.eth_clk_mode == ETH_CLOCK_GPIO0_IN) {
    pinsToAllocate[9].pin = 0;
    pinsToAllocate[9].isOutput = false;
  } else if (es.eth_clk_mode == ETH_CLOCK_GPIO0_OUT) {
    pinsToAllocate[9].pin = 0;
    pinsToAllocate[9].isOutput = true;
  } else if (es.eth_clk_mode == ETH_CLOCK_GPIO16_OUT) {
    pinsToAllocate[9].pin = 16;
    pinsToAllocate[9].isOutput = true;
  } else if (es.eth_clk_mode == ETH_CLOCK_GPIO17_OUT) {
    pinsToAllocate[9].pin = 17;
    pinsToAllocate[9].isOutput = true;
  } else {
    DEBUG_PRINT(F("initE: Failing due to invalid eth_clk_mode ("));
    DEBUG_PRINT(es.eth_clk_mode);
    DEBUG_PRINTLN(F(")"));
    return false;
  }

  if (!pinManager.allocateMultiplePins(pinsToAllocate, 10, PinOwner::Ethernet)) {
    DEBUG_PRINTLN(F("initE: Failed to allocate ethernet pins"));
    return false;
  }

  if (!ETH.begin(
                (uint8_t) es.eth_address, 
                (int)     es.eth_power, 
                (int)     es.eth_mdc, 
                (int)     es.eth_mdio, 
                (eth_phy_type_t)   es.eth_type,
                (eth_clock_mode_t) es.eth_clk_mode
                )) {
    DEBUG_PRINTLN(F("initC: ETH.begin() failed"));
    // de-allocate the allocated pins
    for (managed_pin_type mpt : pinsToAllocate) {
      pinManager.deallocatePin(mpt.pin, PinOwner::Ethernet);
    }
    return false;
  }

  successfullyConfiguredEthernet = true;
  DEBUG_PRINTLN(F("initC: *** Ethernet successfully configured! ***"));
  return true;
#else
  return false; // Ethernet not enabled for build
#endif

}

void WLED::initConnection()
{
  #ifdef WLED_ENABLE_WEBSOCKETS
  ws.onEvent(wsEvent);
  #endif


  WiFi.disconnect(true);        // close old connections
#ifdef ESP8266
  WiFi.setPhyMode(WIFI_PHY_MODE_11N);
#endif

  if (staticIP[0] != 0 && staticGateway[0] != 0) {
    WiFi.config(staticIP, staticGateway, staticSubnet, IPAddress(1, 1, 1, 1));
  } else {
    WiFi.config(IPAddress((uint32_t)0), IPAddress((uint32_t)0), IPAddress((uint32_t)0));
  }

  lastReconnectAttempt = millis();

  if (!WLED_WIFI_CONFIGURED) {
    DEBUG_PRINTLN(F("No connection configured."));
    if (!apActive) initAP();        // instantly go to ap mode
    return;
  } else if (!apActive) {
    if (apBehavior == AP_BEHAVIOR_ALWAYS) {
      DEBUG_PRINTLN(F("Access point ALWAYS enabled."));
      initAP();
    } else {
      DEBUG_PRINTLN(F("Access point disabled (init)."));
      WiFi.softAPdisconnect(true);
      WiFi.mode(WIFI_STA);
    }
  }
  showWelcomePage = false;

  DEBUG_PRINT(F("Connecting to "));
  DEBUG_PRINT(clientSSID);
  DEBUG_PRINTLN("...");

  // convert the "serverDescription" into a valid DNS hostname (alphanumeric)
  char hostname[25];
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

#ifndef WLED_DISABLE_HUESYNC
  IPAddress ipAddress = Network.localIP();
  if (hueIP[0] == 0) {
    hueIP[0] = ipAddress[0];
    hueIP[1] = ipAddress[1];
    hueIP[2] = ipAddress[2];
  }
#endif

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
    // "end" must be called before "begin" is called a 2nd time
    // see https://github.com/esp8266/Arduino/issues/7213
    MDNS.end();
    MDNS.begin(cmDNS);

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

#ifndef WLED_DISABLE_BLYNK
  initBlynk(blynkApiKey, blynkHost, blynkPort);
#endif
  e131.begin(e131Multicast, e131Port, e131Universe, E131_MAX_UNIVERSE_COUNT);
  ddp.begin(false, DDP_DEFAULT_PORT);
  reconnectHue();
  initMqtt();
  interfacesInited = true;
  wasConnected = true;
}

void WLED::handleConnection()
{
  static byte stacO = 0;
  static uint32_t lastHeap = UINT32_MAX;
  static unsigned long heapTime = 0;
  unsigned long now = millis();

  if (now < 2000 && (!WLED_WIFI_CONFIGURED || apBehavior == AP_BEHAVIOR_ALWAYS))
    return;

  if (lastReconnectAttempt == 0) {
    DEBUG_PRINTLN(F("lastReconnectAttempt == 0"));
    initConnection();
    return;
  }

  // reconnect WiFi to clear stale allocations if heap gets too low
  if (now - heapTime > 5000) {
    uint32_t heap = ESP.getFreeHeap();
    if (heap < MIN_HEAP_SIZE && lastHeap < MIN_HEAP_SIZE) {
      DEBUG_PRINT(F("Heap too low! "));
      DEBUG_PRINTLN(heap);
      forceReconnect = true;
      strip.purgeSegments(true); // remove all but one segments from memory
    }
    lastHeap = heap;
    heapTime = now;
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
          initConnection();         // restart search
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
    //send improv failed 6 seconds after second init attempt (24 sec. after provisioning)
    if (improvActive > 2 && now - lastReconnectAttempt > 6000) {
      sendImprovStateResponse(0x03, true);
      improvActive = 2;
    }
    if (now - lastReconnectAttempt > ((stac) ? 300000 : 18000) && WLED_WIFI_CONFIGURED) {
      if (improvActive == 2) improvActive = 3;
      DEBUG_PRINTLN(F("Last reconnect too old."));
      initConnection();
    }
    if (!apActive && now - lastReconnectAttempt > 12000 && (!wasConnected || apBehavior == AP_BEHAVIOR_NO_CONN)) {
      DEBUG_PRINTLN(F("Not connected AP."));
      initAP();
    }
  } else if (!interfacesInited) { //newly connected
    DEBUG_PRINTLN("");
    DEBUG_PRINT(F("Connected! IP address: "));
    DEBUG_PRINTLN(Network.localIP());
    if (improvActive) {
      if (improvError == 3) sendImprovStateResponse(0x00, true);
      sendImprovStateResponse(0x04);
      if (improvActive > 1) sendImprovRPCResponse(0x01);
    }
    initInterfaces();
    userConnected();
    usermods.connected();

    // shut down AP
    if (apBehavior != AP_BEHAVIOR_ALWAYS && apActive) {
      dnsServer.stop();
      WiFi.softAPdisconnect(true);
      apActive = false;
      DEBUG_PRINTLN(F("Access point disabled (handle)."));
    }
  }
}

// If status LED pin is allocated for other uses, does nothing
// else blink at 1Hz when WLED_CONNECTED is false (no WiFi, ?? no Ethernet ??)
// else blink at 2Hz when MQTT is enabled but not connected
// else turn the status LED off
void WLED::handleStatusLED()
{
  #if defined(STATUSLED)
  uint32_t c = 0;

  #if STATUSLED>=0
  if (pinManager.isPinAllocated(STATUSLED)) {
    return; //lower priority if something else uses the same pin
  }
  #endif

  if (WLED_CONNECTED) {
    c = RGBW32(0,255,0,0);
    ledStatusType = 2;
  } else if (WLED_MQTT_CONNECTED) {
    c = RGBW32(0,128,0,0);
    ledStatusType = 4;
  } else if (apActive) {
    c = RGBW32(0,0,255,0);
    ledStatusType = 1;
  }
  if (ledStatusType) {
    if (millis() - ledStatusLastMillis >= (1000/ledStatusType)) {
      ledStatusLastMillis = millis();
      ledStatusState = !ledStatusState;
      #if STATUSLED>=0
      digitalWrite(STATUSLED, ledStatusState);
      #else
      busses.setStatusPixel(ledStatusState ? c : 0);
      #endif
    }
  } else {
    #if STATUSLED>=0
      #ifdef STATUSLEDINVERTED
      digitalWrite(STATUSLED, HIGH);
      #else
      digitalWrite(STATUSLED, LOW);
      #endif
    #else
      busses.setStatusPixel(0);
    #endif
  }
  #endif
}
