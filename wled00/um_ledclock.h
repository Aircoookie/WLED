#pragma once

#include "wled.h"
#include "7segmdisp.h"
#include "beeper.h"

const char * ledClockSettingsKey = "ledclock";
const char * ledClockSettingsKeyAutoBrightness = "autb";
const char * ledClockSettingsKeyMinBrightness = "minb";
const char * ledClockSettingsKeyMaxBrightness = "maxb";
const char * ledClockSettingsKeySeparatorMode = "sepm";
const char * ledClockSettingsKeyHideZero = "hidzer";

const char * ledClockStateKey = "ledclock";

const char * ledClockStateKeyCommand = "cmd";
const char * ledClockStateKeyMode = "mode";

const char * ledClockStateKeyStopwatch = "stopw";
const char * ledClockStateKeyStopwatchRunning = "runs";
const char * ledClockStateKeyStopwatchPaused = "pause";
const char * ledClockStateKeyStopwatchElapsed = "elap";
const char * ledClockStateKeyStopwatchLapTimes = "lapt";
const char * ledClockStateKeyStopwatchLapTimeNr = "lapnr";
const char * ledClockStateKeyStopwatchLastLapTime = "lalap";

const char * ledClockStateKeyTimer = "timer";
const char * ledClockStateKeyTimerRunning = "runs";
const char * ledClockStateKeyTimerPaused = "pause";
const char * ledClockStateKeyTimerLeft = "left";
const char * ledClockStateKeyTimerValue = "val";

unsigned long Timer::_millis() {
    return millis();
}

static uint16_t beep_startup[] { 3, 440, 100, 0, 20, 880, 100 };
static uint16_t beep_connected[] { 5, 880, 100, 0, 20, 880, 100, 0, 20, 880, 100 };

static CRGB selfTestColors[] = { CRGB::Red, CRGB::Green, CRGB::Blue };
static uint8_t selfTestColorCount = sizeof(selfTestColors) / sizeof(CRGB);

static uint16_t normalizedSensorRading;

