/*
 * Server page definitions
 */

void initServer()
{
  //CORS compatiblity
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  
  //settings page
  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){
    serveSettings(request);
  });
  
  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!handleFileRead("/favicon.ico"))
    {
      request->send_P(200, "image/x-icon", favicon, 156);
    }
  });
  
  server.on("/sliders", HTTP_GET, [](AsyncWebServerRequest *request){
    serveIndex(request);
  });
  
  server.on("/welcome", HTTP_GET, [](AsyncWebServerRequest *request){
    serveSettings(request);
  });
  
  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
    serveMessage(request, 200,"Rebooting now...","(takes ~20 seconds, wait for auto-redirect)",79);
    reset();
  });
  
  server.on("/settings/wifi", HTTP_POST, [](AsyncWebServerRequest *request){
    if (!(wifiLock && otaLock)) handleSettingsSet(request, 1);
    serveMessage(request, 200,"WiFi settings saved.","Rebooting now...",255);
    reset();
  });

  server.on("/settings/leds", HTTP_POST, [](AsyncWebServerRequest *request){
    handleSettingsSet(request, 2);
    serveMessage(request, 200,"LED settings saved.","Redirecting...",1);
  });

  server.on("/settings/ui", HTTP_POST, [](AsyncWebServerRequest *request){
    handleSettingsSet(request, 3);
    serveMessage(request, 200,"UI settings saved.","Reloading to apply theme...",122);
  });

  server.on("/settings/sync", HTTP_POST, [](AsyncWebServerRequest *request){
    handleSettingsSet(request, 4);
    if (hueAttempt)
    {
      serveMessage(request, 200,"Hue setup result",hueError,253);
    } else {
      serveMessage(request, 200,"Sync settings saved.","Redirecting...",1);
    }
    hueAttempt = false;
  });

  server.on("/settings/time", HTTP_POST, [](AsyncWebServerRequest *request){
    handleSettingsSet(request, 5);
    serveMessage(request, 200,"Time settings saved.","Redirecting...",1);
  });

  server.on("/settings/sec", HTTP_POST, [](AsyncWebServerRequest *request){
    handleSettingsSet(request, 6);
    serveMessage(request, 200,"Security settings saved.","Rebooting now... (takes ~20 seconds, wait for auto-redirect)",139);
    reset();
  });

  /*server.on("/json", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(500, "application/json", "{\"error\":\"Not implemented\"}");
    });*/

  server.on("/json/effects", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "application/json", JSON_mode_names);
    });

  server.on("/json/palettes", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "application/json", JSON_palette_names);
    });
  
  server.on("/version", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", (String)VERSION);
    });
    
  server.on("/uptime", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", (String)millis());
    });
    
  server.on("/freeheap", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", (String)ESP.getFreeHeap());
    });
    
  server.on("/power", HTTP_GET, [](AsyncWebServerRequest *request){
    String val = "";
    if (strip.currentMilliamps == 0)
    {
      val = "Power calculation disabled";
    } else
    {
      val += (String)strip.currentMilliamps;
      val += "mA currently";
    }
    serveMessage(request, 200, val, "This is just an estimate (does not account for factors like wire resistance). It is NOT a measurement!", 254);
    });

  server.on("/u", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", PAGE_usermod);
    });
    
  server.on("/teapot", HTTP_GET, [](AsyncWebServerRequest *request){
    serveMessage(request, 418, "418. I'm a teapot.", "(Tangible Embedded Advanced Project Of Twinkling)", 254);
    });
    
  server.on("/build", HTTP_GET, [](AsyncWebServerRequest *request){
    getBuildInfo();
    request->send(200, "text/plain", obuf);
    });
    
  //if OTA is allowed
  if (!otaLock){
    server.on("/edit", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "text/html", PAGE_edit);
    });
    #ifdef USEFS
    server.on("/edit", HTTP_PUT, handleFileCreate);
    server.on("/edit", HTTP_DELETE, handleFileDelete);
    server.on("/edit", HTTP_POST, [](){ server->send(200, "text/plain", ""); }, handleFileUpload);
    server.on("/list", HTTP_GET, handleFileList);
    #endif
    //init ota page
    #ifndef WLED_DISABLE_OTA
    //httpUpdater.setup(&server);
    #else
    server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    serveMessage(request, 500, "Not implemented", "OTA updates are unsupported in this build.", 254);
    });
    #endif
  } else
  {
    server.on("/edit", HTTP_GET, [](AsyncWebServerRequest *request){
    serveMessage(request, 500, "Access Denied", txd, 254);
    });
    server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    serveMessage(request, 500, "Access Denied", txd, 254);
    });
    server.on("/list", HTTP_GET, [](AsyncWebServerRequest *request){
    serveMessage(request, 500, "Access Denied", txd, 254);
    });
  }

  //this ceased working somehow
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    serveIndexOrWelcome(request);
  });
  
  //called when the url is not defined here, ajax-in; get-settings
  server.onNotFound([](AsyncWebServerRequest *request){
    DEBUG_PRINTLN("Not-Found HTTP call:");
    DEBUG_PRINTLN("URI: " + server->uri());
    DEBUG_PRINTLN("Body: " + server->arg(0));

    //make API CORS compatible
    if (request->method() == HTTP_OPTIONS)
    {
      request->send(200); return;
    }

    //workaround for subpage issue
    /*if (request->url().length() == 1)
    {
      serveIndexOrWelcome(request);
      return;
    }*/
    
    if(!handleSet(request, request->url())){
      #ifndef WLED_DISABLE_ALEXA
      if(!espalexa.handleAlexaApiCall(request))
      #endif
      request->send(404, "text/plain", "Not Found");
    }
  });
}


