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

#include "WS2812FX.h"

#define IBN 5100
#define PALETTE_SOLID_WRAP (paletteBlend == 1 || paletteBlend == 3)


/*
 * No blinking. Just plain old static light.
 */
uint16_t WS2812FX::mode_static(void) {
  fill(SEGMENT.colors[0]);
  return (SEGMENT_RUNTIME.trans_act == 1) ? 20 : 500;
}


/*
 * Blink/strobe function
 * Alternate between color1 and color2
 * if(strobe == true) then create a strobe effect
 */
uint16_t WS2812FX::blink(uint32_t color1, uint32_t color2, bool strobe, bool do_palette) {
  uint32_t color = ((SEGMENT_RUNTIME.counter_mode_call & 1) == 0) ? color1 : color2;
  if (color == color1 && do_palette)
  {
    for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
      setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));
    }
  } else {
    fill(color);
  }

  if((SEGMENT_RUNTIME.counter_mode_call & 1) == 0) {
    return strobe ? 20 : (100 + ((1986 * (uint32_t)(255 - SEGMENT.speed)) / 255))*(float)(SEGMENT.intensity/128.0);
  } else {
    return strobe ? 50 + ((1986 * (uint32_t)(255 - SEGMENT.speed)) / 255) : (100 + ((1986 * (uint32_t)(255 - SEGMENT.speed)) / 255))*(float)(2.0-(SEGMENT.intensity/128.0));
  }
}


/*
 * Normal blinking. 50% on/off time.
 */
uint16_t WS2812FX::mode_blink(void) {
  return blink(SEGMENT.colors[0], SEGMENT.colors[1], false, true);
}


/*
 * Classic Blink effect. Cycling through the rainbow.
 */
uint16_t WS2812FX::mode_blink_rainbow(void) {
  return blink(color_wheel(SEGMENT_RUNTIME.counter_mode_call & 0xFF), SEGMENT.colors[1], false, false);
}


/*
 * Classic Strobe effect.
 */
uint16_t WS2812FX::mode_strobe(void) {
  return blink(SEGMENT.colors[0], SEGMENT.colors[1], true, true);
}


/*
 * Classic Strobe effect. Cycling through the rainbow.
 */
uint16_t WS2812FX::mode_strobe_rainbow(void) {
  return blink(color_wheel(SEGMENT_RUNTIME.counter_mode_call & 0xFF), SEGMENT.colors[1], true, false);
}


/*
 * Color wipe function
 * LEDs are turned on (color1) in sequence, then turned off (color2) in sequence.
 * if (bool rev == true) then LEDs are turned off in reverse order
 */
uint16_t WS2812FX::color_wipe(uint32_t color1, uint32_t color2, bool rev, bool dopalette) {
  if(SEGMENT_RUNTIME.counter_mode_step < SEGMENT_LENGTH) {
    uint32_t led_offset = SEGMENT_RUNTIME.counter_mode_step;
    uint16_t i = SEGMENT.start + led_offset;
    setPixelColor(i, dopalette ? color_from_palette(i, true, PALETTE_SOLID_WRAP, 0) : color1);
  } else {
    uint32_t led_offset = SEGMENT_RUNTIME.counter_mode_step - SEGMENT_LENGTH;
    if(rev) {
      setPixelColor(SEGMENT.stop - led_offset, color2);
    } else {
      setPixelColor(SEGMENT.start + led_offset, color2);
    }
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % (SEGMENT_LENGTH * 2);
  return SPEED_FORMULA_L;
}


/*
 * Lights all LEDs one after another.
 */
uint16_t WS2812FX::mode_color_wipe(void) {
  return color_wipe(SEGMENT.colors[0], SEGMENT.colors[1], false, true);
}

/*
 * Lights all LEDs one after another. Turns off opposite
 */
uint16_t WS2812FX::mode_color_sweep(void) {
  return color_wipe(SEGMENT.colors[0], SEGMENT.colors[1], true, true);
}


/*
 * Turns all LEDs after each other to a random color.
 * Then starts over with another color.
 */
uint16_t WS2812FX::mode_color_wipe_random(void) {
  if(SEGMENT_RUNTIME.counter_mode_step % SEGMENT_LENGTH == 0) { // aux_param will store our random color wheel index
    SEGMENT_RUNTIME.aux_param = get_random_wheel_index(SEGMENT_RUNTIME.aux_param);
  }
  uint32_t color = color_wheel(SEGMENT_RUNTIME.aux_param);
  return color_wipe(color, color, false, false);
}


/*
 * Random color introduced alternating from start and end of strip.
 */
uint16_t WS2812FX::mode_color_sweep_random(void) {
  if(SEGMENT_RUNTIME.counter_mode_step % SEGMENT_LENGTH == 0) { // aux_param will store our random color wheel index
    SEGMENT_RUNTIME.aux_param = get_random_wheel_index(SEGMENT_RUNTIME.aux_param);
  }
  uint32_t color = color_wheel(SEGMENT_RUNTIME.aux_param);
  return color_wipe(color, color, true, false);
}


/*
 * Lights all LEDs in one random color up. Then switches them
 * to the next random color.
 */
uint16_t WS2812FX::mode_random_color(void) {
  SEGMENT_RUNTIME.aux_param = get_random_wheel_index(SEGMENT_RUNTIME.aux_param); // aux_param will store our random color wheel index
  uint32_t color = color_wheel(SEGMENT_RUNTIME.aux_param);

  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, color);
  }
  return 50 + (20 * (uint32_t)(255 - SEGMENT.speed));
}


/*
 * Lights every LED in a random color. Changes all LED at the same time
 * to new random colors.
 */
uint16_t WS2812FX::mode_dynamic(void) {
  if(SEGMENT.intensity > 127 || SEGMENT_RUNTIME.counter_mode_call == 0) {
    for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
      setPixelColor(i, color_wheel(random8()));
    }
  }
  setPixelColor(SEGMENT.start + random16(SEGMENT_LENGTH), color_wheel(random8()));
  return 50 + (15 * (uint32_t)(255 - SEGMENT.speed));
}


/*
 * Does the "standby-breathing" of well known i-Devices. Fixed Speed.
 * Use mode "fade" if you like to have something similar with a different speed.
 */
uint16_t WS2812FX::mode_breath(void) {
  int lum = SEGMENT_RUNTIME.counter_mode_step;
  if(lum > 255) lum = 511 - lum; // lum = 15 -> 255 -> 15

  uint16_t delay;
  if(lum == 15) delay = 465; // 970 pause before each breath
  else if(lum <=  25) delay = 19; // 19
  else if(lum <=  50) delay = 18; // 18
  else if(lum <=  75) delay = 14; // 14
  else if(lum <= 100) delay = 10; // 10
  else if(lum <= 125) delay = 7; // 7
  else if(lum <= 150) delay = 5; // 5
  else delay = 4; // 4

  if (SEGMENT.palette == 0)
  {
    uint32_t color = SEGMENT.colors[0];
    uint8_t w = ((color >> 24 & 0xFF) * lum) >> 8;
    uint8_t r = ((color >> 16 & 0xFF) * lum) >> 8;
    uint8_t g = ((color >>  8 & 0xFF) * lum) >> 8;
    uint8_t b = ((color       & 0xFF) * lum) >> 8;
    for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
      setPixelColor(i, r, g, b, w);
    }
  } else
  {
    for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
      setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0, lum));
    }
  }

  SEGMENT_RUNTIME.counter_mode_step += 2;
  if(SEGMENT_RUNTIME.counter_mode_step > (512-15)) SEGMENT_RUNTIME.counter_mode_step = 15;
  return delay * (((256 - SEGMENT.speed)/64) +1);
}


/*
 * Fades the LEDs between two colors
 */
uint16_t WS2812FX::mode_fade(void) {
  int lum = SEGMENT_RUNTIME.counter_mode_step;
  if(lum > 255) lum = 511 - lum; // lum = 0 -> 255 -> 0
  
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, color_blend(color_from_palette(i, true, PALETTE_SOLID_WRAP, 0), SEGMENT.colors[1], lum));
  }

  SEGMENT_RUNTIME.counter_mode_step += 4;
  if(SEGMENT_RUNTIME.counter_mode_step > 511) SEGMENT_RUNTIME.counter_mode_step = 0;
  return 5 + ((15 * (uint32_t)(255 - SEGMENT.speed)) / 255);
}


/*
 * Scan mode parent function
 */
uint16_t WS2812FX::scan(bool dual)
{
  if(SEGMENT_RUNTIME.counter_mode_step > (SEGMENT_LENGTH * 2) - 3) {
    SEGMENT_RUNTIME.counter_mode_step = 0;
  }

  fill(SEGMENT.colors[1]);

  int led_offset = SEGMENT_RUNTIME.counter_mode_step - (SEGMENT_LENGTH - 1);
  led_offset = abs(led_offset);

  uint16_t i = SEGMENT.start + led_offset;
  setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));

  if (dual) {
    uint16_t i2 = SEGMENT.start + SEGMENT_LENGTH - led_offset - 1;
    setPixelColor(i2, color_from_palette(i2, true, PALETTE_SOLID_WRAP, 0));
  }

  SEGMENT_RUNTIME.counter_mode_step++;
  return SPEED_FORMULA_L;
}


//TODO add intensity (more than 1 pixel lit)
/*
 * Runs a single pixel back and forth.
 */
uint16_t WS2812FX::mode_scan(void) {
  return scan(false);
}


/*
 * Runs two pixel back and forth in opposite directions.
 */
uint16_t WS2812FX::mode_dual_scan(void) {
  return scan(true);
}


/*
 * Cycles all LEDs at once through a rainbow.
 */
uint16_t WS2812FX::mode_rainbow(void) {
  uint32_t color = color_wheel(SEGMENT_RUNTIME.counter_mode_step);
  fill(color);

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) & 0xFF;
  return 1 + (((uint32_t)(255 - SEGMENT.speed)) / 5);
}


