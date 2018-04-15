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

void handleSettingsSet(byte subPage)
{
  //0: menu 1: wifi 2: leds 3: ui 4: sync 5: time 6: sec
  if (subPage <1 || subPage >6) return;
  
  //WIFI SETTINGS
  if (subPage == 1)
  {
    if (server.hasArg("CS")) clientSSID = server.arg("CS");
    if (server.hasArg("CP"))
    {
      if (!server.arg("CP").indexOf('*') == 0)
      {
        DEBUG_PRINTLN("Setting pass");
        clientPass = server.arg("CP");
      }
    }
    if (server.hasArg("CM")) cmDNS = server.arg("CM");
    if (server.hasArg("AT"))
    {
        int i = server.arg("AT").toInt();
        if (i >= 0 && i <= 255) apWaitTimeSecs = i;
    }
    if (server.hasArg("AS")) apSSID = server.arg("AS");
    apHide = server.hasArg("AH");
    if (server.hasArg("AP"))
    {
      if (!server.arg("AP").indexOf('*') == 0) apPass = server.arg("AP");
    }
    if (server.hasArg("AC"))
    {
      int chan = server.arg("AC").toInt();
      if (chan > 0 && chan < 14) apChannel = chan;
    }
    if (server.hasArg("I0"))
    {
      int i = server.arg("I0").toInt();
      if (i >= 0 && i <= 255) staticIP[0] = i;
    }
    if (server.hasArg("I1"))
    {
      int i = server.arg("I1").toInt();
      if (i >= 0 && i <= 255) staticIP[1] = i;
    }
    if (server.hasArg("I2"))
    {
      int i = server.arg("I2").toInt();
      if (i >= 0 && i <= 255) staticIP[2] = i;
    }
    if (server.hasArg("I3"))
    {
      int i = server.arg("I3").toInt();
      if (i >= 0 && i <= 255) staticIP[3] = i;
    }
    if (server.hasArg("G0"))
    {
      int i = server.arg("G0").toInt();
      if (i >= 0 && i <= 255) staticGateway[0] = i;
    }
    if (server.hasArg("G1"))
    {
      int i = server.arg("G1").toInt();
      if (i >= 0 && i <= 255) staticGateway[1] = i;
    }
    if (server.hasArg("G2"))
    {
      int i = server.arg("G2").toInt();
      if (i >= 0 && i <= 255) staticGateway[2] = i;
    }
    if (server.hasArg("G3"))
    {
      int i = server.arg("G3").toInt();
      if (i >= 0 && i <= 255) staticGateway[3] = i;
    }
    if (server.hasArg("S0"))
    {
      int i = server.arg("S0").toInt();
      if (i >= 0 && i <= 255) staticSubnet[0] = i;
    }
    if (server.hasArg("S1"))
    {
      int i = server.arg("S1").toInt();
      if (i >= 0 && i <= 255) staticSubnet[1] = i;
    }
    if (server.hasArg("S2"))
    {
      int i = server.arg("S2").toInt();
      if (i >= 0 && i <= 255) staticSubnet[2] = i;
    }
    if (server.hasArg("S3"))
    {
      int i = server.arg("S3").toInt();
      if (i >= 0 && i <= 255) staticSubnet[3] = i;
    }
  }

  //LED SETTINGS
  if (subPage == 2)
  {
    if (server.hasArg("LC"))
    {
      int i = server.arg("LC").toInt();
      if (i > 0 && i <= 1200) ledCount = i;
      //RMT eats up too much RAM
      #ifdef ARDUINO_ARCH_ESP32
      if (ledCount > 600) ledCount = 600;
      #endif
    }
    useRGBW = server.hasArg("EW");
    if (server.hasArg("IS")) //ignore settings and save current brightness, colors and fx as default
    {
      colS[0] = col[0];
      colS[1] = col[1];
      colS[2] = col[2];
      if (useRGBW) whiteS = white;
      briS = bri;
      effectDefault = effectCurrent;
      effectSpeedDefault = effectSpeed;
    } else {
      if (server.hasArg("CR"))
      {
        int i = server.arg("CR").toInt();
        if (i >= 0 && i <= 255) colS[0] = i;
      }
      if (server.hasArg("CG"))
      {
        int i = server.arg("CG").toInt();
        if (i >= 0 && i <= 255) colS[1] = i;
      }
      if (server.hasArg("CB"))
      {
        int i = server.arg("CB").toInt();
        if (i >= 0 && i <= 255) colS[2] = i;
      }
      if (server.hasArg("SR"))
      {
        int i = server.arg("SR").toInt();
        if (i >= 0 && i <= 255) colSecS[0] = i;
      }
      if (server.hasArg("SG"))
      {
        int i = server.arg("SG").toInt();
        if (i >= 0 && i <= 255) colSecS[1] = i;
      }
      if (server.hasArg("SB"))
      {
        int i = server.arg("SB").toInt();
        if (i >= 0 && i <= 255) colSecS[2] = i;
      }
      if (server.hasArg("SW"))
      {
        int i = server.arg("SW").toInt();
        if (i >= 0 && i <= 255) whiteSecS = i;
      }
      if (server.hasArg("CW"))
      {
        int i = server.arg("CW").toInt();
        if (i >= 0 && i <= 255) whiteS = i;
      }
      if (server.hasArg("CA"))
      {
        int i = server.arg("CA").toInt();
        if (i >= 0 && i <= 255) briS = i;
      }
      if (server.hasArg("FX"))
      {
        int i = server.arg("FX").toInt();
        if (i >= 0 && i <= 255) effectDefault = i;
      }
      if (server.hasArg("SX"))
      {
        int i = server.arg("SX").toInt();
        if (i >= 0 && i <= 255) effectSpeedDefault = i;
      }
      if (server.hasArg("IX"))
      {
        int i = server.arg("IX").toInt();
        if (i >= 0 && i <= 255) effectIntensityDefault = i;
      }
    }
    turnOnAtBoot = server.hasArg("BO");
    if (server.hasArg("BP"))
    {
      int i = server.arg("BP").toInt();
      if (i >= 0 && i <= 25) bootPreset = i;
    }
    useGammaCorrectionBri = server.hasArg("GB");
    useGammaCorrectionRGB = server.hasArg("GC");
    fadeTransition = server.hasArg("TF");
    sweepTransition = server.hasArg("TS");
    sweepDirection = !server.hasArg("TI");
    if (server.hasArg("TD"))
    {
      int i = server.arg("TD").toInt();
      if (i > 0){
        transitionDelay = i;
      }
    }
    if (server.hasArg("TB"))
    {
      nightlightTargetBri = server.arg("TB").toInt();
    }
    if (server.hasArg("TL"))
    {
      int i = server.arg("TL").toInt();
      if (i > 0) nightlightDelayMins = i;
    }
    nightlightFade = server.hasArg("TW");
    reverseMode = server.hasArg("RV");
    initLedsLast = server.hasArg("EI");
    strip.setReverseMode(reverseMode);
    if (server.hasArg("WO"))
    {
      int i = server.arg("WO").toInt();
      if (i >= -255  && i <= 255) arlsOffset = i;
    }
    if (server.hasArg("BF"))
    {
      int i = server.arg("BF").toInt();
      if (i > 0) briMultiplier = i;
    }
  }

  //UI
  if (subPage == 3)
  {
    if (server.hasArg("DS")) serverDescription = server.arg("DS");
    useHSBDefault = server.hasArg("MD");
    useHSB = useHSBDefault;
    if (server.hasArg("TH")) currentTheme = server.arg("TH").toInt();
    for(int i=0;i<6;i++)
    {
      if (server.hasArg("C"+String(i))) cssCol[i] = server.arg("C"+String(i));
    }
    if (server.hasArg("CF")) cssFont = server.arg("CF");
    buildCssColorString();
  }

  //SYNC
  if (subPage == 4)
  {
    buttonEnabled = server.hasArg("BT");
    if (server.hasArg("UP"))
    {
      udpPort = server.arg("UP").toInt();
    }
    receiveNotificationBrightness = server.hasArg("RB");
    receiveNotificationColor = server.hasArg("RC");
    receiveNotificationEffects = server.hasArg("RX");
    receiveNotifications = (receiveNotificationBrightness || receiveNotificationColor || receiveNotificationEffects);
    notifyDirectDefault = server.hasArg("SD");
    notifyDirect = notifyDirectDefault;
    notifyButton = server.hasArg("SB");
    notifyTwice = server.hasArg("S2");
    alexaEnabled = server.hasArg("AL");
    if (server.hasArg("AI")) alexaInvocationName = server.arg("AI");
    alexaNotify = server.hasArg("SA");
    notifyHue = server.hasArg("SH");
    for (int i=0;i<4;i++){
      String a = "H"+String(i);
      if (server.hasArg(a))
        hueIP[i] = server.arg(a).toInt();
    }
    if (server.hasArg("HL"))
    {
      int i = server.arg("HL").toInt();
      if (i > 0) huePollLightId = i;
    }
    if (server.hasArg("HI"))
    {
      int i = server.arg("HI").toInt();
      if (i > 50) huePollIntervalMs = i;
    }
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
      hueError = "Inactive";
    }
  }

  //TIME
  if (subPage == 5)
  {
    ntpEnabled = server.hasArg("NT");
    useAMPM = !server.hasArg("CF");
    if (server.hasArg("TZ")) currentTimezone = server.arg("TZ").toInt();
    if (server.hasArg("UO")) utcOffsetSecs = server.arg("UO").toInt();
    if (ntpEnabled && WiFi.status() == WL_CONNECTED && !ntpConnected) ntpConnected = ntpUdp.begin(ntpLocalPort); //start if not already connected
    
    if (server.hasArg("OL")){
      overlayDefault = server.arg("OL").toInt();
      overlayCurrent = overlayDefault;
      strip.unlockAll();
    }
    if (server.hasArg("O1")) overlayMin = server.arg("O1").toInt();
    if (server.hasArg("O2")) overlayMax = server.arg("O2").toInt();
    if (server.hasArg("OM")) analogClock12pixel = server.arg("OM").toInt();
    analogClock5MinuteMarks = server.hasArg("O5");
    analogClockSecondsTrail = server.hasArg("OS");
    
    if (server.hasArg("CX")) cronixieDisplay = server.arg("CX");
    bool cbOld = cronixieBacklight;
    cronixieBacklight = server.hasArg("CB");
    if (cbOld != cronixieBacklight && overlayCurrent == 4)
    {
      strip.setCronixieBacklight(cronixieBacklight); overlayRefreshedTime = 0;
    }
    countdownMode = server.hasArg("CE");
    if (server.hasArg("CY")) countdownYear = server.arg("CY").toInt();
    if (server.hasArg("CI")) countdownMonth = server.arg("CI").toInt();
    if (server.hasArg("CD")) countdownDay = server.arg("CD").toInt();
    if (server.hasArg("CH")) countdownHour = server.arg("CH").toInt();
    if (server.hasArg("CM")) countdownMin = server.arg("CM").toInt();
    if (server.hasArg("CS")) countdownSec = server.arg("CS").toInt();
    
    for (int i=1;i<17;i++)
    {
      String a = "M"+String(i);
      if (server.hasArg(a)) saveMacro(i,server.arg(a),false);
    }
    if (server.hasArg("MB")) macroBoot = server.arg("MB").toInt();
    if (server.hasArg("A0")) macroAlexaOn = server.arg("A0").toInt();
    if (server.hasArg("A1")) macroAlexaOff = server.arg("A1").toInt();
    if (server.hasArg("MP")) macroButton = server.arg("MP").toInt();
    if (server.hasArg("ML")) macroLongPress = server.arg("ML").toInt();
    if (server.hasArg("MC")) macroCountdown = server.arg("MC").toInt();
    if (server.hasArg("MN")) macroNl = server.arg("MN").toInt();
  }

  //SECURITY
  if (subPage == 6)
  {
    if (server.hasArg("RS"))
    {
      clearEEPROM();
      serveMessage(200, "All Settings erased.", "Connect to WLED-AP to setup again...",255);
      reset();
    }

    bool pwdCorrect = !otaLock; //always allow access if ota not locked
    if (server.hasArg("OP"))
    {
      if (otaLock && otaPass.equals(server.arg("OP")))
      {
        pwdCorrect = true;
      }
      if (!otaLock && server.arg("OP").length() > 0)
      {
        otaPass = server.arg("OP");
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
  if (subPage == 2) strip.init(useRGBW,ledCount,PIN);
}

bool handleSet(String req)
{
   bool effectUpdated = false;
   if (!(req.indexOf("win") >= 0)) {
        return false;
   }
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
      if (pos < 1) XML_response();
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
   //set individual pixel (range) to current color
   pos = req.indexOf("&I=");
   if (pos > 0){
      int index = req.substring(pos + 3).toInt();
      pos = req.indexOf("I2=");
      if (pos > 0){
        int index2 = req.substring(pos + 3).toInt();
        strip.setRange(index, index2);
      } else
      {
        strip.setIndividual(index);
      }
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
      switch (req.substring(pos + 3).toInt())
      {
        case 0: if (bri != 0){briLast = bri; bri = 0;} break; //off
        case 1: bri = briLast; break; //on
        default: if (bri == 0) //toggle
        {
          bri = briLast;
        } else
        {
          briLast = bri;
          bri = 0;
        }
      }
   }
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
   
   //set custom chase data
   bool _cc_updated = false;
   pos = req.indexOf("C0="); if (pos > 0) {ccStart =  (req.substring(pos + 3).toInt()); _cc_updated = true;}
   pos = req.indexOf("C1="); if (pos > 0) {ccIndex1 = (req.substring(pos + 3).toInt()); _cc_updated = true;}
   pos = req.indexOf("C2="); if (pos > 0) {ccIndex2 = (req.substring(pos + 3).toInt()); _cc_updated = true;}
   pos = req.indexOf("CP="); if (pos > 0) {ccNumPrimary = (req.substring(pos + 3).toInt()); _cc_updated = true;}
   pos = req.indexOf("CS="); if (pos > 0) {ccNumSecondary = (req.substring(pos + 3).toInt()); _cc_updated = true;}
   pos = req.indexOf("CM="); if (pos > 0) {ccStep = (req.substring(pos + 3).toInt()); _cc_updated = true;}
   pos = req.indexOf("CF="); if (pos > 0) {ccFromStart = (req.substring(pos + 3).toInt()); _cc_updated = true;}
   pos = req.indexOf("CE="); if (pos > 0) {ccFromEnd = (req.substring(pos + 3).toInt()); _cc_updated = true;}
   if (ccIndex2 == 255) ccIndex2 = ledCount-1;
   if (_cc_updated) strip.setCustomChase(ccIndex1, ccIndex2, ccStart, ccNumPrimary, ccNumSecondary, ccStep, ccFromStart, ccFromEnd);
   
   //set presets
   if (req.indexOf("CY=") > 0) //preset cycle
   {
      presetCyclingEnabled = true;
      if (req.indexOf("CY=0") > 0)
      {
        presetCyclingEnabled = false;
      }
      bool all = true;
      if (req.indexOf("&PA") > 0)
      {
        presetCycleBri = true;
        all = false;
      }
      if (req.indexOf("&PC") > 0)
      {
        presetCycleCol = true;
        all = false;
      }
      if (req.indexOf("&PX") > 0)
      {
        presetCycleFx = true;
        all = false;
      }
      if (all)
      {
        presetCycleBri = true;
        presetCycleCol = true;
        presetCycleFx = true;
      }
   }
   pos = req.indexOf("PT="); //sets cycle time in ms
   if (pos > 0) {
      int v = req.substring(pos + 3).toInt();
      if (v > 49) presetCycleTime = v;
   }
   pos = req.indexOf("P1="); //sets first preset for cycle
   if (pos > 0) presetCycleMin = req.substring(pos + 3).toInt();

   pos = req.indexOf("P2="); //sets last preset for cycle
   if (pos > 0) presetCycleMax = req.substring(pos + 3).toInt();
   
   pos = req.indexOf("PS="); //saves current in preset
   if (pos > 0) {
      savePreset(req.substring(pos + 3).toInt());
   }
   pos = req.indexOf("PL="); //applies entire preset
   if (pos > 0) {
      applyPreset(req.substring(pos + 3).toInt(), true, true, true);
      effectUpdated = true;
   }
   pos = req.indexOf("PA="); //applies brightness from preset
   if (pos > 0) {
      applyPreset(req.substring(pos + 3).toInt(), true, false, false);
   }
   pos = req.indexOf("PC="); //applies color from preset
   if (pos > 0) {
      applyPreset(req.substring(pos + 3).toInt(), false, true, false);
   }
   pos = req.indexOf("PX="); //applies effects from preset
   if (pos > 0) {
      applyPreset(req.substring(pos + 3).toInt(), false, false, true);
      effectUpdated = true;
   }

   //cronixie
   pos = req.indexOf("NX="); //sets digits to code
   if (pos > 0) {
      cronixieDisplay = req.substring(pos + 3, pos + 9);
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
      if (overlayCurrent == 4) strip.setCronixieBacklight(cronixieBacklight);
      overlayRefreshedTime = 0;
   }

   //internal call, does not send XML response
   pos = req.indexOf("IN");
   if (pos < 1) XML_response();
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
