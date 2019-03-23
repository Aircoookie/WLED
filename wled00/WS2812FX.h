/*
  WS2812FX.h - Library for WS2812 LED effects.
  Harm Aldick - 2016
  www.aldick.org
  LICENSE
  The MIT License (MIT)
  Copyright (c) 2016  Harm Aldick
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.

  Modified for WLED
*/

#ifndef WS2812FX_h
#define WS2812FX_h

#include "NpbWrapper.h"

#define FSH(x) (__FlashStringHelper*)(x)

#define FASTLED_INTERNAL //remove annoying pragma messages
#include "FastLED.h"

#define DEFAULT_BRIGHTNESS (uint8_t)127
#define DEFAULT_MODE       (uint8_t)0
#define DEFAULT_SPEED      (uint8_t)128
#define DEFAULT_COLOR      (uint32_t)0xFFAA00

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

/* each segment uses 37 bytes of SRAM memory, so if you're application fails because of
  insufficient memory, decreasing MAX_NUM_SEGMENTS may help */
#define MAX_NUM_SEGMENTS 1
#define NUM_COLORS        3 /* number of colors per segment */
#define SEGMENT          _segments[_segment_index]
#define SEGMENT_RUNTIME  _segment_runtimes[_segment_index]
#define SEGMENT_LENGTH   (SEGMENT.stop - SEGMENT.start)
#define SPEED_FORMULA_L  5 + (50*(255 - SEGMENT.speed))/SEGMENT_LENGTH
#define RESET_RUNTIME    memset(_segment_runtimes, 0, sizeof(_segment_runtimes))

// some common colors
#define RED        (uint32_t)0xFF0000
#define GREEN      (uint32_t)0x00FF00
#define BLUE       (uint32_t)0x0000FF
#define WHITE      (uint32_t)0xFFFFFF
#define BLACK      (uint32_t)0x000000
#define YELLOW     (uint32_t)0xFFFF00
#define CYAN       (uint32_t)0x00FFFF
#define MAGENTA    (uint32_t)0xFF00FF
#define PURPLE     (uint32_t)0x400080
#define ORANGE     (uint32_t)0xFF3000
#define PINK       (uint32_t)0xFF1493
#define ULTRAWHITE (uint32_t)0xFFFFFFFF

// options
// bit    7: segment is in transition mode
// bits 2-6: TBD
// bit    1: reverse segment
// bit    0: segment is selected
#define NO_OPTIONS   (uint8_t)0x00
#define TRANSITIONAL (uint8_t)0x80
#define REVERSE      (uint8_t)0x02
#define SELECTED     (uint8_t)0x01
#define IS_TRANSITIONAL ((SEGMENT.options & TRANSITIONAL) == TRANSITIONAL)
#define IS_REVERSE      ((SEGMENT.options & REVERSE )     == REVERSE     )
#define IS_SELECTED     ((SEGMENT.options & SELECTED)     == SELECTED    )

#define MODE_COUNT (sizeof(_names)/sizeof(_names[0]))

