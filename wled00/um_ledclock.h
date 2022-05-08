#pragma once

#include "wled.h"
#include "7segmdisp.h"
#include "beeper.h"

const char

    *LedClockSettingsKeys::root = "ledclock",

    *LedClockSettingsKeys::Brightness::autom = "autb",
    *LedClockSettingsKeys::Brightness::min = "minb",

    *LedClockSettingsKeys::Brightness::max = "maxb",
    *LedClockSettingsKeys::Display::separatorMode = "sepm",
    *LedClockSettingsKeys::Display::hideZero = "hidzer",

    *LedClockSettingsKeys::Beeps::mute = "bmute",

    *LedClockSettingsKeys::Beeps::startup = "bstartup",
    *LedClockSettingsKeys::Beeps::wifi = "bwifi",

    *LedClockSettingsKeys::Beeps::Clock::minute = "clbm",
    *LedClockSettingsKeys::Beeps::Clock::hour = "clbh",

    *LedClockSettingsKeys::Beeps::Timer::set = "tmbset",
    *LedClockSettingsKeys::Beeps::Timer::start = "tmbstart",
    *LedClockSettingsKeys::Beeps::Timer::pause = "tmbpause",
    *LedClockSettingsKeys::Beeps::Timer::resume = "tmbresume",
    *LedClockSettingsKeys::Beeps::Timer::reset = "tmbreset",
    *LedClockSettingsKeys::Beeps::Timer::increase = "tmbtinc",
    *LedClockSettingsKeys::Beeps::Timer::hour = "tmbh",
    *LedClockSettingsKeys::Beeps::Timer::minute = "tmbm",
    *LedClockSettingsKeys::Beeps::Timer::second = "tmbs",
    *LedClockSettingsKeys::Beeps::Timer::timeout = "tmbto",

    *LedClockSettingsKeys::Beeps::Stopwatch::start = "swbstart",
    *LedClockSettingsKeys::Beeps::Stopwatch::pause = "swbpause",
    *LedClockSettingsKeys::Beeps::Stopwatch::resume = "swbresume",
    *LedClockSettingsKeys::Beeps::Stopwatch::reset = "swbreset",
    *LedClockSettingsKeys::Beeps::Stopwatch::second = "swbs",
    *LedClockSettingsKeys::Beeps::Stopwatch::minute = "swbm",
    *LedClockSettingsKeys::Beeps::Stopwatch::hour = "swbh",
    *LedClockSettingsKeys::Beeps::Stopwatch::lapTime = "swblapt",

    *LedClockStateKeys::root = "ledclock",

    *LedClockStateKeys::command = "cmd",
    *LedClockStateKeys::mode = "mode",
    *LedClockStateKeys::beep = "beep",

    *LedClockStateKeys::Timer::root = "timer",
    *LedClockStateKeys::Timer::running = "runs",
    *LedClockStateKeys::Timer::paused = "pause",
    *LedClockStateKeys::Timer::left = "left",
    *LedClockStateKeys::Timer::value = "val",

    *LedClockStateKeys::Stopwatch::root = "stopw",
    *LedClockStateKeys::Stopwatch::running = "runs",
    *LedClockStateKeys::Stopwatch::paused = "pause",
    *LedClockStateKeys::Stopwatch::elapsed = "elap",
    *LedClockStateKeys::Stopwatch::lapTimes = "lapt",
    *LedClockStateKeys::Stopwatch::lapTimeNr = "lapnr",
    *LedClockStateKeys::Stopwatch::lastLapTime = "lalap";

unsigned long Timer::_millis() {
    return millis();
}

static uint16_t beep_1x_330_short[] { 1, 330, 50 };
static uint16_t beep_2x_330_short[] { 3, 330, 50, 0, 20, 330, 50 };
static uint16_t beep_3x_330_short[] { 5, 330, 50, 0, 20, 330, 50, 0, 20, 330, 50 };

static uint16_t beep_1x_440_short[] { 1, 440, 50 };
static uint16_t beep_2x_440_short[] { 3, 440, 50, 0, 20, 440, 50 };
static uint16_t beep_3x_440_short[] { 5, 440, 50, 0, 20, 440, 50, 0, 20, 440, 50 };

static uint16_t beep_1x_880_short[] { 1, 880, 50 };
static uint16_t beep_2x_880_short[] { 3, 880, 50, 0, 20, 880, 50 };
static uint16_t beep_3x_880_short[] { 5, 880, 50, 0, 20, 880, 50, 0, 20, 880, 50 };

