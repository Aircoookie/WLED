/*
  WS2812FX.cpp - Library for WS2812 LED effects.
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
  CHANGELOG
  2016-05-28   Initial beta release
  2016-06-03   Code cleanup, minor improvements, new modes
  2016-06-04   2 new fx, fixed setColor (now also resets _mode_color)
*/

#include "Arduino.h"
#include "WS2812FX.h"

#define CALL_MODE(n) (this->*_mode[n])();

void WS2812FX::init(bool supportWhite, uint16_t countPixels, uint8_t pin) {
  for (int i=0; i < _led_count; i++) _locked[i] = false;
  begin(supportWhite,countPixels,pin);
  WS2812FX::setBrightness(_brightness);
  show();
}

void WS2812FX::service() {
  if(_running || _triggered) {
    unsigned long now = millis();

    if(now - _mode_last_call_time > _mode_delay || _triggered) {
      CALL_MODE(_mode_index);
      _counter_mode_call++;
      _mode_last_call_time = now;
      _triggered = false;
    }
  }
}

void WS2812FX::trigger() {
  _triggered = true;
}

void WS2812FX::start() {
  _counter_mode_call = 0;
  _counter_mode_step = 0;
  _mode_last_call_time = 0;
  _running = true;
  show();
}

void WS2812FX::stop() {
  _running = false;
  strip_off();
}

void WS2812FX::setMode(byte m) {
  _counter_mode_call = 0;
  _counter_mode_step = 0;
  _mode_last_call_time = 0;
  _mode_index = constrain(m, 0, MODE_COUNT-1);
  _mode_color = _color;
  _mode_var1 = 0;
  setBrightness(_brightness);
  strip_off_respectLock();
}

void WS2812FX::setSpeed(byte s) {
  _mode_last_call_time = 0;
  _speed = constrain(s, SPEED_MIN, SPEED_MAX);
}

void WS2812FX::increaseSpeed(byte s) {
  s = constrain(_speed + s, SPEED_MIN, SPEED_MAX);
  setSpeed(s);
}

void WS2812FX::decreaseSpeed(byte s) {
  s = constrain(_speed - s, SPEED_MIN, SPEED_MAX);
  setSpeed(s);
}

void WS2812FX::setIntensity(byte in) {
  _intensity=in;
}

