#include "wled.h"

uint32_t WASMFX::now() {
  return strip.now;
}

uint32_t WASMFX::speed() {
  return strip._segments[strip._segment_index].speed;
}

uint32_t WASMFX::intensity() {
  return strip._segments[strip._segment_index].intensity;
}