/*
 * Cycles a rainbow over the entire string of LEDs.
 */
uint16_t WS2812FX::mode_rainbow_cycle(void) {
  for(uint16_t i=0; i < SEGMENT_LENGTH; i++) {
    uint32_t color = color_wheel(((i * 256 / ((uint16_t)(SEGMENT_LENGTH*(float)(SEGMENT.intensity/128.0))+1)) + SEGMENT_RUNTIME.counter_mode_step) & 0xFF);
    setPixelColor(SEGMENT.start + i, color);
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) & 0xFF;
  return 1 + (((uint32_t)(255 - SEGMENT.speed)) / 5);
}


/*
 * theater chase function
 */
uint16_t WS2812FX::theater_chase(uint32_t color1, uint32_t color2, bool dopalette) {
  SEGMENT_RUNTIME.counter_mode_call = SEGMENT_RUNTIME.counter_mode_call % 3;
  for(uint16_t i=0; i < SEGMENT_LENGTH; i++) {
    if((i % 3) == SEGMENT_RUNTIME.counter_mode_call) {
      if (dopalette)
      {
        setPixelColor(SEGMENT.start + i, color_from_palette(SEGMENT.start + i, true, PALETTE_SOLID_WRAP, 0));
      } else {
        setPixelColor(SEGMENT.start + i, color1);
      }
    } else {
      setPixelColor(SEGMENT.start + i, color2);
    }
  }
  return 50 + (2 * (uint32_t)(255 - SEGMENT.speed));
}


/*
 * Theatre-style crawling lights.
 * Inspired by the Adafruit examples.
 */
uint16_t WS2812FX::mode_theater_chase(void) {
  return theater_chase(SEGMENT.colors[0], SEGMENT.colors[1], true);
}


/*
 * Theatre-style crawling lights with rainbow effect.
 * Inspired by the Adafruit examples.
 */
uint16_t WS2812FX::mode_theater_chase_rainbow(void) {
  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) & 0xFF;
  return theater_chase(color_wheel(SEGMENT_RUNTIME.counter_mode_step), SEGMENT.colors[1], false);
}


/*
 * Running lights effect with smooth sine transition base.
 */
uint16_t WS2812FX::running_base(bool saw) {
  uint8_t x_scale = SEGMENT.intensity >> 2;

  for(uint16_t i=0; i < SEGMENT_LENGTH; i++) {
    uint8_t s = 0;
    uint8_t a = i*x_scale - (SEGMENT_RUNTIME.counter_mode_step >> 4);
    if (saw) {
      if (a < 16)
      {
        a = 192 + a*8;
      } else {
        a = map(a,16,255,64,192);
      }
    }
    s = sin8(a);
    setPixelColor(SEGMENT.start + i, color_blend(color_from_palette(SEGMENT.start + i, true, PALETTE_SOLID_WRAP, 0), SEGMENT.colors[1], s));
  }
  SEGMENT_RUNTIME.counter_mode_step += SEGMENT.speed;
  return 20;
}


/*
 * Running lights effect with smooth sine transition.
 */
uint16_t WS2812FX::mode_running_lights(void) {
  return running_base(false);
}


/*
 * Running lights effect with sawtooth transition.
 */
uint16_t WS2812FX::mode_saw(void) {
  return running_base(true);
}


/*
 * Blink several LEDs in random colors on, reset, repeat.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
uint16_t WS2812FX::mode_twinkle(void) {
  if(SEGMENT_RUNTIME.counter_mode_step == 0) {
    fill(SEGMENT.colors[1]);
    SEGMENT_RUNTIME.counter_mode_step = map(SEGMENT.intensity, 0, 255, 1, SEGMENT_LENGTH); // make sure, at least one LED is on
  }

  uint16_t i = SEGMENT.start + random16(SEGMENT_LENGTH);
  setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));

  SEGMENT_RUNTIME.counter_mode_step--;
  return 20 + (5 * (uint16_t)(255 - SEGMENT.speed));
}


/*
 * Dissolve function
 */
uint16_t WS2812FX::dissolve(uint32_t color) {
  bool wa = (SEGMENT.colors[1] != 0 && _brightness < 255); //workaround, can't compare getPixel to color if not full brightness
  
  for (uint16_t j = 0; j <= SEGMENT_LENGTH / 15; j++)
  {
    if (random8() <= SEGMENT.intensity) {
      for (uint8_t times = 0; times < 10; times++) //attempt to spawn a new pixel 5 times
      {
        uint16_t i = SEGMENT.start + random16(SEGMENT_LENGTH);
        if (SEGMENT_RUNTIME.aux_param) { //dissolve to primary/palette
          if (getPixelColor(i) == SEGMENT.colors[1] || wa) {
            if (color == SEGMENT.colors[0])
            {
              setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));
            } else { setPixelColor(i, color); }     
            break; //only spawn 1 new pixel per frame per 50 LEDs
          }
        } else { //dissolve to secondary
          if (getPixelColor(i) != SEGMENT.colors[1]) { setPixelColor(i, SEGMENT.colors[1]); break; }
        }
      }
    }
  }

  if (SEGMENT_RUNTIME.counter_mode_call > (255 - SEGMENT.speed) + 15) 
  {
    SEGMENT_RUNTIME.aux_param = !SEGMENT_RUNTIME.aux_param;
    SEGMENT_RUNTIME.counter_mode_call = 0;
  }
  
  return 20;
}


/*
 * Blink several LEDs on and then off
 */
uint16_t WS2812FX::mode_dissolve(void) {
  return dissolve(SEGMENT.colors[0]);
}


/*
 * Blink several LEDs on and then off in random colors
 */
uint16_t WS2812FX::mode_dissolve_random(void) {
  return dissolve(color_wheel(random8()));
}


/*
 * Blinks one LED at a time.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
uint16_t WS2812FX::mode_sparkle(void) {
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
      setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 1));
  }
  SEGMENT_RUNTIME.aux_param = random16(SEGMENT_LENGTH); // aux_param stores the random led index
  setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.aux_param, SEGMENT.colors[0]);
  return 10 + (uint16_t)(255 - SEGMENT.speed);
}


/*
 * Lights all LEDs in the color. Flashes single white pixels randomly.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
uint16_t WS2812FX::mode_flash_sparkle(void) {
  if(SEGMENT_RUNTIME.counter_mode_call == 0) {
    for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
      setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));
    }
  }

  uint16_t i = SEGMENT.start + SEGMENT_RUNTIME.aux_param;
  setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));

  if(random8(5) == 0) {
    SEGMENT_RUNTIME.aux_param = random16(SEGMENT_LENGTH); // aux_param stores the random led index
    setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.aux_param, SEGMENT.colors[1]);
    return 20;
  } 
  return 20 + (uint16_t)(255-SEGMENT.speed);
}


/*
 * Like flash sparkle. With more flash.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
uint16_t WS2812FX::mode_hyper_sparkle(void) {
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));
  }

  if(random8(5) < 2) {
    for(uint16_t i=0; i < max(1, SEGMENT_LENGTH/3); i++) {
      setPixelColor(SEGMENT.start + random16(SEGMENT_LENGTH), SEGMENT.colors[1]);
    }
    return 20;
  }
  return 20 + (uint16_t)(255-SEGMENT.speed);
}


/*
 * Strobe effect with different strobe count and pause, controlled by speed.
 */
uint16_t WS2812FX::mode_multi_strobe(void) {
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 1));
  }

  uint16_t delay = 50 + 20*(uint16_t)(255-SEGMENT.speed);
  uint16_t count = 2 * ((SEGMENT.speed / 10) + 1);
  if(SEGMENT_RUNTIME.counter_mode_step < count) {
    if((SEGMENT_RUNTIME.counter_mode_step & 1) == 0) {
      for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
        setPixelColor(i, SEGMENT.colors[0]);
      }
      delay = 20;
    } else {
      delay = 50;
    }
  }
  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % (count + 1);
  return delay;
}

/*
 * Android loading circle
 */
uint16_t WS2812FX::mode_android(void) {
  if (SEGMENT_RUNTIME.counter_mode_call == 0)
  {
    SEGMENT_RUNTIME.aux_param = 0;
    SEGMENT_RUNTIME.counter_mode_step = SEGMENT.start;
  }
  
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 1));
  }

  if (SEGMENT_RUNTIME.aux_param2 > ((float)SEGMENT.intensity/255.0)*(float)SEGMENT_LENGTH)
  {
    SEGMENT_RUNTIME.aux_param = 1;
  } else
  {
    if (SEGMENT_RUNTIME.aux_param2 < 2) SEGMENT_RUNTIME.aux_param = 0;
  }

  uint16_t a = SEGMENT_RUNTIME.counter_mode_step;
  
  if (SEGMENT_RUNTIME.aux_param == 0)
  {
    if (SEGMENT_RUNTIME.counter_mode_call %3 == 1) {a++;}
    else {SEGMENT_RUNTIME.aux_param2++;}
  } else
  {
    a++;
    if (SEGMENT_RUNTIME.counter_mode_call %3 != 1) SEGMENT_RUNTIME.aux_param2--;
  }
  
  if (a > SEGMENT.stop) a = SEGMENT.start;

  if (a + SEGMENT_RUNTIME.aux_param2 <= SEGMENT.stop)
  {
    for(int i = a; i < a+SEGMENT_RUNTIME.aux_param2; i++) {
      setPixelColor(i, SEGMENT.colors[0]);
    }
  } else
  {
    for(int i = a; i <= SEGMENT.stop; i++) {
      setPixelColor(i, SEGMENT.colors[0]);
    }
    for(int i = SEGMENT.start; i < SEGMENT_RUNTIME.aux_param2 - (SEGMENT.stop +1 -a); i++) {
      setPixelColor(i, SEGMENT.colors[0]);
    }
  }
  SEGMENT_RUNTIME.counter_mode_step = a;

  return 3 + ((8 * (uint32_t)(255 - SEGMENT.speed)) / SEGMENT_LENGTH);
}

