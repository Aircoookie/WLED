//#define RGBW
#define PIN 2 //strip pin. Only change for ESP32
#define WORKAROUND_ESP32_BITBANG
//see https://github.com/Aircoookie/WLED/issues/2 for flicker free ESP32 support

//automatically uses the right driver method for each platform
#ifdef ARDUINO_ARCH_ESP32
#ifdef WORKAROUND_ESP32_BITBANG
#define PIXELMETHOD NeoEsp32BitBangWs2813Method
#else
#define PIXELMETHOD NeoEsp32RmtWS2813_V3Method
#endif
#else
#define PIXELMETHOD NeoEsp8266Uart800KbpsMethod
#endif

//selects correct feature for RGB/RGBW
#ifdef RGBW
#define PIXELFEATURE NeoGrbwFeature
#else
#define PIXELFEATURE NeoGrbFeature
#endif

/*
  WS2812FX.h - Library for WS2812 LED effects.
  
  Harm Aldick - 2016
  www.aldick.org
  FEATURES
    * A lot of blinken modes and counting
    * WS2812FX can be used as drop-in replacement for Adafruit Neopixel Library
  NOTES
    * Uses the Adafruit Neopixel library. Get it here: 
      https://github.com/adafruit/Adafruit_NeoPixel
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
  Modified to work with WLED - differs from Github WS2812FX
*/

#ifndef WS2812FX_h
#define WS2812FX_h

#include "Arduino.h"
#include <NeoPixelBrightnessBus.h>

#define DEFAULT_BRIGHTNESS 50
#define DEFAULT_MODE 0
#define DEFAULT_SPEED 150
#define DEFAULT_COLOR 0xFFAA00

#define SPEED_MIN 0
#define SPEED_MAX 255

#define BRIGHTNESS_MIN 0
#define BRIGHTNESS_MAX 255

#define MODE_COUNT 58

#define FX_MODE_STATIC                   0
#define FX_MODE_BLINK                    1
#define FX_MODE_BREATH                   2
#define FX_MODE_COLOR_WIPE               3
#define FX_MODE_COLOR_WIPE_RANDOM        4
#define FX_MODE_RANDOM_COLOR             5
#define FX_MODE_SINGLE_DYNAMIC           6
#define FX_MODE_MULTI_DYNAMIC            7
#define FX_MODE_RAINBOW                  8
#define FX_MODE_RAINBOW_CYCLE            9
#define FX_MODE_SCAN                    10
#define FX_MODE_DUAL_SCAN               11
#define FX_MODE_FADE                    12
#define FX_MODE_THEATER_CHASE           13
#define FX_MODE_THEATER_CHASE_RAINBOW   14
#define FX_MODE_RUNNING_LIGHTS          15
#define FX_MODE_TWINKLE                 16
#define FX_MODE_TWINKLE_RANDOM          17
#define FX_MODE_TWINKLE_FADE            18
#define FX_MODE_TWINKLE_FADE_RANDOM     19
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
#define FX_MODE_FIREWORKS_RANDOM        43
#define FX_MODE_MERRY_CHRISTMAS         44
#define FX_MODE_FIRE_FLICKER            45
#define FX_MODE_GRADIENT                46
#define FX_MODE_LOADING                 47
#define FX_MODE_DUAL_COLOR_WIPE_IN_OUT  48
#define FX_MODE_DUAL_COLOR_WIPE_IN_IN   49
#define FX_MODE_DUAL_COLOR_WIPE_OUT_OUT 50
#define FX_MODE_DUAL_COLOR_WIPE_OUT_IN  51
#define FX_MODE_CIRCUS_COMBUSTUS        52
#define FX_MODE_CUSTOM_CHASE            53
#define FX_MODE_CC_ON_RAINBOW           54
#define FX_MODE_CC_ON_RAINBOW_CYCLE     55
#define FX_MODE_CC_BLINK                56
#define FX_MODE_CC_RANDOM               57


