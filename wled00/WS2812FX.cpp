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
  fill(SEGCOLOR(0));
  return (SEGMENT.getOption(7)) ? 20 : 500; //update faster if in transition
}


/*
 * Blink/strobe function
 * Alternate between color1 and color2
 * if(strobe == true) then create a strobe effect
 */
uint16_t WS2812FX::blink(uint32_t color1, uint32_t color2, bool strobe, bool do_palette) {
  uint32_t color = ((SEGENV.call & 1) == 0) ? color1 : color2;
  if (color == color1 && do_palette)
  {
    for(uint16_t i=SEGMENT.start; i < SEGMENT.stop; i++) {
      setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));
    }
  } else {
    fill(color);
  }

  if((SEGENV.call & 1) == 0) {
    return strobe ? 20 : (100 + ((1986 * (uint32_t)(255 - SEGMENT.speed)) / 255))*(float)(SEGMENT.intensity/128.0);
  } else {
    return strobe ? 50 + ((1986 * (uint32_t)(255 - SEGMENT.speed)) / 255) : (100 + ((1986 * (uint32_t)(255 - SEGMENT.speed)) / 255))*(float)(2.0-(SEGMENT.intensity/128.0));
  }
}


/*
 * Normal blinking. 50% on/off time.
 */
uint16_t WS2812FX::mode_blink(void) {
  return blink(SEGCOLOR(0), SEGCOLOR(1), false, true);
}


/*
 * Classic Blink effect. Cycling through the rainbow.
 */
uint16_t WS2812FX::mode_blink_rainbow(void) {
  return blink(color_wheel(SEGENV.call & 0xFF), SEGCOLOR(1), false, false);
}


/*
 * Classic Strobe effect.
 */
uint16_t WS2812FX::mode_strobe(void) {
  return blink(SEGCOLOR(0), SEGCOLOR(1), true, true);
}


/*
 * Classic Strobe effect. Cycling through the rainbow.
 */
uint16_t WS2812FX::mode_strobe_rainbow(void) {
  return blink(color_wheel(SEGENV.call & 0xFF), SEGCOLOR(1), true, false);
}


/*
 * Color wipe function
 * LEDs are turned on (color1) in sequence, then turned off (color2) in sequence.
 * if (bool rev == true) then LEDs are turned off in reverse order
 */
uint16_t WS2812FX::color_wipe(uint32_t color1, uint32_t color2, bool rev, bool dopalette) {
  if(SEGENV.step < SEGLEN) {
    uint32_t led_offset = SEGENV.step;
    uint16_t i = SEGMENT.start + led_offset;
    setPixelColor(i, dopalette ? color_from_palette(i, true, PALETTE_SOLID_WRAP, 0) : color1);
  } else {
    uint32_t led_offset = SEGENV.step - SEGLEN;
    if(rev) {
      setPixelColor(SEGMENT.stop -1 - led_offset, color2);
    } else {
      setPixelColor(SEGMENT.start + led_offset, color2);
    }
  }

  SEGENV.step = (SEGENV.step + 1) % (SEGLEN * 2);
  return SPEED_FORMULA_L;
}


/*
 * Lights all LEDs one after another.
 */
uint16_t WS2812FX::mode_color_wipe(void) {
  return color_wipe(SEGCOLOR(0), SEGCOLOR(1), false, true);
}

/*
 * Lights all LEDs one after another. Turns off opposite
 */
uint16_t WS2812FX::mode_color_sweep(void) {
  return color_wipe(SEGCOLOR(0), SEGCOLOR(1), true, true);
}


/*
 * Turns all LEDs after each other to a random color.
 * Then starts over with another color.
 */
uint16_t WS2812FX::mode_color_wipe_random(void) {
  if(SEGENV.step % SEGLEN == 0) { // aux0 will store our random color wheel index
    SEGENV.aux0 = get_random_wheel_index(SEGENV.aux0);
  }
  uint32_t color = color_wheel(SEGENV.aux0);
  return color_wipe(color, color, false, false);
}


/*
 * Random color introduced alternating from start and end of strip.
 */
uint16_t WS2812FX::mode_color_sweep_random(void) {
  if(SEGENV.step % SEGLEN == 0) { // aux0 will store our random color wheel index
    SEGENV.aux0 = get_random_wheel_index(SEGENV.aux0);
  }
  uint32_t color = color_wheel(SEGENV.aux0);
  return color_wipe(color, color, true, false);
}


/*
 * Lights all LEDs in one random color up. Then switches them
 * to the next random color.
 */
uint16_t WS2812FX::mode_random_color(void) {
  SEGENV.aux0 = get_random_wheel_index(SEGENV.aux0); // aux0 will store our random color wheel index
  uint32_t color = color_wheel(SEGENV.aux0);

  for(uint16_t i=SEGMENT.start; i < SEGMENT.stop; i++) {
    setPixelColor(i, color);
  }
  return 50 + (20 * (uint32_t)(255 - SEGMENT.speed));
}


/*
 * Lights every LED in a random color. Changes all LED at the same time
 * to new random colors.
 */
uint16_t WS2812FX::mode_dynamic(void) {
  if(SEGMENT.intensity > 127 || SEGENV.call == 0) {
    for(uint16_t i=SEGMENT.start; i < SEGMENT.stop; i++) {
      setPixelColor(i, color_wheel(random8()));
    }
  }
  setPixelColor(SEGMENT.start + random16(SEGLEN), color_wheel(random8()));
  return 50 + (15 * (uint32_t)(255 - SEGMENT.speed));
}


/*
 * Does the "standby-breathing" of well known i-Devices. Fixed Speed.
 * Use mode "fade" if you like to have something similar with a different speed.
 */
uint16_t WS2812FX::mode_breath(void) {
  int lum = SEGENV.step;
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
    uint32_t color = SEGCOLOR(0);
    uint8_t w = ((color >> 24 & 0xFF) * lum) >> 8;
    uint8_t r = ((color >> 16 & 0xFF) * lum) >> 8;
    uint8_t g = ((color >>  8 & 0xFF) * lum) >> 8;
    uint8_t b = ((color       & 0xFF) * lum) >> 8;
    for(uint16_t i=SEGMENT.start; i < SEGMENT.stop; i++) {
      setPixelColor(i, r, g, b, w);
    }
  } else
  {
    for(uint16_t i=SEGMENT.start; i < SEGMENT.stop; i++) {
      setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0, lum));
    }
  }

  SEGENV.step += 2;
  if(SEGENV.step > (512-15)) SEGENV.step = 15;
  return delay * (((256 - SEGMENT.speed)/64) +1);
}


/*
 * Fades the LEDs between two colors
 */
uint16_t WS2812FX::mode_fade(void) {
  int lum = SEGENV.step;
  if(lum > 255) lum = 511 - lum; // lum = 0 -> 255 -> 0
  
  for(uint16_t i=SEGMENT.start; i < SEGMENT.stop; i++) {
    setPixelColor(i, color_blend(color_from_palette(i, true, PALETTE_SOLID_WRAP, 0), SEGCOLOR(1), lum));
  }

  SEGENV.step += 4;
  if(SEGENV.step > 511) SEGENV.step = 0;
  return 5 + ((15 * (uint32_t)(255 - SEGMENT.speed)) / 255);
}


/*
 * Scan mode parent function
 */
