#include "inttypes.h"
#include "Arduino.h"

#ifndef WLED_CONST_LEDCLOCK_H
#define WLED_CONST_LEDCLOCK_H

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

#define STOPWATCH_MAX_LAP_TIMES 100

#define BEEP_SILENT 255

class LedClockSettingsKeys {
public:
    static const char *root;

    class Brightness {
    public:
        static const char *autom, *min, *max;
    };

    class Display {
    public:
        static const char *separatorMode, *hideZero;
    };

    class Beeps {
    public:
        static const char *mute, *startup, *wifi;

        class Clock {
        public:
            static const char *minute, *hour;
        };

        class Timer {
        public:
            static const char *set, *start, *pause, *resume, *reset, *increase, *hour, *minute, *second, *timeout;
        };

        class Stopwatch {
        public:
            static const char *start, *pause, *resume, *reset, *second, *minute, *hour, *lapTime;
        };
    };
};

class LedClockStateKeys {
public:
    static const char *root, *command, *mode, *beep;

    class Timer {
    public:
        static const char *root, *running, *paused, *left, *value;
    };

    class Stopwatch{
    public:
        static const char *root, *running, *paused, *elapsed, *lapTimes, *lapTimeNr, *lastLapTime;
    };
};

class LedClockSettings {

public:
    enum SeparatorMode {
        ON, OFF, BLINK
    };

    virtual ~LedClockSettings() {}
    bool autoBrightness = true;
    uint8_t minBrightness = 50; // must NOT be lower than 1
    uint8_t maxBrightness = 255;
    SeparatorMode separatorMode = SeparatorMode::BLINK;
    bool hideZero = true;

    bool muteBeeps = false;

    uint8_t beepStartup, beepWiFi;
    uint8_t clockBeepMinute, clockBeepHour;
    uint8_t timerBeepSet, timerBeepStart, timerBeepPause, timerBeepResume, timerBeepReset, timerBeepIncrease, timerBeepHour, timerBeepMinute, timerBeepSecond, timerBeepTimeout;
    uint8_t stopwatchBeepStart, stopwatchBeepPause, stopwatchBeepResume, stopwatchBeepReset, stopwatchBeepSecond, stopwatchBeepMinute, stopwatchBeepHour, stopwatchBeepLapTime;

    virtual void applySettings() = 0;

    static uint8_t constrainBeep(uint8_t beep);
};

const char JSON_ledclock_beeps[] PROGMEM = R"=====([
"1x 330Hz (short)",
"2x 330Hz (short)",
"3x 330Hz (short)",
"1x 440Hz (short)",
"2x 440Hz (short)",
"3x 440Hz (short)",
"1x 880Hz (short)",
"2x 880Hz (short)",
"3x 880Hz (short)",
"1x 330Hz (medium)",
"2x 330Hz (medium)",
"3x 330Hz (medium)",
"1x 440Hz (medium)",
"2x 440Hz (medium)",
"3x 440Hz (medium)",
"1x 880Hz (medium)",
"2x 880Hz (medium)",
"3x 880Hz (medium)",
"1x 330Hz (long)",
"2x 330Hz (long)",
"3x 330Hz (long)",
"1x 440Hz (long)",
"2x 440Hz (long)",
"3x 440Hz (long)",
"1x 880Hz (long)",
"2x 880Hz (long)",
"3x 880Hz (long)",
"440/880Hz (short)",
"880/440Hz (short)",
"440/880Hz (medium)",
"880/440Hz (medium)",
"440/880Hz (long)",
"880/440Hz (long)",
"Turn Up",
"Turn Down",
"Flip Up",
"Flip Down",
"Tadaaa"
])=====";

// forward declarations

class LedBasedDisplay;

LedBasedDisplay* ledClockDisplay();

void ledClockTimeUpdated();

#endif