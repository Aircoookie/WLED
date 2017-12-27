void alexaOn();
void alexaOff();
void alexaDim();

void alexaInit()
{
  if (alexaEnabled && WiFi.status() == WL_CONNECTED)
  {
    upnpBroadcastResponder.beginUdpMulticast();
    alexa = new Switch(alexaInvocationName, 81, alexaOn, alexaOff);
    upnpBroadcastResponder.addDevice(*alexa);
  }
}

void handleAlexa()
{
  if (alexaEnabled && WiFi.status() == WL_CONNECTED)
  {
    upnpBroadcastResponder.serverLoop();
    alexa->serverLoop();
  }
}

void alexaOn()
{
  if (alexaOnMacro == 255)
  {
    handleSet((alexaNotify)?"win&T=1&IN":"win&T=1&NN&IN");
  } else
  {
    applyMacro(alexaOnMacro);
  }
}

void alexaOff()
{
  if (alexaOffMacro == 255)
  {
    handleSet((alexaNotify)?"win&T=0&IN":"win&T=0&NN&IN");
  } else
  {
    applyMacro(alexaOffMacro);
  }
}

void alexaDim(uint8_t bri)
{
  String ct = (alexaNotify)?"win&IN&A=":"win&NN&IN&A=";
  ct = ct + bri;
  handleSet(ct);
}

