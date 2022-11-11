#include "wled.h"

#ifdef WLED_DEBUG_HOST

size_t NetworkDebugPrinter::write(uint8_t c) {
  begin();
  if (!udpConnected) return 0;
  return debugUdp.write(c);
}

size_t NetworkDebugPrinter::write(const uint8_t *buf, size_t size) {
  if (buf == nullptr) return 0;
  begin();
  if (!udpConnected) return 0;
  return debugUdp.write(buf, size);
}

void NetworkDebugPrinter::begin() {
  if (udpConnected) return;
  if (!WLED_CONNECTED) {
    debugUdp.stop();
    debugPrintHostIP = INADDR_NONE;
    udpConnected = false;
    return;
  }

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

  udpConnected = debugUdp.beginPacket(debugPrintHostIP, netDebugPrintPort);
}

void NetworkDebugPrinter::flush() {
  if (udpConnected) {
    if (!debugUdp.endPacket()) udpConnected = false;  // we were not able to send packet
  }
}

NetworkDebugPrinter NetDebug;

#endif