static uint16_t beep_1x_330_medium[] { 1, 330, 100 };
static uint16_t beep_2x_330_medium[] { 3, 330, 100, 0, 20, 330, 100 };
static uint16_t beep_3x_330_medium[] { 5, 330, 100, 0, 20, 330, 100, 0, 20, 330, 100 };

static uint16_t beep_1x_440_medium[] { 1, 440, 100 };
static uint16_t beep_2x_440_medium[] { 3, 440, 100, 0, 20, 440, 100 };
static uint16_t beep_3x_440_medium[] { 5, 440, 100, 0, 20, 440, 100, 0, 20, 440, 100 };

static uint16_t beep_1x_880_medium[] { 1, 880, 100 };
static uint16_t beep_2x_880_medium[] { 3, 880, 100, 0, 20, 880, 100 };
static uint16_t beep_3x_880_medium[] { 5, 880, 100, 0, 20, 880, 100, 0, 20, 880, 100 };

static uint16_t beep_1x_330_long[] { 1, 330, 200 };
static uint16_t beep_2x_330_long[] { 3, 330, 200, 0, 40, 330, 200 };
static uint16_t beep_3x_330_long[] { 5, 330, 200, 0, 40, 330, 200, 0, 40, 330, 200 };

static uint16_t beep_1x_440_long[] { 1, 440, 200 };
static uint16_t beep_2x_440_long[] { 3, 440, 200, 0, 40, 440, 200 };
static uint16_t beep_3x_440_long[] { 5, 440, 200, 0, 40, 440, 200, 0, 40, 440, 200 };

static uint16_t beep_1x_880_long[] { 1, 880, 200 };
static uint16_t beep_2x_880_long[] { 3, 880, 200, 0, 40, 880, 200 };
static uint16_t beep_3x_880_long[] { 5, 880, 200, 0, 40, 880, 200, 0, 40, 880, 200 };

static uint16_t beep_440_880_short[] { 3, 440, 50, 0, 20, 880, 50 };
static uint16_t beep_880_440_short[] { 3, 880, 50, 0, 20, 440, 50 };

static uint16_t beep_440_880_medium[] { 3, 440, 100, 0, 20, 880, 100 };
static uint16_t beep_880_440_medium[] { 3, 880, 100, 0, 20, 440, 100 };

static uint16_t beep_440_880_long[] { 3, 440, 200, 0, 40, 880, 200 };
static uint16_t beep_880_440_long[] { 3, 880, 200, 0, 40, 440, 200 };

static uint16_t beep_turn_up[] { 7, 740, 100, 0, 20, 831, 100, 0, 20, 880, 100, 0, 20, 932, 100 };
static uint16_t beep_turn_down[] { 7, 932, 100, 0, 20, 880, 100, 0, 20, 831, 100, 0, 20, 740, 100 };

static uint16_t beep_flip_up[] { 3, 261, 50, 0, 20, 440, 50 };
static uint16_t beep_flip_down[] { 3, 440, 50, 0, 20, 261, 50 };

static uint16_t beep_tadaaa[] { 3, 1047, 100, 0, 50, 1047, 300 };

static uint16_t* beeps[] {
    beep_1x_330_short,
    beep_2x_330_short,
    beep_3x_330_short,
    beep_1x_440_short,
    beep_2x_440_short,
    beep_3x_440_short,
    beep_1x_880_short,
    beep_2x_880_short,
    beep_3x_880_short,
    beep_1x_330_medium,
    beep_2x_330_medium,
    beep_3x_330_medium,
    beep_1x_440_medium,
    beep_2x_440_medium,
    beep_3x_440_medium,
    beep_1x_880_medium,
    beep_2x_880_medium,
    beep_3x_880_medium,
    beep_1x_330_long,
    beep_2x_330_long,
    beep_3x_330_long,
    beep_1x_440_long,
    beep_2x_440_long,
    beep_3x_440_long,
    beep_1x_880_long,
    beep_2x_880_long,
    beep_3x_880_long,
    beep_440_880_short,
    beep_880_440_short,
    beep_440_880_medium,
    beep_880_440_medium,
    beep_440_880_long,
    beep_880_440_long,
    beep_turn_up,
    beep_turn_down,
    beep_flip_up,
    beep_flip_down,
    beep_tadaaa
};

static uint8_t beepCount = sizeof(beeps) / sizeof(uint16_t*);

uint8_t LedClockSettings::constrainBeep(uint8_t beep) {
    return beep == BEEP_SILENT ? beep : constrain(beep, 0, beepCount - 1);
}

