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

#include "FX.h"
#include "wled.h"

// WLEDSR Custom Effects
#include "src/dependencies/arti/arti_wled.h"

#define IBN 5100
#define PALETTE_SOLID_WRAP (paletteBlend == 1 || paletteBlend == 3)

/*
 * No blinking. Just plain old static light.
 */
uint16_t WS2812FX::mode_static(void)
{
  fill(SEGCOLOR(0));
  return (SEGMENT.getOption(SEG_OPTION_TRANSITIONAL)) ? FRAMETIME : 350; // update faster if in transition
}

/*
 * Blink/strobe function
 * Alternate between color1 and color2
 * if(strobe == true) then create a strobe effect
 */
uint16_t WS2812FX::blink(uint32_t color1, uint32_t color2, bool strobe, bool do_palette)
{
  uint32_t cycleTime = (255 - SEGMENT.speed) * 20;
  uint32_t onTime = FRAMETIME;
  if (!strobe)
    onTime += ((cycleTime * SEGMENT.intensity) >> 8);
  cycleTime += FRAMETIME * 2;
  uint32_t it = now / cycleTime;
  uint32_t rem = now % cycleTime;

  bool on = false;
  if (it != SEGENV.step // new iteration, force on state for one frame, even if set time is too brief
      || rem <= onTime)
  {
    on = true;
  }

  SEGENV.step = it; // save previous iteration

  uint32_t color = on ? color1 : color2;
  if (color == color1 && do_palette)
  {
    for (uint16_t i = 0; i < SEGLEN; i++)
    {
      setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));
    }
  }
  else
    fill(color);

  return FRAMETIME;
}

/*
 * Normal blinking. 50% on/off time.
 */
uint16_t WS2812FX::mode_blink(void)
{
  return blink(SEGCOLOR(0), SEGCOLOR(1), false, true);
}

/*
 * Classic Blink effect. Cycling through the rainbow.
 */
uint16_t WS2812FX::mode_blink_rainbow(void)
{
  return blink(color_wheel(SEGENV.call & 0xFF), SEGCOLOR(1), false, false);
}

/*
 * Classic Strobe effect.
 */
uint16_t WS2812FX::mode_strobe(void)
{
  return blink(SEGCOLOR(0), SEGCOLOR(1), true, true);
}

/*
 * Classic Strobe effect. Cycling through the rainbow.
 */
uint16_t WS2812FX::mode_strobe_rainbow(void)
{
  return blink(color_wheel(SEGENV.call & 0xFF), SEGCOLOR(1), true, false);
}

/*
 * Color wipe function
 * LEDs are turned on (color1) in sequence, then turned off (color2) in sequence.
 * if (bool rev == true) then LEDs are turned off in reverse order
 */
uint16_t WS2812FX::color_wipe(bool rev, bool useRandomColors)
{
  uint32_t cycleTime = 750 + (255 - SEGMENT.speed) * 150;
  uint32_t perc = now % cycleTime;
  uint16_t prog = (perc * 65535) / cycleTime;
  bool back = (prog > 32767);
  if (back)
  {
    prog -= 32767;
    if (SEGENV.step == 0)
      SEGENV.step = 1;
  }
  else
  {
    if (SEGENV.step == 2)
      SEGENV.step = 3; // trigger color change
  }

  if (useRandomColors)
  {
    if (SEGENV.call == 0)
    {
      SEGENV.aux0 = random8();
      SEGENV.step = 3;
    }
    if (SEGENV.step == 1)
    { // if flag set, change to new random color
      SEGENV.aux1 = get_random_wheel_index(SEGENV.aux0);
      SEGENV.step = 2;
    }
    if (SEGENV.step == 3)
    {
      SEGENV.aux0 = get_random_wheel_index(SEGENV.aux1);
      SEGENV.step = 0;
    }
  }

  uint16_t ledIndex = (prog * SEGLEN) >> 15;
  uint16_t rem = 0;
  rem = (prog * SEGLEN) * 2; // mod 0xFFFF
  rem /= (SEGMENT.intensity + 1);
  if (rem > 255)
    rem = 255;

  uint32_t col1 = useRandomColors ? color_wheel(SEGENV.aux1) : SEGCOLOR(1);
  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    uint16_t index = (rev && back) ? SEGLEN - 1 - i : i;
    uint32_t col0 = useRandomColors ? color_wheel(SEGENV.aux0) : color_from_palette(index, true, PALETTE_SOLID_WRAP, 0);

    if (i < ledIndex)
    {
      setPixelColor(index, back ? col1 : col0);
    }
    else
    {
      setPixelColor(index, back ? col0 : col1);
      if (i == ledIndex)
        setPixelColor(index, color_blend(back ? col0 : col1, back ? col1 : col0, rem));
    }
  }
  return FRAMETIME;
}

/*
 * Lights all LEDs one after another.
 */
uint16_t WS2812FX::mode_color_wipe(void)
{
  return color_wipe(false, false);
}

/*
 * Lights all LEDs one after another. Turns off opposite
 */
uint16_t WS2812FX::mode_color_sweep(void)
{
  return color_wipe(true, false);
}

/*
 * Turns all LEDs after each other to a random color.
 * Then starts over with another color.
 */
uint16_t WS2812FX::mode_color_wipe_random(void)
{
  return color_wipe(false, true);
}

/*
 * Random color introduced alternating from start and end of strip.
 */
uint16_t WS2812FX::mode_color_sweep_random(void)
{
  return color_wipe(true, true);
}

/*
 * Lights all LEDs in one random color up. Then switches them
 * to the next random color.
 */
uint16_t WS2812FX::mode_random_color(void)
{
  uint32_t cycleTime = 200 + (255 - SEGMENT.speed) * 50;
  uint32_t it = now / cycleTime;
  uint32_t rem = now % cycleTime;
  uint16_t fadedur = (cycleTime * SEGMENT.intensity) >> 8;

  uint32_t fade = 255;
  if (fadedur)
  {
    fade = (rem * 255) / fadedur;
    if (fade > 255)
      fade = 255;
  }

  if (SEGENV.call == 0)
  {
    SEGENV.aux0 = random8();
    SEGENV.step = 2;
  }
  if (it != SEGENV.step) // new color
  {
    SEGENV.aux1 = SEGENV.aux0;
    SEGENV.aux0 = get_random_wheel_index(SEGENV.aux0); // aux0 will store our random color wheel index
    SEGENV.step = it;
  }

  fill(color_blend(color_wheel(SEGENV.aux1), color_wheel(SEGENV.aux0), fade));
  return FRAMETIME;
}

/*
 * Lights every LED in a random color. Changes all LED at the same time
 * to new random colors.
 */
uint16_t WS2812FX::dynamic(boolean smooth = false)
{
  if (!SEGENV.allocateData(SEGLEN))
    return mode_static(); // allocation failed

  if (SEGENV.call == 0)
  {
    for (uint16_t i = 0; i < SEGLEN; i++)
      SEGENV.data[i] = random8();
  }

  uint32_t cycleTime = 50 + (255 - SEGMENT.speed) * 15;
  uint32_t it = now / cycleTime;
  if (it != SEGENV.step && SEGMENT.speed != 0) // new color
  {
    for (uint16_t i = 0; i < SEGLEN; i++)
    {
      if (random8() <= SEGMENT.intensity)
        SEGENV.data[i] = random8();
    }
    SEGENV.step = it;
  }

  if (smooth)
  {
    for (uint16_t i = 0; i < SEGLEN; i++)
    {
      blendPixelColor(i, color_wheel(SEGENV.data[i]), 16);
    }
  }
  else
  {
    for (uint16_t i = 0; i < SEGLEN; i++)
    {
      setPixelColor(i, color_wheel(SEGENV.data[i]));
    }
  }
  return FRAMETIME;
}

/*
 * Original effect "Dynamic"
 */
uint16_t WS2812FX::mode_dynamic(void)
{
  return dynamic(false);
}

/*
 * effect "Dynamic" with smoth color-fading
 */
uint16_t WS2812FX::mode_dynamic_smooth(void)
{
  return dynamic(true);
}

/*
 * Does the "standby-breathing" of well known i-Devices.
 */
uint16_t WS2812FX::mode_breath(void)
{
  uint16_t var = 0;
  uint16_t counter = (now * ((SEGMENT.speed >> 3) + 10));
  counter = (counter >> 2) + (counter >> 4); // 0-16384 + 0-2048
  if (counter < 16384)
  {
    if (counter > 8192)
      counter = 8192 - (counter - 8192);
    var = sin16(counter) / 103; // close to parabolic in range 0-8192, max val. 23170
  }

  uint8_t lum = 30 + var;
  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    setPixelColor(i, color_blend(SEGCOLOR(1), color_from_palette(i, true, PALETTE_SOLID_WRAP, 0), lum));
  }

  return FRAMETIME;
}

/*
 * Fades the LEDs between two colors
 */
uint16_t WS2812FX::mode_fade(void)
{
  uint16_t counter = (now * ((SEGMENT.speed >> 3) + 10));
  uint8_t lum = triwave16(counter) >> 8;

  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    setPixelColor(i, color_blend(SEGCOLOR(1), color_from_palette(i, true, PALETTE_SOLID_WRAP, 0), lum));
  }

  return FRAMETIME;
}

/*
 * Scan mode parent function
 */
uint16_t WS2812FX::scan(bool dual)
{
  uint32_t cycleTime = 750 + (255 - SEGMENT.speed) * 150;
  uint32_t perc = now % cycleTime;
  uint16_t prog = (perc * 65535) / cycleTime;
  uint16_t size = 1 + ((SEGMENT.intensity * SEGLEN) >> 9);
  uint16_t ledIndex = (prog * ((SEGLEN * 2) - size * 2)) >> 16;

  fill(SEGCOLOR(1));

  int led_offset = ledIndex - (SEGLEN - size);
  led_offset = abs(led_offset);

  if (dual)
  {
    for (uint16_t j = led_offset; j < led_offset + size; j++)
    {
      uint16_t i2 = SEGLEN - 1 - j;
      setPixelColor(i2, color_from_palette(i2, true, PALETTE_SOLID_WRAP, (SEGCOLOR(2)) ? 2 : 0));
    }
  }

  for (uint16_t j = led_offset; j < led_offset + size; j++)
  {
    setPixelColor(j, color_from_palette(j, true, PALETTE_SOLID_WRAP, 0));
  }

  return FRAMETIME;
}

/*
 * Runs a single pixel back and forth.
 */
uint16_t WS2812FX::mode_scan(void)
{
  return scan(false);
}

/*
 * Runs two pixel back and forth in opposite directions.
 */
uint16_t WS2812FX::mode_dual_scan(void)
{
  return scan(true);
}

/*
 * Cycles all LEDs at once through a rainbow.
 */
uint16_t WS2812FX::mode_rainbow(void)
{
  uint16_t counter = (now * ((SEGMENT.speed >> 2) + 2)) & 0xFFFF;
  counter = counter >> 8;

  if (SEGMENT.intensity < 128)
  {
    fill(color_blend(color_wheel(counter), WHITE, 128 - SEGMENT.intensity));
  }
  else
  {
    fill(color_wheel(counter));
  }

  return FRAMETIME;
}

/*
 * Cycles a rainbow over the entire string of LEDs.
 */
uint16_t WS2812FX::mode_rainbow_cycle(void)
{
  uint16_t counter = (now * ((SEGMENT.speed >> 2) + 2)) & 0xFFFF;
  counter = counter >> 8;

  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    // intensity/29 = 0 (1/16) 1 (1/8) 2 (1/4) 3 (1/2) 4 (1) 5 (2) 6 (4) 7 (8) 8 (16)
    uint8_t index = (i * (16 << (SEGMENT.intensity / 29)) / SEGLEN) + counter;
    setPixelColor(i, color_wheel(index));
  }

  return FRAMETIME;
}

/*
 * Theatre-style crawling lights.
 * Inspired by the Adafruit examples.
 */
uint16_t WS2812FX::mode_theater_chase(void)
{
  return running(SEGCOLOR(0), SEGCOLOR(1), true);
}

/*
 * Theatre-style crawling lights with rainbow effect.
 * Inspired by the Adafruit examples.
 */
uint16_t WS2812FX::mode_theater_chase_rainbow(void)
{
  return running(color_wheel(SEGENV.step), SEGCOLOR(1), true);
}

/*
 * Running lights effect with smooth sine transition base.
 */
uint16_t WS2812FX::running_base(bool saw, bool dual = false)
{
  uint8_t x_scale = SEGMENT.intensity >> 2;
  uint32_t counter = (now * SEGMENT.speed) >> 9;

  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    uint16_t a = i * x_scale - counter;
    if (saw)
    {
      a &= 0xFF;
      if (a < 16)
      {
        a = 192 + a * 8;
      }
      else
      {
        a = map(a, 16, 255, 64, 192);
      }
      a = 255 - a;
    }
    uint8_t s = dual ? sin_gap(a) : sin8(a);
    uint32_t ca = color_blend(SEGCOLOR(1), color_from_palette(i, true, PALETTE_SOLID_WRAP, 0), s);
    if (dual)
    {
      uint16_t b = (SEGLEN - 1 - i) * x_scale - counter;
      uint8_t t = sin_gap(b);
      uint32_t cb = color_blend(SEGCOLOR(1), color_from_palette(i, true, PALETTE_SOLID_WRAP, 2), t);
      ca = color_blend(ca, cb, 127);
    }
    setPixelColor(i, ca);
  }
  return FRAMETIME;
}

/*
 * Running lights in opposite directions.
 * Idea: Make the gap width controllable with a third slider in the future
 */
uint16_t WS2812FX::mode_running_dual(void)
{
  return running_base(false, true);
}

/*
 * Running lights effect with smooth sine transition.
 */
uint16_t WS2812FX::mode_running_lights(void)
{
  return running_base(false);
}

/*
 * Running lights effect with sawtooth transition.
 */
uint16_t WS2812FX::mode_saw(void)
{
  return running_base(true);
}

/*
 * Blink several LEDs in random colors on, reset, repeat.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
uint16_t WS2812FX::mode_twinkle(void)
{
  fill(SEGCOLOR(1));

  uint32_t cycleTime = 20 + (255 - SEGMENT.speed) * 5;
  uint32_t it = now / cycleTime;
  if (it != SEGENV.step)
  {
    uint16_t maxOn = map(SEGMENT.intensity, 0, 255, 1, SEGLEN); // make sure at least one LED is on
    if (SEGENV.aux0 >= maxOn)
    {
      SEGENV.aux0 = 0;
      SEGENV.aux1 = random16(); // new seed for our PRNG
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
    //    setPixelColor(j, color_from_palette(j, true, PALETTE_SOLID_WRAP, 0));
    setPixelColor(j, color_blend(SEGCOLOR(1), color_from_palette(j, true, PALETTE_SOLID_WRAP, 0), 255)); // This supports RGBW.
  }

  return FRAMETIME;
}

/*
 * Dissolve function
 */
uint16_t WS2812FX::dissolve(uint32_t color)
{
  bool wa = (SEGCOLOR(1) != 0 && _brightness < 255); // workaround, can't compare getPixel to color if not full brightness

  for (uint16_t j = 0; j <= SEGLEN / 15; j++)
  {
    if (random8() <= SEGMENT.intensity)
    {
      for (uint8_t times = 0; times < 10; times++) // attempt to spawn a new pixel 5 times
      {
        uint16_t i = random16(SEGLEN);
        if (SEGENV.aux0)
        { // dissolve to primary/palette
          if (getPixelColor(i) == SEGCOLOR(1) || wa)
          {
            if (color == SEGCOLOR(0))
            {
              setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));
            }
            else
            {
              setPixelColor(i, color);
            }
            break; // only spawn 1 new pixel per frame per 50 LEDs
          }
        }
        else
        { // dissolve to secondary
          if (getPixelColor(i) != SEGCOLOR(1))
          {
            setPixelColor(i, SEGCOLOR(1));
            break;
          }
        }
      }
    }
  }

  if (SEGENV.call > (255 - SEGMENT.speed) + 15U)
  {
    SEGENV.aux0 = !SEGENV.aux0;
    SEGENV.call = 0;
  }

  return FRAMETIME;
}

/*
 * Blink several LEDs on and then off
 */
uint16_t WS2812FX::mode_dissolve(void)
{
  return dissolve(SEGCOLOR(0));
}

/*
 * Blink several LEDs on and then off in random colors
 */
uint16_t WS2812FX::mode_dissolve_random(void)
{
  return dissolve(color_wheel(random8()));
}

/*
 * Blinks one LED at a time.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
uint16_t WS2812FX::mode_sparkle(void)
{
  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 1));
  }
  uint32_t cycleTime = 10 + (255 - SEGMENT.speed) * 2;
  uint32_t it = now / cycleTime;
  if (it != SEGENV.step)
  {
    SEGENV.aux0 = random16(SEGLEN); // aux0 stores the random led index
    SEGENV.step = it;
  }

  setPixelColor(SEGENV.aux0, SEGCOLOR(0));
  return FRAMETIME;
}

/*
 * Lights all LEDs in the color. Flashes single col 1 pixels randomly. (List name: Sparkle Dark)
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
uint16_t WS2812FX::mode_flash_sparkle(void)
{
  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));
  }

  if (now - SEGENV.aux0 > SEGENV.step)
  {
    if (random8((255 - SEGMENT.intensity) >> 4) == 0)
    {
      setPixelColor(random16(SEGLEN), SEGCOLOR(1)); // flash
    }
    SEGENV.step = now;
    SEGENV.aux0 = 255 - SEGMENT.speed;
  }
  return FRAMETIME;
}

/*
 * Like flash sparkle. With more flash.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
uint16_t WS2812FX::mode_hyper_sparkle(void)
{
  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));
  }

  if (now - SEGENV.aux0 > SEGENV.step)
  {
    if (random8((255 - SEGMENT.intensity) >> 4) == 0)
    {
      for (uint16_t i = 0; i < MAX(1, SEGLEN / 3); i++)
      {
        setPixelColor(random16(SEGLEN), SEGCOLOR(1));
      }
    }
    SEGENV.step = now;
    SEGENV.aux0 = 255 - SEGMENT.speed;
  }
  return FRAMETIME;
}

/*
 * Strobe effect with different strobe count and pause, controlled by speed.
 */
uint16_t WS2812FX::mode_multi_strobe(void)
{
  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 1));
  }

  SEGENV.aux0 = 50 + 20 * (uint16_t)(255 - SEGMENT.speed);
  uint16_t count = 2 * ((SEGMENT.intensity / 10) + 1);
  if (SEGENV.aux1 < count)
  {
    if ((SEGENV.aux1 & 1) == 0)
    {
      fill(SEGCOLOR(0));
      SEGENV.aux0 = 15;
    }
    else
    {
      SEGENV.aux0 = 50;
    }
  }

  if (now - SEGENV.aux0 > SEGENV.step)
  {
    SEGENV.aux1++;
    if (SEGENV.aux1 > count)
      SEGENV.aux1 = 0;
    SEGENV.step = now;
  }

  return FRAMETIME;
}

/*
 * Android loading circle
 */
uint16_t WS2812FX::mode_android(void)
{

  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 1));
  }

  if (SEGENV.aux1 > ((float)SEGMENT.intensity / 255.0) * (float)SEGLEN)
  {
    SEGENV.aux0 = 1;
  }
  else
  {
    if (SEGENV.aux1 < 2)
      SEGENV.aux0 = 0;
  }

  uint16_t a = SEGENV.step;

  if (SEGENV.aux0 == 0)
  {
    if (SEGENV.call % 3 == 1)
    {
      a++;
    }
    else
    {
      SEGENV.aux1++;
    }
  }
  else
  {
    a++;
    if (SEGENV.call % 3 != 1)
      SEGENV.aux1--;
  }

  if (a >= SEGLEN)
    a = 0;

  if (a + SEGENV.aux1 < SEGLEN)
  {
    for (int i = a; i < a + SEGENV.aux1; i++)
    {
      setPixelColor(i, SEGCOLOR(0));
    }
  }
  else
  {
    for (int i = a; i < SEGLEN; i++)
    {
      setPixelColor(i, SEGCOLOR(0));
    }
    for (int i = 0; i < SEGENV.aux1 - (SEGLEN - a); i++)
    {
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
uint16_t WS2812FX::chase(uint32_t color1, uint32_t color2, uint32_t color3, bool do_palette)
{
  uint16_t counter = now * ((SEGMENT.speed >> 2) + 1);
  uint16_t a = counter * SEGLEN >> 16;

  bool chase_random = (SEGMENT.mode == FX_MODE_CHASE_RANDOM);
  if (chase_random)
  {
    if (a < SEGENV.step) // we hit the start again, choose new color for Chase random
    {
      SEGENV.aux1 = SEGENV.aux0; // store previous random color
      SEGENV.aux0 = get_random_wheel_index(SEGENV.aux0);
    }
    color1 = color_wheel(SEGENV.aux0);
  }
  SEGENV.step = a;

  // Use intensity setting to vary chase up to 1/2 string length
  uint8_t size = 1 + (SEGMENT.intensity * SEGLEN >> 10);

  uint16_t b = a + size; //"trail" of chase, filled with color1
  if (b > SEGLEN)
    b -= SEGLEN;
  uint16_t c = b + size;
  if (c > SEGLEN)
    c -= SEGLEN;

  // background
  if (do_palette)
  {
    for (uint16_t i = 0; i < SEGLEN; i++)
    {
      setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 1));
    }
  }
  else
    fill(color1);

  // if random, fill old background between a and end
  if (chase_random)
  {
    color1 = color_wheel(SEGENV.aux1);
    for (uint16_t i = a; i < SEGLEN; i++)
      setPixelColor(i, color1);
  }

  // fill between points a and b with color2
  if (a < b)
  {
    for (uint16_t i = a; i < b; i++)
      setPixelColor(i, color2);
  }
  else
  {
    for (uint16_t i = a; i < SEGLEN; i++) // fill until end
      setPixelColor(i, color2);
    for (uint16_t i = 0; i < b; i++) // fill from start until b
      setPixelColor(i, color2);
  }

  // fill between points b and c with color2
  if (b < c)
  {
    for (uint16_t i = b; i < c; i++)
      setPixelColor(i, color3);
  }
  else
  {
    for (uint16_t i = b; i < SEGLEN; i++) // fill until end
      setPixelColor(i, color3);
    for (uint16_t i = 0; i < c; i++) // fill from start until c
      setPixelColor(i, color3);
  }

  return FRAMETIME;
}

/*
 * Bicolor chase, more primary color.
 */
uint16_t WS2812FX::mode_chase_color(void)
{
  return chase(SEGCOLOR(1), (SEGCOLOR(2)) ? SEGCOLOR(2) : SEGCOLOR(0), SEGCOLOR(0), true);
}

/*
 * Primary running followed by random color.
 */
uint16_t WS2812FX::mode_chase_random(void)
{
  return chase(SEGCOLOR(1), (SEGCOLOR(2)) ? SEGCOLOR(2) : SEGCOLOR(0), SEGCOLOR(0), false);
}

/*
 * Primary, secondary running on rainbow.
 */
uint16_t WS2812FX::mode_chase_rainbow(void)
{
  uint8_t color_sep = 256 / SEGLEN;
  if (color_sep == 0)
    color_sep = 1; // correction for segments longer than 256 LEDs
  uint8_t color_index = SEGENV.call & 0xFF;
  uint32_t color = color_wheel(((SEGENV.step * color_sep) + color_index) & 0xFF);

  return chase(color, SEGCOLOR(0), SEGCOLOR(1), false);
}

/*
 * Primary running on rainbow.
 */
uint16_t WS2812FX::mode_chase_rainbow_white(void)
{
  uint16_t n = SEGENV.step;
  uint16_t m = (SEGENV.step + 1) % SEGLEN;
  uint32_t color2 = color_wheel(((n * 256 / SEGLEN) + (SEGENV.call & 0xFF)) & 0xFF);
  uint32_t color3 = color_wheel(((m * 256 / SEGLEN) + (SEGENV.call & 0xFF)) & 0xFF);

  return chase(SEGCOLOR(0), color2, color3, false);
}

/*
 * Red - Amber - Green - Blue lights running
 */
uint16_t WS2812FX::mode_colorful(void)
{
  uint8_t numColors = 4; // 3, 4, or 5
  uint32_t cols[9]{0x00FF0000, 0x00EEBB00, 0x0000EE00, 0x000077CC};
  if (SEGMENT.intensity > 160 || SEGMENT.palette)
  { // palette or color
    if (!SEGMENT.palette)
    {
      numColors = 3;
      for (uint8_t i = 0; i < 3; i++)
        cols[i] = SEGCOLOR(i);
    }
    else
    {
      uint16_t fac = 80;
      if (SEGMENT.palette == 52)
      {
        numColors = 5;
        fac = 61;
      } // C9 2 has 5 colors
      for (uint8_t i = 0; i < numColors; i++)
      {
        cols[i] = color_from_palette(i * fac, false, true, 255);
      }
    }
  }
  else if (SEGMENT.intensity < 80) // pastel (easter) colors
  {
    cols[0] = 0x00FF8040;
    cols[1] = 0x00E5D241;
    cols[2] = 0x0077FF77;
    cols[3] = 0x0077F0F0;
  }
  for (uint8_t i = numColors; i < numColors * 2 - 1; i++)
    cols[i] = cols[i - numColors];

  uint32_t cycleTime = 50 + (8 * (uint32_t)(255 - SEGMENT.speed));
  uint32_t it = now / cycleTime;
  if (it != SEGENV.step)
  {
    if (SEGMENT.speed > 0)
      SEGENV.aux0++;
    if (SEGENV.aux0 >= numColors)
      SEGENV.aux0 = 0;
    SEGENV.step = it;
  }

  for (uint16_t i = 0; i < SEGLEN; i += numColors)
  {
    for (uint16_t j = 0; j < numColors; j++)
      setPixelColor(i + j, cols[SEGENV.aux0 + j]);
  }

  return FRAMETIME;
}

/*
 * Emulates a traffic light.
 */
uint16_t WS2812FX::mode_traffic_light(void)
{
  for (uint16_t i = 0; i < SEGLEN; i++)
    setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 1));
  uint32_t mdelay = 500;
  for (int i = 0; i < SEGLEN - 2; i += 3)
  {
    switch (SEGENV.aux0)
    {
    case 0:
      setPixelColor(i, 0x00FF0000);
      mdelay = 150 + (100 * (uint32_t)(255 - SEGMENT.speed));
      break;
    case 1:
      setPixelColor(i, 0x00FF0000);
      mdelay = 150 + (20 * (uint32_t)(255 - SEGMENT.speed));
      setPixelColor(i + 1, 0x00EECC00);
      break;
    case 2:
      setPixelColor(i + 2, 0x0000FF00);
      mdelay = 150 + (100 * (uint32_t)(255 - SEGMENT.speed));
      break;
    case 3:
      setPixelColor(i + 1, 0x00EECC00);
      mdelay = 150 + (20 * (uint32_t)(255 - SEGMENT.speed));
      break;
    }
  }

  if (now - SEGENV.step > mdelay)
  {
    SEGENV.aux0++;
    if (SEGENV.aux0 == 1 && SEGMENT.intensity > 140)
      SEGENV.aux0 = 2; // skip Red + Amber, to get US-style sequence
    if (SEGENV.aux0 > 3)
      SEGENV.aux0 = 0;
    SEGENV.step = now;
  }

  return FRAMETIME;
}

/*
 * Sec flashes running on prim.
 */
#define FLASH_COUNT 4
uint16_t WS2812FX::mode_chase_flash(void)
{
  uint8_t flash_step = SEGENV.call % ((FLASH_COUNT * 2) + 1);

  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));
  }

  uint16_t delay = 10 + ((30 * (uint16_t)(255 - SEGMENT.speed)) / SEGLEN);
  if (flash_step < (FLASH_COUNT * 2))
  {
    if (flash_step % 2 == 0)
    {
      uint16_t n = SEGENV.step;
      uint16_t m = (SEGENV.step + 1) % SEGLEN;
      setPixelColor(n, SEGCOLOR(1));
      setPixelColor(m, SEGCOLOR(1));
      delay = 20;
    }
    else
    {
      delay = 30;
    }
  }
  else
  {
    SEGENV.step = (SEGENV.step + 1) % SEGLEN;
  }
  return delay;
}

/*
 * Prim flashes running, followed by random color.
 */
uint16_t WS2812FX::mode_chase_flash_random(void)
{
  uint8_t flash_step = SEGENV.call % ((FLASH_COUNT * 2) + 1);

  for (uint16_t i = 0; i < SEGENV.step; i++)
  {
    setPixelColor(i, color_wheel(SEGENV.aux0));
  }

  uint16_t delay = 1 + ((10 * (uint16_t)(255 - SEGMENT.speed)) / SEGLEN);
  if (flash_step < (FLASH_COUNT * 2))
  {
    uint16_t n = SEGENV.step;
    uint16_t m = (SEGENV.step + 1) % SEGLEN;
    if (flash_step % 2 == 0)
    {
      setPixelColor(n, SEGCOLOR(0));
      setPixelColor(m, SEGCOLOR(0));
      delay = 20;
    }
    else
    {
      setPixelColor(n, color_wheel(SEGENV.aux0));
      setPixelColor(m, SEGCOLOR(1));
      delay = 30;
    }
  }
  else
  {
    SEGENV.step = (SEGENV.step + 1) % SEGLEN;

    if (SEGENV.step == 0)
    {
      SEGENV.aux0 = get_random_wheel_index(SEGENV.aux0);
    }
  }
  return delay;
}

/*
 * Alternating pixels running function.
 */
uint16_t WS2812FX::running(uint32_t color1, uint32_t color2, bool theatre)
{
  uint8_t width = (theatre ? 3 : 1) + (SEGMENT.intensity >> 4); // window
  uint32_t cycleTime = 50 + (255 - SEGMENT.speed);
  uint32_t it = now / cycleTime;
  bool usePalette = color1 == SEGCOLOR(0);

  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    uint32_t col = color2;
    if (usePalette)
      color1 = color_from_palette(i, true, PALETTE_SOLID_WRAP, 0);
    if (theatre)
    {
      if ((i % width) == SEGENV.aux0)
        col = color1;
    }
    else
    {
      int8_t pos = (i % (width << 1));
      if ((pos < SEGENV.aux0 - width) || ((pos >= SEGENV.aux0) && (pos < SEGENV.aux0 + width)))
        col = color1;
    }
    setPixelColor(i, col);
  }

  if (it != SEGENV.step)
  {
    SEGENV.aux0 = (SEGENV.aux0 + 1) % (theatre ? width : (width << 1));
    SEGENV.step = it;
  }
  return FRAMETIME;
}

/*
 * Alternating color/sec pixels running.
 */
uint16_t WS2812FX::mode_running_color(void)
{
  return running(SEGCOLOR(0), SEGCOLOR(1));
}

/*
 * Alternating red/white pixels running.
 */
uint16_t WS2812FX::mode_candy_cane(void)
{
  return running(RED, WHITE);
}

/*
 * Alternating orange/purple pixels running.
 */
uint16_t WS2812FX::mode_halloween(void)
{
  return running(PURPLE, ORANGE);
}

/*
 * Random colored pixels running. ("Stream")
 */
uint16_t WS2812FX::mode_running_random(void)
{
  uint32_t cycleTime = 25 + (3 * (uint32_t)(255 - SEGMENT.speed));
  uint32_t it = now / cycleTime;
  if (SEGENV.call == 0)
    SEGENV.aux0 = random16(); // random seed for PRNG on start

  uint8_t zoneSize = ((255 - SEGMENT.intensity) >> 4) + 1;
  uint16_t PRNG16 = SEGENV.aux0;

  uint8_t z = it % zoneSize;
  bool nzone = (!z && it != SEGENV.aux1);
  for (int i = SEGLEN - 1; i >= 0; i--)
  { // WLEDSR bugfix
    if (nzone || z >= zoneSize)
    {
      uint8_t lastrand = PRNG16 >> 8;
      int16_t diff = 0;
      while (abs(diff) < 42)
      {                                             // make sure the difference between adjacent colors is big enough
        PRNG16 = (uint16_t)(PRNG16 * 2053) + 13849; // next zone, next 'random' number
        diff = (PRNG16 >> 8) - lastrand;
      }
      if (nzone)
      {
        SEGENV.aux0 = PRNG16; // save next starting seed
        nzone = false;
      }
      z = 0;
    }
    setPixelColor(i, color_wheel(PRNG16 >> 8));
    z++;
  }

  SEGENV.aux1 = it;
  return FRAMETIME;
}

/*
 * K.I.T.T.
 */
uint16_t WS2812FX::mode_larson_scanner(void)
{
  return larson_scanner(false);
}

uint16_t WS2812FX::larson_scanner(bool dual)
{
  uint16_t counter = now * ((SEGMENT.speed >> 2) + 8);
  uint16_t index = counter * SEGLEN >> 16;

  fade_out(SEGMENT.intensity);

  if (SEGENV.step > index && SEGENV.step - index > SEGLEN / 2)
  {
    SEGENV.aux0 = !SEGENV.aux0;
  }

  for (uint16_t i = SEGENV.step; i < index; i++)
  {
    uint16_t j = (SEGENV.aux0) ? i : SEGLEN - 1 - i;
    setPixelColor(j, color_from_palette(j, true, PALETTE_SOLID_WRAP, 0));
  }
  if (dual)
  {
    uint32_t c;
    if (SEGCOLOR(2) != 0)
    {
      c = SEGCOLOR(2);
    }
    else
    {
      c = color_from_palette(index, true, PALETTE_SOLID_WRAP, 0);
    }

    for (uint16_t i = SEGENV.step; i < index; i++)
    {
      uint16_t j = (SEGENV.aux0) ? SEGLEN - 1 - i : i;
      setPixelColor(j, c);
    }
  }

  SEGENV.step = index;
  return FRAMETIME;
}

/*
 * Firing comets from one end. "Lighthouse"
 */
