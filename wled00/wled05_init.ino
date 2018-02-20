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
  buildCssColorString();

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
    serveSettings(0);
  });
  server.on("/settings/wifi", HTTP_GET, [](){
    serveSettings(1);
  });
  server.on("/settings/leds", HTTP_GET, [](){
    serveSettings(2);
  });
  server.on("/settings/ui", HTTP_GET, [](){
    serveSettings(3);
  });
  server.on("/settings/sync", HTTP_GET, [](){
    serveSettings(4);
  });
  server.on("/settings/time", HTTP_GET, [](){
    serveSettings(5);
  });
  server.on("/settings/sec", HTTP_GET, [](){
    serveSettings(6);
  });
  
  server.on("/favicon.ico", HTTP_GET, [](){
    if(!handleFileRead("/favicon.ico")) server.send(200, "image/x-icon", favicon);
  });
  
  server.on("/", HTTP_GET, [](){
    if (!showWelcomePage){
      if(!handleFileRead("/index.htm")) {
        serveIndex();
      }
    }else{
      if(!handleFileRead("/welcome.htm")) {
        serveSettings(255);
      }
    }
  });
  
  server.on("/sliders", HTTP_GET, serveIndex);
  
  server.on("/welcome", HTTP_GET, [](){
    serveSettings(255);
  });
  
  server.on("/reset", HTTP_GET, [](){
    serveMessage(200,"Rebooting now...","(takes ~15 seconds)");
    reset();
  });
  
  server.on("/settings/wifi", HTTP_POST, [](){
    handleSettingsSet(1);
    serveMessage(200,"WiFi settings saved.","Rebooting now...");
    reset();
  });

  server.on("/settings/leds", HTTP_POST, [](){
    handleSettingsSet(2);
    serveMessage(200,"LED settings saved.","",true);
  });

  server.on("/settings/ui", HTTP_POST, [](){
    handleSettingsSet(3);
    serveMessage(200,"UI settings saved.","",true);
  });

  server.on("/settings/sync", HTTP_POST, [](){
    handleSettingsSet(4);
    serveMessage(200,"Sync settings saved.","",true);
  });

  server.on("/settings/time", HTTP_POST, [](){
    handleSettingsSet(5);
    serveMessage(200,"Time settings saved.","If you made changes to NTP, please reboot.",true);
  });

  server.on("/settings/sec", HTTP_POST, [](){
    handleSettingsSet(6);
    serveMessage(200,"Security settings saved.","Rebooting now...");
    reset();
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
    serveMessage(418, "418. I'm a teapot.","(Tangible Embedded Advanced Project Of Twinkling)");
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
    info += "eepver: " + String(EEPVER) + "\r\n";
    #ifdef RGBW
    info += "rgbw: true\r\n";
    #else
    info += "rgbw: false\r\n";
    #endif
    info += "max-leds: " + (String)LEDCOUNT + "\r\n";
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
    httpUpdater.setup(&server);
  } else
  {
    server.on("/edit", HTTP_GET, [](){
    serveMessage(500, "Access Denied", txd);
    });
    server.on("/down", HTTP_GET, [](){
    serveMessage(500, "Access Denied", txd);
    });
    server.on("/cleareeprom", HTTP_GET, [](){
    serveMessage(500, "Access Denied", txd);
    });
    server.on("/update", HTTP_GET, [](){
    serveMessage(500, "Access Denied", txd);
    });
    server.on("/list", HTTP_GET, [](){
    serveMessage(500, "Access Denied", txd);
    });
  }
  //called when the url is not defined here, ajax-in; get-settings
  server.onNotFound([](){
    DEBUG_PRINTLN("Not-Found HTTP call:");
    DEBUG_PRINTLN("URI: " + server.uri());
    DEBUG_PRINTLN("Body: " + server.arg(0));
    if(!handleSet(server.uri())){
      if(!handleAlexaApiCall(server.uri(),server.arg(0)))
      server.send(404, "text/plain", "Not Found");
    }
  });
  //init Alexa hue emulation
  if (alexaEnabled) alexaInit();

  server.begin();
  DEBUG_PRINTLN("HTTP server started");
  // Add service to MDNS
  MDNS.addService("http", "tcp", 80);

  //init ArduinoOTA
  if (aOtaEnabled)
  {
    ArduinoOTA.onStart([]() {
      #ifndef ARDUINO_ARCH_ESP32
      wifi_set_sleep_type(NONE_SLEEP_T);
      #endif
      DEBUG_PRINTLN("Start ArduinoOTA");
    });
    ArduinoOTA.begin();
  }

  // Initialize NeoPixel Strip
  strip.init();
  strip.setLedCount(ledcount);
  strip.setColor(0);
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

void buildCssColorString()
{
  switch (currentTheme)
  {
    default: cssCol[0]="D9B310"; cssCol[1]="0B3C5D"; cssCol[2]="1D2731"; cssCol[3]="328CC1"; cssCol[4]="000"; break; //night
    case 1: cssCol[0]="eee"; cssCol[1]="ddd"; cssCol[2]="b9b9b9"; cssCol[3]="049"; cssCol[4]="777"; break; //modern
    case 2: cssCol[0]="abc"; cssCol[1]="fff"; cssCol[2]="ddd"; cssCol[3]="000"; cssCol[4]="0004"; break; //bright
    case 3: cssCol[0]="c09f80"; cssCol[1]="d7cec7"; cssCol[2]="76323f"; cssCol[3]="888"; cssCol[4]="3334"; break; //wine
    case 4: cssCol[0]="3cc47c"; cssCol[1]="828081"; cssCol[2]="d9a803"; cssCol[3]="1e392a"; cssCol[4]="000a"; break; //electric
    case 5: cssCol[0]="57bc90"; cssCol[1]="a5a5af"; cssCol[2]="015249"; cssCol[3]="88c9d4"; cssCol[4]="0004"; break; //mint
    case 6: cssCol[0]="f7c331"; cssCol[1]="dcc7aa"; cssCol[2]="6b7a8f"; cssCol[3]="f7882f"; cssCol[4]="0007"; break; //amber
    case 7: cssCol[0]="fc3"; cssCol[1]="124"; cssCol[2]="334"; cssCol[3]="f1d"; cssCol[4]="f00"; break; //club
    case 14: cssCol[0]="fc7"; cssCol[1]="49274a"; cssCol[2]="94618e"; cssCol[3]="f4decb"; cssCol[4]="0008"; break; //end
    //case 15 do nothing since custom vals are already loaded
  }
  cssColorString="<style>:root{--aCol:#";
  cssColorString+=cssCol[0];
  cssColorString+=";--bCol:#";
  cssColorString+=cssCol[1];
  cssColorString+=";--cCol:#";
  cssColorString+=cssCol[2];
  cssColorString+=";--dCol:#";
  cssColorString+=cssCol[3];
  cssColorString+=";--sCol:#";
  cssColorString+=cssCol[4];
  cssColorString+=";--tCol:#";
  cssColorString+=cssCol[5];
  cssColorString+=";}";
}

void serveIndex()
{
  if (!arlsTimeout) //do not serve while receiving realtime
  {
    server.setContentLength(strlen_P(PAGE_index0) + cssColorString.length() + strlen_P(PAGE_index1) + strlen_P(PAGE_index2) + strlen_P(PAGE_index3));
    server.send(200, "text/html", "");
    server.sendContent_P(PAGE_index0);
    server.sendContent(cssColorString); 
    server.sendContent_P(PAGE_index1); 
    server.sendContent_P(PAGE_index2);
    server.sendContent_P(PAGE_index3);
  } else {
    server.send(200, "text/plain", "The WLED UI is not available while receiving real-time data.");
  }
}

void serveMessage(int code, String headl, String subl="", bool backToSettings)
{
  String messageBody = "<h2>";
  messageBody += headl;
  messageBody += "</h2>";
  messageBody += subl;
  if (backToSettings)
  {
    messageBody += "<form action=/settings><button type=submit>Back</button></form>";
  }else
  {
    messageBody += "<br><br><button type=\"button\" onclick=\"B()\">Back</button>";
  }
  messageBody += "</body></html>";
  server.setContentLength(strlen_P(PAGE_msg0) + cssColorString.length() + strlen_P(PAGE_msg1) + messageBody.length());
  server.send(code, "text/html", "");
  server.sendContent_P(PAGE_msg0);
  server.sendContent(cssColorString);
  server.sendContent_P(PAGE_msg1);
  server.sendContent(messageBody);
}

void serveSettings(uint8_t subPage)
{
  //0: menu 1: wifi 2: leds 3: ui 4: sync 5: time 6: sec 255: welcomepage
  if (!arlsTimeout) //do not serve while receiving realtime
    {
      int pl0, pl1;
      switch (subPage)
      {
        case 1: pl0 = strlen_P(PAGE_settings_wifi0); pl1 = strlen_P(PAGE_settings_wifi1); break;
        case 2: pl0 = strlen_P(PAGE_settings_leds0); pl1 = strlen_P(PAGE_settings_leds1); break;
        case 3: pl0 = strlen_P(PAGE_settings_ui0); pl1 = strlen_P(PAGE_settings_ui1); break;
        case 4: pl0 = strlen_P(PAGE_settings_sync0); pl1 = strlen_P(PAGE_settings_sync1); break;
        case 5: pl0 = strlen_P(PAGE_settings_time0); pl1 = strlen_P(PAGE_settings_time1); break;
        case 6: pl0 = strlen_P(PAGE_settings_sec0); pl1 = strlen_P(PAGE_settings_sec1); break;
        case 255: pl0 = strlen_P(PAGE_welcome0); pl1 = strlen_P(PAGE_welcome1); break;
        default: pl0 = strlen_P(PAGE_settings0); pl1 = strlen_P(PAGE_settings1);
      }
      
      String settingsBuffer = getSettings(subPage);
      int sCssLength = (subPage >0 && subPage <7)?strlen_P(PAGE_settingsCss):0;
      
      server.setContentLength(pl0 + cssColorString.length() + settingsBuffer.length() + sCssLength + pl1);
      server.send(200, "text/html", "");
      
      switch (subPage)
      {
        case 1: server.sendContent_P(PAGE_settings_wifi0); break;
        case 2: server.sendContent_P(PAGE_settings_leds0); break;
        case 3: server.sendContent_P(PAGE_settings_ui0); break;
        case 4: server.sendContent_P(PAGE_settings_sync0); break;
        case 5: server.sendContent_P(PAGE_settings_time0); break;
        case 6: server.sendContent_P(PAGE_settings_sec0); break;
        case 255: server.sendContent_P(PAGE_welcome0); break;
        default: server.sendContent_P(PAGE_settings0); 
      }
      server.sendContent(settingsBuffer);
      server.sendContent(cssColorString);
      if (subPage >0 && subPage <7) server.sendContent_P(PAGE_settingsCss);
      switch (subPage)
      {
        case 1: server.sendContent_P(PAGE_settings_wifi1); break;
        case 2: server.sendContent_P(PAGE_settings_leds1); break;
        case 3: server.sendContent_P(PAGE_settings_ui1); break;
        case 4: server.sendContent_P(PAGE_settings_sync1); break;
        case 5: server.sendContent_P(PAGE_settings_time1); break;
        case 6: server.sendContent_P(PAGE_settings_sec1); break;
        case 255: server.sendContent_P(PAGE_welcome1); break;
        default: server.sendContent_P(PAGE_settings1); 
      }
    } else {
        server.send(200, "text/plain", "The settings are not available while receiving real-time data.");
    }
}




