void XML_response()
{
   String resp;
   resp = resp + "<?xml version = \"1.0\" ?>";
   resp = resp + "<vs>";
   resp = resp + "<act>";
   resp = resp + bri;
   resp = resp + "</act>";

   for (int i = 0; i < 3; i++)
   {
     resp = resp + "<cl>";
     resp = resp + col[i];
     resp = resp + "</cl>";
   }
   //enable toolbar here
   resp = resp + "</vs>";
   server.send(200, "text/xml", resp);
}

void XML_response_settings()
{
  Serial.println("XML settings response");
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
  resp = resp + "<leds>";
  resp = resp + led_amount;
  resp = resp + "</leds>";
  resp = resp + "<btnon>";
  resp = resp + bool2int(buttonEnabled);
  resp = resp + "</btnon><tfade>";
  resp = resp + bool2int(fadeTransition);
  resp = resp + "</tfade><tdlay>";
  resp = resp + transitionDelay;
  resp = resp + "</tdlay>";
  resp = resp + "<nrcve>";
  resp = resp + bool2int(receiveNotifications);
  resp = resp + "</nrcve><nrbri>";
  resp = resp + bri_n;
  resp = resp + "</nrbri><nsdir>";
  resp = resp + bool2int(notifyDirect);
  resp = resp + "</nsdir><nsbtn>";
  resp = resp + bool2int(notifyButton);
  resp = resp + "</nsbtn><nsfwd>";
  resp = resp + bool2int(notifyForward);
  resp = resp + "</nsfwd><nsips>";
  for (int i = 0; i < notifier_ips_count; i++)
  {
    resp = resp + notifier_ips[i];
    resp = resp + "\n";
  }
  resp = resp + "</nsips>";
  resp = resp + "<noota>0</noota>"; //NI
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
  resp = resp + "</sip><otastat>Not implemented</otastat>";
  resp = resp + "<msg>WLED 0.3pd OK</msg>";
  resp = resp + "</vs>";
  Serial.println(resp);
  server.send(200, "text/xml", resp);
}
