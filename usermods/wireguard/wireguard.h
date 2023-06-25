#pragma once

#include <WireGuard-ESP32.h>

#include "wled.h"

class WireguardUsermod : public Usermod {
   public:
    void setup() { configTzTime(posix_tz.c_str(), ntpServerName); }

    void connected() {
        if (wg.is_initialized()) {
            wg.end();
        }
    }

    void loop() {
        struct tm timeinfo;
        if (!is_enabled || !getLocalTime(&timeinfo, 0)) {
            return;
        }

        if (wg.is_initialized() || !WLED_CONNECTED) {
            return;
        }

        if (preshared_key.length() < 5) {
            wg.begin(local_ip, private_key.c_str(), endpoint_address.c_str(), public_key.c_str(), endpoint_port, NULL);
        } else {
            wg.begin(local_ip, private_key.c_str(), endpoint_address.c_str(), public_key.c_str(), endpoint_port, preshared_key.c_str());
        }
    }

    void addToJsonInfo(JsonObject& root) {
        JsonObject user = root["u"];
        if (user.isNull()) user = root.createNestedObject("u");

        JsonArray infoArr = user.createNestedArray(F("WireGuard VPN"));
        String uiDomString;
        
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo, 0)) {
            uiDomString = "Time out of sync!";
        } else {
            if (wg.is_initialized()) {
                uiDomString = "netif up!";
            } else {
                uiDomString = "netif down :(";
            }
        }
        if(is_enabled) infoArr.add(uiDomString);
    }

    void addToConfig(JsonObject& root) {
        JsonObject top = root.createNestedObject(F("WireGuard VPN"));
        top[F("Endpoint Address")] = endpoint_address;
        top[F("Endpoint Port")] = endpoint_port;
        top[F("Local IP")] = local_ip_buf;
        top[F("PSK")] = preshared_key;
        top[F("Private Key")] = private_key;
        top[F("Public Key")] = public_key;
        top[F("POSIX Timezone")] = posix_tz;
    }

    bool readFromConfig(JsonObject& root) {
        JsonObject top = root[F("WireGuard VPN")];

        bool configComplete = !top.isNull();

        configComplete &= getJsonValue(top[F("Endpoint Address")], endpoint_address);
        configComplete &= getJsonValue(top[F("Endpoint Port")], endpoint_port);
        configComplete &= getJsonValue(top[F("Local IP")], local_ip_buf);
        configComplete &= getJsonValue(top[F("Private Key")], private_key);
        configComplete &= getJsonValue(top[F("Public Key")], public_key);
        configComplete &= getJsonValue(top[F("POSIX Timezone")], posix_tz);
        getJsonValue(top[F("PSK")], preshared_key);

        local_ip.fromString(local_ip_buf);

        is_enabled = configComplete;

        if (wg.is_initialized()) {
            wg.end();
        }

        return configComplete;
    }

    uint16_t getId() { return USERMOD_ID_WIREGUARD; }

   private:
    WireGuard wg;
    String preshared_key;  // [Interface] PrivateKey
    String private_key;    // [Interface] PrivateKey
    String local_ip_buf;
    IPAddress local_ip;
    String public_key;        // [Peer] PublicKey
    String endpoint_address;  // [Peer] Endpoint
    String posix_tz;
    int endpoint_port = 0;  // [Peer] Endpoint
    bool is_enabled = false;
};