#define FX_MODE_STATIC                   0
#define FX_MODE_BLINK                    1
#define FX_MODE_BREATH                   2
#define FX_MODE_COLOR_WIPE               3
#define FX_MODE_COLOR_WIPE_RANDOM        4
#define FX_MODE_RANDOM_COLOR             5
#define FX_MODE_COLOR_SWEEP              6
#define FX_MODE_DYNAMIC                  7
#define FX_MODE_RAINBOW                  8
#define FX_MODE_RAINBOW_CYCLE            9
#define FX_MODE_SCAN                    10
#define FX_MODE_DUAL_SCAN               11
#define FX_MODE_FADE                    12
#define FX_MODE_THEATER_CHASE           13
#define FX_MODE_THEATER_CHASE_RAINBOW   14
#define FX_MODE_RUNNING_LIGHTS          15
#define FX_MODE_SAW                     16
#define FX_MODE_TWINKLE                 17
#define FX_MODE_DISSOLVE                18
#define FX_MODE_DISSOLVE_RANDOM         19
#define FX_MODE_SPARKLE                 20
#define FX_MODE_FLASH_SPARKLE           21
#define FX_MODE_HYPER_SPARKLE           22
#define FX_MODE_STROBE                  23
#define FX_MODE_STROBE_RAINBOW          24
#define FX_MODE_MULTI_STROBE            25
#define FX_MODE_BLINK_RAINBOW           26
#define FX_MODE_ANDROID                 27
#define FX_MODE_CHASE_COLOR             28
#define FX_MODE_CHASE_RANDOM            29
#define FX_MODE_CHASE_RAINBOW           30
#define FX_MODE_CHASE_FLASH             31
#define FX_MODE_CHASE_FLASH_RANDOM      32
#define FX_MODE_CHASE_RAINBOW_WHITE     33
#define FX_MODE_COLORFUL                34
#define FX_MODE_TRAFFIC_LIGHT           35
#define FX_MODE_COLOR_SWEEP_RANDOM      36
#define FX_MODE_RUNNING_COLOR           37
#define FX_MODE_RUNNING_RED_BLUE        38
#define FX_MODE_RUNNING_RANDOM          39
#define FX_MODE_LARSON_SCANNER          40
#define FX_MODE_COMET                   41
#define FX_MODE_FIREWORKS               42
#define FX_MODE_RAIN                    43
#define FX_MODE_MERRY_CHRISTMAS         44
#define FX_MODE_FIRE_FLICKER            45
#define FX_MODE_GRADIENT                46
#define FX_MODE_LOADING                 47
#define FX_MODE_DUAL_COLOR_WIPE_IN_OUT  48
#define FX_MODE_DUAL_COLOR_WIPE_IN_IN   49
#define FX_MODE_DUAL_COLOR_WIPE_OUT_OUT 50
#define FX_MODE_DUAL_COLOR_WIPE_OUT_IN  51
#define FX_MODE_CIRCUS_COMBUSTUS        52
#define FX_MODE_HALLOWEEN               53
#define FX_MODE_TRICOLOR_CHASE          54
#define FX_MODE_TRICOLOR_WIPE           55
#define FX_MODE_TRICOLOR_FADE           56
#define FX_MODE_LIGHTNING               57
#define FX_MODE_ICU                     58
#define FX_MODE_MULTI_COMET             59
#define FX_MODE_DUAL_LARSON_SCANNER     60
#define FX_MODE_RANDOM_CHASE            61
#define FX_MODE_OSCILLATE               62
//Modes that use FastLED -->
#define FX_MODE_PRIDE_2015              63
#define FX_MODE_JUGGLE                  64
#define FX_MODE_PALETTE                 65
#define FX_MODE_FIRE_2012               66
#define FX_MODE_COLORWAVES              67
#define FX_MODE_BPM                     68
#define FX_MODE_FILLNOISE8              69
#define FX_MODE_NOISE16_1               70
#define FX_MODE_NOISE16_2               71
#define FX_MODE_NOISE16_3               72
#define FX_MODE_NOISE16_4               73
#define FX_MODE_COLORTWINKLE            74
#define FX_MODE_LAKE                    75
#define FX_MODE_METEOR                  76
#define FX_MODE_METEOR_SMOOTH           77
#define FX_MODE_RAILWAY                 78
#define FX_MODE_RIPPLE                  79

