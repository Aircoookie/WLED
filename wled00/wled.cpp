#define WLED_DEFINE_GLOBAL_VARS //only in one source file, wled.cpp!
#include "wled.h"
#include "wled_ethernet.h"
#include <Arduino.h>

#if defined(ARDUINO_ARCH_ESP32) && defined(WLED_DISABLE_BROWNOUT_DET)
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#endif

#if defined(WLED_DEBUG) && defined(ARDUINO_ARCH_ESP32)
#include "../tools/ESP32-Chip_info.hpp"
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
  USER_PRINTLN(F("\nWLED RESTART\n"));
  USER_FLUSH();   // WLEDMM: wait until Serial has completed sending buffered data
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
  #ifndef WLED_DISABLE_INFRARED
  handleIR();        // 2nd call to function needed for ESP32 to return valid results -- should be good for ESP8266, too
  #endif
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
  #ifndef WLED_DISABLE_INFRARED
  handleIR();
  #endif
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
  if (millis() - lastMqttReconnectAttempt > 30000 || lastMqttReconnectAttempt == 0) { // lastMqttReconnectAttempt==0 forces immediate broadcast
    lastMqttReconnectAttempt = millis();
    #ifndef WLED_DISABLE_MQTT
    initMqtt();
    #endif
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
    for (uint8_t i = 0; i < WLED_MAX_BUSSES+WLED_MIN_VIRTUAL_BUSSES; i++) {
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
      //DEBUG_PRINT(F("Total PSRAM: "));    DEBUG_PRINT(ESP.getPsramSize()/1024); DEBUG_PRINTLN("kB");
      DEBUG_PRINT(F("Free PSRAM:  "));     DEBUG_PRINT(ESP.getFreePsram()/1024); DEBUG_PRINTLN("kB");
	  DEBUG_PRINT(F("PSRAM in use:")); DEBUG_PRINT(ESP.getPsramSize() - ESP.getFreePsram()); DEBUG_PRINTLN(F(" Bytes"));

    } else {
      //DEBUG_PRINTLN(F("No PSRAM"));
	}
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
  if (!Serial) delay(1000); // WLEDMM make sure that Serial has initalized

  #if !ARDUINO_USB_CDC_ON_BOOT
  Serial.setTimeout(50);  // this causes troubles on new MCUs that have a "virtual" USB Serial (HWCDC)
  #else
  #endif
  #if defined(WLED_DEBUG) && defined(ARDUINO_ARCH_ESP32) && (defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32C3) || ARDUINO_USB_CDC_ON_BOOT)
  delay(2500);  // allow CDC USB serial to initialise
  #endif

  #if ARDUINO_USB_CDC_ON_BOOT
  delay(2500);  // WLEDMM: always allow CDC USB serial to initialise
  Serial.println("wait 1");  // waiting a bit longer ensures that a  debug messages are shown in serial monitor
  delay(2500);
  Serial.println("wait 2");
  delay(2500);

  Serial.flush();
  Serial.setTimeout(350); // WLEDMM: don't change timeout, as it causes crashes later
  // WLEDMM: redirect debug output to HWCDC
  Serial0.setDebugOutput(false);
  Serial.setDebugOutput(true);
  #else
  if (Serial) Serial.setTimeout(50);  // WLEDMM - only when serial is initialized
  #endif

  //Serial0.setDebugOutput(false);
  #ifdef WLED_DEBUG
  Serial.setDebugOutput(true);
  #endif
  USER_FLUSH(); delay(100);
  USER_PRINTLN();
  USER_PRINT(F("---WLED "));
  USER_PRINT(versionString);
  USER_PRINT(" ");
  USER_PRINT(VERSION);
  USER_PRINTLN(F(" INIT---"));
  #ifdef WLED_RELEASE_NAME
  USER_PRINTF(" WLEDMM_%s %s, build %s.\n", versionString, releaseString, TOSTRING(VERSION)); // WLEDMM specific
  #endif

#ifdef ARDUINO_ARCH_ESP32
  DEBUG_PRINT(F("esp32 "));
  DEBUG_PRINTLN(ESP.getSdkVersion());
  #if defined(ESP_ARDUINO_VERSION)
    //DEBUG_PRINTF(F("arduino-esp32  0x%06x\n"), ESP_ARDUINO_VERSION);
    DEBUG_PRINTF("arduino-esp32 v%d.%d.%d\n", int(ESP_ARDUINO_VERSION_MAJOR), int(ESP_ARDUINO_VERSION_MINOR), int(ESP_ARDUINO_VERSION_PATCH));  // availeable since v2.0.0
  #else
    DEBUG_PRINTLN(F("arduino-esp32 v1.0.x\n"));  // we can't say in more detail.
  #endif

  USER_PRINT(F("CPU:   ")); USER_PRINT(ESP.getChipModel());
  USER_PRINT(F(" rev.")); USER_PRINT(ESP.getChipRevision());
  USER_PRINT(F(", ")); USER_PRINT(ESP.getChipCores()); USER_PRINT(F(" core(s)"));
  USER_PRINT(F(", ")); USER_PRINT(ESP.getCpuFreqMHz()); USER_PRINTLN(F("MHz."));
  USER_PRINT(F("FLASH: ")); USER_PRINT((ESP.getFlashChipSize()/1024)/1024);
  USER_PRINT(F("MB, Mode ")); USER_PRINT(ESP.getFlashChipMode());
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
  USER_PRINT(F(", speed ")); USER_PRINT(ESP.getFlashChipSpeed()/1000000);USER_PRINTLN(F("MHz."));
  
  #if defined(WLED_DEBUG) && defined(ARDUINO_ARCH_ESP32)
  showRealSpeed();
  #endif

#else
  // WLEDMM: more info for 8266
  USER_PRINTLN();
  USER_PRINTF("CPU:   ESP8266 (id 0x%08X)", ESP.getChipId());
  USER_PRINT(F(", ")); USER_PRINT(ESP.getCpuFreqMHz()); USER_PRINTLN(F("MHz."));
  USER_PRINT(F("CPU    Last Restart Reason = "));
  USER_PRINT((int)ESP.getResetInfoPtr()->reason); USER_PRINT(F(" -> "));
  USER_PRINTLN(ESP.getResetInfo());

  USER_PRINT(F("FLASH: ")); USER_PRINT((ESP.getFlashChipRealSize()/1024)/1024);
  USER_PRINT(F("MB, Mode ")); USER_PRINT((int)ESP.getFlashChipMode());
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
  USER_PRINT(F(", speed ")); USER_PRINT(ESP.getFlashChipSpeed()/1000000);USER_PRINT(F("MHz; "));
  USER_PRINT(F(" chip ID = 0x"));
  USER_PRINTF("%08X\n", ESP.getFlashChipId());
  USER_PRINTLN();

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
  USER_FLUSH();  // WLEDMM flush buffer now, before anything time-critial is started.

#ifdef WLED_DEBUG
  pinManager.allocatePin(hardwareTX, true, PinOwner::DebugOut); // TX (GPIO1 on ESP32) reserved for debug output
#endif
#ifdef WLED_ENABLE_DMX //reserve GPIO2 as hardcoded DMX pin
  pinManager.allocatePin(2, true, PinOwner::DMX);
#endif

// WLEDMM experimental: support for single neoPixel on Adafruit boards
#if 0
  //#ifdef PIN_NEOPIXEL
  //pinManager.allocatePin(PIN_NEOPIXEL, true, PinOwner::BusDigital);
  //#endif
  #ifdef NEOPIXEL_POWER
    pinManager.allocatePin(NEOPIXEL_POWER, true, PinOwner::Relay);  // just to ensure this GPIO will not get used for other purposes
    pinMode(NEOPIXEL_POWER, OUTPUT);
    digitalWrite(NEOPIXEL_POWER, HIGH);
  #endif
  #ifdef NEOPIXEL_I2C_POWER
    pinManager.allocatePin(NEOPIXEL_I2C_POWER, true, PinOwner::Relay);  // just to ensure this GPIO will not get used for other purposes
    pinMode(NEOPIXEL_I2C_POWER, OUTPUT);
    digitalWrite(NEOPIXEL_I2C_POWER, HIGH);
  #endif
#endif

  USER_PRINTLN();
  DEBUG_PRINTLN(F("Registering usermods ..."));
  registerUsermods();

  for (uint8_t i=1; i<WLED_MAX_BUTTONS; i++) btnPin[i] = -1;

  bool fsinit = false;
  USER_PRINTLN(F("Mount FS"));
#ifdef ARDUINO_ARCH_ESP32
  fsinit = WLED_FS.begin(true);
#else
  fsinit = WLED_FS.begin();
#endif
  if (!fsinit) {
    DEBUG_PRINTLN(F("FS failed!"));
    errorFlag = ERR_FS_BEGIN;
  }
#ifdef WLED_ADD_EEPROM_SUPPORT
  else deEEP();
#else
  initPresetsFile();
#endif
  updateFSInfo();

  USER_PRINTLN(F("done Mounting FS"));

  // generate module IDs must be done before AP setup
  escapedMac = WiFi.macAddress();
  escapedMac.replace(":", "");
  escapedMac.toLowerCase();

  WLED_SET_AP_SSID(); // otherwise it is empty on first boot until config is saved

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

  USER_PRINTLN(F("Usermods setup ..."));
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

  // fill in unique mdns default
  if (strcmp(cmDNS, "x") == 0) sprintf_P(cmDNS, PSTR("wled-%*s"), 6, escapedMac.c_str() + 6);
#ifndef WLED_DISABLE_MQTT
  if (mqttDeviceTopic[0] == 0) sprintf_P(mqttDeviceTopic, PSTR("wled/%*s"), 6, escapedMac.c_str() + 6);
  if (mqttClientID[0] == 0)    sprintf_P(mqttClientID, PSTR("WLED-%*s"), 6, escapedMac.c_str() + 6);
#endif

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
  
  #ifdef ARDUINO_ARCH_ESP32
  #ifdef ARDUINO_RUNNING_CORE
    DEBUG_PRINTF("Arduino core=%d (loop is now on core #%d)\n", int(ARDUINO_RUNNING_CORE), int(xPortGetCoreID()));
  #endif
  #ifdef ARDUINO_EVENT_RUNNING_CORE
      DEBUG_PRINTF("Arduino Event core=%d\n", int(ARDUINO_EVENT_RUNNING_CORE));
  #endif
  #endif

  // WLEDMM : dump GPIO infos (experimental, UI integration pending)
  //#ifdef WLED_DEBUG
  USER_PRINTLN(F("\nGPIO\t| Assigned to\t\t| Info"));
  USER_PRINTLN(F("--------|-----------------------|------------"));
  for(int pinNr = 0; pinNr < WLED_NUM_PINS; pinNr++) { // 49 = highest PIN on ESP32-S3
    if(pinManager.isPinOk(pinNr, false)) {
      //if ((!pinManager.isPinAllocated(pinNr)) && (pinManager.getPinSpecialText(pinNr).length() == 0)) continue;      // un-comment to hide no-name,unused GPIO pins
      bool is_inOut = pinManager.isPinOk(pinNr, true);
#if 0 // for testing
      USER_PRINT(pinManager.isPinAnalog(pinNr) ? "A": " ");
      USER_PRINT(pinManager.isPinADC1(pinNr) ? "1": " ");
      USER_PRINT(pinManager.isPinADC2(pinNr) ? "2": " ");
      USER_PRINT(pinManager.isPinTouch(pinNr) ? "T": " ");
      USER_PRINT(pinManager.isPinPWM(pinNr) ? " P": "  ");
      USER_PRINT(pinManager.isPinINT(pinNr) ? "I ": "  ");
#endif
      USER_PRINTF("%s  %2d\t  %-17s %s\t  %s\n", 
          (is_inOut?"i/o":"in "), 
          pinNr, 
          pinManager.getPinOwnerText(pinNr).c_str(),
          pinManager.getPinConflicts(pinNr).c_str(),
          pinManager.getPinSpecialText(pinNr).c_str()
      );
      USER_FLUSH();  // avoid lost lines (Serial buffer overflow)
    }
  }

#if 0 // for testing
  USER_PRINTLN(F("\n"));
  USER_PRINTF("ADC1-0 = %d, ADC1-3 = %d, ADC1-7 = %d, ADC2-0 = %d, ADC2-1 = %d, ADC2-8 = %d, ADC2-10 = %d\n",
    pinManager.getADCPin(PM_ADC1, 0), pinManager.getADCPin(PM_ADC1, 3), pinManager.getADCPin(PM_ADC1, 7), 
    pinManager.getADCPin(PM_ADC2, 0), pinManager.getADCPin(PM_ADC2, 1), pinManager.getADCPin(PM_ADC2, 8),
    pinManager.getADCPin(PM_ADC2, 10)
  );
  USER_PRINTLN();
  for(int p=0; p<11; p++) {
    if(pinManager.getADCPin(PinManagerClass::ADC1, p) < 255)
      USER_PRINTF("ADC1-%d = %d, ", p, pinManager.getADCPin(PinManagerClass::ADC1, p));
  }
  USER_PRINTLN();
  for(int p=0; p<11; p++) {
    if(pinManager.getADCPin(PinManagerClass::ADC2, p) < 255)
      USER_PRINTF("ADC2-%d = %d, ", p, pinManager.getADCPin(PinManagerClass::ADC2, p));
  }
  USER_PRINTLN(F("\n"));
#endif

  USER_PRINTLN(F("WLED initialization done.\n"));
  delay(50);
  // repeat Ada prompt
  #ifdef WLED_ENABLE_ADALIGHT
  if (!pinManager.isPinAllocated(hardwareRX) && !pinManager.isPinAllocated(hardwareTX)) {
    Serial.println(F("Ada"));
  }
  #endif

  //#endif
  // WLEDMM end
}

