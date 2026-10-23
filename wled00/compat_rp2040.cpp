#include "compat_rp2040.h"
#ifdef ARDUINO_ARCH_RP2040
#include <WiFi.h>

#ifndef MACSTR
  #define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#endif
#ifndef MAC2STR
  #define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#endif

String wledBSSIDstr(int16_t networkItem) {
  uint8_t bssid[6] = {0,0,0,0,0,0};
  if (networkItem < 0) WiFi.BSSID(bssid);
  else                  WiFi.BSSID((uint8_t)networkItem, bssid);
  char buf[18];
  snprintf(buf, sizeof(buf), MACSTR, MAC2STR(bssid));
  return String(buf);
}

void     WledESPCompat::restart()      { rp2040.restart(); }
uint32_t WledESPCompat::getFreeHeap()  { return rp2040.getFreeHeap(); }

// ESP32/ESP8266's ESP.getFreeSketchSpace(): remaining room for a new firmware image.
// arduino-pico has no equivalent call, but the linker already gives us both ends of
// that region as symbols - __flash_binary_end (where this build's own image actually
// ends) and _FS_start (where the LittleFS partition begins, see platformio.ini's
// board_build.filesystem_size) - so this is a real measurement, not a guess.
extern "C" uint8_t _FS_start;
extern "C" uint8_t __flash_binary_end;
uint32_t WledESPCompat::getFreeSketchSpace() {
  return (uint32_t)&_FS_start - (uint32_t)&__flash_binary_end;
}

WledESPCompat ESP;

#endif