static CRGB selfTestColors[] = { CRGB::Red, CRGB::Green, CRGB::Blue };
static uint8_t selfTestColorCount = sizeof(selfTestColors) / sizeof(CRGB);

static uint16_t normalizedSensorRading;

static uint8_t brightness(uint8_t minBrightness, uint8_t maxBrightness) {
    static uint16_t values[BRIGHTNESS_SAMPLES];
    static int i = 0;
    static long total;
    static uint8_t current;

    total -= values[i];
    values[i] = analogRead(BRIGHTNESS_PIN);
    total += values[i];

    i++;
    i %= BRIGHTNESS_SAMPLES;

    normalizedSensorRading = total / BRIGHTNESS_SAMPLES;

    uint8_t target = map(normalizedSensorRading, 0, ADC_MAX_VALUE, minBrightness, maxBrightness);

    if (abs(target - current) > BRIGHTNESS_THRESHOLD) {
        current = target;
    }

    return current;
}

enum TimeChange {
    HOUR, MINUTE, SECOND, NONE
};

static TimeChange timeChangeTime(time_t t, bool clear = false) {
    static time_t prev = t;
    if (clear) prev = t;
    TimeChange c;
    if (hour(prev) != hour(t)) {
        c = HOUR;
    } else if (minute(prev) != minute(t)) {
        c = MINUTE;
    } else if (second(prev) != second(t)) {
        c = SECOND;
    } else {
        c = NONE;
    }
    prev = t;
    return c;
}

static TimeChange timeChangeMillis(unsigned long millis, bool clear = false) {
    static unsigned long prev = millis;
    if (clear) prev = millis;
    TimeChange c;
    if ((prev / 3600000) != (millis / 3600000)) {
        c = HOUR;
    } else if ((prev / 60000) != (millis / 60000)) {
        c = MINUTE;
    } else if ((prev / 1000) != (millis / 1000)) {
        c = SECOND;
    } else {
        c = NONE;
    }
    prev = millis;
    return c;
}

static void outputPixel(uint8_t i, uint8_t r, uint8_t g, uint8_t b) {
    strip.setPixelColor(i, r, g, b);
}

class UsermodLedClock : public Usermod, public LedClockSettings {

private:
    enum Mode {

        ClockMode, TimerMode, StopwatchMode
    };

    enum Command {
        StopwatchStart,
        StopwatchPause,
        StopwatchReset,
        StopwatchLapTime,

        TimerStart,
        TimerPause,
        TimerReset,
        TimerIncrease,
        TimerSet,

        NoOp = 99
    };

    SevenSegmentDisplay dHoursT;
    SevenSegmentDisplay dHoursO;

    SeparatorDisplay sep;

    SevenSegmentDisplay dMinutesT;
    SevenSegmentDisplay dMinutesO;

    LedBasedRowDisplay display;

    Beeper beeper;

    time_t p;

    uint8_t br;

    Timer selfTestTimer;
    uint8_t selfTestCycle = 0;
    uint8_t selfTestIdx = 0;
    bool selfTestDone = false;

    uint32_t* backup = 0;
    uint16_t backupLength = 0;

    Mode mode = Mode::ClockMode;

    Timer stopwatchTimer;
    bool stopwatchRunnig = false;
    bool stopwatchPaused = false;
    unsigned long stopwatchElapsed = 0;
    unsigned long stopwatchPrevLapTime = 0;
    unsigned long stopwatchLapTimes[STOPWATCH_MAX_LAP_TIMES];
    uint8_t stopwatchLapTimeIdx = 0;
    uint32_t stopwatchLaptimeNr = 0;

    Timer timerTimer;
    bool timerRunnig = false;
    bool timerPaused = false;
    unsigned long timerLeft = 0;
    unsigned long timerValue = 0;

public:

    UsermodLedClock():
        dHoursT(&outputPixel, 2),
        dHoursO(&outputPixel, 2),
        sep(&outputPixel),
        dMinutesT(&outputPixel, 2),
        dMinutesO(&outputPixel, 2),
        display(5, &dHoursT, &dHoursO, &sep, &dMinutesT, &dMinutesO),
        beeper(0, BUZZER_PIN),
        selfTestTimer(10),
        stopwatchTimer(0),
        timerTimer(0) {}

    LedBasedDisplay* getDisplay() {
        return &display;
    }