void WLED::beginStrip()
{
  // Initialize NeoPixel Strip and button
  strip.finalizeInit(); // busses created during deserializeConfig()
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
    WLED_SET_AP_SSID();
    strcpy_P(apPass, PSTR(WLED_AP_PASS));
  }
  USER_PRINT(F("Opening access point "));  // WLEDMM
  USER_PRINTLN(apSSID);                    // WLEDMM
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
    DEBUG_PRINTLN(")");
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
  USER_PRINTLN(F("initC: *** Ethernet successfully configured! ***"));  // WLEDMM
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
    USER_PRINTLN(F("No WiFi connection configured."));  // WLEDMM
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

  USER_PRINT(F("Connecting to "));
  USER_PRINT(clientSSID);
  USER_PRINTLN("...");

  // convert the "serverDescription" into a valid DNS hostname (alphanumeric)
  char hostname[25];
  prepareHostname(hostname);

#ifdef ESP8266
  WiFi.hostname(hostname);
#endif

  WiFi.begin(clientSSID, clientPass);

#ifdef ARDUINO_ARCH_ESP32
#ifdef WLEDMM_WIFI_POWERON_HACK
  // WLEDMM - if your board has issues connecting to WiFi, try this
  WiFi.setTxPower(WIFI_POWER_5dBm);  // required for ESP32-C3FH4-RGB
