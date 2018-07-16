/*
 * Sending XML status files to client
 */

void XML_response()
{
   String resp;
   resp = resp + "<?xml version = \"1.0\" ?>";
   resp = resp + "<vs>";
   resp = resp + "<ac>";
   if (nightlightActive && nightlightFade)
   {
     resp = resp + briT;
   } else
   {
    resp = resp + bri;
   }
   resp = resp + "</ac>";

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
   if (useRGBW && !autoRGBtoRGBW) {
     resp = resp + white;
   } else {
     resp = resp + "-1";
   }
   resp = resp + "</wv><md>";
   resp = resp + useHSB;
   resp = resp + "</md><ds>";
   resp = resp + serverDescription;
   resp = resp + "</ds>";
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

  resp.reserve(1000);
  String ds = "d.Sf.";
  String dg = "d.getElementsByClassName";
  String v = ".value=";
  String c = ".checked=";
  String ih = ".innerHTML=";
  String si = ".selectedIndex=";

  if (subPage == 1) {
    resp += ds + "CS" + v + "\"" + clientSSID + "\";";
    resp += ds + "CP" + v + "\"";
    for (int i = 0; i < clientPass.length(); i++)
    {
      resp += "*";
    }
    resp += "\";";
    resp += ds + "I0" + v + staticIP[0] +";";
    resp += ds + "I1" + v + staticIP[1] +";";
    resp += ds + "I2" + v + staticIP[2] +";";
    resp += ds + "I3" + v + staticIP[3] +";";
    resp += ds + "G0" + v + staticGateway[0] +";";
    resp += ds + "G1" + v + staticGateway[1] +";";
    resp += ds + "G2" + v + staticGateway[2] +";";
    resp += ds + "G3" + v + staticGateway[3] +";";
    resp += ds + "S0" + v + staticSubnet[0] +";";
    resp += ds + "S1" + v + staticSubnet[1] +";";
    resp += ds + "S2" + v + staticSubnet[2] +";";
    resp += ds + "S3" + v + staticSubnet[3] +";";
    resp += ds + "CM" + v + "\"" + cmDNS + "\";";
    resp += ds + "AT" + v + apWaitTimeSecs +";";
    resp += ds + "AS" + v + "\"" + apSSID + "\";";
    resp += ds + "AH" + c + apHide + ";";
    resp += ds + "AP" + v + "\"";
    for (int i = 0; i < apPass.length(); i++)
    {
      resp += "*";
    }
    resp += "\";";
    resp += ds + "AC" + v + apChannel +";";
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
    resp += ds + "LC" + v + ledCount +";";
    resp += ds + "CR" + v + colS[0] +";";
    resp += ds + "CG" + v + colS[1] +";";
    resp += ds + "CB" + v + colS[2] +";";
    resp += ds + "CA" + v + briS +";";
    resp += ds + "EW" + c + useRGBW +";";
    resp += ds + "AW" + c + autoRGBtoRGBW +";";
    resp += ds + "CW" + v + whiteS +";";
    resp += ds + "SR" + v + colSecS[0] +";";
    resp += ds + "SG" + v + colSecS[1] +";";
    resp += ds + "SB" + v + colSecS[2] +";";
    resp += ds + "SW" + v + whiteSecS +";";
    resp += ds + "BO" + c + turnOnAtBoot +";";
    resp += ds + "BP" + v + bootPreset +";";
    resp += ds + "FX" + v + effectDefault +";";
    resp += ds + "SX" + v + effectSpeedDefault +";";
    resp += ds + "IX" + v + effectIntensityDefault +";";
    resp += ds + "GB" + c + useGammaCorrectionBri +";";
    resp += ds + "GC" + c + useGammaCorrectionRGB +";";
    resp += ds + "TF" + c + fadeTransition +";";
    resp += ds + "TS" + c + sweepTransition +";";
    resp += ds + "TI" + c + !sweepDirection +";";
    resp += ds + "TD" + v + transitionDelay +";";
    resp += ds + "T2" + c + !disableSecTransition +";";
    resp += ds + "BF" + v + briMultiplier +";";
    resp += ds + "TB" + v + nightlightTargetBri +";";
    resp += ds + "TL" + v + nightlightDelayMins +";";
    resp += ds + "TW" + c + nightlightFade +";";
    resp += ds + "RV" + c + reverseMode +";";
    resp += ds + "EI" + c + initLedsLast +";";
    resp += ds + "WO" + v + arlsOffset +";";
    resp += ds + "SL" + c + skipFirstLed +";";
  }

  if (subPage == 3)
  { 
    resp += ds + "UI" + si + String(uiConfiguration) + ";";
    resp += ds + "DS" + v + "\"" + serverDescription + "\";";
    resp += ds + "MD" + c + useHSBDefault + ";";
    resp += ds + "TH" + si + String(currentTheme) + ";";
    for(int i=0;i<6;i++)
    resp += ds + "C" + i + v + "\"" + cssCol[i] + "\";";
    resp += ds + "CF" + v + "\"" + cssFont + "\";";
  }

  if (subPage == 4)
  {
    resp += ds + "BT" + c + buttonEnabled +";";
    resp += ds + "UP" + v + udpPort +";";
    resp += ds + "RB" + c + receiveNotificationBrightness +";";
    resp += ds + "RC" + c + receiveNotificationColor +";";
    resp += ds + "RX" + c + receiveNotificationEffects +";";
    resp += ds + "SD" + c + notifyDirectDefault +";";
    resp += ds + "SB" + c + notifyButton +";";
    resp += ds + "SH" + c + notifyHue +";";
    resp += ds + "S2" + c + notifyTwice +";";
    resp += ds + "RD" + c + receiveDirect +";";
    resp += ds + "RU" + c + enableRealtimeUI +";";
    resp += ds + "AL" + c + alexaEnabled +";";
    resp += ds + "AI" + v + "\"" + alexaInvocationName + "\";";
    resp += ds + "SA" + c + alexaNotify +";";
    resp += ds + "BK" + v + "\"" + ((blynkEnabled)?"Hidden":"") + "\";";
    resp += ds + "H0" + v + hueIP[0] +";";
    resp += ds + "H1" + v + hueIP[1] +";";
    resp += ds + "H2" + v + hueIP[2] +";";
    resp += ds + "H3" + v + hueIP[3] +";";
    resp += ds + "HL" + v + huePollLightId +";";
    resp += ds + "HI" + v + huePollIntervalMs +";";
    resp += ds + "HP" + c + huePollingEnabled +";";
    resp += ds + "HO" + c + hueApplyOnOff +";";
    resp += ds + "HB" + c + hueApplyBri +";";
    resp += ds + "HC" + c + hueApplyColor +";";
    resp += dg + "(\"hms\")[0]" + ih + "\"" + hueError + "\";";
  }

  if (subPage == 5)
  {
    resp += ds + "NT" + c + ntpEnabled +";";
    resp += ds + "CF" + c + !useAMPM +";";
    resp += ds + "TZ" + si + String(currentTimezone) + ";";
    resp += ds + "UO" + v + utcOffsetSecs +";";
    resp += dg + "(\"times\")[0]" + ih + "\"" + getTimeString() + "\";";
    resp += ds + "OL" + si + String(overlayCurrent) + ";";
    resp += ds + "O1" + v + overlayMin +";";
    resp += ds + "O2" + v + overlayMax +";";
    resp += ds + "OM" + v + analogClock12pixel +";";
    resp += ds + "OS" + c + analogClockSecondsTrail +";";
    resp += ds + "O5" + c + analogClock5MinuteMarks +";";
    resp += ds + "CX" + v + "\"" + cronixieDisplay + "\";";
    resp += ds + "CB" + c + cronixieBacklight +";";
    resp += ds + "CE" + c + countdownMode +";";
    resp += ds + "CY" + v + countdownYear +";";
    resp += ds + "CI" + v + countdownMonth +";";
    resp += ds + "CD" + v + countdownDay +";";
    resp += ds + "CH" + v + countdownHour +";";
    resp += ds + "CM" + v + countdownMin +";";
    resp += ds + "CS" + v + countdownSec +";";
    for (int i=1;i<17;i++)
    {
      resp += ds + "M" + String(i) + v + "\"" + loadMacro(i) + "\";";
    }
    resp += ds + "MB" + v + macroBoot +";";
    resp += ds + "A0" + v + macroAlexaOn +";";
    resp += ds + "A1" + v + macroAlexaOff +";";
    resp += ds + "MP" + v + macroButton +";";
    resp += ds + "ML" + v + macroLongPress +";";
    resp += ds + "MC" + v + macroCountdown +";";
    resp += ds + "MN" + v + macroNl +";";
  }

  if (subPage == 6)
  {
    resp += ds + "NO" + c + otaLock +";";
    resp += ds + "OW" + c + wifiLock +";";
    resp += ds + "AO" + c + aOtaEnabled +";";
    resp += ds + "NA" + c + recoveryAPDisabled +";";
    resp += dg + "(\"msg\")[0]" + ih + "\"WLED "+ versionString +" (build " + VERSION + ") OK\";";
  }
  resp += "}</script>";
  
  return resp;
}
