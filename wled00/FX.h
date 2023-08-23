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

#include "const.h"

#define FASTLED_INTERNAL //remove annoying pragma messages
#define USE_GET_MILLISECOND_TIMER
#include "FastLED.h"

#define DEFAULT_BRIGHTNESS (uint8_t)127
#define DEFAULT_MODE       (uint8_t)0
#define DEFAULT_SPEED      (uint8_t)128
#define DEFAULT_INTENSITY  (uint8_t)128
#define DEFAULT_COLOR      (uint32_t)0xFFAA00

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

//color mangling macros
#ifndef RGBW32
#define RGBW32(r,g,b,w) (uint32_t((byte(w) << 24) | (byte(r) << 16) | (byte(g) << 8) | (byte(b))))
#endif

/* Not used in all effects yet */
#define WLED_FPS         42
#define FRAMETIME_FIXED  (1000/WLED_FPS)
#define FRAMETIME        _frametime

/* each segment uses 52 bytes of SRAM memory, so if you're application fails because of
  insufficient memory, decreasing MAX_NUM_SEGMENTS may help */
#ifdef ESP8266
  #define MAX_NUM_SEGMENTS    16
  /* How many color transitions can run at once */
  #define MAX_NUM_TRANSITIONS  8
  /* How much data bytes all segments combined may allocate */
  #define MAX_SEGMENT_DATA  4096
#else
  #ifndef MAX_NUM_SEGMENTS
    #define MAX_NUM_SEGMENTS  32
  #endif
  #define MAX_NUM_TRANSITIONS 24
  #define MAX_SEGMENT_DATA  20480
#endif

/* How much data bytes each segment should max allocate to leave enough space for other segments,
  assuming each segment uses the same amount of data. 256 for ESP8266, 640 for ESP32. */
#define FAIR_DATA_PER_SEG (MAX_SEGMENT_DATA / MAX_NUM_SEGMENTS)

// NEED WORKAROUND TO ACCESS PRIVATE CLASS VARIABLE '_frametime'
#define MIN_SHOW_DELAY   (_frametime < 16 ? 8 : 15)

#define NUM_COLORS       3 /* number of colors per segment */
#define SEGMENT          _segments[_segment_index]
#define SEGCOLOR(x)      _colors_t[x]
#define SEGENV           _segment_runtimes[_segment_index]
#define SEGLEN           _virtualSegmentLength
#define SEGACT           SEGMENT.stop
#define SPEED_FORMULA_L  5U + (50U*(255U - SEGMENT.speed))/SEGLEN

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
// bits 4-6: See WLEDSR below
// bit    3: mirror effect within segment
// bit    2: segment is on
// bit    1: reverse segment
// bit    0: segment is selected
#define NO_OPTIONS   (uint8_t)0x00
#define TRANSITIONAL (uint8_t)0x80
#define MIRROR       (uint8_t)0x08
#define SEGMENT_ON   (uint8_t)0x04
#define REVERSE      (uint8_t)0x02
#define SELECTED     (uint8_t)0x01
#define IS_TRANSITIONAL ((SEGMENT.options & TRANSITIONAL) == TRANSITIONAL)
#define IS_MIRROR       ((SEGMENT.options & MIRROR      ) == MIRROR      )
#define IS_SEGMENT_ON   ((SEGMENT.options & SEGMENT_ON  ) == SEGMENT_ON  )
#define IS_REVERSE      ((SEGMENT.options & REVERSE     ) == REVERSE     )
#define IS_SELECTED     ((SEGMENT.options & SELECTED    ) == SELECTED    )

#define MODE_COUNT 191// WLEDSR: First 128 for AC (incl reserved), rest for SR

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
#define FX_MODE_AURORA                  38
#define FX_MODE_RUNNING_RANDOM          39
#define FX_MODE_LARSON_SCANNER          40
#define FX_MODE_COMET                   41
#define FX_MODE_FIREWORKS               42
#define FX_MODE_RAIN                    43
#define FX_MODE_TETRIX                  44  //was Merry Christmas prior to 0.12.0 (use "Chase 2" with Red/Green)
#define FX_MODE_FIRE_FLICKER            45
#define FX_MODE_GRADIENT                46
#define FX_MODE_LOADING                 47
#define FX_MODE_POLICE                  48  // candidate for removal (after below three)
#define FX_MODE_FAIRY                   49  //was Police All prior to 0.13.0-b6 (use "Two Dots" with Red/Blue and full intensity)
#define FX_MODE_TWO_DOTS                50
#define FX_MODE_FAIRYTWINKLE            51  //was Two Areas prior to 0.13.0-b6 (use "Two Dots" with full intensity)
#define FX_MODE_RUNNING_DUAL            52
#define FX_MODE_HALLOWEEN               53  // candidate for removal
#define FX_MODE_TRICOLOR_CHASE          54
#define FX_MODE_TRICOLOR_WIPE           55
#define FX_MODE_TRICOLOR_FADE           56
#define FX_MODE_LIGHTNING               57
#define FX_MODE_ICU                     58
#define FX_MODE_MULTI_COMET             59
#define FX_MODE_DUAL_LARSON_SCANNER     60
#define FX_MODE_RANDOM_CHASE            61
#define FX_MODE_OSCILLATE               62
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
#define FX_MODE_TWINKLEFOX              80
#define FX_MODE_TWINKLECAT              81
#define FX_MODE_HALLOWEEN_EYES          82
#define FX_MODE_STATIC_PATTERN          83
#define FX_MODE_TRI_STATIC_PATTERN      84
#define FX_MODE_SPOTS                   85
#define FX_MODE_SPOTS_FADE              86
#define FX_MODE_GLITTER                 87
#define FX_MODE_CANDLE                  88
#define FX_MODE_STARBURST               89
#define FX_MODE_EXPLODING_FIREWORKS     90
#define FX_MODE_BOUNCINGBALLS           91
#define FX_MODE_SINELON                 92
#define FX_MODE_SINELON_DUAL            93
#define FX_MODE_SINELON_RAINBOW         94
#define FX_MODE_POPCORN                 95
#define FX_MODE_DRIP                    96
#define FX_MODE_PLASMA                  97
#define FX_MODE_PERCENT                 98
#define FX_MODE_RIPPLE_RAINBOW          99
#define FX_MODE_HEARTBEAT              100
#define FX_MODE_PACIFICA               101
#define FX_MODE_CANDLE_MULTI           102
#define FX_MODE_SOLID_GLITTER          103
#define FX_MODE_SUNRISE                104
#define FX_MODE_PHASED                 105
#define FX_MODE_TWINKLEUP              106
#define FX_MODE_NOISEPAL               107
#define FX_MODE_SINEWAVE               108
#define FX_MODE_PHASEDNOISE            109
#define FX_MODE_FLOW                   110
#define FX_MODE_CHUNCHUN               111
#define FX_MODE_DANCING_SHADOWS        112
#define FX_MODE_WASHING_MACHINE        113
#define FX_MODE_CANDY_CANE             114  // candidate for removal
#define FX_MODE_BLENDS                 115
#define FX_MODE_TV_SIMULATOR           116
#define FX_MODE_DYNAMIC_SMOOTH         117

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//    Start of Audio Reactive fork (WLEDSR)                                                                                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DEFAULT_Custom1       (uint8_t)128
#define DEFAULT_Custom2       (uint8_t)128
#define DEFAULT_Custom3       (uint8_t)128

// bits 4-6: WLEDSR: used for rotation and reverse
#define ROTATED2D     (uint8_t)0x10 //0x01, 0x02, 0x04, 0x08, 0x0F, 0x10, 0x20, 0x40, 0x80, 0xF0
#define REVERSE2D     (uint8_t)0x40 //0x01, 0x02, 0x04, 0x08, 0x0F, 0x10, 0x20, 0x40, 0x80, 0xF0
#define IS_REVERSE2D     ((SEGMENT.options & REVERSE2D    ) == REVERSE2D    )
#define IS_ROTATED2D      ((SEGMENT.options & ROTATED2D     ) == ROTATED2D     )

