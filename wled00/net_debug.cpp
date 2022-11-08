#include "wled.h"

#ifdef WLED_DEBUG_HOST

NetworkDebugPrinter NetDebug;

void NetworkDebugPrinter::print(const char *s, bool newline) {
  if (!WLED_CONNECTED || !udpConnected || s == nullptr) return;

  if (!debugPrintHostIP && !debugPrintHostIP.fromString(netDebugPrintHost)) {
    #ifdef ESP8266
      WiFi.hostByName(netDebugPrintHost, debugPrintHostIP, 750);
    #else
      #ifdef WLED_USE_ETHERNET
        ETH.hostByName(netDebugPrintHost, debugPrintHostIP);
      #else
        WiFi.hostByName(netDebugPrintHost, debugPrintHostIP);
      #endif
    #endif
  }

  WiFiUDP debugUdp;
  debugUdp.beginPacket(debugPrintHostIP, netDebugPrintPort);
  debugUdp.write(reinterpret_cast<const uint8_t *>(s), strlen(s));
  if (newline) debugUdp.write('\n');
  debugUdp.endPacket();
}

void NetworkDebugPrinter::print(const __FlashStringHelper* s, bool newline) {
  char buf[512];
  strncpy_P(buf, (PGM_P)s, 512);
  print(buf, newline);
}

void NetworkDebugPrinter::print(String s) {
  print(s.c_str());
}

void NetworkDebugPrinter::print(unsigned int n, bool newline) {
  char s[10];
  snprintf_P(s, sizeof(s), PSTR("%d"), n);
  s[9] = '\0';
  print(s, newline);
}

void NetworkDebugPrinter::println() {
  print("", true);
}

void NetworkDebugPrinter::println(const char *s) {
  print(s, true);
}

void NetworkDebugPrinter::println(const __FlashStringHelper* s) {
  print(s, true);
}

void NetworkDebugPrinter::println(String s) {
  print(s.c_str(), true);
}

void NetworkDebugPrinter::println(unsigned int n) {
  print(n, true);
}

void NetworkDebugPrinter::printf(const char *fmt...) {
  va_list args;
  va_start(args, fmt);
  char s[1024];
  vsnprintf(s, sizeof(s), fmt, args);
  va_end(args);
  print(s);
}

#endif