uint16_t WS2812FX::mode_comet(void)
{
  uint16_t counter = now * ((SEGMENT.speed >> 2) + 1);
  uint16_t index = counter * SEGLEN >> 16;
  if (SEGENV.call == 0)
    SEGENV.aux0 = index;

  fade_out(SEGMENT.intensity);

  setPixelColor(index, color_from_palette(index, true, PALETTE_SOLID_WRAP, 0));
  if (index > SEGENV.aux0)
  {
    for (uint16_t i = SEGENV.aux0; i < index; i++)
    {
      setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));
    }
  }
  else if (index < SEGENV.aux0 && index < 10)
  {
    for (uint16_t i = 0; i < index; i++)
    {
      setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));
    }
  }
  SEGENV.aux0 = index++;

  return FRAMETIME;
}

/*
 * Fireworks function.
 */
uint16_t WS2812FX::mode_fireworks()
{
  fade_out(0);
  if (SEGENV.call == 0)
  {
    SEGENV.aux0 = UINT16_MAX;
    SEGENV.aux1 = UINT16_MAX;
  }
  bool valid1 = (SEGENV.aux0 < SEGLEN);
  bool valid2 = (SEGENV.aux1 < SEGLEN);
  uint32_t sv1 = 0, sv2 = 0;
  if (valid1)
    sv1 = getPixelColor(SEGENV.aux0);
  if (valid2)
    sv2 = getPixelColor(SEGENV.aux1);
  blur(255 - SEGMENT.speed);
  if (valid1)
    setPixelColor(SEGENV.aux0, sv1);
  if (valid2)
    setPixelColor(SEGENV.aux1, sv2);

  for (uint16_t i = 0; i < MAX(1, SEGLEN / 20); i++)
  {
    if (random8(129 - (SEGMENT.intensity >> 1)) == 0)
    {
      uint16_t index = random(SEGLEN);
      setPixelColor(index, color_from_palette(random8(), false, false, 0));
      SEGENV.aux1 = SEGENV.aux0;
      SEGENV.aux0 = index;
    }
  }
  return FRAMETIME;
}

// Twinkling LEDs running. Inspired by https://github.com/kitesurfer1404/WS2812FX/blob/master/src/custom/Rain.h
uint16_t WS2812FX::mode_rain()
{
  SEGENV.step += FRAMETIME;
  if (SEGENV.step > SPEED_FORMULA_L)
  {
    SEGENV.step = 0;
    // shift all leds left
    uint32_t ctemp = getPixelColor(0);
    for (uint16_t i = 0; i < SEGLEN - 1; i++)
    {
      setPixelColor(i, getPixelColor(i + 1));
    }
    setPixelColor(SEGLEN - 1, ctemp);
    SEGENV.aux0++;
    SEGENV.aux1++;
    if (SEGENV.aux0 == 0)
      SEGENV.aux0 = UINT16_MAX;
    if (SEGENV.aux1 == 0)
      SEGENV.aux0 = UINT16_MAX;
    if (SEGENV.aux0 == SEGLEN)
      SEGENV.aux0 = 0;
    if (SEGENV.aux1 == SEGLEN)
      SEGENV.aux1 = 0;
  }
  return mode_fireworks();
}

/*
 * Fire flicker function
 */
uint16_t WS2812FX::mode_fire_flicker(void)
{
  uint32_t cycleTime = 40 + (255 - SEGMENT.speed);
  uint32_t it = now / cycleTime;
  if (SEGENV.step == it)
    return FRAMETIME;

  byte w = (SEGCOLOR(0) >> 24);
  byte r = (SEGCOLOR(0) >> 16);
  byte g = (SEGCOLOR(0) >> 8);
  byte b = (SEGCOLOR(0));
  byte lum = (SEGMENT.palette == 0) ? MAX(w, MAX(r, MAX(g, b))) : 255;
  lum /= (((256 - SEGMENT.intensity) / 16) + 1);
  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    byte flicker = random8(lum);
    if (SEGMENT.palette == 0)
    {
      setPixelColor(i, MAX(r - flicker, 0), MAX(g - flicker, 0), MAX(b - flicker, 0), MAX(w - flicker, 0));
    }
    else
    {
      setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0, 255 - flicker));
    }
  }

  SEGENV.step = it;
  return FRAMETIME;
}

/*
 * Gradient run base function
 */
uint16_t WS2812FX::gradient_base(bool loading)
{
  uint16_t counter = now * ((SEGMENT.speed >> 2) + 1);
  uint16_t pp = counter * SEGLEN >> 16;
  if (SEGENV.call == 0)
    pp = 0;
  float val; // 0.0 = sec 1.0 = pri
  float brd = loading ? SEGMENT.intensity : SEGMENT.intensity / 2;
  if (brd < 1.0)
    brd = 1.0;
  int p1 = pp - SEGLEN;
  int p2 = pp + SEGLEN;

  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    if (loading)
    {
      val = abs(((i > pp) ? p2 : pp) - i);
    }
    else
    {
      val = MIN(abs(pp - i), MIN(abs(p1 - i), abs(p2 - i)));
    }
    val = (brd > val) ? val / brd * 255 : 255;
    setPixelColor(i, color_blend(SEGCOLOR(0), color_from_palette(i, true, PALETTE_SOLID_WRAP, 1), val));
  }

  return FRAMETIME;
}

/*
 * Gradient run
 */
uint16_t WS2812FX::mode_gradient(void)
{
  return gradient_base(false);
}

/*
 * Gradient run with hard transition
 */
uint16_t WS2812FX::mode_loading(void)
{
  return gradient_base(true);
}

// American Police Light with all LEDs Red and Blue
uint16_t WS2812FX::police_base(uint32_t color1, uint32_t color2)
{
  uint16_t delay = 1 + (FRAMETIME << 3) / SEGLEN; // longer segments should change faster
  uint32_t it = now / map(SEGMENT.speed, 0, 255, delay << 4, delay);
  uint16_t offset = it % SEGLEN;

  uint16_t width = ((SEGLEN * (SEGMENT.intensity + 1)) >> 9); // max width is half the strip
  if (!width)
    width = 1;
  for (uint16_t i = 0; i < width; i++)
  {
    uint16_t indexR = (offset + i) % SEGLEN;
    uint16_t indexB = (offset + i + (SEGLEN >> 1)) % SEGLEN;
    setPixelColor(indexR, color1);
    setPixelColor(indexB, color2);
  }
  return FRAMETIME;
}

// Police Lights Red and Blue
uint16_t WS2812FX::mode_police()
{
  fill(SEGCOLOR(1));
  return police_base(RED, BLUE);
}

// Police Lights with custom colors
uint16_t WS2812FX::mode_two_dots()
{
  fill(SEGCOLOR(2));
  uint32_t color2 = (SEGCOLOR(1) == SEGCOLOR(2)) ? SEGCOLOR(0) : SEGCOLOR(1);

  return police_base(SEGCOLOR(0), color2);
}

/*
 * Fairy, inspired by https://www.youtube.com/watch?v=zeOw5MZWq24
 */
// 4 bytes
typedef struct Flasher
{
  uint16_t stateStart;
  uint8_t stateDur;
  bool stateOn;
} flasher;

#define FLASHERS_PER_ZONE 6
#define MAX_SHIMMER 92

uint16_t WS2812FX::mode_fairy()
{
  // set every pixel to a 'random' color from palette (using seed so it doesn't change between frames)
  uint16_t PRNG16 = 5100 + _segment_index;
  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    setPixelColor(i, color_from_palette(PRNG16 >> 8, false, false, 0));
  }

  // amount of flasher pixels depending on intensity (0: none, 255: every LED)
  if (SEGMENT.intensity == 0)
    return FRAMETIME;
  uint8_t flasherDistance = ((255 - SEGMENT.intensity) / 28) + 1; // 1-10
  uint16_t numFlashers = (SEGLEN / flasherDistance) + 1;

  uint16_t dataSize = sizeof(flasher) * numFlashers;
  if (!SEGENV.allocateData(dataSize))
    return FRAMETIME; // allocation failed
  Flasher *flashers = reinterpret_cast<Flasher *>(SEGENV.data);
  uint16_t now16 = now & 0xFFFF;

  // Up to 11 flashers in one brightness zone, afterwards a new zone for every 6 flashers
  uint16_t zones = numFlashers / FLASHERS_PER_ZONE;
  if (!zones)
    zones = 1;
  uint8_t flashersInZone = numFlashers / zones;
  uint8_t flasherBri[FLASHERS_PER_ZONE * 2 - 1];

  for (uint16_t z = 0; z < zones; z++)
  {
    uint16_t flasherBriSum = 0;
    uint16_t firstFlasher = z * flashersInZone;
    if (z == zones - 1)
      flashersInZone = numFlashers - (flashersInZone * (zones - 1));

    for (uint16_t f = firstFlasher; f < firstFlasher + flashersInZone; f++)
    {
      uint16_t stateTime = now16 - flashers[f].stateStart;
      // random on/off time reached, switch state
      if (stateTime > flashers[f].stateDur * 10)
      {
        flashers[f].stateOn = !flashers[f].stateOn;
        if (flashers[f].stateOn)
        {
          flashers[f].stateDur = 12 + random8(12 + ((255 - SEGMENT.speed) >> 2)); //*10, 250ms to 1250ms
        }
        else
        {
          flashers[f].stateDur = 20 + random8(6 + ((255 - SEGMENT.speed) >> 2)); //*10, 250ms to 1250ms
        }
        // flashers[f].stateDur = 51 + random8(2 + ((255 - SEGMENT.speed) >> 1));
        flashers[f].stateStart = now16;
        if (stateTime < 255)
        {
          flashers[f].stateStart -= 255 - stateTime; // start early to get correct bri
          flashers[f].stateDur += 26 - stateTime / 10;
          stateTime = 255 - stateTime;
        }
        else
        {
          stateTime = 0;
        }
      }
      if (stateTime > 255)
        stateTime = 255; // for flasher brightness calculation, fades in first 255 ms of state
      // flasherBri[f - firstFlasher] = (flashers[f].stateOn) ? 255-gamma8((510 - stateTime) >> 1) : gamma8((510 - stateTime) >> 1);
      flasherBri[f - firstFlasher] = (flashers[f].stateOn) ? stateTime : 255 - (stateTime >> 0);
      flasherBriSum += flasherBri[f - firstFlasher];
    }
    // dim factor, to create "shimmer" as other pixels get less voltage if a lot of flashers are on
    uint8_t avgFlasherBri = flasherBriSum / flashersInZone;
    uint8_t globalPeakBri = 255 - ((avgFlasherBri * MAX_SHIMMER) >> 8); // 183-255, suitable for 1/5th of LEDs flashers

    for (uint16_t f = firstFlasher; f < firstFlasher + flashersInZone; f++)
    {
      uint8_t bri = (flasherBri[f - firstFlasher] * globalPeakBri) / 255;
      PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
      uint16_t flasherPos = f * flasherDistance;
      setPixelColor(flasherPos, color_blend(SEGCOLOR(1), color_from_palette(PRNG16 >> 8, false, false, 0), bri));
      for (uint16_t i = flasherPos + 1; i < flasherPos + flasherDistance && i < SEGLEN; i++)
      {
        PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
        setPixelColor(i, color_from_palette(PRNG16 >> 8, false, false, 0, globalPeakBri));
      }
    }
  }
  return FRAMETIME;
}

/*
 * Fairytwinkle. Like Colortwinkle, but starting from all lit and not relying on getPixelColor
 * Warning: Uses 4 bytes of segment data per pixel
 */
uint16_t WS2812FX::mode_fairytwinkle()
{
  uint16_t dataSize = sizeof(flasher) * SEGLEN;
  if (!SEGENV.allocateData(dataSize))
    return mode_static(); // allocation failed
  Flasher *flashers = reinterpret_cast<Flasher *>(SEGENV.data);
  uint16_t now16 = now & 0xFFFF;
  uint16_t PRNG16 = 5100 + _segment_index;

  uint16_t riseFallTime = 400 + (255 - SEGMENT.speed) * 3;
  uint16_t maxDur = riseFallTime / 100 + ((255 - SEGMENT.intensity) >> 2) + 13 + ((255 - SEGMENT.intensity) >> 1);

  for (uint16_t f = 0; f < SEGLEN; f++)
  {
    uint16_t stateTime = now16 - flashers[f].stateStart;
    // random on/off time reached, switch state
    if (stateTime > flashers[f].stateDur * 100)
    {
      flashers[f].stateOn = !flashers[f].stateOn;
      bool init = !flashers[f].stateDur;
      if (flashers[f].stateOn)
      {
        flashers[f].stateDur = riseFallTime / 100 + ((255 - SEGMENT.intensity) >> 2) + random8(12 + ((255 - SEGMENT.intensity) >> 1)) + 1;
      }
      else
      {
        flashers[f].stateDur = riseFallTime / 100 + random8(3 + ((255 - SEGMENT.speed) >> 6)) + 1;
      }
      flashers[f].stateStart = now16;
      stateTime = 0;
      if (init)
      {
        flashers[f].stateStart -= riseFallTime;                                                         // start lit
        flashers[f].stateDur = riseFallTime / 100 + random8(12 + ((255 - SEGMENT.intensity) >> 1)) + 5; // fire up a little quicker
        stateTime = riseFallTime;
      }
    }
    if (flashers[f].stateOn && flashers[f].stateDur > maxDur)
      flashers[f].stateDur = maxDur; // react more quickly on intensity change
    if (stateTime > riseFallTime)
      stateTime = riseFallTime; // for flasher brightness calculation, fades in first 255 ms of state
    uint8_t fadeprog = 255 - ((stateTime * 255) / riseFallTime);
    uint8_t flasherBri = (flashers[f].stateOn) ? 255 - gamma8(fadeprog) : gamma8(fadeprog);
    uint16_t lastR = PRNG16;
    uint16_t diff = 0;
    while (diff < 0x4000)
    {                                            // make sure colors of two adjacent LEDs differ enough
      PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
      diff = (PRNG16 > lastR) ? PRNG16 - lastR : lastR - PRNG16;
    }
    setPixelColor(f, color_blend(SEGCOLOR(1), color_from_palette(PRNG16 >> 8, false, false, 0), flasherBri));
  }
  return FRAMETIME;
}

/*
 * Tricolor chase function
 */
uint16_t WS2812FX::tricolor_chase(uint32_t color1, uint32_t color2)
{
  uint32_t cycleTime = 50 + ((255 - SEGMENT.speed) << 1);
  uint32_t it = now / cycleTime;                  // iterator
  uint8_t width = (1 + (SEGMENT.intensity >> 4)); // value of 1-16 for each colour
  uint8_t index = it % (width * 3);

  for (uint16_t i = 0; i < SEGLEN; i++, index++)
  {
    if (index > (width * 3) - 1)
      index = 0;

    uint32_t color = color1;
    if (index > (width << 1) - 1)
      color = color_from_palette(i, true, PALETTE_SOLID_WRAP, 1);
    else if (index > width - 1)
      color = color2;

    setPixelColor(SEGLEN - i - 1, color);
  }
  return FRAMETIME;
}

/*
 * Tricolor chase mode
 */
uint16_t WS2812FX::mode_tricolor_chase(void)
{
  return tricolor_chase(SEGCOLOR(2), SEGCOLOR(0));
}

/*
 * ICU mode
 */
uint16_t WS2812FX::mode_icu(void)
{
  uint16_t dest = SEGENV.step & 0xFFFF;
  uint8_t space = (SEGMENT.intensity >> 3) + 2;

  fill(SEGCOLOR(1));

  byte pindex = map(dest, 0, SEGLEN - SEGLEN / space, 0, 255);
  uint32_t col = color_from_palette(pindex, false, false, 0);

  setPixelColor(dest, col);
  setPixelColor(dest + SEGLEN / space, col);

  if (SEGENV.aux0 == dest)
  { // pause between eye movements
    if (random8(6) == 0)
    { // blink once in a while
      setPixelColor(dest, SEGCOLOR(1));
      setPixelColor(dest + SEGLEN / space, SEGCOLOR(1));
      return 200;
    }
    SEGENV.aux0 = random16(SEGLEN - SEGLEN / space);
    return 1000 + random16(2000);
  }

  if (SEGENV.aux0 > SEGENV.step)
  {
    SEGENV.step++;
    dest++;
  }
  else if (SEGENV.aux0 < SEGENV.step)
  {
    SEGENV.step--;
    dest--;
  }

  setPixelColor(dest, col);
  setPixelColor(dest + SEGLEN / space, col);

  return SPEED_FORMULA_L;
}

/*
 * Custom mode by Aircoookie. Color Wipe, but with 3 colors
 */
uint16_t WS2812FX::mode_tricolor_wipe(void)
{
  uint32_t cycleTime = 1000 + (255 - SEGMENT.speed) * 200;
  uint32_t perc = now % cycleTime;
  uint16_t prog = (perc * 65535) / cycleTime;
  uint16_t ledIndex = (prog * SEGLEN * 3) >> 16;
  uint16_t ledOffset = ledIndex;

  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 2));
  }

  if (ledIndex < SEGLEN)
  { // wipe from 0 to 1
    for (uint16_t i = 0; i < SEGLEN; i++)
    {
      setPixelColor(i, (i > ledOffset) ? SEGCOLOR(0) : SEGCOLOR(1));
    }
  }
  else if (ledIndex < SEGLEN * 2)
  { // wipe from 1 to 2
    ledOffset = ledIndex - SEGLEN;
    for (uint16_t i = ledOffset + 1; i < SEGLEN; i++)
    {
      setPixelColor(i, SEGCOLOR(1));
    }
  }
  else // wipe from 2 to 0
  {
    ledOffset = ledIndex - SEGLEN * 2;
    for (uint16_t i = 0; i <= ledOffset; i++)
    {
      setPixelColor(i, SEGCOLOR(0));
    }
  }

  return FRAMETIME;
}

/*
 * Fades between 3 colors
 * Custom mode by Keith Lord: https://github.com/kitesurfer1404/WS2812FX/blob/master/src/custom/TriFade.h
 * Modified by Aircoookie
 */
uint16_t WS2812FX::mode_tricolor_fade(void)
{
  uint16_t counter = now * ((SEGMENT.speed >> 3) + 1);
  uint32_t prog = (counter * 768) >> 16;

  uint32_t color1 = 0, color2 = 0;
  byte stage = 0;

  if (prog < 256)
  {
    color1 = SEGCOLOR(0);
    color2 = SEGCOLOR(1);
    stage = 0;
  }
  else if (prog < 512)
  {
    color1 = SEGCOLOR(1);
    color2 = SEGCOLOR(2);
    stage = 1;
  }
  else
  {
    color1 = SEGCOLOR(2);
    color2 = SEGCOLOR(0);
    stage = 2;
  }

  byte stp = prog; // % 256
  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    uint32_t color;
    if (stage == 2)
    {
      color = color_blend(color_from_palette(i, true, PALETTE_SOLID_WRAP, 2), color2, stp);
    }
    else if (stage == 1)
    {
      color = color_blend(color1, color_from_palette(i, true, PALETTE_SOLID_WRAP, 2), stp);
    }
    else
    {
      color = color_blend(color1, color2, stp);
    }
    setPixelColor(i, color);
  }

  return FRAMETIME;
}

/*
 * Creates random comets
 * Custom mode by Keith Lord: https://github.com/kitesurfer1404/WS2812FX/blob/master/src/custom/MultiComet.h
 */
uint16_t WS2812FX::mode_multi_comet(void)
{
  uint32_t cycleTime = 10 + (uint32_t)(255 - SEGMENT.speed);
  uint32_t it = now / cycleTime;
  if (SEGENV.step == it)
    return FRAMETIME;
  if (!SEGENV.allocateData(sizeof(uint16_t) * 8))
    return mode_static(); // allocation failed

  fade_out(SEGMENT.intensity);

  uint16_t *comets = reinterpret_cast<uint16_t *>(SEGENV.data);

  for (uint8_t i = 0; i < 8; i++)
  {
    if (comets[i] < SEGLEN)
    {
      uint16_t index = comets[i];
      if (SEGCOLOR(2) != 0)
      {
        setPixelColor(index, i % 2 ? color_from_palette(index, true, PALETTE_SOLID_WRAP, 0) : SEGCOLOR(2));
      }
      else
      {
        setPixelColor(index, color_from_palette(index, true, PALETTE_SOLID_WRAP, 0));
      }
      comets[i]++;
    }
    else
    {
      if (!random(SEGLEN))
      {
        comets[i] = 0;
      }
    }
  }

  SEGENV.step = it;
  return FRAMETIME;
}

/*
 * Creates two Larson scanners moving in opposite directions
 * Custom mode by Keith Lord: https://github.com/kitesurfer1404/WS2812FX/blob/master/src/custom/DualLarson.h
 */
uint16_t WS2812FX::mode_dual_larson_scanner(void)
{
  return larson_scanner(true);
}

/*
 * Running random pixels ("Stream 2")
 * Custom mode by Keith Lord: https://github.com/kitesurfer1404/WS2812FX/blob/master/src/custom/RandomChase.h
 */
uint16_t WS2812FX::mode_random_chase(void)
{
  if (SEGENV.call == 0)
  {
    SEGENV.step = RGBW32(random8(), random8(), random8(), 0);
    SEGENV.aux0 = random16();
  }
  uint16_t prevSeed = random16_get_seed(); // save seed so we can restore it at the end of the function
  uint32_t cycleTime = 25 + (3 * (uint32_t)(255 - SEGMENT.speed));
  uint32_t it = now / cycleTime;
  uint32_t color = SEGENV.step;
  random16_set_seed(SEGENV.aux0);

  for (int i = SEGLEN - 1; i >= 0; i--)
  { // WLEDSR bugfix
    uint8_t r = random8(6) != 0 ? (color >> 16 & 0xFF) : random8();
    uint8_t g = random8(6) != 0 ? (color >> 8 & 0xFF) : random8();
    uint8_t b = random8(6) != 0 ? (color & 0xFF) : random8();
    color = RGBW32(r, g, b, 0);
    setPixelColor(i, r, g, b);
    if (i == SEGLEN - 1 && SEGENV.aux1 != (it & 0xFFFF))
    { // new first color in next frame
      SEGENV.step = color;
      SEGENV.aux0 = random16_get_seed();
    }
  }

  SEGENV.aux1 = it & 0xFFFF;

  random16_set_seed(prevSeed); // restore original seed so other effects can use "random" PRNG
  return FRAMETIME;
}

// 7 bytes
typedef struct Oscillator
{
  int16_t pos;
  int8_t size;
  int8_t dir;
  int8_t speed;
} oscillator;

/*
/  Oscillating bars of color, updated with standard framerate
*/
uint16_t WS2812FX::mode_oscillate(void)
{
  uint8_t numOscillators = 3;
  uint16_t dataSize = sizeof(oscillator) * numOscillators;

  if (!SEGENV.allocateData(dataSize))
    return mode_static(); // allocation failed

  Oscillator *oscillators = reinterpret_cast<Oscillator *>(SEGENV.data);

  if (SEGENV.call == 0)
  {
    oscillators[0] = {(int16_t)(SEGLEN / 4), (int8_t)(SEGLEN / 8), 1, 1};
    oscillators[1] = {(int16_t)(SEGLEN / 4 * 3), (int8_t)(SEGLEN / 8), 1, 2};
    oscillators[2] = {(int16_t)(SEGLEN / 4 * 2), (int8_t)(SEGLEN / 8), -1, 1};
  }

  uint32_t cycleTime = 20 + (2 * (uint32_t)(255 - SEGMENT.speed));
  uint32_t it = now / cycleTime;

  for (uint8_t i = 0; i < numOscillators; i++)
  {
    // if the counter has increased, move the oscillator by the random step
    if (it != SEGENV.step)
      oscillators[i].pos += oscillators[i].dir * oscillators[i].speed;
    oscillators[i].size = SEGLEN / (3 + SEGMENT.intensity / 8);
    if ((oscillators[i].dir == -1) && (oscillators[i].pos <= 0))
    {
      oscillators[i].pos = 0;
      oscillators[i].dir = 1;
      // make bigger steps for faster speeds
      oscillators[i].speed = SEGMENT.speed > 100 ? random8(2, 4) : random8(1, 3);
    }
    if ((oscillators[i].dir == 1) && (oscillators[i].pos >= (SEGLEN - 1)))
    {
      oscillators[i].pos = SEGLEN - 1;
      oscillators[i].dir = -1;
      oscillators[i].speed = SEGMENT.speed > 100 ? random8(2, 4) : random8(1, 3);
    }
  }

  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    uint32_t color = BLACK;
    for (uint8_t j = 0; j < numOscillators; j++)
    {
      if (i >= oscillators[j].pos - oscillators[j].size && i <= oscillators[j].pos + oscillators[j].size)
      {
        color = (color == BLACK) ? SEGCOLOR(j) : color_blend(color, SEGCOLOR(j), 128);
      }
    }
    setPixelColor(i, color);
  }

  SEGENV.step = it;
  return FRAMETIME;
}

uint16_t WS2812FX::mode_lightning(void)
{
  uint16_t ledstart = random16(SEGLEN);              // Determine starting location of flash
  uint16_t ledlen = 1 + random16(SEGLEN - ledstart); // Determine length of flash (not to go beyond NUM_LEDS-1)
  uint8_t bri = 255 / random8(1, 3);

  if (SEGENV.aux1 == 0) // init, leader flash
  {
    SEGENV.aux1 = random8(4, 4 + SEGMENT.intensity / 20); // number of flashes
    SEGENV.aux1 *= 2;

    bri = 52;          // leader has lower brightness
    SEGENV.aux0 = 200; // 200ms delay after leader
  }

  fill(SEGCOLOR(1));

  if (SEGENV.aux1 > 3 && !(SEGENV.aux1 & 0x01))
  { // flash on even number >2
    for (int i = ledstart; i < ledstart + ledlen; i++)
    {
      setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0, bri));
    }
    SEGENV.aux1--;

    SEGENV.step = millis();
    // return random8(4, 10); // each flash only lasts one frame/every 24ms... originally 4-10 milliseconds
  }
  else
  {
    if (millis() - SEGENV.step > SEGENV.aux0)
    {
      SEGENV.aux1--;
      if (SEGENV.aux1 < 2)
        SEGENV.aux1 = 0;

      SEGENV.aux0 = (50 + random8(100)); // delay between flashes
      if (SEGENV.aux1 == 2)
      {
        SEGENV.aux0 = (random8(255 - SEGMENT.speed) * 100); // delay between strikes
      }
      SEGENV.step = millis();
    }
  }
  return FRAMETIME;
}

// Pride2015
// Animated, ever-changing rainbows.
// by Mark Kriegsman: https://gist.github.com/kriegsman/964de772d64c502760e5
uint16_t WS2812FX::mode_pride_2015(void)
{
  uint16_t duration = 10 + SEGMENT.speed;
  uint16_t sPseudotime = SEGENV.step;
  uint16_t sHue16 = SEGENV.aux0;

  uint8_t sat8 = beatsin88(87, 220, 250);
  uint8_t brightdepth = beatsin88(341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88(203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16; // gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 1, 3000);

  sPseudotime += duration * msmultiplier;
  sHue16 += duration * beatsin88(400, 5, 9);
  uint16_t brightnesstheta16 = sPseudotime;
  CRGB fastled_col;

  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 >> 8;

    brightnesstheta16 += brightnessthetainc16;
    uint16_t b16 = sin16(brightnesstheta16) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);

    CRGB newcolor = CHSV(hue8, sat8, bri8);
    fastled_col = col_to_crgb(getPixelColor(i));

    nblend(fastled_col, newcolor, 64);
    setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
  }
  SEGENV.step = sPseudotime;
  SEGENV.aux0 = sHue16;
  return FRAMETIME;
}

// eight colored dots, weaving in and out of sync with each other
uint16_t WS2812FX::mode_juggle(void)
{
  fade_out(SEGMENT.intensity);
  CRGB fastled_col;
  byte dothue = 0;
  for (byte i = 0; i < 8; i++)
  {
    uint16_t index = 0 + beatsin88((128 + SEGMENT.speed) * (i + 7), 0, SEGLEN - 1);
    fastled_col = col_to_crgb(getPixelColor(index));
    fastled_col |= (SEGMENT.palette == 0) ? CHSV(dothue, 220, 255) : ColorFromPalette(currentPalette, dothue, 255);
    setPixelColor(index, fastled_col.red, fastled_col.green, fastled_col.blue);
    dothue += 32;
  }
  return FRAMETIME;
}

uint16_t WS2812FX::mode_palette()
{
  uint16_t counter = 0;
  if (SEGMENT.speed != 0)
  {
    counter = (now * ((SEGMENT.speed >> 3) + 1)) & 0xFFFF;
    counter = counter >> 8;
  }

  bool noWrap = (paletteBlend == 2 || (paletteBlend == 0 && SEGMENT.speed == 0));
  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    uint8_t colorIndex = (i * 255 / SEGLEN) - counter;

    if (noWrap)
      colorIndex = map(colorIndex, 0, 255, 0, 240); // cut off blend at palette "end"

    setPixelColor(i, color_from_palette(colorIndex, false, true, 255));
  }
  return FRAMETIME;
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
// feel of your fire: COOLING (used in step 1 above) (Speed = COOLING), and SPARKING (used
// in step 3 above) (Effect Intensity = Sparking).

uint16_t WS2812FX::mode_fire_2012()
{
  uint32_t it = now >> 5; // div 32

  if (!SEGENV.allocateData(SEGLEN))
    return mode_static(); // allocation failed

  byte *heat = SEGENV.data;

  if (it != SEGENV.step)
  {
    uint8_t ignition = max(7, SEGLEN / 10); // ignition area: 10% of segment length or minimum 7 pixels

    // Step 1.  Cool down every cell a little
    for (uint16_t i = 0; i < SEGLEN; i++)
    {
      uint8_t temp = qsub8(heat[i], random8(0, (((20 + SEGMENT.speed / 3) * 10) / SEGLEN) + 2));
      heat[i] = (temp == 0 && i < ignition) ? 16 : temp; // prevent ignition area from becoming black
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for (uint16_t k = SEGLEN - 1; k > 1; k--)
    {
      heat[k] = (heat[k - 1] + (heat[k - 2] << 1)) / 3; // heat[k-2] multiplied by 2
    }

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if (random8() <= SEGMENT.intensity)
    {
      uint8_t y = random8(ignition);
      if (y < SEGLEN)
        heat[y] = qadd8(heat[y], random8(160, 255));
    }
    SEGENV.step = it;
  }

  // Step 4.  Map from heat cells to LED colors
  for (uint16_t j = 0; j < SEGLEN; j++)
  {
    CRGB color = ColorFromPalette(currentPalette, MIN(heat[j], 240), 255, LINEARBLEND);
    setPixelColor(j, color.red, color.green, color.blue);
  }
  return FRAMETIME;
}

// ColorWavesWithPalettes by Mark Kriegsman: https://gist.github.com/kriegsman/8281905786e8b2632aeb
// This function draws color waves with an ever-changing,
// widely-varying set of parameters, using a color palette.
uint16_t WS2812FX::mode_colorwaves()
{
  uint16_t duration = 10 + SEGMENT.speed;
  uint16_t sPseudotime = SEGENV.step;
  uint16_t sHue16 = SEGENV.aux0;

  uint8_t brightdepth = beatsin88(341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88(203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16; // gHue * 256;
  // uint16_t hueinc16 = beatsin88(113, 300, 1500);
  uint16_t hueinc16 = beatsin88(113, 60, 300) * SEGMENT.intensity * 10 / 255; // Use the Intensity Slider for the hues

  sPseudotime += duration * msmultiplier;
  sHue16 += duration * beatsin88(400, 5, 9);
  uint16_t brightnesstheta16 = sPseudotime;
  CRGB fastled_col;

  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 >> 8;
    uint16_t h16_128 = hue16 >> 7;
    if (h16_128 & 0x100)
    {
      hue8 = 255 - (h16_128 >> 1);
    }
    else
    {
      hue8 = h16_128 >> 1;
    }

    brightnesstheta16 += brightnessthetainc16;
    uint16_t b16 = sin16(brightnesstheta16) + 32768;

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
  return FRAMETIME;
}

// colored stripes pulsing at a defined Beats-Per-Minute (BPM)
uint16_t WS2812FX::mode_bpm()
{
  //  CRGB fastled_col;
  uint32_t stp = (now / 20) & 0xFF;
  uint8_t beat = beatsin8(SEGMENT.speed, 64, 255);
  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    //    fastled_col = ColorFromPalette(currentPalette, stp + (i * 2), beat - stp + (i * 10));
    //    setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
    setPixelColor(i, color_blend(SEGCOLOR(1), color_from_palette(stp + (i * 2), false, PALETTE_SOLID_WRAP, 0), beat - stp + (i * 10))); // This supports RGBW.
  }
  return FRAMETIME;
}

uint16_t WS2812FX::mode_fillnoise8()
{
  if (SEGENV.call == 0)
    SEGENV.step = random16(12345);
  CRGB fastled_col;
  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    uint8_t index = inoise8(i * SEGLEN, SEGENV.step + i * SEGLEN);
    fastled_col = ColorFromPalette(currentPalette, index, 255, LINEARBLEND);
    setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
  }
  SEGENV.step += beatsin8(SEGMENT.speed, 1, 6); // 10,1,4

  return FRAMETIME;
}

uint16_t WS2812FX::mode_noise16_1()
{
  uint16_t scale = 320; // the "zoom factor" for the noise
  CRGB fastled_col;
  SEGENV.step += (1 + SEGMENT.speed / 16);

  for (uint16_t i = 0; i < SEGLEN; i++)
  {

    uint16_t shift_x = beatsin8(11);     // the x position of the noise field swings @ 17 bpm
    uint16_t shift_y = SEGENV.step / 42; // the y position becomes slowly incremented

    uint16_t real_x = (i + shift_x) * scale; // the x position of the noise field swings @ 17 bpm
    uint16_t real_y = (i + shift_y) * scale; // the y position becomes slowly incremented
    uint32_t real_z = SEGENV.step;           // the z position becomes quickly incremented

    uint8_t noise = inoise16(real_x, real_y, real_z) >> 8; // get the noise data and scale it down

    uint8_t index = sin8(noise * 3); // map LED color based on noise data

    fastled_col = ColorFromPalette(currentPalette, index, 255, LINEARBLEND); // With that value, look up the 8 bit colour palette value and assign it to the current LED.
    setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
  }

  return FRAMETIME;
}

uint16_t WS2812FX::mode_noise16_2()
{
  uint16_t scale = 1000; // the "zoom factor" for the noise
                         //  CRGB fastled_col;
  SEGENV.step += (1 + (SEGMENT.speed >> 1));

  for (uint16_t i = 0; i < SEGLEN; i++)
  {

    uint16_t shift_x = SEGENV.step >> 6; // x as a function of time

    uint32_t real_x = (i + shift_x) * scale; // calculate the coordinates within the noise field

    uint8_t noise = inoise16(real_x, 0, 4223) >> 8; // get the noise data and scale it down

    uint8_t index = sin8(noise * 3); // map led color based on noise data

    //    fastled_col = ColorFromPalette(currentPalette, index, noise, LINEARBLEND);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.
    //    setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
    setPixelColor(i, color_blend(SEGCOLOR(1), color_from_palette(index, false, PALETTE_SOLID_WRAP, 0), noise)); // This supports RGBW.
  }

  return FRAMETIME;
}

uint16_t WS2812FX::mode_noise16_3()
{
  uint16_t scale = 800; // the "zoom factor" for the noise
                        //  CRGB fastled_col;
  SEGENV.step += (1 + SEGMENT.speed);

  for (uint16_t i = 0; i < SEGLEN; i++)
  {

    uint16_t shift_x = 4223; // no movement along x and y
    uint16_t shift_y = 1234;

    uint32_t real_x = (i + shift_x) * scale; // calculate the coordinates within the noise field
    uint32_t real_y = (i + shift_y) * scale; // based on the precalculated positions
    uint32_t real_z = SEGENV.step * 8;

    uint8_t noise = inoise16(real_x, real_y, real_z) >> 8; // get the noise data and scale it down

    uint8_t index = sin8(noise * 3); // map led color based on noise data

    //    fastled_col = ColorFromPalette(currentPalette, index, noise, LINEARBLEND);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.
    //    setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
    setPixelColor(i, color_blend(SEGCOLOR(1), color_from_palette(index, false, PALETTE_SOLID_WRAP, 0), noise)); // This supports RGBW.
  }

  return FRAMETIME;
}

// https://github.com/aykevl/ledstrip-spark/blob/master/ledstrip.ino
uint16_t WS2812FX::mode_noise16_4()
{
  CRGB fastled_col;
  uint32_t stp = (now * SEGMENT.speed) >> 7;
  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    int16_t index = inoise16(uint32_t(i) << 12, stp);
    fastled_col = ColorFromPalette(currentPalette, index);
    setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
  }
  return FRAMETIME;
}

