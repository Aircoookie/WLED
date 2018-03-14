/*
 * Sending XML status files to client
 */

void XML_response()
{
   String resp;
   resp = resp + "<?xml version = \"1.0\" ?>";
   resp = resp + "<vs>";
   resp = resp + "<act>";
   if (nightlightActive && nightlightFade)
   {
     resp = resp + briT;
   } else
   {
    resp = resp + bri;
   }
   resp = resp + "</act>";

   for (int i = 0; i < 3; i++)
   {
     resp = resp + "<cl>";
     resp = resp + col[i];
     resp = resp + "</cl>";
   }
   resp = resp + "<ns>";
   resp = resp + notifyDirect;
   resp = resp + "</ns><nr>";
   resp = resp + receiveNotifications;
   resp = resp + "</nr><nl>";
   resp = resp + nightlightActive;
   resp = resp + "</nl><nf>";
   resp = resp + nightlightFade;
   resp = resp + "</nf><nd>";
   resp = resp + nightlightDelayMins;
   resp = resp + "</nd><nt>";
   resp = resp + nightlightTargetBri;
   resp = resp + "</nt><fx>";
   resp = resp + effectCurrent;
   resp = resp + "</fx><sx>";
   resp = resp + effectSpeed;
   resp = resp + "</sx><ix>";
   resp = resp + effectIntensity;
   resp = resp + "</ix><wv>";
   if (useRGBW) {
     resp = resp + white;
   } else {
     resp = resp + "-1";
   }
   resp = resp + "</wv><md>";
   resp = resp + useHSB;
   resp = resp + "</md><desc>";
   resp = resp + serverDescription;
   resp = resp + "</desc>";
   resp = resp + "</vs>";
   server.send(200, "text/xml", resp);
}