    void setup() {
        // digit 1
        dHoursT.setShowZero(!hideZero);
        dHoursT.mapSegment(_7SEG_SEG_A, 6, 7);
        dHoursT.mapSegment(_7SEG_SEG_B, 8, 9);
        dHoursT.mapSegment(_7SEG_SEG_C, 12, 13);
        dHoursT.mapSegment(_7SEG_SEG_D, 1, 0);
        dHoursT.mapSegment(_7SEG_SEG_E, 3, 2);
        dHoursT.mapSegment(_7SEG_SEG_F, 5, 4);
        dHoursT.mapSegment(_7SEG_SEG_G, 11, 10);

        // digit 2
        dHoursO.mapSegment(_7SEG_SEG_A, 20, 21);
        dHoursO.mapSegment(_7SEG_SEG_B, 22, 23);
        dHoursO.mapSegment(_7SEG_SEG_C, 26, 27);
        dHoursO.mapSegment(_7SEG_SEG_D, 15, 14);
        dHoursO.mapSegment(_7SEG_SEG_E, 17, 16);
        dHoursO.mapSegment(_7SEG_SEG_F, 19, 18);
        dHoursO.mapSegment(_7SEG_SEG_G, 25, 24);

        sep.map(2,
            2, 0, 28,
            4, 0, 29);

        // digit 3
        dMinutesT.mapSegment(_7SEG_SEG_A, 36, 37);
        dMinutesT.mapSegment(_7SEG_SEG_B, 38, 39);
        dMinutesT.mapSegment(_7SEG_SEG_C, 42, 43);
        dMinutesT.mapSegment(_7SEG_SEG_D, 31, 30);
        dMinutesT.mapSegment(_7SEG_SEG_E, 33, 32);
        dMinutesT.mapSegment(_7SEG_SEG_F, 35, 34);
        dMinutesT.mapSegment(_7SEG_SEG_G, 41, 40);

        // digit 4
        dMinutesO.mapSegment(_7SEG_SEG_A, 50, 51);
        dMinutesO.mapSegment(_7SEG_SEG_B, 52, 53);
        dMinutesO.mapSegment(_7SEG_SEG_C, 56, 57);
        dMinutesO.mapSegment(_7SEG_SEG_D, 45, 44);
        dMinutesO.mapSegment(_7SEG_SEG_E, 47, 46);
        dMinutesO.mapSegment(_7SEG_SEG_F, 49, 48);
        dMinutesO.mapSegment(_7SEG_SEG_G, 55, 54);

        display.setColor(false, CRGB::Black);
        display.setColor(true, CRGB::Red);
        display.setMode(LedBasedDisplayMode::SET_OFF_LEDS);

        pinMode(BRIGHTNESS_PIN, INPUT);

        beep(beepStartup);
    }

    void beep(uint8_t beep) {
        if (!muteBeeps && beep < beepCount) {
            beeper.play(beeps[beep]);
        }
    }

    void clearDisplay() {
        dHoursT.setSymbol(_7SEG_SYM_EMPTY);
        dHoursO.setSymbol(_7SEG_SYM_EMPTY);
        dMinutesT.setSymbol(_7SEG_SYM_EMPTY);
        dMinutesO.setSymbol(_7SEG_SYM_EMPTY);
    }

    void displayMillis(unsigned long millis) {
        if (millis < 60000) {
            unsigned long seconds = millis / 1000;
            unsigned long hundredths = (millis % 1000) / 10;
            dHoursT.setDigit(seconds / 10);
            dHoursO.setDigit(seconds % 10);
            dMinutesT.setDigit(hundredths / 10);
            dMinutesO.setDigit(hundredths % 10);
        } else if (millis < 3600000) {
            unsigned long minutes = millis / 60000;
            unsigned long seconds = (millis % 60000) / 1000;
            dHoursT.setDigit(minutes / 10);
            dHoursO.setDigit(minutes % 10);
            dMinutesT.setDigit(seconds / 10);
            dMinutesO.setDigit(seconds % 10);
        } else {
            unsigned long hours = millis / 3600000;
            unsigned long minutes = (millis % 3600000) / 60000;
            dHoursT.setDigit(hours / 10);
            dHoursO.setDigit(hours % 10);
            dMinutesT.setDigit(minutes / 10);
            dMinutesO.setDigit(minutes % 10);
        }
    }