uint16_t WS2812FX::scan(bool dual)
{
  if(SEGENV.step > (SEGLEN * 2) - 3) {
    SEGENV.step = 0;
  }

  fill(SEGCOLOR(1));

  int led_offset = SEGENV.step - (SEGLEN - 1);
  led_offset = abs(led_offset);

  uint16_t i = SEGMENT.start + led_offset;
  setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));

  if (dual) {
    uint16_t i2 = SEGMENT.start + SEGLEN - led_offset - 1;
    setPixelColor(i2, color_from_palette(i2, true, PALETTE_SOLID_WRAP, 0));
  }

  SEGENV.step++;
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
  uint32_t color = color_wheel(SEGENV.step);
  fill(color);

  SEGENV.step = (SEGENV.step + 1) & 0xFF;
  return 1 + (((uint32_t)(255 - SEGMENT.speed)) / 5);
}


/*
 * Cycles a rainbow over the entire string of LEDs.
 */
uint16_t WS2812FX::mode_rainbow_cycle(void) {
  for(uint16_t i=0; i < SEGLEN; i++) {
    uint32_t color = color_wheel(((i * 256 / ((uint16_t)(SEGLEN*(float)(SEGMENT.intensity/128.0))+1)) + SEGENV.step) & 0xFF);
    setPixelColor(SEGMENT.start + i, color);
  }

  SEGENV.step = (SEGENV.step + 1) & 0xFF;
  return 1 + (((uint32_t)(255 - SEGMENT.speed)) / 5);
}


/*
 * theater chase function
 */
uint16_t WS2812FX::theater_chase(uint32_t color1, uint32_t color2, bool dopalette) {
  SEGENV.call = SEGENV.call % 3;
  for(uint16_t i=0; i < SEGLEN; i++) {
    if((i % 3) == SEGENV.call) {
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
  return theater_chase(SEGCOLOR(0), SEGCOLOR(1), true);
}


/*
 * Theatre-style crawling lights with rainbow effect.
 * Inspired by the Adafruit examples.
 */
uint16_t WS2812FX::mode_theater_chase_rainbow(void) {
  SEGENV.step = (SEGENV.step + 1) & 0xFF;
  return theater_chase(color_wheel(SEGENV.step), SEGCOLOR(1), false);
}


/*
 * Running lights effect with smooth sine transition base.
 */
uint16_t WS2812FX::running_base(bool saw) {
  uint8_t x_scale = SEGMENT.intensity >> 2;

  for(uint16_t i=0; i < SEGLEN; i++) {
    uint8_t s = 0;
    uint8_t a = i*x_scale - (SEGENV.step >> 4);
    if (saw) {
      if (a < 16)
      {
        a = 192 + a*8;
      } else {
        a = map(a,16,255,64,192);
      }
    }
    s = sin8(a);
    setPixelColor(SEGMENT.start + i, color_blend(color_from_palette(SEGMENT.start + i, true, PALETTE_SOLID_WRAP, 0), SEGCOLOR(1), s));
  }
  SEGENV.step += SEGMENT.speed;
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
  if(SEGENV.step == 0) {
    fill(SEGCOLOR(1));
    SEGENV.step = map(SEGMENT.intensity, 0, 255, 1, SEGLEN); // make sure, at least one LED is on
  }

  uint16_t i = SEGMENT.start + random16(SEGLEN);
  setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));

  SEGENV.step--;
  return 20 + (5 * (uint16_t)(255 - SEGMENT.speed));
}


/*
 * Dissolve function
 */
uint16_t WS2812FX::dissolve(uint32_t color) {
  bool wa = (SEGCOLOR(1) != 0 && _brightness < 255); //workaround, can't compare getPixel to color if not full brightness
  
  for (uint16_t j = 0; j <= SEGLEN / 15; j++)
  {
    if (random8() <= SEGMENT.intensity) {
      for (uint8_t times = 0; times < 10; times++) //attempt to spawn a new pixel 5 times
      {
        uint16_t i = SEGMENT.start + random16(SEGLEN);
        if (SEGENV.aux0) { //dissolve to primary/palette
          if (getPixelColor(i) == SEGCOLOR(1) || wa) {
            if (color == SEGCOLOR(0))
            {
              setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));
            } else { setPixelColor(i, color); }     
            break; //only spawn 1 new pixel per frame per 50 LEDs
          }
        } else { //dissolve to secondary
          if (getPixelColor(i) != SEGCOLOR(1)) { setPixelColor(i, SEGCOLOR(1)); break; }
        }
      }
    }
  }

  if (SEGENV.call > (255 - SEGMENT.speed) + 15) 
  {
    SEGENV.aux0 = !SEGENV.aux0;
    SEGENV.call = 0;
  }
  
  return 20;
}


/*
 * Blink several LEDs on and then off
 */
uint16_t WS2812FX::mode_dissolve(void) {
  return dissolve(SEGCOLOR(0));
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
  for(uint16_t i=SEGMENT.start; i < SEGMENT.stop; i++) {
      setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 1));
  }
  SEGENV.aux0 = random16(SEGLEN); // aux0 stores the random led index
  setPixelColor(SEGMENT.start + SEGENV.aux0, SEGCOLOR(0));
  return 10 + (uint16_t)(255 - SEGMENT.speed);
}


/*
 * Lights all LEDs in the color. Flashes single white pixels randomly.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
uint16_t WS2812FX::mode_flash_sparkle(void) {
  if(SEGENV.call == 0) {
    for(uint16_t i=SEGMENT.start; i < SEGMENT.stop; i++) {
      setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));
    }
  }

  uint16_t i = SEGMENT.start + SEGENV.aux0;
  setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));

  if(random8(5) == 0) {
    SEGENV.aux0 = random16(SEGLEN); // aux0 stores the random led index
    setPixelColor(SEGMENT.start + SEGENV.aux0, SEGCOLOR(1));
    return 20;
  } 
  return 20 + (uint16_t)(255-SEGMENT.speed);
}


/*
 * Like flash sparkle. With more flash.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
uint16_t WS2812FX::mode_hyper_sparkle(void) {
  for(uint16_t i=SEGMENT.start; i < SEGMENT.stop; i++) {
    setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));
  }

  if(random8(5) < 2) {
    for(uint16_t i=0; i < max(1, SEGLEN/3); i++) {
      setPixelColor(SEGMENT.start + random16(SEGLEN), SEGCOLOR(1));
    }
    return 20;
  }
  return 20 + (uint16_t)(255-SEGMENT.speed);
}


/*
 * Strobe effect with different strobe count and pause, controlled by speed.
 */
uint16_t WS2812FX::mode_multi_strobe(void) {
  for(uint16_t i=SEGMENT.start; i < SEGMENT.stop; i++) {
    setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 1));
  }

  uint16_t delay = 50 + 20*(uint16_t)(255-SEGMENT.speed);
  uint16_t count = 2 * ((SEGMENT.speed / 10) + 1);
  if(SEGENV.step < count) {
    if((SEGENV.step & 1) == 0) {
      for(uint16_t i=SEGMENT.start; i < SEGMENT.stop; i++) {
        setPixelColor(i, SEGCOLOR(0));
      }
      delay = 20;
    } else {
      delay = 50;
    }
  }
  SEGENV.step = (SEGENV.step + 1) % (count + 1);
  return delay;
}

/*
 * Android loading circle
 */
