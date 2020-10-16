#include "Network.h"

IPAddress NetworkClass::localIP()
{
#ifdef ARDUINO_ARCH_ESP32
    if (ETH.localIP()[0] != 0) {
        return ETH.localIP();
    }
#endif
    if (WiFi.localIP()[0] != 0) {
        return WiFi.localIP();
    }
    return INADDR_NONE;
}

IPAddress NetworkClass::subnetMask()
{
#ifdef ARDUINO_ARCH_ESP32
    if (ETH.localIP()[0] != 0) {
        return ETH.subnetMask();
    }
#endif
    if (WiFi.localIP()[0] != 0) {
        return WiFi.subnetMask();
    }
    return IPAddress(255, 255, 255, 0);
}

IPAddress NetworkClass::gatewayIP()
{
#ifdef ARDUINO_ARCH_ESP32
    if (ETH.localIP()[0] != 0) {
        return ETH.gatewayIP();
    }
#endif
    if (WiFi.localIP()[0] != 0) {
        return WiFi.gatewayIP();
    }
    return INADDR_NONE;
}

bool NetworkClass::isConnected()
{
#ifdef ARDUINO_ARCH_ESP32
    return WiFi.localIP()[0] != 0 || ETH.localIP()[0] != 0;
#else
    return WiFi.localIP()[0] != 0;
#endif
}

NetworkClass Network;