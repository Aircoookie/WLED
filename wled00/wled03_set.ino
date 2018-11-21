/*
 * Receives client input
 */

void _setRandomColor(bool _sec,bool fromButton=false)
{
  lastRandomIndex = strip.get_random_wheel_index(lastRandomIndex);
  if (_sec){
    colorHStoRGB(lastRandomIndex*256,255,colSec);
  } else {
    colorHStoRGB(lastRandomIndex*256,255,col);
  }
  if (fromButton) colorUpdated(2);
}


//called upon POST settings form submit
void handleSettingsSet(byte subPage)
{
  //0: menu 1: wifi 2: leds 3: ui 4: sync 5: time 6: sec
  if (subPage <1 || subPage >6) return;
  
  //WIFI SETTINGS
  if (subPage == 1)
  {
    strcpy(clientSSID,server.arg("CS").c_str());
    if (server.arg("CP").charAt(0) != '*') strcpy(clientPass, server.arg("CP").c_str());

    strcpy(cmDNS, server.arg("CM").c_str());
    
    int t = server.arg("AT").toInt(); if (t > 9 && t <= 255) apWaitTimeSecs = t;
    strcpy(apSSID, server.arg("AS").c_str());
    apHide = server.hasArg("AH");
    if (server.arg("AP").charAt(0) != '*') strcpy(apPass, server.arg("AP").c_str());
    t = server.arg("AC").toInt(); if (t > 0 && t < 14) apChannel = t;
    
    char k[3]; k[2] = 0;
    for (int i = 0; i<4; i++)
    {
      k[1] = i+48;//ascii 0,1,2,3
      
      k[0] = 'I'; //static IP
      staticIP[i] = server.arg(k).toInt();
      
      k[0] = 'G'; //gateway
      staticGateway[i] = server.arg(k).toInt();
      
      k[0] = 'S'; //subnet
      staticSubnet[i] = server.arg(k).toInt();
    }
  }

  //LED SETTINGS
  if (subPage == 2)
  {
    int t = server.arg("LC").toInt();
    if (t > 0 && t <= 1200) ledCount = t;
    //RMT eats up too much RAM
    #ifdef ARDUINO_ARCH_ESP32
    if (ledCount > 600) ledCount = 600;
    #endif
    useRGBW = server.hasArg("EW");
    autoRGBtoRGBW = server.hasArg("AW");

    //ignore settings and save current brightness, colors and fx as default
    if (server.hasArg("IS"))
    {
      colS[0] = col[0];
      colS[1] = col[1];
      colS[2] = col[2];
      colSecS[0] = colSec[0];
      colSecS[1] = colSec[1];
      colSecS[2] = colSec[2];
      whiteS = white;
      whiteSecS = whiteSec;
      briS = bri;
      effectDefault = effectCurrent;
      effectSpeedDefault = effectSpeed;
      effectIntensityDefault = effectIntensity;
      effectPaletteDefault = effectPalette;
    } else {
      colS[0] = server.arg("CR").toInt();
      colS[1] = server.arg("CG").toInt();
      colS[2] = server.arg("CB").toInt();
      colSecS[0] = server.arg("SR").toInt();
      colSecS[1] = server.arg("SG").toInt();
      colSecS[2] = server.arg("SB").toInt();
      whiteS = server.arg("CW").toInt();
      whiteSecS = server.arg("SW").toInt();
      briS = server.arg("CA").toInt();
      effectDefault = server.arg("FX").toInt();
      effectSpeedDefault = server.arg("SX").toInt();
      effectIntensityDefault = server.arg("IX").toInt();
      effectPaletteDefault = server.arg("FP").toInt();
    }
    saveCurrPresetCycConf = server.hasArg("PC");
    turnOnAtBoot = server.hasArg("BO");
    t = server.arg("BP").toInt();
    if (t <= 25) bootPreset = t;
    useGammaCorrectionBri = server.hasArg("GB");
    useGammaCorrectionRGB = server.hasArg("GC");
    
    fadeTransition = server.hasArg("TF");
    t = server.arg("TD").toInt();
    if (t > 0) transitionDelay = t;
    transitionDelayDefault = t;
    strip.paletteFade = server.hasArg("PF");
    enableSecTransition = server.hasArg("T2");
    
    nightlightTargetBri = server.arg("TB").toInt();
    t = server.arg("TL").toInt();
    if (t > 0) nightlightDelayMinsDefault = t;
    nightlightFade = server.hasArg("TW");
    
    t = server.arg("PB").toInt();
    if (t >= 0 && t < 4) strip.paletteBlend = t;
    initLedsLast = server.hasArg("EI");
    reverseMode = server.hasArg("RV");
    strip.setReverseMode(reverseMode);
    skipFirstLed = server.hasArg("SL");
    t = server.arg("BF").toInt();
    if (t > 0) briMultiplier = t;
  }

  //UI
  if (subPage == 3)
  {
    int t = server.arg("UI").toInt();
    if (t >= 0 && t < 3) uiConfiguration = t;
    strcpy(serverDescription, server.arg("DS").c_str());
    useHSBDefault = server.hasArg("MD");
    useHSB = useHSBDefault;
    currentTheme = server.arg("TH").toInt();
    char k[3]; k[0]='C'; k[2]=0;
    for(int i=0;i<6;i++)
    {
      k[1] = i+48;
      strcpy(cssCol[i],server.arg(k).c_str());
    }
    strcpy(cssFont,server.arg("CF").c_str());
  }

  //SYNC
  if (subPage == 4)
  {
    buttonEnabled = server.hasArg("BT");
    irEnabled = server.hasArg("IR");
    int t = server.arg("UP").toInt();
    if (t > 0) udpPort = t;
    receiveNotificationBrightness = server.hasArg("RB");
    receiveNotificationColor = server.hasArg("RC");
    receiveNotificationEffects = server.hasArg("RX");
    receiveNotifications = (receiveNotificationBrightness || receiveNotificationColor || receiveNotificationEffects);
    notifyDirectDefault = server.hasArg("SD");
    notifyDirect = notifyDirectDefault;
    notifyButton = server.hasArg("SB");
    notifyTwice = server.hasArg("S2");
    
    receiveDirect = server.hasArg("RD");
    e131Multicast = server.hasArg("EM");
    t = server.arg("EU").toInt();
    if (t > 0  && t <= 63999) e131Universe = t;
    t = server.arg("ET").toInt();
    if (t > 99  && t <= 65000) realtimeTimeoutMs = t;
    arlsForceMaxBri = server.hasArg("FB");
    arlsDisableGammaCorrection = server.hasArg("RG");
    t = server.arg("WO").toInt();
    if (t >= -255  && t <= 255) arlsOffset = t;
    enableRealtimeUI = server.hasArg("RU");
    
    alexaEnabled = server.hasArg("AL");
    strcpy(alexaInvocationName, server.arg("AI").c_str());
    notifyAlexa = server.hasArg("SA");
    
    if (server.hasArg("BK") && !server.arg("BK").equals("Hidden")) {
      strcpy(blynkApiKey,server.arg("BK").c_str()); initBlynk(blynkApiKey);
    }

    strcpy(mqttServer, server.arg("MS").c_str());
    strcpy(mqttDeviceTopic, server.arg("MD").c_str());
    strcpy(mqttGroupTopic, server.arg("MG").c_str());
    
    notifyHue = server.hasArg("SH");
    for (int i=0;i<4;i++){
      String a = "H"+String(i);
      hueIP[i] = server.arg(a).toInt();
    }

    t = server.arg("HL").toInt();
    if (t > 0) huePollLightId = t;

    t = server.arg("HI").toInt();
    if (t > 50) huePollIntervalMs = t;

    hueApplyOnOff = server.hasArg("HO");
    hueApplyBri = server.hasArg("HB");
    hueApplyColor = server.hasArg("HC");
    if (server.hasArg("HP"))
    {
      if (!huePollingEnabled) hueAttempt = true;
      if (!setupHue()) hueAttempt = true;
    } else
    {
      huePollingEnabled = false;
      strcpy(hueError,"Inactive");
    }
  }

  //TIME
  if (subPage == 5)
  {
    ntpEnabled = server.hasArg("NT");
    useAMPM = !server.hasArg("CF");
    currentTimezone = server.arg("TZ").toInt();
    utcOffsetSecs = server.arg("UO").toInt();

    //start ntp if not already connected
    if (ntpEnabled && WiFi.status() == WL_CONNECTED && !ntpConnected) ntpConnected = ntpUdp.begin(ntpLocalPort);
    
    if (server.hasArg("OL")){
      overlayDefault = server.arg("OL").toInt();
      if (overlayCurrent != overlayDefault) strip.unlockAll();
      overlayCurrent = overlayDefault;
    }
    
    overlayMin = server.arg("O1").toInt();
    overlayMax = server.arg("O2").toInt();
    analogClock12pixel = server.arg("OM").toInt();
    analogClock5MinuteMarks = server.hasArg("O5");
    analogClockSecondsTrail = server.hasArg("OS");
    
    strcpy(cronixieDisplay,server.arg("CX").c_str());
    bool cbOld = cronixieBacklight;
    cronixieBacklight = server.hasArg("CB");
    if (cbOld != cronixieBacklight && overlayCurrent == 3)
    {
      strip.setCronixieBacklight(cronixieBacklight); overlayRefreshedTime = 0;
    }
    countdownMode = server.hasArg("CE");
    countdownYear = server.arg("CY").toInt();
    countdownMonth = server.arg("CI").toInt();
    countdownDay = server.arg("CD").toInt();
    countdownHour = server.arg("CH").toInt();
    countdownMin = server.arg("CM").toInt();
    countdownSec = server.arg("CS").toInt();
    
    for (int i=1;i<17;i++)
    {
      String a = "M"+String(i);
      if (server.hasArg(a)) saveMacro(i,server.arg(a),false);
    }
    
    macroBoot = server.arg("MB").toInt();
    macroAlexaOn = server.arg("A0").toInt();
    macroAlexaOff = server.arg("A1").toInt();
    macroButton = server.arg("MP").toInt();
    macroLongPress = server.arg("ML").toInt();
    macroCountdown = server.arg("MC").toInt();
    macroNl = server.arg("MN").toInt();

    char k[3]; k[2] = 0;
    for (int i = 0; i<8; i++)
    {
      k[1] = i+48;//ascii 0,1,2,3
      
      k[0] = 'H'; //timer hours
      timerHours[i] = server.arg(k).toInt();
      
      k[0] = 'N'; //minutes
      timerMinutes[i] = server.arg(k).toInt();
      
      k[0] = 'T'; //macros
      timerMacro[i] = server.arg(k).toInt();
    }
  }

  //SECURITY
  if (subPage == 6)
  {
    if (server.hasArg("RS")) //complete factory reset
    {
      clearEEPROM();
      serveMessage(200, "All Settings erased.", "Connect to WLED-AP to setup again",255);
      reset();
    }

    bool pwdCorrect = !otaLock; //always allow access if ota not locked
    if (server.hasArg("OP"))
    {
      if (otaLock && strcmp(otaPass,server.arg("OP").c_str()) == 0)
      {
        pwdCorrect = true;
      }
      if (!otaLock && server.arg("OP").length() > 0)
      {
        strcpy(otaPass,server.arg("OP").c_str());
      }
    }
    
    if (pwdCorrect) //allow changes if correct pwd or no ota active
    {
      otaLock = server.hasArg("NO");
      wifiLock = server.hasArg("OW");
      recoveryAPDisabled = server.hasArg("NA");
      aOtaEnabled = server.hasArg("AO");
    }
  }
  saveSettingsToEEPROM();
  if (subPage == 2) strip.init(useRGBW,ledCount,skipFirstLed);
}

