#ifndef WLED_NET_DEBUG_H
#define WLED_NET_DEBUG_H

#include <WString.h>
#include <WiFiUdp.h>

class NetworkDebugPrinter {
private:
    IPAddress debugPrintHostIP;
public:
    void print(const char *s, bool newline = false);
    void print(const __FlashStringHelper* s, bool newline = false);
    void print(String s);
    void print(unsigned int n, bool newline = false);
    void println();
    void println(const char *s);
    void println(const __FlashStringHelper* s);
    void println(String s);
    void println(unsigned int n);
    void printf(const char *fmt, ...);
};

extern NetworkDebugPrinter NetDebug;

#endif