// based on https://gist.github.com/kriegsman/5408ecd397744ba0393e
uint16_t WS2812FX::mode_colortwinkle()
{
  uint16_t dataSize = (SEGLEN + 7) >> 3; // 1 bit per LED
  if (!SEGENV.allocateData(dataSize))
    return mode_static(); // allocation failed

  CRGB fastled_col, prev;
  fract8 fadeUpAmount = _brightness > 28 ? 8 + (SEGMENT.speed >> 2) : 68 - _brightness, fadeDownAmount = _brightness > 28 ? 8 + (SEGMENT.speed >> 3) : 68 - _brightness;
  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    fastled_col = col_to_crgb(getPixelColor(i));
    prev = fastled_col;
    uint16_t index = i >> 3;
    uint8_t bitNum = i & 0x07;
    bool fadeUp = bitRead(SEGENV.data[index], bitNum);

    if (fadeUp)
    {
      CRGB incrementalColor = fastled_col;
      incrementalColor.nscale8_video(fadeUpAmount);
      fastled_col += incrementalColor;

      if (fastled_col.red == 255 || fastled_col.green == 255 || fastled_col.blue == 255)
      {
        bitWrite(SEGENV.data[index], bitNum, false);
      }
      setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);

      if (col_to_crgb(getPixelColor(i)) == prev)
      { // fix "stuck" pixels
        fastled_col += fastled_col;
        setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
      }
    }
    else
    {
      fastled_col.nscale8(255 - fadeDownAmount);
      setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
    }
  }

  for (uint16_t j = 0; j <= SEGLEN / 50; j++)
  {
    if (random8() <= SEGMENT.intensity)
    {
      for (uint8_t times = 0; times < 5; times++)
      { // attempt to spawn a new pixel 5 times
        int i = random16(SEGLEN);
        if (getPixelColor(i) == 0)
        {
          fastled_col = ColorFromPalette(currentPalette, random8(), 64, NOBLEND);
          uint16_t index = i >> 3;
          uint8_t bitNum = i & 0x07;
          bitWrite(SEGENV.data[index], bitNum, true);
          setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
          break; // only spawn 1 new pixel per frame per 50 LEDs
        }
      }
    }
  }
  return FRAMETIME_FIXED;
}

// Calm effect, like a lake at night
uint16_t WS2812FX::mode_lake()
{
  uint8_t sp = SEGMENT.speed / 10;
  int wave1 = beatsin8(sp + 2, -64, 64);
  int wave2 = beatsin8(sp + 1, -64, 64);
  uint8_t wave3 = beatsin8(sp + 2, 0, 80);
  //  CRGB fastled_col;

  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    int index = cos8((i * 15) + wave1) / 2 + cubicwave8((i * 23) + wave2) / 2;
    uint8_t lum = (index > wave3) ? index - wave3 : 0;
    //    fastled_col = ColorFromPalette(currentPalette, map(index,0,255,0,240), lum, LINEARBLEND);
    //    setPixelColor(i, fastled_col.red, fastled_col.green, fastled_col.blue);
    setPixelColor(i, color_blend(SEGCOLOR(1), color_from_palette(map(index, 0, 255, 0, 240), false, PALETTE_SOLID_WRAP, 0), lum)); // This supports RGBW.
  }
  return FRAMETIME;
}

// meteor effect
// send a meteor from begining to to the end of the strip with a trail that randomly decays.
// adapted from https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/#LEDStripEffectMeteorRain
uint16_t WS2812FX::mode_meteor()
{
  if (!SEGENV.allocateData(SEGLEN))
    return mode_static(); // allocation failed

  byte *trail = SEGENV.data;

  byte meteorSize = 1 + SEGLEN / 10;
  uint16_t counter = now * ((SEGMENT.speed >> 2) + 8);
  uint16_t in = counter * SEGLEN >> 16;

  // fade all leds to colors[1] in LEDs one step
  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    if (random8() <= 255 - SEGMENT.intensity)
    {
      byte meteorTrailDecay = 128 + random8(127);
      trail[i] = scale8(trail[i], meteorTrailDecay);
      setPixelColor(i, color_from_palette(trail[i], false, true, 255));
    }
  }

  // draw meteor
  for (int j = 0; j < meteorSize; j++)
  {
    uint16_t index = in + j;
    if (index >= SEGLEN)
    {
      index = (in + j - SEGLEN);
    }

    trail[index] = 240;
    setPixelColor(index, color_from_palette(trail[index], false, true, 255));
  }

  return FRAMETIME;
}

// smooth meteor effect
// send a meteor from begining to to the end of the strip with a trail that randomly decays.
// adapted from https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/#LEDStripEffectMeteorRain
uint16_t WS2812FX::mode_meteor_smooth()
{
  if (!SEGENV.allocateData(SEGLEN))
    return mode_static(); // allocation failed

  byte *trail = SEGENV.data;

  byte meteorSize = 1 + SEGLEN / 10;
  uint16_t in = map((SEGENV.step >> 6 & 0xFF), 0, 255, 0, SEGLEN - 1);

  // fade all leds to colors[1] in LEDs one step
  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    if (trail[i] != 0 && random8() <= 255 - SEGMENT.intensity)
    {
      int change = 3 - random8(12); // change each time between -8 and +3
      trail[i] += change;
      if (trail[i] > 245)
        trail[i] = 0;
      if (trail[i] > 240)
        trail[i] = 240;
      setPixelColor(i, color_from_palette(trail[i], false, true, 255));
    }
  }

  // draw meteor
  for (int j = 0; j < meteorSize; j++)
  {
    uint16_t index = in + j;
    if (in + j >= SEGLEN)
    {
      index = (in + j - SEGLEN);
    }
    setPixelColor(index, color_blend(getPixelColor(index), color_from_palette(240, false, true, 255), 48));
    trail[index] = 240;
  }

  SEGENV.step += SEGMENT.speed + 1;
  return FRAMETIME;
}

// Railway Crossing / Christmas Fairy lights
uint16_t WS2812FX::mode_railway()
{
  uint16_t dur = 40 + (255 - SEGMENT.speed) * 10;
  uint16_t rampdur = (dur * SEGMENT.intensity) >> 8;
  if (SEGENV.step > dur)
  {
    // reverse direction
    SEGENV.step = 0;
    SEGENV.aux0 = !SEGENV.aux0;
  }
  uint8_t pos = 255;
  if (rampdur != 0)
  {
    uint16_t p0 = (SEGENV.step * 255) / rampdur;
    if (p0 < 255)
      pos = p0;
  }
  if (SEGENV.aux0)
    pos = 255 - pos;
  for (uint16_t i = 0; i < SEGLEN; i += 2)
  {
    setPixelColor(i, color_from_palette(255 - pos, false, false, 255));
    if (i < SEGLEN - 1)
    {
      setPixelColor(i + 1, color_from_palette(pos, false, false, 255));
    }
  }
  SEGENV.step += FRAMETIME;
  return FRAMETIME;
}

// Water ripple
// propagation velocity from speed
// drop rate from intensity

// 4 bytes
typedef struct Ripple
{
  int8_t state; // WLEDSR AC has uint8_t while value can be negative???
  uint8_t color;
  uint16_t pos;
} ripple;

#ifdef ESP8266
#define MAX_RIPPLES 56
#else
#define MAX_RIPPLES 100
#endif
uint16_t WS2812FX::ripple_base(bool rainbow)
{
  uint16_t maxRipples = min(1 + (SEGLEN >> 2), MAX_RIPPLES); // 56 max for 16 segment ESP8266
  uint16_t dataSize = sizeof(ripple) * maxRipples;

  if (!SEGENV.allocateData(dataSize))
    return mode_static(); // allocation failed

  Ripple *ripples = reinterpret_cast<Ripple *>(SEGENV.data);

  // ranbow background or chosen background, all very dim.
  if (rainbow)
  {
    if (SEGENV.call == 0)
    {
      SEGENV.aux0 = random8();
      SEGENV.aux1 = random8();
    }
    if (SEGENV.aux0 == SEGENV.aux1)
    {
      SEGENV.aux1 = random8();
    }
    else if (SEGENV.aux1 > SEGENV.aux0)
    {
      SEGENV.aux0++;
    }
    else
    {
      SEGENV.aux0--;
    }
    fill(color_blend(color_wheel(SEGENV.aux0), BLACK, 235));
  }
  else
  {
    fill(SEGCOLOR(1));
  }

  // draw wave
  for (uint16_t i = 0; i < maxRipples; i++)
  {
    uint16_t ripplestate = ripples[i].state;
    if (ripplestate)
    {
      uint8_t rippledecay = (SEGMENT.speed >> 4) + 1; // faster decay if faster propagation
      uint16_t rippleorigin = ripples[i].pos;
      uint32_t col = color_from_palette(ripples[i].color, false, false, 255);
      uint16_t propagation = ((ripplestate / rippledecay - 1) * SEGMENT.speed);
      int16_t propI = propagation >> 8;
      uint8_t propF = propagation & 0xFF;
      int16_t left = rippleorigin - propI - 1;
      uint8_t amp = (ripplestate < 17) ? triwave8((ripplestate - 1) * 8) : map(ripplestate, 17, 255, 255, 2);

      for (int16_t v = left; v < left + 4; v++)
      {
        uint8_t mag = scale8(cubicwave8((propF >> 2) + (v - left) * 64), amp);
        if (v < SEGLEN && v >= 0)
        {
          setPixelColor(v, color_blend(getPixelColor(v), col, mag));
        }
        int16_t w = left + propI * 2 + 3 - (v - left);
        if (w < SEGLEN && w >= 0)
        {
          setPixelColor(w, color_blend(getPixelColor(w), col, mag));
        }
      }
      ripplestate += rippledecay;
      ripples[i].state = (ripplestate > 254) ? 0 : ripplestate;
    }
    else // randomly create new wave
    {
      if (random16(IBN + 10000) <= SEGMENT.intensity)
      {
        ripples[i].state = 1;
        ripples[i].pos = random16(SEGLEN);
        ripples[i].color = random8(); // color
      }
    }
  }
  return FRAMETIME;
}
#undef MAX_RIPPLES

uint16_t WS2812FX::mode_ripple(void)
{
  return ripple_base(false);
}

uint16_t WS2812FX::mode_ripple_rainbow(void)
{
  return ripple_base(true);
}

//  TwinkleFOX by Mark Kriegsman: https://gist.github.com/kriegsman/756ea6dcae8e30845b5a
//
//  TwinkleFOX: Twinkling 'holiday' lights that fade in and out.
//  Colors are chosen from a palette. Read more about this effect using the link above!

// If COOL_LIKE_INCANDESCENT is set to 1, colors will
// fade out slighted 'reddened', similar to how
// incandescent bulbs change color as they get dim down.
#define COOL_LIKE_INCANDESCENT 1

CRGB IRAM_ATTR WS2812FX::twinklefox_one_twinkle(uint32_t ms, uint8_t salt, bool cat)
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
  uint8_t twinkleDensity = (SEGMENT.intensity >> 5) + 1;

  uint8_t bright = 0;
  if (((slowcycle8 & 0x0E) / 2) < twinkleDensity)
  {
    uint8_t ph = fastcycle8;
    // This is like 'triwave8', which produces a
    // symmetrical up-and-down triangle sawtooth waveform, except that this
    // function produces a triangle wave with a faster attack and a slower decay
    if (cat) // twinklecat, variant where the leds instantly turn on
    {
      bright = 255 - ph;
    }
    else
    { // vanilla twinklefox
      if (ph < 86)
      {
        bright = ph * 3;
      }
      else
      {
        ph -= 86;
        bright = 255 - (ph + (ph / 2));
      }
    }
  }

  uint8_t hue = slowcycle8 - salt;
  CRGB c;
  if (bright > 0)
  {
    c = ColorFromPalette(currentPalette, hue, bright, NOBLEND);
    if (COOL_LIKE_INCANDESCENT == 1)
    {
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
  }
  else
  {
    c = CRGB::Black;
  }
  return c;
}

//  This function loops over each pixel, calculates the
//  adjusted 'clock' that this pixel should use, and calls
//  "CalculateOneTwinkle" on each pixel.  It then displays
//  either the twinkle color of the background color,
//  whichever is brighter.
uint16_t WS2812FX::twinklefox_base(bool cat)
{
  // "PRNG16" is the pseudorandom number generator
  // It MUST be reset to the same starting value each time
  // this function is called, so that the sequence of 'random'
  // numbers that it generates is (paradoxically) stable.
  uint16_t PRNG16 = 11337;

  // Calculate speed
  if (SEGMENT.speed > 100)
    SEGENV.aux0 = 3 + ((255 - SEGMENT.speed) >> 3);
  else
    SEGENV.aux0 = 22 + ((100 - SEGMENT.speed) >> 1);

  // Set up the background color, "bg".
  CRGB bg;
  bg = col_to_crgb(SEGCOLOR(1));
  uint8_t bglight = bg.getAverageLight();
  if (bglight > 64)
  {
    bg.nscale8_video(16); // very bright, so scale to 1/16th
  }
  else if (bglight > 16)
  {
    bg.nscale8_video(64); // not that bright, so scale to 1/4th
  }
  else
  {
    bg.nscale8_video(86); // dim, scale to 1/3rd.
  }

  uint8_t backgroundBrightness = bg.getAverageLight();

  for (uint16_t i = 0; i < SEGLEN; i++)
  {

    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    uint16_t myclockoffset16 = PRNG16;         // use that number as clock offset
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    // use that number as clock speed adjustment factor (in 8ths, from 8/8ths to 23/8ths)
    uint8_t myspeedmultiplierQ5_3 = ((((PRNG16 & 0xFF) >> 4) + (PRNG16 & 0x0F)) & 0x0F) + 0x08;
    uint32_t myclock30 = (uint32_t)((now * myspeedmultiplierQ5_3) >> 3) + myclockoffset16;
    uint8_t myunique8 = PRNG16 >> 8; // get 'salt' value for this pixel

    // We now have the adjusted 'clock' for this pixel, now we call
    // the function that computes what color the pixel should be based
    // on the "brightness = f( time )" idea.
    CRGB c = twinklefox_one_twinkle(myclock30, myunique8, cat);

    uint8_t cbright = c.getAverageLight();
    int16_t deltabright = cbright - backgroundBrightness;
    if (deltabright >= 32 || (!bg))
    {
      // If the new pixel is significantly brighter than the background color,
      // use the new color.
      setPixelColor(i, c.red, c.green, c.blue);
    }
    else if (deltabright > 0)
    {
      // If the new pixel is just slightly brighter than the background color,
      // mix a blend of the new color and the background color
      setPixelColor(i, color_blend(crgb_to_col(bg), crgb_to_col(c), deltabright * 8));
    }
    else
    {
      // if the new pixel is not at all brighter than the background color,
      // just use the background color.
      setPixelColor(i, bg.r, bg.g, bg.b);
    }
  }
  return FRAMETIME;
}

uint16_t WS2812FX::mode_twinklefox()
{
  return twinklefox_base(false);
}

uint16_t WS2812FX::mode_twinklecat()
{
  return twinklefox_base(true);
}

// inspired by https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/#LEDStripEffectBlinkingHalloweenEyes
#define HALLOWEEN_EYE_SPACE 3
#define HALLOWEEN_EYE_WIDTH 1

uint16_t WS2812FX::mode_halloween_eyes()
{
  uint16_t eyeLength = (2 * HALLOWEEN_EYE_WIDTH) + HALLOWEEN_EYE_SPACE;
  if (eyeLength > SEGLEN)
    return mode_static(); // bail if segment too short

  fill(SEGCOLOR(1)); // fill background

  uint8_t state = SEGENV.aux1 >> 8;
  uint16_t stateTime = SEGENV.call;
  if (stateTime == 0)
    stateTime = 2000;

  if (state == 0)
  {                                                // spawn eyes
    SEGENV.aux0 = random16(0, SEGLEN - eyeLength); // start pos
    SEGENV.aux1 = random8();                       // color
    state = 1;
  }

  if (state < 2)
  { // fade eyes
    uint16_t startPos = SEGENV.aux0;
    uint16_t start2ndEye = startPos + HALLOWEEN_EYE_WIDTH + HALLOWEEN_EYE_SPACE;

    uint32_t fadestage = (now - SEGENV.step) * 255 / stateTime;
    if (fadestage > 255)
      fadestage = 255;
    uint32_t c = color_blend(color_from_palette(SEGENV.aux1 & 0xFF, false, false, 0), SEGCOLOR(1), fadestage);

    for (uint16_t i = 0; i < HALLOWEEN_EYE_WIDTH; i++)
    {
      setPixelColor(startPos + i, c);
      setPixelColor(start2ndEye + i, c);
    }
  }

  if (now - SEGENV.step > stateTime)
  {
    state++;
    if (state > 2)
      state = 0;

    if (state < 2)
    {
      stateTime = 100 + (255 - SEGMENT.intensity) * 10; // eye fade time
    }
    else
    {
      uint16_t eyeOffTimeBase = (255 - SEGMENT.speed) * 10;
      stateTime = eyeOffTimeBase + random16(eyeOffTimeBase);
    }
    SEGENV.step = now;
    SEGENV.call = stateTime;
  }

  SEGENV.aux1 = (SEGENV.aux1 & 0xFF) + (state << 8); // save state

  return FRAMETIME;
}

// Speed slider sets amount of LEDs lit, intensity sets unlit
uint16_t WS2812FX::mode_static_pattern()
{
  uint16_t lit = 1 + SEGMENT.speed;
  uint16_t unlit = 1 + SEGMENT.intensity;
  bool drawingLit = true;
  uint16_t cnt = 0;

  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    setPixelColor(i, (drawingLit) ? color_from_palette(i, true, PALETTE_SOLID_WRAP, 0) : SEGCOLOR(1));
    cnt++;
    if (cnt >= ((drawingLit) ? lit : unlit))
    {
      cnt = 0;
      drawingLit = !drawingLit;
    }
  }

  return FRAMETIME;
}

uint16_t WS2812FX::mode_tri_static_pattern()
{
  uint8_t segSize = (SEGMENT.intensity >> 5) + 1;
  uint8_t currSeg = 0;
  uint16_t currSegCount = 0;

  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    if (currSeg % 3 == 0)
    {
      setPixelColor(i, SEGCOLOR(0));
    }
    else if (currSeg % 3 == 1)
    {
      setPixelColor(i, SEGCOLOR(1));
    }
    else
    {
      setPixelColor(i, (SEGCOLOR(2) > 0 ? SEGCOLOR(2) : WHITE));
    }
    currSegCount += 1;
    if (currSegCount >= segSize)
    {
      currSeg += 1;
      currSegCount = 0;
    }
  }

  return FRAMETIME;
}

uint16_t WS2812FX::spots_base(uint16_t threshold)
{
  fill(SEGCOLOR(1));

  uint16_t maxZones = SEGLEN >> 2;
  uint16_t zones = 1 + ((SEGMENT.intensity * maxZones) >> 8);
  uint16_t zoneLen = SEGLEN / zones;
  uint16_t offset = (SEGLEN - zones * zoneLen) >> 1;

  for (uint16_t z = 0; z < zones; z++)
  {
    uint16_t pos = offset + z * zoneLen;
    for (uint16_t i = 0; i < zoneLen; i++)
    {
      uint16_t wave = triwave16((i * 0xFFFF) / zoneLen);
      if (wave > threshold)
      {
        uint16_t index = 0 + pos + i;
        uint8_t s = (wave - threshold) * 255 / (0xFFFF - threshold);
        setPixelColor(index, color_blend(color_from_palette(index, true, PALETTE_SOLID_WRAP, 0), SEGCOLOR(1), 255 - s));
      }
    }
  }

  return FRAMETIME;
}

// Intensity slider sets number of "lights", speed sets LEDs per light
uint16_t WS2812FX::mode_spots()
{
  return spots_base((255 - SEGMENT.speed) << 8);
}

// Intensity slider sets number of "lights", LEDs per light fade in and out
uint16_t WS2812FX::mode_spots_fade()
{
  uint16_t counter = now * ((SEGMENT.speed >> 2) + 8);
  uint16_t t = triwave16(counter);
  uint16_t tr = (t >> 1) + (t >> 2);
  return spots_base(tr);
}

// each needs 12 bytes
typedef struct Ball
{
  unsigned long lastBounceTime;
  float impactVelocity;
  float height;
} ball;

/*
 *  Bouncing Balls Effect
 */
uint16_t WS2812FX::mode_bouncing_balls(void)
{
  // allocate segment data
  uint16_t maxNumBalls = 16;
  uint16_t dataSize = sizeof(ball) * maxNumBalls;
  if (!SEGENV.allocateData(dataSize))
    return mode_static(); // allocation failed

  Ball *balls = reinterpret_cast<Ball *>(SEGENV.data);

  // number of balls based on intensity setting to max of 7 (cycles colors)
  // non-chosen color is a random color
  uint8_t numBalls = int(((SEGMENT.intensity * (maxNumBalls - 0.8f)) / 255) + 1);

  float gravity = -9.81; // standard value of gravity
  float impactVelocityStart = sqrt(-2 * gravity);

  unsigned long time = millis();

  if (SEGENV.call == 0)
  {
    for (uint8_t i = 0; i < maxNumBalls; i++)
      balls[i].lastBounceTime = time;
  }

  bool hasCol2 = SEGCOLOR(2);
  fill(hasCol2 ? BLACK : SEGCOLOR(1));

  for (uint8_t i = 0; i < numBalls; i++)
  {
    float timeSinceLastBounce = (time - balls[i].lastBounceTime) / ((255 - SEGMENT.speed) * 8 / 256 + 1);
    balls[i].height = 0.5 * gravity * pow(timeSinceLastBounce / 1000, 2.0) + balls[i].impactVelocity * timeSinceLastBounce / 1000;

    if (balls[i].height < 0)
    { // start bounce
      balls[i].height = 0;
      // damping for better effect using multiple balls
      float dampening = 0.90 - float(i) / pow(numBalls, 2);
      balls[i].impactVelocity = dampening * balls[i].impactVelocity;
      balls[i].lastBounceTime = time;

      if (balls[i].impactVelocity < 0.015)
      {
        balls[i].impactVelocity = impactVelocityStart;
      }
    }

    uint32_t color = SEGCOLOR(0);
    if (SEGMENT.palette)
    {
      color = color_wheel(i * (256 / MAX(numBalls, 8)));
    }
    else if (hasCol2)
    {
      color = SEGCOLOR(i % NUM_COLORS);
    }

    uint16_t pos = round(balls[i].height * (SEGLEN - 1));
    setPixelColor(pos, color);
  }

  return FRAMETIME;
}

/*
 * Sinelon stolen from FASTLED examples
 */
uint16_t WS2812FX::sinelon_base(bool dual, bool rainbow = false)
{
  fade_out(SEGMENT.intensity);
  uint16_t pos = beatsin16(SEGMENT.speed / 10, 0, SEGLEN - 1);
  if (SEGENV.call == 0)
    SEGENV.aux0 = pos;
  uint32_t color1 = color_from_palette(pos, true, false, 0);
  uint32_t color2 = SEGCOLOR(2);
  if (rainbow)
  {
    color1 = color_wheel((pos & 0x07) * 32);
  }
  setPixelColor(pos, color1);
  if (dual)
  {
    if (!color2)
      color2 = color_from_palette(pos, true, false, 0);
    if (rainbow)
      color2 = color1; // rainbow
    setPixelColor(SEGLEN - 1 - pos, color2);
  }
  if (SEGENV.aux0 != pos)
  {
    if (SEGENV.aux0 < pos)
    {
      for (int i = SEGENV.aux0; i < pos; i++)
      { // WLEDSR bugfix
        setPixelColor(i, color1);
        if (dual)
          setPixelColor(SEGLEN - 1 - i, color2);
      }
    }
    else
    {
      for (int i = SEGENV.aux0; i > pos; i--)
      { // WLEDSR bugfix
        setPixelColor(i, color1);
        if (dual)
          setPixelColor(SEGLEN - 1 - i, color2);
      }
    }
    SEGENV.aux0 = pos;
  }

  return FRAMETIME;
}

uint16_t WS2812FX::mode_sinelon(void)
{
  return sinelon_base(false);
}

uint16_t WS2812FX::mode_sinelon_dual(void)
{
  return sinelon_base(true);
}

uint16_t WS2812FX::mode_sinelon_rainbow(void)
{
  return sinelon_base(false, true);
}

// Rainbow with glitter, inspired by https://gist.github.com/kriegsman/062e10f7f07ba8518af6
uint16_t WS2812FX::mode_glitter()
{
  mode_palette();

  if (SEGMENT.intensity > random8())
  {
    setPixelColor(random16(SEGLEN), ULTRAWHITE);
  }

  return FRAMETIME;
}

// each needs 12 bytes
// Spark type is used for popcorn, 1D fireworks, and drip
typedef struct Spark
{
  float pos;
  float vel;
  uint16_t col;
  uint8_t colIndex;
} spark;

/*
 *  POPCORN
 *  modified from https://github.com/kitesurfer1404/WS2812FX/blob/master/src/custom/Popcorn.h
 */
uint16_t WS2812FX::mode_popcorn(void)
{
  // allocate segment data
  uint16_t maxNumPopcorn = 21; // max 21 on 16 segment ESP8266
  uint16_t dataSize = sizeof(spark) * maxNumPopcorn;
  if (!SEGENV.allocateData(dataSize))
    return mode_static(); // allocation failed

  Spark *popcorn = reinterpret_cast<Spark *>(SEGENV.data);

  float gravity = -0.0001 - (SEGMENT.speed / 200000.0); // m/s/s
  gravity *= SEGLEN;

  bool hasCol2 = SEGCOLOR(2);
  fill(hasCol2 ? BLACK : SEGCOLOR(1));

  uint8_t numPopcorn = SEGMENT.intensity * maxNumPopcorn / 255;
  if (numPopcorn == 0)
    numPopcorn = 1;

  for (uint8_t i = 0; i < numPopcorn; i++)
  {
    if (popcorn[i].pos >= 0.0f)
    { // if kernel is active, update its position
      popcorn[i].pos += popcorn[i].vel;
      popcorn[i].vel += gravity;
    }
    else
    { // if kernel is inactive, randomly pop it
      if (random8() < 2)
      { // POP!!!
        popcorn[i].pos = 0.01f;

        uint16_t peakHeight = 128 + random8(128); // 0-255
        peakHeight = (peakHeight * (SEGLEN - 1)) >> 8;
        popcorn[i].vel = sqrt(-2.0 * gravity * peakHeight);

        if (SEGMENT.palette)
        {
          popcorn[i].colIndex = random8();
        }
        else
        {
          byte col = random8(0, NUM_COLORS);
          if (!hasCol2 || !SEGCOLOR(col))
            col = 0;
          popcorn[i].colIndex = col;
        }
      }
    }
    if (popcorn[i].pos >= 0.0f)
    { // draw now active popcorn (either active before or just popped)
      uint32_t col = color_wheel(popcorn[i].colIndex);
      if (!SEGMENT.palette && popcorn[i].colIndex < NUM_COLORS)
        col = SEGCOLOR(popcorn[i].colIndex);
      uint16_t ledIndex = popcorn[i].pos;
      if (ledIndex < SEGLEN)
        setPixelColor(ledIndex, col);
    }
  }

  return FRAMETIME;
}

// values close to 100 produce 5Hz flicker, which looks very candle-y
// Inspired by https://github.com/avanhanegem/ArduinoCandleEffectNeoPixel
// and https://cpldcpu.wordpress.com/2016/01/05/reverse-engineering-a-real-candle/

uint16_t WS2812FX::candle(bool multi)
{
  if (multi)
  {
    // allocate segment data
    uint16_t dataSize = (SEGLEN - 1) * 3; // max. 1365 pixels (ESP8266)
    if (!SEGENV.allocateData(dataSize))
      return candle(false); // allocation failed
  }

  // max. flicker range controlled by intensity
  uint8_t valrange = SEGMENT.intensity;
  uint8_t rndval = valrange >> 1; // max 127

  // step (how much to move closer to target per frame) coarsely set by speed
  uint8_t speedFactor = 4;
  if (SEGMENT.speed > 252)
  { // epilepsy
    speedFactor = 1;
  }
  else if (SEGMENT.speed > 99)
  { // regular candle (mode called every ~25 ms, so 4 frames to have a new target every 100ms)
    speedFactor = 2;
  }
  else if (SEGMENT.speed > 49)
  { // slower fade
    speedFactor = 3;
  } // else 4 (slowest)

  uint16_t numCandles = (multi) ? SEGLEN : 1;

  for (uint16_t i = 0; i < numCandles; i++)
  {
    uint16_t d = 0; // data location

    uint8_t s = SEGENV.aux0, s_target = SEGENV.aux1, fadeStep = SEGENV.step;
    if (i > 0)
    {
      d = (i - 1) * 3;
      s = SEGENV.data[d];
      s_target = SEGENV.data[d + 1];
      fadeStep = SEGENV.data[d + 2];
    }
    if (fadeStep == 0)
    { // init vals
      s = 128;
      s_target = 130 + random8(4);
      fadeStep = 1;
    }

    bool newTarget = false;
    if (s_target > s)
    { // fade up
      s = qadd8(s, fadeStep);
      if (s >= s_target)
        newTarget = true;
    }
    else
    {
      s = qsub8(s, fadeStep);
      if (s <= s_target)
        newTarget = true;
    }

    if (newTarget)
    {
      s_target = random8(rndval) + random8(rndval); // between 0 and rndval*2 -2 = 252
      if (s_target < (rndval >> 1))
        s_target = (rndval >> 1) + random8(rndval);
      uint8_t offset = (255 - valrange);
      s_target += offset;

      uint8_t dif = (s_target > s) ? s_target - s : s - s_target;

      fadeStep = dif >> speedFactor;
      if (fadeStep == 0)
        fadeStep = 1;
    }

    if (i > 0)
    {
      setPixelColor(i, color_blend(SEGCOLOR(1), color_from_palette(i, true, PALETTE_SOLID_WRAP, 0), s));

      SEGENV.data[d] = s;
      SEGENV.data[d + 1] = s_target;
      SEGENV.data[d + 2] = fadeStep;
    }
    else
    {
      for (uint16_t j = 0; j < SEGLEN; j++)
      {
        setPixelColor(j, color_blend(SEGCOLOR(1), color_from_palette(j, true, PALETTE_SOLID_WRAP, 0), s));
      }

      SEGENV.aux0 = s;
      SEGENV.aux1 = s_target;
      SEGENV.step = fadeStep;
    }
  }

  return FRAMETIME_FIXED;
}

uint16_t WS2812FX::mode_candle()
{
  return candle(false);
}

uint16_t WS2812FX::mode_candle_multi()
{
  return candle(true);
}

/*
/ Fireworks in starburst effect
/ based on the video: https://www.reddit.com/r/arduino/comments/c3sd46/i_made_this_fireworks_effect_for_my_led_strips/
/ Speed sets frequency of new starbursts, intensity is the intensity of the burst
*/
#ifdef ESP8266
#define STARBURST_MAX_FRAG 8 // 52 bytes / star
#else
#define STARBURST_MAX_FRAG 10 // 60 bytes / star
#endif
// each needs 20+STARBURST_MAX_FRAG*4 bytes
typedef struct particle
{
  CRGB color;
  uint32_t birth = 0;
  uint32_t last = 0;
  float vel = 0;
  uint16_t pos = -1;
  float fragment[STARBURST_MAX_FRAG];
} star;