    void loop() {
        beeper.update();

        if (selfTestDone) {
            switch (mode) {
            case Mode::ClockMode:
                switch (timeChangeTime(localTime)) {
                case HOUR: beep(clockBeepHour); break;
                case MINUTE: beep(clockBeepMinute); break;
                default: break;
                }
                if (p != localTime) {
                    p = localTime;

                    int hr = hour(p);
                    if (useAMPM) {
                        if (hr > 11) hr -= 12;
                        if (hr == 0) hr  = 12;
                    }

                    dHoursT.setDigit(hr / 10);
                    dHoursO.setDigit(hr % 10);

                    switch (separatorMode) {
                    case SeparatorMode::ON: sep.setState(true); break;
                    case SeparatorMode::OFF: sep.setState(false); break;
                    case SeparatorMode::BLINK: sep.setState(second(p) % 2); break;
                    }

                    dMinutesT.setDigit(minute(p) / 10);
                    dMinutesO.setDigit(minute(p) % 10);
                }
                break;
            case Mode::TimerMode:
                if (timerRunnig) {
                    unsigned long left;

                    if (timerPaused) {
                        left = timerLeft;
                    } else {
                        unsigned long el = timerTimer.elapsed();
                        if (el > timerLeft) {
                            left = 0;
                        } else {
                            left = timerLeft - el;
                        }
                    }

                    if (left == 0) {
                        beep(timerBeepTimeout);
                        timerLeft = 0;
                        timerRunnig = false;
                        timerPaused = false;
                        sendDataWs();
                    } else {
                        switch (timeChangeMillis(left)) {
                        case HOUR: beep(timerBeepHour); break;
                        case MINUTE: beep(timerBeepMinute); break;
                        case SECOND: if (left < 60000) beep(timerBeepSecond); break;
                        default: break;
                        }
                    }

                    bool on = timerPaused
                        ? ((timerTimer.elapsed() / 1000) % 2) // blinking syncs to elapsed time since paused
                        : ((left / 1000) % 2); // blinking syncs to timer elapsed time

                    if (!timerPaused || on) {
                        displayMillis(left);
                    } else {
                        clearDisplay();
                    }

                    sep.setState(on);
                } else {
                    displayMillis(timerLeft);
                    sep.setState(true);
                }
                break;
            case Mode::StopwatchMode:
                if (stopwatchRunnig) {
                    unsigned long elapsed = stopwatchPaused
                        ? stopwatchElapsed
                        : (stopwatchElapsed + stopwatchTimer.elapsed());

                    bool on = stopwatchPaused
                        ? ((stopwatchTimer.elapsed() / 1000) % 2) // blinking syncs to elapsed time since paused
                        : ((elapsed / 1000) % 2); // blinking syncs to stopwatch elapsed time

                    switch (timeChangeMillis(elapsed)) {
                    case HOUR: beep(stopwatchBeepHour); break;
                    case MINUTE: beep(stopwatchBeepMinute); break;
                    case SECOND: if (elapsed < 60000) beep(stopwatchBeepSecond); break;
                    default: break;
                    }

                    if (!stopwatchPaused || on) {
                        displayMillis(elapsed);
                    } else {
                        clearDisplay();
                    }

                    sep.setState(on);
                } else {
                    displayMillis(stopwatchElapsed);
                    sep.setState(true);
                }
                break;
            default:
                break;
            }

            if (!strip.isUpdating()) {
                br = brightness(minBrightness, maxBrightness);
            }
        } else {
            if (selfTestTimer.fire()) {
                selfTestIdx++;
                if (selfTestIdx >= strip.getLengthTotal()) {
                    selfTestIdx = 0;
                    selfTestCycle++;
                    if (selfTestCycle >= selfTestColorCount) {
                        selfTestDone = true;
                    }
                }
            }
        }
    }

    void connected() {
        beep(beepWiFi);
    }

    void addToJsonInfo(JsonObject& root) {
        JsonObject user = root["u"];
        if (user.isNull()) user = root.createNestedObject("u");
        JsonArray lightArr = user.createNestedArray("Light sensor");
        double reading = normalizedSensorRading;
        lightArr.add((reading / ADC_MAX_VALUE) * ADC_MAX_VOLTAGE);
        lightArr.add("V");
    }

