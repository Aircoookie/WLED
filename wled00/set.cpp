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
  //at this point the password contains asterisks only
  return (str[0] != 0); //false on empty string
}


//called upon POST settings form submit
void handleSettingsSet(AsyncWebServerRequest *request, byte subPage)
{

  //0: menu 1: wifi 2: leds 3: ui 4: sync 5: time 6: sec 7: DMX
  if (subPage <1 || subPage >7) return;

  //WIFI SETTINGS
  if (subPage == 1)
  {
    strlcpy(clientSSID,request->arg(F("CS")).c_str(), 33);

    if (!isAsterisksOnly(request->arg(F("CP")).c_str(), 65)) strlcpy(clientPass, request->arg(F("CP")).c_str(), 65);

    strlcpy(cmDNS, request->arg(F("CM")).c_str(), 33);

    apBehavior = request->arg(F("AB")).toInt();
    strlcpy(apSSID, request->arg(F("AS")).c_str(), 33);
    apHide = request->hasArg(F("AH"));
    int passlen = request->arg(F("AP")).length();
    if (passlen == 0 || (passlen > 7 && !isAsterisksOnly(request->arg(F("AP")).c_str(), 65))) strlcpy(apPass, request->arg(F("AP")).c_str(), 65);
    int t = request->arg(F("AC")).toInt(); if (t > 0 && t < 14) apChannel = t;

    noWifiSleep = request->hasArg(F("WS"));

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
    int t = request->arg(F("LC")).toInt();
    if (t > 0 && t <= MAX_LEDS) ledCount = t;
    #ifdef ESP8266
    #if LEDPIN == 3
    if (ledCount > MAX_LEDS_DMA) ledCount = MAX_LEDS_DMA; //DMA method uses too much ram
    #endif
    #endif
    strip.ablMilliampsMax = request->arg(F("MA")).toInt();
    strip.milliampsPerLed = request->arg(F("LA")).toInt();
    
    useRGBW = request->hasArg(F("EW"));
    strip.setColorOrder(request->arg(F("CO")).toInt());
    strip.rgbwMode = request->arg(F("AW")).toInt();

    briS = request->arg(F("CA")).toInt();

    saveCurrPresetCycConf = request->hasArg(F("PC"));
    turnOnAtBoot = request->hasArg(F("BO"));
    t = request->arg(F("BP")).toInt();
    if (t <= 25) bootPreset = t;
    strip.gammaCorrectBri = request->hasArg(F("GB"));
    strip.gammaCorrectCol = request->hasArg(F("GC"));

    fadeTransition = request->hasArg(F("TF"));
    t = request->arg(F("TD")).toInt();
    if (t > 0) transitionDelay = t;
    transitionDelayDefault = t;
    strip.paletteFade = request->hasArg(F("PF"));

    nightlightTargetBri = request->arg(F("TB")).toInt();
    t = request->arg(F("TL")).toInt();
    if (t > 0) nightlightDelayMinsDefault = t;
    nightlightDelayMins = nightlightDelayMinsDefault;
    nightlightMode = request->arg(F("TW")).toInt();

    t = request->arg(F("PB")).toInt();
    if (t >= 0 && t < 4) strip.paletteBlend = t;
    strip.reverseMode = request->hasArg(F("RV"));
    skipFirstLed = request->hasArg(F("SL"));
    t = request->arg(F("BF")).toInt();
    if (t > 0) briMultiplier = t;
  }

  //UI
  if (subPage == 3)
  {
    strlcpy(serverDescription, request->arg(F("DS")).c_str(), 33);
    syncToggleReceive = request->hasArg(F("ST"));
  }

  //SYNC
  if (subPage == 4)
  {
    buttonEnabled = request->hasArg(F("BT"));
    irEnabled = request->arg(F("IR")).toInt();
    int t = request->arg(F("UP")).toInt();
    if (t > 0) udpPort = t;
    t = request->arg(F("U2")).toInt();
    if (t > 0) udpPort2 = t;
    receiveNotificationBrightness = request->hasArg(F("RB"));
    receiveNotificationColor = request->hasArg(F("RC"));
    receiveNotificationEffects = request->hasArg(F("RX"));
    receiveNotifications = (receiveNotificationBrightness || receiveNotificationColor || receiveNotificationEffects);
    notifyDirectDefault = request->hasArg(F("SD"));
    notifyDirect = notifyDirectDefault;
    notifyButton = request->hasArg(F("SB"));
    notifyAlexa = request->hasArg(F("SA"));
    notifyHue = request->hasArg(F("SH"));
    notifyMacro = request->hasArg(F("SM"));
    notifyTwice = request->hasArg(F("S2"));

    receiveDirect = request->hasArg(F("RD"));
    e131SkipOutOfSequence = request->hasArg(F("ES"));
    e131Multicast = request->hasArg(F("EM"));
    t = request->arg(F("EP")).toInt();
    if (t > 0) e131Port = t;
    t = request->arg(F("EU")).toInt();
    if (t >= 0  && t <= 63999) e131Universe = t;
    t = request->arg(F("DA")).toInt();
    if (t >= 0  && t <= 510) DMXAddress = t;
    t = request->arg(F("DM")).toInt();
    if (t >= DMX_MODE_DISABLED && t <= DMX_MODE_MULTIPLE_RGBW) DMXMode = t;
    t = request->arg(F("ET")).toInt();
    if (t > 99  && t <= 65000) realtimeTimeoutMs = t;
    arlsForceMaxBri = request->hasArg(F("FB"));
    arlsDisableGammaCorrection = request->hasArg(F("RG"));
    t = request->arg(F("WO")).toInt();
    if (t >= -255  && t <= 255) arlsOffset = t;

    alexaEnabled = request->hasArg(F("AL"));
    strlcpy(alexaInvocationName, request->arg(F("AI")).c_str(), 33);

    if (request->hasArg("BK") && !request->arg("BK").equals(F("Hidden"))) {
      strlcpy(blynkApiKey, request->arg("BK").c_str(), 36); initBlynk(blynkApiKey);
    }

    #ifdef WLED_ENABLE_MQTT
    mqttEnabled = request->hasArg(F("MQ"));
    strlcpy(mqttServer, request->arg(F("MS")).c_str(), 33);
    t = request->arg(F("MQPORT")).toInt();
    if (t > 0) mqttPort = t;
    strlcpy(mqttUser, request->arg(F("MQUSER")).c_str(), 41);
    if (!isAsterisksOnly(request->arg(F("MQPASS")).c_str(), 41)) strlcpy(mqttPass, request->arg(F("MQPASS")).c_str(), 41);
    strlcpy(mqttClientID, request->arg(F("MQCID")).c_str(), 41);
    strlcpy(mqttDeviceTopic, request->arg(F("MD")).c_str(), 33);
    strlcpy(mqttGroupTopic, request->arg(F("MG")).c_str(), 33);
    #endif

    #ifndef WLED_DISABLE_HUESYNC
    for (int i=0;i<4;i++){
      String a = "H"+String(i);
      hueIP[i] = request->arg(a).toInt();
    }

    t = request->arg(F("HL")).toInt();
    if (t > 0) huePollLightId = t;

    t = request->arg(F("HI")).toInt();
    if (t > 50) huePollIntervalMs = t;

    hueApplyOnOff = request->hasArg(F("HO"));
    hueApplyBri = request->hasArg(F("HB"));
    hueApplyColor = request->hasArg(F("HC"));
    huePollingEnabled = request->hasArg(F("HP"));
    hueStoreAllowed = true;
    reconnectHue();
    #endif
  }

  //TIME
  if (subPage == 5)
  {
    ntpEnabled = request->hasArg(F("NT"));
    strlcpy(ntpServerName, request->arg(F("NS")).c_str(), 33);
    useAMPM = !request->hasArg(F("CF"));
    currentTimezone = request->arg(F("TZ")).toInt();
    utcOffsetSecs = request->arg(F("UO")).toInt();

    //start ntp if not already connected
    if (ntpEnabled && WLED_CONNECTED && !ntpConnected) ntpConnected = ntpUdp.begin(ntpLocalPort);

    if (request->hasArg(F("OL"))) {
      overlayDefault = request->arg(F("OL")).toInt();
      overlayCurrent = overlayDefault;
    }

    overlayMin = request->arg(F("O1")).toInt();
    overlayMax = request->arg(F("O2")).toInt();
    analogClock12pixel = request->arg(F("OM")).toInt();
    analogClock5MinuteMarks = request->hasArg(F("O5"));
    analogClockSecondsTrail = request->hasArg(F("OS"));

    strcpy(cronixieDisplay,request->arg(F("CX")).c_str());
    cronixieBacklight = request->hasArg(F("CB"));
    countdownMode = request->hasArg(F("CE"));
    countdownYear = request->arg(F("CY")).toInt();
    countdownMonth = request->arg(F("CI")).toInt();
    countdownDay = request->arg(F("CD")).toInt();
    countdownHour = request->arg(F("CH")).toInt();
    countdownMin = request->arg(F("CM")).toInt();
    countdownSec = request->arg(F("CS")).toInt();

    macroAlexaOn = request->arg(F("A0")).toInt();
    macroAlexaOff = request->arg(F("A1")).toInt();
    macroButton = request->arg(F("MP")).toInt();
    macroLongPress = request->arg(F("ML")).toInt();
    macroCountdown = request->arg(F("MC")).toInt();
    macroNl = request->arg(F("MN")).toInt();
    macroDoublePress = request->arg(F("MD")).toInt();

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
    if (request->hasArg(F("RS"))) //complete factory reset
    {
      WLED_FS.format();
      serveMessage(request, 200, F("All Settings erased."), F("Connect to WLED-AP to setup again"),255);
      doReboot = true;
    }

    bool pwdCorrect = !otaLock; //always allow access if ota not locked
    if (request->hasArg(F("OP")))
    {
      if (otaLock && strcmp(otaPass,request->arg(F("OP")).c_str()) == 0)
      {
        pwdCorrect = true;
      }
      if (!otaLock && request->arg(F("OP")).length() > 0)
      {
        strlcpy(otaPass,request->arg(F("OP")).c_str(), 33);
      }
    }

    if (pwdCorrect) //allow changes if correct pwd or no ota active
    {
      otaLock = request->hasArg(F("NO"));
      wifiLock = request->hasArg(F("OW"));
      aOtaEnabled = request->hasArg(F("AO"));
    }
  }
  #ifdef WLED_ENABLE_DMX // include only if DMX is enabled
  if (subPage == 7)
  {
    int t = request->arg(F("PU")).toInt();
    if (t >= 0  && t <= 63999) e131ProxyUniverse = t;

    t = request->arg(F("CN")).toInt();
    if (t>0 && t<16) {
      DMXChannels = t;
    }
    t = request->arg(F("CS")).toInt();
    if (t>0 && t<513) {
      DMXStart = t;
    }
    t = request->arg(F("CG")).toInt();
    if (t>0 && t<513) {
      DMXGap = t;
    }
    t = request->arg(F("SL")).toInt();
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
  if (subPage != 6 || !doReboot) serializeConfig(); //do not save if factory reset
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
bool handleSet(AsyncWebServerRequest *request, const String& req, bool apply)
{
  if (!(req.indexOf("win") >= 0)) return false;

  int pos = 0;
  DEBUG_PRINT(F("API req: "));
  DEBUG_PRINTLN(req);

  strip.applyToAllSelected = true;

  //segment select (sets main segment)
  byte prevMain = strip.getMainSegmentId();
  pos = req.indexOf(F("SM="));
  if (pos > 0) {
    strip.mainSegment = getNumVal(&req, pos);
  }
  byte main = strip.getMainSegmentId();
  if (main != prevMain) setValuesFromMainSeg();

  pos = req.indexOf(F("SS="));
  if (pos > 0) {
    byte t = getNumVal(&req, pos);
    if (t < strip.getMaxSegments()) main = t;
  }

  WS2812FX::Segment& mainseg = strip.getSegment(main);
  pos = req.indexOf(F("SV=")); //segment selected
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
  pos = req.indexOf(F("&S=")); //segment start
  if (pos > 0) {
    startI = getNumVal(&req, pos);
  }
  pos = req.indexOf(F("S2=")); //segment stop
  if (pos > 0) {
    stopI = getNumVal(&req, pos);
  }
  pos = req.indexOf(F("GP=")); //segment grouping
  if (pos > 0) {
    grpI = getNumVal(&req, pos);
    if (grpI == 0) grpI = 1;
  }
  pos = req.indexOf(F("SP=")); //segment spacing
  if (pos > 0) {
    spcI = getNumVal(&req, pos);
  }
  strip.setSegment(main, startI, stopI, grpI, spcI);

  main = strip.getMainSegmentId();

   //set presets
  pos = req.indexOf(F("P1=")); //sets first preset for cycle
  if (pos > 0) presetCycleMin = getNumVal(&req, pos);

  pos = req.indexOf(F("P2=")); //sets last preset for cycle
  if (pos > 0) presetCycleMax = getNumVal(&req, pos);

  //preset cycle
  pos = req.indexOf(F("CY="));
  if (pos > 0)
  {
    char cmd = req.charAt(pos+3);
    if (cmd == '2') presetCyclingEnabled = !presetCyclingEnabled;
    else presetCyclingEnabled = (cmd != '0');
    presetCycCurr = presetCycleMin;
  }

  pos = req.indexOf(F("PT=")); //sets cycle time in ms
  if (pos > 0) {
    int v = getNumVal(&req, pos);
    if (v > 100) presetCycleTime = v/100;
  }

  pos = req.indexOf(F("PS=")); //saves current in preset
  if (pos > 0) savePreset(getNumVal(&req, pos));

  //apply preset
  if (updateVal(&req, "PL=", &presetCycCurr, presetCycleMin, presetCycleMax)) {
    applyPreset(presetCycCurr);
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

  #ifdef WLED_ENABLE_LOXONE
  //lox parser
  pos = req.indexOf(F("LX=")); // Lox primary color
  if (pos > 0) {
    int lxValue = getNumVal(&req, pos);
    if (parseLx(lxValue, col)) {
      bri = 255;
      nightlightActive = false; //always disable nightlight when toggling
    }
  }
  pos = req.indexOf(F("LY=")); // Lox secondary color
  if (pos > 0) {
    int lxValue = getNumVal(&req, pos);
    if(parseLx(lxValue, colSec)) {
      bri = 255;
      nightlightActive = false; //always disable nightlight when toggling
    }
  }

  #endif

  //set hue
  pos = req.indexOf(F("HU="));
  if (pos > 0) {
    uint16_t temphue = getNumVal(&req, pos);
    byte tempsat = 255;
    pos = req.indexOf(F("SA="));
    if (pos > 0) {
      tempsat = getNumVal(&req, pos);
    }
    colorHStoRGB(temphue,tempsat,(req.indexOf(F("H2"))>0)? colSec:col);
  }

  //set white spectrum (kelvin)
  pos = req.indexOf(F("&K="));
  if (pos > 0) {
    colorKtoRGB(getNumVal(&req, pos),(req.indexOf(F("K2"))>0)? colSec:col);
  }

  //set color from HEX or 32bit DEC
  pos = req.indexOf(F("CL="));
  if (pos > 0) {
    colorFromDecOrHexString(col, (char*)req.substring(pos + 3).c_str());
  }
  pos = req.indexOf(F("C2="));
  if (pos > 0) {
    colorFromDecOrHexString(colSec, (char*)req.substring(pos + 3).c_str());
  }
  pos = req.indexOf(F("C3="));
  if (pos > 0) {
    byte t[4];
    colorFromDecOrHexString(t, (char*)req.substring(pos + 3).c_str());
    strip.setColor(2, t[0], t[1], t[2], t[3]);
  }

  //set to random hue SR=0->1st SR=1->2nd
  pos = req.indexOf(F("SR"));
  if (pos > 0) {
    _setRandomColor(getNumVal(&req, pos));
  }

  //swap 2nd & 1st
  pos = req.indexOf(F("SC"));
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
  updateVal(&req, "FP=", &effectPalette, 0, strip.getPaletteCount()-1);

  //set advanced overlay
  pos = req.indexOf(F("OL="));
  if (pos > 0) {
    overlayCurrent = getNumVal(&req, pos);
  }

  //apply macro (deprecated, added for compatibility with pre-0.11 automations)
  pos = req.indexOf(F("&M="));
  if (pos > 0) {
    applyPreset(getNumVal(&req, pos) + 16);
  }

  //toggle send UDP direct notifications
  pos = req.indexOf(F("SN="));
  if (pos > 0) notifyDirect = (req.charAt(pos+3) != '0');

  //toggle receive UDP direct notifications
  pos = req.indexOf(F("RN="));
  if (pos > 0) receiveNotifications = (req.charAt(pos+3) != '0');

  //receive live data via UDP/Hyperion
  pos = req.indexOf(F("RD="));
  if (pos > 0) receiveDirect = (req.charAt(pos+3) != '0');

  //main toggle on/off (parse before nightlight, #1214)
  pos = req.indexOf(F("&T="));
  if (pos > 0) {
    nightlightActive = false; //always disable nightlight when toggling
    switch (getNumVal(&req, pos))
    {
      case 0: if (bri != 0){briLast = bri; bri = 0;} break; //off, only if it was previously on
      case 1: if (bri == 0) bri = briLast; break; //on, only if it was previously off
      default: toggleOnOff(); //toggle
    }
  }

  //toggle nightlight mode
  bool aNlDef = false;
  if (req.indexOf(F("&ND")) > 0) aNlDef = true;
  pos = req.indexOf(F("NL="));
  if (pos > 0)
  {
    if (req.charAt(pos+3) == '0')
    {
      nightlightActive = false;
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
  pos = req.indexOf(F("NT="));
  if (pos > 0) {
    nightlightTargetBri = getNumVal(&req, pos);
    nightlightActiveOld = false; //re-init
  }

  //toggle nightlight fade
  pos = req.indexOf(F("NF="));
  if (pos > 0)
  {
    nightlightMode = getNumVal(&req, pos);

    nightlightActiveOld = false; //re-init
  }
  if (nightlightMode > NL_MODE_SUN) nightlightMode = NL_MODE_SUN;

  #if AUXPIN >= 0
  //toggle general purpose output
  pos = req.indexOf(F("AX="));
  if (pos > 0) {
    auxTime = getNumVal(&req, pos);
    auxActive = true;
    if (auxTime == 0) auxActive = false;
  }
  #endif

  pos = req.indexOf(F("TT="));
  if (pos > 0) transitionDelay = getNumVal(&req, pos);

  //Segment reverse
  pos = req.indexOf(F("RV="));
  if (pos > 0) strip.getSegment(main).setOption(SEG_OPTION_REVERSED, req.charAt(pos+3) != '0');

  //Segment reverse
  pos = req.indexOf(F("MI="));
  if (pos > 0) strip.getSegment(main).setOption(SEG_OPTION_MIRROR, req.charAt(pos+3) != '0');

  //Segment brightness/opacity
  pos = req.indexOf(F("SB="));
  if (pos > 0) {
    byte segbri = getNumVal(&req, pos);
    strip.getSegment(main).setOption(SEG_OPTION_ON, segbri);
    if (segbri) {
      strip.getSegment(main).opacity = segbri;
    }
  }

  //set time (unix timestamp)
  pos = req.indexOf(F("ST="));
  if (pos > 0) {
    setTime(getNumVal(&req, pos));
  }

  //set countdown goal (unix timestamp)
  pos = req.indexOf(F("CT="));
  if (pos > 0) {
    countdownTime = getNumVal(&req, pos);
    if (countdownTime - now() > 0) countdownOverTriggered = false;
  }

  pos = req.indexOf(F("LO="));
  if (pos > 0) {
    realtimeOverride = getNumVal(&req, pos);
    if (realtimeOverride > 2) realtimeOverride = REALTIME_OVERRIDE_ALWAYS;
  }

  pos = req.indexOf(F("RB"));
  if (pos > 0) doReboot = true;

  //cronixie
  #ifndef WLED_DISABLE_CRONIXIE
  //mode, 1 countdown
  pos = req.indexOf(F("NM="));
  if (pos > 0) countdownMode = (req.charAt(pos+3) != '0');
  
  pos = req.indexOf(F("NX=")); //sets digits to code
  if (pos > 0) {
    strlcpy(cronixieDisplay, req.substring(pos + 3, pos + 9).c_str(), 6);
    setCronixie();
  }

  pos = req.indexOf(F("NB="));
  if (pos > 0) //sets backlight
  {
    cronixieBacklight = (req.charAt(pos+3) != '0');
    overlayRefreshedTime = 0;
  }
  #endif

  pos = req.indexOf(F("U0=")); //user var 0
  if (pos > 0) {
    userVar0 = getNumVal(&req, pos);
  }

  pos = req.indexOf(F("U1=")); //user var 1
  if (pos > 0) {
    userVar1 = getNumVal(&req, pos);
  }
  //you can add more if you need

  if (!apply) return true; //when called by JSON API, do not call colorUpdated() here
  
  //internal call, does not send XML response
  pos = req.indexOf(F("IN"));
  if (pos < 1) XML_response(request);

  pos = req.indexOf(F("&NN")); //do not send UDP notifications this time
  colorUpdated((pos > 0) ? NOTIFIER_CALL_MODE_NO_NOTIFY : NOTIFIER_CALL_MODE_DIRECT_CHANGE);

  return true;
}