void WS2812FX::setColor(byte r, byte g, byte b) {
  setColor(((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}


void WS2812FX::setColor(byte r, byte g, byte b, byte w) {
  setColor(((uint32_t)w << 24)|((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}

void WS2812FX::setSecondaryColor(byte r, byte g, byte b) {
  setSecondaryColor(((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}


void WS2812FX::setSecondaryColor(byte r, byte g, byte b, byte w) {
  setSecondaryColor(((uint32_t)w << 24)|((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}

void WS2812FX::setColor(uint32_t c) {
  _color = c;
  _mode_color = _color;
  setBrightness(_brightness);
}

void WS2812FX::setSecondaryColor(uint32_t c) {
  _color_sec = c;
  if (_cronixieMode) _cronixieSecMultiplier = getSafePowerMultiplier(900, 100, c, _brightness);
  setBrightness(_brightness);
}

void WS2812FX::increaseBrightness(byte s) {
  s = constrain(_brightness + s, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
  setBrightness(s);
}

void WS2812FX::decreaseBrightness(byte s) {
  s = constrain(_brightness - s, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
  setBrightness(s);
}

bool WS2812FX::isRunning() {
  return _running; 
}

byte WS2812FX::getMode(void) {
  return _mode_index;
}

byte WS2812FX::getSpeed(void) {
  return _speed;
}

byte WS2812FX::getBrightness(void) {
  return _brightness;
}

byte WS2812FX::getModeCount(void) {
  return MODE_COUNT;
}

uint32_t WS2812FX::getColor(void) {
  return _color; 
}

/* #####################################################
#
#  Color and Blinken Functions
#
##################################################### */

/*
 * Turns everything off. Doh.
 */
void WS2812FX::strip_off() {
  clear();
  show();
}

void WS2812FX::strip_off_respectLock() {
  for(uint16_t i=0; i < _led_count; i++) {
    if (!_locked[i])
    setPixelColor(i, 0);
  }
  show();
}


/*
 * Put a value 0 to 255 in to get a color value.
 * The colours are a transition r -> g -> b -> back to r
 * Inspired by the Adafruit examples.
 */
uint32_t WS2812FX::color_wheel(byte pos) {
  pos = 255 - pos;
  if(pos < 85) {
    return ((uint32_t)(255 - pos * 3) << 16) | ((uint32_t)(0) << 8) | (pos * 3);
  } else if(pos < 170) {
    pos -= 85;
    return ((uint32_t)(0) << 16) | ((uint32_t)(pos * 3) << 8) | (255 - pos * 3);
  } else {
    pos -= 170;
    return ((uint32_t)(pos * 3) << 16) | ((uint32_t)(255 - pos * 3) << 8) | (0);
  }
}


/*
 * Returns a new, random wheel index with a minimum distance of 42 from pos.
 */
byte WS2812FX::get_random_wheel_index(byte pos) {
  byte r = 0;
  byte x = 0;
  byte y = 0;
  byte d = 0;

  while(d < 42) {
    r = random(256);
    x = abs(pos - r);
    y = 255 - x;
    d = minval(x, y);
  }

  return r;
}


/*
 * No blinking. Just plain old static light.
 */
void WS2812FX::mode_static(void) {
  for(uint16_t i=0; i < _led_count; i++) {
    if (!_locked[i])
    setPixelColor(i, _color);
  }
  show();
  _mode_delay = (_fastStandard) ? 25 : 500;
}


/*
 * Normal blinking. on/off duty time set by FX intensity.
 */
void WS2812FX::mode_blink(void) {
  if(_counter_mode_call % 2 == 1) {
    for(uint16_t i=0; i < _led_count; i++) {
      if (!_locked[i])
      setPixelColor(i, _color);
    }
    _mode_delay = (100 + ((1986 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX))*(float)(_intensity/128.0);
  } else {
    for(uint16_t i=0; i < _led_count; i++) {
      if (!_locked[i])
      setPixelColor(i, _color_sec);
    }
    _mode_delay = (100 + ((1986 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX))*(float)(2.0-(_intensity/128.0));
  }
  show();
}


/*
 * Lights all LEDs after each other up. Then turns them in
 * that order off (2nd color). Repeat.
 */
void WS2812FX::mode_color_wipe(void) {
  if(_counter_mode_step < _led_count) {
    if (!_locked[_counter_mode_step])
    setPixelColor(_counter_mode_step, _color);
  } else {
    if (!_locked[_counter_mode_step - _led_count])
    setPixelColor(_counter_mode_step - _led_count, _color_sec);
  }
  show();

  _counter_mode_step = (_counter_mode_step + 1) % (_led_count * 2);

  _mode_delay = 5 + ((50 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * Turns all LEDs after each other to a random color.
 * Then starts over with another color.
 */
void WS2812FX::mode_color_wipe_random(void) {
  if(_counter_mode_step == 0) {
    _mode_color = get_random_wheel_index(_mode_color);
  }
  if (!_locked[_counter_mode_step])
  setPixelColor(_counter_mode_step, color_wheel(_mode_color));
  show();

  _counter_mode_step = (_counter_mode_step + 1) % _led_count;

  _mode_delay = 5 + ((50 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * Lights all LEDs in one random color up. Then switches them
 * to the next random color.
 */
void WS2812FX::mode_random_color(void) {
  _mode_color = get_random_wheel_index(_mode_color);

  for(uint16_t i=0; i < _led_count; i++) {
    if (!_locked[i])
    setPixelColor(i, color_wheel(_mode_color));
  }
  
  show();
  _mode_delay = 100 + ((5000 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
}


/*
 * Lights some pastel colors
 */
void WS2812FX::mode_easter(void) {
  //uint32_t cols[]{0x00F7ECC5,0x00F8D5C7,0x00F9E2E7,0x00BED9D4,0x00F7ECC5,0x00F8D5C7,0x00F9E2E7};
  uint32_t cols[]{0x00FF8040,0x00E5D241,0x0077FF77,0x0077F0F0,0x00FF8040,0x00E5D241,0x0077FF77};
  mode_colorful_internal(cols);
}


/*
 * Lights multiple random leds in a random color (higher intensity, more updates)
 */
void WS2812FX::mode_dynamic(void) {
  if(_counter_mode_call == 0) {
    for(uint16_t i=0; i < _led_count; i++) {
      if (!_locked[i])
      setPixelColor(i, color_wheel(random(256)));
    }
  }
  if (_intensity > 0) //multi dynamic
  {
    for(uint16_t i=0; i < _led_count; i++) {
      if (!_locked[i] && random(256)<_intensity)
      setPixelColor(i, color_wheel(random(256)));
    }
  } else { //single dynamic
    int ran = random(_led_count);
    if (!_locked[ran])
    setPixelColor(ran, color_wheel(random(256)));
  }
  show();
  _mode_delay = 100 + ((5000 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
}


/*
 * Does the "standby-breathing" of well known i-Devices. Fixed Speed.
 * Use mode "fade" if you like to have something similar with a different speed.
 */
void WS2812FX::mode_breath(void) {
  //                                      0    1    2   3   4   5   6    7   8   9  10  11   12   13   14   15   16    // step
  uint16_t breath_delay_steps[] =     {   7,   9,  13, 15, 16, 17, 18, 930, 19, 18, 15, 13,   9,   7,   4,   5,  10 }; // magic numbers for breathing LED
  byte breath_brightness_steps[] = { 150, 125, 100, 75, 50, 25, 16,  15, 16, 25, 50, 75, 100, 125, 150, 220, 255 }; // even more magic numbers!

  if(_counter_mode_call == 0) {
    _mode_color = breath_brightness_steps[0] + 1;
  }

  byte breath_brightness = _mode_color; // we use _mode_color to store the brightness

  if(_counter_mode_step < 8) {
    breath_brightness--;
  } else {
    breath_brightness++;
  }

  // update index of current delay when target brightness is reached, start over after the last step
  if(breath_brightness == breath_brightness_steps[_counter_mode_step]) {
    _counter_mode_step = (_counter_mode_step + 1) % (sizeof(breath_brightness_steps)/sizeof(byte));
  }
  
  for(uint16_t i=0; i < _led_count; i++) {
    if (!_locked[i])
    setPixelColor(i, _color);           // set all LEDs to selected color
  }
  int b = map(breath_brightness, 0, 255, 0, _brightness);  // keep brightness below brightness set by user
  bus->SetBrightness(b);                     // set new brightness to leds
  show();

  _mode_color = breath_brightness;                         // we use _mode_color to store the brightness
  _mode_delay = breath_delay_steps[_counter_mode_step];
}


/*
 * Fades the LEDs on and (almost) off again.
 */
void WS2812FX::mode_fade(void) {

  int y = _counter_mode_step - 127;
  y = 256 - (abs(y) * 2);
  double z = (double)y/256;
  byte w = ((_color >> 24) & 0xFF), ws = ((_color_sec >> 24) & 0xFF);
  byte r = ((_color >> 16) & 0xFF), rs = ((_color_sec >> 16) & 0xFF);
  byte g = ((_color >> 8) & 0xFF), gs = ((_color_sec >> 8) & 0xFF);
  byte b = (_color & 0xFF), bs = (_color_sec & 0xFF);
  w = w+((ws - w)*z);
  r = r+((rs - r)*z);
  g = g+((gs - g)*z);
  b = b+((bs - b)*z);
  for(uint16_t i=0; i < _led_count; i++) {
    if (!_locked[i])
    setPixelColor(i, r, g, b, w);
  }
  show();

  _counter_mode_step = (_counter_mode_step + 1) % 256;
  _mode_delay = 5 + ((15 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
}


/*
 * Runs a single pixel back and forth.
 */
void WS2812FX::mode_scan(void) {
  if(_counter_mode_step > (_led_count*2) - 2) {
    _counter_mode_step = 0;
  }
  _counter_mode_step++;

  int i = _counter_mode_step - (_led_count - 1);
  i = abs(i);

  for(uint16_t x=0; x < _led_count; x++) {
    if (!_locked[x])
    setPixelColor(x, _color_sec);
  }
  if (!_locked[i])
  setPixelColor(abs(i), _color);
  show();

  _mode_delay = 10 + ((30 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * Runs two pixel back and forth in opposite directions.
 */
void WS2812FX::mode_dual_scan(void) {
  if(_counter_mode_step > (_led_count*2) - 2) {
    _counter_mode_step = 0;
  }
  _counter_mode_step++;

  int i = _counter_mode_step - (_led_count - 1);
  i = abs(i);
  for(uint16_t x=0; x < _led_count; x++) {
    if (!_locked[x])
    setPixelColor(x, _color_sec);
  }
  if (!_locked[i])
  setPixelColor(i, _color);
  if (!_locked[_led_count - (i+1)])
  setPixelColor(_led_count - (i+1), _color);
  show();

  _mode_delay = 10 + ((30 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * Cycles all LEDs at once through a rainbow.
 */
void WS2812FX::mode_rainbow(void) {
  uint32_t color = color_wheel(_counter_mode_step);
  for(uint16_t i=0; i < _led_count; i++) {
    if (!_locked[i])
    setPixelColor(i, color);
  }
  show();

  _counter_mode_step = (_counter_mode_step + 1) % 256;

  _mode_delay = 1 + ((50 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
}


/*
 * Cycles a rainbow over the entire string of LEDs.
 */
void WS2812FX::mode_rainbow_cycle(void) {
  for(uint16_t i=0; i < _led_count; i++) {
    if (!_locked[i])
    setPixelColor(i, color_wheel(((i * 256 / ((uint16_t)(_led_count*(float)(_intensity/128.0))+1)) + _counter_mode_step) % 256));
  }
  show();

  _counter_mode_step = (_counter_mode_step + 1) % 256;

  _mode_delay = 1 + ((50 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
}


/*
 * Theatre-style crawling lights.
 * Inspired by the Adafruit examples.
 */
void WS2812FX::mode_theater_chase(void) {
  byte j = _counter_mode_call % 6;
  if(j % 2 == 0) {
    for(uint16_t i=0; i < _led_count; i=i+3) {
      if (!_locked[i+(j/2)])
      setPixelColor(i+(j/2), _color);
    }
    show();
    _mode_delay = 50 + ((500 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
  } else {
    for(uint16_t i=0; i < _led_count; i=i+3) {
      if (!_locked[i+(j/2)])
      setPixelColor(i+(j/2), _color_sec);
    }
    _mode_delay = 1;
  }
}


/*
 * Theatre-style crawling lights with rainbow effect.
 * Inspired by the Adafruit examples.
 */
void WS2812FX::mode_theater_chase_rainbow(void) {
  byte j = _counter_mode_call % 6;
  if(j % 2 == 0) {
    for(uint16_t i=0; i < _led_count; i=i+3) {
      if (!_locked[i+(j/2)])
      setPixelColor(i+(j/2), color_wheel((i+_counter_mode_step) % 256));
    }
    show();
    _mode_delay = 50 + ((500 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
  } else {
    for(uint16_t i=0; i < _led_count; i=i+3) {
      if (!_locked[i+(j/2)])
      setPixelColor(i+(j/2), _color_sec);
    }
    _mode_delay = 1;
  }
  _counter_mode_step = (_counter_mode_step + 1) % 256;
}


/*
 * Running lights effect with smooth sine transition.
 */
void WS2812FX::mode_running_lights(void) {
  byte w = ((_color >> 24) & 0xFF);
  byte r = ((_color >> 16) & 0xFF);
  byte g = ((_color >> 8) & 0xFF);
  byte b = (_color & 0xFF);

  for(uint16_t i=0; i < _led_count; i++) {
    int s = (sin(i+_counter_mode_call) * 127) + 128;
    if (!_locked[i])
    setPixelColor(i, (((uint32_t)(r * s)) / 255), (((uint32_t)(g * s)) / 255), (((uint32_t)(b * s)) / 255), (((uint32_t)(w * s)) / 255));
  }

  show();

  _mode_delay = 35 + ((350 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
}


/*
 * Blink several LEDs on, reset, repeat.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
void WS2812FX::mode_twinkle(void) {
  if(_counter_mode_step == 0) {
    for (int i = 0; i < _led_count; i++)
    {
      setPixelColor(i, _color_sec);
    }
    uint16_t min_leds = maxval(1, _led_count/5); // make sure, at least one LED is on
    uint16_t max_leds = maxval(1, _led_count/2); // make sure, at least one LED is on
    _counter_mode_step = random(min_leds, max_leds);
  }
  int ran = random(_led_count);
  if (!_locked[ran])
  setPixelColor(ran, _mode_color);
  show();

  _counter_mode_step--;
  _mode_delay = 50 + ((1986 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
}


/*
 * Blink several LEDs in random colors on, reset, repeat.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
void WS2812FX::mode_twinkle_random(void) {
  _mode_color = color_wheel(random(256));
  mode_twinkle();
}


/*
 * Blink several LEDs on, fading out.
 */
void WS2812FX::mode_twinkle_fade(void) {

  for(uint16_t i=0; i < _led_count; i++) {
    uint32_t px_rgb = getPixelColor(i);

    byte px_w = (px_rgb & 0xFF000000) >> 24;
    byte px_r = (px_rgb & 0x00FF0000) >> 16;
    byte px_g = (px_rgb & 0x0000FF00) >>  8;
    byte px_b = (px_rgb & 0x000000FF) >>  0;

    // fade out (divide by 2)
    px_w = px_w >> 1;
    px_r = px_r >> 1;
    px_g = px_g >> 1;
    px_b = px_b >> 1;
    if (!_locked[i])
    setPixelColor(i, px_r, px_g, px_b, px_w);
  }

  if(random(256) < _intensity) {
    int ran = random(_led_count);
    if (!_locked[ran])
    setPixelColor(ran, _mode_color);
  }

  show();

  _mode_delay = 100 + ((100 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
}


/*
 * Blink several LEDs in random colors on, fading out.
 */
void WS2812FX::mode_twinkle_fade_random(void) {
  _mode_color = color_wheel(random(256));
  mode_twinkle_fade();
}


/*
 * Blinks one LED at a time.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
void WS2812FX::mode_sparkle(void) {
  for(uint16_t i=0; i < _led_count; i++) {
    if (!_locked[i])
    setPixelColor(i, _color_sec);
  }
  int ran = random(_led_count);
  if (!_locked[ran])
  setPixelColor(ran ,_color);
  show();
  _mode_delay = 10 + ((200 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
}


/*
 * Lights all LEDs in the _color. Flashes single secondary color pixels randomly.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
void WS2812FX::mode_flash_sparkle(void) {
  for(uint16_t i=0; i < _led_count; i++) {
    if (!_locked[i])
    setPixelColor(i, _color);
  }

  if(random(256) <= _intensity) {
    int ran = random(_led_count);
    if (!_locked[ran])
    setPixelColor(ran , _color_sec);
    _mode_delay = 20;
  } else {
    _mode_delay = 20 + ((200 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
  }

  show();
}


/*
 * Like flash sparkle. With more flash.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
void WS2812FX::mode_hyper_sparkle(void) {
  for(uint16_t i=0; i < _led_count; i++) {
    if (!_locked[i])
    setPixelColor(i, _color);
  }

  if(random(256) <= _intensity) {
    for(uint16_t i=0; i < maxval(1, _led_count/3); i++) {
      int ran = random(_led_count);
      if (!_locked[ran])
      setPixelColor(ran , _color_sec);
    }
    _mode_delay = 20;
  } else {
    _mode_delay = 15 + ((120 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
  }
  show();
}


/*
 * Classic Strobe effect.
 */
void WS2812FX::mode_strobe(void) {
  if(_counter_mode_call % 2 == 0) {
    for(uint16_t i=0; i < _led_count; i++) {
      if (!_locked[i])
      setPixelColor(i, _color);
    }
    _mode_delay = 20;
  } else {
    for(uint16_t i=0; i < _led_count; i++) {
      if (!_locked[i])
      setPixelColor(i, _color_sec);
    }
    _mode_delay = 50 + ((1986 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
  }
  show();
}


/*
 * Strobe effect with different strobe count and pause, controlled by _speed.
 */
void WS2812FX::mode_multi_strobe(void) {
  for(uint16_t i=0; i < _led_count; i++) {
    if (!_locked[i])
    setPixelColor(i, _color_sec);
  }

  if(_counter_mode_step < (2 * ((_speed / 10) + 1))) {
    if(_counter_mode_step % 2 == 0) {
      for(uint16_t i=0; i < _led_count; i++) {
        if (!_locked[i])
        setPixelColor(i, _color);
      }
      _mode_delay = 20;
    } else {
      _mode_delay = 50;
    }

  } else {
    _mode_delay = 100 + ((9 - (_speed % 10)) * 125);
  }

  show();
  _counter_mode_step = (_counter_mode_step + 1) % ((2 * ((_speed / 10) + 1)) + 1);
}


/*
 * Classic Strobe effect. Cycling through the rainbow.
 */
void WS2812FX::mode_strobe_rainbow(void) {
  if(_counter_mode_call % 2 == 0) {
    for(uint16_t i=0; i < _led_count; i++) {
      if (!_locked[i])
      setPixelColor(i, color_wheel(_counter_mode_call % 256));
    }
    _mode_delay = 20;
  } else {
    for(uint16_t i=0; i < _led_count; i++) {
      if (!_locked[i])
      setPixelColor(i, _color_sec);
    }
    _mode_delay = 50 + ((1986 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
  }
  show();
}


/*
 * Classic Blink effect. Cycling through the rainbow.
 */
void WS2812FX::mode_blink_rainbow(void) {
  if(_counter_mode_call % 2 == 1) {
    for(uint16_t i=0; i < _led_count; i++) {
      if (!_locked[i])
      setPixelColor(i, color_wheel(_counter_mode_call % 256));
    }
  } else {
    for(uint16_t i=0; i < _led_count; i++) {
      if (!_locked[i])
      setPixelColor(i, _color_sec);
    }
  }
  show();
  _mode_delay = 100 + ((1986 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
}


/*
 * Android loading circle
 */
void WS2812FX::mode_android(void) {
  if (_counter_mode_call == 0) _mode_color = 0; //we use modecolor as bool
  for(uint16_t i=0; i < _led_count; i++) {
    if (!_locked[i])
    setPixelColor(i, _color_sec);
  }

  uint16_t a = _counter_mode_step;
  if (_mode_var1 > ((float)_intensity/255.0)*(float)_led_count)
  {
    _mode_color = 1;
  } else
  {
    if (_mode_var1 < 2) _mode_color = 0;
  }
  
  if (_mode_color == 0)
  {
    if (_counter_mode_call %3 == 1) {a++;}
    else {_mode_var1++;}
  } else
  {
    a++;
    if (_counter_mode_call %3 != 1) _mode_var1--;
  }
  
  if (a >= _led_count) a = 0;

  if (a +_mode_var1  <= _led_count)
  {
    for(int i = a; i < a+_mode_var1; i++) {
      if (!_locked[i])
      setPixelColor(i, _color);
    }
  } else
  {
    for(int i = a; i < _led_count; i++) {
      if (!_locked[i])
      setPixelColor(i, _color);
    }
    for(int i = 0; i < _mode_var1 - (_led_count - a); i++) {
      if (!_locked[i])
      setPixelColor(i, _color);
    }
  }
  _counter_mode_step = a;
  
  show();
  _mode_delay = 3 + ((8 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * _color_sec running on _color.
 */
void WS2812FX::mode_chase_color(void) {
  for(uint16_t i=0; i < _led_count; i++) {
    if (!_locked[i])
    setPixelColor(i, _color_sec);
  }

  uint16_t n = _counter_mode_step;
  uint16_t m = (_counter_mode_step + 1) % _led_count;
  if (!_locked[n])
  setPixelColor(n, _color);
  if (!_locked[m])
  setPixelColor(m, _color);
  show();

  _counter_mode_step = (_counter_mode_step + 1) % _led_count;
  _mode_delay = 10 + ((30 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * color_sec running followed by random color.
 */
void WS2812FX::mode_chase_random(void) {
  if(_counter_mode_step == 0) {
    if (!_locked[_led_count-1])
    setPixelColor(_led_count-1, color_wheel(_mode_color));
    _mode_color = get_random_wheel_index(_mode_color);
  }

  for(uint16_t i=0; i < _counter_mode_step; i++) {
    if (!_locked[i])
    setPixelColor(i, color_wheel(_mode_color));
  }

  uint16_t n = _counter_mode_step;
  uint16_t m = (_counter_mode_step + 1) % _led_count;
  if (!_locked[n])
  setPixelColor(n, _color_sec);
  if (!_locked[m])
  setPixelColor(m, _color_sec);

  show();

  _counter_mode_step = (_counter_mode_step + 1) % _led_count;
  _mode_delay = 10 + ((30 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * color_sec running on rainbow.
 */
void WS2812FX::mode_chase_rainbow(void) {
  for(uint16_t i=0; i < _led_count; i++) {
    if (!_locked[i])
    setPixelColor(i, color_wheel(((i * 256 / _led_count) + (_counter_mode_call % 256)) % 256));
  }

  uint16_t n = _counter_mode_step;
  uint16_t m = (_counter_mode_step + 1) % _led_count;
  if (!_locked[n])
  setPixelColor(n, _color_sec);
  if (!_locked[m])
  setPixelColor(m, _color_sec);
  show();

  _counter_mode_step = (_counter_mode_step + 1) % _led_count;
  _mode_delay = 10 + ((30 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * _color_sec flashes running on _color.
 */
void WS2812FX::mode_chase_flash(void) {
  const static byte flash_count = 4;
  byte flash_step = _counter_mode_call % ((flash_count * 2) + 1);

  for(uint16_t i=0; i < _led_count; i++) {
    if (!_locked[i])
    setPixelColor(i, _color);
  }

  if(flash_step < (flash_count * 2)) {
    if(flash_step % 2 == 0) {
      uint16_t n = _counter_mode_step;
      uint16_t m = (_counter_mode_step + 1) % _led_count;
      if (!_locked[n])
      setPixelColor(n, _color_sec);
      if (!_locked[m])
      setPixelColor(m, _color_sec);
      _mode_delay = 20;
    } else {
      _mode_delay = 30;
    }
  } else {
    _counter_mode_step = (_counter_mode_step + 1) % _led_count;
    _mode_delay = 10 + ((30 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
  }

  show();
}


/*
 * _color_sec flashes running, followed by random color.
 */
void WS2812FX::mode_chase_flash_random(void) {
  const static byte flash_count = 4;
  byte flash_step = _counter_mode_call % ((flash_count * 2) + 1);

  for(uint16_t i=0; i < _counter_mode_step; i++) {
    if (!_locked[i])
    setPixelColor(i, color_wheel(_mode_color));
  }

  if(flash_step < (flash_count * 2)) {
    uint16_t n = _counter_mode_step;
    uint16_t m = (_counter_mode_step + 1) % _led_count;
    if(flash_step % 2 == 0) {
      if (!_locked[n])
      setPixelColor(n, _color_sec);
      if (!_locked[m])
      setPixelColor(m, _color_sec);
      _mode_delay = 20;
    } else {
      if (!_locked[n])
      setPixelColor(n, color_wheel(_mode_color));
      if (!_locked[m])
      setPixelColor(m, 0, 0, 0);
      _mode_delay = 30;
    }
  } else {
    _counter_mode_step = (_counter_mode_step + 1) % _led_count;
    _mode_delay = 1 + ((10 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);

    if(_counter_mode_step == 0) {
      _mode_color = get_random_wheel_index(_mode_color);
    }
  }

  show();
}


/*
 * Rainbow running on _color_sec.
 */
void WS2812FX::mode_chase_rainbow_white(void) {
  for(uint16_t i=0; i < _led_count; i++) {
    if (!_locked[i])
    setPixelColor(i, _color_sec);
  }

  uint16_t n = _counter_mode_step;
  uint16_t m = (_counter_mode_step + 1) % _led_count;
  if (!_locked[n])
  setPixelColor(n, color_wheel(((n * 256 / _led_count) + (_counter_mode_call % 256)) % 256));
  if (!_locked[m])
  setPixelColor(m, color_wheel(((m * 256 / _led_count) + (_counter_mode_call % 256)) % 256));
  show();

  _counter_mode_step = (_counter_mode_step + 1) % _led_count;
  _mode_delay = 10 + ((30 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}

/*
 * Red - Amber - Green - Blue lights running
 */
void WS2812FX::mode_colorful(void) {
  uint32_t cols[]{0x00FF0000,0x00EEBB00,0x0000EE00,0x000077CC,0x00FF0000,0x00EEBB00,0x0000EE00};
  mode_colorful_internal(cols);
}

/*
 * Common function for 4-color-running (Colorful, easter)
 */
void WS2812FX::mode_colorful_internal(uint32_t cols[]) {
  int i = 0;
  for (i; i < _led_count ; i+=4)
  {
    if(!_locked[i])setPixelColor(i, cols[_counter_mode_step]);
    if(!_locked[i+1])setPixelColor(i+1, cols[_counter_mode_step+1]);
    if(!_locked[i+2])setPixelColor(i+2, cols[_counter_mode_step+2]);
    if(!_locked[i+3])setPixelColor(i+3, cols[_counter_mode_step+3]);
  }
  i+=4;
  if(i < _led_count && !_locked[i])
  {
    setPixelColor(i, cols[_counter_mode_step]);
    
    if(i+1 < _led_count && !_locked[i+1])
    {
      setPixelColor(i+1, cols[_counter_mode_step+1]);
      
      if(i+2 < _led_count && !_locked[i+2])
      {
        setPixelColor(i+2, cols[_counter_mode_step+2]);
      }
    }
  }
  
  show();
  if (_speed > SPEED_MIN) _counter_mode_step++; //static if lowest speed
  if (_counter_mode_step >3) _counter_mode_step = 0;
  _mode_delay = 50 + (15 * (uint32_t)(SPEED_MAX - _speed));
}


/*
 * Emulates a traffic light.
 */
void WS2812FX::mode_traffic_light(void) {
  for(uint16_t i=0; i < _led_count; i++) {
    if (!_locked[i])
    setPixelColor(i, _color_sec);
  }
  for (int i = 0; i < _led_count-2 ; i+=3)
  {
    switch (_counter_mode_step)
    {
      case 0: if(!_locked[i])setPixelColor(i, 0x00FF0000); _mode_delay = 150 + (100 * (uint32_t)(SPEED_MAX - _speed));break;
      case 1: if(!_locked[i])setPixelColor(i, 0x00FF0000); _mode_delay = 150 + (20 * (uint32_t)(SPEED_MAX - _speed)); if(!_locked[i+1])setPixelColor(i+1, 0x00EECC00); break;
      case 2: if(!_locked[i+2])setPixelColor(i+2, 0x0000FF00); _mode_delay = 150 + (100 * (uint32_t)(SPEED_MAX - _speed));break;
      case 3: if(!_locked[i+1])setPixelColor(i+1, 0x00EECC00); _mode_delay = 150 + (20 * (uint32_t)(SPEED_MAX - _speed));break;
    }
  }
  show();
  _counter_mode_step++;
  if (_counter_mode_step >3) _counter_mode_step = 0;
}


/*
 * Random color intruduced alternating from start and end of strip.
 */
void WS2812FX::mode_color_sweep_random(void) {
  if(_counter_mode_step == 0 || _counter_mode_step == _led_count) {
    _mode_color = get_random_wheel_index(_mode_color);
  }

  if(_counter_mode_step < _led_count) {
    if (!_locked[_counter_mode_step])
    setPixelColor(_counter_mode_step, color_wheel(_mode_color));
  } else {
    if (!_locked[(_led_count * 2) - _counter_mode_step - 1])
    setPixelColor((_led_count * 2) - _counter_mode_step - 1, color_wheel(_mode_color));
  }
  show();

  _counter_mode_step = (_counter_mode_step + 1) % (_led_count * 2);
  _mode_delay = 5 + ((50 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * Alternating color/2nd pixels running.
 */
void WS2812FX::mode_running_color(void) {
  for(uint16_t i=0; i < _led_count; i++) {
    if((i + _counter_mode_step) % 4 < 2) {
      if (!_locked[i])
      setPixelColor(i, _mode_color);
    } else {
      if (!_locked[i])
      setPixelColor(i, _color_sec);
    }
  }
  show();

  _counter_mode_step = (_counter_mode_step + 1) % 4;
  _mode_delay = 10 + ((30 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * Alternating red/blue pixels running. (RED)
 */
void WS2812FX::mode_running_red_blue(void) {
  for(uint16_t i=0; i < _led_count; i++) {
    if((i + _counter_mode_step) % 4 < 2) {
      if (!_locked[i])
      setPixelColor(i, 255, 0, 0);
    } else {
      if (!_locked[i])
      setPixelColor(i, 0, 0, 255);
    }
  }
  show();

  _counter_mode_step = (_counter_mode_step + 1) % 4;
  _mode_delay = 10 + ((30 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * Random colored pixels running.
 */
void WS2812FX::mode_running_random(void) {
  for(uint16_t i=_led_count-1; i > 0; i--) {
    if (!_locked[i])
    {
      if (!_locked[i-1])
      {
        setPixelColor(i, getPixelColor(i-1));
      } else
      {
        setPixelColor(i, color_wheel(_mode_color));
      }
    }
  }

  if(_counter_mode_step == 0) {
    _mode_color = get_random_wheel_index(_mode_color);
    if (!_locked[0])
    setPixelColor(0, color_wheel(_mode_color));
  }

  show();

  _counter_mode_step = (_counter_mode_step + 1) % 2;

  _mode_delay = 10 + ((30 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * K.I.T.T.
 */
void WS2812FX::mode_larson_scanner(void) {

  for(uint16_t i=0; i < _led_count; i++) {
    uint32_t px_rgb = getPixelColor(i);

    byte px_r = (px_rgb & 0x00FF0000) >> 16;
    byte px_g = (px_rgb & 0x0000FF00) >>  8;
    byte px_b = (px_rgb & 0x000000FF) >>  0;

    // fade out (divide by 2)
    px_r = px_r >> 1;
    px_g = px_g >> 1;
    px_b = px_b >> 1;
    
    if (!_locked[i])
    setPixelColor(i, px_r, px_g, px_b);
  }

  uint16_t pos = 0;

  if(_counter_mode_step < _led_count) {
    pos = _counter_mode_step;
  } else {
    pos = (_led_count * 2) - _counter_mode_step - 2;
  }

  if (!_locked[pos])
  setPixelColor(pos, _color);
  show();

  _counter_mode_step = (_counter_mode_step + 1) % ((_led_count * 2) - 2);
  _mode_delay = 10 + ((10 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * Fireing comets from one end.
 */
void WS2812FX::mode_comet(void) {

  for(uint16_t i=0; i < _led_count; i++) {
    uint32_t px_rgb = getPixelColor(i);

    byte px_r = (px_rgb & 0x00FF0000) >> 16;
    byte px_g = (px_rgb & 0x0000FF00) >>  8;
    byte px_b = (px_rgb & 0x000000FF) >>  0;

    // fade out (divide by 2)
    px_r = px_r >> 1;
    px_g = px_g >> 1;
    px_b = px_b >> 1;

    if (!_locked[i])
    setPixelColor(i, px_r, px_g, px_b);
  }

  if (!_locked[_counter_mode_step])
  setPixelColor(_counter_mode_step, _color);
  show();

  _counter_mode_step = (_counter_mode_step + 1) % _led_count;
  _mode_delay = 10 + ((10 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * Firework sparks.
 */
void WS2812FX::mode_fireworks(void) {
  uint32_t px_rgb = 0;
  byte px_r = 0;
  byte px_g = 0;
  byte px_b = 0;

  for(uint16_t i=0; i < _led_count; i++) {
    px_rgb = getPixelColor(i);

    px_r = (px_rgb & 0x00FF0000) >> 16;
    px_g = (px_rgb & 0x0000FF00) >>  8;
    px_b = (px_rgb & 0x000000FF) >>  0;

    // fade out (divide by 2)
    px_r = px_r >> 1;
    px_g = px_g >> 1;
    px_b = px_b >> 1;

    if (!_locked[i])
    setPixelColor(i, px_r, px_g, px_b);
  }

  // first LED has only one neighbour
  px_r = (((getPixelColor(1) & 0x00FF0000) >> 16) >> 1) + ((getPixelColor(0) & 0x00FF0000) >> 16);
  px_g = (((getPixelColor(1) & 0x0000FF00) >>  8) >> 1) + ((getPixelColor(0) & 0x0000FF00) >>  8);
  px_b = (((getPixelColor(1) & 0x000000FF) >>  0) >> 1) + ((getPixelColor(0) & 0x000000FF) >>  0);
  if (!_locked[0])
  setPixelColor(0, px_r, px_g, px_b);

  // set brightness(i) = ((brightness(i-1)/2 + brightness(i+1)) / 2) + brightness(i)
  for(uint16_t i=1; i < _led_count-1; i++) {
    px_r = ((
            (((getPixelColor(i-1) & 0x00FF0000) >> 16) >> 1) +
            (((getPixelColor(i+1) & 0x00FF0000) >> 16) >> 0) ) >> 1) +
            (((getPixelColor(i  ) & 0x00FF0000) >> 16) >> 0);

    px_g = ((
            (((getPixelColor(i-1) & 0x0000FF00) >> 8) >> 1) +
            (((getPixelColor(i+1) & 0x0000FF00) >> 8) >> 0) ) >> 1) +
            (((getPixelColor(i  ) & 0x0000FF00) >> 8) >> 0);

    px_b = ((
            (((getPixelColor(i-1) & 0x000000FF) >> 0) >> 1) +
            (((getPixelColor(i+1) & 0x000000FF) >> 0) >> 0) ) >> 1) +
            (((getPixelColor(i  ) & 0x000000FF) >> 0) >> 0);

    if (!_locked[i])
    setPixelColor(i, px_r, px_g, px_b);
  }

  // last LED has only one neighbour
  px_r = (((getPixelColor(_led_count-2) & 0x00FF0000) >> 16) >> 2) + ((getPixelColor(_led_count-1) & 0x00FF0000) >> 16);
  px_g = (((getPixelColor(_led_count-2) & 0x0000FF00) >>  8) >> 2) + ((getPixelColor(_led_count-1) & 0x0000FF00) >>  8);
  px_b = (((getPixelColor(_led_count-2) & 0x000000FF) >>  0) >> 2) + ((getPixelColor(_led_count-1) & 0x000000FF) >>  0);
  if (!_locked[_led_count-1])
  setPixelColor(_led_count-1, px_r, px_g, px_b);

  for(uint16_t i=0; i<maxval(1,_led_count/20); i++) {
    if(random(10) == 0) {
      int ran = random(_led_count);
      if (!_locked[ran])
      setPixelColor(random(_led_count), _mode_color);
    }
  }

  show();

  _mode_delay = 20 + ((20 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * Random colored firework sparks.
 */
void WS2812FX::mode_fireworks_random(void) {
  _mode_color = color_wheel(random(256));
  mode_fireworks();
}


/*
 * Alternating red/green pixels running. (RED)
 */
void WS2812FX::mode_merry_christmas(void) {
  for(uint16_t i=0; i < _led_count; i++) {
    if((i + _counter_mode_step) % 4 < 2) {
      if (!_locked[i])
      setPixelColor(i, 255, 0, 0);
    } else {
      if (!_locked[i])
      setPixelColor(i, 0, 255, 0);
    }
  }
  show();

  _counter_mode_step = (_counter_mode_step + 1) % 4;
  _mode_delay = 50 + ((75 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * Random flickering.
 */
void WS2812FX::mode_fire_flicker(void)
{
    byte p_w = (_color & 0xFF000000) >> 24;
    byte p_r = (_color & 0x00FF0000) >> 16;
    byte p_g = (_color & 0x0000FF00) >>  8;
    byte p_b = (_color & 0x000000FF) >>  0;
    byte flicker_val = maxval(p_r,maxval(p_g, maxval(p_b, p_w)))/(((256-_intensity)/16)+1);
    for(uint16_t i=0; i < _led_count; i++)
    {
      int flicker = random(0,flicker_val);
      int r1 = p_r-flicker;
      int g1 = p_g-flicker;
      int b1 = p_b-flicker;
      int w1 = p_w-flicker;
      if(g1<0) g1=0;
      if(r1<0) r1=0;
      if(b1<0) b1=0;
      if(w1<0) w1=0;
      if (!_locked[i])
      setPixelColor(i,r1,g1,b1,w1);
    }
    show();
    _mode_delay = 10 + ((400 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
}

/*
 * Gradient run
 */
void WS2812FX::mode_gradient(void) {
   byte p_w = (_color & 0xFF000000) >> 24;
   byte p_r = (_color & 0x00FF0000) >> 16;
   byte p_g = (_color & 0x0000FF00) >>  8;
   byte p_b = (_color & 0x000000FF) >>  0;
   byte p_w2 = (_color_sec & 0xFF000000) >> 24;
   byte p_r2 = (_color_sec & 0x00FF0000) >> 16;
   byte p_g2 = (_color_sec & 0x0000FF00) >>  8;
   byte p_b2 = (_color_sec & 0x000000FF) >>  0;
   byte nw,nr,ng,nb;
   float per,val; //0.0 = sec 1.0 = pri
   float brd = _intensity/2; if (brd <1.0) brd = 1.0;
   int pp = _counter_mode_step;
   int p1 = pp-_led_count;
   int p2 = pp+_led_count;

   for(uint16_t i=0; i < _led_count; i++)
   {
     if (!_locked[i])
     {
       val = minval(abs(pp-i),minval(abs(p1-i),abs(p2-i)));
       per = val/brd;
       if (per >1.0) per = 1.0;
       nw = p_w+((p_w2 - p_w)*per);
       nr = p_r+((p_r2 - p_r)*per);
       ng = p_g+((p_g2 - p_g)*per);
       nb = p_b+((p_b2 - p_b)*per);
       setPixelColor(i,nr,ng,nb,nw);
     }
   }

   show();
   _counter_mode_step++;
   if (_counter_mode_step >= _led_count) _counter_mode_step = 0;
   if (_speed == 0) _counter_mode_step = _led_count >> 1;
   _mode_delay = 7 + ((25 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}

/*
 * Gradient run with hard transition
 */
void WS2812FX::mode_loading(void) {
   byte p_w = (_color & 0xFF000000) >> 24;
   byte p_r = (_color & 0x00FF0000) >> 16;
   byte p_g = (_color & 0x0000FF00) >>  8;
   byte p_b = (_color & 0x000000FF) >>  0;
   byte p_w2 = (_color_sec & 0xFF000000) >> 24;
   byte p_r2 = (_color_sec & 0x00FF0000) >> 16;
   byte p_g2 = (_color_sec & 0x0000FF00) >>  8;
   byte p_b2 = (_color_sec & 0x000000FF) >>  0;
   byte nw,nr,ng,nb;
   float per,val; //0.0 = sec 1.0 = pri
   float brd = _intensity; if (brd <1.0) brd = 1.0;
   int pp = _counter_mode_step;
   int p1 = pp+_led_count;

   for(uint16_t i=0; i < _led_count; i++)
   {
     if (!_locked[i])
     {
       pp = _counter_mode_step;
       if (i > pp) pp+=_led_count;
       val = abs(pp-i);
       per = val/brd;
       if (per >1.0) per = 1.0;
       nw = p_w+((p_w2 - p_w)*per);
       nr = p_r+((p_r2 - p_r)*per);
       ng = p_g+((p_g2 - p_g)*per);
       nb = p_b+((p_b2 - p_b)*per);
       setPixelColor(i,nr,ng,nb,nw);
     }
   }

   show();
   _counter_mode_step++;
   if (_counter_mode_step >= _led_count) _counter_mode_step = 0;
   if (_speed == 0) _counter_mode_step = _led_count -1;
   _mode_delay = 7 + ((25 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}

/*
 * Lights all LEDs after each other up starting from the outer edges and
 * finishing in the middle. Then turns them in reverse order off. Repeat.
 */
void WS2812FX::mode_dual_color_wipe_in_out(void) {
  int end = _led_count - _counter_mode_step - 1;
  bool odd = (_led_count % 2);
  int mid = odd ? ((_led_count / 2) + 1) : (_led_count / 2);
  if (_counter_mode_step < mid) {
    if (!_locked[_counter_mode_step])
    setPixelColor(_counter_mode_step, _color);
    if (!_locked[end])
    setPixelColor(end, _color);
  }
  else {
    if (odd) {
      // If odd, we need to 'double count' the center LED (once to turn it on,
      // once to turn it off). So trail one behind after the middle LED.
      if (!_locked[_counter_mode_step -1])
      setPixelColor(_counter_mode_step - 1, _color_sec);
      if (!_locked[end+1])
      setPixelColor(end + 1, _color_sec);
    } else {
      if (!_locked[_counter_mode_step])
      setPixelColor(_counter_mode_step, _color_sec);
      if (!_locked[end])
      setPixelColor(end, _color_sec);
    }
  }

  _counter_mode_step++;
  if (odd) {
    if (_counter_mode_step > _led_count) {
      _counter_mode_step = 0;
    }
  } else {
    if (_counter_mode_step >= _led_count) {
      _counter_mode_step = 0;
    }
  }

  show();

  _mode_delay = 5 + ((50 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}

/*
 * Lights all LEDs after each other up starting from the outer edges and
 * finishing in the middle. Then turns them in that order off. Repeat.
 */
void WS2812FX::mode_dual_color_wipe_in_in(void) {
  bool odd = (_led_count % 2);
  int mid = _led_count / 2;
  if (odd) {
    if (_counter_mode_step <= mid) {
      if (!_locked[_counter_mode_step])
      setPixelColor(_counter_mode_step, _color);
      if (!_locked[_led_count - _counter_mode_step - 1])
      setPixelColor(_led_count - _counter_mode_step - 1, _color);
    } else {
      int i = _counter_mode_step - mid;
      if (!_locked[i-1])
      setPixelColor(i - 1, _color_sec);
      if (!_locked[_led_count - i])
      setPixelColor(_led_count - i, _color_sec);
    }
  } else {
    if (_counter_mode_step < mid) {
      if (!_locked[_counter_mode_step])
      setPixelColor(_counter_mode_step, _color);
      if (!_locked[_led_count - _counter_mode_step - 1])
      setPixelColor(_led_count - _counter_mode_step - 1, _color);
    } else {
      int i = _counter_mode_step - mid;
      if (!_locked[i])
      setPixelColor(i, _color_sec);
      if (!_locked[_led_count - i -1])
      setPixelColor(_led_count - i - 1, _color_sec);
    }
  }

  _counter_mode_step++;
  if (odd) {
    if (_counter_mode_step > _led_count) {
      _counter_mode_step = 0;
    }
  } else {
    if (_counter_mode_step >= _led_count) {
      _counter_mode_step = 0;
    }
  }

  show();

  _mode_delay = 5 + ((50 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}

/*
 * Lights all LEDs after each other up starting from the middle and
 * finishing at the edges. Then turns them in that order off. Repeat.
 */
void WS2812FX::mode_dual_color_wipe_out_out(void) {
  int end = _led_count - _counter_mode_step - 1;
  bool odd = (_led_count % 2);
  int mid = _led_count / 2;

  if (odd) {
    if (_counter_mode_step <= mid) {
      if (!_locked[mid + _counter_mode_step])
      setPixelColor(mid + _counter_mode_step, _color);
      if (!_locked[mid - _counter_mode_step])
      setPixelColor(mid - _counter_mode_step, _color);
    } else {
      if (!_locked[_counter_mode_step -1])
      setPixelColor(_counter_mode_step - 1, _color_sec);
      if (!_locked[end +1])
      setPixelColor(end + 1, _color_sec);
    }
  } else {
    if (_counter_mode_step < mid) {
      if (!_locked[mid - _counter_mode_step -1])
      setPixelColor(mid - _counter_mode_step - 1, _color);
      if (!_locked[mid + _counter_mode_step])
      setPixelColor(mid + _counter_mode_step, _color);
    } else {
      if (!_locked[_counter_mode_step])
      setPixelColor(_counter_mode_step, 0);
      if (!_locked[end])
      setPixelColor(end, _color_sec);
    }
  }

  _counter_mode_step++;
  if (odd) {
    if (_counter_mode_step > _led_count) {
      _counter_mode_step = 0;
    }
  } else {
    if (_counter_mode_step >= _led_count) {
      _counter_mode_step = 0;
    }
  }

  show();

  _mode_delay = 5 + ((50 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}

/*
 * Lights all LEDs after each other up starting from the middle and
 * finishing at the edges. Then turns them in reverse order off. Repeat.
 */
void WS2812FX::mode_dual_color_wipe_out_in(void) {
  bool odd = (_led_count % 2);
  int mid = _led_count / 2;

  if (odd) {
    if (_counter_mode_step <= mid) {
      if (!_locked[mid + _counter_mode_step])
      setPixelColor(mid + _counter_mode_step, _color);
      if (!_locked[mid - _counter_mode_step])
      setPixelColor(mid - _counter_mode_step, _color);
    } else {
      int i = _counter_mode_step - mid;
      if (!_locked[i -1])
      setPixelColor(i - 1, _color_sec);
      if (!_locked[_led_count - i])
      setPixelColor(_led_count - i, _color_sec);
    }
  } else {
    if (_counter_mode_step < mid) {
      if (!_locked[mid - _counter_mode_step -1])
      setPixelColor(mid - _counter_mode_step - 1, _color);
      if (!_locked[mid + _counter_mode_step])
      setPixelColor(mid + _counter_mode_step, _color);
    } else {
      int i = _counter_mode_step - mid;
      if (!_locked[i])
      setPixelColor(i, _color_sec);
      if (!_locked[_led_count - i -1])
      setPixelColor(_led_count - i - 1, _color_sec);
    }
  }

  _counter_mode_step++;
  if (odd) {
    if (_counter_mode_step > _led_count) {
      _counter_mode_step = 0;
    }
  } else {
    if (_counter_mode_step >= _led_count) {
      _counter_mode_step = 0;
    }
  }

  show();

  _mode_delay = 5 + ((50 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}

/*
 * Alternating pri/sec/black pixels running.
 */
void WS2812FX::mode_circus_combustus(void) {
  for(uint16_t i=0; i < _led_count; i++) {
    if((i + _counter_mode_step) % 6 < 2) {
      if (!_locked[i])
      setPixelColor(i, _color);
    } else if((i + _color) % 6 < 4){
      if (!_locked[i])
      setPixelColor(i, _color_sec);
    } else {
      if (!_locked[i])
      setPixelColor(i, 0, 0, 0);
    }
  }
  show();

  _counter_mode_step = (_counter_mode_step + 1) % 6;
  _mode_delay = 100 + ((100 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}

void WS2812FX::mode_cc_core()
{
  for (int k = _cc_i1; k <= _cc_i2; k = k + _cc_num1 + _cc_num2)
  {
    for (int i = 0; i < _cc_num1; i++)
    {
      int num = 0;
      num = ((k + i + _counter_ccStep) % _cc_i2) +_cc_i1;
      if (_cc_fs) setPixelColor(num, _color);
      if (_cc_fe) setPixelColor(_cc_i2 - num, _color);
    }
  }
  show();
  _counter_ccStep = (_counter_ccStep + _ccStep) % (_cc_i2 - _cc_i1);
  _mode_delay = 10 + ((250 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
}

void WS2812FX::mode_cc_standard()
{
  for(uint16_t i=0; i < _led_count; i++)
  {
    setPixelColor(i, (_cc_i1 <= i && i <= _cc_i2) ? _color_sec : _color);
  }
  mode_cc_core();
}

void WS2812FX::mode_cc_rainbow()
{
  uint32_t color = color_wheel(_counter_mode_step);
  for(uint16_t i=0; i < _led_count; i++) {
    if (!_locked[i])
    setPixelColor(i, color);
  }
  mode_cc_core();
  _counter_mode_step = (_counter_mode_step + 1) % 256;
}

void WS2812FX::mode_cc_cycle()
{
  for(uint16_t i=0; i < _led_count; i++) {
    if (!_locked[i])
    setPixelColor(i, color_wheel(((i * 256 / _led_count) + _counter_mode_step) % 256));
  }
  mode_cc_core();
  _counter_mode_step = (_counter_mode_step + 1) % 256;
}

void WS2812FX::mode_cc_blink()
{
  for(uint16_t i=0; i < _led_count; i++)
  {
    setPixelColor(i, (_cc_i1 <= i && i <= _cc_i2) ? _color_sec : _color);
  }
  if (_counter_mode_step)
  {
    mode_cc_core();
    _counter_mode_step = 0;
  } else {
    show();
    _counter_mode_step = 1;
    _mode_delay = 10 + ((250 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
  }
}

void WS2812FX::mode_cc_random()
{
  for(uint16_t i=0; i < _led_count; i++) {
    if (!_locked[i])
    setPixelColor(i, color_wheel(random(256)));
  }
  mode_cc_core();
}


//WLED specific methods

void WS2812FX::setIndividual(int i)
{
  if (i >= 0 && i < _led_count)
  {
    setPixelColor(i, _color);
    //show();
    _locked[i] = true;
  }
}

void WS2812FX::setIndividual(int i, uint32_t col)
{
  if (i >= 0 && i < _led_count)
  {
    setPixelColor(i, col);
    //show();
    _locked[i] = true;
  }
}

void WS2812FX::setRange(int i, int i2)
{
  if (i2 >= i)
  {
    for (int x = i; x <= i2; x++)
    {
      if (x >= 0 && x < _led_count)
      {
        setPixelColor(x, _color);
        _locked[x] = true;
      }
    }
  } else
  {
    for (int x = i2; x < _led_count; x++)
    {
      if (x >= 0 && x < _led_count)
      {
        setPixelColor(x, _color);
        _locked[x] = true;
      }
    }
    for (int x = 0; x <= i; x++)
    {
      if (x >= 0 && x < _led_count)
      {
        setPixelColor(x, _color);
        _locked[x] = true;
      }
    }
  }
  //show();
}

void WS2812FX::setRange(int i, int i2, uint32_t col)
{
  if (i2 >= i)
  {
    for (int x = i; x <= i2; x++)
    {
      if (x >= 0 && x < _led_count)
      {
        setPixelColor(x, col);
        _locked[x] = true;
      }
    }
  } else
  {
    for (int x = i2; x < _led_count; x++)
    {
      if (x >= 0 && x < _led_count)
      {
        setPixelColor(x, col);
        _locked[x] = true;
      }
    }
    for (int x = 0; x <= i; x++)
    {
      if (x >= 0 && x < _led_count)
      {
        setPixelColor(x, col);
        _locked[x] = true;
      }
    }
  }
  //show();
}

void WS2812FX::lock(int i)
{
  if (i >= 0 && i < _led_count)
  _locked[i] = true;
}

void WS2812FX::lockRange(int i, int i2)
{
  for (int x = i; x < i2; x++)
  {
    if (x >= 0 && x < _led_count)
      _locked[x] = true;
  }
}

void WS2812FX::lockAll()
{
  for (int x = 0; x < _led_count; x++)
    _locked[x] = true;
}

void WS2812FX::unlock(int i)
{
  if (i >= 0 && i < _led_count)
  _locked[i] = false;
}

void WS2812FX::unlockRange(int i, int i2)
{
  for (int x = i; x < i2; x++)
  {
    if (x >= 0 && x < _led_count)
      _locked[x] = false;
  }
}

void WS2812FX::unlockAll()
{
  for (int x = 0; x < _led_count; x++)
    _locked[x] = false;
}

void WS2812FX::setFastUpdateMode(bool y)
{
  _fastStandard = y;
  if (_mode_index == 0) _mode_delay = 20;
}

void WS2812FX::setReverseMode(bool b)
{
  _reverseMode = b;
}

void WS2812FX::driverModeCronixie(bool b)
{
  _cronixieMode = b;
}

void WS2812FX::setCronixieBacklight(bool b)
{
  _cronixieBacklightEnabled = b;
}

void WS2812FX::setCronixieDigits(byte d[])
{
  for (int i = 0; i<6; i++)
  {
    _cronixieDigits[i] = d[i];
  }
}

double WS2812FX::getPowerEstimate(byte leds, uint32_t c, byte b)
{
  double _mARequired = 100; //ESP power
  double _mul = (double)b/255;
  double _sum = ((c & 0xFF000000) >> 24) + ((c & 0x00FF0000) >> 16) + ((c & 0x0000FF00) >>  8) + ((c & 0x000000FF) >>  0);
  _sum /= (_rgbwMode)? 1024:768;
  double _mAPerLed = 50*(_mul*_sum);
  _mARequired += leds*_mAPerLed;
  return _mARequired;
}

//DISCLAIMER
//This is just a helper function for huge amounts of LEDs.
//It is NOT guaranteed to stay within the safeAmps margin.
//Stay safe with high amperage and have a reasonable safety margin!
//I am NOT to be held liable for burned down garages!
double WS2812FX::getSafePowerMultiplier(double safeMilliAmps, byte leds, uint32_t c, byte b)
{
  double _mARequired = getPowerEstimate(leds,c,b);
  if (_mARequired > safeMilliAmps)
  {
    return safeMilliAmps/_mARequired;
  }
  return 1.0;
}

void WS2812FX::setCCIndex1(byte i1)
{
  if (i1 < _led_count-1) _cc_i1 = i1;
  if (_cc_i2 <= i1) _cc_i2 = i1+1;
  _counter_ccStep = 0;
}

void WS2812FX::setCCIndex2(byte i2)
{
  if (i2 > _cc_i1) _cc_i2 = i2;
  if (_cc_i2 >= _led_count) _cc_i2 = _led_count-1;
  _counter_ccStep = 0;
}

void WS2812FX::setCCStart(byte is)
{
  _cc_is = (is < _cc_i1 || is > _cc_i2) ? _cc_i1 : is;
  _counter_ccStep = 0;
}

void WS2812FX::setCCNum1(byte np)
{
  _cc_num1 = np;
  _counter_ccStep = 0;
}

void WS2812FX::setCCNum2(byte ns)
{
  _cc_num2 = ns;
  _counter_ccStep = 0;
}

void WS2812FX::setCCStep(byte stp)
{
  _ccStep = stp;
  _counter_ccStep = 0;
}

void WS2812FX::setCCFS(bool fs)
{
  _cc_fs = fs;
  _cc_fe = (fs) ? _cc_fe : true;
  _counter_ccStep = 0;
}

void WS2812FX::setCCFE(bool fe)
{
  _cc_fe = fe;
  _cc_fs = (fe) ? _cc_fs : true;
  _counter_ccStep = 0;
}

void WS2812FX::setCustomChase(byte i1, byte i2, byte is, byte np, byte ns, byte stp, bool fs, bool fe)
{
  setCCIndex1(i1);
  setCCIndex2(i2);
  setCCStart(is);
  _cc_num1 = np;
  _cc_num2 = ns;
  _ccStep = stp;
  setCCFS(fs);
  setCCFE(fe);
}

//Added for quick NeoPixelBus compatibility with Adafruit syntax
void WS2812FX::setPixelColorRaw(uint16_t i, byte r, byte g, byte b, byte w)
{
  if (_rgbwMode)
  {
    bus->SetPixelColor(i, RgbwColor(r,g,b,w));
  } else
  {
    bus->SetPixelColor(i, RgbColor(r,g,b));
  }
}

void WS2812FX::setPixelColor(uint16_t i, byte r, byte g, byte b, byte w)
{
  if (_reverseMode) i = _led_count - 1 -i;
  if (!_cronixieMode)
  {
    if (_rgbwMode)
    {
      bus->SetPixelColor(i, RgbwColor(r,g,b,w));
    } else
    {
      bus->SetPixelColor(i, RgbColor(r,g,b));
    }
  } else {
    if(i>6)return;
    byte o = 10*i;
    if (_cronixieBacklightEnabled && _cronixieDigits[i] <11)
    {
      byte rCorr = (int)(((double)((_color_sec>>16) & 0xFF))*_cronixieSecMultiplier);
      byte gCorr = (int)(((double)((_color_sec>>8) & 0xFF))*_cronixieSecMultiplier);
      byte bCorr = (int)(((double)((_color_sec) & 0xFF))*_cronixieSecMultiplier);
      byte wCorr = (int)(((double)((_color_sec>>24) & 0xFF))*_cronixieSecMultiplier);
      for (int j=o; j< o+19; j++)
      {
        setPixelColorRaw(j,rCorr,gCorr,bCorr,wCorr);
      }
    } else
    {
      for (int j=o; j< o+19; j++)
      {
        setPixelColorRaw(j,0,0,0,0);
      }
    }
    switch(_cronixieDigits[i])
    {
      case 0: setPixelColorRaw(o+5,r,g,b,w); break;
      case 1: setPixelColorRaw(o+0,r,g,b,w); break;
      case 2: setPixelColorRaw(o+6,r,g,b,w); break;
      case 3: setPixelColorRaw(o+1,r,g,b,w); break;
      case 4: setPixelColorRaw(o+7,r,g,b,w); break;
      case 5: setPixelColorRaw(o+2,r,g,b,w); break;
      case 6: setPixelColorRaw(o+8,r,g,b,w); break;
      case 7: setPixelColorRaw(o+3,r,g,b,w); break;
      case 8: setPixelColorRaw(o+9,r,g,b,w); break;
      case 9: setPixelColorRaw(o+4,r,g,b,w); break;
      default: break;
    }
  }
}

void WS2812FX::setPixelColor(uint16_t i, byte r, byte g, byte b)
{
  setPixelColor(i,r,g,b,0);
}

void WS2812FX::setPixelColor(uint16_t i, uint32_t c)
{
  setPixelColor(i,(c>>16) & 0xFF,(c>>8) & 0xFF,(c) & 0xFF,(c>>24) & 0xFF);
}

uint32_t WS2812FX::getPixelColor(uint16_t i)
{
  if (_cronixieMode)
  {
    if(i>6)return 0;
    byte o = 10*i;
    switch(_cronixieDigits[i])
    {
      case 0: i=o+5; break;
      case 1: i=o+0; break;
      case 2: i=o+6; break;
      case 3: i=o+1; break;
      case 4: i=o+7; break;
      case 5: i=o+2; break;
      case 6: i=o+8; break;
      case 7: i=o+3; break;
      case 8: i=o+9; break;
      case 9: i=o+4; break;
      default: return 0;
    }
  }
  if (_rgbwMode)
  {
    RgbwColor lColor = bus->GetPixelColorRgbw(i);
    return lColor.W*16777216 + lColor.R*65536 + lColor.G*256 + lColor.B;
  } else {
    RgbColor lColor = bus->GetPixelColor(i);
    return lColor.R*65536 + lColor.G*256 + lColor.B;
  }
}

void WS2812FX::setBrightness(byte b)
{
  _brightness = constrain(b, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
  bus->SetBrightness(_brightness);
  if (_mode_last_call_time + _mode_delay > millis()+50 || b == 0) show(); //only update right away if long time until next refresh
}

void WS2812FX::show()
{
  bus->Show();
}

void WS2812FX::clear()
{
  bus->ClearTo(RgbColor(0));
}

void WS2812FX::begin(bool supportWhite, uint16_t countPixels, uint8_t pin)
{
  if (supportWhite == _rgbwMode && countPixels == _led_count && _locked != NULL) return;
  _rgbwMode = supportWhite;
  _led_count = countPixels;
  _cc_i2 = _led_count -1;
  uint8_t ty = 1;
  if (supportWhite) ty =2;
  bus->Begin((NeoPixelType)ty, countPixels, pin);
  if (_locked != NULL) delete _locked;
  _locked = new bool[countPixels];
}

//For some reason min and max are not declared here

byte WS2812FX::minval (byte v, byte w)
{
  if (w > v) return v;
  return w;
}

byte WS2812FX::maxval (byte v, byte w)
{
  if (w > v) return w;
  return v;
}
