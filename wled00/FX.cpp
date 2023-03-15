/*
  WS2812FX.cpp contains all effect methods
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

  Modified heavily for WLED
*/

#include "wled.h"
#include "FX.h"
#include "fcn_declare.h"

#define IBN 5100

// paletteBlend: 0 - wrap when moving, 1 - always wrap, 2 - never wrap, 3 - none (undefined)
#define PALETTE_SOLID_WRAP   (strip.paletteBlend == 1 || strip.paletteBlend == 3)
#define PALETTE_MOVING_WRAP !(strip.paletteBlend == 2 || (strip.paletteBlend == 0 && SEGMENT.speed == 0))

#define indexToVStrip(index, stripNr) ((index) | (int((stripNr)+1)<<16))

// effect utility functions
uint8_t sin_gap(uint16_t in) {
  if (in & 0x100) return 0;
  return sin8(in + 192); // correct phase shift of sine so that it starts and stops at 0
}

uint16_t triwave16(uint16_t in) {
  if (in < 0x8000) return in *2;
  return 0xFFFF - (in - 0x8000)*2;
}

/*
 * Generates a tristate square wave w/ attac & decay
 * @param x input value 0-255
 * @param pulsewidth 0-127
 * @param attdec attac & decay, max. pulsewidth / 2
 * @returns signed waveform value
 */
int8_t tristate_square8(uint8_t x, uint8_t pulsewidth, uint8_t attdec) {
  int8_t a = 127;
  if (x > 127) {
    a = -127;
    x -= 127;
  }

  if (x < attdec) { //inc to max
    return (int16_t) x * a / attdec;
  }
  else if (x < pulsewidth - attdec) { //max
    return a;
  }
  else if (x < pulsewidth) { //dec to 0
    return (int16_t) (pulsewidth - x) * a / attdec;
  }
  return 0;
}

// effect functions

/*
 * No blinking. Just plain old static light.
 */
uint16_t mode_static(void) {
  SEGMENT.fill(SEGCOLOR(0));
  return FRAMETIME; //we need the target fps even for static mode to get consistent timing
}
static const char _data_FX_MODE_STATIC[] PROGMEM = "Solid";


/*
 * Blink/strobe function
 * Alternate between color1 and color2
 * if(strobe == true) then create a strobe effect
 */
uint16_t blink(uint32_t color1, uint32_t color2, bool strobe, bool do_palette) {
  uint32_t cycleTime = (255 - SEGMENT.speed)*20;
  uint32_t onTime = FRAMETIME;
  if (!strobe) onTime += ((cycleTime * SEGMENT.intensity) >> 8);
  cycleTime += FRAMETIME*2;
  uint32_t it = strip.now / cycleTime;
  uint32_t rem = strip.now % cycleTime;

  bool on = false;
  if (it != SEGENV.step //new iteration, force on state for one frame, even if set time is too brief
      || rem <= onTime) {
    on = true;
  }

  SEGENV.step = it; //save previous iteration

  uint32_t color = on ? color1 : color2;
  if (color == color1 && do_palette)
  {
    for (int i = 0; i < SEGLEN; i++) {
      SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));
    }
  } else SEGMENT.fill(color);

  return FRAMETIME;
}


/*
 * Normal blinking. Intensity sets duty cycle.
 */
uint16_t mode_blink(void) {
  return blink(SEGCOLOR(0), SEGCOLOR(1), false, true);
}
static const char _data_FX_MODE_BLINK[] PROGMEM = "Blink@!,Duty cycle;!,!;!";


/*
 * Classic Blink effect. Cycling through the rainbow.
 */
uint16_t mode_blink_rainbow(void) {
  return blink(SEGMENT.color_wheel(SEGENV.call & 0xFF), SEGCOLOR(1), false, false);
}
static const char _data_FX_MODE_BLINK_RAINBOW[] PROGMEM = "Blink Rainbow@Frequency,Blink duration;!,!;!";


/*
 * Classic Strobe effect.
 */
uint16_t mode_strobe(void) {
  return blink(SEGCOLOR(0), SEGCOLOR(1), true, true);
}
static const char _data_FX_MODE_STROBE[] PROGMEM = "Strobe@!;!,!;!";


/*
 * Classic Strobe effect. Cycling through the rainbow.
 */
uint16_t mode_strobe_rainbow(void) {
  return blink(SEGMENT.color_wheel(SEGENV.call & 0xFF), SEGCOLOR(1), true, false);
}
static const char _data_FX_MODE_STROBE_RAINBOW[] PROGMEM = "Strobe Rainbow@!;,!;!";


/*
 * Color wipe function
 * LEDs are turned on (color1) in sequence, then turned off (color2) in sequence.
 * if (bool rev == true) then LEDs are turned off in reverse order
 */
uint16_t color_wipe(bool rev, bool useRandomColors) {
  uint32_t cycleTime = 750 + (255 - SEGMENT.speed)*150;
  uint32_t perc = strip.now % cycleTime;
  uint16_t prog = (perc * 65535) / cycleTime;
  bool back = (prog > 32767);
  if (back) {
    prog -= 32767;
    if (SEGENV.step == 0) SEGENV.step = 1;
  } else {
    if (SEGENV.step == 2) SEGENV.step = 3; //trigger color change
  }

  if (useRandomColors) {
    if (SEGENV.call == 0) {
      SEGENV.aux0 = random8();
      SEGENV.step = 3;
    }
    if (SEGENV.step == 1) { //if flag set, change to new random color
      SEGENV.aux1 = SEGMENT.get_random_wheel_index(SEGENV.aux0);
      SEGENV.step = 2;
    }
    if (SEGENV.step == 3) {
      SEGENV.aux0 = SEGMENT.get_random_wheel_index(SEGENV.aux1);
      SEGENV.step = 0;
    }
  }

  uint16_t ledIndex = (prog * SEGLEN) >> 15;
  uint16_t rem = 0;
  rem = (prog * SEGLEN) * 2; //mod 0xFFFF
  rem /= (SEGMENT.intensity +1);
  if (rem > 255) rem = 255;

  uint32_t col1 = useRandomColors? SEGMENT.color_wheel(SEGENV.aux1) : SEGCOLOR(1);
  for (int i = 0; i < SEGLEN; i++)
  {
    uint16_t index = (rev && back)? SEGLEN -1 -i : i;
    uint32_t col0 = useRandomColors? SEGMENT.color_wheel(SEGENV.aux0) : SEGMENT.color_from_palette(index, true, PALETTE_SOLID_WRAP, 0);

    if (i < ledIndex)
    {
      SEGMENT.setPixelColor(index, back? col1 : col0);
    } else
    {
      SEGMENT.setPixelColor(index, back? col0 : col1);
      if (i == ledIndex) SEGMENT.setPixelColor(index, color_blend(back? col0 : col1, back? col1 : col0, rem));
    }
  }
  return FRAMETIME;
}


/*
 * Lights all LEDs one after another.
 */
uint16_t mode_color_wipe(void) {
  return color_wipe(false, false);
}
static const char _data_FX_MODE_COLOR_WIPE[] PROGMEM = "Wipe@!,!;!,!;!";


/*
 * Lights all LEDs one after another. Turns off opposite
 */
uint16_t mode_color_sweep(void) {
  return color_wipe(true, false);
}
static const char _data_FX_MODE_COLOR_SWEEP[] PROGMEM = "Sweep@!,!;!,!;!";


/*
 * Turns all LEDs after each other to a random color.
 * Then starts over with another color.
 */
uint16_t mode_color_wipe_random(void) {
  return color_wipe(false, true);
}
static const char _data_FX_MODE_COLOR_WIPE_RANDOM[] PROGMEM = "Wipe Random@!;;!";


/*
 * Random color introduced alternating from start and end of strip.
 */
uint16_t mode_color_sweep_random(void) {
  return color_wipe(true, true);
}
static const char _data_FX_MODE_COLOR_SWEEP_RANDOM[] PROGMEM = "Sweep Random@!;;!";


/*
 * Lights all LEDs up in one random color. Then switches them
 * to the next random color.
 */
uint16_t mode_random_color(void) {
  uint32_t cycleTime = 200 + (255 - SEGMENT.speed)*50;
  uint32_t it = strip.now / cycleTime;
  uint32_t rem = strip.now % cycleTime;
  uint16_t fadedur = (cycleTime * SEGMENT.intensity) >> 8;

  uint32_t fade = 255;
  if (fadedur) {
    fade = (rem * 255) / fadedur;
    if (fade > 255) fade = 255;
  }

  if (SEGENV.call == 0) {
    SEGENV.aux0 = random8();
    SEGENV.step = 2;
  }
  if (it != SEGENV.step) //new color
  {
    SEGENV.aux1 = SEGENV.aux0;
    SEGENV.aux0 = SEGMENT.get_random_wheel_index(SEGENV.aux0); //aux0 will store our random color wheel index
    SEGENV.step = it;
  }

  SEGMENT.fill(color_blend(SEGMENT.color_wheel(SEGENV.aux1), SEGMENT.color_wheel(SEGENV.aux0), fade));
  return FRAMETIME;
}
static const char _data_FX_MODE_RANDOM_COLOR[] PROGMEM = "Random Colors@!,Fade time;;!";


/*
 * Lights every LED in a random color. Changes all LED at the same time
 * to new random colors.
 */
uint16_t mode_dynamic(void) {
  if (!SEGENV.allocateData(SEGLEN)) return mode_static(); //allocation failed

  if(SEGENV.call == 0) {
    //SEGMENT.setUpLeds();  //lossless getPixelColor()
    //SEGMENT.fill(BLACK);
    for (int i = 0; i < SEGLEN; i++) SEGENV.data[i] = random8();
  }

  uint32_t cycleTime = 50 + (255 - SEGMENT.speed)*15;
  uint32_t it = strip.now / cycleTime;
  if (it != SEGENV.step && SEGMENT.speed != 0) //new color
  {
    for (int i = 0; i < SEGLEN; i++) {
      if (random8() <= SEGMENT.intensity) SEGENV.data[i] = random8(); // random color index
    }
    SEGENV.step = it;
  }

  if (SEGMENT.check1) {
    for (int i = 0; i < SEGLEN; i++) {
      SEGMENT.blendPixelColor(i, SEGMENT.color_wheel(SEGENV.data[i]), 16);
    }
  } else {
    for (int i = 0; i < SEGLEN; i++) {
      SEGMENT.setPixelColor(i, SEGMENT.color_wheel(SEGENV.data[i]));
    }
  }
  return FRAMETIME;
}
static const char _data_FX_MODE_DYNAMIC[] PROGMEM = "Dynamic@!,!,,,,Smooth;;!";


/*
 * effect "Dynamic" with smooth color-fading
 */
uint16_t mode_dynamic_smooth(void) {
  bool old = SEGMENT.check1;
  SEGMENT.check1 = true;
  mode_dynamic();
  SEGMENT.check1 = old;
  return FRAMETIME;
 }
static const char _data_FX_MODE_DYNAMIC_SMOOTH[] PROGMEM = "Dynamic Smooth@!,!;;!";


/*
 * Does the "standby-breathing" of well known i-Devices.
 */
uint16_t mode_breath(void) {
  uint16_t var = 0;
  uint16_t counter = (strip.now * ((SEGMENT.speed >> 3) +10));
  counter = (counter >> 2) + (counter >> 4); //0-16384 + 0-2048
  if (counter < 16384) {
    if (counter > 8192) counter = 8192 - (counter - 8192);
    var = sin16(counter) / 103; //close to parabolic in range 0-8192, max val. 23170
  }

  uint8_t lum = 30 + var;
  for (int i = 0; i < SEGLEN; i++) {
    SEGMENT.setPixelColor(i, color_blend(SEGCOLOR(1), SEGMENT.color_from_palette(i, true, PALETTE_SOLID_WRAP, 0), lum));
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_BREATH[] PROGMEM = "Breathe@!;!,!;!";


/*
 * Fades the LEDs between two colors
 */
uint16_t mode_fade(void) {
  uint16_t counter = (strip.now * ((SEGMENT.speed >> 3) +10));
  uint8_t lum = triwave16(counter) >> 8;

  for (int i = 0; i < SEGLEN; i++) {
    SEGMENT.setPixelColor(i, color_blend(SEGCOLOR(1), SEGMENT.color_from_palette(i, true, PALETTE_SOLID_WRAP, 0), lum));
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_FADE[] PROGMEM = "Fade@!;!,!;!";


/*
 * Scan mode parent function
 */
uint16_t scan(bool dual)
{
  uint32_t cycleTime = 750 + (255 - SEGMENT.speed)*150;
  uint32_t perc = strip.now % cycleTime;
  uint16_t prog = (perc * 65535) / cycleTime;
  uint16_t size = 1 + ((SEGMENT.intensity * SEGLEN) >> 9);
  uint16_t ledIndex = (prog * ((SEGLEN *2) - size *2)) >> 16;

  if (!SEGMENT.check2) SEGMENT.fill(SEGCOLOR(1));

  int led_offset = ledIndex - (SEGLEN - size);
  led_offset = abs(led_offset);

  if (dual) {
    for (int j = led_offset; j < led_offset + size; j++) {
      uint16_t i2 = SEGLEN -1 -j;
      SEGMENT.setPixelColor(i2, SEGMENT.color_from_palette(i2, true, PALETTE_SOLID_WRAP, (SEGCOLOR(2))? 2:0));
    }
  }

  for (int j = led_offset; j < led_offset + size; j++) {
    SEGMENT.setPixelColor(j, SEGMENT.color_from_palette(j, true, PALETTE_SOLID_WRAP, 0));
  }

  return FRAMETIME;
}


/*
 * Runs a single pixel back and forth.
 */
uint16_t mode_scan(void) {
  return scan(false);
}
static const char _data_FX_MODE_SCAN[] PROGMEM = "Scan@!,# of dots,,,,,Overlay;!,!,!;!";


/*
 * Runs two pixel back and forth in opposite directions.
 */
uint16_t mode_dual_scan(void) {
  return scan(true);
}
static const char _data_FX_MODE_DUAL_SCAN[] PROGMEM = "Scan Dual@!,# of dots,,,,,Overlay;!,!,!;!";


/*
 * Cycles all LEDs at once through a rainbow.
 */
uint16_t mode_rainbow(void) {
  uint16_t counter = (strip.now * ((SEGMENT.speed >> 2) +2)) & 0xFFFF;
  counter = counter >> 8;

  if (SEGMENT.intensity < 128){
    SEGMENT.fill(color_blend(SEGMENT.color_wheel(counter),WHITE,128-SEGMENT.intensity));
  } else {
    SEGMENT.fill(SEGMENT.color_wheel(counter));
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_RAINBOW[] PROGMEM = "Colorloop@!,Saturation;;!";


/*
 * Cycles a rainbow over the entire string of LEDs.
 */
uint16_t mode_rainbow_cycle(void) {
  uint16_t counter = (strip.now * ((SEGMENT.speed >> 2) +2)) & 0xFFFF;
  counter = counter >> 8;

  for (int i = 0; i < SEGLEN; i++) {
    //intensity/29 = 0 (1/16) 1 (1/8) 2 (1/4) 3 (1/2) 4 (1) 5 (2) 6 (4) 7 (8) 8 (16)
    uint8_t index = (i * (16 << (SEGMENT.intensity /29)) / SEGLEN) + counter;
    SEGMENT.setPixelColor(i, SEGMENT.color_wheel(index));
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_RAINBOW_CYCLE[] PROGMEM = "Rainbow@!,Size;;!";


/*
 * Alternating pixels running function.
 */
uint16_t running(uint32_t color1, uint32_t color2, bool theatre = false) {
  uint8_t width = (theatre ? 3 : 1) + (SEGMENT.intensity >> 4);  // window
  uint32_t cycleTime = 50 + (255 - SEGMENT.speed);
  uint32_t it = strip.now / cycleTime;
  bool usePalette = color1 == SEGCOLOR(0);

  for (int i = 0; i < SEGLEN; i++) {
    uint32_t col = color2;
    if (usePalette) color1 = SEGMENT.color_from_palette(i, true, PALETTE_SOLID_WRAP, 0);
    if (theatre) {
      if ((i % width) == SEGENV.aux0) col = color1;
    } else {
      int8_t pos = (i % (width<<1));
      if ((pos < SEGENV.aux0-width) || ((pos >= SEGENV.aux0) && (pos < SEGENV.aux0+width))) col = color1;
    }
    SEGMENT.setPixelColor(i,col);
  }

  if (it != SEGENV.step) {
    SEGENV.aux0 = (SEGENV.aux0 +1) % (theatre ? width : (width<<1));
    SEGENV.step = it;
  }
  return FRAMETIME;
}


/*
 * Theatre-style crawling lights.
 * Inspired by the Adafruit examples.
 */
uint16_t mode_theater_chase(void) {
  return running(SEGCOLOR(0), SEGCOLOR(1), true);
}
static const char _data_FX_MODE_THEATER_CHASE[] PROGMEM = "Theater@!,Gap size;!,!;!";


/*
 * Theatre-style crawling lights with rainbow effect.
 * Inspired by the Adafruit examples.
 */
uint16_t mode_theater_chase_rainbow(void) {
  return running(SEGMENT.color_wheel(SEGENV.step), SEGCOLOR(1), true);
}
static const char _data_FX_MODE_THEATER_CHASE_RAINBOW[] PROGMEM = "Theater Rainbow@!,Gap size;,!;!";


/*
 * Running lights effect with smooth sine transition base.
 */
uint16_t running_base(bool saw, bool dual=false) {
  uint8_t x_scale = SEGMENT.intensity >> 2;
  uint32_t counter = (strip.now * SEGMENT.speed) >> 9;

  for (int i = 0; i < SEGLEN; i++) {
    uint16_t a = i*x_scale - counter;
    if (saw) {
      a &= 0xFF;
      if (a < 16)
      {
        a = 192 + a*8;
      } else {
        a = map(a,16,255,64,192);
      }
      a = 255 - a;
    }
    uint8_t s = dual ? sin_gap(a) : sin8(a);
    uint32_t ca = color_blend(SEGCOLOR(1), SEGMENT.color_from_palette(i, true, PALETTE_SOLID_WRAP, 0), s);
    if (dual) {
      uint16_t b = (SEGLEN-1-i)*x_scale - counter;
      uint8_t t = sin_gap(b);
      uint32_t cb = color_blend(SEGCOLOR(1), SEGMENT.color_from_palette(i, true, PALETTE_SOLID_WRAP, 2), t);
      ca = color_blend(ca, cb, 127);
    }
    SEGMENT.setPixelColor(i, ca);
  }

  return FRAMETIME;
}


/*
 * Running lights in opposite directions.
 * Idea: Make the gap width controllable with a third slider in the future
 */
uint16_t mode_running_dual(void) {
  return running_base(false, true);
}
static const char _data_FX_MODE_RUNNING_DUAL[] PROGMEM = "Running Dual@!,Wave width;L,!,R;!";


/*
 * Running lights effect with smooth sine transition.
 */
uint16_t mode_running_lights(void) {
  return running_base(false);
}
static const char _data_FX_MODE_RUNNING_LIGHTS[] PROGMEM = "Running@!,Wave width;!,!;!";


/*
 * Running lights effect with sawtooth transition.
 */
uint16_t mode_saw(void) {
  return running_base(true);
}
static const char _data_FX_MODE_SAW[] PROGMEM = "Saw@!,Width;!,!;!";


/*
 * Blink several LEDs in random colors on, reset, repeat.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
uint16_t mode_twinkle(void) {
  SEGMENT.fade_out(224);

  uint32_t cycleTime = 20 + (255 - SEGMENT.speed)*5;
  uint32_t it = strip.now / cycleTime;
  if (it != SEGENV.step)
  {
    uint16_t maxOn = map(SEGMENT.intensity, 0, 255, 1, SEGLEN); // make sure at least one LED is on
    if (SEGENV.aux0 >= maxOn)
    {
      SEGENV.aux0 = 0;
      SEGENV.aux1 = random16(); //new seed for our PRNG
    }
    SEGENV.aux0++;
    SEGENV.step = it;
  }

  uint16_t PRNG16 = SEGENV.aux1;

  for (uint16_t i = 0; i < SEGENV.aux0; i++)
  {
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 13849; // next 'random' number
    uint32_t p = (uint32_t)SEGLEN * (uint32_t)PRNG16;
    uint16_t j = p >> 16;
    SEGMENT.setPixelColor(j, SEGMENT.color_from_palette(j, true, PALETTE_SOLID_WRAP, 0));
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_TWINKLE[] PROGMEM = "Twinkle@!,!;!,!;!;;m12=0"; //pixels


/*
 * Dissolve function
 */
uint16_t dissolve(uint32_t color) {
  //bool wa = (SEGCOLOR(1) != 0 && strip.getBrightness() < 255); //workaround, can't compare getPixel to color if not full brightness
  if (SEGENV.call == 0) {
    SEGMENT.setUpLeds();  //lossless getPixelColor()
    SEGMENT.fill(SEGCOLOR(1));
  }

  for (int j = 0; j <= SEGLEN / 15; j++) {
    if (random8() <= SEGMENT.intensity) {
      for (size_t times = 0; times < 10; times++) //attempt to spawn a new pixel 10 times
      {
        uint16_t i = random16(SEGLEN);
        if (SEGENV.aux0) { //dissolve to primary/palette
          if (SEGMENT.getPixelColor(i) == SEGCOLOR(1) /*|| wa*/) {
            if (color == SEGCOLOR(0)) {
              SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));
            } else {
              SEGMENT.setPixelColor(i, color);
            }
            break; //only spawn 1 new pixel per frame per 50 LEDs
          }
        } else { //dissolve to secondary
          if (SEGMENT.getPixelColor(i) != SEGCOLOR(1)) { SEGMENT.setPixelColor(i, SEGCOLOR(1)); break; }
        }
      }
    }
  }

  if (SEGENV.step > (255 - SEGMENT.speed) + 15U) {
    SEGENV.aux0 = !SEGENV.aux0;
    SEGENV.step = 0;
  } else {
    SEGENV.step++;
  }

  return FRAMETIME;
}


/*
 * Blink several LEDs on and then off
 */
uint16_t mode_dissolve(void) {
  return dissolve(SEGMENT.check1 ? SEGMENT.color_wheel(random8()) : SEGCOLOR(0));
}
static const char _data_FX_MODE_DISSOLVE[] PROGMEM = "Dissolve@Repeat speed,Dissolve speed,,,,Random;!,!;!";


/*
 * Blink several LEDs on and then off in random colors
 */
uint16_t mode_dissolve_random(void) {
  return dissolve(SEGMENT.color_wheel(random8()));
}
static const char _data_FX_MODE_DISSOLVE_RANDOM[] PROGMEM = "Dissolve Rnd@Repeat speed,Dissolve speed;,!;!";


/*
 * Blinks one LED at a time.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
uint16_t mode_sparkle(void) {
  if (!SEGMENT.check2) for(int i = 0; i < SEGLEN; i++) {
    SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(i, true, PALETTE_SOLID_WRAP, 1));
  }
  uint32_t cycleTime = 10 + (255 - SEGMENT.speed)*2;
  uint32_t it = strip.now / cycleTime;
  if (it != SEGENV.step)
  {
    SEGENV.aux0 = random16(SEGLEN); // aux0 stores the random led index
    SEGENV.step = it;
  }

  SEGMENT.setPixelColor(SEGENV.aux0, SEGCOLOR(0));
  return FRAMETIME;
}
static const char _data_FX_MODE_SPARKLE[] PROGMEM = "Sparkle@!,,,,,,Overlay;!,!;!;;m12=0";


/*
 * Lights all LEDs in the color. Flashes single col 1 pixels randomly. (List name: Sparkle Dark)
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
uint16_t mode_flash_sparkle(void) {
  if (!SEGMENT.check2) for(uint16_t i = 0; i < SEGLEN; i++) {
    SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));
  }

  if (strip.now - SEGENV.aux0 > SEGENV.step) {
    if(random8((255-SEGMENT.intensity) >> 4) == 0) {
      SEGMENT.setPixelColor(random16(SEGLEN), SEGCOLOR(1)); //flash
    }
    SEGENV.step = strip.now;
    SEGENV.aux0 = 255-SEGMENT.speed;
  }
  return FRAMETIME;
}
static const char _data_FX_MODE_FLASH_SPARKLE[] PROGMEM = "Sparkle Dark@!,!,,,,,Overlay;Bg,Fx;!;;m12=0";


/*
 * Like flash sparkle. With more flash.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
uint16_t mode_hyper_sparkle(void) {
  if (!SEGMENT.check2) for (int i = 0; i < SEGLEN; i++) {
    SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));
  }

  if (strip.now - SEGENV.aux0 > SEGENV.step) {
    if (random8((255-SEGMENT.intensity) >> 4) == 0) {
      for (int i = 0; i < MAX(1, SEGLEN/3); i++) {
        SEGMENT.setPixelColor(random16(SEGLEN), SEGCOLOR(1));
      }
    }
    SEGENV.step = strip.now;
    SEGENV.aux0 = 255-SEGMENT.speed;
  }
  return FRAMETIME;
}
static const char _data_FX_MODE_HYPER_SPARKLE[] PROGMEM = "Sparkle+@!,!,,,,,Overlay;Bg,Fx;!;;m12=0";


/*
 * Strobe effect with different strobe count and pause, controlled by speed.
 */
uint16_t mode_multi_strobe(void) {
  for (int i = 0; i < SEGLEN; i++) {
    SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(i, true, PALETTE_SOLID_WRAP, 1));
  }

  SEGENV.aux0 = 50 + 20*(uint16_t)(255-SEGMENT.speed);
  uint16_t count = 2 * ((SEGMENT.intensity / 10) + 1);
  if(SEGENV.aux1 < count) {
    if((SEGENV.aux1 & 1) == 0) {
      SEGMENT.fill(SEGCOLOR(0));
      SEGENV.aux0 = 15;
    } else {
      SEGENV.aux0 = 50;
    }
  }

  if (strip.now - SEGENV.aux0 > SEGENV.step) {
    SEGENV.aux1++;
    if (SEGENV.aux1 > count) SEGENV.aux1 = 0;
    SEGENV.step = strip.now;
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_MULTI_STROBE[] PROGMEM = "Strobe Mega@!,!;!,!;!";


/*
 * Android loading circle
 */
uint16_t mode_android(void) {

  for (int i = 0; i < SEGLEN; i++) {
    SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(i, true, PALETTE_SOLID_WRAP, 1));
  }

  if (SEGENV.aux1 > ((float)SEGMENT.intensity/255.0)*(float)SEGLEN)
  {
    SEGENV.aux0 = 1;
  } else
  {
    if (SEGENV.aux1 < 2) SEGENV.aux0 = 0;
  }

  uint16_t a = SEGENV.step;

  if (SEGENV.aux0 == 0)
  {
    if (SEGENV.call %3 == 1) {a++;}
    else {SEGENV.aux1++;}
  } else
  {
    a++;
    if (SEGENV.call %3 != 1) SEGENV.aux1--;
  }

  if (a >= SEGLEN) a = 0;

  if (a + SEGENV.aux1 < SEGLEN)
  {
    for (int i = a; i < a+SEGENV.aux1; i++) {
      SEGMENT.setPixelColor(i, SEGCOLOR(0));
    }
  } else
  {
    for (int i = a; i < SEGLEN; i++) {
      SEGMENT.setPixelColor(i, SEGCOLOR(0));
    }
    for (int i = 0; i < SEGENV.aux1 - (SEGLEN -a); i++) {
      SEGMENT.setPixelColor(i, SEGCOLOR(0));
    }
  }
  SEGENV.step = a;

  return 3 + ((8 * (uint32_t)(255 - SEGMENT.speed)) / SEGLEN);
}
static const char _data_FX_MODE_ANDROID[] PROGMEM = "Android@!,Width;!,!;!;;m12=1"; //vertical


/*
 * color chase function.
 * color1 = background color
 * color2 and color3 = colors of two adjacent leds
 */
uint16_t chase(uint32_t color1, uint32_t color2, uint32_t color3, bool do_palette) {
  uint16_t counter = strip.now * ((SEGMENT.speed >> 2) + 1);
  uint16_t a = counter * SEGLEN  >> 16;

  bool chase_random = (SEGMENT.mode == FX_MODE_CHASE_RANDOM);
  if (chase_random) {
    if (a < SEGENV.step) //we hit the start again, choose new color for Chase random
    {
      SEGENV.aux1 = SEGENV.aux0; //store previous random color
      SEGENV.aux0 = SEGMENT.get_random_wheel_index(SEGENV.aux0);
    }
    color1 = SEGMENT.color_wheel(SEGENV.aux0);
  }
  SEGENV.step = a;

  // Use intensity setting to vary chase up to 1/2 string length
  uint8_t size = 1 + (SEGMENT.intensity * SEGLEN >> 10);

  uint16_t b = a + size; //"trail" of chase, filled with color1
  if (b > SEGLEN) b -= SEGLEN;
  uint16_t c = b + size;
  if (c > SEGLEN) c -= SEGLEN;

  //background
  if (do_palette)
  {
    for (int i = 0; i < SEGLEN; i++) {
      SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(i, true, PALETTE_SOLID_WRAP, 1));
    }
  } else SEGMENT.fill(color1);

  //if random, fill old background between a and end
  if (chase_random)
  {
    color1 = SEGMENT.color_wheel(SEGENV.aux1);
    for (int i = a; i < SEGLEN; i++)
      SEGMENT.setPixelColor(i, color1);
  }

  //fill between points a and b with color2
  if (a < b)
  {
    for (int i = a; i < b; i++)
      SEGMENT.setPixelColor(i, color2);
  } else {
    for (int i = a; i < SEGLEN; i++) //fill until end
      SEGMENT.setPixelColor(i, color2);
    for (int i = 0; i < b; i++) //fill from start until b
      SEGMENT.setPixelColor(i, color2);
  }

  //fill between points b and c with color2
  if (b < c)
  {
    for (int i = b; i < c; i++)
      SEGMENT.setPixelColor(i, color3);
  } else {
    for (int i = b; i < SEGLEN; i++) //fill until end
      SEGMENT.setPixelColor(i, color3);
    for (int i = 0; i < c; i++) //fill from start until c
      SEGMENT.setPixelColor(i, color3);
  }

  return FRAMETIME;
}


/*
 * Bicolor chase, more primary color.
 */
uint16_t mode_chase_color(void) {
  return chase(SEGCOLOR(1), (SEGCOLOR(2)) ? SEGCOLOR(2) : SEGCOLOR(0), SEGCOLOR(0), true);
}
static const char _data_FX_MODE_CHASE_COLOR[] PROGMEM = "Chase@!,Width;!,!,!;!";


/*
 * Primary running followed by random color.
 */
uint16_t mode_chase_random(void) {
  return chase(SEGCOLOR(1), (SEGCOLOR(2)) ? SEGCOLOR(2) : SEGCOLOR(0), SEGCOLOR(0), false);
}
static const char _data_FX_MODE_CHASE_RANDOM[] PROGMEM = "Chase Random@!,Width;!,,!;!";


/*
 * Primary, secondary running on rainbow.
 */
uint16_t mode_chase_rainbow(void) {
  uint8_t color_sep = 256 / SEGLEN;
  if (color_sep == 0) color_sep = 1;                                           // correction for segments longer than 256 LEDs
  uint8_t color_index = SEGENV.call & 0xFF;
  uint32_t color = SEGMENT.color_wheel(((SEGENV.step * color_sep) + color_index) & 0xFF);

  return chase(color, SEGCOLOR(0), SEGCOLOR(1), false);
}
static const char _data_FX_MODE_CHASE_RAINBOW[] PROGMEM = "Chase Rainbow@!,Width;!,!;!";


/*
 * Primary running on rainbow.
 */
uint16_t mode_chase_rainbow_white(void) {
  uint16_t n = SEGENV.step;
  uint16_t m = (SEGENV.step + 1) % SEGLEN;
  uint32_t color2 = SEGMENT.color_wheel(((n * 256 / SEGLEN) + (SEGENV.call & 0xFF)) & 0xFF);
  uint32_t color3 = SEGMENT.color_wheel(((m * 256 / SEGLEN) + (SEGENV.call & 0xFF)) & 0xFF);

  return chase(SEGCOLOR(0), color2, color3, false);
}
static const char _data_FX_MODE_CHASE_RAINBOW_WHITE[] PROGMEM = "Rainbow Runner@!,Size;Bg;!";


/*
 * Red - Amber - Green - Blue lights running
 */
uint16_t mode_colorful(void) {
  uint8_t numColors = 4; //3, 4, or 5
  uint32_t cols[9]{0x00FF0000,0x00EEBB00,0x0000EE00,0x000077CC};
  if (SEGMENT.intensity > 160 || SEGMENT.palette) { //palette or color
    if (!SEGMENT.palette) {
      numColors = 3;
      for (size_t i = 0; i < 3; i++) cols[i] = SEGCOLOR(i);
    } else {
      uint16_t fac = 80;
      if (SEGMENT.palette == 52) {numColors = 5; fac = 61;} //C9 2 has 5 colors
      for (size_t i = 0; i < numColors; i++) {
        cols[i] = SEGMENT.color_from_palette(i*fac, false, true, 255);
      }
    }
  } else if (SEGMENT.intensity < 80) //pastel (easter) colors
  {
    cols[0] = 0x00FF8040;
    cols[1] = 0x00E5D241;
    cols[2] = 0x0077FF77;
    cols[3] = 0x0077F0F0;
  }
  for (size_t i = numColors; i < numColors*2 -1U; i++) cols[i] = cols[i-numColors];

  uint32_t cycleTime = 50 + (8 * (uint32_t)(255 - SEGMENT.speed));
  uint32_t it = strip.now / cycleTime;
  if (it != SEGENV.step)
  {
    if (SEGMENT.speed > 0) SEGENV.aux0++;
    if (SEGENV.aux0 >= numColors) SEGENV.aux0 = 0;
    SEGENV.step = it;
  }

  for (int i = 0; i < SEGLEN; i+= numColors)
  {
    for (int j = 0; j < numColors; j++) SEGMENT.setPixelColor(i + j, cols[SEGENV.aux0 + j]);
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_COLORFUL[] PROGMEM = "Colorful@!,Saturation;1,2,3;!";


/*
 * Emulates a traffic light.
 */
uint16_t mode_traffic_light(void) {
  for (int i=0; i < SEGLEN; i++)
    SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(i, true, PALETTE_SOLID_WRAP, 1));
  uint32_t mdelay = 500;
  for (int i = 0; i < SEGLEN-2 ; i+=3)
  {
    switch (SEGENV.aux0)
    {
      case 0: SEGMENT.setPixelColor(i, 0x00FF0000); mdelay = 150 + (100 * (uint32_t)(255 - SEGMENT.speed));break;
      case 1: SEGMENT.setPixelColor(i, 0x00FF0000); mdelay = 150 + (20 * (uint32_t)(255 - SEGMENT.speed)); SEGMENT.setPixelColor(i+1, 0x00EECC00); break;
      case 2: SEGMENT.setPixelColor(i+2, 0x0000FF00); mdelay = 150 + (100 * (uint32_t)(255 - SEGMENT.speed));break;
      case 3: SEGMENT.setPixelColor(i+1, 0x00EECC00); mdelay = 150 + (20 * (uint32_t)(255 - SEGMENT.speed));break;
    }
  }

  if (strip.now - SEGENV.step > mdelay)
  {
    SEGENV.aux0++;
    if (SEGENV.aux0 == 1 && SEGMENT.intensity > 140) SEGENV.aux0 = 2; //skip Red + Amber, to get US-style sequence
    if (SEGENV.aux0 > 3) SEGENV.aux0 = 0;
    SEGENV.step = strip.now;
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_TRAFFIC_LIGHT[] PROGMEM = "Traffic Light@!,US style;,!;!";


/*
 * Sec flashes running on prim.
 */
#define FLASH_COUNT 4
uint16_t mode_chase_flash(void) {
  uint8_t flash_step = SEGENV.call % ((FLASH_COUNT * 2) + 1);

  for (int i = 0; i < SEGLEN; i++) {
    SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));
  }

  uint16_t delay = 10 + ((30 * (uint16_t)(255 - SEGMENT.speed)) / SEGLEN);
  if(flash_step < (FLASH_COUNT * 2)) {
    if(flash_step % 2 == 0) {
      uint16_t n = SEGENV.step;
      uint16_t m = (SEGENV.step + 1) % SEGLEN;
      SEGMENT.setPixelColor( n, SEGCOLOR(1));
      SEGMENT.setPixelColor( m, SEGCOLOR(1));
      delay = 20;
    } else {
      delay = 30;
    }
  } else {
    SEGENV.step = (SEGENV.step + 1) % SEGLEN;
  }
  return delay;
}
static const char _data_FX_MODE_CHASE_FLASH[] PROGMEM = "Chase Flash@!;Bg,Fx;!";


/*
 * Prim flashes running, followed by random color.
 */
uint16_t mode_chase_flash_random(void) {
  uint8_t flash_step = SEGENV.call % ((FLASH_COUNT * 2) + 1);

  for (int i = 0; i < SEGENV.aux1; i++) {
    SEGMENT.setPixelColor(i, SEGMENT.color_wheel(SEGENV.aux0));
  }

  uint16_t delay = 1 + ((10 * (uint16_t)(255 - SEGMENT.speed)) / SEGLEN);
  if(flash_step < (FLASH_COUNT * 2)) {
    uint16_t n = SEGENV.aux1;
    uint16_t m = (SEGENV.aux1 + 1) % SEGLEN;
    if(flash_step % 2 == 0) {
      SEGMENT.setPixelColor( n, SEGCOLOR(0));
      SEGMENT.setPixelColor( m, SEGCOLOR(0));
      delay = 20;
    } else {
      SEGMENT.setPixelColor( n, SEGMENT.color_wheel(SEGENV.aux0));
      SEGMENT.setPixelColor( m, SEGCOLOR(1));
      delay = 30;
    }
  } else {
    SEGENV.aux1 = (SEGENV.aux1 + 1) % SEGLEN;

    if (SEGENV.aux1 == 0) {
      SEGENV.aux0 = SEGMENT.get_random_wheel_index(SEGENV.aux0);
    }
  }
  return delay;
}
static const char _data_FX_MODE_CHASE_FLASH_RANDOM[] PROGMEM = "Chase Flash Rnd@!;!,!;!";


/*
 * Alternating color/sec pixels running.
 */
uint16_t mode_running_color(void) {
  return running(SEGCOLOR(0), SEGCOLOR(1));
}
static const char _data_FX_MODE_RUNNING_COLOR[] PROGMEM = "Chase 2@!,Width;!,!;!";


/*
 * Random colored pixels running. ("Stream")
 */
uint16_t mode_running_random(void) {
  uint32_t cycleTime = 25 + (3 * (uint32_t)(255 - SEGMENT.speed));
  uint32_t it = strip.now / cycleTime;
  if (SEGENV.call == 0) SEGENV.aux0 = random16(); // random seed for PRNG on start

  uint8_t zoneSize = ((255-SEGMENT.intensity) >> 4) +1;
  uint16_t PRNG16 = SEGENV.aux0;

  uint8_t z = it % zoneSize;
  bool nzone = (!z && it != SEGENV.aux1);
  for (int i=SEGLEN-1; i > 0; i--) {
    if (nzone || z >= zoneSize) {
      uint8_t lastrand = PRNG16 >> 8;
      int16_t diff = 0;
      while (abs(diff) < 42) { // make sure the difference between adjacent colors is big enough
        PRNG16 = (uint16_t)(PRNG16 * 2053) + 13849; // next zone, next 'random' number
        diff = (PRNG16 >> 8) - lastrand;
      }
      if (nzone) {
        SEGENV.aux0 = PRNG16; // save next starting seed
        nzone = false;
      }
      z = 0;
    }
    SEGMENT.setPixelColor(i, SEGMENT.color_wheel(PRNG16 >> 8));
    z++;
  }

  SEGENV.aux1 = it;
  return FRAMETIME;
}
static const char _data_FX_MODE_RUNNING_RANDOM[] PROGMEM = "Stream@!,Zone size;;!";


uint16_t larson_scanner(bool dual) {
  uint16_t counter = strip.now * ((SEGMENT.speed >> 2) +8);
  uint16_t index = counter * SEGLEN  >> 16;

  SEGMENT.fade_out(SEGMENT.intensity);

  if (SEGENV.step > index && SEGENV.step - index > SEGLEN/2) {
    SEGENV.aux0 = !SEGENV.aux0;
  }

  for (int i = SEGENV.step; i < index; i++) {
    uint16_t j = (SEGENV.aux0)?i:SEGLEN-1-i;
    SEGMENT.setPixelColor( j, SEGMENT.color_from_palette(j, true, PALETTE_SOLID_WRAP, 0));
  }
  if (dual) {
    uint32_t c;
    if (SEGCOLOR(2) != 0) {
      c = SEGCOLOR(2);
    } else {
      c = SEGMENT.color_from_palette(index, true, PALETTE_SOLID_WRAP, 0);
    }

    for (int i = SEGENV.step; i < index; i++) {
      uint16_t j = (SEGENV.aux0)?SEGLEN-1-i:i;
      SEGMENT.setPixelColor(j, c);
    }
  }

  SEGENV.step = index;
  return FRAMETIME;
}


/*
 * K.I.T.T.
 */
uint16_t mode_larson_scanner(void){
  return larson_scanner(false);
}
static const char _data_FX_MODE_LARSON_SCANNER[] PROGMEM = "Scanner@!,Fade rate;!,!;!;;m12=0";


/*
 * Creates two Larson scanners moving in opposite directions
 * Custom mode by Keith Lord: https://github.com/kitesurfer1404/WS2812FX/blob/master/src/custom/DualLarson.h
 */
uint16_t mode_dual_larson_scanner(void){
  return larson_scanner(true);
}
static const char _data_FX_MODE_DUAL_LARSON_SCANNER[] PROGMEM = "Scanner Dual@!,Fade rate;!,!,!;!;;m12=0";


/*
 * Firing comets from one end. "Lighthouse"
 */
uint16_t mode_comet(void) {
  uint16_t counter = strip.now * ((SEGMENT.speed >>2) +1);
  uint16_t index = (counter * SEGLEN) >> 16;
  if (SEGENV.call == 0) SEGENV.aux0 = index;

  SEGMENT.fade_out(SEGMENT.intensity);

  SEGMENT.setPixelColor( index, SEGMENT.color_from_palette(index, true, PALETTE_SOLID_WRAP, 0));
  if (index > SEGENV.aux0) {
    for (int i = SEGENV.aux0; i < index ; i++) {
       SEGMENT.setPixelColor( i, SEGMENT.color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));
    }
  } else if (index < SEGENV.aux0 && index < 10) {
    for (int i = 0; i < index ; i++) {
       SEGMENT.setPixelColor( i, SEGMENT.color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));
    }
  }
  SEGENV.aux0 = index++;

  return FRAMETIME;
}
static const char _data_FX_MODE_COMET[] PROGMEM = "Lighthouse@!,Fade rate;!,!;!";


/*
 * Fireworks function.
 */
uint16_t mode_fireworks() {
  const uint16_t width  = strip.isMatrix ? SEGMENT.virtualWidth() : SEGMENT.virtualLength();
  const uint16_t height = SEGMENT.virtualHeight();

  if (SEGENV.call == 0) {
    SEGMENT.setUpLeds();  //lossless getPixelColor()
    SEGMENT.fill(SEGCOLOR(1));
    SEGENV.aux0 = UINT16_MAX;
    SEGENV.aux1 = UINT16_MAX;
  }
  SEGMENT.fade_out(128);

  bool valid1 = (SEGENV.aux0 < width*height);
  bool valid2 = (SEGENV.aux1 < width*height);
  uint32_t sv1 = 0, sv2 = 0;
  if (valid1) sv1 = strip.isMatrix ? SEGMENT.getPixelColorXY(SEGENV.aux0%width, SEGENV.aux0/width) : SEGMENT.getPixelColor(SEGENV.aux0); // get spark color
  if (valid2) sv2 = strip.isMatrix ? SEGMENT.getPixelColorXY(SEGENV.aux1%width, SEGENV.aux1/width) : SEGMENT.getPixelColor(SEGENV.aux1);
  if (!SEGENV.step) SEGMENT.blur(16);
  if (valid1) { if (strip.isMatrix) SEGMENT.setPixelColorXY(SEGENV.aux0%width, SEGENV.aux0/width, sv1); else SEGMENT.setPixelColor(SEGENV.aux0, sv1); } // restore spark color after blur
  if (valid2) { if (strip.isMatrix) SEGMENT.setPixelColorXY(SEGENV.aux1%width, SEGENV.aux1/width, sv2); else SEGMENT.setPixelColor(SEGENV.aux1, sv2); } // restore old spark color after blur

  for (int i=0; i<MAX(1, width/20); i++) {
    if (random8(129 - (SEGMENT.intensity >> 1)) == 0) {
      uint16_t index = random16(width*height);
      uint16_t j = index % width, k = index / width;
      uint32_t col = SEGMENT.color_from_palette(random8(), false, false, 0);
      if (strip.isMatrix) SEGMENT.setPixelColorXY(j, k, col);
      else                SEGMENT.setPixelColor(index, col);
      SEGENV.aux1 = SEGENV.aux0;  // old spark
      SEGENV.aux0 = index;        // remember where spark occured
    }
  }
  return FRAMETIME;
}
static const char _data_FX_MODE_FIREWORKS[] PROGMEM = "Fireworks@,Frequency;!,!;!;12;ix=192,pal=11";


//Twinkling LEDs running. Inspired by https://github.com/kitesurfer1404/WS2812FX/blob/master/src/custom/Rain.h
uint16_t mode_rain()
{
  const uint16_t width  = SEGMENT.virtualWidth();
  const uint16_t height = SEGMENT.virtualHeight();
  SEGENV.step += FRAMETIME;
  if (SEGENV.call && SEGENV.step > SPEED_FORMULA_L) {
    SEGENV.step = 1;
    if (strip.isMatrix) {
      uint32_t ctemp[width];
      for (int i = 0; i<width; i++) ctemp[i] = SEGMENT.getPixelColorXY(i, height-1);
      SEGMENT.move(6,1);  // move all pixels down
      for (int i = 0; i<width; i++) SEGMENT.setPixelColorXY(i, 0, ctemp[i]); // wrap around
      SEGENV.aux0 = (SEGENV.aux0 % width) + (SEGENV.aux0 / width + 1) * width;
      SEGENV.aux1 = (SEGENV.aux1 % width) + (SEGENV.aux1 / width + 1) * width;
    } else {
      //shift all leds left
      uint32_t ctemp = SEGMENT.getPixelColor(0);
      for (int i = 0; i < SEGLEN - 1; i++) {
        SEGMENT.setPixelColor(i, SEGMENT.getPixelColor(i+1));
      }
      SEGMENT.setPixelColor(SEGLEN -1, ctemp); // wrap around
      SEGENV.aux0++;  // increase spark index
      SEGENV.aux1++;
    }
    if (SEGENV.aux0 == 0) SEGENV.aux0 = UINT16_MAX; // reset previous spark positiom
    if (SEGENV.aux1 == 0) SEGENV.aux0 = UINT16_MAX; // reset previous spark positiom
    if (SEGENV.aux0 >= width*height) SEGENV.aux0 = 0;     // ignore
    if (SEGENV.aux1 >= width*height) SEGENV.aux1 = 0;
  }
  return mode_fireworks();
}
static const char _data_FX_MODE_RAIN[] PROGMEM = "Rain@!,Spawning rate;!,!;!;12;ix=128,pal=0";


/*
 * Fire flicker function
 */
uint16_t mode_fire_flicker(void) {
  uint32_t cycleTime = 40 + (255 - SEGMENT.speed);
  uint32_t it = strip.now / cycleTime;
  if (SEGENV.step == it) return FRAMETIME;

  byte w = (SEGCOLOR(0) >> 24);
  byte r = (SEGCOLOR(0) >> 16);
  byte g = (SEGCOLOR(0) >>  8);
  byte b = (SEGCOLOR(0)      );
  byte lum = (SEGMENT.palette == 0) ? MAX(w, MAX(r, MAX(g, b))) : 255;
  lum /= (((256-SEGMENT.intensity)/16)+1);
  for (int i = 0; i < SEGLEN; i++) {
    byte flicker = random8(lum);
    if (SEGMENT.palette == 0) {
      SEGMENT.setPixelColor(i, MAX(r - flicker, 0), MAX(g - flicker, 0), MAX(b - flicker, 0), MAX(w - flicker, 0));
    } else {
      SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(i, true, PALETTE_SOLID_WRAP, 0, 255 - flicker));
    }
  }

  SEGENV.step = it;
  return FRAMETIME;
}
static const char _data_FX_MODE_FIRE_FLICKER[] PROGMEM = "Fire Flicker@!,!;!;!";


/*
 * Gradient run base function
 */
uint16_t gradient_base(bool loading) {
  uint16_t counter = strip.now * ((SEGMENT.speed >> 2) + 1);
  uint16_t pp = counter * SEGLEN >> 16;
  if (SEGENV.call == 0) pp = 0;
  float val; //0.0 = sec 1.0 = pri
  float brd = loading ? SEGMENT.intensity : SEGMENT.intensity/2;
  if (brd <1.0) brd = 1.0;
  int p1 = pp-SEGLEN;
  int p2 = pp+SEGLEN;

  for (int i = 0; i < SEGLEN; i++)
  {
    if (loading)
    {
      val = abs(((i>pp) ? p2:pp) -i);
    } else {
      val = MIN(abs(pp-i),MIN(abs(p1-i),abs(p2-i)));
    }
    val = (brd > val) ? val/brd * 255 : 255;
    SEGMENT.setPixelColor(i, color_blend(SEGCOLOR(0), SEGMENT.color_from_palette(i, true, PALETTE_SOLID_WRAP, 1), val));
  }

  return FRAMETIME;
}


/*
 * Gradient run
 */
uint16_t mode_gradient(void) {
  return gradient_base(false);
}
static const char _data_FX_MODE_GRADIENT[] PROGMEM = "Gradient@!,Spread;!,!;!;;ix=16";


/*
 * Gradient run with hard transition
 */
uint16_t mode_loading(void) {
  return gradient_base(true);
}
static const char _data_FX_MODE_LOADING[] PROGMEM = "Loading@!,Fade;!,!;!;;ix=16";


//American Police Light with all LEDs Red and Blue
uint16_t police_base(uint32_t color1, uint32_t color2)
{
  uint16_t delay = 1 + (FRAMETIME<<3) / SEGLEN;  // longer segments should change faster
  uint32_t it = strip.now / map(SEGMENT.speed, 0, 255, delay<<4, delay);
  uint16_t offset = it % SEGLEN;

  uint16_t width = ((SEGLEN*(SEGMENT.intensity+1))>>9); //max width is half the strip
  if (!width) width = 1;
  for (int i = 0; i < width; i++) {
    uint16_t indexR = (offset + i) % SEGLEN;
    uint16_t indexB = (offset + i + (SEGLEN>>1)) % SEGLEN;
    SEGMENT.setPixelColor(indexR, color1);
    SEGMENT.setPixelColor(indexB, color2);
  }
  return FRAMETIME;
}


//Police Lights Red and Blue
//uint16_t mode_police()
//{
//  SEGMENT.fill(SEGCOLOR(1));
//  return police_base(RED, BLUE);
//}
//static const char _data_FX_MODE_POLICE[] PROGMEM = "Police@!,Width;,Bg;0";


//Police Lights with custom colors
uint16_t mode_two_dots()
{
  if (!SEGMENT.check2) SEGMENT.fill(SEGCOLOR(2));
  uint32_t color2 = (SEGCOLOR(1) == SEGCOLOR(2)) ? SEGCOLOR(0) : SEGCOLOR(1);
  return police_base(SEGCOLOR(0), color2);
}
static const char _data_FX_MODE_TWO_DOTS[] PROGMEM = "Two Dots@!,Dot size,,,,,Overlay;1,2,Bg;!";


/*
 * Fairy, inspired by https://www.youtube.com/watch?v=zeOw5MZWq24
 */
//4 bytes
typedef struct Flasher {
  uint16_t stateStart;
  uint8_t stateDur;
  bool stateOn;
} flasher;

#define FLASHERS_PER_ZONE 6
#define MAX_SHIMMER 92

uint16_t mode_fairy() {
  //set every pixel to a 'random' color from palette (using seed so it doesn't change between frames)
  uint16_t PRNG16 = 5100 + strip.getCurrSegmentId();
  for (int i = 0; i < SEGLEN; i++) {
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; //next 'random' number
    SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(PRNG16 >> 8, false, false, 0));
  }

  //amount of flasher pixels depending on intensity (0: none, 255: every LED)
  if (SEGMENT.intensity == 0) return FRAMETIME;
  uint8_t flasherDistance = ((255 - SEGMENT.intensity) / 28) +1; //1-10
  uint16_t numFlashers = (SEGLEN / flasherDistance) +1;

  uint16_t dataSize = sizeof(flasher) * numFlashers;
  if (!SEGENV.allocateData(dataSize)) return FRAMETIME; //allocation failed
  Flasher* flashers = reinterpret_cast<Flasher*>(SEGENV.data);
  uint16_t now16 = strip.now & 0xFFFF;

  //Up to 11 flashers in one brightness zone, afterwards a new zone for every 6 flashers
  uint16_t zones = numFlashers/FLASHERS_PER_ZONE;
  if (!zones) zones = 1;
  uint8_t flashersInZone = numFlashers/zones;
  uint8_t flasherBri[FLASHERS_PER_ZONE*2 -1];

  for (int z = 0; z < zones; z++) {
    uint16_t flasherBriSum = 0;
    uint16_t firstFlasher = z*flashersInZone;
    if (z == zones-1) flashersInZone = numFlashers-(flashersInZone*(zones-1));

    for (int f = firstFlasher; f < firstFlasher + flashersInZone; f++) {
      uint16_t stateTime = now16 - flashers[f].stateStart;
      //random on/off time reached, switch state
      if (stateTime > flashers[f].stateDur * 10) {
        flashers[f].stateOn = !flashers[f].stateOn;
        if (flashers[f].stateOn) {
          flashers[f].stateDur = 12 + random8(12 + ((255 - SEGMENT.speed) >> 2)); //*10, 250ms to 1250ms
        } else {
          flashers[f].stateDur = 20 + random8(6 + ((255 - SEGMENT.speed) >> 2)); //*10, 250ms to 1250ms
        }
        //flashers[f].stateDur = 51 + random8(2 + ((255 - SEGMENT.speed) >> 1));
        flashers[f].stateStart = now16;
        if (stateTime < 255) {
          flashers[f].stateStart -= 255 -stateTime; //start early to get correct bri
          flashers[f].stateDur += 26 - stateTime/10;
          stateTime = 255 - stateTime;
        } else {
          stateTime = 0;
        }
      }
      if (stateTime > 255) stateTime = 255; //for flasher brightness calculation, fades in first 255 ms of state
      //flasherBri[f - firstFlasher] = (flashers[f].stateOn) ? 255-SEGMENT.gamma8((510 - stateTime) >> 1) : SEGMENT.gamma8((510 - stateTime) >> 1);
      flasherBri[f - firstFlasher] = (flashers[f].stateOn) ? stateTime : 255 - (stateTime >> 0);
      flasherBriSum += flasherBri[f - firstFlasher];
    }
    //dim factor, to create "shimmer" as other pixels get less voltage if a lot of flashers are on
    uint8_t avgFlasherBri = flasherBriSum / flashersInZone;
    uint8_t globalPeakBri = 255 - ((avgFlasherBri * MAX_SHIMMER) >> 8); //183-255, suitable for 1/5th of LEDs flashers

    for (int f = firstFlasher; f < firstFlasher + flashersInZone; f++) {
      uint8_t bri = (flasherBri[f - firstFlasher] * globalPeakBri) / 255;
      PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; //next 'random' number
      uint16_t flasherPos = f*flasherDistance;
      SEGMENT.setPixelColor(flasherPos, color_blend(SEGCOLOR(1), SEGMENT.color_from_palette(PRNG16 >> 8, false, false, 0), bri));
      for (int i = flasherPos+1; i < flasherPos+flasherDistance && i < SEGLEN; i++) {
        PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; //next 'random' number
        SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(PRNG16 >> 8, false, false, 0, globalPeakBri));
      }
    }
  }
  return FRAMETIME;
}
static const char _data_FX_MODE_FAIRY[] PROGMEM = "Fairy@!,# of flashers;!,!;!";


/*
 * Fairytwinkle. Like Colortwinkle, but starting from all lit and not relying on strip.getPixelColor
 * Warning: Uses 4 bytes of segment data per pixel
 */
uint16_t mode_fairytwinkle() {
  uint16_t dataSize = sizeof(flasher) * SEGLEN;
  if (!SEGENV.allocateData(dataSize)) return mode_static(); //allocation failed
  Flasher* flashers = reinterpret_cast<Flasher*>(SEGENV.data);
  uint16_t now16 = strip.now & 0xFFFF;
  uint16_t PRNG16 = 5100 + strip.getCurrSegmentId();

  uint16_t riseFallTime = 400 + (255-SEGMENT.speed)*3;
  uint16_t maxDur = riseFallTime/100 + ((255 - SEGMENT.intensity) >> 2) + 13 + ((255 - SEGMENT.intensity) >> 1);

  for (int f = 0; f < SEGLEN; f++) {
    uint16_t stateTime = now16 - flashers[f].stateStart;
    //random on/off time reached, switch state
    if (stateTime > flashers[f].stateDur * 100) {
      flashers[f].stateOn = !flashers[f].stateOn;
      bool init = !flashers[f].stateDur;
      if (flashers[f].stateOn) {
        flashers[f].stateDur = riseFallTime/100 + ((255 - SEGMENT.intensity) >> 2) + random8(12 + ((255 - SEGMENT.intensity) >> 1)) +1;
      } else {
        flashers[f].stateDur = riseFallTime/100 + random8(3 + ((255 - SEGMENT.speed) >> 6)) +1;
      }
      flashers[f].stateStart = now16;
      stateTime = 0;
      if (init) {
        flashers[f].stateStart -= riseFallTime; //start lit
        flashers[f].stateDur = riseFallTime/100 + random8(12 + ((255 - SEGMENT.intensity) >> 1)) +5; //fire up a little quicker
        stateTime = riseFallTime;
      }
    }
    if (flashers[f].stateOn && flashers[f].stateDur > maxDur) flashers[f].stateDur = maxDur; //react more quickly on intensity change
    if (stateTime > riseFallTime) stateTime = riseFallTime; //for flasher brightness calculation, fades in first 255 ms of state
    uint8_t fadeprog = 255 - ((stateTime * 255) / riseFallTime);
    uint8_t flasherBri = (flashers[f].stateOn) ? 255-gamma8(fadeprog) : gamma8(fadeprog);
    uint16_t lastR = PRNG16;
    uint16_t diff = 0;
    while (diff < 0x4000) { //make sure colors of two adjacent LEDs differ enough
      PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; //next 'random' number
      diff = (PRNG16 > lastR) ? PRNG16 - lastR : lastR - PRNG16;
    }
    SEGMENT.setPixelColor(f, color_blend(SEGCOLOR(1), SEGMENT.color_from_palette(PRNG16 >> 8, false, false, 0), flasherBri));
  }
  return FRAMETIME;
}
static const char _data_FX_MODE_FAIRYTWINKLE[] PROGMEM = "Fairytwinkle@!,!;!,!;!;;m12=0"; //pixels


/*
 * Tricolor chase function
 */
uint16_t tricolor_chase(uint32_t color1, uint32_t color2) {
  uint32_t cycleTime = 50 + ((255 - SEGMENT.speed)<<1);
  uint32_t it = strip.now / cycleTime;  // iterator
  uint8_t width = (1 + (SEGMENT.intensity>>4)); // value of 1-16 for each colour
  uint8_t index = it % (width*3);

  for (int i = 0; i < SEGLEN; i++, index++) {
    if (index > (width*3)-1) index = 0;

    uint32_t color = color1;
    if (index > (width<<1)-1) color = SEGMENT.color_from_palette(i, true, PALETTE_SOLID_WRAP, 1);
    else if (index > width-1) color = color2;

    SEGMENT.setPixelColor(SEGLEN - i -1, color);
  }
  return FRAMETIME;
}


/*
 * Tricolor chase mode
 */
uint16_t mode_tricolor_chase(void) {
  return tricolor_chase(SEGCOLOR(2), SEGCOLOR(0));
}
static const char _data_FX_MODE_TRICOLOR_CHASE[] PROGMEM = "Chase 3@!,Size;1,2,3;!";


/*
 * ICU mode
 */
uint16_t mode_icu(void) {
  uint16_t dest = SEGENV.step & 0xFFFF;
  uint8_t space = (SEGMENT.intensity >> 3) +2;

  if (!SEGMENT.check2) SEGMENT.fill(SEGCOLOR(1));

  byte pindex = map(dest, 0, SEGLEN-SEGLEN/space, 0, 255);
  uint32_t col = SEGMENT.color_from_palette(pindex, false, false, 0);

  SEGMENT.setPixelColor(dest, col);
  SEGMENT.setPixelColor(dest + SEGLEN/space, col);

  if(SEGENV.aux0 == dest) { // pause between eye movements
    if(random8(6) == 0) { // blink once in a while
      SEGMENT.setPixelColor(dest, SEGCOLOR(1));
      SEGMENT.setPixelColor(dest + SEGLEN/space, SEGCOLOR(1));
      return 200;
    }
    SEGENV.aux0 = random16(SEGLEN-SEGLEN/space);
    return 1000 + random16(2000);
  }

  if(SEGENV.aux0 > SEGENV.step) {
    SEGENV.step++;
    dest++;
  } else if (SEGENV.aux0 < SEGENV.step) {
    SEGENV.step--;
    dest--;
  }

  SEGMENT.setPixelColor(dest, col);
  SEGMENT.setPixelColor(dest + SEGLEN/space, col);

  return SPEED_FORMULA_L;
}
static const char _data_FX_MODE_ICU[] PROGMEM = "ICU@!,!,,,,,Overlay;!,!;!";


/*
 * Custom mode by Aircoookie. Color Wipe, but with 3 colors
 */
uint16_t mode_tricolor_wipe(void) {
  uint32_t cycleTime = 1000 + (255 - SEGMENT.speed)*200;
  uint32_t perc = strip.now % cycleTime;
  uint16_t prog = (perc * 65535) / cycleTime;
  uint16_t ledIndex = (prog * SEGLEN * 3) >> 16;
  uint16_t ledOffset = ledIndex;

  for (int i = 0; i < SEGLEN; i++)
  {
    SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(i, true, PALETTE_SOLID_WRAP, 2));
  }

  if(ledIndex < SEGLEN) { //wipe from 0 to 1
    for (int i = 0; i < SEGLEN; i++)
    {
      SEGMENT.setPixelColor(i, (i > ledOffset)? SEGCOLOR(0) : SEGCOLOR(1));
    }
  } else if (ledIndex < SEGLEN*2) { //wipe from 1 to 2
    ledOffset = ledIndex - SEGLEN;
    for (int i = ledOffset +1; i < SEGLEN; i++)
    {
      SEGMENT.setPixelColor(i, SEGCOLOR(1));
    }
  } else //wipe from 2 to 0
  {
    ledOffset = ledIndex - SEGLEN*2;
    for (int i = 0; i <= ledOffset; i++)
    {
      SEGMENT.setPixelColor(i, SEGCOLOR(0));
    }
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_TRICOLOR_WIPE[] PROGMEM = "Tri Wipe@!;1,2,3;!";


/*
 * Fades between 3 colors
 * Custom mode by Keith Lord: https://github.com/kitesurfer1404/WS2812FX/blob/master/src/custom/TriFade.h
 * Modified by Aircoookie
 */
uint16_t mode_tricolor_fade(void) {
  uint16_t counter = strip.now * ((SEGMENT.speed >> 3) +1);
  uint32_t prog = (counter * 768) >> 16;

  uint32_t color1 = 0, color2 = 0;
  byte stage = 0;

  if(prog < 256) {
    color1 = SEGCOLOR(0);
    color2 = SEGCOLOR(1);
    stage = 0;
  } else if(prog < 512) {
    color1 = SEGCOLOR(1);
    color2 = SEGCOLOR(2);
    stage = 1;
  } else {
    color1 = SEGCOLOR(2);
    color2 = SEGCOLOR(0);
    stage = 2;
  }

  byte stp = prog; // % 256
  for (int i = 0; i < SEGLEN; i++) {
    uint32_t color;
    if (stage == 2) {
      color = color_blend(SEGMENT.color_from_palette(i, true, PALETTE_SOLID_WRAP, 2), color2, stp);
    } else if (stage == 1) {
      color = color_blend(color1, SEGMENT.color_from_palette(i, true, PALETTE_SOLID_WRAP, 2), stp);
    } else {
      color = color_blend(color1, color2, stp);
    }
    SEGMENT.setPixelColor(i, color);
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_TRICOLOR_FADE[] PROGMEM = "Tri Fade@!;1,2,3;!";


/*
 * Creates random comets
 * Custom mode by Keith Lord: https://github.com/kitesurfer1404/WS2812FX/blob/master/src/custom/MultiComet.h
 */
uint16_t mode_multi_comet(void) {
  uint32_t cycleTime = 10 + (uint32_t)(255 - SEGMENT.speed);
  uint32_t it = strip.now / cycleTime;
  if (SEGENV.step == it) return FRAMETIME;
  if (!SEGENV.allocateData(sizeof(uint16_t) * 8)) return mode_static(); //allocation failed

  SEGMENT.fade_out(SEGMENT.intensity);

  uint16_t* comets = reinterpret_cast<uint16_t*>(SEGENV.data);

  for (int i=0; i < 8; i++) {
    if(comets[i] < SEGLEN) {
      uint16_t index = comets[i];
      if (SEGCOLOR(2) != 0)
      {
        SEGMENT.setPixelColor(index, i % 2 ? SEGMENT.color_from_palette(index, true, PALETTE_SOLID_WRAP, 0) : SEGCOLOR(2));
      } else
      {
        SEGMENT.setPixelColor(index, SEGMENT.color_from_palette(index, true, PALETTE_SOLID_WRAP, 0));
      }
      comets[i]++;
    } else {
      if(!random(SEGLEN)) {
        comets[i] = 0;
      }
    }
  }

  SEGENV.step = it;
  return FRAMETIME;
}
static const char _data_FX_MODE_MULTI_COMET[] PROGMEM = "Multi Comet";


/*
 * Running random pixels ("Stream 2")
 * Custom mode by Keith Lord: https://github.com/kitesurfer1404/WS2812FX/blob/master/src/custom/RandomChase.h
 */
uint16_t mode_random_chase(void) {
  if (SEGENV.call == 0) {
    SEGENV.step = RGBW32(random8(), random8(), random8(), 0);
    SEGENV.aux0 = random16();
  }
  uint16_t prevSeed = random16_get_seed(); // save seed so we can restore it at the end of the function
  uint32_t cycleTime = 25 + (3 * (uint32_t)(255 - SEGMENT.speed));
  uint32_t it = strip.now / cycleTime;
  uint32_t color = SEGENV.step;
  random16_set_seed(SEGENV.aux0);

  for (int i = SEGLEN -1; i > 0; i--) {
    uint8_t r = random8(6) != 0 ? (color >> 16 & 0xFF) : random8();
    uint8_t g = random8(6) != 0 ? (color >> 8  & 0xFF) : random8();
    uint8_t b = random8(6) != 0 ? (color       & 0xFF) : random8();
    color = RGBW32(r, g, b, 0);
    SEGMENT.setPixelColor(i, r, g, b);
    if (i == SEGLEN -1 && SEGENV.aux1 != (it & 0xFFFF)) { //new first color in next frame
      SEGENV.step = color;
      SEGENV.aux0 = random16_get_seed();
    }
  }

  SEGENV.aux1 = it & 0xFFFF;

  random16_set_seed(prevSeed); // restore original seed so other effects can use "random" PRNG
  return FRAMETIME;
}
static const char _data_FX_MODE_RANDOM_CHASE[] PROGMEM = "Stream 2@!;;";


//7 bytes
typedef struct Oscillator {
  int16_t pos;
  int8_t  size;
  int8_t  dir;
  int8_t  speed;
} oscillator;

/*
/  Oscillating bars of color, updated with standard framerate
*/
uint16_t mode_oscillate(void) {
  uint8_t numOscillators = 3;
  uint16_t dataSize = sizeof(oscillator) * numOscillators;

  if (!SEGENV.allocateData(dataSize)) return mode_static(); //allocation failed

  Oscillator* oscillators = reinterpret_cast<Oscillator*>(SEGENV.data);

  if (SEGENV.call == 0)
  {
    oscillators[0] = {(int16_t)(SEGLEN/4),   (int8_t)(SEGLEN/8),  1, 1};
    oscillators[1] = {(int16_t)(SEGLEN/4*3), (int8_t)(SEGLEN/8),  1, 2};
    oscillators[2] = {(int16_t)(SEGLEN/4*2), (int8_t)(SEGLEN/8), -1, 1};
  }

  uint32_t cycleTime = 20 + (2 * (uint32_t)(255 - SEGMENT.speed));
  uint32_t it = strip.now / cycleTime;

  for (int i = 0; i < numOscillators; i++) {
    // if the counter has increased, move the oscillator by the random step
    if (it != SEGENV.step) oscillators[i].pos += oscillators[i].dir * oscillators[i].speed;
    oscillators[i].size = SEGLEN/(3+SEGMENT.intensity/8);
    if((oscillators[i].dir == -1) && (oscillators[i].pos <= 0)) {
      oscillators[i].pos = 0;
      oscillators[i].dir = 1;
      // make bigger steps for faster speeds
      oscillators[i].speed = SEGMENT.speed > 100 ? random8(2, 4):random8(1, 3);
    }
    if((oscillators[i].dir == 1) && (oscillators[i].pos >= (SEGLEN - 1))) {
      oscillators[i].pos = SEGLEN - 1;
      oscillators[i].dir = -1;
      oscillators[i].speed = SEGMENT.speed > 100 ? random8(2, 4):random8(1, 3);
    }
  }

  for (int i=0; i < SEGLEN; i++) {
    uint32_t color = BLACK;
    for (int j=0; j < numOscillators; j++) {
      if(i >= oscillators[j].pos - oscillators[j].size && i <= oscillators[j].pos + oscillators[j].size) {
        color = (color == BLACK) ? SEGCOLOR(j) : color_blend(color, SEGCOLOR(j), 128);
      }
    }
    SEGMENT.setPixelColor(i, color);
  }

  SEGENV.step = it;
  return FRAMETIME;
}
static const char _data_FX_MODE_OSCILLATE[] PROGMEM = "Oscillate";


//TODO
uint16_t mode_lightning(void) {
  uint16_t ledstart = random16(SEGLEN);               // Determine starting location of flash
  uint16_t ledlen = 1 + random16(SEGLEN -ledstart);   // Determine length of flash (not to go beyond NUM_LEDS-1)
  uint8_t bri = 255/random8(1, 3);

  if (SEGENV.aux1 == 0) //init, leader flash
  {
    SEGENV.aux1 = random8(4, 4 + SEGMENT.intensity/20); //number of flashes
    SEGENV.aux1 *= 2;

    bri = 52; //leader has lower brightness
    SEGENV.aux0 = 200; //200ms delay after leader
  }

  if (!SEGMENT.check2) SEGMENT.fill(SEGCOLOR(1));

  if (SEGENV.aux1 > 3 && !(SEGENV.aux1 & 0x01)) { //flash on even number >2
    for (int i = ledstart; i < ledstart + ledlen; i++)
    {
      SEGMENT.setPixelColor(i,SEGMENT.color_from_palette(i, true, PALETTE_SOLID_WRAP, 0, bri));
    }
    SEGENV.aux1--;

    SEGENV.step = millis();
    //return random8(4, 10); // each flash only lasts one frame/every 24ms... originally 4-10 milliseconds
  } else {
    if (millis() - SEGENV.step > SEGENV.aux0) {
      SEGENV.aux1--;
      if (SEGENV.aux1 < 2) SEGENV.aux1 = 0;

      SEGENV.aux0 = (50 + random8(100)); //delay between flashes
      if (SEGENV.aux1 == 2) {
        SEGENV.aux0 = (random8(255 - SEGMENT.speed) * 100); // delay between strikes
      }
      SEGENV.step = millis();
    }
  }
  return FRAMETIME;
}
static const char _data_FX_MODE_LIGHTNING[] PROGMEM = "Lightning@!,!,,,,,Overlay;!,!;!";


// Pride2015
// Animated, ever-changing rainbows.
// by Mark Kriegsman: https://gist.github.com/kriegsman/964de772d64c502760e5
uint16_t mode_pride_2015(void) {
  uint16_t duration = 10 + SEGMENT.speed;
  uint16_t sPseudotime = SEGENV.step;
  uint16_t sHue16 = SEGENV.aux0;

  uint8_t sat8 = beatsin88( 87, 220, 250);
  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 1, 3000);

  sPseudotime += duration * msmultiplier;
  sHue16 += duration * beatsin88( 400, 5,9);
  uint16_t brightnesstheta16 = sPseudotime;
  CRGB fastled_col;

  for (int i = 0 ; i < SEGLEN; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 >> 8;

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);

    CRGB newcolor = CHSV(hue8, sat8, bri8);
    SEGMENT.blendPixelColor(i, newcolor, 64);
  }
  SEGENV.step = sPseudotime;
  SEGENV.aux0 = sHue16;

  return FRAMETIME;
}
static const char _data_FX_MODE_PRIDE_2015[] PROGMEM = "Pride 2015@!;;";


//eight colored dots, weaving in and out of sync with each other
uint16_t mode_juggle(void) {
  if (SEGENV.call == 0) {
    SEGMENT.setUpLeds();  //lossless getPixelColor()
    SEGMENT.fill(BLACK);
  }

  SEGMENT.fadeToBlackBy(192 - (3*SEGMENT.intensity/4));

  CRGB fastled_col;
  byte dothue = 0;
  for (int i = 0; i < 8; i++) {
    uint16_t index = 0 + beatsin88((16 + SEGMENT.speed)*(i + 7), 0, SEGLEN -1);
    fastled_col = CRGB(SEGMENT.getPixelColor(index));
    fastled_col |= (SEGMENT.palette==0)?CHSV(dothue, 220, 255):ColorFromPalette(SEGPALETTE, dothue, 255);
    SEGMENT.setPixelColor(index, fastled_col);
    dothue += 32;
  }
  return FRAMETIME;
}
static const char _data_FX_MODE_JUGGLE[] PROGMEM = "Juggle@!,Trail;;!;;sx=64,ix=128";


uint16_t mode_palette() {
  uint16_t counter = 0;
  if (SEGMENT.speed != 0)
  {
    counter = (strip.now * ((SEGMENT.speed >> 3) +1)) & 0xFFFF;
    counter = counter >> 8;
  }

  for (int i = 0; i < SEGLEN; i++)
  {
    uint8_t colorIndex = (i * 255 / SEGLEN) - counter;
    SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(colorIndex, false, PALETTE_MOVING_WRAP, 255));
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_PALETTE[] PROGMEM = "Palette@Cycle speed;;!;;c3=0,o2=0";


// WLED limitation: Analog Clock overlay will NOT work when Fire2012 is active
// Fire2012 by Mark Kriegsman, July 2012
// as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
////
// This basic one-dimensional 'fire' simulation works roughly as follows:
// There's a underlying array of 'heat' cells, that model the temperature
// at each point along the line.  Every cycle through the simulation,
// four steps are performed:
//  1) All cells cool down a little bit, losing heat to the air
//  2) The heat from each cell drifts 'up' and diffuses a little
//  3) Sometimes randomly new 'sparks' of heat are added at the bottom
//  4) The heat from each cell is rendered as a color into the leds array
//     The heat-to-color mapping uses a black-body radiation approximation.
//
// Temperature is in arbitrary units from 0 (cold black) to 255 (white hot).
//
// This simulation scales it self a bit depending on SEGLEN; it should look
// "OK" on anywhere from 20 to 100 LEDs without too much tweaking.
//
// I recommend running this simulation at anywhere from 30-100 frames per second,
// meaning an interframe delay of about 10-35 milliseconds.
//
// Looks best on a high-density LED setup (60+ pixels/meter).
//
//
// There are two main parameters you can play with to control the look and
// feel of your fire: COOLING (used in step 1 above) (Speed = COOLING), and SPARKING (used
// in step 3 above) (Effect Intensity = Sparking).
uint16_t mode_fire_2012() {
  const uint16_t strips = SEGMENT.nrOfVStrips();
  if (!SEGENV.allocateData(strips * SEGLEN)) return mode_static(); //allocation failed
  byte* heat = SEGENV.data;

  const uint32_t it = strip.now >> 6; //div 64

  struct virtualStrip {
    static void runStrip(uint16_t stripNr, byte* heat, uint32_t it) {

      const uint8_t ignition = max(3,SEGLEN/10);  // ignition area: 10% of segment length or minimum 3 pixels

      // Step 1.  Cool down every cell a little
      for (int i = 0; i < SEGLEN; i++) {
        uint8_t cool = (it != SEGENV.step) ? random8((((20 + SEGMENT.speed/3) * 16) / SEGLEN)+2) : random(8);
        uint8_t minTemp = 0;
        if (i<ignition) {
          minTemp = (ignition-i)/4 + 16;  // and should not become black
        }
        uint8_t temp = qsub8(heat[i], cool);
        heat[i] = temp<minTemp ? minTemp : temp;
      }

      if (it != SEGENV.step)
      {
        // Step 2.  Heat from each cell drifts 'up' and diffuses a little
        for (int k = SEGLEN -1; k > 1; k--) {
          heat[k] = (heat[k - 1] + (heat[k - 2]<<1) ) / 3;  // heat[k-2] multiplied by 2
        }
      }

      // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
      if (random8() <= SEGMENT.intensity) {
        uint8_t y = random8(ignition);
        uint8_t boost = (32+SEGMENT.custom3*2) * (2*ignition-y) / (2*ignition);
        heat[y] = qadd8(heat[y], random8(64+boost,128+boost));
      }

      // Step 4.  Map from heat cells to LED colors
      for (int j = 0; j < SEGLEN; j++) {
        SEGMENT.setPixelColor(indexToVStrip(j, stripNr), ColorFromPalette(SEGPALETTE, MIN(heat[j],240), 255, NOBLEND));
      }
    }
  };

  for (int stripNr=0; stripNr<strips; stripNr++)
    virtualStrip::runStrip(stripNr, &heat[stripNr * SEGLEN], it);

  if (SEGMENT.is2D()) SEGMENT.blur(32);

  if (it != SEGENV.step)
    SEGENV.step = it;

  return FRAMETIME;
}
static const char _data_FX_MODE_FIRE_2012[] PROGMEM = "Fire 2012@Cooling,Spark rate,,,Boost;;!;1;sx=120,ix=64,m12=1"; // bars


// ColorWavesWithPalettes by Mark Kriegsman: https://gist.github.com/kriegsman/8281905786e8b2632aeb
// This function draws color waves with an ever-changing,
// widely-varying set of parameters, using a color palette.
uint16_t mode_colorwaves() {
  uint16_t duration = 10 + SEGMENT.speed;
  uint16_t sPseudotime = SEGENV.step;
  uint16_t sHue16 = SEGENV.aux0;

  uint8_t brightdepth = beatsin88(341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 60, 300)*SEGMENT.intensity*10/255;  // Use the Intensity Slider for the hues

  sPseudotime += duration * msmultiplier;
  sHue16 += duration * beatsin88(400, 5, 9);
  uint16_t brightnesstheta16 = sPseudotime;

  for (int i = 0 ; i < SEGLEN; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 >> 8;
    uint16_t h16_128 = hue16 >> 7;
    if ( h16_128 & 0x100) {
      hue8 = 255 - (h16_128 >> 1);
    } else {
      hue8 = h16_128 >> 1;
    }

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16(brightnesstheta16) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);

    SEGMENT.blendPixelColor(i, SEGMENT.color_from_palette(hue8, false, PALETTE_SOLID_WRAP, 0, bri8), 128); // 50/50 mix
  }
  SEGENV.step = sPseudotime;
  SEGENV.aux0 = sHue16;

  return FRAMETIME;
}
static const char _data_FX_MODE_COLORWAVES[] PROGMEM = "Colorwaves@!,Hue;!;!";


// colored stripes pulsing at a defined Beats-Per-Minute (BPM)
uint16_t mode_bpm() {
  //CRGB fastled_col;
  uint32_t stp = (strip.now / 20) & 0xFF;
  uint8_t beat = beatsin8(SEGMENT.speed, 64, 255);
  for (int i = 0; i < SEGLEN; i++) {
    //fastled_col = ColorFromPalette(SEGPALETTE, stp + (i * 2), beat - stp + (i * 10));
    //SEGMENT.setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
    SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(stp + (i * 2), false, PALETTE_SOLID_WRAP, 0, beat - stp + (i * 10)));
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_BPM[] PROGMEM = "Bpm@!;!;!;;sx=64";


uint16_t mode_fillnoise8() {
  if (SEGENV.call == 0) SEGENV.step = random16(12345);
  //CRGB fastled_col;
  for (int i = 0; i < SEGLEN; i++) {
    uint8_t index = inoise8(i * SEGLEN, SEGENV.step + i * SEGLEN);
    //fastled_col = ColorFromPalette(SEGPALETTE, index, 255, LINEARBLEND);
    //SEGMENT.setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
    SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(index, false, PALETTE_SOLID_WRAP, 0));
  }
  SEGENV.step += beatsin8(SEGMENT.speed, 1, 6); //10,1,4

  return FRAMETIME;
}
static const char _data_FX_MODE_FILLNOISE8[] PROGMEM = "Fill Noise@!;!;!";


uint16_t mode_noise16_1() {
  uint16_t scale = 320;                                       // the "zoom factor" for the noise
  //CRGB fastled_col;
  SEGENV.step += (1 + SEGMENT.speed/16);

  for (int i = 0; i < SEGLEN; i++) {
    uint16_t shift_x = beatsin8(11);                          // the x position of the noise field swings @ 17 bpm
    uint16_t shift_y = SEGENV.step/42;                        // the y position becomes slowly incremented
    uint16_t real_x = (i + shift_x) * scale;                  // the x position of the noise field swings @ 17 bpm
    uint16_t real_y = (i + shift_y) * scale;                  // the y position becomes slowly incremented
    uint32_t real_z = SEGENV.step;                            // the z position becomes quickly incremented
    uint8_t noise = inoise16(real_x, real_y, real_z) >> 8;    // get the noise data and scale it down
    uint8_t index = sin8(noise * 3);                          // map LED color based on noise data

    //fastled_col = ColorFromPalette(SEGPALETTE, index, 255, LINEARBLEND);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.
    //SEGMENT.setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
    SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(index, false, PALETTE_SOLID_WRAP, 0));
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_NOISE16_1[] PROGMEM = "Noise 1@!;!;!";


uint16_t mode_noise16_2() {
  uint16_t scale = 1000;                                        // the "zoom factor" for the noise
  //CRGB fastled_col;
  SEGENV.step += (1 + (SEGMENT.speed >> 1));

  for (int i = 0; i < SEGLEN; i++) {
    uint16_t shift_x = SEGENV.step >> 6;                        // x as a function of time
    uint32_t real_x = (i + shift_x) * scale;                    // calculate the coordinates within the noise field
    uint8_t noise = inoise16(real_x, 0, 4223) >> 8;             // get the noise data and scale it down
    uint8_t index = sin8(noise * 3);                            // map led color based on noise data

    //fastled_col = ColorFromPalette(SEGPALETTE, index, noise, LINEARBLEND);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.
    //SEGMENT.setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
    SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(index, false, PALETTE_SOLID_WRAP, 0, noise));
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_NOISE16_2[] PROGMEM = "Noise 2@!;!;!";


uint16_t mode_noise16_3() {
  uint16_t scale = 800;                                       // the "zoom factor" for the noise
  //CRGB fastled_col;
  SEGENV.step += (1 + SEGMENT.speed);

  for (int i = 0; i < SEGLEN; i++) {
    uint16_t shift_x = 4223;                                  // no movement along x and y
    uint16_t shift_y = 1234;
    uint32_t real_x = (i + shift_x) * scale;                  // calculate the coordinates within the noise field
    uint32_t real_y = (i + shift_y) * scale;                  // based on the precalculated positions
    uint32_t real_z = SEGENV.step*8;
    uint8_t noise = inoise16(real_x, real_y, real_z) >> 8;    // get the noise data and scale it down
    uint8_t index = sin8(noise * 3);                          // map led color based on noise data

    //fastled_col = ColorFromPalette(SEGPALETTE, index, noise, LINEARBLEND);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.
    //SEGMENT.setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
    SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(index, false, PALETTE_SOLID_WRAP, 0, noise));
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_NOISE16_3[] PROGMEM = "Noise 3@!;!;!";


//https://github.com/aykevl/ledstrip-spark/blob/master/ledstrip.ino
uint16_t mode_noise16_4() {
  //CRGB fastled_col;
  uint32_t stp = (strip.now * SEGMENT.speed) >> 7;
  for (int i = 0; i < SEGLEN; i++) {
    int16_t index = inoise16(uint32_t(i) << 12, stp);
    //fastled_col = ColorFromPalette(SEGPALETTE, index);
    //SEGMENT.setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
    SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(index, false, PALETTE_SOLID_WRAP, 0));
  }
  return FRAMETIME;
}
static const char _data_FX_MODE_NOISE16_4[] PROGMEM = "Noise 4@!;!;!";


//based on https://gist.github.com/kriegsman/5408ecd397744ba0393e
uint16_t mode_colortwinkle() {
  uint16_t dataSize = (SEGLEN+7) >> 3; //1 bit per LED
  if (!SEGENV.allocateData(dataSize)) return mode_static(); //allocation failed

  CRGB fastled_col, prev;
  fract8 fadeUpAmount = strip.getBrightness()>28 ? 8 + (SEGMENT.speed>>2) : 68-strip.getBrightness();
  fract8 fadeDownAmount = strip.getBrightness()>28 ? 8 + (SEGMENT.speed>>3) : 68-strip.getBrightness();
  for (uint16_t i = 0; i < SEGLEN; i++) {
    fastled_col = SEGMENT.getPixelColor(i);
    prev = fastled_col;
    uint16_t index = i >> 3;
    uint8_t  bitNum = i & 0x07;
    bool fadeUp = bitRead(SEGENV.data[index], bitNum);

    if (fadeUp) {
      CRGB incrementalColor = fastled_col;
      incrementalColor.nscale8_video(fadeUpAmount);
      fastled_col += incrementalColor;

      if (fastled_col.red == 255 || fastled_col.green == 255 || fastled_col.blue == 255) {
        bitWrite(SEGENV.data[index], bitNum, false);
      }
      SEGMENT.setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);

      if (SEGMENT.getPixelColor(i) == RGBW32(prev.r, prev.g, prev.b, 0)) {  //fix "stuck" pixels
        fastled_col += fastled_col;
        SEGMENT.setPixelColor(i, fastled_col);
      }
    } else {
      fastled_col.nscale8(255 - fadeDownAmount);
      SEGMENT.setPixelColor(i, fastled_col);
    }
  }

  for (uint16_t j = 0; j <= SEGLEN / 50; j++) {
    if (random8() <= SEGMENT.intensity) {
      for (uint8_t times = 0; times < 5; times++) { //attempt to spawn a new pixel 5 times
        int i = random16(SEGLEN);
        if (SEGMENT.getPixelColor(i) == 0) {
          fastled_col = ColorFromPalette(SEGPALETTE, random8(), 64, NOBLEND);
          uint16_t index = i >> 3;
          uint8_t  bitNum = i & 0x07;
          bitWrite(SEGENV.data[index], bitNum, true);
          SEGMENT.setPixelColor(i, fastled_col);
          break; //only spawn 1 new pixel per frame per 50 LEDs
        }
      }
    }
  }
  return FRAMETIME_FIXED;
}
static const char _data_FX_MODE_COLORTWINKLE[] PROGMEM = "Colortwinkles@Fade speed,Spawn speed;;!;;m12=0"; //pixels


//Calm effect, like a lake at night
uint16_t mode_lake() {
  uint8_t sp = SEGMENT.speed/10;
  int wave1 = beatsin8(sp +2, -64,64);
  int wave2 = beatsin8(sp +1, -64,64);
  uint8_t wave3 = beatsin8(sp +2,   0,80);
  //CRGB fastled_col;

  for (int i = 0; i < SEGLEN; i++)
  {
    int index = cos8((i*15)+ wave1)/2 + cubicwave8((i*23)+ wave2)/2;
    uint8_t lum = (index > wave3) ? index - wave3 : 0;
    //fastled_col = ColorFromPalette(SEGPALETTE, map(index,0,255,0,240), lum, LINEARBLEND);
    //SEGMENT.setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
    SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(index, false, false, 0, lum));
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_LAKE[] PROGMEM = "Lake@!;Fx;!";


// meteor effect
// send a meteor from begining to to the end of the strip with a trail that randomly decays.
// adapted from https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/#LEDStripEffectMeteorRain
uint16_t mode_meteor() {
  if (!SEGENV.allocateData(SEGLEN)) return mode_static(); //allocation failed

  byte* trail = SEGENV.data;

  byte meteorSize= 1+ SEGLEN / 10;
  uint16_t counter = strip.now * ((SEGMENT.speed >> 2) +8);
  uint16_t in = counter * SEGLEN >> 16;

  // fade all leds to colors[1] in LEDs one step
  for (int i = 0; i < SEGLEN; i++) {
    if (random8() <= 255 - SEGMENT.intensity)
    {
      byte meteorTrailDecay = 128 + random8(127);
      trail[i] = scale8(trail[i], meteorTrailDecay);
      SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(i, true, false, 0, trail[i]));
    }
  }

  // draw meteor
  for (int j = 0; j < meteorSize; j++) {
    uint16_t index = in + j;
    if (index >= SEGLEN) {
      index -= SEGLEN;
    }
    trail[index] = 240;
    SEGMENT.setPixelColor(index, SEGMENT.color_from_palette(index, true, false, 0, 255));
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_METEOR[] PROGMEM = "Meteor@!,Trail length;!;!";


// smooth meteor effect
// send a meteor from begining to to the end of the strip with a trail that randomly decays.
// adapted from https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/#LEDStripEffectMeteorRain
uint16_t mode_meteor_smooth() {
  if (!SEGENV.allocateData(SEGLEN)) return mode_static(); //allocation failed

  byte* trail = SEGENV.data;

  byte meteorSize= 1+ SEGLEN / 10;
  uint16_t in = map((SEGENV.step >> 6 & 0xFF), 0, 255, 0, SEGLEN -1);

  // fade all leds to colors[1] in LEDs one step
  for (int i = 0; i < SEGLEN; i++) {
    if (trail[i] != 0 && random8() <= 255 - SEGMENT.intensity)
    {
      int change = 3 - random8(12); //change each time between -8 and +3
      trail[i] += change;
      if (trail[i] > 245) trail[i] = 0;
      if (trail[i] > 240) trail[i] = 240;
      SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(i, true, false, 0, trail[i]));
    }
  }

  // draw meteor
  for (int j = 0; j < meteorSize; j++) {
    uint16_t index = in + j;
    if (index >= SEGLEN) {
      index -= SEGLEN;
    }
    trail[index] = 240;
    SEGMENT.setPixelColor(index, SEGMENT.color_from_palette(index, true, false, 0, 255));
  }

  SEGENV.step += SEGMENT.speed +1;
  return FRAMETIME;
}
static const char _data_FX_MODE_METEOR_SMOOTH[] PROGMEM = "Meteor Smooth@!,Trail length;!;!";


//Railway Crossing / Christmas Fairy lights
uint16_t mode_railway() {
  uint16_t dur = (256 - SEGMENT.speed) * 40;
  uint16_t rampdur = (dur * SEGMENT.intensity) >> 8;
  if (SEGENV.step > dur)
  {
    //reverse direction
    SEGENV.step = 0;
    SEGENV.aux0 = !SEGENV.aux0;
  }
  uint8_t pos = 255;
  if (rampdur != 0)
  {
    uint16_t p0 = (SEGENV.step * 255) / rampdur;
    if (p0 < 255) pos = p0;
  }
  if (SEGENV.aux0) pos = 255 - pos;
  for (int i = 0; i < SEGLEN; i += 2)
  {
    SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(255 - pos, false, false, 255)); // do not use color 1 or 2, always use palette
    if (i < SEGLEN -1)
    {
      SEGMENT.setPixelColor(i + 1, SEGMENT.color_from_palette(pos, false, false, 255)); // do not use color 1 or 2, always use palette
    }
  }
  SEGENV.step += FRAMETIME;
  return FRAMETIME;
}
static const char _data_FX_MODE_RAILWAY[] PROGMEM = "Railway@!,Smoothness;1,2;!";


//Water ripple
//propagation velocity from speed
//drop rate from intensity

//4 bytes
typedef struct Ripple {
  uint8_t state;
  uint8_t color;
  uint16_t pos;
} ripple;

#ifdef ESP8266
  #define MAX_RIPPLES   56
#else
  #define MAX_RIPPLES  100
#endif
uint16_t ripple_base()
{
  uint16_t maxRipples = min(1 + (SEGLEN >> 2), MAX_RIPPLES);  // 56 max for 16 segment ESP8266
  uint16_t dataSize = sizeof(ripple) * maxRipples;

  if (!SEGENV.allocateData(dataSize)) return mode_static(); //allocation failed

  Ripple* ripples = reinterpret_cast<Ripple*>(SEGENV.data);

  //draw wave
  for (int i = 0; i < maxRipples; i++) {
    uint16_t ripplestate = ripples[i].state;
    if (ripplestate) {
      uint8_t rippledecay = (SEGMENT.speed >> 4) +1; //faster decay if faster propagation
      uint16_t rippleorigin = ripples[i].pos;
      uint32_t col = SEGMENT.color_from_palette(ripples[i].color, false, false, 255);
      uint16_t propagation = ((ripplestate/rippledecay - 1) * (SEGMENT.speed + 1));
      int16_t propI = propagation >> 8;
      uint8_t propF = propagation & 0xFF;
      uint8_t amp = (ripplestate < 17) ? triwave8((ripplestate-1)*8) : map(ripplestate,17,255,255,2);

      #ifndef WLED_DISABLE_2D
      if (SEGMENT.is2D()) {
        uint16_t cx = rippleorigin >> 8;
        uint16_t cy = rippleorigin & 0xFF;
        uint8_t mag = scale8(cubicwave8((propF>>2)), amp);
        if (propI > 0) SEGMENT.draw_circle(cx, cy, propI, color_blend(SEGMENT.getPixelColorXY(cx + propI, cy), col, mag));
      } else
      #endif
      {
        int16_t left = rippleorigin - propI -1;
        for (int16_t v = left; v < left +4; v++) {
          uint8_t mag = scale8(cubicwave8((propF>>2)+(v-left)*64), amp);
          SEGMENT.setPixelColor(v, color_blend(SEGMENT.getPixelColor(v), col, mag)); // TODO
          int16_t w = left + propI*2 + 3 -(v-left);
          SEGMENT.setPixelColor(w, color_blend(SEGMENT.getPixelColor(w), col, mag)); // TODO
        }
      }
      ripplestate += rippledecay;
      ripples[i].state = (ripplestate > 254) ? 0 : ripplestate;
    } else {//randomly create new wave
      if (random16(IBN + 10000) <= SEGMENT.intensity) {
        ripples[i].state = 1;
        ripples[i].pos = SEGMENT.is2D() ? ((random8(SEGENV.virtualWidth())<<8) | (random8(SEGENV.virtualHeight()))) : random16(SEGLEN);
        ripples[i].color = random8(); //color
      }
    }
  }

  return FRAMETIME;
}
#undef MAX_RIPPLES


uint16_t mode_ripple(void) {
  if (!SEGMENT.check2) SEGMENT.fill(SEGCOLOR(1));
  return ripple_base();
}
static const char _data_FX_MODE_RIPPLE[] PROGMEM = "Ripple@!,Wave #,,,,,Overlay;,!;!;12";


uint16_t mode_ripple_rainbow(void) {
  if (SEGENV.call ==0) {
    SEGENV.aux0 = random8();
    SEGENV.aux1 = random8();
  }
  if (SEGENV.aux0 == SEGENV.aux1) {
    SEGENV.aux1 = random8();
  } else if (SEGENV.aux1 > SEGENV.aux0) {
    SEGENV.aux0++;
  } else {
    SEGENV.aux0--;
  }
  SEGMENT.fill(color_blend(SEGMENT.color_wheel(SEGENV.aux0),BLACK,235));
  return ripple_base();
}
static const char _data_FX_MODE_RIPPLE_RAINBOW[] PROGMEM = "Ripple Rainbow@!,Wave #;;!;12";


//  TwinkleFOX by Mark Kriegsman: https://gist.github.com/kriegsman/756ea6dcae8e30845b5a
//
//  TwinkleFOX: Twinkling 'holiday' lights that fade in and out.
//  Colors are chosen from a palette. Read more about this effect using the link above!

// If COOL_LIKE_INCANDESCENT is set to 1, colors will
// fade out slighted 'reddened', similar to how
// incandescent bulbs change color as they get dim down.
#define COOL_LIKE_INCANDESCENT 1

CRGB twinklefox_one_twinkle(uint32_t ms, uint8_t salt, bool cat)
{
  // Overall twinkle speed (changed)
  uint16_t ticks = ms / SEGENV.aux0;
  uint8_t fastcycle8 = ticks;
  uint16_t slowcycle16 = (ticks >> 8) + salt;
  slowcycle16 += sin8(slowcycle16);
  slowcycle16 = (slowcycle16 * 2053) + 1384;
  uint8_t slowcycle8 = (slowcycle16 & 0xFF) + (slowcycle16 >> 8);

  // Overall twinkle density.
  // 0 (NONE lit) to 8 (ALL lit at once).
  // Default is 5.
  uint8_t twinkleDensity = (SEGMENT.intensity >> 5) +1;

  uint8_t bright = 0;
  if (((slowcycle8 & 0x0E)/2) < twinkleDensity) {
    uint8_t ph = fastcycle8;
    // This is like 'triwave8', which produces a
    // symmetrical up-and-down triangle sawtooth waveform, except that this
    // function produces a triangle wave with a faster attack and a slower decay
    if (cat) //twinklecat, variant where the leds instantly turn on
    {
      bright = 255 - ph;
    } else { //vanilla twinklefox
      if (ph < 86) {
      bright = ph * 3;
      } else {
        ph -= 86;
        bright = 255 - (ph + (ph/2));
      }
    }
  }

  uint8_t hue = slowcycle8 - salt;
  CRGB c;
  if (bright > 0) {
    c = ColorFromPalette(SEGPALETTE, hue, bright, NOBLEND);
    if(COOL_LIKE_INCANDESCENT == 1) {
      // This code takes a pixel, and if its in the 'fading down'
      // part of the cycle, it adjusts the color a little bit like the
      // way that incandescent bulbs fade toward 'red' as they dim.
      if (fastcycle8 >= 128)
      {
        uint8_t cooling = (fastcycle8 - 128) >> 4;
        c.g = qsub8(c.g, cooling);
        c.b = qsub8(c.b, cooling * 2);
      }
    }
  } else {
    c = CRGB::Black;
  }
  return c;
}

//  This function loops over each pixel, calculates the
//  adjusted 'clock' that this pixel should use, and calls
//  "CalculateOneTwinkle" on each pixel.  It then displays
//  either the twinkle color of the background color,
//  whichever is brighter.
uint16_t twinklefox_base(bool cat)
{
  // "PRNG16" is the pseudorandom number generator
  // It MUST be reset to the same starting value each time
  // this function is called, so that the sequence of 'random'
  // numbers that it generates is (paradoxically) stable.
  uint16_t PRNG16 = 11337;

  // Calculate speed
  if (SEGMENT.speed > 100) SEGENV.aux0 = 3 + ((255 - SEGMENT.speed) >> 3);
  else SEGENV.aux0 = 22 + ((100 - SEGMENT.speed) >> 1);

  // Set up the background color, "bg".
  CRGB bg = CRGB(SEGCOLOR(1));
  uint8_t bglight = bg.getAverageLight();
  if (bglight > 64) {
    bg.nscale8_video(16); // very bright, so scale to 1/16th
  } else if (bglight > 16) {
    bg.nscale8_video(64); // not that bright, so scale to 1/4th
  } else {
    bg.nscale8_video(86); // dim, scale to 1/3rd.
  }

  uint8_t backgroundBrightness = bg.getAverageLight();

  for (int i = 0; i < SEGLEN; i++) {

    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    uint16_t myclockoffset16= PRNG16; // use that number as clock offset
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    // use that number as clock speed adjustment factor (in 8ths, from 8/8ths to 23/8ths)
    uint8_t myspeedmultiplierQ5_3 =  ((((PRNG16 & 0xFF)>>4) + (PRNG16 & 0x0F)) & 0x0F) + 0x08;
    uint32_t myclock30 = (uint32_t)((strip.now * myspeedmultiplierQ5_3) >> 3) + myclockoffset16;
    uint8_t  myunique8 = PRNG16 >> 8; // get 'salt' value for this pixel

    // We now have the adjusted 'clock' for this pixel, now we call
    // the function that computes what color the pixel should be based
    // on the "brightness = f( time )" idea.
    CRGB c = twinklefox_one_twinkle(myclock30, myunique8, cat);

    uint8_t cbright = c.getAverageLight();
    int16_t deltabright = cbright - backgroundBrightness;
    if (deltabright >= 32 || (!bg)) {
      // If the new pixel is significantly brighter than the background color,
      // use the new color.
      SEGMENT.setPixelColor(i, c.red, c.green, c.blue);
    } else if (deltabright > 0) {
      // If the new pixel is just slightly brighter than the background color,
      // mix a blend of the new color and the background color
      SEGMENT.setPixelColor(i, color_blend(RGBW32(bg.r,bg.g,bg.b,0), RGBW32(c.r,c.g,c.b,0), deltabright * 8));
    } else {
      // if the new pixel is not at all brighter than the background color,
      // just use the background color.
      SEGMENT.setPixelColor(i, bg.r, bg.g, bg.b);
    }
  }
  return FRAMETIME;
}


uint16_t mode_twinklefox()
{
  return twinklefox_base(false);
}
static const char _data_FX_MODE_TWINKLEFOX[] PROGMEM = "Twinklefox@!,Twinkle rate;;!";


uint16_t mode_twinklecat()
{
  return twinklefox_base(true);
}
static const char _data_FX_MODE_TWINKLECAT[] PROGMEM = "Twinklecat@!,Twinkle rate;;!";


//inspired by https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/#LEDStripEffectBlinkingHalloweenEyes
uint16_t mode_halloween_eyes()
{
  const uint16_t maxWidth = strip.isMatrix ? SEGMENT.virtualWidth() : SEGLEN;
  const uint16_t HALLOWEEN_EYE_SPACE = MAX(2, strip.isMatrix ? SEGMENT.virtualWidth()>>4: SEGLEN>>5);
  const uint16_t HALLOWEEN_EYE_WIDTH = HALLOWEEN_EYE_SPACE/2;
  uint16_t eyeLength = (2*HALLOWEEN_EYE_WIDTH) + HALLOWEEN_EYE_SPACE;
  if (eyeLength >= maxWidth) return mode_static(); //bail if segment too short

  if (!SEGMENT.check2) SEGMENT.fill(SEGCOLOR(1)); //fill background

  uint8_t state = SEGENV.aux1 >> 8;
  uint16_t stateTime = SEGENV.call;
  if (stateTime == 0) stateTime = 2000;

  if (state == 0) { //spawn eyes
    SEGENV.aux0 = random16(0, maxWidth - eyeLength - 1); //start pos
    SEGENV.aux1 = random8(); //color
    if (strip.isMatrix) SEGMENT.offset = random16(SEGMENT.virtualHeight()-1); // a hack: reuse offset since it is not used in matrices
    state = 1;
  }

  if (state < 2) { //fade eyes
    uint16_t startPos    = SEGENV.aux0;
    uint16_t start2ndEye = startPos + HALLOWEEN_EYE_WIDTH + HALLOWEEN_EYE_SPACE;

    uint32_t fadestage = (strip.now - SEGENV.step)*255 / stateTime;
    if (fadestage > 255) fadestage = 255;
    uint32_t c = color_blend(SEGMENT.color_from_palette(SEGENV.aux1 & 0xFF, false, false, 0), SEGCOLOR(1), fadestage);

    for (int i = 0; i < HALLOWEEN_EYE_WIDTH; i++) {
      if (strip.isMatrix) {
        SEGMENT.setPixelColorXY(startPos    + i, SEGMENT.offset, c);
        SEGMENT.setPixelColorXY(start2ndEye + i, SEGMENT.offset, c);
      } else {
        SEGMENT.setPixelColor(startPos    + i, c);
        SEGMENT.setPixelColor(start2ndEye + i, c);
      }
    }
  }

  if (strip.now - SEGENV.step > stateTime) {
    state++;
    if (state > 2) state = 0;

    if (state < 2) {
      stateTime = 100 + SEGMENT.intensity*10; //eye fade time
    } else {
      uint16_t eyeOffTimeBase = (256 - SEGMENT.speed)*10;
      stateTime = eyeOffTimeBase + random16(eyeOffTimeBase);
    }
    SEGENV.step = strip.now;
    SEGENV.call = stateTime;
  }

  SEGENV.aux1 = (SEGENV.aux1 & 0xFF) + (state << 8); //save state

  return FRAMETIME;
}
static const char _data_FX_MODE_HALLOWEEN_EYES[] PROGMEM = "Halloween Eyes@Duration,Eye fade time,,,,,Overlay;!,!;!;12";


//Speed slider sets amount of LEDs lit, intensity sets unlit
uint16_t mode_static_pattern()
{
  uint16_t lit = 1 + SEGMENT.speed;
  uint16_t unlit = 1 + SEGMENT.intensity;
  bool drawingLit = true;
  uint16_t cnt = 0;

  for (int i = 0; i < SEGLEN; i++) {
    SEGMENT.setPixelColor(i, (drawingLit) ? SEGMENT.color_from_palette(i, true, PALETTE_SOLID_WRAP, 0) : SEGCOLOR(1));
    cnt++;
    if (cnt >= ((drawingLit) ? lit : unlit)) {
      cnt = 0;
      drawingLit = !drawingLit;
    }
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_STATIC_PATTERN[] PROGMEM = "Solid Pattern@Fg size,Bg size;Fg,!;!;;pal=0";


uint16_t mode_tri_static_pattern()
{
  uint8_t segSize = (SEGMENT.intensity >> 5) +1;
  uint8_t currSeg = 0;
  uint16_t currSegCount = 0;

  for (int i = 0; i < SEGLEN; i++) {
    if ( currSeg % 3 == 0 ) {
      SEGMENT.setPixelColor(i, SEGCOLOR(0));
    } else if( currSeg % 3 == 1) {
      SEGMENT.setPixelColor(i, SEGCOLOR(1));
    } else {
      SEGMENT.setPixelColor(i, (SEGCOLOR(2) > 0 ? SEGCOLOR(2) : WHITE));
    }
    currSegCount += 1;
    if (currSegCount >= segSize) {
      currSeg +=1;
      currSegCount = 0;
    }
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_TRI_STATIC_PATTERN[] PROGMEM = "Solid Pattern Tri@,Size;1,2,3;;;pal=0";


uint16_t spots_base(uint16_t threshold)
{
  if (!SEGMENT.check2) SEGMENT.fill(SEGCOLOR(1));

  uint16_t maxZones = SEGLEN >> 2;
  uint16_t zones = 1 + ((SEGMENT.intensity * maxZones) >> 8);
  uint16_t zoneLen = SEGLEN / zones;
  uint16_t offset = (SEGLEN - zones * zoneLen) >> 1;

  for (int z = 0; z < zones; z++)
  {
    uint16_t pos = offset + z * zoneLen;
    for (int i = 0; i < zoneLen; i++)
    {
      uint16_t wave = triwave16((i * 0xFFFF) / zoneLen);
      if (wave > threshold) {
        uint16_t index = 0 + pos + i;
        uint8_t s = (wave - threshold)*255 / (0xFFFF - threshold);
        SEGMENT.setPixelColor(index, color_blend(SEGMENT.color_from_palette(index, true, PALETTE_SOLID_WRAP, 0), SEGCOLOR(1), 255-s));
      }
    }
  }

  return FRAMETIME;
}


//Intensity slider sets number of "lights", speed sets LEDs per light
uint16_t mode_spots()
{
  return spots_base((255 - SEGMENT.speed) << 8);
}
static const char _data_FX_MODE_SPOTS[] PROGMEM = "Spots@,Width,,,,,Overlay;!,!;!";


//Intensity slider sets number of "lights", LEDs per light fade in and out
uint16_t mode_spots_fade()
{
  uint16_t counter = strip.now * ((SEGMENT.speed >> 2) +8);
  uint16_t t = triwave16(counter);
  uint16_t tr = (t >> 1) + (t >> 2);
  return spots_base(tr);
}
static const char _data_FX_MODE_SPOTS_FADE[] PROGMEM = "Spots Fade@Spread,Width,,,,,Overlay;!,!;!";


//each needs 12 bytes
typedef struct Ball {
  unsigned long lastBounceTime;
  float impactVelocity;
  float height;
} ball;

/*
*  Bouncing Balls Effect
*/
uint16_t mode_bouncing_balls(void) {
  //allocate segment data
  const uint16_t strips = SEGMENT.nrOfVStrips(); // adapt for 2D
  const size_t maxNumBalls = 16;
  uint16_t dataSize = sizeof(ball) * maxNumBalls;
  if (!SEGENV.allocateData(dataSize * strips)) return mode_static(); //allocation failed

  Ball* balls = reinterpret_cast<Ball*>(SEGENV.data);

  if (!SEGMENT.check2) SEGMENT.fill(SEGCOLOR(2) ? BLACK : SEGCOLOR(1));

  // virtualStrip idea by @ewowi (Ewoud Wijma)
  // requires virtual strip # to be embedded into upper 16 bits of index in setPixelColor()
  // the following functions will not work on virtual strips: fill(), fade_out(), fadeToBlack(), blur()
  struct virtualStrip {
    static void runStrip(size_t stripNr, Ball* balls) {
      // number of balls based on intensity setting to max of 7 (cycles colors)
      // non-chosen color is a random color
      uint16_t numBalls = (SEGMENT.intensity * (maxNumBalls - 1)) / 255 + 1; // minimum 1 ball
      const float gravity = -9.81; // standard value of gravity
      const bool hasCol2 = SEGCOLOR(2);
      const unsigned long time = millis();

      if (SEGENV.call == 0) {
        for (size_t i = 0; i < maxNumBalls; i++) balls[i].lastBounceTime = time;
      }

      for (size_t i = 0; i < numBalls; i++) {
        float timeSinceLastBounce = (time - balls[i].lastBounceTime)/((255-SEGMENT.speed)/64 +1);
        float timeSec = timeSinceLastBounce/1000.0f;
        balls[i].height = (0.5f * gravity * timeSec + balls[i].impactVelocity) * timeSec; // avoid use pow(x, 2) - its extremely slow !

        if (balls[i].height <= 0.0f) {
          balls[i].height = 0.0f;
          //damping for better effect using multiple balls
          float dampening = 0.9f - float(i)/float(numBalls * numBalls); // avoid use pow(x, 2) - its extremely slow !
          balls[i].impactVelocity = dampening * balls[i].impactVelocity;
          balls[i].lastBounceTime = time;

          if (balls[i].impactVelocity < 0.015f) {
            float impactVelocityStart = sqrtf(-2.0f * gravity) * random8(5,11)/10.0f; // randomize impact velocity
            balls[i].impactVelocity = impactVelocityStart;
          }
        } else if (balls[i].height > 1.0f) {
          continue; // do not draw OOB ball
        }

        uint32_t color = SEGCOLOR(0);
        if (SEGMENT.palette) {
          color = SEGMENT.color_wheel(i*(256/MAX(numBalls, 8)));
        } else if (hasCol2) {
          color = SEGCOLOR(i % NUM_COLORS);
        }

        int pos = roundf(balls[i].height * (SEGLEN - 1));
        if (SEGLEN<32) SEGMENT.setPixelColor(indexToVStrip(pos, stripNr), color); // encode virtual strip into index
        else           SEGMENT.setPixelColor(balls[i].height + (stripNr+1)*10.0f, color);
      }
    }
  };

  for (int stripNr=0; stripNr<strips; stripNr++)
    virtualStrip::runStrip(stripNr, &balls[stripNr * maxNumBalls]);

  return FRAMETIME;
}
static const char _data_FX_MODE_BOUNCINGBALLS[] PROGMEM = "Bouncing Balls@Gravity,# of balls,,,,,Overlay;!,!,!;!;1;m12=1"; //bar


/*
* Sinelon stolen from FASTLED examples
*/
uint16_t sinelon_base(bool dual, bool rainbow=false) {
  SEGMENT.fade_out(SEGMENT.intensity);
  uint16_t pos = beatsin16(SEGMENT.speed/10,0,SEGLEN-1);
  if (SEGENV.call == 0) SEGENV.aux0 = pos;
  uint32_t color1 = SEGMENT.color_from_palette(pos, true, false, 0);
  uint32_t color2 = SEGCOLOR(2);
  if (rainbow) {
    color1 = SEGMENT.color_wheel((pos & 0x07) * 32);
  }
  SEGMENT.setPixelColor(pos, color1);
  if (dual) {
    if (!color2) color2 = SEGMENT.color_from_palette(pos, true, false, 0);
    if (rainbow) color2 = color1; //rainbow
    SEGMENT.setPixelColor(SEGLEN-1-pos, color2);
  }
  if (SEGENV.aux0 != pos) {
    if (SEGENV.aux0 < pos) {
      for (int i = SEGENV.aux0; i < pos ; i++) {
        SEGMENT.setPixelColor(i, color1);
        if (dual) SEGMENT.setPixelColor(SEGLEN-1-i, color2);
      }
    } else {
      for (int i = SEGENV.aux0; i > pos ; i--) {
        SEGMENT.setPixelColor(i, color1);
        if (dual) SEGMENT.setPixelColor(SEGLEN-1-i, color2);
      }
    }
    SEGENV.aux0 = pos;
  }

  return FRAMETIME;
}


uint16_t mode_sinelon(void) {
  return sinelon_base(false);
}
static const char _data_FX_MODE_SINELON[] PROGMEM = "Sinelon@!,Trail;!,!,!;!";


uint16_t mode_sinelon_dual(void) {
  return sinelon_base(true);
}
static const char _data_FX_MODE_SINELON_DUAL[] PROGMEM = "Sinelon Dual@!,Trail;!,!,!;!";


uint16_t mode_sinelon_rainbow(void) {
  return sinelon_base(false, true);
}
static const char _data_FX_MODE_SINELON_RAINBOW[] PROGMEM = "Sinelon Rainbow@!,Trail;,,!;!";


// utility function that will add random glitter to SEGMENT
void glitter_base(uint8_t intensity, uint32_t col = ULTRAWHITE) {
  if (intensity > random8()) {
    if (SEGMENT.is2D()) {
      SEGMENT.setPixelColorXY(random16(SEGMENT.virtualWidth()),random16(SEGMENT.virtualHeight()), col);
    } else {
      SEGMENT.setPixelColor(random16(SEGLEN), col);
    }
  }
}

//Glitter with palette background, inspired by https://gist.github.com/kriegsman/062e10f7f07ba8518af6
uint16_t mode_glitter()
{
  if (!SEGMENT.check2) mode_palette(); // use "* Color 1" palette for solid background (replacing "Solid glitter")
  glitter_base(SEGMENT.intensity, SEGCOLOR(2) ? SEGCOLOR(2) : ULTRAWHITE);
  return FRAMETIME;
}
static const char _data_FX_MODE_GLITTER[] PROGMEM = "Glitter@!,!,,,,,Overlay;1,2,Glitter color;!;;pal=0,m12=0"; //pixels


//Solid colour background with glitter
uint16_t mode_solid_glitter()
{
  SEGMENT.fill(SEGCOLOR(0));
  glitter_base(SEGMENT.intensity, SEGCOLOR(2) ? SEGCOLOR(2) : ULTRAWHITE);
  return FRAMETIME;
}
static const char _data_FX_MODE_SOLID_GLITTER[] PROGMEM = "Solid Glitter@,!;Bg,,Glitter color;;;m12=0";


//each needs 19 bytes
//Spark type is used for popcorn, 1D fireworks, and drip
typedef struct Spark {
  float pos, posX;
  float vel, velX;
  uint16_t col;
  uint8_t colIndex;
} spark;

#define maxNumPopcorn 21 // max 21 on 16 segment ESP8266
/*
*  POPCORN
*  modified from https://github.com/kitesurfer1404/WS2812FX/blob/master/src/custom/Popcorn.h
*/
uint16_t mode_popcorn(void) {
  //allocate segment data
  uint16_t strips = SEGMENT.nrOfVStrips();
  uint16_t dataSize = sizeof(spark) * maxNumPopcorn;
  if (!SEGENV.allocateData(dataSize * strips)) return mode_static(); //allocation failed

  Spark* popcorn = reinterpret_cast<Spark*>(SEGENV.data);

  bool hasCol2 = SEGCOLOR(2);
  if (!SEGMENT.check2) SEGMENT.fill(hasCol2 ? BLACK : SEGCOLOR(1));

  struct virtualStrip {
    static void runStrip(uint16_t stripNr, Spark* popcorn) {
      float gravity = -0.0001 - (SEGMENT.speed/200000.0); // m/s/s
      gravity *= SEGLEN;

      uint8_t numPopcorn = SEGMENT.intensity*maxNumPopcorn/255;
      if (numPopcorn == 0) numPopcorn = 1;

      for(int i = 0; i < numPopcorn; i++) {
        if (popcorn[i].pos >= 0.0f) { // if kernel is active, update its position
          popcorn[i].pos += popcorn[i].vel;
          popcorn[i].vel += gravity;
        } else { // if kernel is inactive, randomly pop it
          if (random8() < 2) { // POP!!!
            popcorn[i].pos = 0.01f;

            uint16_t peakHeight = 128 + random8(128); //0-255
            peakHeight = (peakHeight * (SEGLEN -1)) >> 8;
            popcorn[i].vel = sqrtf(-2.0f * gravity * peakHeight);

            if (SEGMENT.palette)
            {
              popcorn[i].colIndex = random8();
            } else {
              byte col = random8(0, NUM_COLORS);
              if (!SEGCOLOR(2) || !SEGCOLOR(col)) col = 0;
              popcorn[i].colIndex = col;
            }
          }
        }
        if (popcorn[i].pos >= 0.0f) { // draw now active popcorn (either active before or just popped)
          uint32_t col = SEGMENT.color_wheel(popcorn[i].colIndex);
          if (!SEGMENT.palette && popcorn[i].colIndex < NUM_COLORS) col = SEGCOLOR(popcorn[i].colIndex);
          uint16_t ledIndex = popcorn[i].pos;
          if (ledIndex < SEGLEN) SEGMENT.setPixelColor(indexToVStrip(ledIndex, stripNr), col);
        }
      }
    }
  };

  for (int stripNr=0; stripNr<strips; stripNr++)
    virtualStrip::runStrip(stripNr, &popcorn[stripNr * maxNumPopcorn]);

  return FRAMETIME;
}
static const char _data_FX_MODE_POPCORN[] PROGMEM = "Popcorn@!,!,,,,,Overlay;!,!,!;!;;m12=1"; //bar


//values close to 100 produce 5Hz flicker, which looks very candle-y
//Inspired by https://github.com/avanhanegem/ArduinoCandleEffectNeoPixel
//and https://cpldcpu.wordpress.com/2016/01/05/reverse-engineering-a-real-candle/

uint16_t candle(bool multi)
{
  if (multi)
  {
    //allocate segment data
    uint16_t dataSize = (SEGLEN -1) *3; //max. 1365 pixels (ESP8266)
    if (!SEGENV.allocateData(dataSize)) return candle(false); //allocation failed
  }

  //max. flicker range controlled by intensity
  uint8_t valrange = SEGMENT.intensity;
  uint8_t rndval = valrange >> 1; //max 127

  //step (how much to move closer to target per frame) coarsely set by speed
  uint8_t speedFactor = 4;
  if (SEGMENT.speed > 252) { //epilepsy
    speedFactor = 1;
  } else if (SEGMENT.speed > 99) { //regular candle (mode called every ~25 ms, so 4 frames to have a new target every 100ms)
    speedFactor = 2;
  } else if (SEGMENT.speed > 49) { //slower fade
    speedFactor = 3;
  } //else 4 (slowest)

  uint16_t numCandles = (multi) ? SEGLEN : 1;

  for (int i = 0; i < numCandles; i++)
  {
    uint16_t d = 0; //data location

    uint8_t s = SEGENV.aux0, s_target = SEGENV.aux1, fadeStep = SEGENV.step;
    if (i > 0) {
      d = (i-1) *3;
      s = SEGENV.data[d]; s_target = SEGENV.data[d+1]; fadeStep = SEGENV.data[d+2];
    }
    if (fadeStep == 0) { //init vals
      s = 128; s_target = 130 + random8(4); fadeStep = 1;
    }

    bool newTarget = false;
    if (s_target > s) { //fade up
      s = qadd8(s, fadeStep);
      if (s >= s_target) newTarget = true;
    } else {
      s = qsub8(s, fadeStep);
      if (s <= s_target) newTarget = true;
    }

    if (newTarget) {
      s_target = random8(rndval) + random8(rndval); //between 0 and rndval*2 -2 = 252
      if (s_target < (rndval >> 1)) s_target = (rndval >> 1) + random8(rndval);
      uint8_t offset = (255 - valrange);
      s_target += offset;

      uint8_t dif = (s_target > s) ? s_target - s : s - s_target;

      fadeStep = dif >> speedFactor;
      if (fadeStep == 0) fadeStep = 1;
    }

     if (i > 0) {
      SEGMENT.setPixelColor(i, color_blend(SEGCOLOR(1), SEGMENT.color_from_palette(i, true, PALETTE_SOLID_WRAP, 0), s));

      SEGENV.data[d] = s; SEGENV.data[d+1] = s_target; SEGENV.data[d+2] = fadeStep;
    } else {
      for (int j = 0; j < SEGLEN; j++) {
        SEGMENT.setPixelColor(j, color_blend(SEGCOLOR(1), SEGMENT.color_from_palette(j, true, PALETTE_SOLID_WRAP, 0), s));
      }

      SEGENV.aux0 = s; SEGENV.aux1 = s_target; SEGENV.step = fadeStep;
    }
  }

  return FRAMETIME_FIXED;
}


uint16_t mode_candle()
{
  return candle(false);
}
static const char _data_FX_MODE_CANDLE[] PROGMEM = "Candle@!,!;!,!;!;1;sx=96,ix=224,pal=0";


uint16_t mode_candle_multi()
{
  return candle(true);
}
static const char _data_FX_MODE_CANDLE_MULTI[] PROGMEM = "Candle Multi@!,!;!,!;!;;sx=96,ix=224,pal=0";


/*
/ Fireworks in starburst effect
/ based on the video: https://www.reddit.com/r/arduino/comments/c3sd46/i_made_this_fireworks_effect_for_my_led_strips/
/ Speed sets frequency of new starbursts, intensity is the intensity of the burst
*/
#ifdef ESP8266
  #define STARBURST_MAX_FRAG   8 //52 bytes / star
#else
  #define STARBURST_MAX_FRAG  10 //60 bytes / star
#endif
//each needs 20+STARBURST_MAX_FRAG*4 bytes
typedef struct particle {
  CRGB     color;
  uint32_t birth  =0;
  uint32_t last   =0;
  float    vel    =0;
  uint16_t pos    =-1;
  float    fragment[STARBURST_MAX_FRAG];
} star;

uint16_t mode_starburst(void) {
  uint16_t maxData = FAIR_DATA_PER_SEG; //ESP8266: 256 ESP32: 640
  uint8_t segs = strip.getActiveSegmentsNum();
  if (segs <= (strip.getMaxSegments() /2)) maxData *= 2; //ESP8266: 512 if <= 8 segs ESP32: 1280 if <= 16 segs
  if (segs <= (strip.getMaxSegments() /4)) maxData *= 2; //ESP8266: 1024 if <= 4 segs ESP32: 2560 if <= 8 segs
  uint16_t maxStars = maxData / sizeof(star); //ESP8266: max. 4/9/19 stars/seg, ESP32: max. 10/21/42 stars/seg

  uint8_t numStars = 1 + (SEGLEN >> 3);
  if (numStars > maxStars) numStars = maxStars;
  uint16_t dataSize = sizeof(star) * numStars;

  if (!SEGENV.allocateData(dataSize)) return mode_static(); //allocation failed

  uint32_t it = millis();

  star* stars = reinterpret_cast<star*>(SEGENV.data);

  float          maxSpeed                = 375.0f;  // Max velocity
  float          particleIgnition        = 250.0f;  // How long to "flash"
  float          particleFadeTime        = 1500.0f; // Fade out time

  for (int j = 0; j < numStars; j++)
  {
    // speed to adjust chance of a burst, max is nearly always.
    if (random8((144-(SEGMENT.speed >> 1))) == 0 && stars[j].birth == 0)
    {
      // Pick a random color and location.
      uint16_t startPos = random16(SEGLEN-1);
      float multiplier = (float)(random8())/255.0 * 1.0;

      stars[j].color = CRGB(SEGMENT.color_wheel(random8()));
      stars[j].pos = startPos;
      stars[j].vel = maxSpeed * (float)(random8())/255.0 * multiplier;
      stars[j].birth = it;
      stars[j].last = it;
      // more fragments means larger burst effect
      int num = random8(3,6 + (SEGMENT.intensity >> 5));

      for (int i=0; i < STARBURST_MAX_FRAG; i++) {
        if (i < num) stars[j].fragment[i] = startPos;
        else stars[j].fragment[i] = -1;
      }
    }
  }

  if (!SEGMENT.check2) SEGMENT.fill(SEGCOLOR(1));

  for (int j=0; j<numStars; j++)
  {
    if (stars[j].birth != 0) {
      float dt = (it-stars[j].last)/1000.0;

      for (int i=0; i < STARBURST_MAX_FRAG; i++) {
        int var = i >> 1;

        if (stars[j].fragment[i] > 0) {
          //all fragments travel right, will be mirrored on other side
          stars[j].fragment[i] += stars[j].vel * dt * (float)var/3.0;
        }
      }
      stars[j].last = it;
      stars[j].vel -= 3*stars[j].vel*dt;
    }

    CRGB c = stars[j].color;

    // If the star is brand new, it flashes white briefly.
    // Otherwise it just fades over time.
    float fade = 0.0f;
    float age = it-stars[j].birth;

    if (age < particleIgnition) {
      c = CRGB(color_blend(WHITE, RGBW32(c.r,c.g,c.b,0), 254.5f*((age / particleIgnition))));
    } else {
      // Figure out how much to fade and shrink the star based on
      // its age relative to its lifetime
      if (age > particleIgnition + particleFadeTime) {
        fade = 1.0f;                  // Black hole, all faded out
        stars[j].birth = 0;
        c = CRGB(SEGCOLOR(1));
      } else {
        age -= particleIgnition;
        fade = (age / particleFadeTime);  // Fading star
        byte f = 254.5f*fade;
        c = CRGB(color_blend(RGBW32(c.r,c.g,c.b,0), SEGCOLOR(1), f));
      }
    }

    float particleSize = (1.0f - fade) * 2.0f;

    for (size_t index=0; index < STARBURST_MAX_FRAG*2; index++) {
      bool mirrored = index & 0x1;
      uint8_t i = index >> 1;
      if (stars[j].fragment[i] > 0) {
        float loc = stars[j].fragment[i];
        if (mirrored) loc -= (loc-stars[j].pos)*2;
        int start = loc - particleSize;
        int end = loc + particleSize;
        if (start < 0) start = 0;
        if (start == end) end++;
        if (end > SEGLEN) end = SEGLEN;
        for (int p = start; p < end; p++) {
          SEGMENT.setPixelColor(p, c.r, c.g, c.b);
        }
      }
    }
  }
  return FRAMETIME;
}
#undef STARBURST_MAX_FRAG
static const char _data_FX_MODE_STARBURST[] PROGMEM = "Fireworks Starburst@Chance,Fragments,,,,,Overlay;,!;!;;pal=11,m12=0";


/*
 * Exploding fireworks effect
 * adapted from: http://www.anirama.com/1000leds/1d-fireworks/
 * adapted for 2D WLED by blazoncek (Blaz Kristan (AKA blazoncek))
 */
uint16_t mode_exploding_fireworks(void)
{
  const uint16_t cols = strip.isMatrix ? SEGMENT.virtualWidth() : 1;
  const uint16_t rows = strip.isMatrix ? SEGMENT.virtualHeight() : SEGMENT.virtualLength();

  //allocate segment data
  uint16_t maxData = FAIR_DATA_PER_SEG; //ESP8266: 256 ESP32: 640
  uint8_t segs = strip.getActiveSegmentsNum();
  if (segs <= (strip.getMaxSegments() /2)) maxData *= 2; //ESP8266: 512 if <= 8 segs ESP32: 1280 if <= 16 segs
  if (segs <= (strip.getMaxSegments() /4)) maxData *= 2; //ESP8266: 1024 if <= 4 segs ESP32: 2560 if <= 8 segs
  int maxSparks = maxData / sizeof(spark); //ESP8266: max. 21/42/85 sparks/seg, ESP32: max. 53/106/213 sparks/seg

  uint16_t numSparks = min(2 + ((rows*cols) >> 1), maxSparks);
  uint16_t dataSize = sizeof(spark) * numSparks;
  if (!SEGENV.allocateData(dataSize + sizeof(float))) return mode_static(); //allocation failed
  float *dying_gravity = reinterpret_cast<float*>(SEGENV.data + dataSize);

  if (dataSize != SEGENV.aux1) { //reset to flare if sparks were reallocated (it may be good idea to reset segment if bounds change)
    *dying_gravity = 0.0f;
    SEGENV.aux0 = 0;
    SEGENV.aux1 = dataSize;
  }

  SEGMENT.fade_out(252);

  Spark* sparks = reinterpret_cast<Spark*>(SEGENV.data);
  Spark* flare = sparks; //first spark is flare data

  float gravity = -0.0004f - (SEGMENT.speed/800000.0f); // m/s/s
  gravity *= rows;

  if (SEGENV.aux0 < 2) { //FLARE
    if (SEGENV.aux0 == 0) { //init flare
      flare->pos = 0;
      flare->posX = strip.isMatrix ? random16(2,cols-3) : (SEGMENT.intensity > random8()); // will enable random firing side on 1D
      uint16_t peakHeight = 75 + random8(180); //0-255
      peakHeight = (peakHeight * (rows -1)) >> 8;
      flare->vel = sqrtf(-2.0f * gravity * peakHeight);
      flare->velX = strip.isMatrix ? (random8(9)-4)/32.f : 0; // no X velocity on 1D
      flare->col = 255; //brightness
      SEGENV.aux0 = 1;
    }

    // launch
    if (flare->vel > 12 * gravity) {
      // flare
      if (strip.isMatrix) SEGMENT.setPixelColorXY(int(flare->posX), rows - uint16_t(flare->pos) - 1, flare->col, flare->col, flare->col);
      else                SEGMENT.setPixelColor(int(flare->posX) ? rows - int(flare->pos) - 1 : int(flare->pos), flare->col, flare->col, flare->col);
      flare->pos  += flare->vel;
      flare->posX += flare->velX;
      flare->pos  = constrain(flare->pos, 0, rows-1);
      flare->posX = constrain(flare->posX, 0, cols-strip.isMatrix);
      flare->vel  += gravity;
      flare->col  -= 2;
    } else {
      SEGENV.aux0 = 2;  // ready to explode
    }
  } else if (SEGENV.aux0 < 4) {
    /*
     * Explode!
     *
     * Explosion happens where the flare ended.
     * Size is proportional to the height.
     */
    int nSparks = flare->pos + random8(4);
    nSparks = constrain(nSparks, 4, numSparks);

    // initialize sparks
    if (SEGENV.aux0 == 2) {
      for (int i = 1; i < nSparks; i++) {
        sparks[i].pos  = flare->pos;
        sparks[i].posX = flare->posX;
        sparks[i].vel  = (float(random16(20001)) / 10000.0f) - 0.9f; // from -0.9 to 1.1
        sparks[i].vel *= rows<32 ? 0.5f : 1; // reduce velocity for smaller strips
        sparks[i].velX = strip.isMatrix ? (float(random16(10001)) / 10000.0f) - 0.5f : 0; // from -0.5 to 0.5
        sparks[i].col  = 345;//abs(sparks[i].vel * 750.0); // set colors before scaling velocity to keep them bright
        //sparks[i].col = constrain(sparks[i].col, 0, 345);
        sparks[i].colIndex = random8();
        sparks[i].vel  *= flare->pos/rows; // proportional to height
        sparks[i].velX *= strip.isMatrix ? flare->posX/cols : 0; // proportional to width
        sparks[i].vel  *= -gravity *50;
      }
      //sparks[1].col = 345; // this will be our known spark
      *dying_gravity = gravity/2;
      SEGENV.aux0 = 3;
    }

    if (sparks[1].col > 4) {//&& sparks[1].pos > 0) { // as long as our known spark is lit, work with all the sparks
      for (int i = 1; i < nSparks; i++) {
        sparks[i].pos  += sparks[i].vel;
        sparks[i].posX += sparks[i].velX;
        sparks[i].vel  += *dying_gravity;
        sparks[i].velX += strip.isMatrix ? *dying_gravity : 0;
        if (sparks[i].col > 3) sparks[i].col -= 4;

        if (sparks[i].pos > 0 && sparks[i].pos < rows) {
          if (strip.isMatrix && !(sparks[i].posX >= 0 && sparks[i].posX < cols)) continue;
          uint16_t prog = sparks[i].col;
          uint32_t spColor = (SEGMENT.palette) ? SEGMENT.color_wheel(sparks[i].colIndex) : SEGCOLOR(0);
          CRGB c = CRGB::Black; //HeatColor(sparks[i].col);
          if (prog > 300) { //fade from white to spark color
            c = CRGB(color_blend(spColor, WHITE, (prog - 300)*5));
          } else if (prog > 45) { //fade from spark color to black
            c = CRGB(color_blend(BLACK, spColor, prog - 45));
            uint8_t cooling = (300 - prog) >> 5;
            c.g = qsub8(c.g, cooling);
            c.b = qsub8(c.b, cooling * 2);
          }
          if (strip.isMatrix) SEGMENT.setPixelColorXY(int(sparks[i].posX), rows - int(sparks[i].pos) - 1, c.red, c.green, c.blue);
          else                SEGMENT.setPixelColor(int(sparks[i].posX) ? rows - int(sparks[i].pos) - 1 : int(sparks[i].pos), c.red, c.green, c.blue);
        }
      }
      SEGMENT.blur(16);
      *dying_gravity *= .8f; // as sparks burn out they fall slower
    } else {
      SEGENV.aux0 = 6 + random8(10); //wait for this many frames
    }
  } else {
    SEGENV.aux0--;
    if (SEGENV.aux0 < 4) {
      SEGENV.aux0 = 0; //back to flare
    }
  }

  return FRAMETIME;
}
#undef MAX_SPARKS
static const char _data_FX_MODE_EXPLODING_FIREWORKS[] PROGMEM = "Fireworks 1D@Gravity,Firing side;!,!;!;12;pal=11,ix=128";


/*
 * Drip Effect
 * ported of: https://www.youtube.com/watch?v=sru2fXh4r7k
 */
uint16_t mode_drip(void)
{
  //allocate segment data
  uint16_t strips = SEGMENT.nrOfVStrips();
  const int maxNumDrops = 4;
  uint16_t dataSize = sizeof(spark) * maxNumDrops;
  if (!SEGENV.allocateData(dataSize * strips)) return mode_static(); //allocation failed
  Spark* drops = reinterpret_cast<Spark*>(SEGENV.data);

  if (!SEGMENT.check2) SEGMENT.fill(SEGCOLOR(1));

  struct virtualStrip {
    static void runStrip(uint16_t stripNr, Spark* drops) {

      uint8_t numDrops = 1 + (SEGMENT.intensity >> 6); // 255>>6 = 3

      float gravity = -0.0005 - (SEGMENT.speed/50000.0);
      gravity *= SEGLEN-1;
      int sourcedrop = 12;

      for (int j=0;j<numDrops;j++) {
        if (drops[j].colIndex == 0) { //init
          drops[j].pos = SEGLEN-1;    // start at end
          drops[j].vel = 0;           // speed
          drops[j].col = sourcedrop;  // brightness
          drops[j].colIndex = 1;      // drop state (0 init, 1 forming, 2 falling, 5 bouncing)
        }

        SEGMENT.setPixelColor(indexToVStrip(SEGLEN-1, stripNr), color_blend(BLACK,SEGCOLOR(0), sourcedrop));// water source
        if (drops[j].colIndex==1) {
          if (drops[j].col>255) drops[j].col=255;
          SEGMENT.setPixelColor(indexToVStrip(uint16_t(drops[j].pos), stripNr), color_blend(BLACK,SEGCOLOR(0),drops[j].col));

          drops[j].col += map(SEGMENT.speed, 0, 255, 1, 6); // swelling

          if (random8() < drops[j].col/10) {               // random drop
            drops[j].colIndex=2;               //fall
            drops[j].col=255;
          }
        }
        if (drops[j].colIndex > 1) {           // falling
          if (drops[j].pos > 0) {              // fall until end of segment
            drops[j].pos += drops[j].vel;
            if (drops[j].pos < 0) drops[j].pos = 0;
            drops[j].vel += gravity;           // gravity is negative

            for (int i=1;i<7-drops[j].colIndex;i++) { // some minor math so we don't expand bouncing droplets
              uint16_t pos = constrain(uint16_t(drops[j].pos) +i, 0, SEGLEN-1); //this is BAD, returns a pos >= SEGLEN occasionally
              SEGMENT.setPixelColor(indexToVStrip(pos, stripNr), color_blend(BLACK,SEGCOLOR(0),drops[j].col/i)); //spread pixel with fade while falling
            }

            if (drops[j].colIndex > 2) {       // during bounce, some water is on the floor
              SEGMENT.setPixelColor(indexToVStrip(0, stripNr), color_blend(SEGCOLOR(0),BLACK,drops[j].col));
            }
          } else {                             // we hit bottom
            if (drops[j].colIndex > 2) {       // already hit once, so back to forming
              drops[j].colIndex = 0;
              drops[j].col = sourcedrop;

            } else {

              if (drops[j].colIndex==2) {      // init bounce
                drops[j].vel = -drops[j].vel/4;// reverse velocity with damping
                drops[j].pos += drops[j].vel;
              }
              drops[j].col = sourcedrop*2;
              drops[j].colIndex = 5;           // bouncing
            }
          }
        }
      }
    }
  };

  for (int stripNr=0; stripNr<strips; stripNr++)
    virtualStrip::runStrip(stripNr, &drops[stripNr*maxNumDrops]);

  return FRAMETIME;
}
static const char _data_FX_MODE_DRIP[] PROGMEM = "Drip@Gravity,# of drips,,,,,Overlay;!,!;!;;m12=1"; //bar


/*
 * Tetris or Stacking (falling bricks) Effect
 * by Blaz Kristan (AKA blazoncek) (https://github.com/blazoncek, https://blaz.at/home)
 */
//20 bytes
typedef struct Tetris {
  float    pos;
  float    speed;
  uint8_t  col;   // color index
  uint16_t brick; // brick size in pixels
  uint16_t stack; // stack size in pixels
  uint32_t step;  // 2D-fication of SEGENV.step (state)
} tetris;

uint16_t mode_tetrix(void) {
  uint16_t strips = SEGMENT.nrOfVStrips(); // allow running on virtual strips (columns in 2D segment)
  uint16_t dataSize = sizeof(tetris);
  if (!SEGENV.allocateData(dataSize * strips)) return mode_static(); //allocation failed
  Tetris* drops = reinterpret_cast<Tetris*>(SEGENV.data);

  //if (SEGENV.call == 0) SEGMENT.fill(SEGCOLOR(1));  // will fill entire segment (1D or 2D), then use drop->step = 0 below

  // virtualStrip idea by @ewowi (Ewoud Wijma)
  // requires virtual strip # to be embedded into upper 16 bits of index in setPixelcolor()
  // the following functions will not work on virtual strips: fill(), fade_out(), fadeToBlack(), blur()
  struct virtualStrip {
    static void runStrip(size_t stripNr, Tetris *drop) {
      // initialize dropping on first call or segment full
      if (SEGENV.call == 0) {
        drop->stack = 0;                  // reset brick stack size
        drop->step = millis() + 2000;     // start by fading out strip
        if (SEGMENT.check1) drop->col = 0;// use only one color from palette
      }

      if (drop->step == 0) {              // init brick
        // speed calcualtion: a single brick should reach bottom of strip in X seconds
        // if the speed is set to 1 this should take 5s and at 255 it should take 0.25s
        // as this is dependant on SEGLEN it should be taken into account and the fact that effect runs every FRAMETIME s
        int speed = SEGMENT.speed ? SEGMENT.speed : random8(1,255);
        speed = map(speed, 1, 255, 5000, 250); // time taken for full (SEGLEN) drop
        drop->speed = float(SEGLEN * FRAMETIME) / float(speed); // set speed
        drop->pos   = SEGLEN;             // start at end of segment (no need to subtract 1)
        if (!SEGMENT.check1) drop->col = random8(0,15)<<4;   // limit color choices so there is enough HUE gap
        drop->step  = 1;                  // drop state (0 init, 1 forming, 2 falling)
        drop->brick = (SEGMENT.intensity ? (SEGMENT.intensity>>5)+1 : random8(1,5)) * (1+(SEGLEN>>6));  // size of brick
      }

      if (drop->step == 1) {              // forming
        if (random8()>>6) {               // random drop
          drop->step = 2;                 // fall
        }
      }

      if (drop->step == 2) {              // falling
        if (drop->pos > drop->stack) {    // fall until top of stack
          drop->pos -= drop->speed;       // may add gravity as: speed += gravity
          if (int(drop->pos) < int(drop->stack)) drop->pos = drop->stack;
          for (int i=int(drop->pos); i<SEGLEN; i++) {
            uint32_t col = i<int(drop->pos)+drop->brick ? SEGMENT.color_from_palette(drop->col, false, false, 0) : SEGCOLOR(1);
            SEGMENT.setPixelColor(indexToVStrip(i, stripNr), col);
          }
        } else {                          // we hit bottom
          drop->step = 0;                 // proceed with next brick, go back to init
          drop->stack += drop->brick;     // increase the stack size
          if (drop->stack >= SEGLEN) drop->step = millis() + 2000; // fade out stack
        }
      }

      if (drop->step > 2) {               // fade strip
        drop->brick = 0;                  // reset brick size (no more growing)
        if (drop->step > millis()) {
          // allow fading of virtual strip
          for (int i=0; i<SEGLEN; i++) SEGMENT.blendPixelColor(indexToVStrip(i, stripNr), SEGCOLOR(1), 25); // 10% blend
        } else {
          drop->stack = 0;                // reset brick stack size
          drop->step = 0;                 // proceed with next brick
          if (SEGMENT.check1) drop->col += 8;   // gradually increase palette index
        }
      }
    }
  };

  for (int stripNr=0; stripNr<strips; stripNr++)
    virtualStrip::runStrip(stripNr, &drops[stripNr]);

  return FRAMETIME;
}
static const char _data_FX_MODE_TETRIX[] PROGMEM = "Tetrix@!,Width,,,,One color;!,!;!;;sx=0,ix=0,pal=11,m12=1";


/*
/ Plasma Effect
/ adapted from https://github.com/atuline/FastLED-Demos/blob/master/plasma/plasma.ino
*/
uint16_t mode_plasma(void) {
  // initialize phases on start
  if (SEGENV.call == 0) {
    SEGENV.aux0 = random8(0,2);  // add a bit of randomness
  }
  uint8_t thisPhase = beatsin8(6+SEGENV.aux0,-64,64);
  uint8_t thatPhase = beatsin8(7+SEGENV.aux0,-64,64);

  for (int i = 0; i < SEGLEN; i++) {   // For each of the LED's in the strand, set color &  brightness based on a wave as follows:
    uint8_t colorIndex = cubicwave8((i*(2+ 3*(SEGMENT.speed >> 5))+thisPhase) & 0xFF)/2   // factor=23 // Create a wave and add a phase change and add another wave with its own phase change.
                             + cos8((i*(1+ 2*(SEGMENT.speed >> 5))+thatPhase) & 0xFF)/2;  // factor=15 // Hey, you can even change the frequencies if you wish.
    uint8_t thisBright = qsub8(colorIndex, beatsin8(7,0, (128 - (SEGMENT.intensity>>1))));
    //CRGB color = ColorFromPalette(SEGPALETTE, colorIndex, thisBright, LINEARBLEND);
    //SEGMENT.setPixelColor(i, color.red, color.green, color.blue);
    SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(colorIndex, false, PALETTE_SOLID_WRAP, 0, thisBright));
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_PLASMA[] PROGMEM = "Plasma@Phase,!;!;!";


/*
 * Percentage display
 * Intesity values from 0-100 turn on the leds.
 */
uint16_t mode_percent(void) {

  uint8_t percent = SEGMENT.intensity;
  percent = constrain(percent, 0, 200);
  uint16_t active_leds = (percent < 100) ? SEGLEN * percent / 100.0
                                         : SEGLEN * (200 - percent) / 100.0;

  uint8_t size = (1 + ((SEGMENT.speed * SEGLEN) >> 11));
  if (SEGMENT.speed == 255) size = 255;

  if (percent <= 100) {
    for (int i = 0; i < SEGLEN; i++) {
    	if (i < SEGENV.aux1) {
        if (SEGMENT.check1)
          SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(map(percent,0,100,0,255), false, false, 0));
        else
          SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));
    	}
    	else {
        SEGMENT.setPixelColor(i, SEGCOLOR(1));
    	}
    }
  } else {
    for (int i = 0; i < SEGLEN; i++) {
    	if (i < (SEGLEN - SEGENV.aux1)) {
        SEGMENT.setPixelColor(i, SEGCOLOR(1));
    	}
    	else {
        if (SEGMENT.check1)
          SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(map(percent,100,200,255,0), false, false, 0));
        else
          SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));
    	}
    }
  }

  if(active_leds > SEGENV.aux1) {  // smooth transition to the target value
    SEGENV.aux1 += size;
    if (SEGENV.aux1 > active_leds) SEGENV.aux1 = active_leds;
  } else if (active_leds < SEGENV.aux1) {
    if (SEGENV.aux1 > size) SEGENV.aux1 -= size; else SEGENV.aux1 = 0;
    if (SEGENV.aux1 < active_leds) SEGENV.aux1 = active_leds;
  }

 	return FRAMETIME;
}
static const char _data_FX_MODE_PERCENT[] PROGMEM = "Percent@,% of fill,,,,One color;!,!;!";


/*
 * Modulates the brightness similar to a heartbeat
 * (unimplemented?) tries to draw an ECG aproximation on a 2D matrix
 */
uint16_t mode_heartbeat(void) {
  uint8_t bpm = 40 + (SEGMENT.speed >> 3);
  uint32_t msPerBeat = (60000L / bpm);
  uint32_t secondBeat = (msPerBeat / 3);
  uint32_t bri_lower = SEGENV.aux1;
  unsigned long beatTimer = strip.now - SEGENV.step;

  bri_lower = bri_lower * 2042 / (2048 + SEGMENT.intensity);
  SEGENV.aux1 = bri_lower;

  if ((beatTimer > secondBeat) && !SEGENV.aux0) { // time for the second beat?
    SEGENV.aux1 = UINT16_MAX; //3/4 bri
    SEGENV.aux0 = 1;
  }
  if (beatTimer > msPerBeat) { // time to reset the beat timer?
    SEGENV.aux1 = UINT16_MAX; //full bri
    SEGENV.aux0 = 0;
    SEGENV.step = strip.now;
  }

  for (int i = 0; i < SEGLEN; i++) {
    SEGMENT.setPixelColor(i, color_blend(SEGMENT.color_from_palette(i, true, PALETTE_SOLID_WRAP, 0), SEGCOLOR(1), 255 - (SEGENV.aux1 >> 8)));
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_HEARTBEAT[] PROGMEM = "Heartbeat@!,!;!,!;!;;m12=1";


//  "Pacifica"
//  Gentle, blue-green ocean waves.
//  December 2019, Mark Kriegsman and Mary Corey March.
//  For Dan.
//
//
// In this animation, there are four "layers" of waves of light.
//
// Each layer moves independently, and each is scaled separately.
//
// All four wave layers are added together on top of each other, and then
// another filter is applied that adds "whitecaps" of brightness where the
// waves line up with each other more.  Finally, another pass is taken
// over the led array to 'deepen' (dim) the blues and greens.
//
// The speed and scale and motion each layer varies slowly within independent
// hand-chosen ranges, which is why the code has a lot of low-speed 'beatsin8' functions
// with a lot of oddly specific numeric ranges.
//
// These three custom blue-green color palettes were inspired by the colors found in
// the waters off the southern coast of California, https://goo.gl/maps/QQgd97jjHesHZVxQ7
//
// Modified for WLED, based on https://github.com/FastLED/FastLED/blob/master/examples/Pacifica/Pacifica.ino
//
// Add one layer of waves into the led array
CRGB pacifica_one_layer(uint16_t i, CRGBPalette16& p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff)
{
  uint16_t ci = cistart;
  uint16_t waveangle = ioff;
  uint16_t wavescale_half = (wavescale >> 1) + 20;

  waveangle += ((120 + SEGMENT.intensity) * i); //original 250 * i
  uint16_t s16 = sin16(waveangle) + 32768;
  uint16_t cs = scale16(s16, wavescale_half) + wavescale_half;
  ci += (cs * i);
  uint16_t sindex16 = sin16(ci) + 32768;
  uint8_t sindex8 = scale16(sindex16, 240);
  return ColorFromPalette(p, sindex8, bri, LINEARBLEND);
}

uint16_t mode_pacifica()
{
  uint32_t nowOld = strip.now;

  CRGBPalette16 pacifica_palette_1 =
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117,
      0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x14554B, 0x28AA50 };
  CRGBPalette16 pacifica_palette_2 =
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117,
      0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x0C5F52, 0x19BE5F };
  CRGBPalette16 pacifica_palette_3 =
    { 0x000208, 0x00030E, 0x000514, 0x00061A, 0x000820, 0x000927, 0x000B2D, 0x000C33,
      0x000E39, 0x001040, 0x001450, 0x001860, 0x001C70, 0x002080, 0x1040BF, 0x2060FF };

  if (SEGMENT.palette) {
    pacifica_palette_1 = SEGPALETTE;
    pacifica_palette_2 = SEGPALETTE;
    pacifica_palette_3 = SEGPALETTE;
  }

  // Increment the four "color index start" counters, one for each wave layer.
  // Each is incremented at a different speed, and the speeds vary over time.
  uint16_t sCIStart1 = SEGENV.aux0, sCIStart2 = SEGENV.aux1, sCIStart3 = SEGENV.step, sCIStart4 = SEGENV.step >> 16;
  uint32_t deltams = (FRAMETIME >> 2) + ((FRAMETIME * SEGMENT.speed) >> 7);
  uint64_t deltat = (strip.now >> 2) + ((strip.now * SEGMENT.speed) >> 7);
  strip.now = deltat;

  uint16_t speedfactor1 = beatsin16(3, 179, 269);
  uint16_t speedfactor2 = beatsin16(4, 179, 269);
  uint32_t deltams1 = (deltams * speedfactor1) / 256;
  uint32_t deltams2 = (deltams * speedfactor2) / 256;
  uint32_t deltams21 = (deltams1 + deltams2) / 2;
  sCIStart1 += (deltams1 * beatsin88(1011,10,13));
  sCIStart2 -= (deltams21 * beatsin88(777,8,11));
  sCIStart3 -= (deltams1 * beatsin88(501,5,7));
  sCIStart4 -= (deltams2 * beatsin88(257,4,6));
  SEGENV.aux0 = sCIStart1; SEGENV.aux1 = sCIStart2;
  SEGENV.step = sCIStart4; SEGENV.step = (SEGENV.step << 16) + sCIStart3;

  // Clear out the LED array to a dim background blue-green
  //SEGMENT.fill(132618);

  uint8_t basethreshold = beatsin8( 9, 55, 65);
  uint8_t wave = beat8( 7 );

  for (int i = 0; i < SEGLEN; i++) {
    CRGB c = CRGB(2, 6, 10);
    // Render each of four layers, with different scales and speeds, that vary over time
    c += pacifica_one_layer(i, pacifica_palette_1, sCIStart1, beatsin16(3, 11 * 256, 14 * 256), beatsin8(10, 70, 130), 0-beat16(301));
    c += pacifica_one_layer(i, pacifica_palette_2, sCIStart2, beatsin16(4,  6 * 256,  9 * 256), beatsin8(17, 40,  80),   beat16(401));
    c += pacifica_one_layer(i, pacifica_palette_3, sCIStart3,                         6 * 256 , beatsin8(9, 10,38)   , 0-beat16(503));
    c += pacifica_one_layer(i, pacifica_palette_3, sCIStart4,                         5 * 256 , beatsin8(8, 10,28)   ,   beat16(601));

    // Add extra 'white' to areas where the four layers of light have lined up brightly
    uint8_t threshold = scale8( sin8( wave), 20) + basethreshold;
    wave += 7;
    uint8_t l = c.getAverageLight();
    if (l > threshold) {
      uint8_t overage = l - threshold;
      uint8_t overage2 = qadd8(overage, overage);
      c += CRGB(overage, overage2, qadd8(overage2, overage2));
    }

    //deepen the blues and greens
    c.blue  = scale8(c.blue,  145);
    c.green = scale8(c.green, 200);
    c |= CRGB( 2, 5, 7);

    SEGMENT.setPixelColor(i, c.red, c.green, c.blue);
  }

  strip.now = nowOld;
  return FRAMETIME;
}
static const char _data_FX_MODE_PACIFICA[] PROGMEM = "Pacifica@!,Angle;;!;;pal=51";


/*
 * Mode simulates a gradual sunrise
 */
uint16_t mode_sunrise() {
  //speed 0 - static sun
  //speed 1 - 60: sunrise time in minutes
  //speed 60 - 120 : sunset time in minutes - 60;
  //speed above: "breathing" rise and set
  if (SEGENV.call == 0 || SEGMENT.speed != SEGENV.aux0) {
    SEGENV.step = millis(); //save starting time, millis() because now can change from sync
    SEGENV.aux0 = SEGMENT.speed;
  }

  SEGMENT.fill(BLACK);
  uint16_t stage = 0xFFFF;

  uint32_t s10SinceStart = (millis() - SEGENV.step) /100; //tenths of seconds

  if (SEGMENT.speed > 120) { //quick sunrise and sunset
    uint16_t counter = (strip.now >> 1) * (((SEGMENT.speed -120) >> 1) +1);
    stage = triwave16(counter);
  } else if (SEGMENT.speed) { //sunrise
    uint8_t durMins = SEGMENT.speed;
    if (durMins > 60) durMins -= 60;
    uint32_t s10Target = durMins * 600;
    if (s10SinceStart > s10Target) s10SinceStart = s10Target;
    stage = map(s10SinceStart, 0, s10Target, 0, 0xFFFF);
    if (SEGMENT.speed > 60) stage = 0xFFFF - stage; //sunset
  }

  for (int i = 0; i <= SEGLEN/2; i++)
  {
    //default palette is Fire
    uint32_t c = SEGMENT.color_from_palette(0, false, true, 255); //background

    uint16_t wave = triwave16((i * stage) / SEGLEN);

    wave = (wave >> 8) + ((wave * SEGMENT.intensity) >> 15);

    if (wave > 240) { //clipped, full white sun
      c = SEGMENT.color_from_palette( 240, false, true, 255);
    } else { //transition
      c = SEGMENT.color_from_palette(wave, false, true, 255);
    }
    SEGMENT.setPixelColor(i, c);
    SEGMENT.setPixelColor(SEGLEN - i - 1, c);
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_SUNRISE[] PROGMEM = "Sunrise@Time [min],Width;;!;;sx=60";


/*
 * Effects by Andrew Tuline
 */
uint16_t phased_base(uint8_t moder) {                  // We're making sine waves here. By Andrew Tuline.

  uint8_t allfreq = 16;                                          // Base frequency.
  float *phase = reinterpret_cast<float*>(&SEGENV.step);         // Phase change value gets calculated (float fits into unsigned long).
  uint8_t cutOff = (255-SEGMENT.intensity);                      // You can change the number of pixels.  AKA INTENSITY (was 192).
  uint8_t modVal = 5;//SEGMENT.fft1/8+1;                         // You can change the modulus. AKA FFT1 (was 5).

  uint8_t index = strip.now/64;                                  // Set color rotation speed
  *phase += SEGMENT.speed/32.0;                                  // You can change the speed of the wave. AKA SPEED (was .4)

  for (int i = 0; i < SEGLEN; i++) {
    if (moder == 1) modVal = (inoise8(i*10 + i*10) /16);         // Let's randomize our mod length with some Perlin noise.
    uint16_t val = (i+1) * allfreq;                              // This sets the frequency of the waves. The +1 makes sure that led 0 is used.
    if (modVal == 0) modVal = 1;
    val += *phase * (i % modVal +1) /2;                          // This sets the varying phase change of the waves. By Andrew Tuline.
    uint8_t b = cubicwave8(val);                                 // Now we make an 8 bit sinewave.
    b = (b > cutOff) ? (b - cutOff) : 0;                         // A ternary operator to cutoff the light.
    SEGMENT.setPixelColor(i, color_blend(SEGCOLOR(1), SEGMENT.color_from_palette(index, false, false, 0), b));
    index += 256 / SEGLEN;
    if (SEGLEN > 256) index ++;                                  // Correction for segments longer than 256 LEDs
  }

  return FRAMETIME;
}


uint16_t mode_phased(void) {
  return phased_base(0);
}
static const char _data_FX_MODE_PHASED[] PROGMEM = "Phased@!,!;!,!;!";


uint16_t mode_phased_noise(void) {
  return phased_base(1);
}
static const char _data_FX_MODE_PHASEDNOISE[] PROGMEM = "Phased Noise@!,!;!,!;!";


uint16_t mode_twinkleup(void) {                 // A very short twinkle routine with fade-in and dual controls. By Andrew Tuline.
  random16_set_seed(535);                       // The randomizer needs to be re-set each time through the loop in order for the same 'random' numbers to be the same each time through.

  for (int i = 0; i<SEGLEN; i++) {
    uint8_t ranstart = random8();               // The starting value (aka brightness) for each pixel. Must be consistent each time through the loop for this to work.
    uint8_t pixBri = sin8(ranstart + 16 * strip.now/(256-SEGMENT.speed));
    if (random8() > SEGMENT.intensity) pixBri = 0;
    SEGMENT.setPixelColor(i, color_blend(SEGCOLOR(1), SEGMENT.color_from_palette(random8()+strip.now/100, false, PALETTE_SOLID_WRAP, 0), pixBri));
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_TWINKLEUP[] PROGMEM = "Twinkleup@!,Intensity;!,!;!;;m12=0";


// Peaceful noise that's slow and with gradually changing palettes. Does not support WLED palettes or default colours or controls.
uint16_t mode_noisepal(void) {                                    // Slow noise palette by Andrew Tuline.
  uint16_t scale = 15 + (SEGMENT.intensity >> 2); //default was 30
  //#define scale 30

  uint16_t dataSize = sizeof(CRGBPalette16) * 2; //allocate space for 2 Palettes (2 * 16 * 3 = 96 bytes)
  if (!SEGENV.allocateData(dataSize)) return mode_static(); //allocation failed

  CRGBPalette16* palettes = reinterpret_cast<CRGBPalette16*>(SEGENV.data);

  uint16_t changePaletteMs = 4000 + SEGMENT.speed *10; //between 4 - 6.5sec
  if (millis() - SEGENV.step > changePaletteMs)
  {
    SEGENV.step = millis();

    uint8_t baseI = random8();
    palettes[1] = CRGBPalette16(CHSV(baseI+random8(64), 255, random8(128,255)), CHSV(baseI+128, 255, random8(128,255)), CHSV(baseI+random8(92), 192, random8(128,255)), CHSV(baseI+random8(92), 255, random8(128,255)));
  }

  CRGB color;

  //EVERY_N_MILLIS(10) { //(don't have to time this, effect function is only called every 24ms)
  nblendPaletteTowardPalette(palettes[0], palettes[1], 48);               // Blend towards the target palette over 48 iterations.

  if (SEGMENT.palette > 0) palettes[0] = SEGPALETTE;

  for (int i = 0; i < SEGLEN; i++) {
    uint8_t index = inoise8(i*scale, SEGENV.aux0+i*scale);                // Get a value from the noise function. I'm using both x and y axis.
    color = ColorFromPalette(palettes[0], index, 255, LINEARBLEND);       // Use the my own palette.
    SEGMENT.setPixelColor(i, color.red, color.green, color.blue);
  }

  SEGENV.aux0 += beatsin8(10,1,4);                                        // Moving along the distance. Vary it a bit with a sine wave.

  return FRAMETIME;
}
static const char _data_FX_MODE_NOISEPAL[] PROGMEM = "Noise Pal@!,Scale;;!";


// Sine waves that have controllable phase change speed, frequency and cutoff. By Andrew Tuline.
// SEGMENT.speed ->Speed, SEGMENT.intensity -> Frequency (SEGMENT.fft1 -> Color change, SEGMENT.fft2 -> PWM cutoff)
//
uint16_t mode_sinewave(void) {             // Adjustable sinewave. By Andrew Tuline
  //#define qsuba(x, b)  ((x>b)?x-b:0)               // Analog Unsigned subtraction macro. if result <0, then => 0

  uint16_t colorIndex = strip.now /32;//(256 - SEGMENT.fft1);  // Amount of colour change.

  SEGENV.step += SEGMENT.speed/16;                   // Speed of animation.
  uint16_t freq = SEGMENT.intensity/4;//SEGMENT.fft2/8;                       // Frequency of the signal.

  for (int i=0; i<SEGLEN; i++) {                   // For each of the LED's in the strand, set a brightness based on a wave as follows:
    int pixBri = cubicwave8((i*freq)+SEGENV.step);//qsuba(cubicwave8((i*freq)+SEGENV.step), (255-SEGMENT.intensity)); // qsub sets a minimum value called thiscutoff. If < thiscutoff, then bright = 0. Otherwise, bright = 128 (as defined in qsub)..
    //setPixCol(i, i*colorIndex/255, pixBri);
    SEGMENT.setPixelColor(i, color_blend(SEGCOLOR(1), SEGMENT.color_from_palette(i*colorIndex/255, false, PALETTE_SOLID_WRAP, 0), pixBri));
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_SINEWAVE[] PROGMEM = "Sine";


/*
 * Best of both worlds from Palette and Spot effects. By Aircoookie
 */
uint16_t mode_flow(void)
{
  uint16_t counter = 0;
  if (SEGMENT.speed != 0)
  {
    counter = strip.now * ((SEGMENT.speed >> 2) +1);
    counter = counter >> 8;
  }

  uint16_t maxZones = SEGLEN / 6; //only looks good if each zone has at least 6 LEDs
  uint16_t zones = (SEGMENT.intensity * maxZones) >> 8;
  if (zones & 0x01) zones++; //zones must be even
  if (zones < 2) zones = 2;
  uint16_t zoneLen = SEGLEN / zones;
  uint16_t offset = (SEGLEN - zones * zoneLen) >> 1;

  SEGMENT.fill(SEGMENT.color_from_palette(-counter, false, true, 255));

  for (int z = 0; z < zones; z++)
  {
    uint16_t pos = offset + z * zoneLen;
    for (int i = 0; i < zoneLen; i++)
    {
      uint8_t colorIndex = (i * 255 / zoneLen) - counter;
      uint16_t led = (z & 0x01) ? i : (zoneLen -1) -i;
      if (SEGMENT.reverse) led = (zoneLen -1) -led;
      SEGMENT.setPixelColor(pos + led, SEGMENT.color_from_palette(colorIndex, false, true, 255));
    }
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_FLOW[] PROGMEM = "Flow@!,Zones;;!;;m12=1"; //vertical


/*
 * Dots waving around in a sine/pendulum motion.
 * Little pixel birds flying in a circle. By Aircoookie
 */
uint16_t mode_chunchun(void)
{
  SEGMENT.fade_out(254); // add a bit of trail
  uint16_t counter = strip.now * (6 + (SEGMENT.speed >> 4));
  uint16_t numBirds = 2 + (SEGLEN >> 3);  // 2 + 1/8 of a segment
  uint16_t span = (SEGMENT.intensity << 8) / numBirds;

  for (int i = 0; i < numBirds; i++)
  {
    counter -= span;
    uint16_t megumin = sin16(counter) + 0x8000;
    uint16_t bird = uint32_t(megumin * SEGLEN) >> 16;
    uint32_t c = SEGMENT.color_from_palette((i * 255)/ numBirds, false, false, 0);  // no palette wrapping
    bird = constrain(bird, 0, SEGLEN-1);
    SEGMENT.setPixelColor(bird, c);
  }
  return FRAMETIME;
}
static const char _data_FX_MODE_CHUNCHUN[] PROGMEM = "Chunchun@!,Gap size;!,!;!";


//13 bytes
typedef struct Spotlight {
  float speed;
  uint8_t colorIdx;
  int16_t position;
  unsigned long lastUpdateTime;
  uint8_t width;
  uint8_t type;
} spotlight;

#define SPOT_TYPE_SOLID       0
#define SPOT_TYPE_GRADIENT    1
#define SPOT_TYPE_2X_GRADIENT 2
#define SPOT_TYPE_2X_DOT      3
#define SPOT_TYPE_3X_DOT      4
#define SPOT_TYPE_4X_DOT      5
#define SPOT_TYPES_COUNT      6
#ifdef ESP8266
  #define SPOT_MAX_COUNT 17          //Number of simultaneous waves
#else
  #define SPOT_MAX_COUNT 49          //Number of simultaneous waves
#endif

/*
 * Spotlights moving back and forth that cast dancing shadows.
 * Shine this through tree branches/leaves or other close-up objects that cast
 * interesting shadows onto a ceiling or tarp.
 *
 * By Steve Pomeroy @xxv
 */
uint16_t mode_dancing_shadows(void)
{
  uint8_t numSpotlights = map(SEGMENT.intensity, 0, 255, 2, SPOT_MAX_COUNT);  // 49 on 32 segment ESP32, 17 on 16 segment ESP8266
  bool initialize = SEGENV.aux0 != numSpotlights;
  SEGENV.aux0 = numSpotlights;

  uint16_t dataSize = sizeof(spotlight) * numSpotlights;
  if (!SEGENV.allocateData(dataSize)) return mode_static(); //allocation failed
  Spotlight* spotlights = reinterpret_cast<Spotlight*>(SEGENV.data);

  SEGMENT.fill(BLACK);

  unsigned long time = millis();
  bool respawn = false;

  for (size_t i = 0; i < numSpotlights; i++) {
    if (!initialize) {
      // advance the position of the spotlight
      int16_t delta = (float)(time - spotlights[i].lastUpdateTime) *
                  (spotlights[i].speed * ((1.0 + SEGMENT.speed)/100.0));

      if (abs(delta) >= 1) {
        spotlights[i].position += delta;
        spotlights[i].lastUpdateTime = time;
      }

      respawn = (spotlights[i].speed > 0.0 && spotlights[i].position > (SEGLEN + 2))
             || (spotlights[i].speed < 0.0 && spotlights[i].position < -(spotlights[i].width + 2));
    }

    if (initialize || respawn) {
      spotlights[i].colorIdx = random8();
      spotlights[i].width = random8(1, 10);

      spotlights[i].speed = 1.0/random8(4, 50);

      if (initialize) {
        spotlights[i].position = random16(SEGLEN);
        spotlights[i].speed *= random8(2) ? 1.0 : -1.0;
      } else {
        if (random8(2)) {
          spotlights[i].position = SEGLEN + spotlights[i].width;
          spotlights[i].speed *= -1.0;
        }else {
          spotlights[i].position = -spotlights[i].width;
        }
      }

      spotlights[i].lastUpdateTime = time;
      spotlights[i].type = random8(SPOT_TYPES_COUNT);
    }

    uint32_t color = SEGMENT.color_from_palette(spotlights[i].colorIdx, false, false, 0);
    int start = spotlights[i].position;

    if (spotlights[i].width <= 1) {
      if (start >= 0 && start < SEGLEN) {
        SEGMENT.blendPixelColor(start, color, 128);
      }
    } else {
      switch (spotlights[i].type) {
        case SPOT_TYPE_SOLID:
          for (size_t j = 0; j < spotlights[i].width; j++) {
            if ((start + j) >= 0 && (start + j) < SEGLEN) {
              SEGMENT.blendPixelColor(start + j, color, 128);
            }
          }
        break;

        case SPOT_TYPE_GRADIENT:
          for (size_t j = 0; j < spotlights[i].width; j++) {
            if ((start + j) >= 0 && (start + j) < SEGLEN) {
              SEGMENT.blendPixelColor(start + j, color, cubicwave8(map(j, 0, spotlights[i].width - 1, 0, 255)));
            }
          }
        break;

        case SPOT_TYPE_2X_GRADIENT:
          for (size_t j = 0; j < spotlights[i].width; j++) {
            if ((start + j) >= 0 && (start + j) < SEGLEN) {
              SEGMENT.blendPixelColor(start + j, color, cubicwave8(2 * map(j, 0, spotlights[i].width - 1, 0, 255)));
            }
          }
        break;

        case SPOT_TYPE_2X_DOT:
          for (size_t j = 0; j < spotlights[i].width; j += 2) {
            if ((start + j) >= 0 && (start + j) < SEGLEN) {
              SEGMENT.blendPixelColor(start + j, color, 128);
            }
          }
        break;

        case SPOT_TYPE_3X_DOT:
          for (size_t j = 0; j < spotlights[i].width; j += 3) {
            if ((start + j) >= 0 && (start + j) < SEGLEN) {
              SEGMENT.blendPixelColor(start + j, color, 128);
            }
          }
        break;

        case SPOT_TYPE_4X_DOT:
          for (size_t j = 0; j < spotlights[i].width; j += 4) {
            if ((start + j) >= 0 && (start + j) < SEGLEN) {
              SEGMENT.blendPixelColor(start + j, color, 128);
            }
          }
        break;
      }
    }
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_DANCING_SHADOWS[] PROGMEM = "Dancing Shadows@!,# of shadows;!;!";


/*
  Imitates a washing machine, rotating same waves forward, then pause, then backward.
  By Stefan Seegel
*/
uint16_t mode_washing_machine(void) {
  float speed = tristate_square8(strip.now >> 7, 90, 15);
  float quot  = 32.0f - ((float)SEGMENT.speed / 16.0f);
  speed /= quot;

  SEGENV.step += (speed * 128.0f);

  for (int i=0; i<SEGLEN; i++) {
    uint8_t col = sin8(((SEGMENT.intensity / 25 + 1) * 255 * i / SEGLEN) + (SEGENV.step >> 7));
    SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(col, false, PALETTE_SOLID_WRAP, 3));
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_WASHING_MACHINE[] PROGMEM = "Washing Machine@!,!;;!";


/*
  Blends random colors across palette
  Modified, originally by Mark Kriegsman https://gist.github.com/kriegsman/1f7ccbbfa492a73c015e
*/
uint16_t mode_blends(void) {
  uint16_t pixelLen = SEGLEN > UINT8_MAX ? UINT8_MAX : SEGLEN;
  uint16_t dataSize = sizeof(uint32_t) * (pixelLen + 1);  // max segment length of 56 pixels on 16 segment ESP8266
  if (!SEGENV.allocateData(dataSize)) return mode_static(); //allocation failed
  uint32_t* pixels = reinterpret_cast<uint32_t*>(SEGENV.data);
  uint8_t blendSpeed = map(SEGMENT.intensity, 0, UINT8_MAX, 10, 128);
  uint8_t shift = (strip.now * ((SEGMENT.speed >> 3) +1)) >> 8;

  for (int i = 0; i < pixelLen; i++) {
    pixels[i] = color_blend(pixels[i], SEGMENT.color_from_palette(shift + quadwave8((i + 1) * 16), false, PALETTE_SOLID_WRAP, 255), blendSpeed);
    shift += 3;
  }

  uint16_t offset = 0;
  for (int i = 0; i < SEGLEN; i++) {
    SEGMENT.setPixelColor(i, pixels[offset++]);
    if (offset > pixelLen) offset = 0;
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_BLENDS[] PROGMEM = "Blends@Shift speed,Blend speed;;!";


/*
  TV Simulator
  Modified and adapted to WLED by Def3nder, based on "Fake TV Light for Engineers" by Phillip Burgess https://learn.adafruit.com/fake-tv-light-for-engineers/arduino-sketch
*/
//43 bytes
typedef struct TvSim {
  uint32_t totalTime = 0;
  uint32_t fadeTime  = 0;
  uint32_t startTime = 0;
  uint32_t elapsed   = 0;
  uint32_t pixelNum  = 0;
  uint16_t sliderValues = 0;
  uint32_t sceeneStart    = 0;
  uint32_t sceeneDuration = 0;
  uint16_t sceeneColorHue = 0;
  uint8_t  sceeneColorSat = 0;
  uint8_t  sceeneColorBri = 0;
  uint8_t  actualColorR = 0;
  uint8_t  actualColorG = 0;
  uint8_t  actualColorB = 0;
  uint16_t pr = 0; // Prev R, G, B
  uint16_t pg = 0;
  uint16_t pb = 0;
} tvSim;

uint16_t mode_tv_simulator(void) {
  uint16_t nr, ng, nb, r, g, b, i, hue;
  uint8_t  sat, bri, j;

  if (!SEGENV.allocateData(sizeof(tvSim))) return mode_static(); //allocation failed
  TvSim* tvSimulator = reinterpret_cast<TvSim*>(SEGENV.data);

  uint8_t colorSpeed     = map(SEGMENT.speed,     0, UINT8_MAX,  1, 20);
  uint8_t colorIntensity = map(SEGMENT.intensity, 0, UINT8_MAX, 10, 30);

  i = SEGMENT.speed << 8 | SEGMENT.intensity;
  if (i != tvSimulator->sliderValues) {
    tvSimulator->sliderValues = i;
    SEGENV.aux1 = 0;
  }

    // create a new sceene
    if (((millis() - tvSimulator->sceeneStart) >= tvSimulator->sceeneDuration) || SEGENV.aux1 == 0) {
      tvSimulator->sceeneStart    = millis();                                               // remember the start of the new sceene
      tvSimulator->sceeneDuration = random16(60* 250* colorSpeed, 60* 750 * colorSpeed);    // duration of a "movie sceene" which has similar colors (5 to 15 minutes with max speed slider)
      tvSimulator->sceeneColorHue = random16(   0, 768);                                    // random start color-tone for the sceene
      tvSimulator->sceeneColorSat = random8 ( 100, 130 + colorIntensity);                   // random start color-saturation for the sceene
      tvSimulator->sceeneColorBri = random8 ( 200, 240);                                    // random start color-brightness for the sceene
      SEGENV.aux1 = 1;
      SEGENV.aux0 = 0;
    }

    // slightly change the color-tone in this sceene
    if ( SEGENV.aux0 == 0) {
      // hue change in both directions
      j = random8(4 * colorIntensity);
      hue = (random8() < 128) ? ((j < tvSimulator->sceeneColorHue)       ? tvSimulator->sceeneColorHue - j : 767 - tvSimulator->sceeneColorHue - j) :  // negative
                                ((j + tvSimulator->sceeneColorHue) < 767 ? tvSimulator->sceeneColorHue + j : tvSimulator->sceeneColorHue + j - 767) ;  // positive

      // saturation
      j = random8(2 * colorIntensity);
      sat = (tvSimulator->sceeneColorSat - j) < 0 ? 0 : tvSimulator->sceeneColorSat - j;

      // brightness
      j = random8(100);
      bri = (tvSimulator->sceeneColorBri - j) < 0 ? 0 : tvSimulator->sceeneColorBri - j;

      // calculate R,G,B from HSV
      // Source: https://blog.adafruit.com/2012/03/14/constant-brightness-hsb-to-rgb-algorithm/
      { // just to create a local scope for  the variables
        uint8_t temp[5], n = (hue >> 8) % 3;
        uint8_t x = ((((hue & 255) * sat) >> 8) * bri) >> 8;
        uint8_t s = (  (256 - sat) * bri) >> 8;
        temp[0] = temp[3] =       s;
        temp[1] = temp[4] =   x + s;
        temp[2] =           bri - x;
        tvSimulator->actualColorR = temp[n + 2];
        tvSimulator->actualColorG = temp[n + 1];
        tvSimulator->actualColorB = temp[n    ];
      }
    }
    // Apply gamma correction, further expand to 16/16/16
    nr = (uint8_t)gamma8(tvSimulator->actualColorR) * 257; // New R/G/B
    ng = (uint8_t)gamma8(tvSimulator->actualColorG) * 257;
    nb = (uint8_t)gamma8(tvSimulator->actualColorB) * 257;

  if (SEGENV.aux0 == 0) {  // initialize next iteration
    SEGENV.aux0 = 1;

    // randomize total duration and fade duration for the actual color
    tvSimulator->totalTime = random16(250, 2500);                   // Semi-random pixel-to-pixel time
    tvSimulator->fadeTime  = random16(0, tvSimulator->totalTime);   // Pixel-to-pixel transition time
    if (random8(10) < 3) tvSimulator->fadeTime = 0;                 // Force scene cut 30% of time

    tvSimulator->startTime = millis();
  } // end of initialization

  // how much time is elapsed ?
  tvSimulator->elapsed = millis() - tvSimulator->startTime;

  // fade from prev volor to next color
  if (tvSimulator->elapsed < tvSimulator->fadeTime) {
    r = map(tvSimulator->elapsed, 0, tvSimulator->fadeTime, tvSimulator->pr, nr);
    g = map(tvSimulator->elapsed, 0, tvSimulator->fadeTime, tvSimulator->pg, ng);
    b = map(tvSimulator->elapsed, 0, tvSimulator->fadeTime, tvSimulator->pb, nb);
  } else { // Avoid divide-by-zero in map()
    r = nr;
    g = ng;
    b = nb;
  }

  // set strip color
  for (i = 0; i < SEGLEN; i++) {
    SEGMENT.setPixelColor(i, r >> 8, g >> 8, b >> 8);  // Quantize to 8-bit
  }

  // if total duration has passed, remember last color and restart the loop
  if ( tvSimulator->elapsed >= tvSimulator->totalTime) {
    tvSimulator->pr = nr; // Prev RGB = new RGB
    tvSimulator->pg = ng;
    tvSimulator->pb = nb;
    SEGENV.aux0 = 0;
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_TV_SIMULATOR[] PROGMEM = "TV Simulator@!,!;;";


/*
  Aurora effect
*/

//CONFIG
#ifdef ESP8266
  #define W_MAX_COUNT  9          //Number of simultaneous waves
#else
  #define W_MAX_COUNT 20          //Number of simultaneous waves
#endif
#define W_MAX_SPEED 6             //Higher number, higher speed
#define W_WIDTH_FACTOR 6          //Higher number, smaller waves

//24 bytes
class AuroraWave {
  private:
    uint16_t ttl;
    CRGB basecolor;
    float basealpha;
    uint16_t age;
    uint16_t width;
    float center;
    bool goingleft;
    float speed_factor;
    bool alive = true;

  public:
    void init(uint32_t segment_length, CRGB color) {
      ttl = random(500, 1501);
      basecolor = color;
      basealpha = random(60, 101) / (float)100;
      age = 0;
      width = random(segment_length / 20, segment_length / W_WIDTH_FACTOR); //half of width to make math easier
      if (!width) width = 1;
      center = random(101) / (float)100 * segment_length;
      goingleft = random(0, 2) == 0;
      speed_factor = (random(10, 31) / (float)100 * W_MAX_SPEED / 255);
      alive = true;
    }

    CRGB getColorForLED(int ledIndex) {
      if(ledIndex < center - width || ledIndex > center + width) return 0; //Position out of range of this wave

      CRGB rgb;

      //Offset of this led from center of wave
      //The further away from the center, the dimmer the LED
      float offset = ledIndex - center;
      if (offset < 0) offset = -offset;
      float offsetFactor = offset / width;

      //The age of the wave determines it brightness.
      //At half its maximum age it will be the brightest.
      float ageFactor = 0.1;
      if((float)age / ttl < 0.5) {
        ageFactor = (float)age / (ttl / 2);
      } else {
        ageFactor = (float)(ttl - age) / ((float)ttl * 0.5);
      }

      //Calculate color based on above factors and basealpha value
      float factor = (1 - offsetFactor) * ageFactor * basealpha;
      rgb.r = basecolor.r * factor;
      rgb.g = basecolor.g * factor;
      rgb.b = basecolor.b * factor;

      return rgb;
    };

    //Change position and age of wave
    //Determine if its sill "alive"
    void update(uint32_t segment_length, uint32_t speed) {
      if(goingleft) {
        center -= speed_factor * speed;
      } else {
        center += speed_factor * speed;
      }

      age++;

      if(age > ttl) {
        alive = false;
      } else {
        if(goingleft) {
          if(center + width < 0) {
            alive = false;
          }
        } else {
          if(center - width > segment_length) {
            alive = false;
          }
        }
      }
    };

    bool stillAlive() {
      return alive;
    };
};

uint16_t mode_aurora(void) {
  //aux1 = Wavecount
  //aux2 = Intensity in last loop

  AuroraWave* waves;

//TODO: I am not sure this is a correct way of handling memory allocation since if it fails on 1st run
// it will display static effect but on second run it may crash ESP since data will be nullptr

  if(SEGENV.aux0 != SEGMENT.intensity || SEGENV.call == 0) {
    //Intensity slider changed or first call
    SEGENV.aux1 = map(SEGMENT.intensity, 0, 255, 2, W_MAX_COUNT);
    SEGENV.aux0 = SEGMENT.intensity;

    if(!SEGENV.allocateData(sizeof(AuroraWave) * SEGENV.aux1)) { // 26 on 32 segment ESP32, 9 on 16 segment ESP8266
      return mode_static(); //allocation failed
    }

    waves = reinterpret_cast<AuroraWave*>(SEGENV.data);

    for (int i = 0; i < SEGENV.aux1; i++) {
      waves[i].init(SEGLEN, CRGB(SEGMENT.color_from_palette(random8(), false, false, random(0, 3))));
    }
  } else {
    waves = reinterpret_cast<AuroraWave*>(SEGENV.data);
  }

  for (int i = 0; i < SEGENV.aux1; i++) {
    //Update values of wave
    waves[i].update(SEGLEN, SEGMENT.speed);

    if(!(waves[i].stillAlive())) {
      //If a wave dies, reinitialize it starts over.
      waves[i].init(SEGLEN, CRGB(SEGMENT.color_from_palette(random8(), false, false, random(0, 3))));
    }
  }

  uint8_t backlight = 1; //dimmer backlight if less active colors
  if (SEGCOLOR(0)) backlight++;
  if (SEGCOLOR(1)) backlight++;
  if (SEGCOLOR(2)) backlight++;
  //Loop through LEDs to determine color
  for (int i = 0; i < SEGLEN; i++) {
    CRGB mixedRgb = CRGB(backlight, backlight, backlight);

    //For each LED we must check each wave if it is "active" at this position.
    //If there are multiple waves active on a LED we multiply their values.
    for (int  j = 0; j < SEGENV.aux1; j++) {
      CRGB rgb = waves[j].getColorForLED(i);

      if(rgb != CRGB(0)) {
        mixedRgb += rgb;
      }
    }

    SEGMENT.setPixelColor(i, mixedRgb[0], mixedRgb[1], mixedRgb[2]);
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_AURORA[] PROGMEM = "Aurora@!,!;1,2,3;!;;sx=24,pal=50";

// WLED-SR effects

/////////////////////////
//     Perlin Move     //
/////////////////////////
// 16 bit perlinmove. Use Perlin Noise instead of sinewaves for movement. By Andrew Tuline.
// Controls are speed, # of pixels, faderate.
uint16_t mode_perlinmove(void) {

  SEGMENT.fade_out(255-SEGMENT.custom1);
  for (int i = 0; i < SEGMENT.intensity/16 + 1; i++) {
    uint16_t locn = inoise16(millis()*128/(260-SEGMENT.speed)+i*15000, millis()*128/(260-SEGMENT.speed)); // Get a new pixel location from moving noise.
    uint16_t pixloc = map(locn, 50*256, 192*256, 0, SEGLEN-1);                                            // Map that to the length of the strand, and ensure we don't go over.
    SEGMENT.setPixelColor(pixloc, SEGMENT.color_from_palette(pixloc%255, false, PALETTE_SOLID_WRAP, 0));
  }

  return FRAMETIME;
} // mode_perlinmove()
static const char _data_FX_MODE_PERLINMOVE[] PROGMEM = "Perlin Move@!,# of pixels,Fade rate;!,!;!";


/////////////////////////
//     Waveins         //
/////////////////////////
// Uses beatsin8() + phase shifting. By: Andrew Tuline
uint16_t mode_wavesins(void) {

  for (int i = 0; i < SEGLEN; i++) {
    uint8_t bri = sin8(millis()/4 + i * SEGMENT.intensity);
    uint8_t index = beatsin8(SEGMENT.speed, SEGMENT.custom1, SEGMENT.custom1+SEGMENT.custom2, 0, i * (SEGMENT.custom3<<3)); // custom3 is reduced resolution slider
    //SEGMENT.setPixelColor(i, ColorFromPalette(SEGPALETTE, index, bri, LINEARBLEND));
    SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(index, false, PALETTE_SOLID_WRAP, 0, bri));
  }

  return FRAMETIME;
} // mode_waveins()
static const char _data_FX_MODE_WAVESINS[] PROGMEM = "Wavesins@!,Brightness variation,Starting color,Range of colors,Color variation;!;!";


//////////////////////////////
//     Flow Stripe          //
//////////////////////////////
// By: ldirko  https://editor.soulmatelights.com/gallery/392-flow-led-stripe , modifed by: Andrew Tuline
uint16_t mode_FlowStripe(void) {

  const uint16_t hl = SEGLEN * 10 / 13;
  uint8_t hue = millis() / (SEGMENT.speed+1);
  uint32_t t = millis() / (SEGMENT.intensity/8+1);

  for (int i = 0; i < SEGLEN; i++) {
    int c = (abs(i - hl) / hl) * 127;
    c = sin8(c);
    c = sin8(c / 2 + t);
    byte b = sin8(c + t/8);
    SEGMENT.setPixelColor(i, CHSV(b + hue, 255, 255));
  }

  return FRAMETIME;
} // mode_FlowStripe()
static const char _data_FX_MODE_FLOWSTRIPE[] PROGMEM = "Flow Stripe@Hue speed,Effect speed;;";


#ifndef WLED_DISABLE_2D
///////////////////////////////////////////////////////////////////////////////
//***************************  2D routines  ***********************************
#define XY(x,y) SEGMENT.XY(x,y)


// Black hole
uint16_t mode_2DBlackHole(void) {            // By: Stepko https://editor.soulmatelights.com/gallery/1012 , Modified by: Andrew Tuline
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();
  uint16_t x, y;

  // initialize on first call
  if (SEGENV.call == 0) {
    SEGMENT.setUpLeds();
    SEGMENT.fill(BLACK);
  }

  SEGMENT.fadeToBlackBy(16 + (SEGMENT.speed>>3)); // create fading trails
  float t = (float)(millis())/128;              // timebase
  // outer stars
  for (size_t i = 0; i < 8; i++) {
    x = beatsin8(SEGMENT.custom1>>3,   0, cols - 1, 0, ((i % 2) ? 128 : 0) + t * i);
    y = beatsin8(SEGMENT.intensity>>3, 0, rows - 1, 0, ((i % 2) ? 192 : 64) + t * i);
    SEGMENT.addPixelColorXY(x, y, CHSV(i*32, 255, 255));
  }
  // inner stars
  for (size_t i = 0; i < 4; i++) {
    x = beatsin8(SEGMENT.custom2>>3, cols/4, cols - 1 - cols/4, 0, ((i % 2) ? 128 : 0) + t * i);
    y = beatsin8(SEGMENT.custom3   , rows/4, rows - 1 - rows/4, 0, ((i % 2) ? 192 : 64) + t * i);
    SEGMENT.addPixelColorXY(x, y, CHSV(i*32, 255, 255));
  }
  // central white dot
  SEGMENT.setPixelColorXY(cols/2, rows/2, CHSV(0, 0, 255));
  // blur everything a bit
  SEGMENT.blur(16);

  return FRAMETIME;
} // mode_2DBlackHole()
static const char _data_FX_MODE_2DBLACKHOLE[] PROGMEM = "Black Hole@Fade rate,Outer Y freq.,Outer X freq.,Inner X freq.,Inner Y freq.;;;2";


////////////////////////////
//     2D Colored Bursts  //
////////////////////////////
uint16_t mode_2DColoredBursts() {              // By: ldirko   https://editor.soulmatelights.com/gallery/819-colored-bursts , modified by: Andrew Tuline
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  if (SEGENV.call == 0) {
    SEGMENT.setUpLeds();
    SEGMENT.fill(BLACK);
    SEGENV.aux0 = 0; // start with red hue
  }

  bool dot = SEGMENT.check3;
  bool grad = SEGMENT.check1;

  byte numLines = SEGMENT.intensity/16 + 1;

  SEGENV.aux0++;  // hue
  SEGMENT.fadeToBlackBy(40);
  for (size_t i = 0; i < numLines; i++) {
    byte x1 = beatsin8(2 + SEGMENT.speed/16, 0, (cols - 1));
    byte x2 = beatsin8(1 + SEGMENT.speed/16, 0, (cols - 1));
    byte y1 = beatsin8(5 + SEGMENT.speed/16, 0, (rows - 1), 0, i * 24);
    byte y2 = beatsin8(3 + SEGMENT.speed/16, 0, (rows - 1), 0, i * 48 + 64);
    CRGB color = ColorFromPalette(SEGPALETTE, i * 255 / numLines + (SEGENV.aux0&0xFF), 255, LINEARBLEND);

    byte xsteps = abs8(x1 - y1) + 1;
    byte ysteps = abs8(x2 - y2) + 1;
    byte steps = xsteps >= ysteps ? xsteps : ysteps;
    //Draw gradient line
    for (size_t i = 1; i <= steps; i++) {
      uint8_t rate = i * 255 / steps;
      byte dx = lerp8by8(x1, y1, rate);
      byte dy = lerp8by8(x2, y2, rate);
      //SEGMENT.setPixelColorXY(dx, dy, grad ? color.nscale8_video(255-rate) : color); // use addPixelColorXY for different look
      SEGMENT.addPixelColorXY(dx, dy, color); // use setPixelColorXY for different look
      if (grad) SEGMENT.fadePixelColorXY(dx, dy, rate);
    }

    if (dot) { //add white point at the ends of line
      SEGMENT.setPixelColorXY(x1, x2, WHITE);
      SEGMENT.setPixelColorXY(y1, y2, DARKSLATEGRAY);
    }
  }
  if (SEGMENT.custom3) SEGMENT.blur(SEGMENT.custom3/2);

  return FRAMETIME;
} // mode_2DColoredBursts()
static const char _data_FX_MODE_2DCOLOREDBURSTS[] PROGMEM = "Colored Bursts@Speed,# of lines,,,Blur,Gradient,,Dots;;!;2;c3=16";


/////////////////////
//      2D DNA     //
/////////////////////
uint16_t mode_2Ddna(void) {         // dna originally by by ldirko at https://pastebin.com/pCkkkzcs. Updated by Preyy. WLED conversion by Andrew Tuline.
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  if (SEGENV.call == 0) {
    SEGMENT.setUpLeds();
    SEGMENT.fill(BLACK);
  }

  SEGMENT.fadeToBlackBy(64);

  for (int i = 0; i < cols; i++) {
    SEGMENT.setPixelColorXY(i, beatsin8(SEGMENT.speed/8, 0, rows-1, 0, i*4    ), ColorFromPalette(SEGPALETTE, i*5+millis()/17, beatsin8(5, 55, 255, 0, i*10), LINEARBLEND));
    SEGMENT.setPixelColorXY(i, beatsin8(SEGMENT.speed/8, 0, rows-1, 0, i*4+128), ColorFromPalette(SEGPALETTE, i*5+128+millis()/17, beatsin8(5, 55, 255, 0, i*10+128), LINEARBLEND));
  }
  SEGMENT.blur(SEGMENT.intensity>>3);

  return FRAMETIME;
} // mode_2Ddna()
static const char _data_FX_MODE_2DDNA[] PROGMEM = "DNA@Scroll speed,Blur;;!;2";


/////////////////////////
//     2D DNA Spiral   //
/////////////////////////
uint16_t mode_2DDNASpiral() {               // By: ldirko  https://editor.soulmatelights.com/gallery/810 , modified by: Andrew Tuline
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  if (SEGENV.call == 0) {
    SEGMENT.setUpLeds();
    SEGMENT.fill(BLACK);
  }

  uint8_t speeds = SEGMENT.speed/2 + 1;
  uint8_t freq = SEGMENT.intensity/8;

  uint32_t ms = millis() / 20;
  SEGMENT.fadeToBlackBy(135);

  for (int i = 0; i < rows; i++) {
    uint16_t x  = beatsin8(speeds, 0, cols - 1, 0, i * freq) + beatsin8(speeds - 7, 0, cols - 1, 0, i * freq + 128);
    uint16_t x1 = beatsin8(speeds, 0, cols - 1, 0, 128 + i * freq) + beatsin8(speeds - 7, 0, cols - 1, 0, 128 + 64 + i * freq);
    uint8_t hue = (i * 128 / rows) + ms;
    // skip every 4th row every now and then (fade it more)
    if ((i + ms / 8) & 3) {
      // draw a gradient line between x and x1
      x = x / 2; x1 = x1 / 2;
      uint8_t steps = abs8(x - x1) + 1;
      for (size_t k = 1; k <= steps; k++) {
        uint8_t rate = k * 255 / steps;
        uint8_t dx = lerp8by8(x, x1, rate);
        //SEGMENT.setPixelColorXY(dx, i, ColorFromPalette(SEGPALETTE, hue, 255, LINEARBLEND).nscale8_video(rate));
        SEGMENT.addPixelColorXY(dx, i, ColorFromPalette(SEGPALETTE, hue, 255, LINEARBLEND)); // use setPixelColorXY for different look
        SEGMENT.fadePixelColorXY(dx, i, rate);
      }
      SEGMENT.setPixelColorXY(x, i, DARKSLATEGRAY);
      SEGMENT.setPixelColorXY(x1, i, WHITE);
    }
  }

  return FRAMETIME;
} // mode_2DDNASpiral()
static const char _data_FX_MODE_2DDNASPIRAL[] PROGMEM = "DNA Spiral@Scroll speed,Y frequency;;!;2";


/////////////////////////
//     2D Drift        //
/////////////////////////
uint16_t mode_2DDrift() {              // By: Stepko   https://editor.soulmatelights.com/gallery/884-drift , Modified by: Andrew Tuline
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  if (SEGENV.call == 0) {
    SEGMENT.setUpLeds();
    SEGMENT.fill(BLACK);
  }

  SEGMENT.fadeToBlackBy(128);

  const uint16_t maxDim = MAX(cols, rows)/2;
  unsigned long t = millis() / (32 - (SEGMENT.speed>>3));
  for (float i = 1; i < maxDim; i += 0.25) {
    float angle = radians(t * (maxDim - i));
    uint16_t myX = (cols>>1) + (uint16_t)(sin_t(angle) * i) + (cols%2);
    uint16_t myY = (rows>>1) + (uint16_t)(cos_t(angle) * i) + (rows%2);
    SEGMENT.setPixelColorXY(myX, myY, ColorFromPalette(SEGPALETTE, (i * 20) + (t / 20), 255, LINEARBLEND));
  }
  SEGMENT.blur(SEGMENT.intensity>>3);

  return FRAMETIME;
} // mode_2DDrift()
static const char _data_FX_MODE_2DDRIFT[] PROGMEM = "Drift@Rotation speed,Blur amount;;!;2";


//////////////////////////
//     2D Firenoise     //
//////////////////////////
uint16_t mode_2Dfirenoise(void) {               // firenoise2d. By Andrew Tuline. Yet another short routine.
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  if (SEGENV.call == 0) {
    SEGMENT.setUpLeds();
    SEGMENT.fill(BLACK);
  }

  uint16_t xscale = SEGMENT.intensity*4;
  uint32_t yscale = SEGMENT.speed*8;
  uint8_t indexx = 0;

  SEGPALETTE = CRGBPalette16( CRGB(0,0,0), CRGB(0,0,0), CRGB(0,0,0), CRGB(0,0,0),
                              CRGB::Red, CRGB::Red, CRGB::Red, CRGB::DarkOrange,
                              CRGB::DarkOrange,CRGB::DarkOrange, CRGB::Orange, CRGB::Orange,
                              CRGB::Yellow, CRGB::Orange, CRGB::Yellow, CRGB::Yellow);

  for (int j=0; j < cols; j++) {
    for (int i=0; i < rows; i++) {
      indexx = inoise8(j*yscale*rows/255, i*xscale+millis()/4);                                           // We're moving along our Perlin map.
      SEGMENT.setPixelColorXY(j, i, ColorFromPalette(SEGPALETTE, min(i*(indexx)>>4, 255), i*255/cols, LINEARBLEND)); // With that value, look up the 8 bit colour palette value and assign it to the current LED.
    } // for i
  } // for j

  return FRAMETIME;
} // mode_2Dfirenoise()
static const char _data_FX_MODE_2DFIRENOISE[] PROGMEM = "Firenoise@X scale,Y scale;;!;2";


//////////////////////////////
//     2D Frizzles          //
//////////////////////////////
uint16_t mode_2DFrizzles(void) {                 // By: Stepko https://editor.soulmatelights.com/gallery/640-color-frizzles , Modified by: Andrew Tuline
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  if (SEGENV.call == 0) {
    SEGMENT.setUpLeds();
    SEGMENT.fill(BLACK);
  }

  SEGMENT.fadeToBlackBy(16);
  for (size_t i = 8; i > 0; i--) {
    SEGMENT.addPixelColorXY(beatsin8(SEGMENT.speed/8 + i, 0, cols - 1),
                            beatsin8(SEGMENT.intensity/8 - i, 0, rows - 1),
                            ColorFromPalette(SEGPALETTE, beatsin8(12, 0, 255), 255, LINEARBLEND));
  }
  SEGMENT.blur(SEGMENT.custom1>>3);

  return FRAMETIME;
} // mode_2DFrizzles()
static const char _data_FX_MODE_2DFRIZZLES[] PROGMEM = "Frizzles@X frequency,Y frequency,Blur;;!;2";


///////////////////////////////////////////
//   2D Cellular Automata Game of life   //
///////////////////////////////////////////
typedef struct ColorCount {
  CRGB color;
  int8_t count;
} colorCount;

uint16_t mode_2Dgameoflife(void) { // Written by Ewoud Wijma, inspired by https://natureofcode.com/book/chapter-7-cellular-automata/ and https://github.com/DougHaber/nlife-color
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();
  const uint16_t dataSize = sizeof(CRGB) * SEGMENT.length();  // using width*height prevents reallocation if mirroring is enabled
  const uint16_t crcBufferLen = 2; //(SEGMENT.width() + SEGMENT.height())*71/100; // roughly sqrt(2)/2 for better repetition detection (Ewowi)

  if (!SEGENV.allocateData(dataSize + sizeof(uint16_t)*crcBufferLen)) return mode_static(); //allocation failed
  CRGB *prevLeds = reinterpret_cast<CRGB*>(SEGENV.data);
  uint16_t *crcBuffer = reinterpret_cast<uint16_t*>(SEGENV.data + dataSize); 

  CRGB backgroundColor = SEGCOLOR(1);

  if (SEGENV.call == 0) SEGMENT.setUpLeds();

  if (SEGENV.call == 0 || strip.now - SEGMENT.step > 3000) {
    SEGENV.step = strip.now;
    SEGENV.aux0 = 0;
    random16_set_seed(millis()>>2); //seed the random generator

    //give the leds random state and colors (based on intensity, colors from palette or all posible colors are chosen)
    for (int x = 0; x < cols; x++) for (int y = 0; y < rows; y++) {
      uint8_t state = random8()%2;
      if (state == 0)
        SEGMENT.setPixelColorXY(x,y, backgroundColor);
      else
        SEGMENT.setPixelColorXY(x,y, SEGMENT.color_from_palette(random8(), false, PALETTE_SOLID_WRAP, 255));
    }

    for (int y = 0; y < rows; y++) for (int x = 0; x < cols; x++) prevLeds[XY(x,y)] = CRGB::Black;
    memset(crcBuffer, 0, sizeof(uint16_t)*crcBufferLen);
  } else if (strip.now - SEGENV.step < FRAMETIME_FIXED * (uint32_t)map(SEGMENT.speed,0,255,64,4)) {
    // update only when appropriate time passes (in 42 FPS slots)
    return FRAMETIME;
  }

  //copy previous leds (save previous generation)
  //NOTE: using lossy getPixelColor() is a benefit as endlessly repeating patterns will eventually fade out causing a reset
  for (int x = 0; x < cols; x++) for (int y = 0; y < rows; y++) prevLeds[XY(x,y)] = SEGMENT.getPixelColorXY(x,y);

  //calculate new leds
  for (int x = 0; x < cols; x++) for (int y = 0; y < rows; y++) {

    colorCount colorsCount[9]; // count the different colors in the 3*3 matrix
    for (int i=0; i<9; i++) colorsCount[i] = {backgroundColor, 0}; // init colorsCount

    // iterate through neighbors and count them and their different colors
    int neighbors = 0;
    for (int i = -1; i <= 1; i++) for (int j = -1; j <= 1; j++) { // iterate through 3*3 matrix
      if (i==0 && j==0) continue; // ignore itself
      // wrap around segment
      int16_t xx = x+i, yy = y+j;
      if (x+i < 0) xx = cols-1; else if (x+i >= cols) xx = 0;
      if (y+j < 0) yy = rows-1; else if (y+j >= rows) yy = 0;

      uint16_t xy = XY(xx, yy); // previous cell xy to check
      // count different neighbours and colors
      if (prevLeds[xy] != backgroundColor) {
        neighbors++;
        bool colorFound = false;
        int k;
        for (k=0; k<9 && colorsCount[i].count != 0; k++)
          if (colorsCount[k].color == prevLeds[xy]) {
            colorsCount[k].count++;
            colorFound = true;
          }
        if (!colorFound) colorsCount[k] = {prevLeds[xy], 1}; //add new color found in the array
      }
    } // i,j

    // Rules of Life
    uint32_t col = prevLeds[XY(x,y)];
    uint32_t bgc = RGBW32(backgroundColor.r, backgroundColor.g, backgroundColor.b, 0);
    if      ((col != bgc) && (neighbors <  2)) SEGMENT.setPixelColorXY(x,y, bgc); // Loneliness
    else if ((col != bgc) && (neighbors >  3)) SEGMENT.setPixelColorXY(x,y, bgc); // Overpopulation
    else if ((col == bgc) && (neighbors == 3)) {                                  // Reproduction
      // find dominant color and assign it to a cell
      colorCount dominantColorCount = {backgroundColor, 0};
      for (int i=0; i<9 && colorsCount[i].count != 0; i++)
        if (colorsCount[i].count > dominantColorCount.count) dominantColorCount = colorsCount[i];
      // assign the dominant color w/ a bit of randomness to avoid "gliders"
      if (dominantColorCount.count > 0 && random8(128)) SEGMENT.setPixelColorXY(x,y, dominantColorCount.color);
    } else if ((col == bgc) && (neighbors == 2) && !random8(128)) {               // Mutation
      SEGMENT.setPixelColorXY(x,y, SEGMENT.color_from_palette(random8(), false, PALETTE_SOLID_WRAP, 255));
    }
    // else do nothing!
  } //x,y

  // calculate CRC16 of leds
  uint16_t crc = crc16((const unsigned char*)prevLeds, dataSize);
  // check if we had same CRC and reset if needed
  bool repetition = false;
  for (int i=0; i<crcBufferLen && !repetition; i++) repetition = (crc == crcBuffer[i]); // (Ewowi)
  // same CRC would mean image did not change or was repeating itself
  if (!repetition) SEGENV.step = strip.now; //if no repetition avoid reset
  // remember CRCs across frames
  crcBuffer[SEGENV.aux0] = crc;
  ++SEGENV.aux0 %= crcBufferLen;

  return FRAMETIME;
} // mode_2Dgameoflife()
static const char _data_FX_MODE_2DGAMEOFLIFE[] PROGMEM = "Game Of Life@!;!,!;!;2";


/////////////////////////
//     2D Hiphotic     //
/////////////////////////
uint16_t mode_2DHiphotic() {                        //  By: ldirko  https://editor.soulmatelights.com/gallery/810 , Modified by: Andrew Tuline
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();
  const uint32_t a = strip.now / ((SEGMENT.custom3>>1)+1);

  for (int x = 0; x < cols; x++) {
    for (int y = 0; y < rows; y++) {
      SEGMENT.setPixelColorXY(x, y, SEGMENT.color_from_palette(sin8(cos8(x * SEGMENT.speed/16 + a / 3) + sin8(y * SEGMENT.intensity/16 + a / 4) + a), false, PALETTE_SOLID_WRAP, 0));
    }
  }

  return FRAMETIME;
} // mode_2DHiphotic()
static const char _data_FX_MODE_2DHIPHOTIC[] PROGMEM = "Hiphotic@X scale,Y scale,,,Speed;!;!;2";


/////////////////////////
//     2D Julia        //
/////////////////////////
// Sliders are:
// intensity = Maximum number of iterations per pixel.
// Custom1 = Location of X centerpoint
// Custom2 = Location of Y centerpoint
// Custom3 = Size of the area (small value = smaller area)
typedef struct Julia {
  float xcen;
  float ycen;
  float xymag;
} julia;

uint16_t mode_2DJulia(void) {                           // An animated Julia set by Andrew Tuline.
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  if (!SEGENV.allocateData(sizeof(julia))) return mode_static();
  Julia* julias = reinterpret_cast<Julia*>(SEGENV.data);

  float reAl;
  float imAg;

  if (SEGENV.call == 0) {           // Reset the center if we've just re-started this animation.
    julias->xcen = 0.;
    julias->ycen = 0.;
    julias->xymag = 1.0;

    SEGMENT.custom1 = 128;              // Make sure the location widgets are centered to start.
    SEGMENT.custom2 = 128;
    SEGMENT.custom3 = 16;
    SEGMENT.intensity = 24;
  }

  julias->xcen  = julias->xcen  + (float)(SEGMENT.custom1 - 128)/100000.f;
  julias->ycen  = julias->ycen  + (float)(SEGMENT.custom2 - 128)/100000.f;
  julias->xymag = julias->xymag + (float)((SEGMENT.custom3 - 16)<<3)/100000.f; // reduced resolution slider
  if (julias->xymag < 0.01f) julias->xymag = 0.01f;
  if (julias->xymag > 1.0f) julias->xymag = 1.0f;

  float xmin = julias->xcen - julias->xymag;
  float xmax = julias->xcen + julias->xymag;
  float ymin = julias->ycen - julias->xymag;
  float ymax = julias->ycen + julias->xymag;

  // Whole set should be within -1.2,1.2 to -.8 to 1.
  xmin = constrain(xmin, -1.2f, 1.2f);
  xmax = constrain(xmax, -1.2f, 1.2f);
  ymin = constrain(ymin, -0.8f, 1.0f);
  ymax = constrain(ymax, -0.8f, 1.0f);

  float dx;                       // Delta x is mapped to the matrix size.
  float dy;                       // Delta y is mapped to the matrix size.

  int maxIterations = 15;         // How many iterations per pixel before we give up. Make it 8 bits to match our range of colours.
  float maxCalc = 16.0;           // How big is each calculation allowed to be before we give up.

  maxIterations = SEGMENT.intensity/2;


  // Resize section on the fly for some animaton.
  reAl = -0.94299f;               // PixelBlaze example
  imAg = 0.3162f;

  reAl += sin_t((float)millis()/305.f)/20.f;
  imAg += sin_t((float)millis()/405.f)/20.f;

  dx = (xmax - xmin) / (cols);     // Scale the delta x and y values to our matrix size.
  dy = (ymax - ymin) / (rows);

  // Start y
  float y = ymin;
  for (int j = 0; j < rows; j++) {

    // Start x
    float x = xmin;
    for (int i = 0; i < cols; i++) {

      // Now we test, as we iterate z = z^2 + c does z tend towards infinity?
      float a = x;
      float b = y;
      int iter = 0;

      while (iter < maxIterations) {    // Here we determine whether or not we're out of bounds.
        float aa = a * a;
        float bb = b * b;
        float len = aa + bb;
        if (len > maxCalc) {            // |z| = sqrt(a^2+b^2) OR z^2 = a^2+b^2 to save on having to perform a square root.
          break;  // Bail
        }

       // This operation corresponds to z -> z^2+c where z=a+ib c=(x,y). Remember to use 'foil'.
        b = 2*a*b + imAg;
        a = aa - bb + reAl;
        iter++;
      } // while

      // We color each pixel based on how long it takes to get to infinity, or black if it never gets there.
      if (iter == maxIterations) {
        SEGMENT.setPixelColorXY(i, j, 0);
      } else {
        SEGMENT.setPixelColorXY(i, j, SEGMENT.color_from_palette(iter*255/maxIterations, false, PALETTE_SOLID_WRAP, 0));
      }
      x += dx;
    }
    y += dy;
  }
//  SEGMENT.blur(64);

  return FRAMETIME;
} // mode_2DJulia()
static const char _data_FX_MODE_2DJULIA[] PROGMEM = "Julia@,Max iterations per pixel,X center,Y center,Area size;!;!;2;ix=24,c1=128,c2=128,c3=16";


//////////////////////////////
//     2D Lissajous         //
//////////////////////////////
uint16_t mode_2DLissajous(void) {            // By: Andrew Tuline
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  SEGMENT.fadeToBlackBy(SEGMENT.intensity);

  //for (int i=0; i < 4*(cols+rows); i ++) {
  for (int i=0; i < 256; i ++) {
    //float xlocn = float(sin8(now/4+i*(SEGMENT.speed>>5))) / 255.0f;
    //float ylocn = float(cos8(now/4+i*2)) / 255.0f;
    uint8_t xlocn = sin8(millis()/4+i*(SEGMENT.speed>>5));
    uint8_t ylocn = cos8(millis()/4+i*2);
    xlocn = map(xlocn,0,255,0,cols-1);
    ylocn = map(ylocn,0,255,0,rows-1);
    SEGMENT.setPixelColorXY(xlocn, ylocn, SEGMENT.color_from_palette(millis()/100+i, false, PALETTE_SOLID_WRAP, 0));
  }

  return FRAMETIME;
} // mode_2DLissajous()
static const char _data_FX_MODE_2DLISSAJOUS[] PROGMEM = "Lissajous@X frequency,Fade rate;!;!;2";


///////////////////////
//    2D Matrix      //
///////////////////////
uint16_t mode_2Dmatrix(void) {                  // Matrix2D. By Jeremy Williams. Adapted by Andrew Tuline & improved by merkisoft and ewowi.
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  if (SEGENV.call == 0) {
    SEGMENT.setUpLeds();
    SEGMENT.fill(BLACK);
  }

  uint8_t fade = map(SEGMENT.custom1, 0, 255, 50, 250);    // equals trail size
  uint8_t speed = (256-SEGMENT.speed) >> map(MIN(rows, 150), 0, 150, 0, 3);    // slower speeds for small displays

  CRGB spawnColor;
  CRGB trailColor;
  if (SEGMENT.check1) {
    spawnColor = SEGCOLOR(0);
    trailColor = SEGCOLOR(1);
  } else {
    spawnColor = CRGB(175,255,175);
    trailColor = CRGB(27,130,39);
  }

  if (strip.now - SEGENV.step >= speed) {
    SEGENV.step = strip.now;
    for (int row=rows-1; row>=0; row--) {
      for (int col=0; col<cols; col++) {
        CRGB pix = SEGMENT.getPixelColorXY(col, row);
        if (pix == spawnColor) {
          SEGMENT.setPixelColorXY(col, row, trailColor);  // create trail
          if (row < rows-1) SEGMENT.setPixelColorXY(col, row+1, spawnColor);
        } else {
          // fade other pixels
          SEGMENT.setPixelColorXY(col, row, pix.nscale8(fade));
        }
      }
    }

    // check for empty screen to ensure code spawn
    bool emptyScreen = true;
    for (int x=0; x<cols; x++) for (int y=0; y<rows; y++) {
      if (SEGMENT.getPixelColorXY(x,y)) {
        emptyScreen = false;
        break;
      }
    }

    // spawn new falling code
    if (random8() < SEGMENT.intensity || emptyScreen) {
      uint8_t spawnX = random8(cols);
      SEGMENT.setPixelColorXY(spawnX, 0, spawnColor);
    }
  } // if millis

  return FRAMETIME;
} // mode_2Dmatrix()
static const char _data_FX_MODE_2DMATRIX[] PROGMEM = "Matrix@!,Spawning rate,Trail,,,Custom color;Spawn,Trail;;2";


/////////////////////////
//     2D Metaballs    //
/////////////////////////
uint16_t mode_2Dmetaballs(void) {   // Metaballs by Stefan Petrick. Cannot have one of the dimensions be 2 or less. Adapted by Andrew Tuline.
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  float speed = 0.25f * (1+(SEGMENT.speed>>6));

  // get some 2 random moving points
  uint8_t x2 = map(inoise8(strip.now * speed, 25355, 685), 0, 255, 0, cols-1);
  uint8_t y2 = map(inoise8(strip.now * speed, 355, 11685), 0, 255, 0, rows-1);

  uint8_t x3 = map(inoise8(strip.now * speed, 55355, 6685), 0, 255, 0, cols-1);
  uint8_t y3 = map(inoise8(strip.now * speed, 25355, 22685), 0, 255, 0, rows-1);

  // and one Lissajou function
  uint8_t x1 = beatsin8(23 * speed, 0, cols-1);
  uint8_t y1 = beatsin8(28 * speed, 0, rows-1);

  for (int y = 0; y < rows; y++) {
    for (int x = 0; x < cols; x++) {
      // calculate distances of the 3 points from actual pixel
      // and add them together with weightening
      uint16_t dx = abs(x - x1);
      uint16_t dy = abs(y - y1);
      uint16_t dist = 2 * sqrt16((dx * dx) + (dy * dy));

      dx = abs(x - x2);
      dy = abs(y - y2);
      dist += sqrt16((dx * dx) + (dy * dy));

      dx = abs(x - x3);
      dy = abs(y - y3);
      dist += sqrt16((dx * dx) + (dy * dy));

      // inverse result
      byte color = dist ? 1000 / dist : 255;

      // map color between thresholds
      if (color > 0 and color < 60) {
        SEGMENT.setPixelColorXY(x, y, SEGMENT.color_from_palette(map(color * 9, 9, 531, 0, 255), false, PALETTE_SOLID_WRAP, 0));
      } else {
        SEGMENT.setPixelColorXY(x, y, SEGMENT.color_from_palette(0, false, PALETTE_SOLID_WRAP, 0));
      }
      // show the 3 points, too
      SEGMENT.setPixelColorXY(x1, y1, WHITE);
      SEGMENT.setPixelColorXY(x2, y2, WHITE);
      SEGMENT.setPixelColorXY(x3, y3, WHITE);
    }
  }

  return FRAMETIME;
} // mode_2Dmetaballs()
static const char _data_FX_MODE_2DMETABALLS[] PROGMEM = "Metaballs@!;;!;2";


//////////////////////
//    2D Noise      //
//////////////////////
uint16_t mode_2Dnoise(void) {                  // By Andrew Tuline
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  const uint16_t scale  = SEGMENT.intensity+2;

  for (int y = 0; y < rows; y++) {
    for (int x = 0; x < cols; x++) {
      uint8_t pixelHue8 = inoise8(x * scale, y * scale, millis() / (16 - SEGMENT.speed/16));
      SEGMENT.setPixelColorXY(x, y, ColorFromPalette(SEGPALETTE, pixelHue8));
    }
  }

  return FRAMETIME;
} // mode_2Dnoise()
static const char _data_FX_MODE_2DNOISE[] PROGMEM = "Noise2D@!,Scale;;!;2";


//////////////////////////////
//     2D Plasma Ball       //
//////////////////////////////
uint16_t mode_2DPlasmaball(void) {                   // By: Stepko https://editor.soulmatelights.com/gallery/659-plasm-ball , Modified by: Andrew Tuline
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  if (SEGENV.call == 0) {
    SEGMENT.setUpLeds();
    SEGMENT.fill(BLACK);
  }

  SEGMENT.fadeToBlackBy(SEGMENT.custom1>>2);

  float t = millis() / (33 - SEGMENT.speed/8);
  for (int i = 0; i < cols; i++) {
    uint16_t thisVal = inoise8(i * 30, t, t);
    uint16_t thisMax = map(thisVal, 0, 255, 0, cols-1);
    for (int j = 0; j < rows; j++) {
      uint16_t thisVal_ = inoise8(t, j * 30, t);
      uint16_t thisMax_ = map(thisVal_, 0, 255, 0, rows-1);
      uint16_t x = (i + thisMax_ - cols / 2);
      uint16_t y = (j + thisMax - cols / 2);
      uint16_t cx = (i + thisMax_);
      uint16_t cy = (j + thisMax);

      SEGMENT.addPixelColorXY(i, j, ((x - y > -2) && (x - y < 2)) ||
                                    ((cols - 1 - x - y) > -2 && (cols - 1 - x - y < 2)) ||
                                    (cols - cx == 0) ||
                                    (cols - 1 - cx == 0) ||
                                    ((rows - cy == 0) ||
                                    (rows - 1 - cy == 0)) ? ColorFromPalette(SEGPALETTE, beat8(5), thisVal, LINEARBLEND) : CRGB::Black);
    }
  }
  SEGMENT.blur(SEGMENT.custom2>>5);

  return FRAMETIME;
} // mode_2DPlasmaball()
static const char _data_FX_MODE_2DPLASMABALL[] PROGMEM = "Plasma Ball@Speed,,Fade,Blur;;!;2";


////////////////////////////////
//  2D Polar Lights           //
////////////////////////////////
//static float fmap(const float x, const float in_min, const float in_max, const float out_min, const float out_max) {
//  return (out_max - out_min) * (x - in_min) / (in_max - in_min) + out_min;
//}
uint16_t mode_2DPolarLights(void) {        // By: Kostyantyn Matviyevskyy  https://editor.soulmatelights.com/gallery/762-polar-lights , Modified by: Andrew Tuline
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  CRGBPalette16 auroraPalette  = {0x000000, 0x003300, 0x006600, 0x009900, 0x00cc00, 0x00ff00, 0x33ff00, 0x66ff00, 0x99ff00, 0xccff00, 0xffff00, 0xffcc00, 0xff9900, 0xff6600, 0xff3300, 0xff0000};

  if (SEGENV.call == 0) {
    SEGMENT.setUpLeds();
    SEGMENT.fill(BLACK);
    SEGENV.step = 0;
  }

  float adjustHeight = (float)map(rows, 8, 32, 28, 12); // maybe use mapf() ???
  uint16_t adjScale = map(cols, 8, 64, 310, 63);
/*
  if (SEGENV.aux1 != SEGMENT.custom1/12) {   // Hacky palette rotation. We need that black.
    SEGENV.aux1 = SEGMENT.custom1/12;
    for (int i = 0; i < 16; i++) {
      long ilk;
      ilk = (long)currentPalette[i].r << 16;
      ilk += (long)currentPalette[i].g << 8;
      ilk += (long)currentPalette[i].b;
      ilk = (ilk << SEGENV.aux1) | (ilk >> (24 - SEGENV.aux1));
      currentPalette[i].r = ilk >> 16;
      currentPalette[i].g = ilk >> 8;
      currentPalette[i].b = ilk;
    }
  }
*/
  uint16_t _scale = map(SEGMENT.intensity, 0, 255, 30, adjScale);
  byte _speed = map(SEGMENT.speed, 0, 255, 128, 16);

  for (int x = 0; x < cols; x++) {
    for (int y = 0; y < rows; y++) {
      SEGENV.step++;
      SEGMENT.setPixelColorXY(x, y, ColorFromPalette(auroraPalette,
                                      qsub8(
                                        inoise8((SEGENV.step%2) + x * _scale, y * 16 + SEGENV.step % 16, SEGENV.step / _speed),
                                        fabsf((float)rows / 2.0f - (float)y) * adjustHeight)));
    }
  }

  return FRAMETIME;
} // mode_2DPolarLights()
static const char _data_FX_MODE_2DPOLARLIGHTS[] PROGMEM = "Polar Lights@!,Scale;;;2";


/////////////////////////
//     2D Pulser       //
/////////////////////////
uint16_t mode_2DPulser(void) {                       // By: ldirko   https://editor.soulmatelights.com/gallery/878-pulse-test , modifed by: Andrew Tuline
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  if (SEGENV.call == 0) {
    SEGMENT.setUpLeds();
    SEGMENT.fill(BLACK);
  }

  SEGMENT.fadeToBlackBy(8 - (SEGMENT.intensity>>5));

  uint32_t a = strip.now / (18 - SEGMENT.speed / 16);
  uint16_t x = (a / 14) % cols;
  uint16_t y = map((sin8(a * 5) + sin8(a * 4) + sin8(a * 2)), 0, 765, rows-1, 0);
  SEGMENT.setPixelColorXY(x, y, ColorFromPalette(SEGPALETTE, map(y, 0, rows-1, 0, 255), 255, LINEARBLEND));

  SEGMENT.blur(1 + (SEGMENT.intensity>>4));

  return FRAMETIME;
} // mode_2DPulser()
static const char _data_FX_MODE_2DPULSER[] PROGMEM = "Pulser@!,Blur;;!;2";


/////////////////////////
//     2D Sindots      //
/////////////////////////
uint16_t mode_2DSindots(void) {                             // By: ldirko   https://editor.soulmatelights.com/gallery/597-sin-dots , modified by: Andrew Tuline
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  if (SEGENV.call == 0) {
    SEGMENT.setUpLeds();
    SEGMENT.fill(BLACK);
  }

  SEGMENT.fadeToBlackBy(SEGMENT.custom1>>3);

  byte t1 = millis() / (257 - SEGMENT.speed); // 20;
  byte t2 = sin8(t1) / 4 * 2;
  for (int i = 0; i < 13; i++) {
    byte x = sin8(t1 + i * SEGMENT.intensity/8)*(cols-1)/255;  // max index now 255x15/255=15!
    byte y = sin8(t2 + i * SEGMENT.intensity/8)*(rows-1)/255;  // max index now 255x15/255=15!
    SEGMENT.setPixelColorXY(x, y, ColorFromPalette(SEGPALETTE, i * 255 / 13, 255, LINEARBLEND));
  }
  SEGMENT.blur(SEGMENT.custom2>>3);

  return FRAMETIME;
} // mode_2DSindots()
static const char _data_FX_MODE_2DSINDOTS[] PROGMEM = "Sindots@!,Dot distance,Fade rate,Blur;;!;2";


//////////////////////////////
//     2D Squared Swirl     //
//////////////////////////////
// custom3 affects the blur amount.
uint16_t mode_2Dsquaredswirl(void) {            // By: Mark Kriegsman. https://gist.github.com/kriegsman/368b316c55221134b160
                                                          // Modifed by: Andrew Tuline
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  if (SEGENV.call == 0) {
    SEGMENT.setUpLeds();
    SEGMENT.fill(BLACK);
  }

  const uint8_t kBorderWidth = 2;

  SEGMENT.fadeToBlackBy(24);

  uint8_t blurAmount = SEGMENT.custom3>>1; // reduced resolution slider
  SEGMENT.blur(blurAmount);

  // Use two out-of-sync sine waves
  uint8_t i = beatsin8(19, kBorderWidth, cols-kBorderWidth);
  uint8_t j = beatsin8(22, kBorderWidth, cols-kBorderWidth);
  uint8_t k = beatsin8(17, kBorderWidth, cols-kBorderWidth);
  uint8_t m = beatsin8(18, kBorderWidth, rows-kBorderWidth);
  uint8_t n = beatsin8(15, kBorderWidth, rows-kBorderWidth);
  uint8_t p = beatsin8(20, kBorderWidth, rows-kBorderWidth);

  uint16_t ms = millis();

  SEGMENT.addPixelColorXY(i, m, ColorFromPalette(SEGPALETTE, ms/29, 255, LINEARBLEND));
  SEGMENT.addPixelColorXY(j, n, ColorFromPalette(SEGPALETTE, ms/41, 255, LINEARBLEND));
  SEGMENT.addPixelColorXY(k, p, ColorFromPalette(SEGPALETTE, ms/73, 255, LINEARBLEND));

  return FRAMETIME;
} // mode_2Dsquaredswirl()
static const char _data_FX_MODE_2DSQUAREDSWIRL[] PROGMEM = "Squared Swirl@,,,,Blur;;!;2";


//////////////////////////////
//     2D Sun Radiation     //
//////////////////////////////
uint16_t mode_2DSunradiation(void) {                   // By: ldirko https://editor.soulmatelights.com/gallery/599-sun-radiation  , modified by: Andrew Tuline
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  if (!SEGENV.allocateData(sizeof(byte)*(cols+2)*(rows+2))) return mode_static(); //allocation failed
  byte *bump = reinterpret_cast<byte*>(SEGENV.data);

  if (SEGENV.call == 0) {
    SEGMENT.setUpLeds();
    SEGMENT.fill(BLACK);
  }

  unsigned long t = millis() / 4;
  int index = 0;
  uint8_t someVal = SEGMENT.speed/4;             // Was 25.
  for (int j = 0; j < (rows + 2); j++) {
    for (int i = 0; i < (cols + 2); i++) {
      byte col = (inoise8_raw(i * someVal, j * someVal, t)) / 2;
      bump[index++] = col;
    }
  }

  int yindex = cols + 3;
  int16_t vly = -(rows / 2 + 1);
  for (int y = 0; y < rows; y++) {
    ++vly;
    int16_t vlx = -(cols / 2 + 1);
    for (int x = 0; x < cols; x++) {
      ++vlx;
      int8_t nx = bump[x + yindex + 1] - bump[x + yindex - 1];
      int8_t ny = bump[x + yindex + (cols + 2)] - bump[x + yindex - (cols + 2)];
      byte difx = abs8(vlx * 7 - nx);
      byte dify = abs8(vly * 7 - ny);
      int temp = difx * difx + dify * dify;
      int col = 255 - temp / 8; //8 its a size of effect
      if (col < 0) col = 0;
      SEGMENT.setPixelColorXY(x, y, HeatColor(col / (3.0f-(float)(SEGMENT.intensity)/128.f)));
    }
    yindex += (cols + 2);
  }

  return FRAMETIME;
} // mode_2DSunradiation()
static const char _data_FX_MODE_2DSUNRADIATION[] PROGMEM = "Sun Radiation@Variance,Brightness;;;2";


/////////////////////////
//     2D Tartan       //
/////////////////////////
uint16_t mode_2Dtartan(void) {          // By: Elliott Kember  https://editor.soulmatelights.com/gallery/3-tartan , Modified by: Andrew Tuline
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  if (SEGENV.call == 0) {
    SEGMENT.setUpLeds();
    SEGMENT.fill(BLACK);
  }

  uint8_t hue;
  int offsetX = beatsin16(3, -360, 360);
  int offsetY = beatsin16(2, -360, 360);

  for (int x = 0; x < cols; x++) {
    for (int y = 0; y < rows; y++) {
      hue = x * beatsin16(10, 1, 10) + offsetY;
      SEGMENT.setPixelColorXY(x, y, ColorFromPalette(SEGPALETTE, hue, sin8(x * SEGMENT.speed + offsetX) * sin8(x * SEGMENT.speed + offsetX) / 255, LINEARBLEND));
      hue = y * 3 + offsetX;
      SEGMENT.addPixelColorXY(x, y, ColorFromPalette(SEGPALETTE, hue, sin8(y * SEGMENT.intensity + offsetY) * sin8(y * SEGMENT.intensity + offsetY) / 255, LINEARBLEND));
    }
  }

  return FRAMETIME;
} // mode_2DTartan()
static const char _data_FX_MODE_2DTARTAN[] PROGMEM = "Tartan@X scale,Y scale;;!;2";


/////////////////////////
//     2D spaceships   //
/////////////////////////
uint16_t mode_2Dspaceships(void) {    //// Space ships by stepko (c)05.02.21 [https://editor.soulmatelights.com/gallery/639-space-ships], adapted by Blaz Kristan (AKA blazoncek)
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  if (SEGENV.call == 0) {
    SEGMENT.setUpLeds();
    SEGMENT.fill(BLACK);
  }

  uint32_t tb = strip.now >> 12;  // every ~4s
  if (tb > SEGENV.step) {
    int8_t dir = ++SEGENV.aux0;
    dir  += (int)random8(3)-1;
    if      (dir > 7) SEGENV.aux0 = 0;
    else if (dir < 0) SEGENV.aux0 = 7;
    else              SEGENV.aux0 = dir;
    SEGENV.step = tb + random8(4);
  }

  SEGMENT.fadeToBlackBy(map(SEGMENT.speed, 0, 255, 248, 16));
  SEGMENT.move(SEGENV.aux0, 1);

  for (size_t i = 0; i < 8; i++) {
    byte x = beatsin8(12 + i, 2, cols - 3);
    byte y = beatsin8(15 + i, 2, rows - 3);
    CRGB color = ColorFromPalette(SEGPALETTE, beatsin8(12 + i, 0, 255), 255);
    SEGMENT.addPixelColorXY(x, y, color);
    if (cols > 24 || rows > 24) {
      SEGMENT.addPixelColorXY(x+1, y, color);
      SEGMENT.addPixelColorXY(x-1, y, color);
      SEGMENT.addPixelColorXY(x, y+1, color);
      SEGMENT.addPixelColorXY(x, y-1, color);
    }
  }
  SEGMENT.blur(SEGMENT.intensity>>3);

  return FRAMETIME;
}
static const char _data_FX_MODE_2DSPACESHIPS[] PROGMEM = "Spaceships@!,Blur;;!;2";


/////////////////////////
//     2D Crazy Bees   //
/////////////////////////
//// Crazy bees by stepko (c)12.02.21 [https://editor.soulmatelights.com/gallery/651-crazy-bees], adapted by Blaz Kristan (AKA blazoncek)
#define MAX_BEES 5
uint16_t mode_2Dcrazybees(void) {
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  byte n = MIN(MAX_BEES, (rows * cols) / 256 + 1);

  typedef struct Bee {
    uint8_t posX, posY, aimX, aimY, hue;
    int8_t deltaX, deltaY, signX, signY, error;
    void aimed(uint16_t w, uint16_t h) {
      random16_set_seed(millis());
      aimX = random8(0, w);
      aimY = random8(0, h);
      hue = random8();
      deltaX = abs(aimX - posX);
      deltaY = abs(aimY - posY);
      signX = posX < aimX ? 1 : -1;
      signY = posY < aimY ? 1 : -1;
      error = deltaX - deltaY;
    };
  } bee_t;

  if (!SEGENV.allocateData(sizeof(bee_t)*MAX_BEES)) return mode_static(); //allocation failed
  bee_t *bee = reinterpret_cast<bee_t*>(SEGENV.data);

  if (SEGENV.call == 0) {
    SEGMENT.setUpLeds();
    SEGMENT.fill(BLACK);
    for (size_t i = 0; i < n; i++) {
      bee[i].posX = random8(0, cols);
      bee[i].posY = random8(0, rows);
      bee[i].aimed(cols, rows);
    }
  }

  if (millis() > SEGENV.step) {
    SEGENV.step = millis() + (FRAMETIME * 8 / ((SEGMENT.speed>>5)+1));

    SEGMENT.fadeToBlackBy(32);

    for (size_t i = 0; i < n; i++) {
      SEGMENT.addPixelColorXY(bee[i].aimX + 1, bee[i].aimY, CHSV(bee[i].hue, 255, 255));
      SEGMENT.addPixelColorXY(bee[i].aimX, bee[i].aimY + 1, CHSV(bee[i].hue, 255, 255));
      SEGMENT.addPixelColorXY(bee[i].aimX - 1, bee[i].aimY, CHSV(bee[i].hue, 255, 255));
      SEGMENT.addPixelColorXY(bee[i].aimX, bee[i].aimY - 1, CHSV(bee[i].hue, 255, 255));
      if (bee[i].posX != bee[i].aimX || bee[i].posY != bee[i].aimY) {
        SEGMENT.setPixelColorXY(bee[i].posX, bee[i].posY, CRGB(CHSV(bee[i].hue, 60, 255)));
        int8_t error2 = bee[i].error * 2;
        if (error2 > -bee[i].deltaY) {
          bee[i].error -= bee[i].deltaY;
          bee[i].posX += bee[i].signX;
        }
        if (error2 < bee[i].deltaX) {
          bee[i].error += bee[i].deltaX;
          bee[i].posY += bee[i].signY;
        }
      } else {
        bee[i].aimed(cols, rows);
      }
    }
    SEGMENT.blur(SEGMENT.intensity>>4);
  }
  return FRAMETIME;
}
static const char _data_FX_MODE_2DCRAZYBEES[] PROGMEM = "Crazy Bees@!,Blur;;;2";


/////////////////////////
//     2D Ghost Rider  //
/////////////////////////
//// Ghost Rider by stepko (c)2021 [https://editor.soulmatelights.com/gallery/716-ghost-rider], adapted by Blaz Kristan (AKA blazoncek)
#define LIGHTERS_AM 64  // max lighters (adequate for 32x32 matrix)
uint16_t mode_2Dghostrider(void) {
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  typedef struct Lighter {
    int16_t  gPosX;
    int16_t  gPosY;
    uint16_t gAngle;
    int8_t   angleSpeed;
    uint16_t lightersPosX[LIGHTERS_AM];
    uint16_t lightersPosY[LIGHTERS_AM];
    uint16_t Angle[LIGHTERS_AM];
    uint16_t time[LIGHTERS_AM];
    bool     reg[LIGHTERS_AM];
    int8_t   Vspeed;
  } lighter_t;

  if (!SEGENV.allocateData(sizeof(lighter_t))) return mode_static(); //allocation failed
  lighter_t *lighter = reinterpret_cast<lighter_t*>(SEGENV.data);

  const size_t maxLighters = min(cols + rows, LIGHTERS_AM);

  if (SEGENV.call == 0) SEGMENT.setUpLeds();
  if (SEGENV.aux0 != cols || SEGENV.aux1 != rows) {
    SEGENV.aux0 = cols;
    SEGENV.aux1 = rows;
    SEGMENT.fill(BLACK);
    random16_set_seed(strip.now);
    lighter->angleSpeed = random8(0,20) - 10;
    lighter->Vspeed = 5;
    lighter->gPosX = (cols/2) * 10;
    lighter->gPosY = (rows/2) * 10;
    for (size_t i = 0; i < maxLighters; i++) {
      lighter->lightersPosX[i] = lighter->gPosX;
      lighter->lightersPosY[i] = lighter->gPosY + i;
      lighter->time[i] = i * 2;
    }
  }

  if (millis() > SEGENV.step) {
    SEGENV.step = millis() + 1024 / (cols+rows);

    SEGMENT.fadeToBlackBy((SEGMENT.speed>>2)+64);

    CRGB color = CRGB::White;
    SEGMENT.wu_pixel(lighter->gPosX * 256 / 10, lighter->gPosY * 256 / 10, color);

    lighter->gPosX += lighter->Vspeed * sin_t(radians(lighter->gAngle));
    lighter->gPosY += lighter->Vspeed * cos_t(radians(lighter->gAngle));
    lighter->gAngle += lighter->angleSpeed;
    if (lighter->gPosX < 0)               lighter->gPosX = (cols - 1) * 10;
    if (lighter->gPosX > (cols - 1) * 10) lighter->gPosX = 0;
    if (lighter->gPosY < 0)               lighter->gPosY = (rows - 1) * 10;
    if (lighter->gPosY > (rows - 1) * 10) lighter->gPosY = 0;
    for (size_t i = 0; i < maxLighters; i++) {
      lighter->time[i] += random8(5, 20);
      if (lighter->time[i] >= 255 ||
        (lighter->lightersPosX[i] <= 0) ||
          (lighter->lightersPosX[i] >= (cols - 1) * 10) ||
          (lighter->lightersPosY[i] <= 0) ||
          (lighter->lightersPosY[i] >= (rows - 1) * 10)) {
        lighter->reg[i] = true;
      }
      if (lighter->reg[i]) {
        lighter->lightersPosY[i] = lighter->gPosY;
        lighter->lightersPosX[i] = lighter->gPosX;
        lighter->Angle[i] = lighter->gAngle + random(-10, 10);
        lighter->time[i] = 0;
        lighter->reg[i] = false;
      } else {
        lighter->lightersPosX[i] += -7 * sin_t(radians(lighter->Angle[i]));
        lighter->lightersPosY[i] += -7 * cos_t(radians(lighter->Angle[i]));
      }
      SEGMENT.wu_pixel(lighter->lightersPosX[i] * 256 / 10, lighter->lightersPosY[i] * 256 / 10, ColorFromPalette(SEGPALETTE, (256 - lighter->time[i])));
    }
    SEGMENT.blur(SEGMENT.intensity>>3);
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_2DGHOSTRIDER[] PROGMEM = "Ghost Rider@Fade rate,Blur;;!;2";


////////////////////////////
//     2D Floating Blobs  //
////////////////////////////
//// Floating Blobs by stepko (c)2021 [https://editor.soulmatelights.com/gallery/573-blobs], adapted by Blaz Kristan (AKA blazoncek)
#define MAX_BLOBS 8
uint16_t mode_2Dfloatingblobs(void) {
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  typedef struct Blob {
    float x[MAX_BLOBS], y[MAX_BLOBS];
    float sX[MAX_BLOBS], sY[MAX_BLOBS]; // speed
    float r[MAX_BLOBS];
    bool grow[MAX_BLOBS];
    byte color[MAX_BLOBS];
  } blob_t;

  uint8_t Amount = (SEGMENT.intensity>>5) + 1; // NOTE: be sure to update MAX_BLOBS if you change this

  if (!SEGENV.allocateData(sizeof(blob_t))) return mode_static(); //allocation failed
  blob_t *blob = reinterpret_cast<blob_t*>(SEGENV.data);

  if (SEGENV.call == 0) SEGMENT.setUpLeds();
  if (SEGENV.aux0 != cols || SEGENV.aux1 != rows) {
    SEGENV.aux0 = cols; // re-initialise if virtual size changes
    SEGENV.aux1 = rows;
    SEGMENT.fill(BLACK);
    for (size_t i = 0; i < MAX_BLOBS; i++) {
      blob->r[i]  = random8(1, cols>8 ? (cols/4) : 2);
      blob->sX[i] = (float) random8(3, cols) / (float)(256 - SEGMENT.speed); // speed x
      blob->sY[i] = (float) random8(3, rows) / (float)(256 - SEGMENT.speed); // speed y
      blob->x[i]  = random8(0, cols-1);
      blob->y[i]  = random8(0, rows-1);
      blob->color[i] = random8();
      blob->grow[i]  = (blob->r[i] < 1.f);
      if (blob->sX[i] == 0) blob->sX[i] = 1;
      if (blob->sY[i] == 0) blob->sY[i] = 1;
    }
  }

  SEGMENT.fadeToBlackBy(20);

  // Bounce balls around
  for (size_t i = 0; i < Amount; i++) {
    if (SEGENV.step < millis()) blob->color[i] = add8(blob->color[i], 4); // slowly change color
    // change radius if needed
    if (blob->grow[i]) {
      // enlarge radius until it is >= 4
      blob->r[i] += (fabsf(blob->sX[i]) > fabsf(blob->sY[i]) ? fabsf(blob->sX[i]) : fabsf(blob->sY[i])) * 0.05f;
      if (blob->r[i] >= MIN(cols/4.f,2.f)) {
        blob->grow[i] = false;
      }
    } else {
      // reduce radius until it is < 1
      blob->r[i] -= (fabsf(blob->sX[i]) > fabsf(blob->sY[i]) ? fabsf(blob->sX[i]) : fabsf(blob->sY[i])) * 0.05f;
      if (blob->r[i] < 1.f) {
        blob->grow[i] = true;
      }
    }
    uint32_t c = SEGMENT.color_from_palette(blob->color[i], false, false, 0);
    if (blob->r[i] > 1.f) SEGMENT.fill_circle(blob->x[i], blob->y[i], roundf(blob->r[i]), c);
    else                  SEGMENT.setPixelColorXY(blob->x[i], blob->y[i], c);
    // move x
    if (blob->x[i] + blob->r[i] >= cols - 1) blob->x[i] += (blob->sX[i] * ((cols - 1 - blob->x[i]) / blob->r[i] + 0.005f));
    else if (blob->x[i] - blob->r[i] <= 0)   blob->x[i] += (blob->sX[i] * (blob->x[i] / blob->r[i] + 0.005f));
    else                                     blob->x[i] += blob->sX[i];
    // move y
    if (blob->y[i] + blob->r[i] >= rows - 1) blob->y[i] += (blob->sY[i] * ((rows - 1 - blob->y[i]) / blob->r[i] + 0.005f));
    else if (blob->y[i] - blob->r[i] <= 0)   blob->y[i] += (blob->sY[i] * (blob->y[i] / blob->r[i] + 0.005f));
    else                                     blob->y[i] += blob->sY[i];
    // bounce x
    if (blob->x[i] < 0.01f) {
      blob->sX[i] = (float)random8(3, cols) / (256 - SEGMENT.speed);
      blob->x[i]  = 0.01f;
    } else if (blob->x[i] > (float)cols - 1.01f) {
      blob->sX[i] = (float)random8(3, cols) / (256 - SEGMENT.speed);
      blob->sX[i] = -blob->sX[i];
      blob->x[i]  = (float)cols - 1.01f;
    }
    // bounce y
    if (blob->y[i] < 0.01f) {
      blob->sY[i] = (float)random8(3, rows) / (256 - SEGMENT.speed);
      blob->y[i]  = 0.01f;
    } else if (blob->y[i] > (float)rows - 1.01f) {
      blob->sY[i] = (float)random8(3, rows) / (256 - SEGMENT.speed);
      blob->sY[i] = -blob->sY[i];
      blob->y[i]  = (float)rows - 1.01f;
    }
  }
  SEGMENT.blur(SEGMENT.custom1>>2);

  if (SEGENV.step < millis()) SEGENV.step = millis() + 2000; // change colors every 2 seconds

  return FRAMETIME;
}
#undef MAX_BLOBS
static const char _data_FX_MODE_2DBLOBS[] PROGMEM = "Blobs@!,# blobs,Blur;!;!;2;c1=8";


////////////////////////////
//     2D Scrolling text  //
////////////////////////////
uint16_t mode_2Dscrollingtext(void) {
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  int letterWidth;
  int letterHeight;
  switch (map(SEGMENT.custom2, 0, 255, 1, 5)) {
    default:
    case 1: letterWidth = 4; letterHeight =  6; break;
    case 2: letterWidth = 5; letterHeight =  8; break;
    case 3: letterWidth = 6; letterHeight =  8; break;
    case 4: letterWidth = 7; letterHeight =  9; break;
    case 5: letterWidth = 5; letterHeight = 12; break;
  }
  const int yoffset = map(SEGMENT.intensity, 0, 255, -rows/2, rows/2) + (rows-letterHeight)/2;
  char text[33] = {'\0'};
  if (SEGMENT.name) for (size_t i=0,j=0; i<strlen(SEGMENT.name); i++) if (SEGMENT.name[i]>31 && SEGMENT.name[i]<128) text[j++] = SEGMENT.name[i];

  if (!strlen(text) || !strncmp_P(text,PSTR("#DATE"),5) || !strncmp_P(text,PSTR("#DDMM"),5) || !strncmp_P(text,PSTR("#MMDD"),5) || !strncmp_P(text,PSTR("#TIME"),5) || !strncmp_P(text,PSTR("#HHMM"),5)) { // fallback if empty segment name: display date and time
    char sec[5];
    byte AmPmHour = hour(localTime);
    boolean isitAM = true;
    if (useAMPM) {
      if (AmPmHour > 11) { AmPmHour -= 12; isitAM = false; }
      if (AmPmHour == 0) { AmPmHour  = 12; }
    }
    if (useAMPM) sprintf_P(sec, PSTR(" %2s"), (isitAM ? "AM" : "PM"));
    else         sprintf_P(sec, PSTR(":%02d"), second(localTime));
    if      (!strncmp_P(text,PSTR("#DATE"),5)) sprintf_P(text, PSTR("%d.%d.%d"), day(localTime), month(localTime), year(localTime));
    else if (!strncmp_P(text,PSTR("#DDMM"),5)) sprintf_P(text, PSTR("%d.%d"), day(localTime), month(localTime));
    else if (!strncmp_P(text,PSTR("#MMDD"),5)) sprintf_P(text, PSTR("%d/%d"), month(localTime), day(localTime));
    else if (!strncmp_P(text,PSTR("#TIME"),5)) sprintf_P(text, PSTR("%2d:%02d%s"), AmPmHour, minute(localTime), sec);
    else if (!strncmp_P(text,PSTR("#HHMM"),5)) sprintf_P(text, PSTR("%2d:%02d"), AmPmHour, minute(localTime));
    else sprintf_P(text, PSTR("%s %d, %d %2d:%02d%s"), monthShortStr(month(localTime)), day(localTime), year(localTime), AmPmHour, minute(localTime), sec);
  }
  const int numberOfLetters = strlen(text);

  if (SEGENV.step < millis()) {
    if ((numberOfLetters * letterWidth) > cols) ++SEGENV.aux0 %= (numberOfLetters * letterWidth) + cols;      // offset
    else                                          SEGENV.aux0  = (cols + (numberOfLetters * letterWidth))/2;
    ++SEGENV.aux1 &= 0xFF; // color shift
    SEGENV.step = millis() + map(SEGMENT.speed, 0, 255, 10*FRAMETIME_FIXED, 2*FRAMETIME_FIXED);
    if (!SEGMENT.check2) {
      for (int y = 0; y < rows; y++) for (int x = 0; x < cols; x++ )
        SEGMENT.blendPixelColorXY(x, y, SEGCOLOR(1), 255 - (SEGMENT.custom1>>1));
    }
  }
  for (int i = 0; i < numberOfLetters; i++) {
    if (int(cols) - int(SEGENV.aux0) + letterWidth*(i+1) < 0) continue; // don't draw characters off-screen
    uint32_t col1 = SEGMENT.color_from_palette(SEGENV.aux1, false, PALETTE_SOLID_WRAP, 0);
    uint32_t col2 = BLACK;
    if (SEGMENT.check1 && SEGMENT.palette == 0) {
      col1 = SEGCOLOR(0);
      col2 = SEGCOLOR(2);
    }
    SEGMENT.drawCharacter(text[i], int(cols) - int(SEGENV.aux0) + letterWidth*i, yoffset, letterWidth, letterHeight, col1, col2);
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_2DSCROLLTEXT[] PROGMEM = "Scrolling Text@!,Y Offset,Trail,Font size,,Gradient,Overlay;!,!,Gradient;!;2;ix=128,c1=0,rev=0,mi=0,rY=0,mY=0";


////////////////////////////
//     2D Drift Rose      //
////////////////////////////
//// Drift Rose by stepko (c)2021 [https://editor.soulmatelights.com/gallery/1369-drift-rose-pattern], adapted by Blaz Kristan (AKA blazoncek)
uint16_t mode_2Ddriftrose(void) {
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  const float CX = (cols-cols%2)/2.f - .5f;
  const float CY = (rows-rows%2)/2.f - .5f;
  const float L = min(cols, rows) / 2.f;

  if (SEGENV.call == 0) {
    SEGMENT.setUpLeds();
    SEGMENT.fill(BLACK);
  }

  SEGMENT.fadeToBlackBy(32+(SEGMENT.speed>>3));
  for (size_t i = 1; i < 37; i++) {
    uint32_t x = (CX + (sin_t(radians(i * 10)) * (beatsin8(i, 0, L*2)-L))) * 255.f;
    uint32_t y = (CY + (cos_t(radians(i * 10)) * (beatsin8(i, 0, L*2)-L))) * 255.f;
    SEGMENT.wu_pixel(x, y, CHSV(i * 10, 255, 255));
  }
  SEGMENT.blur((SEGMENT.intensity>>4)+1);

  return FRAMETIME;
}
static const char _data_FX_MODE_2DDRIFTROSE[] PROGMEM = "Drift Rose@Fade,Blur;;;2";

#endif // WLED_DISABLE_2D


///////////////////////////////////////////////////////////////////////////////
/********************     audio enhanced routines     ************************/
///////////////////////////////////////////////////////////////////////////////


/* use the following code to pass AudioReactive usermod variables to effect

  uint8_t  *binNum = (uint8_t*)&SEGENV.aux1, *maxVol = (uint8_t*)(&SEGENV.aux1+1); // just in case assignment
  bool      samplePeak = false;
  float     FFT_MajorPeak = 1.0;
  uint8_t  *fftResult = nullptr;
  float    *fftBin = nullptr;
  um_data_t *um_data;
  if (usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    volumeSmth    = *(float*)   um_data->u_data[0];
    volumeRaw     = *(float*)   um_data->u_data[1];
    fftResult     =  (uint8_t*) um_data->u_data[2];
    samplePeak    = *(uint8_t*) um_data->u_data[3];
    FFT_MajorPeak = *(float*)   um_data->u_data[4];
    my_magnitude  = *(float*)   um_data->u_data[5];
    maxVol        =  (uint8_t*) um_data->u_data[6];  // requires UI element (SEGMENT.customX?), changes source element
    binNum        =  (uint8_t*) um_data->u_data[7];  // requires UI element (SEGMENT.customX?), changes source element
    fftBin        =  (float*)   um_data->u_data[8];
  } else {
    // add support for no audio data
    um_data = simulateSound(SEGMENT.soundSim);
  }
*/


// a few constants needed for AudioReactive effects

// for 22Khz sampling
#define MAX_FREQUENCY   11025    // sample frequency / 2 (as per Nyquist criterion)
#define MAX_FREQ_LOG10  4.04238f // log10(MAX_FREQUENCY)

// for 20Khz sampling
//#define MAX_FREQUENCY   10240
//#define MAX_FREQ_LOG10  4.0103f

// for 10Khz sampling
//#define MAX_FREQUENCY   5120
//#define MAX_FREQ_LOG10  3.71f


/////////////////////////////////
//     * Ripple Peak           //
/////////////////////////////////
uint16_t mode_ripplepeak(void) {                // * Ripple peak. By Andrew Tuline.
                                                          // This currently has no controls.
  #define maxsteps 16                                     // Case statement wouldn't allow a variable.

  uint16_t maxRipples = 16;
  uint16_t dataSize = sizeof(Ripple) * maxRipples;
  if (!SEGENV.allocateData(dataSize)) return mode_static(); //allocation failed
  Ripple* ripples = reinterpret_cast<Ripple*>(SEGENV.data);

  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  uint8_t samplePeak    = *(uint8_t*)um_data->u_data[3];
  #ifdef ESP32
  float   FFT_MajorPeak = *(float*)  um_data->u_data[4];
  #endif
  uint8_t *maxVol       =  (uint8_t*)um_data->u_data[6];
  uint8_t *binNum       =  (uint8_t*)um_data->u_data[7];

  // printUmData();

  if (SEGENV.call == 0) {
    SEGENV.aux0 = 255;
    SEGMENT.custom1 = *binNum;
    SEGMENT.custom2 = *maxVol * 2;
  }

  *binNum = SEGMENT.custom1;                              // Select a bin.
  *maxVol = SEGMENT.custom2 / 2;                          // Our volume comparator.

  SEGMENT.fade_out(240);                                  // Lower frame rate means less effective fading than FastLED
  SEGMENT.fade_out(240);

  for (int i = 0; i < SEGMENT.intensity/16; i++) {   // Limit the number of ripples.
    if (samplePeak) ripples[i].state = 255;

    switch (ripples[i].state) {
      case 254:     // Inactive mode
        break;

      case 255:                                           // Initialize ripple variables.
        ripples[i].pos = random16(SEGLEN);
        #ifdef ESP32
          if (FFT_MajorPeak > 1)                          // log10(0) is "forbidden" (throws exception)
          ripples[i].color = (int)(log10f(FFT_MajorPeak)*128);
          else ripples[i].color = 0;
        #else
          ripples[i].color = random8();
        #endif
        ripples[i].state = 0;
        break;

      case 0:
        SEGMENT.setPixelColor(ripples[i].pos, color_blend(SEGCOLOR(1), SEGMENT.color_from_palette(ripples[i].color, false, PALETTE_SOLID_WRAP, 0), SEGENV.aux0));
        ripples[i].state++;
        break;

      case maxsteps:                                      // At the end of the ripples. 254 is an inactive mode.
        ripples[i].state = 254;
        break;

      default:                                            // Middle of the ripples.
        SEGMENT.setPixelColor((ripples[i].pos + ripples[i].state + SEGLEN) % SEGLEN, color_blend(SEGCOLOR(1), SEGMENT.color_from_palette(ripples[i].color, false, PALETTE_SOLID_WRAP, 0), SEGENV.aux0/ripples[i].state*2));
        SEGMENT.setPixelColor((ripples[i].pos - ripples[i].state + SEGLEN) % SEGLEN, color_blend(SEGCOLOR(1), SEGMENT.color_from_palette(ripples[i].color, false, PALETTE_SOLID_WRAP, 0), SEGENV.aux0/ripples[i].state*2));
        ripples[i].state++;                               // Next step.
        break;
    } // switch step
  } // for i

  return FRAMETIME;
} // mode_ripplepeak()
static const char _data_FX_MODE_RIPPLEPEAK[] PROGMEM = "Ripple Peak@Fade rate,Max # of ripples,Select bin,Volume (min);!,!;!;1v;c2=0,m12=0,si=0"; // Pixel, Beatsin


#ifndef WLED_DISABLE_2D
/////////////////////////
//    * 2D Swirl       //
/////////////////////////
// By: Mark Kriegsman https://gist.github.com/kriegsman/5adca44e14ad025e6d3b , modified by Andrew Tuline
uint16_t mode_2DSwirl(void) {
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  if (SEGENV.call == 0) {
    SEGMENT.setUpLeds();
    SEGMENT.fill(BLACK);
  }

  const uint8_t borderWidth = 2;

  SEGMENT.blur(SEGMENT.custom1);

  uint8_t  i = beatsin8( 27*SEGMENT.speed/255, borderWidth, cols - borderWidth);
  uint8_t  j = beatsin8( 41*SEGMENT.speed/255, borderWidth, rows - borderWidth);
  uint8_t ni = (cols - 1) - i;
  uint8_t nj = (cols - 1) - j;
  uint16_t ms = millis();

  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  float   volumeSmth  = *(float*)   um_data->u_data[0]; //ewowi: use instead of sampleAvg???
  int16_t volumeRaw   = *(int16_t*) um_data->u_data[1];

  // printUmData();

  SEGMENT.addPixelColorXY( i, j, ColorFromPalette(SEGPALETTE, (ms / 11 + volumeSmth*4), volumeRaw * SEGMENT.intensity / 64, LINEARBLEND)); //CHSV( ms / 11, 200, 255);
  SEGMENT.addPixelColorXY( j, i, ColorFromPalette(SEGPALETTE, (ms / 13 + volumeSmth*4), volumeRaw * SEGMENT.intensity / 64, LINEARBLEND)); //CHSV( ms / 13, 200, 255);
  SEGMENT.addPixelColorXY(ni,nj, ColorFromPalette(SEGPALETTE, (ms / 17 + volumeSmth*4), volumeRaw * SEGMENT.intensity / 64, LINEARBLEND)); //CHSV( ms / 17, 200, 255);
  SEGMENT.addPixelColorXY(nj,ni, ColorFromPalette(SEGPALETTE, (ms / 29 + volumeSmth*4), volumeRaw * SEGMENT.intensity / 64, LINEARBLEND)); //CHSV( ms / 29, 200, 255);
  SEGMENT.addPixelColorXY( i,nj, ColorFromPalette(SEGPALETTE, (ms / 37 + volumeSmth*4), volumeRaw * SEGMENT.intensity / 64, LINEARBLEND)); //CHSV( ms / 37, 200, 255);
  SEGMENT.addPixelColorXY(ni, j, ColorFromPalette(SEGPALETTE, (ms / 41 + volumeSmth*4), volumeRaw * SEGMENT.intensity / 64, LINEARBLEND)); //CHSV( ms / 41, 200, 255);

  return FRAMETIME;
} // mode_2DSwirl()
static const char _data_FX_MODE_2DSWIRL[] PROGMEM = "Swirl@!,Sensitivity,Blur;,Bg Swirl;!;2v;ix=64,si=0"; // Beatsin // TODO: color 1 unused?


/////////////////////////
//    * 2D Waverly     //
/////////////////////////
// By: Stepko, https://editor.soulmatelights.com/gallery/652-wave , modified by Andrew Tuline
uint16_t mode_2DWaverly(void) {
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  if (SEGENV.call == 0) {
    SEGMENT.setUpLeds();
    SEGMENT.fill(BLACK);
  }

  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  float   volumeSmth  = *(float*)   um_data->u_data[0];

  SEGMENT.fadeToBlackBy(SEGMENT.speed);

  long t = millis() / 2;
  for (int i = 0; i < cols; i++) {
    uint16_t thisVal = (1 + SEGMENT.intensity/64) * inoise8(i * 45 , t , t)/2;
    // use audio if available
    if (um_data) {
      thisVal /= 32; // reduce intensity of inoise8()
      thisVal *= volumeSmth;
    }
    uint16_t thisMax = map(thisVal, 0, 512, 0, rows);

    for (int j = 0; j < thisMax; j++) {
      SEGMENT.addPixelColorXY(i, j, ColorFromPalette(SEGPALETTE, map(j, 0, thisMax, 250, 0), 255, LINEARBLEND));
      SEGMENT.addPixelColorXY((cols - 1) - i, (rows - 1) - j, ColorFromPalette(SEGPALETTE, map(j, 0, thisMax, 250, 0), 255, LINEARBLEND));
    }
  }
  SEGMENT.blur(16);

  return FRAMETIME;
} // mode_2DWaverly()
static const char _data_FX_MODE_2DWAVERLY[] PROGMEM = "Waverly@Amplification,Sensitivity;;!;2v;ix=64,si=0"; // Beatsin

#endif // WLED_DISABLE_2D

// float version of map()
static float mapf(float x, float in_min, float in_max, float out_min, float out_max){
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Gravity struct requited for GRAV* effects
typedef struct Gravity {
  int    topLED;
  int    gravityCounter;
} gravity;

///////////////////////
//   * GRAVCENTER    //
///////////////////////
uint16_t mode_gravcenter(void) {                // Gravcenter. By Andrew Tuline.

  const uint16_t dataSize = sizeof(gravity);
  if (!SEGENV.allocateData(dataSize)) return mode_static(); //allocation failed
  Gravity* gravcen = reinterpret_cast<Gravity*>(SEGENV.data);

  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  float   volumeSmth  = *(float*)  um_data->u_data[0];

  //SEGMENT.fade_out(240);
  SEGMENT.fade_out(251);  // 30%

  float segmentSampleAvg = volumeSmth * (float)SEGMENT.intensity / 255.0f;
  segmentSampleAvg *= 0.125; // divide by 8, to compensate for later "sensitivty" upscaling

  float mySampleAvg = mapf(segmentSampleAvg*2.0, 0, 32, 0, (float)SEGLEN/2.0); // map to pixels available in current segment
  uint16_t tempsamp = constrain(mySampleAvg, 0, SEGLEN/2);     // Keep the sample from overflowing.
  uint8_t gravity = 8 - SEGMENT.speed/32;

  for (int i=0; i<tempsamp; i++) {
    uint8_t index = inoise8(i*segmentSampleAvg+millis(), 5000+i*segmentSampleAvg);
    SEGMENT.setPixelColor(i+SEGLEN/2, color_blend(SEGCOLOR(1), SEGMENT.color_from_palette(index, false, PALETTE_SOLID_WRAP, 0), segmentSampleAvg*8));
    SEGMENT.setPixelColor(SEGLEN/2-i-1, color_blend(SEGCOLOR(1), SEGMENT.color_from_palette(index, false, PALETTE_SOLID_WRAP, 0), segmentSampleAvg*8));
  }

  if (tempsamp >= gravcen->topLED)
    gravcen->topLED = tempsamp-1;
  else if (gravcen->gravityCounter % gravity == 0)
    gravcen->topLED--;

  if (gravcen->topLED >= 0) {
    SEGMENT.setPixelColor(gravcen->topLED+SEGLEN/2, SEGMENT.color_from_palette(millis(), false, PALETTE_SOLID_WRAP, 0));
    SEGMENT.setPixelColor(SEGLEN/2-1-gravcen->topLED, SEGMENT.color_from_palette(millis(), false, PALETTE_SOLID_WRAP, 0));
  }
  gravcen->gravityCounter = (gravcen->gravityCounter + 1) % gravity;

  return FRAMETIME;
} // mode_gravcenter()
static const char _data_FX_MODE_GRAVCENTER[] PROGMEM = "Gravcenter@Rate of fall,Sensitivity;!,!;!;1v;ix=128,m12=2,si=0"; // Circle, Beatsin


///////////////////////
//   * GRAVCENTRIC   //
///////////////////////
uint16_t mode_gravcentric(void) {                     // Gravcentric. By Andrew Tuline.

  uint16_t dataSize = sizeof(gravity);
  if (!SEGENV.allocateData(dataSize)) return mode_static();     //allocation failed
  Gravity* gravcen = reinterpret_cast<Gravity*>(SEGENV.data);

  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  float   volumeSmth  = *(float*)  um_data->u_data[0];

  // printUmData();

  //SEGMENT.fade_out(240);
  //SEGMENT.fade_out(240); // twice? really?
  SEGMENT.fade_out(253);  // 50%

  float segmentSampleAvg = volumeSmth * (float)SEGMENT.intensity / 255.0;
  segmentSampleAvg *= 0.125f; // divide by 8, to compensate for later "sensitivty" upscaling

  float mySampleAvg = mapf(segmentSampleAvg*2.0, 0.0f, 32.0f, 0.0f, (float)SEGLEN/2.0); // map to pixels availeable in current segment
  int tempsamp = constrain(mySampleAvg, 0, SEGLEN/2);     // Keep the sample from overflowing.
  uint8_t gravity = 8 - SEGMENT.speed/32;

  for (int i=0; i<tempsamp; i++) {
    uint8_t index = segmentSampleAvg*24+millis()/200;
    SEGMENT.setPixelColor(i+SEGLEN/2, SEGMENT.color_from_palette(index, false, PALETTE_SOLID_WRAP, 0));
    SEGMENT.setPixelColor(SEGLEN/2-1-i, SEGMENT.color_from_palette(index, false, PALETTE_SOLID_WRAP, 0));
  }

  if (tempsamp >= gravcen->topLED)
    gravcen->topLED = tempsamp-1;
  else if (gravcen->gravityCounter % gravity == 0)
    gravcen->topLED--;

  if (gravcen->topLED >= 0) {
    SEGMENT.setPixelColor(gravcen->topLED+SEGLEN/2, CRGB::Gray);
    SEGMENT.setPixelColor(SEGLEN/2-1-gravcen->topLED, CRGB::Gray);
  }
  gravcen->gravityCounter = (gravcen->gravityCounter + 1) % gravity;

  return FRAMETIME;
} // mode_gravcentric()
static const char _data_FX_MODE_GRAVCENTRIC[] PROGMEM = "Gravcentric@Rate of fall,Sensitivity;!,!;!;1v;ix=128,m12=3,si=0"; // Corner, Beatsin


///////////////////////
//   * GRAVIMETER    //
///////////////////////
uint16_t mode_gravimeter(void) {                // Gravmeter. By Andrew Tuline.

  uint16_t dataSize = sizeof(gravity);
  if (!SEGENV.allocateData(dataSize)) return mode_static(); //allocation failed
  Gravity* gravcen = reinterpret_cast<Gravity*>(SEGENV.data);

  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  float   volumeSmth  = *(float*)  um_data->u_data[0];

  //SEGMENT.fade_out(240);
  SEGMENT.fade_out(249);  // 25%

  float segmentSampleAvg = volumeSmth * (float)SEGMENT.intensity / 255.0;
  segmentSampleAvg *= 0.25; // divide by 4, to compensate for later "sensitivty" upscaling

  float mySampleAvg = mapf(segmentSampleAvg*2.0, 0, 64, 0, (SEGLEN-1)); // map to pixels availeable in current segment
  int tempsamp = constrain(mySampleAvg,0,SEGLEN-1);       // Keep the sample from overflowing.
  uint8_t gravity = 8 - SEGMENT.speed/32;

  for (int i=0; i<tempsamp; i++) {
    uint8_t index = inoise8(i*segmentSampleAvg+millis(), 5000+i*segmentSampleAvg);
    SEGMENT.setPixelColor(i, color_blend(SEGCOLOR(1), SEGMENT.color_from_palette(index, false, PALETTE_SOLID_WRAP, 0), segmentSampleAvg*8));
  }

  if (tempsamp >= gravcen->topLED)
    gravcen->topLED = tempsamp;
  else if (gravcen->gravityCounter % gravity == 0)
    gravcen->topLED--;

  if (gravcen->topLED > 0) {
    SEGMENT.setPixelColor(gravcen->topLED, SEGMENT.color_from_palette(millis(), false, PALETTE_SOLID_WRAP, 0));
  }
  gravcen->gravityCounter = (gravcen->gravityCounter + 1) % gravity;

  return FRAMETIME;
} // mode_gravimeter()
static const char _data_FX_MODE_GRAVIMETER[] PROGMEM = "Gravimeter@Rate of fall,Sensitivity;!,!;!;1v;ix=128,m12=2,si=0"; // Circle, Beatsin


//////////////////////
//   * JUGGLES      //
//////////////////////
uint16_t mode_juggles(void) {                   // Juggles. By Andrew Tuline.
  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  float   volumeSmth   = *(float*)  um_data->u_data[0];

  SEGMENT.fade_out(224); // 6.25%
  uint16_t my_sampleAgc = fmax(fmin(volumeSmth, 255.0), 0);

  for (size_t i=0; i<SEGMENT.intensity/32+1U; i++) {
    SEGMENT.setPixelColor(beatsin16(SEGMENT.speed/4+i*2,0,SEGLEN-1), color_blend(SEGCOLOR(1), SEGMENT.color_from_palette(millis()/4+i*2, false, PALETTE_SOLID_WRAP, 0), my_sampleAgc));
  }

  return FRAMETIME;
} // mode_juggles()
static const char _data_FX_MODE_JUGGLES[] PROGMEM = "Juggles@!,# of balls;!,!;!;1v;m12=0,si=0"; // Pixels, Beatsin


//////////////////////
//   * MATRIPIX     //
//////////////////////
uint16_t mode_matripix(void) {                  // Matripix. By Andrew Tuline.
  // even with 1D effect we have to take logic for 2D segments for allocation as fill_solid() fills whole segment

  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  int16_t volumeRaw    = *(int16_t*)um_data->u_data[1];

  if (SEGENV.call == 0) {
    SEGMENT.setUpLeds();
    SEGMENT.fill(BLACK);
  }

  uint8_t secondHand = micros()/(256-SEGMENT.speed)/500 % 16;
  if(SEGENV.aux0 != secondHand) {
    SEGENV.aux0 = secondHand;

    int pixBri = volumeRaw * SEGMENT.intensity / 64;
    for (int i=0; i<SEGLEN-1; i++) SEGMENT.setPixelColor(i, SEGMENT.getPixelColor(i+1)); // shift left
    SEGMENT.setPixelColor(SEGLEN-1, color_blend(SEGCOLOR(1), SEGMENT.color_from_palette(millis(), false, PALETTE_SOLID_WRAP, 0), pixBri));
  }

  return FRAMETIME;
} // mode_matripix()
static const char _data_FX_MODE_MATRIPIX[] PROGMEM = "Matripix@!,Brightness;!,!;!;1v;ix=64,m12=2,si=1"; //,rev=1,mi=1,rY=1,mY=1 Circle, WeWillRockYou, reverseX


//////////////////////
//   * MIDNOISE     //
//////////////////////
uint16_t mode_midnoise(void) {                  // Midnoise. By Andrew Tuline.
// Changing xdist to SEGENV.aux0 and ydist to SEGENV.aux1.

  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  float   volumeSmth   = *(float*)  um_data->u_data[0];

  SEGMENT.fade_out(SEGMENT.speed);
  SEGMENT.fade_out(SEGMENT.speed);

  float tmpSound2 = volumeSmth * (float)SEGMENT.intensity / 256.0;  // Too sensitive.
  tmpSound2 *= (float)SEGMENT.intensity / 128.0;              // Reduce sensitity/length.

  int maxLen = mapf(tmpSound2, 0, 127, 0, SEGLEN/2);
  if (maxLen >SEGLEN/2) maxLen = SEGLEN/2;

  for (int i=(SEGLEN/2-maxLen); i<(SEGLEN/2+maxLen); i++) {
    uint8_t index = inoise8(i*volumeSmth+SEGENV.aux0, SEGENV.aux1+i*volumeSmth);  // Get a value from the noise function. I'm using both x and y axis.
    SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(index, false, PALETTE_SOLID_WRAP, 0));
  }

  SEGENV.aux0=SEGENV.aux0+beatsin8(5,0,10);
  SEGENV.aux1=SEGENV.aux1+beatsin8(4,0,10);

  return FRAMETIME;
} // mode_midnoise()
static const char _data_FX_MODE_MIDNOISE[] PROGMEM = "Midnoise@Fade rate,Max. length;!,!;!;1v;ix=128,m12=1,si=0"; // Bar, Beatsin


//////////////////////
//   * NOISEFIRE    //
//////////////////////
// I am the god of hellfire. . . Volume (only) reactive fire routine. Oh, look how short this is.
uint16_t mode_noisefire(void) {                 // Noisefire. By Andrew Tuline.
  CRGBPalette16 myPal = CRGBPalette16(CHSV(0,255,2),    CHSV(0,255,4),    CHSV(0,255,8), CHSV(0, 255, 8),  // Fire palette definition. Lower value = darker.
                                      CHSV(0, 255, 16), CRGB::Red,        CRGB::Red,     CRGB::Red,
                                      CRGB::DarkOrange, CRGB::DarkOrange, CRGB::Orange,  CRGB::Orange,
                                      CRGB::Yellow,     CRGB::Orange,     CRGB::Yellow,  CRGB::Yellow);

  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  float   volumeSmth   = *(float*)  um_data->u_data[0];

  if (SEGENV.call == 0) SEGMENT.fill(BLACK);

  for (int i = 0; i < SEGLEN; i++) {
    uint16_t index = inoise8(i*SEGMENT.speed/64,millis()*SEGMENT.speed/64*SEGLEN/255);  // X location is constant, but we move along the Y at the rate of millis(). By Andrew Tuline.
    index = (255 - i*256/SEGLEN) * index/(256-SEGMENT.intensity);                       // Now we need to scale index so that it gets blacker as we get close to one of the ends.
                                                                                        // This is a simple y=mx+b equation that's been scaled. index/128 is another scaling.

    CRGB color = ColorFromPalette(myPal, index, volumeSmth*2, LINEARBLEND);     // Use the my own palette.
    SEGMENT.setPixelColor(i, color);
  }

  return FRAMETIME;
} // mode_noisefire()
static const char _data_FX_MODE_NOISEFIRE[] PROGMEM = "Noisefire@!,!;;;1v;m12=2,si=0"; // Circle, Beatsin


///////////////////////
//   * Noisemeter    //
///////////////////////
uint16_t mode_noisemeter(void) {                // Noisemeter. By Andrew Tuline.

  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  float   volumeSmth   = *(float*)  um_data->u_data[0];
  int16_t volumeRaw    = *(int16_t*)um_data->u_data[1];

  //uint8_t fadeRate = map(SEGMENT.speed,0,255,224,255);
  uint8_t fadeRate = map(SEGMENT.speed,0,255,200,254);
  SEGMENT.fade_out(fadeRate);

  float tmpSound2 = volumeRaw * 2.0 * (float)SEGMENT.intensity / 255.0;
  int maxLen = mapf(tmpSound2, 0, 255, 0, SEGLEN); // map to pixels availeable in current segment              // Still a bit too sensitive.
  if (maxLen <0) maxLen = 0;
  if (maxLen >SEGLEN) maxLen = SEGLEN;

  for (int i=0; i<maxLen; i++) {                                    // The louder the sound, the wider the soundbar. By Andrew Tuline.
    uint8_t index = inoise8(i*volumeSmth+SEGENV.aux0, SEGENV.aux1+i*volumeSmth);  // Get a value from the noise function. I'm using both x and y axis.
    SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(index, false, PALETTE_SOLID_WRAP, 0));
  }

  SEGENV.aux0+=beatsin8(5,0,10);
  SEGENV.aux1+=beatsin8(4,0,10);

  return FRAMETIME;
} // mode_noisemeter()
static const char _data_FX_MODE_NOISEMETER[] PROGMEM = "Noisemeter@Fade rate,Width;!,!;!;1v;ix=128,m12=2,si=0"; // Circle, Beatsin


//////////////////////
//   * PIXELWAVE    //
//////////////////////
uint16_t mode_pixelwave(void) {                 // Pixelwave. By Andrew Tuline.
  // even with 1D effect we have to take logic for 2D segments for allocation as fill_solid() fills whole segment

  if (SEGENV.call == 0) {
    SEGMENT.setUpLeds();
    SEGMENT.fill(BLACK);
  }

  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  int16_t volumeRaw    = *(int16_t*)um_data->u_data[1];

  uint8_t secondHand = micros()/(256-SEGMENT.speed)/500+1 % 16;
  if (SEGENV.aux0 != secondHand) {
    SEGENV.aux0 = secondHand;

    int pixBri = volumeRaw * SEGMENT.intensity / 64;

    SEGMENT.setPixelColor(SEGLEN/2, color_blend(SEGCOLOR(1), SEGMENT.color_from_palette(millis(), false, PALETTE_SOLID_WRAP, 0), pixBri));
    for (int i = SEGLEN - 1; i > SEGLEN/2; i--)   SEGMENT.setPixelColor(i, SEGMENT.getPixelColor(i-1)); //move to the left
    for (int i = 0; i < SEGLEN/2; i++)            SEGMENT.setPixelColor(i, SEGMENT.getPixelColor(i+1)); // move to the right
  }

  return FRAMETIME;
} // mode_pixelwave()
static const char _data_FX_MODE_PIXELWAVE[] PROGMEM = "Pixelwave@!,Sensitivity;!,!;!;1v;ix=64,m12=2,si=0"; // Circle, Beatsin


//////////////////////
//   * PLASMOID     //
//////////////////////
typedef struct Plasphase {
  int16_t    thisphase;
  int16_t    thatphase;
} plasphase;

uint16_t mode_plasmoid(void) {                  // Plasmoid. By Andrew Tuline.
  // even with 1D effect we have to take logic for 2D segments for allocation as fill_solid() fills whole segment
  if (!SEGENV.allocateData(sizeof(plasphase))) return mode_static(); //allocation failed
  Plasphase* plasmoip = reinterpret_cast<Plasphase*>(SEGENV.data);

  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  float   volumeSmth   = *(float*)  um_data->u_data[0];

  SEGMENT.fadeToBlackBy(32);

  plasmoip->thisphase += beatsin8(6,-4,4);                          // You can change direction and speed individually.
  plasmoip->thatphase += beatsin8(7,-4,4);                          // Two phase values to make a complex pattern. By Andrew Tuline.

  for (int i=0; i<SEGLEN; i++) {                          // For each of the LED's in the strand, set a brightness based on a wave as follows.
    // updated, similar to "plasma" effect - softhack007
    uint8_t thisbright = cubicwave8(((i*(1 + (3*SEGMENT.speed/32)))+plasmoip->thisphase) & 0xFF)/2;
    thisbright += cos8(((i*(97 +(5*SEGMENT.speed/32)))+plasmoip->thatphase) & 0xFF)/2; // Let's munge the brightness a bit and animate it all with the phases.

    uint8_t colorIndex=thisbright;
    if (volumeSmth * SEGMENT.intensity / 64 < thisbright) {thisbright = 0;}

    SEGMENT.addPixelColor(i, color_blend(SEGCOLOR(1), SEGMENT.color_from_palette(colorIndex, false, PALETTE_SOLID_WRAP, 0), thisbright));
  }

  return FRAMETIME;
} // mode_plasmoid()
static const char _data_FX_MODE_PLASMOID[] PROGMEM = "Plasmoid@Phase,# of pixels;!,!;!;1v;sx=128,ix=128,m12=0,si=0"; // Pixels, Beatsin


///////////////////////
//   * PUDDLEPEAK    //
///////////////////////
// Andrew's crappy peak detector. If I were 40+ years younger, I'd learn signal processing.
uint16_t mode_puddlepeak(void) {                // Puddlepeak. By Andrew Tuline.

  uint16_t size = 0;
  uint8_t fadeVal = map(SEGMENT.speed,0,255, 224, 254);
  uint16_t pos = random(SEGLEN);                          // Set a random starting position.

  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  uint8_t samplePeak = *(uint8_t*)um_data->u_data[3];
  uint8_t *maxVol    =  (uint8_t*)um_data->u_data[6];
  uint8_t *binNum    =  (uint8_t*)um_data->u_data[7];
  float   volumeSmth   = *(float*)  um_data->u_data[0];

  if (SEGENV.call == 0) {
    SEGMENT.custom1 = *binNum;
    SEGMENT.custom2 = *maxVol * 2;
  }

  *binNum = SEGMENT.custom1;                              // Select a bin.
  *maxVol = SEGMENT.custom2 / 2;                          // Our volume comparator.

  SEGMENT.fade_out(fadeVal);

  if (samplePeak == 1) {
    size = volumeSmth * SEGMENT.intensity /256 /4 + 1;    // Determine size of the flash based on the volume.
    if (pos+size>= SEGLEN) size = SEGLEN - pos;
  }

  for (int i=0; i<size; i++) {                            // Flash the LED's.
    SEGMENT.setPixelColor(pos+i, SEGMENT.color_from_palette(millis(), false, PALETTE_SOLID_WRAP, 0));
  }

  return FRAMETIME;
} // mode_puddlepeak()
static const char _data_FX_MODE_PUDDLEPEAK[] PROGMEM = "Puddlepeak@Fade rate,Puddle size,Select bin,Volume (min);!,!;!;1v;c2=0,m12=0,si=0"; // Pixels, Beatsin


//////////////////////
//   * PUDDLES      //
//////////////////////
uint16_t mode_puddles(void) {                   // Puddles. By Andrew Tuline.
  uint16_t size = 0;
  uint8_t fadeVal = map(SEGMENT.speed, 0, 255, 224, 254);
  uint16_t pos = random16(SEGLEN);                        // Set a random starting position.

  SEGMENT.fade_out(fadeVal);

  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  int16_t volumeRaw    = *(int16_t*)um_data->u_data[1];

  if (volumeRaw > 1) {
    size = volumeRaw * SEGMENT.intensity /256 /8 + 1;        // Determine size of the flash based on the volume.
    if (pos+size >= SEGLEN) size = SEGLEN - pos;
  }

  for (int i=0; i<size; i++) {                          // Flash the LED's.
    SEGMENT.setPixelColor(pos+i, SEGMENT.color_from_palette(millis(), false, PALETTE_SOLID_WRAP, 0));
  }

  return FRAMETIME;
} // mode_puddles()
static const char _data_FX_MODE_PUDDLES[] PROGMEM = "Puddles@Fade rate,Puddle size;!,!;!;1v;m12=0,si=0"; // Pixels, Beatsin


//////////////////////
//     * PIXELS     //
//////////////////////
uint16_t mode_pixels(void) {                    // Pixels. By Andrew Tuline.

  if (!SEGENV.allocateData(32*sizeof(uint8_t))) return mode_static(); //allocation failed
  uint8_t *myVals = reinterpret_cast<uint8_t*>(SEGENV.data); // Used to store a pile of samples because WLED frame rate and WLED sample rate are not synchronized. Frame rate is too low.

  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    um_data = simulateSound(SEGMENT.soundSim);
  }
  float   volumeSmth   = *(float*)  um_data->u_data[0];

  myVals[millis()%32] = volumeSmth;    // filling values semi randomly

  SEGMENT.fade_out(64+(SEGMENT.speed>>1));

  for (int i=0; i <SEGMENT.intensity/8; i++) {
    uint16_t segLoc = random16(SEGLEN);                    // 16 bit for larger strands of LED's.
    SEGMENT.setPixelColor(segLoc, color_blend(SEGCOLOR(1), SEGMENT.color_from_palette(myVals[i%32]+i*4, false, PALETTE_SOLID_WRAP, 0), volumeSmth));
  }

  return FRAMETIME;
} // mode_pixels()
static const char _data_FX_MODE_PIXELS[] PROGMEM = "Pixels@Fade rate,# of pixels;!,!;!;1v;m12=0,si=0"; // Pixels, Beatsin


///////////////////////////////
//     BEGIN FFT ROUTINES    //
///////////////////////////////


//////////////////////
//    ** Blurz      //
//////////////////////
uint16_t mode_blurz(void) {                    // Blurz. By Andrew Tuline.
  // even with 1D effect we have to take logic for 2D segments for allocation as fill_solid() fills whole segment

  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  uint8_t *fftResult = (uint8_t*)um_data->u_data[2];

  if (SEGENV.call == 0) {
    SEGMENT.fill(BLACK);
    SEGENV.aux0 = 0;
  }

  int fadeoutDelay = (256 - SEGMENT.speed) / 32;
  if ((fadeoutDelay <= 1 ) || ((SEGENV.call % fadeoutDelay) == 0)) SEGMENT.fade_out(SEGMENT.speed);

  SEGENV.step += FRAMETIME;
  if (SEGENV.step > SPEED_FORMULA_L) {
    uint16_t segLoc = random16(SEGLEN);
    SEGMENT.setPixelColor(segLoc, color_blend(SEGCOLOR(1), SEGMENT.color_from_palette(2*fftResult[SEGENV.aux0%16]*240/(SEGLEN-1), false, PALETTE_SOLID_WRAP, 0), 2*fftResult[SEGENV.aux0%16]));
    ++(SEGENV.aux0) %= 16; // make sure it doesn't cross 16

    SEGENV.step = 1;
    SEGMENT.blur(SEGMENT.intensity);
  }

  return FRAMETIME;
} // mode_blurz()
static const char _data_FX_MODE_BLURZ[] PROGMEM = "Blurz@Fade rate,Blur;!,Color mix;!;1f;m12=0,si=0"; // Pixels, Beatsin


/////////////////////////
//   ** DJLight        //
/////////////////////////
uint16_t mode_DJLight(void) {                   // Written by ??? Adapted by Will Tatam.
  const int mid = SEGLEN / 2;

  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  uint8_t *fftResult = (uint8_t*)um_data->u_data[2];

  if (SEGENV.call == 0) {
    SEGMENT.setUpLeds();
    SEGMENT.fill(BLACK);
  }

  uint8_t secondHand = micros()/(256-SEGMENT.speed)/500+1 % 64;
  if (SEGENV.aux0 != secondHand) {                        // Triggered millis timing.
    SEGENV.aux0 = secondHand;

    CRGB color = CRGB(fftResult[15]/2, fftResult[5]/2, fftResult[0]/2); // 16-> 15 as 16 is out of bounds
    SEGMENT.setPixelColor(mid, color.fadeToBlackBy(map(fftResult[4], 0, 255, 255, 4)));     // TODO - Update

    for (int i = SEGLEN - 1; i > mid; i--)   SEGMENT.setPixelColor(i, SEGMENT.getPixelColor(i-1)); // move to the left
    for (int i = 0; i < mid; i++)            SEGMENT.setPixelColor(i, SEGMENT.getPixelColor(i+1)); // move to the right
  }

  return FRAMETIME;
} // mode_DJLight()
static const char _data_FX_MODE_DJLIGHT[] PROGMEM = "DJ Light@Speed;;;1f;m12=2,si=0"; // Circle, Beatsin


////////////////////
//   ** Freqmap   //
////////////////////
uint16_t mode_freqmap(void) {                   // Map FFT_MajorPeak to SEGLEN. Would be better if a higher framerate.
  // Start frequency = 60 Hz and log10(60) = 1.78
  // End frequency = MAX_FREQUENCY in Hz and lo10(MAX_FREQUENCY) = MAX_FREQ_LOG10

  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  float FFT_MajorPeak = *(float*)um_data->u_data[4];
  float my_magnitude  = *(float*)um_data->u_data[5] / 4.0f;
  if (FFT_MajorPeak < 1) FFT_MajorPeak = 1;                                         // log10(0) is "forbidden" (throws exception)

  if (SEGENV.call == 0) SEGMENT.fill(BLACK);
  int fadeoutDelay = (256 - SEGMENT.speed) / 32;
  if ((fadeoutDelay <= 1 ) || ((SEGENV.call % fadeoutDelay) == 0)) SEGMENT.fade_out(SEGMENT.speed);

  int locn = (log10f((float)FFT_MajorPeak) - 1.78f) * (float)SEGLEN/(MAX_FREQ_LOG10 - 1.78f);  // log10 frequency range is from 1.78 to 3.71. Let's scale to SEGLEN.
  if (locn < 1) locn = 0; // avoid underflow

  if (locn >=SEGLEN) locn = SEGLEN-1;
  uint16_t pixCol = (log10f(FFT_MajorPeak) - 1.78f) * 255.0f/(MAX_FREQ_LOG10 - 1.78f);   // Scale log10 of frequency values to the 255 colour index.
  if (FFT_MajorPeak < 61.0f) pixCol = 0;                                                 // handle underflow

  uint16_t bright = (int)my_magnitude;

  SEGMENT.setPixelColor(locn, color_blend(SEGCOLOR(1), SEGMENT.color_from_palette(SEGMENT.intensity+pixCol, false, PALETTE_SOLID_WRAP, 0), bright));

  return FRAMETIME;
} // mode_freqmap()
static const char _data_FX_MODE_FREQMAP[] PROGMEM = "Freqmap@Fade rate,Starting color;!,!;!;1f;m12=0,si=0"; // Pixels, Beatsin


///////////////////////
//   ** Freqmatrix   //
///////////////////////
uint16_t mode_freqmatrix(void) {                // Freqmatrix. By Andreas Pleschung.
  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  float FFT_MajorPeak = *(float*)um_data->u_data[4];
  float volumeSmth    = *(float*)um_data->u_data[0];

  if (SEGENV.call == 0) {
    SEGMENT.setUpLeds();
    SEGMENT.fill(BLACK);
  }

  uint8_t secondHand = micros()/(256-SEGMENT.speed)/500 % 16;
  if(SEGENV.aux0 != secondHand) {
    SEGENV.aux0 = secondHand;

    uint8_t sensitivity = map(SEGMENT.custom3, 0, 31, 1, 10); // reduced resolution slider
    int pixVal = (volumeSmth * SEGMENT.intensity * sensitivity) / 256.0f;
    if (pixVal > 255) pixVal = 255;

    float intensity = map(pixVal, 0, 255, 0, 100) / 100.0f;  // make a brightness from the last avg

    CRGB color = CRGB::Black;

    if (FFT_MajorPeak > MAX_FREQUENCY) FFT_MajorPeak = 1;
    // MajorPeak holds the freq. value which is most abundant in the last sample.
    // With our sampling rate of 10240Hz we have a usable freq range from roughtly 80Hz to 10240/2 Hz
    // we will treat everything with less than 65Hz as 0

    if (FFT_MajorPeak < 80) {
      color = CRGB::Black;
    } else {
      int upperLimit = 80 + 42 * SEGMENT.custom2;
      int lowerLimit = 80 + 3 * SEGMENT.custom1;
      uint8_t i =  lowerLimit!=upperLimit ? map(FFT_MajorPeak, lowerLimit, upperLimit, 0, 255) : FFT_MajorPeak;  // may under/overflow - so we enforce uint8_t
      uint16_t b = 255 * intensity;
      if (b > 255) b = 255;
      color = CHSV(i, 240, (uint8_t)b); // implicit conversion to RGB supplied by FastLED
    }

    // shift the pixels one pixel up
    SEGMENT.setPixelColor(0, color);
    for (int i = SEGLEN - 1; i > 0; i--) SEGMENT.setPixelColor(i, SEGMENT.getPixelColor(i-1)); //move to the left
  }

  return FRAMETIME;
} // mode_freqmatrix()
static const char _data_FX_MODE_FREQMATRIX[] PROGMEM = "Freqmatrix@Time delay,Sound effect,Low bin,High bin,Sensivity;;;1f;m12=3,si=0"; // Corner, Beatsin


//////////////////////
//   ** Freqpixels  //
//////////////////////
// Start frequency = 60 Hz and log10(60) = 1.78
// End frequency = 5120 Hz and lo10(5120) = 3.71
//  SEGMENT.speed select faderate
//  SEGMENT.intensity select colour index
uint16_t mode_freqpixels(void) {                // Freqpixel. By Andrew Tuline.
  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  float FFT_MajorPeak = *(float*)um_data->u_data[4];
  float my_magnitude  = *(float*)um_data->u_data[5] / 16.0f;
  if (FFT_MajorPeak < 1) FFT_MajorPeak = 1;                                         // log10(0) is "forbidden" (throws exception)

  uint16_t fadeRate = 2*SEGMENT.speed - SEGMENT.speed*SEGMENT.speed/255;    // Get to 255 as quick as you can.

  if (SEGENV.call == 0) SEGMENT.fill(BLACK);
  int fadeoutDelay = (256 - SEGMENT.speed) / 64;
  if ((fadeoutDelay <= 1 ) || ((SEGENV.call % fadeoutDelay) == 0)) SEGMENT.fade_out(fadeRate);

  for (int i=0; i < SEGMENT.intensity/32+1; i++) {
    uint16_t locn = random16(0,SEGLEN);
    uint8_t pixCol = (log10f(FFT_MajorPeak) - 1.78f) * 255.0f/(MAX_FREQ_LOG10 - 1.78f);  // Scale log10 of frequency values to the 255 colour index.
    if (FFT_MajorPeak < 61.0f) pixCol = 0;                                               // handle underflow
    SEGMENT.setPixelColor(locn, color_blend(SEGCOLOR(1), SEGMENT.color_from_palette(SEGMENT.intensity+pixCol, false, PALETTE_SOLID_WRAP, 0), (int)my_magnitude));
  }

  return FRAMETIME;
} // mode_freqpixels()
static const char _data_FX_MODE_FREQPIXELS[] PROGMEM = "Freqpixels@Fade rate,Starting color and # of pixels;;;1f;m12=0,si=0"; // Pixels, Beatsin


//////////////////////
//   ** Freqwave    //
//////////////////////
// Assign a color to the central (starting pixels) based on the predominant frequencies and the volume. The color is being determined by mapping the MajorPeak from the FFT
// and then mapping this to the HSV color circle. Currently we are sampling at 10240 Hz, so the highest frequency we can look at is 5120Hz.
//
// SEGMENT.custom1: the lower cut off point for the FFT. (many, most time the lowest values have very little information since they are FFT conversion artifacts. Suggested value is close to but above 0
// SEGMENT.custom2: The high cut off point. This depends on your sound profile. Most music looks good when this slider is between 50% and 100%.
// SEGMENT.custom3: "preamp" for the audio signal for audio10.
//
// I suggest that for this effect you turn the brightness to 95%-100% but again it depends on your soundprofile you find yourself in.
// Instead of using colorpalettes, This effect works on the HSV color circle with red being the lowest frequency
//
// As a compromise between speed and accuracy we are currently sampling with 10240Hz, from which we can then determine with a 512bin FFT our max frequency is 5120Hz.
// Depending on the music stream you have you might find it useful to change the frequency mapping.
uint16_t mode_freqwave(void) {                  // Freqwave. By Andreas Pleschung.
  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  float FFT_MajorPeak = *(float*)um_data->u_data[4];
  float volumeSmth    = *(float*)um_data->u_data[0];

  if (SEGENV.call == 0) {
    SEGMENT.setUpLeds();
    SEGMENT.fill(BLACK);
  }

  uint8_t secondHand = micros()/(256-SEGMENT.speed)/500 % 16;
  if(SEGENV.aux0 != secondHand) {
    SEGENV.aux0 = secondHand;

    float sensitivity = mapf(SEGMENT.custom3, 1, 31, 1, 10); // reduced resolution slider
    float pixVal = volumeSmth * (float)SEGMENT.intensity / 256.0f * sensitivity;
    if (pixVal > 255) pixVal = 255;

    float intensity = mapf(pixVal, 0, 255, 0, 100) / 100.0f;  // make a brightness from the last avg

    CRGB color = 0;

    if (FFT_MajorPeak > MAX_FREQUENCY) FFT_MajorPeak = 1.0f;
    // MajorPeak holds the freq. value which is most abundant in the last sample.
    // With our sampling rate of 10240Hz we have a usable freq range from roughtly 80Hz to 10240/2 Hz
    // we will treat everything with less than 65Hz as 0

    if (FFT_MajorPeak < 80) {
      color = CRGB::Black;
    } else {
      int upperLimit = 80 + 42 * SEGMENT.custom2;
      int lowerLimit = 80 + 3 * SEGMENT.custom1;
      uint8_t i =  lowerLimit!=upperLimit ? map(FFT_MajorPeak, lowerLimit, upperLimit, 0, 255) : FFT_MajorPeak; // may under/overflow - so we enforce uint8_t
      uint16_t b = 255.0 * intensity;
      if (b > 255) b=255;
      color = CHSV(i, 240, (uint8_t)b); // implicit conversion to RGB supplied by FastLED
    }

    SEGMENT.setPixelColor(SEGLEN/2, color);

    // shift the pixels one pixel outwards
    for (int i = SEGLEN - 1; i > SEGLEN/2; i--)   SEGMENT.setPixelColor(i, SEGMENT.getPixelColor(i-1)); //move to the left
    for (int i = 0; i < SEGLEN/2; i++)            SEGMENT.setPixelColor(i, SEGMENT.getPixelColor(i+1)); // move to the right
  }

  return FRAMETIME;
} // mode_freqwave()
static const char _data_FX_MODE_FREQWAVE[] PROGMEM = "Freqwave@Time delay,Sound effect,Low bin,High bin,Pre-amp;;;1f;m12=2,si=0"; // Circle, Beatsin


///////////////////////
//    ** Gravfreq    //
///////////////////////
uint16_t mode_gravfreq(void) {                  // Gravfreq. By Andrew Tuline.

  uint16_t dataSize = sizeof(gravity);
  if (!SEGENV.allocateData(dataSize)) return mode_static(); //allocation failed
  Gravity* gravcen = reinterpret_cast<Gravity*>(SEGENV.data);

  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  float   FFT_MajorPeak = *(float*)um_data->u_data[4];
  float   volumeSmth    = *(float*)um_data->u_data[0];
  if (FFT_MajorPeak < 1) FFT_MajorPeak = 1;                                         // log10(0) is "forbidden" (throws exception)

  SEGMENT.fade_out(250);

  float segmentSampleAvg = volumeSmth * (float)SEGMENT.intensity / 255.0f;
  segmentSampleAvg *= 0.125; // divide by 8,  to compensate for later "sensitivty" upscaling

  float mySampleAvg = mapf(segmentSampleAvg*2.0f, 0,32, 0, (float)SEGLEN/2.0); // map to pixels availeable in current segment
  int tempsamp = constrain(mySampleAvg,0,SEGLEN/2);     // Keep the sample from overflowing.
  uint8_t gravity = 8 - SEGMENT.speed/32;

  for (int i=0; i<tempsamp; i++) {

    //uint8_t index = (log10((int)FFT_MajorPeak) - (3.71-1.78)) * 255; //int? shouldn't it be floor() or similar
    uint8_t index = (log10f(FFT_MajorPeak) - (MAX_FREQ_LOG10 - 1.78f)) * 255; //int? shouldn't it be floor() or similar

    SEGMENT.setPixelColor(i+SEGLEN/2, SEGMENT.color_from_palette(index, false, PALETTE_SOLID_WRAP, 0));
    SEGMENT.setPixelColor(SEGLEN/2-i-1, SEGMENT.color_from_palette(index, false, PALETTE_SOLID_WRAP, 0));
  }

  if (tempsamp >= gravcen->topLED)
    gravcen->topLED = tempsamp-1;
  else if (gravcen->gravityCounter % gravity == 0)
    gravcen->topLED--;

  if (gravcen->topLED >= 0) {
    SEGMENT.setPixelColor(gravcen->topLED+SEGLEN/2, CRGB::Gray);
    SEGMENT.setPixelColor(SEGLEN/2-1-gravcen->topLED, CRGB::Gray);
  }
  gravcen->gravityCounter = (gravcen->gravityCounter + 1) % gravity;

  return FRAMETIME;
} // mode_gravfreq()
static const char _data_FX_MODE_GRAVFREQ[] PROGMEM = "Gravfreq@Rate of fall,Sensivity;!,!;!;1f;ix=128,m12=0,si=0"; // Pixels, Beatsin


//////////////////////
//   ** Noisemove   //
//////////////////////
uint16_t mode_noisemove(void) {                 // Noisemove.    By: Andrew Tuline
  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  uint8_t *fftResult = (uint8_t*)um_data->u_data[2];

  if (SEGENV.call == 0) SEGMENT.fill(BLACK);
  //SEGMENT.fade_out(224);                                          // Just in case something doesn't get faded.
  int fadeoutDelay = (256 - SEGMENT.speed) / 96;
  if ((fadeoutDelay <= 1 ) || ((SEGENV.call % fadeoutDelay) == 0)) SEGMENT.fadeToBlackBy(4+ SEGMENT.speed/4);

  uint8_t numBins = map(SEGMENT.intensity,0,255,0,16);    // Map slider to fftResult bins.
  for (int i=0; i<numBins; i++) {                         // How many active bins are we using.
    uint16_t locn = inoise16(millis()*SEGMENT.speed+i*50000, millis()*SEGMENT.speed);   // Get a new pixel location from moving noise.
    locn = map(locn, 7500, 58000, 0, SEGLEN-1);           // Map that to the length of the strand, and ensure we don't go over.
    SEGMENT.setPixelColor(locn, color_blend(SEGCOLOR(1), SEGMENT.color_from_palette(i*64, false, PALETTE_SOLID_WRAP, 0), fftResult[i % 16]*4));
  }

  return FRAMETIME;
} // mode_noisemove()
static const char _data_FX_MODE_NOISEMOVE[] PROGMEM = "Noisemove@Speed of perlin movement,Fade rate;!,!;!;1f;m12=0,si=0"; // Pixels, Beatsin


//////////////////////
//   ** Rocktaves   //
//////////////////////
uint16_t mode_rocktaves(void) {                 // Rocktaves. Same note from each octave is same colour.    By: Andrew Tuline
  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  float   FFT_MajorPeak = *(float*)  um_data->u_data[4];
  float   my_magnitude  = *(float*)   um_data->u_data[5] / 16.0f;

  if (SEGENV.call == 0) SEGMENT.fill(BLACK);
  SEGMENT.fadeToBlackBy(16);                        // Just in case something doesn't get faded.

  float frTemp = FFT_MajorPeak;
  uint8_t octCount = 0;                                   // Octave counter.
  uint8_t volTemp = 0;

  volTemp = 32.0f + my_magnitude * 1.5f;                  // brightness = volume (overflows are handled in next lines)
  if (my_magnitude < 48) volTemp = 0;                     // We need to squelch out the background noise.
  if (my_magnitude > 144) volTemp = 255;                  // everything above this is full brightness

  while ( frTemp > 249 ) {
    octCount++;                                           // This should go up to 5.
    frTemp = frTemp/2;
  }

  frTemp -=132;                                           // This should give us a base musical note of C3
  frTemp = fabsf(frTemp * 2.1f);                            // Fudge factors to compress octave range starting at 0 and going to 255;

  uint16_t i = map(beatsin8(8+octCount*4, 0, 255, 0, octCount*8), 0, 255, 0, SEGLEN-1);
  i = constrain(i, 0, SEGLEN-1);
  SEGMENT.addPixelColor(i, color_blend(SEGCOLOR(1), SEGMENT.color_from_palette((uint8_t)frTemp, false, PALETTE_SOLID_WRAP, 0), volTemp));

  return FRAMETIME;
} // mode_rocktaves()
static const char _data_FX_MODE_ROCKTAVES[] PROGMEM = "Rocktaves@;!,!;!;1f;m12=1,si=0"; // Bar, Beatsin


///////////////////////
//   ** Waterfall    //
///////////////////////
// Combines peak detection with FFT_MajorPeak and FFT_Magnitude.
uint16_t mode_waterfall(void) {                   // Waterfall. By: Andrew Tuline
  if (SEGENV.call == 0) SEGMENT.fill(BLACK);

  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  uint8_t samplePeak    = *(uint8_t*)um_data->u_data[3];
  float   FFT_MajorPeak = *(float*)  um_data->u_data[4];
  uint8_t *maxVol       =  (uint8_t*)um_data->u_data[6];
  uint8_t *binNum       =  (uint8_t*)um_data->u_data[7];
  float   my_magnitude  = *(float*)   um_data->u_data[5] / 8.0f;

  if (FFT_MajorPeak < 1) FFT_MajorPeak = 1;                                         // log10(0) is "forbidden" (throws exception)

  if (SEGENV.call == 0) {
    SEGMENT.setUpLeds();
    SEGMENT.fill(BLACK);
    SEGENV.aux0 = 255;
    SEGMENT.custom1 = *binNum;
    SEGMENT.custom2 = *maxVol * 2;
  }

  *binNum = SEGMENT.custom1;                              // Select a bin.
  *maxVol = SEGMENT.custom2 / 2;                          // Our volume comparator.

  uint8_t secondHand = micros() / (256-SEGMENT.speed)/500 + 1 % 16;
  if (SEGENV.aux0 != secondHand) {                        // Triggered millis timing.
    SEGENV.aux0 = secondHand;

    //uint8_t pixCol = (log10f((float)FFT_MajorPeak) - 2.26f) * 177;  // 10Khz sampling - log10 frequency range is from 2.26 (182hz) to 3.7 (5012hz). Let's scale accordingly.
    uint8_t pixCol = (log10f(FFT_MajorPeak) - 2.26f) * 150;           // 22Khz sampling - log10 frequency range is from 2.26 (182hz) to 3.967 (9260hz). Let's scale accordingly.
    if (FFT_MajorPeak < 182.0f) pixCol = 0;                           // handle underflow

    if (samplePeak) {
      SEGMENT.setPixelColor(SEGLEN-1, CHSV(92,92,92));
    } else {
      SEGMENT.setPixelColor(SEGLEN-1, color_blend(SEGCOLOR(1), SEGMENT.color_from_palette(pixCol+SEGMENT.intensity, false, PALETTE_SOLID_WRAP, 0), (int)my_magnitude));
    }
    for (int i=0; i<SEGLEN-1; i++) SEGMENT.setPixelColor(i, SEGMENT.getPixelColor(i+1)); // shift left
  }

  return FRAMETIME;
} // mode_waterfall()
static const char _data_FX_MODE_WATERFALL[] PROGMEM = "Waterfall@!,Adjust color,Select bin,Volume (min);!,!;!;1f;c2=0,m12=2,si=0"; // Circles, Beatsin


#ifndef WLED_DISABLE_2D
/////////////////////////
//     ** 2D GEQ       //
/////////////////////////
uint16_t mode_2DGEQ(void) { // By Will Tatam. Code reduction by Ewoud Wijma.
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const int NUM_BANDS = map(SEGMENT.custom1, 0, 255, 1, 16);
  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  if (!SEGENV.allocateData(cols*sizeof(uint16_t))) return mode_static(); //allocation failed
  uint16_t *previousBarHeight = reinterpret_cast<uint16_t*>(SEGENV.data); //array of previous bar heights per frequency band

  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  uint8_t *fftResult = (uint8_t*)um_data->u_data[2];

  if (SEGENV.call == 0) for (int i=0; i<cols; i++) previousBarHeight[i] = 0;

  bool rippleTime = false;
  if (millis() - SEGENV.step >= (256U - SEGMENT.intensity)) {
    SEGENV.step = millis();
    rippleTime = true;
  }

  if (SEGENV.call == 0) SEGMENT.fill(BLACK);
  int fadeoutDelay = (256 - SEGMENT.speed) / 64;
  if ((fadeoutDelay <= 1 ) || ((SEGENV.call % fadeoutDelay) == 0)) SEGMENT.fadeToBlackBy(SEGMENT.speed);

  for (int x=0; x < cols; x++) {
    uint8_t  band       = map(x, 0, cols-1, 0, NUM_BANDS - 1);
    if (NUM_BANDS < 16) band = map(band, 0, NUM_BANDS - 1, 0, 15); // always use full range. comment out this line to get the previous behaviour.
    band = constrain(band, 0, 15);
    uint16_t colorIndex = band * 17;
    uint16_t barHeight  = map(fftResult[band], 0, 255, 0, rows); // do not subtract -1 from rows here
    if (barHeight > previousBarHeight[x]) previousBarHeight[x] = barHeight; //drive the peak up

    uint32_t ledColor = BLACK;
    for (int y=0; y < barHeight; y++) {
      if (SEGMENT.check1) //color_vertical / color bars toggle
        colorIndex = map(y, 0, rows-1, 0, 255);

      ledColor = SEGMENT.color_from_palette(colorIndex, false, PALETTE_SOLID_WRAP, 0);
      SEGMENT.setPixelColorXY(x, rows-1 - y, ledColor);
    }
    if (previousBarHeight[x] > 0)
      SEGMENT.setPixelColorXY(x, rows - previousBarHeight[x], (SEGCOLOR(2) != BLACK) ? SEGCOLOR(2) : ledColor);

    if (rippleTime && previousBarHeight[x]>0) previousBarHeight[x]--;    //delay/ripple effect
  }

  return FRAMETIME;
} // mode_2DGEQ()
static const char _data_FX_MODE_2DGEQ[] PROGMEM = "GEQ@Fade speed,Ripple decay,# of bands,,,Color bars;!,,Peaks;!;2f;c1=255,c2=64,pal=11,si=0"; // Beatsin


/////////////////////////
//  ** 2D Funky plank  //
/////////////////////////
uint16_t mode_2DFunkyPlank(void) {              // Written by ??? Adapted by Will Tatam.
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  int NUMB_BANDS = map(SEGMENT.custom1, 0, 255, 1, 16);
  int barWidth = (cols / NUMB_BANDS);
  int bandInc = 1;
  if (barWidth == 0) {
    // Matrix narrower than fft bands
    barWidth = 1;
    bandInc = (NUMB_BANDS / cols);
  }

  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  uint8_t *fftResult = (uint8_t*)um_data->u_data[2];

  if (SEGENV.call == 0) {
    SEGMENT.setUpLeds();
    SEGMENT.fill(BLACK);
  }

  uint8_t secondHand = micros()/(256-SEGMENT.speed)/500+1 % 64;
  if (SEGENV.aux0 != secondHand) {                        // Triggered millis timing.
    SEGENV.aux0 = secondHand;

    // display values of
    int b = 0;
    for (int band = 0; band < NUMB_BANDS; band += bandInc, b++) {
      int hue = fftResult[band % 16];
      int v = map(fftResult[band % 16], 0, 255, 10, 255);
      for (int w = 0; w < barWidth; w++) {
         int xpos = (barWidth * b) + w;
         SEGMENT.setPixelColorXY(xpos, 0, CHSV(hue, 255, v));
      }
    }

    // Update the display:
    for (int i = (rows - 1); i > 0; i--) {
      for (int j = (cols - 1); j >= 0; j--) {
        SEGMENT.setPixelColorXY(j, i, SEGMENT.getPixelColorXY(j, i-1));
      }
    }
  }

  return FRAMETIME;
} // mode_2DFunkyPlank
static const char _data_FX_MODE_2DFUNKYPLANK[] PROGMEM = "Funky Plank@Scroll speed,,# of bands;;;2f;si=0"; // Beatsin


/////////////////////////
//     2D Akemi        //
/////////////////////////
static uint8_t akemi[] PROGMEM = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,2,2,3,3,3,3,3,3,2,2,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,2,3,3,0,0,0,0,0,0,3,3,2,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,2,3,0,0,0,6,5,5,4,0,0,0,3,2,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,2,3,0,0,6,6,5,5,5,5,4,4,0,0,3,2,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,2,3,0,6,5,5,5,5,5,5,5,5,4,0,3,2,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,2,3,0,6,5,5,5,5,5,5,5,5,5,5,4,0,3,2,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,3,2,0,6,5,5,5,5,5,5,5,5,5,5,4,0,2,3,0,0,0,0,0,0,0,
  0,0,0,0,0,0,3,2,3,6,5,5,7,7,5,5,5,5,7,7,5,5,4,3,2,3,0,0,0,0,0,0,
  0,0,0,0,0,2,3,1,3,6,5,1,7,7,7,5,5,1,7,7,7,5,4,3,1,3,2,0,0,0,0,0,
  0,0,0,0,0,8,3,1,3,6,5,1,7,7,7,5,5,1,7,7,7,5,4,3,1,3,8,0,0,0,0,0,
  0,0,0,0,0,8,3,1,3,6,5,5,1,1,5,5,5,5,1,1,5,5,4,3,1,3,8,0,0,0,0,0,
  0,0,0,0,0,2,3,1,3,6,5,5,5,5,5,5,5,5,5,5,5,5,4,3,1,3,2,0,0,0,0,0,
  0,0,0,0,0,0,3,2,3,6,5,5,5,5,5,5,5,5,5,5,5,5,4,3,2,3,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,6,5,5,5,5,5,7,7,5,5,5,5,5,4,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,6,5,5,5,5,5,5,5,5,5,5,5,5,4,0,0,0,0,0,0,0,0,0,
  1,0,0,0,0,0,0,0,0,6,5,5,5,5,5,5,5,5,5,5,5,5,4,0,0,0,0,0,0,0,0,2,
  0,2,2,2,0,0,0,0,0,6,5,5,5,5,5,5,5,5,5,5,5,5,4,0,0,0,0,0,2,2,2,0,
  0,0,0,3,2,0,0,0,6,5,4,4,4,4,4,4,4,4,4,4,4,4,4,4,0,0,0,2,2,0,0,0,
  0,0,0,3,2,0,0,0,6,5,5,5,5,5,5,5,5,5,5,5,5,5,5,4,0,0,0,2,3,0,0,0,
  0,0,0,0,3,2,0,0,0,0,3,3,0,3,3,0,0,3,3,0,3,3,0,0,0,0,2,2,0,0,0,0,
  0,0,0,0,3,2,0,0,0,0,3,2,0,3,2,0,0,3,2,0,3,2,0,0,0,0,2,3,0,0,0,0,
  0,0,0,0,0,3,2,0,0,3,2,0,0,3,2,0,0,3,2,0,0,3,2,0,0,2,3,0,0,0,0,0,
  0,0,0,0,0,3,2,2,2,2,0,0,0,3,2,0,0,3,2,0,0,0,3,2,2,2,3,0,0,0,0,0,
  0,0,0,0,0,0,3,3,3,0,0,0,0,3,2,0,0,3,2,0,0,0,0,3,3,3,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,0,0,3,2,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,0,0,3,2,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,0,0,3,2,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,0,0,3,2,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

uint16_t mode_2DAkemi(void) {
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  uint16_t counter = (strip.now * ((SEGMENT.speed >> 2) +2)) & 0xFFFF;
  counter = counter >> 8;

  const float lightFactor  = 0.15f;
  const float normalFactor = 0.4f;

  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    um_data = simulateSound(SEGMENT.soundSim);
  }
  uint8_t *fftResult = (uint8_t*)um_data->u_data[2];
  float base = fftResult[0]/255.0f;

  //draw and color Akemi
  for (int y=0; y < rows; y++) for (int x=0; x < cols; x++) {
    CRGB color;
    CRGB soundColor = ORANGE;
    CRGB faceColor  = SEGMENT.color_wheel(counter);
    CRGB armsAndLegsColor = SEGCOLOR(1) > 0 ? SEGCOLOR(1) : 0xFFE0A0; //default warmish white 0xABA8FF; //0xFF52e5;//
    uint8_t ak = pgm_read_byte_near(akemi + ((y * 32)/rows) * 32 + (x * 32)/cols); // akemi[(y * 32)/rows][(x * 32)/cols]
    switch (ak) {
      case 3: armsAndLegsColor.r *= lightFactor;  armsAndLegsColor.g *= lightFactor;  armsAndLegsColor.b *= lightFactor;  color = armsAndLegsColor; break; //light arms and legs 0x9B9B9B
      case 2: armsAndLegsColor.r *= normalFactor; armsAndLegsColor.g *= normalFactor; armsAndLegsColor.b *= normalFactor; color = armsAndLegsColor; break; //normal arms and legs 0x888888
      case 1: color = armsAndLegsColor; break; //dark arms and legs 0x686868
      case 6: faceColor.r *= lightFactor;  faceColor.g *= lightFactor;  faceColor.b *= lightFactor;  color=faceColor; break; //light face 0x31AAFF
      case 5: faceColor.r *= normalFactor; faceColor.g *= normalFactor; faceColor.b *= normalFactor; color=faceColor; break; //normal face 0x0094FF
      case 4: color = faceColor; break; //dark face 0x007DC6
      case 7: color = SEGCOLOR(2) > 0 ? SEGCOLOR(2) : 0xFFFFFF; break; //eyes and mouth default white
      case 8: if (base > 0.4) {soundColor.r *= base; soundColor.g *= base; soundColor.b *= base; color=soundColor;} else color = armsAndLegsColor; break;
      default: color = BLACK; break;
    }

    if (SEGMENT.intensity > 128 && fftResult && fftResult[0] > 128) { //dance if base is high
      SEGMENT.setPixelColorXY(x, 0, BLACK);
      SEGMENT.setPixelColorXY(x, y+1, color);
    } else
      SEGMENT.setPixelColorXY(x, y, color);
  }

  //add geq left and right
  if (um_data && fftResult) {
    for (int x=0; x < cols/8; x++) {
      uint16_t band = x * cols/8;
      band = constrain(band, 0, 15);
      uint16_t barHeight = map(fftResult[band], 0, 255, 0, 17*rows/32);
      CRGB color = SEGMENT.color_from_palette((band * 35), false, PALETTE_SOLID_WRAP, 0);

      for (int y=0; y < barHeight; y++) {
        SEGMENT.setPixelColorXY(x, rows/2-y, color);
        SEGMENT.setPixelColorXY(cols-1-x, rows/2-y, color);
      }
    }
  }

  return FRAMETIME;
} // mode_2DAkemi
static const char _data_FX_MODE_2DAKEMI[] PROGMEM = "Akemi@Color speed,Dance;Head palette,Arms & Legs,Eyes & Mouth;Face palette;2f;si=0"; //beatsin


// Distortion waves - ldirko
// https://editor.soulmatelights.com/gallery/1089-distorsion-waves
// apated for WLD by @blazoncek
uint16_t mode_2Ddistortionwaves() {
  if (!strip.isMatrix) return mode_static(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  uint8_t speed = SEGMENT.speed/32;
  uint8_t scale = SEGMENT.intensity/32;

  uint8_t  w = 2;

  uint16_t a  = millis()/32;
  uint16_t a2 = a/2;
  uint16_t a3 = a/3;

  uint16_t cx =  beatsin8(10-speed,0,cols-1)*scale;
  uint16_t cy =  beatsin8(12-speed,0,rows-1)*scale;
  uint16_t cx1 = beatsin8(13-speed,0,cols-1)*scale;
  uint16_t cy1 = beatsin8(15-speed,0,rows-1)*scale;
  uint16_t cx2 = beatsin8(17-speed,0,cols-1)*scale;
  uint16_t cy2 = beatsin8(14-speed,0,rows-1)*scale;
  
  uint16_t xoffs = 0;
  for (int x = 0; x < cols; x++) {
    xoffs += scale;
    uint16_t yoffs = 0;

    for (int y = 0; y < rows; y++) {
       yoffs += scale;

      byte rdistort = cos8((cos8(((x<<3)+a )&255)+cos8(((y<<3)-a2)&255)+a3   )&255)>>1; 
      byte gdistort = cos8((cos8(((x<<3)-a2)&255)+cos8(((y<<3)+a3)&255)+a+32 )&255)>>1; 
      byte bdistort = cos8((cos8(((x<<3)+a3)&255)+cos8(((y<<3)-a) &255)+a2+64)&255)>>1; 

      byte valueR = rdistort+ w*  (a- ( ((xoffs - cx)  * (xoffs - cx)  + (yoffs - cy)  * (yoffs - cy))>>7  ));
      byte valueG = gdistort+ w*  (a2-( ((xoffs - cx1) * (xoffs - cx1) + (yoffs - cy1) * (yoffs - cy1))>>7 ));
      byte valueB = bdistort+ w*  (a3-( ((xoffs - cx2) * (xoffs - cx2) + (yoffs - cy2) * (yoffs - cy2))>>7 ));

      valueR = gamma8(cos8(valueR));
      valueG = gamma8(cos8(valueG));
      valueB = gamma8(cos8(valueB));

      SEGMENT.setPixelColorXY(x, y, RGBW32(valueR, valueG, valueB, 0)); 
    }
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_2DDISTORTIONWAVES[] PROGMEM = "Distortion Waves@!,Scale;;;2;";

#endif // WLED_DISABLE_2D


/*
  LED Clock helpers
*/

float _lc_map8f(uint8_t factor, float min, float max) {
  return min + (factor / (float) 255) * (max - min);
}

float _lc_random_float(float min, float max) {
  return min + (random16(UINT16_MAX) / (float) UINT16_MAX) * (max - min);
}


/*
  2sofix original effect
*/

#define _LC_2SX_VMIN .05
#define _LC_2SX_VMAX .5
#define _LC_2SX_RMIN .5
#define _LC_2SX_RMAX 3

typedef struct {
  bool inited = false;
  uint8_t hue;
  uint8_t width;
  uint8_t height;
  float x;
  float y;
  float vx;
  float vy;
} _lc2sofixData;

uint16_t mode_lc2sofix() {
  if (!SEGENV.allocateData(sizeof(_lc2sofixData))) return mode_static(); //allocation failed
  _lc2sofixData* d = reinterpret_cast<_lc2sofixData*>(SEGENV.data);

  if (!d->inited) {
    d->inited = true;

    d->width = SEGMENT.virtualWidth();
    d->height = SEGMENT.virtualHeight();

    d->x = _lc_random_float(0, d->width);
    d->y = _lc_random_float(0, d->height);

    d->vx = _lc_map8f(SEGMENT.speed, _LC_2SX_VMIN, _LC_2SX_VMAX);
    d->vy = _lc_map8f(SEGMENT.speed, _LC_2SX_VMIN, _LC_2SX_VMAX);
  }

  d->x = constrain(d->x + d->vx, 0, d->width);
  d->y = constrain(d->y + d->vy, 0, d->height);

  if (d->x <= 0 || d->x >= d->width) {
      d->vx = d->vx < 0 ? _lc_map8f(SEGMENT.speed, _LC_2SX_VMIN, _LC_2SX_VMAX) : -_lc_map8f(SEGMENT.speed, _LC_2SX_VMIN, _LC_2SX_VMAX);
  }

  if (d->y <= 0 || d->y >= d->height) {
      d->vy = d->vy < 0 ? _lc_map8f(SEGMENT.speed, _LC_2SX_VMIN, _LC_2SX_VMAX) : -_lc_map8f(SEGMENT.speed, _LC_2SX_VMIN, _LC_2SX_VMAX);
  }

  d->hue += 1;

  for (uint8_t x = 0; x < d->width; ++x) {
      for (uint8_t y = 0; y < d->height; ++y) {
          float dist = sqrtf(powf(x - d->x, 2) + powf(y - d->y, 2));

          float radius = sqrtf(powf(d->width, 2) + powf(d->height, 2))
            * _lc_map8f(255 - SEGMENT.intensity, _LC_2SX_RMIN, _LC_2SX_RMAX);

          float dNorm = dist / radius;
          uint8_t hueOffset = fmod(dNorm, 1) * 255;
          uint8_t hue = (d->hue + hueOffset) % 256;

          SEGMENT.setPixelColorXY(x, y, CHSV(hue, 255, 255));
      }
  }

  return FRAMETIME;
}

static const char _data_FX_MODE_LC_2SOFIX[] PROGMEM = "2sofix@Speed,Scale;;;2";


/*
  Vortex
*/

#define _LC_VTX_INC_MIN 0.5
#define _LC_VTX_INC_MAX 20
#define _LC_VTX_INT_MIN 0
#define _LC_VTX_INT_MAX 360

typedef struct {
  bool inited = false;
  uint8_t width;
  uint8_t height;
  float maxRadius;
  float rotation;
} _lcVortexData;

uint16_t mode_lcVortex() {
  if (!SEGENV.allocateData(sizeof(_lcVortexData))) return mode_static(); //allocation failed
  _lcVortexData* d = reinterpret_cast<_lcVortexData*>(SEGENV.data);

  if (!d->inited) {
    d->inited = true;

    d->width = SEGMENT.virtualWidth();
    d->height = SEGMENT.virtualHeight();

    d->maxRadius = sqrtf(powf(d->width / (float) 2, 2) + powf(d->height / (float) 2, 2));

    d->rotation = 0;
  }

  d->rotation += _lc_map8f(SEGMENT.speed, _LC_VTX_INC_MIN, _LC_VTX_INC_MAX);
  d->rotation = fmodf(d->rotation, 360);

  for (uint8_t x = 0; x < d->width; ++x) {
      for (uint8_t y = 0; y < d->height; ++y) {
          float _x = x - d->width / (float) 2;
          float _y = -(y - d->height / (float) 2);

          float absX = abs(_x);
          float absY = abs(_y);

          float deg = _x == 0 || _y == 0 ? 0 : atan(absX / absY) * (180 / PI);

          if (_x > 0 && _y <= 0) {
            deg = 90 + (deg == 0 ? 0 : 90 - deg);
          } else if (_y < 0 && _x <= 0) {
            deg += 180;
          } else if (_x < 0 && _y >= 0) {
            deg = 270 + (deg == 0 ? 0 : 90 - deg);
          }

          deg += d->rotation;

          float radius = (absX == 0 || absY == 0)
            ? (absX == 0 ? absY : absX)
            : sqrtf(powf(absX, 2) + powf(absY, 2));

          deg += (radius / d->maxRadius) * _lc_map8f(SEGMENT.intensity, _LC_VTX_INT_MIN, _LC_VTX_INT_MAX);
          deg = fmodf(deg, 360);

          uint16_t palIdx = (deg / 360) * 255; // map [0-360]f to [0-255]i
          uint32_t color = SEGMENT.color_from_palette(palIdx, false, true, 3, 255);

          SEGMENT.setPixelColorXY(x, y, color);
      }
  }

  return FRAMETIME;
}

static const char _data_FX_MODE_LC_VORTEX[] PROGMEM = "Vortex@Speed,Strength;;!;2";


/*
  Concentric
*/

#define _LC_CON_INC_MIN .003
#define _LC_CON_INC_MAX .05
#define _LC_CON_SCALE_MIN .5
#define _LC_CON_SCALE_MAX 3

typedef struct {
  bool inited = false;
  uint8_t width;
  uint8_t height;
  float radius;
  float offset;
} _lcConcentricData;

uint16_t mode_lcConcentric() {
  if (!SEGENV.allocateData(sizeof(_lcConcentricData))) return mode_static(); //allocation failed
  _lcConcentricData* d = reinterpret_cast<_lcConcentricData*>(SEGENV.data);

  if (!d->inited) {
    d->inited = true;

    d->width = SEGMENT.virtualWidth();
    d->height = SEGMENT.virtualHeight();

    d->radius = sqrtf(powf(d->width / (float) 2, 2) + powf(d->height / (float) 2, 2));

    d->offset = 0;
  }

  d->offset += _lc_map8f(SEGMENT.speed, _LC_CON_INC_MIN, _LC_CON_INC_MAX);
  d->offset = fmodf(d->offset, 1);

  for (uint8_t x = 0; x < d->width; ++x) {
      for (uint8_t y = 0; y < d->height; ++y) {
          float _x = x - d->width / (float) 2;
          float _y = -(y - d->height / (float) 2);

          float absX = abs(_x);
          float absY = abs(_y);

          float radius = (absX == 0 || absY == 0)
            ? (absX == 0 ? absY : absX)
            : sqrtf(powf(absX, 2) + powf(absY, 2));

          float scaledRadius = d->radius * _lc_map8f(SEGMENT.intensity, _LC_CON_SCALE_MIN, _LC_CON_SCALE_MAX);
          float idx = fmodf((1 - d->offset) + radius / scaledRadius, 1);
          uint32_t color = SEGMENT.color_from_palette(idx * 255, false, true, 3, 255);

          SEGMENT.setPixelColorXY(x, y, color);
      }
  }

  return FRAMETIME;
}

static const char _data_FX_LC_CONCENTRIC[] PROGMEM = "Concentric@Speed,Scale;;!;2";


//////////////////////////////////////////////////////////////////////////////////////////
// mode data
static const char _data_RESERVED[] PROGMEM = "RSVD";

// add (or replace reserved) effect mode and data into vector
// use id==255 to find unallocatd gaps (with "Reserved" data string)
// if vector size() is smaller than id (single) data is appended at the end (regardless of id)
void WS2812FX::addEffect(uint8_t id, mode_ptr mode_fn, const char *mode_name) {
  if (id == 255) { // find empty slot
    for (size_t i=1; i<_mode.size(); i++) if (_modeData[i] == _data_RESERVED) { id = i; break; }
  }
  if (id < _mode.size()) {
    if (_modeData[id] != _data_RESERVED) return; // do not overwrite alerady added effect
    _mode[id]     = mode_fn;
    _modeData[id] = mode_name;
  } else {
    _mode.push_back(mode_fn);
    _modeData.push_back(mode_name);
    if (_modeCount < _mode.size()) _modeCount++;
  }
}

void WS2812FX::setupEffectData() {
  // Solid must be first! (assuming vector is empty upon call to setup)
  _mode.push_back(&mode_static);
  _modeData.push_back(_data_FX_MODE_STATIC);
  // fill reserved word in case there will be any gaps in the array
  for (size_t i=1; i<_modeCount; i++) {
    _mode.push_back(&mode_static);
    _modeData.push_back(_data_RESERVED);
  }
  // now replace all pre-allocated effects
  // --- 1D non-audio effects ---
  addEffect(FX_MODE_BLINK, &mode_blink, _data_FX_MODE_BLINK);
  addEffect(FX_MODE_BREATH, &mode_breath, _data_FX_MODE_BREATH);
  addEffect(FX_MODE_COLOR_WIPE, &mode_color_wipe, _data_FX_MODE_COLOR_WIPE);
  addEffect(FX_MODE_COLOR_WIPE_RANDOM, &mode_color_wipe_random, _data_FX_MODE_COLOR_WIPE_RANDOM);
  addEffect(FX_MODE_RANDOM_COLOR, &mode_random_color, _data_FX_MODE_RANDOM_COLOR);
  addEffect(FX_MODE_COLOR_SWEEP, &mode_color_sweep, _data_FX_MODE_COLOR_SWEEP);
  addEffect(FX_MODE_DYNAMIC, &mode_dynamic, _data_FX_MODE_DYNAMIC);
  addEffect(FX_MODE_RAINBOW, &mode_rainbow, _data_FX_MODE_RAINBOW);
  addEffect(FX_MODE_RAINBOW_CYCLE, &mode_rainbow_cycle, _data_FX_MODE_RAINBOW_CYCLE);
  addEffect(FX_MODE_SCAN, &mode_scan, _data_FX_MODE_SCAN);
  addEffect(FX_MODE_DUAL_SCAN, &mode_dual_scan, _data_FX_MODE_DUAL_SCAN);
  addEffect(FX_MODE_FADE, &mode_fade, _data_FX_MODE_FADE);
  addEffect(FX_MODE_THEATER_CHASE, &mode_theater_chase, _data_FX_MODE_THEATER_CHASE);
  addEffect(FX_MODE_THEATER_CHASE_RAINBOW, &mode_theater_chase_rainbow, _data_FX_MODE_THEATER_CHASE_RAINBOW);
  addEffect(FX_MODE_RUNNING_LIGHTS, &mode_running_lights, _data_FX_MODE_RUNNING_LIGHTS);
  addEffect(FX_MODE_SAW, &mode_saw, _data_FX_MODE_SAW);
  addEffect(FX_MODE_TWINKLE, &mode_twinkle, _data_FX_MODE_TWINKLE);
  addEffect(FX_MODE_DISSOLVE, &mode_dissolve, _data_FX_MODE_DISSOLVE);
  addEffect(FX_MODE_DISSOLVE_RANDOM, &mode_dissolve_random, _data_FX_MODE_DISSOLVE_RANDOM);
  addEffect(FX_MODE_SPARKLE, &mode_sparkle, _data_FX_MODE_SPARKLE);
  addEffect(FX_MODE_FLASH_SPARKLE, &mode_flash_sparkle, _data_FX_MODE_FLASH_SPARKLE);
  addEffect(FX_MODE_HYPER_SPARKLE, &mode_hyper_sparkle, _data_FX_MODE_HYPER_SPARKLE);
  addEffect(FX_MODE_STROBE, &mode_strobe, _data_FX_MODE_STROBE);
  addEffect(FX_MODE_STROBE_RAINBOW, &mode_strobe_rainbow, _data_FX_MODE_STROBE_RAINBOW);
  addEffect(FX_MODE_MULTI_STROBE, &mode_multi_strobe, _data_FX_MODE_MULTI_STROBE);
  addEffect(FX_MODE_BLINK_RAINBOW, &mode_blink_rainbow, _data_FX_MODE_BLINK_RAINBOW);
  addEffect(FX_MODE_ANDROID, &mode_android, _data_FX_MODE_ANDROID);
  addEffect(FX_MODE_CHASE_COLOR, &mode_chase_color, _data_FX_MODE_CHASE_COLOR);
  addEffect(FX_MODE_CHASE_RANDOM, &mode_chase_random, _data_FX_MODE_CHASE_RANDOM);
  addEffect(FX_MODE_CHASE_RAINBOW, &mode_chase_rainbow, _data_FX_MODE_CHASE_RAINBOW);
  addEffect(FX_MODE_CHASE_FLASH, &mode_chase_flash, _data_FX_MODE_CHASE_FLASH);
  addEffect(FX_MODE_CHASE_FLASH_RANDOM, &mode_chase_flash_random, _data_FX_MODE_CHASE_FLASH_RANDOM);
  addEffect(FX_MODE_CHASE_RAINBOW_WHITE, &mode_chase_rainbow_white, _data_FX_MODE_CHASE_RAINBOW_WHITE);
  addEffect(FX_MODE_COLORFUL, &mode_colorful, _data_FX_MODE_COLORFUL);
  addEffect(FX_MODE_TRAFFIC_LIGHT, &mode_traffic_light, _data_FX_MODE_TRAFFIC_LIGHT);
  addEffect(FX_MODE_COLOR_SWEEP_RANDOM, &mode_color_sweep_random, _data_FX_MODE_COLOR_SWEEP_RANDOM);
  addEffect(FX_MODE_RUNNING_COLOR, &mode_running_color, _data_FX_MODE_RUNNING_COLOR);
  addEffect(FX_MODE_AURORA, &mode_aurora, _data_FX_MODE_AURORA);
  addEffect(FX_MODE_RUNNING_RANDOM, &mode_running_random, _data_FX_MODE_RUNNING_RANDOM);
  addEffect(FX_MODE_LARSON_SCANNER, &mode_larson_scanner, _data_FX_MODE_LARSON_SCANNER);
  addEffect(FX_MODE_COMET, &mode_comet, _data_FX_MODE_COMET);
  addEffect(FX_MODE_FIREWORKS, &mode_fireworks, _data_FX_MODE_FIREWORKS);
  addEffect(FX_MODE_RAIN, &mode_rain, _data_FX_MODE_RAIN);
  addEffect(FX_MODE_TETRIX, &mode_tetrix, _data_FX_MODE_TETRIX);
  addEffect(FX_MODE_FIRE_FLICKER, &mode_fire_flicker, _data_FX_MODE_FIRE_FLICKER);
  addEffect(FX_MODE_GRADIENT, &mode_gradient, _data_FX_MODE_GRADIENT);
  addEffect(FX_MODE_LOADING, &mode_loading, _data_FX_MODE_LOADING);

  addEffect(FX_MODE_FAIRY, &mode_fairy, _data_FX_MODE_FAIRY);
  addEffect(FX_MODE_TWO_DOTS, &mode_two_dots, _data_FX_MODE_TWO_DOTS);
  addEffect(FX_MODE_FAIRYTWINKLE, &mode_fairytwinkle, _data_FX_MODE_FAIRYTWINKLE);
  addEffect(FX_MODE_RUNNING_DUAL, &mode_running_dual, _data_FX_MODE_RUNNING_DUAL);

  addEffect(FX_MODE_TRICOLOR_CHASE, &mode_tricolor_chase, _data_FX_MODE_TRICOLOR_CHASE);
  addEffect(FX_MODE_TRICOLOR_WIPE, &mode_tricolor_wipe, _data_FX_MODE_TRICOLOR_WIPE);
  addEffect(FX_MODE_TRICOLOR_FADE, &mode_tricolor_fade, _data_FX_MODE_TRICOLOR_FADE);
  addEffect(FX_MODE_LIGHTNING, &mode_lightning, _data_FX_MODE_LIGHTNING);
  addEffect(FX_MODE_ICU, &mode_icu, _data_FX_MODE_ICU);
  addEffect(FX_MODE_MULTI_COMET, &mode_multi_comet, _data_FX_MODE_MULTI_COMET);
  addEffect(FX_MODE_DUAL_LARSON_SCANNER, &mode_dual_larson_scanner, _data_FX_MODE_DUAL_LARSON_SCANNER);
  addEffect(FX_MODE_RANDOM_CHASE, &mode_random_chase, _data_FX_MODE_RANDOM_CHASE);
  addEffect(FX_MODE_OSCILLATE, &mode_oscillate, _data_FX_MODE_OSCILLATE);
  addEffect(FX_MODE_PRIDE_2015, &mode_pride_2015, _data_FX_MODE_PRIDE_2015);
  addEffect(FX_MODE_JUGGLE, &mode_juggle, _data_FX_MODE_JUGGLE);
  addEffect(FX_MODE_PALETTE, &mode_palette, _data_FX_MODE_PALETTE);
  addEffect(FX_MODE_FIRE_2012, &mode_fire_2012, _data_FX_MODE_FIRE_2012);
  addEffect(FX_MODE_COLORWAVES, &mode_colorwaves, _data_FX_MODE_COLORWAVES);
  addEffect(FX_MODE_BPM, &mode_bpm, _data_FX_MODE_BPM);
  addEffect(FX_MODE_FILLNOISE8, &mode_fillnoise8, _data_FX_MODE_FILLNOISE8);
  addEffect(FX_MODE_NOISE16_1, &mode_noise16_1, _data_FX_MODE_NOISE16_1);
  addEffect(FX_MODE_NOISE16_2, &mode_noise16_2, _data_FX_MODE_NOISE16_2);
  addEffect(FX_MODE_NOISE16_3, &mode_noise16_3, _data_FX_MODE_NOISE16_3);
  addEffect(FX_MODE_NOISE16_4, &mode_noise16_4, _data_FX_MODE_NOISE16_4);
  addEffect(FX_MODE_COLORTWINKLE, &mode_colortwinkle, _data_FX_MODE_COLORTWINKLE);
  addEffect(FX_MODE_LAKE, &mode_lake, _data_FX_MODE_LAKE);
  addEffect(FX_MODE_METEOR, &mode_meteor, _data_FX_MODE_METEOR);
  addEffect(FX_MODE_METEOR_SMOOTH, &mode_meteor_smooth, _data_FX_MODE_METEOR_SMOOTH);
  addEffect(FX_MODE_RAILWAY, &mode_railway, _data_FX_MODE_RAILWAY);
  addEffect(FX_MODE_RIPPLE, &mode_ripple, _data_FX_MODE_RIPPLE);
  addEffect(FX_MODE_TWINKLEFOX, &mode_twinklefox, _data_FX_MODE_TWINKLEFOX);
  addEffect(FX_MODE_TWINKLECAT, &mode_twinklecat, _data_FX_MODE_TWINKLECAT);
  addEffect(FX_MODE_HALLOWEEN_EYES, &mode_halloween_eyes, _data_FX_MODE_HALLOWEEN_EYES);
  addEffect(FX_MODE_STATIC_PATTERN, &mode_static_pattern, _data_FX_MODE_STATIC_PATTERN);
  addEffect(FX_MODE_TRI_STATIC_PATTERN, &mode_tri_static_pattern, _data_FX_MODE_TRI_STATIC_PATTERN);
  addEffect(FX_MODE_SPOTS, &mode_spots, _data_FX_MODE_SPOTS);
  addEffect(FX_MODE_SPOTS_FADE, &mode_spots_fade, _data_FX_MODE_SPOTS_FADE);
  addEffect(FX_MODE_GLITTER, &mode_glitter, _data_FX_MODE_GLITTER);
  addEffect(FX_MODE_CANDLE, &mode_candle, _data_FX_MODE_CANDLE);
  addEffect(FX_MODE_STARBURST, &mode_starburst, _data_FX_MODE_STARBURST);
  addEffect(FX_MODE_EXPLODING_FIREWORKS, &mode_exploding_fireworks, _data_FX_MODE_EXPLODING_FIREWORKS);
  addEffect(FX_MODE_BOUNCINGBALLS, &mode_bouncing_balls, _data_FX_MODE_BOUNCINGBALLS);
  addEffect(FX_MODE_SINELON, &mode_sinelon, _data_FX_MODE_SINELON);
  addEffect(FX_MODE_SINELON_DUAL, &mode_sinelon_dual, _data_FX_MODE_SINELON_DUAL);
  addEffect(FX_MODE_SINELON_RAINBOW, &mode_sinelon_rainbow, _data_FX_MODE_SINELON_RAINBOW);
  addEffect(FX_MODE_POPCORN, &mode_popcorn, _data_FX_MODE_POPCORN);
  addEffect(FX_MODE_DRIP, &mode_drip, _data_FX_MODE_DRIP);
  addEffect(FX_MODE_PLASMA, &mode_plasma, _data_FX_MODE_PLASMA);
  addEffect(FX_MODE_PERCENT, &mode_percent, _data_FX_MODE_PERCENT);
  addEffect(FX_MODE_RIPPLE_RAINBOW, &mode_ripple_rainbow, _data_FX_MODE_RIPPLE_RAINBOW);
  addEffect(FX_MODE_HEARTBEAT, &mode_heartbeat, _data_FX_MODE_HEARTBEAT);
  addEffect(FX_MODE_PACIFICA, &mode_pacifica, _data_FX_MODE_PACIFICA);
  addEffect(FX_MODE_CANDLE_MULTI, &mode_candle_multi, _data_FX_MODE_CANDLE_MULTI);
  addEffect(FX_MODE_SOLID_GLITTER, &mode_solid_glitter, _data_FX_MODE_SOLID_GLITTER);
  addEffect(FX_MODE_SUNRISE, &mode_sunrise, _data_FX_MODE_SUNRISE);
  addEffect(FX_MODE_PHASED, &mode_phased, _data_FX_MODE_PHASED);
  addEffect(FX_MODE_TWINKLEUP, &mode_twinkleup, _data_FX_MODE_TWINKLEUP);
  addEffect(FX_MODE_NOISEPAL, &mode_noisepal, _data_FX_MODE_NOISEPAL);
  addEffect(FX_MODE_SINEWAVE, &mode_sinewave, _data_FX_MODE_SINEWAVE);
  addEffect(FX_MODE_PHASEDNOISE, &mode_phased_noise, _data_FX_MODE_PHASEDNOISE);
  addEffect(FX_MODE_FLOW, &mode_flow, _data_FX_MODE_FLOW);
  addEffect(FX_MODE_CHUNCHUN, &mode_chunchun, _data_FX_MODE_CHUNCHUN);
  addEffect(FX_MODE_DANCING_SHADOWS, &mode_dancing_shadows, _data_FX_MODE_DANCING_SHADOWS);
  addEffect(FX_MODE_WASHING_MACHINE, &mode_washing_machine, _data_FX_MODE_WASHING_MACHINE);

  addEffect(FX_MODE_BLENDS, &mode_blends, _data_FX_MODE_BLENDS);
  addEffect(FX_MODE_TV_SIMULATOR, &mode_tv_simulator, _data_FX_MODE_TV_SIMULATOR);
  addEffect(FX_MODE_DYNAMIC_SMOOTH, &mode_dynamic_smooth, _data_FX_MODE_DYNAMIC_SMOOTH);

  // --- 1D audio effects ---
  addEffect(FX_MODE_PIXELS, &mode_pixels, _data_FX_MODE_PIXELS);
  addEffect(FX_MODE_PIXELWAVE, &mode_pixelwave, _data_FX_MODE_PIXELWAVE);
  addEffect(FX_MODE_JUGGLES, &mode_juggles, _data_FX_MODE_JUGGLES);
  addEffect(FX_MODE_MATRIPIX, &mode_matripix, _data_FX_MODE_MATRIPIX);
  addEffect(FX_MODE_GRAVIMETER, &mode_gravimeter, _data_FX_MODE_GRAVIMETER);
  addEffect(FX_MODE_PLASMOID, &mode_plasmoid, _data_FX_MODE_PLASMOID);
  addEffect(FX_MODE_PUDDLES, &mode_puddles, _data_FX_MODE_PUDDLES);
  addEffect(FX_MODE_MIDNOISE, &mode_midnoise, _data_FX_MODE_MIDNOISE);
  addEffect(FX_MODE_NOISEMETER, &mode_noisemeter, _data_FX_MODE_NOISEMETER);
  addEffect(FX_MODE_FREQWAVE, &mode_freqwave, _data_FX_MODE_FREQWAVE);
  addEffect(FX_MODE_FREQMATRIX, &mode_freqmatrix, _data_FX_MODE_FREQMATRIX);

  addEffect(FX_MODE_WATERFALL, &mode_waterfall, _data_FX_MODE_WATERFALL);
  addEffect(FX_MODE_FREQPIXELS, &mode_freqpixels, _data_FX_MODE_FREQPIXELS);

  addEffect(FX_MODE_NOISEFIRE, &mode_noisefire, _data_FX_MODE_NOISEFIRE);
  addEffect(FX_MODE_PUDDLEPEAK, &mode_puddlepeak, _data_FX_MODE_PUDDLEPEAK);
  addEffect(FX_MODE_NOISEMOVE, &mode_noisemove, _data_FX_MODE_NOISEMOVE);

  addEffect(FX_MODE_PERLINMOVE, &mode_perlinmove, _data_FX_MODE_PERLINMOVE);
  addEffect(FX_MODE_RIPPLEPEAK, &mode_ripplepeak, _data_FX_MODE_RIPPLEPEAK);

  addEffect(FX_MODE_FREQMAP, &mode_freqmap, _data_FX_MODE_FREQMAP);
  addEffect(FX_MODE_GRAVCENTER, &mode_gravcenter, _data_FX_MODE_GRAVCENTER);
  addEffect(FX_MODE_GRAVCENTRIC, &mode_gravcentric, _data_FX_MODE_GRAVCENTRIC);
  addEffect(FX_MODE_GRAVFREQ, &mode_gravfreq, _data_FX_MODE_GRAVFREQ);
  addEffect(FX_MODE_DJLIGHT, &mode_DJLight, _data_FX_MODE_DJLIGHT);

  addEffect(FX_MODE_BLURZ, &mode_blurz, _data_FX_MODE_BLURZ);

  addEffect(FX_MODE_FLOWSTRIPE, &mode_FlowStripe, _data_FX_MODE_FLOWSTRIPE);

  addEffect(FX_MODE_WAVESINS, &mode_wavesins, _data_FX_MODE_WAVESINS);
  addEffect(FX_MODE_ROCKTAVES, &mode_rocktaves, _data_FX_MODE_ROCKTAVES);

  // --- 2D  effects ---
#ifndef WLED_DISABLE_2D
  addEffect(FX_MODE_2DSPACESHIPS, &mode_2Dspaceships, _data_FX_MODE_2DSPACESHIPS);
  addEffect(FX_MODE_2DCRAZYBEES, &mode_2Dcrazybees, _data_FX_MODE_2DCRAZYBEES);
  addEffect(FX_MODE_2DGHOSTRIDER, &mode_2Dghostrider, _data_FX_MODE_2DGHOSTRIDER);
  addEffect(FX_MODE_2DBLOBS, &mode_2Dfloatingblobs, _data_FX_MODE_2DBLOBS);
  addEffect(FX_MODE_2DSCROLLTEXT, &mode_2Dscrollingtext, _data_FX_MODE_2DSCROLLTEXT);
  addEffect(FX_MODE_2DDRIFTROSE, &mode_2Ddriftrose, _data_FX_MODE_2DDRIFTROSE);
  addEffect(FX_MODE_2DDISTORTIONWAVES, &mode_2Ddistortionwaves, _data_FX_MODE_2DDISTORTIONWAVES);

  addEffect(FX_MODE_2DGEQ, &mode_2DGEQ, _data_FX_MODE_2DGEQ); // audio

  addEffect(FX_MODE_2DNOISE, &mode_2Dnoise, _data_FX_MODE_2DNOISE);

  addEffect(FX_MODE_2DFIRENOISE, &mode_2Dfirenoise, _data_FX_MODE_2DFIRENOISE);
  addEffect(FX_MODE_2DSQUAREDSWIRL, &mode_2Dsquaredswirl, _data_FX_MODE_2DSQUAREDSWIRL);

  //non audio
  addEffect(FX_MODE_2DDNA, &mode_2Ddna, _data_FX_MODE_2DDNA);
  addEffect(FX_MODE_2DMATRIX, &mode_2Dmatrix, _data_FX_MODE_2DMATRIX);
  addEffect(FX_MODE_2DMETABALLS, &mode_2Dmetaballs, _data_FX_MODE_2DMETABALLS);
  addEffect(FX_MODE_2DFUNKYPLANK, &mode_2DFunkyPlank, _data_FX_MODE_2DFUNKYPLANK); // audio

  addEffect(FX_MODE_2DPULSER, &mode_2DPulser, _data_FX_MODE_2DPULSER);

  addEffect(FX_MODE_2DDRIFT, &mode_2DDrift, _data_FX_MODE_2DDRIFT);
  addEffect(FX_MODE_2DWAVERLY, &mode_2DWaverly, _data_FX_MODE_2DWAVERLY); // audio
  addEffect(FX_MODE_2DSUNRADIATION, &mode_2DSunradiation, _data_FX_MODE_2DSUNRADIATION);
  addEffect(FX_MODE_2DCOLOREDBURSTS, &mode_2DColoredBursts, _data_FX_MODE_2DCOLOREDBURSTS);
  addEffect(FX_MODE_2DJULIA, &mode_2DJulia, _data_FX_MODE_2DJULIA);

  addEffect(FX_MODE_2DGAMEOFLIFE, &mode_2Dgameoflife, _data_FX_MODE_2DGAMEOFLIFE);
  addEffect(FX_MODE_2DTARTAN, &mode_2Dtartan, _data_FX_MODE_2DTARTAN);
  addEffect(FX_MODE_2DPOLARLIGHTS, &mode_2DPolarLights, _data_FX_MODE_2DPOLARLIGHTS);
  addEffect(FX_MODE_2DSWIRL, &mode_2DSwirl, _data_FX_MODE_2DSWIRL); // audio
  addEffect(FX_MODE_2DLISSAJOUS, &mode_2DLissajous, _data_FX_MODE_2DLISSAJOUS);
  addEffect(FX_MODE_2DFRIZZLES, &mode_2DFrizzles, _data_FX_MODE_2DFRIZZLES);
  addEffect(FX_MODE_2DPLASMABALL, &mode_2DPlasmaball, _data_FX_MODE_2DPLASMABALL);

  addEffect(FX_MODE_2DHIPHOTIC, &mode_2DHiphotic, _data_FX_MODE_2DHIPHOTIC);
  addEffect(FX_MODE_2DSINDOTS, &mode_2DSindots, _data_FX_MODE_2DSINDOTS);
  addEffect(FX_MODE_2DDNASPIRAL, &mode_2DDNASpiral, _data_FX_MODE_2DDNASPIRAL);
  addEffect(FX_MODE_2DBLACKHOLE, &mode_2DBlackHole, _data_FX_MODE_2DBLACKHOLE);

  addEffect(FX_MODE_2DAKEMI, &mode_2DAkemi, _data_FX_MODE_2DAKEMI); // audio

  addEffect(FX_MODE_LC_2SOFIX, &mode_lc2sofix, _data_FX_MODE_LC_2SOFIX);
  addEffect(FX_MODE_LC_VORTEX, &mode_lcVortex, _data_FX_MODE_LC_VORTEX);
  addEffect(FX_MODE_LC_CONCENTRIC, &mode_lcConcentric, _data_FX_LC_CONCENTRIC);
#endif // WLED_DISABLE_2D
}
