/*
 * Setup code
 */

void wledInit()
{
  EEPROM.begin(EEPSIZE);
  ledCount = EEPROM.read(229) + ((EEPROM.read(398) << 8) & 0xFF00);
  if (ledCount > 1200 || ledCount == 0) ledCount = 30;
  #ifndef ARDUINO_ARCH_ESP32
  #if LEDPIN == 3
  if (ledCount > 300) ledCount = 300; //DMA method uses too much ram
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
  userBeginPreConnection();
  if (strcmp(clientSSID,"Your_Network") == 0) showWelcomePage = true;
  WiFi.persistent(false);

  if (macroBoot>0) applyMacro(macroBoot);
  Serial.println("Ada");

  if (udpPort > 0 && udpPort != ntpLocalPort)
  {
    udpConnected = notifierUdp.begin(udpPort);
    if (udpConnected && udpRgbPort != udpPort) udpRgbConnected = rgbUdp.begin(udpRgbPort);
  }
  if (ntpEnabled && WLED_CONNECTED)
  ntpConnected = ntpUdp.begin(ntpLocalPort);

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

  //HTTP server page init
  initServer();

  strip.service();

  server.begin();
  DEBUG_PRINTLN("HTTP server started");

  //init ArduinoOTA
  if (true) {
    #ifndef WLED_DISABLE_OTA
    if (aOtaEnabled)
    {
      ArduinoOTA.onStart([]() {
        #ifndef ARDUINO_ARCH_ESP32
        wifi_set_sleep_type(NONE_SLEEP_T);
        #endif
        DEBUG_PRINTLN("Start ArduinoOTA");
      });
      if (strlen(cmDNS) > 0) ArduinoOTA.setHostname(cmDNS);
      ArduinoOTA.begin();
    }
    #endif

    strip.service();
    // Set up mDNS responder:
    if (strlen(cmDNS) > 0 && WLED_CONNECTED)
    {
      MDNS.begin(cmDNS);
      DEBUG_PRINTLN("mDNS responder started");
      // Add service to MDNS
      MDNS.addService("http", "tcp", 80);
      MDNS.addService("wled", "tcp", 80);
    }
    strip.service();

    initBlynk(blynkApiKey);
    initE131();
    reconnectHue();
  } else {
    e131Enabled = false;
  }

  initConnection();
  userBegin();
}


void beginStrip()
{
  // Initialize NeoPixel Strip and button

#ifdef BTNPIN
  pinMode(BTNPIN, INPUT_PULLUP);
#endif

  if (bootPreset>0) applyPreset(bootPreset, turnOnAtBoot, true, true);
  colorUpdated(0);

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
  if (recoveryAPDisabled) return;
  bool set = apSSID[0];
  if (!set || resetAP) strcpy(apSSID, "WLED-AP");
  if (resetAP) strcpy(apPass,"wled1234");
  DEBUG_PRINT("Opening access point ");
  DEBUG_PRINTLN(apSSID);
  WiFi.softAPConfig(IPAddress(4, 3, 2, 1), IPAddress(4, 3, 2, 1), IPAddress(255,255,255,0));
  WiFi.softAP(apSSID, apPass, apChannel, apHide);
  if (!set) apSSID[0] = 0;
  
  if (!apActive) //start captive portal if AP active
  {
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, "*", WiFi.softAPIP());
  }
  apActive = true;
}

void initConnection()
{
  WiFi.disconnect(); //close old connections

  if (staticIP[0] != 0)
  {
    WiFi.config(staticIP, staticGateway, staticSubnet, IPAddress(8,8,8,8));
  } else
  {
    WiFi.config(0U, 0U, 0U);
  }

  lastReconnectAttempt = millis();

  if (apAlwaysOn)
  {
    initAP();
  } else if (!apActive)
  {
    DEBUG_PRINTLN("Access point disabled.");
    WiFi.softAPdisconnect(true);
  }

  if (!WLED_WIFI_CONFIGURED)
  {
    DEBUG_PRINT("No connection configured. ");
    initAP(); //instantly go to ap mode
    return;
  }
  showWelcomePage = false;
  
  DEBUG_PRINT("Connecting to ");
  DEBUG_PRINT(clientSSID);
  DEBUG_PRINTLN("...");

  #ifndef ARDUINO_ARCH_ESP32
   WiFi.hostname(serverDescription);
  #endif
   WiFi.begin(clientSSID, clientPass);
  #ifdef ARDUINO_ARCH_ESP32
   WiFi.setHostname(serverDescription);
  #endif
}

void initInterfaces() {
  if (hueIP[0] == 0)
  {
    hueIP[0] = WiFi.localIP()[0];
    hueIP[1] = WiFi.localIP()[1];
    hueIP[2] = WiFi.localIP()[2];
  }

  //init Alexa hue emulation
  if (alexaEnabled) alexaInit();

  initMqtt();

  #ifndef WLED_DISABLE_OTA
    if (aOtaEnabled)
    {
      ArduinoOTA.onStart([]() {
        #ifndef ARDUINO_ARCH_ESP32
        wifi_set_sleep_type(NONE_SLEEP_T);
        #endif
        DEBUG_PRINTLN("Start ArduinoOTA");
      });
      if (strlen(cmDNS) > 0) ArduinoOTA.setHostname(cmDNS);
      ArduinoOTA.begin();
    }
  #endif

  strip.service();
  // Set up mDNS responder:
  if (strlen(cmDNS) > 0)
  {
    MDNS.begin(cmDNS);
    DEBUG_PRINTLN("mDNS responder started");
    // Add service to MDNS
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("wled", "tcp", 80);
  }
  strip.service();

  initBlynk(blynkApiKey);
  initE131();
  reconnectHue();
    
  interfacesInited = true;
}

byte stacO = 0;

void handleConnection() {
  byte stac = wifi_softap_get_station_num();
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
  if (forceReconnect) {
    DEBUG_PRINTLN("Forcing reconnect.");
    initConnection();
    interfacesInited = false;
    forceReconnect = false;
    return;
  }
  if (!WLED_CONNECTED) {
    if (interfacesInited) {
      DEBUG_PRINTLN("Disconnected!");
      interfacesInited = false;
      initConnection();
    }
    if (millis() - lastReconnectAttempt > 300000 && WLED_WIFI_CONFIGURED) initConnection();
    if (!apActive && millis() - lastReconnectAttempt > apWaitTimeSecs*1000) initAP(); 
  } else if (!interfacesInited) { //newly connected
    DEBUG_PRINTLN("");
    DEBUG_PRINT("Connected! IP address: ");
    DEBUG_PRINTLN(WiFi.localIP());
    initInterfaces();

    //shut down AP
    if (!apAlwaysOn && apActive)
    {
      dnsServer.stop();
      DEBUG_PRINTLN("Access point disabled.");
      WiFi.softAPdisconnect(true);
      apActive = false;
    }
  }
}


bool checkClientIsMobile(String useragent)
{
  //to save complexity this function is not comprehensive
  if (useragent.indexOf("Android") >= 0) return true;
  if (useragent.indexOf("iPhone") >= 0) return true;
  if (useragent.indexOf("iPod") >= 0) return true;
  if (useragent.indexOf("iPad") >= 0) return true;
  return false;
}
