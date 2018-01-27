/*
 * Receives client input
 */

void _setRandomColor(bool _sec)
{
  lastRandomIndex = strip.get_random_wheel_index(lastRandomIndex);
  uint32_t _color = strip.color_wheel(lastRandomIndex);
  if (_sec){
    white_sec = ((_color >> 24) & 0xFF);
    col_sec[0] = ((_color >> 16) & 0xFF);
    col_sec[1] = ((_color >> 8) & 0xFF);
    col_sec[2] = (_color & 0xFF);
  } else {
    white = ((_color >> 24) & 0xFF);
    col[0] = ((_color >> 16) & 0xFF);
    col[1] = ((_color >> 8) & 0xFF);
    col[2] = (_color & 0xFF);
  }
}

void handleSettingsSet()
{
  if (server.hasArg("CSSID")) clientssid = server.arg("CSSID");
  if (server.hasArg("CPASS"))
  {
    if (!server.arg("CPASS").indexOf('*') == 0)
    {
      DEBUG_PRINTLN("Setting pass");
      clientpass = server.arg("CPASS");
    }
  }
  if (server.hasArg("CMDNS")) cmdns = server.arg("CMDNS");
  if (server.hasArg("APWTM"))
  {
      int i = server.arg("APWTM").toInt();
      if (i >= 0 && i <= 255) apWaitTimeSecs = i;
  }
  if (server.hasArg("APSSID")) apssid = server.arg("APSSID");
  aphide = server.hasArg("APHSSID");
  if (server.hasArg("APPASS"))
  {
    if (!server.arg("APPASS").indexOf('*') == 0) appass = server.arg("APPASS");
  }
  if (server.hasArg("APCHAN"))
  {
    int chan = server.arg("APCHAN").toInt();
    if (chan > 0 && chan < 14) apchannel = chan;
  }
  if (server.hasArg("RESET")) //might be dangerous in case arg is always sent
  {
    clearEEPROM();
    server.send(200, "text/plain", "Settings erased. Rebooting...");
    reset();
  }
  if (server.hasArg("CSIP0"))
  {
    int i = server.arg("CSIP0").toInt();
    if (i >= 0 && i <= 255) staticip[0] = i;
  }
  if (server.hasArg("CSIP1"))
  {
    int i = server.arg("CSIP1").toInt();
    if (i >= 0 && i <= 255) staticip[1] = i;
  }
  if (server.hasArg("CSIP2"))
  {
    int i = server.arg("CSIP2").toInt();
    if (i >= 0 && i <= 255) staticip[2] = i;
  }
  if (server.hasArg("CSIP3"))
  {
    int i = server.arg("CSIP3").toInt();
    if (i >= 0 && i <= 255) staticip[3] = i;
  }
  if (server.hasArg("CSGW0"))
  {
    int i = server.arg("CSGW0").toInt();
    if (i >= 0 && i <= 255) staticgateway[0] = i;
  }
  if (server.hasArg("CSGW1"))
  {
    int i = server.arg("CSGW1").toInt();
    if (i >= 0 && i <= 255) staticgateway[1] = i;
  }
  if (server.hasArg("CSGW2"))
  {
    int i = server.arg("CSGW2").toInt();
    if (i >= 0 && i <= 255) staticgateway[2] = i;
  }
  if (server.hasArg("CSGW3"))
  {
    int i = server.arg("CSGW3").toInt();
    if (i >= 0 && i <= 255) staticgateway[3] = i;
  }
  if (server.hasArg("CSSN0"))
  {
    int i = server.arg("CSSN0").toInt();
    if (i >= 0 && i <= 255) staticsubnet[0] = i;
  }
  if (server.hasArg("CSSN1"))
  {
    int i = server.arg("CSSN1").toInt();
    if (i >= 0 && i <= 255) staticsubnet[1] = i;
  }
  if (server.hasArg("CSSN2"))
  {
    int i = server.arg("CSSN2").toInt();
    if (i >= 0 && i <= 255) staticsubnet[2] = i;
  }
  if (server.hasArg("CSSN3"))
  {
    int i = server.arg("CSSN3").toInt();
    if (i >= 0 && i <= 255) staticsubnet[3] = i;
  }
  if (server.hasArg("DESC")) serverDescription = server.arg("DESC");
  useHSBDefault = server.hasArg("COLMD");
  useHSB = useHSBDefault;
  if (server.hasArg("LEDCN"))
  {
    int i = server.arg("LEDCN").toInt();
    if (i >= 0 && i <= LEDCOUNT) ledcount = i;
    strip.setLedCount(ledcount);
  }
  if (server.hasArg("CBEOR")) //ignore settings and save current brightness, colors and fx as default
  {
    col_s[0] = col[0];
    col_s[1] = col[1];
    col_s[2] = col[2];
    if (useRGBW) white_s = white;
    bri_s = bri;
    effectDefault = effectCurrent;
    effectSpeedDefault = effectSpeed;
  } else {
    if (server.hasArg("CLDFR"))
    {
      int i = server.arg("CLDFR").toInt();
      if (i >= 0 && i <= 255) col_s[0] = i;
    }
    if (server.hasArg("CLDFG"))
    {
      int i = server.arg("CLDFG").toInt();
      if (i >= 0 && i <= 255) col_s[1] = i;
    }
    if (server.hasArg("CLDFB"))
    {
      int i = server.arg("CLDFB").toInt();
      if (i >= 0 && i <= 255) col_s[2] = i;
    }
    if (server.hasArg("CSECR"))
    {
      int i = server.arg("CSECR").toInt();
      if (i >= 0 && i <= 255) col_sec_s[0] = i;
    }
    if (server.hasArg("CSECG"))
    {
      int i = server.arg("CSECG").toInt();
      if (i >= 0 && i <= 255) col_sec_s[1] = i;
    }
    if (server.hasArg("CSECB"))
    {
      int i = server.arg("CSECB").toInt();
      if (i >= 0 && i <= 255) col_sec_s[2] = i;
    }
    if (server.hasArg("CSECW"))
    {
      int i = server.arg("CSECW").toInt();
      if (i >= 0 && i <= 255) white_sec_s = i;
    }
    if (server.hasArg("CLDFW"))
    {
      int i = server.arg("CLDFW").toInt();
      if (i >= 0 && i <= 255)
      {
        useRGBW = true;
        white_s = i;
      } else {
        useRGBW = false;
        white_s = 0;
      }
    }
    if (server.hasArg("CLDFA"))
    {
      int i = server.arg("CLDFA").toInt();
      if (i >= 0 && i <= 255) bri_s = i;
    }
    if (server.hasArg("FXDEF"))
    {
      int i = server.arg("FXDEF").toInt();
      if (i >= 0 && i <= 255) effectDefault = i;
    }
    if (server.hasArg("SXDEF"))
    {
      int i = server.arg("SXDEF").toInt();
      if (i >= 0 && i <= 255) effectSpeedDefault = i;
    }
  }
  turnOnAtBoot = server.hasArg("BOOTN");
  if (server.hasArg("BOOTP"))
  {
    int i = server.arg("BOOTP").toInt();
    if (i >= 0 && i <= 25) bootPreset = i;
  }
  useGammaCorrectionBri = server.hasArg("GCBRI");
  useGammaCorrectionRGB = server.hasArg("GCRGB");
  buttonEnabled = server.hasArg("BTNON");
  fadeTransition = server.hasArg("TFADE");
  sweepTransition = server.hasArg("TSWEE");
  sweepDirection = !server.hasArg("TSDIR");
  if (server.hasArg("TDLAY"))
  {
    int i = server.arg("TDLAY").toInt();
    if (i > 0){
      transitionDelay = i;
    }
  }
  if (server.hasArg("TLBRI"))
  {
    bri_nl = server.arg("TLBRI").toInt();
  }
  if (server.hasArg("TLDUR"))
  {
    int i = server.arg("TLDUR").toInt();
    if (i > 0) nightlightDelayMins = i;
  }
  nightlightFade = server.hasArg("TLFDE");
  if (server.hasArg("NUDPP"))
  {
    udpPort = server.arg("NUDPP").toInt();
  }
  receiveNotifications = server.hasArg("NRCVE");
  receiveNotificationsDefault = receiveNotifications;
  if (server.hasArg("NRBRI"))
  {
    int i = server.arg("NRBRI").toInt();
    if (i > 0) bri_n = i;
  }
  notifyDirectDefault = server.hasArg("NSDIR");
  notifyDirect = notifyDirectDefault;
  notifyButton = server.hasArg("NSBTN");
  alexaEnabled = server.hasArg("ALEXA");
  if (server.hasArg("AINVN")) alexaInvocationName = server.arg("AINVN");
  alexaNotify = server.hasArg("NSALX");
  ntpEnabled = server.hasArg("NTPON");
  if (server.hasArg("OLDEF"))
  {
    int i = server.arg("OLDEF").toInt();
    if (i >= 0  && i <= 255) overlayDefault = i;
  }
  if (server.hasArg("WOFFS"))
  {
    int i = server.arg("WOFFS").toInt();
    if (i >= -255  && i <= 255) arlsOffset = i;
    arlsSign = (i>=0)?true:false;
  }
  if (server.hasArg("OPASS"))
  {
    if (!otaLock)
    {
      if (server.arg("OPASS").length() > 0)
      otapass = server.arg("OPASS");
    } else if (!server.hasArg("NOOTA"))
    {
      if (otapass.equals(server.arg("OPASS")))
      {
        otaLock = false;
      }
    }
  }
  if (server.hasArg("NOOTA")) otaLock = true;
  if (server.hasArg("NORAP")) {
    if (!otaLock) recoveryAPDisabled = true;
  } else {
    recoveryAPDisabled = false;
  }
  saveSettingsToEEPROM();
}