String getSettings(byte subPage)
{
  //0: menu 1: wifi 2: leds 3: ui 4: sync 5: time 6: sec
  DEBUG_PRINT("settings resp");
  DEBUG_PRINTLN(subPage);
  
  String resp = "";
  if (subPage <1 || subPage >6) return resp;
  
  String ds = "d.Sf.";
  String dg = "d.getElementsByClassName";
  String v = ".value=";
  String c = ".checked=";
  String ih = ".innerHTML=";
  String si = ".selectedIndex=";

  if (subPage == 1) {
    resp += ds + "CSSID" + v + "\"" + clientSSID + "\";";
    resp += ds + "CPASS" + v + "\"";
    for (int i = 0; i < clientPass.length(); i++)
    {
      resp += "*";
    }
    resp += "\";";
    resp += ds + "CSIP0" + v + staticIP[0] +";";
    resp += ds + "CSIP1" + v + staticIP[1] +";";
    resp += ds + "CSIP2" + v + staticIP[2] +";";
    resp += ds + "CSIP3" + v + staticIP[3] +";";
    resp += ds + "CSGW0" + v + staticGateway[0] +";";
    resp += ds + "CSGW1" + v + staticGateway[1] +";";
    resp += ds + "CSGW2" + v + staticGateway[2] +";";
    resp += ds + "CSGW3" + v + staticGateway[3] +";";
    resp += ds + "CSSN0" + v + staticSubnet[0] +";";
    resp += ds + "CSSN1" + v + staticSubnet[1] +";";
    resp += ds + "CSSN2" + v + staticSubnet[2] +";";
    resp += ds + "CSSN3" + v + staticSubnet[3] +";";
    resp += ds + "CMDNS" + v + "\"" + cmDNS + "\";";
    resp += ds + "APWTM" + v + apWaitTimeSecs +";";
    resp += ds + "APSSID" + v + "\"" + apSSID + "\";";
    resp += ds + "APHSSID" + c + apHide + ";";
    resp += ds + "APPASS" + v + "\"";
    for (int i = 0; i < apPass.length(); i++)
    {
      resp += "*";
    }
    resp += "\";";
    resp += ds + "APCHAN" + v + apChannel +";";
    resp += dg + "(\"sip\")[0]" + ih + "\"";
    if (!WiFi.localIP()[0] == 0)
    {
      resp += WiFi.localIP()[0];
      resp += + ".";
      resp += WiFi.localIP()[1];
      resp += ".";
      resp += WiFi.localIP()[2];
      resp += ".";
      resp += WiFi.localIP()[3];
    } else
    {
      resp += "Not connected";
    }
    resp += "\";";
    resp += dg + "(\"sip\")[1]" + ih + "\"";
    if (!WiFi.softAPIP()[0] == 0)
    {
      resp += WiFi.softAPIP()[0];
      resp += + ".";
      resp += WiFi.softAPIP()[1];
      resp += ".";
      resp += WiFi.softAPIP()[2];
      resp += ".";
      resp += WiFi.softAPIP()[3];
    } else
    {
      resp += "Not active";
    }
    resp += "\";";
  }
  
  if (subPage == 2) {
    resp += ds + "LEDCN" + v + ledCount +";";
    resp += ds + "CLDFR" + v + colS[0] +";";
    resp += ds + "CLDFG" + v + colS[1] +";";
    resp += ds + "CLDFB" + v + colS[2] +";";
    resp += ds + "CLDFA" + v + briS +";";
    if (useRGBW) {
      resp += ds + "CLDFW" + v + whiteS +";";
    } else {
      resp += ds + "CLDFW" + v + "-1;";
    }
    resp += ds + "CSECR" + v + colSecS[0] +";";
    resp += ds + "CSECG" + v + colSecS[1] +";";
    resp += ds + "CSECB" + v + colSecS[2] +";";
    resp += ds + "CSECW" + v + whiteSecS +";";
    resp += ds + "BOOTN" + c + turnOnAtBoot +";";
    resp += ds + "BOOTP" + v + bootPreset +";";
    resp += ds + "FXDEF" + v + effectDefault +";";
    resp += ds + "SXDEF" + v + effectSpeedDefault +";";
    resp += ds + "IXDEF" + v + effectIntensityDefault +";";
    resp += ds + "GCBRI" + c + useGammaCorrectionBri +";";
    resp += ds + "GCRGB" + c + useGammaCorrectionRGB +";";
    resp += ds + "TFADE" + c + fadeTransition +";";
    resp += ds + "TSWEE" + c + sweepTransition +";";
    resp += ds + "TSDIR" + c + !sweepDirection +";";
    resp += ds + "TDLAY" + v + transitionDelay +";";
    resp += ds + "NRBRI" + v + briMultiplier +";";
    resp += ds + "TLBRI" + v + nightlightTargetBri +";";
    resp += ds + "TLDUR" + v + nightlightDelayMins +";";
    resp += ds + "TLFDE" + c + nightlightFade +";";
    resp += ds + "LEDRV" + c + reverseMode +";";
    resp += ds + "WOFFS" + v + arlsOffset +";";
  }

  if (subPage == 3)
  { 
    resp += ds + "DESC" + v + "\"" + serverDescription + "\";";
    resp += ds + "COLMD" + c + useHSBDefault + ";";
    resp += ds + "THEME" + si + String(currentTheme) + ";";
    for(int i=0;i<6;i++)
    resp += ds + "CCOL" + i + v + "\"" + cssCol[i] + "\";";
    resp += ds + "CFONT" + v + "\"" + cssFont + "\";";
  }

  if (subPage == 4)
  {
    resp += ds + "BTNON" + c + buttonEnabled +";";
    resp += ds + "NUDPP" + v + udpPort +";";
    resp += ds + "NRCBR" + c + receiveNotificationBrightness +";";
    resp += ds + "NRCCL" + c + receiveNotificationColor +";";
    resp += ds + "NRCFX" + c + receiveNotificationEffects +";";
    resp += ds + "NSDIR" + c + notifyDirectDefault +";";
    resp += ds + "NSBTN" + c + notifyButton +";";
    resp += ds + "NSHUE" + c + notifyHue +";";
    resp += ds + "NS2XS" + c + notifyTwice +";";
    resp += ds + "ALEXA" + c + alexaEnabled +";";
    resp += ds + "AINVN" + v + "\"" + alexaInvocationName + "\";";
    resp += ds + "NSALX" + c + alexaNotify +";";
    resp += ds + "HUIP0" + v + hueIP[0] +";";
    resp += ds + "HUIP1" + v + hueIP[1] +";";
    resp += ds + "HUIP2" + v + hueIP[2] +";";
    resp += ds + "HUIP3" + v + hueIP[3] +";";
    resp += ds + "HUELI" + v + huePollLightId +";";
    resp += ds + "HUEPI" + v + huePollIntervalMs +";";
    resp += ds + "HUEPL" + c + huePollingEnabled +";";
    resp += ds + "HURIO" + c + hueApplyOnOff +";";
    resp += ds + "HURBR" + c + hueApplyBri +";";
    resp += ds + "HURCL" + c + hueApplyColor +";";
    resp += dg + "(\"hms\")[0]" + ih + "\"" + hueError + "\";";
  }

  if (subPage == 5)
  {
    resp += ds + "NTPON" + c + ntpEnabled +";";
    resp += ds + "CL24H" + c + !useAMPM +";";
    resp += ds + "TZONE" + si + String(currentTimezone) + ";";
    resp += ds + "UTCOS" + v + utcOffsetSecs +";";
    resp += dg + "(\"times\")[0]" + ih + "\"" + getTimeString() + "\";";
    resp += ds + "OLMDE" + si + String(overlayCurrent) + ";";
    resp += ds + "OLIN1" + v + overlayMin +";";
    resp += ds + "OLIN2" + v + overlayMax +";";
    resp += ds + "OLINM" + v + analogClock12pixel +";";
    resp += ds + "OLSTR" + c + analogClockSecondsTrail +";";
    resp += ds + "OL5MI" + c + analogClock5MinuteMarks +";";
    resp += ds + "CRONX" + v + "\"" + cronixieDisplay + "\";";
    resp += ds + "CROBL" + c + cronixieBacklight +";";
    resp += ds + "CLCND" + c + countdownMode +";";
    resp += ds + "CDGYR" + v + countdownYear +";";
    resp += ds + "CDGMN" + v + countdownMonth +";";
    resp += ds + "CDGDY" + v + countdownDay +";";
    resp += ds + "CDGHR" + v + countdownHour +";";
    resp += ds + "CDGMI" + v + countdownMin +";";
    resp += ds + "CDGSC" + v + countdownSec +";";
    for (int i=1;i<17;i++)
    {
      resp += ds + "MC" + String(i) + v + "\"" + loadMacro(i) + "\";";
    }
    resp += ds + "MCRBT" + v + macroBoot +";";
    resp += ds + "MCA0I" + v + macroAlexaOn +";";
    resp += ds + "MCA0O" + v + macroAlexaOff +";";
    resp += ds + "MCB0D" + v + macroButton +";";
    resp += ds + "MCB0L" + v + macroLongPress +";";
    resp += ds + "MCNTD" + v + macroCountdown +";";
    resp += ds + "MCNLO" + v + macroNl +";";
  }

  if (subPage == 6)
  {
    resp += ds + "NOOTA" + c + otaLock +";";
    resp += ds + "OWIFI" + c + wifiLock +";";
    resp += ds + "AROTA" + c + aOtaEnabled +";";
    resp += ds + "NORAP" + c + recoveryAPDisabled +";";
    resp += dg + "(\"msg\")[0]" + ih + "\"WLED "+ versionString +" (build " + VERSION + ") OK\";";
  }
  resp += "}</script>";
  
  return resp;
}
