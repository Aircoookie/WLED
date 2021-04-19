/*
 * Usermod for detecting people entering/leaving a staircase and switching the
 * staircase on/off.
 *
 * Edit the Animated_Staircase_config.h file to compile this usermod for your
 * specific configuration.
 * 
 * See the accompanying README.md file for more info.
 */
#pragma once
#include "wled.h"
#include "Animated_Staircase_config.h"
#define USERMOD_ID_ANIMATED_STAIRCASE 1011

class Animated_Staircase : public Usermod {
 private:

  /* configuration (available in API and stored in flash) */
  bool enabled = false;                 // Enable this usermod
  unsigned long segment_delay_ms = 150; // Time between switching each segment
  unsigned long on_time_ms = 5 * 1000;  // The time for the light to stay on
  int8_t topPIRorTriggerPin = -1;       // disabled
  int8_t bottomPIRorTriggerPin = -1;    // disabled
  int8_t topEchoPin = -1;       // disabled
  int8_t bottomEchoPin = -1;    // disabled
  bool useUSSensorTop = false;          // using PIR or UltraSound sensor?
  bool useUSSensorBottom = false;       // using PIR or UltraSound sensor?
  unsigned int topMaxTimeUs = 1749;     // default echo timout, top
  unsigned int bottomMaxTimeUs = 1749;  // default echo timout, bottom

  /* runtime variables */
  bool initDone = false;

  // Time between checking of the sensors
  const int scanDelay = 50;

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

  // Time of the last sensor check
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

  // These values are used by the API to read the
  // last sensor state, or trigger a sensor
  // through the API
  bool topSensorRead = false;
  bool topSensorWrite = false;
  bool bottomSensorRead = false;
  bool bottomSensorWrite = false;  

