#include "inttypes.h"
#include "Arduino.h"

#ifndef WLED_CONST_LEDCLOCK_H
#define WLED_CONST_LEDCLOCK_H

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

// How to configure the display?
//
// Think of your clock display as 2D matrix of pixels rather than a 7 segment display.
//
// Some pixels of this 2D matrix are missing, others are present. To get the layout
// where the pixels are missing or present, first decide on how many LEDs you want per segment.
//
// For example,  if you have only one LED per segment, the layout is the following:
//
//  -#--#---#--#-
//  #-##-###-##-#
//  -#--#---#--#-
//  #-##-###-##-#
//  -#--#---#--#-
//
// Where the symbol '-' represents a missing pixel and symbol '#' represents a pixel that presents.
// Note the two 'separator' pixels in the middle 'horizontal' column.
//
// If you have 2 LEDs per segment,  the layout is the following:
//
// -##--##---##--##-
// #--##--#-#--##--#
// #--##--###--##--#
// -##--##---##--##-
// #--##--###--##--#
// #--##--#-#--##--#
// -##--##---##--##-
//
// Similarly,  on case of 3 LEDs per segment:
//
// -###--###---###--###-
// #---##---#-#---##---#
// #---##---###---##---#
// #---##---#-#---##---#
// -###--###---###--###-
// #---##---#-#---##---#
// #---##---###---##---#
// #---##---#-#---##---#
// -###--###---###--###-
//
// Next you need to replace all the '-' symbols with -1 and all the '#' symbols with LED indices. Which index
// you write in place of a particular '#' symbol depends on the physical layout of your LED strip.
//
// In the example below there are 2 LEDs per segment, and the LED strip begins with the bottommost segment of
// the first number then continues with the leftmost segments from the bottom to the top. I added extra spaces
// between the numbers and the separator column just for better clarity.

#define LC_LEDS_PER_SEGM 2 // LEDs per segment

#define LC_LEDMAP \
-1,  6,  7, -1,    -1, 20, 21, -1,    -1,    -1, 36, 37, -1,    -1, 50, 51, -1, \
 5, -1, -1,  8,    19, -1, -1, 22,    -1,    35, -1, -1, 38,    49, -1, -1, 52, \
 4, -1, -1,  9,    18, -1, -1, 23,    29,    34, -1, -1, 39,    48, -1, -1, 53, \
-1, 11, 10, -1,    -1, 25, 24, -1,    -1,    -1, 41, 40, -1,    -1, 55, 54, -1, \
 3, -1, -1, 12,    17, -1, -1, 26,    28,    33, -1, -1, 42,    47, -1, -1, 56, \
 2, -1, -1, 13,    16, -1, -1, 27,    -1,    32, -1, -1, 43,    46, -1, -1, 57, \
-1,  1,  0, -1,    -1, 15, 14, -1,    -1,    -1, 31, 30, -1,    -1, 45, 44, -1  \

// Next configure the separator LEDs by first defining how many of them you have:
#define LC_SEP_LEDS 2

// Then write their row indices separated by commas below (notice the separator LEDs #28 and #29 having row indices 2 and 4 in the above LED map):
#define LC_SEP_LED_ROWS 2, 4

// Finally, don't forget to change the total number of LEDs (DEFAULT_LED_COUNT) in `platformio_override.ini`,
// set it to the total size of the LED matrix you have. This number is calculated below and a compiler warning
// is emitted if it does not match with DEFAULT_LED_COUNT.

#define LC_COLS     (((LC_LEDS_PER_SEGM + 2) * 4) + 1)
#define LC_ROWS      ((LC_LEDS_PER_SEGM * 2) + 3)
#define LC_TOTAL_LEDS (LC_COLS * LC_ROWS) // DEFAULT_LED_COUNT should be set to this value

#if !defined(DEFAULT_LED_COUNT) || DEFAULT_LED_COUNT != LC_TOTAL_LEDS
#define LC_XSTR(x) LC_STR(x)
#define LC_STR(x) #x
#pragma message "Macro DEFAULT_LED_COUNT is not defined or is not equal to the calculated number of total LEDs: " LC_XSTR(LC_TOTAL_LEDS)
#endif

// Your display still does not work properly? Verify all the steps above, and if everything looks right, follow these steps:
//   1. navigate your browser http://wled-ip/edit
//   2. right click on /ledmap.json
//   3. choose 'Delete'
//   4. reboot WLED
// The file ledmap.json should now be regenerated with the correct settings and your display should now work properly.


#define LC_PHYSICAL_LEDS (LC_SEP_LEDS + 4 * 7 * LC_LEDS_PER_SEGM)

#define LC_R(c) (byte((c) >> 16))
#define LC_G(c) (byte((c) >> 8))
#define LC_B(c) (byte(c))
#define LC_W(c) (byte((c) >> 24))

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
    static const char *root, *command, *mode, *beep, *blendingMode, *canvasColor;

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

// custom effects

#define FX_MODE_LC_2SOFIX     187
#define FX_MODE_LC_VORTEX     188
#define FX_MODE_LC_CONCENTRIC 189

// forward declarations

void ledClockTimeUpdated();

#endif