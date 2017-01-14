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
   resp = resp + notifyMaster;
   resp = resp + "</ns>";
   resp = resp + "<nr>";
   resp = resp + receiveNotifications;
   resp = resp + "</nr>";
   resp = resp + "<nl>";
   resp = resp + nightlightActive;
   resp = resp + "</nl>";
   resp = resp + "<fx>";
   resp = resp + effectCurrent;
   resp = resp + "</fx>";
   resp = resp + "<sx>";
   resp = resp + effectSpeed;
   resp = resp + "</sx>";
   resp = resp + "<desc>";
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
  for (int i = 0; i < 3; i++)
  {
    resp = resp + "<cldef>";
    resp = resp + col_s[i];
    resp = resp + "</cldef>";
  }
  resp = resp + "<cldfa>";
  resp = resp + bri_s;
  resp = resp + "</cldfa>";
  resp = resp + "<fxdef>";
  resp = resp + effectDefault;
  resp = resp + "</fxdef>";
  resp = resp + "<sxdef>";
  resp = resp + effectSpeedDefault;
  resp = resp + "</sxdef>";
  resp = resp + "<btnon>";
  resp = resp + bool2int(buttonEnabled);
  resp = resp + "</btnon><tfade>";
  resp = resp + bool2int(fadeTransition);
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
  resp = resp + bool2int(nightlightFade);
  resp = resp + "</tlfde>";
  resp = resp + "<nudpp>";
  resp = resp + udpPort;
  resp = resp + "</nudpp>";
  resp = resp + "<nrcve>";
  resp = resp + bool2int(receiveNotificationsDefault);
  resp = resp + "</nrcve><nrbri>";
  resp = resp + bri_n;
  resp = resp + "</nrbri><nsdir>";
  resp = resp + bool2int(notifyDirect);
  resp = resp + "</nsdir><nsbtn>";
  resp = resp + bool2int(notifyButton);
  resp = resp + "</nsbtn><nsfwd>";
  resp = resp + bool2int(notifyNightlight);
  resp = resp + "</nsfwd>";
  resp = resp + "<ntpon>";
  resp = resp + bool2int(ntpEnabled);
  resp = resp + "</ntpon>";
  DEBUG_PRINTLN("pretime");
  resp = resp + "<times>";
  resp = resp + getTimeString();
  resp = resp + "</times>";
  DEBUG_PRINTLN("posttime");
  resp = resp + "<noota>";
  resp = resp + bool2int(ota_lock);
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
  resp = resp + "<msg>WLED 0.3pd OK</msg>";
  resp = resp + "</vs>";
  DEBUG_PRINTLN(resp);
  server.send(200, "text/xml", resp);
}