const char name_0[] PROGMEM = "Static";
const char name_1[] PROGMEM = "Blink";
const char name_2[] PROGMEM = "Breath";
const char name_3[] PROGMEM = "Color Wipe";
const char name_4[] PROGMEM = "Color Wipe Random";
const char name_5[] PROGMEM = "Random Color";
const char name_6[] PROGMEM = "Color Sweep";
const char name_7[] PROGMEM = "Dynamic";
const char name_8[] PROGMEM = "Rainbow";
const char name_9[] PROGMEM = "Rainbow Cycle";
const char name_10[] PROGMEM = "Scan";
const char name_11[] PROGMEM = "Dual Scan";
const char name_12[] PROGMEM = "Fade";
const char name_13[] PROGMEM = "Theater Chase";
const char name_14[] PROGMEM = "Theater Chase Rainbow";
const char name_15[] PROGMEM = "Running Lights";
const char name_16[] PROGMEM = "Saw";
const char name_17[] PROGMEM = "Twinkle";
const char name_18[] PROGMEM = "Dissolve";
const char name_19[] PROGMEM = "Dissolve Random";
const char name_20[] PROGMEM = "Sparkle";
const char name_21[] PROGMEM = "Flash Sparkle";
const char name_22[] PROGMEM = "Hyper Sparkle";
const char name_23[] PROGMEM = "Strobe";
const char name_24[] PROGMEM = "Strobe Rainbow";
const char name_25[] PROGMEM = "Multi Strobe";
const char name_26[] PROGMEM = "Blink Rainbow";
const char name_27[] PROGMEM = "Android";
const char name_28[] PROGMEM = "Chase Color";
const char name_29[] PROGMEM = "Chase Random";
const char name_30[] PROGMEM = "Chase Rainbow";
const char name_31[] PROGMEM = "Chase Flash";
const char name_32[] PROGMEM = "Chase Flash Random";
const char name_33[] PROGMEM = "Chase Rainbow White";
const char name_34[] PROGMEM = "Colorful";
const char name_35[] PROGMEM = "Traffic Light";
const char name_36[] PROGMEM = "Color Sweep Random";
const char name_37[] PROGMEM = "Running Color";
const char name_38[] PROGMEM = "Running Red Blue";
const char name_39[] PROGMEM = "Running Random";
const char name_40[] PROGMEM = "Larson Scanner";
const char name_41[] PROGMEM = "Comet";
const char name_42[] PROGMEM = "Fireworks";
const char name_43[] PROGMEM = "Rain";
const char name_44[] PROGMEM = "Merry Christmas";
const char name_45[] PROGMEM = "Fire Flicker";
const char name_46[] PROGMEM = "Gradient";
const char name_47[] PROGMEM = "Loading";
const char name_48[] PROGMEM = "Dual Color Wipe In Out";
const char name_49[] PROGMEM = "Dual Color Wipe In In";
const char name_50[] PROGMEM = "Dual Color Wipe Out Out";
const char name_51[] PROGMEM = "Dual Color Wipe Out In";
const char name_52[] PROGMEM = "Circus Combustus";
const char name_53[] PROGMEM = "Halloween";
const char name_54[] PROGMEM = "Tricolor Chase";
const char name_55[] PROGMEM = "Tricolor Wipe";
const char name_56[] PROGMEM = "Tricolor Fade";
const char name_57[] PROGMEM = "Lightning";
const char name_58[] PROGMEM = "ICU";
const char name_59[] PROGMEM = "Multi Comet";
const char name_60[] PROGMEM = "Dual Larson Scanner";
const char name_61[] PROGMEM = "Random Chase";
const char name_62[] PROGMEM = "Oscillate";
const char name_63[] PROGMEM = "Pride 2015";
const char name_64[] PROGMEM = "Juggle";
const char name_65[] PROGMEM = "Pallette";
const char name_66[] PROGMEM = "Fire 2012";
const char name_67[] PROGMEM = "Colorwaves";
const char name_68[] PROGMEM = "BPM";
const char name_69[] PROGMEM = "Fill Noise 8";
const char name_70[] PROGMEM = "Noise 16 1";
const char name_71[] PROGMEM = "Noise 16 2";
const char name_72[] PROGMEM = "Noise 16 3";
const char name_73[] PROGMEM = "Noise 16 4";
const char name_74[] PROGMEM = "Color Twinkle";
const char name_75[] PROGMEM = "Lake";
const char name_76[] PROGMEM = "Meteor";
const char name_77[] PROGMEM = "Meteor Smooth";
const char name_78[] PROGMEM = "Railway";
const char name_79[] PROGMEM = "Ripple";

