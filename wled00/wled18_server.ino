/*
 * Server page definitions
 */

void initServer()
{
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
    serveMessage(200,"WiFi settings saved.","Rebooting now...",255);
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

  server.on("/json", HTTP_ANY, [](){
    server.send(500, "application/json", "{\"error\":\"Not implemented\"}");
    });

  server.on("/json/effects", HTTP_GET, [](){
    server.setContentLength(strlen_P(JSON_mode_names));
    server.send(200, "application/json", "");
    server.sendContent_P(JSON_mode_names);
    });

  server.on("/json/palettes", HTTP_GET, [](){
    server.setContentLength(strlen_P(JSON_palette_names));
    server.send(200, "application/json", "");
    server.sendContent_P(JSON_palette_names);
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
    String val = "";
    if (strip.currentMilliamps == 0)
    {
      val = "Power calculation disabled";
    } else
    {
      val += (String)strip.currentMilliamps;
      val += "mA currently";
    }
    serveMessage(200, val, "This is just an estimate (does not account for factors like wire resistance). It is NOT a measurement!", 254);
    });

  server.on("/u", HTTP_GET, [](){
    server.setContentLength(strlen_P(PAGE_usermod));
    server.send(200, "text/html", "");
    server.sendContent_P(PAGE_usermod);
    });
    
  server.on("/teapot", HTTP_GET, [](){
    serveMessage(418, "418. I'm a teapot.", "(Tangible Embedded Advanced Project Of Twinkling)", 254);
    });
    
  server.on("/build", HTTP_GET, [](){
    getBuildInfo();
    server.send(200, "text/plain", obuf);
    });
  //if OTA is allowed
  if (!otaLock){
    server.on("/edit", HTTP_GET, [](){
    server.send(200, "text/html", PAGE_edit);
    });
    #ifdef USEFS
    server.on("/edit", HTTP_PUT, handleFileCreate);
    server.on("/edit", HTTP_DELETE, handleFileDelete);
    server.on("/edit", HTTP_POST, [](){ server.send(200, "text/plain", ""); }, handleFileUpload);
    server.on("/list", HTTP_GET, handleFileList);
    #endif
    //init ota page
    #ifndef WLED_DISABLE_OTA
    httpUpdater.setup(&server);
    #else
    server.on("/update", HTTP_GET, [](){
    serveMessage(500, "Not implemented", "OTA updates are unsupported in this build.", 254);
    });
    #endif
  } else
  {
    server.on("/edit", HTTP_GET, [](){
    serveMessage(500, "Access Denied", txd, 254);
    });
    server.on("/update", HTTP_GET, [](){
    serveMessage(500, "Access Denied", txd, 254);
    });
    server.on("/list", HTTP_GET, [](){
    serveMessage(500, "Access Denied", txd, 254);
    });
  }

  //this ceased working somehow
  /*server.on("/", HTTP_GET, [](){
    serveIndexOrWelcome();
  });*/
  
  //called when the url is not defined here, ajax-in; get-settings
  server.onNotFound([](){
    DEBUG_PRINTLN("Not-Found HTTP call:");
    DEBUG_PRINTLN("URI: " + server.uri());
    DEBUG_PRINTLN("Body: " + server.arg(0));

    //make API CORS compatible
    if (server.method() == HTTP_OPTIONS)
    {
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.sendHeader("Access-Control-Max-Age", "10000");
      server.sendHeader("Access-Control-Allow-Methods", "PUT,POST,GET,OPTIONS");
      server.sendHeader("Access-Control-Allow-Headers", "*");
      server.send(200);
      return;
    }

    //workaround for subpage issue
    if (server.uri().length() == 1)
    {
      serveIndexOrWelcome();
      return;
    }
    
    if(!handleSet(server.uri())){
      #ifndef WLED_DISABLE_ALEXA
      if(!espalexa.handleAlexaApiCall(server.uri(),server.arg(0)))
      #endif
      server.send(404, "text/plain", "Not Found");
    }
  });
  
  #ifndef ARDUINO_ARCH_ESP32
  const char * headerkeys[] = {"User-Agent"};
  server.collectHeaders(headerkeys,sizeof(headerkeys)/sizeof(char*));
  #else
  String ua = "User-Agent";
  server.collectHeaders(ua);
  #endif
}


void serveIndexOrWelcome()
{
  if (!showWelcomePage){
    serveIndex();
  }else{
    serveSettings(255);
  }
}

