#include "Network.h"

IPAddress NetworkClass::localIP()
{
#ifndef ESP8266
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
#ifndef ESP8266
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
#ifndef ESP8266
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
#ifdef ESP8266
    return WiFi.localIP()[0] != 0;
#else // ESP32
    return WiFi.localIP()[0] != 0 || ETH.localIP()[0] != 0;
#endif
}

NetworkClass Network;