/*
 * Setup code
 */

void wledInit()
{
  EEPROM.begin(EEPSIZE);
  ledCount = EEPROM.read(229) + ((EEPROM.read(398) << 8) & 0xFF00);
  if (ledCount > MAX_LEDS || ledCount == 0) ledCount = 30;

  #ifdef ESP8266
  #if LEDPIN == 3
  if (ledCount > MAX_LEDS_DMA) ledCount = MAX_LEDS_DMA; //DMA method uses too much ram
  #endif
  #endif
  Serial.begin(115200);
  Serial.setTimeout(50);
  DEBUG_PRINTLN();
  DEBUG_PRINT("---WLED "); DEBUG_PRINT(versionString); DEBUG_PRINT(" "); DEBUG_PRINT(VERSION); DEBUG_PRINTLN(" INIT---");
  #ifdef ARDUINO_ARCH_ESP32
  DEBUG_PRINT("esp32 ");   DEBUG_PRINTLN(ESP.getSdkVersion());
  #else
  DEBUG_PRINT("esp8266 "); DEBUG_PRINTLN(ESP.getCoreVersion());
  #endif
  int heapPreAlloc = ESP.getFreeHeap();
  DEBUG_PRINT("heap ");
  DEBUG_PRINTLN(ESP.getFreeHeap());

  strip.init(EEPROM.read(372),ledCount,EEPROM.read(2204)); //init LEDs quickly
  strip.setBrightness(0);

  DEBUG_PRINT("LEDs inited. heap usage ~");
  DEBUG_PRINTLN(heapPreAlloc - ESP.getFreeHeap());

  #ifndef WLED_DISABLE_FILESYSTEM
   #ifdef ARDUINO_ARCH_ESP32
    SPIFFS.begin(true);
   #endif
    SPIFFS.begin();
  #endif

  DEBUG_PRINTLN("Load EEPROM");
  loadSettingsFromEEPROM(true);
  beginStrip();
  userSetup();
  if (strcmp(clientSSID,DEFAULT_CLIENT_SSID) == 0) showWelcomePage = true;
  WiFi.persistent(false);

  if (macroBoot>0) applyMacro(macroBoot);
  Serial.println("Ada");

  //generate module IDs
  escapedMac = WiFi.macAddress();
  escapedMac.replace(":", "");
  escapedMac.toLowerCase();
  if (strcmp(cmDNS,"x") == 0) //fill in unique mdns default
  {
    strcpy(cmDNS, "wled-");
    sprintf(cmDNS+5, "%*s", 6, escapedMac.c_str()+6);
  }
  if (mqttDeviceTopic[0] == 0)
  {
    strcpy(mqttDeviceTopic, "wled/");
    sprintf(mqttDeviceTopic+5, "%*s", 6, escapedMac.c_str()+6);
  }
  if (mqttClientID[0] == 0)
  {
    strcpy(mqttClientID, "WLED-");
    sprintf(mqttClientID+5, "%*s", 6, escapedMac.c_str()+6);
  }

  strip.service();

  #ifndef WLED_DISABLE_OTA
    if (aOtaEnabled)
    {
      ArduinoOTA.onStart([]() {
        #ifdef ESP8266
        wifi_set_sleep_type(NONE_SLEEP_T);
        #endif
        DEBUG_PRINTLN("Start ArduinoOTA");
      });
      if (strlen(cmDNS) > 0) ArduinoOTA.setHostname(cmDNS);
    }
  #endif
  #ifdef WLED_ENABLE_DMX
    dmx.init(512); // initialize with bus length
  #endif
  //HTTP server page init
  initServer();
}


void beginStrip()
{
  // Initialize NeoPixel Strip and button
  strip.setShowCallback(handleOverlayDraw);

#ifdef BTNPIN
  pinMode(BTNPIN, INPUT_PULLUP);
#endif

  if (bootPreset>0) applyPreset(bootPreset, turnOnAtBoot, true, true);
  colorUpdated(NOTIFIER_CALL_MODE_INIT);

  //init relay pin
  #if RLYPIN >= 0
    pinMode(RLYPIN, OUTPUT);
    #if RLYMDE
      digitalWrite(RLYPIN, bri);
    #else
      digitalWrite(RLYPIN, !bri);
    #endif
  #endif

  //disable button if it is "pressed" unintentionally
#ifdef BTNPIN
  if(digitalRead(BTNPIN) == LOW) buttonEnabled = false;
#else
  buttonEnabled = false;
#endif
}


void initAP(bool resetAP=false){
  if (apBehavior == AP_BEHAVIOR_BUTTON_ONLY && !resetAP) return;

  if (!apSSID[0] || resetAP) strcpy(apSSID, "WLED-AP");
  if (resetAP) strcpy(apPass,DEFAULT_AP_PASS);
  DEBUG_PRINT("Opening access point ");
  DEBUG_PRINTLN(apSSID);
  WiFi.softAPConfig(IPAddress(4, 3, 2, 1), IPAddress(4, 3, 2, 1), IPAddress(255,255,255,0));
  WiFi.softAP(apSSID, apPass, apChannel, apHide);
  
  if (!apActive) //start captive portal if AP active
  {
    DEBUG_PRINTLN("Init AP interfaces");
    server.begin();
    if (udpPort > 0 && udpPort != ntpLocalPort)
    {
      udpConnected = notifierUdp.begin(udpPort);
    }
    if (udpRgbPort > 0 && udpRgbPort != ntpLocalPort && udpRgbPort != udpPort)
    {
      udpRgbConnected = rgbUdp.begin(udpRgbPort);
    }

    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, "*", WiFi.softAPIP());
  }
  apActive = true;
}