void serveRealtimeError(bool settings)
{
  String mesg = "The ";
  mesg += (settings)?"settings":"WLED";
  mesg += " UI is not available while receiving real-time data (";
  if (realtimeIP[0] == 0)
  {
    mesg += "E1.31";
  } else {
    mesg += "UDP from ";
    mesg += realtimeIP[0];
    for (int i = 1; i < 4; i++)
    {
      mesg += ".";
      mesg += realtimeIP[i];
    }
  }
  mesg += ").";
  server.send(200, "text/plain", mesg);
}


void getCSSColors()
{
  char cs[6][9];
  getThemeColors(cs);
  oappend("<style>:root{--aCol:#"); oappend(cs[0]);
  oappend(";--bCol:#");             oappend(cs[1]);
  oappend(";--cCol:#");             oappend(cs[2]);
  oappend(";--dCol:#");             oappend(cs[3]);
  oappend(";--sCol:#");             oappend(cs[4]);
  oappend(";--tCol:#");             oappend(cs[5]);
  oappend(";--cFn:");               oappend(cssFont);
  oappend(";}");
}


void serveIndex()
{
  bool serveMobile = false;
  if (uiConfiguration == 0) serveMobile = checkClientIsMobile(server.header("User-Agent"));
  else if (uiConfiguration == 2) serveMobile = true;

  if (realtimeActive && !enableRealtimeUI) //do not serve while receiving realtime
  {
    serveRealtimeError(false);
    return;
  }

  //error message is not gzipped
  #ifdef WLED_DISABLE_MOBILE_UI
  if (!serveMobile) server.sendHeader("Content-Encoding","gzip");
  #else
  server.sendHeader("Content-Encoding","gzip");
  #endif
  
  server.send_P(200, "text/html",
                (serveMobile) ? PAGE_indexM   : PAGE_index0,
                (serveMobile) ? PAGE_indexM_L : PAGE_index0_L);
}


void serveMessage(int code, String headl, String subl="", int optionType)
{
  olen = 0;
  getCSSColors();
  
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
  server.setContentLength(strlen_P(PAGE_msg0) + olen + strlen_P(PAGE_msg1) + messageBody.length());
  server.send(code, "text/html", "");
  server.sendContent_P(PAGE_msg0);
  server.sendContent(obuf);
  server.sendContent_P(PAGE_msg1);
  server.sendContent(messageBody);
}


void serveSettings(byte subPage)
{
  if (realtimeActive && !enableRealtimeUI) //do not serve while receiving realtime
  {
    serveRealtimeError(true);
    return;
  }
  
  #ifdef WLED_DISABLE_MOBILE_UI //disable welcome page if not enough storage
   if (subPage == 255) {serveIndex(); return;}
  #endif
  
  int pl0, pl1;
  switch (subPage) //0: menu 1: wifi 2: leds 3: ui 4: sync 5: time 6: sec 255: welcomepage
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

  uint16_t sCssLength = (subPage >0 && subPage <7)?strlen_P(PAGE_settingsCss):0;
  
  getSettingsJS(subPage);

  getCSSColors();
  
  server.setContentLength(pl0 + olen + sCssLength + pl1);
  server.send(200, "text/html", "");
  
  switch (subPage)
  {
    case 1:   server.sendContent_P(PAGE_settings_wifi0); break;
    case 2:   server.sendContent_P(PAGE_settings_leds0); break;
    case 3:   server.sendContent_P(PAGE_settings_ui0  ); break;
    case 4:   server.sendContent_P(PAGE_settings_sync0); break;
    case 5:   server.sendContent_P(PAGE_settings_time0); break;
    case 6:   server.sendContent_P(PAGE_settings_sec0 ); break;
    case 255: server.sendContent_P(PAGE_welcome0      ); break;
    default:  server.sendContent_P(PAGE_settings0     ); 
  }
  server.sendContent(obuf);

  if (subPage >0 && subPage <7) server.sendContent_P(PAGE_settingsCss);
  switch (subPage)
  {
    case 1:   server.sendContent_P(PAGE_settings_wifi1); break;
    case 2:   server.sendContent_P(PAGE_settings_leds1); break;
    case 3:   server.sendContent_P(PAGE_settings_ui1  ); break;
    case 4:   server.sendContent_P(PAGE_settings_sync1); break;
    case 5:   server.sendContent_P(PAGE_settings_time1); break;
    case 6:   server.sendContent_P(PAGE_settings_sec1 ); break;
    case 255: server.sendContent_P(PAGE_welcome1      ); break;
    default:  server.sendContent_P(PAGE_settings1     ); 
  }
}