static const __FlashStringHelper* _names[] = {
  FSH(name_0),
  FSH(name_1),
  FSH(name_2),
  FSH(name_3),
  FSH(name_4),
  FSH(name_5),
  FSH(name_6),
  FSH(name_7),
  FSH(name_8),
  FSH(name_9),
  FSH(name_10),
  FSH(name_11),
  FSH(name_12),
  FSH(name_13),
  FSH(name_14),
  FSH(name_15),
  FSH(name_16),
  FSH(name_17),
  FSH(name_18),
  FSH(name_19),
  FSH(name_20),
  FSH(name_21),
  FSH(name_22),
  FSH(name_23),
  FSH(name_24),
  FSH(name_25),
  FSH(name_26),
  FSH(name_27),
  FSH(name_28),
  FSH(name_29),
  FSH(name_30),
  FSH(name_31),
  FSH(name_32),
  FSH(name_33),
  FSH(name_34),
  FSH(name_35),
  FSH(name_36),
  FSH(name_37),
  FSH(name_38),
  FSH(name_39),
  FSH(name_40),
  FSH(name_41),
  FSH(name_42),
  FSH(name_43),
  FSH(name_44),
  FSH(name_45),
  FSH(name_46),
  FSH(name_47),
  FSH(name_48),
  FSH(name_49),
  FSH(name_50),
  FSH(name_51),
  FSH(name_52),
  FSH(name_53),
  FSH(name_54),
  FSH(name_55),
  FSH(name_56),
  FSH(name_57),
  FSH(name_58),
  FSH(name_59),
  FSH(name_60),
  FSH(name_61),
  FSH(name_62),
  FSH(name_63),
  FSH(name_64),
  FSH(name_65),
  FSH(name_66),
  FSH(name_67),
  FSH(name_68),
  FSH(name_69),
  FSH(name_70),
  FSH(name_71),
  FSH(name_72),
  FSH(name_73),
  FSH(name_74),
  FSH(name_75),
  FSH(name_76),
  FSH(name_77),
  FSH(name_78),
  FSH(name_79)
};

class WS2812FX {
  typedef uint16_t (WS2812FX::*mode_ptr)(void);
  
  // segment parameters
  public:
    typedef struct Segment { // 21 bytes
      uint16_t start;
      uint16_t stop; //segment invalid if stop == 0
      uint8_t speed;
      uint8_t intensity;
      uint8_t palette;
      uint8_t mode;
      uint8_t options; //bit pattern: msb first: transitional tbd tbd tbd tbd paused reverse selected
      uint32_t colors[NUM_COLORS];
      //member functions
      uint32_t color(uint8_t n)
      {
        return colors[n];
      }
      void setOption(uint8_t n, bool val)
      {
        if (val) {
          options |= 0x01 << n;
        } else
        {
          options &= ~(0x01 << n);
        }
      }
      bool getOption(uint8_t n)
      {
        return ((options >> n) & 0x01);
      }
    } segment;

  // segment runtime parameters
    typedef struct Segment_runtime { // 16 bytes
      unsigned long next_time;
      uint32_t counter_mode_step;
      uint32_t counter_mode_call;
      uint16_t aux_param;
      uint16_t aux_param2;
      void reset(){next_time = 0; counter_mode_step = 0; counter_mode_call = 0; aux_param = 0; aux_param2 = 0;};
    } segment_runtime;

