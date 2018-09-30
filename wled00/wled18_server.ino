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

  server.on("/generate_204", HTTP_GET, [](){
    serveIndex();
  });

  server.on("/fwlink", HTTP_GET, [](){
    serveIndex();
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

  server.on("/u", HTTP_GET, [](){
    server.setContentLength(strlen_P(PAGE_usermod));
    server.send(200, "text/html", "");
    server.sendContent_P(PAGE_usermod);
    });
    
  server.on("/teapot", HTTP_GET, [](){
    serveMessage(418, "418. I'm a teapot.","(Tangible Embedded Advanced Project Of Twinkling)",254);
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
    httpUpdater.setup(&server);
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

    //workaround for subpage issue
    if (server.uri().length() == 1)
    {
      serveIndexOrWelcome();
      return;
    }
    
    if(!handleSet(server.uri())){
      if(!handleAlexaApiCall(server.uri(),server.arg(0)))
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
    case 12: cs[0]="fbe8a6"; cs[1]="d2fdff"; cs[2]="b4dfe5"; cs[3]="f4976c"; cs[4]=""; cs[5]="303c6c"; break; //c64
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

void serveIndex()
{
  bool serveMobile = false;
  if (uiConfiguration == 0) serveMobile = checkClientIsMobile(server.header("User-Agent"));
  else if (uiConfiguration == 2) serveMobile = true;

  if (!realtimeActive || enableRealtimeUI) //do not serve while receiving realtime
  {
    if (serveMobile)
    {
      server.setContentLength(strlen_P(PAGE_indexM));
      server.send(200, "text/html", "");
      server.sendContent_P(PAGE_indexM);
    } else
    {
      server.setContentLength(strlen_P(PAGE_index0) + cssColorString.length() + strlen_P(PAGE_index1) + strlen_P(PAGE_index2) + strlen_P(PAGE_index3));
      server.send(200, "text/html", "");
      server.sendContent_P(PAGE_index0);
      server.sendContent(cssColorString); 
      server.sendContent_P(PAGE_index1); 
      server.sendContent_P(PAGE_index2);
      server.sendContent_P(PAGE_index3);
    }
  } else {
    serveRealtimeError(false);
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
  if (!realtimeActive || enableRealtimeUI) //do not serve while receiving realtime
    {
      #ifdef WLED_FLASH_512K_MODE //disable welcome page if not enough storage
      if (subPage == 255) {serveIndex(); return;}
      #endif
      
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
      
      getSettingsJS(subPage);
      int sCssLength = (subPage >0 && subPage <7)?strlen_P(PAGE_settingsCss):0;
      
      server.setContentLength(pl0 + cssColorString.length() + olen + sCssLength + pl1);
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
      server.sendContent(obuf);
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
        serveRealtimeError(true);
    }
}

