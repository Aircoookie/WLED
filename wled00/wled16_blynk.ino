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

void updateBlynk()
{
  if (onlyAP) return;
  Blynk.virtualWrite(V0, bri);
  //we need a RGB -> HSB convert here
  Blynk.virtualWrite(V3, bri? 1:0);
  Blynk.virtualWrite(V4, effectCurrent);
  Blynk.virtualWrite(V5, effectSpeed);
  Blynk.virtualWrite(V6, effectIntensity);
  Blynk.virtualWrite(V7, nightlightActive);
  Blynk.virtualWrite(V8, notifyDirect);
}

BLYNK_WRITE(V0)
{
 bri = param.asInt();//bri
 colorUpdated(9);
}

BLYNK_WRITE(V1)
{
 blHue = param.asInt();//hue
 colorHStoRGB(blHue*10,blSat,(false)? colSec:col);
 colorUpdated(9);
}

BLYNK_WRITE(V2)
{
 blSat = param.asInt();//sat
 colorHStoRGB(blHue*10,blSat,(false)? colSec:col);
 colorUpdated(9);
}

BLYNK_WRITE(V3)
{
  handleSet((param.asInt()>0)?"win&T=1&IN":"win&T=0&IN");//power
}

BLYNK_WRITE(V4)
{
 effectCurrent = param.asInt()-1;//fx
 strip.setMode(effectCurrent);
 colorUpdated(6);
}

BLYNK_WRITE(V5)
{
 effectSpeed = param.asInt();//sx
 strip.setSpeed(effectSpeed);
 colorUpdated(6);
}

BLYNK_WRITE(V6)
{
 effectIntensity = param.asInt();//ix
 strip.setIntensity(effectIntensity);
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