    void addToJsonState(JsonObject& root) {
        JsonObject state = root.createNestedObject(LedClockStateKeys::root);
        state[LedClockStateKeys::mode] = mode;
        switch (mode) {
        case Mode::StopwatchMode: {
            JsonObject stopwatch = state.createNestedObject(LedClockStateKeys::Stopwatch::root);

            stopwatch[LedClockStateKeys::Stopwatch::running] = stopwatchRunnig;
            stopwatch[LedClockStateKeys::Stopwatch::paused] = stopwatchPaused;
            stopwatch[LedClockStateKeys::Stopwatch::elapsed] = stopwatchRunnig
                ? (stopwatchPaused
                    ? stopwatchElapsed
                    : (stopwatchElapsed + stopwatchTimer.elapsed()))
                : 0;

            JsonArray arr = stopwatch.createNestedArray(LedClockStateKeys::Stopwatch::lapTimes);
            for (uint8_t i = stopwatchLapTimeIdx; i > 0; --i) {
                arr.add(stopwatchLapTimes[i - 1]);
            }
            if (stopwatchLaptimeNr != stopwatchLapTimeIdx) {
                for (uint8_t i = STOPWATCH_MAX_LAP_TIMES; i > stopwatchLapTimeIdx; --i) {
                    arr.add(stopwatchLapTimes[i - 1]);
                }
            }

            stopwatch[LedClockStateKeys::Stopwatch::lapTimeNr] = stopwatchLaptimeNr;
            stopwatch[LedClockStateKeys::Stopwatch::lastLapTime] = stopwatchPrevLapTime;
            break;
        }
        case Mode::TimerMode: {
            JsonObject timer = state.createNestedObject(LedClockStateKeys::Timer::root);

            timer[LedClockStateKeys::Timer::running] = timerRunnig;
            timer[LedClockStateKeys::Timer::paused] = timerPaused;
            timer[LedClockStateKeys::Timer::left] = timerRunnig
                ? (timerPaused
                    ? timerLeft
                    : (timerLeft - timerTimer.elapsed()))
                : timerValue;

            timer[LedClockStateKeys::Timer::value] = timerValue;
            break;
        }
        default:
            break;
        }
    }

    void readFromJsonState(JsonObject& root) {
        JsonObject state = root[LedClockStateKeys::root];
        if (!state.isNull()) {
            Mode m = state[LedClockStateKeys::mode] | mode;
            if (m != mode) {
                if (m != Mode::StopwatchMode) {
                    resetStopwatch();
                }
                if (m != Mode::TimerMode) {
                    resetTimer();
                }
                mode = m;
                if (mode == Mode::ClockMode) {
                    timeChangeTime(localTime, true);
                }
            }

            uint8_t b = state[LedClockStateKeys::beep] | 255;
            if (b < beepCount) {
                beeper.play(beeps[b]);
            }

            Command c = state[LedClockStateKeys::command] | Command::NoOp;
            switch (c) {
            case Command::StopwatchStart:
                timeChangeMillis(0, true);
                if (stopwatchRunnig) {
                    beep(stopwatchBeepResume);
                } else {
                    beep(stopwatchBeepStart);
                    stopwatchRunnig = true;
                }
                stopwatchPaused = false;
                stopwatchTimer.setOriginToNow();
                break;
            case Command::StopwatchPause:
                beep(stopwatchBeepPause);
                stopwatchRunnig = true;
                stopwatchPaused = true;
                stopwatchElapsed += stopwatchTimer.elapsed();
                stopwatchTimer.setOriginToNow();
                break;
            case Command::StopwatchReset:
                beep(stopwatchBeepReset);
                resetStopwatch();
                break;
            case Command::StopwatchLapTime:
                if (stopwatchRunnig && !stopwatchPaused) {
                    beep(stopwatchBeepLapTime);
                    unsigned long elapsed = stopwatchElapsed + stopwatchTimer.elapsed();
                    stopwatchLapTimes[stopwatchLapTimeIdx++] = elapsed - stopwatchPrevLapTime;
                    stopwatchPrevLapTime = elapsed;
                    if (stopwatchLapTimeIdx == STOPWATCH_MAX_LAP_TIMES) {
                        stopwatchLapTimeIdx = 0;
                    }
                    stopwatchLaptimeNr++;
                }
                break;
            case Command::TimerStart:
                if (timerRunnig) {
                    beep(timerBeepResume);
                } else {
                    beep(timerBeepStart);
                }
                if (timerLeft == 0) {
                    timerLeft = timerValue;
                }
                timeChangeMillis(timerLeft, true);
                timerRunnig = true;
                timerPaused = false;
                timerTimer.setOriginToNow();
                break;
            case Command::TimerPause:
                beep(timerBeepPause);
                timerRunnig = true;
                timerPaused = true;
                timerLeft -= timerTimer.elapsed();
                timerTimer.setOriginToNow();
                break;
            case Command::TimerReset:
                beep(timerBeepReset);
                resetTimer();
                break;
            case Command::TimerIncrease: {
                if (timerRunnig && !timerPaused) {
                    beep(timerBeepIncrease);
                    unsigned long val = state[LedClockStateKeys::Timer::value];
                    timerLeft += val;

                    unsigned long el = timerTimer.elapsed();
                    unsigned long left;
                    if (el > timerLeft) {
                        left = 0;
                    } else {
                        left = timerLeft - el;
                    }

                    timeChangeMillis(left, true);
                }
                break;
            }
            case Command::TimerSet: {
                if (!timerRunnig) {
                    beep(timerBeepSet);
                    unsigned long val = state[LedClockStateKeys::Timer::value];
                    timerValue = val;
                    timerLeft = val;
                }
                break;
            }
            default:
                break;
            }
        }
    }

