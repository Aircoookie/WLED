#include "wled.h"

#ifdef WLED_DEBUG_HOST

NetworkDebugPrinter NetDebug;

void NetworkDebugPrinter::printchar(char s) {
  debugUdp.write(s);
}

void NetworkDebugPrinter::print(const char *s) {
  if (!WLED_CONNECTED || s == nullptr) {
    return;
  }
  IPAddress debugPrintHostIP;
  if (!debugPrintHostIP.fromString(netDebugPrintHost)) {
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
  debugUdp.beginPacket(debugPrintHostIP, netDebugPrintPort);
  //for (size_t i=0; i<strlen(s); i++) printchar(s[i]);
  debugUdp.write(reinterpret_cast<const uint8_t *>(s), strlen(s));
  debugUdp.endPacket();
}

void NetworkDebugPrinter::print(const __FlashStringHelper* s) {
  print(reinterpret_cast<const char *>(s));
}

void NetworkDebugPrinter::print(String s) {
  print(s.c_str());
}

void NetworkDebugPrinter::print(unsigned int n) {
  char s[10];
  snprintf_P(s, sizeof(s), PSTR("%d"), n);
  s[9] = '\0';
  print(s);
}

void NetworkDebugPrinter::println() {
  print("\n");
}

void NetworkDebugPrinter::println(const char *s) {
  print(s);
  print("\n");
}

void NetworkDebugPrinter::println(const __FlashStringHelper* s) {
  print(s);
  print("\n");
}

void NetworkDebugPrinter::println(String s) {
  print(s);
  print("\n");
}

void NetworkDebugPrinter::println(unsigned int n) {
  print(n);
  print("\n");
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
