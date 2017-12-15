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
  EEPROM.begin(1024);
  loadSettingsFromEEPROM(true);
  DEBUG_PRINT("CC: SSID: ");
  DEBUG_PRINT(clientssid);

  WiFi.disconnect(); //close old connections

  if (staticip[0] != 0)
  {
    WiFi.config(staticip, staticgateway, staticsubnet);
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
    String settingsBuffer = getSettings();
    server.setContentLength(strlen_P(PAGE_settings0) + strlen_P(PAGE_settings1) + settingsBuffer.length());
    server.send(200, "text/html", "");
    server.sendContent_P(PAGE_settings0); 
    server.sendContent(settingsBuffer); 
    server.sendContent_P(PAGE_settings1);
  });
  server.on("/favicon.ico", HTTP_GET, [](){
    if(!handleFileRead("/favicon.ico")) server.send(200, "image/x-icon", favicon);
  });
  server.on("/", HTTP_GET, [](){
    if(!handleFileRead("/index.htm")) {
      server.setContentLength(strlen_P(PAGE_index0) + strlen_P(PAGE_index1) + strlen_P(PAGE_index2));
      server.send(200, "text/html", "");
      server.sendContent_P(PAGE_index0); 
      server.sendContent_P(PAGE_index1); 
      server.sendContent_P(PAGE_index2); 
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
    httpUpdater.setup(&server);
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
    if(!handleSet(server.uri())){
      server.send(404, "text/plain", "FileNotFound");
    }
  });

  server.begin();
  DEBUG_PRINTLN("HTTP server started");
  // Add service to MDNS
  MDNS.addService("http", "tcp", 80);
  //Init alexa service
  alexaInit();
  
  // Initialize NeoPixel Strip
  strip.init();
  strip.setLedCount(ledcount);
  strip.setMode(effectCurrent);
  strip.setColor(0);
  strip.setSpeed(effectSpeed);
  strip.setBrightness(255);
  strip.start();
  #ifdef CRONIXIE
  strip.driverModeCronixie(true);
  setCronixie(cronixieDefault);
  #endif
  if (bootPreset>0) applyPreset(bootPreset, turnOnAtBoot, true, true);
  colorUpdated(0);
  pinMode(buttonPin, INPUT_PULLUP);
}

void initAP(){
  WiFi.softAP(apssid.c_str(), appass.c_str(), apchannel, aphide);
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
      String save = apssid;
      onlyAP = true;
      #ifdef CRONIXIE
        if (apssid.length() <1) apssid = "CRONIXIE-AP";
      #else
        if (apssid.length() <1) apssid = "WLED-AP";
      #endif
      initAP();
      apssid = save;
      return;
    }
  }
}