//these id's should never change as they are saved in api commands of presets
//currently 118 to 127 is reserved for future AC effects (use Reserved keyword in Json)
//they must match the order of the effects in JSON_mode_names (if unused, use Reserved keyword in Json)
#define FX_MODE_PIXELS                 128
#define FX_MODE_PIXELWAVE              129
#define FX_MODE_JUGGLES                130
#define FX_MODE_MATRIPIX               131
#define FX_MODE_GRAVIMETER             132
#define FX_MODE_PLASMOID               133
#define FX_MODE_PUDDLES                134
#define FX_MODE_MIDNOISE               135
#define FX_MODE_NOISEMETER             136
#define FX_MODE_FREQWAVE               137
#define FX_MODE_FREQMATRIX             138
#define FX_MODE_2DGEQ                  139
#define FX_MODE_WATERFALL              140
#define FX_MODE_FREQPIXELS             141
#define FX_MODE_BINMAP                 142
#define FX_MODE_NOISEFIRE              143
#define FX_MODE_PUDDLEPEAK             144
#define FX_MODE_NOISEMOVE              145
#define FX_MODE_2DNOISE                146
#define FX_MODE_PERLINMOVE             147
#define FX_MODE_RIPPLEPEAK             148
#define FX_MODE_2DFIRENOISE            149
#define FX_MODE_2DSQUAREDSWIRL         150
#define FX_MODE_2DFIRE2012             151
#define FX_MODE_2DDNA                  152
#define FX_MODE_2DMATRIX               153
#define FX_MODE_2DMETABALLS            154
#define FX_MODE_FREQMAP                155
#define FX_MODE_GRAVCENTER             156
#define FX_MODE_GRAVCENTRIC            157
#define FX_MODE_GRAVFREQ               158
#define FX_MODE_DJLIGHT                159
#define FX_MODE_2DFUNKYPLANK           160
#define FX_MODE_2DCENTERBARS           161
#define FX_MODE_2DPULSER               162
#define FX_MODE_BLURZ                  163
#define FX_MODE_2DDRIFT                 164
#define FX_MODE_2DWAVERLY               165
#define FX_MODE_2DSUNRADIATION          166
#define FX_MODE_2DCOLOREDBURSTS         167
#define FX_MODE_2DJULIA                 168
#define FX_MODE_2DPOOLNOISE            169 //reserved in JSON_mode_names
#define FX_MODE_2DTWISTER              170 //reserved in JSON_mode_names
#define FX_MODE_2DCAELEMENTATY         171 //reserved in JSON_mode_names
#define FX_MODE_2DGAMEOFLIFE           172
#define FX_MODE_2DTARTAN               173
#define FX_MODE_2DPOLARLIGHTS          174
#define FX_MODE_2DSWIRL                175
#define FX_MODE_2DLISSAJOUS            176
#define FX_MODE_2DFRIZZLES             177
#define FX_MODE_2DPLASMABALL           178
#define FX_MODE_FLOWSTRIPE             179
#define FX_MODE_2DHIPHOTIC             180
#define FX_MODE_2DSINDOTS              181
#define FX_MODE_2DDNASPIRAL            182
#define FX_MODE_2DBLACKHOLE            183
#define FX_MODE_WAVESINS               184
#define FX_MODE_ROCKTAVES              185
#define FX_MODE_2DAKEMI                186
#define FX_MODE_CUSTOMEFFECT           187 //WLEDSR Custom Effects
#define FX_MODE_3DRIPPLES              188
#define FX_MODE_3DSphereMove           189

#define FX_MODE_JBL                    190

