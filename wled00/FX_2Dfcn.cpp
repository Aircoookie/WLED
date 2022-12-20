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
// followed by horizontal pixels, when Segment::maxWidth logical pixels are added they
// are followed by next row (down) of Segment::maxWidth pixels (and so forth)
// note: matrix may be comprised of multiple panels each with different orientation
// but ledmap takes care of that. ledmap is constructed upon initialization
// so matrix should disable regular ledmap processing
void WS2812FX::setUpMatrix() {
#ifndef WLED_DISABLE_2D
  // erase old ledmap, just in case.
  if (customMappingTable != nullptr) delete[] customMappingTable;
  customMappingTable = nullptr;
  customMappingSize = 0;

  // isMatrix is set in cfg.cpp or set.cpp
  if (isMatrix) {
    Segment::maxWidth  = hPanels * panelW;
    Segment::maxHeight = vPanels * panelH;

    // safety check
    if (Segment::maxWidth * Segment::maxHeight > MAX_LEDS || Segment::maxWidth == 1 || Segment::maxHeight == 1) {
      Segment::maxWidth = _length;
      Segment::maxHeight = 1;
      isMatrix = false;
      return;
    }

    customMappingSize  = Segment::maxWidth * Segment::maxHeight;
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

          startL = (matrix.vertical ? y : x) * panelW + (matrix.vertical ? x : y) * Segment::maxWidth * panelH; // logical index (top-left corner)
          startP = p * panelW * panelH; // physical index (top-left corner)

          uint8_t H = panel[h*j + i].vertical ? panelW : panelH;
          uint8_t W = panel[h*j + i].vertical ? panelH : panelW;
          for (uint16_t l=0, q=0; l<H; l++) {
            for (uint16_t k=0; k<W; k++, q++) {
              y = (panel[h*j + i].vertical ? panel[h*j + i].rightStart : panel[h*j + i].bottomStart) ? H - l - 1 : l;
              x = (panel[h*j + i].vertical ? panel[h*j + i].bottomStart : panel[h*j + i].rightStart) ? W - k - 1 : k;
              x = (panel[h*j + i].serpentine && l%2) ? (W - x - 1) : x;
              offset = (panel[h*j + i].vertical ? y : x) + (panel[h*j + i].vertical ? x : y) * Segment::maxWidth;
              customMappingTable[startL + offset] = startP + q;
            }
          }
        }
      }
      #ifdef WLED_DEBUG
      DEBUG_PRINT(F("Matrix ledmap:"));
      for (uint16_t i=0; i<customMappingSize; i++) {
        if (!(i%Segment::maxWidth)) DEBUG_PRINTLN();
        DEBUG_PRINTF("%4d,", customMappingTable[i]);
      }
      DEBUG_PRINTLN();
      #endif
    } else {
      // memory allocation error
      Segment::maxWidth = _length;
      Segment::maxHeight = 1;
      isMatrix = false;
      return;
    }
  } else { 
    // not a matrix set up
    Segment::maxWidth = _length;
    Segment::maxHeight = 1;
  }
#endif
}

// absolute matrix version of setPixelColor()
void IRAM_ATTR WS2812FX::setPixelColorXY(int x, int y, uint32_t col)
{
#ifndef WLED_DISABLE_2D
  if (!isMatrix) return; // not a matrix set-up
  uint16_t index = y * Segment::maxWidth + x;
  if (index >= customMappingSize) return; // customMappingSize is always W * H of matrix in 2D setup
#else
  uint16_t index = x;
  if (index >= _length) return;
#endif
  if (index < customMappingSize) index = customMappingTable[index];
  busses.setPixelColor(index, col);
}

// returns RGBW values of pixel
uint32_t WS2812FX::getPixelColorXY(uint16_t x, uint16_t y) {
#ifndef WLED_DISABLE_2D
  uint16_t index = (y * Segment::maxWidth + x);
  if (index >= customMappingSize) return 0; // customMappingSize is always W * H of matrix in 2D setup
#else
  uint16_t index = x;
  if (index >= _length) return 0;
#endif
  if (index < customMappingSize) index = customMappingTable[index];
  return busses.getPixelColor(index);
}

///////////////////////////////////////////////////////////
// Segment:: routines
///////////////////////////////////////////////////////////

#ifndef WLED_DISABLE_2D

