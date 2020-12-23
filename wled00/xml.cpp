#include "wled.h"

/*
 * Sending XML status files to client
 */

//macro to convert F to const
#define SET_F(x)  (const char*)F(x)

//build XML response to HTTP /win API request
void XML_response(AsyncWebServerRequest *request, char* dest)
{
  char sbuf[(dest == nullptr)?1024:1]; //allocate local buffer if none passed
  obuf = (dest == nullptr)? sbuf:dest;

  olen = 0;
  oappend(SET_F("<?xml version=\"1.0\" ?><vs><ac>"));
  oappendi((nightlightActive && nightlightMode > NL_MODE_SET) ? briT : bri);
  oappend(SET_F("</ac>"));

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
  oappend(SET_F("<ns>"));
  oappendi(notifyDirect);
  oappend(SET_F("</ns><nr>"));
  oappendi(receiveNotifications);
  oappend(SET_F("</nr><nl>"));
  oappendi(nightlightActive);
  oappend(SET_F("</nl><nf>"));
  oappendi(nightlightMode > NL_MODE_SET);
  oappend(SET_F("</nf><nd>"));
  oappendi(nightlightDelayMins);
  oappend(SET_F("</nd><nt>"));
  oappendi(nightlightTargetBri);
  oappend(SET_F("</nt><fx>"));
  oappendi(effectCurrent);
  oappend(SET_F("</fx><sx>"));
  oappendi(effectSpeed);
  oappend(SET_F("</sx><ix>"));
  oappendi(effectIntensity);
  oappend(SET_F("</ix><fp>"));
  oappendi(effectPalette);
  oappend(SET_F("</fp><wv>"));
  if (strip.rgbwMode) {
   oappendi(col[3]);
  } else {
   oappend("-1");
  }
  oappend(SET_F("</wv><ws>"));
  oappendi(colSec[3]);
  oappend(SET_F("</ws><ps>"));
  oappendi((currentPreset < 1) ? 0:currentPreset);
  oappend(SET_F("</ps><cy>"));
  oappendi(presetCyclingEnabled);
  oappend(SET_F("</cy><ds>"));
  oappend(serverDescription);
  if (realtimeMode)
  {
    oappend(SET_F(" (live)"));
  }
  oappend(SET_F("</ds><ss>"));
  oappendi(strip.getMainSegmentId());
  oappend(SET_F("</ss></vs>"));
  if (request != nullptr) request->send(200, "text/xml", obuf);
}

void URL_response(AsyncWebServerRequest *request)
{
  char sbuf[256];
  char s2buf[100];
  obuf = s2buf;
  olen = 0;

  char s[16];
  oappend(SET_F("http://"));
  IPAddress localIP = Network.localIP();
  sprintf(s, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);
  oappend(s);

  oappend(SET_F("/win&A="));
  oappendi(bri);
  oappend(SET_F("&CL=h"));
  for (int i = 0; i < 3; i++)
  {
   sprintf(s,"%02X", col[i]);
   oappend(s); 
  }
  oappend(SET_F("&C2=h"));
  for (int i = 0; i < 3; i++)
  {
   sprintf(s,"%02X", colSec[i]);
   oappend(s);
  }
  oappend(SET_F("&FX="));
  oappendi(effectCurrent);
  oappend(SET_F("&SX="));
  oappendi(effectSpeed);
  oappend(SET_F("&IX="));
  oappendi(effectIntensity);
  oappend(SET_F("&FP="));
  oappendi(effectPalette);

  obuf = sbuf;
  olen = 0;

  oappend(SET_F("<html><body><a href=\""));
  oappend(s2buf);
  oappend(SET_F("\" target=\"_blank\">"));
  oappend(s2buf);  
  oappend(SET_F("</a></body></html>"));

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
      oappend(SET_F(".selectedIndex="));
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
      oappend(SET_F("d.getElementsByClassName"));
      oappend(key);
      oappend(SET_F(".innerHTML=\""));
      oappend(val);
      oappend("\";");
      break;
  }
}


