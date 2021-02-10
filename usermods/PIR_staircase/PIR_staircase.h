/*
 * Usermod for detecting people entering/leaving a staircase and switching the
 * staircase on/off.
 *
 * Edit the PIR_staircase_config.h file to compile this usermod for your
 * specific configuration.
 * 
 * See the accompanying README.md file for more info.
 */
#pragma once
#include "wled.h"
#include "PIR_staircase_config.h"
#define USERMOD_ID_PIR_STAIRCASE 1011

/* Initial configuration (available in API and stored in flash) */
bool enabled = true;                   // Enable this usermod
unsigned long segment_delay_ms = 150;  // Time between switching each segment
unsigned long on_time_ms = 5 * 1000;   // The time for the light to stay on
#ifndef topPIR_PIN
unsigned int topMaxTimeUs = 1749;  // default echo timout, top
#endif
#ifndef bottomPIR_PIN
unsigned int bottomMaxTimeUs = 1749;  // default echo timout, bottom
#endif

// Time between checking of the PIRs (or ultrasonic sensors)
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

  // Indicates which Sensor was seen last (to determine
  // the direction when swiping off)
#define LOWER false
#define UPPER true
  bool lastSensor = LOWER;

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

  // These values are used by the API to read the
  // last sensor state, or trigger a sensor
  // through the API
  bool topSensorRead = false;
  bool topSensorWrite = false;
  bool bottomSensorRead = false;
  bool bottomSensorWrite = false;  

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

        // We may need to copy mode and colors from segment 0 to make sure
        // changes are propagated even when the config is changed during a wipe
        // segments->mode = mainsegment.mode;
        // segments->colors[0] = mainsegment.colors[0];
      } else {
        segments->setOption(SEG_OPTION_ON, 0, 1);
      }
      // Always mark segments as "transitional", we are animating the staircase
      segments->setOption(SEG_OPTION_TRANSITIONAL, 1, 1);
    }
    colorUpdated(NOTIFIER_CALL_MODE_DIRECT_CHANGE);
  }

  /*
   * Detects if an object is within ultrasound range.
   * signalPin: The pin where the pulse is sent
   * echoPin:   The pin where the echo is received
   * maxTimeUs: Detection timeout in microseconds. If an echo is
   *            received within this time, an object is detected
   *            and the function will return true.
   *
   * The speed of sound is 343 meters per second at 20 degress Celcius.
   * Since the sound has to travel back and forth, the detection
   * distance for the sensor in cm is (0.0343 * maxTimeUs) / 2.
   *
   * For practical reasons, here are some useful distances:
   *
   * Distance =	maxtime
   *     5 cm =  292 uS
   *    10 cm =  583 uS
   *    20 cm = 1166 uS
   *    30 cm = 1749 uS
   *    50 cm = 2915 uS
   *   100 cm = 5831 uS
   */
  bool ultrasoundRead(uint8_t signalPin,
                      uint8_t echoPin,
                      unsigned int maxTimeUs) {
    digitalWrite(signalPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(signalPin, LOW);
    return pulseIn(echoPin, HIGH, maxTimeUs) > 0;
  }

  void checkSensors() {
    if ((millis() - lastScanTime) > scanDelay) {
      lastScanTime = millis();

#ifdef bottomPIR_PIN
      bottomSensorRead = bottomSensorWrite || ((bottomPIR_PIN) == HIGH);
#else
      bottomSensorRead = bottomSensorWrite ||
          ultrasoundRead(bottomTriggerPin, bottomEchoPin, bottomMaxTimeUs);
#endif

#ifdef topPIR_PIN
      topSensorRead = topSensorWrite || (digitalRead(topPIR_PIN) == HIGH);
#else
      topSensorRead = topSensorWrite || ultrasoundRead(topTriggerPin, topEchoPin, topMaxTimeUs);
#endif

      // Values read, reset the flags for next API call
      topSensorWrite = false;
      bottomSensorWrite = false;

      if (topSensorRead != bottomSensorRead) {
        lastSwitchTime = millis();

        if (on) {
          lastSensor = topSensorRead;
        } else {
          // If the bottom PIR triggered, we need to swipe up, ON
          swipe = bottomSensorRead;

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
    if (on && ((millis() - lastSwitchTime) > on_time_ms)) {
      // Swipe OFF in the direction of the last PIR detection
      swipe = lastSensor;
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

  void writeSettingsToJson(JsonObject& root) {
    JsonObject staircase = root["staircase"];
    if (staircase.isNull()) {
      staircase = root.createNestedObject("staircase");
    }
    staircase["enabled"] = enabled;
    staircase["segment-delay-ms"] = segment_delay_ms;
    staircase["on-time-s"] = on_time_ms / 1000;

#ifdef topTriggerPin
    staircase["top-echo-us"] = topMaxTimeUs;
#endif
#ifdef bottomTriggerPin
    staircase["bottom-echo-us"] = bottomMaxTimeUs;
#endif
  }

  void writeSensorsToJson(JsonObject& root) {
    JsonObject staircase = root["staircase"];
    if (staircase.isNull()) {
      staircase = root.createNestedObject("staircase");
    }
    staircase["topsensor"] = topSensorRead;
    staircase["bottomsensor"] = bottomSensorRead;
  }

  void readSettingsFromJson(JsonObject& root) {
    JsonObject staircase = root["staircase"];

    bool shouldEnable = staircase["enabled"] | enabled;
    if (shouldEnable != enabled) {
      enable(shouldEnable);
    }

    segment_delay_ms = staircase["segment-delay-ms"] | segment_delay_ms;
    on_time_ms = (staircase["on-time-s"] | (on_time_ms / 1000)) * 1000;

#ifdef topTriggerPin
    topMaxTimeUs = staircase["top-echo-us"] | topMaxTimeUs;
#endif
#ifdef bottomTriggerPin
    bottomMaxTimeUs = staircase["bottom-echo-us"] | bottomMaxTimeUs;
#endif
  }

  void readSensorsFromJson(JsonObject& root) {
    JsonObject staircase = root["staircase"];
    bottomSensorWrite = bottomSensorRead || (staircase["bottomsensor"].as<bool>());
    topSensorWrite = topSensorRead || (staircase["topsensor"].as<bool>());
  }

  void enable(bool enable) {
    if (enable) {
      Serial.println("PIR Staircase enabled.");
      Serial.print("Delay between steps: ");
      Serial.print(segment_delay_ms, DEC);
      Serial.print(" milliseconds.\nStairs switch off after: ");
      Serial.print(on_time_ms / 1000, DEC);
      Serial.println(" seconds.");

#ifdef bottomPIR_PIN
      pinMode(bottomPIR_PIN, INPUT);
#else
      pinMode(bottomTriggerPin, OUTPUT);
      pinMode(bottomEchoPin, INPUT);
#endif

#ifdef topPIR_PIN
      pinMode(topPIR_PIN, INPUT);
#else
      pinMode(topTriggerPin, OUTPUT);
      pinMode(topEchoPin, INPUT);
#endif
    } else {
      // Restore segment options
      WS2812FX::Segment mainsegment = strip.getSegment(mainSegmentId);
      WS2812FX::Segment* segments = strip.getSegments();
      for (int i = 0; i < MAX_NUM_SEGMENTS; i++, segments++) {
        if (!segments->isActive()) {
          maxSegmentId = i - 1;
          break;
        }
        segments->setOption(SEG_OPTION_ON, 1, 1);
      }
      colorUpdated(NOTIFIER_CALL_MODE_DIRECT_CHANGE);
      Serial.println("PIR Staircase disabled.");
    }
    enabled = enable;
  }

 public:
  void setup() { enable(enabled); }

  void loop() {
    if (!enabled) {
      return;
    }

    checkSensors();
    autoPowerOff();
    updateSwipe();

    // Write changed settings from the json api into flash
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
   *   "enabled" : true
   *   "segment-delay-ms" : 150,
   *   "on-time-s" : 5
   * }
   *
   */
  void addToJsonState(JsonObject& root) {
    writeSettingsToJson(root);
    writeSensorsToJson(root);
    Serial.println("Staircase config exposed in API.");
  }

  /*
   * Reads configuration settings from the json API.
   * See void addToJsonState(JsonObject& root)
   */
  void readFromJsonState(JsonObject& root) {
    readSettingsFromJson(root);
    readSensorsFromJson(root);

    // The call to serializeConfig() must be done in the main loop,
    // so we set a flag to signal the main loop to save state.
    saveState = true;

    Serial.println("Staircase config read from API.");
  }

  /*
   * Writes the configuration to internal flash memory.
   */
  void addToConfig(JsonObject& root) {
    writeSettingsToJson(root);
    Serial.println("Staircase config saved.");
  }

  /*
   * Reads the configuration to internal flash memory before setup() is called.
   */
  void readFromConfig(JsonObject& root) {
    readSettingsFromJson(root);
    Serial.println("Staircase config loaded.");
  }

  /*
   * Shows the delay between steps and power-off time in the "info"
   * tab of the web-UI.
   */
  void addToJsonInfo(JsonObject& root) {
    JsonObject staircase = root["u"];
    if (staircase.isNull()) {
      staircase = root.createNestedObject("u");
    }

    if (enabled) {
      JsonArray usermodEnabled =
          staircase.createNestedArray("Staircase enabled");  // name
      usermodEnabled.add("yes");                             // value

      JsonArray segmentDelay =
          staircase.createNestedArray("Delay between stairs");  // name
      segmentDelay.add(segment_delay_ms);                       // value
      segmentDelay.add(" milliseconds");                        // unit

      JsonArray onTime =
          staircase.createNestedArray("Power-off stairs after");  // name
      onTime.add(on_time_ms / 1000);                              // value
      onTime.add(" seconds");                                     // unit
    } else {
      JsonArray usermodEnabled =
          staircase.createNestedArray("Staircase enabled");  // name
      usermodEnabled.add("no");                              // value
    }
  }
};