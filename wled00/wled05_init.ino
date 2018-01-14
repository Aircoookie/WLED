/*
 * Setup code
 */

void wledInit()
{
  Serial.begin(115200);

  #ifdef USEFS
  SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {    
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      #ifdef DEBUG
      Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
      #endif
    }
    DEBUG_PRINTF("\n");
  }
  #endif
  
  DEBUG_PRINTLN("Init EEPROM");
  EEPROM.begin(EEPSIZE);
  loadSettingsFromEEPROM(true);
  DEBUG_PRINT("CC: SSID: ");
  DEBUG_PRINT(clientssid);

  WiFi.disconnect(); //close old connections

  if (staticip[0] != 0)
  {
    WiFi.config(staticip, staticgateway, staticsubnet, staticdns);
  } else
  {
    WiFi.config(0U, 0U, 0U);
  }

  if (apssid.length()>0)
  {
    DEBUG_PRINT("USING AP");
    DEBUG_PRINTLN(apssid.length());
    initAP();
  } else
  {
    DEBUG_PRINTLN("NO AP");
    WiFi.softAPdisconnect(true);
  }

  initCon();

  DEBUG_PRINTLN("");
  DEBUG_PRINT("Connected! IP address: ");
  DEBUG_PRINTLN(WiFi.localIP());
  
  // Set up mDNS responder:
  if (cmdns != NULL && !onlyAP && !MDNS.begin(cmdns.c_str())) {
    DEBUG_PRINTLN("Error setting up MDNS responder!");
    down();
  }
  DEBUG_PRINTLN("mDNS responder started");

  if (udpPort > 0 && udpPort != ntpLocalPort && WiFi.status() == WL_CONNECTED)
  {
    udpConnected = notifierUdp.begin(udpPort);
  }
  if (ntpEnabled && WiFi.status() == WL_CONNECTED)
  ntpConnected = ntpUdp.begin(ntpLocalPort);

  //SERVER INIT
  //settings page
  server.on("/settings", HTTP_GET, [](){
    if (!arlsTimeout) //do not serve while receiving realtime
    {
      String settingsBuffer = getSettings();
      server.setContentLength(strlen_P(PAGE_settings0) + strlen_P(PAGE_settings1) + settingsBuffer.length());
      server.send(200, "text/html", "");
      server.sendContent_P(PAGE_settings0); 
      server.sendContent(settingsBuffer); 
      server.sendContent_P(PAGE_settings1);
    } else {
        server.send(200, "text/plain", "The settings are not available while receiving real-time data.");
    }
  });
  server.on("/favicon.ico", HTTP_GET, [](){
    if(!handleFileRead("/favicon.ico")) server.send(200, "image/x-icon", favicon);
  });
  server.on("/", HTTP_GET, [](){
    if(!handleFileRead("/index.htm")) {
      if (!arlsTimeout) //do not serve while receiving realtime
      {
        server.setContentLength(strlen_P(PAGE_index0) + strlen_P(PAGE_index1) + strlen_P(PAGE_index2));
        server.send(200, "text/html", "");
        server.sendContent_P(PAGE_index0); 
        server.sendContent_P(PAGE_index1); 
        server.sendContent_P(PAGE_index2);
      } else {
        server.send(200, "text/plain", "The WLED UI is not available while receiving real-time data.");
      }
    }
  });
  server.on("/reset", HTTP_GET, [](){
    server.send(200, "text/plain", "Rebooting...");
    reset();
  });
  server.on("/set-settings", HTTP_POST, [](){
    handleSettingsSet();
    if(!handleFileRead("/settingssaved.htm")) server.send(200, "text/html", PAGE_settingssaved);
  });
  server.on("/version", HTTP_GET, [](){
    server.send(200, "text/plain", (String)VERSION);
    });
  server.on("/uptime", HTTP_GET, [](){
    server.send(200, "text/plain", (String)millis());
    });
  server.on("/freeheap", HTTP_GET, [](){
    server.send(200, "text/plain", (String)ESP.getFreeHeap());
    });
  server.on("/power", HTTP_GET, [](){
    String val = (String)(int)strip.getPowerEstimate(ledcount,strip.getColor(),strip.getBrightness());
    val += "mA currently\nNotice: This is just an estimate which does not take into account several factors (like effects and wire resistance). It is NOT an accurate measurement!";
    server.send(200, "text/plain", val);
    });
  server.on("/teapot", HTTP_GET, [](){
    server.send(418, "text/plain", "418. I'm a teapot. (Tangible Embedded Advanced Project Of Twinkling)");
    });
  server.on("/build", HTTP_GET, [](){
    String info = "hard-coded build info:\r\n\n";
    #ifdef ARDUINO_ARCH_ESP32
    info += "platform: esp32\r\n";
    #else
    info += "platform: esp8266\r\n";
    #endif
    info += "name: " + versionName + "\r\n";
    info += "version: " + (String)VERSION + "\r\n";
    #ifdef RGBW
    info += "rgbw: true\r\n";
    #else
    info += "rgbw: false\r\n";
    #endif
    info += "max-leds: " + (String)LEDCOUNT + "\r\n";
    info += "max-direct: " + (String)MAXDIRECT + "\r\n";
    #ifdef USEOVERLAYS
    info += "overlays: true\r\n";
    #else
    info += "overlays: false\r\n";
    #endif
    #ifdef CRONIXIE
    info += "cronixie: true\r\n";
    #else
    info += "cronixie: false\r\n";
    #endif
    #ifdef USEFS
    info += "spiffs: true\r\n";
    #else
    info += "spiffs: false\r\n";
    #endif
    #ifdef DEBUG
    info += "debug: true\r\n";
    #else
    info += "debug: false\r\n";
    #endif
    info += "button-pin: gpio" + String(buttonPin) + "\r\n";
    #ifdef ARDUINO_ARCH_ESP32
    info += "strip-pin: gpio" + String(PIN) + "\r\n";
    #else
    info += "strip-pin: gpio2\r\n";
    #endif
    server.send(200, "text/plain", info);
    });
  //if OTA is allowed
  if (!otaLock){
    server.on("/edit", HTTP_GET, [](){
    if(!handleFileRead("/edit.htm")) server.send(200, "text/html", PAGE_edit);
    });
    #ifdef USEFS
    server.on("/edit", HTTP_PUT, handleFileCreate);
    server.on("/edit", HTTP_DELETE, handleFileDelete);
    server.on("/edit", HTTP_POST, [](){ server.send(200, "text/plain", ""); }, handleFileUpload);
    server.on("/list", HTTP_GET, handleFileList);
    #endif
    server.on("/down", HTTP_GET, down);
    server.on("/cleareeprom", HTTP_GET, clearEEPROM);
    //init ota page
    httpUpdater.setup(&server); //only for ESP8266
    //init ArduinoOTA
    ArduinoOTA.onStart([]() {
      #ifndef ARDUINO_ARCH_ESP32
      wifi_set_sleep_type(NONE_SLEEP_T);
      #endif
      DEBUG_PRINTLN("Start ArduinoOTA");
    });
    ArduinoOTA.begin();
  } else
  {
    server.on("/edit", HTTP_GET, [](){
    server.send(500, "text/plain", "OTA lock active");
    });
    server.on("/down", HTTP_GET, [](){
    server.send(500, "text/plain", "OTA lock active");
    });
    server.on("/cleareeprom", HTTP_GET, [](){
    server.send(500, "text/plain", "OTA lock active");
    });
    server.on("/update", HTTP_GET, [](){
    server.send(500, "text/plain", "OTA lock active");
    });
    server.on("/list", HTTP_GET, [](){
    server.send(500, "text/plain", "OTA lock active");
    });
  }
  //called when the url is not defined here, ajax-in; get-settings
  server.onNotFound([](){
    DEBUG_PRINTLN("Not-Found HTTP call:");
    DEBUG_PRINTLN("URI: " + server.uri());
    DEBUG_PRINTLN("Body: " + server.arg(0));
    if(!handleSet(server.uri())){
      if(!handleAlexaApiCall(server.uri(),server.arg(0)))
      server.send(404, "text/plain", "FileNotFound");
    }
  });
  //init Alexa hue emulation
  if (alexaEnabled) alexaInit();

  server.begin();
  DEBUG_PRINTLN("HTTP server started");
  // Add service to MDNS
  MDNS.addService("http", "tcp", 80);

  // Initialize NeoPixel Strip
  strip.init();
  strip.setLedCount(ledcount);
  strip.setMode(effectCurrent);
  strip.setColor(0);
  strip.setSpeed(effectSpeed);
  strip.setBrightness(255);
  strip.start();

  pinMode(buttonPin, INPUT_PULLUP);
  #ifdef CRONIXIE
  strip.driverModeCronixie(true);
  strip.setCronixieBacklight(cronixieBacklight);
  setCronixie(cronixieDefault);
  #endif
  if (bootPreset>0) applyPreset(bootPreset, turnOnAtBoot, true, true);
  colorUpdated(0);
  if(digitalRead(buttonPin) == LOW) buttonEnabled = false; //disable button if it is "pressed" unintentionally
}

void initAP(){
  String save = apssid;
  #ifdef CRONIXIE
    if (apssid.length() <1) apssid = "CRONIXIE-AP";
  #else
    if (apssid.length() <1) apssid = "WLED-AP";
  #endif
  WiFi.softAP(apssid.c_str(), appass.c_str(), apchannel, aphide);
  apssid = save;
}

void initCon()
{
  int fail_count = 0;
  if (clientssid.length() <1) fail_count = 33;
  WiFi.begin(clientssid.c_str(), clientpass.c_str());
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    DEBUG_PRINTLN("C_NC");
    fail_count++;
    if (!recoveryAPDisabled && fail_count > apWaitTimeSecs*2)
    {
      WiFi.disconnect();
      DEBUG_PRINTLN("Can't connect. Opening AP...");
      onlyAP = true;
      initAP();
      return;
    }
  }
}

