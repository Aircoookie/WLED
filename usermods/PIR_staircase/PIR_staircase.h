#pragma once

#include "math.h"
#include "wled.h"
#define USERMOD_ID_PIR_STAIRCASE 1011

/*
 * Usermod for detecting people entering/leaving a staircase and switching the
 * staircase on/off. See the accompanying README.md file.
 */

// Please change the pin numbering to match your board.
const int topPIR_PIN = D7;
const int bottomPIR_PIN = D6;

// Time between switching on/off each segment, stored in config
unsigned long segment_delay_ms = 150;

// The time for the light to stay on
unsigned long on_time_ms = 5 * 1000;

// Time between checking of the PIRs
const int scanDelay = 50;

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

  bool saveState = false;

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

      // Serial.print(millis(),DEC);
      // Serial.print(" ");
      // Serial.print(bottomPIR);
      // Serial.print(" ");
      // Serial.println(topPIR);

      if (bottomPIR != topPIR) {
        lastSwitchTime = millis();

        if (on) {
          lastPIR = topPIR;
        } else {
          // If the bottom PIR triggered, we need to swipe up, ON
          swipe = bottomPIR;

          if (swipe) {
            Serial.println("ON -> Swipe up.");
          } else {
            Serial.println("ON -> Swipe down.");
          }

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
    if (on && (millis() - lastSwitchTime) > on_time_ms) {
      // Swipe OFF in the direction of the last PIR detection
      swipe = lastPIR;
      on = false;

      if (swipe) {
        Serial.println("OFF -> Swipe up.");
      } else {
        Serial.println("OFF -> Swipe down.");
      }
    }
  }

  void updateSwipe() {
    if ((millis() - lastTime) > segment_delay_ms) {
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
    Serial.print("Delay between steps: ");
    Serial.print(segment_delay_ms, DEC);
    Serial.print(" milliseconds.\nStairs switch off after: ");
    Serial.print(on_time_ms / 1000, DEC);
    Serial.println(" seconds.");

    pinMode(bottomPIR_PIN, INPUT);
    pinMode(topPIR_PIN, INPUT);
  }

  void loop() {
    checkPIRs();
    autoPowerOff();
    updateSwipe();

    // Write changed settings from the json api into flash now
    if (saveState) {
      serializeConfig();
      saveState = false;
    }
  }

  uint16_t getId() { return USERMOD_ID_PIR_STAIRCASE; }

  /*
   * Shows configuration settings to the json API. This object looks like:
   *
   * "staircase" : {
   *   "segment-delay-ms" : 150,
   *   "on-time-s" : 5
   * }
   *
   */
  void addToJsonState(JsonObject& root) {
    JsonObject staircase = root["staircase"];
    if (staircase.isNull()) {
      staircase = root.createNestedObject("staircase");
    }
    staircase["segment-delay-ms"] = segment_delay_ms;
    staircase["on-time-s"] = on_time_ms / 1000;
  }

  /*
   * Reads configuration settings from the json API.
   * See void addToJsonState(JsonObject& root)
   */
  void readFromJsonState(JsonObject& root) {
    JsonObject staircase = root["staircase"];
    unsigned long s = staircase["segment-delay-ms"] | 150;
    unsigned long o = (staircase["on-time-s"] | 5) * 1000;

    if (s != segment_delay_ms) {
      segment_delay_ms = s;
      saveState = true;
    }

    if (o != on_time_ms / 1000) {
      on_time_ms = s * 1000;
      saveState = true;
    }

    if (saveState) {
      Serial.println("Staircase settings changed by API call.");
    }
  }

  /*
   * Writes the configuration to internal flash memory.
   */
  void addToConfig(JsonObject& root) {
    JsonObject staircase = root["staircase"];
    if (staircase.isNull()) {
      staircase = root.createNestedObject("staircase");
    }

    staircase["segment-delay-ms"] = segment_delay_ms;
    staircase["on-time-s"] = on_time_ms / 1000;
    Serial.println("Staircase config saved.");
  }

  /*
   * Reads the configuration to internal flash memory before setup() is called.
   */
  void readFromConfig(JsonObject& root) {
    JsonObject staircase = root["staircase"];
    segment_delay_ms = staircase["segment-delay-ms"] | 150;
    on_time_ms = (staircase["on-time-s"] | 5) * 1000;
    Serial.println("Staircase config loaded.");
  }

  /*
   * Shows the delay between steps and power-off time in the "info"
   * tab of the web-UI.
   */
  void addToJsonInfo(JsonObject& root) {
    int reading = 20;
    // this code adds "u":{"Light":[20," lux"]} to the info object
    JsonObject staircase = root["u"];
    if (staircase.isNull()) {
      staircase = root.createNestedObject("u");
    }

    JsonArray segmentDelay =
        staircase.createNestedArray("Delay between stairs");  // name
    segmentDelay.add(segment_delay_ms);                       // value
    segmentDelay.add(" milliseconds");                        // unit

    JsonArray onTime =
        staircase.createNestedArray("Power-off stairs after");  // name
    onTime.add(on_time_ms / 1000);                              // value
    onTime.add(" seconds");                                     // unit
  }
};