#include "wled.h"

/*
 * Receives client input
 */

void _setRandomColor(bool _sec,bool fromButton)
{
  lastRandomIndex = strip.get_random_wheel_index(lastRandomIndex);
  if (_sec){
    colorHStoRGB(lastRandomIndex*256,255,colSec);
  } else {
    colorHStoRGB(lastRandomIndex*256,255,col);
  }
  if (fromButton) colorUpdated(2);
}


bool isAsterisksOnly(const char* str, byte maxLen)
{
  for (byte i = 0; i < maxLen; i++) {
    if (str[i] == 0) break;
    if (str[i] != '*') return false;
  }
  return true;
}


//called upon POST settings form submit
void handleSettingsSet(AsyncWebServerRequest *request, byte subPage)
{

  //0: menu 1: wifi 2: leds 3: ui 4: sync 5: time 6: sec 7: DMX
  if (subPage <1 || subPage >7) return;

  //WIFI SETTINGS
  if (subPage == 1)
  {
    strlcpy(clientSSID,request->arg("CS").c_str(), 33);

    if (!isAsterisksOnly(request->arg("CP").c_str(), 65)) strlcpy(clientPass, request->arg("CP").c_str(), 65);

    strlcpy(cmDNS, request->arg("CM").c_str(), 33);

    apBehavior = request->arg("AB").toInt();
    strlcpy(apSSID, request->arg("AS").c_str(), 33);
    apHide = request->hasArg("AH");
    int passlen = request->arg("AP").length();
    if (passlen == 0 || (passlen > 7 && !isAsterisksOnly(request->arg("AP").c_str(), 65))) strlcpy(apPass, request->arg("AP").c_str(), 65);
    int t = request->arg("AC").toInt(); if (t > 0 && t < 14) apChannel = t;

    noWifiSleep = request->hasArg("WS");

    char k[3]; k[2] = 0;
    for (int i = 0; i<4; i++)
    {
      k[1] = i+48;//ascii 0,1,2,3

      k[0] = 'I'; //static IP
      staticIP[i] = request->arg(k).toInt();

      k[0] = 'G'; //gateway
      staticGateway[i] = request->arg(k).toInt();

      k[0] = 'S'; //subnet
      staticSubnet[i] = request->arg(k).toInt();
    }
  }

  //LED SETTINGS
  if (subPage == 2)
  {
    int t = request->arg("LC").toInt();
    if (t > 0 && t <= MAX_LEDS) ledCount = t;
    #ifdef ESP8266
    #if LEDPIN == 3
    if (ledCount > MAX_LEDS_DMA) ledCount = MAX_LEDS_DMA; //DMA method uses too much ram
    #endif
    #endif
    strip.ablMilliampsMax = request->arg("MA").toInt();
    strip.milliampsPerLed = request->arg("LA").toInt();
    
    useRGBW = request->hasArg("EW");
    strip.colorOrder = request->arg("CO").toInt();
    strip.rgbwMode = request->arg("AW").toInt();

    briS = request->arg("CA").toInt();

    saveCurrPresetCycConf = request->hasArg("PC");
    turnOnAtBoot = request->hasArg("BO");
    t = request->arg("BP").toInt();
    if (t <= 25) bootPreset = t;
    strip.gammaCorrectBri = request->hasArg("GB");
    strip.gammaCorrectCol = request->hasArg("GC");

    fadeTransition = request->hasArg("TF");
    t = request->arg("TD").toInt();
    if (t > 0) transitionDelay = t;
    transitionDelayDefault = t;
    strip.paletteFade = request->hasArg("PF");

    t = request->arg("SQ").toInt();
    if (t > 0) soundSquelch = t;
    
    nightlightTargetBri = request->arg("TB").toInt();
    t = request->arg("TL").toInt();
    if (t > 0) nightlightDelayMinsDefault = t;
    nightlightDelayMins = nightlightDelayMinsDefault;
    nightlightMode = request->arg("TW").toInt();

    t = request->arg("PB").toInt();
    if (t >= 0 && t < 4) strip.paletteBlend = t;
    strip.reverseMode = request->hasArg("RV");
    skipFirstLed = request->hasArg("SL");
    t = request->arg("BF").toInt();
    if (t > 0) briMultiplier = t;

    #ifndef ESP8266
    strip.matrixWidth = request->arg("LCW").toInt();
    strip.matrixHeight = request->arg("LCH").toInt();
    strip.matrixSerpentine = request->hasArg("LCWHS");
    #endif // ESP8266
  }

  //UI
  if (subPage == 3)
  {
    strlcpy(serverDescription, request->arg("DS").c_str(), 33);
    syncToggleReceive = request->hasArg("ST");
  }

  //SYNC
  if (subPage == 4)
  {
    buttonEnabled = request->hasArg("BT");
    irEnabled = request->arg("IR").toInt();
    int t = request->arg("UP").toInt();
    if (t > 0) udpPort = t;
    receiveNotificationBrightness = request->hasArg("RB");
    receiveNotificationColor = request->hasArg("RC");
    receiveNotificationEffects = request->hasArg("RX");
    receiveNotifications = (receiveNotificationBrightness || receiveNotificationColor || receiveNotificationEffects);
    notifyDirectDefault = request->hasArg("SD");
    notifyDirect = notifyDirectDefault;
    notifyButton = request->hasArg("SB");
    notifyAlexa = request->hasArg("SA");
    notifyHue = request->hasArg("SH");
    notifyMacro = request->hasArg("SM");
    notifyTwice = request->hasArg("S2");

    receiveDirect = request->hasArg("RD");
    e131SkipOutOfSequence = request->hasArg("ES");
    e131Multicast = request->hasArg("EM");
    t = request->arg("EP").toInt();
    if (t > 0) e131Port = t;
    t = request->arg("EU").toInt();
    if (t >= 0  && t <= 63999) e131Universe = t;
    t = request->arg("DA").toInt();
    if (t >= 0  && t <= 510) DMXAddress = t;
    t = request->arg("DM").toInt();
    if (t >= DMX_MODE_DISABLED && t <= DMX_MODE_MULTIPLE_DRGB) DMXMode = t;
    t = request->arg("ET").toInt();
    if (t > 99  && t <= 65000) realtimeTimeoutMs = t;
    arlsForceMaxBri = request->hasArg("FB");
    arlsDisableGammaCorrection = request->hasArg("RG");
    t = request->arg("WO").toInt();
    if (t >= -255  && t <= 255) arlsOffset = t;

    alexaEnabled = request->hasArg("AL");
    strlcpy(alexaInvocationName, request->arg("AI").c_str(), 33);

    if (request->hasArg("BK") && !request->arg("BK").equals("Hidden")) {
      strlcpy(blynkApiKey, request->arg("BK").c_str(), 36); initBlynk(blynkApiKey);
    }
    t = request->arg("ASE").toInt();
    if (t == 0) {
      // 0 == udp audio sync off
      Serial.print("Setting audio sync settings");
      audioSyncEnabled &= ~(1 << 0);
      audioSyncEnabled &= ~(1 << 1);
    }
    else if (t == 1) {
      // 1 == transmit only
      Serial.print("Setting audio sync settings");
      audioSyncEnabled |= 1 << 0;
      audioSyncEnabled &= ~(1 << 1);
    }
    else if (t == 2) {
      // 2 == receive only
      Serial.print("Setting audio sync settings");
      audioSyncEnabled &= ~(1 << 0);
      audioSyncEnabled |= 1 << 1;
    }
    Serial.print(audioSyncEnabled);
    t = request->arg("ASP").toInt();
    audioSyncPort = t;

    #ifdef WLED_ENABLE_MQTT
    mqttEnabled = request->hasArg("MQ");
    strlcpy(mqttServer, request->arg("MS").c_str(), 33);
    t = request->arg("MQPORT").toInt();
    if (t > 0) mqttPort = t;
    strlcpy(mqttUser, request->arg("MQUSER").c_str(), 41);
    if (!isAsterisksOnly(request->arg("MQPASS").c_str(), 41)) strlcpy(mqttPass, request->arg("MQPASS").c_str(), 41);
    strlcpy(mqttClientID, request->arg("MQCID").c_str(), 41);
    strlcpy(mqttDeviceTopic, request->arg("MD").c_str(), 33);
    strlcpy(mqttGroupTopic, request->arg("MG").c_str(), 33);
    #endif

    #ifndef WLED_DISABLE_HUESYNC
    for (int i=0;i<4;i++){
      String a = "H"+String(i);
      hueIP[i] = request->arg(a).toInt();
    }

    t = request->arg("HL").toInt();
    if (t > 0) huePollLightId = t;

    t = request->arg("HI").toInt();
    if (t > 50) huePollIntervalMs = t;

    hueApplyOnOff = request->hasArg("HO");
    hueApplyBri = request->hasArg("HB");
    hueApplyColor = request->hasArg("HC");
    huePollingEnabled = request->hasArg("HP");
    hueStoreAllowed = true;
    reconnectHue();
    #endif
  }

  //TIME
  if (subPage == 5)
  {
    ntpEnabled = request->hasArg("NT");
    strlcpy(ntpServerName, request->arg("NS").c_str(), 33);
    useAMPM = !request->hasArg("CF");
    currentTimezone = request->arg("TZ").toInt();
    utcOffsetSecs = request->arg("UO").toInt();

    //start ntp if not already connected
    if (ntpEnabled && WLED_CONNECTED && !ntpConnected) ntpConnected = ntpUdp.begin(ntpLocalPort);

    if (request->hasArg("OL")){
      overlayDefault = request->arg("OL").toInt();
      overlayCurrent = overlayDefault;
    }

    overlayMin = request->arg("O1").toInt();
    overlayMax = request->arg("O2").toInt();
    analogClock12pixel = request->arg("OM").toInt();
    analogClock5MinuteMarks = request->hasArg("O5");
    analogClockSecondsTrail = request->hasArg("OS");

    strcpy(cronixieDisplay,request->arg("CX").c_str());
    cronixieBacklight = request->hasArg("CB");
    countdownMode = request->hasArg("CE");
    countdownYear = request->arg("CY").toInt();
    countdownMonth = request->arg("CI").toInt();
    countdownDay = request->arg("CD").toInt();
    countdownHour = request->arg("CH").toInt();
    countdownMin = request->arg("CM").toInt();
    countdownSec = request->arg("CS").toInt();

    for (int i=1;i<17;i++)
    {
      String a = "M"+String(i);
      if (request->hasArg(a.c_str())) saveMacro(i,request->arg(a),false);
    }

    macroBoot = request->arg("MB").toInt();
    macroAlexaOn = request->arg("A0").toInt();
    macroAlexaOff = request->arg("A1").toInt();
    macroButton = request->arg("MP").toInt();
    macroLongPress = request->arg("ML").toInt();
    macroCountdown = request->arg("MC").toInt();
    macroNl = request->arg("MN").toInt();
    macroDoublePress = request->arg("MD").toInt();

    char k[3]; k[2] = 0;
    for (int i = 0; i<8; i++)
    {
      k[1] = i+48;//ascii 0,1,2,3

      k[0] = 'H'; //timer hours
      timerHours[i] = request->arg(k).toInt();

      k[0] = 'N'; //minutes
      timerMinutes[i] = request->arg(k).toInt();

      k[0] = 'T'; //macros
      timerMacro[i] = request->arg(k).toInt();

      k[0] = 'W'; //weekdays
      timerWeekday[i] = request->arg(k).toInt();
    }
  }

  //SECURITY
  if (subPage == 6)
  {
    if (request->hasArg("RS")) //complete factory reset
    {
      clearEEPROM();
      serveMessage(request, 200, "All Settings erased.", "Connect to WLED-AP to setup again",255);
      doReboot = true;
    }

    bool pwdCorrect = !otaLock; //always allow access if ota not locked
    if (request->hasArg("OP"))
    {
      if (otaLock && strcmp(otaPass,request->arg("OP").c_str()) == 0)
      {
        pwdCorrect = true;
      }
      if (!otaLock && request->arg("OP").length() > 0)
      {
        strlcpy(otaPass,request->arg("OP").c_str(), 33);
      }
    }

    if (pwdCorrect) //allow changes if correct pwd or no ota active
    {
      otaLock = request->hasArg("NO");
      wifiLock = request->hasArg("OW");
      aOtaEnabled = request->hasArg("AO");
    }
  }
  #ifdef WLED_ENABLE_DMX // include only if DMX is enabled
  if (subPage == 7)
  {
    int t = request->arg("PU").toInt();
    if (t >= 0  && t <= 63999) e131ProxyUniverse = t;

    t = request->arg("CN").toInt();
    if (t>0 && t<16) {
      DMXChannels = t;
    }
    t = request->arg("CS").toInt();
    if (t>0 && t<513) {
      DMXStart = t;
    }
    t = request->arg("CG").toInt();
    if (t>0 && t<513) {
      DMXGap = t;
    }
    t = request->arg("SL").toInt();
    if (t>=0 && t < MAX_LEDS) {
      DMXStartLED = t;
    }
    for (int i=0; i<15; i++) {
      String argname = "CH" + String((i+1));
      t = request->arg(argname).toInt();
      DMXFixtureMap[i] = t;
    }
  }
  
  #endif
  if (subPage != 6 || !doReboot) saveSettingsToEEPROM(); //do not save if factory reset
  if (subPage == 2) {
    strip.init(useRGBW,ledCount,skipFirstLed);
  }
  if (subPage == 4) alexaInit();
}