/*
 * color chase function.
 * color1 = background color
 * color2 and color3 = colors of two adjacent leds
 */
uint16_t WS2812FX::chase(uint32_t color1, uint32_t color2, uint32_t color3, bool dopalette) {
  uint16_t a = SEGMENT_RUNTIME.counter_mode_step;
  uint16_t b = (a + 1) % SEGMENT_LENGTH;
  uint16_t c = (b + 1) % SEGMENT_LENGTH;

  if (dopalette) color1 = color_from_palette(SEGMENT.start + a, true, PALETTE_SOLID_WRAP, 1);

  setPixelColor(SEGMENT.start + a, color1);
  setPixelColor(SEGMENT.start + b, color2);
  setPixelColor(SEGMENT.start + c, color3);

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
  return SPEED_FORMULA_L;
}


/*
 * Bicolor chase, more primary color.
 */
uint16_t WS2812FX::mode_chase_color(void) {
  return chase(SEGMENT.colors[1], SEGMENT.colors[0], SEGMENT.colors[0], true);
}


/*
 * Primary running followed by random color.
 */
uint16_t WS2812FX::mode_chase_random(void) {
  if(SEGMENT_RUNTIME.counter_mode_step == 0) {
    SEGMENT_RUNTIME.aux_param = get_random_wheel_index(SEGMENT_RUNTIME.aux_param);
  }
  return chase(color_wheel(SEGMENT_RUNTIME.aux_param), SEGMENT.colors[0], SEGMENT.colors[0], false);
}


/*
 * Primary running on rainbow.
 */
uint16_t WS2812FX::mode_chase_rainbow_white(void) {
  uint16_t n = SEGMENT_RUNTIME.counter_mode_step;
  uint16_t m = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
  uint32_t color2 = color_wheel(((n * 256 / SEGMENT_LENGTH) + (SEGMENT_RUNTIME.counter_mode_call & 0xFF)) & 0xFF);
  uint32_t color3 = color_wheel(((m * 256 / SEGMENT_LENGTH) + (SEGMENT_RUNTIME.counter_mode_call & 0xFF)) & 0xFF);

  return chase(SEGMENT.colors[0], color2, color3, false);
}


/*
 * Red - Amber - Green - Blue lights running
 */
uint16_t WS2812FX::mode_colorful(void) {
  uint32_t cols[]{0x00FF0000,0x00EEBB00,0x0000EE00,0x000077CC,0x00FF0000,0x00EEBB00,0x0000EE00};
  if (SEGMENT.intensity < 127) //pastel (easter) colors
  {
    cols[0] = 0x00FF8040;
    cols[1] = 0x00E5D241;
    cols[2] = 0x0077FF77;
    cols[3] = 0x0077F0F0;
    for (uint8_t i = 4; i < 7; i++) cols[i] = cols[i-4];
  }
  int i = SEGMENT.start;
  for (i; i <= SEGMENT.stop ; i+=4)
  {
    setPixelColor(i, cols[SEGMENT_RUNTIME.counter_mode_step]);
    setPixelColor(i+1, cols[SEGMENT_RUNTIME.counter_mode_step+1]);
    setPixelColor(i+2, cols[SEGMENT_RUNTIME.counter_mode_step+2]);
    setPixelColor(i+3, cols[SEGMENT_RUNTIME.counter_mode_step+3]);
  }
  i+=4;
  if(i <= SEGMENT.stop)
  {
    setPixelColor(i, cols[SEGMENT_RUNTIME.counter_mode_step]);
    
    if(i+1 <= SEGMENT.stop)
    {
      setPixelColor(i+1, cols[SEGMENT_RUNTIME.counter_mode_step+1]);
      
      if(i+2 <= SEGMENT.stop)
      {
        setPixelColor(i+2, cols[SEGMENT_RUNTIME.counter_mode_step+2]);
      }
    }
  }
  
  if (SEGMENT.speed > 0) SEGMENT_RUNTIME.counter_mode_step++; //static if lowest speed
  if (SEGMENT_RUNTIME.counter_mode_step >3) SEGMENT_RUNTIME.counter_mode_step = 0;
  return 50 + (15 * (uint32_t)(255 - SEGMENT.speed));
}


/*
 * Emulates a traffic light.
 */
uint16_t WS2812FX::mode_traffic_light(void) {
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++)
    setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 1));
  uint32_t mdelay = 500;
  for (int i = SEGMENT.start; i < SEGMENT.stop-1 ; i+=3)
  {
    switch (SEGMENT_RUNTIME.counter_mode_step)
    {
      case 0: setPixelColor(i, 0x00FF0000); mdelay = 150 + (100 * (uint32_t)(255 - SEGMENT.speed));break;
      case 1: setPixelColor(i, 0x00FF0000); mdelay = 150 + (20 * (uint32_t)(255 - SEGMENT.speed)); setPixelColor(i+1, 0x00EECC00); break;
      case 2: setPixelColor(i+2, 0x0000FF00); mdelay = 150 + (100 * (uint32_t)(255 - SEGMENT.speed));break;
      case 3: setPixelColor(i+1, 0x00EECC00); mdelay = 150 + (20 * (uint32_t)(255 - SEGMENT.speed));break;
    }
  }

  SEGMENT_RUNTIME.counter_mode_step++;
  if (SEGMENT_RUNTIME.counter_mode_step >3) SEGMENT_RUNTIME.counter_mode_step = 0;
  return mdelay;
}


/*
 * Primary, secondary running on rainbow.
 */
uint16_t WS2812FX::mode_chase_rainbow(void) {
  uint8_t color_sep = 256 / SEGMENT_LENGTH;
  uint8_t color_index = SEGMENT_RUNTIME.counter_mode_call & 0xFF;
  uint32_t color = color_wheel(((SEGMENT_RUNTIME.counter_mode_step * color_sep) + color_index) & 0xFF);

  return chase(color, SEGMENT.colors[0], SEGMENT.colors[1], 0);
}


/*
 * Sec flashes running on prim.
 */
uint16_t WS2812FX::mode_chase_flash(void) {
  const static uint8_t flash_count = 4;
  uint8_t flash_step = SEGMENT_RUNTIME.counter_mode_call % ((flash_count * 2) + 1);

  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));
  }

  uint16_t delay = 10 + ((30 * (uint16_t)(255 - SEGMENT.speed)) / SEGMENT_LENGTH);
  if(flash_step < (flash_count * 2)) {
    if(flash_step % 2 == 0) {
      uint16_t n = SEGMENT_RUNTIME.counter_mode_step;
      uint16_t m = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
      setPixelColor(SEGMENT.start + n, SEGMENT.colors[1]);
      setPixelColor(SEGMENT.start + m, SEGMENT.colors[1]);
      delay = 20;
    } else {
      delay = 30;
    }
  } else {
    SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
  }
  return delay;
}


/*
 * Prim flashes running, followed by random color.
 */
uint16_t WS2812FX::mode_chase_flash_random(void) {
  const static uint8_t flash_count = 4;
  uint8_t flash_step = SEGMENT_RUNTIME.counter_mode_call % ((flash_count * 2) + 1);

  for(uint16_t i=0; i < SEGMENT_RUNTIME.counter_mode_step; i++) {
    setPixelColor(SEGMENT.start + i, color_wheel(SEGMENT_RUNTIME.aux_param));
  }

  uint16_t delay = 1 + ((10 * (uint16_t)(255 - SEGMENT.speed)) / SEGMENT_LENGTH);
  if(flash_step < (flash_count * 2)) {
    uint16_t n = SEGMENT_RUNTIME.counter_mode_step;
    uint16_t m = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
    if(flash_step % 2 == 0) {
      setPixelColor(SEGMENT.start + n, SEGMENT.colors[0]);
      setPixelColor(SEGMENT.start + m, SEGMENT.colors[0]);
      delay = 20;
    } else {
      setPixelColor(SEGMENT.start + n, color_wheel(SEGMENT_RUNTIME.aux_param));
      setPixelColor(SEGMENT.start + m, SEGMENT.colors[1]);
      delay = 30;
    }
  } else {
    SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;

    if(SEGMENT_RUNTIME.counter_mode_step == 0) {
      SEGMENT_RUNTIME.aux_param = get_random_wheel_index(SEGMENT_RUNTIME.aux_param);
    }
  }
  return delay;
}


/*
 * Alternating pixels running function.
 */
uint16_t WS2812FX::running(uint32_t color1, uint32_t color2) {
  for(uint16_t i=0; i < SEGMENT_LENGTH; i++) {
    if((i + SEGMENT_RUNTIME.counter_mode_step) % 4 < 2) {
      if (color1 == SEGMENT.colors[0])
      {
        setPixelColor(SEGMENT.stop - i, color_from_palette(SEGMENT.stop - i, true, PALETTE_SOLID_WRAP, 0));
      } else
      {
        setPixelColor(SEGMENT.stop - i, color1);
      }
    } else {
      setPixelColor(SEGMENT.stop - i, color2);
    }
  }

  if (SEGMENT.speed != 0) SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) & 0x3;
  return  35 + ((350 * (uint32_t)(255 - SEGMENT.speed)) / 255);
}

/*
 * Alternating color/sec pixels running.
 */
uint16_t WS2812FX::mode_running_color(void) {
  return running(SEGMENT.colors[0], SEGMENT.colors[1]);
}


/*
 * Alternating red/blue pixels running.
 */
uint16_t WS2812FX::mode_running_red_blue(void) {
  return running(RED, BLUE);
}


/*
 * Alternating red/green pixels running.
 */
uint16_t WS2812FX::mode_merry_christmas(void) {
  return running(RED, GREEN);
}


