#include "wled.h"
#include "FX.h"


#ifndef WLED_DISABLE_MODE_BLEND
void Segment::renderTransition() {
  uint16_t len = virtualLength();

  switch (transitionStyle) {
    case TRANSITION_STYLE_PUSH_RIGHT: {
      uint16_t pos = (uint32_t(progress()) * uint32_t(len)) / 0xFFFFU;
      // old mode
      for (int i = pos; i != len; ++i) {
        setPixelColor(i, buffer2[i - pos]);
      }
      // new mode
      for (int i = 0; i != pos; ++i) {
        setPixelColor(i, buffer1[i - pos + len]);
      }
      return;
    }
    case TRANSITION_STYLE_PUSH_LEFT: {
      uint16_t pos = (uint32_t(0xFFFFU - progress()) * uint32_t(len)) / 0xFFFFU;
      // old mode
      for (int i = 0; i != pos; ++i) {
        setPixelColor(i, buffer2[i - pos + len]);
      }
      // new mode
      for (int i = pos; i != len; ++i) {
        setPixelColor(i, buffer1[i - pos]);
      }
      return;
    }
  }

  // Transitions where both buffers are aligned
  for (int i = 0; i != len; ++i) {
    switch (transitionStyle) {
      case TRANSITION_STYLE_SWIPE_RIGHT: {
        uint16_t pos = (i * 0xFFFFU) / length();
        setPixelColor(i, progress() <= pos ? buffer2[i] : buffer1[i]);
        break;
      }
      case TRANSITION_STYLE_SWIPE_LEFT: {
        uint16_t pos = 0xFFFFU - (i * 0xFFFFU) / virtualLength();
        setPixelColor(i, progress() <= pos ? buffer2[i] : buffer1[i]);
        break;
      }
      case TRANSITION_STYLE_OUTSIDE_IN: {
        uint16_t len = virtualLength();
        uint16_t halfLen = len >> 1;
        uint16_t pos = ((i < halfLen ? i : len - i) * 0xFFFFU) / halfLen;
        setPixelColor(i, progress() <= pos ? buffer2[i] : buffer1[i]);
        break;
      }
      case TRANSITION_STYLE_INSIDE_OUT: {
        uint16_t len = virtualLength();
        uint16_t halfLen = len >> 1;
        uint16_t pos = 0xFFFFU - ((i < halfLen ? i : len - i) * 0xFFFFU) / halfLen;
        setPixelColor(i, progress() <= pos ? buffer2[i] : buffer1[i]);
        break;
      }
      case TRANSITION_STYLE_FAIRY_DUST: {
        uint32_t len = virtualLength();
        uint32_t shuffled = hashInt(i) % len;
        uint16_t pos = (shuffled * 0xFFFFU) / len;
        setPixelColor(i, progress() <= pos ? buffer2[i] : buffer1[i]);
        break;
      }
      case TRANSITION_STYLE_FADE:
      default: {
        setPixelColor(i, color_blend(buffer1[i], buffer2[i], 0xFFFFU - progress(), true));
        break;
      }
    }
  }
}
#endif

