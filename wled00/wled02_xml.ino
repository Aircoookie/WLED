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
   if (rgbwEnabled) {resp = resp + whiteVal;}
   else {resp = resp + "-1";}
   resp = resp + "</wv><md>";
   resp = resp + useHSB;
   resp = resp + "</md><desc>";
   resp = resp + serverDescription;
   resp = resp + "</desc>";
   //enable toolbar here
   resp = resp + "</vs>";
   server.send(200, "text/xml", resp);
}

void XML_response_settings()
{
  DEBUG_PRINTLN("XML settings response");
  String resp;
  resp = resp + "<?xml version = \"1.0\" ?>";
  resp = resp + "<vs>";
  resp = resp + "<cssid>";
  resp = resp + clientssid;
  resp = resp + "</cssid>";
  resp = resp + "<cpass>";
  for (int i = 0; i < clientpass.length(); i++)
  {
    resp = resp + "*";
  }
  resp = resp + "</cpass>";
  for (int i = 0; i < 4; i++)
  {
    resp = resp + "<csips>";
    resp = resp + staticip[i];
    resp = resp + "</csips>";
  }
  for (int i = 0; i < 4; i++)
  {
    resp = resp + "<csgws>";
    resp = resp + staticgateway[i];
    resp = resp + "</csgws>";
  }
  for (int i = 0; i < 4; i++)
  {
    resp = resp + "<cssns>";
    resp = resp + staticsubnet[i];
    resp = resp + "</cssns>";
  }
  resp = resp + "<cmdns>";
  resp = resp + cmdns;
  resp = resp + "</cmdns>";
  resp = resp + "<apssid>";
  resp = resp + apssid;
  resp = resp + "</apssid>";
  resp = resp + "<aphssid>";
  resp = resp + aphide;
  resp = resp + "</aphssid>";
  resp = resp + "<appass>";
  for (int i = 0; i < appass.length(); i++)
  {
    resp = resp + "*";
  }
  resp = resp + "</appass>";
  resp = resp + "<apchan>";
  resp = resp + apchannel;
  resp = resp + "</apchan>";
  resp = resp + "<desc>";
  resp = resp + serverDescription;
  resp = resp + "</desc>";
  resp = resp + "<colmd>";
  resp = resp + useHSBDefault;
  resp = resp + "</colmd>";
  resp = resp + "<ledcn>";
  resp = resp + ledcount;
  resp = resp + "</ledcn>";
  for (int i = 0; i < 3; i++)
  {
    resp = resp + "<cldef>";
    resp = resp + col_s[i];
    resp = resp + "</cldef>";
  }
  resp = resp + "<cldfa>";
  resp = resp + bri_s;
  resp = resp + "</cldfa>";
  resp = resp + "<bootn>";
  resp = resp + turnOnAtBoot;
  resp = resp + "</bootn>";
  resp = resp + "<fxdef>";
  resp = resp + effectDefault;
  resp = resp + "</fxdef>";
  resp = resp + "<sxdef>";
  resp = resp + effectSpeedDefault;
  resp = resp + "</sxdef>";
  resp = resp + "<gcbri>";
  resp = resp + useGammaCorrectionBri;
  resp = resp + "</gcbri><gcrgb>";
  resp = resp + useGammaCorrectionRGB;
  resp = resp + "</gcrgb>";
  resp = resp + "<btnon>";
  resp = resp + buttonEnabled;
  resp = resp + "</btnon><tfade>";
  resp = resp + fadeTransition;
  resp = resp + "</tfade><tdlay>";
  resp = resp + transitionDelay;
  resp = resp + "</tdlay>";
  resp = resp + "<tlbri>";
  resp = resp + bri_nl;
  resp = resp + "</tlbri>";
  resp = resp + "<tldur>";
  resp = resp + nightlightDelayMins;
  resp = resp + "</tldur>";
  resp = resp + "<tlfde>";
  resp = resp + nightlightFade;
  resp = resp + "</tlfde>";
  resp = resp + "<nudpp>";
  resp = resp + udpPort;
  resp = resp + "</nudpp>";
  resp = resp + "<nrcve>";
  resp = resp + receiveNotificationsDefault;
  resp = resp + "</nrcve><nrbri>";
  resp = resp + bri_n;
  resp = resp + "</nrbri><nsdir>";
  resp = resp + notifyDirectDefault;
  resp = resp + "</nsdir><nsbtn>";
  resp = resp + notifyButton;
  resp = resp + "</nsbtn><nsfwd>0</nsfwd>"; //legacy
  resp = resp + "<ntpon>";
  resp = resp + ntpEnabled;
  resp = resp + "</ntpon>";
  resp = resp + "<alexa>";
  resp = resp + alexaEnabled;
  resp = resp + "</alexa><ainvn>";
  resp = resp + alexaInvocationName;
  resp = resp + "</ainvn><nsalx>";
  resp = resp + alexaNotify;
  resp = resp + "</nsalx>";
  DEBUG_PRINTLN("pretime");
  resp = resp + "<times>";
  resp = resp + getTimeString();
  resp = resp + "</times>";
  resp = resp + "<oldef>";
  resp = resp + overlayDefault;
  resp = resp + "</oldef>";
  resp = resp + "<woffs>";
  resp = resp + abs(arlsOffset);
  resp = resp + "</woffs>";
  resp = resp + "<woffn>";
  resp = resp + !arlsSign;
  resp = resp + "</woffn>";
  resp = resp + "<noota>";
  resp = resp + ota_lock;
  resp = resp +"</noota>";
  resp = resp + "<norap>0</norap>"; //NI
  resp = resp + "<sip>";
  if (!WiFi.localIP()[0] == 0)
  {
    resp = resp + WiFi.localIP()[0];
    resp = resp + ".";
    resp = resp + WiFi.localIP()[1];
    resp = resp + ".";
    resp = resp + WiFi.localIP()[2];
    resp = resp + ".";
    resp = resp + WiFi.localIP()[3];
  } else
  {
    resp = resp + "Not connected";
  }
  resp = resp + "</sip><sip>";
  if (!WiFi.softAPIP()[0] == 0)
  {
    resp = resp + WiFi.softAPIP()[0];
    resp = resp + ".";
    resp = resp + WiFi.softAPIP()[1];
    resp = resp + ".";
    resp = resp + WiFi.softAPIP()[2];
    resp = resp + ".";
    resp = resp + WiFi.softAPIP()[3];
  } else
  {
    resp = resp + "Not active";
  }
  resp = resp + "</sip>";
  resp = resp + "<msg>WLED 0.3 (build ";
  resp = resp + VERSION;
  resp = resp + ") OK</msg>";
  resp = resp + "</vs>";
  DEBUG_PRINTLN(resp);
  server.send(200, "text/xml", resp);
}