uint16_t WS2812FX::mode_android(void) {
  if (SEGENV.call == 0)
  {
    SEGENV.aux0 = 0;
    SEGENV.step = SEGMENT.start;
  }
  
  for(uint16_t i=SEGMENT.start; i < SEGMENT.stop; i++) {
    setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 1));
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
  
  if (a >= SEGMENT.stop) a = SEGMENT.start;

  if (a + SEGENV.aux1 < SEGMENT.stop)
  {
    for(int i = a; i < a+SEGENV.aux1; i++) {
      setPixelColor(i, SEGCOLOR(0));
    }
  } else
  {
    for(int i = a; i < SEGMENT.stop; i++) {
      setPixelColor(i, SEGCOLOR(0));
    }
    for(int i = SEGMENT.start; i < SEGENV.aux1 - (SEGMENT.stop -a); i++) {
      setPixelColor(i, SEGCOLOR(0));
    }
  }
  SEGENV.step = a;

  return 3 + ((8 * (uint32_t)(255 - SEGMENT.speed)) / SEGLEN);
}

/*
 * color chase function.
 * color1 = background color
 * color2 and color3 = colors of two adjacent leds
 */
uint16_t WS2812FX::chase(uint32_t color1, uint32_t color2, uint32_t color3, bool dopalette) {
  uint16_t a = SEGENV.step;
  uint16_t b = (a + 1) % SEGLEN;
  uint16_t c = (b + 1) % SEGLEN;

  if (dopalette) color1 = color_from_palette(SEGMENT.start + a, true, PALETTE_SOLID_WRAP, 1);

  setPixelColor(SEGMENT.start + a, color1);
  setPixelColor(SEGMENT.start + b, color2);
  setPixelColor(SEGMENT.start + c, color3);

  SEGENV.step = (SEGENV.step + 1) % SEGLEN;
  return SPEED_FORMULA_L;
}


/*
 * Bicolor chase, more primary color.
 */
uint16_t WS2812FX::mode_chase_color(void) {
  return chase(SEGCOLOR(1), SEGCOLOR(0), SEGCOLOR(0), true);
}


/*
 * Primary running followed by random color.
 */
uint16_t WS2812FX::mode_chase_random(void) {
  if(SEGENV.step == 0) {
    SEGENV.aux0 = get_random_wheel_index(SEGENV.aux0);
  }
  return chase(color_wheel(SEGENV.aux0), SEGCOLOR(0), SEGCOLOR(0), false);
}


/*
 * Primary running on rainbow.
 */
uint16_t WS2812FX::mode_chase_rainbow_white(void) {
  uint16_t n = SEGENV.step;
  uint16_t m = (SEGENV.step + 1) % SEGLEN;
  uint32_t color2 = color_wheel(((n * 256 / SEGLEN) + (SEGENV.call & 0xFF)) & 0xFF);
  uint32_t color3 = color_wheel(((m * 256 / SEGLEN) + (SEGENV.call & 0xFF)) & 0xFF);

  return chase(SEGCOLOR(0), color2, color3, false);
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
  for (i; i < SEGMENT.stop ; i+=4)
  {
    setPixelColor(i, cols[SEGENV.step]);
    setPixelColor(i+1, cols[SEGENV.step+1]);
    setPixelColor(i+2, cols[SEGENV.step+2]);
    setPixelColor(i+3, cols[SEGENV.step+3]);
  }
  i+=4;
  if(i < SEGMENT.stop)
  {
    setPixelColor(i, cols[SEGENV.step]);
    
    if(i+1 < SEGMENT.stop)
    {
      setPixelColor(i+1, cols[SEGENV.step+1]);
      
      if(i+2 < SEGMENT.stop)
      {
        setPixelColor(i+2, cols[SEGENV.step+2]);
      }
    }
  }
  
  if (SEGMENT.speed > 0) SEGENV.step++; //static if lowest speed
  if (SEGENV.step >3) SEGENV.step = 0;
  return 50 + (15 * (uint32_t)(255 - SEGMENT.speed));
}


/*
 * Emulates a traffic light.
 */
uint16_t WS2812FX::mode_traffic_light(void) {
  for(uint16_t i=SEGMENT.start; i < SEGMENT.stop; i++)
    setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 1));
  uint32_t mdelay = 500;
  for (int i = SEGMENT.start; i < SEGMENT.stop-2 ; i+=3)
  {
    switch (SEGENV.step)
    {
      case 0: setPixelColor(i, 0x00FF0000); mdelay = 150 + (100 * (uint32_t)(255 - SEGMENT.speed));break;
      case 1: setPixelColor(i, 0x00FF0000); mdelay = 150 + (20 * (uint32_t)(255 - SEGMENT.speed)); setPixelColor(i+1, 0x00EECC00); break;
      case 2: setPixelColor(i+2, 0x0000FF00); mdelay = 150 + (100 * (uint32_t)(255 - SEGMENT.speed));break;
      case 3: setPixelColor(i+1, 0x00EECC00); mdelay = 150 + (20 * (uint32_t)(255 - SEGMENT.speed));break;
    }
  }

  SEGENV.step++;
  if (SEGENV.step >3) SEGENV.step = 0;
  return mdelay;
}


/*
 * Primary, secondary running on rainbow.
 */
uint16_t WS2812FX::mode_chase_rainbow(void) {
  uint8_t color_sep = 256 / SEGLEN;
  uint8_t color_index = SEGENV.call & 0xFF;
  uint32_t color = color_wheel(((SEGENV.step * color_sep) + color_index) & 0xFF);

  return chase(color, SEGCOLOR(0), SEGCOLOR(1), 0);
}


/*
 * Sec flashes running on prim.
 */
#define FLASH_COUNT 4
uint16_t WS2812FX::mode_chase_flash(void) {
  uint8_t flash_step = SEGENV.call % ((FLASH_COUNT * 2) + 1);

  for(uint16_t i=SEGMENT.start; i < SEGMENT.stop; i++) {
    setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));
  }

  uint16_t delay = 10 + ((30 * (uint16_t)(255 - SEGMENT.speed)) / SEGLEN);
  if(flash_step < (FLASH_COUNT * 2)) {
    if(flash_step % 2 == 0) {
      uint16_t n = SEGENV.step;
      uint16_t m = (SEGENV.step + 1) % SEGLEN;
      setPixelColor(SEGMENT.start + n, SEGCOLOR(1));
      setPixelColor(SEGMENT.start + m, SEGCOLOR(1));
      delay = 20;
    } else {
      delay = 30;
    }
  } else {
    SEGENV.step = (SEGENV.step + 1) % SEGLEN;
  }
  return delay;
}


/*
 * Prim flashes running, followed by random color.
 */
uint16_t WS2812FX::mode_chase_flash_random(void) {
  uint8_t flash_step = SEGENV.call % ((FLASH_COUNT * 2) + 1);

  for(uint16_t i=0; i < SEGENV.step; i++) {
    setPixelColor(SEGMENT.start + i, color_wheel(SEGENV.aux0));
  }

  uint16_t delay = 1 + ((10 * (uint16_t)(255 - SEGMENT.speed)) / SEGLEN);
  if(flash_step < (FLASH_COUNT * 2)) {
    uint16_t n = SEGENV.step;
    uint16_t m = (SEGENV.step + 1) % SEGLEN;
    if(flash_step % 2 == 0) {
      setPixelColor(SEGMENT.start + n, SEGCOLOR(0));
      setPixelColor(SEGMENT.start + m, SEGCOLOR(0));
      delay = 20;
    } else {
      setPixelColor(SEGMENT.start + n, color_wheel(SEGENV.aux0));
      setPixelColor(SEGMENT.start + m, SEGCOLOR(1));
      delay = 30;
    }
  } else {
    SEGENV.step = (SEGENV.step + 1) % SEGLEN;

    if(SEGENV.step == 0) {
      SEGENV.aux0 = get_random_wheel_index(SEGENV.aux0);
    }
  }
  return delay;
}


