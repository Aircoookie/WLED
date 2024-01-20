#include "wled.h"

#include "html_ui.h"
#include "html_settings.h"
#include "html_other.h"
#ifdef WLED_ENABLE_PIXART
  #include "html_pixart.h"
#endif
#ifndef WLED_DISABLE_PXMAGIC
  #include "html_pxmagic.h"
#endif
#include "html_cpal.h"

/*
 * Integrated HTTP web server page declarations
 */

bool handleIfNoneMatchCacheHeader(AsyncWebServerRequest* request, int code, uint16_t eTagSuffix = 0);
void setStaticContentCacheHeaders(AsyncWebServerResponse *response, int code, uint16_t eTagSuffix = 0);
void handleStaticContent(AsyncWebServerRequest *request, const String &path, int code, const String &contentType, const uint8_t *content, size_t len, bool gzip = true, uint16_t eTagSuffix = 0);

// define flash strings once (saves flash memory)
static const char s_redirecting[] PROGMEM = "Redirecting...";
static const char s_content_enc[] PROGMEM = "Content-Encoding";
static const char s_unlock_ota [] PROGMEM = "Please unlock OTA in security settings!";
static const char s_unlock_cfg [] PROGMEM = "Please unlock settings using PIN code!";

//Is this an IP?
bool isIp(String str) {
  for (size_t i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

void handleUpload(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!correctPIN) {
    if (final) request->send(401, "text/plain", FPSTR(s_unlock_cfg));
    return;
  }
  if (!index) {
    String finalname = filename;
    if (finalname.charAt(0) != '/') {
      finalname = '/' + finalname; // prepend slash if missing
    }

    request->_tempFile = WLED_FS.open(finalname, "w");
    DEBUG_PRINT(F("Uploading "));
    DEBUG_PRINTLN(finalname);
    if (finalname.equals("/presets.json")) presetsModifiedTime = toki.second();
  }
  if (len) {
    request->_tempFile.write(data,len);
  }
  if (final) {
    request->_tempFile.close();
    if (filename.indexOf(F("cfg.json")) >= 0) { // check for filename with or without slash
      doReboot = true;
      request->send(200, "text/plain", F("Configuration restore successful.\nRebooting..."));
    } else {
      if (filename.indexOf(F("palette")) >= 0 && filename.indexOf(F(".json")) >= 0) strip.loadCustomPalettes();
      request->send(200, "text/plain", F("File Uploaded!"));
    }
    cacheInvalidate++;
  }
}

void createEditHandler(bool enable) {
  if (editHandler != nullptr) server.removeHandler(editHandler);
  if (enable) {
    #ifdef WLED_ENABLE_FS_EDITOR
      #ifdef ARDUINO_ARCH_ESP32
      editHandler = &server.addHandler(new SPIFFSEditor(WLED_FS));//http_username,http_password));
      #else
      editHandler = &server.addHandler(new SPIFFSEditor("","",WLED_FS));//http_username,http_password));
      #endif
    #else
      editHandler = &server.on("/edit", HTTP_GET, [](AsyncWebServerRequest *request){
        serveMessage(request, 501, "Not implemented", F("The FS editor is disabled in this build."), 254);
      });
    #endif
  } else {
    editHandler = &server.on("/edit", HTTP_ANY, [](AsyncWebServerRequest *request){
      serveMessage(request, 401, "Access Denied", FPSTR(s_unlock_cfg), 254);
    });
  }
}

bool captivePortal(AsyncWebServerRequest *request)
{
  if (ON_STA_FILTER(request)) return false; //only serve captive in AP mode
  String hostH;
  if (!request->hasHeader("Host")) return false;
  hostH = request->getHeader("Host")->value();

  if (!isIp(hostH) && hostH.indexOf("wled.me") < 0 && hostH.indexOf(cmDNS) < 0) {
    DEBUG_PRINTLN("Captive portal");
    AsyncWebServerResponse *response = request->beginResponse(302);
    response->addHeader(F("Location"), F("http://4.3.2.1"));
    request->send(response);
    return true;
  }
  return false;
}

void initServer()
{
  //CORS compatiblity
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Origin"), "*");
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Methods"), "*");
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Headers"), "*");

#ifdef WLED_ENABLE_WEBSOCKETS
  #ifndef WLED_DISABLE_2D 
  server.on("/liveview2D", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleStaticContent(request, "", 200, "text/html", PAGE_liveviewws2D, PAGE_liveviewws2D_length);
  });
  #endif
#endif
  server.on("/liveview", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleStaticContent(request, "", 200, "text/html", PAGE_liveview, PAGE_liveview_length);
  });

  //settings page
  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){
    serveSettings(request);
  });

  // "/settings/settings.js&p=x" request also handled by serveSettings()

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleStaticContent(request, "/style.css", 200, "text/css", PAGE_settingsCss, PAGE_settingsCss_length);
  });

  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleStaticContent(request, "/favicon.ico", 200, "image/x-icon", favicon, favicon_length, false);
  });

  server.on("/skin.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (handleFileRead(request, "/skin.css")) return;
    AsyncWebServerResponse *response = request->beginResponse(200, "text/css");
    request->send(response);
  });

  server.on("/welcome", HTTP_GET, [](AsyncWebServerRequest *request){
    serveSettings(request);
  });

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
    serveMessage(request, 200,F("Rebooting now..."),F("Please wait ~10 seconds..."),129);
    doReboot = true;
  });

  server.on("/settings", HTTP_POST, [](AsyncWebServerRequest *request){
    serveSettings(request, true);
  });

  server.on("/json", HTTP_GET, [](AsyncWebServerRequest *request){
    serveJson(request);
  });

  AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/json", [](AsyncWebServerRequest *request) {
    bool verboseResponse = false;
    bool isConfig = false;

    if (!requestJSONBufferLock(14)) return;

    DeserializationError error = deserializeJson(*pDoc, (uint8_t*)(request->_tempObject));
    JsonObject root = pDoc->as<JsonObject>();
    if (error || root.isNull()) {
      releaseJSONBufferLock();
      serveJsonError(request, 400, ERR_JSON);
      return;
    }
    if (root.containsKey("pin")) checkSettingsPIN(root["pin"].as<const char*>());

    const String& url = request->url();
    isConfig = url.indexOf("cfg") > -1;
    if (!isConfig) {
      /*
      #ifdef WLED_DEBUG
        DEBUG_PRINTLN(F("Serialized HTTP"));
        serializeJson(root,Serial);
        DEBUG_PRINTLN();
      #endif
      */
      verboseResponse = deserializeState(root);
    } else {
      if (!correctPIN && strlen(settingsPIN)>0) {
        releaseJSONBufferLock();
        serveJsonError(request, 401, ERR_DENIED);
        return;
      }
      verboseResponse = deserializeConfig(root); //use verboseResponse to determine whether cfg change should be saved immediately
    }
    releaseJSONBufferLock();

    if (verboseResponse) {
      if (!isConfig) {
        lastInterfaceUpdate = millis(); // prevent WS update until cooldown
        interfaceUpdateCallMode = CALL_MODE_WS_SEND; // schedule WS update
        serveJson(request); return; //if JSON contains "v"
      } else {
        doSerializeConfig = true; //serializeConfig(); //Save new settings to FS
      }
    }
    request->send(200, "application/json", F("{\"success\":true}"));
  }, JSON_BUFFER_SIZE);
  server.addHandler(handler);

  server.on("/version", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", (String)VERSION);
  });

  server.on("/uptime", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", (String)millis());
  });

  server.on("/freeheap", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", (String)ESP.getFreeHeap());
  });