uint8_t brightness(uint8_t minBrightness, uint8_t maxBrightness) {
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
        dHoursT(&strip, 2),
        dHoursO(&strip, 2),
        sep(&strip),
        dMinutesT(&strip, 2),
        dMinutesO(&strip, 2),
        display(5, &dHoursT, &dHoursO, &sep, &dMinutesT, &dMinutesO),
        beeper(0, BUZZER_PIN),
        selfTestTimer(20),
        stopwatchTimer(0),
        timerTimer(0) {}

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

        beeper.play(beep_startup);
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
                        timerLeft = 0;
                        timerRunnig = false;
                        timerPaused = false;
                        sendDataWs();
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
        beeper.play(beep_connected);
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
        JsonObject state = root.createNestedObject(ledClockStateKey);
        state[ledClockStateKeyMode] = mode;
        switch (mode) {
        case Mode::StopwatchMode: {
            JsonObject stopwatch = state.createNestedObject(ledClockStateKeyStopwatch);

            stopwatch[ledClockStateKeyStopwatchRunning] = stopwatchRunnig;
            stopwatch[ledClockStateKeyStopwatchPaused] = stopwatchPaused;
            stopwatch[ledClockStateKeyStopwatchElapsed] = stopwatchRunnig
                ? (stopwatchPaused
                    ? stopwatchElapsed
                    : (stopwatchElapsed + stopwatchTimer.elapsed()))
                : 0;

            JsonArray arr = stopwatch.createNestedArray(ledClockStateKeyStopwatchLapTimes);
            for (uint8_t i = stopwatchLapTimeIdx; i > 0; --i) {
                arr.add(stopwatchLapTimes[i - 1]);
            }
            if (stopwatchLaptimeNr != stopwatchLapTimeIdx) {
                for (uint8_t i = STOPWATCH_MAX_LAP_TIMES; i > stopwatchLapTimeIdx; --i) {
                    arr.add(stopwatchLapTimes[i - 1]);
                }
            }

            stopwatch[ledClockStateKeyStopwatchLapTimeNr] = stopwatchLaptimeNr;
            stopwatch[ledClockStateKeyStopwatchLastLapTime] = stopwatchPrevLapTime;
            break;
        }
        case Mode::TimerMode: {
            JsonObject timer = state.createNestedObject(ledClockStateKeyTimer);

            timer[ledClockStateKeyTimerRunning] = timerRunnig;
            timer[ledClockStateKeyTimerPaused] = timerPaused;
            timer[ledClockStateKeyTimerLeft] = timerRunnig
                ? (timerPaused
                    ? timerLeft
                    : (timerLeft - timerTimer.elapsed()))
                : timerValue;

            timer[ledClockStateKeyTimerValue] = timerValue;
            break;
        }
        default:
            break;
        }
    }

    void readFromJsonState(JsonObject& root) {
        JsonObject state = root[ledClockStateKey];
        if (!state.isNull()) {
            Mode m = state[ledClockStateKeyMode] | mode;
            if (m != mode) {
                if (m != Mode::StopwatchMode) {
                    resetStopwatch();
                }
                if (m != Mode::TimerMode) {
                    resetTimer();
                }
                mode = m;
            }
            Command c = state[ledClockStateKeyCommand] | Command::NoOp;
            switch (c) {
            case Command::StopwatchStart:
                stopwatchRunnig = true;
                stopwatchPaused = false;
                stopwatchTimer.setOriginToNow();
                break;
            case Command::StopwatchPause:
                stopwatchRunnig = true;
                stopwatchPaused = true;
                stopwatchElapsed += stopwatchTimer.elapsed();
                stopwatchTimer.setOriginToNow();
                break;
            case Command::StopwatchReset:
                resetStopwatch();
                break;
            case Command::StopwatchLapTime:
                if (stopwatchRunnig && !stopwatchPaused) {
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
                if (timerLeft == 0) {
                    timerLeft = timerValue;
                }
                timerRunnig = true;
                timerPaused = false;
                timerTimer.setOriginToNow();
                break;
            case Command::TimerPause:
                timerRunnig = true;
                timerPaused = true;
                timerLeft -= timerTimer.elapsed();
                timerTimer.setOriginToNow();
                break;
            case Command::TimerReset:
                resetTimer();
                break;
            case Command::TimerIncrease: {
                if (timerRunnig && !timerPaused) {
                    unsigned long val = state[ledClockStateKeyTimerValue];
                    timerLeft += val;
                }
                break;
            }
            case Command::TimerSet: {
                if (!timerRunnig) {
                    unsigned long val = state[ledClockStateKeyTimerValue];
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
        JsonObject top = root.createNestedObject(ledClockSettingsKey);

        top[ledClockSettingsKeyAutoBrightness] = autoBrightness;
        top[ledClockSettingsKeyMinBrightness] = minBrightness;
        top[ledClockSettingsKeyMaxBrightness] = maxBrightness;
        top[ledClockSettingsKeySeparatorMode] = separatorMode;
        top[ledClockSettingsKeyHideZero] = hideZero;
    }

    bool readFromConfig(JsonObject& root) {
        JsonObject top = root[ledClockSettingsKey];

        bool configComplete = !top.isNull();

        configComplete &= getJsonValue(top[ledClockSettingsKeyAutoBrightness], autoBrightness, true);
        configComplete &= getJsonValue(top[ledClockSettingsKeyMinBrightness], minBrightness, 50);
        configComplete &= getJsonValue(top[ledClockSettingsKeyMaxBrightness], maxBrightness, 255);
        configComplete &= getJsonValue(top[ledClockSettingsKeySeparatorMode], separatorMode, SeparatorMode::BLINK);
        configComplete &= getJsonValue(top[ledClockSettingsKeyHideZero], hideZero, true);

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