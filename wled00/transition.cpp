#include "wled.h"
#include "FX.h"

#ifndef WLED_DISABLE_MODE_BLEND
void transition_fade() {

  if (SEGMENT.is2D()) {
    uint16_t width = SEGMENT.virtualWidth();
    uint16_t height = SEGMENT.virtualHeight();
    for (int x = 0; x != width; ++x) {
      for (int y = 0; y != height; ++y) {
        int i = y * width + x;

        SEGMENT.setPixelColorXY(x, y, color_blend(SEGMENT.buffer1[i], SEGMENT.buffer2[i], 0xFFFFU - SEGMENT.progress(), true));
      }
    }
  } else {
    for (int i = 0; i != SEGMENT.virtualLength(); ++i) {
      SEGMENT.setPixelColor(i, color_blend(SEGMENT.buffer1[i], SEGMENT.buffer2[i], 0xFFFFU - SEGMENT.progress(), true));
    }
  }

}
static const char _data_TRANSITION_STYLE_FADE[] PROGMEM = "Fade";

void transition_swipe_right() {
  if (SEGMENT.is2D()) {
    uint16_t width = SEGMENT.virtualWidth();
    for (int x = 0; x != width; ++x) {
      for (int y = 0; y != SEGMENT.virtualHeight(); ++y) {
        int i = y * width + x;

        uint16_t pos = (x * 0xFFFFU) / width;
        SEGMENT.setPixelColorXY(x, y, SEGMENT.progress() <= pos ? SEGMENT.buffer2[i] : SEGMENT.buffer1[i]);
      }
    }
  } else {
    uint16_t len = SEGMENT.virtualLength();
    for (int i = 0; i != len; ++i) {
      uint16_t pos = (i * 0xFFFFU) / len;
      SEGMENT.setPixelColor(i, SEGMENT.progress() <= pos ? SEGMENT.buffer2[i] : SEGMENT.buffer1[i]);
    }
  }
}
static const char _data_TRANSITION_STYLE_SWIPE_RIGHT[] PROGMEM = "Swipe Right";

void transition_swipe_left() {
  if (SEGMENT.is2D()) {
    uint16_t width = SEGMENT.virtualWidth();
    for (int x = 0; x != width; ++x) {
      for (int y = 0; y != SEGMENT.virtualHeight(); ++y) {
        int i = y * width + x;

        uint16_t pos = 0xFFFFU - (x * 0xFFFFU) / width;
        SEGMENT.setPixelColorXY(x, y, SEGMENT.progress() <= pos ? SEGMENT.buffer2[i] : SEGMENT.buffer1[i]);
      }
    }
  } else {
    uint16_t len = SEGMENT.virtualLength();
    for (int i = 0; i != len; ++i) {
      uint16_t pos = 0xFFFFU - (i * 0xFFFFU) / len;
      SEGMENT.setPixelColor(i, SEGMENT.progress() <= pos ? SEGMENT.buffer2[i] : SEGMENT.buffer1[i]);
    }
  }
}
static const char _data_TRANSITION_STYLE_SWIPE_LEFT[] PROGMEM = "Swipe Left";

#ifndef WLED_DISABLE_2D
void transition_swipe_up() {
  if (SEGMENT.is2D()) {
    uint16_t width = SEGMENT.virtualWidth();
    uint16_t height = SEGMENT.virtualHeight();
    for (int x = 0; x != width; ++x) {
      for (int y = 0; y != height; ++y) {
        int i = y * width + x;

        uint16_t pos = 0xFFFFU - (y * 0xFFFFU) / height;
        SEGMENT.setPixelColorXY(x, y, SEGMENT.progress() <= pos ? SEGMENT.buffer2[i] : SEGMENT.buffer1[i]);
      }
    }
  } else {
    // Transition is 2D only, fall back to fade
    transition_fade();
  }
}
static const char _data_TRANSITION_STYLE_SWIPE_UP[] PROGMEM = "Swipe Up";

