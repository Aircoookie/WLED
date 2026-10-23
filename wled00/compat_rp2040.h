#pragma once
// Small compatibility helpers for RP2040/RP2350 (arduino-pico), filling gaps where its
// WiFi library lacks ESP32/ESP8266 convenience methods WLED's code relies on.
#ifdef ARDUINO_ARCH_RP2040

#include <Arduino.h>

// Equivalent of ESP32/ESP8266's WiFi.BSSIDstr()/WiFi.BSSIDstr(i): arduino-pico's WiFi.BSSID()
// only returns raw bytes via an out-buffer, with no formatted-string convenience method.
// Pass no argument for the currently-connected AP, or a scan result index.
String wledBSSIDstr(int16_t networkItem = -1);

// Minimal stand-in for the ESP32/ESP8266 "ESP" global object, covering only the methods
// WLED calls without an existing platform guard (most ESP.* call sites are already wrapped
// in #ifdef ARDUINO_ARCH_ESP32/ESP8266 elsewhere and never reach this platform at all).
// Extend this as new unguarded call sites surface, rather than editing each call site.
class WledESPCompat {
public:
  void     restart();
  uint32_t getFreeHeap();
  uint32_t getFreeSketchSpace();
};
extern WledESPCompat ESP;

#endif
