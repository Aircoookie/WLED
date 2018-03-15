/*
 * Setup code
 */

void wledInit()
{
  EEPROM.begin(EEPSIZE);
  if (!EEPROM.read(397)) strip.init(); //quick init
  
  Serial.begin(115200);
  
  #ifdef USEFS
  SPIFFS.begin();
  #endif
  
  DEBUG_PRINTLN("Load EEPROM");
  loadSettingsFromEEPROM(true);
  if (!initLedsLast) initStrip();
  DEBUG_PRINT("C-SSID: ");
  DEBUG_PRINT(clientSSID);
  buildCssColorString();
  userBeginPreConnection();

  WiFi.disconnect(); //close old connections

  if (staticIP[0] != 0)
  {
    WiFi.config(staticIP, staticGateway, staticSubnet, staticDNS);
  } else
  {
    WiFi.config(0U, 0U, 0U);
  }

  if (apSSID.length()>0)
  {
    DEBUG_PRINT("USING AP");
    DEBUG_PRINTLN(apSSID.length());
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

  if (hueIP[0] == 0)
  {
    hueIP[0] = WiFi.localIP()[0];
    hueIP[1] = WiFi.localIP()[1];
    hueIP[2] = WiFi.localIP()[2];
  }
  
  // Set up mDNS responder:
  if (cmDNS != NULL && !onlyAP && !MDNS.begin(cmDNS.c_str())) {
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

  //start captive portal
  if (onlyAP || apSSID.length() > 0)
  {
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, "*", WiFi.softAPIP());
    dnsActive = true;
  }

  //SERVER INIT
  //settings page
  server.on("/settings", HTTP_GET, [](){
    serveSettings(0);
  });
  server.on("/settings/wifi", HTTP_GET, [](){
    if (!(wifiLock && otaLock))
    {
      serveSettings(1);
    }else{
      serveMessage(500, "Access Denied", txd, 254);
    }
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
    if(!handleFileRead("/favicon.ico"))
    {
      server.send_P(200, "image/x-icon", favicon, 156);
    }
  });
  
  server.on("/", HTTP_GET, [](){
    serveIndexOrWelcome();
  });

  server.on("/generate_204", HTTP_GET, [](){
    serveIndexOrWelcome();
  });

  server.on("/fwlink", HTTP_GET, [](){
    serveIndexOrWelcome();
  });
  
  server.on("/sliders", HTTP_GET, serveIndex);
  
  server.on("/welcome", HTTP_GET, [](){
    serveSettings(255);
  });
  
  server.on("/reset", HTTP_GET, [](){
    serveMessage(200,"Rebooting now...","(takes ~20 seconds, wait for auto-redirect)",79);
    reset();
  });
  
  server.on("/settings/wifi", HTTP_POST, [](){
    if (!(wifiLock && otaLock)) handleSettingsSet(1);
    serveMessage(200,"WiFi settings saved.","Rebooting now... (takes ~20 seconds, wait for auto-redirect)",139);
    reset();
  });

  server.on("/settings/leds", HTTP_POST, [](){
    handleSettingsSet(2);
    serveMessage(200,"LED settings saved.","Redirecting...",1);
  });

  server.on("/settings/ui", HTTP_POST, [](){
    handleSettingsSet(3);
    serveMessage(200,"UI settings saved.","Reloading to apply theme...",122);
  });

  server.on("/settings/sync", HTTP_POST, [](){
    handleSettingsSet(4);
    if (hueAttempt)
    {
      serveMessage(200,"Hue setup result",hueError,253);
    } else {
      serveMessage(200,"Sync settings saved.","Redirecting...",1);
    }
    hueAttempt = false;
  });

  server.on("/settings/time", HTTP_POST, [](){
    handleSettingsSet(5);
    serveMessage(200,"Time settings saved.","Redirecting...",1);
  });

  server.on("/settings/sec", HTTP_POST, [](){
    handleSettingsSet(6);
    serveMessage(200,"Security settings saved.","Rebooting now... (takes ~20 seconds, wait for auto-redirect)",139);
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
    String val = (String)(int)strip.getPowerEstimate(ledCount,strip.getColor(),strip.getBrightness());
    val += "mA currently";
    serveMessage(200,val,"This is just an estimate (does not take into account several factors like effects and wire resistance). It is NOT an accurate measurement!",254);
    });
    
  server.on("/teapot", HTTP_GET, [](){
    serveMessage(418, "418. I'm a teapot.","(Tangible Embedded Advanced Project Of Twinkling)",254);
    });
    
  server.on("/build", HTTP_GET, [](){
    String info = "hard-coded build info:\r\n\n";
    #ifdef ARDUINO_ARCH_ESP32
    info += "platform: esp32\r\n";
    #else
    info += "platform: esp8266\r\n";
    #endif
    info += "version: " + versionString + "\r\n";
    info += "build: " + (String)VERSION + "\r\n";
    info += "eepver: " + String(EEPVER) + "\r\n";
    #ifdef RGBW
    info += "rgbw: true\r\n";
    #else
    info += "rgbw: false\r\n";
    #endif
    info += "max-leds: " + (String)LEDCOUNT + "\r\n";
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
    serveMessage(500, "Access Denied", txd, 254);
    });
    server.on("/down", HTTP_GET, [](){
    serveMessage(500, "Access Denied", txd, 254);
    });
    server.on("/cleareeprom", HTTP_GET, [](){
    serveMessage(500, "Access Denied", txd, 254);
    });
    server.on("/update", HTTP_GET, [](){
    serveMessage(500, "Access Denied", txd, 254);
    });
    server.on("/list", HTTP_GET, [](){
    serveMessage(500, "Access Denied", txd, 254);
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

  if (initLedsLast) initStrip();
  userBegin();
  if (macroBoot>0) applyMacro(macroBoot);
}

void initStrip()
{
  // Initialize NeoPixel Strip and button
  if (initLedsLast) strip.init();
  strip.setLedCount(ledCount);
  strip.setReverseMode(reverseMode);
  strip.setColor(0);
  strip.setBrightness(255);
  strip.start();

  pinMode(buttonPin, INPUT_PULLUP);

  if (bootPreset>0) applyPreset(bootPreset, turnOnAtBoot, true, true);
  colorUpdated(0);
  if(digitalRead(buttonPin) == LOW) buttonEnabled = false; //disable button if it is "pressed" unintentionally
}

void initAP(){
  String save = apSSID;
  if (apSSID.length() <1) apSSID = "WLED-AP";
  WiFi.softAP(apSSID.c_str(), apPass.c_str(), apChannel, apHide);
  apSSID = save;
}

void initCon()
{
  int fail_count = 0;
  if (clientSSID.length() <1 || clientSSID.equals("Your_Network")) fail_count = apWaitTimeSecs*2;
  WiFi.begin(clientSSID.c_str(), clientPass.c_str());
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
      if (con) DEBUG_PRINTLN("rofl");
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

void buildCssColorString()
{
  String cs[]={"","","","","",""};
  switch (currentTheme)
  {
    default: cs[0]="D9B310"; cs[1]="0B3C5D"; cs[2]="1D2731"; cs[3]="328CC1"; cs[4]="000"; cs[5]="328CC1"; break; //night
    case 1: cs[0]="eee"; cs[1]="ddd"; cs[2]="b9b9b9"; cs[3]="049"; cs[4]="777"; cs[5]="049"; break; //modern
    case 2: cs[0]="abc"; cs[1]="fff"; cs[2]="ddd"; cs[3]="000"; cs[4]="0004"; cs[5]="000"; break; //bright
    case 3: cs[0]="c09f80"; cs[1]="d7cec7"; cs[2]="76323f"; cs[3]="888"; cs[4]="3334"; cs[5]="888"; break; //wine
    case 4: cs[0]="3cc47c"; cs[1]="828081"; cs[2]="d9a803"; cs[3]="1e392a"; cs[4]="000a"; cs[5]="1e392a"; break; //electric
    case 5: cs[0]="57bc90"; cs[1]="a5a5af"; cs[2]="015249"; cs[3]="88c9d4"; cs[4]="0004"; cs[5]="88c9d4"; break; //mint
    case 6: cs[0]="f7c331"; cs[1]="dcc7aa"; cs[2]="6b7a8f"; cs[3]="f7882f"; cs[4]="0007"; cs[5]="f7882f"; break; //amber
    case 7: cs[0]="fc3"; cs[1]="124"; cs[2]="334"; cs[3]="f1d"; cs[4]="f00"; cs[5]="f1d"; break;//club
    case 8: cs[0]="0ac"; cs[1]="124"; cs[2]="224"; cs[3]="003eff"; cs[4]="003eff"; cs[5]="003eff"; break;//air
    case 9: cs[0]="f70"; cs[1]="421"; cs[2]="221"; cs[3]="a50"; cs[4]="f70"; cs[5]="f70"; break;//nixie
    case 10: cs[0]="2d2"; cs[1]="010"; cs[2]="121"; cs[3]="060"; cs[4]="040"; cs[5]="3f3"; break; //terminal
    case 11: cs[0]="867ADE"; cs[1]="4033A3"; cs[2]="483AAA"; cs[3]="483AAA"; cs[4]=""; cs[5]="867ADE"; break; //c64
    case 14: cs[0]="fc7"; cs[1]="49274a"; cs[2]="94618e"; cs[3]="f4decb"; cs[4]="0008"; cs[5]="f4decb"; break; //end
    case 15: for (int i=0;i<6;i++)cs[i]=cssCol[i];//custom
  }
  cssColorString="<style>:root{--aCol:#";
  cssColorString+=cs[0];
  cssColorString+=";--bCol:#";
  cssColorString+=cs[1];
  cssColorString+=";--cCol:#";
  cssColorString+=cs[2];
  cssColorString+=";--dCol:#";
  cssColorString+=cs[3];
  cssColorString+=";--sCol:#";
  cssColorString+=cs[4];
  cssColorString+=";--tCol:#";
  cssColorString+=cs[5];
  cssColorString+=";--cFn:";
  cssColorString+=cssFont;
  cssColorString+=";}";
}

void serveIndexOrWelcome()
{
  if (!showWelcomePage){
    if(!handleFileRead("/index.htm")) {
      serveIndex();
    }
  }else{
    if(!handleFileRead("/welcome.htm")) {
      serveSettings(255);
    }
  }
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

void serveMessage(int code, String headl, String subl="", int optionType)
{
  String messageBody = "<h2>";
  messageBody += headl;
  messageBody += "</h2>";
  messageBody += subl;
  switch(optionType)
  {
    case 255: break; //simple message
    case 254: messageBody += "<br><br><button type=\"button\" onclick=\"B()\">Back</button>"; break; //back button
    case 253: messageBody += "<br><br><form action=/settings><button type=submit>Back</button></form>"; //button to settings
  }
  if (optionType < 60) //redirect to settings after optionType seconds
  {
    messageBody += "<script>setTimeout(RS," + String(optionType*1000) + ")</script>";
  } else if (optionType < 120) //redirect back after optionType-60 seconds
  {
    messageBody += "<script>setTimeout(B," + String((optionType-60)*1000) + ")</script>";
  } else if (optionType < 180) //reload parent after optionType-120 seconds
  {
    messageBody += "<script>setTimeout(RP," + String((optionType-120)*1000) + ")</script>";
  }
  messageBody += "</body></html>";
  server.setContentLength(strlen_P(PAGE_msg0) + cssColorString.length() + strlen_P(PAGE_msg1) + messageBody.length());
  server.send(code, "text/html", "");
  server.sendContent_P(PAGE_msg0);
  server.sendContent(cssColorString);
  server.sendContent_P(PAGE_msg1);
  server.sendContent(messageBody);
}

void serveSettings(byte subPage)
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