void transition_swipe_down() {
  if (SEGMENT.is2D()) {
    uint16_t width = SEGMENT.virtualWidth();
    uint16_t height = SEGMENT.virtualHeight();
    for (int x = 0; x != width; ++x) {
      for (int y = 0; y != height; ++y) {
        int i = y * width + x;

        uint16_t pos = (y * 0xFFFFU) / height;
        SEGMENT.setPixelColorXY(x, y, SEGMENT.progress() <= pos ? SEGMENT.buffer2[i] : SEGMENT.buffer1[i]);
      }
    }
  } else {
    // Transition is 2D only, fall back to fade
    transition_fade();
  }
}
static const char _data_TRANSITION_STYLE_SWIPE_DOWN[] PROGMEM = "Swipe Down";
#endif

void transition_push_right() {
  if (SEGMENT.is2D()) {
    uint16_t width = SEGMENT.virtualWidth();
    uint16_t height = SEGMENT.virtualHeight();
    uint16_t pos = (uint32_t(SEGMENT.progress()) * uint32_t(width)) / 0xFFFFU;
    // old mode
    for (int x = pos; x != width; ++x) {
      for (int y = 0; y != height; ++y) {
        SEGMENT.setPixelColorXY(x, y, SEGMENT.buffer2[y * width + (x - pos)]);
      }
    }
    // new mode
    for (int x = 0; x != pos; ++x) {
      for (int y = 0; y != height; ++y) {
        SEGMENT.setPixelColorXY(x, y, SEGMENT.buffer1[y * width + (x - pos + width)]);
      }
    }
  } else {
    uint16_t len = SEGMENT.virtualLength();
    uint16_t pos = (uint32_t(SEGMENT.progress()) * uint32_t(len)) / 0xFFFFU;
    // old mode
    for (int i = pos; i != len; ++i) {
      SEGMENT.setPixelColor(i, SEGMENT.buffer2[i - pos]);
    }
    // new mode
    for (int i = 0; i != pos; ++i) {
      SEGMENT.setPixelColor(i, SEGMENT.buffer1[i - pos + len]);
    }
  }
}
static const char _data_TRANSITION_STYLE_PUSH_RIGHT[] PROGMEM = "Push Right";

void transition_push_left() {
  if (SEGMENT.is2D()) {
    uint16_t width = SEGMENT.virtualWidth();
    uint16_t height = SEGMENT.virtualHeight();
    uint16_t pos = (uint32_t(0xFFFFU - SEGMENT.progress()) * uint32_t(width)) / 0xFFFFU;
    // old mode
    for (int x = 0; x != pos; ++x) {
      for (int y = 0; y != height; ++y) {
        SEGMENT.setPixelColorXY(x, y, SEGMENT.buffer2[y * width + (x - pos + width)]);
      }
    }
    // new mode
    for (int x = pos; x != width; ++x) {
      for (int y = 0; y != height; ++y) {
        SEGMENT.setPixelColorXY(x, y, SEGMENT.buffer1[y * width + (x - pos)]);
      }
    }
  } else {
    uint16_t len = SEGMENT.virtualLength();
    uint16_t pos = (uint32_t(0xFFFFU - SEGMENT.progress()) * uint32_t(len)) / 0xFFFFU;
    // old mode
    for (int i = 0; i != pos; ++i) {
      SEGMENT.setPixelColor(i, SEGMENT.buffer2[i - pos + len]);
    }
    // new mode
    for (int i = pos; i != len; ++i) {
      SEGMENT.setPixelColor(i, SEGMENT.buffer1[i - pos]);
    }
  }
}
static const char _data_TRANSITION_STYLE_PUSH_LEFT[] PROGMEM = "Push Left";