#ifdef WLED_ENABLE_USERMOD_PAGE
  server.on("/u", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleStaticContent(request, "", 200, "text/html", PAGE_usermod, PAGE_usermod_length);
  });
#endif

  server.on("/teapot", HTTP_GET, [](AsyncWebServerRequest *request){
    serveMessage(request, 418, F("418. I'm a teapot."), F("(Tangible Embedded Advanced Project Of Twinkling)"), 254);
  });

  server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {},
        [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data,
                      size_t len, bool final) {handleUpload(request, filename, index, data, len, final);}
  );

  createEditHandler(correctPIN);

#ifndef WLED_DISABLE_OTA
  //init ota page
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    if (otaLock) {
      serveMessage(request, 401, "Access Denied", FPSTR(s_unlock_ota), 254);
    } else
      serveSettings(request); // checks for "upd" in URL and handles PIN
  });

  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
    if (!correctPIN) {
      serveSettings(request, true); // handle PIN page POST request
      return;
    }
    if (otaLock) {
      serveMessage(request, 401, "Access Denied", FPSTR(s_unlock_ota), 254);
      return;
    }
    if (Update.hasError()) {
      serveMessage(request, 500, F("Update failed!"), F("Please check your file and retry!"), 254);
    } else {
      serveMessage(request, 200, F("Update successful!"), F("Rebooting..."), 131);
      doReboot = true;
    }
  },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    if (!correctPIN || otaLock) return;
    if(!index){
      DEBUG_PRINTLN(F("OTA Update Start"));
      #if WLED_WATCHDOG_TIMEOUT > 0
      WLED::instance().disableWatchdog();
      #endif
      usermods.onUpdateBegin(true); // notify usermods that update is about to begin (some may require task de-init)
      lastEditTime = millis(); // make sure PIN does not lock during update
      strip.suspend();
      #ifdef ESP8266
      strip.resetSegments();  // free as much memory as you can
      Update.runAsync(true);
      #endif
      Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000);
    }
    if(!Update.hasError()) Update.write(data, len);
    if(final){
      if(Update.end(true)){
        DEBUG_PRINTLN(F("Update Success"));
      } else {
        DEBUG_PRINTLN(F("Update Failed"));
        strip.resume();
        usermods.onUpdateBegin(false); // notify usermods that update has failed (some may require task init)
        #if WLED_WATCHDOG_TIMEOUT > 0
        WLED::instance().enableWatchdog();
        #endif
      }
    }
  });
