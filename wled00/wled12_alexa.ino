void alexaOn();
void alexaOff();

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
  if (alexaNotify)
  {
    handleSet("win&T=1&IN");
  } else
  {
    handleSet("win&T=1&NN&IN");
  }
}

void alexaOff()
{
  if (alexaNotify)
  {
    handleSet("win&T=0&IN");
  } else
  {
    handleSet("win&T=0&NN&IN");
  }
}