/*
 * Alternating pixels running function.
 */
uint16_t WS2812FX::running(uint32_t color1, uint32_t color2) {
  for(uint16_t i=0; i < SEGLEN; i++) {
    if((i + SEGENV.step) % 4 < 2) {
      if (color1 == SEGCOLOR(0))
      {
        setPixelColor(SEGMENT.stop -i, color_from_palette(SEGMENT.stop -i -1, true, PALETTE_SOLID_WRAP, 0));
      } else
      {
        setPixelColor(SEGMENT.stop -i -1, color1);
      }
    } else {
      setPixelColor(SEGMENT.stop -i -1, color2);
    }
  }

  if (SEGMENT.speed != 0) SEGENV.step = (SEGENV.step + 1) & 0x3;
  return  35 + ((350 * (uint32_t)(255 - SEGMENT.speed)) / 255);
}

/*
 * Alternating color/sec pixels running.
 */
uint16_t WS2812FX::mode_running_color(void) {
  return running(SEGCOLOR(0), SEGCOLOR(1));
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
  for(uint16_t i=SEGLEN-1; i > 0; i--) {
    setPixelColor(SEGMENT.start + i, getPixelColor(SEGMENT.start + i - 1));
  }

  if(SEGENV.step == 0) {
    SEGENV.aux0 = get_random_wheel_index(SEGENV.aux0);
    setPixelColor(SEGMENT.start, color_wheel(SEGENV.aux0));
  }

  SEGENV.step++;
  if (SEGENV.step > ((255-SEGMENT.intensity) >> 4))
  {
    SEGENV.step = 0;
  }
  return SPEED_FORMULA_L;
}


/*
 * K.I.T.T.
 */
uint16_t WS2812FX::mode_larson_scanner(void) {
  fade_out(SEGMENT.intensity);

  uint16_t index = 0;
  if(SEGENV.step < SEGLEN) {
    index = SEGMENT.start + SEGENV.step;
  } else {
    index = SEGMENT.start + ((SEGLEN * 2) - SEGENV.step) - 2;
  }
  setPixelColor(index, color_from_palette(index, true, PALETTE_SOLID_WRAP, 0));

  SEGENV.step = (SEGENV.step + 1) % ((SEGLEN * 2) - 2);
  return SPEED_FORMULA_L;
}


/*
 * Firing comets from one end.
 */
uint16_t WS2812FX::mode_comet(void) {
  fade_out(SEGMENT.intensity);

  uint16_t index = SEGMENT.start + SEGENV.step;
  setPixelColor(index, color_from_palette(index, true, PALETTE_SOLID_WRAP, 0));

  SEGENV.step = (SEGENV.step + 1) % SEGLEN;
  return SPEED_FORMULA_L;
}


/*
 * Fireworks function.
 */
uint16_t WS2812FX::mode_fireworks() {
  fade_out(0);
  if (SEGENV.call == 0) {
    SEGENV.aux0 = UINT16_MAX;
    SEGENV.aux1 = UINT16_MAX;
  }
  bool valid1 = (SEGENV.aux0  < SEGMENT.stop && SEGENV.aux0  >= SEGMENT.start);
  bool valid2 = (SEGENV.aux1 < SEGMENT.stop && SEGENV.aux1 >= SEGMENT.start);
  uint32_t sv1 = 0, sv2 = 0;
  if (valid1) sv1 = getPixelColor(SEGENV.aux0);
  if (valid2) sv2 = getPixelColor(SEGENV.aux1);
  blur(255-SEGMENT.speed);
  if (valid1) setPixelColor(SEGENV.aux0 , sv1);
  if (valid2) setPixelColor(SEGENV.aux1, sv2);

  for(uint16_t i=0; i<max(1, SEGLEN/20); i++) {
    if(random8(129 - (SEGMENT.intensity >> 1)) == 0) {
      uint16_t index = SEGMENT.start + random(SEGLEN);
      setPixelColor(index, color_from_palette(random8(), false, false, 0));
      SEGENV.aux1 = SEGENV.aux0;
      SEGENV.aux0 = index;
    }
  }
  return 22;
}


//Twinkling LEDs running. Inspired by https://github.com/kitesurfer1404/WS2812FX/blob/master/src/custom/Rain.h
uint16_t WS2812FX::mode_rain()
{
  SEGENV.step += 22;
  if (SEGENV.step > SPEED_FORMULA_L) {
    SEGENV.step = 0;
    //shift all leds right
    uint32_t ctemp = getPixelColor(SEGMENT.stop -1);
    for(uint16_t i=SEGMENT.stop -1; i>SEGMENT.start; i--) {
      setPixelColor(i, getPixelColor(i-1));
    }
    setPixelColor(SEGMENT.start, ctemp);
    SEGENV.aux0++;
    SEGENV.aux1++;
    if (SEGENV.aux0 == 0) SEGENV.aux0 = UINT16_MAX;
    if (SEGENV.aux1 == 0) SEGENV.aux0 = UINT16_MAX;
    if (SEGENV.aux0 == SEGMENT.stop) SEGENV.aux0 = SEGMENT.start;
    if (SEGENV.aux1 == SEGMENT.stop) SEGENV.aux1 = SEGMENT.start;
  }
  return mode_fireworks();
}


/*
 * Fire flicker function
 */
uint16_t WS2812FX::mode_fire_flicker(void) {
  byte w = (SEGCOLOR(0) >> 24) & 0xFF;
  byte r = (SEGCOLOR(0) >> 16) & 0xFF;
  byte g = (SEGCOLOR(0) >>  8) & 0xFF;
  byte b = (SEGCOLOR(0)        & 0xFF);
  byte lum = (SEGMENT.palette == 0) ? max(w, max(r, max(g, b))) : 255;
  lum /= (((256-SEGMENT.intensity)/16)+1);
  for(uint16_t i=SEGMENT.start; i < SEGMENT.stop; i++) {
    byte flicker = random8(lum);
    if (SEGMENT.palette == 0) {
      setPixelColor(i, max(r - flicker, 0), max(g - flicker, 0), max(b - flicker, 0), max(w - flicker, 0));
    } else {
      setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0, 255 - flicker));
    }
  }
  return 20 + random((255 - SEGMENT.speed),(2 * (uint16_t)(255 - SEGMENT.speed)));
}


/*
 * Gradient run base function
 */