#define floatNull -32768 //WLEDSR Custom Effects
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//    End of Audio Reactive fork (WLEDSR)                                                                                                //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class WS2812FX {
  typedef uint16_t (WS2812FX::*mode_ptr)(void);

  // pre show callback
  typedef void (*show_callback) (void);

  static WS2812FX* instance;

  // segment parameters
  public:

    // FastLED array, so we can refer to leds[i] instead of getPixel() and setPixel()
    CRGB leds[MAX_LEDS+1];                      // See const.h for a value of 1500. The plus 1 is just in case we go over with XY().

    typedef struct Segment { // 31 (32 in memory) bytes
      uint16_t start;
      uint16_t stop;    //segment invalid if stop == 0
      uint16_t offset;
      uint8_t speed;
      uint8_t intensity;
      uint8_t custom1;  // WLEDSR
      uint8_t custom2;  // WLEDSR
      uint8_t custom3;  // WLEDSR
      uint8_t palette;
      uint8_t mode;
      uint8_t options;  //bit pattern: msb first: transitional needspixelstate tbd tbd (paused) on reverse selected
      uint8_t grouping, spacing;
      uint8_t opacity;
      uint32_t colors[NUM_COLORS];
      uint8_t  cct;     //0==1900K, 255==10091K
      uint8_t  _capabilities;
      uint16_t width;   // WLEDSR ewowi20210624: add width/height and startX/Y stopX/Y for 2D segments
      uint16_t height;  // WLEDSR
      uint16_t startX;  // WLEDSR
      uint16_t startY;  // WLEDSR
      uint16_t stopX;   // WLEDSR
      uint16_t stopY;   // WLEDSR

      char *name;
      bool setColor(uint8_t slot, uint32_t c, uint8_t segn) { //returns true if changed
        if (slot >= NUM_COLORS || segn >= MAX_NUM_SEGMENTS) return false;
        if (c == colors[slot]) return false;
        uint8_t b = (slot == 1) ? cct : opacity;
        ColorTransition::startTransition(b, colors[slot], instance->_transitionDur, segn, slot);
        colors[slot] = c; return true;
      }
      void setCCT(uint16_t k, uint8_t segn) {
        if (segn >= MAX_NUM_SEGMENTS) return;
        if (k > 255) { //kelvin value, convert to 0-255
          if (k < 1900)  k = 1900;
          if (k > 10091) k = 10091;
          k = (k - 1900) >> 5;
        }
        if (cct == k) return;
        ColorTransition::startTransition(cct, colors[1], instance->_transitionDur, segn, 1);
        cct = k;
      }
      void setOpacity(uint8_t o, uint8_t segn) {
        if (segn >= MAX_NUM_SEGMENTS) return;
        if (opacity == o) return;
        ColorTransition::startTransition(opacity, colors[0], instance->_transitionDur, segn, 0);
        opacity = o;
      }
      void setOption(uint8_t n, bool val, uint8_t segn = 255)
      {
        bool prevOn = false;
        if (n == SEG_OPTION_ON) {
          prevOn = getOption(SEG_OPTION_ON);
          if (!val && prevOn) { //fade off
            ColorTransition::startTransition(opacity, colors[0], instance->_transitionDur, segn, 0);
          }
        }

        if (val) {
          options |= 0x01 << n;
        } else
        {
          options &= ~(0x01 << n);
        }

        if (n == SEG_OPTION_ON && val && !prevOn) { //fade on
          ColorTransition::startTransition(0, colors[0], instance->_transitionDur, segn, 0);
        }
      }
      bool getOption(uint8_t n)
      {
        return ((options >> n) & 0x01);
      }
      inline bool isSelected()
      {
        return getOption(0);
      }
      inline bool isActive()
      {
        return stop > start;
      }
      inline uint16_t length()
      {
        return (stopX - startX + 1) * (stopY - startY + 1); //WLEDSR: calculate length using SEGMENT x/y. Used by SEGLEN
      }
      inline uint16_t groupLength()
      {
        return grouping + spacing;
      }
      uint16_t virtualLength()
      {
        uint16_t groupLen = groupLength();
        uint16_t vLength = (length() + groupLen - 1) / groupLen;
        if (options & MIRROR)
          vLength = (vLength + 1) /2;  // divide by 2 if mirror, leave at least a single LED
        return vLength;
      }
      uint8_t differs(Segment& b);
      inline uint8_t getLightCapabilities() {return _capabilities;}
      void refreshLightCapabilities();
    } segment;

  // segment runtime parameters
    typedef struct Segment_runtime { // 28 bytes
      unsigned long next_time;  // millis() of next update
      uint32_t step;  // custom "step" var
      uint32_t call;  // call counter
      uint16_t aux0;  // custom var
      uint16_t aux1;  // custom var
      byte* data = nullptr;
      bool allocateData(uint16_t len){
        if (data && _dataLen == len) return true; //already allocated
        deallocateData();
        if (WS2812FX::instance->_usedSegmentData + len > MAX_SEGMENT_DATA) return false; //not enough memory
        // if possible use SPI RAM on ESP32
        #if defined(ARDUINO_ARCH_ESP32) && defined(WLED_USE_PSRAM)
        if (psramFound())
          data = (byte*) ps_malloc(len);
        else
        #endif
          data = (byte*) malloc(len);
        if (!data) return false; //allocation failed
        WS2812FX::instance->_usedSegmentData += len;
        _dataLen = len;
        memset(data, 0, len);
        return true;
      }
      void deallocateData(){
        free(data);
        data = nullptr;
        WS2812FX::instance->_usedSegmentData -= _dataLen;
        _dataLen = 0;
      }

      /**
       * If reset of this segment was request, clears runtime
       * settings of this segment.
       * Must not be called while an effect mode function is running
       * because it could access the data buffer and this method
       * may free that data buffer.
       */
      void resetIfRequired() {
        if (_requiresReset) {
          next_time = 0; step = 0; call = 0; aux0 = 0; aux1 = 0;
          deallocateData();
          _requiresReset = false;
        }
      }

      /**
       * Flags that before the next effect is calculated,
       * the internal segment state should be reset.
       * Call resetIfRequired before calling the next effect function.
       * Safe to call from interrupts and network requests.
       */
      inline void markForReset() { _requiresReset = true; }
      private:
        uint16_t _dataLen = 0;
        bool _requiresReset = false;
    } segment_runtime;

    typedef struct ColorTransition { // 12 bytes
      uint32_t colorOld = 0;
      uint32_t transitionStart;
      uint16_t transitionDur;
      uint8_t segment = 0xFF; //lower 6 bits: the segment this transition is for (255 indicates transition not in use/available) upper 2 bits: color channel
      uint8_t briOld = 0;
      static void startTransition(uint8_t oldBri, uint32_t oldCol, uint16_t dur, uint8_t segn, uint8_t slot) {
        if (segn >= MAX_NUM_SEGMENTS || slot >= NUM_COLORS || dur == 0) return;
        if (instance->_brightness == 0) return; //do not need transitions if master bri is off
        if (!instance->_segments[segn].getOption(SEG_OPTION_ON)) return; //not if segment is off either
        uint8_t tIndex = 0xFF; //none found
        uint16_t tProgression = 0;
        uint8_t s = segn + (slot << 6); //merge slot and segment into one byte

        for (uint8_t i = 0; i < MAX_NUM_TRANSITIONS; i++) {
          uint8_t tSeg = instance->transitions[i].segment;
          //see if this segment + color already has a running transition
          if (tSeg == s) {
            tIndex = i; break;
          }
          if (tSeg == 0xFF) { //free transition
            tIndex = i; tProgression = 0xFFFF;
          }
        }

        if (tIndex == 0xFF) { //no slot found yet
          for (uint8_t i = 0; i < MAX_NUM_TRANSITIONS; i++) {
            //find most progressed transition to overwrite
            uint16_t prog = instance->transitions[i].progress();
            if (prog > tProgression) {
              tIndex = i; tProgression = prog;
            }
          }
        }

        ColorTransition& t = instance->transitions[tIndex];
        if (t.segment == s) //this is an active transition on the same segment+color
        {
          bool wasTurningOff = (oldBri == 0);
          t.briOld = t.currentBri(wasTurningOff, slot);
          t.colorOld = t.currentColor(oldCol);
        } else {
          t.briOld = oldBri;
          t.colorOld = oldCol;
          uint8_t prevSeg = t.segment & 0x3F;
          if (prevSeg < MAX_NUM_SEGMENTS) instance->_segments[prevSeg].setOption(SEG_OPTION_TRANSITIONAL, false);
        }
        t.transitionDur = dur;
        t.transitionStart = millis();
        t.segment = s;
        instance->_segments[segn].setOption(SEG_OPTION_TRANSITIONAL, true);
        //refresh immediately, required for Solid mode
        if (instance->_segment_runtimes[segn].next_time > t.transitionStart + 22) instance->_segment_runtimes[segn].next_time = t.transitionStart;
      }
      uint16_t progress(bool allowEnd = false) { //transition progression between 0-65535
        uint32_t timeNow = millis();
        if (timeNow - transitionStart > transitionDur) {
          if (allowEnd) {
            uint8_t segn = segment & 0x3F;
            if (segn < MAX_NUM_SEGMENTS) instance->_segments[segn].setOption(SEG_OPTION_TRANSITIONAL, false);
            segment = 0xFF;
          }
          return 0xFFFF;
        }
        uint32_t elapsed = timeNow - transitionStart;
        uint32_t prog = elapsed * 0xFFFF / transitionDur;
        return (prog > 0xFFFF) ? 0xFFFF : prog;
      }
      uint32_t currentColor(uint32_t colorNew) {
        return instance->color_blend(colorOld, colorNew, progress(true), true);
      }
      uint8_t currentBri(bool turningOff = false, uint8_t slot = 0) {
        uint8_t segn = segment & 0x3F;
        if (segn >= MAX_NUM_SEGMENTS) return 0;
        uint8_t briNew = instance->_segments[segn].opacity;
        if (slot == 0) {
          if (!instance->_segments[segn].getOption(SEG_OPTION_ON) || turningOff) briNew = 0;
        } else { //transition slot 1 brightness for CCT transition
          briNew = instance->_segments[segn].cct;
        }
        uint32_t prog = progress() + 1;
        return ((briNew * prog) + (briOld * (0x10000 - prog))) >> 16;
      }
    } color_transition;

    WS2812FX() {
      WS2812FX::instance = this;
      //assign each member of the _mode[] array to its respective function reference
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
      _mode[FX_MODE_AURORA]                  = &WS2812FX::mode_aurora;
      _mode[FX_MODE_RUNNING_RANDOM]          = &WS2812FX::mode_running_random;
      _mode[FX_MODE_LARSON_SCANNER]          = &WS2812FX::mode_larson_scanner;
      _mode[FX_MODE_COMET]                   = &WS2812FX::mode_comet;
      _mode[FX_MODE_FIREWORKS]               = &WS2812FX::mode_fireworks;
      _mode[FX_MODE_RAIN]                    = &WS2812FX::mode_rain;
      _mode[FX_MODE_TETRIX]                  = &WS2812FX::mode_tetrix;
      _mode[FX_MODE_FIRE_FLICKER]            = &WS2812FX::mode_fire_flicker;
      _mode[FX_MODE_GRADIENT]                = &WS2812FX::mode_gradient;
      _mode[FX_MODE_LOADING]                 = &WS2812FX::mode_loading;
      _mode[FX_MODE_POLICE]                  = &WS2812FX::mode_police;
      _mode[FX_MODE_FAIRY]                   = &WS2812FX::mode_fairy;
      _mode[FX_MODE_TWO_DOTS]                = &WS2812FX::mode_two_dots;
      _mode[FX_MODE_FAIRYTWINKLE]            = &WS2812FX::mode_fairytwinkle;
      _mode[FX_MODE_RUNNING_DUAL]            = &WS2812FX::mode_running_dual;
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
      _mode[FX_MODE_HALLOWEEN_EYES]          = &WS2812FX::mode_halloween_eyes;
      _mode[FX_MODE_STATIC_PATTERN]          = &WS2812FX::mode_static_pattern;
      _mode[FX_MODE_TRI_STATIC_PATTERN]      = &WS2812FX::mode_tri_static_pattern;
      _mode[FX_MODE_SPOTS]                   = &WS2812FX::mode_spots;
      _mode[FX_MODE_SPOTS_FADE]              = &WS2812FX::mode_spots_fade;
      _mode[FX_MODE_GLITTER]                 = &WS2812FX::mode_glitter;
      _mode[FX_MODE_CANDLE]                  = &WS2812FX::mode_candle;
      _mode[FX_MODE_STARBURST]               = &WS2812FX::mode_starburst;
      _mode[FX_MODE_EXPLODING_FIREWORKS]     = &WS2812FX::mode_exploding_fireworks;
      _mode[FX_MODE_BOUNCINGBALLS]           = &WS2812FX::mode_bouncing_balls;
      _mode[FX_MODE_SINELON]                 = &WS2812FX::mode_sinelon;
      _mode[FX_MODE_SINELON_DUAL]            = &WS2812FX::mode_sinelon_dual;
      _mode[FX_MODE_SINELON_RAINBOW]         = &WS2812FX::mode_sinelon_rainbow;
      _mode[FX_MODE_POPCORN]                 = &WS2812FX::mode_popcorn;
      _mode[FX_MODE_DRIP]                    = &WS2812FX::mode_drip;
      _mode[FX_MODE_PLASMA]                  = &WS2812FX::mode_plasma;
      _mode[FX_MODE_PERCENT]                 = &WS2812FX::mode_percent;
      _mode[FX_MODE_RIPPLE_RAINBOW]          = &WS2812FX::mode_ripple_rainbow;
      _mode[FX_MODE_HEARTBEAT]               = &WS2812FX::mode_heartbeat;
      _mode[FX_MODE_PACIFICA]                = &WS2812FX::mode_pacifica;
      _mode[FX_MODE_CANDLE_MULTI]            = &WS2812FX::mode_candle_multi;
      _mode[FX_MODE_SOLID_GLITTER]           = &WS2812FX::mode_solid_glitter;
      _mode[FX_MODE_SUNRISE]                 = &WS2812FX::mode_sunrise;
      _mode[FX_MODE_PHASED]                  = &WS2812FX::mode_phased;
      _mode[FX_MODE_TWINKLEUP]               = &WS2812FX::mode_twinkleup;
      _mode[FX_MODE_NOISEPAL]                = &WS2812FX::mode_noisepal;
      _mode[FX_MODE_SINEWAVE]                = &WS2812FX::mode_sinewave;
      _mode[FX_MODE_PHASEDNOISE]             = &WS2812FX::mode_phased_noise;
      _mode[FX_MODE_FLOW]                    = &WS2812FX::mode_flow;
      _mode[FX_MODE_CHUNCHUN]                = &WS2812FX::mode_chunchun;
      _mode[FX_MODE_DANCING_SHADOWS]         = &WS2812FX::mode_dancing_shadows;
      _mode[FX_MODE_WASHING_MACHINE]         = &WS2812FX::mode_washing_machine;
      _mode[FX_MODE_CANDY_CANE]              = &WS2812FX::mode_candy_cane;
      _mode[FX_MODE_BLENDS]                  = &WS2812FX::mode_blends;
      _mode[FX_MODE_TV_SIMULATOR]            = &WS2812FX::mode_tv_simulator;
      _mode[FX_MODE_DYNAMIC_SMOOTH]          = &WS2812FX::mode_dynamic_smooth;


// Technical notes
// ===============
// Defines which effects are incuded in the build. Currently only WLEDSR_LARGE used.
// It is possible to just comment any effect you don't like, but it is recommended to use the ifdefs.
// Note that this is the only place to activate or de-activate effects. No need to (un)comment anything else then
      // _mode[<Effect>]          = &WS2812FX::mode_<effect>;
// in this version, commented effects will show in the UI but will not be executed / compiled,
// instead of that, the blink effect will be shown. This is done in WS2812FX::service() in fx_fcn.cpp
// to do: find out if the effect list can only be loaded with effects which are uncommented here.
// Note: as in this version, numbering is made future prove (e.g. range for AC, range for SR, fixed numbering).
//       If you have old presets, they need to be converted to the new numbering.
//       Harry Baas made a VB script to convert them.
// Note: If all SR effects are disabled: 2.5% of 1.310.720 bytes = 32.768 bytes are saved.
// Note: the order of numbering is based on the order in which effects are added, so newer effects get higher numbers

#define WLEDSR_SMALL  1
#define WLEDSR_MEDIUM 1
#define WLEDSR_LARGE  1

//WLED AC effects disabled by WLEDSR setting
#ifdef WLEDSR_LARGE
      _mode[FX_MODE_TWINKLEFOX]              = &WS2812FX::mode_twinklefox;
      _mode[FX_MODE_TWINKLECAT]              = &WS2812FX::mode_twinklecat;
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//    Start of Audio Reactive fork (WLEDSR)                                                                                              //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

      _mode[FX_MODE_PIXELS]                  = &WS2812FX::mode_pixels;
      _mode[FX_MODE_PIXELWAVE]               = &WS2812FX::mode_pixelwave;
      _mode[FX_MODE_JUGGLES]                 = &WS2812FX::mode_juggles;
      _mode[FX_MODE_MATRIPIX]                = &WS2812FX::mode_matripix;
      _mode[FX_MODE_GRAVIMETER]              = &WS2812FX::mode_gravimeter;
      _mode[FX_MODE_PLASMOID]                = &WS2812FX::mode_plasmoid;
      _mode[FX_MODE_PUDDLES]                 = &WS2812FX::mode_puddles;
      _mode[FX_MODE_MIDNOISE]                = &WS2812FX::mode_midnoise;
      _mode[FX_MODE_NOISEMETER]              = &WS2812FX::mode_noisemeter;
      _mode[FX_MODE_FREQWAVE]                = &WS2812FX::mode_freqwave;
      _mode[FX_MODE_FREQMATRIX]              = &WS2812FX::mode_freqmatrix;
      _mode[FX_MODE_2DGEQ]                   = &WS2812FX::mode_2DGEQ;
      _mode[FX_MODE_WATERFALL]               = &WS2812FX::mode_waterfall;
      _mode[FX_MODE_FREQPIXELS]              = &WS2812FX::mode_freqpixels;
      _mode[FX_MODE_BINMAP]                  = &WS2812FX::mode_binmap;
      _mode[FX_MODE_NOISEFIRE]               = &WS2812FX::mode_noisefire;
      _mode[FX_MODE_PUDDLEPEAK]              = &WS2812FX::mode_puddlepeak;
      _mode[FX_MODE_NOISEMOVE]               = &WS2812FX::mode_noisemove;
      _mode[FX_MODE_2DNOISE]                 = &WS2812FX::mode_2Dnoise;
      _mode[FX_MODE_PERLINMOVE]              = &WS2812FX::mode_perlinmove;
      _mode[FX_MODE_RIPPLEPEAK]              = &WS2812FX::mode_ripplepeak;
      _mode[FX_MODE_2DFIRENOISE]             = &WS2812FX::mode_2Dfirenoise;
      _mode[FX_MODE_2DSQUAREDSWIRL]          = &WS2812FX::mode_2Dsquaredswirl;
      _mode[FX_MODE_2DFIRE2012]              = &WS2812FX::mode_2Dfire2012;
      _mode[FX_MODE_2DDNA]                   = &WS2812FX::mode_2Ddna;
      _mode[FX_MODE_2DMATRIX]                = &WS2812FX::mode_2Dmatrix;
      _mode[FX_MODE_2DMETABALLS]             = &WS2812FX::mode_2Dmetaballs;
      _mode[FX_MODE_FREQMAP]                 = &WS2812FX::mode_freqmap;
      _mode[FX_MODE_GRAVCENTER]              = &WS2812FX::mode_gravcenter;
      _mode[FX_MODE_GRAVCENTRIC]             = &WS2812FX::mode_gravcentric;
      _mode[FX_MODE_GRAVFREQ]                = &WS2812FX::mode_gravfreq;
      _mode[FX_MODE_DJLIGHT]                 = &WS2812FX::mode_DJLight;
      _mode[FX_MODE_2DFUNKYPLANK]            = &WS2812FX::mode_2DFunkyPlank;
      _mode[FX_MODE_2DCENTERBARS]            = &WS2812FX::mode_2DCenterBars;
      _mode[FX_MODE_2DPULSER]                = &WS2812FX::mode_2DPulser;
      _mode[FX_MODE_BLURZ]                   = &WS2812FX::mode_blurz;
      _mode[FX_MODE_2DSUNRADIATION]          = &WS2812FX::mode_2DSunradiation;
      _mode[FX_MODE_2DWAVERLY]               = &WS2812FX::mode_2DWaverly;
      _mode[FX_MODE_2DDRIFT]                 = &WS2812FX::mode_2DDrift;
      _mode[FX_MODE_2DCOLOREDBURSTS]         = &WS2812FX::mode_2DColoredBursts;
      _mode[FX_MODE_2DTARTAN]                = &WS2812FX::mode_2Dtartan;
      _mode[FX_MODE_2DPOLARLIGHTS]           = &WS2812FX::mode_2DPolarLights;
      _mode[FX_MODE_2DSWIRL]                 = &WS2812FX::mode_2DSwirl;
      _mode[FX_MODE_2DLISSAJOUS]             = &WS2812FX::mode_2DLissajous;
      _mode[FX_MODE_2DFRIZZLES]              = &WS2812FX::mode_2DFrizzles;
      _mode[FX_MODE_2DPLASMABALL]            = &WS2812FX::mode_2DPlasmaball;
      _mode[FX_MODE_FLOWSTRIPE]              = &WS2812FX::mode_FlowStripe;
      _mode[FX_MODE_2DHIPHOTIC]              = &WS2812FX::mode_2DHiphotic;
      _mode[FX_MODE_2DSINDOTS]               = &WS2812FX::mode_2DSindots;
      _mode[FX_MODE_2DDNASPIRAL]             = &WS2812FX::mode_2DDNASpiral;
      _mode[FX_MODE_2DBLACKHOLE]             = &WS2812FX::mode_2DBlackHole;
      _mode[FX_MODE_WAVESINS]                = &WS2812FX::mode_wavesins;
      _mode[FX_MODE_ROCKTAVES]               = &WS2812FX::mode_rocktaves;
      _mode[FX_MODE_2DAKEMI]                 = &WS2812FX::mode_2DAkemi;
      _mode[FX_MODE_CUSTOMEFFECT]            = &WS2812FX::mode_customEffect; //WLEDSR Custom Effects
      _mode[FX_MODE_3DRIPPLES]               = &WS2812FX::mode_3DRipples;
      _mode[FX_MODE_3DSphereMove]         = &WS2812FX::mode_3DSphereMove;
      _mode[FX_MODE_JBL]                     = &WS2812FX::mode_jbl;

#ifdef WLEDSR_LARGE
    // _mode[FX_MODE_2DPOOLNOISE]              = &WS2812FX::mode_2DPoolnoise; //code not in fx.cpp
    // _mode[FX_MODE_2DTWISTER]                = &WS2812FX::mode_2DTwiser; //code not in fx.cpp
    // _mode[FX_MODE_2DCAElementary]                = &WS2812FX::mode_2DCAElementary; //code not in fx.cpp

     _mode[FX_MODE_2DJULIA]                  = &WS2812FX::mode_2DJulia;
     _mode[FX_MODE_2DGAMEOFLIFE]             = &WS2812FX::mode_2Dgameoflife;
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//    End of Audio Reactive fork (WLEDSR)                                                                                                //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


      _brightness = DEFAULT_BRIGHTNESS;
      currentPalette = CRGBPalette16(CRGB::Black);
      targetPalette = CloudColors_p;
      ablMilliampsMax = ABL_MILLIAMPS_DEFAULT;
      currentMilliamps = 0;
      timebase = 0;
      resetSegments();
    }

    void
      finalizeInit(),
      service(void),
      blur(uint8_t),
      fill(uint32_t),
      fade_out(uint8_t r),
      setMode(uint8_t segid, uint8_t m),
      setColor(uint8_t slot, uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0),
      setColor(uint8_t slot, uint32_t c),
      setCCT(uint16_t k),
      setBrightness(uint8_t b, bool direct = false),
      setRange(uint16_t i, uint16_t i2, uint32_t col),
      setShowCallback(show_callback cb),
      setTransition(uint16_t t),
      setTransitionMode(bool t),
      calcGammaTable(float),
      trigger(void),
      setReset(uint8_t n),
      setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t grouping = 0, uint8_t spacing = 0, uint16_t offset = UINT16_MAX),
      setMainSegmentId(uint8_t n),
      restartRuntime(),
      resetSegments(),
      makeAutoSegments(bool forceReset = false),
      fixInvalidSegments(),
      setPixelColor(int n, uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0),
      show(void),
			setTargetFps(uint8_t fps),
      deserializeMap(uint8_t n=0);

    inline void setPixelColor(int n, uint32_t c) {setPixelColor(n, byte(c>>16), byte(c>>8), byte(c), byte(c>>24));}

    bool
      gammaCorrectBri = false,
      gammaCorrectCol = true,
      // REMOVED IN 7b969bb
      // applyToAllSelected = true,
      // setEffectConfig(uint8_t m, uint8_t s, uint8_t i, uint8_t c1, uint8_t c2, uint8_t c3, uint8_t p), //WLEDSR: add c1,c2,c3
      checkSegmentAlignment(void),
      hasRGBWBus(void),
      hasCCTBus(void),
      // return true if the strip is being sent pixel updates
      isUpdating(void);

    uint8_t
      paletteFade = 0,
      paletteBlend = 0,
      milliampsPerLed = 55,
      autoWhiteMode = RGBW_MODE_DUAL,
      cctBlending = 0,
      getBrightness(void),
      getModeCount(void),
      getPaletteCount(void),
      getMaxSegments(void),
      getActiveSegmentsNum(void),
      getFirstSelectedSegId(void),
      getMainSegmentId(void),
      getLastActiveSegmentId(void),
      getTargetFps(void),
      setPixelSegment(uint8_t n),
      gamma8(uint8_t),
      gamma8_cal(uint8_t, float),
      get_random_wheel_index(uint8_t);

    inline uint8_t sin_gap(uint16_t in) {
      if (in & 0x100) return 0;
      return sin8(in + 192); // correct phase shift of sine so that it starts and stops at 0
    }

    int8_t
      tristate_square8(uint8_t x, uint8_t pulsewidth, uint8_t attdec);

    uint16_t
      ablMilliampsMax,
      currentMilliamps,
      triwave16(uint16_t),
      getLengthTotal(void),
      getLengthPhysical(void),
      getFps(),
      getMinShowDelay(); // Fixes private class variable compiler error. Unsure if this is the correct way of fixing the root problem. -THATDONFC

    uint32_t
      now,
      timebase,
      color_wheel(uint8_t),
      color_from_palette(uint16_t, bool mapping, bool wrap, uint8_t mcol, uint8_t pbri = 255),
      color_blend(uint32_t,uint32_t,uint16_t,bool b16=false),
      currentColor(uint32_t colorNew, uint8_t tNr),
      gamma32(uint32_t),
      getLastShow(void),
      getPixelColor(uint16_t);

    WS2812FX::Segment
      &getSegment(uint8_t n),
      &getFirstSelectedSeg(void),
      &getMainSegment(void);

    WS2812FX::Segment*
      getSegments(void);

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
      mode_aurora(void),
      mode_running_random(void),
      mode_larson_scanner(void),
      mode_comet(void),
      mode_fireworks(void),
      mode_rain(void),
      mode_tetrix(void),
      mode_halloween(void),
      mode_fire_flicker(void),
      mode_gradient(void),
      mode_loading(void),
      mode_police(void),
      mode_fairy(void),
      mode_two_dots(void),
      mode_fairytwinkle(void),
      mode_running_dual(void),
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
      mode_ripple(void),
      mode_twinklefox(void),
      mode_twinklecat(void),
      mode_halloween_eyes(void),
      mode_static_pattern(void),
      mode_tri_static_pattern(void),
      mode_spots(void),
      mode_spots_fade(void),
      mode_glitter(void),
      mode_candle(void),
      mode_starburst(void),
      mode_exploding_fireworks(void),
      mode_bouncing_balls(void),
      mode_sinelon(void),
      mode_sinelon_dual(void),
      mode_sinelon_rainbow(void),
      mode_popcorn(void),
      mode_drip(void),
      mode_plasma(void),
      mode_percent(void),
      mode_ripple_rainbow(void),
      mode_heartbeat(void),
      mode_pacifica(void),
      mode_candle_multi(void),
      mode_solid_glitter(void),
      mode_sunrise(void),
      mode_phased(void),
      mode_twinkleup(void),
      mode_noisepal(void),
      mode_sinewave(void),
      mode_phased_noise(void),
      mode_flow(void),
      mode_chunchun(void),
      mode_dancing_shadows(void),
      mode_washing_machine(void),
      mode_candy_cane(void),
      mode_blends(void),
      mode_tv_simulator(void),
      mode_dynamic_smooth(void);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//    Start of Audio Reactive fork (WLEDSR)                                                                                              //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void
      fade2black(uint8_t r),
      set2DSegment(uint8_t n),
      setStripOrPanelWidthAndHeight(),
      noise8_help(uint8_t),
      mapNoiseToLEDsUsingPalette(),
      blur1d( CRGB* leds, fract8 blur_amount),
      blur2d( CRGB* leds, fract8 blur_amount),
      blurRows( CRGB* leds, fract8 blur_amount),
      blurColumns(CRGB* leds, fract8 blur_amount),
      fill_solid( struct CRGB * leds, const struct CRGB& color),
      fadeToBlackBy( CRGB* leds, uint8_t fadeBy),
      nscale8(       CRGB* leds, uint8_t scale),
      setPixels(CRGB* leds);

    bool
      segmentsAreIdentical(Segment* a, Segment* b);

    uint16_t
      XY(uint16_t, uint16_t),            // ewowi20210624: new XY: segmentToLogical: Maps XY in 2D segment to logical index. Works for 1D strips and 2D panels
      logicalToPhysical(uint16_t); // ewowi20210624: previous XY. Maps logical led index to physical led index. Works for 1D strips and 2D panels

    uint16_t
      matrixWidth = 30,
      matrixHeight = 1;

    uint8_t
      matrixPanels,
      matrixHorizontalPanels = 1,
      matrixVerticalPanels = 1,
      stripOrMatrixPanel = 0;

    uint8_t
      panelFirstLedTopBottom,
      panelFirstLedLeftRight,
      panelOrientationHorVert,
      panelSerpentine,
      panelTranspose;

    uint16_t
      mode_pixels(void),
      mode_pixelwave(void),
      mode_juggles(void),
      mode_matripix(void),
      mode_gravimeter(void),
      mode_plasmoid(void),
      mode_puddles(void),
      mode_midnoise(void),
      mode_noisemeter(void),
      mode_freqwave(void),
      mode_freqmatrix(void),
      mode_2DGEQ(void),
      mode_waterfall(void),
      mode_freqpixels(void),
      mode_binmap(void),
      mode_noisefire(void),
      mode_puddlepeak(void),
      mode_noisemove(void),
      mode_2Dnoise(void),
      mode_perlinmove(void),
      mode_ripplepeak(void),
      mode_2Dfirenoise(void),
      mode_2Dsquaredswirl(void),
      mode_2Dfire2012(void),
      mode_2Ddna(void),
      mode_2Dmatrix(void),
      mode_2Dmetaballs(void),
      mode_freqmap(void),
      mode_gravcenter(void),
      mode_gravcentric(void),
      mode_gravfreq(void),
      mode_DJLight(void),
      mode_2DFunkyPlank(void),
      mode_2DCenterBars(void),
      mode_2DPulser(void),
      mode_blurz(void),
      mode_2Dgameoflife(void),
      mode_2Dtartan(void),
      mode_2DPolarLights(void),
      mode_2DSwirl(void),
      mode_2DLissajous(void),
      mode_2DFrizzles(void),
      mode_2DPlasmaball(void),
      mode_FlowStripe(void),
      mode_2DHiphotic(void),
      mode_2DSindots(void),
      mode_2DDNASpiral(void),
      mode_2DBlackHole(void),
      mode_wavesins(void),
      mode_rocktaves(void),
      mode_2DAkemi(void),
      mode_2DSunradiation(void),
      mode_2DWaverly(void),
      mode_2DDrift(void),
      mode_2DColoredBursts(void),
      mode_2DJulia(void),
      mode_customEffect(void),     //WLEDSR Custom Effects
      mode_3DRipples(void),
      mode_jbl(void),
      mode_3DSphereMove(void);
    //  mode_2DPoolnoise(void),
    //  mode_2DTwister(void);
    //  mode_2DCAElementary(void);

    uint16_t
      GEQ_base(bool, bool, bool); //private???

    uint16_t
      _lengthRaw; //private? not in AC (anymore)

    bool
      _skipFirstMode; //private? not in AC (anymore)

    //WLEDSR Custom Effects
    float arti_external_function(uint8_t function, float par1 = floatNull, float par2 = floatNull, float par3 = floatNull, float par4 = floatNull, float par5 = floatNull);
    float arti_get_external_variable(uint8_t variable, float par1 = floatNull, float par2 = floatNull, float par3 = floatNull);
    void arti_set_external_variable(float value, uint8_t variable, float par1 = floatNull, float par2 = floatNull, float par3 = floatNull);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//    End of Audio Reactive fork (WLEDSR)                                                                                                //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  private:
    uint32_t crgb_to_col(CRGB fastled);
    CRGB col_to_crgb(uint32_t);
    CRGBPalette16 currentPalette;
    CRGBPalette16 targetPalette;

    uint16_t _length, _virtualSegmentLength;
    uint16_t _rand16seed;
    uint8_t _brightness;
    uint16_t _usedSegmentData = 0;
    uint16_t _transitionDur = 750;

		uint8_t _targetFps = 42;
		uint16_t _frametime = (1000/42);
    uint16_t _cumulativeFps = 2;

    bool
      _isOffRefreshRequired = false, //periodic refresh is required for the strip to remain off.
      _hasWhiteChannel = false,
      _triggered;

    mode_ptr _mode[MODE_COUNT]; // SRAM footprint: 4 bytes per element

    show_callback _callback = nullptr;

    // mode helper functions
    uint16_t
      blink(uint32_t, uint32_t, bool strobe, bool),
      candle(bool),
      color_wipe(bool, bool),
      dynamic(bool),
      scan(bool),
      running_base(bool,bool),
      larson_scanner(bool),
      sinelon_base(bool,bool),
      dissolve(uint32_t),
      chase(uint32_t, uint32_t, uint32_t, bool),
      gradient_base(bool),
      ripple_base(bool),
      police_base(uint32_t, uint32_t),
      running(uint32_t, uint32_t, bool theatre=false),
      tricolor_chase(uint32_t, uint32_t),
      twinklefox_base(bool),
      spots_base(uint16_t),
      phased_base(uint8_t);

    CRGB twinklefox_one_twinkle(uint32_t ms, uint8_t salt, bool cat);
    CRGB pacifica_one_layer(uint16_t i, CRGBPalette16& p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff);

    void
      blendPixelColor(uint16_t n, uint32_t color, uint8_t blend),
      startTransition(uint8_t oldBri, uint32_t oldCol, uint16_t dur, uint8_t segn, uint8_t slot),
      estimateCurrentAndLimitBri(void),
      load_gradient_palette(uint8_t),
      handle_palette(void);

    uint16_t* customMappingTable = nullptr;
    uint16_t  customMappingSize  = 0;

    uint32_t _lastPaletteChange = 0;
    uint32_t _lastShow = 0;

    uint32_t _colors_t[3];
    uint8_t _bri_t;

    bool _no_rgb = false;

    uint8_t _segment_index = 0;
    uint8_t _segment_index_palette_last = 99;
    uint8_t _mainSegment;

    segment _segments[MAX_NUM_SEGMENTS] = { // SRAM footprint: 27 bytes per element
      //WLEDSR: add f1,2,3
      // start, stop, offset, speed, intensity, custom1, custom2, custom3, palette, mode, options, grouping, spacing, opacity (unused), color[], capabilities
      {0, 7, 0, DEFAULT_SPEED, DEFAULT_INTENSITY, DEFAULT_Custom1, DEFAULT_Custom2, DEFAULT_Custom3, 0, DEFAULT_MODE, NO_OPTIONS, 1, 0, 255, {DEFAULT_COLOR}, 0}
    };
    segment_runtime _segment_runtimes[MAX_NUM_SEGMENTS]; // SRAM footprint: 28 bytes per element
    friend class Segment_runtime;

    ColorTransition transitions[MAX_NUM_TRANSITIONS]; //12 bytes per element
    friend class ColorTransition;

    uint16_t
      segmentToLogical(uint16_t i),
      transitionProgress(uint8_t tNr);
  public:
    inline bool hasWhiteChannel(void) {return _hasWhiteChannel;}
    inline bool isOffRefreshRequired(void) {return _isOffRefreshRequired;}
};

