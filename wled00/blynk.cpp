#include "src/dependencies/blynk/Blynk/BlynkHandlers.h"
#include "wled.h"

/*
 * Remote light control with the free Blynk app
 */

uint16_t blHue = 0;
byte blSat = 255;

void initBlynk(const char *auth) {
#ifndef WLED_DISABLE_BLYNK
  if (!WLED_CONNECTED)
    return;
  blynkEnabled = (auth[0] != 0);
  if (blynkEnabled)
    Blynk.config(auth);
#endif
}

void initLocalBlynk(const char *auth, const char *addr, const int port) {
#ifndef WLED_DISABLE_BLYNK
  if (!WLED_CONNECTED)
    return;
  blynkEnabled = (auth[0] != 0);
  if (!blynkEnabled)
    return;
  IPAddress blynkIP;
  if (blynkIP.fromString(addr)) // see if server is IP or domain
  {
    Blynk.config(auth, blynkIP, port);
  } else {
    Blynk.config(auth, addr, port);
  }
#endif
}

void handleBlynk() {
#ifndef WLED_DISABLE_BLYNK
  if (WLED_CONNECTED && blynkEnabled)
    Blynk.run();
#endif
}

void updateBlynk() {
#ifndef WLED_DISABLE_BLYNK
  if (WLED_CONNECTED) {
    Blynk.virtualWrite(V0, bri);
    // we need a RGB -> HSB convert here
    Blynk.virtualWrite(V3, bri ? 1 : 0);
    Blynk.virtualWrite(V4, effectCurrent);
    Blynk.virtualWrite(V5, effectSpeed);
    Blynk.virtualWrite(V6, effectIntensity);
    Blynk.virtualWrite(V7, nightlightActive);
    Blynk.virtualWrite(V8, notifyDirect);
  }
#endif
}

#ifndef WLED_DISABLE_BLYNK
BLYNK_WRITE(V0) {
  // Brightness (range: 0-255)
  bri = param.asInt();
  colorUpdated(NOTIFIER_CALL_MODE_BLYNK);
}

BLYNK_WRITE(V1) {
  // Hue (range: 0-6553)
  blHue = param.asInt();
  colorHStoRGB(blHue * 10, blSat, (false) ? colSec : col);
  colorUpdated(NOTIFIER_CALL_MODE_BLYNK);
}

BLYNK_WRITE(V2) {
  // Saturation (range: 0-255)
  blSat = param.asInt();
  colorHStoRGB(blHue * 10, blSat, (false) ? colSec : col);
  colorUpdated(NOTIFIER_CALL_MODE_BLYNK);
}

BLYNK_WRITE(V3) {
  // Power (range: 0-1)
  bool on = param.asInt();
  if (on ^ bri) {
    toggleOnOff();
    colorUpdated(NOTIFIER_CALL_MODE_BLYNK);
  }
}

BLYNK_WRITE(V4) {
  // Effect (fx) (range: 1-111)
  // See https://github.com/Aircoookie/WLED/wiki/List-of-effects-and-palettes
  effectCurrent = param.asInt() - 1;
  colorUpdated(NOTIFIER_CALL_MODE_BLYNK);
}

BLYNK_WRITE(V5) {
  // Effect speed (sx) (range: 0-255)
  effectSpeed = param.asInt();
  colorUpdated(NOTIFIER_CALL_MODE_BLYNK);
}

BLYNK_WRITE(V6) {
  // Effect intensity (ix) (range: 0-255)
  effectIntensity = param.asInt();
  colorUpdated(NOTIFIER_CALL_MODE_BLYNK);
}

BLYNK_WRITE(V7) {
  // Nightlight (range: 0-1)
  nightlightActive = param.asInt();
}

BLYNK_WRITE(V8) {
  // Sync (range: 0-1)
  notifyDirect = param.asInt(); // send notifications
}
#endif
