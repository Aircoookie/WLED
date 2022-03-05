#ifndef TFTS_H
#define TFTS_H

#include "wled.h"
#include <FS.h>

#include <TFT_eSPI.h>
#include "Hardware.h"
#include "ChipSelect.h"

class TFTs : public TFT_eSPI {
private:
  uint8_t digits[NUM_DIGITS];


  // These read 16- and 32-bit types from the SD card file.
  // BMP data is stored little-endian, Arduino is little-endian too.
  // May need to reverse subscript order if porting elsewhere.

  uint16_t read16(fs::File &f) {
    uint16_t result;
    ((uint8_t *)&result)[0] = f.read(); // LSB
    ((uint8_t *)&result)[1] = f.read(); // MSB
    return result;
  }

  uint32_t read32(fs::File &f) {
    uint32_t result;
    ((uint8_t *)&result)[0] = f.read(); // LSB
    ((uint8_t *)&result)[1] = f.read();
    ((uint8_t *)&result)[2] = f.read();
    ((uint8_t *)&result)[3] = f.read(); // MSB
    return result;
  }

  uint16_t output_buffer[TFT_HEIGHT][TFT_WIDTH];
  

  // These BMP functions are stolen directly from the TFT_SPIFFS_BMP example in the TFT_eSPI library.
  // Unfortunately, they aren't part of the library itself, so I had to copy them.
  // I've modified drawBmp to buffer the whole image at once instead of doing it line-by-line.

  //// BEGIN STOLEN CODE

  // Draw directly from file stored in RGB565 format. Fastest
  bool drawBin(const char *filename) {
    fs::File bmpFS;

    // Open requested file on SD card
    bmpFS = WLED_FS.open(filename, "r");

    size_t sz = bmpFS.size();
    if (sz > 64800) {
      bmpFS.close();
      return(false);
    }

    uint16_t r, g, b, dimming = 255;
    int16_t w, h, y, row, col;

    //draw img that is shorter than 240pix into the center
    w = 135;
    h = sz / (w * 2);
    y = (height() - h) /2;
    
    uint8_t lineBuffer[w * 2];

    if (!realtimeMode || realtimeOverride) strip.service();

    #ifdef ELEKSTUBE_DIMMING
    dimming=bri;
    #endif

    // 0,0 coordinates are top left
    for (row = 0; row < h; row++) {

      bmpFS.read(lineBuffer, sizeof(lineBuffer));
      uint8_t PixM, PixL;
      
      // Colors are already in 16-bit R5, G6, B5 format
      for (col = 0; col < w; col++)
      {
        if (dimming == 255) { // not needed, copy directly
          output_buffer[row][col] = (lineBuffer[col*2+1] << 8) | (lineBuffer[col*2]);
        } else {
          // 16 BPP pixel format: R5, G6, B5 ; bin: RRRR RGGG GGGB BBBB
          PixM = lineBuffer[col*2+1];
          PixL = lineBuffer[col*2];
          // align to 8-bit value (MSB left aligned)
          r = (PixM) & 0xF8;
          g = ((PixM << 5) | (PixL >> 3)) & 0xFC;
          b = (PixL << 3) & 0xF8;
          r *= dimming;
          g *= dimming;
          b *= dimming;
          r = r >> 8;
          g = g >> 8;
          b = b >> 8;
          output_buffer[row][col] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }
      }
    }

    bool oldSwapBytes = getSwapBytes();
    setSwapBytes(true);
    pushImage(0, y, 135, h, (uint16_t *)output_buffer);
    setSwapBytes(oldSwapBytes);

    bmpFS.close();

    return(true);
  }

  bool drawBmp(const char *filename) {
    fs::File bmpFS;

    // Open requested file on SD card
    bmpFS = WLED_FS.open(filename, "r");

    uint32_t seekOffset;
    int16_t w, h, row;
    uint16_t r, g, b, dimming = 255;

    uint16_t magic = read16(bmpFS);
    if (magic == 0xFFFF) {
      Serial.println(F("BMP not found!"));
      bmpFS.close();
      return(false);
    }

    read32(bmpFS);
    read32(bmpFS);
    seekOffset = read32(bmpFS);
    read32(bmpFS);
    w = read32(bmpFS);
    h = read32(bmpFS);

    if ((read16(bmpFS) != 1) || (read16(bmpFS) != 24) || (read32(bmpFS) != 0)) {
      Serial.println(F("BMP format not recognized."));
      bmpFS.close();
      return(false);
    }

    //draw img that is shorter than 240pix into the center
    int16_t y = (height() - h) /2;

    bmpFS.seek(seekOffset);

    uint16_t padding = (4 - ((w * 3) & 3)) & 3;
    uint8_t lineBuffer[w * 3 + padding];
    
    uint8_t serviceStrip = (!realtimeMode || realtimeOverride) ? 7 : 0;
    // row is decremented as the BMP image is drawn bottom up
    for (row = h-1; row >= 0; row--) {
      if ((row & 0b00000111) == serviceStrip) strip.service(); //still refresh backlight to mitigate stutter every few rows
      bmpFS.read(lineBuffer, sizeof(lineBuffer));
      uint8_t*  bptr = lineBuffer;
      
      #ifdef ELEKSTUBE_DIMMING
      dimming=bri;
      #endif
      // Convert 24 to 16 bit colours while copying to output buffer.
      for (uint16_t col = 0; col < w; col++)
      {
        b = *bptr++;
        g = *bptr++;
        r = *bptr++;
        if (dimming != 255) { // only dimm when needed
          b *= dimming;
          g *= dimming;
          r *= dimming;
          b = b >> 8;
          g = g >> 8;
          r = r >> 8;
        }
        output_buffer[row][col] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
      }
    }
    
    bool oldSwapBytes = getSwapBytes();
    setSwapBytes(true);
    pushImage(0, y, w, h, (uint16_t *)output_buffer);
    setSwapBytes(oldSwapBytes);

    bmpFS.close();
    return(true);
  }