class WS2812FX : public NeoPixelBrightnessBus<PIXELFEATURE, PIXELMETHOD> {
  typedef void (WS2812FX::*mode_ptr)(void);
  public:
    WS2812FX(uint16_t n) : NeoPixelBrightnessBus<PIXELFEATURE, PIXELMETHOD>(n,PIN) {

      _mode[FX_MODE_STATIC]                = &WS2812FX::mode_static;
      _mode[FX_MODE_BLINK]                 = &WS2812FX::mode_blink;
      _mode[FX_MODE_BREATH]                = &WS2812FX::mode_breath;
      _mode[FX_MODE_COLOR_WIPE]            = &WS2812FX::mode_color_wipe;
      _mode[FX_MODE_COLOR_WIPE_RANDOM]     = &WS2812FX::mode_color_wipe_random;
      _mode[FX_MODE_RANDOM_COLOR]          = &WS2812FX::mode_random_color;
      _mode[FX_MODE_SINGLE_DYNAMIC]        = &WS2812FX::mode_single_dynamic;
      _mode[FX_MODE_MULTI_DYNAMIC]         = &WS2812FX::mode_multi_dynamic;
      _mode[FX_MODE_RAINBOW]               = &WS2812FX::mode_rainbow;
      _mode[FX_MODE_RAINBOW_CYCLE]         = &WS2812FX::mode_rainbow_cycle;
      _mode[FX_MODE_SCAN]                  = &WS2812FX::mode_scan;
      _mode[FX_MODE_DUAL_SCAN]             = &WS2812FX::mode_dual_scan;
      _mode[FX_MODE_FADE]                  = &WS2812FX::mode_fade;
      _mode[FX_MODE_THEATER_CHASE]         = &WS2812FX::mode_theater_chase;
      _mode[FX_MODE_THEATER_CHASE_RAINBOW] = &WS2812FX::mode_theater_chase_rainbow;
      _mode[FX_MODE_RUNNING_LIGHTS]        = &WS2812FX::mode_running_lights;
      _mode[FX_MODE_TWINKLE]               = &WS2812FX::mode_twinkle;
      _mode[FX_MODE_TWINKLE_RANDOM]        = &WS2812FX::mode_twinkle_random;
      _mode[FX_MODE_TWINKLE_FADE]          = &WS2812FX::mode_twinkle_fade;
      _mode[FX_MODE_TWINKLE_FADE_RANDOM]   = &WS2812FX::mode_twinkle_fade_random;
      _mode[FX_MODE_SPARKLE]               = &WS2812FX::mode_sparkle;
      _mode[FX_MODE_FLASH_SPARKLE]         = &WS2812FX::mode_flash_sparkle;
      _mode[FX_MODE_HYPER_SPARKLE]         = &WS2812FX::mode_hyper_sparkle;
      _mode[FX_MODE_STROBE]                = &WS2812FX::mode_strobe;
      _mode[FX_MODE_STROBE_RAINBOW]        = &WS2812FX::mode_strobe_rainbow;
      _mode[FX_MODE_MULTI_STROBE]          = &WS2812FX::mode_multi_strobe;
      _mode[FX_MODE_BLINK_RAINBOW]         = &WS2812FX::mode_blink_rainbow;
      _mode[FX_MODE_ANDROID]               = &WS2812FX::mode_android;
      _mode[FX_MODE_CHASE_COLOR]           = &WS2812FX::mode_chase_color;
      _mode[FX_MODE_CHASE_RANDOM]          = &WS2812FX::mode_chase_random;
      _mode[FX_MODE_CHASE_RAINBOW]         = &WS2812FX::mode_chase_rainbow;
      _mode[FX_MODE_CHASE_FLASH]           = &WS2812FX::mode_chase_flash;
      _mode[FX_MODE_CHASE_FLASH_RANDOM]    = &WS2812FX::mode_chase_flash_random;
      _mode[FX_MODE_CHASE_RAINBOW_WHITE]   = &WS2812FX::mode_chase_rainbow_white;
      _mode[FX_MODE_COLORFUL]              = &WS2812FX::mode_colorful;
      _mode[FX_MODE_TRAFFIC_LIGHT]         = &WS2812FX::mode_traffic_light;
      _mode[FX_MODE_COLOR_SWEEP_RANDOM]    = &WS2812FX::mode_color_sweep_random;
      _mode[FX_MODE_RUNNING_COLOR]         = &WS2812FX::mode_running_color;
      _mode[FX_MODE_RUNNING_RED_BLUE]      = &WS2812FX::mode_running_red_blue;
      _mode[FX_MODE_RUNNING_RANDOM]        = &WS2812FX::mode_running_random;
      _mode[FX_MODE_LARSON_SCANNER]        = &WS2812FX::mode_larson_scanner;
      _mode[FX_MODE_COMET]                 = &WS2812FX::mode_comet;
      _mode[FX_MODE_FIREWORKS]             = &WS2812FX::mode_fireworks;
      _mode[FX_MODE_FIREWORKS_RANDOM]      = &WS2812FX::mode_fireworks_random;
      _mode[FX_MODE_MERRY_CHRISTMAS]       = &WS2812FX::mode_merry_christmas;
      _mode[FX_MODE_FIRE_FLICKER]          = &WS2812FX::mode_fire_flicker;
      _mode[FX_MODE_GRADIENT]              = &WS2812FX::mode_gradient;
      _mode[FX_MODE_LOADING]               = &WS2812FX::mode_loading;
      _mode[FX_MODE_DUAL_COLOR_WIPE_IN_OUT]  = &WS2812FX::mode_dual_color_wipe_in_out;
      _mode[FX_MODE_DUAL_COLOR_WIPE_IN_IN]   = &WS2812FX::mode_dual_color_wipe_in_in;
      _mode[FX_MODE_DUAL_COLOR_WIPE_OUT_OUT] = &WS2812FX::mode_dual_color_wipe_out_out;
      _mode[FX_MODE_DUAL_COLOR_WIPE_OUT_IN]  = &WS2812FX::mode_dual_color_wipe_out_in;
      _mode[FX_MODE_CIRCUS_COMBUSTUS]        = &WS2812FX::mode_circus_combustus;
      _mode[FX_MODE_CUSTOM_CHASE]            = &WS2812FX::mode_cc_standard;
      _mode[FX_MODE_CC_ON_RAINBOW]           = &WS2812FX::mode_cc_rainbow;
      _mode[FX_MODE_CC_ON_RAINBOW_CYCLE]     = &WS2812FX::mode_cc_cycle;
      _mode[FX_MODE_CC_BLINK]                = &WS2812FX::mode_cc_blink;
      _mode[FX_MODE_CC_RANDOM]               = &WS2812FX::mode_cc_random;

      _mode_index = DEFAULT_MODE;
      _speed = DEFAULT_SPEED;
      _brightness = DEFAULT_BRIGHTNESS;
      _running = false;
      _led_count = n;
      _mode_last_call_time = 0;
      _mode_delay = 0;
      _color = DEFAULT_COLOR;
      _mode_color = DEFAULT_COLOR;
      _color_sec = 0;
      _mode_var1 = 0;
      _cc_fs = true;
      _cc_fe = false;
      _cc_is = 0;
      _cc_i1 = 0;
      _cc_i2 = n-1;
      _cc_num1 = 5;
      _cc_num2 = 5;
      _ccStep = 1;
      _counter_mode_call = 0;
      _counter_mode_step = 0;
      _counter_ccStep = 0;
      _fastStandard = false;
      _reverseMode = false;
      _locked = new bool[n];
      _cronixieDigits = new byte[6];
    }

