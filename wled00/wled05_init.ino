/*
 * Setup code
 */

void wledInit()
{ 
  EEPROM.begin(EEPSIZE);
  ledCount = ((EEPROM.read(229) << 0) & 0xFF) + ((EEPROM.read(398) << 8) & 0xFF00); if (ledCount > 1200 || ledCount == 0) ledCount = 10;
  //RMT eats up too much RAM
  #ifdef ARDUINO_ARCH_ESP32
   if (ledCount > 600) ledCount = 600;
  #endif
  if (!EEPROM.read(397)) strip.init(EEPROM.read(372),ledCount,EEPROM.read(2204)); //quick init

  Serial.begin(115200);
  Serial.setTimeout(50);
  
  #ifdef USEFS
   SPIFFS.begin();
  #endif
  
  DEBUG_PRINTLN("Load EEPROM");
  loadSettingsFromEEPROM(true);
  if (!initLedsLast) initStrip();
  DEBUG_PRINT("CSSID: ");
  DEBUG_PRINT(clientSSID);
  userBeginPreConnection();
  if (strcmp(clientSSID,"Your_Network") == 0) showWelcomePage = true;

  initCon();

  DEBUG_PRINTLN("");
  DEBUG_PRINT("Connected! IP address: ");
  DEBUG_PRINTLN(WiFi.localIP());

  if (hueIP[0] == 0)
  {
    hueIP[0] = WiFi.localIP()[0];
    hueIP[1] = WiFi.localIP()[1];
    hueIP[2] = WiFi.localIP()[2];
  }

  if (udpPort > 0 && udpPort != ntpLocalPort)
  {
    udpConnected = notifierUdp.begin(udpPort);
    if (udpConnected && udpRgbPort != udpPort) udpRgbConnected = rgbUdp.begin(udpRgbPort);
  }
  if (ntpEnabled && WiFi.status() == WL_CONNECTED)
  ntpConnected = ntpUdp.begin(ntpLocalPort);

  //start captive portal if AP active
  if (onlyAP || strlen(apSSID) > 0) 
  {
    dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
    dnsServer.start(53, "wled.me", WiFi.softAPIP());
    dnsActive = true;
  }

  prepareIds(); //UUID from MAC (for Alexa and MQTT)
  if (mqttDeviceTopic[0] == 0)
  {
    strcpy(mqttDeviceTopic, "wled/");
    strcat(mqttDeviceTopic, escapedMac.c_str());
  }
  
  //smartInit, we only init some resources when connected
  if (!onlyAP && WiFi.status() == WL_CONNECTED)
  {
    mqttTCPClient = new WiFiClient();
    mqtt = new PubSubClient(*mqttTCPClient);
    mqttInit = initMQTT();
  }
   
  if (!initLedsLast) strip.service();

  //HTTP server page init
  initServer();
  
  if (!initLedsLast) strip.service();
  //init Alexa hue emulation
  if (alexaEnabled && !onlyAP) alexaInit();

  server.begin();
  DEBUG_PRINTLN("HTTP server started");

  //init ArduinoOTA
  if (!onlyAP) {
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
  
    if (!initLedsLast) strip.service();
    // Set up mDNS responder:
    if (strlen(cmDNS) > 0 && !onlyAP)
    {
      MDNS.begin(cmDNS);
      DEBUG_PRINTLN("mDNS responder started");
      // Add service to MDNS
      MDNS.addService("http", "tcp", 80);
    }
    if (!initLedsLast) strip.service();

    initBlynk(blynkApiKey);
    initE131();

    hueClient = new HTTPClient();
  } else {
    e131Enabled = false;
  }

  if (initLedsLast) initStrip();
  userBegin();

  if (macroBoot>0) applyMacro(macroBoot);
  Serial.println("Ada");
}


void initStrip()
{
  // Initialize NeoPixel Strip and button
  if (initLedsLast) strip.init(useRGBW,ledCount,skipFirstLed);
  strip.setReverseMode(reverseMode);
  strip.setColor(0);
  strip.setBrightness(255);

  pinMode(BTNPIN, INPUT_PULLUP);

  if (bootPreset>0) applyPreset(bootPreset, turnOnAtBoot, true, true);
  colorUpdated(0);

  //disable button if it is "pressed" unintentionally
  if(digitalRead(BTNPIN) == LOW) buttonEnabled = false;
}


