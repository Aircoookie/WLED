#include "wled.h"

#if !defined(WLED_DISABLE_HOMEKIT) && defined(ARDUINO_ARCH_ESP32)

void updateOn(bool on);
void updateBrightness(int brightness);
void updateColor(double hue, double sat);

void homekitInit() {
    if (WLED_CONNECTED) {
        if (homekit_device == nullptr) {
            EHK_DEBUGLN("Starting HomeKit Server.");
            uint16_t hsv[3] = {0};
            colorFromRGB(col[0], col[1], col[2], hsv);
            uint16_t hue = hsv[0];
            uint8_t sat = map(hsv[1], 0, 255, 0, 100);
            uint8_t brightness = map(bri, 0, UINT8_MAX, 0, 100);
            homekit_device = new HKDevice(brightness > 0, hue, sat, brightness, updateOn, updateBrightness, updateColor);
            homekit_server.add_device(homekit_device);
            homekit_server.begin();
        } else {
            // Any changes to mdns host name, or wifi change will need to reconfigure mdns
            homekit_server.reconnect();
        }
    } else {
        EHK_DEBUGLN("WLED not connected to wifi, will not start Homekit.");
    }
}

void handleHomeKit() {
    if (!WLED_CONNECTED) return;
    homekit_server.poll();
}

// Callbacks

void updateOn(bool on) {
    if (on) {
        if (bri == 0) {
            bri = briLast;
            stateUpdated(CALL_MODE_HOMEKIT);
        }
    } else {
        if (bri > 0) {
            briLast = bri;
            bri = 0;
            stateUpdated(CALL_MODE_HOMEKIT);
        } 
    }
}

void updateBrightness(int brightness) {
        bri = map(brightness, 0, 100, 0, UINT8_MAX);
        stateUpdated(CALL_MODE_HOMEKIT);
}

void updateColor(double hue, double sat) {
    byte rgb[4] {0};
    uint16_t hueMapped = map(hue, 0, 360, 0, UINT16_MAX);
    uint8_t satMapped = map(sat, 0, 100, 0, UINT8_MAX);

    colorHStoRGB(hueMapped, satMapped, rgb);

    uint32_t color = ((rgb[0] << 16) | (rgb[1] << 8) | (rgb[2]));

    strip.setColor(0, color);
    stateUpdated(CALL_MODE_HOMEKIT);
}

#else
 void homekitInit(){}
 void handleHomeKit(){}
#endif