uint16_t WS2812FX::gradient_base(bool loading) {
  if (SEGENV.call == 0) SEGENV.step = 0;
  float per,val; //0.0 = sec 1.0 = pri
  float brd = SEGMENT.intensity;
  if (!loading) brd = SEGMENT.intensity/2; 
  if (brd <1.0) brd = 1.0;
  int pp = SEGENV.step;
  int p1 = pp-SEGLEN;
  int p2 = pp+SEGLEN;
  
  for(uint16_t i=SEGMENT.start; i < SEGMENT.stop; i++)
  {
    if (loading)
    {
      val = abs(((i>pp) ? p2:pp) -i);
    } else {
      val = min(abs(pp-i),min(abs(p1-i),abs(p2-i)));
    }
    per = val/brd;
    if (per >1.0) per = 1.0;
    setPixelColor(SEGMENT.start + i, color_blend(SEGCOLOR(0), color_from_palette(SEGMENT.start + i, true, PALETTE_SOLID_WRAP, 1), per*255));
  }
  
  SEGENV.step++;
  if (SEGENV.step >= SEGMENT.stop) SEGENV.step = SEGMENT.start;
  if (SEGMENT.speed == 0) SEGENV.step = SEGMENT.start + (SEGLEN >> 1);
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
  int end = SEGLEN - SEGENV.step - 1; 
  bool odd = (SEGLEN % 2) == 1; 
  int mid = odd ? ((SEGLEN / 2) + 1) : (SEGLEN / 2);  
  if (SEGENV.step < mid) {
    byte pindex = map(SEGENV.step, 0, mid -1, 0, 255);
    uint32_t col = color_from_palette(pindex, false, false, 0);
    
    setPixelColor(SEGMENT.start + SEGENV.step, col); 
    setPixelColor(SEGMENT.start + end, col); 
  } else {  
    if (odd) {  
      // If odd, we need to 'double count' the center LED (once to turn it on,  
      // once to turn it off). So trail one behind after the middle LED.  
      setPixelColor(SEGMENT.start + SEGENV.step - 1, SEGCOLOR(1)); 
      setPixelColor(SEGMENT.start + end + 1, SEGCOLOR(1)); 
    } else {  
      setPixelColor(SEGMENT.start + SEGENV.step, SEGCOLOR(1)); 
      setPixelColor(SEGMENT.start + end, SEGCOLOR(1)); 
    } 
  } 
   SEGENV.step++; 
  if (odd) {  
    if (SEGENV.step > SEGLEN) { 
      SEGENV.step = 0;  
    } 
  } else {  
    if (SEGENV.step >= SEGLEN) {  
      SEGENV.step = 0;  
    } 
  } 
  return SPEED_FORMULA_L;
}


/* 
* Lights all LEDs after each other up starting from the outer edges and  
* finishing in the middle. Then turns them in that order off. Repeat.  
*/ 
uint16_t WS2812FX::mode_dual_color_wipe_in_in(void) { 
  bool odd = (SEGLEN % 2) == 1; 
  int mid = SEGLEN / 2;
  byte pindex = 0;
  uint32_t col = 0;
  if (SEGENV.step <= mid)
  {
    pindex = map(SEGENV.step, 0, mid, 0, 255);
    col = color_from_palette(pindex, false, false, 0);
  }
  if (odd) {  
    if (SEGENV.step <= mid) { 
      setPixelColor(SEGMENT.start + SEGENV.step, col); 
      setPixelColor(SEGMENT.start + SEGLEN - SEGENV.step - 1, col);  
    } else {  
      int i = SEGENV.step - mid;  
      setPixelColor(SEGMENT.start + i - 1, SEGCOLOR(1)); 
      setPixelColor(SEGMENT.start + SEGLEN - i, SEGCOLOR(1));  
    } 
  } else {  
    if (SEGENV.step < mid) {  
      setPixelColor(SEGMENT.start + SEGENV.step, col); 
      setPixelColor(SEGMENT.start + SEGLEN - SEGENV.step - 1, col);  
    } else {  
      int i = SEGENV.step - mid;  
      setPixelColor(SEGMENT.start + i, SEGCOLOR(1)); 
      setPixelColor(SEGMENT.start + SEGLEN - i - 1, SEGCOLOR(1));  
    } 
  } 
   SEGENV.step++; 
  if (odd) {  
    if (SEGENV.step > SEGLEN) { 
      SEGENV.step = 0;  
    } 
  } else {  
    if (SEGENV.step >= SEGLEN) {  
      SEGENV.step = 0;  
    } 
  } 
  return SPEED_FORMULA_L;
}


/* 
* Lights all LEDs after each other up starting from the middle and 
* finishing at the edges. Then turns them off in that order. Repeat. 
*/ 
uint16_t WS2812FX::mode_dual_color_wipe_out_out(void) { 
  int end = SEGLEN - SEGENV.step - 1; 
  bool odd = (SEGLEN % 2) == 1; 
  int mid = SEGLEN / 2;
  byte pindex = 0;
  uint32_t col = 0;
  if (SEGENV.step <= mid)
  {
    pindex = map(SEGENV.step, 0, mid, 255, 0);
    col = color_from_palette(pindex, false, false, 0);
  }
  if (odd) { 
    if (SEGENV.step <= mid) { 
      setPixelColor(SEGMENT.start + mid + SEGENV.step, col); 
      setPixelColor(SEGMENT.start + mid - SEGENV.step, col); 
    } else {  
      setPixelColor(SEGMENT.start + SEGENV.step - 1, SEGCOLOR(1)); 
      setPixelColor(SEGMENT.start + end + 1, SEGCOLOR(1)); 
    } 
  } else {  
    if (SEGENV.step < mid) {  
      setPixelColor(SEGMENT.start + mid - SEGENV.step - 1, col); 
      setPixelColor(SEGMENT.start + mid + SEGENV.step, col); 
    } else {  
      setPixelColor(SEGMENT.start + SEGENV.step, SEGCOLOR(1)); 
      setPixelColor(SEGMENT.start + end, SEGCOLOR(1)); 
    } 
  } 
   SEGENV.step++; 
  if (odd) {  
    if (SEGENV.step > SEGLEN) { 
      SEGENV.step = 0;  
    } 
  } else {  
    if (SEGENV.step >= SEGLEN) {  
      SEGENV.step = 0;  
    } 
  } 
  return SPEED_FORMULA_L;
} 


/* 
* Lights all LEDs after each other up starting from the middle and 
* finishing at the edges. Then turns them off in reverse order. Repeat.  
*/ 
uint16_t WS2812FX::mode_dual_color_wipe_out_in(void) {  
  bool odd = (SEGLEN % 2) == 1; 
  int mid = SEGLEN / 2;
  byte pindex = 0;
  uint32_t col = 0;
  if (SEGENV.step <= mid)
  {
    pindex = map(SEGENV.step, 0, mid, 255, 0);
    col = color_from_palette(pindex, false, false, 0);
  }
   if (odd) { 
    if (SEGENV.step <= mid) { 
      setPixelColor(SEGMENT.start + mid + SEGENV.step, col); 
      setPixelColor(SEGMENT.start + mid - SEGENV.step, col); 
    } else {  
      int i = SEGENV.step - mid;  
      setPixelColor(SEGMENT.start + i - 1, SEGCOLOR(1)); 
      setPixelColor(SEGMENT.start + SEGLEN - i, SEGCOLOR(1));  
    } 
  } else {  
    if (SEGENV.step < mid) {  
      setPixelColor(SEGMENT.start + mid - SEGENV.step - 1, col); 
      setPixelColor(SEGMENT.start + mid + SEGENV.step, col); 
    } else {  
      int i = SEGENV.step - mid;  
      setPixelColor(SEGMENT.start + i, SEGCOLOR(1)); 
      setPixelColor(SEGMENT.start + SEGLEN - i - 1, SEGCOLOR(1));  
    } 
  } 
   SEGENV.step++; 
  if (odd) {  
    if (SEGENV.step > SEGLEN) { 
      SEGENV.step = 0;  
    } 
  } else {  
    if (SEGENV.step >= SEGLEN) {  
      SEGENV.step = 0;  
    } 
  } 
  return SPEED_FORMULA_L;
}


/*
 * Tricolor chase function
 */
uint16_t WS2812FX::tricolor_chase(uint32_t color1, uint32_t color2) {
  uint16_t index = SEGENV.step % 6;
  for(uint16_t i=0; i < SEGLEN; i++, index++) {
    if(index > 5) index = 0;

    uint32_t color = color1;
    if(index > 3) color = color_from_palette(i, true, PALETTE_SOLID_WRAP, 1);
    else if(index > 1) color = color2;

    setPixelColor(SEGMENT.stop - i -1, color);
  }

  SEGENV.step++;
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
  return tricolor_chase(SEGCOLOR(2), SEGCOLOR(0));
}