    void
      show(void),
      setPixelColor(uint16_t i, byte r, byte g, byte b),
      init(void),
      service(void),
      start(void),
      stop(void),
      setMode(byte m),
      setCustomChase(byte i1, byte i2, byte is, byte np, byte ns, byte stp, bool fs, bool fe),
      setCCIndex1(byte i1),
      setCCIndex2(byte i2),
      setCCStart(byte is),
      setCCNum1(byte np),
      setCCNum2(byte ns),
      setCCStep(byte stp),
      setCCFS(bool fs),
      setCCFE(bool fe),
      setSpeed(byte s),
      setIntensity(byte in),
      increaseSpeed(byte s),
      decreaseSpeed(byte s),
      setColor(byte r, byte g, byte b),
      setColor(byte r, byte g, byte b, byte w),
      setColor(uint32_t c),
      setSecondaryColor(byte r, byte g, byte b),
      setSecondaryColor(byte r, byte g, byte b, byte w),
      setSecondaryColor(uint32_t c),
      setBrightness(byte b),
      increaseBrightness(byte s),
      decreaseBrightness(byte s),
      setReverseMode(bool b),
      driverModeCronixie(bool b),
      setCronixieDigits(byte* d),
      setCronixieBacklight(bool b),
      setIndividual(int i),
      setIndividual(int i, uint32_t col),
      setRange(int i, int i2),
      setRange(int i, int i2, uint32_t col),
      lock(int i),
      lockRange(int i, int i2),
      lockAll(void),
      unlock(int i),
      unlockRange(int i, int i2),
      unlockAll(void),
      setFastUpdateMode(bool b),
      trigger(void),
      setLedCount(uint16_t i),
      setFade(int sp);