#ifndef WLED_DISABLE_MODE_BLEND
void Segment::render2DTransition() {
  uint16_t width = virtualWidth();
  uint16_t height = virtualHeight();

  // Sometimes the timing works out such that "pos" calculated below doesn't hit
  // all the possible coordinates. This can happen e.g. when the frame rate is low.
  // As a result, there can be some pixels left in a transitional state. This is a
  // problem if the target effect doesn't update all the pixels. In order to ensure
  // there are no artifacts left from the transition, we need to clear the segment
  // before we render the transition.
  fill(BLACK);

  switch (transitionStyle) {
    case TRANSITION_STYLE_PUSH_RIGHT: {
      uint16_t pos = (uint32_t(progress()) * uint32_t(width)) / 0xFFFFU;
      // old mode
      for (int x = pos; x != width; ++x) {
        for (int y = 0; y != height; ++y) {
          setPixelColorXY(x, y, buffer2[y * width + (x - pos)]);
        }
      }
      // new mode
      for (int x = 0; x != pos; ++x) {
        for (int y = 0; y != height; ++y) {
          setPixelColorXY(x, y, buffer1[y * width + (x - pos + width)]);
        }
      }
      return;
    }
    case TRANSITION_STYLE_PUSH_LEFT: {
      uint16_t pos = (uint32_t(0xFFFFU - progress()) * uint32_t(width)) / 0xFFFFU;
      // old mode
      for (int x = 0; x != pos; ++x) {
        for (int y = 0; y != height; ++y) {
          setPixelColorXY(x, y, buffer2[y * width + (x - pos + width)]);
        }
      }
      // new mode
      for (int x = pos; x != width; ++x) {
        for (int y = 0; y != height; ++y) {
          setPixelColorXY(x, y, buffer1[y * width + (x - pos)]);
        }
      }
      return;
    }
    case TRANSITION_STYLE_PUSH_UP: {
      uint16_t pos = (uint32_t(0xFFFFU - progress()) * uint32_t(height)) / 0xFFFFU;
      // old mode
      for (int x = 0; x != width; ++x) {
        for (int y = 0; y != pos; ++y) {
          setPixelColorXY(x, y, buffer2[(y - pos + height) * width + x]);
        }
      }
      // new mode
      for (int x = 0; x != width; ++x) {
        for (int y = pos; y != height; ++y) {
          setPixelColorXY(x, y, buffer1[(y - pos) * width + x]);
        }
      }
      return;
    }
    case TRANSITION_STYLE_PUSH_DOWN: {
      uint16_t pos = (uint32_t(progress()) * uint32_t(height)) / 0xFFFFU;
      // old mode
      for (int x = 0; x != width; ++x) {
        for (int y = pos; y != height; ++y) {
          setPixelColorXY(x, y, buffer2[(y - pos) * width + x]);
        }
      }
      // new mode
      for (int x = 0; x != width; ++x) {
        for (int y = 0; y != pos; ++y) {
          setPixelColorXY(x, y, buffer1[(y - pos + height) * width + x]);
        }
      }
      return;
    }
  }

  // Transitions where both buffers are aligned
  for (int x = 0; x != width; ++x) for (int y = 0; y != height; ++y) {
    int i = y * width + x;

    switch (transitionStyle) {
      case TRANSITION_STYLE_SWIPE_RIGHT: {
        uint16_t pos = (x * 0xFFFFU) / width;
        setPixelColorXY(x, y, progress() <= pos ? buffer2[i] : buffer1[i]);
        break;
      }
      case TRANSITION_STYLE_SWIPE_LEFT: {
        uint16_t pos = 0xFFFFU - (x * 0xFFFFU) / width;
        setPixelColorXY(x, y, progress() <= pos ? buffer2[i] : buffer1[i]);
        break;
      }
      case TRANSITION_STYLE_SWIPE_UP: {
        uint16_t pos = 0xFFFFU - (y * 0xFFFFU) / height;
        setPixelColorXY(x, y, progress() <= pos ? buffer2[i] : buffer1[i]);
        break;
      }
      case TRANSITION_STYLE_SWIPE_DOWN: {
        uint16_t pos = (y * 0xFFFFU) / height;
        setPixelColorXY(x, y, progress() <= pos ? buffer2[i] : buffer1[i]);
        break;
      }
      case TRANSITION_STYLE_OUTSIDE_IN: {
        uint16_t halfWidth = width >> 1;
        uint16_t pos = ((x < halfWidth ? x : width - x) * 0xFFFFU) / halfWidth;
        setPixelColorXY(x, y, progress() <= pos ? buffer2[i] : buffer1[i]);
        break;
      }
      case TRANSITION_STYLE_INSIDE_OUT: {
        uint16_t halfWidth = width >> 1;
        uint16_t pos = 0xFFFFU - ((x < halfWidth ? x : width - x) * 0xFFFFU) / halfWidth;
        setPixelColorXY(x, y, progress() <= pos ? buffer2[i] : buffer1[i]);
        break;
      }
      case TRANSITION_STYLE_FAIRY_DUST: {
        uint32_t len = virtualLength();
        uint32_t shuffled = hashInt(i) % len;
        uint16_t pos = (shuffled * 0xFFFFU) / len;
        setPixelColorXY(x, y, progress() <= pos ? buffer2[i] : buffer1[i]);
        break;
      }
      case TRANSITION_STYLE_FADE:
      default: {
        setPixelColorXY(x, y, color_blend(buffer1[i], buffer2[i], 0xFFFFU - progress(), true));
      }
    }
  }
}
#endif

