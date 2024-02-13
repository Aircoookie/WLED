#ifndef __7SEGMENTDISP_H
#define __7SEGMENTDISP_H

#include <stdarg.h>

#define FASTLED_INTERNAL
#define USE_GET_MILLISECOND_TIMER
#include <FastLED.h>
#undef FASTLED_INTERNAL
#undef USE_GET_MILLISECOND_TIMER

// 0x00000000
//    GFEDCBA
//
//    _a__
//   f    b
//   |_g__|
//   e    c
//   |_d__|
//
//     x (0,1..n) x
//   (1,0) x x (1,n+1)
//   (2,0) x x (2,n+1)
//     x (3,1..n) x
//   (4,0) x x (4,n+1)
//   (5,0) x x (5,n+1)
//     x (6,1..n) x

#define _7SEG_SEG_A 0
#define _7SEG_SEG_B 1
#define _7SEG_SEG_C 2
#define _7SEG_SEG_D 3
#define _7SEG_SEG_E 4
#define _7SEG_SEG_F 5
#define _7SEG_SEG_G 6

#define _7SEG_SYM_EMPTY       0b00000000

#define _7SEG_SYM_0           0b00111111
#define _7SEG_SYM_1           0b00000110
#define _7SEG_SYM_2           0b01011011
#define _7SEG_SYM_3           0b01001111
#define _7SEG_SYM_4           0b01100110
#define _7SEG_SYM_5           0b01101101
#define _7SEG_SYM_6           0b01111101
#define _7SEG_SYM_7           0b00000111
#define _7SEG_SYM_8           0b01111111
#define _7SEG_SYM_9           0b01101111

#define _7SEG_SYM_A           0b01110111
#define _7SEG_SYM_B           0b01111100
#define _7SEG_SYM_C           0b00111001
#define _7SEG_SYM_D           0b01011110
#define _7SEG_SYM_E           0b01111001
#define _7SEG_SYM_F           0b01110001
#define _7SEG_SYM_G           0b00111101
#define _7SEG_SYM_H           0b01110100
#define _7SEG_SYM_I           0b00110000
#define _7SEG_SYM_J           0b00011110
#define _7SEG_SYM_K           0b01110101
#define _7SEG_SYM_L           0b00111000
#define _7SEG_SYM_M           0b00010101
#define _7SEG_SYM_N           0b00110111
#define _7SEG_SYM_O           0b00111111
#define _7SEG_SYM_P           0b01110011
#define _7SEG_SYM_Q           0b01100111
#define _7SEG_SYM_R           0b00110011
#define _7SEG_SYM_S           0b01101101
#define _7SEG_SYM_T           0b01111000
#define _7SEG_SYM_U           0b00111110
#define _7SEG_SYM_V           0b00101110
#define _7SEG_SYM_W           0b00101010
#define _7SEG_SYM_X           0b01110110
#define _7SEG_SYM_Y           0b01101110
#define _7SEG_SYM_Z           0b01001011

#define _7SEG_SYM_DASH        0b01000000
#define _7SEG_SYM_UNDERSCORE  0b00001000

#define _7SEG_MASK(seg) (0b00000001 << seg)

#define _7SEG_IDX(seg, i) ((seg) * _ledsPerSegment + (i))

#define _7SEG_MODE(mode, state) (mode == LedBasedDisplayMode::SET_ALL_LEDS || (state && mode == LedBasedDisplayMode::SET_ON_LEDS) || (!state && mode == LedBasedDisplayMode::SET_OFF_LEDS))

#define _7SEG_UNDEF 255

typedef void (*LedBasedDisplayOutput)(uint16_t x, uint16_t y, uint16_t r, uint16_t g, uint16_t b);

enum LedBasedDisplayMode {

    SET_NO_LEDS,
    SET_OFF_LEDS,
    SET_ON_LEDS,
    SET_ALL_LEDS
};

class LedBasedDisplay {

    public:
        virtual ~LedBasedDisplay() {}

        virtual uint16_t rowCount() = 0;
        virtual uint16_t columnCount() = 0;

        virtual CRGB getBackgroundColor() = 0;
        virtual void setBackgroundColor(CRGB color) = 0;

        virtual CRGB* getLedColor(uint16_t row, uint16_t column, bool state) = 0;
        virtual void setLedColor(uint16_t row, uint16_t column, bool state, CRGB color) = 0;

