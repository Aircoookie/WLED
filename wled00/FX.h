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
#define DEFAULT_C1         (uint8_t)128
#define DEFAULT_C2         (uint8_t)128
#define DEFAULT_C3         (uint8_t)128

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

//colors.cpp (.h does not like including other .h)
uint32_t color_blend(uint32_t,uint32_t,uint16_t,bool b16);
uint32_t color_add(uint32_t,uint32_t);

/* Not used in all effects yet */
#define WLED_FPS         42
#define FRAMETIME_FIXED  (1000/WLED_FPS)
//#define FRAMETIME        _frametime
#define FRAMETIME        strip.getFrameTime()

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

#define MIN_SHOW_DELAY   (_frametime < 16 ? 8 : 15)

#define NUM_COLORS       3 /* number of colors per segment */
//#define SEGMENT          _segments[_segment_index]
//#define SEGCOLOR(x)      _colors_t[x]
//#define SEGENV           _segment_runtimes[_segment_index]
//#define SEGLEN           _virtualSegmentLength
#define SEGMENT          strip.getSegment(strip.getCurrSegmentId())
#define SEGENV           strip.getSegmentRuntime(strip.getCurrSegmentId())
#define SEGCOLOR(x)      strip.segColor(x)
#define SEGLEN           strip.segLen()
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
// bits 4-6: TBD
// bit    3: mirror effect within segment
// bit    2: segment is on
// bit    1: reverse segment
// bit    0: segment is selected
#define NO_OPTIONS   (uint16_t)0x0000
#define TRANSPOSED   (uint16_t)0x0400 // rotated 90deg & reversed
#define REVERSE_Y_2D (uint16_t)0x0200
#define MIRROR_Y_2D  (uint16_t)0x0100
#define TRANSITIONAL (uint16_t)0x0080
#define MIRROR       (uint16_t)0x0008
#define SEGMENT_ON   (uint16_t)0x0004
#define REVERSE      (uint16_t)0x0002
#define SELECTED     (uint16_t)0x0001

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
// new 2D effects
#define FX_MODE_SPACESHIPS             118
#define FX_MODE_CRAZYBEES              119
#define FX_MODE_GHOST_RIDER            120
#define FX_MODE_BLOBS                  121
#define FX_MODE_SCROLL_TEXT            122
#define FX_MODE_DRIFT_ROSE             123

// WLED-SR effects
#ifndef USERMOD_AUDIOREACTIVE

  #define FX_MODE_PERLINMOVE              53 // should be moved to 53
  #define FX_MODE_FLOWSTRIPE             114 // should be moved to 114
  #define FX_MODE_WAVESINS                48 // should be moved to 48
  #define FX_MODE_2DBLACKHOLE            124 // non audio
  #define FX_MODE_2DDNASPIRAL            125 // non audio
  #define FX_MODE_2DHIPHOTIC             126 // non audio
  #define FX_MODE_2DPLASMABALL           127 // non audio
  #define FX_MODE_2DSINDOTS              128 // non audio
  #define FX_MODE_PIXELWAVE              129 // audio enhanced
  #define FX_MODE_JUGGLES                130 // audio enhanced
  #define FX_MODE_MATRIPIX               131 // audio enhanced
  #define FX_MODE_GRAVIMETER             132 // audio enhanced
  #define FX_MODE_PLASMOID               133 // audio enhanced
  #define FX_MODE_PUDDLES                134 // audio enhanced
  #define FX_MODE_MIDNOISE               135 // audio enhanced
  #define FX_MODE_NOISEMETER             136 // audio enhanced
  #define FX_MODE_2DFRIZZLES             137 // non audio
  #define FX_MODE_2DLISSAJOUS            138 // non audio
  #define FX_MODE_2DPOLARLIGHTS          139 // non audio
  #define FX_MODE_2DTARTAN               140 // non audio
  #define FX_MODE_2DGAMEOFLIFE           141 // non audio
  #define FX_MODE_2DJULIA                142 // non audio
  #define FX_MODE_NOISEFIRE              143 // audio enhanced
  #define FX_MODE_PUDDLEPEAK             144 // audio enhanced
  #define FX_MODE_2DCOLOREDBURSTS        145 // non audio
  #define FX_MODE_2DSUNRADIATION         146 // non audio
  #define FX_MODE_2DNOISE                147 // non audio
  #define FX_MODE_RIPPLEPEAK             148 // audio enhanced
  #define FX_MODE_2DFIRENOISE            149 // non audio
  #define FX_MODE_2DSQUAREDSWIRL         150 // non audio
  #define FX_MODE_2DDNA                  151 // non audio
  #define FX_MODE_2DMATRIX               152 // non audio
  #define FX_MODE_2DMETABALLS            153 // non audio
  #define FX_MODE_2DPULSER               154 // non audio
  #define FX_MODE_2DDRIFT                155 // non audio
  #define FX_MODE_2DWAVERLY              156 // audio enhanced
  #define FX_MODE_GRAVCENTER             157 // audio enhanced
  #define FX_MODE_GRAVCENTRIC            158 // audio enhanced
  #define FX_MODE_2DSWIRL                159 // audio enhanced
  #define FX_MODE_2DAKEMI                160 // audio enhanced

  #define MODE_COUNT                     161