//get values for settings form in javascript
void getSettingsJS(byte subPage, char* dest)
{
  //0: menu 1: wifi 2: leds 3: ui 4: sync 5: time 6: sec
  DEBUG_PRINT(F("settings resp"));
  DEBUG_PRINTLN(subPage);
  obuf = dest;
  olen = 0;

  if (subPage <1 || subPage >7) return;

  if (subPage == 1) {
    sappends('s',SET_F("CS"),clientSSID);

    byte l = strlen(clientPass);
    char fpass[l+1]; //fill password field with ***
    fpass[l] = 0;
    memset(fpass,'*',l);
    sappends('s',SET_F("CP"),fpass);

    char k[3]; k[2] = 0; //IP addresses
    for (int i = 0; i<4; i++)
    {
      k[1] = 48+i; //ascii 0,1,2,3
      k[0] = 'I'; sappend('v',k,staticIP[i]);
      k[0] = 'G'; sappend('v',k,staticGateway[i]);
      k[0] = 'S'; sappend('v',k,staticSubnet[i]);
    }

    sappends('s',SET_F("CM"),cmDNS);
    sappend('i',SET_F("AB"),apBehavior);
    sappends('s',SET_F("AS"),apSSID);
    sappend('c',SET_F("AH"),apHide);

    l = strlen(apPass);
    char fapass[l+1]; //fill password field with ***
    fapass[l] = 0;
    memset(fapass,'*',l);
    sappends('s',SET_F("AP"),fapass);

    sappend('v',SET_F("AC"),apChannel);
    sappend('c',SET_F("WS"),noWifiSleep);


    if (Network.isConnected()) //is connected
    {
      char s[32];
      IPAddress localIP = Network.localIP();
      sprintf(s, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);

      #if defined(ARDUINO_ARCH_ESP32) && defined(WLED_USE_ETHERNET)
      if (Network.isEthernet()) strcat_P(s ,SET_F(" (Ethernet)"));
      #endif
      sappends('m',SET_F("(\"sip\")[0]"),s);
    } else
    {
      sappends('m',SET_F("(\"sip\")[0]"),(char*)F("Not connected"));
    }

    if (WiFi.softAPIP()[0] != 0) //is active
    {
      char s[16];
      IPAddress apIP = WiFi.softAPIP();
      sprintf(s, "%d.%d.%d.%d", apIP[0], apIP[1], apIP[2], apIP[3]);
      sappends('m',SET_F("(\"sip\")[1]"),s);
    } else
    {
      sappends('m',SET_F("(\"sip\")[1]"),(char*)F("Not active"));
    }
  }

  if (subPage == 2) {
    #ifdef ESP8266
    #if LEDPIN == 3
    oappend(SET_F("d.Sf.LC.max=500;"));
    #else
    oappend(SET_F("d.Sf.LC.max=1500;"));
    #endif
    #endif
    sappend('v',SET_F("LC"),ledCount);
    sappend('v',SET_F("MA"),strip.ablMilliampsMax);
    sappend('v',SET_F("LA"),strip.milliampsPerLed);
    if (strip.currentMilliamps)
    {
      sappends('m',SET_F("(\"pow\")[0]"),"");
      olen -= 2; //delete ";
      oappendi(strip.currentMilliamps);
      oappend(SET_F("mA\";"));
    }

    sappend('v',SET_F("CA"),briS);
    sappend('c',SET_F("EW"),useRGBW);
    sappend('i',SET_F("CO"),strip.getColorOrder());
    sappend('v',SET_F("AW"),strip.rgbwMode);

    sappend('c',SET_F("BO"),turnOnAtBoot);
    sappend('v',SET_F("BP"),bootPreset);

    sappend('c',SET_F("GB"),strip.gammaCorrectBri);
    sappend('c',SET_F("GC"),strip.gammaCorrectCol);
    sappend('c',SET_F("TF"),fadeTransition);
    sappend('v',SET_F("TD"),transitionDelayDefault);
    sappend('c',SET_F("PF"),strip.paletteFade);
    sappend('v',SET_F("BF"),briMultiplier);
    sappend('v',SET_F("TB"),nightlightTargetBri);
    sappend('v',SET_F("TL"),nightlightDelayMinsDefault);
    sappend('v',SET_F("TW"),nightlightMode);
    sappend('i',SET_F("PB"),strip.paletteBlend);
    sappend('c',SET_F("RV"),strip.reverseMode);
    sappend('c',SET_F("SL"),skipFirstLed);
  }

  if (subPage == 3)
  {
    sappends('s',SET_F("DS"),serverDescription);
    sappend('c',SET_F("ST"),syncToggleReceive);
  }

  if (subPage == 4)
  {
    sappend('c',SET_F("BT"),buttonEnabled);
    sappend('v',SET_F("IR"),irEnabled);
    sappend('v',SET_F("UP"),udpPort);
    sappend('v',SET_F("U2"),udpPort2);
    sappend('c',SET_F("RB"),receiveNotificationBrightness);
    sappend('c',SET_F("RC"),receiveNotificationColor);
    sappend('c',SET_F("RX"),receiveNotificationEffects);
    sappend('c',SET_F("SD"),notifyDirectDefault);
    sappend('c',SET_F("SB"),notifyButton);
    sappend('c',SET_F("SH"),notifyHue);
    sappend('c',SET_F("SM"),notifyMacro);
    sappend('c',SET_F("S2"),notifyTwice);
    sappend('c',SET_F("RD"),receiveDirect);
    sappend('v',SET_F("EP"),e131Port);
    sappend('c',SET_F("ES"),e131SkipOutOfSequence);
    sappend('c',SET_F("EM"),e131Multicast);
    sappend('v',SET_F("EU"),e131Universe);
    sappend('v',SET_F("DA"),DMXAddress);
    sappend('v',SET_F("DM"),DMXMode);
    sappend('v',SET_F("ET"),realtimeTimeoutMs);
    sappend('c',SET_F("FB"),arlsForceMaxBri);
    sappend('c',SET_F("RG"),arlsDisableGammaCorrection);
    sappend('v',SET_F("WO"),arlsOffset);
    sappend('c',SET_F("AL"),alexaEnabled);
    sappends('s',SET_F("AI"),alexaInvocationName);
    sappend('c',SET_F("SA"),notifyAlexa);
    sappends('s',SET_F("BK"),(char*)((blynkEnabled)?SET_F("Hidden"):""));
    sappends('s',SET_F("BH"),blynkHost);
    sappend('v',SET_F("BP"),blynkPort);

    #ifdef WLED_ENABLE_MQTT
    sappend('c',SET_F("MQ"),mqttEnabled);
    sappends('s',SET_F("MS"),mqttServer);
    sappend('v',SET_F("MQPORT"),mqttPort);
    sappends('s',SET_F("MQUSER"),mqttUser);
    byte l = strlen(mqttPass);
    char fpass[l+1]; //fill password field with ***
    fpass[l] = 0;
    memset(fpass,'*',l);
    sappends('s',SET_F("MQPASS"),fpass);
    sappends('s',SET_F("MQCID"),mqttClientID);
    sappends('s',SET_F("MD"),mqttDeviceTopic);
    sappends('s',SET_F("MG"),mqttGroupTopic);
    #endif

    #ifndef WLED_DISABLE_HUESYNC
    sappend('v',SET_F("H0"),hueIP[0]);
    sappend('v',SET_F("H1"),hueIP[1]);
    sappend('v',SET_F("H2"),hueIP[2]);
    sappend('v',SET_F("H3"),hueIP[3]);
    sappend('v',SET_F("HL"),huePollLightId);
    sappend('v',SET_F("HI"),huePollIntervalMs);
    sappend('c',SET_F("HP"),huePollingEnabled);
    sappend('c',SET_F("HO"),hueApplyOnOff);
    sappend('c',SET_F("HB"),hueApplyBri);
    sappend('c',SET_F("HC"),hueApplyColor);
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
      default: sprintf(hueErrorString,(char*)F("Bridge Error %i"),hueError);
    }
    
    sappends('m',SET_F("(\"sip\")[0]"),hueErrorString);
    #endif
  }

  if (subPage == 5)
  {
    sappend('c',SET_F("NT"),ntpEnabled);
    sappends('s',SET_F("NS"),ntpServerName);
    sappend('c',SET_F("CF"),!useAMPM);
    sappend('i',SET_F("TZ"),currentTimezone);
    sappend('v',SET_F("UO"),utcOffsetSecs);
    char tm[32];
    getTimeString(tm);
    sappends('m',SET_F("(\"times\")[0]"),tm);
    sappend('i',SET_F("OL"),overlayCurrent);
    sappend('v',SET_F("O1"),overlayMin);
    sappend('v',SET_F("O2"),overlayMax);
    sappend('v',SET_F("OM"),analogClock12pixel);
    sappend('c',SET_F("OS"),analogClockSecondsTrail);
    sappend('c',SET_F("O5"),analogClock5MinuteMarks);
    sappends('s',SET_F("CX"),cronixieDisplay);
    sappend('c',SET_F("CB"),cronixieBacklight);
    sappend('c',SET_F("CE"),countdownMode);
    sappend('v',SET_F("CY"),countdownYear);
    sappend('v',SET_F("CI"),countdownMonth);
    sappend('v',SET_F("CD"),countdownDay);
    sappend('v',SET_F("CH"),countdownHour);
    sappend('v',SET_F("CM"),countdownMin);
    sappend('v',SET_F("CS"),countdownSec);

    sappend('v',SET_F("A0"),macroAlexaOn);
    sappend('v',SET_F("A1"),macroAlexaOff);
    sappend('v',SET_F("MP"),macroButton);
    sappend('v',SET_F("ML"),macroLongPress);
    sappend('v',SET_F("MC"),macroCountdown);
    sappend('v',SET_F("MN"),macroNl);
    sappend('v',SET_F("MD"),macroDoublePress);

    char k[4];
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
    sappend('c',SET_F("NO"),otaLock);
    sappend('c',SET_F("OW"),wifiLock);
    sappend('c',SET_F("AO"),aOtaEnabled);
    sappends('m',SET_F("(\"sip\")[0]"),(char*)F("WLED "));
    olen -= 2; //delete ";
    oappend(versionString);
    oappend(SET_F(" (build "));
    oappendi(VERSION);
    oappend(SET_F(")\";"));
  }
  
  #ifdef WLED_ENABLE_DMX // include only if DMX is enabled
  if (subPage == 7)
  {
    sappend('v',SET_F("PU"),e131ProxyUniverse);
    
    sappend('v',SET_F("CN"),DMXChannels);
    sappend('v',SET_F("CG"),DMXGap);
    sappend('v',SET_F("CS"),DMXStart);
    sappend('v',SET_F("SL"),DMXStartLED);
    
    sappend('i',SET_F("CH1"),DMXFixtureMap[0]);
    sappend('i',SET_F("CH2"),DMXFixtureMap[1]);
    sappend('i',SET_F("CH3"),DMXFixtureMap[2]);
    sappend('i',SET_F("CH4"),DMXFixtureMap[3]);
    sappend('i',SET_F("CH5"),DMXFixtureMap[4]);
    sappend('i',SET_F("CH6"),DMXFixtureMap[5]);
    sappend('i',SET_F("CH7"),DMXFixtureMap[6]);
    sappend('i',SET_F("CH8"),DMXFixtureMap[7]);
    sappend('i',SET_F("CH9"),DMXFixtureMap[8]);
    sappend('i',SET_F("CH10"),DMXFixtureMap[9]);
    sappend('i',SET_F("CH11"),DMXFixtureMap[10]);
    sappend('i',SET_F("CH12"),DMXFixtureMap[11]);
    sappend('i',SET_F("CH13"),DMXFixtureMap[12]);
    sappend('i',SET_F("CH14"),DMXFixtureMap[13]);
    sappend('i',SET_F("CH15"),DMXFixtureMap[14]);
    }
  #endif
  oappend(SET_F("}</script>"));
}
