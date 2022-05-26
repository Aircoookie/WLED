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

          uint8_t H = panel[h*j + i].vertical ? panelW : panelH;
          uint8_t W = panel[h*j + i].vertical ? panelH : panelW;
          for (uint16_t l=0, q=0; l<H; l++) {
            for (uint16_t k=0; k<W; k++, q++) {
              y = (panel[h*j + i].vertical ? panel[h*j + i].rightStart : panel[h*j + i].bottomStart) ? H - l - 1 : l;
              x = (panel[h*j + i].vertical ? panel[h*j + i].bottomStart : panel[h*j + i].rightStart) ? W - k - 1 : k;
              x = (panel[h*j + i].serpentine && l%2) ? (W - x - 1) : x;
              offset = (panel[h*j + i].vertical ? y : x) + (panel[h*j + i].vertical ? x : y) * matrixWidth;
              customMappingTable[startL + offset] = startP + q;
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

// XY(x,y) - gets pixel index within current segment
uint16_t IRAM_ATTR WS2812FX::XY(uint16_t x, uint16_t y) {
  uint16_t width  = SEGMENT.virtualWidth();   // segment width in logical pixels
  uint16_t height = SEGMENT.virtualHeight();  // segment height in logical pixels
/*
  // it may be unnecessary to perform transpose since pixels should be always addressed using XY() function
  if (SEGMENT.getOption(SEG_OPTION_TRANSPOSED)) {
    uint16_t t;
    // swap X & Y if segment transposed
    t = x; x = y; y = t;
    // swap width & height if segment transposed
    t = width; width = height; height = t;
  }
*/
  return (x%width) + (y%height) * width;
}

// get2DPixelIndex(x,y,seg) - returns an index of segment pixel in a matrix layout
// index still needs to undergo ledmap processing to represent actual physical pixel
// matrix is always organized by matrixHeight number of matrixWidth pixels from top to bottom, left to right
// so: pixel at get2DPixelIndex(5,6) in a 2D segment with [start=10, stop=19, startY=20, stopY=29 : 10x10 pixels]
// corresponds to pixel with logical index of 847 (0 based) if a 2D segment belongs to a 32x32 matrix.
// math: (matrixWidth * (startY + y)) + start + x => (32 * (20+6)) + 10 + 5 = 847
uint16_t IRAM_ATTR WS2812FX::get2DPixelIndex(uint16_t x, uint16_t y, uint8_t seg) {
  if (seg == 255) seg = _segment_index;
  x %= _segments[seg].width();  // just in case constrain x (wrap around)
  y %= _segments[seg].height(); // just in case constrain y (wrap around)
  return ((_segments[seg].startY + y) * matrixWidth) + _segments[seg].start + x;
}

void IRAM_ATTR WS2812FX::setPixelColorXY(uint16_t x, uint16_t y, byte r, byte g, byte b, byte w)
{
  if (!isMatrix) return; // not a matrix set-up

  if (_bri_t < 255) {  
    r = scale8(r, _bri_t);
    g = scale8(g, _bri_t);
    b = scale8(b, _bri_t);
    w = scale8(w, _bri_t);
  }
  uint32_t col = RGBW32(r, g, b, w);

  if (SEGMENT.getOption(SEG_OPTION_TRANSPOSED)) { uint16_t t = x; x = y; y = t; } // swap X & Y if segment transposed

  x *= SEGMENT.groupLength(); // expand to physical pixels
  y *= SEGMENT.groupLength(); // expand to physical pixels
  if (x >= SEGMENT.width() || y >= SEGMENT.height()) return;  // if pixel would fall out of segment just exit

  for (uint8_t j = 0; j < SEGMENT.grouping; j++) {   // groupping vertically
    for (uint8_t g = 0; g < SEGMENT.grouping; g++) { // groupping horizontally
      uint16_t index, xX = (x+g), yY = (y+j);
      if (xX >= SEGMENT.width() || yY >= SEGMENT.height()) continue; // we have reached one dimension's end

      if (SEGMENT.getOption(SEG_OPTION_REVERSED)  ) xX = SEGMENT.width()  - xX - 1;
      if (SEGMENT.getOption(SEG_OPTION_REVERSED_Y)) yY = SEGMENT.height() - yY - 1;

      index = get2DPixelIndex(xX, yY);
      if (index < customMappingSize) index = customMappingTable[index];
      busses.setPixelColor(index, col);

      if (SEGMENT.getOption(SEG_OPTION_MIRROR)) { //set the corresponding horizontally mirrored pixel
        index = get2DPixelIndex(SEGMENT.width() - xX - 1, yY);
        if (index < customMappingSize) index = customMappingTable[index];
        busses.setPixelColor(index, col);
      }
      if (SEGMENT.getOption(SEG_OPTION_MIRROR_Y)) { //set the corresponding vertically mirrored pixel
        index = get2DPixelIndex(xX, SEGMENT.height() - yY - 1);
        if (index < customMappingSize) index = customMappingTable[index];
        busses.setPixelColor(index, col);
      }
    }
  }
}

// not working correctly ATM
uint32_t WS2812FX::getPixelColorXY(uint16_t x, uint16_t y) {
  if (SEGMENT.getOption(SEG_OPTION_TRANSPOSED)) { uint16_t t = x; x = y; y = t; } // swap X & Y if segment transposed

  x *= SEGMENT.groupLength(); // expand to physical pixels
  y *= SEGMENT.groupLength(); // expand to physical pixels
  if (x >= SEGMENT.width() || y >= SEGMENT.height()) return 0;

  if (SEGMENT.getOption(SEG_OPTION_REVERSED)  ) x = SEGMENT.width()  - x - 1;
  if (SEGMENT.getOption(SEG_OPTION_REVERSED_Y)) y = SEGMENT.height() - y - 1;

  uint16_t index = get2DPixelIndex(x, y);
  if (index < customMappingSize) index = customMappingTable[index];

  return busses.getPixelColor(index);
}

/*
 * Blends the specified color with the existing pixel color.
 */
void WS2812FX::blendPixelColorXY(uint16_t x, uint16_t y, uint32_t color, uint8_t blend) {
  setPixelColorXY(x, y, color_blend(getPixelColorXY(x,y), color, blend));
}

// blurRow: perform a blur on a row of a rectangular matrix
void WS2812FX::blurRow(uint16_t row, fract8 blur_amount, CRGB* leds) {
  uint16_t width  = SEGMENT.virtualWidth();
  uint16_t height = SEGMENT.virtualHeight();
  if (row >= height) return;
  // blur one row
  uint8_t keep = 255 - blur_amount;
  uint8_t seep = blur_amount >> 1;
  CRGB carryover = CRGB::Black;
  for (uint16_t x = 0; x < width; x++) {
    CRGB cur = leds ? leds[XY(x,row)] : col_to_crgb(getPixelColorXY(x, row));
    CRGB part = cur;
    part.nscale8(seep);
    cur.nscale8(keep);
    cur += carryover;
    if (x) {
      CRGB prev = (leds ? leds[XY(x-1,row)] : col_to_crgb(getPixelColorXY(x-1, row))) + part;
      if (leds) leds[XY(x-1,row)] = prev;
      else      setPixelColorXY(x-1, row, prev);
    }
    if (leds) leds[XY(x,row)] = cur;
    else      setPixelColorXY(x, row, cur);
    carryover = part;
  }
}

// blurCol: perform a blur on a column of a rectangular matrix
void WS2812FX::blurCol(uint16_t col, fract8 blur_amount, CRGB* leds) {
  uint16_t width  = SEGMENT.virtualWidth();
  uint16_t height = SEGMENT.virtualHeight();
  if (col >= width) return;
  // blur one column
  uint8_t keep = 255 - blur_amount;
  uint8_t seep = blur_amount >> 1;
  CRGB carryover = CRGB::Black;
  for (uint16_t i = 0; i < height; i++) {
    CRGB cur = leds ? leds[XY(col,i)] : col_to_crgb(getPixelColorXY(col, i));
    CRGB part = cur;
    part.nscale8(seep);
    cur.nscale8(keep);
    cur += carryover;
    if (i) {
      CRGB prev = (leds ? leds[XY(col,i-1)] : col_to_crgb(getPixelColorXY(col, i-1))) + part;
      if (leds) leds[XY(col,i-1)] = prev;
      else      setPixelColorXY(col, i-1, prev);
    }
    if (leds) leds[XY(col,i)] = cur;
    else      setPixelColorXY(col, i, cur);
    carryover = part;
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

void WS2812FX::blur1d(CRGB* leds, fract8 blur_amount) {
  uint16_t height = SEGMENT.virtualHeight();
  for (uint16_t y = 0; y < height; y++) blurRow(y, blur_amount, leds);
}

void WS2812FX::blur2d(CRGB* leds, fract8 blur_amount) {
  uint16_t width  = SEGMENT.virtualWidth();  // same as SEGLEN
  uint16_t height = SEGMENT.virtualHeight();
  for (uint16_t i = 0; i < height; i++) blurRow(i, blur_amount, leds); // blur all rows
  for (uint16_t k = 0; k < width; k++)  blurCol(k, blur_amount, leds); // blur all columns
}

void WS2812FX::moveX(CRGB *leds, int8_t delta) {
  uint16_t width  = SEGMENT.virtualWidth();  // same as SEGLEN
  uint16_t height = SEGMENT.virtualHeight();
  if (delta) {
    if (delta > 0) {
      for (uint8_t y = 0; y < height; y++) {
        for (uint8_t x = 0; x < width; x++) {
          leds[XY(x, y)] = leds[XY(x + delta, y)];
        }
      }
    } else {
      for (uint8_t y = 0; y < height; y++) {
        for (uint8_t x = width - 1; x > 0; x--) {
          leds[XY(x, y)] = leds[XY(x + delta, y)];
        }
      }
    }
  }
}

void WS2812FX::moveY(CRGB *leds, int8_t delta) {
  uint16_t width  = SEGMENT.virtualWidth();  // same as SEGLEN
  uint16_t height = SEGMENT.virtualHeight();
  if (delta) {
    if (delta > 0) {
      for (uint8_t x = 0; x < height; x++) {
        for (uint8_t y = 0; y < width; y++) {
          leds[XY(x, y)] = leds[XY(x, y + delta)];
        }
      }
    } else {
      for (uint8_t x = 0; x < height; x++) {
        for (uint8_t y = width - 1; y > 0; y--) {
          leds[XY(x, y)] = leds[XY(x, y + delta)];
        }
      }
    }
  }
}


//ewowi20210628: new functions moved from colorutils: add segment awareness

void WS2812FX::fill_solid(CRGB* leds, CRGB color) {
  uint16_t w  = SEGMENT.virtualWidth();
  uint16_t h = SEGMENT.virtualHeight();
  for(uint16_t y = 0; y < h; y++) for (uint16_t x = 0; x < w; x++) {
    if (leds) leds[XY(x,y)] = color;
    else setPixelColorXY(x, y, color);
  }
}

// by stepko, taken from https://editor.soulmatelights.com/gallery/573-blobs
void WS2812FX::fill_circle(CRGB* leds, uint16_t cx, uint16_t cy, uint8_t radius, CRGB col) {
  for (int16_t y = -radius; y <= radius; y++) {
    for (int16_t x = -radius; x <= radius; x++) {
      if (x * x + y * y <= radius * radius &&
          int16_t(cx)+x>=0 && int16_t(cy)+y>=0 &&
          int16_t(cx)+x<SEGMENT.virtualWidth() && int16_t(cy)+y<SEGMENT.virtualHeight())
        leds[XY(cx + x, cy + y)] += col;
    }
  }
}

void WS2812FX::fadeToBlackBy(CRGB* leds, uint8_t fadeBy) {
  nscale8(leds, 255 - fadeBy);
}

void WS2812FX::nscale8(CRGB* leds, uint8_t scale) {
  uint16_t w  = SEGMENT.virtualWidth();
  uint16_t h = SEGMENT.virtualHeight();
  for(uint16_t y = 0; y < h; y++) for (uint16_t x = 0; x < w; x++) {
    if (leds) leds[XY(x,y)].nscale8(scale);
    else setPixelColorXY(x, y, col_to_crgb(getPixelColorXY(x, y)).nscale8(scale));
  }
}

void WS2812FX::setPixels(CRGB* leds) {
  uint16_t w = SEGMENT.virtualWidth();
  uint16_t h = SEGMENT.virtualHeight();
  for (uint16_t y = 0; y < h; y++) for (uint16_t x = 0; x < w; x++) setPixelColorXY(x, y, leds[XY(x,y)]);
}

//line function
void WS2812FX::drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, CRGB c, CRGB *leds) {
  int16_t dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int16_t dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
  int16_t err = (dx>dy ? dx : -dy)/2, e2;
  for (;;) {
    if (leds == nullptr) setPixelColorXY(x0,y0,c);
    else                 leds[XY(x0,y0)] = c;
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
}

// font curtesy of https://github.com/idispatch/raster-fonts
static unsigned char console_font_6x8[] PROGMEM = {

    /*
     * code=0, hex=0x00, ascii="^@"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=1, hex=0x01, ascii="^A"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x6C,  /* 011011 */
    0x44,  /* 010001 */
    0x54,  /* 010101 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=2, hex=0x02, ascii="^B"
     */
    0x38,  /* 001110 */
    0x7C,  /* 011111 */
    0x54,  /* 010101 */
    0x7C,  /* 011111 */
    0x44,  /* 010001 */
    0x7C,  /* 011111 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=3, hex=0x03, ascii="^C"
     */
    0x00,  /* 000000 */
    0x28,  /* 001010 */
    0x7C,  /* 011111 */
    0x7C,  /* 011111 */
    0x7C,  /* 011111 */
    0x38,  /* 001110 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=4, hex=0x04, ascii="^D"
     */
    0x00,  /* 000000 */
    0x10,  /* 000100 */
    0x38,  /* 001110 */
    0x7C,  /* 011111 */
    0x7C,  /* 011111 */
    0x38,  /* 001110 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=5, hex=0x05, ascii="^E"
     */
    0x10,  /* 000100 */
    0x38,  /* 001110 */
    0x38,  /* 001110 */
    0x10,  /* 000100 */
    0x7C,  /* 011111 */
    0x7C,  /* 011111 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=6, hex=0x06, ascii="^F"
     */
    0x00,  /* 000000 */
    0x10,  /* 000100 */
    0x38,  /* 001110 */
    0x7C,  /* 011111 */
    0x7C,  /* 011111 */
    0x10,  /* 000100 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=7, hex=0x07, ascii="^G"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x30,  /* 001100 */
    0x30,  /* 001100 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=8, hex=0x08, ascii="^H"
     */
    0xFC,  /* 111111 */
    0xFC,  /* 111111 */
    0xFC,  /* 111111 */
    0xCC,  /* 110011 */
    0xCC,  /* 110011 */
    0xFC,  /* 111111 */
    0xFC,  /* 111111 */
    0xFC,  /* 111111 */

    /*
     * code=9, hex=0x09, ascii="^I"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x78,  /* 011110 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x78,  /* 011110 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=10, hex=0x0A, ascii="^J"
     */
    0xFC,  /* 111111 */
    0xFC,  /* 111111 */
    0x84,  /* 100001 */
    0xB4,  /* 101101 */
    0xB4,  /* 101101 */
    0x84,  /* 100001 */
    0xFC,  /* 111111 */
    0xFC,  /* 111111 */

    /*
     * code=11, hex=0x0B, ascii="^K"
     */
    0x00,  /* 000000 */
    0x1C,  /* 000111 */
    0x0C,  /* 000011 */
    0x34,  /* 001101 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x30,  /* 001100 */
    0x00,  /* 000000 */

    /*
     * code=12, hex=0x0C, ascii="^L"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x10,  /* 000100 */
    0x38,  /* 001110 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=13, hex=0x0D, ascii="^M"
     */
    0x10,  /* 000100 */
    0x18,  /* 000110 */
    0x14,  /* 000101 */
    0x10,  /* 000100 */
    0x30,  /* 001100 */
    0x70,  /* 011100 */
    0x60,  /* 011000 */
    0x00,  /* 000000 */

    /*
     * code=14, hex=0x0E, ascii="^N"
     */
    0x0C,  /* 000011 */
    0x34,  /* 001101 */
    0x2C,  /* 001011 */
    0x34,  /* 001101 */
    0x2C,  /* 001011 */
    0x6C,  /* 011011 */
    0x60,  /* 011000 */
    0x00,  /* 000000 */

    /*
     * code=15, hex=0x0F, ascii="^O"
     */
    0x00,  /* 000000 */
    0x54,  /* 010101 */
    0x38,  /* 001110 */
    0x6C,  /* 011011 */
    0x38,  /* 001110 */
    0x54,  /* 010101 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=16, hex=0x10, ascii="^P"
     */
    0x20,  /* 001000 */
    0x30,  /* 001100 */
    0x38,  /* 001110 */
    0x3C,  /* 001111 */
    0x38,  /* 001110 */
    0x30,  /* 001100 */
    0x20,  /* 001000 */
    0x00,  /* 000000 */

    /*
     * code=17, hex=0x11, ascii="^Q"
     */
    0x08,  /* 000010 */
    0x18,  /* 000110 */
    0x38,  /* 001110 */
    0x78,  /* 011110 */
    0x38,  /* 001110 */
    0x18,  /* 000110 */
    0x08,  /* 000010 */
    0x00,  /* 000000 */

    /*
     * code=18, hex=0x12, ascii="^R"
     */
    0x10,  /* 000100 */
    0x38,  /* 001110 */
    0x7C,  /* 011111 */
    0x10,  /* 000100 */
    0x7C,  /* 011111 */
    0x38,  /* 001110 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=19, hex=0x13, ascii="^S"
     */
    0x28,  /* 001010 */
    0x28,  /* 001010 */
    0x28,  /* 001010 */
    0x28,  /* 001010 */
    0x28,  /* 001010 */
    0x00,  /* 000000 */
    0x28,  /* 001010 */
    0x00,  /* 000000 */

    /*
     * code=20, hex=0x14, ascii="^T"
     */
    0x3C,  /* 001111 */
    0x54,  /* 010101 */
    0x54,  /* 010101 */
    0x34,  /* 001101 */
    0x14,  /* 000101 */
    0x14,  /* 000101 */
    0x14,  /* 000101 */
    0x00,  /* 000000 */

    /*
     * code=21, hex=0x15, ascii="^U"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x30,  /* 001100 */
    0x28,  /* 001010 */
    0x18,  /* 000110 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=22, hex=0x16, ascii="^V"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x78,  /* 011110 */
    0x78,  /* 011110 */
    0x00,  /* 000000 */

    /*
     * code=23, hex=0x17, ascii="^W"
     */
    0x10,  /* 000100 */
    0x38,  /* 001110 */
    0x7C,  /* 011111 */
    0x10,  /* 000100 */
    0x7C,  /* 011111 */
    0x38,  /* 001110 */
    0x10,  /* 000100 */
    0x38,  /* 001110 */

    /*
     * code=24, hex=0x18, ascii="^X"
     */
    0x10,  /* 000100 */
    0x38,  /* 001110 */
    0x7C,  /* 011111 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=25, hex=0x19, ascii="^Y"
     */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x7C,  /* 011111 */
    0x38,  /* 001110 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=26, hex=0x1A, ascii="^Z"
     */
    0x00,  /* 000000 */
    0x10,  /* 000100 */
    0x18,  /* 000110 */
    0x7C,  /* 011111 */
    0x18,  /* 000110 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=27, hex=0x1B, ascii="^["
     */
    0x00,  /* 000000 */
    0x10,  /* 000100 */
    0x30,  /* 001100 */
    0x7C,  /* 011111 */
    0x30,  /* 001100 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=28, hex=0x1C, ascii="^\"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x7C,  /* 011111 */
    0x00,  /* 000000 */

    /*
     * code=29, hex=0x1D, ascii="^]"
     */
    0x00,  /* 000000 */
    0x28,  /* 001010 */
    0x28,  /* 001010 */
    0x7C,  /* 011111 */
    0x28,  /* 001010 */
    0x28,  /* 001010 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=30, hex=0x1E, ascii="^^"
     */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x38,  /* 001110 */
    0x38,  /* 001110 */
    0x7C,  /* 011111 */
    0x7C,  /* 011111 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=31, hex=0x1F, ascii="^_"
     */
    0x7C,  /* 011111 */
    0x7C,  /* 011111 */
    0x38,  /* 001110 */
    0x38,  /* 001110 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=32, hex=0x20, ascii=" "
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=33, hex=0x21, ascii="!"
     */
    0x10,  /* 000100 */
    0x38,  /* 001110 */
    0x38,  /* 001110 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=34, hex=0x22, ascii="""
     */
    0x6C,  /* 011011 */
    0x6C,  /* 011011 */
    0x48,  /* 010010 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=35, hex=0x23, ascii="#"
     */
    0x00,  /* 000000 */
    0x28,  /* 001010 */
    0x7C,  /* 011111 */
    0x28,  /* 001010 */
    0x28,  /* 001010 */
    0x7C,  /* 011111 */
    0x28,  /* 001010 */
    0x00,  /* 000000 */

    /*
     * code=36, hex=0x24, ascii="$"
     */
    0x20,  /* 001000 */
    0x38,  /* 001110 */
    0x40,  /* 010000 */
    0x30,  /* 001100 */
    0x08,  /* 000010 */
    0x70,  /* 011100 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=37, hex=0x25, ascii="%"
     */
    0x64,  /* 011001 */
    0x64,  /* 011001 */
    0x08,  /* 000010 */
    0x10,  /* 000100 */
    0x20,  /* 001000 */
    0x4C,  /* 010011 */
    0x4C,  /* 010011 */
    0x00,  /* 000000 */

    /*
     * code=38, hex=0x26, ascii="&"
     */
    0x20,  /* 001000 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x20,  /* 001000 */
    0x54,  /* 010101 */
    0x48,  /* 010010 */
    0x34,  /* 001101 */
    0x00,  /* 000000 */

    /*
     * code=39, hex=0x27, ascii="'"
     */
    0x30,  /* 001100 */
    0x30,  /* 001100 */
    0x20,  /* 001000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=40, hex=0x28, ascii="("
     */
    0x10,  /* 000100 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=41, hex=0x29, ascii=")"
     */
    0x20,  /* 001000 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x20,  /* 001000 */
    0x00,  /* 000000 */

    /*
     * code=42, hex=0x2A, ascii="*"
     */
    0x00,  /* 000000 */
    0x28,  /* 001010 */
    0x38,  /* 001110 */
    0x7C,  /* 011111 */
    0x38,  /* 001110 */
    0x28,  /* 001010 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=43, hex=0x2B, ascii="+"
     */
    0x00,  /* 000000 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x7C,  /* 011111 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=44, hex=0x2C, ascii=","
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x30,  /* 001100 */
    0x30,  /* 001100 */
    0x20,  /* 001000 */

    /*
     * code=45, hex=0x2D, ascii="-"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x7C,  /* 011111 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=46, hex=0x2E, ascii="."
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x30,  /* 001100 */
    0x30,  /* 001100 */
    0x00,  /* 000000 */

    /*
     * code=47, hex=0x2F, ascii="/"
     */
    0x00,  /* 000000 */
    0x04,  /* 000001 */
    0x08,  /* 000010 */
    0x10,  /* 000100 */
    0x20,  /* 001000 */
    0x40,  /* 010000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=48, hex=0x30, ascii="0"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x4C,  /* 010011 */
    0x54,  /* 010101 */
    0x64,  /* 011001 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=49, hex=0x31, ascii="1"
     */
    0x10,  /* 000100 */
    0x30,  /* 001100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=50, hex=0x32, ascii="2"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x04,  /* 000001 */
    0x18,  /* 000110 */
    0x20,  /* 001000 */
    0x40,  /* 010000 */
    0x7C,  /* 011111 */
    0x00,  /* 000000 */

    /*
     * code=51, hex=0x33, ascii="3"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x04,  /* 000001 */
    0x38,  /* 001110 */
    0x04,  /* 000001 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=52, hex=0x34, ascii="4"
     */
    0x08,  /* 000010 */
    0x18,  /* 000110 */
    0x28,  /* 001010 */
    0x48,  /* 010010 */
    0x7C,  /* 011111 */
    0x08,  /* 000010 */
    0x08,  /* 000010 */
    0x00,  /* 000000 */

    /*
     * code=53, hex=0x35, ascii="5"
     */
    0x7C,  /* 011111 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x78,  /* 011110 */
    0x04,  /* 000001 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=54, hex=0x36, ascii="6"
     */
    0x18,  /* 000110 */
    0x20,  /* 001000 */
    0x40,  /* 010000 */
    0x78,  /* 011110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=55, hex=0x37, ascii="7"
     */
    0x7C,  /* 011111 */
    0x04,  /* 000001 */
    0x08,  /* 000010 */
    0x10,  /* 000100 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x00,  /* 000000 */

    /*
     * code=56, hex=0x38, ascii="8"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=57, hex=0x39, ascii="9"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x3C,  /* 001111 */
    0x04,  /* 000001 */
    0x08,  /* 000010 */
    0x30,  /* 001100 */
    0x00,  /* 000000 */

    /*
     * code=58, hex=0x3A, ascii=":"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x30,  /* 001100 */
    0x30,  /* 001100 */
    0x00,  /* 000000 */
    0x30,  /* 001100 */
    0x30,  /* 001100 */
    0x00,  /* 000000 */

    /*
     * code=59, hex=0x3B, ascii=";"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x30,  /* 001100 */
    0x30,  /* 001100 */
    0x00,  /* 000000 */
    0x30,  /* 001100 */
    0x30,  /* 001100 */
    0x20,  /* 001000 */

    /*
     * code=60, hex=0x3C, ascii="<"
     */
    0x08,  /* 000010 */
    0x10,  /* 000100 */
    0x20,  /* 001000 */
    0x40,  /* 010000 */
    0x20,  /* 001000 */
    0x10,  /* 000100 */
    0x08,  /* 000010 */
    0x00,  /* 000000 */

    /*
     * code=61, hex=0x3D, ascii="="
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x7C,  /* 011111 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x7C,  /* 011111 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=62, hex=0x3E, ascii=">"
     */
    0x20,  /* 001000 */
    0x10,  /* 000100 */
    0x08,  /* 000010 */
    0x04,  /* 000001 */
    0x08,  /* 000010 */
    0x10,  /* 000100 */
    0x20,  /* 001000 */
    0x00,  /* 000000 */

    /*
     * code=63, hex=0x3F, ascii="?"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x04,  /* 000001 */
    0x18,  /* 000110 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=64, hex=0x40, ascii="@"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x5C,  /* 010111 */
    0x54,  /* 010101 */
    0x5C,  /* 010111 */
    0x40,  /* 010000 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=65, hex=0x41, ascii="A"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x7C,  /* 011111 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x00,  /* 000000 */

    /*
     * code=66, hex=0x42, ascii="B"
     */
    0x78,  /* 011110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x78,  /* 011110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x78,  /* 011110 */
    0x00,  /* 000000 */

    /*
     * code=67, hex=0x43, ascii="C"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=68, hex=0x44, ascii="D"
     */
    0x78,  /* 011110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x78,  /* 011110 */
    0x00,  /* 000000 */

    /*
     * code=69, hex=0x45, ascii="E"
     */
    0x7C,  /* 011111 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x78,  /* 011110 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x7C,  /* 011111 */
    0x00,  /* 000000 */

    /*
     * code=70, hex=0x46, ascii="F"
     */
    0x7C,  /* 011111 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x78,  /* 011110 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x00,  /* 000000 */

    /*
     * code=71, hex=0x47, ascii="G"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x40,  /* 010000 */
    0x5C,  /* 010111 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x3C,  /* 001111 */
    0x00,  /* 000000 */

    /*
     * code=72, hex=0x48, ascii="H"
     */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x7C,  /* 011111 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x00,  /* 000000 */

    /*
     * code=73, hex=0x49, ascii="I"
     */
    0x38,  /* 001110 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=74, hex=0x4A, ascii="J"
     */
    0x04,  /* 000001 */
    0x04,  /* 000001 */
    0x04,  /* 000001 */
    0x04,  /* 000001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=75, hex=0x4B, ascii="K"
     */
    0x44,  /* 010001 */
    0x48,  /* 010010 */
    0x50,  /* 010100 */
    0x60,  /* 011000 */
    0x50,  /* 010100 */
    0x48,  /* 010010 */
    0x44,  /* 010001 */
    0x00,  /* 000000 */

    /*
     * code=76, hex=0x4C, ascii="L"
     */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x7C,  /* 011111 */
    0x00,  /* 000000 */

    /*
     * code=77, hex=0x4D, ascii="M"
     */
    0x44,  /* 010001 */
    0x6C,  /* 011011 */
    0x54,  /* 010101 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x00,  /* 000000 */

    /*
     * code=78, hex=0x4E, ascii="N"
     */
    0x44,  /* 010001 */
    0x64,  /* 011001 */
    0x54,  /* 010101 */
    0x4C,  /* 010011 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x00,  /* 000000 */

    /*
     * code=79, hex=0x4F, ascii="O"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=80, hex=0x50, ascii="P"
     */
    0x78,  /* 011110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x78,  /* 011110 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x00,  /* 000000 */

    /*
     * code=81, hex=0x51, ascii="Q"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x54,  /* 010101 */
    0x48,  /* 010010 */
    0x34,  /* 001101 */
    0x00,  /* 000000 */

    /*
     * code=82, hex=0x52, ascii="R"
     */
    0x78,  /* 011110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x78,  /* 011110 */
    0x48,  /* 010010 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x00,  /* 000000 */

    /*
     * code=83, hex=0x53, ascii="S"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x40,  /* 010000 */
    0x38,  /* 001110 */
    0x04,  /* 000001 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=84, hex=0x54, ascii="T"
     */
    0x7C,  /* 011111 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=85, hex=0x55, ascii="U"
     */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=86, hex=0x56, ascii="V"
     */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x28,  /* 001010 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=87, hex=0x57, ascii="W"
     */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x54,  /* 010101 */
    0x54,  /* 010101 */
    0x54,  /* 010101 */
    0x54,  /* 010101 */
    0x28,  /* 001010 */
    0x00,  /* 000000 */

    /*
     * code=88, hex=0x58, ascii="X"
     */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x28,  /* 001010 */
    0x10,  /* 000100 */
    0x28,  /* 001010 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x00,  /* 000000 */

    /*
     * code=89, hex=0x59, ascii="Y"
     */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x28,  /* 001010 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=90, hex=0x5A, ascii="Z"
     */
    0x78,  /* 011110 */
    0x08,  /* 000010 */
    0x10,  /* 000100 */
    0x20,  /* 001000 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x78,  /* 011110 */
    0x00,  /* 000000 */

    /*
     * code=91, hex=0x5B, ascii="["
     */
    0x38,  /* 001110 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=92, hex=0x5C, ascii="\"
     */
    0x00,  /* 000000 */
    0x40,  /* 010000 */
    0x20,  /* 001000 */
    0x10,  /* 000100 */
    0x08,  /* 000010 */
    0x04,  /* 000001 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=93, hex=0x5D, ascii="]"
     */
    0x38,  /* 001110 */
    0x08,  /* 000010 */
    0x08,  /* 000010 */
    0x08,  /* 000010 */
    0x08,  /* 000010 */
    0x08,  /* 000010 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=94, hex=0x5E, ascii="^"
     */
    0x10,  /* 000100 */
    0x28,  /* 001010 */
    0x44,  /* 010001 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=95, hex=0x5F, ascii="_"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0xFC,  /* 111111 */

    /*
     * code=96, hex=0x60, ascii="`"
     */
    0x30,  /* 001100 */
    0x30,  /* 001100 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=97, hex=0x61, ascii="a"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x38,  /* 001110 */
    0x04,  /* 000001 */
    0x3C,  /* 001111 */
    0x44,  /* 010001 */
    0x3C,  /* 001111 */
    0x00,  /* 000000 */

    /*
     * code=98, hex=0x62, ascii="b"
     */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x78,  /* 011110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x78,  /* 011110 */
    0x00,  /* 000000 */

    /*
     * code=99, hex=0x63, ascii="c"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x40,  /* 010000 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=100, hex=0x64, ascii="d"
     */
    0x04,  /* 000001 */
    0x04,  /* 000001 */
    0x3C,  /* 001111 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x3C,  /* 001111 */
    0x00,  /* 000000 */

    /*
     * code=101, hex=0x65, ascii="e"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x78,  /* 011110 */
    0x40,  /* 010000 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=102, hex=0x66, ascii="f"
     */
    0x18,  /* 000110 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x78,  /* 011110 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x00,  /* 000000 */

    /*
     * code=103, hex=0x67, ascii="g"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x3C,  /* 001111 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x3C,  /* 001111 */
    0x04,  /* 000001 */
    0x38,  /* 001110 */

    /*
     * code=104, hex=0x68, ascii="h"
     */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x70,  /* 011100 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x00,  /* 000000 */

    /*
     * code=105, hex=0x69, ascii="i"
     */
    0x10,  /* 000100 */
    0x00,  /* 000000 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x18,  /* 000110 */
    0x00,  /* 000000 */

    /*
     * code=106, hex=0x6A, ascii="j"
     */
    0x08,  /* 000010 */
    0x00,  /* 000000 */
    0x18,  /* 000110 */
    0x08,  /* 000010 */
    0x08,  /* 000010 */
    0x08,  /* 000010 */
    0x48,  /* 010010 */
    0x30,  /* 001100 */

    /*
     * code=107, hex=0x6B, ascii="k"
     */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x48,  /* 010010 */
    0x50,  /* 010100 */
    0x60,  /* 011000 */
    0x50,  /* 010100 */
    0x48,  /* 010010 */
    0x00,  /* 000000 */

    /*
     * code=108, hex=0x6C, ascii="l"
     */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x18,  /* 000110 */
    0x00,  /* 000000 */

    /*
     * code=109, hex=0x6D, ascii="m"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x68,  /* 011010 */
    0x54,  /* 010101 */
    0x54,  /* 010101 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x00,  /* 000000 */

    /*
     * code=110, hex=0x6E, ascii="n"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x70,  /* 011100 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x00,  /* 000000 */

    /*
     * code=111, hex=0x6F, ascii="o"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=112, hex=0x70, ascii="p"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x78,  /* 011110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x78,  /* 011110 */
    0x40,  /* 010000 */

    /*
     * code=113, hex=0x71, ascii="q"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x3C,  /* 001111 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x3C,  /* 001111 */
    0x04,  /* 000001 */

    /*
     * code=114, hex=0x72, ascii="r"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x58,  /* 010110 */
    0x24,  /* 001001 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x70,  /* 011100 */
    0x00,  /* 000000 */

    /*
     * code=115, hex=0x73, ascii="s"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x38,  /* 001110 */
    0x40,  /* 010000 */
    0x38,  /* 001110 */
    0x04,  /* 000001 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=116, hex=0x74, ascii="t"
     */
    0x00,  /* 000000 */
    0x20,  /* 001000 */
    0x78,  /* 011110 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x28,  /* 001010 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=117, hex=0x75, ascii="u"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x58,  /* 010110 */
    0x28,  /* 001010 */
    0x00,  /* 000000 */

    /*
     * code=118, hex=0x76, ascii="v"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x28,  /* 001010 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=119, hex=0x77, ascii="w"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x54,  /* 010101 */
    0x7C,  /* 011111 */
    0x28,  /* 001010 */
    0x00,  /* 000000 */

    /*
     * code=120, hex=0x78, ascii="x"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x30,  /* 001100 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x00,  /* 000000 */

    /*
     * code=121, hex=0x79, ascii="y"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x38,  /* 001110 */
    0x10,  /* 000100 */
    0x60,  /* 011000 */

    /*
     * code=122, hex=0x7A, ascii="z"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x78,  /* 011110 */
    0x08,  /* 000010 */
    0x30,  /* 001100 */
    0x40,  /* 010000 */
    0x78,  /* 011110 */
    0x00,  /* 000000 */

    /*
     * code=123, hex=0x7B, ascii="{"
     */
    0x18,  /* 000110 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x60,  /* 011000 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x18,  /* 000110 */
    0x00,  /* 000000 */

    /*
     * code=124, hex=0x7C, ascii="|"
     */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=125, hex=0x7D, ascii="}"
     */
    0x30,  /* 001100 */
    0x08,  /* 000010 */
    0x08,  /* 000010 */
    0x0C,  /* 000011 */
    0x08,  /* 000010 */
    0x08,  /* 000010 */
    0x30,  /* 001100 */
    0x00,  /* 000000 */

    /*
     * code=126, hex=0x7E, ascii="~"
     */
    0x28,  /* 001010 */
    0x50,  /* 010100 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=127, hex=0x7F, ascii="^?"
     */
    0x10,  /* 000100 */
    0x38,  /* 001110 */
    0x6C,  /* 011011 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x7C,  /* 011111 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=128, hex=0x80, ascii="!^@"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x10,  /* 000100 */
    0x30,  /* 001100 */

    /*
     * code=129, hex=0x81, ascii="!^A"
     */
    0x48,  /* 010010 */
    0x00,  /* 000000 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x58,  /* 010110 */
    0x28,  /* 001010 */
    0x00,  /* 000000 */

    /*
     * code=130, hex=0x82, ascii="!^B"
     */
    0x0C,  /* 000011 */
    0x00,  /* 000000 */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x78,  /* 011110 */
    0x40,  /* 010000 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=131, hex=0x83, ascii="!^C"
     */
    0x38,  /* 001110 */
    0x00,  /* 000000 */
    0x38,  /* 001110 */
    0x04,  /* 000001 */
    0x3C,  /* 001111 */
    0x44,  /* 010001 */
    0x3C,  /* 001111 */
    0x00,  /* 000000 */

    /*
     * code=132, hex=0x84, ascii="!^D"
     */
    0x28,  /* 001010 */
    0x00,  /* 000000 */
    0x38,  /* 001110 */
    0x04,  /* 000001 */
    0x3C,  /* 001111 */
    0x44,  /* 010001 */
    0x3C,  /* 001111 */
    0x00,  /* 000000 */

    /*
     * code=133, hex=0x85, ascii="!^E"
     */
    0x30,  /* 001100 */
    0x00,  /* 000000 */
    0x38,  /* 001110 */
    0x04,  /* 000001 */
    0x3C,  /* 001111 */
    0x44,  /* 010001 */
    0x3C,  /* 001111 */
    0x00,  /* 000000 */

    /*
     * code=134, hex=0x86, ascii="!^F"
     */
    0x38,  /* 001110 */
    0x28,  /* 001010 */
    0x38,  /* 001110 */
    0x04,  /* 000001 */
    0x3C,  /* 001111 */
    0x44,  /* 010001 */
    0x3C,  /* 001111 */
    0x00,  /* 000000 */

    /*
     * code=135, hex=0x87, ascii="!^G"
     */
    0x00,  /* 000000 */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x40,  /* 010000 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x10,  /* 000100 */
    0x30,  /* 001100 */

    /*
     * code=136, hex=0x88, ascii="!^H"
     */
    0x38,  /* 001110 */
    0x00,  /* 000000 */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x78,  /* 011110 */
    0x40,  /* 010000 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=137, hex=0x89, ascii="!^I"
     */
    0x28,  /* 001010 */
    0x00,  /* 000000 */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x78,  /* 011110 */
    0x40,  /* 010000 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=138, hex=0x8A, ascii="!^J"
     */
    0x30,  /* 001100 */
    0x00,  /* 000000 */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x78,  /* 011110 */
    0x40,  /* 010000 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=139, hex=0x8B, ascii="!^K"
     */
    0x28,  /* 001010 */
    0x00,  /* 000000 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x18,  /* 000110 */
    0x00,  /* 000000 */

    /*
     * code=140, hex=0x8C, ascii="!^L"
     */
    0x10,  /* 000100 */
    0x28,  /* 001010 */
    0x00,  /* 000000 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x18,  /* 000110 */
    0x00,  /* 000000 */

    /*
     * code=141, hex=0x8D, ascii="!^M"
     */
    0x20,  /* 001000 */
    0x00,  /* 000000 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x18,  /* 000110 */
    0x00,  /* 000000 */

    /*
     * code=142, hex=0x8E, ascii="!^N"
     */
    0x28,  /* 001010 */
    0x00,  /* 000000 */
    0x10,  /* 000100 */
    0x28,  /* 001010 */
    0x44,  /* 010001 */
    0x7C,  /* 011111 */
    0x44,  /* 010001 */
    0x00,  /* 000000 */

    /*
     * code=143, hex=0x8F, ascii="!^O"
     */
    0x38,  /* 001110 */
    0x28,  /* 001010 */
    0x38,  /* 001110 */
    0x6C,  /* 011011 */
    0x44,  /* 010001 */
    0x7C,  /* 011111 */
    0x44,  /* 010001 */
    0x00,  /* 000000 */

    /*
     * code=144, hex=0x90, ascii="!^P"
     */
    0x0C,  /* 000011 */
    0x00,  /* 000000 */
    0x7C,  /* 011111 */
    0x40,  /* 010000 */
    0x78,  /* 011110 */
    0x40,  /* 010000 */
    0x7C,  /* 011111 */
    0x00,  /* 000000 */

    /*
     * code=145, hex=0x91, ascii="!^Q"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x78,  /* 011110 */
    0x14,  /* 000101 */
    0x7C,  /* 011111 */
    0x50,  /* 010100 */
    0x3C,  /* 001111 */
    0x00,  /* 000000 */

    /*
     * code=146, hex=0x92, ascii="!^R"
     */
    0x3C,  /* 001111 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x7C,  /* 011111 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x5C,  /* 010111 */
    0x00,  /* 000000 */

    /*
     * code=147, hex=0x93, ascii="!^S"
     */
    0x38,  /* 001110 */
    0x00,  /* 000000 */
    0x30,  /* 001100 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x30,  /* 001100 */
    0x00,  /* 000000 */

    /*
     * code=148, hex=0x94, ascii="!^T"
     */
    0x28,  /* 001010 */
    0x00,  /* 000000 */
    0x30,  /* 001100 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x30,  /* 001100 */
    0x00,  /* 000000 */

    /*
     * code=149, hex=0x95, ascii="!^U"
     */
    0x60,  /* 011000 */
    0x00,  /* 000000 */
    0x30,  /* 001100 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x30,  /* 001100 */
    0x00,  /* 000000 */

    /*
     * code=150, hex=0x96, ascii="!^V"
     */
    0x38,  /* 001110 */
    0x00,  /* 000000 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x58,  /* 010110 */
    0x28,  /* 001010 */
    0x00,  /* 000000 */

    /*
     * code=151, hex=0x97, ascii="!^W"
     */
    0x60,  /* 011000 */
    0x00,  /* 000000 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x58,  /* 010110 */
    0x28,  /* 001010 */
    0x00,  /* 000000 */

    /*
     * code=152, hex=0x98, ascii="!^X"
     */
    0x28,  /* 001010 */
    0x00,  /* 000000 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x38,  /* 001110 */
    0x10,  /* 000100 */
    0x60,  /* 011000 */

    /*
     * code=153, hex=0x99, ascii="!^Y"
     */
    0x48,  /* 010010 */
    0x30,  /* 001100 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x30,  /* 001100 */
    0x00,  /* 000000 */

    /*
     * code=154, hex=0x9A, ascii="!^Z"
     */
    0x28,  /* 001010 */
    0x00,  /* 000000 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x30,  /* 001100 */
    0x00,  /* 000000 */

    /*
     * code=155, hex=0x9B, ascii="!^["
     */
    0x00,  /* 000000 */
    0x10,  /* 000100 */
    0x38,  /* 001110 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x38,  /* 001110 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=156, hex=0x9C, ascii="!^\"
     */
    0x18,  /* 000110 */
    0x24,  /* 001001 */
    0x20,  /* 001000 */
    0x78,  /* 011110 */
    0x20,  /* 001000 */
    0x24,  /* 001001 */
    0x5C,  /* 010111 */
    0x00,  /* 000000 */

    /*
     * code=157, hex=0x9D, ascii="!^]"
     */
    0x44,  /* 010001 */
    0x28,  /* 001010 */
    0x10,  /* 000100 */
    0x7C,  /* 011111 */
    0x10,  /* 000100 */
    0x7C,  /* 011111 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=158, hex=0x9E, ascii="!^^"
     */
    0x60,  /* 011000 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x68,  /* 011010 */
    0x5C,  /* 010111 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x00,  /* 000000 */

    /*
     * code=159, hex=0x9F, ascii="!^_"
     */
    0x08,  /* 000010 */
    0x14,  /* 000101 */
    0x10,  /* 000100 */
    0x38,  /* 001110 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x50,  /* 010100 */
    0x20,  /* 001000 */

    /*
     * code=160, hex=0xA0, ascii="! "
     */
    0x18,  /* 000110 */
    0x00,  /* 000000 */
    0x38,  /* 001110 */
    0x04,  /* 000001 */
    0x3C,  /* 001111 */
    0x44,  /* 010001 */
    0x3C,  /* 001111 */
    0x00,  /* 000000 */

    /*
     * code=161, hex=0xA1, ascii="!!"
     */
    0x18,  /* 000110 */
    0x00,  /* 000000 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x18,  /* 000110 */
    0x00,  /* 000000 */

    /*
     * code=162, hex=0xA2, ascii="!""
     */
    0x18,  /* 000110 */
    0x00,  /* 000000 */
    0x30,  /* 001100 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x30,  /* 001100 */
    0x00,  /* 000000 */

    /*
     * code=163, hex=0xA3, ascii="!#"
     */
    0x18,  /* 000110 */
    0x00,  /* 000000 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x58,  /* 010110 */
    0x28,  /* 001010 */
    0x00,  /* 000000 */

    /*
     * code=164, hex=0xA4, ascii="!$"
     */
    0x28,  /* 001010 */
    0x50,  /* 010100 */
    0x00,  /* 000000 */
    0x70,  /* 011100 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x00,  /* 000000 */

    /*
     * code=165, hex=0xA5, ascii="!%"
     */
    0x28,  /* 001010 */
    0x50,  /* 010100 */
    0x00,  /* 000000 */
    0x48,  /* 010010 */
    0x68,  /* 011010 */
    0x58,  /* 010110 */
    0x48,  /* 010010 */
    0x00,  /* 000000 */

    /*
     * code=166, hex=0xA6, ascii="!&"
     */
    0x38,  /* 001110 */
    0x04,  /* 000001 */
    0x3C,  /* 001111 */
    0x44,  /* 010001 */
    0x3C,  /* 001111 */
    0x00,  /* 000000 */
    0x3C,  /* 001111 */
    0x00,  /* 000000 */

    /*
     * code=167, hex=0xA7, ascii="!'"
     */
    0x30,  /* 001100 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x30,  /* 001100 */
    0x00,  /* 000000 */
    0x78,  /* 011110 */
    0x00,  /* 000000 */

    /*
     * code=168, hex=0xA8, ascii="!("
     */
    0x10,  /* 000100 */
    0x00,  /* 000000 */
    0x10,  /* 000100 */
    0x30,  /* 001100 */
    0x40,  /* 010000 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=169, hex=0xA9, ascii="!)"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x7C,  /* 011111 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=170, hex=0xAA, ascii="!*"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0xFC,  /* 111111 */
    0x04,  /* 000001 */
    0x04,  /* 000001 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=171, hex=0xAB, ascii="!+"
     */
    0x40,  /* 010000 */
    0x48,  /* 010010 */
    0x50,  /* 010100 */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x08,  /* 000010 */
    0x1C,  /* 000111 */
    0x00,  /* 000000 */

    /*
     * code=172, hex=0xAC, ascii="!,"
     */
    0x40,  /* 010000 */
    0x48,  /* 010010 */
    0x50,  /* 010100 */
    0x2C,  /* 001011 */
    0x54,  /* 010101 */
    0x1C,  /* 000111 */
    0x04,  /* 000001 */
    0x00,  /* 000000 */

    /*
     * code=173, hex=0xAD, ascii="!-"
     */
    0x10,  /* 000100 */
    0x00,  /* 000000 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x38,  /* 001110 */
    0x38,  /* 001110 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=174, hex=0xAE, ascii="!."
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x24,  /* 001001 */
    0x48,  /* 010010 */
    0x24,  /* 001001 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=175, hex=0xAF, ascii="!/"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x48,  /* 010010 */
    0x24,  /* 001001 */
    0x48,  /* 010010 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=176, hex=0xB0, ascii="!0"
     */
    0x54,  /* 010101 */
    0x00,  /* 000000 */
    0xA8,  /* 101010 */
    0x00,  /* 000000 */
    0x54,  /* 010101 */
    0x00,  /* 000000 */
    0xA8,  /* 101010 */
    0x00,  /* 000000 */

    /*
     * code=177, hex=0xB1, ascii="!1"
     */
    0x54,  /* 010101 */
    0xA8,  /* 101010 */
    0x54,  /* 010101 */
    0xA8,  /* 101010 */
    0x54,  /* 010101 */
    0xA8,  /* 101010 */
    0x54,  /* 010101 */
    0xA8,  /* 101010 */

    /*
     * code=178, hex=0xB2, ascii="!2"
     */
    0xA8,  /* 101010 */
    0xFC,  /* 111111 */
    0x54,  /* 010101 */
    0xFC,  /* 111111 */
    0xA8,  /* 101010 */
    0xFC,  /* 111111 */
    0x54,  /* 010101 */
    0xFC,  /* 111111 */

    /*
     * code=179, hex=0xB3, ascii="!3"
     */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */

    /*
     * code=180, hex=0xB4, ascii="!4"
     */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0xF0,  /* 111100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */

    /*
     * code=181, hex=0xB5, ascii="!5"
     */
    0x10,  /* 000100 */
    0xF0,  /* 111100 */
    0x10,  /* 000100 */
    0xF0,  /* 111100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */

    /*
     * code=182, hex=0xB6, ascii="!6"
     */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0xD0,  /* 110100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */

    /*
     * code=183, hex=0xB7, ascii="!7"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0xF0,  /* 111100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */

    /*
     * code=184, hex=0xB8, ascii="!8"
     */
    0x00,  /* 000000 */
    0xF0,  /* 111100 */
    0x10,  /* 000100 */
    0xF0,  /* 111100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */

    /*
     * code=185, hex=0xB9, ascii="!9"
     */
    0x50,  /* 010100 */
    0xD0,  /* 110100 */
    0x10,  /* 000100 */
    0xD0,  /* 110100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */

    /*
     * code=186, hex=0xBA, ascii="!:"
     */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */

    /*
     * code=187, hex=0xBB, ascii="!;"
     */
    0x00,  /* 000000 */
    0xF0,  /* 111100 */
    0x10,  /* 000100 */
    0xD0,  /* 110100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */

    /*
     * code=188, hex=0xBC, ascii="!<"
     */
    0x50,  /* 010100 */
    0xD0,  /* 110100 */
    0x10,  /* 000100 */
    0xF0,  /* 111100 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=189, hex=0xBD, ascii="!="
     */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0xF0,  /* 111100 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=190, hex=0xBE, ascii="!>"
     */
    0x10,  /* 000100 */
    0xF0,  /* 111100 */
    0x10,  /* 000100 */
    0xF0,  /* 111100 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=191, hex=0xBF, ascii="!?"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0xF0,  /* 111100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */

    /*
     * code=192, hex=0xC0, ascii="!@"
     */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x1C,  /* 000111 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=193, hex=0xC1, ascii="!A"
     */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0xFC,  /* 111111 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=194, hex=0xC2, ascii="!B"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0xFC,  /* 111111 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */

    /*
     * code=195, hex=0xC3, ascii="!C"
     */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x1C,  /* 000111 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */

    /*
     * code=196, hex=0xC4, ascii="!D"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0xFC,  /* 111111 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=197, hex=0xC5, ascii="!E"
     */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0xFC,  /* 111111 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */

    /*
     * code=198, hex=0xC6, ascii="!F"
     */
    0x10,  /* 000100 */
    0x1C,  /* 000111 */
    0x10,  /* 000100 */
    0x1C,  /* 000111 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */

    /*
     * code=199, hex=0xC7, ascii="!G"
     */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x5C,  /* 010111 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */

    /*
     * code=200, hex=0xC8, ascii="!H"
     */
    0x50,  /* 010100 */
    0x5C,  /* 010111 */
    0x40,  /* 010000 */
    0x7C,  /* 011111 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=201, hex=0xC9, ascii="!I"
     */
    0x00,  /* 000000 */
    0x7C,  /* 011111 */
    0x40,  /* 010000 */
    0x5C,  /* 010111 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */

    /*
     * code=202, hex=0xCA, ascii="!J"
     */
    0x50,  /* 010100 */
    0xDC,  /* 110111 */
    0x00,  /* 000000 */
    0xFC,  /* 111111 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=203, hex=0xCB, ascii="!K"
     */
    0x00,  /* 000000 */
    0xFC,  /* 111111 */
    0x00,  /* 000000 */
    0xDC,  /* 110111 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */

    /*
     * code=204, hex=0xCC, ascii="!L"
     */
    0x50,  /* 010100 */
    0x5C,  /* 010111 */
    0x40,  /* 010000 */
    0x5C,  /* 010111 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */

    /*
     * code=205, hex=0xCD, ascii="!M"
     */
    0x00,  /* 000000 */
    0xFC,  /* 111111 */
    0x00,  /* 000000 */
    0xFC,  /* 111111 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=206, hex=0xCE, ascii="!N"
     */
    0x50,  /* 010100 */
    0xDC,  /* 110111 */
    0x00,  /* 000000 */
    0xDC,  /* 110111 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */

    /*
     * code=207, hex=0xCF, ascii="!O"
     */
    0x10,  /* 000100 */
    0xFC,  /* 111111 */
    0x00,  /* 000000 */
    0xFC,  /* 111111 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=208, hex=0xD0, ascii="!P"
     */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0xFC,  /* 111111 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=209, hex=0xD1, ascii="!Q"
     */
    0x00,  /* 000000 */
    0xFC,  /* 111111 */
    0x00,  /* 000000 */
    0xFC,  /* 111111 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */

    /*
     * code=210, hex=0xD2, ascii="!R"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0xFC,  /* 111111 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */

    /*
     * code=211, hex=0xD3, ascii="!S"
     */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x7C,  /* 011111 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=212, hex=0xD4, ascii="!T"
     */
    0x10,  /* 000100 */
    0x1C,  /* 000111 */
    0x10,  /* 000100 */
    0x1C,  /* 000111 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=213, hex=0xD5, ascii="!U"
     */
    0x00,  /* 000000 */
    0x1C,  /* 000111 */
    0x10,  /* 000100 */
    0x1C,  /* 000111 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */

    /*
     * code=214, hex=0xD6, ascii="!V"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x7C,  /* 011111 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */

    /*
     * code=215, hex=0xD7, ascii="!W"
     */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0xDC,  /* 110111 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */

    /*
     * code=216, hex=0xD8, ascii="!X"
     */
    0x10,  /* 000100 */
    0xFC,  /* 111111 */
    0x00,  /* 000000 */
    0xFC,  /* 111111 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */

    /*
     * code=217, hex=0xD9, ascii="!Y"
     */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0xF0,  /* 111100 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=218, hex=0xDA, ascii="!Z"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x1C,  /* 000111 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */

    /*
     * code=219, hex=0xDB, ascii="!["
     */
    0xFC,  /* 111111 */
    0xFC,  /* 111111 */
    0xFC,  /* 111111 */
    0xFC,  /* 111111 */
    0xFC,  /* 111111 */
    0xFC,  /* 111111 */
    0xFC,  /* 111111 */
    0xFC,  /* 111111 */

    /*
     * code=220, hex=0xDC, ascii="!\"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0xFC,  /* 111111 */
    0xFC,  /* 111111 */
    0xFC,  /* 111111 */
    0xFC,  /* 111111 */

    /*
     * code=221, hex=0xDD, ascii="!]"
     */
    0xE0,  /* 111000 */
    0xE0,  /* 111000 */
    0xE0,  /* 111000 */
    0xE0,  /* 111000 */
    0xE0,  /* 111000 */
    0xE0,  /* 111000 */
    0xE0,  /* 111000 */
    0xE0,  /* 111000 */

    /*
     * code=222, hex=0xDE, ascii="!^"
     */
    0x1C,  /* 000111 */
    0x1C,  /* 000111 */
    0x1C,  /* 000111 */
    0x1C,  /* 000111 */
    0x1C,  /* 000111 */
    0x1C,  /* 000111 */
    0x1C,  /* 000111 */
    0x1C,  /* 000111 */

    /*
     * code=223, hex=0xDF, ascii="!_"
     */
    0xFC,  /* 111111 */
    0xFC,  /* 111111 */
    0xFC,  /* 111111 */
    0xFC,  /* 111111 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=224, hex=0xE0, ascii="!`"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x34,  /* 001101 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x34,  /* 001101 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=225, hex=0xE1, ascii="!a"
     */
    0x00,  /* 000000 */
    0x70,  /* 011100 */
    0x48,  /* 010010 */
    0x70,  /* 011100 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x70,  /* 011100 */
    0x40,  /* 010000 */

    /*
     * code=226, hex=0xE2, ascii="!b"
     */
    0x78,  /* 011110 */
    0x48,  /* 010010 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x00,  /* 000000 */

    /*
     * code=227, hex=0xE3, ascii="!c"
     */
    0x00,  /* 000000 */
    0x7C,  /* 011111 */
    0x28,  /* 001010 */
    0x28,  /* 001010 */
    0x28,  /* 001010 */
    0x28,  /* 001010 */
    0x28,  /* 001010 */
    0x00,  /* 000000 */

    /*
     * code=228, hex=0xE4, ascii="!d"
     */
    0x78,  /* 011110 */
    0x48,  /* 010010 */
    0x20,  /* 001000 */
    0x10,  /* 000100 */
    0x20,  /* 001000 */
    0x48,  /* 010010 */
    0x78,  /* 011110 */
    0x00,  /* 000000 */

    /*
     * code=229, hex=0xE5, ascii="!e"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x3C,  /* 001111 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x30,  /* 001100 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=230, hex=0xE6, ascii="!f"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x70,  /* 011100 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */

    /*
     * code=231, hex=0xE7, ascii="!g"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x28,  /* 001010 */
    0x50,  /* 010100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=232, hex=0xE8, ascii="!h"
     */
    0x38,  /* 001110 */
    0x10,  /* 000100 */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x10,  /* 000100 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=233, hex=0xE9, ascii="!i"
     */
    0x30,  /* 001100 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x78,  /* 011110 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x30,  /* 001100 */
    0x00,  /* 000000 */

    /*
     * code=234, hex=0xEA, ascii="!j"
     */
    0x00,  /* 000000 */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x28,  /* 001010 */
    0x28,  /* 001010 */
    0x6C,  /* 011011 */
    0x00,  /* 000000 */

    /*
     * code=235, hex=0xEB, ascii="!k"
     */
    0x30,  /* 001100 */
    0x40,  /* 010000 */
    0x20,  /* 001000 */
    0x10,  /* 000100 */
    0x38,  /* 001110 */
    0x48,  /* 010010 */
    0x30,  /* 001100 */
    0x00,  /* 000000 */

    /*
     * code=236, hex=0xEC, ascii="!l"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x28,  /* 001010 */
    0x54,  /* 010101 */
    0x54,  /* 010101 */
    0x28,  /* 001010 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=237, hex=0xED, ascii="!m"
     */
    0x00,  /* 000000 */
    0x10,  /* 000100 */
    0x38,  /* 001110 */
    0x54,  /* 010101 */
    0x54,  /* 010101 */
    0x38,  /* 001110 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=238, hex=0xEE, ascii="!n"
     */
    0x00,  /* 000000 */
    0x38,  /* 001110 */
    0x40,  /* 010000 */
    0x78,  /* 011110 */
    0x40,  /* 010000 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=239, hex=0xEF, ascii="!o"
     */
    0x00,  /* 000000 */
    0x30,  /* 001100 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=240, hex=0xF0, ascii="!p"
     */
    0x00,  /* 000000 */
    0x78,  /* 011110 */
    0x00,  /* 000000 */
    0x78,  /* 011110 */
    0x00,  /* 000000 */
    0x78,  /* 011110 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=241, hex=0xF1, ascii="!q"
     */
    0x00,  /* 000000 */
    0x10,  /* 000100 */
    0x38,  /* 001110 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=242, hex=0xF2, ascii="!r"
     */
    0x40,  /* 010000 */
    0x30,  /* 001100 */
    0x08,  /* 000010 */
    0x30,  /* 001100 */
    0x40,  /* 010000 */
    0x00,  /* 000000 */
    0x78,  /* 011110 */
    0x00,  /* 000000 */

    /*
     * code=243, hex=0xF3, ascii="!s"
     */
    0x08,  /* 000010 */
    0x30,  /* 001100 */
    0x40,  /* 010000 */
    0x30,  /* 001100 */
    0x08,  /* 000010 */
    0x00,  /* 000000 */
    0x78,  /* 011110 */
    0x00,  /* 000000 */

    /*
     * code=244, hex=0xF4, ascii="!t"
     */
    0x00,  /* 000000 */
    0x08,  /* 000010 */
    0x14,  /* 000101 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */

    /*
     * code=245, hex=0xF5, ascii="!u"
     */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x50,  /* 010100 */
    0x20,  /* 001000 */
    0x00,  /* 000000 */

    /*
     * code=246, hex=0xF6, ascii="!v"
     */
    0x00,  /* 000000 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */
    0x7C,  /* 011111 */
    0x00,  /* 000000 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=247, hex=0xF7, ascii="!w"
     */
    0x00,  /* 000000 */
    0x28,  /* 001010 */
    0x50,  /* 010100 */
    0x00,  /* 000000 */
    0x28,  /* 001010 */
    0x50,  /* 010100 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=248, hex=0xF8, ascii="!x"
     */
    0x30,  /* 001100 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x30,  /* 001100 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=249, hex=0xF9, ascii="!y"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x30,  /* 001100 */
    0x30,  /* 001100 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=250, hex=0xFA, ascii="!z"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x20,  /* 001000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=251, hex=0xFB, ascii="!{"
     */
    0x00,  /* 000000 */
    0x1C,  /* 000111 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x20,  /* 001000 */
    0x00,  /* 000000 */

    /*
     * code=252, hex=0xFC, ascii="!|"
     */
    0x50,  /* 010100 */
    0x28,  /* 001010 */
    0x28,  /* 001010 */
    0x28,  /* 001010 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=253, hex=0xFD, ascii="!}"
     */
    0x60,  /* 011000 */
    0x10,  /* 000100 */
    0x20,  /* 001000 */
    0x70,  /* 011100 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=254, hex=0xFE, ascii="!~"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x78,  /* 011110 */
    0x78,  /* 011110 */
    0x78,  /* 011110 */
    0x78,  /* 011110 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=255, hex=0xFF, ascii="!^ź"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00   /* 000000 */
};

void WS2812FX::drawCharacter(unsigned char chr, int16_t x, int16_t y, CRGB color, CRGB *leds) {
  uint16_t width  = SEGMENT.virtualWidth();
  uint16_t height = SEGMENT.virtualHeight();

  for (uint8_t i = 0; i<8; i++) { // character height
    int16_t y0 = y + i;
    if (y0 < 0) continue; // drawing off-screen
    if (y0 >= height) break; // drawing off-screen
    uint8_t bits = pgm_read_byte_near(&console_font_6x8[(chr * 8) + i]);
    for (uint8_t j = 0; j<6; j++) { // character width
      int16_t x0 = x + 5 - j;
      if ((x0 >= 0 || x0 < width) && ((bits>>(j+1)) & 0x01)) { // bit set & drawing on-screen
        if (leds) leds[XY(x0,y0)] = color;
        else      setPixelColorXY(x0, y0, color);
      }
    }
  }
}


#define WU_WEIGHT(a,b) ((uint8_t) (((a)*(b)+(a)+(b))>>8))
void WS2812FX::wu_pixel(CRGB *leds, uint32_t x, uint32_t y, CRGB c) {      //awesome wu_pixel procedure by reddit u/sutaburosu
  // extract the fractional parts and derive their inverses
  uint8_t xx = x & 0xff, yy = y & 0xff, ix = 255 - xx, iy = 255 - yy;
  // calculate the intensities for each affected pixel
  uint8_t wu[4] = {WU_WEIGHT(ix, iy), WU_WEIGHT(xx, iy),
                   WU_WEIGHT(ix, yy), WU_WEIGHT(xx, yy)};
  // multiply the intensities by the colour, and saturating-add them to the pixels
  for (uint8_t i = 0; i < 4; i++) {
    uint16_t xy = XY((x >> 8) + (i & 1), (y >> 8) + ((i >> 1) & 1));
    leds[xy].r = qadd8(leds[xy].r, c.r * wu[i] >> 8);
    leds[xy].g = qadd8(leds[xy].g, c.g * wu[i] >> 8);
    leds[xy].b = qadd8(leds[xy].b, c.b * wu[i] >> 8);
  }
}
#undef WU_WEIGHT