    void resetStopwatch() {
        stopwatchRunnig = false;
        stopwatchPaused = false;
        stopwatchElapsed = 0;
        stopwatchPrevLapTime = 0;
        stopwatchLapTimeIdx = 0;
        stopwatchLaptimeNr = 0;
    }

    void resetTimer() {
        timerRunnig = false;
        timerPaused = false;
        timerLeft = timerValue;
    }

    void addToConfig(JsonObject& root) {
        JsonObject top = root.createNestedObject(LedClockSettingsKeys::root);

        top[LedClockSettingsKeys::Brightness::autom] = autoBrightness;
        top[LedClockSettingsKeys::Brightness::min] = minBrightness;
        top[LedClockSettingsKeys::Brightness::max] = maxBrightness;

        top[LedClockSettingsKeys::Display::separatorMode] = separatorMode;
        top[LedClockSettingsKeys::Display::hideZero] = hideZero;

        top[LedClockSettingsKeys::Beeps::mute] = muteBeeps;

        top[LedClockSettingsKeys::Beeps::startup] = beepStartup;
        top[LedClockSettingsKeys::Beeps::wifi] = beepWiFi;

        top[LedClockSettingsKeys::Beeps::Clock::hour] = clockBeepHour;
        top[LedClockSettingsKeys::Beeps::Clock::minute] = clockBeepMinute;

        top[LedClockSettingsKeys::Beeps::Timer::set] = timerBeepSet;
        top[LedClockSettingsKeys::Beeps::Timer::start] = timerBeepStart;
        top[LedClockSettingsKeys::Beeps::Timer::pause] = timerBeepPause;
        top[LedClockSettingsKeys::Beeps::Timer::resume] = timerBeepResume;
        top[LedClockSettingsKeys::Beeps::Timer::reset] = timerBeepReset;
        top[LedClockSettingsKeys::Beeps::Timer::increase] = timerBeepIncrease;
        top[LedClockSettingsKeys::Beeps::Timer::hour] = timerBeepHour;
        top[LedClockSettingsKeys::Beeps::Timer::minute] = timerBeepMinute;
        top[LedClockSettingsKeys::Beeps::Timer::second] = timerBeepSecond;
        top[LedClockSettingsKeys::Beeps::Timer::timeout] = timerBeepTimeout;

        top[LedClockSettingsKeys::Beeps::Stopwatch::start] = stopwatchBeepStart;
        top[LedClockSettingsKeys::Beeps::Stopwatch::pause] = stopwatchBeepPause;
        top[LedClockSettingsKeys::Beeps::Stopwatch::resume] = stopwatchBeepResume;
        top[LedClockSettingsKeys::Beeps::Stopwatch::reset] = stopwatchBeepReset;
        top[LedClockSettingsKeys::Beeps::Stopwatch::second] = stopwatchBeepSecond;
        top[LedClockSettingsKeys::Beeps::Stopwatch::minute] = stopwatchBeepMinute;
        top[LedClockSettingsKeys::Beeps::Stopwatch::hour] = stopwatchBeepHour;
        top[LedClockSettingsKeys::Beeps::Stopwatch::lapTime] = stopwatchBeepLapTime;
    }