/*
 * Alternating orange/purple pixels running.
 */
uint16_t WS2812FX::mode_halloween(void) {
  return running(PURPLE, ORANGE);
}


/*
 * Random colored pixels running.
 */
uint16_t WS2812FX::mode_running_random(void) {
  for(uint16_t i=SEGMENT_LENGTH-1; i > 0; i--) {
    setPixelColor(SEGMENT.start + i, getPixelColor(SEGMENT.start + i - 1));
  }

  if(SEGMENT_RUNTIME.counter_mode_step == 0) {
    SEGMENT_RUNTIME.aux_param = get_random_wheel_index(SEGMENT_RUNTIME.aux_param);
    setPixelColor(SEGMENT.start, color_wheel(SEGMENT_RUNTIME.aux_param));
  }

  SEGMENT_RUNTIME.counter_mode_step++;
  if (SEGMENT_RUNTIME.counter_mode_step > ((255-SEGMENT.intensity) >> 4))
  {
    SEGMENT_RUNTIME.counter_mode_step = 0;
  }
  return SPEED_FORMULA_L;
}


/*
 * K.I.T.T.
 */
uint16_t WS2812FX::mode_larson_scanner(void) {
  fade_out(SEGMENT.intensity);

  uint16_t index = 0;
  if(SEGMENT_RUNTIME.counter_mode_step < SEGMENT_LENGTH) {
    index = SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step;
  } else {
    index = SEGMENT.start + ((SEGMENT_LENGTH * 2) - SEGMENT_RUNTIME.counter_mode_step) - 2;
  }
  setPixelColor(index, color_from_palette(index, true, PALETTE_SOLID_WRAP, 0));

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % ((SEGMENT_LENGTH * 2) - 2);
  return SPEED_FORMULA_L;
}


/*
 * Firing comets from one end.
 */
uint16_t WS2812FX::mode_comet(void) {
  fade_out(SEGMENT.intensity);

  uint16_t index = SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step;
  setPixelColor(index, color_from_palette(index, true, PALETTE_SOLID_WRAP, 0));

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
  return SPEED_FORMULA_L;
}


/*
 * Fireworks function.
 */
uint16_t WS2812FX::mode_fireworks() {
  fade_out(0);
  if (SEGMENT_RUNTIME.counter_mode_call == 0) {
    SEGMENT_RUNTIME.aux_param = UINT16_MAX;
    SEGMENT_RUNTIME.aux_param2 = UINT16_MAX;
  }
  bool valid1 = (SEGMENT_RUNTIME.aux_param  <= SEGMENT.stop && SEGMENT_RUNTIME.aux_param  >= SEGMENT.start);
  bool valid2 = (SEGMENT_RUNTIME.aux_param2 <= SEGMENT.stop && SEGMENT_RUNTIME.aux_param2 >= SEGMENT.start);
  uint32_t sv1 = 0, sv2 = 0;
  if (valid1) sv1 = getPixelColor(SEGMENT_RUNTIME.aux_param);
  if (valid2) sv2 = getPixelColor(SEGMENT_RUNTIME.aux_param2);
  blur(255-SEGMENT.speed);
  if (valid1) setPixelColor(SEGMENT_RUNTIME.aux_param , sv1);
  if (valid2) setPixelColor(SEGMENT_RUNTIME.aux_param2, sv2);

  for(uint16_t i=0; i<max(1, SEGMENT_LENGTH/20); i++) {
    if(random8(129 - (SEGMENT.intensity >> 1)) == 0) {
      uint16_t index = SEGMENT.start + random(SEGMENT_LENGTH);
      setPixelColor(index, color_from_palette(random8(), false, false, 0));
      SEGMENT_RUNTIME.aux_param2 = SEGMENT_RUNTIME.aux_param;
      SEGMENT_RUNTIME.aux_param = index;
    }
  }
  return 22;
}


//Twinkling LEDs running. Inspired by https://github.com/kitesurfer1404/WS2812FX/blob/master/src/custom/Rain.h
uint16_t WS2812FX::mode_rain()
{
  SEGMENT_RUNTIME.counter_mode_step += 22;
  if (SEGMENT_RUNTIME.counter_mode_step > SPEED_FORMULA_L) {
    SEGMENT_RUNTIME.counter_mode_step = 0;
    //shift all leds right
    uint32_t ctemp = getPixelColor(SEGMENT.stop);
    for(uint16_t i=SEGMENT.stop; i>SEGMENT.start; i--) {
      setPixelColor(i, getPixelColor(i-1));
    }
    setPixelColor(SEGMENT.start, ctemp);
    SEGMENT_RUNTIME.aux_param++;
    SEGMENT_RUNTIME.aux_param2++;
    if (SEGMENT_RUNTIME.aux_param == 0) SEGMENT_RUNTIME.aux_param = UINT16_MAX;
    if (SEGMENT_RUNTIME.aux_param2 == 0) SEGMENT_RUNTIME.aux_param = UINT16_MAX;
    if (SEGMENT_RUNTIME.aux_param == SEGMENT.stop +1) SEGMENT_RUNTIME.aux_param = SEGMENT.start;
    if (SEGMENT_RUNTIME.aux_param2 == SEGMENT.stop +1) SEGMENT_RUNTIME.aux_param2 = SEGMENT.start;
  }
  return mode_fireworks();
}


/*
 * Fire flicker function
 */
uint16_t WS2812FX::mode_fire_flicker(void) {
  byte w = (SEGMENT.colors[0] >> 24) & 0xFF;
  byte r = (SEGMENT.colors[0] >> 16) & 0xFF;
  byte g = (SEGMENT.colors[0] >>  8) & 0xFF;
  byte b = (SEGMENT.colors[0]        & 0xFF);
  byte lum = (SEGMENT.palette == 0) ? max(w, max(r, max(g, b))) : 255;
  lum /= (((256-SEGMENT.intensity)/16)+1);
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    byte flicker = random8(lum);
    if (SEGMENT.palette == 0) {
      setPixelColor(i, max(r - flicker, 0), max(g - flicker, 0), max(b - flicker, 0), max(w - flicker, 0));
    } else {
      setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0, 255 - flicker));
    }
  }
  return 10 + (2 * (uint16_t)(255 - SEGMENT.speed));
}


/*
 * Gradient run base function
 */
uint16_t WS2812FX::gradient_base(bool loading) {
  if (SEGMENT_RUNTIME.counter_mode_call == 0) SEGMENT_RUNTIME.counter_mode_step = 0;
  float per,val; //0.0 = sec 1.0 = pri
  float brd = SEGMENT.intensity;
  if (!loading) brd = SEGMENT.intensity/2; 
  if (brd <1.0) brd = 1.0;
  int pp = SEGMENT_RUNTIME.counter_mode_step;
  int p1 = pp-SEGMENT_LENGTH;
  int p2 = pp+SEGMENT_LENGTH;
  
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++)
  {
    if (loading)
    {
      val = abs(((i>pp) ? p2:pp) -i);
    } else {
      val = min(abs(pp-i),min(abs(p1-i),abs(p2-i)));
    }
    per = val/brd;
    if (per >1.0) per = 1.0;
    setPixelColor(SEGMENT.start + i, color_blend(SEGMENT.colors[0], color_from_palette(SEGMENT.start + i, true, PALETTE_SOLID_WRAP, 1), per*255));
  }
  
  SEGMENT_RUNTIME.counter_mode_step++;
  if (SEGMENT_RUNTIME.counter_mode_step > SEGMENT.stop) SEGMENT_RUNTIME.counter_mode_step = SEGMENT.start;
  if (SEGMENT.speed == 0) SEGMENT_RUNTIME.counter_mode_step = SEGMENT.start + (SEGMENT_LENGTH >> 1);
  return SPEED_FORMULA_L;
}


/*
 * Gradient run
 */
uint16_t WS2812FX::mode_gradient(void) {
  return gradient_base(false);
}


/*
 * Gradient run with hard transition
 */
uint16_t WS2812FX::mode_loading(void) {
  return gradient_base(true);
}


/*  
 * Lights all LEDs after each other up starting from the outer edges and  
 * finishing in the middle. Then turns them in reverse order off. Repeat. 
 */ 
uint16_t WS2812FX::mode_dual_color_wipe_in_out(void) {  
  int end = SEGMENT_LENGTH - SEGMENT_RUNTIME.counter_mode_step - 1; 
  bool odd = (SEGMENT_LENGTH % 2) == 1; 
  int mid = odd ? ((SEGMENT_LENGTH / 2) + 1) : (SEGMENT_LENGTH / 2);  
  if (SEGMENT_RUNTIME.counter_mode_step < mid) {
    byte pindex = map(SEGMENT_RUNTIME.counter_mode_step, 0, mid -1, 0, 255);
    uint32_t col = color_from_palette(pindex, false, false, 0);
    
    setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step, col); 
    setPixelColor(SEGMENT.start + end, col); 
  } else {  
    if (odd) {  
      // If odd, we need to 'double count' the center LED (once to turn it on,  
      // once to turn it off). So trail one behind after the middle LED.  
      setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step - 1, SEGMENT.colors[1]); 
      setPixelColor(SEGMENT.start + end + 1, SEGMENT.colors[1]); 
    } else {  
      setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[1]); 
      setPixelColor(SEGMENT.start + end, SEGMENT.colors[1]); 
    } 
  } 
   SEGMENT_RUNTIME.counter_mode_step++; 
  if (odd) {  
    if (SEGMENT_RUNTIME.counter_mode_step > SEGMENT_LENGTH) { 
      SEGMENT_RUNTIME.counter_mode_step = 0;  
    } 
  } else {  
    if (SEGMENT_RUNTIME.counter_mode_step >= SEGMENT_LENGTH) {  
      SEGMENT_RUNTIME.counter_mode_step = 0;  
    } 
  } 
  return SPEED_FORMULA_L;
}


