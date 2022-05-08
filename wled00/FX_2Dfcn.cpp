/*
  FX_2Dfcn.cpp contains all 2D utility functions
  
  LICENSE
  The MIT License (MIT)
  Copyright (c) 2022  Blaz Kristan (https://blaz.at/home)
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

  Parts of the code adapted from WLED Sound Reactive
*/
#include "wled.h"
#include "FX.h"
#include "palettes.h"

// setUpMatrix() - constructs ledmap array from matrix of panels with WxH pixels
// this converts physical (possibly irregular) LED arrangement into well defined
// array of logical pixels: fist entry corresponds to left-topmost logical pixel
// followed by horizontal pixels, when matrixWidth logical pixels are added they
// are followed by next row (down) of matrixWidth pixels (and so forth)
// note: matrix may be comprised of multiple panels each with different orientation
// but ledmap takes care of that. ledmap is constructed upon initialization
// so matrix should disable regular ledmap processing
void WS2812FX::setUpMatrix() {
  // erase old ledmap, just in case.
  if (customMappingTable != nullptr) delete[] customMappingTable;
  customMappingTable = nullptr;
  customMappingSize = 0;

  if (isMatrix) {
    matrixWidth  = hPanels * panelW;
    matrixHeight = vPanels * panelH;

    // safety check
    if (matrixWidth * matrixHeight > MAX_LEDS) {
      matrixWidth = getLengthTotal();
      matrixHeight = 1;
      isMatrix = false;
      return;
    }

    customMappingSize  = matrixWidth * matrixHeight;
    customMappingTable = new uint16_t[customMappingSize];

    if (customMappingTable != nullptr) {
      uint16_t startL; // index in custom mapping array (logical strip)
      uint16_t startP; // position of 1st pixel of panel on (virtual) strip
      uint16_t x, y, offset;
      uint8_t h = matrix.vertical ? vPanels : hPanels;
      uint8_t v = matrix.vertical ? hPanels : vPanels;

      for (uint8_t j=0, p=0; j<v; j++) {
        for (uint8_t i=0; i<h; i++, p++) {
          y = (matrix.vertical ? matrix.rightStart : matrix.bottomStart) ? v - j - 1 : j;
          x = (matrix.vertical ? matrix.bottomStart : matrix.rightStart) ? h - i - 1 : i;
          x = matrix.serpentine && j%2 ? h - x - 1 : x;

          startL = (matrix.vertical ? y : x) * panelW + (matrix.vertical ? x : y) * matrixWidth * panelH; // logical index (top-left corner)
          startP = p * panelW * panelH; // physical index (top-left corner)

          for (uint16_t l=0; l<panelH; l++) {
            y = panel[h*j + i].bottomStart ? (panelH - l - 1) : l;
            for (uint16_t k=0; k<panelW; k++) {
              x = panel[h*j + i].rightStart ? (panelW - k - 1) : k;
              if (panel[h*j + i].vertical) {
                y = (panel[h*j + i].serpentine && x%2) ? (panelH - y - 1) : y;
                offset = y + x * panelH;
              } else {
                x = (panel[h*j + i].serpentine && y%2) ? (panelW - x - 1) : x;
                offset = x + y * panelW;
              }
              customMappingTable[startL + k + l * matrixWidth] = startP + offset;
            }
          }
        }
      }
      #ifdef WLED_DEBUG
      DEBUG_PRINT(F("Matrix ledmap:"));
      for (uint16_t i=0; i<customMappingSize; i++) {
        if (!(i%matrixWidth)) DEBUG_PRINTLN();
        DEBUG_PRINTF("%4d,", customMappingTable[i]);
      }
      DEBUG_PRINTLN();
      #endif
    } else {
      // memory allocation error
      matrixWidth = getLengthTotal();
      matrixHeight = 1;
      isMatrix = false;
      return;
    }
  } else { 
    // not a matrix set up
    matrixWidth = getLengthTotal();
    matrixHeight = 1;
  }
}

// XY(x,y,seg) - returns an index of segment pixel in a matrix layout
// index still needs to undergo ledmap processing to represent actual physical pixel
// matrix is always organized by matrixHeight number of matrixWidth pixels from top to bottom, left to right
// so: pixel at XY(5,6) in a 2D segment with [start=10, stop=19, startY=20, stopY=29 : 10x10 pixels]
// corresponds to pixel with logical index of 847 (0 based) if a 2D segment belongs to a 32x32 matrix.
// math: (matrixWidth * (startY + y)) + start + x => (32 * (20+6)) + 10 + 5 = 847
uint16_t IRAM_ATTR WS2812FX::XY(uint16_t x, uint16_t y, uint8_t seg) {
  if (seg == 255) seg = _segment_index;
  x %= _segments[seg].width();  // just in case constrain x (wrap around)
  y %= _segments[seg].height(); // just in case constrain y (wrap around)
  return ((_segments[seg].startY + y) * matrixWidth) + _segments[seg].start + x;
}