    bool readFromConfig(JsonObject& root) {
        JsonObject top = root[LedClockSettingsKeys::root];

        bool configComplete = !top.isNull();

        configComplete &= getJsonValue(top[LedClockSettingsKeys::Brightness::autom], autoBrightness, true);
        configComplete &= getJsonValue(top[LedClockSettingsKeys::Brightness::min], minBrightness, 50);
        configComplete &= getJsonValue(top[LedClockSettingsKeys::Brightness::max], maxBrightness, 255);

        configComplete &= getJsonValue(top[LedClockSettingsKeys::Display::separatorMode], separatorMode, SeparatorMode::BLINK);
        configComplete &= getJsonValue(top[LedClockSettingsKeys::Display::hideZero], hideZero, true);

        configComplete &= getJsonValue(top[LedClockSettingsKeys::Beeps::mute], muteBeeps, false);

        configComplete &= getJsonValue(top[LedClockSettingsKeys::Beeps::startup], beepStartup, 33);
        configComplete &= getJsonValue(top[LedClockSettingsKeys::Beeps::wifi], beepWiFi, 27);

        configComplete &= getJsonValue(top[LedClockSettingsKeys::Beeps::Clock::minute], clockBeepMinute, BEEP_SILENT);
        configComplete &= getJsonValue(top[LedClockSettingsKeys::Beeps::Clock::hour], clockBeepHour, BEEP_SILENT);

        configComplete &= getJsonValue(top[LedClockSettingsKeys::Beeps::Timer::set], timerBeepSet, 35);
        configComplete &= getJsonValue(top[LedClockSettingsKeys::Beeps::Timer::start], timerBeepStart, 12);
        configComplete &= getJsonValue(top[LedClockSettingsKeys::Beeps::Timer::pause], timerBeepPause, 9);
        configComplete &= getJsonValue(top[LedClockSettingsKeys::Beeps::Timer::resume], timerBeepResume, 12);
        configComplete &= getJsonValue(top[LedClockSettingsKeys::Beeps::Timer::reset], timerBeepReset, 16);
        configComplete &= getJsonValue(top[LedClockSettingsKeys::Beeps::Timer::increase], timerBeepIncrease, 35);
        configComplete &= getJsonValue(top[LedClockSettingsKeys::Beeps::Timer::hour], timerBeepHour, 8);
        configComplete &= getJsonValue(top[LedClockSettingsKeys::Beeps::Timer::minute], timerBeepMinute, 7);
        configComplete &= getJsonValue(top[LedClockSettingsKeys::Beeps::Timer::second], timerBeepSecond, 6);
        configComplete &= getJsonValue(top[LedClockSettingsKeys::Beeps::Timer::timeout], timerBeepTimeout, 37);

        configComplete &= getJsonValue(top[LedClockSettingsKeys::Beeps::Stopwatch::start], stopwatchBeepStart, 12);
        configComplete &= getJsonValue(top[LedClockSettingsKeys::Beeps::Stopwatch::pause], stopwatchBeepPause, 9);
        configComplete &= getJsonValue(top[LedClockSettingsKeys::Beeps::Stopwatch::resume], stopwatchBeepResume, 12);
        configComplete &= getJsonValue(top[LedClockSettingsKeys::Beeps::Stopwatch::reset], stopwatchBeepReset, 16);
        configComplete &= getJsonValue(top[LedClockSettingsKeys::Beeps::Stopwatch::second], stopwatchBeepSecond, 6);
        configComplete &= getJsonValue(top[LedClockSettingsKeys::Beeps::Stopwatch::minute], stopwatchBeepMinute, 7);
        configComplete &= getJsonValue(top[LedClockSettingsKeys::Beeps::Stopwatch::hour], stopwatchBeepHour, 8);
        configComplete &= getJsonValue(top[LedClockSettingsKeys::Beeps::Stopwatch::lapTime], stopwatchBeepLapTime, 35);

        return configComplete;
    }

    void applySettings() {
        dHoursT.setShowZero(!hideZero);
    }

    void handleOverlayDraw() {
        if (selfTestDone) {
            backupStrip();
            display.update();
            if (autoBrightness && bri > 0 && bri != br) {
                bri = br;
                stateUpdated(CALL_MODE_DIRECT_CHANGE);
            }
        } else {
            for (uint8_t i = 0, n = strip.getLengthTotal(); i < n; ++i) {
                CRGB color = i <= selfTestIdx
                    ? selfTestColors[selfTestCycle]
                    : (selfTestCycle > 0
                        ? selfTestColors[selfTestCycle - 1]
                        : CRGB::Black);

                strip.setPixelColor(i, color.r, color.g, color.b);
            }
        }
    }

    void rollbackOverlayDraw() {
        rollbackStrip();
    }

    void backupStrip() {
        uint16_t length = strip.getLengthTotal();
        if (backupLength != length) {
            backupLength = length;
            backup = (uint32_t *) realloc(backup, sizeof(uint32_t) * length);
        }
        for (int i = 0; i < length; ++i) {
            backup[i] = strip.getPixelColor(i);
        }
    }

    void rollbackStrip() {
        for (int i = 0, n = min(strip.getLengthTotal(), backupLength); i < n; ++i) {
            strip.setPixelColor(i, backup[i]);
        }
    }

    uint16_t getId() {
        return USERMOD_ID_LEDCLOCK;
    }
};

LedBasedDisplay* ledClockDisplay() {
    UsermodLedClock* mod = (UsermodLedClock*) usermods.lookup(USERMOD_ID_LEDCLOCK);
    return mod->getDisplay();
}

void ledClockTimeUpdated() {
    timeChangeTime(localTime, true);
}