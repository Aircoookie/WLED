#pragma once

#include "math.h"
#include "wled.h"

/*
 * Usermod for detecting people entering/leaving a staircase and switching the
 * staircase on/off. See the accompanying README.md file.
 */
#define USERMOD_ID_PIR_STAIRCASE 1011

// Time between switching on/off each segment
const unsigned long SEGMENT_DELAY_MS = 150;

// The time for the light to stay on
const unsigned long ON_TIME = 5 * 1000;

// Time between checking of the PIRs
const int scanDelay = 50;

// Please add #ifdef for your board configuration or just change the pins
#if defined(D5) && defined(D6)
const int topPIR_PIN    = D6;
const int bottomPIR_PIN = D5;
#elif ARDUINO_ARCH_ESP32
const int topPIR_PIN    = GPIO_NUM_17; // d1_mini esp32
const int bottomPIR_PIN = GPIO_NUM_16; // d1_mini esp32
#else
const int topPIR_PIN    = GPIO_NUM_2; // d1_mini ESP8266
const int bottomPIR_PIN = GPIO_NUM_0; // d1_mini ESP8266
#endif

class PIR_staircase : public Usermod {
 private:
  // Lights on or off.
  // Flipping this will start a transition.
  bool on = false;

  // Swipe direction for current transition
#define SWIPE_UP true
#define SWIPE_DOWN false
  bool swipe = SWIPE_UP;

  // Indicates which PIR was seen last (to determine
  // the direction when swiping off)
#define LOWER false
#define UPPER true
  bool lastPIR = LOWER;

  // Time of the last transition action
  unsigned long lastTime = 0;

  // Time of the last PIR check
  unsigned long lastScanTime = 0;

  // Last time the lights were switched on or off
  unsigned long lastSwitchTime = 0;

  // segment id between onIndex and offIndex are on.
  // controll the swipe by setting/moving these indices around.
  // onIndex must be less than or equal to offIndex
  byte onIndex = 0;
  byte offIndex = 0;

  // The maximum number of configured segments.
  // Dynamically updated based on user configuration.
  byte maxSegmentId = 1;
  byte mainSegmentId = 0;

  void updateSegments() {
    mainSegmentId = strip.getMainSegmentId();
    WS2812FX::Segment mainsegment = strip.getSegment(mainSegmentId);
    WS2812FX::Segment* segments = strip.getSegments();
    for (int i = 0; i < MAX_NUM_SEGMENTS; i++, segments++) {
      if (!segments->isActive()) {
        maxSegmentId = i - 1;
        break;
      }

      if (i >= onIndex && i < offIndex) {
        segments->setOption(SEG_OPTION_ON, 1, 1);

        // We need to mode and colors from segment 0 to make sure changes
        // are propagated even when the config is changed during a wipe
        segments->mode = mainsegment.mode;
        segments->colors[0] = mainsegment.colors[0];
      } else {
        segments->setOption(SEG_OPTION_ON, 0, 1);
      }
    }
    colorUpdated(NOTIFIER_CALL_MODE_DIRECT_CHANGE);
  }

  void checkPIRs() {
    if ((millis() - lastScanTime) > scanDelay) {
      lastScanTime = millis();

      bool bottomPIR = digitalRead(bottomPIR_PIN) == HIGH;
      bool topPIR = digitalRead(topPIR_PIN) == HIGH;

      if (bottomPIR != topPIR) {
        lastSwitchTime = millis();

        if (on) {
          lastPIR = topPIR;
        } else {
          // If the bottom PIR triggered, we need to swipe up, ON
          swipe = bottomPIR;

          if (onIndex == offIndex) {
            // Position the indices for a correct on-swipe
            if (swipe == SWIPE_UP) {
              onIndex = mainSegmentId;
            } else {
              onIndex = maxSegmentId;
            }
            offIndex = onIndex;
          }
          on = true;
        }
      }
    }
  }

  void autoPowerOff() {
    if (on && (millis() - lastSwitchTime) > ON_TIME) {
      // Swipe OFF in the direction of the last PIR detection
      swipe = lastPIR;
      on = false;
    }
  }

  void updateSwipe() {
    if ((millis() - lastTime) > SEGMENT_DELAY_MS) {
      lastTime = millis();

      byte oldOnIndex = onIndex;
      byte oldOffIndex = offIndex;

      if (on) {
        // Turn on all segments
        onIndex = MAX(mainSegmentId, onIndex - 1);
        offIndex = MIN(maxSegmentId + 1, offIndex + 1);
      } else {
        if (swipe == SWIPE_UP) {
          onIndex = MIN(offIndex, onIndex + 1);
        } else {
          offIndex = MAX(onIndex, offIndex - 1);
        }
      }

      updateSegments();
    }
  }

 public:
  void setup() {
    Serial.println("PIR Staircase enabled.");
    pinMode(bottomPIR_PIN, INPUT);
    pinMode(topPIR_PIN, INPUT);
  }

  void loop() {
    checkPIRs();
    autoPowerOff();
    updateSwipe();
  }

  uint16_t getId() { return USERMOD_ID_PIR_STAIRCASE; }
};