void IRAM_ATTR WS2812FX::setPixelColorXY(uint16_t x, uint16_t y, byte r, byte g, byte b, byte w)
{
  if (!isMatrix) return; // not a matrix set-up
  uint8_t segIdx = SEGLEN ? _segment_index : _mainSegment;
  if (SEGLEN && _bri_t < 255) {  
    r = scale8(r, _bri_t);
    g = scale8(g, _bri_t);
    b = scale8(b, _bri_t);
    w = scale8(w, _bri_t);
  }
  uint32_t col = RGBW32(r, g, b, w);

  uint16_t width  = _segments[segIdx].virtualWidth();   // segment width in logical pixels
  uint16_t height = _segments[segIdx].virtualHeight();  // segment height in logical pixels
  if (_segments[segIdx].options & MIRROR     ) width  = (width  + 1) /2;  // divide by 2 if mirror, leave at least a single LED
  if (_segments[segIdx].options & MIRROR_Y_2D) height = (height + 1) /2;  // divide by 2 if mirror, leave at least a single LED
  if (_segments[segIdx].options & TRANSPOSED ) { uint16_t t = x; x = y; y = t; } // swap X & Y if segment transposed

  x *= _segments[segIdx].groupLength();
  y *= _segments[segIdx].groupLength();
  if (x >= width || y >= height) return;  // if pixel would fall out of segment just exit

  for (uint8_t j = 0; j < _segments[segIdx].grouping; j++) {        // groupping vertically
    for (uint8_t g = 0; g < _segments[segIdx].grouping; g++) {      // groupping horizontally
      uint16_t index, xX = (x+g), yY = (y+j);
      if (xX >= width || yY >= height) continue; // we have reached one dimension's end

      if (_segments[segIdx].options & REVERSE     ) xX = width  - xX - 1;
      if (_segments[segIdx].options & REVERSE_Y_2D) yY = height - yY - 1;

      index = XY(xX, yY, segIdx);
      if (index < customMappingSize) index = customMappingTable[index];
      busses.setPixelColor(index, col);

      if (_segments[segIdx].options & MIRROR) { //set the corresponding horizontally mirrored pixel
        index = XY(_segments[segIdx].stop - xX - 1, yY, segIdx);
        if (index < customMappingSize) index = customMappingTable[index];
        busses.setPixelColor(index, col);
      }
      if (_segments[segIdx].options & MIRROR_Y_2D) { //set the corresponding vertically mirrored pixel
        index = XY(xX, _segments[segIdx].stopY - yY - 1, segIdx);
        if (index < customMappingSize) index = customMappingTable[index];
        busses.setPixelColor(index, col);
      }
    }
  }
}


uint32_t WS2812FX::getPixelColorXY(uint16_t x, uint16_t y)
{
  uint8_t segIdx  = _segment_index;
  uint16_t width  = _segments[segIdx].virtualWidth();   // segment width in logical pixels
  uint16_t height = _segments[segIdx].virtualHeight();  // segment height in logical pixels
  if (_segments[segIdx].options & MIRROR     ) width  = (width  + 1) /2;  // divide by 2 if mirror, leave at least a single LED
  if (_segments[segIdx].options & MIRROR_Y_2D) height = (height + 1) /2;  // divide by 2 if mirror, leave at least a single LED
  if (_segments[segIdx].options & TRANSPOSED ) { uint16_t t = x; x = y; y = t; } // swap X & Y if segment transposed

  x *= _segments[segIdx].groupLength();
  y *= _segments[segIdx].groupLength();
  if (x >= width || y >= height) return 0;

  if (_segments[segIdx].options & REVERSE     ) x = width  - x - 1;
  if (_segments[segIdx].options & REVERSE_Y_2D) y = height - y - 1;

  uint16_t index = XY(x, y, segIdx);
  if (index < customMappingSize) index = customMappingTable[index];

  return busses.getPixelColor(index);
}

// blurRows: perform a blur1d on every row of a rectangular matrix
void WS2812FX::blurRows(fract8 blur_amount, CRGB* leds)
{
  uint16_t width  = SEGMENT.virtualWidth();
  uint16_t height = SEGMENT.virtualHeight();
  uint8_t keep = 255 - blur_amount;
  uint8_t seep = blur_amount >> 1;
  CRGB carryover = CRGB::Black;
  for (uint16_t y = 0; y < height; y++) for (uint16_t x = 0; x < width; x++) {
      CRGB cur = leds ? leds[x + y * width] : col_to_crgb(getPixelColorXY(x,y));
      CRGB part = cur;
      part.nscale8(seep);
      cur.nscale8(keep);
      cur += carryover;
      if (x) {
        if (leds) leds[(x-1) + y * width] += part;
        else setPixelColorXY(x-1, y, col_to_crgb(getPixelColorXY(x-1, y)) + part);
      }
      if (leds) leds[x + y * width] = cur;
      else setPixelColorXY(x, y, cur);
      carryover = part;
    }
}

