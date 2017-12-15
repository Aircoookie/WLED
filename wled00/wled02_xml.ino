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
     resp = resp + bri_t;
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
   resp = resp + "</nl><fx>";
   resp = resp + effectCurrent;
   resp = resp + "</fx><sx>";
   resp = resp + effectSpeed;
   resp = resp + "</sx><wv>";
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
   //enable toolbar here
   resp = resp + "</vs>";
   server.send(200, "text/xml", resp);
}

String getSettings()
{
  DEBUG_PRINTLN("settings resp");
  
  String resp = "";
  String ds = "d.Sf.";
  String dg = "d.getElementsByClassName";
  String v = ".value=";
  String c = ".checked=";
  String ih = ".innerHTML=";
  
  resp += ds + "CSSID" + v + "\"" + clientssid + "\";";
  resp += ds + "CPASS" + v + "\"";
  for (int i = 0; i < clientpass.length(); i++)
  {
    resp += "*";
  }
  resp += "\";";
  resp += ds + "CSIP0" + v + staticip[0] +";";
  resp += ds + "CSIP1" + v + staticip[1] +";";
  resp += ds + "CSIP2" + v + staticip[2] +";";
  resp += ds + "CSIP3" + v + staticip[3] +";";
  resp += ds + "CSGW0" + v + staticgateway[0] +";";
  resp += ds + "CSGW1" + v + staticgateway[1] +";";
  resp += ds + "CSGW2" + v + staticgateway[2] +";";
  resp += ds + "CSGW3" + v + staticgateway[3] +";";
  resp += ds + "CSSN0" + v + staticsubnet[0] +";";
  resp += ds + "CSSN1" + v + staticsubnet[1] +";";
  resp += ds + "CSSN2" + v + staticsubnet[2] +";";
  resp += ds + "CSSN3" + v + staticsubnet[3] +";";
  resp += ds + "CMDNS" + v + "\"" + cmdns + "\";";
  resp += ds + "APWTM" + v + apWaitTimeSecs +";";
  resp += ds + "APSSID" + v + "\"" + apssid + "\";";
  resp += ds + "APHSSID" + c + aphide + ";";
  resp += ds + "APPASS" + v + "\"";
  for (int i = 0; i < clientpass.length(); i++)
  {
    resp += "*";
  }
  resp += "\";";
  resp += ds + "APCHAN" + v + apchannel +";";

  resp += ds + "DESC" + v + "\"" + serverDescription + "\";";
  resp += ds + "COLMD" + c + useHSBDefault + ";";
  resp += ds + "LEDCN" + v + ledcount +";";
  resp += ds + "CLDFR" + v + col_s[0] +";";
  resp += ds + "CLDFG" + v + col_s[1] +";";
  resp += ds + "CLDFB" + v + col_s[2] +";";
  resp += ds + "CLDFA" + v + bri_s +";";
  if (useRGBW) {
    resp += ds + "CLDFW" + v + white_s +";";
  } else {
    resp += ds + "CLDFW" + v + "-1;";
  }
  resp += ds + "BOOTN" + c + turnOnAtBoot +";";
  resp += ds + "BOOTP" + v + bootPreset +";";
  resp += ds + "FXDEF" + v + effectDefault +";";
  resp += ds + "SXDEF" + v + effectSpeedDefault +";";
  resp += ds + "GCBRI" + c + useGammaCorrectionBri +";";
  resp += ds + "GCRGB" + c + useGammaCorrectionRGB +";";
  resp += ds + "BTNON" + c + buttonEnabled +";";
  resp += ds + "TFADE" + c + fadeTransition +";";
  resp += ds + "TSWEE" + c + sweepTransition +";";
  resp += ds + "TSDIR" + c + !sweepDirection +";";
  resp += ds + "TDLAY" + v + transitionDelay +";";
  resp += ds + "TLBRI" + v + bri_nl +";";
  resp += ds + "TLDUR" + v + nightlightDelayMins +";";
  resp += ds + "TLFDE" + c + nightlightFade +";";
  resp += ds + "NUDPP" + v + udpPort +";";
  resp += ds + "NRBRI" + v + bri_n +";";
  resp += ds + "NRCVE" + c + receiveNotificationsDefault +";";
  resp += ds + "NSDIR" + c + notifyDirectDefault +";";
  resp += ds + "NSBTN" + c + notifyButton +";";
  resp += ds + "NTPON" + c + ntpEnabled +";";
  resp += ds + "ALEXA" + c + alexaEnabled +";";
  resp += ds + "AINVN" + v + "\"" + alexaInvocationName + "\";";
  resp += ds + "NSALX" + c + alexaNotify +";";
  resp += dg + "(\"times\")[0]" + ih + "\"" + getTimeString() + "\";";
  resp += ds + "OLDEF" + v + overlayDefault +";";
  resp += ds + "WOFFS" + v + arlsOffset +";";
  resp += ds + "NOOTA" + c + otaLock +";";
  resp += ds + "NORAP" + c + recoveryAPDisabled +";";
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
  resp += dg + "(\"msg\")[0]" + ih + "\"WLED 0.5dev (build " + VERSION + ") OK\";";
  
  return resp;
}