/*
 * ICU mode
 */
uint16_t WS2812FX::mode_icu(void) {
  uint16_t dest = SEGENV.step & 0xFFFF;

  fill(SEGCOLOR(1));

  byte pindex = map(dest, 0, SEGLEN/2, 0, 255);
  uint32_t col = color_from_palette(pindex, false, false, 0);
 
  setPixelColor(SEGMENT.start + dest, col);
  setPixelColor(SEGMENT.start + dest + SEGLEN/2, col);

  if(SEGENV.aux0 == dest) { // pause between eye movements
    if(random8(6) == 0) { // blink once in a while
      setPixelColor(SEGMENT.start + dest, SEGCOLOR(1));
      setPixelColor(SEGMENT.start + dest + SEGLEN/2, SEGCOLOR(1));
      return 200;
    }
    SEGENV.aux0 = random16(SEGLEN/2);
    return 1000 + random16(2000);
  }

  if(SEGENV.aux0 > SEGENV.step) {
    SEGENV.step++;
    dest++;
  } else if (SEGENV.aux0 < SEGENV.step) {
    SEGENV.step--;
    dest--;
  }

  setPixelColor(SEGMENT.start + dest, col);
  setPixelColor(SEGMENT.start + dest + SEGLEN/2, col);

  return SPEED_FORMULA_L;
}


/*
 * Custom mode by Aircoookie. Color Wipe, but with 3 colors
 */
