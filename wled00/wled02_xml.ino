/*
 * Sending XML status files to client
 */

//build XML response to HTTP /win API request
char* XML_response(AsyncWebServerRequest *request, char* dest = nullptr)
{
  char sbuf[(dest == nullptr)?1024:1]; //allocate local buffer if none passed
  obuf = (dest == nullptr)? sbuf:dest;

  olen = 0;
  oappend((const char*)F("<?xml version=\"1.0\" ?><vs><ac>"));
  oappendi((nightlightActive && nightlightFade) ? briT : bri);
  oappend("</ac>");

  for (int i = 0; i < 3; i++)
  {
   oappend("<cl>");
   oappendi(col[i]);
   oappend("</cl>");
  }
  for (int i = 0; i < 3; i++)
  {
   oappend("<cs>");
   oappendi(colSec[i]);
   oappend("</cs>");
  }
  oappend("<ns>");
  oappendi(notifyDirect);
  oappend("</ns><nr>");
  oappendi(receiveNotifications);
  oappend("</nr><nl>");
  oappendi(nightlightActive);
  oappend("</nl><nf>");
  oappendi(nightlightFade);
  oappend("</nf><nd>");
  oappendi(nightlightDelayMins);
  oappend("</nd><nt>");
  oappendi(nightlightTargetBri);
  oappend("</nt><fx>");
  oappendi(effectCurrent);
  oappend("</fx><sx>");
  oappendi(effectSpeed);
  oappend("</sx><ix>");
  oappendi(effectIntensity);
  oappend("</ix><fp>");
  oappendi(effectPalette);
  oappend("</fp><wv>");
  if (strip.rgbwMode) {
   oappendi(col[3]);
  } else {
   oappend("-1");
  }
  oappend("</wv><ws>");
  oappendi(colSec[3]);
  oappend("</ws><ps>");
  oappendi((currentPreset < 1) ? 0:currentPreset);
  oappend("</ps><cy>");
  oappendi(presetCyclingEnabled);
  oappend("</cy><ds>");
  if (realtimeMode)
  {
    String mesg = "Live ";
    if (realtimeMode == REALTIME_MODE_E131)
    {
      mesg += "E1.31 mode ";
      mesg += DMXMode;
      mesg += F(" at DMX Address ");
      mesg += DMXAddress;
      mesg += " from ";
      mesg += realtimeIP[0];
      for (int i = 1; i < 4; i++)
      {
        mesg += ".";
        mesg += realtimeIP[i];
      }
      mesg += " seq=";
      mesg += e131LastSequenceNumber;
    } else if (realtimeMode == REALTIME_MODE_UDP || realtimeMode == REALTIME_MODE_HYPERION) {
      mesg += "UDP from ";
      mesg += realtimeIP[0];
      for (int i = 1; i < 4; i++)
      {
        mesg += ".";
        mesg += realtimeIP[i];
      }
    } else if (realtimeMode == REALTIME_MODE_ADALIGHT) {
      mesg += F("USB Adalight");
    } else { //generic
      mesg += "data";
    }
    oappend((char*)mesg.c_str());
  } else {
    oappend(serverDescription);
  }
  oappend("</ds><ss>");
  oappendi(strip.getMainSegmentId());
  oappend("</ss></vs>");
  if (request != nullptr) request->send(200, "text/xml", obuf);
}

char* URL_response(AsyncWebServerRequest *request)
{
  char sbuf[256]; //allocate local buffer if none passed
  char s2buf[100];
  obuf = s2buf;
  olen = 0;

  char s[16];
  oappend("http://");
  IPAddress localIP = WiFi.localIP();
  sprintf(s, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);
  oappend(s);
  oappend("/win&A=");
  oappendi(bri);
  oappend("&CL=h");
  for (int i = 0; i < 3; i++)
  {
   sprintf(s,"%02X", col[i]);
   oappend(s); 
  }
  oappend("&C2=h");
  for (int i = 0; i < 3; i++)
  {
   sprintf(s,"%02X", colSec[i]);
   oappend(s);
  }
  oappend("&FX=");
  oappendi(effectCurrent);
  oappend("&SX=");
  oappendi(effectSpeed);
  oappend("&IX=");
  oappendi(effectIntensity);
  oappend("&FP=");
  oappendi(effectPalette);

  obuf = sbuf;
  olen = 0;

  oappend((const char*)F("<html><body><a href=\""));
  oappend(s2buf);
  oappend((const char*)F("\" target=\"_blank\">"));
  oappend(s2buf);  
  oappend((const char*)F("</a></body></html>"));

  if (request != nullptr) request->send(200, "text/html", obuf);
}

