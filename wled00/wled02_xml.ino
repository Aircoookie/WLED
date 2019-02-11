/*
 * Sending XML status files to client
 */

//build XML response to HTTP /win API request
void XML_response(bool isHTTP, bool includeTheme)
{
  olen = 0;
  oappend("<?xml version = \"1.0\" ?><vs><ac>");
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
  if (useRGBW && !autoRGBtoRGBW) {
   oappendi(col[3]);
  } else {
   oappend("-1");
  }
  oappend("</wv><ws>");
  oappendi(colSec[3]);
  oappend("</ws><md>");
  oappendi(useHSB);
  oappend("</md><cy>");
  oappendi(presetCyclingEnabled);
  oappend("</cy><ds>");
  oappend(serverDescription);
  oappend("</ds>");
  if (includeTheme)
  {
    char cs[6][9];
    getThemeColors(cs);
    oappend("<th><ca>#");
    oappend(cs[0]);
    oappend("</ca><cb>#");
    oappend(cs[1]);
    oappend("</cb><cc>#");
    oappend(cs[2]);
    oappend("</cc><cd>#");
    oappend(cs[3]);
    oappend("</cd><cu>#");
    oappend(cs[4]);
    oappend("</cu><ct>#");
    oappend(cs[5]);
    oappend("</ct><cf>");
    oappend(cssFont);
    oappend("</cf></th>");
  }
  oappend("</vs>");
  if (isHTTP) server.send(200, "text/xml", obuf);
}

//append a numeric setting to string buffer
void sappend(char stype, char* key, int val)
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
void sappends(char stype, char* key, char* val)
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
void getSettingsJS(byte subPage)
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
    sappend('v',"MA",strip.ablMilliampsMax);
    if (strip.currentMilliamps)
    {
      sappends('m',"(\"pow\")[0]","");
      olen -= 2; //delete ";
      oappendi(strip.currentMilliamps);
      oappend("mA\";");
    }
    sappend('v',"CR",colS[0]);
    sappend('v',"CG",colS[1]);
    sappend('v',"CB",colS[2]);
    sappend('v',"CA",briS);
    sappend('c',"EW",useRGBW);
    sappend('i',"CO",strip.colorOrder);
    sappend('c',"AW",autoRGBtoRGBW);
    sappend('v',"CW",colS[3]);
    sappend('v',"SR",colSecS[0]);
    sappend('v',"SG",colSecS[1]);
    sappend('v',"SB",colSecS[2]);
    sappend('v',"SW",colSecS[3]);
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
    sappend('c',"T2",enableSecTransition);
    sappend('v',"BF",briMultiplier);
    sappend('v',"TB",nightlightTargetBri);
    sappend('v',"TL",nightlightDelayMinsDefault);
    sappend('c',"TW",nightlightFade);
    sappend('i',"PB",strip.paletteBlend);
    sappend('c',"RV",reverseMode);
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
    sappend('c',"IR",irEnabled);
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
    sappend('v',"ET",realtimeTimeoutMs);
    sappend('c',"FB",arlsForceMaxBri);
    sappend('c',"RG",arlsDisableGammaCorrection);
    sappend('v',"WO",arlsOffset);
    sappend('c',"RU",enableRealtimeUI);
    sappend('c',"AL",alexaEnabled);
    sappends('s',"AI",alexaInvocationName);
    sappend('c',"SA",notifyAlexa);
    sappends('s',"BK",(char*)((blynkEnabled)?"Hidden":""));
    sappends('s',"MS",mqttServer);
    sappends('s',"MD",mqttDeviceTopic);
    sappends('s',"MG",mqttGroupTopic);
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


