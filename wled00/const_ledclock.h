#include "inttypes.h"

#ifndef WLED_CONST_LEDCLOCK_H
#define WLED_CONST_LEDCLOCK_H

#define SERVER_DESC "LED Clock"
#define AP_NAME "LED Clock AP"
#define MDNS_PREFIX "ledclock"
#define MQTT_DEVICE_TOPIC "ledclock"
#define MQTT_CLIENT_ID "LEDCLOCK"

#define BUZZER_PIN 12
#define ADC_MAX_VALUE 4095
#define ADC_MAX_VOLTAGE 3.3
#define BRIGHTNESS_SAMPLES 1024
#define BRIGHTNESS_THRESHOLD 5
#define BRIGHTNESS_PIN 34

extern const char * ledClockSettingsKey;
extern const char * ledClockSettingsKeyAutoBrightness;
extern const char * ledClockSettingsKeyMinBrightness;
extern const char * ledClockSettingsKeyMaxBrightness;
extern const char * ledClockSettingsKeySeparatorMode;

enum SeparatorMode {
    ON, OFF, BLINK
};

class LedClockSettings {

public:
    virtual ~LedClockSettings() {}
    bool autoBrightness = true;
    uint8_t minBrightness = 50; // must NOT be lower than 1
    uint8_t maxBrightness = 255;
    SeparatorMode separatorMode = SeparatorMode::BLINK;
};

#endif