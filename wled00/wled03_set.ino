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
void handleSettingsSet(AsyncWebServerRequest *request, byte subPage)
{
  //0: menu 1: wifi 2: leds 3: ui 4: sync 5: time 6: sec
  if (subPage <1 || subPage >6) return;
  
  //WIFI SETTINGS
  if (subPage == 1)
  {
    strcpy(clientSSID,request->arg("CS").c_str());
    if (request->arg("CP").charAt(0) != '*') strcpy(clientPass, request->arg("CP").c_str());

    strcpy(cmDNS, request->arg("CM").c_str());
    
    int t = request->arg("AT").toInt(); if (t > 9 && t <= 255) apWaitTimeSecs = t;
    strcpy(apSSID, request->arg("AS").c_str());
    apHide = request->hasArg("AH");
    if (request->arg("AP").charAt(0) != '*') strcpy(apPass, request->arg("AP").c_str());
    t = request->arg("AC").toInt(); if (t > 0 && t < 14) apChannel = t;
    
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
    if (t > 0 && t <= 1200) ledCount = t;
    //RMT eats up too much RAM
    #ifdef ARDUINO_ARCH_ESP32
    if (ledCount > 600) ledCount = 600;
    #endif
    strip.ablMilliampsMax = request->arg("MA").toInt();
    useRGBW = request->hasArg("EW");
    strip.colorOrder = request->arg("CO").toInt(); 
    autoRGBtoRGBW = request->hasArg("AW");

    //ignore settings and save current brightness, colors and fx as default
    if (request->hasArg("IS"))
    {
      for (byte i=0; i<4; i++)
      {
        colS[i] = col[i];
        colSecS[i] = colSec[i];
      }
      briS = bri;
      effectDefault = effectCurrent;
      effectSpeedDefault = effectSpeed;
      effectIntensityDefault = effectIntensity;
      effectPaletteDefault = effectPalette;
    } else {
      colS[0] = request->arg("CR").toInt();
      colS[1] = request->arg("CG").toInt();
      colS[2] = request->arg("CB").toInt();
      colSecS[0] = request->arg("SR").toInt();
      colSecS[1] = request->arg("SG").toInt();
      colSecS[2] = request->arg("SB").toInt();
      colS[3] = request->arg("CW").toInt();
      colSecS[3] = request->arg("SW").toInt();
      briS = request->arg("CA").toInt();
      effectDefault = request->arg("FX").toInt();
      effectSpeedDefault = request->arg("SX").toInt();
      effectIntensityDefault = request->arg("IX").toInt();
      effectPaletteDefault = request->arg("FP").toInt();
    }
    saveCurrPresetCycConf = request->hasArg("PC");
    turnOnAtBoot = request->hasArg("BO");
    t = request->arg("BP").toInt();
    if (t <= 25) bootPreset = t;
    useGammaCorrectionBri = request->hasArg("GB");
    useGammaCorrectionRGB = request->hasArg("GC");
    
    fadeTransition = request->hasArg("TF");
    t = request->arg("TD").toInt();
    if (t > 0) transitionDelay = t;
    transitionDelayDefault = t;
    strip.paletteFade = request->hasArg("PF");
    enableSecTransition = request->hasArg("T2");
    
    nightlightTargetBri = request->arg("TB").toInt();
    t = request->arg("TL").toInt();
    if (t > 0) nightlightDelayMinsDefault = t;
    nightlightFade = request->hasArg("TW");
    
    t = request->arg("PB").toInt();
    if (t >= 0 && t < 4) strip.paletteBlend = t;
    reverseMode = request->hasArg("RV");
    strip.setReverseMode(reverseMode);
    skipFirstLed = request->hasArg("SL");
    t = request->arg("BF").toInt();
    if (t > 0) briMultiplier = t;
  }

  //UI
  if (subPage == 3)
  {
    int t = request->arg("UI").toInt();
    if (t >= 0 && t < 3) uiConfiguration = t;
    strcpy(serverDescription, request->arg("DS").c_str());
    useHSBDefault = request->hasArg("MD");
    useHSB = useHSBDefault;
    currentTheme = request->arg("TH").toInt();
    char k[3]; k[0]='C'; k[2]=0;
    for(int i=0;i<6;i++)
    {
      k[1] = i+48;
      strcpy(cssCol[i],request->arg(k).c_str());
    }
    strcpy(cssFont,request->arg("CF").c_str());
  }

  //SYNC
  if (subPage == 4)
  {
    buttonEnabled = request->hasArg("BT");
    irEnabled = request->hasArg("IR");
    int t = request->arg("UP").toInt();
    if (t > 0) udpPort = t;
    receiveNotificationBrightness = request->hasArg("RB");
    receiveNotificationColor = request->hasArg("RC");
    receiveNotificationEffects = request->hasArg("RX");
    receiveNotifications = (receiveNotificationBrightness || receiveNotificationColor || receiveNotificationEffects);
    notifyDirectDefault = request->hasArg("SD");
    notifyDirect = notifyDirectDefault;
    notifyButton = request->hasArg("SB");
    notifyTwice = request->hasArg("S2");
    
    receiveDirect = request->hasArg("RD");
    e131Multicast = request->hasArg("EM");
    t = request->arg("EU").toInt();
    if (t > 0  && t <= 63999) e131Universe = t;
    t = request->arg("ET").toInt();
    if (t > 99  && t <= 65000) realtimeTimeoutMs = t;
    arlsForceMaxBri = request->hasArg("FB");
    arlsDisableGammaCorrection = request->hasArg("RG");
    t = request->arg("WO").toInt();
    if (t >= -255  && t <= 255) arlsOffset = t;
    enableRealtimeUI = request->hasArg("RU");
    
    alexaEnabled = request->hasArg("AL");
    strcpy(alexaInvocationName, request->arg("AI").c_str());
    notifyAlexa = request->hasArg("SA");
    
    if (request->hasArg("BK") && !request->arg("BK").equals("Hidden")) {
      strcpy(blynkApiKey,request->arg("BK").c_str()); initBlynk(blynkApiKey);
    }

    strcpy(mqttServer, request->arg("MS").c_str());
    strcpy(mqttDeviceTopic, request->arg("MD").c_str());
    strcpy(mqttGroupTopic, request->arg("MG").c_str());
    
    notifyHue = request->hasArg("SH");
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
    if (request->hasArg("HP"))
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
    ntpEnabled = request->hasArg("NT");
    useAMPM = !request->hasArg("CF");
    currentTimezone = request->arg("TZ").toInt();
    utcOffsetSecs = request->arg("UO").toInt();

    //start ntp if not already connected
    if (ntpEnabled && WiFi.status() == WL_CONNECTED && !ntpConnected) ntpConnected = ntpUdp.begin(ntpLocalPort);
    
    if (request->hasArg("OL")){
      overlayDefault = request->arg("OL").toInt();
      if (overlayCurrent != overlayDefault) strip.unlockAll();
      overlayCurrent = overlayDefault;
    }
    
    overlayMin = request->arg("O1").toInt();
    overlayMax = request->arg("O2").toInt();
    analogClock12pixel = request->arg("OM").toInt();
    analogClock5MinuteMarks = request->hasArg("O5");
    analogClockSecondsTrail = request->hasArg("OS");
    
    strcpy(cronixieDisplay,request->arg("CX").c_str());
    bool cbOld = cronixieBacklight;
    cronixieBacklight = request->hasArg("CB");
    if (cbOld != cronixieBacklight && overlayCurrent == 3)
    {
      strip.setCronixieBacklight(cronixieBacklight); overlayRefreshedTime = 0;
    }
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
      reset();
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
        strcpy(otaPass,request->arg("OP").c_str());
      }
    }
    
    if (pwdCorrect) //allow changes if correct pwd or no ota active
    {
      otaLock = request->hasArg("NO");
      wifiLock = request->hasArg("OW");
      recoveryAPDisabled = request->hasArg("NA");
      aOtaEnabled = request->hasArg("AO");
    }
  }
  saveSettingsToEEPROM();
  if (subPage == 2) strip.init(useRGBW,ledCount,skipFirstLed);
  if (subPage == 4) alexaInit();
}


