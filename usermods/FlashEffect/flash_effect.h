#pragma once

#include "../../wled00/wled.h"

/**
platformio_override.ini example to enable this mod:

    [platformio]
    default_envs = xiao32C3_with_flash_effect

    [env:xiao32C3_with_flash_effect]
    ;; ESP32s3dev_8MB but adding flash effect UserMod
    extends = env:esp32s3dev_8MB
    build_flags = ${env:esp32s3dev_8MB.build_flags} -DUSERMOD_FLASH_EFFECT
*/

struct FlashEffect : Usermod {
  void setup() override {
    static const char spec[] PROGMEM = "Flash@Duration;!,!;!;01";
    strip.addEffect(255, &flashEffect_, spec);
  }

  void loop() override {}

  /**
    * onStateChanged() is used to detect WLED state change
    * @mode parameter is CALL_MODE_... parameter used for notifications
    */
  void onStateChange(uint8_t mode) override {
    Usermod::onStateChange(mode);
    // do something if WLED state changed (color, brightness, effect, preset, etc)
  }

private:
  static uint16_t flashEffect_() {
    // Use 'aux0' to store effect start time.
    if (SEGMENT.call == 0)
      SEGMENT.aux0 = strip.now;
    uint32_t maxDuration = 1000;
    uint32_t duration = SEGMENT.speed * maxDuration / 255 + FRAMETIME * 2;
    uint32_t color = SEGCOLOR(1);
    if (strip.now - SEGMENT.aux0 < duration)
      color = SEGCOLOR(0);
    SEGMENT.fill(color);
    return FRAMETIME;
  }
};