/* 
* Lights all LEDs after each other up starting from the outer edges and  
* finishing in the middle. Then turns them in that order off. Repeat.  
*/ 
uint16_t WS2812FX::mode_dual_color_wipe_in_in(void) { 
  bool odd = (SEGMENT_LENGTH % 2) == 1; 
  int mid = SEGMENT_LENGTH / 2;
  byte pindex = 0;
  uint32_t col = 0;
  if (SEGMENT_RUNTIME.counter_mode_step <= mid)
  {
    pindex = map(SEGMENT_RUNTIME.counter_mode_step, 0, mid, 0, 255);
    col = color_from_palette(pindex, false, false, 0);
  }
  if (odd) {  
    if (SEGMENT_RUNTIME.counter_mode_step <= mid) { 
      setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step, col); 
      setPixelColor(SEGMENT.start + SEGMENT_LENGTH - SEGMENT_RUNTIME.counter_mode_step - 1, col);  
    } else {  
      int i = SEGMENT_RUNTIME.counter_mode_step - mid;  
      setPixelColor(SEGMENT.start + i - 1, SEGMENT.colors[1]); 
      setPixelColor(SEGMENT.start + SEGMENT_LENGTH - i, SEGMENT.colors[1]);  
    } 
  } else {  
    if (SEGMENT_RUNTIME.counter_mode_step < mid) {  
      setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step, col); 
      setPixelColor(SEGMENT.start + SEGMENT_LENGTH - SEGMENT_RUNTIME.counter_mode_step - 1, col);  
    } else {  
      int i = SEGMENT_RUNTIME.counter_mode_step - mid;  
      setPixelColor(SEGMENT.start + i, SEGMENT.colors[1]); 
      setPixelColor(SEGMENT.start + SEGMENT_LENGTH - i - 1, SEGMENT.colors[1]);  
    } 
  } 
   SEGMENT_RUNTIME.counter_mode_step++; 
  if (odd) {  
    if (SEGMENT_RUNTIME.counter_mode_step > SEGMENT_LENGTH) { 
      SEGMENT_RUNTIME.counter_mode_step = 0;  
    } 
  } else {  
    if (SEGMENT_RUNTIME.counter_mode_step >= SEGMENT_LENGTH) {  
      SEGMENT_RUNTIME.counter_mode_step = 0;  
    } 
  } 
  return SPEED_FORMULA_L;
}


/* 
* Lights all LEDs after each other up starting from the middle and 
* finishing at the edges. Then turns them off in that order. Repeat. 
*/ 
uint16_t WS2812FX::mode_dual_color_wipe_out_out(void) { 
  int end = SEGMENT_LENGTH - SEGMENT_RUNTIME.counter_mode_step - 1; 
  bool odd = (SEGMENT_LENGTH % 2) == 1; 
  int mid = SEGMENT_LENGTH / 2;
  byte pindex = 0;
  uint32_t col = 0;
  if (SEGMENT_RUNTIME.counter_mode_step <= mid)
  {
    pindex = map(SEGMENT_RUNTIME.counter_mode_step, 0, mid, 255, 0);
    col = color_from_palette(pindex, false, false, 0);
  }
  if (odd) { 
    if (SEGMENT_RUNTIME.counter_mode_step <= mid) { 
      setPixelColor(SEGMENT.start + mid + SEGMENT_RUNTIME.counter_mode_step, col); 
      setPixelColor(SEGMENT.start + mid - SEGMENT_RUNTIME.counter_mode_step, col); 
    } else {  
      setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step - 1, SEGMENT.colors[1]); 
      setPixelColor(SEGMENT.start + end + 1, SEGMENT.colors[1]); 
    } 
  } else {  
    if (SEGMENT_RUNTIME.counter_mode_step < mid) {  
      setPixelColor(SEGMENT.start + mid - SEGMENT_RUNTIME.counter_mode_step - 1, col); 
      setPixelColor(SEGMENT.start + mid + SEGMENT_RUNTIME.counter_mode_step, col); 
    } else {  
      setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[1]); 
      setPixelColor(SEGMENT.start + end, SEGMENT.colors[1]); 
    } 
  } 
   SEGMENT_RUNTIME.counter_mode_step++; 
  if (odd) {  
    if (SEGMENT_RUNTIME.counter_mode_step > SEGMENT_LENGTH) { 
      SEGMENT_RUNTIME.counter_mode_step = 0;  
    } 
  } else {  
    if (SEGMENT_RUNTIME.counter_mode_step >= SEGMENT_LENGTH) {  
      SEGMENT_RUNTIME.counter_mode_step = 0;  
    } 
  } 
  return SPEED_FORMULA_L;
} 


/* 
* Lights all LEDs after each other up starting from the middle and 
* finishing at the edges. Then turns them off in reverse order. Repeat.  
*/ 
uint16_t WS2812FX::mode_dual_color_wipe_out_in(void) {  
  bool odd = (SEGMENT_LENGTH % 2) == 1; 
  int mid = SEGMENT_LENGTH / 2;
  byte pindex = 0;
  uint32_t col = 0;
  if (SEGMENT_RUNTIME.counter_mode_step <= mid)
  {
    pindex = map(SEGMENT_RUNTIME.counter_mode_step, 0, mid, 255, 0);
    col = color_from_palette(pindex, false, false, 0);
  }
   if (odd) { 
    if (SEGMENT_RUNTIME.counter_mode_step <= mid) { 
      setPixelColor(SEGMENT.start + mid + SEGMENT_RUNTIME.counter_mode_step, col); 
      setPixelColor(SEGMENT.start + mid - SEGMENT_RUNTIME.counter_mode_step, col); 
    } else {  
      int i = SEGMENT_RUNTIME.counter_mode_step - mid;  
      setPixelColor(SEGMENT.start + i - 1, SEGMENT.colors[1]); 
      setPixelColor(SEGMENT.start + SEGMENT_LENGTH - i, SEGMENT.colors[1]);  
    } 
  } else {  
    if (SEGMENT_RUNTIME.counter_mode_step < mid) {  
      setPixelColor(SEGMENT.start + mid - SEGMENT_RUNTIME.counter_mode_step - 1, col); 
      setPixelColor(SEGMENT.start + mid + SEGMENT_RUNTIME.counter_mode_step, col); 
    } else {  
      int i = SEGMENT_RUNTIME.counter_mode_step - mid;  
      setPixelColor(SEGMENT.start + i, SEGMENT.colors[1]); 
      setPixelColor(SEGMENT.start + SEGMENT_LENGTH - i - 1, SEGMENT.colors[1]);  
    } 
  } 
   SEGMENT_RUNTIME.counter_mode_step++; 
  if (odd) {  
    if (SEGMENT_RUNTIME.counter_mode_step > SEGMENT_LENGTH) { 
      SEGMENT_RUNTIME.counter_mode_step = 0;  
    } 
  } else {  
    if (SEGMENT_RUNTIME.counter_mode_step >= SEGMENT_LENGTH) {  
      SEGMENT_RUNTIME.counter_mode_step = 0;  
    } 
  } 
  return SPEED_FORMULA_L;
}


/*
 * Tricolor chase function
 */
uint16_t WS2812FX::tricolor_chase(uint32_t color1, uint32_t color2) {
  uint16_t index = SEGMENT_RUNTIME.counter_mode_step % 6;
  for(uint16_t i=0; i < SEGMENT_LENGTH; i++, index++) {
    if(index > 5) index = 0;

    uint32_t color = color1;
    if(index > 3) color = color_from_palette(i, true, PALETTE_SOLID_WRAP, 1);
    else if(index > 1) color = color2;

    setPixelColor(SEGMENT.stop - i, color);
  }

  SEGMENT_RUNTIME.counter_mode_step++;
  return  35 + ((350 * (uint32_t)(255 - SEGMENT.speed)) / 255);
}


/*
 * Alternating white/red/black pixels running. PLACEHOLDER
 */
uint16_t WS2812FX::mode_circus_combustus(void) {
  return tricolor_chase(RED, WHITE);
}


/*
 * Tricolor chase mode
 */
uint16_t WS2812FX::mode_tricolor_chase(void) {
  return tricolor_chase(SEGMENT.colors[2], SEGMENT.colors[0]);
}


/*
 * ICU mode
 */
uint16_t WS2812FX::mode_icu(void) {
  uint16_t dest = SEGMENT_RUNTIME.counter_mode_step & 0xFFFF;

  fill(SEGMENT.colors[1]);

  byte pindex = map(dest, 0, SEGMENT_LENGTH/2, 0, 255);
  uint32_t col = color_from_palette(pindex, false, false, 0);
 
  setPixelColor(SEGMENT.start + dest, col);
  setPixelColor(SEGMENT.start + dest + SEGMENT_LENGTH/2, col);

  if(SEGMENT_RUNTIME.aux_param == dest) { // pause between eye movements
    if(random8(6) == 0) { // blink once in a while
      setPixelColor(SEGMENT.start + dest, SEGMENT.colors[1]);
      setPixelColor(SEGMENT.start + dest + SEGMENT_LENGTH/2, SEGMENT.colors[1]);
      return 200;
    }
    SEGMENT_RUNTIME.aux_param = random16(SEGMENT_LENGTH/2);
    return 1000 + random16(2000);
  }

  if(SEGMENT_RUNTIME.aux_param > SEGMENT_RUNTIME.counter_mode_step) {
    SEGMENT_RUNTIME.counter_mode_step++;
    dest++;
  } else if (SEGMENT_RUNTIME.aux_param < SEGMENT_RUNTIME.counter_mode_step) {
    SEGMENT_RUNTIME.counter_mode_step--;
    dest--;
  }

  setPixelColor(SEGMENT.start + dest, col);
  setPixelColor(SEGMENT.start + dest + SEGMENT_LENGTH/2, col);

  return SPEED_FORMULA_L;
}