  bool drawClk(const char *filename) {
    fs::File bmpFS;

    // Open requested file on SD card
    bmpFS = WLED_FS.open(filename, "r");

    if (!bmpFS)
    {
      Serial.print("File not found: ");
      Serial.println(filename);
      return(false);
    }

    uint16_t  r, g, b, dimming = 255, magic;
    int16_t w, h, row, col;
    
    magic = read16(bmpFS);
    if (magic != 0x4B43) { // look for "CK" header
      Serial.print(F("File not a CLK. Magic: "));
      Serial.println(magic);
      bmpFS.close();
      return(false);
    }

    w = read16(bmpFS);
    h = read16(bmpFS);
    int16_t x = (width() - w) / 2;
    int16_t y = (height() - h) / 2;
    
    uint8_t lineBuffer[w * 2];
    
    #ifdef ELEKSTUBE_DIMMING
    dimming=bri;
    #endif
    
    if (!realtimeMode || realtimeOverride) strip.service();

    // 0,0 coordinates are top left
    for (row = 0; row < h; row++) {

      bmpFS.read(lineBuffer, sizeof(lineBuffer));
      uint8_t PixM, PixL;
      
      // Colors are already in 16-bit R5, G6, B5 format
      for (col = 0; col < w; col++)
      {
        if (dimming == 255) { // not needed, copy directly
          output_buffer[row][col+x] = (lineBuffer[col*2+1] << 8) | (lineBuffer[col*2]);
        } else {
          // 16 BPP pixel format: R5, G6, B5 ; bin: RRRR RGGG GGGB BBBB
          PixM = lineBuffer[col*2+1];
          PixL = lineBuffer[col*2];
          // align to 8-bit value (MSB left aligned)
          r = (PixM) & 0xF8;
          g = ((PixM << 5) | (PixL >> 3)) & 0xFC;
          b = (PixL << 3) & 0xF8;
          r *= dimming;
          g *= dimming;
          b *= dimming;
          r = r >> 8;
          g = g >> 8;
          b = b >> 8;
          output_buffer[row][col+x] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }
      }
    }

    bool oldSwapBytes = getSwapBytes();
    setSwapBytes(true);
    pushImage(0, y, w, h, (uint16_t *)output_buffer);
    setSwapBytes(oldSwapBytes);

    bmpFS.close();
    return(true);
  }


public:
  TFTs() : TFT_eSPI(), chip_select()
    { for (uint8_t digit=0; digit < NUM_DIGITS; digit++) digits[digit] = 0; }

  // no == Do not send to TFT. yes == Send to TFT if changed. force == Send to TFT.
  enum show_t { no, yes, force };
  // A digit of 0xFF means blank the screen.
  const static uint8_t blanked = 255;
  
  void begin() {
    pinMode(TFT_ENABLE_PIN, OUTPUT);
    digitalWrite(TFT_ENABLE_PIN, HIGH); //enable displays on boot

    // Start with all displays selected.
    chip_select.begin();
    chip_select.setAll();

    // Initialize the super class.
    init();
  }

  void showDigit(uint8_t digit) {
    chip_select.setDigit(digit);

    if (digits[digit] == blanked) {
      fillScreen(TFT_BLACK); return;
    }

    // Filenames are no bigger than "255.bmp\0"
    char file_name[10];
    // Fastest, raw RGB565
    sprintf(file_name, "/%d.bin", digits[digit]);
    if (WLED_FS.exists(file_name)) {
      drawBin(file_name); return;
    }
    // Fast, see https://github.com/aly-fly/EleksTubeHAX on how to create this clk format
    sprintf(file_name, "/%d.clk", digits[digit]);
    if (WLED_FS.exists(file_name)) {
      drawClk(file_name); return;
    }
    // Slow, regular RGB888 bmp
    sprintf(file_name, "/%d.bmp", digits[digit]);
    drawBmp(file_name);
  } 

  void setDigit(uint8_t digit, uint8_t value, show_t show=yes) {
    uint8_t old_value = digits[digit];
    digits[digit] = value; 

    if (show != no && (old_value != value || show == force)) {
      showDigit(digit);
    }
  }
  uint8_t getDigit(uint8_t digit)                 { return digits[digit]; }

  void showAllDigits()               { for (uint8_t digit=0; digit < NUM_DIGITS; digit++) showDigit(digit); }

  // Making chip_select public so we don't have to proxy all methods, and the caller can just use it directly.
  ChipSelect chip_select;
};

#endif // TFTS_H