#else

  #define FX_MODE_PIXELS                 128
  #define FX_MODE_PIXELWAVE              129 // audio enhanced
  #define FX_MODE_JUGGLES                130 // audio enhanced
  #define FX_MODE_MATRIPIX               131 // audio enhanced
  #define FX_MODE_GRAVIMETER             132 // audio enhanced
  #define FX_MODE_PLASMOID               133 // audio enhanced
  #define FX_MODE_PUDDLES                134 // audio enhanced
  #define FX_MODE_MIDNOISE               135 // audio enhanced
  #define FX_MODE_NOISEMETER             136 // audio enhanced
  #define FX_MODE_FREQWAVE               137
  #define FX_MODE_FREQMATRIX             138
  #define FX_MODE_2DGEQ                  139
  #define FX_MODE_WATERFALL              140
  #define FX_MODE_FREQPIXELS             141
  #define FX_MODE_BINMAP                 142
  #define FX_MODE_NOISEFIRE              143 // audio enhanced
  #define FX_MODE_PUDDLEPEAK             144 // audio enhanced
  #define FX_MODE_NOISEMOVE              145
  #define FX_MODE_2DNOISE                146 // non audio
  #define FX_MODE_PERLINMOVE             147 // should be moved to 53
  #define FX_MODE_RIPPLEPEAK             148 // audio enhanced
  #define FX_MODE_2DFIRENOISE            149 // non audio
  #define FX_MODE_2DSQUAREDSWIRL         150 // non audio
  //#define FX_MODE_2DFIRE2012             151 // implemented in native Fire2012
  #define FX_MODE_2DDNA                  152 // non audio
  #define FX_MODE_2DMATRIX               153 // non audio
  #define FX_MODE_2DMETABALLS            154 // non audio
  #define FX_MODE_FREQMAP                155
  #define FX_MODE_GRAVCENTER             156 // audio enhanced
  #define FX_MODE_GRAVCENTRIC            157 // audio enhanced
  #define FX_MODE_GRAVFREQ               158
  #define FX_MODE_DJLIGHT                159
  #define FX_MODE_2DFUNKYPLANK           160
  //#define FX_MODE_2DCENTERBARS           161 // obsolete by X & Y mirroring
  #define FX_MODE_2DPULSER               162 // non audio
  #define FX_MODE_BLURZ                  163
  #define FX_MODE_2DDRIFT                164 // non audio
  #define FX_MODE_2DWAVERLY              165 // audio enhanced
  #define FX_MODE_2DSUNRADIATION         166 // non audio
  #define FX_MODE_2DCOLOREDBURSTS        167 // non audio
  #define FX_MODE_2DJULIA                168 // non audio
  #define FX_MODE_2DPOOLNOISE            169 // reserved in JSON_mode_names
  #define FX_MODE_2DTWISTER              170 // reserved in JSON_mode_names
  #define FX_MODE_2DCAELEMENTATY         171 // reserved in JSON_mode_names
  #define FX_MODE_2DGAMEOFLIFE           172 // non audio
  #define FX_MODE_2DTARTAN               173 // non audio
  #define FX_MODE_2DPOLARLIGHTS          174 // non audio
  #define FX_MODE_2DSWIRL                175 // audio enhanced
  #define FX_MODE_2DLISSAJOUS            176 // non audio
  #define FX_MODE_2DFRIZZLES             177 // non audio
  #define FX_MODE_2DPLASMABALL           178 // non audio
  #define FX_MODE_FLOWSTRIPE             179 // should be moved to 114
  #define FX_MODE_2DHIPHOTIC             180 // non audio
  #define FX_MODE_2DSINDOTS              181 // non audio
  #define FX_MODE_2DDNASPIRAL            182 // non audio
  #define FX_MODE_2DBLACKHOLE            183 // non audio
  #define FX_MODE_WAVESINS               184 // should be moved to 48
  #define FX_MODE_ROCKTAVES              185
  #define FX_MODE_2DAKEMI                186 // audio enhanced
  //#define FX_MODE_CUSTOMEFFECT           187 //WLEDSR Custom Effects

  #define MODE_COUNT                     187