//helper to get int value at a position in string
int getNumVal(String* req, uint16_t pos)
{
  return req->substring(pos + 3).toInt();
}


//HTTP API request parser
bool handleSet(AsyncWebServerRequest *request, String req)
{
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
    if (pos < 1) XML_response(request, false);
    return true;
    //if you save a macro in one request, other commands in that request are ignored due to unwanted behavior otherwise
  }
   
  //set brigthness
  pos = req.indexOf("&A=");
  if (pos > 0) {
    bri = getNumVal(&req, pos);
  }

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
   
  //set red value
  pos = req.indexOf("&R=");
  if (pos > 0) {
    col[0] = getNumVal(&req, pos);
  }
  //set green value
  pos = req.indexOf("&G=");
  if (pos > 0) {
    col[1] = getNumVal(&req, pos);
  }
  //set blue value
  pos = req.indexOf("&B=");
  if (pos > 0) {
    col[2] = getNumVal(&req, pos);
  }
  //set white value
  pos = req.indexOf("&W=");
  if (pos > 0) {
    col[3] = getNumVal(&req, pos);
  }
   
  //set 2nd red value
  pos = req.indexOf("R2=");
  if (pos > 0) {
    colSec[0] = getNumVal(&req, pos);
  }
  //set 2nd green value
  pos = req.indexOf("G2=");
  if (pos > 0) {
    colSec[1] = getNumVal(&req, pos);
  }
  //set 2nd blue value
  pos = req.indexOf("B2=");
  if (pos > 0) {
    colSec[2] = getNumVal(&req, pos);
  }
  //set 2nd white value
  pos = req.indexOf("W2=");
  if (pos > 0) {
    colSec[3] = getNumVal(&req, pos);
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
   
  //set 2nd to white
  pos = req.indexOf("SW");
  if (pos > 0) {
    if(useRGBW) {
      colSec[3] = 255;
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
    colSec[3] = 0;
    colSec[0] = 0;
    colSec[1] = 0;
    colSec[2] = 0;
  }
   
  //set to random hue SR=0->1st SR=1->2nd
  pos = req.indexOf("SR");
  if (pos > 0) {
    _setRandomColor(getNumVal(&req, pos));
  }
  //set 2nd to 1st
  pos = req.indexOf("SP");
  if (pos > 0) {
    colSec[0] = col[0];
    colSec[1] = col[1];
    colSec[2] = col[2];
    colSec[3] = col[3];
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
   
  //set current effect index
  pos = req.indexOf("FX=");
  if (pos > 0) {
    effectCurrent = getNumVal(&req, pos);
    presetCyclingEnabled = false;
  }
  //set effect speed
  pos = req.indexOf("SX=");
  if (pos > 0) {
    effectSpeed = getNumVal(&req, pos);
  }
  //set effect intensity
  pos = req.indexOf("IX=");
  if (pos > 0) {
    effectIntensity = req.substring(pos + 3).toInt();
  }
  //set effect palette (only for FastLED effects)
  pos = req.indexOf("FP=");
  if (pos > 0) {
    effectPalette = req.substring(pos + 3).toInt();
  }

  //set hue polling light: 0 -off
  pos = req.indexOf("HP=");
  if (pos > 0) {
    int id = getNumVal(&req, pos);
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
    useHSB = getNumVal(&req, pos);
  }
  //set advanced overlay
  pos = req.indexOf("OL=");
  if (pos > 0) {
    overlayCurrent = getNumVal(&req, pos);
    strip.unlockAll();
  }
  //(un)lock pixel (ranges)
  pos = req.indexOf("&L=");
  if (pos > 0){
    int index = getNumVal(&req, pos);
    pos = req.indexOf("L2=");
    if (pos > 0){
      int index2 = getNumVal(&req, pos);
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
    applyMacro(getNumVal(&req, pos));
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
    auxTime = getNumVal(&req, pos);
    auxActive = true;
    if (auxTime == 0) auxActive = false;
  }
  pos = req.indexOf("TT=");
  if (pos > 0) {
    transitionDelay = getNumVal(&req, pos);
  }
   
  //main toggle on/off
  pos = req.indexOf("&T=");
  if (pos > 0) {
    nightlightActive = false; //always disable nightlight when toggling
    switch (getNumVal(&req, pos))
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
    setTime(getNumVal(&req, pos));
  }
   
  //set countdown goal (unix timestamp)
  pos = req.indexOf("CT=");
  if (pos > 0) {
    countdownTime = getNumVal(&req, pos);
    if (countdownTime - now() > 0) countdownOverTriggered = false;
  }
   
  //set presets
  pos = req.indexOf("P1="); //sets first preset for cycle
  if (pos > 0) presetCycleMin = getNumVal(&req, pos);
  
  pos = req.indexOf("P2="); //sets last preset for cycle
  if (pos > 0) presetCycleMax = getNumVal(&req, pos);
   
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
    int v = getNumVal(&req, pos);
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
    savePreset(getNumVal(&req, pos));
  }
  pos = req.indexOf("PL="); //applies entire preset
  if (pos > 0) {
    applyPreset(getNumVal(&req, pos), presetApplyBri, presetApplyCol, presetApplyFx);
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
    userVar0 = getNumVal(&req, pos);
  }
  pos = req.indexOf("U1="); //user var 1
  if (pos > 0) {
    userVar1 = getNumVal(&req, pos);
  }
  //you can add more if you need
   
  //internal call, does not send XML response
  pos = req.indexOf("IN");
  if (pos < 1) XML_response(request, (req.indexOf("IT") > 0)); //include theme if firstload
  
  pos = req.indexOf("NN"); //do not send UDP notifications this time
  colorUpdated((pos > 0) ? 5:1);
  
  return true;
}
