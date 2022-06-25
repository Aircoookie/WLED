#pragma once
#include "wled.h"

/*
 * Usermod for analog clock
 */
class AnalogClockUsermod : public Usermod {
private:
    struct Segment {
        // config
        int16_t firstLed  = 0;
        int16_t lastLed   = 59;
        int16_t centerLed = 0;

        // runtime
        int16_t size;

        Segment() {
            update();
        }

        void validateAndUpdate() {
            if (firstLed < 0 || firstLed >= strip.getLengthTotal() ||
                    lastLed < firstLed || lastLed >= strip.getLengthTotal()) {
                *this = {};
                return;
            }
            if (centerLed < firstLed || centerLed > lastLed) {
                centerLed = firstLed;
            }
            update();
        }

        void update() {
            size = lastLed - firstLed + 1;
        }
    };

    // configuration (available in API and stored in flash)
    bool     enabled          = false;
    Segment  mainSegment;
    uint32_t hourColor        = 0x0000FF;
    uint32_t minuteColor      = 0x00FF00;
    bool     secondsEnabled   = true;
    Segment  secondsSegment;
    uint32_t secondColor      = 0xFF0000;
    bool     blendColors      = true;
    int16_t  secondsTrail     = 0;

    void validateAndUpdate() {
        mainSegment.validateAndUpdate();
        secondsSegment.validateAndUpdate();
    }

    uint16_t adjustToSegment(double progress, Segment const& segment) {
        int16_t led = segment.centerLed + progress * segment.size;
        return led > segment.lastLed
                ? segment.firstLed + led - segment.lastLed - 1
                : led;
    }

    void setPixelColor(uint16_t n, uint32_t c) {
        if (!blendColors) {
            strip.setPixelColor(n, c);
        } else {
            uint32_t oldC = strip.getPixelColor(n);
            strip.setPixelColor(n, qadd8(R(oldC), R(c)), qadd8(G(oldC), G(c)), qadd8(B(oldC), B(c)), qadd8(W(oldC), W(c)));
        }
    }

    String colorToHexString(uint32_t c) {
        char buffer[9];
        sprintf(buffer, "%06X", c);
        return buffer;
    }

    bool hexStringToColor(String const& s, uint32_t& c, uint32_t def) {
        errno = 0;
        char* ep;
        unsigned long long r = strtoull(s.c_str(), &ep, 16);
        if (*ep == 0 && errno != ERANGE) {
            c = r;
            return true;
        } else {
            c = def;
            return false;
        }
    }

public:
    AnalogClockUsermod() {
    }

    void handleOverlayDraw() override {
        if (!enabled) {
            return;
        }

        double secondP = second(localTime) / 60.0;
        double minuteP = minute(localTime) / 60.0;
        double hourP = (hour(localTime) % 12) / 12.0 + minuteP / 12.0;

        if (secondsEnabled) {
            uint16_t secondLed = adjustToSegment(secondP, secondsSegment);
            setPixelColor(secondLed, secondColor);

            for (uint16_t i = 1; i < secondsTrail + 1; ++i) {
                uint16_t trailLed = i <= secondLed ? secondLed - i : secondsSegment.lastLed - i + 1;
                uint8_t trailBright = 255 / (secondsTrail + 1) * (secondsTrail - i + 1);
                setPixelColor(trailLed, trailBright << 16);
            }
        }

        setPixelColor(adjustToSegment(minuteP, mainSegment), minuteColor);
        setPixelColor(adjustToSegment(hourP, mainSegment), hourColor);
    }

    void addToConfig(JsonObject& root) override {
        validateAndUpdate();

        JsonObject top = root.createNestedObject("Analog Clock");
        top["Overlay Enabled"]               = enabled;
        top["First LED (Main Ring)"]         = mainSegment.firstLed;
        top["Last LED (Main Ring)"]          = mainSegment.lastLed;
        top["Center/12h LED (Main Ring)"]    = mainSegment.centerLed;
        top["Hour Color (RRGGBB)"]           = colorToHexString(hourColor);
        top["Minute Color (RRGGBB)"]         = colorToHexString(minuteColor);
        top["Show Seconds"]                  = secondsEnabled;
        top["First LED (Seconds Ring)"]      = secondsSegment.firstLed;
        top["Last LED (Seconds Ring)"]       = secondsSegment.lastLed;
        top["Center/12h LED (Seconds Ring)"] = secondsSegment.centerLed;
        top["Second Color (RRGGBB)"]         = colorToHexString(secondColor);
        top["Seconds Trail (0-99)"]          = secondsTrail;
        top["Blend Colors"]                  = blendColors;
    }

    bool readFromConfig(JsonObject& root) override {
        JsonObject top = root["Analog Clock"];

        bool configComplete = !top.isNull();

        String color;
        configComplete &= getJsonValue(top["Overlay Enabled"], enabled, false);
        configComplete &= getJsonValue(top["First LED (Main Ring)"], mainSegment.firstLed, 0);
        configComplete &= getJsonValue(top["Last LED (Main Ring)"], mainSegment.lastLed, 59);
        configComplete &= getJsonValue(top["Center/12h LED (Main Ring)"], mainSegment.centerLed, 0);
        configComplete &= getJsonValue(top["Hour Color (RRGGBB)"], color, "0000FF") && hexStringToColor(color, hourColor, 0x0000FF);
        configComplete &= getJsonValue(top["Minute Color (RRGGBB)"], color, "00FF00") && hexStringToColor(color, minuteColor, 0x00FF00);
        configComplete &= getJsonValue(top["Show Seconds"], secondsEnabled, true);
        configComplete &= getJsonValue(top["First LED (Seconds Ring)"], secondsSegment.firstLed, 0);
        configComplete &= getJsonValue(top["Last LED (Seconds Ring)"], secondsSegment.lastLed, 59);
        configComplete &= getJsonValue(top["Center/12h LED (Seconds Ring)"], secondsSegment.centerLed, 0);
        configComplete &= getJsonValue(top["Second Color (RRGGBB)"], color, "FF0000") && hexStringToColor(color, secondColor, 0xFF0000);
        configComplete &= getJsonValue(top["Seconds Trail (0-99)"], secondsTrail, 0);
        configComplete &= getJsonValue(top["Blend Colors"], blendColors, true);

        validateAndUpdate();

        return configComplete;
    }

    uint16_t getId() override {
        return USERMOD_ID_ANALOG_CLOCK;
    }
};