#endif


// segment parameters
typedef struct Segment { // 35 (36 in memory) bytes
  uint16_t start; // start index / start X coordinate 2D (left)
  uint16_t stop;  // stop index / stop X coordinate 2D (right); segment is invalid if stop == 0
  uint16_t offset;
  uint8_t  speed;
  uint8_t  intensity;
  uint8_t  palette;
  uint8_t  mode;
  uint16_t options; //bit pattern: msb first: [transposed mirrorY reverseY] transitional (tbd) paused needspixelstate mirrored on reverse selected
  uint8_t  grouping, spacing;
  uint8_t  opacity;
  uint32_t colors[NUM_COLORS];
  uint8_t  cct; //0==1900K, 255==10091K
  uint8_t  _capabilities;
  uint8_t  custom1, custom2, custom3; // custom FX parameters
  uint16_t startY;  // start Y coodrinate 2D (top)
  uint16_t stopY;   // stop Y coordinate 2D (bottom)
  char *name;
  inline bool     getOption(uint8_t n)   { return ((options >> n) & 0x01); }
  inline bool     isSelected()           { return getOption(0); }
  inline bool     isActive()             { return stop > start; }
  inline uint16_t width()                { return stop - start; }
  inline uint16_t height()               { return stopY - startY; }
  inline uint16_t length()               { return width(); }
  inline uint16_t groupLength()          { return grouping + spacing; }
  inline uint8_t  getLightCapabilities() { return _capabilities; }
  bool setColor(uint8_t slot, uint32_t c, uint8_t segn); //returns true if changed
  void setCCT(uint16_t k, uint8_t segn);
  void setOpacity(uint8_t o, uint8_t segn);
  void setOption(uint8_t n, bool val, uint8_t segn = 255);
  // 2D matrix
  uint16_t virtualWidth();
  uint16_t virtualHeight();
  // 1D strip
  uint16_t virtualLength();
  uint8_t differs(Segment& b);
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

  bool allocateData(uint16_t len);
  void deallocateData();
  void resetIfRequired();
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


// color transitions
typedef struct ColorTransition { // 12 bytes
  uint32_t colorOld = 0;
  uint32_t transitionStart;
  uint16_t transitionDur;
  uint8_t segment = 0xFF; //lower 6 bits: the segment this transition is for (255 indicates transition not in use/available) upper 2 bits: color channel
  uint8_t briOld = 0;

  static void startTransition(uint8_t oldBri, uint32_t oldCol, uint8_t segn, uint8_t slot);
  uint8_t currentBri(bool turningOff = false, uint8_t slot = 0);
  uint16_t progress(bool allowEnd = false); //transition progression between 0-65535
  uint32_t currentColor(uint32_t colorNew) {
    return color_blend(colorOld, colorNew, progress(true), true);
  }
} color_transition;


// main "strip" class
class WS2812FX {
  typedef uint16_t (*mode_ptr)(void); // pointer to mode function
  typedef void (*show_callback)(void); // pre show callback

  static WS2812FX* instance;
  
  public:

    WS2812FX() {
      WS2812FX::instance = this;
      setupEffectData();
      _brightness = DEFAULT_BRIGHTNESS;
      currentPalette = CRGBPalette16(CRGB::Black);
      targetPalette = CloudColors_p;
      ablMilliampsMax = ABL_MILLIAMPS_DEFAULT;
      currentMilliamps = 0;
      timebase = 0;
      resetSegments();
    }

    static WS2812FX* getInstance(void) { return instance; }

    void
      finalizeInit(),
      service(void),
      blur(uint8_t),
      fill(uint32_t c, uint8_t seg=255),
      fade_out(uint8_t r),
      fadeToBlackBy(uint8_t fadeBy),
      setMode(uint8_t segid, uint8_t m),
      setColor(uint8_t slot, uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0),
      setColor(uint8_t slot, uint32_t c),
      setCCT(uint16_t k),
      setBrightness(uint8_t b, bool direct = false),
      setRange(uint16_t i, uint16_t i2, uint32_t col),
      setTransitionMode(bool t),
      calcGammaTable(float),
      setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t grouping = 0, uint8_t spacing = 0, uint16_t offset = UINT16_MAX, uint16_t startY=0, uint16_t stopY=0),
      setMainSegmentId(uint8_t n),
      restartRuntime(),
      resetSegments(),
      makeAutoSegments(bool forceReset = false),
      fixInvalidSegments(),
      setPixelColor(int n, uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0),
      setPixelColor(float i, uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0, bool aa = false),
      blendPixelColor(uint16_t n, uint32_t color, uint8_t blend),
      show(void),
			setTargetFps(uint8_t fps),
      deserializeMap(uint8_t n=0);

    void addEffect(uint8_t id, mode_ptr mode_fn, const char *mode_name) { if (id < _modeCount) { _mode[id] = mode_fn; _modeData[id] = mode_name;} }
    void setupEffectData(void); // defined in FX.cpp

    // outsmart the compiler :) by correctly overloading
    inline void setPixelColor(int n, uint32_t c)                 {setPixelColor(n, byte(c>>16), byte(c>>8), byte(c), byte(c>>24));}
    inline void setPixelColor(int n, CRGB c)                     {setPixelColor(n, c.red, c.green, c.blue);}
    inline void setPixelColor(float i, uint32_t c, bool aa=true) {setPixelColor(i, byte(c>>16), byte(c>>8), byte(c), byte(c>>24), aa);}
    inline void setPixelColor(float i, CRGB c, bool aa=true)     {setPixelColor(i, c.red, c.green, c.blue, 0, aa);}
    inline void trigger(void) { _triggered = true; } // Forces the next frame to be computed on all active segments.
    inline void setShowCallback(show_callback cb) { _callback = cb; }
    inline void setTransition(uint16_t t) { _transitionDur = t; }

    bool
      gammaCorrectBri = false,
      gammaCorrectCol = true,
      checkSegmentAlignment(void),
      hasRGBWBus(void),
      hasCCTBus(void),
      // return true if the strip is being sent pixel updates
      isUpdating(void);

    inline bool hasWhiteChannel(void) {return _hasWhiteChannel;}
    inline bool isOffRefreshRequired(void) {return _isOffRefreshRequired;}

    uint8_t
      paletteFade = 0,
      paletteBlend = 0,
      milliampsPerLed = 55,
      cctBlending = 0,
      getActiveSegmentsNum(void),
      getFirstSelectedSegId(void),
      getLastActiveSegmentId(void),
      setPixelSegment(uint8_t n),
      gamma8(uint8_t),
      gamma8_cal(uint8_t, float),
      get_random_wheel_index(uint8_t);

    inline uint8_t getBrightness(void) { return _brightness; }
    inline uint8_t getMaxSegments(void) { return MAX_NUM_SEGMENTS; }
    inline uint8_t getCurrSegmentId(void) { return _segment_index; }
    inline uint8_t getMainSegmentId(void) { return _mainSegment; }
    inline uint8_t getPaletteCount() { return 13 + GRADIENT_PALETTE_COUNT; }
    inline uint8_t getTargetFps() { return _targetFps; }
    inline uint8_t getModeCount() { return _modeCount; }
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
      getLengthPhysical(void),
      getFps();

    inline uint16_t getFrameTime(void) { return _frametime; }
    inline uint16_t getMinShowDelay(void) { return MIN_SHOW_DELAY; }
    inline uint16_t getLengthTotal(void) { return _length; }
    inline uint16_t segLen(void) { return _virtualSegmentLength; }

    uint32_t
      now,
      timebase,
      color_wheel(uint8_t),
      color_from_palette(uint16_t, bool mapping, bool wrap, uint8_t mcol, uint8_t pbri = 255),
      currentColor(uint32_t colorNew, uint8_t tNr),
      gamma32(uint32_t),
      getPixelColor(uint16_t);

    inline uint32_t getLastShow(void) { return _lastShow; }
    inline uint32_t segColor(uint8_t i) { return _colors_t[i]; }

    const char *
      getModeData(uint8_t id = 0) { return id<_modeCount ? _modeData[id] : nullptr; }

    const char **
      getModeDataSrc(void) { return _modeData; }

    inline Segment& getSegment(uint8_t id) { return _segments[id >= MAX_NUM_SEGMENTS ? getMainSegmentId() : id]; }
    inline Segment& getFirstSelectedSeg(void) { return _segments[getFirstSelectedSegId()]; }
    inline Segment& getMainSegment(void) { return _segments[getMainSegmentId()]; }
    inline Segment* getSegments(void) { return _segments; }
    inline Segment_runtime& getSegmentRuntime(uint8_t id) { return _segment_runtimes[id >= MAX_NUM_SEGMENTS ? getMainSegmentId() : id]; }

  // 2D support (panels)
    bool
      isMatrix = false;

    uint8_t
      hPanels = 1,
      vPanels = 1;

    uint16_t
      panelH = 8,
      panelW = 8,
      matrixWidth = DEFAULT_LED_COUNT,
      matrixHeight = 1;

    #define WLED_MAX_PANELS 64
    typedef struct panel_bitfield_t {
      unsigned char
        bottomStart : 1, // starts at bottom?
        rightStart  : 1, // starts on right?
        vertical    : 1, // is vertical?
        serpentine  : 1; // is serpentine?
    } Panel;
    Panel
      matrix = {0,0,0,0},
      panel[WLED_MAX_PANELS] = {{0,0,0,0}};

    void
      setUpMatrix(),
      setPixelColorXY(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0),
      setPixelColorXY(float x, float y, byte r, byte g, byte b, byte w = 0, bool aa = false),
      blendPixelColorXY(uint16_t x, uint16_t y, uint32_t color, uint8_t blend),
      addPixelColorXY(uint16_t x, uint16_t y, uint32_t color),
      blur1d(CRGB* leds, fract8 blur_amount),
      blur1d(uint16_t i, bool vertical, fract8 blur_amount, CRGB* leds=nullptr), // 1D box blur (with weight)
      blur2d(CRGB* leds, fract8 blur_amount),
      blurRow(uint16_t row, fract8 blur_amount, CRGB* leds=nullptr),
      blurCol(uint16_t col, fract8 blur_amount, CRGB* leds=nullptr),
      moveX(CRGB *leds, int8_t delta),
      moveY(CRGB *leds, int8_t delta),
      move(uint8_t dir, uint8_t delta, CRGB *leds=nullptr),
      fill_solid(CRGB* leds, CRGB c),
      fill_circle(CRGB* leds, uint16_t cx, uint16_t cy, uint8_t radius, CRGB c),
      fadeToBlackBy(CRGB* leds, uint8_t fadeBy),
      nscale8(CRGB* leds, uint8_t scale),
      setPixels(CRGB* leds),
      drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, CRGB c, CRGB *leds = nullptr),
      drawCharacter(unsigned char chr, int16_t x, int16_t y, uint8_t w, uint8_t h, CRGB color, CRGB *leds = nullptr),
      wu_pixel(CRGB *leds, uint32_t x, uint32_t y, CRGB c);

    // outsmart the compiler :) by correctly overloading
    inline void setPixelColorXY(int x, int y, uint32_t c) { setPixelColorXY(x, y, byte(c>>16), byte(c>>8), byte(c), byte(c>>24)); }
    inline void setPixelColorXY(int x, int y, CRGB c)     { setPixelColorXY(x, y, c.red, c.green, c.blue, 0); }
    inline void setPixelColorXY(float x, float y, uint32_t c, bool aa=true) { setPixelColorXY(x, y, byte(c>>16), byte(c>>8), byte(c), byte(c>>24), aa); }
    inline void setPixelColorXY(float x, float y, CRGB c, bool aa=true)     { setPixelColorXY(x, y, c.red, c.green, c.blue, 0, aa); }
    inline void drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint32_t c) { drawLine(x0, y0, x1, y1, CRGB(byte(c>>16), byte(c>>8), byte(c))); }
    inline void drawCharacter(unsigned char chr, int16_t x, int16_t y, uint8_t w, uint8_t h, uint32_t c) { drawCharacter(chr, x, y, w, h, CRGB(byte(c>>16), byte(c>>8), byte(c))); }

    uint16_t
      XY(uint16_t, uint16_t),
      get2DPixelIndex(uint16_t x, uint16_t y, uint8_t seg=255);

    uint32_t
      getPixelColorXY(uint16_t, uint16_t);

  // end 2D support

    CRGBPalette16 currentPalette;
    CRGBPalette16 targetPalette;

  private:
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

    uint8_t _modeCount = MODE_COUNT;
    // TODO: allocate memory using new or malloc()
    mode_ptr _mode[MODE_COUNT]; // SRAM footprint: 4 bytes per element
    const char *_modeData[MODE_COUNT]; // mode (effect) name and its slider control data array

    show_callback _callback = nullptr;

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
      // start, stop, offset, speed, intensity, palette, mode, options, grouping, spacing, opacity (unused), color[], capabilities, custom 1, custom 2, custom 3
      {0, 7, 0, DEFAULT_SPEED, DEFAULT_INTENSITY, 0, DEFAULT_MODE, NO_OPTIONS, 1, 0, 255, {DEFAULT_COLOR}, 0, DEFAULT_C1, DEFAULT_C2, DEFAULT_C3, 0, 1}
    };
    friend class Segment;

    segment_runtime _segment_runtimes[MAX_NUM_SEGMENTS]; // SRAM footprint: 28 bytes per element
    friend class Segment_runtime;

    ColorTransition transitions[MAX_NUM_TRANSITIONS]; //12 bytes per element
    friend class ColorTransition;

    void
      estimateCurrentAndLimitBri(void),
      load_gradient_palette(uint8_t),
      handle_palette(void);

    uint16_t
      transitionProgress(uint8_t tNr);
};

extern const char JSON_mode_names[];
extern const char JSON_palette_names[];

#endif