/*
 * Custom mode by Aircoookie. Color Wipe, but with 3 colors
 */
uint16_t WS2812FX::mode_tricolor_wipe(void)
{
  if(SEGMENT_RUNTIME.counter_mode_step < SEGMENT_LENGTH) {
    uint32_t led_offset = SEGMENT_RUNTIME.counter_mode_step;
    setPixelColor(SEGMENT.start + led_offset, SEGMENT.colors[0]);
  } else if (SEGMENT_RUNTIME.counter_mode_step < SEGMENT_LENGTH*2) {
    uint32_t led_offset = SEGMENT_RUNTIME.counter_mode_step - SEGMENT_LENGTH;
    setPixelColor(SEGMENT.start + led_offset, SEGMENT.colors[1]);
  } else
  {
    uint32_t led_offset = SEGMENT_RUNTIME.counter_mode_step - SEGMENT_LENGTH*2;
    setPixelColor(SEGMENT.start + led_offset, color_from_palette(SEGMENT.start + led_offset, true, PALETTE_SOLID_WRAP, 2));
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % (SEGMENT_LENGTH * 3);
  return SPEED_FORMULA_L;
}


/*
 * Fades between 3 colors
 * Custom mode by Keith Lord: https://github.com/kitesurfer1404/WS2812FX/blob/master/src/custom/TriFade.h
 * Modified by Aircoookie
 */
uint16_t WS2812FX::mode_tricolor_fade(void)
{
  uint32_t color1 = 0, color2 = 0;
  byte stage = 0;

  if(SEGMENT_RUNTIME.counter_mode_step < 256) {
    color1 = SEGMENT.colors[0];
    color2 = SEGMENT.colors[1];
    stage = 0;
  } else if(SEGMENT_RUNTIME.counter_mode_step < 512) {
    color1 = SEGMENT.colors[1];
    color2 = SEGMENT.colors[2];
    stage = 1;
  } else {
    color1 = SEGMENT.colors[2];
    color2 = SEGMENT.colors[0];
    stage = 2;
  }

  byte stp = SEGMENT_RUNTIME.counter_mode_step % 256;
  uint32_t color = 0;
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    if (stage == 2) {
      color = color_blend(color_from_palette(i, true, PALETTE_SOLID_WRAP, 2), color2, stp); 
    } else if (stage == 1) {
      color = color_blend(color1, color_from_palette(i, true, PALETTE_SOLID_WRAP, 2), stp); 
    } else {
      color = color_blend(color1, color2, stp);
    }
    setPixelColor(i, color);
  }

  SEGMENT_RUNTIME.counter_mode_step += 4;
  if(SEGMENT_RUNTIME.counter_mode_step >= 768) SEGMENT_RUNTIME.counter_mode_step = 0;

  return 5 + ((uint32_t)(255 - SEGMENT.speed) / 10);
}


/*
 * Creates random comets
 * Custom mode by Keith Lord: https://github.com/kitesurfer1404/WS2812FX/blob/master/src/custom/MultiComet.h
 */
uint16_t WS2812FX::mode_multi_comet(void)
{
  fade_out(SEGMENT.intensity);

  static uint16_t comets[] = {UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX};

  for(uint8_t i=0; i < 8; i++) {
    if(comets[i] < SEGMENT_LENGTH) {
      uint16_t index = SEGMENT.start + comets[i];
      if (SEGMENT.colors[2] != 0)
      {
        setPixelColor(index, i % 2 ? color_from_palette(index, true, PALETTE_SOLID_WRAP, 0) : SEGMENT.colors[2]);
      } else
      {
        setPixelColor(index, color_from_palette(index, true, PALETTE_SOLID_WRAP, 0));
      }
      comets[i]++;
    } else {
      if(!random(SEGMENT_LENGTH)) {
        comets[i] = 0;
      }
    }
  }
  return SPEED_FORMULA_L;
}


/*
 * Creates two Larson scanners moving in opposite directions
 * Custom mode by Keith Lord: https://github.com/kitesurfer1404/WS2812FX/blob/master/src/custom/DualLarson.h
 */
uint16_t WS2812FX::mode_dual_larson_scanner(void){
  if (SEGMENT_RUNTIME.aux_param)
  {
    SEGMENT_RUNTIME.counter_mode_step--;
  } else
  {
    SEGMENT_RUNTIME.counter_mode_step++;
  }

  fade_out(SEGMENT.intensity);

  uint16_t index = SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step;
  setPixelColor(index, color_from_palette(index, true, PALETTE_SOLID_WRAP, 0));
  index = SEGMENT.stop - SEGMENT_RUNTIME.counter_mode_step;
  if (SEGMENT.colors[2] != 0)
  {
    setPixelColor(index, SEGMENT.colors[2]);
  } else
  {
    setPixelColor(index, color_from_palette(index, true, PALETTE_SOLID_WRAP, 0));
  }

  if(SEGMENT_RUNTIME.counter_mode_step >= (SEGMENT.stop - SEGMENT.start) || SEGMENT_RUNTIME.counter_mode_step <= 0)
  SEGMENT_RUNTIME.aux_param = !SEGMENT_RUNTIME.aux_param;
  
  return SPEED_FORMULA_L;
}


/*
 * Running random pixels
 * Custom mode by Keith Lord: https://github.com/kitesurfer1404/WS2812FX/blob/master/src/custom/RandomChase.h
 */
uint16_t WS2812FX::mode_random_chase(void)
{
  for(uint16_t i=SEGMENT.stop; i>SEGMENT.start; i--) {
    setPixelColor(i, getPixelColor(i-1));
  }
  uint32_t color = getPixelColor(SEGMENT.start + 1);
  uint8_t r = random8(6) != 0 ? (color >> 16 & 0xFF) : random8();
  uint8_t g = random8(6) != 0 ? (color >> 8  & 0xFF) : random8();
  uint8_t b = random8(6) != 0 ? (color       & 0xFF) : random8();
  setPixelColor(SEGMENT.start, r, g, b);

  return SPEED_FORMULA_L;
}


typedef struct Oscillator {
  int16_t pos;
  int8_t  size;
  int8_t  dir;
  int8_t  speed;
} oscillator;

uint16_t WS2812FX::mode_oscillate(void)
{
  static oscillator oscillators[2] = {
    {SEGMENT_LENGTH/4,   SEGMENT_LENGTH/8,  1, 1},
    {SEGMENT_LENGTH/4*2, SEGMENT_LENGTH/8, -1, 1}
    //{SEGMENT_LENGTH/4*3, SEGMENT_LENGTH/8,  1, 2}
  };

  for(int8_t i=0; i < sizeof(oscillators)/sizeof(oscillators[0]); i++) {
    oscillators[i].pos += oscillators[i].dir * oscillators[i].speed;
    if((oscillators[i].dir == -1) && (oscillators[i].pos <= 0)) {
      oscillators[i].pos = 0;
      oscillators[i].dir = 1;
      oscillators[i].speed = random8(1, 3);
    }
    if((oscillators[i].dir == 1) && (oscillators[i].pos >= (SEGMENT_LENGTH - 1))) {
      oscillators[i].pos = SEGMENT_LENGTH - 1;
      oscillators[i].dir = -1;
      oscillators[i].speed = random8(1, 3);
    }
  }

  for(int16_t i=0; i < SEGMENT_LENGTH; i++) {
    uint32_t color = BLACK;
    for(int8_t j=0; j < sizeof(oscillators)/sizeof(oscillators[0]); j++) {
      if(i >= oscillators[j].pos - oscillators[j].size && i <= oscillators[j].pos + oscillators[j].size) {
        color = (color == BLACK) ? SEGMENT.colors[j] : color_blend(color, SEGMENT.colors[j], 128);
      }
    }
    setPixelColor(SEGMENT.start + i, color);
  }
  return 15 + (uint32_t)(255 - SEGMENT.speed);
}


uint16_t WS2812FX::mode_lightning(void)
{
  uint16_t ledstart = SEGMENT.start + random8(SEGMENT_LENGTH);                               // Determine starting location of flash
  uint16_t ledlen = random8(SEGMENT.stop - ledstart);                      // Determine length of flash (not to go beyond NUM_LEDS-1)
  uint8_t bri = 255/random8(1, 3);   

  if (SEGMENT_RUNTIME.counter_mode_step == 0)
  {
    SEGMENT_RUNTIME.aux_param = random8(3, 3 + SEGMENT.intensity/20); //number of flashes
    bri = 52; 
    SEGMENT_RUNTIME.aux_param2 = 1;
  }

  fill(SEGMENT.colors[1]);
  
  if (SEGMENT_RUNTIME.aux_param2) {
    for (int i = ledstart; i < ledstart + ledlen; i++)
    {
      if (SEGMENT.palette == 0)
      {
        setPixelColor(i,bri,bri,bri,bri);
      } else {
        setPixelColor(i,color_from_palette(i, true, PALETTE_SOLID_WRAP, 0, bri));
      }
    }
    SEGMENT_RUNTIME.aux_param2 = 0;
    SEGMENT_RUNTIME.counter_mode_step++;
    return random8(4, 10);                                    // each flash only lasts 4-10 milliseconds
  }

  SEGMENT_RUNTIME.aux_param2 = 1;
  if (SEGMENT_RUNTIME.counter_mode_step == 1) return (200);                       // longer delay until next flash after the leader

  if (SEGMENT_RUNTIME.counter_mode_step <= SEGMENT_RUNTIME.aux_param) return (50 + random8(100));  // shorter delay between strokes

  SEGMENT_RUNTIME.counter_mode_step = 0;
  return (random8(255 - SEGMENT.speed) * 100);                            // delay between strikes
}


