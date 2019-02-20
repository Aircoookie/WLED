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
    serveMessage(request, 200,"Rebooting now...","Please wait ~10 seconds...",129);
    doReboot = true;
  });
  
  server.on("/settings/wifi", HTTP_POST, [](AsyncWebServerRequest *request){
    if (!(wifiLock && otaLock)) handleSettingsSet(request, 1);
    serveMessage(request, 200,"WiFi settings saved.","Rebooting now...",255);
    doReboot = true;
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
    serveMessage(request, 200,"Sync settings saved.","Redirecting...",1);
  });

  server.on("/settings/time", HTTP_POST, [](AsyncWebServerRequest *request){
    handleSettingsSet(request, 5);
    serveMessage(request, 200,"Time settings saved.","Redirecting...",1);
  });

  server.on("/settings/sec", HTTP_POST, [](AsyncWebServerRequest *request){
    handleSettingsSet(request, 6);
    serveMessage(request, 200,"Security settings saved.","Rebooting now, please wait ~10 seconds...",129);
    doReboot = true;
  });

  server.on("/json/effects", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "application/json", JSON_mode_names);
    });

  server.on("/json/palettes", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "application/json", JSON_palette_names);
    });

  server.on("/json/info", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(500, "application/json", "{\"error\":\"Not implemented\"}");
    });

  server.on("/json", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(500, "application/json", "{\"error\":\"Not implemented\"}");
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
    server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
      serveMessage(request, 200, "WLED Software Update", "Installed version: " + String(versionString) + "<br>Download the latest binary: "
                                                         "<a href=\"https://github.com/Aircoookie/WLED/releases\"><img src=\"https://img.shields.io/github/release/Aircoookie/WLED.svg?style=flat-square\"></a>"
                                                         "<br><form method='POST' action='/update' enctype='multipart/form-data'>"
                                                         "<input type='file' class=\"bt\" name='update' required><br><input type='submit' class=\"bt\" value='Update!'></form>", 254);
    });
    
    server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
      if (Update.hasError())
      {
        serveMessage(request, 500, "Failed updating firmware!", "Please check your file and retry!", 254); return;
      }
      serveMessage(request, 200, "Successfully updated firmware!", "Please wait while the module reboots...", 131); 
      doReboot = true;
    },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
      if(!index){
        DEBUG_PRINTLN("OTA Update Start");
        #ifndef ARDUINO_ARCH_ESP32
        Update.runAsync(true);
        #endif
        Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000);
      }
      if(!Update.hasError()) Update.write(data, len);
      if(final){
        if(Update.end(true)){
          DEBUG_PRINTLN("Update Success");
        } else {
          DEBUG_PRINTLN("Update Failed");
        }
      }
    });
    
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

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    serveIndexOrWelcome(request);
  });
  
  //called when the url is not defined here, ajax-in; get-settings
  server.onNotFound([](AsyncWebServerRequest *request){
    DEBUG_PRINTLN("Not-Found HTTP call:");
    DEBUG_PRINTLN("URI: " + request->url());

    //make API CORS compatible
    if (request->method() == HTTP_OPTIONS)
    {
      request->send(200); return;
    }
    
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
    uint32_t optt = optionType;

    if (optt < 60) //redirect to settings after optionType seconds
    {
      messageBody += "<script>setTimeout(RS," + String(optt*1000) + ")</script>";
    } else if (optt < 120) //redirect back after optionType-60 seconds, unused
    {
      //messageBody += "<script>setTimeout(B," + String((optt-60)*1000) + ")</script>";
    } else if (optt < 180) //reload parent after optionType-120 seconds
    {
      messageBody += "<script>setTimeout(RP," + String((optt-120)*1000) + ")</script>";
    } else if (optt == 253)
    {
      messageBody += "<br><br><form action=/settings><button class=\"bt\" type=submit>Back</button></form>"; //button to settings
    } else if (optt == 254)
    {
      messageBody += "<br><br><button type=\"button\" class=\"bt\" onclick=\"B()\">Back</button>";
    }
    return messageBody;
  }
  return String();
}


void serveMessage(AsyncWebServerRequest* request, uint16_t code, String headl, String subl="", byte optionT=255)
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