//helper to get int value at a position in string
int getNumVal(const String* req, uint16_t pos)
{
  return req->substring(pos+3).toInt();
}


//helper to get int value at a position in string
bool updateVal(const String* req, const char* key, byte* val, byte minv, byte maxv)
{
  int pos = req->indexOf(key);
  if (pos < 1) return false;

  if (req->charAt(pos+3) == '~') {
    int out = getNumVal(req, pos+1);
    if (out == 0)
    {
      if (req->charAt(pos+4) == '-')
      {
        *val = (*val <= minv)? maxv : *val -1;
      } else {
        *val = (*val >= maxv)? minv : *val +1;
      }
    } else {
      out += *val;
      if (out > maxv) out = maxv;
      if (out < minv) out = minv;
      *val = out;
    }
  } else
  {
    *val = getNumVal(req, pos);
  }
  return true;
}


//HTTP API request parser
bool handleSet(AsyncWebServerRequest *request, const String& req)
{
  if (!(req.indexOf("win") >= 0)) return false;

  int pos = 0;
  DEBUG_PRINT("API req: ");
  DEBUG_PRINTLN(req);

  //write presets and macros saved to flash directly?
  bool persistSaves = true;
  pos = req.indexOf("NP");
  if (pos > 0) {
    persistSaves = false;
  }

  //save macro, requires &MS=<slot>(<macro>) format
  pos = req.indexOf("&MS=");
  if (pos > 0) {
    int i = req.substring(pos + 4).toInt();
    pos = req.indexOf('(') +1;
    if (pos > 0) {
      int en = req.indexOf(')');
      String mc = req.substring(pos);
      if (en > 0) mc = req.substring(pos, en);
      saveMacro(i, mc, persistSaves);
    }

    pos = req.indexOf("IN");
    if (pos < 1) XML_response(request);
    return true;
    //if you save a macro in one request, other commands in that request are ignored due to unwanted behavior otherwise
  }

  strip.applyToAllSelected = true;

  //segment select (sets main segment)
  byte prevMain = strip.getMainSegmentId();
  pos = req.indexOf("SM=");
  if (pos > 0) {
    strip.mainSegment = getNumVal(&req, pos);
  }
  byte main = strip.getMainSegmentId();
  if (main != prevMain) setValuesFromMainSeg();

  pos = req.indexOf("SS=");
  if (pos > 0) {
    byte t = getNumVal(&req, pos);
    if (t < strip.getMaxSegments()) main = t;
  }

  WS2812FX::Segment& mainseg = strip.getSegment(main);
  pos = req.indexOf("SV="); //segment selected
  if (pos > 0) {
    byte t = getNumVal(&req, pos);
    if (t == 2) {
      for (uint8_t i = 0; i < strip.getMaxSegments(); i++)
      {
        strip.getSegment(i).setOption(SEG_OPTION_SELECTED, 0);
      }
    }
    mainseg.setOption(SEG_OPTION_SELECTED, t);
  }

  uint16_t startI = mainseg.start;
  uint16_t stopI = mainseg.stop;
  uint8_t grpI = mainseg.grouping;
  uint16_t spcI = mainseg.spacing;
  pos = req.indexOf("&S="); //segment start
  if (pos > 0) {
    startI = getNumVal(&req, pos);
  }
  pos = req.indexOf("S2="); //segment stop
  if (pos > 0) {
    stopI = getNumVal(&req, pos);
  }
  pos = req.indexOf("GP="); //segment grouping
  if (pos > 0) {
    grpI = getNumVal(&req, pos);
    if (grpI == 0) grpI = 1;
  }
  pos = req.indexOf("SP="); //segment spacing
  if (pos > 0) {
    spcI = getNumVal(&req, pos);
  }
  strip.setSegment(main, startI, stopI, grpI, spcI);

  main = strip.getMainSegmentId();

   //set presets
  pos = req.indexOf("P1="); //sets first preset for cycle
  if (pos > 0) presetCycleMin = getNumVal(&req, pos);

  pos = req.indexOf("P2="); //sets last preset for cycle
  if (pos > 0) presetCycleMax = getNumVal(&req, pos);

  //preset cycle
  pos = req.indexOf("CY=");
  if (pos > 0)
  {
    presetCyclingEnabled = (req.charAt(pos+3) != '0');
    presetCycCurr = presetCycleMin;
  }

  pos = req.indexOf("PT="); //sets cycle time in ms
  if (pos > 0) {
    int v = getNumVal(&req, pos);
    if (v > 100) presetCycleTime = v/100;
  }

  pos = req.indexOf("PA="); //apply brightness from preset
  if (pos > 0) presetApplyBri = (req.charAt(pos+3) != '0');

  pos = req.indexOf("PS="); //saves current in preset
  if (pos > 0) savePreset(getNumVal(&req, pos), persistSaves);

  //apply preset
  if (updateVal(&req, "PL=", &presetCycCurr, presetCycleMin, presetCycleMax)) {
    applyPreset(presetCycCurr, presetApplyBri);
  }

  //set brightness
  updateVal(&req, "&A=", &bri);

  //set colors
  updateVal(&req, "&R=", &col[0]);
  updateVal(&req, "&G=", &col[1]);
  updateVal(&req, "&B=", &col[2]);
  updateVal(&req, "&W=", &col[3]);
  updateVal(&req, "R2=", &colSec[0]);
  updateVal(&req, "G2=", &colSec[1]);
  updateVal(&req, "B2=", &colSec[2]);
  updateVal(&req, "W2=", &colSec[3]);

  //set hue
  pos = req.indexOf("HU=");
  if (pos > 0) {
    uint16_t temphue = getNumVal(&req, pos);
    byte tempsat = 255;
    pos = req.indexOf("SA=");
    if (pos > 0) {
      tempsat = getNumVal(&req, pos);
    }
    colorHStoRGB(temphue,tempsat,(req.indexOf("H2")>0)? colSec:col);
  }

  //set color from HEX or 32bit DEC
  pos = req.indexOf("CL=");
  if (pos > 0) {
    colorFromDecOrHexString(col, (char*)req.substring(pos + 3).c_str());
  }
  pos = req.indexOf("C2=");
  if (pos > 0) {
    colorFromDecOrHexString(colSec, (char*)req.substring(pos + 3).c_str());
  }
  pos = req.indexOf("C3=");
  if (pos > 0) {
    byte t[4];
    colorFromDecOrHexString(t, (char*)req.substring(pos + 3).c_str());
    strip.setColor(2, t[0], t[1], t[2], t[3]);
  }

  //set to random hue SR=0->1st SR=1->2nd
  pos = req.indexOf("SR");
  if (pos > 0) {
    _setRandomColor(getNumVal(&req, pos));
  }

  //swap 2nd & 1st
  pos = req.indexOf("SC");
  if (pos > 0) {
    byte temp;
    for (uint8_t i=0; i<4; i++)
    {
      temp = col[i];
      col[i] = colSec[i];
      colSec[i] = temp;
    }
  }

  //set effect parameters
  if (updateVal(&req, "FX=", &effectCurrent, 0, strip.getModeCount()-1)) presetCyclingEnabled = false;
  updateVal(&req, "SX=", &effectSpeed);
  updateVal(&req, "IX=", &effectIntensity);
  updateVal(&req, "F1=", &effectFFT1);
  updateVal(&req, "F2=", &effectFFT2);
  updateVal(&req, "F3=", &effectFFT3);
  updateVal(&req, "FP=", &effectPalette, 0, strip.getPaletteCount()-1);

  //set advanced overlay
  pos = req.indexOf("OL=");
  if (pos > 0) {
    overlayCurrent = getNumVal(&req, pos);
  }

  //apply macro
  pos = req.indexOf("&M=");
  if (pos > 0) {
    applyMacro(getNumVal(&req, pos));
  }

  //toggle send UDP direct notifications
  pos = req.indexOf("SN=");
  if (pos > 0) notifyDirect = (req.charAt(pos+3) != '0');

  //toggle receive UDP direct notifications
  pos = req.indexOf("RN=");
  if (pos > 0) receiveNotifications = (req.charAt(pos+3) != '0');

  //receive live data via UDP/Hyperion
  pos = req.indexOf("RD=");
  if (pos > 0) receiveDirect = (req.charAt(pos+3) != '0');

  //toggle nightlight mode
  bool aNlDef = false;
  if (req.indexOf("&ND") > 0) aNlDef = true;
  pos = req.indexOf("NL=");
  if (pos > 0)
  {
    if (req.charAt(pos+3) == '0')
    {
      nightlightActive = false;
      bri = briT;
    } else {
      nightlightActive = true;
      if (!aNlDef) nightlightDelayMins = getNumVal(&req, pos);
      nightlightStartTime = millis();
    }
  } else if (aNlDef)
  {
    nightlightActive = true;
    nightlightStartTime = millis();
  }

  //set nightlight target brightness
  pos = req.indexOf("NT=");
  if (pos > 0) {
    nightlightTargetBri = getNumVal(&req, pos);
    nightlightActiveOld = false; //re-init
  }

  //toggle nightlight fade
  pos = req.indexOf("NF=");
  if (pos > 0)
  {
    nightlightMode = getNumVal(&req, pos);

    nightlightActiveOld = false; //re-init
  }
  if (nightlightMode > NL_MODE_SUN) nightlightMode = NL_MODE_SUN;

  #if AUXPIN >= 0
  //toggle general purpose output
  pos = req.indexOf("AX=");
  if (pos > 0) {
    auxTime = getNumVal(&req, pos);
    auxActive = true;
    if (auxTime == 0) auxActive = false;
  }
  #endif

  pos = req.indexOf("TT=");
  if (pos > 0) transitionDelay = getNumVal(&req, pos);

  //main toggle on/off
  pos = req.indexOf("&T=");
  if (pos > 0) {
    nightlightActive = false; //always disable nightlight when toggling
    switch (getNumVal(&req, pos))
    {
      case 0: if (bri != 0){briLast = bri; bri = 0;} break; //off, only if it was previously on
      case 1: if (bri == 0) bri = briLast; break; //on, only if it was previously off
      default: toggleOnOff(); //toggle
    }
  }

  //Segment reverse
  pos = req.indexOf("RV=");
  if (pos > 0) strip.getSegment(main).setOption(SEG_OPTION_REVERSED, req.charAt(pos+3) != '0');

  //Segment reverse
  pos = req.indexOf("MI=");
  if (pos > 0) strip.getSegment(main).setOption(SEG_OPTION_MIRROR, req.charAt(pos+3) != '0');

  //Segment brightness/opacity
  pos = req.indexOf("SB=");
  if (pos > 0) {
    byte segbri = getNumVal(&req, pos);
    strip.getSegment(main).setOption(SEG_OPTION_ON, segbri);
    if (segbri) {
      strip.getSegment(main).opacity = segbri;
    }
  }

  //set time (unix timestamp)
  pos = req.indexOf("ST=");
  if (pos > 0) {
    setTime(getNumVal(&req, pos));
  }

  //set countdown goal (unix timestamp)
  pos = req.indexOf("CT=");
  if (pos > 0) {
    countdownTime = getNumVal(&req, pos);
    if (countdownTime - now() > 0) countdownOverTriggered = false;
  }

  pos = req.indexOf("LO=");
  if (pos > 0) {
    realtimeOverride = getNumVal(&req, pos);
    if (realtimeOverride > 2) realtimeOverride = REALTIME_OVERRIDE_ALWAYS;
  }

  pos = req.indexOf("RB");
  if (pos > 0) doReboot = true;

  //cronixie
  #ifndef WLED_DISABLE_CRONIXIE
  //mode, 1 countdown
  pos = req.indexOf("NM=");
  if (pos > 0) countdownMode = (req.charAt(pos+3) != '0');
  
  pos = req.indexOf("NX="); //sets digits to code
  if (pos > 0) {
    strlcpy(cronixieDisplay, req.substring(pos + 3, pos + 9).c_str(), 6);
    setCronixie();
  }

  pos = req.indexOf("NB=");
  if (pos > 0) //sets backlight
  {
    cronixieBacklight = (req.charAt(pos+3) != '0');
    overlayRefreshedTime = 0;
  }
  #endif

  pos = req.indexOf("U0="); //user var 0
  if (pos > 0) {
    userVar0 = getNumVal(&req, pos);
  }

  pos = req.indexOf("U1="); //user var 1
  if (pos > 0) {
    userVar1 = getNumVal(&req, pos);
  }
  //you can add more if you need

  pos = req.indexOf("DX="); // delay in ms  050720 ajn
  if (pos > 0) delay(getNumVal(&req,pos));
 
  //internal call, does not send XML response
  pos = req.indexOf("IN");
  if (pos < 1) XML_response(request);

  pos = req.indexOf("&NN"); //do not send UDP notifications this time
  colorUpdated((pos > 0) ? NOTIFIER_CALL_MODE_NO_NOTIFY : NOTIFIER_CALL_MODE_DIRECT_CHANGE);


  
  return true;
}
