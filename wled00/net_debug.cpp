#include "wled.h"

#ifdef WLED_DEBUG_NET

NetworkDebugPrinter NetDebug;

void NetworkDebugPrinter::print(const char *s) {
#ifdef WLED_DEBUG
    Serial.print(s);
#endif
    if (!WLED_CONNECTED || !netDebugPrintEnabled) {
        return;
    }
    IPAddress debugPrintHostIP;
    if (!debugPrintHostIP.fromString(netDebugPrintHost)) {
        #ifdef ESP8266
        WiFi.hostByName(netDebugPrintHost, debugPrintHostIP, 750);
        #else
        WiFi.hostByName(netDebugPrintHost, debugPrintHostIP);
        #endif
    }
    debugUdp.beginPacket(debugPrintHostIP, netDebugPrintPort);
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
    snprintf(s, sizeof(s), "%d", n);
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