// Pride2015
// Animated, ever-changing rainbows.
// by Mark Kriegsman: https://gist.github.com/kriegsman/964de772d64c502760e5
uint16_t WS2812FX::mode_pride_2015(void)
{
  uint16_t duration = 10 + SEGMENT.speed;
  uint16_t sPseudotime = SEGMENT_RUNTIME.counter_mode_step;
  uint16_t sHue16 = SEGMENT_RUNTIME.aux_param;
 
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
  
  for( uint16_t i = SEGMENT.start ; i <= SEGMENT.stop; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 >> 8;

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);
    
    CRGB newcolor = CHSV( hue8, sat8, bri8);
    fastled_col = fastled_from_col(getPixelColor(i));
    
    nblend( fastled_col, newcolor, 64);
    setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
  }
  SEGMENT_RUNTIME.counter_mode_step = sPseudotime;
  SEGMENT_RUNTIME.aux_param = sHue16;
  return 20;
}


//eight colored dots, weaving in and out of sync with each other
uint16_t WS2812FX::mode_juggle(void){
  fade_out(SEGMENT.intensity);
  CRGB fastled_col;
  byte dothue = 0;
  for ( byte i = 0; i < 8; i++) {
    uint16_t index = SEGMENT.start + beatsin16(i + 7, 0, SEGMENT_LENGTH -1);
    fastled_col = fastled_from_col(getPixelColor(index));
    fastled_col |= (SEGMENT.palette==0)?CHSV(dothue, 220, 255):ColorFromPalette(currentPalette, dothue, 255);
    setPixelColor(index, fastled_col.red, fastled_col.green, fastled_col.blue);
    dothue += 32;
  }
  return 10 + (uint16_t)(255 - SEGMENT.speed)/4;
}


uint16_t WS2812FX::mode_palette(void)
{
  bool noWrap = (paletteBlend == 2 || (paletteBlend == 0 && SEGMENT.speed == 0));
  for (uint16_t i = SEGMENT.start; i <= SEGMENT.stop; i++)
  {
    uint8_t colorIndex = map(i,SEGMENT.start,SEGMENT.stop,0,255) - (SEGMENT_RUNTIME.counter_mode_step >> 6 & 0xFF);
    
    if (noWrap) colorIndex = map(colorIndex, 0, 255, 0, 240); //cut off blend at palette "end"
    
    setPixelColor(i, color_from_palette(colorIndex, false, true, 255));
  }
  SEGMENT_RUNTIME.counter_mode_step += SEGMENT.speed;
  if (SEGMENT.speed == 0) SEGMENT_RUNTIME.counter_mode_step = 0;
  return 20;
}


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
// This simulation scales it self a bit depending on NUM_LEDS; it should look
// "OK" on anywhere from 20 to 100 LEDs without too much tweaking. 
//
// I recommend running this simulation at anywhere from 30-100 frames per second,
// meaning an interframe delay of about 10-35 milliseconds.
//
// Looks best on a high-density LED setup (60+ pixels/meter).
//
//
// There are two main parameters you can play with to control the look and
// feel of your fire: COOLING (used in step 1 above), and SPARKING (used
// in step 3 above) (Effect Intensity = Sparking).
//
// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100 
#define COOLING  75

uint16_t WS2812FX::mode_fire_2012(void)
{
  // Step 1.  Cool down every cell a little
  for( int i = SEGMENT.start; i <= SEGMENT.stop; i++) {
    _locked[i] = qsub8(_locked[i],  random8(0, ((COOLING * 10) / SEGMENT_LENGTH) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for( int k= SEGMENT.stop; k >= SEGMENT.start + 2; k--) {
    _locked[k] = (_locked[k - 1] + _locked[k - 2] + _locked[k - 2] ) / 3;
  }
  
  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if( random8() <= SEGMENT.intensity ) {
    int y = SEGMENT.start + random8(7);
    if (y <= SEGMENT.stop) _locked[y] = qadd8(_locked[y], random8(160,255) );
  }

  // Step 4.  Map from heat cells to LED colors
  for( int j = SEGMENT.start; j <= SEGMENT.stop; j++) {
    CRGB color = ColorFromPalette( currentPalette, min(_locked[j],240), 255, LINEARBLEND);
    setPixelColor(j, color.red, color.green, color.blue);
  }
  return 10 + (uint16_t)(255 - SEGMENT.speed)/6;
}


// ColorWavesWithPalettes by Mark Kriegsman: https://gist.github.com/kriegsman/8281905786e8b2632aeb
// This function draws color waves with an ever-changing,
// widely-varying set of parameters, using a color palette.
uint16_t WS2812FX::mode_colorwaves(void)
{
  uint16_t duration = 10 + SEGMENT.speed;
  uint16_t sPseudotime = SEGMENT_RUNTIME.counter_mode_step;
  uint16_t sHue16 = SEGMENT_RUNTIME.aux_param;

  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 300, 1500);

  sPseudotime += duration * msmultiplier;
  sHue16 += duration * beatsin88( 400, 5, 9);
  uint16_t brightnesstheta16 = sPseudotime;
  CRGB fastled_col;

  for ( uint16_t i = SEGMENT.start ; i <= SEGMENT.stop; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 >> 8;
    uint16_t h16_128 = hue16 >> 7;
    if ( h16_128 & 0x100) {
      hue8 = 255 - (h16_128 >> 1);
    } else {
      hue8 = h16_128 >> 1;
    }

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);

    CRGB newcolor = ColorFromPalette(currentPalette, hue8, bri8);
    fastled_col = fastled_from_col(getPixelColor(i));

    nblend(fastled_col, newcolor, 128);
    setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
  }
  SEGMENT_RUNTIME.counter_mode_step = sPseudotime;
  SEGMENT_RUNTIME.aux_param = sHue16;
  return 20;
}


// colored stripes pulsing at a defined Beats-Per-Minute (BPM)
uint16_t WS2812FX::mode_bpm(void)
{
  CRGB fastled_col;
  uint8_t beat = beatsin8(SEGMENT.speed, 64, 255);
  for (uint16_t i = SEGMENT.start; i <= SEGMENT.stop; i++) {
    fastled_col = ColorFromPalette(currentPalette, SEGMENT_RUNTIME.counter_mode_step + (i * 2), beat - SEGMENT_RUNTIME.counter_mode_step + (i * 10));
    setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
  }
  SEGMENT_RUNTIME.counter_mode_step++;
  if (SEGMENT_RUNTIME.counter_mode_step >= 255) SEGMENT_RUNTIME.counter_mode_step = 0;
  return 20;
}


uint16_t WS2812FX::mode_fillnoise8(void)
{
  if (SEGMENT_RUNTIME.counter_mode_call == 0) SEGMENT_RUNTIME.counter_mode_step = random16(12345);
  CRGB fastled_col;
  for (uint16_t i = SEGMENT.start; i <= SEGMENT.stop; i++) {
    uint8_t index = inoise8(i * SEGMENT_LENGTH, SEGMENT_RUNTIME.counter_mode_step + i * SEGMENT_LENGTH) % 255;
    fastled_col = ColorFromPalette(currentPalette, index, 255, LINEARBLEND);
    setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
  }
  SEGMENT_RUNTIME.counter_mode_step += beatsin8(SEGMENT.speed, 1, 6); //10,1,4

  return 20;
}

uint16_t WS2812FX::mode_noise16_1(void)
{
  uint16_t scale = 320;                                      // the "zoom factor" for the noise
  CRGB fastled_col;
  SEGMENT_RUNTIME.counter_mode_step += (1 + SEGMENT.speed/16);

  for (uint16_t i = SEGMENT.start; i <= SEGMENT.stop; i++) {

    uint16_t shift_x = beatsin8(11);                           // the x position of the noise field swings @ 17 bpm
    uint16_t shift_y = SEGMENT_RUNTIME.counter_mode_step/42;             // the y position becomes slowly incremented


    uint16_t real_x = (i + shift_x) * scale;                  // the x position of the noise field swings @ 17 bpm
    uint16_t real_y = (i + shift_y) * scale;                  // the y position becomes slowly incremented
    uint32_t real_z = SEGMENT_RUNTIME.counter_mode_step;                          // the z position becomes quickly incremented

    uint8_t noise = inoise16(real_x, real_y, real_z) >> 8;   // get the noise data and scale it down

    uint8_t index = sin8(noise * 3);                         // map LED color based on noise data

    fastled_col = ColorFromPalette(currentPalette, index, 255, LINEARBLEND);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.
    setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
  }

  return 20;
}


uint16_t WS2812FX::mode_noise16_2(void)
{
  uint16_t scale = 1000;                                       // the "zoom factor" for the noise
  CRGB fastled_col;
  SEGMENT_RUNTIME.counter_mode_step += (1 + SEGMENT.speed);

  for (uint16_t i = SEGMENT.start; i <= SEGMENT.stop; i++) {

    uint16_t shift_x = SEGMENT_RUNTIME.counter_mode_step >> 6;                         // x as a function of time
    uint16_t shift_y = SEGMENT_RUNTIME.counter_mode_step/42;

    uint32_t real_x = (i + shift_x) * scale;                  // calculate the coordinates within the noise field

    uint8_t noise = inoise16(real_x, 0, 4223) >> 8;    // get the noise data and scale it down

    uint8_t index = sin8(noise * 3);                          // map led color based on noise data

    fastled_col = ColorFromPalette(currentPalette, index, noise, LINEARBLEND);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.
    setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
  }

  return 20;
}


uint16_t WS2812FX::mode_noise16_3(void)
{
  uint16_t scale = 800;                                       // the "zoom factor" for the noise
  CRGB fastled_col;
  SEGMENT_RUNTIME.counter_mode_step += (1 + SEGMENT.speed);

  for (uint16_t i = SEGMENT.start; i <= SEGMENT.stop; i++) {

    uint16_t shift_x = 4223;                                  // no movement along x and y
    uint16_t shift_y = 1234;

    uint32_t real_x = (i + shift_x) * scale;                  // calculate the coordinates within the noise field
    uint32_t real_y = (i + shift_y) * scale;                  // based on the precalculated positions
    uint32_t real_z = SEGMENT_RUNTIME.counter_mode_step*8;  

    uint8_t noise = inoise16(real_x, real_y, real_z) >> 8;    // get the noise data and scale it down

    uint8_t index = sin8(noise * 3);                          // map led color based on noise data

    fastled_col = ColorFromPalette(currentPalette, index, noise, LINEARBLEND);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.
    setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
  }

  return 20;
}


//https://github.com/aykevl/ledstrip-spark/blob/master/ledstrip.ino
uint16_t WS2812FX::mode_noise16_4(void)
{
  CRGB fastled_col;
  SEGMENT_RUNTIME.counter_mode_step += SEGMENT.speed;
  for (uint16_t i = SEGMENT.start; i <= SEGMENT.stop; i++) {
    int16_t index = inoise16(uint32_t(i - SEGMENT.start) << 12, SEGMENT_RUNTIME.counter_mode_step/8);
    fastled_col = ColorFromPalette(currentPalette, index);
    setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
  }
  return 20;
}


//based on https://gist.github.com/kriegsman/5408ecd397744ba0393e
uint16_t WS2812FX::mode_colortwinkle()
{
  CRGB fastled_col, prev;
  fract8 fadeUpAmount = 8 + (SEGMENT.speed/4), fadeDownAmount = 5 + (SEGMENT.speed/7);
  for( uint16_t i = SEGMENT.start; i <= SEGMENT.stop; i++) {
    fastled_col = fastled_from_col(getPixelColor(i));
    prev = fastled_col;
    if(_locked[i]) {  
      CRGB incrementalColor = fastled_col;
      incrementalColor.nscale8_video( fadeUpAmount);
      fastled_col += incrementalColor;

      if( fastled_col.red == 255 || fastled_col.green == 255 || fastled_col.blue == 255) {
        _locked[i] = false;
      }
      setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);

      if (fastled_from_col(getPixelColor(i)) == prev) //fix "stuck" pixels
      {
        fastled_col += fastled_col;
        setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
      }
    } else {
      fastled_col.nscale8( 255 - fadeDownAmount);
      setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
    }
  }

  for (uint16_t j = 0; j <= SEGMENT_LENGTH / 50; j++)
  {
    if ( random8() <= SEGMENT.intensity ) {
      for (uint8_t times = 0; times < 5; times++) //attempt to spawn a new pixel 5 times
      {
        int i = SEGMENT.start + random16(SEGMENT_LENGTH);
        if(getPixelColor(i) == 0) {
          fastled_col = ColorFromPalette(currentPalette, random8(), 64, NOBLEND);
          _locked[i] = true;
          setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
          break; //only spawn 1 new pixel per frame per 50 LEDs
        }
      }
    }
  }
  return 20;
}