    bool 
      isRunning(void),
      isLocked(int i);

    byte
      get_random_wheel_index(byte),
      getMode(void),
      getSpeed(void),
      getIntensity(void),
      getBrightness(void),
      getModeCount(void);

    uint32_t
      color_wheel(byte),
      getColor(void);

    double
      getPowerEstimate(byte leds, uint32_t c, byte b),
      getSafePowerMultiplier(double safeMilliAmps, byte leds, uint32_t c, byte b);

  private:

    void
      begin(void),
      clear(void),
      setPixelColor(uint16_t i, uint32_t c),
      setPixelColor(uint16_t i, byte r, byte g, byte b, byte w),
      setPixelColorRaw(uint16_t i, byte r, byte g, byte b, byte w),
      dofade(void),
      strip_off(void),
      strip_off_respectLock(void),
      mode_static(void),
      mode_blink(void),
      mode_color_wipe(void),
      mode_color_wipe_random(void),
      mode_random_color(void),
      mode_single_dynamic(void),
      mode_multi_dynamic(void),
      mode_breath(void),
      mode_fade(void),
      mode_scan(void),
      mode_dual_scan(void),
      mode_theater_chase(void),
      mode_theater_chase_rainbow(void),
      mode_rainbow(void),
      mode_rainbow_cycle(void),
      mode_running_lights(void),
      mode_twinkle(void),
      mode_twinkle_random(void),
      mode_twinkle_fade(void),
      mode_twinkle_fade_random(void),
      mode_sparkle(void),
      mode_flash_sparkle(void),
      mode_hyper_sparkle(void),
      mode_strobe(void),
      mode_strobe_rainbow(void),
      mode_multi_strobe(void),
      mode_blink_rainbow(void),
      mode_android(void),
      mode_chase_color(void),
      mode_chase_random(void),
      mode_chase_rainbow(void),
      mode_chase_flash(void),
      mode_chase_flash_random(void),
      mode_chase_rainbow_white(void),
      mode_colorful(void),
      mode_traffic_light(void),
      mode_color_sweep_random(void),
      mode_running_color(void),
      mode_running_red_blue(void),
      mode_running_random(void),
      mode_larson_scanner(void),
      mode_comet(void),
      mode_fireworks(void),
      mode_fireworks_random(void),
      mode_merry_christmas(void),
      mode_fire_flicker(void),
      mode_gradient(void),
      mode_loading(void),
      mode_dual_color_wipe_in_out(void),
      mode_dual_color_wipe_in_in(void),
      mode_dual_color_wipe_out_out(void),
      mode_dual_color_wipe_out_in(void),
      mode_circus_combustus(void),
      mode_cc_core(void),
      mode_cc_standard(void),
      mode_cc_rainbow(void),
      mode_cc_cycle(void),
      mode_cc_blink(void),
      mode_cc_random(void);

    bool
      _triggered,
      _fastStandard,
      _reverseMode,
      _cronixieMode,
      _cronixieBacklightEnabled,
      _cc_fs,
      _cc_fe,
      _running;

    bool*
      _locked;

    byte
      minval(byte v, byte w),
      maxval(byte v, byte w),
      _mode_index,
      _speed,
      _intensity,
      _cc_i1,
      _cc_i2,
      _cc_is,
      _cc_num1,
      _cc_num2,
      _ccStep,
      _brightness;

    byte*
      _cronixieDigits;

    uint16_t
      _led_count;

    uint32_t
      getPixelColor(uint16_t i),
      _color,
      _color_sec,
      _counter_mode_call,
      _counter_mode_step,
      _counter_ccStep,
      _mode_var1,
      _mode_color,
      _mode_delay;

    double
      _cronixieSecMultiplier;

    unsigned long
      _mode_last_call_time;

    mode_ptr
      _mode[MODE_COUNT];
};

#endif
