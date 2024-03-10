#ifndef WLED_DISABLE_GIF

#include "GifDecoder.h"
#include "wled.h"

File file;
char lastFilename[34] = "/";
GifDecoder<32,32,12> decoder;
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
//uint16_t fillPixX, fillPixY;

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
      activeSeg->setPixelColorXY(outX + i, outY + j, gamma8(red), gamma8(green), gamma8(blue));
    }
  }
}

#define IMAGE_ERROR_NONE 0
#define IMAGE_ERROR_NO_NAME 1
#define IMAGE_ERROR_SEG_LIMIT 2
#define IMAGE_ERROR_UNSUPPORTED_FORMAT 3
#define IMAGE_ERROR_FILE_MISSING 4
#define IMAGE_ERROR_DECODER_ALLOC 5
#define IMAGE_ERROR_GIF_DECODE 6
#define IMAGE_ERROR_FRAME_DECODE 7
#define IMAGE_ERROR_WAITING 254
#define IMAGE_ERROR_PREV 255

// renders an image (.gif only; .bmp and .fseq to be added soon) from FS to a segment
byte renderImageToSegment(Segment &seg) {
  if (!seg.name) return IMAGE_ERROR_NO_NAME;
  if (activeSeg && activeSeg != &seg) return IMAGE_ERROR_SEG_LIMIT; // only one segment at a time
  activeSeg = &seg;

  if (strncmp(lastFilename +1, seg.name, 32) != 0) { // segment name changed, load new image
    strncpy(lastFilename +1, seg.name, 32);
    gifDecodeFailed = false;
    if (strcmp(lastFilename + strlen(lastFilename) - 4, ".gif") != 0) {
      gifDecodeFailed = true;
      return IMAGE_ERROR_UNSUPPORTED_FORMAT;
    }
    if (file) file.close();
    openGif(lastFilename);
    if (!file) { gifDecodeFailed = true; return IMAGE_ERROR_FILE_MISSING; }
    //if (!decoder) decoder = new GifDecoder<32,32,12>();
    //if (!decoder) { gifDecodeFailed = true; return IMAGE_ERROR_DECODER_ALLOC; }
    decoder.setScreenClearCallback(screenClearCallback);
    decoder.setUpdateScreenCallback(updateScreenCallback);
    decoder.setDrawPixelCallback(drawPixelCallback);
    decoder.setFileSeekCallback(fileSeekCallback);
    decoder.setFilePositionCallback(filePositionCallback);
    decoder.setFileReadCallback(fileReadCallback);
    decoder.setFileReadBlockCallback(fileReadBlockCallback);
    decoder.setFileSizeCallback(fileSizeCallback);
    Serial.println("Starting decoding");
    if(decoder.startDecoding() < 0) { gifDecodeFailed = true; return IMAGE_ERROR_GIF_DECODE; }
    Serial.println("Decoding started");
  }

  if (gifDecodeFailed) return IMAGE_ERROR_PREV;
  if (!file) { gifDecodeFailed = true; return IMAGE_ERROR_FILE_MISSING; }
  //if (!decoder) { gifDecodeFailed = true; return IMAGE_ERROR_DECODER_ALLOC; }

  // speed 0 = half speed, 128 = normal, 255 = full FX FPS
  // TODO: 0 = 4x slow, 64 = 2x slow, 128 = normal, 192 = 2x fast, 255 = 4x fast
  uint32_t wait = currentFrameDelay * 2 - seg.speed * currentFrameDelay / 128;

  if((millis() - lastFrameDisplayTime) < wait) return IMAGE_ERROR_WAITING;

  decoder.getSize(&gifWidth, &gifHeight);
  //fillPixX = (seg.width()+(gifWidth-1)) / gifWidth;
  //fillPixY = (seg.height()+(gifHeight-1)) / gifHeight;
  int result = decoder.decodeFrame(false);
  if (result < 0) { gifDecodeFailed = true; return IMAGE_ERROR_FRAME_DECODE; }
  //long lastFrameDelay = currentFrameDelay;
  currentFrameDelay = decoder.getFrameDelay_ms();
  //long tooSlowBy = (millis() - lastFrameDisplayTime) - wait; // if last frame was longer than intended, compensate
  //currentFrameDelay -= tooSlowBy; // TODO this is broken
  lastFrameDisplayTime = millis();

  return IMAGE_ERROR_NONE;
}

void endImagePlayback() {
  if (file) file.close();
  //delete decoder;
  gifDecodeFailed = false;
  activeSeg = nullptr;
  lastFilename[0] = '\0';
}

#endif