bool handleSet(String req)
{
  bool effectUpdated = false;
  if (!(req.indexOf("win") >= 0)) return false;

  int pos = 0;
  DEBUG_PRINT("API req: ");
  DEBUG_PRINTLN(req);
  
  //save macro, requires &MS=<slot>(<macro>) format
  pos = req.indexOf("&MS=");
  if (pos > 0) {
    int i = req.substring(pos + 4).toInt();
    pos = req.indexOf('(') +1;
    if (pos > 0) { 
      int en = req.indexOf(')');
      String mc = req.substring(pos);
      if (en > 0) mc = req.substring(pos, en);
      saveMacro(i, mc); 
    }
    
    pos = req.indexOf("IN");
    if (pos < 1) XML_response(true, false);
    return true;
    //if you save a macro in one request, other commands in that request are ignored due to unwanted behavior otherwise
  }
   
  //set brigthness
  pos = req.indexOf("&A=");
  if (pos > 0) {
    bri = req.substring(pos + 3).toInt();
  }

  //set hue
  pos = req.indexOf("HU=");
  if (pos > 0) {
    uint16_t temphue = req.substring(pos + 3).toInt();
    byte tempsat = 255;
    pos = req.indexOf("SA=");
    if (pos > 0) {
      tempsat = req.substring(pos + 3).toInt();
    }
    colorHStoRGB(temphue,tempsat,(req.indexOf("H2")>0)? colSec:col);
  }
   
  //set red value
  pos = req.indexOf("&R=");
  if (pos > 0) {
    col[0] = req.substring(pos + 3).toInt();
  }
  //set green value
  pos = req.indexOf("&G=");
  if (pos > 0) {
    col[1] = req.substring(pos + 3).toInt();
  }
  //set blue value
  pos = req.indexOf("&B=");
  if (pos > 0) {
    col[2] = req.substring(pos + 3).toInt();
  }
  //set white value
  pos = req.indexOf("&W=");
  if (pos > 0) {
    white = req.substring(pos + 3).toInt();
  }
   
  //set 2nd red value
  pos = req.indexOf("R2=");
  if (pos > 0) {
    colSec[0] = req.substring(pos + 3).toInt();
  }
  //set 2nd green value
  pos = req.indexOf("G2=");
  if (pos > 0) {
    colSec[1] = req.substring(pos + 3).toInt();
  }
  //set 2nd blue value
  pos = req.indexOf("B2=");
  if (pos > 0) {
    colSec[2] = req.substring(pos + 3).toInt();
  }
  //set 2nd white value
  pos = req.indexOf("W2=");
  if (pos > 0) {
    whiteSec = req.substring(pos + 3).toInt();
  }
   
  //set color from HEX or 32bit DEC
  pos = req.indexOf("CL=");
  if (pos > 0) {
    colorFromDecOrHexString(col, &white, (char*)req.substring(pos + 3).c_str());
  }
  pos = req.indexOf("C2=");
  if (pos > 0) {
    colorFromDecOrHexString(colSec, &whiteSec, (char*)req.substring(pos + 3).c_str());
  }
   
  //set 2nd to white
  pos = req.indexOf("SW");
  if (pos > 0) {
    if(useRGBW) {
      whiteSec = 255;
      colSec[0] = 0;
      colSec[1] = 0;
      colSec[2] = 0;
    } else {
      colSec[0] = 255;
      colSec[1] = 255;
      colSec[2] = 255;
    }
  }
   
  //set 2nd to black
  pos = req.indexOf("SB");
  if (pos > 0) {
    whiteSec = 0;
    colSec[0] = 0;
    colSec[1] = 0;
    colSec[2] = 0;
  }
   
  //set to random hue SR=0->1st SR=1->2nd
  pos = req.indexOf("SR");
  if (pos > 0) {
    _setRandomColor(req.substring(pos + 3).toInt());
  }
  //set 2nd to 1st
  pos = req.indexOf("SP");
  if (pos > 0) {
    colSec[0] = col[0];
    colSec[1] = col[1];
    colSec[2] = col[2];
    whiteSec = white;
  }
  //swap 2nd & 1st
  pos = req.indexOf("SC");
  if (pos > 0) {
    byte _temp[4];
    for (int i = 0; i<3; i++)
    {
      _temp[i] = col[i];
      col[i] = colSec[i];
      colSec[i] = _temp[i];
    }
    _temp[3] = white;
    white = whiteSec;
    whiteSec = _temp[3];
  }
   
  //set current effect index
  pos = req.indexOf("FX=");
  if (pos > 0) {
    if (effectCurrent != req.substring(pos + 3).toInt())
    {
      effectCurrent = req.substring(pos + 3).toInt();
      strip.setMode(effectCurrent);
      effectUpdated = true;
    }
  }
  //set effect speed
  pos = req.indexOf("SX=");
  if (pos > 0) {
    if (effectSpeed != req.substring(pos + 3).toInt())
    {
      effectSpeed = req.substring(pos + 3).toInt();
      strip.setSpeed(effectSpeed);
      effectUpdated = true;
    }
  }
  //set effect intensity
  pos = req.indexOf("IX=");
  if (pos > 0) {
    if (effectIntensity != req.substring(pos + 3).toInt())
    {
      effectIntensity = req.substring(pos + 3).toInt();
      strip.setIntensity(effectIntensity);
      effectUpdated = true;
    }
  }
  //set effect palette (only for FastLED effects)
  pos = req.indexOf("FP=");
  if (pos > 0) {
    if (effectPalette != req.substring(pos + 3).toInt())
    {
      effectPalette = req.substring(pos + 3).toInt();
      strip.setPalette(effectPalette);
      effectUpdated = true;
    }
  }

  //set hue polling light: 0 -off
  pos = req.indexOf("HP=");
  if (pos > 0) {
    int id = req.substring(pos + 3).toInt();
    if (id > 0)
    {
      if (id < 100) huePollLightId = id;
      setupHue();
    } else {
      huePollingEnabled = false;
    }
  }
   
  //set default control mode (0 - RGB, 1 - HSB)
  pos = req.indexOf("MD=");
  if (pos > 0) {
    useHSB = req.substring(pos + 3).toInt();
  }
  //set advanced overlay
  pos = req.indexOf("OL=");
  if (pos > 0) {
    overlayCurrent = req.substring(pos + 3).toInt();
    strip.unlockAll();
  }
  //(un)lock pixel (ranges)
  pos = req.indexOf("&L=");
  if (pos > 0){
    int index = req.substring(pos + 3).toInt();
    pos = req.indexOf("L2=");
    if (pos > 0){
      int index2 = req.substring(pos + 3).toInt();
      if (req.indexOf("UL") > 0)
      {
        strip.unlockRange(index, index2);
      } else
      {
        strip.lockRange(index, index2);
      }
    } else
    {
      if (req.indexOf("UL") > 0)
      {
        strip.unlock(index);
      } else
      {
        strip.lock(index);
      }
    }
  }

  //apply macro
  pos = req.indexOf("&M=");
  if (pos > 0) {
    applyMacro(req.substring(pos + 3).toInt());
  }
  //toggle send UDP direct notifications
  if (req.indexOf("SN=") > 0)
  {
    notifyDirect = true;
    if (req.indexOf("SN=0") > 0)
    {
      notifyDirect = false;
    }
  }
   
  //toggle receive UDP direct notifications
  if (req.indexOf("RN=") > 0)
  {
    receiveNotifications = true;
    if (req.indexOf("RN=0") > 0)
    {
      receiveNotifications = false;
    }
  }
   
  //toggle nightlight mode
  bool aNlDef = false;
  if (req.indexOf("&ND") > 0) aNlDef = true;
  pos = req.indexOf("NL=");
  if (pos > 0)
  {
    if (req.indexOf("NL=0") > 0)
    {
      nightlightActive = false;
      bri = briT;
    } else {
      nightlightActive = true;
      if (!aNlDef) nightlightDelayMins = req.substring(pos + 3).toInt();
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
    nightlightTargetBri = req.substring(pos + 3).toInt();
    nightlightActiveOld = false; //re-init
  }
   
  //toggle nightlight fade
  if (req.indexOf("NF=") > 0)
  {
    if (req.indexOf("NF=0") > 0)
    {
      nightlightFade = false;
    } else {
      nightlightFade = true;
    }
    nightlightActiveOld = false; //re-init
  }
   
  //toggle general purpose output
  pos = req.indexOf("AX=");
  if (pos > 0) {
    auxTime = req.substring(pos + 3).toInt();
    auxActive = true;
    if (auxTime == 0) auxActive = false;
  }
  pos = req.indexOf("TT=");
  if (pos > 0) {
    transitionDelay = req.substring(pos + 3).toInt();
  }
   
  //main toggle on/off
  pos = req.indexOf("&T=");
  if (pos > 0) {
    nightlightActive = false; //always disable nightlight when toggling
    switch (req.substring(pos + 3).toInt())
    {
      case 0: if (bri != 0){briLast = bri; bri = 0;} break; //off
      case 1: bri = briLast; break; //on
      default: toggleOnOff(); //toggle
    }
  }
   
  //deactivate nightlight if target brightness is reached
  if (bri == nightlightTargetBri) nightlightActive = false;
  //set time (unix timestamp)
  pos = req.indexOf("ST=");
  if (pos > 0) {
    setTime(req.substring(pos+3).toInt());
  }
   
  //set countdown goal (unix timestamp)
  pos = req.indexOf("CT=");
  if (pos > 0) {
    countdownTime = req.substring(pos+3).toInt();
    if (countdownTime - now() > 0) countdownOverTriggered = false;
  }
   
  //set presets
  pos = req.indexOf("P1="); //sets first preset for cycle
  if (pos > 0) presetCycleMin = req.substring(pos + 3).toInt();
  
  pos = req.indexOf("P2="); //sets last preset for cycle
  if (pos > 0) presetCycleMax = req.substring(pos + 3).toInt();
   
  if (req.indexOf("CY=") > 0) //preset cycle
  {
    presetCyclingEnabled = true;
    if (req.indexOf("CY=0") > 0)
    {
      presetCyclingEnabled = false;
    }
    presetCycCurr = presetCycleMin;
  }
  pos = req.indexOf("PT="); //sets cycle time in ms
  if (pos > 0) {
    int v = req.substring(pos + 3).toInt();
    if (v > 49) presetCycleTime = v;
  }
  if (req.indexOf("PA=") > 0) //apply brightness from preset
  {
    presetApplyBri = true;
    if (req.indexOf("PA=0") > 0) presetApplyBri = false;
  }
  if (req.indexOf("PC=") > 0) //apply color from preset
  {
    presetApplyCol = true;
    if (req.indexOf("PC=0") > 0) presetApplyCol = false;
  }
  if (req.indexOf("PX=") > 0) //apply effects from preset
  {
    presetApplyFx = true;
    if (req.indexOf("PX=0") > 0) presetApplyFx = false;
  }
  
  pos = req.indexOf("PS="); //saves current in preset
  if (pos > 0) {
    savePreset(req.substring(pos + 3).toInt());
  }
  pos = req.indexOf("PL="); //applies entire preset
  if (pos > 0) {
    applyPreset(req.substring(pos + 3).toInt(), presetApplyBri, presetApplyCol, presetApplyFx);
    if (presetApplyFx) effectUpdated = true;
  }
  
  //cronixie
  pos = req.indexOf("NX="); //sets digits to code
  if (pos > 0) {
    strcpy(cronixieDisplay,req.substring(pos + 3, pos + 9).c_str());
    setCronixie();
  }
  pos = req.indexOf("NM="); //mode, 1 countdown
  if (pos > 0) {
    countdownMode = true;
    if (req.indexOf("NM=0") > 0)
    {
      countdownMode = false;
    }
  }
  if (req.indexOf("NB=") > 0) //sets backlight
  {
    cronixieBacklight = true;
    if (req.indexOf("NB=0") > 0)
    {
      cronixieBacklight = false;
    }
    if (overlayCurrent == 3) strip.setCronixieBacklight(cronixieBacklight);
    overlayRefreshedTime = 0;
  }
  
  pos = req.indexOf("U0="); //user var 0
  if (pos > 0) {
    userVar0 = req.substring(pos + 3).toInt();
  }
  pos = req.indexOf("U1="); //user var 1
  if (pos > 0) {
    userVar1 = req.substring(pos + 3).toInt();
  }
  //you can add more if you need
   
  //internal call, does not send XML response
  pos = req.indexOf("IN");
  if (pos < 1) XML_response(true, (req.indexOf("IT") > 0)); //include theme if firstload
  //do not send UDP notifications this time
  pos = req.indexOf("NN");
  if (pos > 0)
  {
    colorUpdated(5);
    return true;
  }
  if (effectUpdated)
  {
    colorUpdated(6);
  } else
  {
    colorUpdated(1);
  }
  return true;
}