uint16_t WS2812FX::mode_tricolor_wipe(void)
{
  if(SEGENV.step < SEGLEN) {
    uint32_t led_offset = SEGENV.step;
    setPixelColor(SEGMENT.start + led_offset, SEGCOLOR(0));
  } else if (SEGENV.step < SEGLEN*2) {
    uint32_t led_offset = SEGENV.step - SEGLEN;
    setPixelColor(SEGMENT.start + led_offset, SEGCOLOR(1));
  } else
  {
    uint32_t led_offset = SEGENV.step - SEGLEN*2;
    setPixelColor(SEGMENT.start + led_offset, color_from_palette(SEGMENT.start + led_offset, true, PALETTE_SOLID_WRAP, 2));
  }

  SEGENV.step = (SEGENV.step + 1) % (SEGLEN * 3);
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

  if(SEGENV.step < 256) {
    color1 = SEGCOLOR(0);
    color2 = SEGCOLOR(1);
    stage = 0;
  } else if(SEGENV.step < 512) {
    color1 = SEGCOLOR(1);
    color2 = SEGCOLOR(2);
    stage = 1;
  } else {
    color1 = SEGCOLOR(2);
    color2 = SEGCOLOR(0);
    stage = 2;
  }

  byte stp = SEGENV.step % 256;
  uint32_t color = 0;
  for(uint16_t i=SEGMENT.start; i < SEGMENT.stop; i++) {
    if (stage == 2) {
      color = color_blend(color_from_palette(i, true, PALETTE_SOLID_WRAP, 2), color2, stp); 
    } else if (stage == 1) {
      color = color_blend(color1, color_from_palette(i, true, PALETTE_SOLID_WRAP, 2), stp); 
    } else {
      color = color_blend(color1, color2, stp);
    }
    setPixelColor(i, color);
  }

  SEGENV.step += 4;
  if(SEGENV.step >= 768) SEGENV.step = 0;

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
    if(comets[i] < SEGLEN) {
      uint16_t index = SEGMENT.start + comets[i];
      if (SEGCOLOR(2) != 0)
      {
        setPixelColor(index, i % 2 ? color_from_palette(index, true, PALETTE_SOLID_WRAP, 0) : SEGCOLOR(2));
      } else
      {
        setPixelColor(index, color_from_palette(index, true, PALETTE_SOLID_WRAP, 0));
      }
      comets[i]++;
    } else {
      if(!random(SEGLEN)) {
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
  if (SEGENV.aux0)
  {
    SEGENV.step--;
  } else
  {
    SEGENV.step++;
  }

  fade_out(SEGMENT.intensity);

  uint16_t index = SEGMENT.start + SEGENV.step;
  setPixelColor(index, color_from_palette(index, true, PALETTE_SOLID_WRAP, 0));
  index = SEGMENT.stop - SEGENV.step -1;
  if (SEGCOLOR(2) != 0)
  {
    setPixelColor(index, SEGCOLOR(2));
  } else
  {
    setPixelColor(index, color_from_palette(index, true, PALETTE_SOLID_WRAP, 0));
  }

  if(SEGENV.step >= SEGLEN -1 || SEGENV.step <= 0)
  SEGENV.aux0 = !SEGENV.aux0;
  
  return SPEED_FORMULA_L;
}


/*
 * Running random pixels
 * Custom mode by Keith Lord: https://github.com/kitesurfer1404/WS2812FX/blob/master/src/custom/RandomChase.h
 */
uint16_t WS2812FX::mode_random_chase(void)
{
  for(uint16_t i=SEGMENT.stop -1; i>SEGMENT.start; i--) {
    setPixelColor(i, getPixelColor(i-1));
  }
  uint32_t color = getPixelColor(SEGMENT.start);
  if (SEGLEN > 1) color = getPixelColor(SEGMENT.start + 1);
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
    {SEGLEN/4,   SEGLEN/8,  1, 1},
    {SEGLEN/4*2, SEGLEN/8, -1, 1}
    //{SEGLEN/4*3, SEGLEN/8,  1, 2}
  };

  for(int8_t i=0; i < sizeof(oscillators)/sizeof(oscillators[0]); i++) {
    oscillators[i].pos += oscillators[i].dir * oscillators[i].speed;
    if((oscillators[i].dir == -1) && (oscillators[i].pos <= 0)) {
      oscillators[i].pos = 0;
      oscillators[i].dir = 1;
      oscillators[i].speed = random8(1, 3);
    }
    if((oscillators[i].dir == 1) && (oscillators[i].pos >= (SEGLEN - 1))) {
      oscillators[i].pos = SEGLEN - 1;
      oscillators[i].dir = -1;
      oscillators[i].speed = random8(1, 3);
    }
  }

  for(int16_t i=0; i < SEGLEN; i++) {
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
  uint16_t ledstart = SEGMENT.start + random8(SEGLEN);               // Determine starting location of flash
  uint16_t ledlen = random8(SEGMENT.stop -1 -ledstart);                      // Determine length of flash (not to go beyond NUM_LEDS-1)
  uint8_t bri = 255/random8(1, 3);   

  if (SEGENV.step == 0)
  {
    SEGENV.aux0 = random8(3, 3 + SEGMENT.intensity/20); //number of flashes
    bri = 52; 
    SEGENV.aux1 = 1;
  }

  fill(SEGCOLOR(1));
  
  if (SEGENV.aux1) {
    for (int i = ledstart; i < ledstart + ledlen; i++)
    {
      if (SEGMENT.palette == 0)
      {
        setPixelColor(i,bri,bri,bri,bri);
      } else {
        setPixelColor(i,color_from_palette(i, true, PALETTE_SOLID_WRAP, 0, bri));
      }
    }
    SEGENV.aux1 = 0;
    SEGENV.step++;
    return random8(4, 10);                                    // each flash only lasts 4-10 milliseconds
  }

  SEGENV.aux1 = 1;
  if (SEGENV.step == 1) return (200);                       // longer delay until next flash after the leader

  if (SEGENV.step <= SEGENV.aux0) return (50 + random8(100));  // shorter delay between strokes

  SEGENV.step = 0;
  return (random8(255 - SEGMENT.speed) * 100);                            // delay between strikes
}


// Pride2015
// Animated, ever-changing rainbows.
// by Mark Kriegsman: https://gist.github.com/kriegsman/964de772d64c502760e5
uint16_t WS2812FX::mode_pride_2015(void)
{
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
  
  for( uint16_t i = SEGMENT.start ; i < SEGMENT.stop; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 >> 8;

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);
    
    CRGB newcolor = CHSV( hue8, sat8, bri8);
    fastled_col = col_to_crgb(getPixelColor(i));
    
    nblend( fastled_col, newcolor, 64);
    setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
  }
  SEGENV.step = sPseudotime;
  SEGENV.aux0 = sHue16;
  return 20;
}


//eight colored dots, weaving in and out of sync with each other
uint16_t WS2812FX::mode_juggle(void){
  fade_out(SEGMENT.intensity);
  CRGB fastled_col;
  byte dothue = 0;
  for ( byte i = 0; i < 8; i++) {
    uint16_t index = SEGMENT.start + beatsin16(i + 7, 0, SEGLEN -1);
    fastled_col = col_to_crgb(getPixelColor(index));
    fastled_col |= (SEGMENT.palette==0)?CHSV(dothue, 220, 255):ColorFromPalette(currentPalette, dothue, 255);
    setPixelColor(index, fastled_col.red, fastled_col.green, fastled_col.blue);
    dothue += 32;
  }
  return 10 + (uint16_t)(255 - SEGMENT.speed)/4;
}


uint16_t WS2812FX::mode_palette(void)
{
  bool noWrap = (paletteBlend == 2 || (paletteBlend == 0 && SEGMENT.speed == 0));
  for (uint16_t i = SEGMENT.start; i < SEGMENT.stop; i++)
  {
    uint8_t colorIndex = map(i,SEGMENT.start,SEGMENT.stop -1,0,255) - (SEGENV.step >> 6 & 0xFF);
    
    if (noWrap) colorIndex = map(colorIndex, 0, 255, 0, 240); //cut off blend at palette "end"
    
    setPixelColor(i, color_from_palette(colorIndex, false, true, 255));
  }
  SEGENV.step += SEGMENT.speed;
  if (SEGMENT.speed == 0) SEGENV.step = 0;
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
  for( int i = SEGMENT.start; i < SEGMENT.stop; i++) {
    _locked[i] = qsub8(_locked[i],  random8(0, (((20 + SEGMENT.speed /3) * 10) / SEGLEN) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for( int k= SEGMENT.stop -1; k >= SEGMENT.start + 2; k--) {
    _locked[k] = (_locked[k - 1] + _locked[k - 2] + _locked[k - 2] ) / 3;
  }
  
  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if( random8() <= SEGMENT.intensity ) {
    int y = SEGMENT.start + random8(7);
    if (y < SEGMENT.stop) _locked[y] = qadd8(_locked[y], random8(160,255) );
  }

  // Step 4.  Map from heat cells to LED colors
  for( int j = SEGMENT.start; j < SEGMENT.stop; j++) {
    CRGB color = ColorFromPalette( currentPalette, min(_locked[j],240), 255, LINEARBLEND);
    setPixelColor(j, color.red, color.green, color.blue);
  }
  return 20;
}


// ColorWavesWithPalettes by Mark Kriegsman: https://gist.github.com/kriegsman/8281905786e8b2632aeb
// This function draws color waves with an ever-changing,
// widely-varying set of parameters, using a color palette.
uint16_t WS2812FX::mode_colorwaves(void)
{
  uint16_t duration = 10 + SEGMENT.speed;
  uint16_t sPseudotime = SEGENV.step;
  uint16_t sHue16 = SEGENV.aux0;

  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 300, 1500);

  sPseudotime += duration * msmultiplier;
  sHue16 += duration * beatsin88( 400, 5, 9);
  uint16_t brightnesstheta16 = sPseudotime;
  CRGB fastled_col;

  for ( uint16_t i = SEGMENT.start ; i < SEGMENT.stop; i++) {
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
    fastled_col = col_to_crgb(getPixelColor(i));

    nblend(fastled_col, newcolor, 128);
    setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
  }
  SEGENV.step = sPseudotime;
  SEGENV.aux0 = sHue16;
  return 20;
}


// colored stripes pulsing at a defined Beats-Per-Minute (BPM)
uint16_t WS2812FX::mode_bpm(void)
{
  CRGB fastled_col;
  uint8_t beat = beatsin8(SEGMENT.speed, 64, 255);
  for (uint16_t i = SEGMENT.start; i < SEGMENT.stop; i++) {
    fastled_col = ColorFromPalette(currentPalette, SEGENV.step + (i * 2), beat - SEGENV.step + (i * 10));
    setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
  }
  SEGENV.step++;
  if (SEGENV.step >= 255) SEGENV.step = 0;
  return 20;
}


uint16_t WS2812FX::mode_fillnoise8(void)
{
  if (SEGENV.call == 0) SEGENV.step = random16(12345);
  CRGB fastled_col;
  for (uint16_t i = SEGMENT.start; i < SEGMENT.stop; i++) {
    uint8_t index = inoise8(i * SEGLEN, SEGENV.step + i * SEGLEN) % 255;
    fastled_col = ColorFromPalette(currentPalette, index, 255, LINEARBLEND);
    setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
  }
  SEGENV.step += beatsin8(SEGMENT.speed, 1, 6); //10,1,4

  return 20;
}

uint16_t WS2812FX::mode_noise16_1(void)
{
  uint16_t scale = 320;                                      // the "zoom factor" for the noise
  CRGB fastled_col;
  SEGENV.step += (1 + SEGMENT.speed/16);

  for (uint16_t i = SEGMENT.start; i < SEGMENT.stop; i++) {

    uint16_t shift_x = beatsin8(11);                           // the x position of the noise field swings @ 17 bpm
    uint16_t shift_y = SEGENV.step/42;             // the y position becomes slowly incremented


    uint16_t real_x = (i + shift_x) * scale;                  // the x position of the noise field swings @ 17 bpm
    uint16_t real_y = (i + shift_y) * scale;                  // the y position becomes slowly incremented
    uint32_t real_z = SEGENV.step;                          // the z position becomes quickly incremented

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
  SEGENV.step += (1 + SEGMENT.speed);

  for (uint16_t i = SEGMENT.start; i < SEGMENT.stop; i++) {

    uint16_t shift_x = SEGENV.step >> 6;                         // x as a function of time
    uint16_t shift_y = SEGENV.step/42;

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
  SEGENV.step += (1 + SEGMENT.speed);

  for (uint16_t i = SEGMENT.start; i < SEGMENT.stop; i++) {

    uint16_t shift_x = 4223;                                  // no movement along x and y
    uint16_t shift_y = 1234;

    uint32_t real_x = (i + shift_x) * scale;                  // calculate the coordinates within the noise field
    uint32_t real_y = (i + shift_y) * scale;                  // based on the precalculated positions
    uint32_t real_z = SEGENV.step*8;  

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
  SEGENV.step += SEGMENT.speed;
  for (uint16_t i = SEGMENT.start; i < SEGMENT.stop; i++) {
    int16_t index = inoise16(uint32_t(i - SEGMENT.start) << 12, SEGENV.step/8);
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
  for( uint16_t i = SEGMENT.start; i < SEGMENT.stop; i++) {
    fastled_col = col_to_crgb(getPixelColor(i));
    prev = fastled_col;
    if(_locked[i]) {  
      CRGB incrementalColor = fastled_col;
      incrementalColor.nscale8_video( fadeUpAmount);
      fastled_col += incrementalColor;

      if( fastled_col.red == 255 || fastled_col.green == 255 || fastled_col.blue == 255) {
        _locked[i] = false;
      }
      setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);

      if (col_to_crgb(getPixelColor(i)) == prev) //fix "stuck" pixels
      {
        fastled_col += fastled_col;
        setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
      }
    } else {
      fastled_col.nscale8( 255 - fadeDownAmount);
      setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
    }
  }

  for (uint16_t j = 0; j <= SEGLEN / 50; j++)
  {
    if ( random8() <= SEGMENT.intensity ) {
      for (uint8_t times = 0; times < 5; times++) //attempt to spawn a new pixel 5 times
      {
        int i = SEGMENT.start + random16(SEGLEN);
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

  for (uint16_t i = SEGMENT.start; i < SEGMENT.stop; i++)
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
  byte meteorSize= 1+ SEGLEN / 10;
  uint16_t in = SEGMENT.start + SEGENV.step;

  // fade all leds to colors[1] in LEDs one step
  for (uint16_t i = SEGMENT.start; i < SEGMENT.stop; i++) {
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
    if(in + j >= SEGMENT.stop) {
      index = SEGMENT.start + (in + j - SEGMENT.stop);
    }

    _locked[index] = 240;
    setPixelColor(index, color_from_palette(_locked[index], false, true, 255));
  }

  SEGENV.step = (SEGENV.step + 1) % (SEGLEN);
  return SPEED_FORMULA_L;
}


// smooth meteor effect
// send a meteor from begining to to the end of the strip with a trail that randomly decays.
// adapted from https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/#LEDStripEffectMeteorRain
uint16_t WS2812FX::mode_meteor_smooth() {
  byte meteorSize= 1+ SEGLEN / 10;
  uint16_t in = map((SEGENV.step >> 6 & 0xFF), 0, 255, SEGMENT.start, SEGMENT.stop -1);

  // fade all leds to colors[1] in LEDs one step
  for (uint16_t i = SEGMENT.start; i < SEGMENT.stop; i++) {
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
    if(in + j >= SEGMENT.stop) {
      index = SEGMENT.start + (in + j - SEGMENT.stop);
    }
    setPixelColor(index, color_blend(getPixelColor(index), color_from_palette(240, false, true, 255), 48));
    _locked[index] = 240;
  }

  SEGENV.step += SEGMENT.speed +1;
  return 20;
}


//Railway Crossing / Christmas Fairy lights
uint16_t WS2812FX::mode_railway()
{
  uint16_t dur = 40 + (255 - SEGMENT.speed) * 10;
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
  for (uint16_t i = SEGMENT.start; i < SEGMENT.stop; i += 2)
  {
    setPixelColor(i, color_from_palette(255 - pos, false, false, 255));
    if (i < SEGMENT.stop -1)
    {
      setPixelColor(i + 1, color_from_palette(pos, false, false, 255));
    }
  }
  SEGENV.step += 20;
  return 20;
}


//Water ripple
//propagation velocity from speed
//drop rate from intensity
uint16_t WS2812FX::mode_ripple()
{
  uint16_t maxripples = SEGLEN / 4;
  if (maxripples == 0) return mode_static();

  fill(SEGCOLOR(1));

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
        if (w < SEGMENT.stop && w >= SEGMENT.start)
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
        uint16_t origin = SEGMENT.start + random16(SEGLEN);
        _locked[storeI+1] = origin >> 8;
        _locked[storeI+2] = origin & 0xFF; 
        _locked[storeI+3] = random8();
      }
    }
  }
  return 20;
}


//  TwinkleFOX by Mark Kriegsman: https://gist.github.com/kriegsman/756ea6dcae8e30845b5a
//
//  TwinkleFOX: Twinkling 'holiday' lights that fade in and out.
//  Colors are chosen from a palette. Read more about this effect using the link above!

// If COOL_LIKE_INCANDESCENT is set to 1, colors will
// fade out slighted 'reddened', similar to how
// incandescent bulbs change color as they get dim down.
#define COOL_LIKE_INCANDESCENT 1

CRGB WS2812FX::twinklefox_one_twinkle(uint32_t ms, uint8_t salt)
{
  // Overall twinkle speed (changed)
  uint16_t ticks = ms / (32 - (SEGMENT.speed >> 3));
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
    if (ph < 86) {
    bright = ph * 3;
    } else {
      ph -= 86;
      bright = 255 - (ph + (ph/2));
    }
  }

  uint8_t hue = slowcycle8 - salt;
  CRGB c;
  if (bright > 0) {
    c = ColorFromPalette(currentPalette, hue, bright, NOBLEND);
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
uint16_t WS2812FX::mode_twinklefox()
{
  // "PRNG16" is the pseudorandom number generator
  // It MUST be reset to the same starting value each time
  // this function is called, so that the sequence of 'random'
  // numbers that it generates is (paradoxically) stable.
  uint16_t PRNG16 = 11337;

  uint32_t clock32 = millis();

  // Set up the background color, "bg".
  // if AUTO_SELECT_BACKGROUND_COLOR == 1, and the first two colors of
  // the current palette are identical, then a deeply faded version of
  // that color is used for the background color
  CRGB bg;
  bg = col_to_crgb(SEGCOLOR(1));
  uint8_t bglight = bg.getAverageLight();
  if (bglight > 64) {
    bg.nscale8_video(16); // very bright, so scale to 1/16th
  } else if (bglight > 16) {
    bg.nscale8_video(64); // not that bright, so scale to 1/4th
  } else {
    bg.nscale8_video(86); // dim, scale to 1/3rd.
  }

  uint8_t backgroundBrightness = bg.getAverageLight();

  for (uint16_t i = SEGMENT.start; i < SEGMENT.stop; i++) {
  
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    uint16_t myclockoffset16= PRNG16; // use that number as clock offset
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    // use that number as clock speed adjustment factor (in 8ths, from 8/8ths to 23/8ths)
    uint8_t myspeedmultiplierQ5_3 =  ((((PRNG16 & 0xFF)>>4) + (PRNG16 & 0x0F)) & 0x0F) + 0x08;
    uint32_t myclock30 = (uint32_t)((clock32 * myspeedmultiplierQ5_3) >> 3) + myclockoffset16;
    uint8_t  myunique8 = PRNG16 >> 8; // get 'salt' value for this pixel

    // We now have the adjusted 'clock' for this pixel, now we call
    // the function that computes what color the pixel should be based
    // on the "brightness = f( time )" idea.
    CRGB c = twinklefox_one_twinkle(myclock30, myunique8);

    uint8_t cbright = c.getAverageLight();
    int16_t deltabright = cbright - backgroundBrightness;
    if (deltabright >= 32 || (!bg)) {
      // If the new pixel is significantly brighter than the background color,
      // use the new color.
      setPixelColor(i, c);
    } else if (deltabright > 0) {
      // If the new pixel is just slightly brighter than the background color,
      // mix a blend of the new color and the background color
      setPixelColor(i, color_blend(crgb_to_col(bg), crgb_to_col(c), deltabright * 8));
    } else {
      // if the new pixel is not at all brighter than the background color,
      // just use the background color.
      setPixelColor(i, bg);
    }
  }
  return 20;
}
