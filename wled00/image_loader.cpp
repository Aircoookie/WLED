#include "GifDecoder.h"
#include "wled.h"

File file;
char lastFilename[34] = "/";
GifDecoder<64, 64, 12, true> decoder;
bool gifDecodeFailed = false;
long lastFrameDisplayTime = 0, currentFrameDelay = 0;

bool fileSeekCallback(unsigned long position) {
  return file.seek(position);
}

unsigned long filePositionCallback(void) {
  return file.position();
}

int fileReadCallback(void) {
  return file.read();
}

int fileReadBlockCallback(void * buffer, int numberOfBytes) {
  return file.read((uint8_t*)buffer, numberOfBytes);
}

int fileSizeCallback(void) {
  return file.size();
}

bool openGif(const char *filename) {
  file = WLED_FS.open(filename, "r");

  if (!file) return false;
  return true;
}

Segment* activeSeg;
uint16_t gifWidth, gifHeight;
uint16_t fillPixX, fillPixY;

void screenClearCallback(void) {
  activeSeg->fill(0);
}

void updateScreenCallback(void) {}

void drawPixelCallback(int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue) {
  // simple nearest-neighbor scaling 
  int16_t outY = y * activeSeg->height() / gifHeight;
  int16_t outX = x * activeSeg->width()  / gifWidth;
  // set multiple pixels if upscaling
  for (int16_t i = 0; i < (activeSeg->width()+(gifWidth-1)) / gifWidth; i++) {
    for (int16_t j = 0; j < (activeSeg->height()+(gifHeight-1)) / gifHeight; j++) {
      activeSeg->setPixelColorXY(outX + i, outY + j, red, green, blue);
    }
  }
}

// renders an image (.gif only; .bmp and .fseq to be added soon) from FS to a segment
bool renderImageToSegment(Segment &seg) {
  if (!seg.name) return false;
  activeSeg = &seg;

  if (strncmp(lastFilename +1, seg.name, 32) != 0) {
    Serial.println("segname changed");
    strncpy(lastFilename +1, seg.name, 32);
    gifDecodeFailed = false;
    if (strcmp(lastFilename + strlen(lastFilename) - 4, ".gif") != 0) {
      //DEBUG_PRINTLN(F("Image file not found or not a .gif"));
      gifDecodeFailed = true;
      return false;
    }
    if (file) file.close();
    Serial.print("opening gif: ");
    Serial.println(openGif(lastFilename));
    if (!file) { gifDecodeFailed = true; return false; }
    decoder.setScreenClearCallback(screenClearCallback);
    decoder.setUpdateScreenCallback(updateScreenCallback);
    decoder.setDrawPixelCallback(drawPixelCallback);
    decoder.setFileSeekCallback(fileSeekCallback);
    decoder.setFilePositionCallback(filePositionCallback);
    decoder.setFileReadCallback(fileReadCallback);
    decoder.setFileReadBlockCallback(fileReadBlockCallback);
    decoder.setFileSizeCallback(fileSizeCallback);
    Serial.println("Starting decoding");
    if(decoder.startDecoding() < 0) { gifDecodeFailed = true; return false; }
    Serial.println("Decoding started");
  }

  if (gifDecodeFailed) return false;
  if (!file) { gifDecodeFailed = true; return false; }

  if((millis() - lastFrameDisplayTime) > currentFrameDelay) {
    decoder.getSize(&gifWidth, &gifHeight);
    fillPixX = (seg.width()+(gifWidth-1)) / gifWidth;
    fillPixY = (seg.height()+(gifHeight-1)) / gifHeight;
    Serial.println("decoding frame");
    int result = decoder.decodeFrame(false);
    if (result < 0) { gifDecodeFailed = true; return false; }
    currentFrameDelay = decoder.getFrameDelay_ms();
    lastFrameDisplayTime = millis();
  }
  return true;
}