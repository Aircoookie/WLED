/**
 * @file       BlynkWiFiCommon.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief
 *
 */

#ifndef BlynkWiFiCommon_h
#define BlynkWiFiCommon_h

#ifndef BLYNK_INFO_CONNECTION
#define BLYNK_INFO_CONNECTION "WiFi"
#endif

#include "BlynkApiArduino.h"
#include "BlynkProtocol.h"
#include "BlynkArduinoClient.h"

class BlynkWifiCommon
    : public BlynkProtocol<BlynkArduinoClient>
{
    typedef BlynkProtocol<BlynkArduinoClient> Base;
public:
    BlynkWifiCommon(BlynkArduinoClient& transp)
        : Base(transp)
    {}

    void connectWiFi(const char* ssid, const char* pass)
    {
        int status = WL_IDLE_STATUS;
        // check for the presence of the shield:
        if (WiFi.status() == WL_NO_SHIELD) {
            BLYNK_FATAL("WiFi shield not present");
        }

#ifdef BLYNK_DEBUG
        BLYNK_LOG2(BLYNK_F("WiFi firmware: "), WiFi.firmwareVersion());
#endif

        // attempt to connect to Wifi network:
        while (true) {
            BLYNK_LOG2(BLYNK_F("Connecting to "), ssid);
            if (pass && strlen(pass)) {
                status = WiFi.begin((char*)ssid, (char*)pass);
            } else {
                status = WiFi.begin((char*)ssid);
            }
            if (status == WL_CONNECTED) {
                break;
            } else {
                BlynkDelay(5000);
            }
        }

        IPAddress myip = WiFi.localIP();
        BLYNK_LOG_IP("IP: ", myip);
    }

    void config(const char* auth,
                const char* domain = BLYNK_DEFAULT_DOMAIN,
                uint16_t    port   = BLYNK_DEFAULT_PORT)
    {
        Base::begin(auth);
        this->conn.begin(domain, port);
    }

    void config(const char* auth,
                IPAddress   ip,
                uint16_t    port = BLYNK_DEFAULT_PORT)
    {
        Base::begin(auth);
        this->conn.begin(ip, port);
    }

    void begin(const char* auth,
               const char* ssid,
               const char* pass,
               const char* domain = BLYNK_DEFAULT_DOMAIN,
               uint16_t    port   = BLYNK_DEFAULT_PORT)
    {
        connectWiFi(ssid, pass);
        config(auth, domain, port);
        while(this->connect() != true) {}
    }

    void begin(const char* auth,
               const char* ssid,
               const char* pass,
               IPAddress   ip,
               uint16_t    port   = BLYNK_DEFAULT_PORT)
    {
        connectWiFi(ssid, pass);
        config(auth, ip, port);
        while(this->connect() != true) {}
    }

};

#endif
