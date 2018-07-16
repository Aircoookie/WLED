/*
 * Remote light control with the free Blynk app
 */

uint16_t blHue = 0;
byte blSat = 255;

void initBlynk(const char* auth)
{
  if (WiFi.status() != WL_CONNECTED) return;
  blynkEnabled = (auth[0] != 0);
  if (blynkEnabled) Blynk.config(auth);
}

void handleBlynk()
{
  if (WiFi.status() == WL_CONNECTED && blynkEnabled)
  Blynk.run();
}

BLYNK_WRITE(V0)
{
 bri = param.asInt();//bri
 colorUpdated(1);
}

BLYNK_WRITE(V1)
{
 blHue = param.asInt();//hue
 colorHStoRGB(blHue*10,blSat,(false)? colSec:col);
 colorUpdated(1);
}

BLYNK_WRITE(V2)
{
 blSat = param.asInt();//sat
 colorHStoRGB(blHue*10,blSat,(false)? colSec:col);
 colorUpdated(1);
}

BLYNK_WRITE(V3)
{
  handleSet((param.asInt()>0)?"win&T=1&IN":"win&T=0&IN");//power
}

BLYNK_WRITE(V4)
{
 effectCurrent = param.asInt()-1;//fx
 colorUpdated(6);
}

BLYNK_WRITE(V5)
{
 effectSpeed = param.asInt();//sx
 colorUpdated(6);
}

BLYNK_WRITE(V6)
{
 effectIntensity = param.asInt();//ix
 colorUpdated(6);
}

BLYNK_WRITE(V7)
{
  handleSet((param.asInt()>0)?"win&ND&IN":"win&NL=0&IN");//nl
}

BLYNK_WRITE(V8)
{
  notifyDirect = (param.asInt()>0); //send notifications
}