        virtual void update(uint16_t rowOffset = 0, uint16_t columnOffset = 0, uint16_t width = 0, uint16_t height = 0) = 0;

        virtual void setPaintBackground(bool paint) = 0;
        virtual void setMode(LedBasedDisplayMode mode) = 0;

        void setRowColor(uint16_t row, bool state, CRGB color);
        void setColumnColor(uint16_t column, bool state, CRGB color);
        void setColor(bool state, CRGB color);
};

class SevenSegmentDisplay : public LedBasedDisplay {

    public:
        SevenSegmentDisplay(LedBasedDisplayOutput output, uint16_t ledsPerSegment);

        virtual ~SevenSegmentDisplay() override;

        virtual uint16_t rowCount() override;
        virtual uint16_t columnCount() override;

        virtual CRGB getBackgroundColor() override;
        virtual void setBackgroundColor(CRGB color) override;

        virtual CRGB* getLedColor(uint16_t row, uint16_t column, bool state) override;
        virtual void setLedColor(uint16_t row, uint16_t column, bool state, CRGB color) override;

        virtual void update(uint16_t rowOffset = 0, uint16_t columnOffset = 0, uint16_t width = 0, uint16_t height = 0) override;

        virtual void setPaintBackground(bool paint) override;
        virtual void setMode(LedBasedDisplayMode mode) override;

        uint16_t segmentOfCoords(uint16_t row, uint16_t column);

        void setSymbol(uint16_t symbol);
        void setDigit(uint16_t digit);
        void setCharacter(char charcter);
        void setShowZero(bool showZero);

    private:
        LedBasedDisplayOutput _output;
        CRGB _backgroundColor;
        CRGB* _offColors;
        CRGB* _onColors;

        uint16_t _ledsPerSegment;
        uint16_t _value;

        bool _paintBackground;
        bool _showZero;

        LedBasedDisplayMode _mode;

        uint16_t _internalIndex(uint16_t segment, uint16_t row, uint16_t column);
};

class SeparatorDisplay : public LedBasedDisplay {

    public:
        SeparatorDisplay(LedBasedDisplayOutput output, uint16_t ledCount);

        virtual ~SeparatorDisplay() override;

        virtual uint16_t rowCount() override;
        virtual uint16_t columnCount() override;

        virtual CRGB getBackgroundColor() override;
        virtual void setBackgroundColor(CRGB color) override;

        virtual CRGB* getLedColor(uint16_t row, uint16_t column, bool state) override;
        virtual void setLedColor(uint16_t row, uint16_t column, bool state, CRGB color) override;

        virtual void update(uint16_t rowOffset = 0, uint16_t columnOffset = 0, uint16_t width = 0, uint16_t height = 0) override;

        virtual void setPaintBackground(bool paint) override;
        virtual void setMode(LedBasedDisplayMode mode) override;

        void addLed(uint16_t row, uint16_t column);
        void setState(bool state);

    private:
        struct _Mapping {
            uint16_t row;
            uint16_t column;
            CRGB offColor;
            CRGB onColor;
        };

        LedBasedDisplayOutput _output;
        uint16_t _maxLeds;
        uint16_t _ledCount;
        struct _Mapping* _mappings;
        CRGB _backgroundColor;
        bool _paintBackground;
        bool _state;
        LedBasedDisplayMode _mode;
};

class LedBasedRowDisplay : public LedBasedDisplay {

    public:
        LedBasedRowDisplay(uint16_t displayCount, ...);

        virtual ~LedBasedRowDisplay() override;

        virtual uint16_t rowCount() override;
        virtual uint16_t columnCount() override;

        virtual CRGB getBackgroundColor() override;
        virtual void setBackgroundColor(CRGB color) override;

        virtual CRGB* getLedColor(uint16_t row, uint16_t column, bool state) override;
        virtual void setLedColor(uint16_t row, uint16_t column, bool state, CRGB color) override;

        virtual void update(uint16_t rowOffset = 0, uint16_t columnOffset = 0, uint16_t width = 0, uint16_t height = 0) override;

        virtual void setPaintBackground(bool paint) override;
        virtual void setMode(LedBasedDisplayMode mode) override;

    private:
        uint16_t _displayCount;
        LedBasedDisplay** _displays;
};

#endif