#else
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    serveMessage(request, 501, "Not implemented", F("OTA updating is disabled in this build."), 254);
  });
#endif


  #ifdef WLED_ENABLE_DMX
  server.on("/dmxmap", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", PAGE_dmxmap     , dmxProcessor);
  });
  #else
  server.on("/dmxmap", HTTP_GET, [](AsyncWebServerRequest *request){
    serveMessage(request, 501, "Not implemented", F("DMX support is not enabled in this build."), 254);
  });
  #endif

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (captivePortal(request)) return;
    if (!showWelcomePage || request->hasArg(F("sliders"))) {
      handleStaticContent(request, "/index.htm", 200, "text/html", PAGE_index, PAGE_index_L);
    } else {
      serveSettings(request);
    }
  });

#ifdef WLED_ENABLE_PIXART
  server.on("/pixart.htm", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleStaticContent(request, "/pixart.htm", 200, "text/html", PAGE_pixart, PAGE_pixart_L);
  });
  #endif

#ifndef WLED_DISABLE_PXMAGIC
  server.on("/pxmagic.htm", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleStaticContent(request, "/pxmagic.htm", 200, "text/html", PAGE_pxmagic, PAGE_pxmagic_L);
  });
#endif

  server.on("/cpal.htm", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleStaticContent(request, "/cpal.htm", 200, "text/html", PAGE_cpal, PAGE_cpal_L);
  });

  #ifdef WLED_ENABLE_WEBSOCKETS
  server.addHandler(&ws);
  #endif

  //called when the url is not defined here, ajax-in; get-settings
  server.onNotFound([](AsyncWebServerRequest *request){
    DEBUG_PRINTLN("Not-Found HTTP call:");
    DEBUG_PRINTLN("URI: " + request->url());
    if (captivePortal(request)) return;

    //make API CORS compatible
    if (request->method() == HTTP_OPTIONS)
    {
      AsyncWebServerResponse *response = request->beginResponse(200);
      response->addHeader(F("Access-Control-Max-Age"), F("7200"));
      request->send(response);
      return;
    }

    if(handleSet(request, request->url())) return;
    #ifndef WLED_DISABLE_ALEXA
    if(espalexa.handleAlexaApiCall(request)) return;
    #endif
    handleStaticContent(request, request->url(), 404, "text/html", PAGE_404, PAGE_404_length);
  });
}

void generateEtag(char *etag, uint16_t eTagSuffix) {
  sprintf_P(etag, PSTR("%7d-%02x-%04x"), VERSION, cacheInvalidate, eTagSuffix);
}

