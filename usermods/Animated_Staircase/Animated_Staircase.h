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
  const unsigned int scanDelay = 50;

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

  // strings to reduce flash memory usage (used more than twice)
  static const char _name[];
  static const char _enabled[];
  static const char _segmentDelay[];
  static const char _onTime[];
  static const char _useTopUltrasoundSensor[];
  static const char _topPIRorTrigger_pin[];
  static const char _topEcho_pin[];
  static const char _useBottomUltrasoundSensor[];
  static const char _bottomPIRorTrigger_pin[];
  static const char _bottomEcho_pin[];
  static const char _topEchoTime[];
  static const char _bottomEchoTime[];
  static const char _[];

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
    // TODO: add logic to wait until PIR sensor deactivates
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

  // send sesnor values to JSON API
  void writeSensorsToJson(JsonObject& staircase) {
    staircase[F("top-sensor")] = topSensorRead;
    staircase[F("bottom-sensor")] = bottomSensorRead;
  }

  // allow overrides from JSON API
  void readSensorsFromJson(JsonObject& staircase) {
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

      // TODO: attach interrupts
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
    // TODO: attach interrupts in enable()

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
    JsonObject staircase = root[FPSTR(_name)];
    if (staircase.isNull()) {
      staircase = root.createNestedObject(FPSTR(_name));
    }
    writeSensorsToJson(staircase);
    DEBUG_PRINTLN(F("Staircase sensor state exposed in API."));
  }

  /*
   * Reads configuration settings from the json API.
   * See void addToJsonState(JsonObject& root)
   */
  void readFromJsonState(JsonObject& root) {
    if (!initDone) return;  // prevent crash on boot applyPreset()
    JsonObject staircase = root[FPSTR(_name)];
    if (!staircase.isNull()) {
      if (staircase[FPSTR(_enabled)].is<bool>()) {
        enabled   = staircase[FPSTR(_enabled)].as<bool>();
      } else {
        String str = staircase[FPSTR(_enabled)]; // checkbox -> off or on
        enabled = (bool)(str!="off"); // off is guaranteed to be present
      }
      readSensorsFromJson(root);
      DEBUG_PRINTLN(F("Staircase sensor state read from API."));
    }
  }

  /*
   * Writes the configuration to internal flash memory.
   */
  void addToConfig(JsonObject& root) {
    JsonObject staircase = root[FPSTR(_name)];
    if (staircase.isNull()) {
      staircase = root.createNestedObject(FPSTR(_name));
    }
    staircase[FPSTR(_enabled)]                   = enabled;
    staircase[FPSTR(_segmentDelay)]              = segment_delay_ms;
    staircase[FPSTR(_onTime)]                    = on_time_ms / 1000;
    staircase[FPSTR(_useTopUltrasoundSensor)]    = useUSSensorTop;
    staircase[FPSTR(_topPIRorTrigger_pin)]       = topPIRorTriggerPin;
    staircase[FPSTR(_topEcho_pin)]               = useUSSensorTop ? topEchoPin : -1;
    staircase[FPSTR(_useBottomUltrasoundSensor)] = useUSSensorBottom;
    staircase[FPSTR(_bottomPIRorTrigger_pin)]    = bottomPIRorTriggerPin;
    staircase[FPSTR(_bottomEcho_pin)]            = useUSSensorBottom ? bottomEchoPin : -1;
    staircase[FPSTR(_topEchoTime)]               = topMaxTimeUs;
    staircase[FPSTR(_bottomEchoTime)]            = bottomMaxTimeUs;
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

    JsonObject staircase = root[FPSTR(_name)];
    if (!staircase.isNull()) {
      if (staircase[FPSTR(_enabled)].is<bool>()) {
        enabled   = staircase[FPSTR(_enabled)].as<bool>();
      } else {
        String str = staircase[FPSTR(_enabled)]; // checkbox -> off or on
        enabled = (bool)(str!="off"); // off is guaranteed to be present
      }
      segment_delay_ms = min(10000,max(10,staircase[FPSTR(_segmentDelay)].as<int>()));  // max delay 10s
      on_time_ms = min(900,max(10,staircase[FPSTR(_onTime)].as<int>())) * 1000;    // min 10s, max 15min

      if (staircase[FPSTR(_useTopUltrasoundSensor)].is<bool>()) {
        useUSSensorTop = staircase[FPSTR(_useTopUltrasoundSensor)].as<bool>();
      } else {
        String str = staircase[FPSTR(_useTopUltrasoundSensor)]; // checkbox -> off or on
        useUSSensorTop = (bool)(str!="off"); // off is guaranteed to be present
      }

      topPIRorTriggerPin = min(39,max(-1,staircase[FPSTR(_topPIRorTrigger_pin)].as<int>()));
      topEchoPin         = min(39,max(-1,staircase[FPSTR(_topEcho_pin)].as<int>()));

      if (staircase[FPSTR(_useBottomUltrasoundSensor)].is<bool>()) {
        useUSSensorBottom = staircase[FPSTR(_useBottomUltrasoundSensor)].as<bool>();
      } else {
        String str = staircase[FPSTR(_useBottomUltrasoundSensor)]; // checkbox -> off or on
        useUSSensorBottom = (bool)(str!="off"); // off is guaranteed to be present
      }
      bottomPIRorTriggerPin = min(39,max(-1,staircase[FPSTR(_bottomPIRorTrigger_pin)].as<int>()));
      bottomEchoPin         = min(39,max(-1,staircase[FPSTR(_bottomEcho_pin)].as<int>()));
      topMaxTimeUs          = min(18000,max(300,staircase[FPSTR(_topEchoTime)].as<int>()));     // max distnace ~3m (a noticable lag of 18ms may be expected)
      bottomMaxTimeUs       = min(18000,max(300,staircase[FPSTR(_bottomEchoTime)].as<int>()));  // max distance ~3m (a noticable lag of 18ms may be expected)
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

// strings to reduce flash memory usage (used more than twice)
const char Animated_Staircase::_name[]                      PROGMEM = "staircase";
const char Animated_Staircase::_enabled[]                   PROGMEM = "enabled";
const char Animated_Staircase::_segmentDelay[]              PROGMEM = "segment-delay-ms";
const char Animated_Staircase::_onTime[]                    PROGMEM = "on-time-s";
const char Animated_Staircase::_useTopUltrasoundSensor[]    PROGMEM = "useTopUltrasoundSensor";
const char Animated_Staircase::_topPIRorTrigger_pin[]       PROGMEM = "topPIRorTrigger_pin";
const char Animated_Staircase::_topEcho_pin[]               PROGMEM = "topEcho_pin";
const char Animated_Staircase::_useBottomUltrasoundSensor[] PROGMEM = "useBottomUltrasoundSensor";
const char Animated_Staircase::_bottomPIRorTrigger_pin[]    PROGMEM = "bottomPIRorTrigger_pin";
const char Animated_Staircase::_bottomEcho_pin[]            PROGMEM = "bottomEcho_pin";
const char Animated_Staircase::_topEchoTime[]               PROGMEM = "top-echo-us";
const char Animated_Staircase::_bottomEchoTime[]            PROGMEM = "bottom-echo-us";