#ifndef WLED_DISABLE_2D
void transition_push_up() {
  if (SEGMENT.is2D()) {
    uint16_t width = SEGMENT.virtualWidth();
    uint16_t height = SEGMENT.virtualHeight();
    uint16_t pos = (uint32_t(0xFFFFU - SEGMENT.progress()) * uint32_t(height)) / 0xFFFFU;
    // old mode
    for (int x = 0; x != width; ++x) {
      for (int y = 0; y != pos; ++y) {
        SEGMENT.setPixelColorXY(x, y, SEGMENT.buffer2[(y - pos + height) * width + x]);
      }
    }
    // new mode
    for (int x = 0; x != width; ++x) {
      for (int y = pos; y != height; ++y) {
        SEGMENT.setPixelColorXY(x, y, SEGMENT.buffer1[(y - pos) * width + x]);
      }
    }
  } else {
    // Transition is 2D only, fall back to fade
    transition_fade();
  }
}
static const char _data_TRANSITION_STYLE_PUSH_UP[] PROGMEM = "Push Up";

void transition_push_down() {
  if (SEGMENT.is2D()) {
    uint16_t width = SEGMENT.virtualWidth();
    uint16_t height = SEGMENT.virtualHeight();
    uint16_t pos = (uint32_t(SEGMENT.progress()) * uint32_t(height)) / 0xFFFFU;
    // old mode
    for (int x = 0; x != width; ++x) {
      for (int y = pos; y != height; ++y) {
        SEGMENT.setPixelColorXY(x, y, SEGMENT.buffer2[(y - pos) * width + x]);
      }
    }
    // new mode
    for (int x = 0; x != width; ++x) {
      for (int y = 0; y != pos; ++y) {
        SEGMENT.setPixelColorXY(x, y, SEGMENT.buffer1[(y - pos + height) * width + x]);
      }
    }
  } else {
    // Transition is 2D only, fall back to fade
    transition_fade();
  }
}
static const char _data_TRANSITION_STYLE_PUSH_DOWN[] PROGMEM = "Push Down";
#endif

void transition_outside_in() {
  if (SEGMENT.is2D()) {
    uint16_t width = SEGMENT.virtualWidth();
    uint16_t height = SEGMENT.virtualHeight();
    uint16_t halfWidth = width >> 1;
    for (int x = 0; x != width; ++x) {
      for (int y = 0; y != height; ++y) {
        int i = y * width + x;
        uint16_t pos = ((x < halfWidth ? x : width - x) * 0xFFFFU) / halfWidth;
        SEGMENT.setPixelColorXY(x, y, SEGMENT.progress() <= pos ? SEGMENT.buffer2[i] : SEGMENT.buffer1[i]);
      }
    }
  } else {
    uint16_t len = SEGMENT.virtualLength();
    uint16_t halfLen = len >> 1;
    for (int i = 0; i != len; ++i) {
      uint16_t pos = ((i < halfLen ? i : len - i) * 0xFFFFU) / halfLen;
      SEGMENT.setPixelColor(i, SEGMENT.progress() <= pos ? SEGMENT.buffer2[i] : SEGMENT.buffer1[i]);
    }
  }
}
static const char _data_TRANSITION_STYLE_OUTSIDE_IN[] PROGMEM = "Outside In";

void transition_inside_out() {
  if (SEGMENT.is2D()) {
    uint16_t width = SEGMENT.virtualWidth();
    uint16_t height = SEGMENT.virtualHeight();
    uint16_t halfWidth = width >> 1;
    for (int x = 0; x != width; ++x) {
      for (int y = 0; y != height; ++y) {
        int i = y * width + x;
        uint16_t pos = 0xFFFFU - ((x < halfWidth ? x : width - x) * 0xFFFFU) / halfWidth;
        SEGMENT.setPixelColorXY(x, y, SEGMENT.progress() <= pos ? SEGMENT.buffer2[i] : SEGMENT.buffer1[i]);
      }
    }
  } else {
    uint16_t len = SEGMENT.virtualLength();
    uint16_t halfLen = len >> 1;
    for (int i = 0; i != len; ++i) {
      uint16_t pos = 0xFFFFU - ((i < halfLen ? i : len - i) * 0xFFFFU) / halfLen;
      SEGMENT.setPixelColor(i, SEGMENT.progress() <= pos ? SEGMENT.buffer2[i] : SEGMENT.buffer1[i]);
    }
  }
}
static const char _data_TRANSITION_STYLE_INSIDE_OUT[] PROGMEM = "Inside Out";