void serveIndexOrWelcome(AsyncWebServerRequest *request)
{
  if (!showWelcomePage){
    serveIndex(request);
  } else {
    serveSettings(request);
  }
}

void serveRealtimeError(AsyncWebServerRequest *request, bool settings)
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
  request->send(200, "text/plain", mesg);
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


void serveIndex(AsyncWebServerRequest* request)
{
  if (realtimeActive && !enableRealtimeUI) //do not serve while receiving realtime
  {
    serveRealtimeError(request, false);
    return;
  }
  
  bool serveMobile = false;
  if (uiConfiguration == 0 && request->hasHeader("User-Agent")) serveMobile = checkClientIsMobile(request->getHeader("User-Agent")->value());
  else if (uiConfiguration == 2) serveMobile = true;

  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", 
                                      (serveMobile) ? PAGE_indexM : PAGE_index,
                                      (serveMobile) ? PAGE_indexM_L : PAGE_index_L);

  //error message is not gzipped
  #ifdef WLED_DISABLE_MOBILE_UI
  if (!serveMobile) response->addHeader("Content-Encoding","gzip");
  #else
  response->addHeader("Content-Encoding","gzip");
  #endif
  
  request->send(response);
}


String msgProcessor(const String& var)
{
  if (var == "CSS") return String(obuf);
  if (var == "MSG") {
    String messageBody = "";
    messageBody += messageHead;
    messageBody += "</h2>";
    messageBody += messageSub;

    if (optionType < 60) //redirect to settings after optionType seconds
    {
      messageBody += "<script>setTimeout(RS," + String(optionType*1000) + ")</script>";
    } else if (optionType < 120) //redirect back after optionType-60 seconds
    {
      messageBody += "<script>setTimeout(B," + String((optionType-60)*1000) + ")</script>";
    } else if (optionType < 180) //reload parent after optionType-120 seconds
    {
      messageBody += "<script>setTimeout(RP," + String((optionType-120)*1000) + ")</script>";
    } else if (optionType == 253)
    {
      messageBody += "<br><br><form action=/settings><button type=submit>Back</button></form>"; //button to settings
    } else if (optionType == 254)
    {
      messageBody += "<br><br><button type=\"button\" onclick=\"B()\">Back</button>";
    }
    return messageBody;
  }
  return String();
}


void serveMessage(AsyncWebServerRequest* request, uint16_t code, String headl, String subl="", uint32_t optionT=255)
{
  olen = 0;
  getCSSColors();
  messageHead = headl;
  messageSub = subl;
  optionType = optionT;
  
  request->send_P(code, "text/html", PAGE_msg, msgProcessor);
}


String settingsProcessor(const String& var)
{
  if (var == "CSS") return String(obuf);
  if (var == "SCSS") return String(PAGE_settingsCss);
  return String();
}


void serveSettings(AsyncWebServerRequest* request)
{
  byte subPage = 0;
  String url = request->url();
  if (url.indexOf("sett") >= 0) 
  {
    if      (url.indexOf("wifi") > 0) subPage = 1;
    else if (url.indexOf("leds") > 0) subPage = 2;
    else if (url.indexOf("ui")   > 0) subPage = 3;
    else if (url.indexOf("sync") > 0) subPage = 4;
    else if (url.indexOf("time") > 0) subPage = 5;
    else if (url.indexOf("sec")  > 0) subPage = 6;
  } else subPage = 255; //welcome page
  
  if (realtimeActive && !enableRealtimeUI) //do not serve while receiving realtime
  {
    serveRealtimeError(request, true);
    return;
  }

  if (subPage == 1 && wifiLock && otaLock)
  {
    serveMessage(request, 500, "Access Denied", txd, 254); return;
  }
  
  #ifdef WLED_DISABLE_MOBILE_UI //disable welcome page if not enough storage
   if (subPage == 255) {serveIndex(request); return;}
  #endif
  
  getSettingsJS(subPage);

  getCSSColors();
  
  switch (subPage)
  {
    case 1:   request->send_P(200, "text/html", PAGE_settings_wifi, settingsProcessor); break;
    case 2:   request->send_P(200, "text/html", PAGE_settings_leds, settingsProcessor); break;
    case 3:   request->send_P(200, "text/html", PAGE_settings_ui  , settingsProcessor); break;
    case 4:   request->send_P(200, "text/html", PAGE_settings_sync, settingsProcessor); break;
    case 5:   request->send_P(200, "text/html", PAGE_settings_time, settingsProcessor); break;
    case 6:   request->send_P(200, "text/html", PAGE_settings_sec , settingsProcessor); break;
    case 255: request->send_P(200, "text/html", PAGE_welcome      , settingsProcessor); break;
    default:  request->send_P(200, "text/html", PAGE_settings     , settingsProcessor); 
  }
}