uint16_t WS2812FX::mode_starburst(void)
{
  uint16_t maxData = FAIR_DATA_PER_SEG; // ESP8266: 256 ESP32: 640
  uint8_t segs = getActiveSegmentsNum();
  if (segs <= (MAX_NUM_SEGMENTS / 2))
    maxData *= 2; // ESP8266: 512 if <= 8 segs ESP32: 1280 if <= 16 segs
  if (segs <= (MAX_NUM_SEGMENTS / 4))
    maxData *= 2;                             // ESP8266: 1024 if <= 4 segs ESP32: 2560 if <= 8 segs
  uint16_t maxStars = maxData / sizeof(star); // ESP8266: max. 4/9/19 stars/seg, ESP32: max. 10/21/42 stars/seg

  uint8_t numStars = 1 + (SEGLEN >> 3);
  if (numStars > maxStars)
    numStars = maxStars;
  uint16_t dataSize = sizeof(star) * numStars;

  if (!SEGENV.allocateData(dataSize))
    return mode_static(); // allocation failed

  uint32_t it = millis();

  star *stars = reinterpret_cast<star *>(SEGENV.data);

  float maxSpeed = 375.0f;          // Max velocity
  float particleIgnition = 250.0f;  // How long to "flash"
  float particleFadeTime = 1500.0f; // Fade out time

  for (int j = 0; j < numStars; j++)
  {
    // speed to adjust chance of a burst, max is nearly always.
    if (random8((144 - (SEGMENT.speed >> 1))) == 0 && stars[j].birth == 0)
    {
      // Pick a random color and location.
      uint16_t startPos = random16(SEGLEN - 1);
      float multiplier = (float)(random8()) / 255.0 * 1.0;

      stars[j].color = col_to_crgb(color_wheel(random8()));
      stars[j].pos = startPos;
      stars[j].vel = maxSpeed * (float)(random8()) / 255.0 * multiplier;
      stars[j].birth = it;
      stars[j].last = it;
      // more fragments means larger burst effect
      int num = random8(3, 6 + (SEGMENT.intensity >> 5));

      for (int i = 0; i < STARBURST_MAX_FRAG; i++)
      {
        if (i < num)
          stars[j].fragment[i] = startPos;
        else
          stars[j].fragment[i] = -1;
      }
    }
  }

  fill(SEGCOLOR(1));

  for (int j = 0; j < numStars; j++)
  {
    if (stars[j].birth != 0)
    {
      float dt = (it - stars[j].last) / 1000.0;

      for (int i = 0; i < STARBURST_MAX_FRAG; i++)
      {
        int var = i >> 1;

        if (stars[j].fragment[i] > 0)
        {
          // all fragments travel right, will be mirrored on other side
          stars[j].fragment[i] += stars[j].vel * dt * (float)var / 3.0;
        }
      }
      stars[j].last = it;
      stars[j].vel -= 3 * stars[j].vel * dt;
    }

    CRGB c = stars[j].color;

    // If the star is brand new, it flashes white briefly.
    // Otherwise it just fades over time.
    float fade = 0.0f;
    float age = it - stars[j].birth;

    if (age < particleIgnition)
    {
      c = col_to_crgb(color_blend(WHITE, crgb_to_col(c), 254.5f * ((age / particleIgnition))));
    }
    else
    {
      // Figure out how much to fade and shrink the star based on
      // its age relative to its lifetime
      if (age > particleIgnition + particleFadeTime)
      {
        fade = 1.0f; // Black hole, all faded out
        stars[j].birth = 0;
        c = col_to_crgb(SEGCOLOR(1));
      }
      else
      {
        age -= particleIgnition;
        fade = (age / particleFadeTime); // Fading star
        byte f = 254.5f * fade;
        c = col_to_crgb(color_blend(crgb_to_col(c), SEGCOLOR(1), f));
      }
    }

    float particleSize = (1.0 - fade) * 2;

    for (uint8_t index = 0; index < STARBURST_MAX_FRAG * 2; index++)
    {
      bool mirrored = index & 0x1;
      uint8_t i = index >> 1;
      if (stars[j].fragment[i] > 0)
      {
        float loc = stars[j].fragment[i];
        if (mirrored)
          loc -= (loc - stars[j].pos) * 2;
        int start = loc - particleSize;
        int end = loc + particleSize;
        if (start < 0)
          start = 0;
        if (start == end)
          end++;
        if (end > SEGLEN)
          end = SEGLEN;
        for (int p = start; p < end; p++)
        {
          setPixelColor(p, c.r, c.g, c.b);
        }
      }
    }
  }
  return FRAMETIME;
}
#undef STARBURST_MAX_FRAG

/*
 * Exploding fireworks effect
 * adapted from: http://www.anirama.com/1000leds/1d-fireworks/
 */
uint16_t WS2812FX::mode_exploding_fireworks(void)
{
  // allocate segment data
  uint16_t maxData = FAIR_DATA_PER_SEG; // ESP8266: 256 ESP32: 640
  uint8_t segs = getActiveSegmentsNum();
  if (segs <= (MAX_NUM_SEGMENTS / 2))
    maxData *= 2; // ESP8266: 512 if <= 8 segs ESP32: 1280 if <= 16 segs
  if (segs <= (MAX_NUM_SEGMENTS / 4))
    maxData *= 2;                          // ESP8266: 1024 if <= 4 segs ESP32: 2560 if <= 8 segs
  int maxSparks = maxData / sizeof(spark); // ESP8266: max. 21/42/85 sparks/seg, ESP32: max. 53/106/213 sparks/seg

  uint16_t numSparks = min(2 + (SEGLEN >> 1), maxSparks);
  uint16_t dataSize = sizeof(spark) * numSparks;
  if (!SEGENV.allocateData(dataSize))
    return mode_static(); // allocation failed

  if (dataSize != SEGENV.aux1)
  { // reset to flare if sparks were reallocated
    SEGENV.aux0 = 0;
    SEGENV.aux1 = dataSize;
  }

  fill(BLACK);

  bool actuallyReverse = SEGMENT.getOption(SEG_OPTION_REVERSED);
  // have fireworks start in either direction based on intensity
  SEGMENT.setOption(SEG_OPTION_REVERSED, SEGENV.step);

  Spark *sparks = reinterpret_cast<Spark *>(SEGENV.data);
  Spark *flare = sparks; // first spark is flare data

  float gravity = -0.0004 - (SEGMENT.speed / 800000.0); // m/s/s
  gravity *= SEGLEN;

  if (SEGENV.aux0 < 2)
  { // FLARE
    if (SEGENV.aux0 == 0)
    { // init flare
      flare->pos = 0;
      uint16_t peakHeight = 75 + random8(180); // 0-255
      peakHeight = (peakHeight * (SEGLEN - 1)) >> 8;
      flare->vel = sqrt(-2.0 * gravity * peakHeight);
      flare->col = 255; // brightness

      SEGENV.aux0 = 1;
    }

    // launch
    if (flare->vel > 12 * gravity)
    {
      // flare
      setPixelColor(int(flare->pos), flare->col, flare->col, flare->col);

      flare->pos += flare->vel;
      flare->pos = constrain(flare->pos, 0, SEGLEN - 1);
      flare->vel += gravity;
      flare->col -= 2;
    }
    else
    {
      SEGENV.aux0 = 2; // ready to explode
    }
  }
  else if (SEGENV.aux0 < 4)
  {
    /*
     * Explode!
     *
     * Explosion happens where the flare ended.
     * Size is proportional to the height.
     */
    int nSparks = flare->pos;
    nSparks = constrain(nSparks, 0, numSparks);
    static float dying_gravity;

    // initialize sparks
    if (SEGENV.aux0 == 2)
    {
      for (int i = 1; i < nSparks; i++)
      {
        sparks[i].pos = flare->pos;
        sparks[i].vel = (float(random16(0, 20000)) / 10000.0) - 0.9; // from -0.9 to 1.1
        sparks[i].col = 345;                                         // abs(sparks[i].vel * 750.0); // set colors before scaling velocity to keep them bright
        // sparks[i].col = constrain(sparks[i].col, 0, 345);
        sparks[i].colIndex = random8();
        sparks[i].vel *= flare->pos / SEGLEN; // proportional to height
        sparks[i].vel *= -gravity * 50;
      }
      // sparks[1].col = 345; // this will be our known spark
      dying_gravity = gravity / 2;
      SEGENV.aux0 = 3;
    }

    if (sparks[1].col > 4)
    { //&& sparks[1].pos > 0) { // as long as our known spark is lit, work with all the sparks
      for (int i = 1; i < nSparks; i++)
      {
        sparks[i].pos += sparks[i].vel;
        sparks[i].vel += dying_gravity;
        if (sparks[i].col > 3)
          sparks[i].col -= 4;

        if (sparks[i].pos > 0 && sparks[i].pos < SEGLEN)
        {
          uint16_t prog = sparks[i].col;
          uint32_t spColor = (SEGMENT.palette) ? color_wheel(sparks[i].colIndex) : SEGCOLOR(0);
          CRGB c = CRGB::Black; // HeatColor(sparks[i].col);
          if (prog > 300)
          { // fade from white to spark color
            c = col_to_crgb(color_blend(spColor, WHITE, (prog - 300) * 5));
          }
          else if (prog > 45)
          { // fade from spark color to black
            c = col_to_crgb(color_blend(BLACK, spColor, prog - 45));
            uint8_t cooling = (300 - prog) >> 5;
            c.g = qsub8(c.g, cooling);
            c.b = qsub8(c.b, cooling * 2);
          }
          setPixelColor(int(sparks[i].pos), c.red, c.green, c.blue);
        }
      }
      dying_gravity *= .99; // as sparks burn out they fall slower
    }
    else
    {
      SEGENV.aux0 = 6 + random8(10); // wait for this many frames
    }
  }
  else
  {
    SEGENV.aux0--;
    if (SEGENV.aux0 < 4)
    {
      SEGENV.aux0 = 0;                                                 // back to flare
      SEGENV.step = actuallyReverse ^ (SEGMENT.intensity > random8()); // decide firing side
    }
  }

  SEGMENT.setOption(SEG_OPTION_REVERSED, actuallyReverse);

  return FRAMETIME;
}
#undef MAX_SPARKS

/*
 * Drip Effect
 * ported of: https://www.youtube.com/watch?v=sru2fXh4r7k
 */
uint16_t WS2812FX::mode_drip(void)
{
  // allocate segment data
  uint8_t numDrops = 4;
  uint16_t dataSize = sizeof(spark) * numDrops;
  if (!SEGENV.allocateData(dataSize))
    return mode_static(); // allocation failed

  fill(SEGCOLOR(1));

  Spark *drops = reinterpret_cast<Spark *>(SEGENV.data);

  numDrops = 1 + (SEGMENT.intensity >> 6); // 255>>6 = 3

  float gravity = -0.0005 - (SEGMENT.speed / 50000.0);
  gravity *= SEGLEN;
  int sourcedrop = 12;

  for (uint8_t j = 0; j < numDrops; j++)
  {
    if (drops[j].colIndex == 0)
    {                            // init
      drops[j].pos = SEGLEN - 1; // start at end
      drops[j].vel = 0;          // speed
      drops[j].col = sourcedrop; // brightness
      drops[j].colIndex = 1;     // drop state (0 init, 1 forming, 2 falling, 5 bouncing)
    }

    setPixelColor(SEGLEN - 1, color_blend(BLACK, SEGCOLOR(0), sourcedrop)); // water source
    if (drops[j].colIndex == 1)
    {
      if (drops[j].col > 255)
        drops[j].col = 255;
      setPixelColor(uint16_t(drops[j].pos), color_blend(BLACK, SEGCOLOR(0), drops[j].col));

      drops[j].col += map(SEGMENT.speed, 0, 255, 1, 6); // swelling

      if (random8() < drops[j].col / 10)
      {                        // random drop
        drops[j].colIndex = 2; // fall
        drops[j].col = 255;
      }
    }
    if (drops[j].colIndex > 1)
    { // falling
      if (drops[j].pos > 0)
      { // fall until end of segment
        drops[j].pos += drops[j].vel;
        if (drops[j].pos < 0)
          drops[j].pos = 0;
        drops[j].vel += gravity; // gravity is negative

        for (uint16_t i = 1; i < 7 - drops[j].colIndex; i++)
        {                                                                        // some minor math so we don't expand bouncing droplets
          uint16_t pos = constrain(uint16_t(drops[j].pos) + i, 0, SEGLEN - 1);   // this is BAD, returns a pos >= SEGLEN occasionally
          setPixelColor(pos, color_blend(BLACK, SEGCOLOR(0), drops[j].col / i)); // spread pixel with fade while falling
        }

        if (drops[j].colIndex > 2)
        { // during bounce, some water is on the floor
          setPixelColor(0, color_blend(SEGCOLOR(0), BLACK, drops[j].col));
        }
      }
      else
      { // we hit bottom
        if (drops[j].colIndex > 2)
        { // already hit once, so back to forming
          drops[j].colIndex = 0;
          drops[j].col = sourcedrop;
        }
        else
        {

          if (drops[j].colIndex == 2)
          {                                   // init bounce
            drops[j].vel = -drops[j].vel / 4; // reverse velocity with damping
            drops[j].pos += drops[j].vel;
          }
          drops[j].col = sourcedrop * 2;
          drops[j].colIndex = 5; // bouncing
        }
      }
    }
  }
  return FRAMETIME;
}

/*
 * Tetris or Stacking (falling bricks) Effect
 * by Blaz Kristan (https://github.com/blazoncek, https://blaz.at/home)
 */
// 12 bytes
typedef struct Tetris
{
  float pos;
  float speed;
  uint32_t col;
} tetris;

uint16_t WS2812FX::mode_tetrix(void)
{

  uint16_t dataSize = sizeof(tetris);
  if (!SEGENV.allocateData(dataSize))
    return mode_static(); // allocation failed
  Tetris *drop = reinterpret_cast<Tetris *>(SEGENV.data);

  // initialize dropping on first call or segment full
  if (SEGENV.call == 0 || SEGENV.aux1 >= SEGLEN)
  {
    SEGENV.aux1 = 0; // reset brick stack size
    SEGENV.step = 0;
    fill(SEGCOLOR(1));
    return 250; // short wait
  }

  if (SEGENV.step == 0)
  {                                                                                                         // init
    drop->speed = 0.0238 * (SEGMENT.speed ? (SEGMENT.speed >> 2) + 1 : random8(6, 64));                     // set speed
    drop->pos = SEGLEN;                                                                                     // start at end of segment (no need to subtract 1)
    drop->col = color_from_palette(random8(0, 15) << 4, false, false, 0);                                   // limit color choices so there is enough HUE gap
    SEGENV.step = 1;                                                                                        // drop state (0 init, 1 forming, 2 falling)
    SEGENV.aux0 = (SEGMENT.intensity ? (SEGMENT.intensity >> 5) + 1 : random8(1, 5)) * (1 + (SEGLEN >> 6)); // size of brick
  }

  if (SEGENV.step == 1)
  { // forming
    if (random8() >> 6)
    {                  // random drop
      SEGENV.step = 2; // fall
    }
  }

  if (SEGENV.step > 1)
  { // falling
    if (drop->pos > SEGENV.aux1)
    {                           // fall until top of stack
      drop->pos -= drop->speed; // may add gravity as: speed += gravity
      if (int(drop->pos) < SEGENV.aux1)
        drop->pos = SEGENV.aux1;
      for (uint16_t i = int(drop->pos); i < SEGLEN; i++)
        setPixelColor(i, i < int(drop->pos) + SEGENV.aux0 ? drop->col : SEGCOLOR(1));
    }
    else
    {                             // we hit bottom
      SEGENV.step = 0;            // go back to init
      SEGENV.aux1 += SEGENV.aux0; // increase the stack size
      if (SEGENV.aux1 >= SEGLEN)
        return 1000; // wait for a second
    }
  }
  return FRAMETIME;
}

/*
/ Plasma Effect
/ adapted from https://github.com/atuline/FastLED-Demos/blob/master/plasma/plasma.ino
*/
uint16_t WS2812FX::mode_plasma(void)
{
  // initialize phases on start
  if (SEGENV.call == 0)
  {
    SEGENV.aux0 = random8(0, 2); // add a bit of randomness
  }
  uint8_t thisPhase = beatsin8(6 + SEGENV.aux0, -64, 64);
  uint8_t thatPhase = beatsin8(7 + SEGENV.aux0, -64, 64);

  for (int i = 0; i < SEGLEN; i++)
  {                                                                                              // For each of the LED's in the strand, set color &  brightness based on a wave as follows:
    uint8_t colorIndex = cubicwave8((i * (2 + 3 * (SEGMENT.speed >> 5)) + thisPhase) & 0xFF) / 2 // factor=23 // Create a wave and add a phase change and add another wave with its own phase change.
                         + cos8((i * (1 + 2 * (SEGMENT.speed >> 5)) + thatPhase) & 0xFF) / 2;    // factor=15 // Hey, you can even change the frequencies if you wish.
    uint8_t thisBright = qsub8(colorIndex, beatsin8(7, 0, (128 - (SEGMENT.intensity >> 1))));
    //    CRGB color = ColorFromPalette(currentPalette, colorIndex, thisBright, LINEARBLEND);
    //    setPixelColor(i, color.red, color.green, color.blue);
    setPixelColor(i, color_blend(SEGCOLOR(1), color_from_palette(colorIndex, false, PALETTE_SOLID_WRAP, 0), thisBright)); // This supports RGBW.
  }

  return FRAMETIME;
}

/*
 * Percentage display
 * Intesity values from 0-100 turn on the leds.
 */
uint16_t WS2812FX::mode_percent(void)
{

  uint8_t percent = MAX(0, MIN(200, SEGMENT.intensity));
  uint16_t active_leds = (percent < 100) ? SEGLEN * percent / 100.0
                                         : SEGLEN * (200 - percent) / 100.0;

  uint8_t size = (1 + ((SEGMENT.speed * SEGLEN) >> 11));
  if (SEGMENT.speed == 255)
    size = 255;

  if (percent < 100)
  {
    for (uint16_t i = 0; i < SEGLEN; i++)
    {
      if (i < SEGENV.step)
      {
        setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));
      }
      else
      {
        setPixelColor(i, SEGCOLOR(1));
      }
    }
  }
  else
  {
    for (uint16_t i = 0; i < SEGLEN; i++)
    {
      if (i < (SEGLEN - SEGENV.step))
      {
        setPixelColor(i, SEGCOLOR(1));
      }
      else
      {
        setPixelColor(i, color_from_palette(i, true, PALETTE_SOLID_WRAP, 0));
      }
    }
  }

  if (active_leds > SEGENV.step)
  { // smooth transition to the target value
    SEGENV.step += size;
    if (SEGENV.step > active_leds)
      SEGENV.step = active_leds;
  }
  else if (active_leds < SEGENV.step)
  {
    if (SEGENV.step > size)
      SEGENV.step -= size;
    else
      SEGENV.step = 0;
    if (SEGENV.step < active_leds)
      SEGENV.step = active_leds;
  }

  return FRAMETIME;
}

/*
/ Modulates the brightness similar to a heartbeat
*/
uint16_t WS2812FX::mode_heartbeat(void)
{
  uint8_t bpm = 40 + (SEGMENT.speed >> 4);
  uint32_t msPerBeat = (60000 / bpm);
  uint32_t secondBeat = (msPerBeat / 3);

  uint32_t bri_lower = SEGENV.aux1;
  bri_lower = bri_lower * 2042 / (2048 + SEGMENT.intensity);
  SEGENV.aux1 = bri_lower;

  unsigned long beatTimer = millis() - SEGENV.step;
  if ((beatTimer > secondBeat) && !SEGENV.aux0)
  {                           // time for the second beat?
    SEGENV.aux1 = UINT16_MAX; // full bri
    SEGENV.aux0 = 1;
  }
  if (beatTimer > msPerBeat)
  {                           // time to reset the beat timer?
    SEGENV.aux1 = UINT16_MAX; // full bri
    SEGENV.aux0 = 0;
    SEGENV.step = millis();
  }

  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    setPixelColor(i, color_blend(color_from_palette(i, true, PALETTE_SOLID_WRAP, 0), SEGCOLOR(1), 255 - (SEGENV.aux1 >> 8)));
  }

  return FRAMETIME;
}

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
uint16_t WS2812FX::mode_pacifica()
{
  uint32_t nowOld = now;

  CRGBPalette16 pacifica_palette_1 =
      {0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117,
       0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x14554B, 0x28AA50};
  CRGBPalette16 pacifica_palette_2 =
      {0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117,
       0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x0C5F52, 0x19BE5F};
  CRGBPalette16 pacifica_palette_3 =
      {0x000208, 0x00030E, 0x000514, 0x00061A, 0x000820, 0x000927, 0x000B2D, 0x000C33,
       0x000E39, 0x001040, 0x001450, 0x001860, 0x001C70, 0x002080, 0x1040BF, 0x2060FF};

  if (SEGMENT.palette)
  {
    pacifica_palette_1 = currentPalette;
    pacifica_palette_2 = currentPalette;
    pacifica_palette_3 = currentPalette;
  }

  // Increment the four "color index start" counters, one for each wave layer.
  // Each is incremented at a different speed, and the speeds vary over time.
  uint16_t sCIStart1 = SEGENV.aux0, sCIStart2 = SEGENV.aux1, sCIStart3 = SEGENV.step, sCIStart4 = SEGENV.step >> 16;
  // static uint16_t sCIStart1, sCIStart2, sCIStart3, sCIStart4;
  // uint32_t deltams = 26 + (SEGMENT.speed >> 3);
  uint32_t deltams = (FRAMETIME >> 2) + ((FRAMETIME * SEGMENT.speed) >> 7);
  uint64_t deltat = (now >> 2) + ((now * SEGMENT.speed) >> 7);
  now = deltat;

  uint16_t speedfactor1 = beatsin16(3, 179, 269);
  uint16_t speedfactor2 = beatsin16(4, 179, 269);
  uint32_t deltams1 = (deltams * speedfactor1) / 256;
  uint32_t deltams2 = (deltams * speedfactor2) / 256;
  uint32_t deltams21 = (deltams1 + deltams2) / 2;
  sCIStart1 += (deltams1 * beatsin88(1011, 10, 13));
  sCIStart2 -= (deltams21 * beatsin88(777, 8, 11));
  sCIStart3 -= (deltams1 * beatsin88(501, 5, 7));
  sCIStart4 -= (deltams2 * beatsin88(257, 4, 6));
  SEGENV.aux0 = sCIStart1;
  SEGENV.aux1 = sCIStart2;
  SEGENV.step = sCIStart4;
  SEGENV.step = (SEGENV.step << 16) + sCIStart3;

  // Clear out the LED array to a dim background blue-green
  // fill(132618);

  uint8_t basethreshold = beatsin8(9, 55, 65);
  uint8_t wave = beat8(7);

  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    CRGB c = CRGB(2, 6, 10);
    // Render each of four layers, with different scales and speeds, that vary over time
    c += pacifica_one_layer(i, pacifica_palette_1, sCIStart1, beatsin16(3, 11 * 256, 14 * 256), beatsin8(10, 70, 130), 0 - beat16(301));
    c += pacifica_one_layer(i, pacifica_palette_2, sCIStart2, beatsin16(4, 6 * 256, 9 * 256), beatsin8(17, 40, 80), beat16(401));
    c += pacifica_one_layer(i, pacifica_palette_3, sCIStart3, 6 * 256, beatsin8(9, 10, 38), 0 - beat16(503));
    c += pacifica_one_layer(i, pacifica_palette_3, sCIStart4, 5 * 256, beatsin8(8, 10, 28), beat16(601));

    // Add extra 'white' to areas where the four layers of light have lined up brightly
    uint8_t threshold = scale8(sin8(wave), 20) + basethreshold;
    wave += 7;
    uint8_t l = c.getAverageLight();
    if (l > threshold)
    {
      uint8_t overage = l - threshold;
      uint8_t overage2 = qadd8(overage, overage);
      c += CRGB(overage, overage2, qadd8(overage2, overage2));
    }

    // deepen the blues and greens
    c.blue = scale8(c.blue, 145);
    c.green = scale8(c.green, 200);
    c |= CRGB(2, 5, 7);

    setPixelColor(i, c.red, c.green, c.blue);
  }

  now = nowOld;
  return FRAMETIME;
}

// Add one layer of waves into the led array
CRGB WS2812FX::pacifica_one_layer(uint16_t i, CRGBPalette16 &p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff)
{
  uint16_t ci = cistart;
  uint16_t waveangle = ioff;
  uint16_t wavescale_half = (wavescale >> 1) + 20;

  waveangle += ((120 + SEGMENT.intensity) * i); // original 250 * i
  uint16_t s16 = sin16(waveangle) + 32768;
  uint16_t cs = scale16(s16, wavescale_half) + wavescale_half;
  ci += (cs * i);
  uint16_t sindex16 = sin16(ci) + 32768;
  uint8_t sindex8 = scale16(sindex16, 240);
  return ColorFromPalette(p, sindex8, bri, LINEARBLEND);
}

// Solid colour background with glitter
uint16_t WS2812FX::mode_solid_glitter()
{
  fill(SEGCOLOR(0));

  if (SEGMENT.intensity > random8())
  {
    setPixelColor(random16(SEGLEN), ULTRAWHITE);
  }
  return FRAMETIME;
}

/*
 * Mode simulates a gradual sunrise
 */
uint16_t WS2812FX::mode_sunrise()
{
  // speed 0 - static sun
  // speed 1 - 60: sunrise time in minutes
  // speed 60 - 120 : sunset time in minutes - 60;
  // speed above: "breathing" rise and set
  if (SEGENV.call == 0 || SEGMENT.speed != SEGENV.aux0)
  {
    SEGENV.step = millis(); // save starting time, millis() because now can change from sync
    SEGENV.aux0 = SEGMENT.speed;
  }

  fill(0);
  uint16_t stage = 0xFFFF;

  uint32_t s10SinceStart = (millis() - SEGENV.step) / 100; // tenths of seconds

  if (SEGMENT.speed > 120)
  { // quick sunrise and sunset
    uint16_t counter = (now >> 1) * (((SEGMENT.speed - 120) >> 1) + 1);
    stage = triwave16(counter);
  }
  else if (SEGMENT.speed)
  { // sunrise
    uint8_t durMins = SEGMENT.speed;
    if (durMins > 60)
      durMins -= 60;
    uint32_t s10Target = durMins * 600;
    if (s10SinceStart > s10Target)
      s10SinceStart = s10Target;
    stage = map(s10SinceStart, 0, s10Target, 0, 0xFFFF);
    if (SEGMENT.speed > 60)
      stage = 0xFFFF - stage; // sunset
  }

  for (uint16_t i = 0; i <= SEGLEN / 2; i++)
  {
    // default palette is Fire
    uint32_t c = color_from_palette(0, false, true, 255); // background

    uint16_t wave = triwave16((i * stage) / SEGLEN);

    wave = (wave >> 8) + ((wave * SEGMENT.intensity) >> 15);

    if (wave > 240)
    { // clipped, full white sun
      c = color_from_palette(240, false, true, 255);
    }
    else
    { // transition
      c = color_from_palette(wave, false, true, 255);
    }
    setPixelColor(i, c);
    setPixelColor(SEGLEN - i - 1, c);
  }

  return FRAMETIME;
}

/*
 * Effects by Andrew Tuline
 */
uint16_t WS2812FX::phased_base(uint8_t moder)
{ // We're making sine waves here. By Andrew Tuline.

  uint8_t allfreq = 16; // Base frequency.
  // float* phasePtr = reinterpret_cast<float*>(SEGENV.step);       // Phase change value gets calculated.
  static float phase = 0;                     // phasePtr[0];
  uint8_t cutOff = (255 - SEGMENT.intensity); // You can change the number of pixels.  AKA INTENSITY (was 192).
  uint8_t modVal = 5;                         // SEGMENT.custom1/8+1;                         // You can change the modulus. AKA Custom1 (was 5).

  uint8_t index = now / 64;      // Set color rotation speed
  phase += SEGMENT.speed / 32.0; // You can change the speed of the wave. AKA SPEED (was .4)
  // phasePtr[0] = phase;

  for (int i = 0; i < SEGLEN; i++)
  {
    if (moder == 1)
      modVal = (inoise8(i * 10 + i * 10) / 16); // Let's randomize our mod length with some Perlin noise.
    uint16_t val = (i + 1) * allfreq;           // This sets the frequency of the waves. The +1 makes sure that leds[0] is used.
    if (modVal == 0)
      modVal = 1;
    val += phase * (i % modVal + 1) / 2; // This sets the varying phase change of the waves. By Andrew Tuline.
    uint8_t b = cubicwave8(val);         // Now we make an 8 bit sinewave.
    b = (b > cutOff) ? (b - cutOff) : 0; // A ternary operator to cutoff the light.
    setPixelColor(i, color_blend(SEGCOLOR(1), color_from_palette(index, false, false, 0), b));
    index += 256 / SEGLEN;
    if (SEGLEN > 256)
      index++; // Correction for segments longer than 256 LEDs
  }

  return FRAMETIME;
}

uint16_t WS2812FX::mode_phased(void)
{
  return phased_base(0);
}

uint16_t WS2812FX::mode_phased_noise(void)
{
  return phased_base(1);
}

uint16_t WS2812FX::mode_twinkleup(void)
{                         // A very short twinkle routine with fade-in and dual controls. By Andrew Tuline.
  random16_set_seed(535); // The randomizer needs to be re-set each time through the loop in order for the same 'random' numbers to be the same each time through.

  for (int i = 0; i < SEGLEN; i++)
  {
    uint8_t pixBri = beatsin8(SEGMENT.speed / 2 + random8() / 4, SEGMENT.custom1, 255, 0, random8()); // Every pixel gets a different timebase.

    /*
        if (i < SEGLEN/3) {                                 // You can use this to shape a 1D candle, where it's brightest at 1/3rd of SEGLEN and dull at both ends.
          pixBri = pixBri*i/(SEGLEN/3);
        } else {
          pixBri = pixBri*(SEGLEN-i)/SEGLEN;
        }
    */

    if (random8() > SEGMENT.intensity)
      pixBri = 0;
    setPixelColor(i, color_blend(SEGCOLOR(1), color_from_palette(random8() + now / 100, false, PALETTE_SOLID_WRAP, 0), pixBri));
  }

  return FRAMETIME;
}

// Peaceful noise that's slow and with gradually changing palettes. Does not support WLED palettes or default colours or controls.
uint16_t WS2812FX::mode_noisepal(void)
{                                                 // Slow noise palette by Andrew Tuline.
  uint16_t scale = 15 + (SEGMENT.intensity >> 2); // default was 30
  // #define scale 30

  uint16_t dataSize = sizeof(CRGBPalette16) * 2; // allocate space for 2 Palettes (2 * 16 * 3 = 96 bytes)
  if (!SEGENV.allocateData(dataSize))
    return mode_static(); // allocation failed

  CRGBPalette16 *palettes = reinterpret_cast<CRGBPalette16 *>(SEGENV.data);

  uint16_t changePaletteMs = 4000 + SEGMENT.speed * 10; // between 4 - 6.5sec
  if (millis() - SEGENV.step > changePaletteMs)
  {
    SEGENV.step = millis();

    uint8_t baseI = random8();
    palettes[1] = CRGBPalette16(CHSV(baseI + random8(64), 255, random8(128, 255)), CHSV(baseI + 128, 255, random8(128, 255)), CHSV(baseI + random8(92), 192, random8(128, 255)), CHSV(baseI + random8(92), 255, random8(128, 255)));
  }

  CRGB color;

  // EVERY_N_MILLIS(10) { //(don't have to time this, effect function is only called every 24ms)
  nblendPaletteTowardPalette(palettes[0], palettes[1], 48); // Blend towards the target palette over 48 iterations.

  if (SEGMENT.palette > 0)
    palettes[0] = currentPalette;

  for (int i = 0; i < SEGLEN; i++)
  {
    uint8_t index = inoise8(i * scale, SEGENV.aux0 + i * scale);    // Get a value from the noise function. I'm using both x and y axis.
    color = ColorFromPalette(palettes[0], index, 255, LINEARBLEND); // Use the my own palette.
    setPixelColor(i, color.red, color.green, color.blue);
  }

  SEGENV.aux0 += beatsin8(10, 1, 4); // Moving along the distance. Vary it a bit with a sine wave.

  return FRAMETIME;
}

// Sine waves that have controllable phase change speed, frequency and cutoff. By Andrew Tuline.
// SEGMENT.speed ->Speed, SEGMENT.intensity -> Frequency (SEGMENT.custom1 -> Color change, SEGMENT.custom2 -> PWM cutoff)
//
uint16_t WS2812FX::mode_sinewave(void)
{ // Adjustable sinewave. By Andrew Tuline
  // #define qsuba(x, b)  ((x>b)?x-b:0)               // Analog Unsigned subtraction macro. if result <0, then => 0

  uint16_t colorIndex = now / 32; //(256 - SEGMENT.custom1);  // Amount of colour change.

  SEGENV.step += SEGMENT.speed / 16;     // Speed of animation.
  uint16_t freq = SEGMENT.intensity / 4; // SEGMENT.custom2/8;                       // Frequency of the signal.

  for (int i = 0; i < SEGLEN; i++)
  {                                                    // For each of the LED's in the strand, set a brightness based on a wave as follows:
    int pixBri = cubicwave8((i * freq) + SEGENV.step); // qsuba(cubicwave8((i*freq)+SEGENV.step), (255-SEGMENT.intensity)); // qsub sets a minimum value called thiscutoff. If < thiscutoff, then bright = 0. Otherwise, bright = 128 (as defined in qsub)..
    // setPixCol(i, i*colorIndex/255, pixBri);
    setPixelColor(i, color_blend(SEGCOLOR(1), color_from_palette(i * colorIndex / 255, false, PALETTE_SOLID_WRAP, 0), pixBri));
  }

  return FRAMETIME;
}

/*
 * Best of both worlds from Palette and Spot effects. By Aircoookie
 */
