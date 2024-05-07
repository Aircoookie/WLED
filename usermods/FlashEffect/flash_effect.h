#pragma once

#include "wled.h"

/**
platformio_override.ini example to enable this mod:

    [platformio]
    default_envs = xiao32C3_with_flash_effect

    [env:xiao32C3_with_flash_effect]
    ;; ESP32s3dev_8MB but adding flash effect UserMod
    extends = env:esp32s3dev_8MB
    build_flags = ${env:esp32s3dev_8MB.build_flags} -DUSERMOD_FLASH_EFFECT
*/

const char uiSpec_[] PROGMEM = "Flash@Duration;!,!;!;01";

struct FlashEffect : Usermod {
  void setup() override {
    strip.addEffect(255, &flashEffect_, uiSpec_);
  }

  void loop() override {}

private:
  static uint16_t flashEffect_() {
    uint32_t maxDuration = 1000;
    uint32_t duration = SEGMENT.speed * maxDuration / 255 + FRAMETIME * 2;
    SEGMENT.fill(strip.now < duration ? SEGCOLOR(0) : SEGCOLOR(1));
    return FRAMETIME;
  }
};