void transition_fairy_dust() {
  if (SEGMENT.is2D()) {
    uint16_t width = SEGMENT.virtualWidth();
    uint16_t height = SEGMENT.virtualHeight();
    uint32_t len = SEGMENT.virtualLength();
    for (int x = 0; x != width; ++x) {
      for (int y = 0; y != height; ++y) {
        int i = y * width + x;

        uint32_t shuffled = hashInt(i) % len;
        uint16_t pos = (shuffled * 0xFFFFU) / len;
        SEGMENT.setPixelColorXY(x, y, SEGMENT.progress() <= pos ? SEGMENT.buffer2[i] : SEGMENT.buffer1[i]);
      }
    }
  } else {
    uint16_t len = SEGMENT.virtualLength();
    for (int i = 0; i != len; ++i) {
      uint32_t shuffled = hashInt(i) % len;
      uint16_t pos = (shuffled * 0xFFFFU) / len;
      SEGMENT.setPixelColor(i, SEGMENT.progress() <= pos ? SEGMENT.buffer2[i] : SEGMENT.buffer1[i]);
    }
  }
}
static const char _data_TRANSITION_STYLE_FAIRY_DUST[] PROGMEM = "Fairy Dust";

void WS2812FX::addTransitionStyle(uint8_t id, transition_ptr func, const char *name, bool only2D = false) {
  if (id < _transitionStyles.size()) {
    _transitionStyles[id] = TransitionStyleData(func, name, only2D);
  }
}

void WS2812FX::setupTransitionStyleData() {
  if (!_transitionStyleCount) return;
  _transitionStyles.resize(_transitionStyleCount);

  addTransitionStyle(TRANSITION_STYLE_FADE, &transition_fade, _data_TRANSITION_STYLE_FADE);
  addTransitionStyle(TRANSITION_STYLE_SWIPE_RIGHT, &transition_swipe_right, _data_TRANSITION_STYLE_SWIPE_RIGHT);
  addTransitionStyle(TRANSITION_STYLE_SWIPE_LEFT, &transition_swipe_left, _data_TRANSITION_STYLE_SWIPE_LEFT);
#ifndef WLED_DISABLE_2D
  addTransitionStyle(TRANSITION_STYLE_SWIPE_UP, &transition_swipe_up, _data_TRANSITION_STYLE_SWIPE_UP, true); // 2D only
  addTransitionStyle(TRANSITION_STYLE_SWIPE_DOWN, &transition_swipe_down, _data_TRANSITION_STYLE_SWIPE_DOWN, true); // 2D only
#endif // WLED_DISABLE_2D
  addTransitionStyle(TRANSITION_STYLE_PUSH_RIGHT, &transition_push_right, _data_TRANSITION_STYLE_PUSH_RIGHT);
  addTransitionStyle(TRANSITION_STYLE_PUSH_LEFT, &transition_push_left, _data_TRANSITION_STYLE_PUSH_LEFT);
#ifndef WLED_DISABLE_2D
  addTransitionStyle(TRANSITION_STYLE_PUSH_UP, &transition_push_up, _data_TRANSITION_STYLE_PUSH_UP, true); // 2D only
  addTransitionStyle(TRANSITION_STYLE_PUSH_DOWN, &transition_push_down, _data_TRANSITION_STYLE_PUSH_DOWN, true); // 2D only
#endif // WLED_DISABLE_2D
  addTransitionStyle(TRANSITION_STYLE_OUTSIDE_IN, &transition_outside_in, _data_TRANSITION_STYLE_OUTSIDE_IN);
  addTransitionStyle(TRANSITION_STYLE_INSIDE_OUT, &transition_inside_out, _data_TRANSITION_STYLE_INSIDE_OUT);
  addTransitionStyle(TRANSITION_STYLE_FAIRY_DUST, &transition_fairy_dust, _data_TRANSITION_STYLE_FAIRY_DUST);
}
#endif
