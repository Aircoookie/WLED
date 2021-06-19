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

  // Draw directly from file stored in RGB565 format
  bool drawBin(const char *filename) {
    fs::File bmpFS;


    // Open requested file on SD card
    bmpFS = WLED_FS.open(filename, "r");

    if (!bmpFS)
    {
      Serial.print(F("File not found: "));
      Serial.println(filename);
      return(false);
    }

    size_t sz = bmpFS.size();
    if (sz <= 64800)
    {
      bool oldSwapBytes = getSwapBytes();
      setSwapBytes(true);

      int16_t h = sz / (135 * 2);

      //draw img that is shorter than 240pix into the center
      int16_t y = (height() - h) /2;

      bmpFS.read((uint8_t *) output_buffer,sz);

      if (!realtimeMode || realtimeOverride) strip.service();

      pushImage(0, y, 135, h, (uint16_t *)output_buffer);

      setSwapBytes(oldSwapBytes);
    }

    bmpFS.close();

    return(true);
  }

  bool drawBmp(const char *filename) {
    fs::File bmpFS;

    // Open requested file on SD card
    bmpFS = WLED_FS.open(filename, "r");

    if (!bmpFS)
    {
      Serial.print(F("File not found: "));
      Serial.println(filename);
      return(false);
    }

    uint32_t seekOffset;
    int16_t w, h, row;
    uint8_t  r, g, b;

    uint16_t magic = read16(bmpFS);
    if (magic == 0xFFFF) {
      Serial.println(F("BMP not found!"));
      bmpFS.close();
      return(false);
    }
    
    if (magic != 0x4D42) {
      Serial.print(F("File not a BMP. Magic: "));
      Serial.println(magic);
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

    bool oldSwapBytes = getSwapBytes();
    setSwapBytes(true);
    bmpFS.seek(seekOffset);

    uint16_t padding = (4 - ((w * 3) & 3)) & 3;
    uint8_t lineBuffer[w * 3 + padding];
    
    uint8_t serviceStrip = (!realtimeMode || realtimeOverride) ? 7 : 0;
    // row is decremented as the BMP image is drawn bottom up
    for (row = h-1; row >= 0; row--) {
      if ((row & 0b00000111) == serviceStrip) strip.service(); //still refresh backlight to mitigate stutter every few rows
      bmpFS.read(lineBuffer, sizeof(lineBuffer));
      uint8_t*  bptr = lineBuffer;
      
      // Convert 24 to 16 bit colours while copying to output buffer.
      for (uint16_t col = 0; col < w; col++)
      {
        b = *bptr++;
        g = *bptr++;
        r = *bptr++;
        output_buffer[row][col] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
      }
    }
    
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
      fillScreen(TFT_BLACK);
    }
    else {
      // Filenames are no bigger than "255.bmp\0"
      char file_name[10];
      sprintf(file_name, "/%d.bmp", digits[digit]);
      if (WLED_FS.exists(file_name)) {
        drawBmp(file_name);
      } else {
        sprintf(file_name, "/%d.bin", digits[digit]);
        drawBin(file_name);
      }
    }
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