//10 names per line
// WLEDSR: second part
// Technical notes
// ===============
// If an effect name is followed by an @, slider and color control is effective.
// See setEffectParameters in index.js for implementation
// If not effective then:
//      - For AC effects (id<128) 2 sliders and 3 colors and the palette will be shown
//      - For SR effects (id>128) 5 sliders and 3 colors and the palette will be shown
// If effective (@)
//      - a ; seperates slider controls (left) from color controls (middle) and palette control (right)
//      - if left, middle or right is empty no controls are shown
//      - a , seperates slider controls (max 5) or color controls (max 3). Palette has only one value
//      - a ! means that the default is used.
//             - For sliders: Effect speeds, Effect intensity, Custom 1, Custom 2, Custom 3
//               - Blazoncek default values: e.g. Sensitivity=128
//             - For colors: Fx color, Background color, Custom
//             - For palette: prompt Color palette
//
// Note: In this version only SR-effects have been specified.
// Note: If palette is on and no colors are specified 1,2 and 3 is shown in each color circle.
//       If a color is specified, the 1,2 or 3 is replaced by that specification.
// Note: Effects can override default pattern behaviour
//       - FadeToBlack can override the background setting
//       - Defining SEGCOL(<i>) can override a specific palette using these values (e.g. Color Gradient)