    WS2812FX() {
      _mode[FX_MODE_STATIC]                  = &WS2812FX::mode_static;
      _mode[FX_MODE_BLINK]                   = &WS2812FX::mode_blink;
      _mode[FX_MODE_COLOR_WIPE]              = &WS2812FX::mode_color_wipe;
      _mode[FX_MODE_COLOR_WIPE_RANDOM]       = &WS2812FX::mode_color_wipe_random;
      _mode[FX_MODE_RANDOM_COLOR]            = &WS2812FX::mode_random_color;
      _mode[FX_MODE_COLOR_SWEEP]             = &WS2812FX::mode_color_sweep;
      _mode[FX_MODE_DYNAMIC]                 = &WS2812FX::mode_dynamic;
      _mode[FX_MODE_RAINBOW]                 = &WS2812FX::mode_rainbow;
      _mode[FX_MODE_RAINBOW_CYCLE]           = &WS2812FX::mode_rainbow_cycle;
      _mode[FX_MODE_SCAN]                    = &WS2812FX::mode_scan;
      _mode[FX_MODE_DUAL_SCAN]               = &WS2812FX::mode_dual_scan;
      _mode[FX_MODE_FADE]                    = &WS2812FX::mode_fade;
      _mode[FX_MODE_THEATER_CHASE]           = &WS2812FX::mode_theater_chase;
      _mode[FX_MODE_THEATER_CHASE_RAINBOW]   = &WS2812FX::mode_theater_chase_rainbow;
      _mode[FX_MODE_SAW]                     = &WS2812FX::mode_saw;
      _mode[FX_MODE_TWINKLE]                 = &WS2812FX::mode_twinkle;
      _mode[FX_MODE_DISSOLVE]                = &WS2812FX::mode_dissolve;
      _mode[FX_MODE_DISSOLVE_RANDOM]         = &WS2812FX::mode_dissolve_random;
      _mode[FX_MODE_SPARKLE]                 = &WS2812FX::mode_sparkle;
      _mode[FX_MODE_FLASH_SPARKLE]           = &WS2812FX::mode_flash_sparkle;
      _mode[FX_MODE_HYPER_SPARKLE]           = &WS2812FX::mode_hyper_sparkle;
      _mode[FX_MODE_STROBE]                  = &WS2812FX::mode_strobe;
      _mode[FX_MODE_STROBE_RAINBOW]          = &WS2812FX::mode_strobe_rainbow;
      _mode[FX_MODE_MULTI_STROBE]            = &WS2812FX::mode_multi_strobe;
      _mode[FX_MODE_BLINK_RAINBOW]           = &WS2812FX::mode_blink_rainbow;
      _mode[FX_MODE_ANDROID]                 = &WS2812FX::mode_android;
      _mode[FX_MODE_CHASE_COLOR]             = &WS2812FX::mode_chase_color;
      _mode[FX_MODE_CHASE_RANDOM]            = &WS2812FX::mode_chase_random;
      _mode[FX_MODE_CHASE_RAINBOW]           = &WS2812FX::mode_chase_rainbow;
      _mode[FX_MODE_CHASE_FLASH]             = &WS2812FX::mode_chase_flash;
      _mode[FX_MODE_CHASE_FLASH_RANDOM]      = &WS2812FX::mode_chase_flash_random;
      _mode[FX_MODE_CHASE_RAINBOW_WHITE]     = &WS2812FX::mode_chase_rainbow_white;
      _mode[FX_MODE_COLORFUL]                = &WS2812FX::mode_colorful;
      _mode[FX_MODE_TRAFFIC_LIGHT]           = &WS2812FX::mode_traffic_light;
      _mode[FX_MODE_COLOR_SWEEP_RANDOM]      = &WS2812FX::mode_color_sweep_random;
      _mode[FX_MODE_RUNNING_COLOR]           = &WS2812FX::mode_running_color;
      _mode[FX_MODE_RUNNING_RED_BLUE]        = &WS2812FX::mode_running_red_blue;
      _mode[FX_MODE_RUNNING_RANDOM]          = &WS2812FX::mode_running_random;
      _mode[FX_MODE_LARSON_SCANNER]          = &WS2812FX::mode_larson_scanner;
      _mode[FX_MODE_COMET]                   = &WS2812FX::mode_comet;
      _mode[FX_MODE_FIREWORKS]               = &WS2812FX::mode_fireworks;
      _mode[FX_MODE_RAIN]                    = &WS2812FX::mode_rain;
      _mode[FX_MODE_MERRY_CHRISTMAS]         = &WS2812FX::mode_merry_christmas;
      _mode[FX_MODE_FIRE_FLICKER]            = &WS2812FX::mode_fire_flicker;
      _mode[FX_MODE_GRADIENT]                = &WS2812FX::mode_gradient;
      _mode[FX_MODE_LOADING]                 = &WS2812FX::mode_loading;
      _mode[FX_MODE_DUAL_COLOR_WIPE_IN_OUT]  = &WS2812FX::mode_dual_color_wipe_in_out;
      _mode[FX_MODE_DUAL_COLOR_WIPE_IN_IN]   = &WS2812FX::mode_dual_color_wipe_in_in;
      _mode[FX_MODE_DUAL_COLOR_WIPE_OUT_OUT] = &WS2812FX::mode_dual_color_wipe_out_out;
      _mode[FX_MODE_DUAL_COLOR_WIPE_OUT_IN]  = &WS2812FX::mode_dual_color_wipe_out_in;
      _mode[FX_MODE_CIRCUS_COMBUSTUS]        = &WS2812FX::mode_circus_combustus;
      _mode[FX_MODE_HALLOWEEN]               = &WS2812FX::mode_halloween;
      _mode[FX_MODE_TRICOLOR_CHASE]          = &WS2812FX::mode_tricolor_chase;
      _mode[FX_MODE_TRICOLOR_WIPE]           = &WS2812FX::mode_tricolor_wipe;
      _mode[FX_MODE_TRICOLOR_FADE]           = &WS2812FX::mode_tricolor_fade;
      _mode[FX_MODE_BREATH]                  = &WS2812FX::mode_breath;
      _mode[FX_MODE_RUNNING_LIGHTS]          = &WS2812FX::mode_running_lights;
      _mode[FX_MODE_LIGHTNING]               = &WS2812FX::mode_lightning;
      _mode[FX_MODE_ICU]                     = &WS2812FX::mode_icu;
      _mode[FX_MODE_MULTI_COMET]             = &WS2812FX::mode_multi_comet;
      _mode[FX_MODE_DUAL_LARSON_SCANNER]     = &WS2812FX::mode_dual_larson_scanner;
      _mode[FX_MODE_RANDOM_CHASE]            = &WS2812FX::mode_random_chase;
      _mode[FX_MODE_OSCILLATE]               = &WS2812FX::mode_oscillate;
      _mode[FX_MODE_FIRE_2012]               = &WS2812FX::mode_fire_2012;
      _mode[FX_MODE_PRIDE_2015]              = &WS2812FX::mode_pride_2015;
      _mode[FX_MODE_BPM]                     = &WS2812FX::mode_bpm;
      _mode[FX_MODE_JUGGLE]                  = &WS2812FX::mode_juggle;
      _mode[FX_MODE_PALETTE]                 = &WS2812FX::mode_palette;
      _mode[FX_MODE_COLORWAVES]              = &WS2812FX::mode_colorwaves;
      _mode[FX_MODE_FILLNOISE8]              = &WS2812FX::mode_fillnoise8;
      _mode[FX_MODE_NOISE16_1]               = &WS2812FX::mode_noise16_1;
      _mode[FX_MODE_NOISE16_2]               = &WS2812FX::mode_noise16_2;
      _mode[FX_MODE_NOISE16_3]               = &WS2812FX::mode_noise16_3;
      _mode[FX_MODE_NOISE16_4]               = &WS2812FX::mode_noise16_4;
      _mode[FX_MODE_COLORTWINKLE]            = &WS2812FX::mode_colortwinkle;
      _mode[FX_MODE_LAKE]                    = &WS2812FX::mode_lake;
      _mode[FX_MODE_METEOR]                  = &WS2812FX::mode_meteor;
      _mode[FX_MODE_METEOR_SMOOTH]           = &WS2812FX::mode_meteor_smooth;
      _mode[FX_MODE_RAILWAY]                 = &WS2812FX::mode_railway;
      _mode[FX_MODE_RIPPLE]                  = &WS2812FX::mode_ripple;

      _brightness = DEFAULT_BRIGHTNESS;
      _num_segments = 1;
      _segments[0].mode = DEFAULT_MODE;
      _segments[0].colors[0] = DEFAULT_COLOR;
      _segments[0].start = 0;
      _segments[0].speed = DEFAULT_SPEED;
      currentPalette = CRGBPalette16(CRGB::Black);
      targetPalette = CloudColors_p;
      _reverseMode = false;
      _skipFirstMode = false;
      colorOrder = 0;
      paletteFade = 0;
      paletteBlend = 0;
      ablMilliampsMax = 850;
      currentMilliamps = 0;
      _locked = nullptr;
      _modeUsesLock = false;
      bus = new NeoPixelWrapper();
      RESET_RUNTIME;
    }