bool handleIfNoneMatchCacheHeader(AsyncWebServerRequest *request, int code, uint16_t eTagSuffix) {
  // Only send 304 (Not Modified) if response code is 200 (OK)
  if (code != 200) return false;

  AsyncWebHeader *header = request->getHeader("If-None-Match");
  char etag[14];
  generateEtag(etag, eTagSuffix);
  if (header && header->value() == etag) {
    AsyncWebServerResponse *response = request->beginResponse(304);
    setStaticContentCacheHeaders(response, code, eTagSuffix);
    request->send(response);
    return true;
  }
  return false;
}

void setStaticContentCacheHeaders(AsyncWebServerResponse *response, int code, uint16_t eTagSuffix) {
  // Only send ETag for 200 (OK) responses
  if (code != 200) return;

  // https://medium.com/@codebyamir/a-web-developers-guide-to-browser-caching-cc41f3b73e7c
  #ifndef WLED_DEBUG
  // this header name is misleading, "no-cache" will not disable cache,
  // it just revalidates on every load using the "If-None-Match" header with the last ETag value
  response->addHeader(F("Cache-Control"), "no-cache");
  #else
  response->addHeader(F("Cache-Control"), "no-store,max-age=0");  // prevent caching if debug build
  #endif
  char etag[14];
  generateEtag(etag, eTagSuffix);
  response->addHeader(F("ETag"), etag);
}

/**
 * Handels the request for a static file.
 * If the file was found in the filesystem, it will be sent to the client.
 * Otherwise it will be checked if the browser cached the file and if so, a 304 response will be sent.
 * If the file was not found in the filesystem and not in the browser cache, the request will be handled as a 200 response with the content of the page.
 *
 * @param request The request object
 * @param path If a file with this path exists in the filesystem, it will be sent to the client. Set to "" to skip this check.
 * @param code The HTTP status code
 * @param contentType The content type of the web page
 * @param content Content of the web page
 * @param len Length of the content
 * @param gzip Optional. Defaults to true. If false, the gzip header will not be added.
 * @param eTagSuffix Optional. Defaults to 0. A suffix that will be added to the ETag header. This can be used to invalidate the cache for a specific page.
 */
void handleStaticContent(AsyncWebServerRequest *request, const String &path, int code, const String &contentType, const uint8_t *content, size_t len, bool gzip, uint16_t eTagSuffix) {
  if (path != "" && handleFileRead(request, path)) return;
  if (handleIfNoneMatchCacheHeader(request, code, eTagSuffix)) return;
  AsyncWebServerResponse *response = request->beginResponse_P(code, contentType, content, len);
  if (gzip) response->addHeader(FPSTR(s_content_enc), "gzip");
  setStaticContentCacheHeaders(response, code, eTagSuffix);
  request->send(response);
}



String msgProcessor(const String& var)
{
  if (var == "MSG") {
    String messageBody = messageHead;
    messageBody += F("</h2>");
    messageBody += messageSub;
    uint32_t optt = optionType;

    if (optt < 60) //redirect to settings after optionType seconds
    {
      messageBody += F("<script>setTimeout(RS,");
      messageBody +=String(optt*1000);
      messageBody += F(")</script>");
    } else if (optt < 120) //redirect back after optionType-60 seconds, unused
    {
      //messageBody += "<script>setTimeout(B," + String((optt-60)*1000) + ")</script>";
    } else if (optt < 180) //reload parent after optionType-120 seconds
    {
      messageBody += F("<script>setTimeout(RP,");
      messageBody += String((optt-120)*1000);
      messageBody += F(")</script>");
    } else if (optt == 253)
    {
      messageBody += F("<br><br><form action=/settings><button class=\"bt\" type=submit>Back</button></form>"); //button to settings
    } else if (optt == 254)
    {
      messageBody += F("<br><br><button type=\"button\" class=\"bt\" onclick=\"B()\">Back</button>");
    }
    return messageBody;
  }
  return String();
}


void serveMessage(AsyncWebServerRequest* request, uint16_t code, const String& headl, const String& subl, byte optionT)
{
  messageHead = headl;
  messageSub = subl;
  optionType = optionT;

  request->send_P(code, "text/html", PAGE_msg, msgProcessor);
}