//Calm effect, like a lake at night
uint16_t WS2812FX::mode_lake() {
  uint8_t sp = SEGMENT.speed/10;
  int wave1 = beatsin8(sp +2, -64,64);
  int wave2 = beatsin8(sp +1, -64,64);
  uint8_t wave3 = beatsin8(sp +2,   0,80);
  CRGB fastled_col;

  for (uint16_t i = SEGMENT.start; i <= SEGMENT.stop; i++)
  {
    int index = cos8((i*15)+ wave1)/2 + cubicwave8((i*23)+ wave2)/2;           
    uint8_t lum = (index > wave3) ? index - wave3 : 0;
    fastled_col = ColorFromPalette(currentPalette, map(index,0,255,0,240), lum, LINEARBLEND);
    setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
  }
  return 33;
}


// meteor effect
// send a meteor from begining to to the end of the strip with a trail that randomly decays.
// adapted from https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/#LEDStripEffectMeteorRain
uint16_t WS2812FX::mode_meteor() {
  byte meteorSize= 1+ SEGMENT_LENGTH / 10;
  uint16_t in = SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step;

  // fade all leds to colors[1] in LEDs one step
  for (uint16_t i = SEGMENT.start; i <= SEGMENT.stop; i++) {
    if (random8() <= 255 - SEGMENT.intensity)
    {
      byte meteorTrailDecay = 128 + random8(127);
      _locked[i] = scale8(_locked[i], meteorTrailDecay);
      setPixelColor(i, color_from_palette(_locked[i], false, true, 255));
    }
  }
  
  // draw meteor
  for(int j = 0; j < meteorSize; j++) {  
    uint16_t index = in + j;   
    if(in + j > SEGMENT.stop) {
      index = SEGMENT.start + (in + j - SEGMENT.stop) -1;
    }

    _locked[index] = 240;
    setPixelColor(index, color_from_palette(_locked[index], false, true, 255));
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % (SEGMENT_LENGTH);
  return SPEED_FORMULA_L;
}


// smooth meteor effect
// send a meteor from begining to to the end of the strip with a trail that randomly decays.
// adapted from https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/#LEDStripEffectMeteorRain
uint16_t WS2812FX::mode_meteor_smooth() {
  byte meteorSize= 1+ SEGMENT_LENGTH / 10;
  uint16_t in = map((SEGMENT_RUNTIME.counter_mode_step >> 6 & 0xFF), 0, 255, SEGMENT.start, SEGMENT.stop);

  // fade all leds to colors[1] in LEDs one step
  for (uint16_t i = SEGMENT.start; i <= SEGMENT.stop; i++) {
    if (_locked[i] != 0 && random8() <= 255 - SEGMENT.intensity)
    {
      int change = 3 - random8(12); //change each time between -8 and +3
      _locked[i] += change;
      if (_locked[i] > 245) _locked[i] = 0;
      if (_locked[i] > 240) _locked[i] = 240;
      setPixelColor(i, color_from_palette(_locked[i], false, true, 255));
    }
  }
  
  // draw meteor
  for(int j = 0; j < meteorSize; j++) {  
    uint16_t index = in + j;   
    if(in + j > SEGMENT.stop) {
      index = SEGMENT.start + (in + j - SEGMENT.stop) -1;
    }
    setPixelColor(index, color_blend(getPixelColor(index), color_from_palette(240, false, true, 255), 48));
    _locked[index] = 240;
  }

  SEGMENT_RUNTIME.counter_mode_step += SEGMENT.speed +1;
  return 20;
}


//Railway Crossing / Christmas Fairy lights
uint16_t WS2812FX::mode_railway()
{
  uint16_t dur = 40 + (255 - SEGMENT.speed) * 10;
  uint16_t rampdur = (dur * SEGMENT.intensity) >> 8;
  if (SEGMENT_RUNTIME.counter_mode_step > dur)
  {
    //reverse direction
    SEGMENT_RUNTIME.counter_mode_step = 0;
    SEGMENT_RUNTIME.aux_param = !SEGMENT_RUNTIME.aux_param;
  }
  uint8_t pos = 255;
  if (rampdur != 0)
  {
    uint16_t p0 = (SEGMENT_RUNTIME.counter_mode_step * 255) / rampdur;
    if (p0 < 255) pos = p0;
  }
  if (SEGMENT_RUNTIME.aux_param) pos = 255 - pos;
  for (uint16_t i = SEGMENT.start; i <= SEGMENT.stop; i += 2)
  {
    setPixelColor(i, color_from_palette(255 - pos, false, false, 255));
    if (i != SEGMENT.stop)
    {
      setPixelColor(i + 1, color_from_palette(pos, false, false, 255));
    }
  }
  SEGMENT_RUNTIME.counter_mode_step += 20;
  return 20;
}


//Water ripple
//propagation velocity from speed
//drop rate from intensity
uint16_t WS2812FX::mode_ripple()
{
  uint16_t maxripples = SEGMENT_LENGTH / 4;
  if (maxripples == 0) return mode_static();

  fill(SEGMENT.colors[1]);

  //draw wave
  for (uint16_t rippleI = 0; rippleI < maxripples; rippleI++)
  {
    uint16_t storeI = SEGMENT.start + 4*rippleI;
    uint16_t ripplestate = _locked[storeI];
    if (ripplestate)
    {
      uint8_t rippledecay = (SEGMENT.speed >> 4) +1; //faster decay if faster propagation
      uint16_t rippleorigin = (_locked[storeI+1] << 8) + _locked[storeI+2];
      uint32_t col = color_from_palette(_locked[storeI+3], false, false, 255);
      uint16_t propagation = ((ripplestate/rippledecay -1) * SEGMENT.speed);
      int16_t propI = propagation >> 8;
      uint8_t propF = propagation & 0xFF;
      int16_t left = rippleorigin - propI -1;
      uint8_t amp = (ripplestate < 17) ? triwave8((ripplestate-1)*8) : map(ripplestate,17,255,255,2);

      for (int16_t v = left; v < left +4; v++)
      {
        uint8_t mag = scale8(cubicwave8((propF>>2)+(v-left)*64), amp);
        if (v >= SEGMENT.start)
        {
          setPixelColor(v, color_blend(getPixelColor(v), col, mag));
        }
        int16_t w = left + propI*2 + 3 -(v-left);
        if (w <= SEGMENT.stop && w >= SEGMENT.start)
        {
          setPixelColor(w, color_blend(getPixelColor(w), col, mag));
        }
      }  
      ripplestate += rippledecay;
      _locked[storeI] = (ripplestate > 254) ? 0 : ripplestate;
    } else //randomly create new wave
    {
      if (random16(IBN + 10000) <= SEGMENT.intensity)
      {
        _locked[storeI] = 1;
        uint16_t origin = SEGMENT.start + random16(SEGMENT_LENGTH);
        _locked[storeI+1] = origin >> 8;
        _locked[storeI+2] = origin & 0xFF; 
        _locked[storeI+3] = random8();
      }
    }
  }
  return 20;
}