    void
      init(bool supportWhite, uint16_t countPixels, bool skipFirst),
      service(void),
      blur(uint8_t),
      fade_out(uint8_t r),
      setMode(uint8_t m),
      setSpeed(uint8_t s),
      setIntensity(uint8_t i),
      setPalette(uint8_t p),
      setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0),
      setSecondaryColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0),
      setColor(uint32_t c),
      setSecondaryColor(uint32_t c),
      setBrightness(uint8_t b),
      setReverseMode(bool b),
      driverModeCronixie(bool b),
      setCronixieDigits(byte* d),
      setCronixieBacklight(bool b),
      setIndividual(uint16_t i, uint32_t col),
      setRange(uint16_t i, uint16_t i2, uint32_t col),
      lock(uint16_t i),
      lockRange(uint16_t i, uint16_t i2),
      unlock(uint16_t i),
      unlockRange(uint16_t i, uint16_t i2),
      unlockAll(void),
      setTransitionMode(bool t),
      trigger(void),
      setSegment(uint8_t n, uint16_t start, uint16_t stop),
      resetSegments(),
      setPixelColor(uint16_t n, uint32_t c),
      setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0),
      show(void);

    bool
      setEffectConfig(uint8_t m, uint8_t s, uint8_t i, uint8_t p);

    uint8_t
      paletteFade,
      paletteBlend,
      colorOrder,
      getBrightness(void),
      getMode(void),
      getSpeed(void),
      getNumSegments(void),
      getModeCount(void),
      getPaletteCount(void),
      getMaxSegments(void),
      get_random_wheel_index(uint8_t);

    uint32_t
      color_wheel(uint8_t),
      color_from_palette(uint16_t, bool, bool, uint8_t, uint8_t pbri = 255),
      color_blend(uint32_t,uint32_t,uint8_t),
      getPixelColor(uint16_t),
      getColor(void);

    WS2812FX::Segment&
      getSegment(uint8_t n);

    WS2812FX::Segment_runtime
      getSegmentRuntime(void);

    WS2812FX::Segment*
      getSegments(void);

    // mode helper functions
    uint16_t
      ablMilliampsMax,
      currentMilliamps,
      blink(uint32_t, uint32_t, bool strobe, bool),
      color_wipe(uint32_t, uint32_t, bool , bool),
      scan(bool),
      theater_chase(uint32_t, uint32_t, bool),
      running_base(bool),
      dissolve(uint32_t),
      chase(uint32_t, uint32_t, uint32_t, bool),
      gradient_base(bool),
      running(uint32_t, uint32_t),
      tricolor_chase(uint32_t, uint32_t);

    // builtin modes
    uint16_t
      mode_static(void),
      mode_blink(void),
      mode_blink_rainbow(void),
      mode_strobe(void),
      mode_strobe_rainbow(void),
      mode_color_wipe(void),
      mode_color_sweep(void),
      mode_color_wipe_random(void),
      mode_color_sweep_random(void),
      mode_random_color(void),
      mode_dynamic(void),
      mode_breath(void),
      mode_fade(void),
      mode_scan(void),
      mode_dual_scan(void),
      mode_theater_chase(void),
      mode_theater_chase_rainbow(void),
      mode_rainbow(void),
      mode_rainbow_cycle(void),
      mode_running_lights(void),
      mode_saw(void),
      mode_twinkle(void),
      mode_dissolve(void),
      mode_dissolve_random(void),
      mode_sparkle(void),
      mode_flash_sparkle(void),
      mode_hyper_sparkle(void),
      mode_multi_strobe(void),
      mode_android(void),
      mode_chase_color(void),
      mode_chase_random(void),
      mode_chase_rainbow(void),
      mode_chase_flash(void),
      mode_chase_flash_random(void),
      mode_chase_rainbow_white(void),
      mode_colorful(void),
      mode_traffic_light(void),
      mode_running_color(void),
      mode_running_red_blue(void),
      mode_running_random(void),
      mode_larson_scanner(void),
      mode_comet(void),
      mode_fireworks(void),
      mode_rain(void),
      mode_merry_christmas(void),
      mode_halloween(void),
      mode_fire_flicker(void),
      mode_gradient(void),
      mode_loading(void),
      mode_dual_color_wipe_in_out(void),
      mode_dual_color_wipe_in_in(void),
      mode_dual_color_wipe_out_out(void),
      mode_dual_color_wipe_out_in(void),
      mode_circus_combustus(void),
      mode_bicolor_chase(void),
      mode_tricolor_chase(void),
      mode_tricolor_wipe(void),
      mode_tricolor_fade(void),
      mode_lightning(void),
      mode_icu(void),
      mode_multi_comet(void),
      mode_dual_larson_scanner(void),
      mode_random_chase(void),
      mode_oscillate(void),
      mode_fire_2012(void),
      mode_pride_2015(void),
      mode_bpm(void),
      mode_juggle(void),
      mode_palette(void),
      mode_colorwaves(void),
      mode_fillnoise8(void),
      mode_noise16_1(void),
      mode_noise16_2(void),
      mode_noise16_3(void),
      mode_noise16_4(void),
      mode_colortwinkle(void),
      mode_lake(void),
      mode_meteor(void),
      mode_meteor_smooth(void),
      mode_railway(void),
      mode_ripple(void);

  const __FlashStringHelper* getModeName(uint8_t m);

  private:
    NeoPixelWrapper *bus;

    CRGB fastled_from_col(uint32_t);
    CRGBPalette16 currentPalette;
    CRGBPalette16 targetPalette;
  
    uint16_t _length;
    uint16_t _rand16seed;
    uint8_t _brightness;

    void handle_palette(void);
    void fill(uint32_t);
    bool modeUsesLock(uint8_t);

    bool
      _modeUsesLock,
      _rgbwMode,
      _reverseMode,
      _cronixieMode,
      _cronixieBacklightEnabled,
      _skipFirstMode,
      _triggered;

    byte* _locked;
    byte _cronixieDigits[6];

    mode_ptr _mode[MODE_COUNT]; // SRAM footprint: 4 bytes per element

    
    uint32_t _lastPaletteChange = 0;
    uint32_t _lastShow = 0;
    
    uint8_t _segment_index = 0;
    uint8_t _segment_index_palette_last = 99;
    uint8_t _num_segments = 1;
    segment _segments[MAX_NUM_SEGMENTS] = { // SRAM footprint: 21 bytes per element
      // start, stop, speed, intensity, palette, mode, options, color[]
      { 0, 7, DEFAULT_SPEED, 128, 0, DEFAULT_MODE, NO_OPTIONS, {DEFAULT_COLOR}}
    };
    segment_runtime _segment_runtimes[MAX_NUM_SEGMENTS]; // SRAM footprint: 16 bytes per element
};