//get colors from current theme as c strings
void getThemeColors(char o[][9])
{
  switch (currentTheme)
  {
    //       accent color (aCol)     background (bCol)       panel (cCol)            controls (dCol)         shadows (sCol)          text (tCol)    
    default: strcpy(o[0], "D9B310"); strcpy(o[1], "0B3C5D"); strcpy(o[2], "1D2731"); strcpy(o[3], "328CC1"); strcpy(o[4], "000");    strcpy(o[5], "328CC1"); break; //night
    case 1:  strcpy(o[0], "eee");    strcpy(o[1], "ddd");    strcpy(o[2], "b9b9b9"); strcpy(o[3], "049");    strcpy(o[4], "777");    strcpy(o[5], "049");    break; //modern
    case 2:  strcpy(o[0], "abb");    strcpy(o[1], "fff");    strcpy(o[2], "ddd");    strcpy(o[3], "000");    strcpy(o[4], "0004");   strcpy(o[5], "000");    break; //bright
    case 3:  strcpy(o[0], "c09f80"); strcpy(o[1], "d7cec7"); strcpy(o[2], "76323f"); strcpy(o[3], "888");    strcpy(o[4], "3334");   strcpy(o[5], "888");    break; //wine
    case 4:  strcpy(o[0], "3cc47c"); strcpy(o[1], "828081"); strcpy(o[2], "d9a803"); strcpy(o[3], "1e392a"); strcpy(o[4], "000a");   strcpy(o[5], "1e392a"); break; //electric
    case 5:  strcpy(o[0], "57bc90"); strcpy(o[1], "a5a5af"); strcpy(o[2], "015249"); strcpy(o[3], "88c9d4"); strcpy(o[4], "0004");   strcpy(o[5], "88c9d4"); break; //mint
    case 6:  strcpy(o[0], "f7c331"); strcpy(o[1], "dca");    strcpy(o[2], "6b7a8f"); strcpy(o[3], "f7882f"); strcpy(o[4], "0007");   strcpy(o[5], "f7882f"); break; //amber
    case 7:  strcpy(o[0], "fff");    strcpy(o[1], "333");    strcpy(o[2], "222");    strcpy(o[3], "666");    strcpy(o[4], "");       strcpy(o[5], "fff");    break; //dark
    case 8:  strcpy(o[0], "0ac");    strcpy(o[1], "124");    strcpy(o[2], "224");    strcpy(o[3], "003eff"); strcpy(o[4], "003eff"); strcpy(o[5], "003eff"); break; //air
    case 9:  strcpy(o[0], "f70");    strcpy(o[1], "421");    strcpy(o[2], "221");    strcpy(o[3], "a50");    strcpy(o[4], "f70");    strcpy(o[5], "f70");    break; //nixie
    case 10: strcpy(o[0], "2d2");    strcpy(o[1], "010");    strcpy(o[2], "121");    strcpy(o[3], "060");    strcpy(o[4], "040");    strcpy(o[5], "3f3");    break; //terminal
    case 11: strcpy(o[0], "867ADE"); strcpy(o[1], "4033A3"); strcpy(o[2], "483AAA"); strcpy(o[3], "483AAA"); strcpy(o[4], "");       strcpy(o[5], "867ADE"); break; //c64
    case 12: strcpy(o[0], "fbe8a6"); strcpy(o[1], "d2fdff"); strcpy(o[2], "b4dfe5"); strcpy(o[3], "f4976c"); strcpy(o[4], "");       strcpy(o[5], "303c6c"); break; //easter
    case 13: strcpy(o[0], "d4af37"); strcpy(o[1], "173305"); strcpy(o[2], "308505"); strcpy(o[3], "f21313"); strcpy(o[4], "f002");   strcpy(o[5], "d4af37"); break; //christmas
    case 14: strcpy(o[0], "fc7");    strcpy(o[1], "49274a"); strcpy(o[2], "94618e"); strcpy(o[3], "f4decb"); strcpy(o[4], "0008");   strcpy(o[5], "f4decb"); break; //end
    case 15: for (int i=0;i<6;i++) strcpy(o[i], cssCol[i]); //custom
  }
}
