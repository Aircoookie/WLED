/*
 * Sending XML status files to client
 */

void XML_response()
{
   olen = 0;
   oappend("<?xml version = \"1.0\" ?><vs><ac>");
   if (nightlightActive && nightlightFade)
   {
     oappendi(briT);
   } else
   {
     oappendi(bri);
   }
   oappend("</ac>");

   for (int i = 0; i < 3; i++)
   {
     oappend("<cl>");
     oappendi(col[i]);
     oappend("</cl>");
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
   if (useRGBW && !autoRGBtoRGBW) {
     oappendi(white);
   } else {
     oappend("-1");
   }
   oappend("</wv><md>");
   oappendi(useHSB);
   oappend("</md><ds>");
   oappend(serverDescription);
   oappend("</ds></vs>");
   server.send(200, "text/xml", obuf);
}

void sappend(char stype, char* key, int val) //append a setting to string buffer
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

void sappends(char stype, char* key, char* val) //append a string setting
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

void getSettingsJS(byte subPage) //get values for settings form in javascript
{
  //0: menu 1: wifi 2: leds 3: ui 4: sync 5: time 6: sec
  DEBUG_PRINT("settings resp");
  DEBUG_PRINTLN(subPage);
  
  olen = 0; obuf[0] = 0; //clear buffer
  if (subPage <1 || subPage >6) return;

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
    sappend('v',"AT",apWaitTimeSecs);
    sappends('s',"AS",apSSID);
    sappend('c',"AH",apHide);
    
    l = strlen(apPass);
    char fapass[l+1]; //fill password field with ***
    fapass[l] = 0;
    memset(fapass,'*',l); 
    sappends('s',"AP",fapass);
    
    sappend('v',"AC",apChannel);

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
    sappend('v',"LC",ledCount);
    sappend('v',"CR",colS[0]);
    sappend('v',"CG",colS[1]);
    sappend('v',"CB",colS[2]);
    sappend('v',"CA",briS);
    sappend('c',"EW",useRGBW);
    sappend('c',"AW",autoRGBtoRGBW);
    sappend('v',"CW",whiteS);
    sappend('v',"SR",colSecS[0]);
    sappend('v',"SG",colSecS[1]);
    sappend('v',"SB",colSecS[2]);
    sappend('v',"SW",whiteSecS);
    sappend('c',"BO",turnOnAtBoot);
    sappend('v',"BP",bootPreset);
    sappend('v',"FX",effectDefault);
    sappend('v',"SX",effectSpeedDefault);
    sappend('v',"IX",effectIntensityDefault);
    sappend('v',"FP",effectPaletteDefault);
    sappend('c',"GB",useGammaCorrectionBri);
    sappend('c',"GC",useGammaCorrectionRGB);
    sappend('c',"TF",fadeTransition);
    sappend('v',"TD",transitionDelay);
    sappend('c',"PF",strip.paletteFade);
    sappend('c',"T2",!disableSecTransition);
    sappend('v',"BF",briMultiplier);
    sappend('v',"TB",nightlightTargetBri);
    sappend('v',"TL",nightlightDelayMins);
    sappend('c',"TW",nightlightFade);
    sappend('c',"RV",reverseMode);
    sappend('c',"EI",initLedsLast);
    sappend('c',"SL",skipFirstLed);
  }

  if (subPage == 3)
  { 
    sappend('i',"UI",uiConfiguration);
    sappends('s',"DS",serverDescription);
    sappend('c',"MD",useHSBDefault);
    sappend('i',"TH",currentTheme);
    char k[3]; k[0] = 'C'; k[2] = 0; //keys
    for (int i=0; i<6; i++)
    {
      k[1] = 48+i; //ascii 0,1,2,3,4,5
      sappends('s',k,cssCol[i]);
    }
    sappends('s',"CF",cssFont);
  }

  if (subPage == 4)
  {
    sappend('c',"BT",buttonEnabled);
    sappend('v',"UP",udpPort);
    sappend('c',"RB",receiveNotificationBrightness);
    sappend('c',"RC",receiveNotificationColor);
    sappend('c',"RX",receiveNotificationEffects);
    sappend('c',"SD",notifyDirectDefault);
    sappend('c',"SB",notifyButton);
    sappend('c',"SH",notifyHue);
    sappend('c',"S2",notifyTwice);
    sappend('c',"RD",receiveDirect);
    sappend('c',"EM",e131Multicast);
    sappend('v',"EU",e131Universe);
    sappend('v',"ET",arlsTimeoutMillis);
    sappend('c',"FB",arlsForceMaxBri);
    sappend('c',"RG",arlsDisableGammaCorrection);
    sappend('v',"WO",arlsOffset);
    sappend('c',"RU",enableRealtimeUI);
    sappend('c',"AL",alexaEnabled);
    sappends('s',"AI",alexaInvocationName);
    sappend('c',"SA",alexaNotify);
    sappends('s',"BK",(char*)((blynkEnabled)?"Hidden":""));
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
    sappends('m',"(\"hms\")[0]",hueError);
  }

  if (subPage == 5)
  {
    sappend('c',"NT",ntpEnabled);
    sappend('c',"CF",!useAMPM);
    sappend('i',"TZ",currentTimezone);
    sappend('v',"UO",utcOffsetSecs);
    sappends('m',"(\"times\")[0]",(char*)getTimeString().c_str());
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
      sprintf(k+1,"%i",i);
      sappends('s',k,(char*)loadMacro(i).c_str());
    }
    
    sappend('v',"MB",macroBoot);
    sappend('v',"A0",macroAlexaOn);
    sappend('v',"A1",macroAlexaOff);
    sappend('v',"MP",macroButton);
    sappend('v',"ML",macroLongPress);
    sappend('v',"MC",macroCountdown);
    sappend('v',"MN",macroNl);
  }

  if (subPage == 6)
  {
    sappend('c',"NO",otaLock);
    sappend('c',"OW",wifiLock);
    sappend('c',"AO",aOtaEnabled);
    sappend('c',"NA",recoveryAPDisabled);
    sappends('m',"(\"msg\")[0]","WLED ");
    olen -= 2; //delete ";
    oappend(versionString);
    oappend(" (build ");
    oappendi(VERSION);
    oappend(") OK\";");
  }
  oappend("}</script>");
}
