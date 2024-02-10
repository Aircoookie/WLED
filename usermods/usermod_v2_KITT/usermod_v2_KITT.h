#pragma once
#include "wled.h"

#ifndef FX_MODE_KITT
#define FX_MODE_KITT    255  // Auto-allocate number
#endif

class KITTUsermod : public Usermod {
  private:
    // strings to reduce flash memory usage (used more than twice)
    static const char _FX_MODE_KITT[];

    /*
     * Uses speed, intensity (tail), custom1 (delay) and option1 checkbox for dual mode
     */
    static uint16_t mode_kitt(void) {
        if (SEGLEN == 1) {
            SEGMENT.fill(SEGCOLOR(0));
            return FRAMETIME;
        }
        if (SEGMENT.call == 0) {
            SEGMENT.aux0 = 0;
            SEGMENT.step = 0;
        }
        SEGMENT.fade_out(255 - SEGMENT.intensity); // Fade to SEGCOLOR(1) - BG
        if (SEGMENT.step > millis())
            return FRAMETIME; // delay hasn't finished yet
        else
            SEGMENT.step = 0;

        // delay before move
        if (SEGMENT.call % (((255 - SEGMENT.speed) >> 4) + 1) == 0) {
            if (SEGMENT.aux0 < 2*SEGLEN) {
                uint16_t pos = (SEGMENT.aux0 / SEGLEN) & 0x1 ?
                    SEGLEN - 1 - (SEGMENT.aux0 % SEGLEN) : // odd down
                    SEGMENT.aux0 % SEGLEN; // even up
                uint32_t color1 = SEGMENT.color_from_palette(pos, true, false, 0);
                SEGMENT.setPixelColor(pos, color1);
                if (SEGMENT.check1) { // dual
                    SEGMENT.setPixelColor(SEGLEN - 1 - pos, color1);
                }
                SEGMENT.aux0++;
            } else {
                SEGMENT.step = millis() + SEGMENT.custom1 * 10; // multiply by 10ms
                SEGMENT.aux0 = 0; // wrap after delay
            }
        }
        return FRAMETIME;
    }

  public:
    // gets called once at boot.
    // parameters are already read by readFromConfig, busses & segments have been created by beginStrip()
    void setup() {
        strip.addEffect(FX_MODE_KITT, &mode_kitt, _FX_MODE_KITT);
    }
    void loop() {} // must be overridden
    /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
    uint16_t getId() {
      return USERMOD_ID_KITT;
    }
};

// config strings to reduce flash memory usage (used more than twice)
// <Effect parameters>(Speed & Fade & Dual checkbox);<Colors>(2 Colours, Fg & Bg);<Palette>(Enabled);<Flags>(1Dim);<Defaults>
const char KITTUsermod::_FX_MODE_KITT[] PROGMEM = "KITT@!,Tail,Delay,,,Dual;!,!;!;1;sx=210";