void serveJsonError(AsyncWebServerRequest* request, uint16_t code, uint16_t error)
{
    AsyncJsonResponse *response = new AsyncJsonResponse(64);
    if (error < ERR_NOT_IMPL) response->addHeader("Retry-After", "1");
    response->setContentType("application/json");
    response->setCode(code);
    JsonObject obj = response->getRoot();
    obj[F("error")] = error;
    response->setLength();
    request->send(response);
}

#ifdef WLED_ENABLE_DMX
String dmxProcessor(const String& var)
{
  String mapJS;
  #ifdef WLED_ENABLE_DMX
    if (var == "DMXVARS") {
      mapJS += "\nCN=" + String(DMXChannels) + ";\n";
      mapJS += "CS=" + String(DMXStart) + ";\n";
      mapJS += "CG=" + String(DMXGap) + ";\n";
      mapJS += "LC=" + String(strip.getLengthTotal()) + ";\n";
      mapJS += "var CH=[";
      for (int i=0;i<15;i++) {
        mapJS += String(DMXFixtureMap[i]) + ",";
      }
      mapJS += "0];";
    }
  #endif

  return mapJS;
}
#endif


void serveSettingsJS(AsyncWebServerRequest* request)
{
  char buf[SETTINGS_STACK_BUF_SIZE+37];
  buf[0] = 0;
  byte subPage = request->arg(F("p")).toInt();
  if (subPage > 10) {
    strcpy_P(buf, PSTR("alert('Settings for this request are not implemented.');"));
    request->send(501, "application/javascript", buf);
    return;
  }
  if (subPage > 0 && !correctPIN && strlen(settingsPIN)>0) {
    strcpy_P(buf, PSTR("alert('PIN incorrect.');"));
    request->send(401, "application/javascript", buf);
    return;
  }
  strcat_P(buf,PSTR("function GetV(){var d=document;"));
  getSettingsJS(subPage, buf+strlen(buf));  // this may overflow by 35bytes!!!
  strcat_P(buf,PSTR("}"));
  request->send(200, "application/javascript", buf);
}