#endif
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

#ifndef WLED_DISABLE_ALEXA
  // init Alexa hue emulation
  if (alexaEnabled)
    alexaInit();
#endif

#ifndef WLED_DISABLE_OTA
  if (aOtaEnabled)
    ArduinoOTA.begin();
#endif

  strip.service();

  #ifndef WLED_DISABLE_OTA   // WLEDMM
  if (aOtaEnabled) {
    USER_PRINT(F("           ArduinoOTA: "));
    USER_PRINTLN(ArduinoOTA.getHostname());
  }
  #endif                     // WLEDMM end

  // Set up mDNS responder:
  if (strlen(cmDNS) > 0) {
    // "end" must be called before "begin" is called a 2nd time
    // see https://github.com/esp8266/Arduino/issues/7213
    MDNS.end();
    MDNS.begin(cmDNS);

    USER_PRINTF("mDNS started: %s.local\n", cmDNS); // WLEDMM
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
#ifndef WLED_DISABLE_MQTT
  initMqtt();
#endif
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
    USER_PRINTLN(F("Forcing reconnect."));
    initConnection();
    interfacesInited = false;
    forceReconnect = false;
    wasConnected = false;
    return;
  }
  if (!Network.isConnected()) {
    if (interfacesInited) {
      USER_PRINTLN(F("Disconnected!"));
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
    USER_PRINT(F("Connected! IP address: "));
    USER_PRINTLN(Network.localIP());
    if (improvActive) {
      if (improvError == 3) sendImprovStateResponse(0x00, true);
      sendImprovStateResponse(0x04);
      if (improvActive > 1) sendImprovRPCResponse(0x01);
    }
    initInterfaces();
    userConnected();
    usermods.connected();
    lastMqttReconnectAttempt = 0; // force immediate update

    // shut down AP
    if (apBehavior != AP_BEHAVIOR_ALWAYS && apActive) {
      dnsServer.stop();
      WiFi.softAPdisconnect(true);
      apActive = false;
      USER_PRINTLN(F("Access point disabled (handle)."));
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