  void updateSegments() {
//    mainSegmentId = strip.getMainSegmentId();
//    WS2812FX::Segment mainsegment = strip.getSegment(mainSegmentId);
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

    if (!useUSSensorBottom)
      bottomSensorRead = bottomSensorWrite || (digitalRead(bottomPIRorTriggerPin) == HIGH);
    else
      bottomSensorRead = bottomSensorWrite || ultrasoundRead(bottomPIRorTriggerPin, bottomEchoPin, bottomMaxTimeUs);

    if (!useUSSensorTop)
      topSensorRead = topSensorWrite || (digitalRead(topPIRorTriggerPin) == HIGH);
    else
      topSensorRead = topSensorWrite || ultrasoundRead(topPIRorTriggerPin, topEchoPin, topMaxTimeUs);

      // Values read, reset the flags for next API call
      topSensorWrite = false;
      bottomSensorWrite = false;

      if (topSensorRead != bottomSensorRead) {
        lastSwitchTime = millis();

        if (on) {
          lastSensor = topSensorRead;
        } else {
          // If the bottom sensor triggered, we need to swipe up, ON
          swipe = bottomSensorRead;

          if (swipe) {
            DEBUG_PRINTLN(F("ON -> Swipe up."));
          } else {
            DEBUG_PRINTLN(F("ON -> Swipe down."));
          }

          if (onIndex == offIndex) {
            // Position the indices for a correct on-swipe
            if (swipe == SWIPE_UP) {
              onIndex = mainSegmentId;
            } else {
              onIndex = maxSegmentId+1;
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
      // Swipe OFF in the direction of the last sensor detection
      swipe = lastSensor;
      on = false;

      if (swipe) {
        DEBUG_PRINTLN(F("OFF -> Swipe up."));
      } else {
        DEBUG_PRINTLN(F("OFF -> Swipe down."));
      }
    }
  }

  void updateSwipe() {
    if ((millis() - lastTime) > segment_delay_ms) {
      lastTime = millis();

//      byte oldOnIndex = onIndex;
//      byte oldOffIndex = offIndex;

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

  void writeSensorsToJson(JsonObject& root) {
    JsonObject staircase = root[F("staircase")];
    if (staircase.isNull()) {
      staircase = root.createNestedObject(F("staircase"));
    }
    staircase[F("top-sensor")] = topSensorRead;
    staircase[F("bottom-sensor")] = bottomSensorRead;
  }

  void readSensorsFromJson(JsonObject& root) {
    JsonObject staircase = root[F("staircase")];
    bottomSensorWrite = bottomSensorRead || (staircase[F("bottom-sensor")].as<bool>());
    topSensorWrite = topSensorRead || (staircase[F("top-sensor")].as<bool>());
  }

  void enable(bool enable) {
    if (enable) {
      DEBUG_PRINTLN(F("Animated Staircase enabled."));
      DEBUG_PRINT(F("Delay between steps: "));
      DEBUG_PRINT(segment_delay_ms);
      DEBUG_PRINT(F(" milliseconds.\nStairs switch off after: "));
      DEBUG_PRINT(on_time_ms / 1000);
      DEBUG_PRINTLN(F(" seconds."));

      if (!useUSSensorBottom)
        pinMode(bottomPIRorTriggerPin, INPUT);
      else {
        pinMode(bottomPIRorTriggerPin, OUTPUT);
        pinMode(bottomEchoPin, INPUT);
      }

      if (!useUSSensorTop)
        pinMode(topPIRorTriggerPin, INPUT);
      else {
        pinMode(topPIRorTriggerPin, OUTPUT);
        pinMode(topEchoPin, INPUT);
      }
    } else {
      // Restore segment options
//      WS2812FX::Segment mainsegment = strip.getSegment(mainSegmentId);
      WS2812FX::Segment* segments = strip.getSegments();
      for (int i = 0; i < MAX_NUM_SEGMENTS; i++, segments++) {
        if (!segments->isActive()) {
          maxSegmentId = i - 1;
          break;
        }
        segments->setOption(SEG_OPTION_ON, 1, 1);
      }
      colorUpdated(NOTIFIER_CALL_MODE_DIRECT_CHANGE);
      DEBUG_PRINTLN(F("Animated Staircase disabled."));
    }
    enabled = enable;
  }

 public:
  void setup() {
    // allocate pins
    if (topPIRorTriggerPin >= 0) {
      if (!pinManager.allocatePin(topPIRorTriggerPin,useUSSensorTop))
        topPIRorTriggerPin = -1;
    }
    if (topEchoPin >= 0) {
      if (!pinManager.allocatePin(topEchoPin,false))
        topEchoPin = -1;
    }
    if (bottomPIRorTriggerPin >= 0) {
      if (!pinManager.allocatePin(bottomPIRorTriggerPin,useUSSensorBottom))
        bottomPIRorTriggerPin = -1;
    }
    if (bottomEchoPin >= 0) {
      if (!pinManager.allocatePin(bottomPIRorTriggerPin,false))
        bottomEchoPin = -1;
    }

    // validate pins
    if ( topPIRorTriggerPin < 0 || bottomPIRorTriggerPin < 0 ||
        (useUSSensorTop && topEchoPin < 0) || (useUSSensorBottom && bottomEchoPin < 0) )
      enabled = false;

    enable(enabled);
    initDone = true;
  }

  void loop() {
    if (!enabled) return;
    checkSensors();
    autoPowerOff();
    updateSwipe();
  }

  uint16_t getId() { return USERMOD_ID_ANIMATED_STAIRCASE; }

  void addToJsonState(JsonObject& root) {
//    writeSettingsToJson(root);
//    writeSensorsToJson(root);
//    DEBUG_PRINTLN(F("Staircase config exposed in API."));
  }

  /*
   * Reads configuration settings from the json API.
   * See void addToJsonState(JsonObject& root)
   */
  void readFromJsonState(JsonObject& root) {
    if (!initDone) return;  // prevent crash on boot applyPreset()
    JsonObject staircase = root[F("staircase")];
    if (!staircase.isNull()) {
      if (staircase[F("enabled")].is<bool>()) {
        enabled   = staircase[F("enabled")].as<bool>();
      } else {
        String str = staircase[F("enabled")]; // checkbox -> off or on
        enabled = (bool)(str!="off"); // off is guaranteed to be present
      }
    }
  }

  /*
   * Writes the configuration to internal flash memory.
   */
  void addToConfig(JsonObject& root) {
    JsonObject staircase = root[F("staircase")];
    if (staircase.isNull()) {
      staircase = root.createNestedObject(F("staircase"));
    }
    staircase[F("enabled")] = enabled;
    staircase[F("segment-delay-ms")] = segment_delay_ms;
    staircase[F("on-time-s")] = on_time_ms / 1000;
    staircase[F("useTopUltrasoundSensor")] = useUSSensorTop;
    staircase[F("topPIRorTrigger_pin")] = topPIRorTriggerPin;
    staircase[F("topEcho_pin")] = topEchoPin;
    staircase[F("useBottomUltrasoundSensor")] = useUSSensorBottom;
    staircase[F("bottomPIRorTrigger_pin")] = bottomPIRorTriggerPin;
    staircase[F("bottomEcho_pin")] = bottomEchoPin;
    staircase[F("top-echo-us")] = topMaxTimeUs;
    staircase[F("bottom-echo-us")] = bottomMaxTimeUs;
    DEBUG_PRINTLN(F("Staircase config saved."));
  }

  /*
   * Reads the configuration to internal flash memory before setup() is called.
   */
  void readFromConfig(JsonObject& root) {
    bool oldUseUSSensorTop = useUSSensorTop;
    bool oldUseUSSensorBottom = useUSSensorBottom;
    int8_t oldTopAPin = topPIRorTriggerPin;
    int8_t oldTopBPin = topEchoPin;
    int8_t oldBottomAPin = bottomPIRorTriggerPin;
    int8_t oldBottomBPin = bottomEchoPin;

    JsonObject staircase = root[F("staircase")];
    if (!staircase.isNull()) {
      if (staircase[F("enabled")].is<bool>()) {
        enabled   = staircase[F("enabled")].as<bool>();
      } else {
        String str = staircase[F("enabled")]; // checkbox -> off or on
        enabled = (bool)(str!="off"); // off is guaranteed to be present
      }
      segment_delay_ms = staircase[F("segment-delay-ms")];
      on_time_ms = (int)staircase[F("on-time-s")] * 1000;
      if (staircase[F("useTopUltrasoundSensor")].is<bool>()) {
        useUSSensorTop = staircase[F("useTopUltrasoundSensor")].as<bool>();
      } else {
        String str = staircase[F("useTopUltrasoundSensor")]; // checkbox -> off or on
        useUSSensorTop = (bool)(str!="off"); // off is guaranteed to be present
      }
      topPIRorTriggerPin = staircase[F("topPIRorTrigger_pin")];
      topEchoPin = staircase[F("topEcho_pin")];
      useUSSensorBottom = staircase[F("useBottomUltrasoundSensor")].as<bool>();
      if (staircase[F("useBottomUltrasoundSensor")].is<bool>()) {
        useUSSensorBottom = staircase[F("useBottomUltrasoundSensor")].as<bool>();
      } else {
        String str = staircase[F("useBottomUltrasoundSensor")]; // checkbox -> off or on
        useUSSensorBottom = (bool)(str!="off"); // off is guaranteed to be present
      }
      bottomPIRorTriggerPin = staircase[F("bottomPIRorTrigger_pin")];
      bottomEchoPin = staircase[F("bottomEcho_pin")];
      topMaxTimeUs = staircase[F("top-echo-us")];
      bottomMaxTimeUs = staircase[F("bottom-echo-us")];
      DEBUG_PRINTLN(F("Staircase config (re)loaded."));
    } else {
      DEBUG_PRINTLN(F("No config found. (Using defaults.)"));
    }
    if (!initDone) {
      // first run: reading from cfg.json
    } else {
      // changing paramters from settings page
      bool changed = false;
      if ((oldUseUSSensorTop != useUSSensorTop) ||
          (oldUseUSSensorBottom != useUSSensorBottom) ||
          (oldTopAPin != topPIRorTriggerPin) ||
          (oldTopBPin != topEchoPin) ||
          (oldBottomAPin != bottomPIRorTriggerPin) ||
          (oldBottomBPin != bottomEchoPin)) {
        changed = true;
        pinManager.deallocatePin(oldTopAPin);
        pinManager.deallocatePin(oldTopBPin);
        pinManager.deallocatePin(oldBottomAPin);
        pinManager.deallocatePin(oldBottomBPin);
      }
      if (changed) setup();
    }
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
      JsonArray usermodEnabled = staircase.createNestedArray(F("Staircase enabled"));  // name
      usermodEnabled.add("yes");                             // value

      JsonArray segmentDelay = staircase.createNestedArray(F("Delay between stairs"));  // name
      segmentDelay.add(segment_delay_ms);                       // value
      segmentDelay.add("ms");                        // unit

      JsonArray onTime = staircase.createNestedArray(F("Power-off stairs after"));  // name
      onTime.add(on_time_ms / 1000);                              // value
      onTime.add("s");                                     // unit
    } else {
      JsonArray usermodEnabled = staircase.createNestedArray(F("Staircase enabled"));  // name
      usermodEnabled.add("no");                              // value
    }
  }
};