// XY(x,y) - gets pixel index within current segment (often used to reference leds[] array element)
uint16_t IRAM_ATTR Segment::XY(uint16_t x, uint16_t y) {
  uint16_t width  = virtualWidth();   // segment width in logical pixels
  uint16_t height = virtualHeight();  // segment height in logical pixels
  return (x%width) + (y%height) * width;
}

void IRAM_ATTR Segment::setPixelColorXY(int x, int y, uint32_t col)
{
  if (Segment::maxHeight==1) return; // not a matrix set-up
  if (x >= virtualWidth() || y >= virtualHeight() || x<0 || y<0) return;  // if pixel would fall out of virtual segment just exit

  if (leds) leds[XY(x,y)] = col;

  uint8_t _bri_t = currentBri(on ? opacity : 0);
  if (_bri_t < 255) {
    byte r = scale8(R(col), _bri_t);
    byte g = scale8(G(col), _bri_t);
    byte b = scale8(B(col), _bri_t);
    byte w = scale8(W(col), _bri_t);
    col = RGBW32(r, g, b, w);
  }

  if (reverse  ) x = virtualWidth()  - x - 1;
  if (reverse_y) y = virtualHeight() - y - 1;
  if (transpose) { uint16_t t = x; x = y; y = t; } // swap X & Y if segment transposed

  x *= groupLength(); // expand to physical pixels
  y *= groupLength(); // expand to physical pixels
  if (x >= width() || y >= height()) return;  // if pixel would fall out of segment just exit

  for (int j = 0; j < grouping; j++) {   // groupping vertically
    for (int g = 0; g < grouping; g++) { // groupping horizontally
      uint16_t xX = (x+g), yY = (y+j);
      if (xX >= width() || yY >= height()) continue; // we have reached one dimension's end

      strip.setPixelColorXY(start + xX, startY + yY, col);

      if (mirror) { //set the corresponding horizontally mirrored pixel
        if (transpose) strip.setPixelColorXY(start + xX, startY + height() - yY - 1, col);
        else           strip.setPixelColorXY(start + width() - xX - 1, startY + yY, col);
      }
      if (mirror_y) { //set the corresponding vertically mirrored pixel
        if (transpose) strip.setPixelColorXY(start + width() - xX - 1, startY + yY, col);
        else           strip.setPixelColorXY(start + xX, startY + height() - yY - 1, col);
      }
      if (mirror_y && mirror) { //set the corresponding vertically AND horizontally mirrored pixel
        strip.setPixelColorXY(width() - xX - 1, height() - yY - 1, col);
      }
    }
  }
}

// anti-aliased version of setPixelColorXY()
void Segment::setPixelColorXY(float x, float y, uint32_t col, bool aa)
{
  if (Segment::maxHeight==1) return; // not a matrix set-up
  if (x<0.0f || x>1.0f || y<0.0f || y>1.0f) return; // not normalized

  const uint16_t cols = virtualWidth();
  const uint16_t rows = virtualHeight();

  float fX = x * (cols-1);
  float fY = y * (rows-1);
  if (aa) {
    uint16_t xL = roundf(fX-0.49f);
    uint16_t xR = roundf(fX+0.49f);
    uint16_t yT = roundf(fY-0.49f);
    uint16_t yB = roundf(fY+0.49f);
    float    dL = (fX - xL)*(fX - xL);
    float    dR = (xR - fX)*(xR - fX);
    float    dT = (fY - yT)*(fY - yT);
    float    dB = (yB - fY)*(yB - fY);
    uint32_t cXLYT = getPixelColorXY(xL, yT);
    uint32_t cXRYT = getPixelColorXY(xR, yT);
    uint32_t cXLYB = getPixelColorXY(xL, yB);
    uint32_t cXRYB = getPixelColorXY(xR, yB);

    if (xL!=xR && yT!=yB) {
      setPixelColorXY(xL, yT, color_blend(col, cXLYT, uint8_t(sqrtf(dL*dT)*255.0f))); // blend TL pixel
      setPixelColorXY(xR, yT, color_blend(col, cXRYT, uint8_t(sqrtf(dR*dT)*255.0f))); // blend TR pixel
      setPixelColorXY(xL, yB, color_blend(col, cXLYB, uint8_t(sqrtf(dL*dB)*255.0f))); // blend BL pixel
      setPixelColorXY(xR, yB, color_blend(col, cXRYB, uint8_t(sqrtf(dR*dB)*255.0f))); // blend BR pixel
    } else if (xR!=xL && yT==yB) {
      setPixelColorXY(xR, yT, color_blend(col, cXLYT, uint8_t(dL*255.0f))); // blend L pixel
      setPixelColorXY(xR, yT, color_blend(col, cXRYT, uint8_t(dR*255.0f))); // blend R pixel
    } else if (xR==xL && yT!=yB) {
      setPixelColorXY(xR, yT, color_blend(col, cXLYT, uint8_t(dT*255.0f))); // blend T pixel
      setPixelColorXY(xL, yB, color_blend(col, cXLYB, uint8_t(dB*255.0f))); // blend B pixel
    } else {
      setPixelColorXY(xL, yT, col); // exact match (x & y land on a pixel)
    }
  } else {
    setPixelColorXY(uint16_t(roundf(fX)), uint16_t(roundf(fY)), col);
  }
}

