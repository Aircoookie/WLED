/*
  FX_2Dfcn.cpp contains all 2D utility functions

  Copyright (c) 2022  Blaz Kristan (https://blaz.at/home)
  Licensed under the EUPL v. 1.2 or later
  Adapted from code originally licensed under the MIT license

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
  // isMatrix is set in cfg.cpp or set.cpp
  if (isMatrix) {
    // calculate width dynamically because it may have gaps
    Segment::maxWidth = 1;
    Segment::maxHeight = 1;
    for (size_t i = 0; i < panel.size(); i++) {
      Panel &p = panel[i];
      if (p.xOffset + p.width > Segment::maxWidth) {
        Segment::maxWidth = p.xOffset + p.width;
      }
      if (p.yOffset + p.height > Segment::maxHeight) {
        Segment::maxHeight = p.yOffset + p.height;
      }
    }

    // safety check
    if (Segment::maxWidth * Segment::maxHeight > MAX_LEDS || Segment::maxWidth <= 1 || Segment::maxHeight <= 1) {
      DEBUG_PRINTLN(F("2D Bounds error."));
      isMatrix = false;
      Segment::maxWidth = _length;
      Segment::maxHeight = 1;
      panels = 0;
      panel.clear(); // release memory allocated by panels
      resetSegments();
      return;
    }

    customMappingSize = 0; // prevent use of mapping if anything goes wrong

    if (customMappingTable) delete[] customMappingTable;
    customMappingTable = new uint16_t[getLengthTotal()];

    if (customMappingTable) {
      customMappingSize = getLengthTotal();

      // fill with empty in case we don't fill the entire matrix
      unsigned matrixSize = Segment::maxWidth * Segment::maxHeight;
      for (unsigned i = 0; i<matrixSize; i++) customMappingTable[i] = 0xFFFFU;
      for (unsigned i = matrixSize; i<getLengthTotal(); i++) customMappingTable[i] = i; // trailing LEDs for ledmap (after matrix) if it exist

      // we will try to load a "gap" array (a JSON file)
      // the array has to have the same amount of values as mapping array (or larger)
      // "gap" array is used while building ledmap (mapping array)
      // and discarded afterwards as it has no meaning after the process
      // content of the file is just raw JSON array in the form of [val1,val2,val3,...]
      // there are no other "key":"value" pairs in it
      // allowed values are: -1 (missing pixel/no LED attached), 0 (inactive/unused pixel), 1 (active/used pixel)
      char    fileName[32]; strcpy_P(fileName, PSTR("/2d-gaps.json")); // reduce flash footprint
      bool    isFile = WLED_FS.exists(fileName);
      size_t  gapSize = 0;
      int8_t *gapTable = nullptr;

      if (isFile && requestJSONBufferLock(20)) {
        DEBUG_PRINT(F("Reading LED gap from "));
        DEBUG_PRINTLN(fileName);
        // read the array into global JSON buffer
        if (readObjectFromFile(fileName, nullptr, pDoc)) {
          // the array is similar to ledmap, except it has only 3 values:
          // -1 ... missing pixel (do not increase pixel count)
          //  0 ... inactive pixel (it does count, but should be mapped out (-1))
          //  1 ... active pixel (it will count and will be mapped)
          JsonArray map = pDoc->as<JsonArray>();
          gapSize = map.size();
          if (!map.isNull() && gapSize >= matrixSize) { // not an empty map
            gapTable = new int8_t[gapSize];
            if (gapTable) for (size_t i = 0; i < gapSize; i++) {
              gapTable[i] = constrain(map[i], -1, 1);
            }
          }
        }
        DEBUG_PRINTLN(F("Gaps loaded."));
        releaseJSONBufferLock();
      }

      unsigned x, y, pix=0; //pixel
      for (size_t pan = 0; pan < panel.size(); pan++) {
        Panel &p = panel[pan];
        unsigned h = p.vertical ? p.height : p.width;
        unsigned v = p.vertical ? p.width  : p.height;
        for (size_t j = 0; j < v; j++){
          for(size_t i = 0; i < h; i++) {
            y = (p.vertical?p.rightStart:p.bottomStart) ? v-j-1 : j;
            x = (p.vertical?p.bottomStart:p.rightStart) ? h-i-1 : i;
            x = p.serpentine && j%2 ? h-x-1 : x;
            size_t index = (p.yOffset + (p.vertical?x:y)) * Segment::maxWidth + p.xOffset + (p.vertical?y:x);
            if (!gapTable || (gapTable && gapTable[index] >  0)) customMappingTable[index] = pix; // a useful pixel (otherwise -1 is retained)
            if (!gapTable || (gapTable && gapTable[index] >= 0)) pix++; // not a missing pixel
          }
        }
      }

      // delete gap array as we no longer need it
      if (gapTable) delete[] gapTable;

      #ifdef WLED_DEBUG
      DEBUG_PRINT(F("Matrix ledmap:"));
      for (unsigned i=0; i<customMappingSize; i++) {
        if (!(i%Segment::maxWidth)) DEBUG_PRINTLN();
        DEBUG_PRINTF_P(PSTR("%4d,"), customMappingTable[i]);
      }
      DEBUG_PRINTLN();
      #endif
    } else { // memory allocation error
      DEBUG_PRINTLN(F("ERROR 2D LED map allocation error."));
      isMatrix = false;
      panels = 0;
      panel.clear();
      Segment::maxWidth = _length;
      Segment::maxHeight = 1;
      resetSegments();
    }
  }
#else
  isMatrix = false; // no matter what config says
#endif
}


///////////////////////////////////////////////////////////
// Segment:: routines
///////////////////////////////////////////////////////////

#ifndef WLED_DISABLE_2D

// XY(x,y) - gets pixel index within current segment (often used to reference leds[] array element)
uint16_t IRAM_ATTR_YN Segment::XY(int x, int y)
{
  unsigned width  = virtualWidth();   // segment width in logical pixels (can be 0 if segment is inactive)
  unsigned height = virtualHeight();  // segment height in logical pixels (is always >= 1)
  return isActive() ? (x%width) + (y%height) * width : 0;
}

void IRAM_ATTR_YN Segment::setPixelColorXY(int x, int y, uint32_t col)
{
  if (!isActive()) return; // not active
  if ((unsigned)x >= virtualWidth() || (unsigned)y >= virtualHeight() || x<0 || y<0) return;  // if pixel would fall out of virtual segment just exit

  uint8_t _bri_t = currentBri();
  if (_bri_t < 255) {
    col = color_fade(col, _bri_t);
  }

  if (reverse  ) x = virtualWidth()  - x - 1;
  if (reverse_y) y = virtualHeight() - y - 1;
  if (transpose) { std::swap(x,y); } // swap X & Y if segment transposed

  x *= groupLength(); // expand to physical pixels
  y *= groupLength(); // expand to physical pixels

  int W = width();
  int H = height();
  if (x >= W || y >= H) return;  // if pixel would fall out of segment just exit

  uint32_t tmpCol = col;
  for (int j = 0; j < grouping; j++) {   // groupping vertically
    for (int g = 0; g < grouping; g++) { // groupping horizontally
      int xX = (x+g), yY = (y+j);
      if (xX >= W || yY >= H) continue;  // we have reached one dimension's end

#ifndef WLED_DISABLE_MODE_BLEND
      // if blending modes, blend with underlying pixel
      if (_modeBlend) tmpCol = color_blend(strip.getPixelColorXY(start + xX, startY + yY), col, 0xFFFFU - progress(), true);
#endif

      strip.setPixelColorXY(start + xX, startY + yY, tmpCol);

      if (mirror) { //set the corresponding horizontally mirrored pixel
        if (transpose) strip.setPixelColorXY(start + xX, startY + height() - yY - 1, tmpCol);
        else           strip.setPixelColorXY(start + width() - xX - 1, startY + yY, tmpCol);
      }
      if (mirror_y) { //set the corresponding vertically mirrored pixel
        if (transpose) strip.setPixelColorXY(start + width() - xX - 1, startY + yY, tmpCol);
        else           strip.setPixelColorXY(start + xX, startY + height() - yY - 1, tmpCol);
      }
      if (mirror_y && mirror) { //set the corresponding vertically AND horizontally mirrored pixel
        strip.setPixelColorXY(start + width() - xX - 1, startY + height() - yY - 1, tmpCol);
      }
    }
  }
}

#ifdef WLED_USE_AA_PIXELS
// anti-aliased version of setPixelColorXY()
void Segment::setPixelColorXY(float x, float y, uint32_t col, bool aa)
{
  if (!isActive()) return; // not active
  if (x<0.0f || x>1.0f || y<0.0f || y>1.0f) return; // not normalized

  const unsigned cols = virtualWidth();
  const unsigned rows = virtualHeight();

  float fX = x * (cols-1);
  float fY = y * (rows-1);
  if (aa) {
    unsigned xL = roundf(fX-0.49f);
    unsigned xR = roundf(fX+0.49f);
    unsigned yT = roundf(fY-0.49f);
    unsigned yB = roundf(fY+0.49f);
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
#endif

// returns RGBW values of pixel
uint32_t IRAM_ATTR_YN Segment::getPixelColorXY(int x, int y) const {
  if (!isActive()) return 0; // not active
  if ((unsigned)x >= virtualWidth() || (unsigned)y >= virtualHeight() || x<0 || y<0) return 0;  // if pixel would fall out of virtual segment just exit
  if (reverse  ) x = virtualWidth()  - x - 1;
  if (reverse_y) y = virtualHeight() - y - 1;
  if (transpose) { std::swap(x,y); } // swap X & Y if segment transposed
  x *= groupLength(); // expand to physical pixels
  y *= groupLength(); // expand to physical pixels
  if (x >= width() || y >= height()) return 0;
  return strip.getPixelColorXY(start + x, startY + y);
}

// blurRow: perform a blur on a row of a rectangular matrix
void Segment::blurRow(uint32_t row, fract8 blur_amount, bool smear){
  if (!isActive() || blur_amount == 0) return; // not active
  const unsigned cols = virtualWidth();
  const unsigned rows = virtualHeight();

  if (row >= rows) return;
  // blur one row
  uint8_t keep = smear ? 255 : 255 - blur_amount;
  uint8_t seep = blur_amount >> 1;
  uint32_t carryover = BLACK;
  uint32_t lastnew;
  uint32_t last;
  uint32_t curnew = BLACK;
  for (unsigned x = 0; x < cols; x++) {
    uint32_t cur = getPixelColorXY(x, row);
    uint32_t part = color_fade(cur, seep);
    curnew = color_fade(cur, keep);
    if (x > 0) {
      if (carryover)
        curnew = color_add(curnew, carryover, true);
      uint32_t prev = color_add(lastnew, part, true);
      if (last != prev) // optimization: only set pixel if color has changed
        setPixelColorXY(x - 1, row, prev);
    } else // first pixel
      setPixelColorXY(x, row, curnew);
    lastnew = curnew;
    last = cur; // save original value for comparison on next iteration
    carryover = part;
  }
  setPixelColorXY(cols-1, row, curnew); // set last pixel
}

// blurCol: perform a blur on a column of a rectangular matrix
void Segment::blurCol(uint32_t col, fract8 blur_amount, bool smear) {
  if (!isActive() || blur_amount == 0) return; // not active
  const unsigned cols = virtualWidth();
  const unsigned rows = virtualHeight();

  if (col >= cols) return;
  // blur one column
  uint8_t keep = smear ? 255 : 255 - blur_amount;
  uint8_t seep = blur_amount >> 1;
  uint32_t carryover = BLACK;
  uint32_t lastnew;
  uint32_t last;
  uint32_t curnew = BLACK;
  for (unsigned y = 0; y < rows; y++) {
    uint32_t cur = getPixelColorXY(col, y);
    uint32_t part = color_fade(cur, seep);
    curnew = color_fade(cur, keep);
    if (y > 0) {
      if (carryover)
        curnew = color_add(curnew, carryover, true);
      uint32_t prev = color_add(lastnew, part, true);      
      if (last != prev) // optimization: only set pixel if color has changed
        setPixelColorXY(col, y - 1, prev);
    } else // first pixel
      setPixelColorXY(col, y, curnew);
    lastnew = curnew;
    last = cur; //save original value for comparison on next iteration
    carryover = part;        
  }
  setPixelColorXY(col, rows - 1, curnew);
}

void Segment::blur2D(uint8_t blur_amount, bool smear) {
  if (!isActive() || blur_amount == 0) return; // not active
  const unsigned cols = virtualWidth();
  const unsigned rows = virtualHeight();

  const uint8_t keep = smear ? 255 : 255 - blur_amount;
  const uint8_t seep = blur_amount >> (1 + smear);
  uint32_t lastnew;
  uint32_t last;
  for (unsigned row = 0; row < rows; row++) {
    uint32_t carryover = BLACK;
    uint32_t curnew = BLACK;
    for (unsigned x = 0; x < cols; x++) {
      uint32_t cur = getPixelColorXY(x, row);
      uint32_t part = color_fade(cur, seep);
      curnew = color_fade(cur, keep);
      if (x > 0) {
        if (carryover) curnew = color_add(curnew, carryover, true);
        uint32_t prev = color_add(lastnew, part, true);
        // optimization: only set pixel if color has changed
        if (last != prev) setPixelColorXY(x - 1, row, prev);
      } else setPixelColorXY(x, row, curnew); // first pixel
      lastnew = curnew;
      last = cur; // save original value for comparison on next iteration
      carryover = part;
    }
    setPixelColorXY(cols-1, row, curnew); // set last pixel
  }
  for (unsigned col = 0; col < cols; col++) {
    uint32_t carryover = BLACK;
    uint32_t curnew = BLACK;
    for (unsigned y = 0; y < rows; y++) {
      uint32_t cur = getPixelColorXY(col, y);
      uint32_t part = color_fade(cur, seep);
      curnew = color_fade(cur, keep);
      if (y > 0) {
        if (carryover) curnew = color_add(curnew, carryover, true);
        uint32_t prev = color_add(lastnew, part, true);      
        // optimization: only set pixel if color has changed
        if (last != prev) setPixelColorXY(col, y - 1, prev);
      } else setPixelColorXY(col, y, curnew); // first pixel
      lastnew = curnew;
      last = cur; //save original value for comparison on next iteration
      carryover = part;        
    }
    setPixelColorXY(col, rows - 1, curnew);
  }
}

// 2D Box blur
void Segment::box_blur(unsigned radius, bool smear) {
  if (!isActive() || radius == 0) return; // not active
  if (radius > 3) radius = 3;
  const unsigned d = (1 + 2*radius) * (1 + 2*radius); // averaging divisor
  const unsigned cols = virtualWidth();
  const unsigned rows = virtualHeight();
  uint16_t *tmpRSum = new uint16_t[cols*rows];
  uint16_t *tmpGSum = new uint16_t[cols*rows];
  uint16_t *tmpBSum = new uint16_t[cols*rows];
  uint16_t *tmpWSum = new uint16_t[cols*rows];
  // fill summed-area table (https://en.wikipedia.org/wiki/Summed-area_table)
  for (unsigned x = 0; x < cols; x++) {
    unsigned rS, gS, bS, wS;
    unsigned index;
    rS = gS = bS = wS = 0;
    for (unsigned y = 0; y < rows; y++) {
      index = x * cols + y;
      if (x > 0) {
        unsigned index2 = (x - 1) * cols + y;
        tmpRSum[index] = tmpRSum[index2];
        tmpGSum[index] = tmpGSum[index2];
        tmpBSum[index] = tmpBSum[index2];
        tmpWSum[index] = tmpWSum[index2];
      } else {
        tmpRSum[index] = 0;
        tmpGSum[index] = 0;
        tmpBSum[index] = 0;
        tmpWSum[index] = 0;
      }
      uint32_t c = getPixelColorXY(x, y);
      rS += R(c);
      gS += G(c);
      bS += B(c);
      wS += W(c);
      tmpRSum[index] += rS;
      tmpGSum[index] += gS;
      tmpBSum[index] += bS;
      tmpWSum[index] += wS;
    }
  }
  // do a box blur using pre-calculated sums
  for (unsigned x = 0; x < cols; x++) {
    for (unsigned y = 0; y < rows; y++) {
      // sum = D + A - B - C where k = (x,y)
      // +----+-+---- (x)
      // |    | |
      // +----A-B
      // |    |k|
      // +----C-D
      // |
      //(y)
      unsigned x0 = x < radius ? 0 : x - radius;
      unsigned y0 = y < radius ? 0 : y - radius;
      unsigned x1 = x >= cols - radius ? cols - 1 : x + radius;
      unsigned y1 = y >= rows - radius ? rows - 1 : y + radius;
      unsigned A = x0 * cols + y0;
      unsigned B = x1 * cols + y0;
      unsigned C = x0 * cols + y1;
      unsigned D = x1 * cols + y1;
      unsigned r = tmpRSum[D] + tmpRSum[A] - tmpRSum[C] - tmpRSum[B];
      unsigned g = tmpGSum[D] + tmpGSum[A] - tmpGSum[C] - tmpGSum[B];
      unsigned b = tmpBSum[D] + tmpBSum[A] - tmpBSum[C] - tmpBSum[B];
      unsigned w = tmpWSum[D] + tmpWSum[A] - tmpWSum[C] - tmpWSum[B];
      setPixelColorXY(x, y, RGBW32(r/d, g/d, b/d, w/d));
    }
  }
  delete[] tmpRSum;
  delete[] tmpGSum;
  delete[] tmpBSum;
  delete[] tmpWSum;
}

void Segment::moveX(int8_t delta, bool wrap) {
  if (!isActive()) return; // not active
  const int cols = virtualWidth();
  const int rows = virtualHeight();
  if (!delta || abs(delta) >= cols) return;
  uint32_t newPxCol[cols];
  for (int y = 0; y < rows; y++) {
    if (delta > 0) {
      for (int x = 0; x < cols-delta; x++)    newPxCol[x] = getPixelColorXY((x + delta), y);
      for (int x = cols-delta; x < cols; x++) newPxCol[x] = getPixelColorXY(wrap ? (x + delta) - cols : x, y);
    } else {
      for (int x = cols-1; x >= -delta; x--) newPxCol[x] = getPixelColorXY((x + delta), y);
      for (int x = -delta-1; x >= 0; x--)    newPxCol[x] = getPixelColorXY(wrap ? (x + delta) + cols : x, y);
    }
    for (int x = 0; x < cols; x++) setPixelColorXY(x, y, newPxCol[x]);
  }
}

void Segment::moveY(int8_t delta, bool wrap) {
  if (!isActive()) return; // not active
  const int cols = virtualWidth();
  const int rows = virtualHeight();
  if (!delta || abs(delta) >= rows) return;
  uint32_t newPxCol[rows];
  for (int x = 0; x < cols; x++) {
    if (delta > 0) {
      for (int y = 0; y < rows-delta; y++)    newPxCol[y] = getPixelColorXY(x, (y + delta));
      for (int y = rows-delta; y < rows; y++) newPxCol[y] = getPixelColorXY(x, wrap ? (y + delta) - rows : y);
    } else {
      for (int y = rows-1; y >= -delta; y--) newPxCol[y] = getPixelColorXY(x, (y + delta));
      for (int y = -delta-1; y >= 0; y--)    newPxCol[y] = getPixelColorXY(x, wrap ? (y + delta) + rows : y);
    }
    for (int y = 0; y < rows; y++) setPixelColorXY(x, y, newPxCol[y]);
  }
}

// move() - move all pixels in desired direction delta number of pixels
// @param dir direction: 0=left, 1=left-up, 2=up, 3=right-up, 4=right, 5=right-down, 6=down, 7=left-down
// @param delta number of pixels to move
// @param wrap around
void Segment::move(uint8_t dir, uint8_t delta, bool wrap) {
  if (delta==0) return;
  switch (dir) {
    case 0: moveX( delta, wrap);                      break;
    case 1: moveX( delta, wrap); moveY( delta, wrap); break;
    case 2:                      moveY( delta, wrap); break;
    case 3: moveX(-delta, wrap); moveY( delta, wrap); break;
    case 4: moveX(-delta, wrap);                      break;
    case 5: moveX(-delta, wrap); moveY(-delta, wrap); break;
    case 6:                      moveY(-delta, wrap); break;
    case 7: moveX( delta, wrap); moveY(-delta, wrap); break;
  }
}

void Segment::drawCircle(uint16_t cx, uint16_t cy, uint8_t radius, uint32_t col, bool soft) {
  if (!isActive() || radius == 0) return; // not active
  if (soft) {
    // Xiaolin Wu’s algorithm
    int rsq = radius*radius;
    int x = 0;
    int y = radius;
    unsigned oldFade = 0;
    while (x < y) {
      float yf = sqrtf(float(rsq - x*x)); // needs to be floating point
      unsigned fade = float(0xFFFF) * (ceilf(yf) - yf); // how much color to keep
      if (oldFade > fade) y--;
      oldFade = fade;
      setPixelColorXY(cx+x, cy+y, color_blend(col, getPixelColorXY(cx+x, cy+y), fade, true));
      setPixelColorXY(cx-x, cy+y, color_blend(col, getPixelColorXY(cx-x, cy+y), fade, true));
      setPixelColorXY(cx+x, cy-y, color_blend(col, getPixelColorXY(cx+x, cy-y), fade, true));
      setPixelColorXY(cx-x, cy-y, color_blend(col, getPixelColorXY(cx-x, cy-y), fade, true));
      setPixelColorXY(cx+y, cy+x, color_blend(col, getPixelColorXY(cx+y, cy+x), fade, true));
      setPixelColorXY(cx-y, cy+x, color_blend(col, getPixelColorXY(cx-y, cy+x), fade, true));
      setPixelColorXY(cx+y, cy-x, color_blend(col, getPixelColorXY(cx+y, cy-x), fade, true));
      setPixelColorXY(cx-y, cy-x, color_blend(col, getPixelColorXY(cx-y, cy-x), fade, true));
      setPixelColorXY(cx+x, cy+y-1, color_blend(getPixelColorXY(cx+x, cy+y-1), col, fade, true));
      setPixelColorXY(cx-x, cy+y-1, color_blend(getPixelColorXY(cx-x, cy+y-1), col, fade, true));
      setPixelColorXY(cx+x, cy-y+1, color_blend(getPixelColorXY(cx+x, cy-y+1), col, fade, true));
      setPixelColorXY(cx-x, cy-y+1, color_blend(getPixelColorXY(cx-x, cy-y+1), col, fade, true));
      setPixelColorXY(cx+y-1, cy+x, color_blend(getPixelColorXY(cx+y-1, cy+x), col, fade, true));
      setPixelColorXY(cx-y+1, cy+x, color_blend(getPixelColorXY(cx-y+1, cy+x), col, fade, true));
      setPixelColorXY(cx+y-1, cy-x, color_blend(getPixelColorXY(cx+y-1, cy-x), col, fade, true));
      setPixelColorXY(cx-y+1, cy-x, color_blend(getPixelColorXY(cx-y+1, cy-x), col, fade, true));
      x++;
    }
  } else {
    // Bresenham’s Algorithm
    int d = 3 - (2*radius);
    int y = radius, x = 0;
    while (y >= x) {
      setPixelColorXY(cx+x, cy+y, col);
      setPixelColorXY(cx-x, cy+y, col);
      setPixelColorXY(cx+x, cy-y, col);
      setPixelColorXY(cx-x, cy-y, col);
      setPixelColorXY(cx+y, cy+x, col);
      setPixelColorXY(cx-y, cy+x, col);
      setPixelColorXY(cx+y, cy-x, col);
      setPixelColorXY(cx-y, cy-x, col);
      x++;
      if (d > 0) {
        y--;
        d += 4 * (x - y) + 10;
      } else {
        d += 4 * x + 6;
      }
    }
  }
}

// by stepko, taken from https://editor.soulmatelights.com/gallery/573-blobs
void Segment::fillCircle(uint16_t cx, uint16_t cy, uint8_t radius, uint32_t col, bool soft) {
  if (!isActive() || radius == 0) return; // not active
  // draw soft bounding circle
  if (soft) drawCircle(cx, cy, radius, col, soft);
  // fill it
  const int cols = virtualWidth();
  const int rows = virtualHeight();
  for (int y = -radius; y <= radius; y++) {
    for (int x = -radius; x <= radius; x++) {
      if (x * x + y * y <= radius * radius &&
          int(cx)+x>=0 && int(cy)+y>=0 &&
          int(cx)+x<cols && int(cy)+y<rows)
        setPixelColorXY(cx + x, cy + y, col);
    }
  }
}

//line function
void Segment::drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint32_t c, bool soft) {
  if (!isActive()) return; // not active
  const int cols = virtualWidth();
  const int rows = virtualHeight();
  if (x0 >= cols || x1 >= cols || y0 >= rows || y1 >= rows) return;

  const int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1; // x distance & step
  const int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; // y distance & step

  // single pixel (line length == 0)
  if (dx+dy == 0) {
    setPixelColorXY(x0, y0, c);
    return;
  }

  if (soft) {
    // Xiaolin Wu’s algorithm
    const bool steep = dy > dx;
    if (steep) {
      // we need to go along longest dimension
      std::swap(x0,y0);
      std::swap(x1,y1);
    }
    if (x0 > x1) {
      // we need to go in increasing fashion
      std::swap(x0,x1);
      std::swap(y0,y1);
    }
    float gradient = x1-x0 == 0 ? 1.0f : float(y1-y0) / float(x1-x0);
    float intersectY = y0;
    for (int x = x0; x <= x1; x++) {
      unsigned keep = float(0xFFFF) * (intersectY-int(intersectY)); // how much color to keep
      unsigned seep = 0xFFFF - keep; // how much background to keep
      int y = int(intersectY);
      if (steep) std::swap(x,y);  // temporaryly swap if steep
      // pixel coverage is determined by fractional part of y co-ordinate
      setPixelColorXY(x, y, color_blend(c, getPixelColorXY(x, y), keep, true));
      setPixelColorXY(x+int(steep), y+int(!steep), color_blend(c, getPixelColorXY(x+int(steep), y+int(!steep)), seep, true));
      intersectY += gradient;
      if (steep) std::swap(x,y);  // restore if steep
    }
  } else {
    // Bresenham's algorithm
    int err = (dx>dy ? dx : -dy)/2;   // error direction
    for (;;) {
      setPixelColorXY(x0, y0, c);
      if (x0==x1 && y0==y1) break;
      int e2 = err;
      if (e2 >-dx) { err -= dy; x0 += sx; }
      if (e2 < dy) { err += dx; y0 += sy; }
    }
  }
}

#include "src/font/console_font_4x6.h"
#include "src/font/console_font_5x8.h"
#include "src/font/console_font_5x12.h"
#include "src/font/console_font_6x8.h"
#include "src/font/console_font_7x9.h"

// draws a raster font character on canvas
// only supports: 4x6=24, 5x8=40, 5x12=60, 6x8=48 and 7x9=63 fonts ATM
void Segment::drawCharacter(unsigned char chr, int16_t x, int16_t y, uint8_t w, uint8_t h, uint32_t color, uint32_t col2, int8_t rotate) {
  if (!isActive()) return; // not active
  if (chr < 32 || chr > 126) return; // only ASCII 32-126 supported
  chr -= 32; // align with font table entries
  const int cols = virtualWidth();
  const int rows = virtualHeight();
  const int font = w*h;

  CRGB col = CRGB(color);
  CRGBPalette16 grad = CRGBPalette16(col, col2 ? CRGB(col2) : col);

  //if (w<5 || w>6 || h!=8) return;
  for (int i = 0; i<h; i++) { // character height
    uint8_t bits = 0;
    switch (font) {
      case 24: bits = pgm_read_byte_near(&console_font_4x6[(chr * h) + i]); break;  // 5x8 font
      case 40: bits = pgm_read_byte_near(&console_font_5x8[(chr * h) + i]); break;  // 5x8 font
      case 48: bits = pgm_read_byte_near(&console_font_6x8[(chr * h) + i]); break;  // 6x8 font
      case 63: bits = pgm_read_byte_near(&console_font_7x9[(chr * h) + i]); break;  // 7x9 font
      case 60: bits = pgm_read_byte_near(&console_font_5x12[(chr * h) + i]); break; // 5x12 font
      default: return;
    }
    col = ColorFromPalette(grad, (i+1)*255/h, 255, NOBLEND);
    for (int j = 0; j<w; j++) { // character width
      int x0, y0;
      switch (rotate) {
        case -1: x0 = x + (h-1) - i; y0 = y + (w-1) - j; break; // -90 deg
        case -2:
        case  2: x0 = x + j;         y0 = y + (h-1) - i; break; // 180 deg
        case  1: x0 = x + i;         y0 = y + j;         break; // +90 deg
        default: x0 = x + (w-1) - j; y0 = y + i;         break; // no rotation
      }
      if (x0 < 0 || x0 >= cols || y0 < 0 || y0 >= rows) continue; // drawing off-screen
      if (((bits>>(j+(8-w))) & 0x01)) { // bit set
        setPixelColorXY(x0, y0, col);
      }
    }
  }
}

#define WU_WEIGHT(a,b) ((uint8_t) (((a)*(b)+(a)+(b))>>8))
void Segment::wu_pixel(uint32_t x, uint32_t y, CRGB c) {      //awesome wu_pixel procedure by reddit u/sutaburosu
  if (!isActive()) return; // not active
  // extract the fractional parts and derive their inverses
  unsigned xx = x & 0xff, yy = y & 0xff, ix = 255 - xx, iy = 255 - yy;
  // calculate the intensities for each affected pixel
  uint8_t wu[4] = {WU_WEIGHT(ix, iy), WU_WEIGHT(xx, iy),
                   WU_WEIGHT(ix, yy), WU_WEIGHT(xx, yy)};
  // multiply the intensities by the colour, and saturating-add them to the pixels
  for (int i = 0; i < 4; i++) {
    int wu_x = (x >> 8) + (i & 1);        // precalculate x
    int wu_y = (y >> 8) + ((i >> 1) & 1); // precalculate y
    CRGB led = getPixelColorXY(wu_x, wu_y);
    CRGB oldLed = led;
    led.r = qadd8(led.r, c.r * wu[i] >> 8);
    led.g = qadd8(led.g, c.g * wu[i] >> 8);
    led.b = qadd8(led.b, c.b * wu[i] >> 8);
    if (led != oldLed) setPixelColorXY(wu_x, wu_y, led); // don't repaint if same color
  }
}
#undef WU_WEIGHT

#endif // WLED_DISABLE_2D
