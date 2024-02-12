#pragma once
#include "wled.h"

#define PSBUFSIZ 20
#ifndef BUSSTATUS_PINS
#define BUSSTATUS_PINS { -1, 1, 2}
#endif
#define BUSSTATUS_SIZ (sizeof((int8_t[])BUSSTATUS_PINS)/sizeof(int8_t))
#ifndef FX_MODE_BUSSTATUS
#define FX_MODE_BUSSTATUS 255  // Temporary Effect ID
#endif

class BusStatusUsermod : public Usermod {

  private:
    char buf[PSBUFSIZ];

    // strings to reduce flash memory usage (used more than twice)
    static const char _FX_MODE_BUSSTATUS[];
    static const int8_t status_gpios[BUSSTATUS_SIZ];

    static uint8_t get_bustype(uint8_t gpio) {
        Bus* bus;
        uint8_t numPins;
        uint8_t pinArr[8];
        for (uint8_t busNr = 0; busNr < busses.getNumBusses(); busNr++) {
            bus = busses.getBus(busNr);
            if (bus) {
                numPins = bus->getPins(pinArr);
                for (uint8_t i = 0; i < numPins; i++) {
                    if (pinArr[i] == gpio) {
                        return bus->getType();
                    }
                }
            }
        }
        return TYPE_NONE;
    }

    // errorFlag -> Red
    static uint32_t device_status_colour() {
        // assumes max error flag is 36
        uint8_t red_val = errorFlag * 7;
        if (WLED_CONNECTED) return RGBW32(red_val,255,0,0); // Green
        else if (WLED_MQTT_CONNECTED) return RGBW32(red_val,128,128,0); // Cyan
        else if (apActive) return RGBW32(red_val,0,255,0); // Blue
        else return RGBW32(red_val,0,0,0); // Red
    }

    static const char* device_status_string() {
        if (WLED_CONNECTED) return "Wifi";
        else if (WLED_MQTT_CONNECTED) return "MQTT";
        else if (apActive) return "HotSpot";
        else return "Unknown";
    }

    char* device_error_string() {
        snprintf(buf, PSBUFSIZ, "0x%X", errorFlag);
        return buf;
    }

    // configurable status colours for the first 3 only
    static uint32_t bus_status_colour(int8_t gpio) {
        uint8_t busType = get_bustype(gpio);
        if (IS_DMX(busType)) return SEGCOLOR(0); // Blue
        else if (IS_DIGITAL(busType)) return SEGCOLOR(1); // Green
        else if (IS_PWM(busType)) return SEGCOLOR(2); // Magenta
        else return 0UL; // Black
    }

    const char* bus_status_string(int8_t gpio) {
        // bus types in const.h
        uint8_t busType = get_bustype(gpio);
        if (busType == TYPE_NONE) return "Unconfigured";
        else if (IS_DMX(busType)) return "DMX";
        else if (IS_DIGITAL(busType)) return "Digital LED";
        else if (IS_PWM(busType)) return "PWM LED";
        else snprintf(buf, PSBUFSIZ, "Type %d", busType);
        return buf;
    }

    // modification of static/blink
    static uint16_t mode_busstatus(void) {
        uint32_t status_colours[BUSSTATUS_SIZ];
        for (uint16_t i = 0; i < BUSSTATUS_SIZ; i++) {
            if (status_gpios[i] < 0) {
                status_colours[i] = device_status_colour();
            } else {
                status_colours[i] = bus_status_colour(status_gpios[i]);
            }
        }
        uint16_t i;
        for (uint16_t s = 0; s < SEGLEN; s++) {
            i = s % BUSSTATUS_SIZ;
            SEGMENT.setPixelColor(s, status_colours[i]);
        }
        return strip.isOffRefreshRequired() ? FRAMETIME : 350;
    }

  public:
    // gets called once at boot. Do all initialization that doesn't depend on network here
    // parameters are already read by readFromConfig, busses & segments have been created by beginStrip()
    void setup() {
        strip.addEffect(FX_MODE_BUSSTATUS, &mode_busstatus, _FX_MODE_BUSSTATUS);
    }

    // gets called every time WiFi is (re-)connected. Initialize own network
    // interfaces here
    void connected() {}

    // main loop
    void loop() {}

    /*
     * addToJsonInfo() adds info to the main UI Info section
     */
    void addToJsonInfo(JsonObject& root) {
        JsonObject user = root["u"];
        if (user.isNull()) user = root.createNestedObject("u");

        for (uint8_t i = 0; i < BUSSTATUS_SIZ; i++) {
            int8_t gpio = status_gpios[i];
            if (gpio < 0) {
                user.createNestedArray("Device Status").add(device_status_string());
                user.createNestedArray("Error Status").add(device_error_string());
            } else {
                snprintf_P(buf, PSBUFSIZ, "GPIO %d Status", gpio);
                JsonArray arr = user.createNestedArray(buf);
                arr.add(bus_status_string(gpio));
            }
        }
    }

    /*
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void addToJsonState(JsonObject& root) {}

    /*
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void readFromJsonState(JsonObject& root) {}

    /*
     * addToConfig() can be used to add custom persistent settings to the cfg.json file in the "um" (usermod) object.
     * It will be called by WLED when settings are actually saved (for example, LED settings are saved)
     * Use serializeconfig sparingly and always in the loop, never in network callbacks!
     *
     * addToConfig() will also not yet add your setting to one of the settings pages automatically.
     * To make that work you still have to add the setting to the HTML, xml.cpp and set.cpp manually.
     */
    void addToConfig(JsonObject& root) {}

    /*
     * readFromConfig() can be used to read back the custom settings you added with addToConfig().
     * Called before beginStrip() on startup so busses and segments not created yet.
     * Also called on settings page save
     *
     * The function should return true if configuration was successfully loaded or false if there was no configuration.
     */
    bool readFromConfig(JsonObject& root) {
        return true;
    }

    /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
    uint16_t getId() {
      return USERMOD_ID_BUSSTATUS;
    }
};

const int8_t BusStatusUsermod::status_gpios[BUSSTATUS_SIZ] = BUSSTATUS_PINS;
// config strings to reduce flash memory usage (used more than twice)
// <Effect parameters>(None);<Colors>(3 Colours);<Palette>(None);<Flags>(1Dim);<Defaults>
const char BusStatusUsermod::_FX_MODE_BUSSTATUS[] PROGMEM = "Bus Status@;1,2,3;;1;";