// returns RGBW values of pixel
uint32_t Segment::getPixelColorXY(uint16_t x, uint16_t y) {
  int i = XY(x,y);
  if (leds) return RGBW32(leds[i].r, leds[i].g, leds[i].b, 0);
  if (reverse  ) x = virtualWidth()  - x - 1;
  if (reverse_y) y = virtualHeight() - y - 1;
  if (transpose) { uint16_t t = x; x = y; y = t; } // swap X & Y if segment transposed
  x *= groupLength(); // expand to physical pixels
  y *= groupLength(); // expand to physical pixels
  if (x >= width() || y >= height()) return 0;
  return strip.getPixelColorXY(start + x, startY + y);
}

// Blends the specified color with the existing pixel color.
void Segment::blendPixelColorXY(uint16_t x, uint16_t y, uint32_t color, uint8_t blend) {
  setPixelColorXY(x, y, color_blend(getPixelColorXY(x,y), color, blend));
}

// Adds the specified color with the existing pixel color perserving color balance.
void Segment::addPixelColorXY(int x, int y, uint32_t color) {
  setPixelColorXY(x, y, color_add(getPixelColorXY(x,y), color));
}

void Segment::fadePixelColorXY(uint16_t x, uint16_t y, uint8_t fade) {
  CRGB pix = CRGB(getPixelColorXY(x,y)).nscale8_video(fade);
  setPixelColor(x, y, pix);
}

// blurRow: perform a blur on a row of a rectangular matrix
void Segment::blurRow(uint16_t row, fract8 blur_amount) {
  const uint16_t cols = virtualWidth();
  const uint16_t rows = virtualHeight();

  if (row >= rows) return;
  // blur one row
  uint8_t keep = 255 - blur_amount;
  uint8_t seep = blur_amount >> 1;
  CRGB carryover = CRGB::Black;
  for (uint16_t x = 0; x < cols; x++) {
    CRGB cur = getPixelColorXY(x, row);
    CRGB part = cur;
    part.nscale8(seep);
    cur.nscale8(keep);
    cur += carryover;
    if (x) {
      CRGB prev = CRGB(getPixelColorXY(x-1, row)) + part;
      setPixelColorXY(x-1, row, prev);
    }
    setPixelColorXY(x, row, cur);
    carryover = part;
  }
}

// blurCol: perform a blur on a column of a rectangular matrix
void Segment::blurCol(uint16_t col, fract8 blur_amount) {
  const uint16_t cols = virtualWidth();
  const uint16_t rows = virtualHeight();

  if (col >= cols) return;
  // blur one column
  uint8_t keep = 255 - blur_amount;
  uint8_t seep = blur_amount >> 1;
  CRGB carryover = CRGB::Black;
  for (uint16_t i = 0; i < rows; i++) {
    CRGB cur = getPixelColorXY(col, i);
    CRGB part = cur;
    part.nscale8(seep);
    cur.nscale8(keep);
    cur += carryover;
    if (i) {
      CRGB prev = CRGB(getPixelColorXY(col, i-1)) + part;
      setPixelColorXY(col, i-1, prev);
    }
    setPixelColorXY(col, i, cur);
    carryover = part;
  }
}