const char JSON_mode_names[] PROGMEM = R"=====([
"Solid@;!;",
"Blink@!,;!,!,;!",
"Breathe@!,;!,!;!",
"Wipe@!,!;!,!,;!",
"Wipe Random@!,;1,2,3;!",
"Random Colors@!,Fade time;1,2,3;!",
"Sweep@!,!;!,!,;!",
"Dynamic@!,!;1,2,3;!",
"Colorloop@!,Saturation;1,2,3;!",
"Rainbow@!,Size;1,2,3;!",
"Scan@!,# of dots;!,!,;!",
"Scan Dual@!,# of dots;!,!,;!",
"Fade@!,;!,!,;!",
"Theater@!,Gap size;!,!,;!",
"Theater Rainbow@!,Gap size;1,2,3;!",
"Running@!,Wave width;!,!,;!",
"Saw@!,Width;!,!,;!",
"Twinkle@!,;!,!,;!",
"Dissolve@Repeat speed,Dissolve speed;!,!,;!",
"Dissolve Rnd@Repeat speed,Dissolve speed;,!,;!",
"Sparkle@!,;!,!,;!",
"Sparkle Dark@!,!;Bg,Fx,;!",
"Sparkle+@!,!;Bg,Fx,;!",
"Strobe@!,;!,!,;!",
"Strobe Rainbow@!,;,!,;!",
"Strobe Mega@!,!;!,!,;!",
"Blink Rainbow@Frequency,Blink duration;!,!,;!",
"Android@!,Width;!,!,;!",
"Chase@!,Width;!,!,!;!",
"Chase Random@!,Width;!,,!;!",
"Chase Rainbow@!,Width;!,!,;0",
"Chase Flash@!,;Bg,Fx,!;!",
"Chase Flash Rnd@!,;,Fx,;!",
"Rainbow Runner@!,Size;Bg,,;!",
"Colorful@!,Saturation;1,2,3;!",
"Traffic Light@!,;,!,;!",
"Sweep Random",
"Chase 2@!,Width;!,!,;!",
"Aurora@!=24,!;1,2,3;!=50",
"Stream",
"Scanner",
"Lighthouse",
"Fireworks@Sharpness=96,Frequency=192;!,2,;!=11",
"Rain@Fade rate=128,Frequency=128;!,2,;!",
"Tetrix@!=224,Width=0;!,!,;!=11",
"Fire Flicker@!,!;!,,;!",
"Gradient@!,Spread=16;!,!,;!",
"Loading@!,Fade=16;!,!,;!",
"Police@!,Width;,Bg,;0",
"Fairy",
"Two Dots@!,Dot size;1,2,Bg;!",
"Fairy Twinkle",
"Running Dual",
"Halloween",
"Tri Chase@!,Size;1,2,3;0",
"Tri Wipe@!,Width;1,2,3;0",
"Tri Fade",
"Lightning",
"ICU",
"Multi Comet",
"Scanner Dual",
"Stream 2",
"Oscillate",
"Pride 2015@!,;;",
"Juggle@!=16,Trail=240;!,!,;!",
"Palette@!,;1,2,3;!",
"Fire 2012@Spark rate=120,Decay=64;1,2,3;!",
"Colorwaves",
"Bpm@!=64,;1,2,3;!",
"Fill Noise",
"Noise 1",
"Noise 2",
"Noise 3",
"Noise 4",
"Colortwinkles@Fade speed,Spawn speed;1,2,3;!",
"Lake@!,;1,2,3;!",
"Meteor@!,Trail length;!,!,;!",
"Meteor Smooth@!,Trail length;!,!,;!",
"Railway",
"Ripple",
"Twinklefox",
"Twinklecat",
"Halloween Eyes",
"Solid Pattern@Fg size,Bg size;Fg,Bg,;!=0",
"Solid Pattern Tri@,Size;1,2,3;!=0",
"Spots@Spread,Width;!,!,;!",
"Spots Fade@Spread,Width;!,!,;!",
"Glitter",
"Candle@Flicker rate=96,Flicker intensity=224;!,!,;0",
"Fireworks Starburst",
"Fireworks 1D@Gravity,Firing side;!,!,;!",
"Bouncing Balls@Gravity,# of balls;!,!,;!",
"Sinelon",
"Sinelon Dual",
"Sinelon Rainbow",
"Popcorn",
"Drip@Gravity,# of drips;!,!;!",
"Plasma@Phase,;1,2,3;!",
"Percent@,% of fill;!,!,;!",
"Ripple Rainbow",
"Heartbeat@!,!;!,!,;!",
"Pacifica",
"Candle Multi@Flicker rate=96,Flicker intensity=224;!,!,;0",
"Solid Glitter@,!;!,,;0",
"Sunrise@Time [min]=60,;;0",
"Phased",
"Twinkleup@!,Intensity;!,!,;!",
"Noise Pal",
"Sine",
"Phased Noise",
"Flow",
"Chunchun@!,Gap size;!,!,;!",
"Dancing Shadows@!,# of shadows;!,,;!",
"Washing Machine",
"Candy Cane@!,Width;;",
"Blends@Shift speed,Blend speed;1,2,3,!",
"TV Simulator",
"Dynamic Smooth",
"Reserved0",
"Reserved1",
"Reserved2",
"Reserved3",
"Reserved4",
"Reserved5",
"Reserved6",
"Reserved7",
"Reserved8",
"Reserved9",
" ♪ Pixels@Fade rate,# of pixels;,!;!",
" ♪ Pixelwave@!,Sensitivity=64;!,!;!",
" ♪ Juggles@!,# of balls;,!;!",
" ♪ Matripix@!,Brightness=64;,!;!",
" ♪ Gravimeter@Rate of fall,Sensitivity=128;,!;!",
" ♪ Plasmoid@Phase=128,# of pixels=128;,!;!",
" ♪ Puddles@Fade rate,Puddle size;!,!;!",
" ♪ Midnoise@Fade rate,Maximum length=128;,!;!",
" ♪ Noisemeter@Fade rate,Width=128;!,!;!",
" ♫ Freqwave@Time delay,Sound effect,Low bin,High bin,Pre-amp;;",
" ♫ Freqmatrix@Time delay,Sound effect,Low bin,High bin,Sensivity;;",
" ♫ 2D GEQ@Bar speed,Ripple decay;,,Peak Color;!",
" ♫ Waterfall@!,Adjust color,,Select bin, Volume (minimum);!,!;!",
" ♫ Freqpixels@Fade rate,Starting colour and # of pixels;;",
" ♫ Binmap@;!,!;!",
" ♪ Noisefire@!,!;;",
" ♪ Puddlepeak@Fade rate,Puddle size,,Select bin,Volume (minimum);!,!;!",
" ♫ Noisemove@Speed of perlin movement,Fade rate;,!;!",
"2D Noise@Speed,Scale;;!",
"Perlin Move@!,# of pixels,fade rate;,!;!",
" ♪ Ripple Peak@Fade rate,Max # of ripples,,Select bin,Volume (minimum);!,!;!",
"2D FireNoise@X scale,Y scale;;",
"2D Squared Swirl@,,,,Blur;,,;!",
"2D Fire2012@Speed;;",
"2D DNA@Scroll speed,Blur;;!",
"2D Matrix@Falling speed,Spawning rate,Trail,Custom color ☑;Spawn,Trail;",
"2D Metaballs@;;",
" ♫ Freqmap@Fade rate,Starting color;,!;!",
" ♪ Gravcenter@Rate of fall,Sensitivity=128;,!;!",
" ♪ Gravcentric@Rate of fall,Sensitivity=128;!;!",
" ♫ Gravfreq@Rate of fall,Sensivity=128;,!;!",
" ♫ DJ Light@Speed;;",
" ♫ 2D Funky Plank@Scroll speed,,# of bands;;",
" ♫ 2D CenterBars@Bar speed=250,Ripple decay=250,Center ↔ ☑=192,Center ↕ ☑=192, Color ↕ ☑=192;,,Peak Color;!=11",
"2D Pulser@Speed,Blur;;!",
" ♫ Blurz@Fade rate,Blur amount;,Color mix;!",
"2D Drift@Rotation speed,Blur amount;;!",
" ♪ 2D Waverly@Amplification,Sensitivity=64;;!",
"2D Sun Radiation@Variance,Brightness;;",
"2D Colored Bursts@Speed,Number of lines;;!",
"2D Julia@,Max iterations per pixel,X center,Y center,Area size;;!",
"Reserved for PoolNoise",
"Reserved for Twister",
"Reserved for Elementary",
"2D Game Of Life@!,Use Palette ☑;!,!;!",
"2D Tartan@X scale,Y scale;;!",
"2D Polar Lights@Speed,X scale,Palette;;",
" ♪ 2D Swirl@!,Sensitivity=64,Blur;,Bg Swirl;!",
"2D Lissajous@X frequency,Fadetime;;!",
"2D Frizzles@X frequency,Y frequency;;!",
"2D Plasma Ball@Speed;;!",
"Flow Stripe@Hue speed,Effect speed;;",
"2D Hiphotic@X scale,Y scale;;!",
"2D Sindots@Speed,Dot distance;;!",
"2D DNA Spiral@Speed,Frequency;;!",
"2D Black Hole@Outer X frequency,Inner X frequency,Inner Y frequency;;",
"Wavesins@Speed,Brightness variation,Starting Color,Range of Colors,Color variation;;!",
" ♫ Rocktaves@;,!;!",
" ♫ 2D Akemi@Color speed,Dance ☑;Head palette,Arms & Legs,Eyes & Mouth;Face palette",
" ⚙️ Custom Effect@Speed,Intensity,Custom 1, Custom 2, Custom 3;!;!",
"3D Ripples@Speed=128,Interval=128;!;!",
"3D Sphere Move@Speed=128,Interval=128;!;!",
" ♪ JBL@Sensitivity,Color change speed;!,!;!"
])=====";

//WLEDSR: second part (not SR specific, but in latest SR, not in AC (Pallettes added in WLEDSR from Retro Clown->END))
const char JSON_palette_names[] PROGMEM = R"=====([
"Default","* Random Cycle","* Color 1","* Colors 1&2","* Color Gradient","* Colors Only","Party","Cloud","Lava","Ocean",
"Forest","Rainbow","Rainbow Bands","Sunset","Rivendell","Breeze","Red & Blue","Yellowout","Analogous","Splash",
"Pastel","Sunset 2","Beech","Vintage","Departure","Landscape","Beach","Sherbet","Hult","Hult 64",
"Drywet","Jul","Grintage","Rewhi","Tertiary","Fire","Icefire","Cyane","Light Pink","Autumn",
"Magenta","Magred","Yelmag","Yelblu","Orange & Teal","Tiamat","April Night","Orangery","C9","Sakura",
"Aurora","Atlantica","C9 2","C9 New","Temperature","Aurora 2","Retro Clown","Candy","Toxy Reaf","Fairy Reaf",
"Semi Blue","Pink Candy","Red Reaf","Aqua Flash","Yelblu Hot","Lite Light","Red Flash","Blink Red","Red Shift","Red Tide",
"Candy2"
])=====";

#endif