uint16_t WS2812FX::mode_flow(void)
{
  uint16_t counter = 0;
  if (SEGMENT.speed != 0)
  {
    counter = now * ((SEGMENT.speed >> 2) + 1);
    counter = counter >> 8;
  }

  uint16_t maxZones = SEGLEN / 6; // only looks good if each zone has at least 6 LEDs
  uint16_t zones = (SEGMENT.intensity * maxZones) >> 8;
  if (zones & 0x01)
    zones++; // zones must be even
  if (zones < 2)
    zones = 2;
  uint16_t zoneLen = SEGLEN / zones;
  uint16_t offset = (SEGLEN - zones * zoneLen) >> 1;

  fill(color_from_palette(-counter, false, true, 255));

  for (uint16_t z = 0; z < zones; z++)
  {
    uint16_t pos = offset + z * zoneLen;
    for (uint16_t i = 0; i < zoneLen; i++)
    {
      uint8_t colorIndex = (i * 255 / zoneLen) - counter;
      uint16_t led = (z & 0x01) ? i : (zoneLen - 1) - i;
      if (IS_REVERSE)
        led = (zoneLen - 1) - led;
      setPixelColor(pos + led, color_from_palette(colorIndex, false, true, 255));
    }
  }

  return FRAMETIME;
}

/*
 * Dots waving around in a sine/pendulum motion.
 * Little pixel birds flying in a circle. By Aircoookie
 */
uint16_t WS2812FX::mode_chunchun(void)
{
  fill(SEGCOLOR(1));
  uint16_t counter = now * (6 + (SEGMENT.speed >> 4));
  uint16_t numBirds = 2 + (SEGLEN >> 3); // 2 + 1/8 of a segment
  uint16_t span = (SEGMENT.intensity << 8) / numBirds;

  for (uint16_t i = 0; i < numBirds; i++)
  {
    counter -= span;
    uint16_t megumin = sin16(counter) + 0x8000;
    uint32_t bird = (megumin * SEGLEN) >> 16;
    uint32_t c = color_from_palette((i * 255) / numBirds, false, false, 0); // no palette wrapping
    setPixelColor(bird, c);
  }
  return FRAMETIME;
}

// 13 bytes
typedef struct Spotlight
{
  float speed;
  uint8_t colorIdx;
  int16_t position;
  unsigned long lastUpdateTime;
  uint8_t width;
  uint8_t type;
} spotlight;

#define SPOT_TYPE_SOLID 0
#define SPOT_TYPE_GRADIENT 1
#define SPOT_TYPE_2X_GRADIENT 2
#define SPOT_TYPE_2X_DOT 3
#define SPOT_TYPE_3X_DOT 4
#define SPOT_TYPE_4X_DOT 5
#define SPOT_TYPES_COUNT 6
#ifdef ESP8266
#define SPOT_MAX_COUNT 17 // Number of simultaneous waves
#else
#define SPOT_MAX_COUNT 49 // Number of simultaneous waves
#endif

/*
 * Spotlights moving back and forth that cast dancing shadows.
 * Shine this through tree branches/leaves or other close-up objects that cast
 * interesting shadows onto a ceiling or tarp.
 *
 * By Steve Pomeroy @xxv
 */
uint16_t WS2812FX::mode_dancing_shadows(void)
{
  uint8_t numSpotlights = map(SEGMENT.intensity, 0, 255, 2, SPOT_MAX_COUNT); // 49 on 32 segment ESP32, 17 on 16 segment ESP8266
  bool initialize = SEGENV.aux0 != numSpotlights;
  SEGENV.aux0 = numSpotlights;

  uint16_t dataSize = sizeof(spotlight) * numSpotlights;
  if (!SEGENV.allocateData(dataSize))
    return mode_static(); // allocation failed
  Spotlight *spotlights = reinterpret_cast<Spotlight *>(SEGENV.data);

  fill(BLACK);

  unsigned long time = millis();
  bool respawn = false;

  for (uint8_t i = 0; i < numSpotlights; i++)
  {
    if (!initialize)
    {
      // advance the position of the spotlight
      int16_t delta = (float)(time - spotlights[i].lastUpdateTime) *
                      (spotlights[i].speed * ((1.0 + SEGMENT.speed) / 100.0));

      if (abs(delta) >= 1)
      {
        spotlights[i].position += delta;
        spotlights[i].lastUpdateTime = time;
      }

      respawn = (spotlights[i].speed > 0.0 && spotlights[i].position > (SEGLEN + 2)) || (spotlights[i].speed < 0.0 && spotlights[i].position < -(spotlights[i].width + 2));
    }

    if (initialize || respawn)
    {
      spotlights[i].colorIdx = random8();
      spotlights[i].width = random8(1, 10);

      spotlights[i].speed = 1.0 / random8(4, 50);

      if (initialize)
      {
        spotlights[i].position = random16(SEGLEN);
        spotlights[i].speed *= random8(2) ? 1.0 : -1.0;
      }
      else
      {
        if (random8(2))
        {
          spotlights[i].position = SEGLEN + spotlights[i].width;
          spotlights[i].speed *= -1.0;
        }
        else
        {
          spotlights[i].position = -spotlights[i].width;
        }
      }

      spotlights[i].lastUpdateTime = time;
      spotlights[i].type = random8(SPOT_TYPES_COUNT);
    }

    uint32_t color = color_from_palette(spotlights[i].colorIdx, false, false, 0);
    int start = spotlights[i].position;

    if (spotlights[i].width <= 1)
    {
      if (start >= 0 && start < SEGLEN)
      {
        blendPixelColor(start, color, 128);
      }
    }
    else
    {
      switch (spotlights[i].type)
      {
      case SPOT_TYPE_SOLID:
        for (uint8_t j = 0; j < spotlights[i].width; j++)
        {
          if ((start + j) >= 0 && (start + j) < SEGLEN)
          {
            blendPixelColor(start + j, color, 128);
          }
        }
        break;

      case SPOT_TYPE_GRADIENT:
        for (uint8_t j = 0; j < spotlights[i].width; j++)
        {
          if ((start + j) >= 0 && (start + j) < SEGLEN)
          {
            blendPixelColor(start + j, color,
                            cubicwave8(map(j, 0, spotlights[i].width - 1, 0, 255)));
          }
        }
        break;

      case SPOT_TYPE_2X_GRADIENT:
        for (uint8_t j = 0; j < spotlights[i].width; j++)
        {
          if ((start + j) >= 0 && (start + j) < SEGLEN)
          {
            blendPixelColor(start + j, color,
                            cubicwave8(2 * map(j, 0, spotlights[i].width - 1, 0, 255)));
          }
        }
        break;

      case SPOT_TYPE_2X_DOT:
        for (uint8_t j = 0; j < spotlights[i].width; j += 2)
        {
          if ((start + j) >= 0 && (start + j) < SEGLEN)
          {
            blendPixelColor(start + j, color, 128);
          }
        }
        break;

      case SPOT_TYPE_3X_DOT:
        for (uint8_t j = 0; j < spotlights[i].width; j += 3)
        {
          if ((start + j) >= 0 && (start + j) < SEGLEN)
          {
            blendPixelColor(start + j, color, 128);
          }
        }
        break;

      case SPOT_TYPE_4X_DOT:
        for (uint8_t j = 0; j < spotlights[i].width; j += 4)
        {
          if ((start + j) >= 0 && (start + j) < SEGLEN)
          {
            blendPixelColor(start + j, color, 128);
          }
        }
        break;
      }
    }
  }

  return FRAMETIME;
}

/*
  Imitates a washing machine, rotating same waves forward, then pause, then backward.
  By Stefan Seegel
*/
uint16_t WS2812FX::mode_washing_machine(void)
{
  float speed = tristate_square8(now >> 7, 90, 15);
  float quot = 32.0f - ((float)SEGMENT.speed / 16.0f);
  speed /= quot;

  SEGENV.step += (speed * 128.0f);

  for (int i = 0; i < SEGLEN; i++)
  {
    uint8_t col = sin8(((SEGMENT.intensity / 25 + 1) * 255 * i / SEGLEN) + (SEGENV.step >> 7));
    setPixelColor(i, color_from_palette(col, false, PALETTE_SOLID_WRAP, 3));
  }

  return FRAMETIME;
}

/*
  Blends random colors across palette
  Modified, originally by Mark Kriegsman https://gist.github.com/kriegsman/1f7ccbbfa492a73c015e
*/
uint16_t WS2812FX::mode_blends(void)
{
  uint16_t pixelLen = SEGLEN > UINT8_MAX ? UINT8_MAX : SEGLEN;
  uint16_t dataSize = sizeof(uint32_t) * (pixelLen + 1); // max segment length of 56 pixels on 16 segment ESP8266
  if (!SEGENV.allocateData(dataSize))
    return mode_static(); // allocation failed
  uint32_t *pixels = reinterpret_cast<uint32_t *>(SEGENV.data);
  uint8_t blendSpeed = map(SEGMENT.intensity, 0, UINT8_MAX, 10, 128);
  uint8_t shift = (now * ((SEGMENT.speed >> 3) + 1)) >> 8;

  for (int i = 0; i < pixelLen; i++)
  {
    pixels[i] = color_blend(pixels[i], color_from_palette(shift + quadwave8((i + 1) * 16), false, PALETTE_SOLID_WRAP, 255), blendSpeed);
    shift += 3;
  }

  uint16_t offset = 0;
  for (int i = 0; i < SEGLEN; i++)
  {
    setPixelColor(i, pixels[offset++]);
    if (offset > pixelLen)
      offset = 0;
  }

  return FRAMETIME;
}

/*
  TV Simulator
  Modified and adapted to WLED by Def3nder, based on "Fake TV Light for Engineers" by Phillip Burgess https://learn.adafruit.com/fake-tv-light-for-engineers/arduino-sketch
*/
// 43 bytes
typedef struct TvSim
{
  uint32_t totalTime = 0;
  uint32_t fadeTime = 0;
  uint32_t startTime = 0;
  uint32_t elapsed = 0;
  uint32_t pixelNum = 0;
  uint16_t sliderValues = 0;
  uint32_t sceeneStart = 0;
  uint32_t sceeneDuration = 0;
  uint16_t sceeneColorHue = 0;
  uint8_t sceeneColorSat = 0;
  uint8_t sceeneColorBri = 0;
  uint8_t actualColorR = 0;
  uint8_t actualColorG = 0;
  uint8_t actualColorB = 0;
  uint16_t pr = 0; // Prev R, G, B
  uint16_t pg = 0;
  uint16_t pb = 0;
} tvSim;

uint16_t WS2812FX::mode_tv_simulator(void)
{
  uint16_t nr, ng, nb, r, g, b, i, hue;
  uint8_t sat, bri, j;

  if (!SEGENV.allocateData(sizeof(tvSim)))
    return mode_static(); // allocation failed
  TvSim *tvSimulator = reinterpret_cast<TvSim *>(SEGENV.data);

  uint8_t colorSpeed = map(SEGMENT.speed, 0, UINT8_MAX, 1, 20);
  uint8_t colorIntensity = map(SEGMENT.intensity, 0, UINT8_MAX, 10, 30);

  i = SEGMENT.speed << 8 | SEGMENT.intensity;
  if (i != tvSimulator->sliderValues)
  {
    tvSimulator->sliderValues = i;
    SEGENV.aux1 = 0;
  }

  // create a new sceene
  if (((millis() - tvSimulator->sceeneStart) >= tvSimulator->sceeneDuration) || SEGENV.aux1 == 0)
  {
    tvSimulator->sceeneStart = millis();                                                  // remember the start of the new sceene
    tvSimulator->sceeneDuration = random16(60 * 250 * colorSpeed, 60 * 750 * colorSpeed); // duration of a "movie sceene" which has similar colors (5 to 15 minutes with max speed slider)
    tvSimulator->sceeneColorHue = random16(0, 768);                                       // random start color-tone for the sceene
    tvSimulator->sceeneColorSat = random8(100, 130 + colorIntensity);                     // random start color-saturation for the sceene
    tvSimulator->sceeneColorBri = random8(200, 240);                                      // random start color-brightness for the sceene
    SEGENV.aux1 = 1;
    SEGENV.aux0 = 0;
  }

  // slightly change the color-tone in this sceene
  if (SEGENV.aux0 == 0)
  {
    // hue change in both directions
    j = random8(4 * colorIntensity);
    hue = (random8() < 128) ? ((j < tvSimulator->sceeneColorHue) ? tvSimulator->sceeneColorHue - j : 767 - tvSimulator->sceeneColorHue - j) : // negative
              ((j + tvSimulator->sceeneColorHue) < 767 ? tvSimulator->sceeneColorHue + j : tvSimulator->sceeneColorHue + j - 767);            // positive

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
      uint8_t s = ((256 - sat) * bri) >> 8;
      temp[0] = temp[3] = s;
      temp[1] = temp[4] = x + s;
      temp[2] = bri - x;
      tvSimulator->actualColorR = temp[n + 2];
      tvSimulator->actualColorG = temp[n + 1];
      tvSimulator->actualColorB = temp[n];
    }
  }
  // Apply gamma correction, further expand to 16/16/16
  nr = (uint8_t)gamma8(tvSimulator->actualColorR) * 257; // New R/G/B
  ng = (uint8_t)gamma8(tvSimulator->actualColorG) * 257;
  nb = (uint8_t)gamma8(tvSimulator->actualColorB) * 257;

  if (SEGENV.aux0 == 0)
  { // initialize next iteration
    SEGENV.aux0 = 1;

    // randomize total duration and fade duration for the actual color
    tvSimulator->totalTime = random16(250, 2500);                // Semi-random pixel-to-pixel time
    tvSimulator->fadeTime = random16(0, tvSimulator->totalTime); // Pixel-to-pixel transition time
    if (random8(10) < 3)
      tvSimulator->fadeTime = 0; // Force scene cut 30% of time

    tvSimulator->startTime = millis();
  } // end of initialization

  // how much time is elapsed ?
  tvSimulator->elapsed = millis() - tvSimulator->startTime;

  // fade from prev volor to next color
  if (tvSimulator->elapsed < tvSimulator->fadeTime)
  {
    r = map(tvSimulator->elapsed, 0, tvSimulator->fadeTime, tvSimulator->pr, nr);
    g = map(tvSimulator->elapsed, 0, tvSimulator->fadeTime, tvSimulator->pg, ng);
    b = map(tvSimulator->elapsed, 0, tvSimulator->fadeTime, tvSimulator->pb, nb);
  }
  else
  { // Avoid divide-by-zero in map()
    r = nr;
    g = ng;
    b = nb;
  }

  // set strip color
  for (i = 0; i < SEGLEN; i++)
  {
    setPixelColor(i, r >> 8, g >> 8, b >> 8); // Quantize to 8-bit
  }

  // if total duration has passed, remember last color and restart the loop
  if (tvSimulator->elapsed >= tvSimulator->totalTime)
  {
    tvSimulator->pr = nr; // Prev RGB = new RGB
    tvSimulator->pg = ng;
    tvSimulator->pb = nb;
    SEGENV.aux0 = 0;
  }

  return FRAMETIME;
}

/*
  Aurora effect
*/

// CONFIG
#ifdef ESP8266
#define W_MAX_COUNT 9 // Number of simultaneous waves
#else
#define W_MAX_COUNT 20 // Number of simultaneous waves
#endif
#define W_MAX_SPEED 6    // Higher number, higher speed
#define W_WIDTH_FACTOR 6 // Higher number, smaller waves

// 24 bytes
class AuroraWave
{
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
  void init(uint32_t segment_length, CRGB color)
  {
    ttl = random(500, 1501);
    basecolor = color;
    basealpha = random(60, 101) / (float)100;
    age = 0;
    width = random(segment_length / 20, segment_length / W_WIDTH_FACTOR); // half of width to make math easier
    if (!width)
      width = 1;
    center = random(101) / (float)100 * segment_length;
    goingleft = random(0, 2) == 0;
    speed_factor = (random(10, 31) / (float)100 * W_MAX_SPEED / 255);
    alive = true;
  }

  CRGB getColorForLED(int ledIndex)
  {
    if (ledIndex < center - width || ledIndex > center + width)
      return 0; // Position out of range of this wave

    CRGB rgb;

    // Offset of this led from center of wave
    // The further away from the center, the dimmer the LED
    float offset = ledIndex - center;
    if (offset < 0)
      offset = -offset;
    float offsetFactor = offset / width;

    // The age of the wave determines it brightness.
    // At half its maximum age it will be the brightest.
    float ageFactor = 0.1;
    if ((float)age / ttl < 0.5)
    {
      ageFactor = (float)age / (ttl / 2);
    }
    else
    {
      ageFactor = (float)(ttl - age) / ((float)ttl * 0.5);
    }

    // Calculate color based on above factors and basealpha value
    float factor = (1 - offsetFactor) * ageFactor * basealpha;
    rgb.r = basecolor.r * factor;
    rgb.g = basecolor.g * factor;
    rgb.b = basecolor.b * factor;

    return rgb;
  };

  // Change position and age of wave
  // Determine if its sill "alive"
  void update(uint32_t segment_length, uint32_t speed)
  {
    if (goingleft)
    {
      center -= speed_factor * speed;
    }
    else
    {
      center += speed_factor * speed;
    }

    age++;

    if (age > ttl)
    {
      alive = false;
    }
    else
    {
      if (goingleft)
      {
        if (center + width < 0)
        {
          alive = false;
        }
      }
      else
      {
        if (center - width > segment_length)
        {
          alive = false;
        }
      }
    }
  };

  bool stillAlive()
  {
    return alive;
  };
};