// 1D Box blur (with added weight - blur_amount: [0=no blur, 255=max blur])
void Segment::box_blur(uint16_t i, bool vertical, fract8 blur_amount) {
  const uint16_t cols = virtualWidth();
  const uint16_t rows = virtualHeight();
  const uint16_t dim1 = vertical ? rows : cols;
  const uint16_t dim2 = vertical ? cols : rows;
  if (i >= dim2) return;
  const float seep = blur_amount/255.f;
  const float keep = 3.f - 2.f*seep;
  // 1D box blur
  CRGB tmp[dim1];
  for (uint16_t j = 0; j < dim1; j++) {
    uint16_t x = vertical ? i : j;
    uint16_t y = vertical ? j : i;
    uint16_t xp = vertical ? x : x-1;
    uint16_t yp = vertical ? y-1 : y;
    uint16_t xn = vertical ? x : x+1;
    uint16_t yn = vertical ? y+1 : y;
    CRGB curr = getPixelColorXY(x,y);
    CRGB prev = (xp<0 || yp<0) ? CRGB::Black : getPixelColorXY(xp,yp);
    CRGB next = ((vertical && yn>=dim1) || (!vertical && xn>=dim1)) ? CRGB::Black : getPixelColorXY(xn,yn);
    uint16_t r, g, b;
    r = (curr.r*keep + (prev.r + next.r)*seep) / 3;
    g = (curr.g*keep + (prev.g + next.g)*seep) / 3;
    b = (curr.b*keep + (prev.b + next.b)*seep) / 3;
    tmp[j] = CRGB(r,g,b);
  }
  for (uint16_t j = 0; j < dim1; j++) {
    uint16_t x = vertical ? i : j;
    uint16_t y = vertical ? j : i;
    setPixelColorXY(x, y, tmp[j]);
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

void Segment::blur1d(fract8 blur_amount) {
  const uint16_t rows = virtualHeight();
  for (uint16_t y = 0; y < rows; y++) blurRow(y, blur_amount);
}

void Segment::moveX(int8_t delta) {
  const uint16_t cols = virtualWidth();
  const uint16_t rows = virtualHeight();
  if (!delta) return;
  if (delta > 0) {
    for (uint8_t y = 0; y < rows; y++) for (uint8_t x = 0; x < cols-1; x++) {
      if (x + delta >= cols) break;
      setPixelColorXY(x, y, getPixelColorXY((x + delta)%cols, y));
    }
  } else {
    for (uint8_t y = 0; y < rows; y++) for (int16_t x = cols-1; x >= 0; x--) {
      if (x + delta < 0) break;
      setPixelColorXY(x, y, getPixelColorXY(x + delta, y));
    }
  }
}

void Segment::moveY(int8_t delta) {
  const uint16_t cols = virtualWidth();
  const uint16_t rows = virtualHeight();
  if (!delta) return;
  if (delta > 0) {
    for (uint8_t x = 0; x < cols; x++) for (uint8_t y = 0; y < rows-1; y++) {
      if (y + delta >= rows) break;
      setPixelColorXY(x, y, getPixelColorXY(x, (y + delta)));
    }
  } else {
    for (uint8_t x = 0; x < cols; x++) for (int16_t y = rows-1; y >= 0; y--) {
      if (y + delta < 0) break;
      setPixelColorXY(x, y, getPixelColorXY(x, y + delta));
    }
  }
}

// move() - move all pixels in desired direction delta number of pixels
// @param dir direction: 0=left, 1=left-up, 2=up, 3=right-up, 4=right, 5=right-down, 6=down, 7=left-down
// @param delta number of pixels to move
void Segment::move(uint8_t dir, uint8_t delta) {
  if (delta==0) return;
  switch (dir) {
    case 0: moveX( delta);                break;
    case 1: moveX( delta); moveY( delta); break;
    case 2:                moveY( delta); break;
    case 3: moveX(-delta); moveY( delta); break;
    case 4: moveX(-delta);                break;
    case 5: moveX(-delta); moveY(-delta); break;
    case 6:                moveY(-delta); break;
    case 7: moveX( delta); moveY(-delta); break;
  }
}

// by stepko, taken from https://editor.soulmatelights.com/gallery/573-blobs
void Segment::fill_circle(uint16_t cx, uint16_t cy, uint8_t radius, CRGB col) {
  const uint16_t cols = virtualWidth();
  const uint16_t rows = virtualHeight();
  for (int16_t y = -radius; y <= radius; y++) {
    for (int16_t x = -radius; x <= radius; x++) {
      if (x * x + y * y <= radius * radius &&
          int16_t(cx)+x>=0 && int16_t(cy)+y>=0 &&
          int16_t(cx)+x<cols && int16_t(cy)+y<rows)
        addPixelColorXY(cx + x, cy + y, col);
    }
  }
}

void Segment::nscale8(uint8_t scale) {
  const uint16_t cols = virtualWidth();
  const uint16_t rows = virtualHeight();
  for(uint16_t y = 0; y < rows; y++) for (uint16_t x = 0; x < cols; x++) {
    setPixelColorXY(x, y, CRGB(getPixelColorXY(x, y)).nscale8(scale));
  }
}

//line function
void Segment::drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint32_t c) {
  const uint16_t cols = virtualWidth();
  const uint16_t rows = virtualHeight();
  if (x0 >= cols || x1 >= cols || y0 >= rows || y1 >= rows) return;
  const int16_t dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
  const int16_t dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
  int16_t err = (dx>dy ? dx : -dy)/2, e2;
  for (;;) {
    addPixelColorXY(x0,y0,c);
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
}

#include "src/font/console_font_4x6.h"
#include "src/font/console_font_5x8.h"
#include "src/font/console_font_5x12.h"
#include "src/font/console_font_6x8.h"
#include "src/font/console_font_7x9.h"

// draws a raster font character on canvas
// only supports: 4x6=24, 5x8=40, 5x12=60, 6x8=48 and 7x9=63 fonts ATM
void Segment::drawCharacter(unsigned char chr, int16_t x, int16_t y, uint8_t w, uint8_t h, uint32_t color) {
  if (chr < 32 || chr > 126) return; // only ASCII 32-126 supported
  chr -= 32; // align with font table entries
  const uint16_t cols = virtualWidth();
  const uint16_t rows = virtualHeight();
  const int font = w*h;

  //if (w<5 || w>6 || h!=8) return;
  for (int i = 0; i<h; i++) { // character height
    int16_t y0 = y + i;
    if (y0 < 0) continue; // drawing off-screen
    if (y0 >= rows) break; // drawing off-screen
    uint8_t bits = 0;
    switch (font) {
      case 24: bits = pgm_read_byte_near(&console_font_4x6[(chr * h) + i]); break;  // 5x8 font
      case 40: bits = pgm_read_byte_near(&console_font_5x8[(chr * h) + i]); break;  // 5x8 font
      case 48: bits = pgm_read_byte_near(&console_font_6x8[(chr * h) + i]); break;  // 6x8 font
      case 63: bits = pgm_read_byte_near(&console_font_7x9[(chr * h) + i]); break;  // 7x9 font
      case 60: bits = pgm_read_byte_near(&console_font_5x12[(chr * h) + i]); break; // 5x12 font
      default: return;
    }
    for (int j = 0; j<w; j++) { // character width
      int16_t x0 = x + (w-1) - j;
      if ((x0 >= 0 || x0 < cols) && ((bits>>(j+(8-w))) & 0x01)) { // bit set & drawing on-screen
        addPixelColorXY(x0, y0, color);
      }
    }
  }
}

#define WU_WEIGHT(a,b) ((uint8_t) (((a)*(b)+(a)+(b))>>8))
void Segment::wu_pixel(uint32_t x, uint32_t y, CRGB c) {      //awesome wu_pixel procedure by reddit u/sutaburosu
  // extract the fractional parts and derive their inverses
  uint8_t xx = x & 0xff, yy = y & 0xff, ix = 255 - xx, iy = 255 - yy;
  // calculate the intensities for each affected pixel
  uint8_t wu[4] = {WU_WEIGHT(ix, iy), WU_WEIGHT(xx, iy),
                   WU_WEIGHT(ix, yy), WU_WEIGHT(xx, yy)};
  // multiply the intensities by the colour, and saturating-add them to the pixels
  for (int i = 0; i < 4; i++) {
    CRGB led = getPixelColorXY((x >> 8) + (i & 1), (y >> 8) + ((i >> 1) & 1));
    led.r = qadd8(led.r, c.r * wu[i] >> 8);
    led.g = qadd8(led.g, c.g * wu[i] >> 8);
    led.b = qadd8(led.b, c.b * wu[i] >> 8);
    setPixelColorXY(int((x >> 8) + (i & 1)), int((y >> 8) + ((i >> 1) & 1)), led);
  }
}
#undef WU_WEIGHT

#endif // WLED_DISABLE_2D