void initAP(){
  bool set = apSSID[0];
  if (!set) strcpy(apSSID,"WLED-AP");
  WiFi.softAP(apSSID, apPass, apChannel, apHide);
  if (!set) apSSID[0] = 0;
}


void initCon()
{
  WiFi.disconnect(); //close old connections

  if (staticIP[0] != 0)
  {
    WiFi.config(staticIP, staticGateway, staticSubnet, staticDNS);
  } else
  {
    WiFi.config(0U, 0U, 0U);
  }

  if (strlen(apSSID)>0)
  {
    DEBUG_PRINT(" USING AP");
    DEBUG_PRINTLN(strlen(apSSID));
    initAP();
  } else
  {
    DEBUG_PRINTLN(" NO AP");
    WiFi.softAPdisconnect(true);
  }
  int fail_count = 0;
  if (strlen(clientSSID) <1 || strcmp(clientSSID,"Your_Network") == 0)
    fail_count = apWaitTimeSecs*2; //instantly go to ap mode
  #ifndef ARDUINO_ARCH_ESP32
   WiFi.hostname(serverDescription);
  #endif
   WiFi.begin(clientSSID, clientPass);
  #ifdef ARDUINO_ARCH_ESP32
   WiFi.setHostname(serverDescription);
  #endif
  unsigned long lastTry = 0;
  bool con = false;
  while(!con)
  {
    yield();
    if (!initLedsLast)
    {
      handleTransitions();
      handleButton();
      handleOverlays();
      if (briT) strip.service();
    }
    if (millis()-lastTry > 499) {
      con = (WiFi.status() == WL_CONNECTED);
      lastTry = millis();
      DEBUG_PRINTLN("C_NC");
      if (!recoveryAPDisabled && fail_count > apWaitTimeSecs*2)
      {
        WiFi.disconnect();
        DEBUG_PRINTLN("Can't connect. Opening AP...");
        onlyAP = true;
        initAP();
        return;
      }
      fail_count++;
    }
  }
}


//fill string buffer with build info
void getBuildInfo()
{
  olen = 0;
  oappend("hard-coded build info:\r\n\n");
  #ifdef ARDUINO_ARCH_ESP32
  oappend("platform: esp32");
  #else
  oappend("platform: esp8266");
  #endif
  oappend("\r\nversion: ");
  oappend(versionString);
  oappend("\r\nbuild: ");
  oappendi(VERSION);
  oappend("\r\neepver: ");
  oappendi(EEPVER);
  oappend("\r\nopt: ");
  #ifndef WLED_DISABLE_ALEXA
  oappend("alexa ");
  #endif
  #ifndef WLED_DISABLE_BLYNK
  oappend("blynk ");
  #endif
  #ifndef WLED_DISABLE_CRONIXIE
  oappend("cronixie ");
  #endif
  #ifndef WLED_DISABLE_HUESYNC
  oappend("huesync ");
  #endif
  #ifndef WLED_DISABLE_MOBILE_UI
  oappend("mobile-ui ");
  #endif
  #ifndef WLED_DISABLE_OTA
  oappend("ota");
  #endif
  #ifdef USEFS
  oappend("\r\nspiffs: true\r\n");
  #else
  oappend("\r\nspiffs: false\r\n");
  #endif
  #ifdef WLED_DEBUG
  oappend("debug: true\r\n");
  #else
  oappend("debug: false\r\n");
  #endif
  oappend("button-pin: gpio");
  oappendi(BTNPIN);
  oappend("\r\nstrip-pin: gpio");
  oappendi(LEDPIN);
  oappend("\r\nbrand: wled");
  oappend("\r\nbuild-type: dev\r\n");
}


bool checkClientIsMobile(String useragent)
{
  //to save complexity this function is not comprehensive
  if (useragent.indexOf("Android") >= 0) return true;
  if (useragent.indexOf("iPhone") >= 0) return true;
  if (useragent.indexOf("iPod") >= 0) return true;
  return false;
}