//10 names per line
const char JSON_mode_names[] PROGMEM = R"=====([
"Solid","Blink","Breathe","Wipe","Wipe Random","Random Colors","Sweep","Dynamic","Colorloop","Rainbow",
"Scan","Dual Scan","Fade","Chase","Chase Rainbow","Running","Saw","Twinkle","Dissolve","Dissolve Rnd",
"Sparkle","Dark Sparkle","Sparkle+","Strobe","Strobe Rainbow","Mega Strobe","Blink Rainbow","Android","Chase","Chase Random",
"Chase Rainbow","Chase Flash","Chase Flash Rnd","Rainbow Runner","Colorful","Traffic Light","Sweep Random","Running 2","Red & Blue","Stream",
"Scanner","Lighthouse","Fireworks","Rain","Merry Christmas","Fire Flicker","Gradient","Loading","In Out","In In",
"Out Out","Out In","Circus","Halloween","Tri Chase","Tri Wipe","Tri Fade","Lightning","ICU","Multi Comet",
"Dual Scanner","Stream 2","Oscillate","Pride 2015","Juggle","Palette","Fire 2012","Colorwaves","BPM","Fill Noise","Noise 1",
"Noise 2","Noise 3","Noise 4","Colortwinkle","Lake","Meteor","Smooth Meteor","Railway","Ripple"
])=====";


const char JSON_palette_names[] PROGMEM = R"=====([
"Default","Random Cycle","Primary Color","Based on Primary","Set Colors","Based on Set","Party","Cloud","Lava","Ocean",
"Forest","Rainbow","Rainbow Bands","Sunset","Rivendell","Breeze","Red & Blue","Yellowout","Analogous","Splash",
"Pastel","Sunset 2","Beech","Vintage","Departure","Landscape","Beach","Sherbet","Hult","Hult 64",
"Drywet","Jul","Grintage","Rewhi","Tertiary","Fire","Icefire","Cyane","Light Pink","Autumn",
"Magenta","Magred","Yelmag","Yelblu","Orange & Teal","Tiamat","April Night"
])=====";

#endif