void initConnection()
{
  WiFi.disconnect(); //close old connections
  #ifdef ESP8266
  WiFi.setPhyMode(WIFI_PHY_MODE_11N);
  #endif

  if (staticIP[0] != 0 && staticGateway[0] != 0)
  {
    WiFi.config(staticIP, staticGateway, staticSubnet, IPAddress(8,8,8,8));
  } else
  {
    WiFi.config(0U, 0U, 0U);
  }

  lastReconnectAttempt = millis();

  if (!WLED_WIFI_CONFIGURED)
  {
    DEBUG_PRINT("No connection configured. ");
    if (!apActive) initAP(); //instantly go to ap mode
    return;
  } else if (!apActive) {
    if (apBehavior == AP_BEHAVIOR_ALWAYS)
    {
      initAP();
    } else
    {
      DEBUG_PRINTLN("Access point disabled.");
      WiFi.softAPdisconnect(true);
    }
  }
  showWelcomePage = false;
  
  DEBUG_PRINT("Connecting to ");
  DEBUG_PRINT(clientSSID);
  DEBUG_PRINTLN("...");

  #ifdef ESP8266
   WiFi.hostname(serverDescription);
  #endif
  
   WiFi.begin(clientSSID, clientPass);
   
  #ifdef ARDUINO_ARCH_ESP32
   WiFi.setSleep(!noWifiSleep);
   WiFi.setHostname(serverDescription);
  #else
   wifi_set_sleep_type((noWifiSleep) ? NONE_SLEEP_T : MODEM_SLEEP_T);
  #endif
}

void initInterfaces() {
  DEBUG_PRINTLN("Init STA interfaces");
  
  if (hueIP[0] == 0)
  {
    hueIP[0] = WiFi.localIP()[0];
    hueIP[1] = WiFi.localIP()[1];
    hueIP[2] = WiFi.localIP()[2];
  }

  //init Alexa hue emulation
  if (alexaEnabled) alexaInit();

  #ifndef WLED_DISABLE_OTA
   if (aOtaEnabled) ArduinoOTA.begin();
  #endif

  strip.service();
  // Set up mDNS responder:
  if (strlen(cmDNS) > 0)
  {
    if (!aOtaEnabled) MDNS.begin(cmDNS);

    DEBUG_PRINTLN("mDNS started");
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("wled", "tcp", 80);
    MDNS.addServiceTxt("wled", "tcp", "mac", escapedMac.c_str());
  }
  server.begin();

  if (udpPort > 0 && udpPort != ntpLocalPort)
  {
    udpConnected = notifierUdp.begin(udpPort);
    if (udpConnected && udpRgbPort != udpPort) udpRgbConnected = rgbUdp.begin(udpRgbPort);
  }
  if (ntpEnabled) ntpConnected = ntpUdp.begin(ntpLocalPort);

  initBlynk(blynkApiKey);
  e131.begin((e131Multicast) ? E131_MULTICAST : E131_UNICAST , e131Universe, E131_MAX_UNIVERSE_COUNT);
  reconnectHue();
  initMqtt();
  interfacesInited = true;
  wasConnected = true;
}

byte stacO = 0;
uint32_t lastHeap;
unsigned long heapTime = 0;

void handleConnection() {
  if (millis() < 2000 && (!WLED_WIFI_CONFIGURED || apBehavior == AP_BEHAVIOR_ALWAYS)) return;
  if (lastReconnectAttempt == 0) initConnection();

  //reconnect WiFi to clear stale allocations if heap gets too low
  if (millis() - heapTime > 5000)
  {
    uint32_t heap = ESP.getFreeHeap();
    if (heap < 9000 && lastHeap < 9000) {
      DEBUG_PRINT("Heap too low! ");
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
    if (stac != stacO)
    {
      stacO = stac;
      DEBUG_PRINT("Connected AP clients: ");
      DEBUG_PRINTLN(stac);
      if (!WLED_CONNECTED && WLED_WIFI_CONFIGURED) { //trying to connect, but not connected
        if (stac) WiFi.disconnect(); //disable search so that AP can work
        else initConnection(); //restart search 
      }
    }
  }
  if (forceReconnect) {
    DEBUG_PRINTLN("Forcing reconnect.");
    initConnection();
    interfacesInited = false;
    forceReconnect = false;
    wasConnected = false;
    return;
  }
  if (!WLED_CONNECTED) {
    if (interfacesInited) {
      DEBUG_PRINTLN("Disconnected!");
      interfacesInited = false;
      initConnection();
    }
    if (millis() - lastReconnectAttempt > ((stac) ? 300000 : 20000) && WLED_WIFI_CONFIGURED) initConnection();
    if (!apActive && millis() - lastReconnectAttempt > 12000 && (!wasConnected || apBehavior == AP_BEHAVIOR_NO_CONN)) initAP(); 
  } else if (!interfacesInited) { //newly connected
    DEBUG_PRINTLN("");
    DEBUG_PRINT("Connected! IP address: ");
    DEBUG_PRINTLN(WiFi.localIP());
    initInterfaces();
    userConnected();

    //shut down AP
    if (apBehavior != AP_BEHAVIOR_ALWAYS && apActive)
    {
      dnsServer.stop();
      WiFi.softAPdisconnect(true);
      apActive = false;
      DEBUG_PRINTLN("Access point disabled.");
    }
  }
}

//by https://github.com/tzapu/WiFiManager/blob/master/WiFiManager.cpp
int getSignalQuality(int rssi)
{
  int quality = 0;

  if (rssi <= -100) {
    quality = 0;
  } else if (rssi >= -50) {
    quality = 100;
  } else {
    quality = 2 * (rssi + 100);
  }
  return quality;
}