void serveSettings(AsyncWebServerRequest* request, bool post) {
  byte subPage = 0, originalSubPage = 0;
  const String& url = request->url();

  if (url.indexOf("sett") >= 0) {
    if      (url.indexOf(".js")  > 0) subPage = SUBPAGE_JS;
    else if (url.indexOf(".css") > 0) subPage = SUBPAGE_CSS;
    else if (url.indexOf("wifi") > 0) subPage = SUBPAGE_WIFI;
    else if (url.indexOf("leds") > 0) subPage = SUBPAGE_LEDS;
    else if (url.indexOf("ui")   > 0) subPage = SUBPAGE_UI;
    else if (url.indexOf("sync") > 0) subPage = SUBPAGE_SYNC;
    else if (url.indexOf("time") > 0) subPage = SUBPAGE_TIME;
    else if (url.indexOf("sec")  > 0) subPage = SUBPAGE_SEC;
    else if (url.indexOf("dmx")  > 0) subPage = SUBPAGE_DMX;
    else if (url.indexOf("um")   > 0) subPage = SUBPAGE_UM;
    else if (url.indexOf("2D")   > 0) subPage = SUBPAGE_2D;
    else if (url.indexOf("lock") > 0) subPage = SUBPAGE_LOCK;
  }
  else if (url.indexOf("/update") >= 0) subPage = SUBPAGE_UPDATE; // update page, for PIN check
  //else if (url.indexOf("/edit")   >= 0) subPage = 10;
  else subPage = SUBPAGE_WELCOME;

  if (!correctPIN && strlen(settingsPIN) > 0 && (subPage > 0 && subPage < 11)) {
    originalSubPage = subPage;
    subPage = SUBPAGE_PINREQ; // require PIN
  }

  // if OTA locked or too frequent PIN entry requests fail hard
  if ((subPage == SUBPAGE_WIFI && wifiLock && otaLock) || (post && !correctPIN && millis()-lastEditTime < PIN_RETRY_COOLDOWN))
  {
    serveMessage(request, 401, "Access Denied", FPSTR(s_unlock_ota), 254); return;
  }

  if (post) { //settings/set POST request, saving
    if (subPage != SUBPAGE_WIFI || !(wifiLock && otaLock)) handleSettingsSet(request, subPage);

    char s[32];
    char s2[45] = "";

    switch (subPage) {
      case SUBPAGE_WIFI   : strcpy_P(s, PSTR("WiFi")); strcpy_P(s2, PSTR("Please connect to the new IP (if changed)")); break;
      case SUBPAGE_LEDS   : strcpy_P(s, PSTR("LED")); break;
      case SUBPAGE_UI     : strcpy_P(s, PSTR("UI")); break;
      case SUBPAGE_SYNC   : strcpy_P(s, PSTR("Sync")); break;
      case SUBPAGE_TIME   : strcpy_P(s, PSTR("Time")); break;
      case SUBPAGE_SEC    : strcpy_P(s, PSTR("Security")); if (doReboot) strcpy_P(s2, PSTR("Rebooting, please wait ~10 seconds...")); break;
      case SUBPAGE_DMX    : strcpy_P(s, PSTR("DMX")); break;
      case SUBPAGE_UM     : strcpy_P(s, PSTR("Usermods")); break;
      case SUBPAGE_2D     : strcpy_P(s, PSTR("2D")); break;
      case SUBPAGE_PINREQ : strcpy_P(s, correctPIN ? PSTR("PIN accepted") : PSTR("PIN rejected")); break;
    }

    if (subPage != SUBPAGE_PINREQ) strcat_P(s, PSTR(" settings saved."));

    if (subPage == SUBPAGE_PINREQ && correctPIN) {
      subPage = originalSubPage; // on correct PIN load settings page the user intended
    } else {
      if (!s2[0]) strcpy_P(s2, s_redirecting);

      bool redirectAfter9s = (subPage == SUBPAGE_WIFI || ((subPage == SUBPAGE_SEC || subPage == SUBPAGE_UM) && doReboot));
      serveMessage(request, (correctPIN ? 200 : 401), s, s2, redirectAfter9s ? 129 : (correctPIN ? 1 : 3));
      return;
    }
  }

  int code = 200;
  String contentType = "text/html";
  const uint8_t* content;
  size_t len;

  switch (subPage) {
    case SUBPAGE_WIFI    :  content = PAGE_settings_wifi; len = PAGE_settings_wifi_length; break;
    case SUBPAGE_LEDS    :  content = PAGE_settings_leds; len = PAGE_settings_leds_length; break;
    case SUBPAGE_UI      :  content = PAGE_settings_ui;   len = PAGE_settings_ui_length;   break;
    case SUBPAGE_SYNC    :  content = PAGE_settings_sync; len = PAGE_settings_sync_length; break;
    case SUBPAGE_TIME    :  content = PAGE_settings_time; len = PAGE_settings_time_length; break;
    case SUBPAGE_SEC     :  content = PAGE_settings_sec;  len = PAGE_settings_sec_length;  break;
#ifdef WLED_ENABLE_DMX
    case SUBPAGE_DMX     :  content = PAGE_settings_dmx;  len = PAGE_settings_dmx_length;  break;
#endif
    case SUBPAGE_UM      :  content = PAGE_settings_um;   len = PAGE_settings_um_length;   break;
    case SUBPAGE_UPDATE  :  content = PAGE_update;        len = PAGE_update_length;        break;
#ifndef WLED_DISABLE_2D
    case SUBPAGE_2D      :  content = PAGE_settings_2D;   len = PAGE_settings_2D_length;   break;
#endif
    case SUBPAGE_LOCK    : {
      correctPIN = !strlen(settingsPIN); // lock if a pin is set
      createEditHandler(correctPIN);
      serveMessage(request, 200, strlen(settingsPIN) > 0 ? PSTR("Settings locked") : PSTR("No PIN set"), FPSTR(s_redirecting), 1);
      return;
    }
    case SUBPAGE_PINREQ  :  content = PAGE_settings_pin;  len = PAGE_settings_pin_length; code = 401;               break;
    case SUBPAGE_CSS     :  content = PAGE_settingsCss;   len = PAGE_settingsCss_length;  contentType = "text/css"; break;
    case SUBPAGE_JS      :  serveSettingsJS(request); return;
    case SUBPAGE_WELCOME :  content = PAGE_welcome;       len = PAGE_welcome_length;       break;
    default:                content = PAGE_settings;      len = PAGE_settings_length;      break;
  }
  handleStaticContent(request, "", code, contentType, content, len);
}
