void wledInit()
{
  Serial.begin(115200);
    Serial.println();

    for(uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }

  SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {    
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    Serial.printf("\n");
  }
  Serial.println("Init EEPROM");
  EEPROM.begin(1024);
  loadSettingsFromEEPROM();

  Serial.print("CC: SSID: ");
  Serial.print(clientssid);

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
    Serial.print("USING AP");
    Serial.println(apssid.length());
    initAP();
  } else
  {
    Serial.println("NO AP");
    WiFi.softAPdisconnect(true);
  }

  initCon();

  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
  
  // Set up mDNS responder:
  if (cmdns != NULL && !only_ap && !MDNS.begin(cmdns.c_str())) {
    Serial.println("Error setting up MDNS responder!");
    down();
  }
    Serial.println("mDNS responder started");

  //SERVER INIT
  //settings page
  server.on("/settings", HTTP_GET, [](){
    if(!handleFileRead("/settings.htm")) server.send(404, "text/plain", "FileNotFound");
  });
  server.on("/button.png", HTTP_GET, [](){
    if(!handleFileRead("/button.png")) server.send(404, "text/plain", "FileNotFound");
  });
  server.on("/moon.png", HTTP_GET, [](){
    if(!handleFileRead("/moon.png")) server.send(404, "text/plain", "FileNotFound");
  });
  server.on("/favicon.ico", HTTP_GET, [](){
    if(!handleFileRead("/favicon.ico")) server.send(404, "text/plain", "FileNotFound");
  });
  server.on("/", HTTP_GET, [](){
    if(!handleFileRead("/index.htm")) server.send(404, "text/plain", "FileNotFound");
  });
  server.on("/reset", HTTP_GET, [](){
    server.send(200, "text/plain", "Rebooting... Go to main page when lights turn on.");
    reset();
  });
  server.on("/set-settings", HTTP_POST, [](){
    handleSettingsSet();
    if(!handleFileRead("/settingssaved.htm")) server.send(404, "text/plain", "SettingsSaved");
  });
  if (!ota_lock){
    server.on("/edit", HTTP_GET, [](){
    if(!handleFileRead("/edit.htm")) server.send(404, "text/plain", "FileNotFound");
    });
    server.on("/edit", HTTP_PUT, handleFileCreate);
    server.on("/edit", HTTP_DELETE, handleFileDelete);
    server.on("/edit", HTTP_POST, [](){ server.send(200, "text/plain", ""); }, handleFileUpload);
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
  }
  //called when the url is not defined here, ajax-in; get-settings
  server.onNotFound([](){
    if(!handleSet(server.uri())){
      server.send(404, "text/plain", "FileNotFound");
    }
  });

  server.begin();
  Serial.println("HTTP server started");

  // Add service to MDNS
  MDNS.addService("http", "tcp", 80);
  // Initialize NeoPixel Strip
  strip.Begin();
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
    Serial.println("C_NC");
    fail_count++;
    if (fail_count > 32)
    {
      WiFi.disconnect();
      Serial.println("Can't connect to network. Opening AP...");
      String save = apssid;
      only_ap = true;
      if (apssid.length() <1) apssid = "WLED-AP";
      initAP();
      apssid = save;
      return;
    }
  }
}