//append a numeric setting to string buffer
void sappend(char stype, const char* key, int val)
{
  char ds[] = "d.Sf.";

  switch(stype)
  {
    case 'c': //checkbox
      oappend(ds);
      oappend(key);
      oappend(".checked=");
      oappendi(val);
      oappend(";");
      break;
    case 'v': //numeric
      oappend(ds);
      oappend(key);
      oappend(".value=");
      oappendi(val);
      oappend(";");
      break;
    case 'i': //selectedIndex
      oappend(ds);
      oappend(key);
      oappend(".selectedIndex=");
      oappendi(val);
      oappend(";");
      break;
  }
}

//append a string setting to buffer
void sappends(char stype, const char* key, char* val)
{
  switch(stype)
  {
    case 's': //string (we can interpret val as char*)
      oappend("d.Sf.");
      oappend(key);
      oappend(".value=\"");
      oappend(val);
      oappend("\";");
      break;
    case 'm': //message
      oappend("d.getElementsByClassName");
      oappend(key);
      oappend(".innerHTML=\"");
      oappend(val);
      oappend("\";");
      break;
  }
}


//get values for settings form in javascript
void getSettingsJS(byte subPage, char* dest)
{
  //0: menu 1: wifi 2: leds 3: ui 4: sync 5: time 6: sec
  DEBUG_PRINT("settings resp");
  DEBUG_PRINTLN(subPage);
  obuf = dest;
  olen = 0;

  if (subPage <1 || subPage >7) return;

  if (subPage == 1) {
    sappends('s',"CS",clientSSID);

    byte l = strlen(clientPass);
    char fpass[l+1]; //fill password field with ***
    fpass[l] = 0;
    memset(fpass,'*',l);
    sappends('s',"CP",fpass);

    char k[3]; k[2] = 0; //IP addresses
    for (int i = 0; i<4; i++)
    {
      k[1] = 48+i; //ascii 0,1,2,3
      k[0] = 'I'; sappend('v',k,staticIP[i]);
      k[0] = 'G'; sappend('v',k,staticGateway[i]);
      k[0] = 'S'; sappend('v',k,staticSubnet[i]);
    }

    sappends('s',"CM",cmDNS);
    sappend('i',"AB",apBehavior);
    sappends('s',"AS",apSSID);
    sappend('c',"AH",apHide);

    l = strlen(apPass);
    char fapass[l+1]; //fill password field with ***
    fapass[l] = 0;
    memset(fapass,'*',l);
    sappends('s',"AP",fapass);

    sappend('v',"AC",apChannel);
    sappend('c',"WS",noWifiSleep);


    if (WiFi.localIP()[0] != 0) //is connected
    {
      char s[16];
      IPAddress localIP = WiFi.localIP();
      sprintf(s, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);
      sappends('m',"(\"sip\")[0]",s);
    } else
    {
      sappends('m',"(\"sip\")[0]","Not connected");
    }

    if (WiFi.softAPIP()[0] != 0) //is active
    {
      char s[16];
      IPAddress apIP = WiFi.softAPIP();
      sprintf(s, "%d.%d.%d.%d", apIP[0], apIP[1], apIP[2], apIP[3]);
      sappends('m',"(\"sip\")[1]",s);
    } else
    {
      sappends('m',"(\"sip\")[1]","Not active");
    }
  }

  if (subPage == 2) {
    #ifdef ESP8266
    #if LEDPIN == 3
    oappend("d.Sf.LC.max=500;");
    #endif
    #endif
    sappend('v',"LC",ledCount);
    sappend('v',"MA",strip.ablMilliampsMax);
    sappend('v',"LA",strip.milliampsPerLed);
    if (strip.currentMilliamps)
    {
      sappends('m',"(\"pow\")[0]","");
      olen -= 2; //delete ";
      oappendi(strip.currentMilliamps);
      oappend("mA\";");
    }

    sappend('v',"CA",briS);
    sappend('c',"EW",useRGBW);
    sappend('i',"CO",strip.colorOrder);
    sappend('v',"AW",strip.rgbwMode);

    sappend('c',"BO",turnOnAtBoot);
    sappend('v',"BP",bootPreset);

    sappend('c',"GB",strip.gammaCorrectBri);
    sappend('c',"GC",strip.gammaCorrectCol);
    sappend('c',"TF",fadeTransition);
    sappend('v',"TD",transitionDelayDefault);
    sappend('c',"PF",strip.paletteFade);
    sappend('v',"BF",briMultiplier);
    sappend('v',"TB",nightlightTargetBri);
    sappend('v',"TL",nightlightDelayMinsDefault);
    sappend('c',"TW",nightlightFade);
    sappend('i',"PB",strip.paletteBlend);
    sappend('c',"RV",strip.reverseMode);
    sappend('c',"SL",skipFirstLed);
  }

  if (subPage == 3)
  {
    sappends('s',"DS",serverDescription);
    sappend('c',"ST",syncToggleReceive);
  }

  if (subPage == 4)
  {
    sappend('c',"BT",buttonEnabled);
    sappend('v',"IR",irEnabled);
    sappend('v',"UP",udpPort);
    sappend('c',"RB",receiveNotificationBrightness);
    sappend('c',"RC",receiveNotificationColor);
    sappend('c',"RX",receiveNotificationEffects);
    sappend('c',"SD",notifyDirectDefault);
    sappend('c',"SB",notifyButton);
    sappend('c',"SH",notifyHue);
    sappend('c',"SM",notifyMacro);
    sappend('c',"S2",notifyTwice);
    sappend('c',"RD",receiveDirect);
    sappend('c',"EM",e131Multicast);
    sappend('v',"EU",e131Universe);
    sappend('v',"DA",DMXAddress);
    sappend('v',"DM",DMXMode);
    sappend('v',"ET",realtimeTimeoutMs);
    sappend('c',"FB",arlsForceMaxBri);
    sappend('c',"RG",arlsDisableGammaCorrection);
    sappend('v',"WO",arlsOffset);
    sappend('c',"AL",alexaEnabled);
    sappends('s',"AI",alexaInvocationName);
    sappend('c',"SA",notifyAlexa);
    sappends('s',"BK",(char*)((blynkEnabled)?"Hidden":""));

    #ifdef WLED_ENABLE_MQTT
    sappend('c',"MQ",mqttEnabled);
    sappends('s',"MS",mqttServer);
    sappend('v',"MQPORT",mqttPort);
    sappends('s',"MQUSER",mqttUser);
    sappends('s',"MQPASS",mqttPass);
    byte l = strlen(mqttPass);
    char fpass[l+1]; //fill password field with ***
    fpass[l] = 0;
    memset(fpass,'*',l);
    sappends('s',"MQPASS",fpass);
    sappends('s',"MQCID",mqttClientID);
    sappends('s',"MD",mqttDeviceTopic);
    sappends('s',"MG",mqttGroupTopic);
    #endif

    #ifndef WLED_DISABLE_HUESYNC
    sappend('v',"H0",hueIP[0]);
    sappend('v',"H1",hueIP[1]);
    sappend('v',"H2",hueIP[2]);
    sappend('v',"H3",hueIP[3]);
    sappend('v',"HL",huePollLightId);
    sappend('v',"HI",huePollIntervalMs);
    sappend('c',"HP",huePollingEnabled);
    sappend('c',"HO",hueApplyOnOff);
    sappend('c',"HB",hueApplyBri);
    sappend('c',"HC",hueApplyColor);
    char hueErrorString[25];
    switch (hueError)
    {
      case HUE_ERROR_INACTIVE     : strcpy(hueErrorString,(char*)F("Inactive"));                break;
      case HUE_ERROR_ACTIVE       : strcpy(hueErrorString,(char*)F("Active"));                  break;
      case HUE_ERROR_UNAUTHORIZED : strcpy(hueErrorString,(char*)F("Unauthorized"));            break;
      case HUE_ERROR_LIGHTID      : strcpy(hueErrorString,(char*)F("Invalid light ID"));        break;
      case HUE_ERROR_PUSHLINK     : strcpy(hueErrorString,(char*)F("Link button not pressed")); break;
      case HUE_ERROR_JSON_PARSING : strcpy(hueErrorString,(char*)F("JSON parsing error"));      break;
      case HUE_ERROR_TIMEOUT      : strcpy(hueErrorString,(char*)F("Timeout"));                 break;
      default: sprintf(hueErrorString,"Bridge Error %i",hueError);
    }
    
    sappends('m',"(\"hms\")[0]",hueErrorString);
    #endif
  }

  if (subPage == 5)
  {
    sappend('c',"NT",ntpEnabled);
    sappends('s',"NS",ntpServerName);
    sappend('c',"CF",!useAMPM);
    sappend('i',"TZ",currentTimezone);
    sappend('v',"UO",utcOffsetSecs);
    char tm[32];
    getTimeString(tm);
    sappends('m',"(\"times\")[0]",tm);
    sappend('i',"OL",overlayCurrent);
    sappend('v',"O1",overlayMin);
    sappend('v',"O2",overlayMax);
    sappend('v',"OM",analogClock12pixel);
    sappend('c',"OS",analogClockSecondsTrail);
    sappend('c',"O5",analogClock5MinuteMarks);
    sappends('s',"CX",cronixieDisplay);
    sappend('c',"CB",cronixieBacklight);
    sappend('c',"CE",countdownMode);
    sappend('v',"CY",countdownYear);
    sappend('v',"CI",countdownMonth);
    sappend('v',"CD",countdownDay);
    sappend('v',"CH",countdownHour);
    sappend('v',"CM",countdownMin);
    sappend('v',"CS",countdownSec);
    char k[4]; k[0]= 'M';
    for (int i=1;i<17;i++)
    {
      char m[65];
      loadMacro(i, m);
      sprintf(k+1,"%i",i);
      sappends('s',k,m);
    }

    sappend('v',"MB",macroBoot);
    sappend('v',"A0",macroAlexaOn);
    sappend('v',"A1",macroAlexaOff);
    sappend('v',"MP",macroButton);
    sappend('v',"ML",macroLongPress);
    sappend('v',"MC",macroCountdown);
    sappend('v',"MN",macroNl);
    sappend('v',"MD",macroDoublePress);

    k[2] = 0; //Time macros
    for (int i = 0; i<8; i++)
    {
      k[1] = 48+i; //ascii 0,1,2,3
      k[0] = 'H'; sappend('v',k,timerHours[i]);
      k[0] = 'N'; sappend('v',k,timerMinutes[i]);
      k[0] = 'T'; sappend('v',k,timerMacro[i]);
      k[0] = 'W'; sappend('v',k,timerWeekday[i]);
    }
  }

  if (subPage == 6)
  {
    sappend('c',"NO",otaLock);
    sappend('c',"OW",wifiLock);
    sappend('c',"AO",aOtaEnabled);
    sappends('m',"(\"msg\")[0]","WLED ");
    olen -= 2; //delete ";
    oappend(versionString);
    oappend(" (build ");
    oappendi(VERSION);
    oappend(") OK\";");
  }
  
  #ifdef WLED_ENABLE_DMX // include only if DMX is enabled
  if (subPage == 7)
  {
    sappend('v',"CN",DMXChannels);
    sappend('v',"CG",DMXGap);
    sappend('v',"CS",DMXStart);
    
    sappend('i',"CH1",DMXFixtureMap[0]);
    sappend('i',"CH2",DMXFixtureMap[1]);
    sappend('i',"CH3",DMXFixtureMap[2]);
    sappend('i',"CH4",DMXFixtureMap[3]);
    sappend('i',"CH5",DMXFixtureMap[4]);
    sappend('i',"CH6",DMXFixtureMap[5]);
    sappend('i',"CH7",DMXFixtureMap[6]);
    sappend('i',"CH8",DMXFixtureMap[7]);
    sappend('i',"CH9",DMXFixtureMap[8]);
    sappend('i',"CH10",DMXFixtureMap[9]);
    sappend('i',"CH11",DMXFixtureMap[10]);
    sappend('i',"CH12",DMXFixtureMap[11]);
    sappend('i',"CH13",DMXFixtureMap[12]);
    sappend('i',"CH14",DMXFixtureMap[13]);
    sappend('i',"CH15",DMXFixtureMap[14]);
    }
  #endif
  oappend("}</script>");
}