uint16_t WS2812FX::mode_aurora(void)
{
  // aux1 = Wavecount
  // aux2 = Intensity in last loop

  AuroraWave *waves;

  if (SEGENV.aux0 != SEGMENT.intensity || SEGENV.call == 0)
  {
    // Intensity slider changed or first call
    SEGENV.aux1 = map(SEGMENT.intensity, 0, 255, 2, W_MAX_COUNT);
    SEGENV.aux0 = SEGMENT.intensity;

    if (!SEGENV.allocateData(sizeof(AuroraWave) * SEGENV.aux1))
    {                       // 26 on 32 segment ESP32, 9 on 16 segment ESP8266
      return mode_static(); // allocation failed
    }

    waves = reinterpret_cast<AuroraWave *>(SEGENV.data);

    for (int i = 0; i < SEGENV.aux1; i++)
    {
      waves[i].init(SEGLEN, col_to_crgb(color_from_palette(random8(), false, false, random(0, 3))));
    }
  }
  else
  {
    waves = reinterpret_cast<AuroraWave *>(SEGENV.data);
  }

  for (int i = 0; i < SEGENV.aux1; i++)
  {
    // Update values of wave
    waves[i].update(SEGLEN, SEGMENT.speed);

    if (!(waves[i].stillAlive()))
    {
      // If a wave dies, reinitialize it starts over.
      waves[i].init(SEGLEN, col_to_crgb(color_from_palette(random8(), false, false, random(0, 3))));
    }
  }

  uint8_t backlight = 1; // dimmer backlight if less active colors
  if (SEGCOLOR(0))
    backlight++;
  if (SEGCOLOR(1))
    backlight++;
  if (SEGCOLOR(2))
    backlight++;
  // Loop through LEDs to determine color
  for (int i = 0; i < SEGLEN; i++)
  {
    CRGB mixedRgb = CRGB(backlight, backlight, backlight);

    // For each LED we must check each wave if it is "active" at this position.
    // If there are multiple waves active on a LED we multiply their values.
    for (int j = 0; j < SEGENV.aux1; j++)
    {
      CRGB rgb = waves[j].getColorForLED(i);

      if (rgb != CRGB(0))
      {
        mixedRgb += rgb;
      }
    }

    setPixelColor(i, mixedRgb[0], mixedRgb[1], mixedRgb[2]);
  }

  return FRAMETIME;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//    Start of Audio Reactive fork (WLEDSR)                                                                                                      //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Order of routines should be alphabetical for each type. They are:
//
// 1D non-reactive
// 2D non-reactive
//
// 1D volume reactive
// 1D frequency reactive
//
// 2D volume reactive
// 2D frequency reactive
//

// Sound reactive external variables.
extern int sampleRaw;
extern float sampleAvg;
extern bool samplePeak;
extern uint8_t myVals[32];
// extern int sampleAgc;
extern int rawSampleAgc;
extern float sampleAgc;
extern uint8_t squelch;
extern byte soundSquelch;
extern byte soundAgc;
extern uint8_t maxVol;
extern uint8_t binNum;

extern float sampleReal; // "sample" as float, to provide bits that are lost otherwise. Needed for AGC.
extern float multAgc;    // sampleReal * multAgc = sampleAgc. Our multiplier

// FFT based variables
extern float FFT_MajorPeak;
extern float FFT_Magnitude;
extern float fftBin[];  // raw FFT data
extern int fftResult[]; // summary of bins array. 16 summary bins.
extern float fftAvg[];

///////////////////////////////////////
// Helper function(s)                //
///////////////////////////////////////

double mapf(double x, double in_min, double in_max, double out_min, double out_max);

////////////////////////////
//       set Pixels       //
////////////////////////////

void WS2812FX::setPixels(CRGB *leds)
{ // ewowi20210703: use segmentToLogical (rotated and mirrored) to find the right led
  for (int i = 0; i < SEGLEN; i++)
  {
    setPixelColor(i, leds[segmentToLogical(i)].red, leds[segmentToLogical(i)].green, leds[segmentToLogical(i)].blue);
  }
}

/////////////////////////////
//  Non-Reactive Routines  //
/////////////////////////////

/////////////////////////
//     Perlin Move     //
/////////////////////////

// 16 bit perlinmove. Use Perlin Noise instead of sinewaves for movement. By Andrew Tuline.
// Controls are speed, # of pixels, faderate.

uint16_t WS2812FX::mode_perlinmove(void)
{

  fade_out(255 - SEGMENT.custom1);
  for (int i = 0; i < SEGMENT.intensity / 16 + 1; i++)
  {
    uint16_t locn = inoise16(millis() * 128 / (260 - SEGMENT.speed) + i * 15000, millis() * 128 / (260 - SEGMENT.speed)); // Get a new pixel location from moving noise.
    uint16_t pixloc = map(locn, 50 * 256, 192 * 256, 0, SEGLEN) % (SEGLEN);                                               // Map that to the length of the strand, and ensure we don't go over.
    setPixelColor(pixloc, color_from_palette(pixloc % 255, false, PALETTE_SOLID_WRAP, 0));
  }

  return FRAMETIME;
} // mode_perlinmove()

/////////////////////////
//     Waveins         //
/////////////////////////

uint16_t WS2812FX::mode_wavesins(void)
{ // Uses beatsin8() + phase shifting. By: Andrew Tuline

  for (int i = 0; i < SEGLEN; i++)
  {
    uint8_t bri = sin8(millis() / 4 + i * (int)SEGMENT.intensity);
    //    leds[i] = CHSV(beatsin8(SEGMENT.speed, SEGMENT.custom1, SEGMENT.custom1+SEGMENT.custom2, 0, i * SEGMENT.custom3), 255, bri);
    leds[segmentToLogical(i)] = ColorFromPalette(currentPalette, beatsin8(SEGMENT.speed, SEGMENT.custom1, SEGMENT.custom1 + SEGMENT.custom2, 0, i * SEGMENT.custom3), bri, LINEARBLEND);
  }

  setPixels(leds);
  return FRAMETIME;
} // mode_waveins()

//////////////////////////////
//     Flow Stripe          //
//////////////////////////////

uint16_t WS2812FX::mode_FlowStripe(void)
{ // By: ldirko  https://editor.soulmatelights.com/gallery/392-flow-led-stripe , modifed by: Andrew Tuline

  const float hl = SEGLEN / 1.3;
  uint8_t hue = millis() / (SEGMENT.speed + 1);
  int t = millis() / (SEGMENT.intensity / 8 + 1);

  for (int i = 0; i < SEGLEN; i++)
  {
    int c = (abs(i - hl) / hl) * 127;
    c = sin8(c);
    c = sin8(c / 2 + t);
    byte b = sin8(c + t / 8);
    leds[segmentToLogical(i)] = CHSV(b + hue, 255, 255);
  }

  setPixels(leds);
  return FRAMETIME;
} // mode_FlowStripe()

//////////////////////////////////////////////
//     START of 2D NON-REACTIVE ROUTINES    //
//////////////////////////////////////////////

// static uint16_t x = 0;
// static uint16_t y = 0;
// static uint16_t z = 0;
// static int speed2D = 20;

// uint8_t colorLoop = 1;

// Scale determines how far apart the pixels in our noise matrix are.  Try
// changing these values around to see how it affects the motion of the display.  The
// higher the value of scale, the more "zoomed out" the noise iwll be.  A value
// of 1 will be so zoomed in, you'll mostly see solid colors.
// static int scale_2d = 30; // scale is set dynamically once we've started up

// blur1d: one-dimensional blur filter. Spreads light to 2 line neighbors.
// blur2d: two-dimensional blur filter. Spreads light to 8 XY neighbors.
//
//           0 = no spread at all
//          64 = moderate spreading
//         172 = maximum smooth, even spreading
//
//         173..255 = wider spreading, but increasing flicker
//
//         Total light is NOT entirely conserved, so many repeated
//         calls to 'blur' will also result in the light fading,
//         eventually all the way to black; this is by design so that
//         it can be used to (slowly) clear the LEDs to black.
void WS2812FX::blur1d(CRGB *leds, fract8 blur_amount)
{
  uint8_t keep = 255 - blur_amount;
  uint8_t seep = blur_amount >> 1;
  CRGB carryover = CRGB::Black;
  for (uint16_t x = 0; x <= SEGMENT.width; x++)
    for (uint16_t y = 0; y <= SEGMENT.height; y++)
    { // ewowi20210629: <= to blur all pixels
      CRGB cur = leds[XY(x, y)];
      CRGB part = cur;
      part.nscale8(seep);
      cur.nscale8(keep);
      cur += carryover;
      if (x > 0) // ewowi20210701: need to test if y test also necessary
        leds[XY(x - 1, y)] += part;
      // else if (y)
      //   leds[XY(x,y-1)] += part;
      leds[XY(x, y)] = cur;
      carryover = part;
    }
}

void WS2812FX::blur2d(CRGB *leds, fract8 blur_amount)
{
  blurRows(leds, blur_amount);
  blurColumns(leds, blur_amount);
}

// blurRows: perform a blur1d on every row of a rectangular matrix
void WS2812FX::blurRows(CRGB *leds, fract8 blur_amount)
{
  blur1d(leds, blur_amount); // ewowi20210629: this will do all rows of the segment
                             //  for( uint8_t row = 0; row < height; row++) {
                             //      CRGB* rowbase = leds + (row * width);
                             //      blur1d( rowbase, width, blur_amount);
                             //  }
}

// blurColumns: perform a blur1d on each column of a rectangular matrix
void WS2812FX::blurColumns(CRGB *leds, fract8 blur_amount)
{
  // blur columns
  uint8_t keep = 255 - blur_amount;
  uint8_t seep = blur_amount >> 1;
  for (uint16_t col = 0; col < SEGMENT.width; col++)
  {
    CRGB carryover = CRGB::Black;
    for (uint16_t i = 0; i < SEGMENT.height; i++)
    {
      CRGB cur = leds[XY(col, i)];
      CRGB part = cur;
      part.nscale8(seep);
      cur.nscale8(keep);
      cur += carryover;
      if (i)
        leds[XY(col, i - 1)] += part;
      leds[XY(col, i)] = cur;
      carryover = part;
    }
  }
}

// ewowi20210628: new functions moved from colorutils: add segment awareness

void WS2812FX::fill_solid(struct CRGB *leds, const struct CRGB &color)
{
  for (uint16_t x = 0; x <= SEGMENT.width; x++)
    for (uint16_t y = 0; y <= SEGMENT.height; y++)
    {
      leds[XY(x, y)] = color;
    }
}

void WS2812FX::fadeToBlackBy(CRGB *leds, uint8_t fadeBy)
{
  nscale8(leds, 255 - fadeBy);
}

void WS2812FX::nscale8(CRGB *leds, uint8_t scale)
{
  for (uint16_t x = 0; x <= SEGMENT.width; x++)
    for (uint16_t y = 0; y <= SEGMENT.height; y++)
    {
      leds[XY(x, y)].nscale8(scale);
    }
}

uint16_t WS2812FX::XY(uint16_t x, uint16_t y)
{ // ewowi20210703: new XY: segmentToReal: Maps XY in 2D segment to to rotated and mirrored logical index. Works for 1D strips and 2D panels
  return segmentToLogical(x % SEGMENT.width + y % SEGMENT.height * SEGMENT.width);
}

// Use https://wokwi.com/arduino/projects/300565972972995085 to create layout examples
#define RIGHT 1
#define BOTTOM 1
#define HORIZONTAL 0
uint16_t WS2812FX::logicalToPhysical(uint16_t i)
{ // ewowi20210624: previous XY. Maps logical led index to physical led index. Works for 1D strips and 2D panels
  // By Sutaburosu (major and minor flip) and Ewoud Wijma (panels)

  int x = i % matrixWidth;
  int y = matrixWidth ? i / matrixWidth : 0;

  if (x >= matrixWidth || y >= matrixHeight)
    return SEGLEN + 1; // Off the charts, so it's only useable by routines that use leds[x]!!!!
  uint16_t major, minor, sz_major, sz_minor;

  // Width, Height and Size of panel. Same as matrixWidth and Height if only one panel
  uint16_t panelWidth = (matrixPanels && matrixHorizontalPanels) ? (matrixWidth / matrixHorizontalPanels) : matrixWidth;
  uint16_t panelHeight = (matrixPanels && matrixVerticalPanels) ? (matrixHeight / matrixVerticalPanels) : matrixHeight;
  uint16_t panelSize = panelWidth * panelHeight;

  // Horizontal and vertical panel number. 0 if only one panel
  uint8_t panelHorizontalNr = x / panelWidth;
  uint8_t panelVerticalNr = y / panelHeight;

  uint16_t panelFirstLed = 0; // 0 if only one panel
  if (panelOrientationHorVert == HORIZONTAL)
  {
    if (matrixPanels)
      panelFirstLed = panelSize * (panelHorizontalNr + matrixHorizontalPanels * panelVerticalNr);
    major = x % panelWidth, minor = y % panelHeight, sz_major = panelWidth, sz_minor = panelHeight;
  }
  else
  { // vertical
    if (matrixPanels)
      panelFirstLed = panelSize * (panelVerticalNr + matrixVerticalPanels * panelHorizontalNr);
    major = y % panelHeight, minor = x % panelWidth, sz_major = panelHeight, sz_minor = panelWidth;
  }

  bool flipmajor = (panelOrientationHorVert == HORIZONTAL) ? panelFirstLedLeftRight == RIGHT : panelFirstLedTopBottom == BOTTOM;
  bool flipminor = (panelOrientationHorVert == HORIZONTAL) ? panelFirstLedTopBottom == BOTTOM : panelFirstLedLeftRight == RIGHT;
  // By: Sutaburosu -  Who wrote this VERY COOL and VERY short and MUCH better XY() routine. Thanks!!
  // flip minor if needed, this needs to be done before flipmajor because minor value needed to identify serpentine row
  if (flipminor)
    minor = sz_minor - 1 - minor;
  if (flipmajor ^ ((minor & 1) && panelSerpentine))
    major = sz_major - 1 - major; // A line of magic.
  // &=Binary AND, minor&1 is odd rows, ^=Binary XOR => flapmajor or serpentine (odd) row, but not both (XOR)

  if (panelTranspose)
    return major * (uint16_t)sz_minor + minor + panelFirstLed;
  else
    return minor * (uint16_t)sz_major + major + panelFirstLed;
}

uint16_t WS2812FX::mode_2DBlackHole()
{ // By: Stepko https://editor.soulmatelights.com/gallery/1012 , Modified by: Andrew Tuline

  fadeToBlackBy(leds, 32);
  double t = (float)(millis()) / 128;
  for (byte i = 0; i < 8; i++)
  {
    leds[XY(beatsin8(SEGMENT.custom1 / 8, 0, SEGMENT.width - 1, 0, ((i % 2) ? 128 : 0) + t * i), beatsin8(10, 0, SEGMENT.height - 1, 0, ((i % 2) ? 192 : 64) + t * i))] += CHSV(i * 32, 255, 255);
  }
  for (byte i = 0; i < 8; i++)
  {
    leds[XY(beatsin8(SEGMENT.custom2 / 8, SEGMENT.width / 4, SEGMENT.width - 1 - SEGMENT.width / 4, 0, ((i % 2) ? 128 : 0) + t * i), beatsin8(SEGMENT.custom3 / 8, SEGMENT.height / 4, SEGMENT.height - 1 - SEGMENT.height / 4, 0, ((i % 2) ? 192 : 64) + t * i))] += CHSV(i * 32, 255, 255);
  }
  leds[XY(SEGMENT.width / 2, SEGMENT.height / 2)] = CHSV(0, 0, 255);
  blur2d(leds, 16);

  setPixels(leds);
  return FRAMETIME;
} // mode_2DBlackHole()

////////////////////////////
//     2D Colored Bursts  //
////////////////////////////

uint16_t WS2812FX::mode_2DColoredBursts()
{ // By: ldirko   https://editor.soulmatelights.com/gallery/819-colored-bursts , modified by: Andrew Tuline

  bool dot = false;
  bool grad = true;

  static byte hue = 0;
  static byte numLines = 10;

  hue++;
  numLines = SEGMENT.intensity / 16;
  fadeToBlackBy(leds, 40);

  for (byte i = 0; i < numLines; i++)
  {
    byte x1 = beatsin8(2 + SEGMENT.speed / 16, 0, (SEGMENT.width - 1));
    byte x2 = beatsin8(1 + SEGMENT.speed / 16, 0, (SEGMENT.width - 1));
    byte y1 = beatsin8(5 + SEGMENT.speed / 16, 0, (SEGMENT.height - 1), 0, i * 24);
    byte y2 = beatsin8(3 + SEGMENT.speed / 16, 0, (SEGMENT.height - 1), 0, i * 48 + 64);
    CRGB color = ColorFromPalette(currentPalette, i * 255 / numLines + hue, 255, LINEARBLEND);

    byte xsteps = abs8(x1 - y1) + 1;
    byte ysteps = abs8(x2 - y2) + 1;
    byte steps = xsteps >= ysteps ? xsteps : ysteps;

    for (byte i = 1; i <= steps; i++)
    {
      byte dx = lerp8by8(x1, y1, i * 255 / steps);
      byte dy = lerp8by8(x2, y2, i * 255 / steps);
      int index = XY(dx, dy);
      leds[index] += color; // change to += for brightness look
      if (grad)
        leds[index] %= (i * 255 / steps); // Draw gradient line
    }

    if (dot)
    { // add white point at the ends of line
      leds[XY(x1, x2)] += CRGB::White;
      leds[XY(y1, y2)] += CRGB::White;
    }
  }
  blur2d(leds, 4);

  setPixels(leds); // Use this ONLY if we're going to display via leds[x] method.
  return FRAMETIME;
} // mode_2DColoredBursts()

/////////////////////
//      2D DNA     //
/////////////////////

uint16_t WS2812FX::mode_2Ddna(void)
{ // dna originally by by ldirko at https://pastebin.com/pCkkkzcs. Updated by Preyy. WLED conversion by Andrew Tuline.

  fadeToBlackBy(leds, 64);

  for (int i = 0; i < SEGMENT.width; i++)
  { // change to height if you want to re-orient, and swap the 4 lines below.
    //     leds[XY(beatsin8(SEGMENT.speed/8, 0, SEGMENT.width-1, 0, i*4), i)] = ColorFromPalette(currentPalette, i*5+millis()/17, beatsin8(5, 55, 255, 0, i*10), LINEARBLEND);
    //     leds[XY(beatsin8(SEGMENT.speed/8, 0, SEGMENT.width-1, 0, i*4+128), i)] = ColorFromPalette(currentPalette,i*5+128+millis()/17, beatsin8(5, 55, 255, 0, i*10+128), LINEARBLEND);        // 180 degrees (128) out of phase
    leds[XY(i, beatsin8(SEGMENT.speed / 8, 0, SEGMENT.height - 1, 0, i * 4))] = ColorFromPalette(currentPalette, i * 5 + millis() / 17, beatsin8(5, 55, 255, 0, i * 10), LINEARBLEND);
    leds[XY(i, beatsin8(SEGMENT.speed / 8, 0, SEGMENT.height - 1, 0, i * 4 + 128))] = ColorFromPalette(currentPalette, i * 5 + 128 + millis() / 17, beatsin8(5, 55, 255, 0, i * 10 + 128), LINEARBLEND); // 180 degrees (128) out of phase
  }

  blur2d(leds, SEGMENT.intensity / 8);

  setPixels(leds);

  return FRAMETIME;
} // mode_2Ddna()

/////////////////////////
//     2D DNA Spiral   //
/////////////////////////

uint16_t WS2812FX::mode_2DDNASpiral()
{ // By: ldirko  https://editor.soulmatelights.com/gallery/810 , modified by: Andrew Tuline

  uint8_t speeds = SEGMENT.speed / 2;
  uint8_t freq = SEGMENT.intensity / 8;

  static byte hue = 0;
  int ms = millis() / 20;
  nscale8(leds, 120);

  for (int i = 0; i < SEGMENT.height; i++)
  {
    int x = beatsin8(speeds, 0, SEGMENT.width - 1, 0, i * freq) + beatsin8(speeds - 7, 0, SEGMENT.width - 1, 0, i * freq + 128);
    int x1 = beatsin8(speeds, 0, SEGMENT.width - 1, 0, 128 + i * freq) + beatsin8(speeds - 7, 0, SEGMENT.width - 1, 0, 128 + 64 + i * freq);
    hue = i * 128 / SEGMENT.width + ms; // ewowi20210629: not width - 1 to avoid crash if width = 1
    if ((i + ms / 8) & 3)
    {
      x = x / 2;
      x1 = x1 / 2;
      byte steps = abs8(x - x1) + 1;
      for (byte k = 1; k <= steps; k++)
      {
        byte dx = lerp8by8(x, x1, k * 255 / steps);
        int index = XY(dx, i);
        leds[index] += ColorFromPalette(currentPalette, hue, 255, LINEARBLEND);
        leds[index] %= (k * 255 / steps); // for draw gradient line
      }
      leds[XY(x, i)] += CRGB::DarkSlateGray;
      leds[XY(x1, i)] += CRGB::White;
    }
  }

  setPixels(leds); // Use this ONLY if we're going to display via leds[x] method.
  return FRAMETIME;
} // mode_2DDNASpiral()

/////////////////////////
//     2D Drift        //
/////////////////////////

uint16_t WS2812FX::mode_2DDrift()
{ // By: Stepko   https://editor.soulmatelights.com/gallery/884-drift , Modified by: Andrew Tuline

#define CenterX ((SEGMENT.width / 2) - 0.5)
#define CenterY ((SEGMENT.height / 2) - 0.5)
  const byte maxDim = max(SEGMENT.width, SEGMENT.height);
  fadeToBlackBy(leds, 128);
  unsigned long t = millis() / (32 - SEGMENT.speed / 8);
  for (float i = 1; i < maxDim / 2; i += 0.25)
  {
    double angle = radians(t * (maxDim / 2 - i));
    int myX = (int)(CenterX + sin(angle) * i);
    int myY = (int)(CenterY + cos(angle) * i);
    leds[XY(myX, myY)] += ColorFromPalette(currentPalette, (i * 20) + (t / 20), 255, LINEARBLEND);
  }
  blur2d(leds, SEGMENT.intensity / 8);

  setPixels(leds); // Use this ONLY if we're going to display via leds[x] method.
  return FRAMETIME;
} // mode_2DDrift()

/////////////////////////
//     2D Fire2012     //
/////////////////////////

uint16_t WS2812FX::mode_2Dfire2012(void)
{ // Fire2012 by Mark Kriegsman. Converted to WLED by Andrew Tuline.

  const uint8_t COOLING = 50;
  const uint8_t SPARKING = 50;

  CRGBPalette16 currentPalette = CRGBPalette16(CRGB::Black, CRGB::Red, CRGB::Orange, CRGB::Yellow);

  if (millis() - SEGENV.step >= ((256 - SEGMENT.speed) >> 2))
  {
    SEGENV.step = millis();
    // static byte *heat = (uint16_t *)dataStore;

    if (!SEGENV.allocateData(sizeof(byte) * 4096))
      return mode_static(); // allocation failed
    byte *heat = reinterpret_cast<byte *>(SEGENV.data);

    for (uint16_t mw = 0; mw < SEGMENT.width; mw++)
    { // Move along the width of the flame

      uint16_t cell;
      // Step 1.  Cool down every cell a little
      for (uint16_t mh = 0; mh < SEGMENT.height; mh++)
      {
        cell = (mw * SEGMENT.width + mh) % 4096;
        heat[cell] = qsub8(heat[cell], random16(0, ((COOLING * 10) / SEGMENT.height) + 2));
      }

      // Step 2.  Heat from each cell drifts 'up' and diffuses a little
      for (uint16_t mh = SEGMENT.height - 1; mh >= 2; mh--)
      {
        cell = (mw * SEGMENT.width + mh) % 4096;
        heat[cell] = (heat[cell - 1] + heat[cell - 2] + heat[cell - 2]) / 3;
      }

      // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
      if (random8(0, 255) < SPARKING)
      {
        uint8_t mh = random8(3);
        cell = (mw * SEGMENT.width + mh) % 4096;
        heat[cell] = qadd8(heat[cell], random8(160, 255));
      }

      // Step 4.  Map from heat cells to LED colors
      for (uint16_t mh = 0; mh < SEGMENT.height; mh++)
      {
        cell = (mw * SEGMENT.width + mh) % 4096;
        byte colorindex = scale8(heat[cell], 240);
        uint16_t pixelnumber = (SEGMENT.height - 1) - mh;                              // Flip it upside down.
        leds[XY(mw, pixelnumber)] = ColorFromPalette(currentPalette, colorindex, 255); // Otherwise, it was leds[XY(mw,mh)] = . . .
      }                                                                                // for mh
    }                                                                                  // for mw

    setPixels(leds);

  } // if millis

  return FRAMETIME;
} // mode_2Dfire2012()

//////////////////////////
//     2D Firenoise     //
//////////////////////////

uint16_t WS2812FX::mode_2Dfirenoise(void)
{ // firenoise2d. By Andrew Tuline. Yet another short routine.

  uint16_t xscale = SEGMENT.intensity * 4;
  //  uint32_t xscale = 600;                                  // How far apart they are
  //  uint32_t yscale = 1000;                                 // How fast they move
  uint32_t yscale = SEGMENT.speed * 8;
  uint8_t indexx = 0;

  currentPalette = CRGBPalette16(CRGB(0, 0, 0), CRGB(0, 0, 0), CRGB(0, 0, 0), CRGB(0, 0, 0),
                                 CRGB::Red, CRGB::Red, CRGB::Red, CRGB::DarkOrange,
                                 CRGB::DarkOrange, CRGB::DarkOrange, CRGB::Orange, CRGB::Orange,
                                 CRGB::Yellow, CRGB::Orange, CRGB::Yellow, CRGB::Yellow);

  for (int j = 0; j < SEGMENT.width; j++)
  {
    for (int i = 0; i < SEGMENT.height; i++)
    {

      indexx = inoise8(j * yscale * SEGMENT.height / 255, i * xscale + millis() / 4);                                       // We're moving along our Perlin map.
      leds[XY(j, i)] = ColorFromPalette(currentPalette, min(i * (indexx) >> 4, 255), i * 255 / SEGMENT.width, LINEARBLEND); // With that value, look up the 8 bit colour palette value and assign it to the current LED.

      // This perlin fire is by /u/ldirko
      //      int a = millis();
      //      leds[XY(i,j)] = ColorFromPalette (currentPalette, qsub8(inoise8 (i * 60 , j * 60+ a , a /3), abs8(j - (SEGMENT.height-1)) * 255 / (SEGMENT.height-1)), 255);

    } // for i
  }   // for j

  setPixels(leds);

  return FRAMETIME;
} // mode_2Dfirenoise()

//////////////////////////////
//     2D Frizzles          //
//////////////////////////////

uint16_t WS2812FX::mode_2DFrizzles(void)
{ // By: Stepko https://editor.soulmatelights.com/gallery/640-color-frizzles , Modified by: Andrew Tuline

  fadeToBlackBy(leds, 16);
  for (int i = 8; i > 0; i--)
  { // WLEDSR bugfix
    leds[XY(beatsin8(SEGMENT.speed / 8 + i, 0, SEGMENT.width - 1), beatsin8(SEGMENT.intensity / 8 - i, 0, SEGMENT.height - 1))] += ColorFromPalette(currentPalette, beatsin8(12, 0, 255), 255, LINEARBLEND);
  }
  blur2d(leds, 16);

  setPixels(leds);
  return FRAMETIME;
} // mode_2DFrizzles()

///////////////////////////////////////////
//   2D Cellular Automata Game of life   //
///////////////////////////////////////////

typedef struct ColorCount
{
  CRGB color;
  int8_t count;
} colorCount;

uint16_t WS2812FX::mode_2Dgameoflife(void)
{ // Written by Ewoud Wijma, inspired by https://natureofcode.com/book/chapter-7-cellular-automata/ and https://github.com/DougHaber/nlife-color

  // slow down based on speed parameter
  if (millis() - SEGENV.step >= ((255 - SEGMENT.speed) * 4))
  {
    SEGENV.step = millis();

    CRGB prevLeds[32 * 32]; // MAX_LED causes a panic, but this will do

    // array of patterns. Needed to identify repeating patterns. A pattern is one iteration of leds, without the color (on/off only)
    const int patternsSize = (SEGMENT.width + SEGMENT.height) * 2; // seems to be a good value to catch also repetition in moving patterns
    if (!SEGENV.allocateData(sizeof(String) * patternsSize))
      return mode_static(); // allocation failed
    String *patterns = reinterpret_cast<String *>(SEGENV.data);

    CRGB backgroundColor = SEGCOLOR(1);

    static unsigned long resetMillis; // triggers reset if more than 3 seconds from millis()

    if (SEGENV.call == 0)
    { // effect starts
      // check if no pixels on screen (there could be due to previous effect, which we then take as starting point)
      bool allZero = true;
      for (int x = 0; x < SEGMENT.width && allZero; x++)
        for (int y = 0; y < SEGMENT.height && allZero; y++)
          if (leds[XY(x, y)].r > 10 || leds[XY(x, y)].g > 10 || leds[XY(x, y)].b > 10) // looks like some pixels are not completely off
            allZero = false;
      if (!allZero)
        resetMillis = millis(); // avoid reset
    }

    // reset leds if effect repeats (wait 3 seconds after repetition)
    if (millis() - resetMillis > 3000)
    {
      resetMillis = millis();

      random16_set_seed(millis()); // seed the random generator

      // give the leds random state and colors (based on intensity, colors from palette or all posible colors are chosen)
      for (int x = 0; x < SEGMENT.width; x++)
        for (int y = 0; y < SEGMENT.height; y++)
        {
          uint8_t state = random8() % 2;
          if (state == 0)
            leds[XY(x, y)] = backgroundColor;
          else
            leds[XY(x, y)] = SEGMENT.intensity < 128 ? (CRGB)color_wheel(random8()) : CRGB(random8(), random8(), random8());
        }

      // init patterns
      SEGENV.aux0 = 0; // ewowi20210629: pka static! patternsize: round robin index of next slot to add pattern
      for (int i = 0; i < patternsSize; i++)
        patterns[i] = "";
    }
    else
    {
      // copy previous leds
      for (int x = 0; x < SEGMENT.width; x++)
        for (int y = 0; y < SEGMENT.height; y++)
          prevLeds[XY(x, y)] = leds[XY(x, y)];

      // calculate new leds
      for (int x = 0; x < SEGMENT.width; x++)
        for (int y = 0; y < SEGMENT.height; y++)
        {
          colorCount colorsCount[9]; // count the different colors in the 9*9 matrix
          for (int i = 0; i < 9; i++)
            colorsCount[i] = {backgroundColor, 0}; // init colorsCount

          // iterate through neighbors and count them and their different colors
          int neighbors = 0;
          for (int i = -1; i <= 1; i++)
            for (int j = -1; j <= 1; j++)
            {                                                                                                       // iterate through 9*9 matrix
              uint16_t xy = XY((x + i + SEGMENT.width) % SEGMENT.width, (y + j + SEGMENT.height) % SEGMENT.height); // cell xy to check

              // count different neighbours and colors, except the centre cell
              if (xy != XY(x, y) && prevLeds[xy] != backgroundColor)
              {
                neighbors++;
                bool colorFound = false;
                int i;
                for (i = 0; i < 9 && colorsCount[i].count != 0; i++)
                  if (colorsCount[i].color == prevLeds[xy])
                  {
                    colorsCount[i].count++;
                    colorFound = true;
                  }

                if (!colorFound)
                  colorsCount[i] = {prevLeds[xy], 1}; // add new color found in the array
              }
            } // i,j

          // Rules of Life
          if ((leds[XY(x, y)] != backgroundColor) && (neighbors < 2))
            leds[XY(x, y)] = backgroundColor; // Loneliness
          else if ((leds[XY(x, y)] != backgroundColor) && (neighbors > 3))
            leds[XY(x, y)] = backgroundColor; // Overpopulation
          else if ((leds[XY(x, y)] == backgroundColor) && (neighbors == 3))
          { // Reproduction
            // find dominantcolor and assign to cell
            colorCount dominantColorCount = {backgroundColor, 0};
            for (int i = 0; i < 9 && colorsCount[i].count != 0; i++)
              if (colorsCount[i].count > dominantColorCount.count)
                dominantColorCount = colorsCount[i];
            if (dominantColorCount.count > 0)
              leds[XY(x, y)] = dominantColorCount.color; // assign the dominant color
          }
          // else do nothing!
        } // x,y

      // create new pattern
      String pattern = "";
      for (int x = 0; x < SEGMENT.width; x += MAX(SEGMENT.width / 8, 1))
        for (int y = 0; y < SEGMENT.height; y += MAX(SEGMENT.height / 8, 1))
          pattern += leds[XY(x, y)] == backgroundColor ? " " : "o"; // string representation if on/off

      // check if repetition of patterns occurs
      bool repetition = false;
      for (int i = 0; i < patternsSize && !repetition; i++)
        repetition = patterns[(SEGENV.aux0 - 1 - i + patternsSize) % patternsSize] == pattern;

      // add current pattern to array and increase index (round robin)
      patterns[SEGENV.aux0] = pattern;
      SEGENV.aux0 = (SEGENV.aux0 + 1) % patternsSize;

      if (!repetition)
        resetMillis = millis(); // if no repetition avoid reset
    }                           // not reset

    setPixels(leds);
  } // millis

  return FRAMETIME;
} // mode_2Dgameoflife()

/////////////////////////
//     2D Hiphotic     //
/////////////////////////

uint16_t WS2812FX::mode_2DHiphotic()
{ //  By: ldirko  https://editor.soulmatelights.com/gallery/810 , Modified by: Andrew Tuline

  int a = millis() / 8;

  for (int x = 0; x < SEGMENT.width; x++)
  {
    for (int y = 0; y < SEGMENT.height; y++)
    {
      int index = XY(x, y);
      //      leds[index].b = sin8((x - 8) * cos8((y + 20) * 4) / 4 + a);
      //      leds[index].g = (sin8(x * 16 + a / 3) + cos8(y * 8 + a / 2)) / 2;
      //      leds[index].r = sin8(cos8(x * 8 + a / 3) + sin8(y * 8 + a / 4) + a);
      //      leds[index] = ColorFromPalette(currentPalette, sin8(cos8(x * 8 + a / 3) + sin8(y * 8 + a / 4) + a), 255, LINEARBLEND);
      leds[index] = ColorFromPalette(currentPalette, sin8(cos8(x * SEGMENT.speed / 16 + a / 3) + sin8(y * SEGMENT.intensity / 16 + a / 4) + a), 255, LINEARBLEND);
    }
  }

  setPixels(leds); // Use this ONLY if we're going to display via leds[x] method.
  return FRAMETIME;
} // mode_2DHiphotic()

/////////////////////////
//     2D Julia        //
/////////////////////////

// Sliders are:
//
// intensity = Maximum number of iterations per pixel.
// Custom1 = Location of X centerpoint
// Custom2 = Location of Y centerpoint
// Custom3 = Size of the area (small value = smaller area)

typedef struct Julia
{ // We can't use the 'static' keyword for persistent variables, so we have to go the LONG route to support them.
  float xcen;
  float ycen;
  float xymag;
} julia;

uint16_t WS2812FX::mode_2DJulia(void)
{ // An animated Julia set by Andrew Tuline.

  if (!SEGENV.allocateData(sizeof(julia)))
    return mode_static();                                 // We use this method for allocating memory for static variables.
  Julia *julias = reinterpret_cast<Julia *>(SEGENV.data); // Because 'static' doesn't work with SEGMENTS.

  float reAl;
  float imAg;

  if (SEGENV.call == 0)
  { // Reset the center if we've just re-started this animation.
    julias->xcen = 0.;
    julias->ycen = 0.;
    julias->xymag = 1.0;

    SEGMENT.custom1 = 128; // Make sure the location widgets are centered to start. Too bad
    SEGMENT.custom2 = 128; // it doesn't show up on the UI.
    SEGMENT.custom3 = 128;
    SEGMENT.intensity = 24;
  }

  julias->xcen = julias->xcen + (float)(SEGMENT.custom1 - 128) / 100000.;
  julias->ycen = julias->ycen + (float)(SEGMENT.custom2 - 128) / 100000.;
  julias->xymag = julias->xymag + (float)(SEGMENT.custom3 - 128) / 100000.;
  if (julias->xymag < 0.01)
    julias->xymag = 0.01;
  if (julias->xymag > 1.0)
    julias->xymag = 1.0;

  float xmin = julias->xcen - julias->xymag;
  float xmax = julias->xcen + julias->xymag;
  float ymin = julias->ycen - julias->xymag;
  float ymax = julias->ycen + julias->xymag;

  // Whole set should be within -1.2,1.2 to -.8 to 1.
  xmin = constrain(xmin, -1.2, 1.2);
  xmax = constrain(xmax, -1.2, 1.2);
  ymin = constrain(ymin, -.8, 1.0);
  ymax = constrain(ymax, -.8, 1.0);

  float dx; // Delta x is mapped to the matrix size.
  float dy; // Delta y is mapped to the matrix size.

  int maxIterations = 15; // How many iterations per pixel before we give up. Make it 8 bits to match our range of colours.
  float maxCalc = 16.0;   // How big is each calculation allowed to be before we give up.

  maxIterations = SEGMENT.intensity / 2;

  // Resize section on the fly for some animaton.
  reAl = -0.94299; // PixelBlaze example
  imAg = 0.3162;

  reAl += sin((float)millis() / 305.) / 20.;
  imAg += sin((float)millis() / 405.) / 20.;

  //  Serial.print(reAl,4); Serial.print("\t"); Serial.print(imAg,4); Serial.println(" ");

  dx = (xmax - xmin) / (SEGMENT.width); // Scale the delta x and y values to our matrix size.
  dy = (ymax - ymin) / (SEGMENT.height);

  // Start y
  float y = ymin;
  for (int j = 0; j < SEGMENT.height; j++)
  {

    // Start x
    float x = xmin;
    for (int i = 0; i < SEGMENT.width; i++)
    {

      // Now we test, as we iterate z = z^2 + c does z tend towards infinity?
      float a = x;
      float b = y;
      int iter = 0;

      while (iter < maxIterations)
      { // Here we determine whether or not we're out of bounds.
        float aa = a * a;
        float bb = b * b;
        float len = aa + bb;
        if (len > maxCalc)
        {        // |z| = sqrt(a^2+b^2) OR z^2 = a^2+b^2 to save on having to perform a square root.
          break; // Bail
        }

        // This operation corresponds to z -> z^2+c where z=a+ib c=(x,y). Remember to use 'foil'.
        b = 2 * a * b + imAg;
        a = aa - bb + reAl;
        iter++;
      } // while

      // We color each pixel based on how long it takes to get to infinity, or black if it never gets there.
      if (iter == maxIterations)
      {
        //        leds[XY(i,j)] = CRGB::Black;            // Calculation kept on going, so it was within the set.
        setPixelColor(XY(i, j), 0);
      }
      else
      {
        //        leds[XY(i,j)] = CHSV(iter*255/maxIterations,255,255);   // Near the edge of the set.
        setPixelColor(XY(i, j), color_from_palette(iter * 255 / maxIterations, false, PALETTE_SOLID_WRAP, 0));
      }
      x += dx;
    }
    y += dy;
  }

  //  blur2d( leds, 64);

  //  setPixels(leds);       // Use this ONLY if we're going to display via leds[x] method.
  return FRAMETIME;

} // mode_2DJulia()

//////////////////////////////
//     2D Lissajous         //
//////////////////////////////

uint16_t WS2812FX::mode_2DLissajous(void)
{ // By: Andrew Tuline

  fadeToBlackBy(leds, SEGMENT.intensity);

  for (int i = 0; i < 256; i++)
  {

    uint8_t xlocn = sin8(millis() / 2 + i * SEGMENT.speed / 64);
    uint8_t ylocn = cos8(millis() / 2 + i * 128 / 64);

    xlocn = map(xlocn, 0, 255, 0, SEGMENT.width - 1);
    ylocn = map(ylocn, 0, 255, 0, SEGMENT.height - 1);
    leds[XY(xlocn, ylocn)] = ColorFromPalette(currentPalette, millis() / 100 + i, 255, LINEARBLEND);
  }

  setPixels(leds);
  return FRAMETIME;
} // mode_2DLissajous()

///////////////////////
//    2D Matrix      //
///////////////////////

uint16_t WS2812FX::mode_2Dmatrix(void)
{ // Matrix2D. By Jeremy Williams. Adapted by Andrew Tuline & improved by merkisoft and ewowi.

  if (SEGENV.call == 0)
    fill_solid(leds, 0);

  int fade = map(SEGMENT.custom1, 0, 255, 50, 250);                                 // equals trail size
  int speed = (256 - SEGMENT.speed) >> map(MIN(SEGMENT.height, 150), 0, 150, 0, 3); // slower speeds for small displays

  CRGB spawnColor;
  CRGB trailColor;

  if (SEGMENT.custom2 > 128)
  {
    spawnColor = SEGCOLOR(0);
    trailColor = SEGCOLOR(1);
  }
  else
  {
    spawnColor = CRGB(175, 255, 175);
    trailColor = CRGB(27, 130, 39);
  }

  if (millis() - SEGENV.step >= speed)
  {
    SEGENV.step = millis();
    // if (SEGMENT.custom3 < 128) {									            // check for orientation, slider in first quarter, default orientation
    for (int16_t row = SEGMENT.height - 1; row >= 0; row--)
    {
      for (int16_t col = 0; col < SEGMENT.width; col++)
      {
        if (leds[XY(col, row)] == spawnColor)
        {
          leds[XY(col, row)] = trailColor; // create trail
          if (row < SEGMENT.height - 1)
            leds[XY(col, row + 1)] = spawnColor;
        }
      }
    }

    // fade all leds
    for (int x = 0; x < SEGMENT.width; x++)
      for (int y = 0; y < SEGMENT.height; y++)
      {
        if (leds[XY(x, y)] != spawnColor)
          leds[XY(x, y)].nscale8(fade); // only fade trail
      }

    // check for empty screen to ensure code spawn
    bool emptyScreen = true;
    for (int x = 0; x < SEGMENT.width; x++)
      for (int y = 0; y < SEGMENT.height; y++)
      {
        if (leds[XY(x, y)])
        {
          emptyScreen = false;
          break;
        }
      }

    // spawn new falling code
    // if (SEGMENT.custom3 <=255) {
    if (random8() < SEGMENT.intensity || emptyScreen)
    {
      uint8_t spawnX = random8(SEGMENT.width);
      leds[XY(spawnX, 0)] = spawnColor;
    }

    setPixels(leds);
  } // if millis

  return FRAMETIME;
} // mode_2Dmatrix()

/////////////////////////
//     2D Metaballs    //
/////////////////////////

uint16_t WS2812FX::mode_2Dmetaballs(void)
{ // Metaballs by Stefan Petrick. Cannot have one of the dimensions be 2 or less. Adapted by Andrew Tuline.

  float speed = 1;

  // get some 2 random moving points
  uint8_t x2 = inoise8(millis() * speed, 25355, 685) / 16;
  uint8_t y2 = inoise8(millis() * speed, 355, 11685) / 16;

  uint8_t x3 = inoise8(millis() * speed, 55355, 6685) / 16;
  uint8_t y3 = inoise8(millis() * speed, 25355, 22685) / 16;

  // and one Lissajou function
  uint8_t x1 = beatsin8(23 * speed, 0, 15);
  uint8_t y1 = beatsin8(28 * speed, 0, 15);

  for (uint16_t y = 0; y < SEGMENT.height; y++)
  {
    for (uint16_t x = 0; x < SEGMENT.width; x++)
    {

      // calculate distances of the 3 points from actual pixel
      // and add them together with weightening
      uint16_t dx = abs(x - x1);
      uint16_t dy = abs(y - y1);
      uint16_t dist = 2 * sqrt((dx * dx) + (dy * dy));

      dx = abs(x - x2);
      dy = abs(y - y2);
      dist += sqrt((dx * dx) + (dy * dy));

      dx = abs(x - x3);
      dy = abs(y - y3);
      dist += sqrt((dx * dx) + (dy * dy));

      // inverse result
      byte color = 1000 / dist; // dist=0?1000: 1000 / dist;

      // map color between thresholds
      if (color > 0 and color < 60)
      {
        leds[XY(x, y)] = ColorFromPalette(currentPalette, color * 9, 255);
      }
      else
      {
        leds[XY(x, y)] = ColorFromPalette(currentPalette, 0, 255);
      }
      // show the 3 points, too
      leds[XY(x1, y1)] = CRGB(255, 255, 255);
      leds[XY(x2, y2)] = CRGB(255, 255, 255);
      leds[XY(x3, y3)] = CRGB(255, 255, 255);
    }
  }

  setPixels(leds);

  return FRAMETIME;
} // mode_2Dmetaballs()

//////////////////////
//    2D Noise      //
//////////////////////

uint16_t WS2812FX::mode_2Dnoise(void)
{ // By Andrew Tuline

  uint8_t scale = SEGMENT.intensity + 2;

  for (uint16_t y = 0; y < SEGMENT.height; y++)
  {
    for (uint16_t x = 0; x < SEGMENT.width; x++)
    {
      uint8_t pixelHue8 = inoise8(x * scale, y * scale, millis() / (16 - SEGMENT.speed / 16));
      leds[XY(x, y)] = ColorFromPalette(currentPalette, pixelHue8);
    }
  }

  setPixels(leds);
  return FRAMETIME;
} // mode_2Dnoise()

//////////////////////////////
//     2D Plasma Ball       //
//////////////////////////////

uint16_t WS2812FX::mode_2DPlasmaball(void)
{ // By: Stepko https://editor.soulmatelights.com/gallery/659-plasm-ball , Modified by: Andrew Tuline

  fadeToBlackBy(leds, 64);
  double t = millis() / (33 - SEGMENT.speed / 8);
  for (uint16_t i = 0; i < SEGMENT.width; i++)
  {
    uint16_t thisVal = inoise8(i * 30, t, t);
    uint16_t thisMax = map(thisVal, 0, 255, 0, SEGMENT.width);
    for (uint16_t j = 0; j < SEGMENT.height; j++)
    {
      uint16_t thisVal_ = inoise8(t, j * 30, t);
      uint16_t thisMax_ = map(thisVal_, 0, 255, 0, SEGMENT.height);
      uint16_t x = (i + thisMax_ - (SEGMENT.width * 2 - SEGMENT.width) / 2);
      uint16_t y = (j + thisMax - (SEGMENT.width * 2 - SEGMENT.width) / 2);
      uint16_t cx = (i + thisMax_);
      uint16_t cy = (j + thisMax);

      leds[XY(i, j)] += ((x - y > -2) && (x - y < 2)) ||
                                ((SEGMENT.width - 1 - x - y) > -2 && (SEGMENT.width - 1 - x - y < 2)) ||
                                (SEGMENT.width - cx == 0) ||
                                (SEGMENT.width - 1 - cx == 0) ||
                                ((SEGMENT.height - cy == 0) ||
                                 (SEGMENT.height - 1 - cy == 0))
                            ? ColorFromPalette(currentPalette, beat8(5), thisVal, LINEARBLEND)
                            : CHSV(0, 0, 0);
    }
  }
  blur2d(leds, 4);

  setPixels(leds);
  return FRAMETIME;
} // mode_2DPlasmaball()

////////////////////////////////
//  2D Polar Lights           //
////////////////////////////////

static float fmap(const float x, const float in_min, const float in_max, const float out_min, const float out_max)
{
  return (out_max - out_min) * (x - in_min) / (in_max - in_min) + out_min;
}

uint16_t WS2812FX::mode_2DPolarLights()
{ // By: Kostyantyn Matviyevskyy  https://editor.soulmatelights.com/gallery/762-polar-lights , Modified by: Andrew Tuline

  CRGBPalette16 currentPalette = {0x000000, 0x003300, 0x006600, 0x009900, 0x00cc00, 0x00ff00, 0x33ff00, 0x66ff00, 0x99ff00, 0xccff00, 0xffff00, 0xffcc00, 0xff9900, 0xff6600, 0xff3300, 0xff0000};

  float adjustHeight = fmap(SEGMENT.height, 8, 32, 28, 12);

  uint16_t adjScale = map(SEGMENT.width, 8, 64, 310, 63);

  static unsigned long timer; // Cannot be uint16_t value (aka aux0)

  if (SEGENV.aux1 != SEGMENT.custom1 / 12)
  { // Hacky palette rotation. We need that black.

    SEGENV.aux1 = SEGMENT.custom1;
    for (int i = 0; i < 16; i++)
    {
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

  uint16_t _scale = map(SEGMENT.intensity, 1, 255, 30, adjScale);
  byte _speed = map(SEGMENT.speed, 1, 255, 128, 16);

  for (uint16_t x = 0; x < SEGMENT.width; x++)
  {
    for (uint16_t y = 0; y < SEGMENT.height; y++)
    {
      timer++;
      leds[XY(x, y)] = ColorFromPalette(currentPalette,
                                        qsub8(
                                            inoise8(SEGENV.aux0 % 2 + x * _scale,
                                                    y * 16 + timer % 16,
                                                    timer / _speed),
                                            fabs((float)SEGMENT.height / 2 - (float)y) * adjustHeight));
    }
  }

  setPixels(leds);
  return FRAMETIME;
} // mode_2DPolarLights()

/////////////////////////
//     2D Pulser       //
/////////////////////////

uint16_t WS2812FX::mode_2DPulser()
{ // By: ldirko   https://editor.soulmatelights.com/gallery/878-pulse-test , modifed by: Andrew Tuline

  if (SEGENV.call == 0)
    FastLED.clear();

  static byte r = 16;
  uint16_t a = millis() / (18 - SEGMENT.speed / 16);
  byte x = (a / 14) % SEGMENT.width;
  byte y = (sin8(a * 5) + sin8(a * 4) + sin8(a * 2)) / 3 * r / 255;
  uint16_t index = XY(x, (SEGMENT.height / 2 - r / 2 + y) % SEGMENT.width);
  leds[index] = ColorFromPalette(currentPalette, y * 16 - 100, 255, LINEARBLEND);
  blur2d(leds, SEGMENT.intensity / 16);

  setPixels(leds); // Use this ONLY if we're going to display via leds[x] method.
  return FRAMETIME;
} // mode_2DPulser()

/////////////////////////
//     2D Sindots      //
/////////////////////////

uint16_t WS2812FX::mode_2DSindots()
{ // By: ldirko   https://editor.soulmatelights.com/gallery/597-sin-dots , modified by: Andrew Tuline

  fadeToBlackBy(leds, 15);
  byte t1 = millis() / (257 - SEGMENT.speed); // 20;
  byte t2 = sin8(t1) / 4 * 2;
  for (uint16_t i = 0; i < 13; i++)
  {
    byte x = sin8(t1 + i * SEGMENT.intensity / 8) * (SEGMENT.width - 1) / 255;  //   max index now 255x15/255=15!
    byte y = sin8(t2 + i * SEGMENT.intensity / 8) * (SEGMENT.height - 1) / 255; //  max index now 255x15/255=15!
    leds[XY(x, y)] = ColorFromPalette(currentPalette, i * 255 / 13, 255, LINEARBLEND);
  }
  blur2d(leds, 16);

  setPixels(leds); // Use this ONLY if we're going to display via leds[x] method.
  return FRAMETIME;
} // mode_2DSindots()

//////////////////////////////
//     2D Squared Swirl     //
//////////////////////////////

uint16_t WS2812FX::mode_2Dsquaredswirl(void)
{ // By: Mark Kriegsman. https://gist.github.com/kriegsman/368b316c55221134b160
  // Modifed by: Andrew Tuline
  // custom3 affects the blur amount.

  const uint8_t kBorderWidth = 2;

  fadeToBlackBy(leds, 24);
  // uint8_t blurAmount = dim8_raw( beatsin8(20,64,128) );  //3,64,192
  uint8_t blurAmount = SEGMENT.custom3;
  blur2d(leds, blurAmount);

  // Use two out-of-sync sine waves
  uint8_t i = beatsin8(19, kBorderWidth, SEGMENT.width - kBorderWidth);
  uint8_t j = beatsin8(22, kBorderWidth, SEGMENT.width - kBorderWidth);
  uint8_t k = beatsin8(17, kBorderWidth, SEGMENT.width - kBorderWidth);
  uint8_t m = beatsin8(18, kBorderWidth, SEGMENT.height - kBorderWidth);
  uint8_t n = beatsin8(15, kBorderWidth, SEGMENT.height - kBorderWidth);
  uint8_t p = beatsin8(20, kBorderWidth, SEGMENT.height - kBorderWidth);

  uint16_t ms = millis();

  leds[XY(i, m)] += ColorFromPalette(currentPalette, ms / 29, 255, LINEARBLEND);
  leds[XY(j, n)] += ColorFromPalette(currentPalette, ms / 41, 255, LINEARBLEND);
  leds[XY(k, p)] += ColorFromPalette(currentPalette, ms / 73, 255, LINEARBLEND);

  setPixels(leds);

  return FRAMETIME;
} // mode_2Dsquaredswirl()

//////////////////////////////
//     2D Sun Radiation     //
//////////////////////////////

uint16_t WS2812FX::mode_2DSunradiation(void)
{ // By: ldirko https://editor.soulmatelights.com/gallery/599-sun-radiation  , modified by: Andrew Tuline
  // Does not yet support segments.

  static CRGB chsvLut[256];
  static byte bump[1156]; // Don't go beyond a 32x32 matrix!!!  or (SEGMENT.width+2) * (mtrixHeight+2)

  if (SEGMENT.intensity != SEGENV.aux0)
  {
    SEGENV.aux0 = SEGMENT.intensity;
    for (int j = 0; j < 256; j++)
    {
      chsvLut[j] = HeatColor(j / (3.0 - (float)(SEGMENT.intensity) / 128.)); // 256 pallette color
    }
  }

  int t = millis() / 4;
  int index = 0;
  uint8_t someVal = SEGMENT.speed / 4; // Was 25.
  for (uint16_t j = 0; j < (SEGMENT.height + 2); j++)
  {
    for (uint16_t i = 0; i < (SEGMENT.width + 2); i++)
    {
      byte col = (inoise8_raw(i * someVal, j * someVal, t)) / 2;
      bump[index++] = col;
    }
  }

  int yindex = SEGMENT.width + 3;
  int16_t vly = -(SEGMENT.height / 2 + 1);
  for (uint16_t y = 0; y < SEGMENT.height; y++)
  {
    ++vly;
    int16_t vlx = -(SEGMENT.width / 2 + 1);
    for (uint16_t x = 0; x < SEGMENT.width; x++)
    {
      ++vlx;
      int8_t nx = bump[x + yindex + 1] - bump[x + yindex - 1];
      int8_t ny = bump[x + yindex + (SEGMENT.width + 2)] - bump[x + yindex - (SEGMENT.width + 2)];
      byte difx = abs8(vlx * 7 - nx);
      byte dify = abs8(vly * 7 - ny);
      int temp = difx * difx + dify * dify;
      int col = 255 - temp / 8; // 8 its a size of effect
      if (col < 0)
        col = 0;
      leds[XY(x, y)] = chsvLut[col]; // thx sutubarosu ))
    }
    yindex += (SEGMENT.width + 2);
  }

  setPixels(leds);
  return FRAMETIME;
} // mode_2DSunradiation()

/////////////////////////
//    * 2D Swirl        //
/////////////////////////

uint16_t WS2812FX::mode_2DSwirl(void)
{ // By: Mark Kriegsman https://gist.github.com/kriegsman/5adca44e14ad025e6d3b , modified by Andrew Tuline

  const uint8_t borderWidth = 2;

  blur2d(leds, SEGMENT.custom1);

  uint8_t i = beatsin8(27 * SEGMENT.speed / 255, borderWidth, SEGMENT.height - borderWidth);
  uint8_t j = beatsin8(41 * SEGMENT.speed / 255, borderWidth, SEGMENT.width - borderWidth);
  uint8_t ni = (SEGMENT.width - 1) - i;
  uint8_t nj = (SEGMENT.width - 1) - j;
  uint16_t ms = millis();

  int tmpSound = (soundAgc) ? rawSampleAgc : sampleRaw;

  leds[XY(i, j)] += ColorFromPalette(currentPalette, (ms / 11 + sampleAvg * 4), tmpSound * SEGMENT.intensity / 64, LINEARBLEND);   // CHSV( ms / 11, 200, 255);
  leds[XY(j, i)] += ColorFromPalette(currentPalette, (ms / 13 + sampleAvg * 4), tmpSound * SEGMENT.intensity / 64, LINEARBLEND);   // CHSV( ms / 13, 200, 255);
  leds[XY(ni, nj)] += ColorFromPalette(currentPalette, (ms / 17 + sampleAvg * 4), tmpSound * SEGMENT.intensity / 64, LINEARBLEND); // CHSV( ms / 17, 200, 255);
  leds[XY(nj, ni)] += ColorFromPalette(currentPalette, (ms / 29 + sampleAvg * 4), tmpSound * SEGMENT.intensity / 64, LINEARBLEND); // CHSV( ms / 29, 200, 255);
  leds[XY(i, nj)] += ColorFromPalette(currentPalette, (ms / 37 + sampleAvg * 4), tmpSound * SEGMENT.intensity / 64, LINEARBLEND);  // CHSV( ms / 37, 200, 255);
  leds[XY(ni, j)] += ColorFromPalette(currentPalette, (ms / 41 + sampleAvg * 4), tmpSound * SEGMENT.intensity / 64, LINEARBLEND);  // CHSV( ms / 41, 200, 255);

  setPixels(leds);
  return FRAMETIME;
} // mode_2DSwirl()

/////////////////////////
//     2D Tartan       //
/////////////////////////

uint16_t WS2812FX::mode_2Dtartan()
{ // By: Elliott Kember  https://editor.soulmatelights.com/gallery/3-tartan , Modified by: Andrew Tuline

  uint8_t hue;
  int offsetX = beatsin16(3, -360, 360);
  int offsetY = beatsin16(2, -360, 360);

  for (uint16_t x = 0; x < SEGMENT.width; x++)
  {
    for (uint16_t y = 0; y < SEGMENT.height; y++)
    {
      uint16_t index = XY(x, y);
      hue = x * beatsin16(10, 1, 10) + offsetY;
      leds[index] = ColorFromPalette(currentPalette, hue, sin8(x * SEGMENT.speed + offsetX) * sin8(x * SEGMENT.speed + offsetX) / 255, LINEARBLEND);
      hue = y * 3 + offsetX;
      leds[index] += ColorFromPalette(currentPalette, hue, sin8(y * SEGMENT.intensity + offsetY) * sin8(y * SEGMENT.intensity + offsetY) / 255, LINEARBLEND);
    }
  }

  setPixels(leds); // Use this ONLY if we're going to display via leds[x] method.
  return FRAMETIME;
} // mode_2DTartan()

/////////////////////////
//    * 2D Waverly     //
/////////////////////////

uint16_t WS2812FX::mode_2DWaverly(void)
{ // By: Stepko, https://editor.soulmatelights.com/gallery/652-wave , modified by Andrew Tuline

  fadeToBlackBy(leds, SEGMENT.speed);

  long t = millis() / 2;
  for (uint16_t i = 0; i < SEGMENT.width; i++)
  {
    //  byte thisVal = inoise8(i * 45 , t , t);
    // byte thisMax = map(thisVal, 0, 255, 0, SEGMENT.height);

    int tmpSound = (soundAgc) ? sampleAgc : sampleAvg;

    uint16_t thisVal = tmpSound * SEGMENT.intensity / 64 * inoise8(i * 45, t, t) / 64;
    uint16_t thisMax = map(thisVal, 0, 512, 0, SEGMENT.height);

    for (uint16_t j = 0; j < thisMax; j++)
    {
      leds[XY(i, j)] += ColorFromPalette(currentPalette, map(j, 0, thisMax, 250, 0), 255, LINEARBLEND);
      leds[XY((SEGMENT.width - 1) - i, (SEGMENT.height - 1) - j)] += ColorFromPalette(currentPalette, map(j, 0, thisMax, 250, 0), 255, LINEARBLEND);
    }
  }
  blur2d(leds, 16);

  setPixels(leds);
  return FRAMETIME;
} // mode_2DWaverly()

////////////////////////////////
//   Begin volume routines    //
////////////////////////////////

///////////////////////
//   * GRAVCENTER    //
///////////////////////

typedef struct Gravity
{
  int topLED;
  int gravityCounter;
} gravity;

uint16_t WS2812FX::mode_gravcenter(void)
{ // Gravcenter. By Andrew Tuline.

  uint16_t dataSize = sizeof(gravity);
  if (!SEGENV.allocateData(dataSize))
    return mode_static(); // allocation failed
  Gravity *gravcen = reinterpret_cast<Gravity *>(SEGENV.data);

  fade_out(240);

  float tmpSound = (soundAgc) ? sampleAgc : sampleAvg;
  float segmentSampleAvg = tmpSound * (float)SEGMENT.intensity / 255.0;
  segmentSampleAvg *= 0.125; // divide by 8, to compensate for later "sensitivty" upscaling

  float mySampleAvg = mapf(segmentSampleAvg * 2.0, 0, 32, 0, (float)SEGLEN / 2.0); // map to pixels availeable in current segment
  int tempsamp = constrain(mySampleAvg, 0, SEGLEN / 2);                            // Keep the sample from overflowing.
  uint8_t gravity = 8 - SEGMENT.speed / 32;

  for (int i = 0; i < tempsamp; i++)
  {
    uint8_t index = inoise8(i * segmentSampleAvg + millis(), 5000 + i * segmentSampleAvg);
    setPixelColor(i + SEGLEN / 2, color_blend(SEGCOLOR(1), color_from_palette(index, false, PALETTE_SOLID_WRAP, 0), segmentSampleAvg * 8));
    setPixelColor(SEGLEN / 2 - i - 1, color_blend(SEGCOLOR(1), color_from_palette(index, false, PALETTE_SOLID_WRAP, 0), segmentSampleAvg * 8));
  }

  if (tempsamp >= gravcen->topLED)
    gravcen->topLED = tempsamp - 1;
  else if (gravcen->gravityCounter % gravity == 0)
    gravcen->topLED--;

  if (gravcen->topLED >= 0)
  {
    setPixelColor(gravcen->topLED + SEGLEN / 2, color_from_palette(millis(), false, PALETTE_SOLID_WRAP, 0));
    setPixelColor(SEGLEN / 2 - 1 - gravcen->topLED, color_from_palette(millis(), false, PALETTE_SOLID_WRAP, 0));
  }
  gravcen->gravityCounter = (gravcen->gravityCounter + 1) % gravity;

  return FRAMETIME;
} // mode_gravcenter()

///////////////////////
//   * GRAVCENTRIC   //
///////////////////////

uint16_t WS2812FX::mode_gravcentric(void)
{ // Gravcentric. By Andrew Tuline.

  uint16_t dataSize = sizeof(gravity);
  if (!SEGENV.allocateData(dataSize))
    return mode_static(); // allocation failed
  Gravity *gravcen = reinterpret_cast<Gravity *>(SEGENV.data);

  fade_out(240);
  fade_out(240);

  float tmpSound = (soundAgc) ? sampleAgc : sampleAvg;
  float segmentSampleAvg = tmpSound * (float)SEGMENT.intensity / 255.0;
  segmentSampleAvg *= 0.125; // divide by 8, to compensate for later "sensitivty" upscaling

  float mySampleAvg = mapf(segmentSampleAvg * 2.0, 0, 32, 0, (float)SEGLEN / 2.0); // map to pixels availeable in current segment
  int tempsamp = constrain(mySampleAvg, 0, SEGLEN / 2);                            // Keep the sample from overflowing.
  uint8_t gravity = 8 - SEGMENT.speed / 32;

  for (int i = 0; i < tempsamp; i++)
  {
    uint8_t index = segmentSampleAvg * 24 + millis() / 200;
    setPixelColor(i + SEGLEN / 2, color_from_palette(index, false, PALETTE_SOLID_WRAP, 0));
    setPixelColor(SEGLEN / 2 - 1 - i, color_from_palette(index, false, PALETTE_SOLID_WRAP, 0));
  }

  if (tempsamp >= gravcen->topLED)
    gravcen->topLED = tempsamp - 1;
  else if (gravcen->gravityCounter % gravity == 0)
    gravcen->topLED--;

  if (gravcen->topLED >= 0)
  {
    setPixelColor(gravcen->topLED + SEGLEN / 2, CRGB::Gray);
    setPixelColor(SEGLEN / 2 - 1 - gravcen->topLED, CRGB::Gray);
  }
  gravcen->gravityCounter = (gravcen->gravityCounter + 1) % gravity;

  return FRAMETIME;
} // mode_gravcentric()

///////////////////////
//   * GRAVIMETER    //
///////////////////////
#ifndef SR_DEBUG_AGC
uint16_t WS2812FX::mode_gravimeter(void)
{ // Gravmeter. By Andrew Tuline.

  uint16_t dataSize = sizeof(gravity);
  if (!SEGENV.allocateData(dataSize))
    return mode_static(); // allocation failed
  Gravity *gravcen = reinterpret_cast<Gravity *>(SEGENV.data);

  fade_out(240);

  float tmpSound = (soundAgc) ? sampleAgc : sampleAvg;
  float segmentSampleAvg = tmpSound * (float)SEGMENT.intensity / 255.0;
  segmentSampleAvg *= 0.25; // divide by 4, to compensate for later "sensitivty" upscaling

  float mySampleAvg = mapf(segmentSampleAvg * 2.0, 0, 64, 0, (SEGLEN - 1)); // map to pixels availeable in current segment
  int tempsamp = constrain(mySampleAvg, 0, SEGLEN - 1);                     // Keep the sample from overflowing.
  uint8_t gravity = 8 - SEGMENT.speed / 32;

  for (int i = 0; i < tempsamp; i++)
  {
    uint8_t index = inoise8(i * segmentSampleAvg + millis(), 5000 + i * segmentSampleAvg);
    setPixelColor(i, color_blend(SEGCOLOR(1), color_from_palette(index, false, PALETTE_SOLID_WRAP, 0), segmentSampleAvg * 8));
  }

  if (tempsamp >= gravcen->topLED)
    gravcen->topLED = tempsamp;
  else if (gravcen->gravityCounter % gravity == 0)
    gravcen->topLED--;

  if (gravcen->topLED > 0)
  {
    setPixelColor(gravcen->topLED, color_from_palette(millis(), false, PALETTE_SOLID_WRAP, 0));
  }
  gravcen->gravityCounter = (gravcen->gravityCounter + 1) % gravity;

  return FRAMETIME;
} // mode_gravimeter()

#else
// This an abuse of the gravimeter effect for AGC debugging
// instead of sound volume, it uses the AGC gain multiplier as input
uint16_t WS2812FX::mode_gravimeter(void)
{ // Gravmeter. By Andrew Tuline.

  uint16_t dataSize = sizeof(gravity);
  if (!SEGENV.allocateData(dataSize))
    return mode_static(); // allocation failed
  Gravity *gravcen = reinterpret_cast<Gravity *>(SEGENV.data);

  fade_out(240);

  float tmpSound = multAgc; // AGC gain
  if (soundAgc == 0)
  {
    if ((sampleAvg > 1.0) && (sampleReal > 0.05))
      tmpSound = (float)sampleRaw / sampleReal; // current non-AGC gain
    else
      tmpSound = ((float)sampleGain / 40.0 * (float)inputLevel / 128.0) + 1.0 / 16.0; // non-AGC gain from presets
  }

  if (tmpSound > 2)
    tmpSound = ((tmpSound - 2.0) / 2) + 2; // compress ranges > 2
  if (tmpSound > 1)
    tmpSound = ((tmpSound - 1.0) / 2) + 1; // compress ranges > 1

  float segmentSampleAvg = 64.0 * tmpSound * (float)SEGMENT.intensity / 128.0;
  float mySampleAvg = mapf(segmentSampleAvg, 0, 128, 0, (SEGLEN - 1)); // map to pixels availeable in current segment
  int tempsamp = constrain(mySampleAvg, 0, SEGLEN - 1);                // Keep the sample from overflowing.

  // tempsamp = SEGLEN - tempsamp;                                      // uncomment to invert direction
  segmentSampleAvg = fmax(64.0 - fmin(segmentSampleAvg, 63), 8); // inverted brightness

  uint8_t gravity = 8 - SEGMENT.speed / 32;

  if (sampleAvg > 1) // disable bar "body" if below squelch
  {
    for (int i = 0; i < tempsamp; i++)
    {
      uint8_t index = inoise8(i * segmentSampleAvg + millis(), 5000 + i * segmentSampleAvg);
      setPixelColor(i, color_blend(SEGCOLOR(1), color_from_palette(index, false, PALETTE_SOLID_WRAP, 0), segmentSampleAvg * 4.0));
    }
  }
  if (tempsamp >= gravcen->topLED)
    gravcen->topLED = tempsamp;
  else if (gravcen->gravityCounter % gravity == 0)
    gravcen->topLED--;

  if (gravcen->topLED > 0)
  {
    setPixelColor(gravcen->topLED, color_from_palette(millis(), false, PALETTE_SOLID_WRAP, 0));
  }
  gravcen->gravityCounter = (gravcen->gravityCounter + 1) % gravity;

  return FRAMETIME;
} // mode_gravimeter()
#endif

//////////////////////
//       JBL        //
//////////////////////


uint16_t pos = 0;
uint8_t hue = 0;
int hueDelay = 0;

uint8_t red(uint32_t c)
{
  return (c >> 16);
}
uint8_t green(uint32_t c)
{
  return (c >> 8);
}
uint8_t blue(uint32_t c)
{
  return (c);
}

uint16_t WS2812FX::mode_jbl()
{
  hueDelay++;

  if (hueDelay > SEGMENT.intensity)
  {
    hueDelay = 0;
    hue++;
  }

  if (hue > 254)
  {
    hue = 0;
  } 

  float speed = 0;

  uint16_t counter = 0;

  if (sampleAgc > (255 - SEGMENT.speed))
  {
    speed = 1000;
  }
  else
  {
    speed = 20;
  };

  pos += speed;

  counter = pos;
  /* counter = (now * ((speed >> 3 ) + 1 )) & 0xFFFF;
  counter = counter >> 8; */

  // counter += speed * now;
  counter = counter >> 8;

  for (uint16_t i = 0; i < SEGLEN; i++)
  {
    uint8_t colorIndex = (i * 255 / SEGLEN) - counter;

    uint32_t paletteColor = color_from_palette(colorIndex, false, true, 255);

    uint8_t r = red(paletteColor);

    CRGB rgb(CHSV(hue, 255, r));

    setPixelColor(i, rgb.r, rgb.g, rgb.b);
  };

  // setPixelColor(0, rgb.r, rgb.g, rgb.b);

  return FRAMETIME;
} // mode_jbl()

//////////////////////
//   * JUGGLES      //
//////////////////////

uint16_t WS2812FX::mode_juggles(void)
{ // Juggles. By Andrew Tuline.

  fade_out(224);
  int my_sampleAgc = fmax(fmin(sampleAgc, 255.0), 0);

  for (int i = 0; i < SEGMENT.intensity / 32 + 1; i++)
  {
    setPixelColor(beatsin16(SEGMENT.speed / 4 + i * 2, 0, SEGLEN - 1), color_blend(SEGCOLOR(1), color_from_palette(millis() / 4 + i * 2, false, PALETTE_SOLID_WRAP, 0), my_sampleAgc));
  }

  return FRAMETIME;
} // mode_juggles()

//////////////////////
//   * MATRIPIX     //
//////////////////////

uint16_t WS2812FX::mode_matripix(void)
{ // Matripix. By Andrew Tuline.
  if (SEGENV.call == 0)
    fill_solid(leds, 0);

  uint8_t secondHand = micros() / (256 - SEGMENT.speed) / 500 % 16;
  if (SEGENV.aux0 != secondHand)
  {
    SEGENV.aux0 = secondHand;
    uint8_t tmpSound = (soundAgc) ? rawSampleAgc : sampleRaw;
    int pixBri = tmpSound * SEGMENT.intensity / 64;
    leds[segmentToLogical(SEGLEN - 1)] = color_blend(SEGCOLOR(1), color_from_palette(millis(), false, PALETTE_SOLID_WRAP, 0), pixBri);
    for (int i = 0; i < SEGLEN - 1; i++)
      leds[segmentToLogical(i)] = leds[segmentToLogical(i + 1)];
  }

  setPixels(leds);
  return FRAMETIME;
} // mode_matripix()

//////////////////////
//   * MIDNOISE     //
//////////////////////

uint16_t WS2812FX::mode_midnoise(void)
{ // Midnoise. By Andrew Tuline.

  // Changing xdist to SEGENV.aux0 and ydist to SEGENV.aux1.

  fade_out(SEGMENT.speed);
  fade_out(SEGMENT.speed);

  float tmpSound = (soundAgc) ? sampleAgc : sampleAvg;
  float tmpSound2 = tmpSound * (float)SEGMENT.intensity / 256.0; // Too sensitive.
  tmpSound2 *= (float)SEGMENT.intensity / 128.0;                 // Reduce sensitity/length.

  int maxLen = mapf(tmpSound2, 0, 127, 0, SEGLEN / 2);
  if (maxLen > SEGLEN / 2)
    maxLen = SEGLEN / 2;

  for (int i = (SEGLEN / 2 - maxLen); i < (SEGLEN / 2 + maxLen); i++)
  {
    uint8_t index = inoise8(i * tmpSound + SEGENV.aux0, SEGENV.aux1 + i * tmpSound); // Get a value from the noise function. I'm using both x and y axis.
    setPixelColor(i, color_from_palette(index, false, PALETTE_SOLID_WRAP, 0));
  }

  SEGENV.aux0 = SEGENV.aux0 + beatsin8(5, 0, 10);
  SEGENV.aux1 = SEGENV.aux1 + beatsin8(4, 0, 10);

  return FRAMETIME;
} // mode_midnoise()

//////////////////////
//   * NOISEFIRE    //
//////////////////////

// I am the god of hellfire. . . Volume (only) reactive fire routine. Oh, look how short this is.
uint16_t WS2812FX::mode_noisefire(void)
{ // Noisefire. By Andrew Tuline.

  currentPalette = CRGBPalette16(CHSV(0, 255, 2), CHSV(0, 255, 4), CHSV(0, 255, 8), CHSV(0, 255, 8), // Fire palette definition. Lower value = darker.
                                 CHSV(0, 255, 16), CRGB::Red, CRGB::Red, CRGB::Red,
                                 CRGB::DarkOrange, CRGB::DarkOrange, CRGB::Orange, CRGB::Orange,
                                 CRGB::Yellow, CRGB::Orange, CRGB::Yellow, CRGB::Yellow);

  for (int i = 0; i < SEGLEN; i++)
  {
    uint16_t index = inoise8(i * SEGMENT.speed / 64, millis() * SEGMENT.speed / 64 * SEGLEN / 255); // X location is constant, but we move along the Y at the rate of millis(). By Andrew Tuline.
    index = (255 - i * 256 / SEGLEN) * index / (256 - SEGMENT.intensity);                           // Now we need to scale index so that it gets blacker as we get close to one of the ends.
                                                                                                    // This is a simple y=mx+b equation that's been scaled. index/128 is another scaling.

    uint8_t tmpSound = (soundAgc) ? sampleAgc : sampleAvg;

    CRGB color = ColorFromPalette(currentPalette, index, tmpSound * 2, LINEARBLEND); // Use the my own palette.
    leds[segmentToLogical(i)] = color;
  }

  setPixels(leds);
  return FRAMETIME;
} // mode_noisefire()

///////////////////////
//   * Noisemeter    //
///////////////////////

uint16_t WS2812FX::mode_noisemeter(void)
{ // Noisemeter. By Andrew Tuline.

  uint8_t fadeRate = map(SEGMENT.speed, 0, 255, 224, 255);
  fade_out(fadeRate);

  float tmpSound = (soundAgc) ? rawSampleAgc : sampleRaw;
  float tmpSound2 = tmpSound * 2.0 * (float)SEGMENT.intensity / 255.0;
  int maxLen = mapf(tmpSound2, 0, 255, 0, SEGLEN); // map to pixels availeable in current segment              // Still a bit too sensitive.
  if (maxLen > SEGLEN)
    maxLen = SEGLEN;

  tmpSound = soundAgc ? sampleAgc : sampleAvg; // now use smoothed value (sampleAvg or sampleAgc)
  for (int i = 0; i < maxLen; i++)
  {                                                                                  // The louder the sound, the wider the soundbar. By Andrew Tuline.
    uint8_t index = inoise8(i * tmpSound + SEGENV.aux0, SEGENV.aux1 + i * tmpSound); // Get a value from the noise function. I'm using both x and y axis.
    setPixelColor(i, color_from_palette(index, false, PALETTE_SOLID_WRAP, 0));
  }

  SEGENV.aux0 += beatsin8(5, 0, 10);
  SEGENV.aux1 += beatsin8(4, 0, 10);

  return FRAMETIME;
} // mode_noisemeter()

//////////////////////
//     * PIXELS     //
//////////////////////

uint16_t WS2812FX::mode_pixels(void)
{ // Pixels. By Andrew Tuline.

  fade_out(SEGMENT.speed);

  for (int i = 0; i < SEGMENT.intensity / 16; i++)
  {
    uint16_t segLoc = random(SEGLEN); // 16 bit for larger strands of LED's.
    setPixelColor(segLoc, color_blend(SEGCOLOR(1), color_from_palette(myVals[i % 32] + i * 4, false, PALETTE_SOLID_WRAP, 0), sampleAgc));
  }

  return FRAMETIME;
} // mode_pixels()

//////////////////////
//   * PIXELWAVE    //
//////////////////////

uint16_t WS2812FX::mode_pixelwave(void)
{ // Pixelwave. By Andrew Tuline.

  if (SEGENV.call == 0)
    fill_solid(leds, 0);
  uint8_t secondHand = micros() / (256 - SEGMENT.speed) / 500 + 1 % 16;

  if (SEGENV.aux0 != secondHand)
  {
    SEGENV.aux0 = secondHand;

    uint8_t tmpSound = (soundAgc) ? rawSampleAgc : sampleRaw;
    int pixBri = tmpSound * SEGMENT.intensity / 64;
    leds[segmentToLogical(SEGLEN / 2)] = color_blend(SEGCOLOR(1), color_from_palette(millis(), false, PALETTE_SOLID_WRAP, 0), pixBri);

    for (int i = SEGLEN - 1; i > SEGLEN / 2; i--)
    { // Move to the right.
      leds[segmentToLogical(i)] = leds[segmentToLogical(i - 1)];
    }
    for (int i = 0; i < SEGLEN / 2; i++)
    { // Move to the left.
      leds[segmentToLogical(i)] = leds[segmentToLogical(i + 1)];
    }
  }

  setPixels(leds);
  return FRAMETIME;
} // mode_pixelwave()

//////////////////////
//   * PLASMOID     //
//////////////////////

typedef struct Plasphase
{
  int16_t thisphase;
  int16_t thatphase;
} plasphase;

uint16_t WS2812FX::mode_plasmoid(void)
{ // Plasmoid. By Andrew Tuline.

  uint16_t dataSize = sizeof(plasphase);
  if (!SEGENV.allocateData(dataSize))
    return mode_static(); // allocation failed
  Plasphase *plasmoip = reinterpret_cast<Plasphase *>(SEGENV.data);

  fadeToBlackBy(leds, 64);

  plasmoip->thisphase += beatsin8(6, -4, 4); // You can change direction and speed individually.
  plasmoip->thatphase += beatsin8(7, -4, 4); // Two phase values to make a complex pattern. By Andrew Tuline.

  for (int i = 0; i < SEGLEN; i++)
  { // For each of the LED's in the strand, set a brightness based on a wave as follows.
    // updated, similar to "plasma" effect - softhack007
    uint8_t thisbright = cubicwave8(((i * (1 + (3 * SEGMENT.speed / 32))) + plasmoip->thisphase) & 0xFF) / 2;
    thisbright += cos8(((i * (97 + (5 * SEGMENT.speed / 32))) + plasmoip->thatphase) & 0xFF) / 2; // Let's munge the brightness a bit and animate it all with the phases.

    uint8_t colorIndex = thisbright;
    int tmpSound = (soundAgc) ? sampleAgc : sampleAvg;
    if (tmpSound * SEGMENT.intensity / 64 < thisbright)
    {
      thisbright = 0;
    }

    leds[segmentToLogical(i)] += color_blend(SEGCOLOR(1), color_from_palette(colorIndex, false, PALETTE_SOLID_WRAP, 0), thisbright);
  }

  setPixels(leds);

  return FRAMETIME;
} // mode_plasmoid()

///////////////////////
//   * PUDDLEPEAK    //
///////////////////////

// Andrew's crappy peak detector. If I were 40+ years younger, I'd learn signal processing.
uint16_t WS2812FX::mode_puddlepeak(void)
{ // Puddlepeak. By Andrew Tuline.

  uint16_t size = 0;
  uint8_t fadeVal = map(SEGMENT.speed, 0, 255, 224, 255);
  uint16_t pos = random(SEGLEN); // Set a random starting position.

  binNum = SEGMENT.custom2;     // Select a bin.
  maxVol = SEGMENT.custom3 / 4; // Our volume comparator.

  fade_out(fadeVal);

  if (samplePeak == 1)
  {
    size = sampleAgc * SEGMENT.intensity / 256 / 4 + 1; // Determine size of the flash based on the volume.
    if (pos + size >= SEGLEN)
      size = SEGLEN - pos;
  }

  for (int i = 0; i < size; i++)
  { // Flash the LED's.
    setPixelColor(pos + i, color_from_palette(millis(), false, PALETTE_SOLID_WRAP, 0));
  }

  return FRAMETIME;
} // mode_puddlepeak()

//////////////////////
//   * PUDDLES      //
//////////////////////

uint16_t WS2812FX::mode_puddles(void)
{ // Puddles. By Andrew Tuline.

  uint16_t size = 0;
  uint8_t fadeVal = map(SEGMENT.speed, 0, 255, 224, 255);
  uint16_t pos = random(SEGLEN); // Set a random starting position.

  fade_out(fadeVal);

  float tmpSound = (soundAgc) ? rawSampleAgc : sampleRaw;

  if (tmpSound > 1)
  {
    size = tmpSound * SEGMENT.intensity / 256 / 8 + 1; // Determine size of the flash based on the volume.
    if (pos + size >= SEGLEN)
      size = SEGLEN - pos;
  }

  for (int i = 0; i < size; i++)
  { // Flash the LED's.
    setPixelColor(pos + i, color_from_palette(millis(), false, PALETTE_SOLID_WRAP, 0));
  }

  return FRAMETIME;
} // mode_puddles()

/////////////////////////////////
//     * Ripple Peak           //
/////////////////////////////////

uint16_t WS2812FX::mode_ripplepeak(void)
{ // * Ripple peak. By Andrew Tuline.

  // This currently has no controls.
#define maxsteps 16 // Case statement wouldn't allow a variable.

  uint16_t maxRipples = 16;
  uint16_t dataSize = sizeof(ripple) * maxRipples;

  if (!SEGENV.allocateData(dataSize))
    return mode_static(); // allocation failed

  Ripple *ripples = reinterpret_cast<Ripple *>(SEGENV.data);

  //  static uint8_t colour;                                  // Ripple colour is randomized.
  //  static uint16_t centre;                                 // Center of the current ripple.
  //  static int8_t steps = -1;                               // -1 is the initializing step.

  //  static uint8_t ripFade = 255;                           // Starting brightness, which we'll say is SEGENV.aux0.
  if (SEGENV.call == 0)
    SEGENV.aux0 = 255;

  binNum = SEGMENT.custom2;     // Select a bin.
  maxVol = SEGMENT.custom3 / 2; // Our volume comparator.

  fade_out(240); // Lower frame rate means less effective fading than FastLED
  fade_out(240);

  for (uint16_t i = 0; i < SEGMENT.intensity / 16; i++)
  { // Limit the number of ripples.

    if (samplePeak)
    {

      ripples[i].state = -1;
    }

    switch (ripples[i].state)
    {

    case -2: // Inactive mode
      break;

    case -1: // Initialize ripple variables.
      ripples[i].pos = random16(SEGLEN);

#ifdef ESP32
      ripples[i].color = (int)(log10f(FFT_MajorPeak) * 128);
#else
      ripples[i].color = random8();
#endif

      ripples[i].state = 0;
      break;

    case 0:
      setPixelColor(ripples[i].pos, color_blend(SEGCOLOR(1), color_from_palette(ripples[i].color, false, PALETTE_SOLID_WRAP, 0), SEGENV.aux0));
      ripples[i].state++;
      break;

    case maxsteps: // At the end of the ripples. -2 is an inactive mode.
      ripples[i].state = -2;
      break;

    default: // Middle of the ripples.

      setPixelColor((ripples[i].pos + ripples[i].state + SEGLEN) % SEGLEN, color_blend(SEGCOLOR(1), color_from_palette(ripples[i].color, false, PALETTE_SOLID_WRAP, 0), SEGENV.aux0 / ripples[i].state * 2));
      setPixelColor((ripples[i].pos - ripples[i].state + SEGLEN) % SEGLEN, color_blend(SEGCOLOR(1), color_from_palette(ripples[i].color, false, PALETTE_SOLID_WRAP, 0), SEGENV.aux0 / ripples[i].state * 2));
      ripples[i].state++; // Next step.
      break;
    } // switch step
  }   // for i

  return FRAMETIME;
} // mode_ripplepeak()

///////////////////////////////
//     BEGIN FFT ROUTINES    //
///////////////////////////////

double mapf(double x, double in_min, double in_max, double out_min, double out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

////////////////////
//    ** Binmap   //
////////////////////

uint16_t WS2812FX::mode_binmap(void)
{ // Binmap. Scale raw fftBin[] values to SEGLEN. Shows just how noisy those bins are.

#define FIRSTBIN 3  // The first 3 bins are garbage.
#define LASTBIN 255 // Don't use the highest bins, as they're (almost) a mirror of the first 256.

  float maxVal = 512; // Kind of a guess as to the maximum output value per combined logarithmic bins.

  float binScale = (((float)sampleGain / 40.0) + 1.0 / 16) * ((float)inputLevel / 128.0); // non-AGC gain multiplier
  if (soundAgc)
    binScale = multAgc; // AGC gain
  if (sampleAvg < 1)
    binScale = 0.001; // silentium!

#if 0
  //The next lines are good for debugging, however too much flickering for non-developers ;-)
  float my_magnitude = FFT_Magnitude / 16.0;    // scale magnitude to be aligned with scaling of FFT bins
  my_magnitude *= binScale;                     // apply gain
  maxVal = fmax(64, my_magnitude);              // set maxVal = max FFT result
#endif

  for (int i = 0; i < SEGLEN; i++)
  {

    uint16_t startBin = FIRSTBIN + i * (LASTBIN - FIRSTBIN) / SEGLEN;     // This is the START bin for this particular pixel.
    uint16_t endBin = FIRSTBIN + (i + 1) * (LASTBIN - FIRSTBIN) / SEGLEN; // This is the END bin for this particular pixel.
    if (endBin > startBin)
      endBin--; // avoid overlapping

    double sumBin = 0;

    for (int j = startBin; j <= endBin; j++)
    {
      sumBin += (fftBin[j] < soundSquelch * 1.75) ? 0 : fftBin[j]; // We need some sound temporary squelch for fftBin, because we didn't do it for the raw bins in audio_reactive.h
    }

    sumBin = sumBin / (endBin - startBin + 1);           // Normalize it.
    sumBin = sumBin * (i + 5) / (endBin - startBin + 5); // Disgusting frequency adjustment calculation. Lows were too bright. Am open to quick 'n dirty alternatives.

    sumBin = sumBin * 8; // Need to use the 'log' version for this. Why " * 8" ??
    sumBin *= binScale;  // apply gain

    if (sumBin > maxVal)
      sumBin = maxVal; // Make sure our bin isn't higher than the max . . which we capped earlier.

    uint8_t bright = constrain(mapf(sumBin, 0, maxVal, 0, 255), 0, 255); // Map the brightness in relation to maxVal and crunch to 8 bits.

    setPixelColor(i, color_blend(SEGCOLOR(1), color_from_palette(i * 8 + millis() / 50, false, PALETTE_SOLID_WRAP, 0), bright)); // 'i' is just an index in the palette. The FFT value, bright, is the intensity.

  } // for i

  return FRAMETIME;
} // mode_binmap()

//////////////////////
//    ** Blurz       //
//////////////////////

uint16_t WS2812FX::mode_blurz(void)
{ // Blurz. By Andrew Tuline.

  if (SEGENV.call == 0)
  {
    fill_solid(leds, 0);
    SEGENV.aux0 = 0;
  }

  uint8_t blurAmt = SEGMENT.intensity;

  fade_out(SEGMENT.speed);

  uint16_t segLoc = random(SEGLEN);
  leds[segmentToLogical(segLoc)] = color_blend(SEGCOLOR(1), color_from_palette(2 * fftResult[SEGENV.aux0 % 16] * 240 / (SEGLEN - 1), false, PALETTE_SOLID_WRAP, 0), 2 * fftResult[SEGENV.aux0 % 16]);
  SEGENV.aux0++;
  SEGENV.aux0 = SEGENV.aux0 % 16;

  blur1d(leds, blurAmt);

  setPixels(leds);
  return FRAMETIME;
} // mode_blurz()

/////////////////////////
//   ** DJLight        //
/////////////////////////

uint16_t WS2812FX::mode_DJLight(void)
{                        // Written by ??? Adapted by Will Tatam.
  int NUM_LEDS = SEGLEN; // aka SEGLEN
  int mid = NUM_LEDS / 2;

  uint8_t secondHand = micros() / (256 - SEGMENT.speed) / 500 + 1 % 64;

  if (SEGENV.aux0 != secondHand)
  { // Triggered millis timing.
    SEGENV.aux0 = secondHand;

    leds[segmentToLogical(mid)] = CRGB(fftResult[15] / 2, fftResult[5] / 2, fftResult[0] / 2); // 16-> 15 as 16 is out of bounds
    leds[segmentToLogical(mid)].fadeToBlackBy(map(fftResult[1 * 4], 0, 255, 255, 10));         // TODO - Update

    // move to the left
    for (int i = NUM_LEDS - 1; i > mid; i--)
    {
      leds[segmentToLogical(i)] = leds[segmentToLogical(i - 1)];
    }
    // move to the right
    for (int i = 0; i < mid; i++)
    {
      leds[segmentToLogical(i)] = leds[segmentToLogical(i + 1)];
    }
  }

  setPixels(leds);
  return FRAMETIME;
} // mode_DJLight()

////////////////////
//   ** Freqmap   //
////////////////////

uint16_t WS2812FX::mode_freqmap(void)
{ // Map FFT_MajorPeak to SEGLEN. Would be better if a higher framerate.
  // Start frequency = 60 Hz and log10(60) = 1.78
  // End frequency = 5120 Hz and lo10(5120) = 3.71

  float my_magnitude = FFT_Magnitude / 4.0;
  if (soundAgc)
    my_magnitude *= multAgc;
  if (sampleAvg < 1)
    my_magnitude = 0.001; // noise gate closed - mute

  fade_out(SEGMENT.speed);

  int locn = (log10f(FFT_MajorPeak) - 1.78) * (float)SEGLEN / (3.71 - 1.78); // log10 frequency range is from 1.78 to 3.71. Let's scale to SEGLEN.

  if (locn >= SEGLEN)
    locn = SEGLEN - 1;
  if (locn < 1)
    locn = 0;
  uint16_t pixCol = (log10f(FFT_MajorPeak) - 1.78) * 255.0 / (3.71 - 1.78); // Scale log10 of frequency values to the 255 colour index.
  uint16_t bright = (int)my_magnitude;

  setPixelColor(locn, color_blend(SEGCOLOR(1), color_from_palette(SEGMENT.intensity + pixCol, false, PALETTE_SOLID_WRAP, 0), bright));

  return FRAMETIME;
} // mode_freqmap()

///////////////////////
//   ** Freqmatrix   //
///////////////////////

uint16_t WS2812FX::mode_freqmatrix(void)
{ // Freqmatrix. By Andreas Pleschung.

  uint8_t secondHand = micros() / (256 - SEGMENT.speed) / 500 % 16;

  if (SEGENV.aux0 != secondHand)
  {
    SEGENV.aux0 = secondHand;

    double sensitivity = mapf(SEGMENT.custom3, 1, 255, 1, 10);
    int pixVal = sampleAgc * SEGMENT.intensity / 256 * sensitivity;
    if (pixVal > 255)
      pixVal = 255;

    double intensity = map(pixVal, 0, 255, 0, 100) / 100.0; // make a brightness from the last avg

    CRGB color = 0;
    CHSV c;

    if (FFT_MajorPeak > 5120)
      FFT_MajorPeak = 1.0f;
    // MajorPeak holds the freq. value which is most abundant in the last sample.
    // With our sampling rate of 10240Hz we have a usable freq range from roughtly 80Hz to 10240/2 Hz
    // we will treat everything with less than 65Hz as 0
    // Serial.printf("%5d ", FFT_MajorPeak, 0);
    if (FFT_MajorPeak < 80)
    {
      color = CRGB::Black;
    }
    else
    {
      int upperLimit = 20 * SEGMENT.custom2;
      int lowerLimit = 2 * SEGMENT.custom1;
      int i = lowerLimit != upperLimit ? map(FFT_MajorPeak, lowerLimit, upperLimit, 0, 255) : FFT_MajorPeak;
      uint16_t b = 255 * intensity;
      if (b > 255)
        b = 255;
      c = CHSV(i, 240, (uint8_t)b);
      color = c; // implicit conversion to RGB supplied by FastLED
    }

    // Serial.println(color);
    leds[segmentToLogical(0)] = color;

    // shift the pixels one pixel up
    for (int i = SEGLEN; i > 0; i--)
    { // Move up
      leds[segmentToLogical(i)] = leds[segmentToLogical(i - 1)];
    }

    // fadeval = fade;

    // DISPLAY ARRAY
    setPixels(leds);
  }

  return FRAMETIME;
} // mode_freqmatrix()

//////////////////////
//   ** Freqpixels  //
//////////////////////

// Start frequency = 60 Hz and log10(60) = 1.78
// End frequency = 5120 Hz and lo10(5120) = 3.71

//  SEGMENT.speed select faderate
//  SEGMENT.intensity select colour index

uint16_t WS2812FX::mode_freqpixels(void)
{ // Freqpixel. By Andrew Tuline.

  uint16_t fadeRate = 2 * SEGMENT.speed - SEGMENT.speed * SEGMENT.speed / 255; // Get to 255 as quick as you can.

  float my_magnitude = FFT_Magnitude / 16.0;
  if (soundAgc)
    my_magnitude *= multAgc;
  if (sampleAvg < 1)
    my_magnitude = 0.001; // noise gate closed - mute

  fade_out(fadeRate);

  for (int i = 0; i < SEGMENT.intensity / 32 + 1; i++)
  {
    uint16_t locn = random16(0, SEGLEN);
    uint8_t pixCol = (log10f(FFT_MajorPeak) - 1.78) * 255.0 / (3.71 - 1.78); // Scale log10 of frequency values to the 255 colour index.
    setPixelColor(locn, color_blend(SEGCOLOR(1), color_from_palette(SEGMENT.intensity + pixCol, false, PALETTE_SOLID_WRAP, 0), (int)my_magnitude));
  }
  return FRAMETIME;
} // mode_freqpixels()

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

uint16_t WS2812FX::mode_freqwave(void)
{ // Freqwave. By Andreas Pleschung.

  // Instead of using colorpalettes, This effect works on the HSV color circle with red being the lowest frequency
  //
  // As a compromise between speed and accuracy we are currently sampling with 10240Hz, from which we can then determine with a 512bin FFT our max frequency is 5120Hz.
  // Depending on the music stream you have you might find it useful to change the frequency mapping.

  uint8_t secondHand = micros() / (256 - SEGMENT.speed) / 500 % 16;

  //  uint8_t secondHand = millis()/(256-SEGMENT.speed) % 10;
  if (SEGENV.aux0 != secondHand)
  {
    SEGENV.aux0 = secondHand;

    // uint8_t fade = SEGMENT.custom3;
    // uint8_t fadeval;

    float tmpSound = (soundAgc) ? sampleAgc : sampleAvg;

    float sensitivity = mapf(SEGMENT.custom3, 1, 255, 1, 10);
    float pixVal = tmpSound * (float)SEGMENT.intensity / 256.0 * sensitivity;
    if (pixVal > 255)
      pixVal = 255;

    float intensity = mapf(pixVal, 0, 255, 0, 100) / 100.0; // make a brightness from the last avg

    CRGB color = 0;
    CHSV c;

    if (FFT_MajorPeak > 5120)
      FFT_MajorPeak = 1.0f;
    // MajorPeak holds the freq. value which is most abundant in the last sample.
    // With our sampling rate of 10240Hz we have a usable freq range from roughtly 80Hz to 10240/2 Hz
    // we will treat everything with less than 65Hz as 0
    // Serial.printf("%5d ", FFT_MajorPeak, 0);
    if (FFT_MajorPeak < 80)
    {
      color = CRGB::Black;
    }
    else
    {
      int upperLimit = 20 * SEGMENT.custom2;
      int lowerLimit = 2 * SEGMENT.custom1;
      int i = lowerLimit != upperLimit ? map(FFT_MajorPeak, lowerLimit, upperLimit, 0, 255) : FFT_MajorPeak;
      if (i < 0)
        i = 0;
      uint16_t b = 255.0 * intensity;
      if (b > 255)
        b = 255;
      c = CHSV(i, 240, (uint8_t)b);
      color = c; // implicit conversion to RGB supplied by FastLED
    }

    // Serial.println(color);
    leds[segmentToLogical(SEGLEN / 2)] = color;

    // shift the pixels one pixel outwards
    for (int i = SEGLEN; i > SEGLEN / 2; i--)
    { // Move to the right.
      leds[segmentToLogical(i)] = leds[segmentToLogical(i - 1)];
    }
    for (int i = 0; i < SEGLEN / 2; i++)
    { // Move to the left.
      leds[segmentToLogical(i)] = leds[segmentToLogical(i + 1)];
    }

    // DISPLAY ARRAY
    setPixels(leds);
  }

  return FRAMETIME;
} // mode_freqwave()

///////////////////////
//    ** Gravfreq    //
///////////////////////

uint16_t WS2812FX::mode_gravfreq(void)
{ // Gravfreq. By Andrew Tuline.

  uint16_t dataSize = sizeof(gravity);
  if (!SEGENV.allocateData(dataSize))
    return mode_static(); // allocation failed
  Gravity *gravcen = reinterpret_cast<Gravity *>(SEGENV.data);

  fade_out(240);

  float tmpSound = (soundAgc) ? sampleAgc : sampleAvg;
  float segmentSampleAvg = tmpSound * (float)SEGMENT.intensity / 255.0;
  segmentSampleAvg *= 0.125; // divide by 8,  to compensate for later "sensitivty" upscaling

  float mySampleAvg = mapf(segmentSampleAvg * 2.0, 0, 32, 0, (float)SEGLEN / 2.0); // map to pixels availeable in current segment
  int tempsamp = constrain(mySampleAvg, 0, SEGLEN / 2);                            // Keep the sample from overflowing.
  uint8_t gravity = 8 - SEGMENT.speed / 32;

  for (int i = 0; i < tempsamp; i++)
  {

    int index = (log10f(FFT_MajorPeak) - (3.71 - 1.78)) * 255;
    if (index < 0)
      index = 0;

    setPixelColor(i + SEGLEN / 2, color_from_palette(index, false, PALETTE_SOLID_WRAP, 0));
    setPixelColor(SEGLEN / 2 - i - 1, color_from_palette(index, false, PALETTE_SOLID_WRAP, 0));
  }

  if (tempsamp >= gravcen->topLED)
    gravcen->topLED = tempsamp - 1;
  else if (gravcen->gravityCounter % gravity == 0)
    gravcen->topLED--;

  if (gravcen->topLED >= 0)
  {
    setPixelColor(gravcen->topLED + SEGLEN / 2, CRGB::Gray);
    setPixelColor(SEGLEN / 2 - 1 - gravcen->topLED, CRGB::Gray);
  }
  gravcen->gravityCounter = (gravcen->gravityCounter + 1) % gravity;

  return FRAMETIME;
} // mode_gravfreq()

//////////////////////
//   ** Noisemove   //
//////////////////////

uint16_t WS2812FX::mode_noisemove(void)
{ // Noisemove.    By: Andrew Tuline

  fade_out(224); // Just in case something doesn't get faded.

  uint8_t numBins = map(SEGMENT.intensity, 0, 255, 0, 16); // Map slider to fftResult bins.

  for (int i = 0; i < numBins; i++)
  {                                                                                           // How many active bins are we using.
    uint16_t locn = inoise16(millis() * SEGMENT.speed + i * 50000, millis() * SEGMENT.speed); // Get a new pixel location from moving noise.

    locn = map(locn, 7500, 58000, 0, SEGLEN - 1); // Map that to the length of the strand, and ensure we don't go over.
    locn = locn % SEGLEN;                         // Just to be bloody sure.

    setPixelColor(locn, color_blend(SEGCOLOR(1), color_from_palette(i * 64, false, PALETTE_SOLID_WRAP, 0), fftResult[i % 16] * 4));
  }

  return FRAMETIME;
} // mode_noisemove()

//////////////////////
//   ** Rocktaves   //
//////////////////////

uint16_t WS2812FX::mode_rocktaves(void)
{ // Rocktaves. Same note from each octave is same colour.    By: Andrew Tuline

  fadeToBlackBy(leds, 64); // Just in case something doesn't get faded.

  float frTemp = FFT_MajorPeak;
  uint8_t octCount = 0; // Octave counter.
  uint8_t volTemp = 0;

  float my_magnitude = FFT_Magnitude / 16.0; // scale magnitude to be aligned with scaling of FFT bins
  if (soundAgc)
    my_magnitude *= multAgc; // apply gain
  if (sampleAvg < 1)
    my_magnitude = 0.001; // mute

  if (my_magnitude > 32)
    volTemp = 255; // We need to squelch out the background noise.

  while (frTemp > 249)
  {
    octCount++; // This should go up to 5.
    frTemp = frTemp / 2;
  }

  frTemp -= 132;               // This should give us a base musical note of C3
  frTemp = fabs(frTemp * 2.1); // Fudge factors to compress octave range starting at 0 and going to 255;

  // Serial.print(frTemp); Serial.print("\t"); Serial.print(volTemp); Serial.print("\t");Serial.print(octCount); Serial.print("\t"); Serial.println(FFT_Magnitude);

  //    leds[beatsin8(8+octCount*4,0,SEGLEN-1,0,octCount*8)] += CHSV((uint8_t)frTemp,255,volTemp);                 // Back and forth with different frequencies and phase shift depending on current octave.

  leds[segmentToLogical(mapf(beatsin8(8 + octCount * 4, 0, 255, 0, octCount * 8), 0, 255, 0, SEGLEN - 1))] += color_blend(SEGCOLOR(1), color_from_palette((uint8_t)frTemp, false, PALETTE_SOLID_WRAP, 0), volTemp);

  setPixels(leds);
  return FRAMETIME;
} // mode_rockdaves()

///////////////////////
//   ** Waterfall    //
///////////////////////

// Combines peak detection with FFT_MajorPeak and FFT_Magnitude.

uint16_t WS2812FX::mode_waterfall(void)
{ // Waterfall. By: Andrew Tuline

  if (SEGENV.call == 0)
    fill_solid(leds, 0);

  binNum = SEGMENT.custom2;     // Select a bin.
  maxVol = SEGMENT.custom3 / 2; // Our volume comparator.

  uint8_t secondHand = micros() / (256 - SEGMENT.speed) / 500 + 1 % 16;

  if (SEGENV.aux0 != secondHand)
  { // Triggered millis timing.
    SEGENV.aux0 = secondHand;

    float my_magnitude = FFT_Magnitude / 8.0;
    if (soundAgc)
      my_magnitude *= multAgc;
    if (sampleAvg < 1)
      my_magnitude = 0.001; // noise gate closed - mute

    int pixCol = (log10f(FFT_MajorPeak) - 2.26) * 177; // log10 frequency range is from 2.26 to 3.7. Let's scale accordingly.
    if (pixCol < 0)
      pixCol = 0;

    if (samplePeak)
    {
      leds[segmentToLogical(SEGLEN - 1)] = CHSV(92, 92, 92);
    }
    else
    {
      leds[segmentToLogical(SEGLEN - 1)] = color_blend(SEGCOLOR(1), color_from_palette(pixCol + SEGMENT.intensity, false, PALETTE_SOLID_WRAP, 0), (int)my_magnitude);
    }
    for (int i = 0; i < SEGLEN - 1; i++)
      leds[segmentToLogical(i)] = leds[segmentToLogical(i + 1)];
  }

  setPixels(leds);
  return FRAMETIME;
} // mode_waterfall()

//////////////////////////////////////////////
//       START of 2D REACTIVE ROUTINES      //
//////////////////////////////////////////////

/////////////////////////
//     ** 2D GEQ       //
/////////////////////////

uint16_t WS2812FX::GEQ_base(bool centered_horizontal, bool centered_vertical, bool color_vertical)
{ // By Will Tatam. Refactor by Ewoud Wijma.
  fadeToBlackBy(leds, SEGMENT.speed);

  bool rippleTime;
  if (millis() - SEGENV.step >= 255 - SEGMENT.intensity)
  {
    SEGENV.step = millis();
    rippleTime = true;
  }
  else
    rippleTime = false;

  static int previousBarHeight[64]; // array of previous bar heights per frequency band

  int xCount = SEGMENT.width;
  if (centered_vertical)
    xCount /= 2;

  for (int x = 0; x < xCount; x++)
  {
    int band = map(x, 0, xCount - 1, 0, 15);
    int barHeight = map(fftResult[band], 0, 255, 0, SEGMENT.height);
    if ((barHeight % 2 == 1) && centered_horizontal)
      barHeight++;                                                                          // get an even barHeight if centered_horizontal
    int yStartBar = centered_horizontal ? (SEGMENT.height - barHeight) / 2 : 0;             // lift up the bar if centered_horizontal
    int yStartPeak = centered_horizontal ? (SEGMENT.height - previousBarHeight[x]) / 2 : 0; // lift up the peaks if centered_horizontal

    for (int y = 0; y < SEGMENT.height; y++)
    {
      CRGB heightColor = CRGB::Black;
      uint16_t colorIndex;
      if (color_vertical)
      {
        if (centered_horizontal)
          colorIndex = map(abs(y - (SEGMENT.height - 1) / 2.0), 0, SEGMENT.height / 2 - 1, 0, 255);
        else
          colorIndex = map(y, 0, SEGMENT.height - 1, 0, 255);
      }
      else
        colorIndex = band * 17;
      heightColor = color_from_palette(colorIndex, false, PALETTE_SOLID_WRAP, 0);

      CRGB ledColor = CRGB::Black; // if not part of bars or peak, make black (not fade to black)

      // bar
      if (y >= yStartBar && y < yStartBar + barHeight)
        ledColor = heightColor;

      // low and high peak (must exist && on peak position && only below if centered_horizontal effect)
      if ((previousBarHeight[x] > 0) && (SEGMENT.intensity < 255) && (y == yStartPeak || y == yStartPeak + previousBarHeight[x] - 1) && (centered_horizontal || y != yStartPeak))
        ledColor = SEGCOLOR(2) == CRGB::Black ? heightColor : CRGB(SEGCOLOR(2)); // low peak

      if (centered_vertical)
      {
        leds[XY(SEGMENT.width / 2 + x, SEGMENT.height - 1 - y)] = ledColor;
        leds[XY(SEGMENT.width / 2 - 1 - x, SEGMENT.height - 1 - y)] = ledColor;
      }
      else
        leds[XY(x, SEGMENT.height - 1 - y)] = ledColor;
    }

    if (rippleTime)
      previousBarHeight[x] -= centered_horizontal ? 2 : 1; // delay/ripple effect
    if (barHeight > previousBarHeight[x])
      previousBarHeight[x] = barHeight; // drive the peak up
  }

  setPixels(leds);
  return FRAMETIME;
} // GEQ_base

uint16_t WS2812FX::mode_2DGEQ(void)
{ // By Will Tatam. Code reduction by Ewoud Wijma.
  return GEQ_base(false, false, false);
} // mode_2DGEQ()

/////////////////////////
//   ** 2D CenterBars  //
/////////////////////////

uint16_t WS2812FX::mode_2DCenterBars(void)
{ // Written by Scott Marley Adapted by  Spiro-C..
  return GEQ_base(SEGMENT.custom1 > 128, SEGMENT.custom2 > 128, SEGMENT.custom3 > 128);
} // mode_2DCenterBars()

/////////////////////////
//  ** 2D Funky plank  //
/////////////////////////

uint16_t WS2812FX::mode_2DFunkyPlank(void)
{ // Written by ??? Adapted by Will Tatam.

  int NUMB_BANDS = map(SEGMENT.custom1, 0, 255, 1, 16);
  int barWidth = (SEGMENT.width / NUMB_BANDS);
  int bandInc = 1;
  if (barWidth == 0)
  {
    // Matrix narrower than fft bands
    barWidth = 1;
    bandInc = (NUMB_BANDS / SEGMENT.width);
  }

  uint8_t secondHand = micros() / (256 - SEGMENT.speed) / 500 + 1 % 64;

  if (SEGENV.aux0 != secondHand)
  { // Triggered millis timing.
    SEGENV.aux0 = secondHand;

    // display values of
    int b = 0;
    for (int band = 0; band < NUMB_BANDS; band += bandInc)
    {
      int hue = fftResult[band];
      int v = map(fftResult[band], 0, 255, 10, 255);
      //     if(hue > 0) Serial.printf("Band: %u Value: %u\n", band, hue);
      for (int w = 0; w < barWidth; w++)
      {
        int xpos = (barWidth * b) + w;
        leds[XY(xpos, 0)] = CHSV(hue, 255, v);
      }
      b++;
    }

    // Update the display:
    for (int i = (SEGMENT.height - 1); i > 0; i--)
    {
      for (int j = (SEGMENT.width - 1); j >= 0; j--)
      {
        int src = XY(j, (i - 1));
        int dst = XY(j, i);
        leds[dst] = leds[src];
      }
    }
  }

  setPixels(leds);
  return FRAMETIME;
} // mode_2DFunkyPlank

uint16_t WS2812FX::mode_2DAkemi(void)
{

  uint16_t counter = (now * ((SEGMENT.speed >> 2) + 2)) & 0xFFFF;
  counter = counter >> 8;

  // Akemi
  uint8_t akemi[32][32] = {
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 3, 3, 3, 3, 3, 3, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 3, 0, 0, 0, 0, 0, 0, 3, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 0, 0, 0, 6, 5, 5, 4, 0, 0, 0, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 0, 0, 6, 6, 5, 5, 5, 5, 4, 4, 0, 0, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 0, 6, 5, 5, 5, 5, 5, 5, 5, 5, 4, 0, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 2, 3, 0, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 0, 3, 2, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 3, 2, 0, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 0, 2, 3, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 3, 2, 3, 6, 5, 5, 7, 7, 5, 5, 5, 5, 7, 7, 5, 5, 4, 3, 2, 3, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 2, 3, 1, 3, 6, 5, 1, 7, 7, 7, 5, 5, 1, 7, 7, 7, 5, 4, 3, 1, 3, 2, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 8, 3, 1, 3, 6, 5, 1, 7, 7, 7, 5, 5, 1, 7, 7, 7, 5, 4, 3, 1, 3, 8, 9, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 8, 3, 1, 3, 6, 5, 5, 1, 1, 5, 5, 5, 5, 1, 1, 5, 5, 4, 3, 1, 3, 8, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 2, 3, 1, 3, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 3, 1, 3, 2, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 3, 2, 3, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 3, 2, 3, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 5, 5, 5, 5, 5, 7, 7, 5, 5, 5, 5, 5, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {1, 0, 0, 0, 0, 0, 0, 0, 0, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 0, 0, 0, 0, 0, 0, 0, 0, 2},
      {0, 2, 2, 2, 0, 0, 0, 0, 0, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 0, 0, 0, 0, 0, 2, 2, 2, 0},
      {0, 0, 0, 3, 2, 0, 0, 0, 6, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 2, 2, 0, 0, 0},
      {0, 0, 0, 3, 2, 0, 0, 0, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 0, 0, 0, 2, 3, 0, 0, 0},
      {0, 0, 0, 0, 3, 2, 0, 0, 0, 0, 3, 3, 0, 3, 3, 0, 0, 3, 3, 0, 3, 3, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0},
      {0, 0, 0, 0, 3, 2, 0, 0, 0, 0, 3, 2, 0, 3, 2, 0, 0, 3, 2, 0, 3, 2, 0, 0, 0, 0, 2, 3, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 3, 2, 0, 0, 3, 2, 0, 0, 3, 2, 0, 0, 3, 2, 0, 0, 3, 2, 0, 0, 2, 3, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 3, 2, 2, 2, 2, 0, 0, 0, 3, 2, 0, 0, 3, 2, 0, 0, 0, 3, 2, 2, 2, 3, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 3, 3, 3, 0, 0, 0, 0, 3, 2, 0, 0, 3, 2, 0, 0, 0, 0, 3, 3, 3, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 0, 0, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 0, 0, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 0, 0, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 0, 0, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  };

  // draw and color Akemi
  for (int y = 0; y < SEGMENT.height; y++)
    for (int x = 0; x < SEGMENT.width; x++)
    {
      CRGB color = BLACK;
      CRGB faceColor = color_wheel(counter);
      CRGB armsAndLegsColor = SEGCOLOR(1) > 0 ? SEGCOLOR(1) : 0xFFE0A0; // default warmish white 0xABA8FF; //0xFF52e5;//
      CRGB soundColor = ORANGE;
      double lightFactor = 0.15;
      double normalFactor = 0.4;
      double base = fftResult[0] / 255.0;
      switch (akemi[y * 32 / SEGMENT.height][x * 32 / SEGMENT.width])
      {
      case 0:
        color = BLACK;
        break;
      case 3:
        armsAndLegsColor.r *= lightFactor;
        armsAndLegsColor.g *= lightFactor;
        armsAndLegsColor.b *= lightFactor;
        color = armsAndLegsColor;
        break; // light arms and legs 0x9B9B9B
      case 2:
        armsAndLegsColor.r *= normalFactor;
        armsAndLegsColor.g *= normalFactor;
        armsAndLegsColor.b *= normalFactor;
        color = armsAndLegsColor;
        break; // normal arms and legs 0x888888
      case 1:
        color = armsAndLegsColor;
        break; // dark arms and legs 0x686868
      case 6:
        faceColor.r *= lightFactor;
        faceColor.g *= lightFactor;
        faceColor.b *= lightFactor;
        color = faceColor;
        break; // light face 0x31AAFF
      case 5:
        faceColor.r *= normalFactor;
        faceColor.g *= normalFactor;
        faceColor.b *= normalFactor;
        color = faceColor;
        break; // normal face 0x0094FF
      case 4:
        color = faceColor;
        break; // dark face 0x007DC6
      case 7:
        color = SEGCOLOR(2) > 0 ? SEGCOLOR(2) : 0xFFFFFF;
        break; // eyes and mouth default white
      case 8:
        if (base > 0.4)
        {
          soundColor.r *= base;
          soundColor.g *= base;
          soundColor.b *= base;
          color = soundColor;
        }
        else
          color = armsAndLegsColor;
        break;
      default:
        color = BLACK;
      }

      if (SEGMENT.intensity > 128 && fftResult[0] > 128) // dance if base is high
      {
        leds[XY(x, 0)] = BLACK;
        leds[XY(x, y + 1)] = color;
      }
      else
        leds[XY(x, y)] = color;
    }

  // add geq left and right
  for (int x = 0; x < SEGMENT.width / 8; x++)
  {
    int band = x * SEGMENT.width / 8;
    int barHeight = map(fftResult[band], 0, 255, 0, 17 * SEGMENT.height / 32);
    CRGB color = color_from_palette((band * 35), false, PALETTE_SOLID_WRAP, 0);

    for (int y = 0; y < barHeight; y++)
    {
      leds[XY(x, SEGMENT.height / 2 - y)] = color;
      leds[XY(SEGMENT.width - 1 - x, SEGMENT.height / 2 - y)] = color;
    }
  }

  setPixels(leds);

  return FRAMETIME;
} // mode_2DAkemi

// 3D !!!!!!!!!!

float distance(uint16_t x1, uint16_t y1, uint16_t z1, uint16_t x2, uint16_t y2, uint16_t z2)
{
  return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) + (z1 - z2) * (z1 - z2));
}

uint16_t WS2812FX::mode_3DRipples(void)
{
  float ripple_interval = 1.3 * (SEGMENT.intensity / 128.0);

  fill(CRGB::Black);

  // workaround to get width, height and depth
  uint16_t mW = strip.matrixWidth;
  uint16_t mH = 1;
  uint16_t mD = 1;
  // balance dimensions
  while (mW > mH)
  {
    if (mH < mD)
      mH++;
    else
      mD++;
    mW = strip.matrixWidth / mH / mD;
  }

  for (int z = 0; z < mD; z++)
  {
    for (int x = 0; x < mW; x++)
    {
      float d = distance(3.5, 3.5, 0, x, z, 0) / 9.899495 * mH;
      uint16_t height = floor(mH / 2.0 + sinf(d / ripple_interval + SEGENV.call / ((256.0 - SEGMENT.speed) / 20.0)) * mH / 2.0); // between 0 and 8

      setPixelColor(x + height * mW + z * mW * mH, color_from_palette(SEGENV.call, true, PALETTE_SOLID_WRAP, 0));
    }
  }

  return FRAMETIME;
} // mode_3DRipples

uint16_t WS2812FX::mode_3DSphereMove(void)
{
  uint16_t origin_x, origin_y, origin_z, d;
  float diameter;

  fill(CRGB::Black);

  uint32_t interval = SEGENV.call / ((256.0 - SEGMENT.speed) / 20.0);

  // workaround to get width, height and depth
  uint16_t mW = strip.matrixWidth;
  uint16_t mH = 1;
  uint16_t mD = 1;
  // balance dimensions
  while (mW > mH)
  {
    if (mH < mD)
      mH++;
    else
      mD++;
    mW = strip.matrixWidth / mH / mD;
  }

  origin_x = 3.5 + sinf(interval) * 2.5;
  origin_y = 3.5 + cosf(interval) * 2.5;
  origin_z = 3.5 + cosf(interval) * 2.0;

  diameter = 2.0 + sinf(interval / 3.0);

  for (int x = 0; x < mW; x++)
  {
    for (int y = 0; y < mH; y++)
    {
      for (int z = 0; z < mD; z++)
      {
        d = distance(x, y, z, origin_x, origin_y, origin_z);

        if (d > diameter && d < diameter + 1)
        {
          setPixelColor(x + y * mW + z * mW * mH, color_from_palette(SEGENV.call, true, PALETTE_SOLID_WRAP, 0));
        }
      }
    }
  }

  return FRAMETIME;
} // mode_3DSphereMove
