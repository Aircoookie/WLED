#pragma once

#include "wled.h"
#include "7segmdisp.h"

class UsermodLedClock : public Usermod {

private:
    SevenSegmentDisplay dHoursT;
    SevenSegmentDisplay dHoursO;

    SeparatorDisplay sep;

    SevenSegmentDisplay dMinutesT;
    SevenSegmentDisplay dMinutesO;

    LedBasedRowDisplay display;

    time_t p;

public:
    UsermodLedClock():
        dHoursT(&strip, 2),
        dHoursO(&strip, 2),
        sep(&strip),
        dMinutesT(&strip, 2),
        dMinutesO(&strip, 2),
        display(5, &dHoursT, &dHoursO, &sep, &dMinutesT, &dMinutesO) {}

    void setup() {
        // digit 1
        dHoursT.setShowZero(false);
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
    }

    void loop() {
        if (p != localTime) {
            p = localTime;

            dHoursT.setDigit(hour(p) / 10);
            dHoursO.setDigit(hour(p) % 10);

            sep.setState(second(p) % 2);

            dMinutesT.setDigit(minute(p) / 10);
            dMinutesO.setDigit(minute(p) % 10);
        }
    }

    void handleOverlayDraw() {
        display.update();
    }

    uint16_t getId() {
        return USERMOD_ID_LEDCLOCK;
    }
};