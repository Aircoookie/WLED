#include "wled.h"

uint32_t WASMFX::now() {
  return strip.now;
}

uint32_t WASMFX::speed() {
  return SEGMENT.speed;
}

uint32_t WASMFX::intensity() {
  return SEGMENT.intensity;
}

uint32_t WASMFX::len() {
  return strip._virtualSegmentLength;
}