boolean handleSet(String req)
{
   boolean effectUpdated = false;
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
      col_sec[0] = req.substring(pos + 3).toInt();
   }
   //set 2nd green value
   pos = req.indexOf("G2=");
   if (pos > 0) {
      col_sec[1] = req.substring(pos + 3).toInt();
   }
   //set 2nd blue value
   pos = req.indexOf("B2=");
   if (pos > 0) {
      col_sec[2] = req.substring(pos + 3).toInt();
   }
   //set 2nd white value
   pos = req.indexOf("W2=");
   if (pos > 0) {
      white_sec = req.substring(pos + 3).toInt();
   }
   //set 2nd to white
   pos = req.indexOf("SW");
   if (pos > 0) {
      if(useRGBW) {
        white_sec = 255;
        col_sec[0] = 0;
        col_sec[1] = 0;
        col_sec[2] = 0;
      } else {
        col_sec[0] = 255;
        col_sec[1] = 255;
        col_sec[2] = 255;
      }
   }
   //set 2nd to black
   pos = req.indexOf("SB");
   if (pos > 0) {
      white_sec = 0;
      col_sec[0] = 0;
      col_sec[1] = 0;
      col_sec[2] = 0;
   }
   //set to random hue SR=0->1st SR=1->2nd
   pos = req.indexOf("SR");
   if (pos > 0) {
      _setRandomColor(req.substring(pos + 3).toInt());
   }
   //set 2nd to 1st
   pos = req.indexOf("SP");
   if (pos > 0) {
      col_sec[0] = col[0];
      col_sec[1] = col[1];
      col_sec[2] = col[2];
      white_sec = white;
   }
   //swap 2nd & 1st
   pos = req.indexOf("SC");
   if (pos > 0) {
      uint8_t _temp[4];
      for (int i = 0; i<3; i++)
      {
        _temp[i] = col[i];
        col[i] = col_sec[i];
        col_sec[i] = _temp[i];
      }
      _temp[3] = white;
      white = white_sec;
      white_sec = _temp[3];
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
   if (req.indexOf("NL=") > 0)
   {
      if (req.indexOf("NL=0") > 0)
      {
        nightlightActive = false;
        bri = bri_t;
      } else {
        nightlightActive = true;
        nightlightStartTime = millis();
      }
   }
   //set nightlight delay
   pos = req.indexOf("ND=");
   if (pos > 0) {
      nightlightDelayMins = req.substring(pos + 3).toInt();
      nightlightActive_old = false; //re-init
   }
   //set nightlight target brightness
   pos = req.indexOf("NT=");
   if (pos > 0) {
      bri_nl = req.substring(pos + 3).toInt();
      nightlightActive_old = false; //re-init
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
      nightlightActive_old = false; //re-init
   }
   //toggle general purpose output
   pos = req.indexOf("AX=");
   if (pos > 0) {
      auxTime = req.substring(pos + 3).toInt();
      auxActive = true;
      if (auxTime == 0) auxActive = false;
   }
   //main toggle on/off
   pos = req.indexOf("&T=");
   if (pos > 0) {
      switch (req.substring(pos + 3).toInt())
      {
        case 0: if (bri != 0){bri_last = bri; bri = 0;} break; //off
        case 1: bri = bri_last; break; //on
        default: if (bri == 0) //toggle
        {
          bri = bri_last;
        } else
        {
          bri_last = bri;
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
   pos = req.indexOf("C0="); if (pos > 0) {cc_start =  (req.substring(pos + 3).toInt()); _cc_updated = true;}
   pos = req.indexOf("C1="); if (pos > 0) {cc_index1 = (req.substring(pos + 3).toInt()); _cc_updated = true;}
   pos = req.indexOf("C2="); if (pos > 0) {cc_index2 = (req.substring(pos + 3).toInt()); _cc_updated = true;}
   pos = req.indexOf("CP="); if (pos > 0) {cc_numPrimary = (req.substring(pos + 3).toInt()); _cc_updated = true;}
   pos = req.indexOf("CS="); if (pos > 0) {cc_numSecondary = (req.substring(pos + 3).toInt()); _cc_updated = true;}
   pos = req.indexOf("CM="); if (pos > 0) {cc_step = (req.substring(pos + 3).toInt()); _cc_updated = true;}
   pos = req.indexOf("CF="); if (pos > 0) {cc_fromStart = (req.substring(pos + 3).toInt()); _cc_updated = true;}
   pos = req.indexOf("CE="); if (pos > 0) {cc_fromEnd = (req.substring(pos + 3).toInt()); _cc_updated = true;}
   if (_cc_updated) strip.setCustomChase(cc_index1, cc_index2, cc_start, cc_numPrimary, cc_numSecondary, cc_step, cc_fromStart, cc_fromEnd);
   
   //set presets
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
   #ifdef CRONIXIE
   pos = req.indexOf("NX="); //sets digits to code
   if (pos > 0) {
      setCronixie(req.substring(pos + 3, pos + 9).c_str());
   }
   pos = req.indexOf("NM="); //mode, 1 countdown
   if (pos > 0) {
      cronixieCountdown = true;
      if (req.indexOf("NM=0") > 0)
      {
        cronixieCountdown = false;
      }
   }
   if (req.indexOf("NB=") > 0) //sets backlight
   {
      cronixieBacklight = true;
      if (req.indexOf("NB=0") > 0)
      {
        cronixieBacklight = false;
      }
      strip.setCronixieBacklight(cronixieBacklight);
      cronixieRefreshedTime = 0;
   }
   #endif
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