// blurColumns: perform a blur1d on each column of a rectangular matrix
void WS2812FX::blurColumns(fract8 blur_amount, CRGB* leds)
{
  uint16_t width  = SEGMENT.virtualWidth();
  uint16_t height = SEGMENT.virtualHeight();
  // blur columns
  uint8_t keep = 255 - blur_amount;
  uint8_t seep = blur_amount >> 1;
  for ( uint16_t x = 0; x < width; x++) {
    CRGB carryover = CRGB::Black;
    for ( uint16_t y = 0; y < height; y++) {
      CRGB cur = leds ? leds[x + y * width] : col_to_crgb(getPixelColorXY(x,y));
      CRGB part = cur;
      part.nscale8(seep);
      cur.nscale8(keep);
      cur += carryover;
      if (y) {
        if (leds) leds[x + (y-1) * width] += part;
        else setPixelColorXY(x, y-1, col_to_crgb(getPixelColorXY(x,y-1)) + part);
      }
      if (leds) leds[x + y * width] = cur;
      else setPixelColorXY(x, y, cur);
      carryover = part;
    }
  }
}

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

void WS2812FX::blur1d(fract8 blur_amount, CRGB* leds)
{
  blurRows(blur_amount, leds);
}

void WS2812FX::blur2d(fract8 blur_amount, CRGB* leds)
{
  blurRows(blur_amount, leds);
  blurColumns(blur_amount, leds);
}

//ewowi20210628: new functions moved from colorutils: add segment awareness

/*
 * Fills segment with color
 */
void WS2812FX::fill2D(uint32_t c) {
  uint16_t w  = SEGMENT.virtualWidth();
  uint16_t h = SEGMENT.virtualHeight();
  for(uint16_t y = 0; y < h; y++) for (uint16_t x = 0; x < w; x++) {
    setPixelColorXY(x, y, c);
  }
}

void WS2812FX::fill_solid(const struct CRGB& color, CRGB* leds) {
  uint16_t w  = SEGMENT.virtualWidth();
  uint16_t h = SEGMENT.virtualHeight();
  for(uint16_t y = 0; y < h; y++) for (uint16_t x = 0; x < w; x++) {
    if (leds) leds[x + y * w] = color;
    else setPixelColorXY(x, y, color);
  }
}

/*
 * fade out function, higher rate = quicker fade
 * TODO: may be better to use approach of nscale8()
 */
void WS2812FX::fade_out2D(uint8_t rate) {
  uint16_t w  = SEGMENT.virtualWidth();
  uint16_t h = SEGMENT.virtualHeight();
  rate = (255-rate) >> 1;
  float mappedRate = float(rate) +1.1;

  uint32_t color = SEGCOLOR(1); // target color
  int w2 = W(color);
  int r2 = R(color);
  int g2 = G(color);
  int b2 = B(color);

  for(uint16_t y = 0; y < h; y++) for (uint16_t x = 0; x < w; x++) {
    color = getPixelColorXY(x, y);
    int w1 = W(color);
    int r1 = R(color);
    int g1 = G(color);
    int b1 = B(color);

    int wdelta = (w2 - w1) / mappedRate;
    int rdelta = (r2 - r1) / mappedRate;
    int gdelta = (g2 - g1) / mappedRate;
    int bdelta = (b2 - b1) / mappedRate;

    // if fade isn't complete, make sure delta is at least 1 (fixes rounding issues)
    wdelta += (w2 == w1) ? 0 : (w2 > w1) ? 1 : -1;
    rdelta += (r2 == r1) ? 0 : (r2 > r1) ? 1 : -1;
    gdelta += (g2 == g1) ? 0 : (g2 > g1) ? 1 : -1;
    bdelta += (b2 == b1) ? 0 : (b2 > b1) ? 1 : -1;

    setPixelColorXY(x, y, r1 + rdelta, g1 + gdelta, b1 + bdelta, w1 + wdelta);
  }
}

void WS2812FX::fadeToBlackBy(uint8_t fadeBy, CRGB* leds) {
  nscale8(255 - fadeBy, leds);
}

void WS2812FX::nscale8(uint8_t scale, CRGB* leds) {
  uint16_t w  = SEGMENT.virtualWidth();
  uint16_t h = SEGMENT.virtualHeight();
  for(uint16_t y = 0; y < h; y++) for (uint16_t x = 0; x < w; x++) {
    if (leds) leds[x + y * w].nscale8(scale);
    else setPixelColorXY(x, y, col_to_crgb(getPixelColorXY(x, y)).nscale8(scale));
  }
}

void WS2812FX::setPixels(CRGB* leds) {
  uint16_t w = SEGMENT.virtualWidth();
  uint16_t h = SEGMENT.virtualHeight();
  for (uint16_t y = 0; y < h; y++) for (uint16_t x = 0; x < w; x++) setPixelColorXY(x, y, leds[x + y*w]);
}
