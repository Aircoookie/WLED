/*
 * Usermod for detecting people entering/leaving a staircase and switching the
 * every stair depending on distance of the person.
 *
 * Edit the Distance_Staircase_config.h file to compile this usermod for your
 * specific configuration.
 * 
 * See the accompanying README.md file for more info.
 */
#pragma once
#include "wled.h"

class Distance_Staircase : public Usermod {
  private:

    class Timer {
      unsigned long delay_ms;
      unsigned long lastTime = 0;
    public:
      explicit Timer(unsigned long delay_ms): delay_ms{delay_ms} {}

      void reset() {
        lastTime = millis();
      }

      bool wouldBlock() {
        return ((millis() - lastTime) < delay_ms);
      }

      bool isEarly() {
        if (wouldBlock()) {
          return true;
        }
        reset();
        return false;
      }
    };

    enum WalkDirection {
      Up, Down
    };

    enum AnimationState : uint8_t {
      None, Enter, FollowDistance, Finish, CoolDown, Reset
    };

    struct State {
      private:
        static const long on_time_ms          = 20000; // The time for the light to stay on
        static const long invite_time_ms      = 5000;  // The time for the light to stay on without movement
        long lastChange = millis();

        AnimationState _animation = None;
      public:
      WalkDirection direction = Up;

      State() {}
      State(AnimationState ani, WalkDirection dir): _animation{ani}, direction{dir} {
        lastChange = millis();
      }
      
      const AnimationState& animation() {
        return _animation;
      }

      void set(AnimationState ani) {
        _animation = ani;
        lastChange = millis();
      }

      bool inviteTimeOver() {
        return (millis() - lastChange) > invite_time_ms;
      }

      bool onTimeOver() {
        return (millis() - lastChange) > on_time_ms;
      }
    };

    template<int SIZE = 6>
    class SmoothMeasure {
      int lastDistancesMeasuredPosition = 0;
      int lastDistancesMeasured[SIZE] = {0};
      int init = 0;
    public:
      int value = 0, lastValue = 0;

      SmoothMeasure() {}

      bool hasChanged() {
        return value > 0 && value != lastValue;
      }

      void reset() {
        init = 0;
        lastDistancesMeasuredPosition = 0;
      }

      void feed(int cm) {
        lastDistancesMeasured[lastDistancesMeasuredPosition] = cm;
        lastDistancesMeasuredPosition = (lastDistancesMeasuredPosition + 1) % SIZE;

        if (init != SIZE) {
          init++;
        }

        int min = std::numeric_limits<int>::max();
        int max = std::numeric_limits<int>::min();
        int sum = 0;
        for (auto measure : lastDistancesMeasured) {
          min = MIN(min, measure);
          max = MAX(max, measure);
          sum += measure;
        }
        sum -= min + max;
        lastValue = value;
        value = sum / (SIZE - 2);
      }
    };

    State state;

    Timer animationTimer = Timer(500);
    Timer finishSwipeTimer = Timer(250);
    Timer scanTimer = Timer(50);
    Timer coolDownTimer = Timer(3000);

    /* configuration (available in API and stored in flash) */
    bool enabled = false;           // Enable this usermod
    int8_t triggerPin      = -1;    // disabled
    int8_t echoPin         = -1;    // disabled
    int8_t topPIRPin       = -1;    // disabled
    int8_t bottomPIRPin    = -1;    // disabled

    int endOfStairsDistance = 145;

    /* runtime variables */
    bool initDone = false;

    bool HAautodiscovery = true;

    // segment id between onIndex and offIndex are on.
    // control the swipe by setting/moving these indices around.
    // onIndex must be less than or equal to offIndex
    byte onIndex = 0;
    byte offIndex = 0;

    // The maximum number of configured segments.
    // Dynamically updated based on user configuration.
    byte maxSegmentId = 1;
    byte minSegmentId = 0;

    bool topSensorState    = false;
    bool bottomSensorState = false;
    SmoothMeasure<3> distanceState;
    WalkDirection lastActiveSensor = Down;


    // strings to reduce flash memory usage (used more than twice)
    static const char _name[];
    static const char _enabled[];
    static const char _onTime[];
    static const char _endOfStairsDistance[];
    static const char _HAautodiscovery[];
    static const char _topPIRPin[];
    static const char _bottomPIRPin[];
    static const char _sonarTriggerPin[];
    static const char _sonarEchoPin[];
    
    enum MotionType {
      Bottom, Top, WentUp, WentDown
    };

    const char* motionTypeName(MotionType type) {
      switch (type) {
      case Bottom: return "Bottom";
      case Top: return "Top";
      case WentUp: return "WentUp";
      case WentDown: return "WentDown";
      }
      return "unknown";
    }

    void publishMqtt(MotionType type, bool state) {
#ifndef WLED_DISABLE_MQTT
      //Check if MQTT Connected, otherwise it will crash the 8266
      if (WLED_MQTT_CONNECTED){
        char subuf[64];
        sprintf_P(subuf, PSTR("%s/motion/%s"), mqttDeviceTopic, motionTypeName(type));
        mqtt->publish(subuf, 0, false, state ? "on" : "off");
      }
#endif
    }

    void publishDistanceMqtt() {
#ifndef WLED_DISABLE_MQTT
      //Check if MQTT Connected, otherwise it will crash the 8266
      if (WLED_MQTT_CONNECTED){
        char subuf[64];
        char dist[16];
        sprintf_P(subuf, PSTR("%s/distance/0"), mqttDeviceTopic);
        sprintf_P(dist, (PGM_P)F("%03d"), distanceState.value);
        mqtt->publish(subuf, 0, true, dist);
      }
#endif
    }

    void publichHomeAssistantAutodiscoveryMotionSensor(MotionType type) {
        StaticJsonDocument<600> doc;
        char uid[24], json_str[1024], buf[128];

        const char* name = motionTypeName(type);

        sprintf_P(buf, PSTR("%s %s Motion"), serverDescription, name); //max length: 33 + 7 = 40
        doc[F("name")] = buf;
        sprintf_P(buf, PSTR("%s/motion/%s"), mqttDeviceTopic, name);   //max length: 33 + 7 = 40
        doc[F("stat_t")] = buf;
        doc[F("pl_on")]  = "on";
        doc[F("pl_off")] = "off";
        sprintf_P(uid, PSTR("%s_%s_motion"), escapedMac.c_str(), name);
        doc[F("uniq_id")] = uid;
        doc[F("dev_cla")] = F("motion");

        JsonObject device = doc.createNestedObject(F("device")); // attach the sensor to the same device
        device[F("name")] = serverDescription;
        device[F("ids")]  = String(F("wled-sensor-")) + mqttClientID;
        device[F("mf")]   = "WLED";
        device[F("mdl")]  = F("FOSS");
        device[F("sw")]   = versionString;
    
        sprintf_P(buf, PSTR("homeassistant/binary_sensor/%s/config"), uid);
        DEBUG_PRINTLN(buf);
        size_t payload_size = serializeJson(doc, json_str);
        DEBUG_PRINTLN(json_str);

        mqtt->publish(buf, 0, true, json_str, payload_size); // do we really need to retain?
    }

    void publichHomeAssistantAutodiscoveryDistanceSensor() {
        StaticJsonDocument<600> doc;
        char uid[24], json_str[1024], buf[128];

        sprintf_P(buf, PSTR("%s Distance"), serverDescription); //max length: 33 + 7 = 40
        doc[F("name")] = buf;
        sprintf_P(buf, PSTR("%s/distance/0"), mqttDeviceTopic);   //max length: 33 + 7 = 40
        doc[F("stat_t")] = buf;
        sprintf_P(uid, PSTR("%s_distance"), escapedMac.c_str());
        doc[F("uniq_id")] = uid;
        doc[F("dev_cla")] = F("distance");
        doc[F("unit_of_measurement")] = F("cm");

        JsonObject device = doc.createNestedObject(F("device")); // attach the sensor to the same device
        device[F("name")] = serverDescription;
        device[F("ids")]  = String(F("wled-sensor-")) + mqttClientID;
        device[F("mf")]   = "WLED";
        device[F("mdl")]  = F("FOSS");
        device[F("sw")]   = versionString;
    
        sprintf_P(buf, PSTR("homeassistant/sensor/%s/config"), uid);
        DEBUG_PRINTLN(buf);
        size_t payload_size = serializeJson(doc, json_str);
        DEBUG_PRINTLN(json_str);

        mqtt->publish(buf, 0, true, json_str, payload_size); // do we really need to retain?
    }

    void onMqttConnect(bool sessionPresent) {
#ifndef WLED_DISABLE_MQTT
      if (HAautodiscovery) {
        publichHomeAssistantAutodiscoveryMotionSensor(Top);
        publichHomeAssistantAutodiscoveryMotionSensor(Bottom);
        publichHomeAssistantAutodiscoveryMotionSensor(WentUp);
        publichHomeAssistantAutodiscoveryMotionSensor(WentDown);
        publichHomeAssistantAutodiscoveryDistanceSensor();
      }
#endif
    }

    void updateSegments() {
      for (int i = minSegmentId; i <= maxSegmentId; i++) {
        Segment &seg = strip.getSegment(i);
        if (!seg.isActive()) {
          continue; // skip gaps
        }
        if (onIndex <= i && i < offIndex) {
          seg.setOption(SEG_OPTION_ON, true);
        } else {
          seg.setOption(SEG_OPTION_ON, false);
        }
      }
      strip.trigger();  // force strip refresh
      stateChanged = true;  // inform external devices/UI of change
      colorUpdated(CALL_MODE_DIRECT_CHANGE);
    }

    int ultrasoundRead(int8_t signalPin, int8_t echoPin) {
      if (signalPin<0 || echoPin<0) return false;
      digitalWrite(signalPin, LOW);
      delayMicroseconds(2);
      digitalWrite(signalPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(signalPin, LOW);
      int duration = pulseIn(echoPin, HIGH, 100000);
      int cm = (duration / 2) / 29.1;     // Divide by 29.1 or multiply by 0.0343
      if (cm < 1 || cm > 400) {
        return -1;
      }
      return cm;
    }

    void checkSensors() {
      if (scanTimer.isEarly()) {
        return;
      }
      
      int topMovement = digitalRead(topPIRPin);
      if (topMovement != topSensorState) {
        topSensorState = topMovement;
        publishMqtt(Top, topSensorState);
        if (topMovement) {
          lastActiveSensor = Up;
        }
      }

      int bottomMovement = digitalRead(bottomPIRPin);
      if (bottomMovement != bottomSensorState) {
        bottomSensorState = bottomMovement;
        publishMqtt(Bottom, bottomSensorState);
        if (bottomMovement) {
          lastActiveSensor = Down;
        }
      }

      if (state.animation() != None) {
        int distance = ultrasoundRead(triggerPin, echoPin);
        distanceState.feed(distance);
        if (distanceState.hasChanged()) {
          publishDistanceMqtt();
        }
      }
    }

    void animateNoneState() {
      if (topSensorState || bottomSensorState) {
        state = State(Enter, topSensorState? Down : Up);
        if (state.direction == Up) {
          onIndex = maxSegmentId - 1;   
          offIndex = maxSegmentId;
        } else {
          onIndex = minSegmentId;
          offIndex = minSegmentId + 2;
        }
      }
    }

    void animateEnterState() {
      if (state.inviteTimeOver() && !bottomSensorState && !topSensorState) {
        state.set(Reset);
      }
      if (0 < distanceState.value && distanceState.value < endOfStairsDistance) {
        state.set(FollowDistance);
      }
    }

    void animateFollowDistanceState() {
      if (animationTimer.isEarly()) {
        return;
      }
      
      if ((lastActiveSensor == state.direction && distanceState.value > endOfStairsDistance) ||     // Person went through
          (state.onTimeOver() && !bottomSensorState && !topSensorState))
      {
          state.set(Finish);
      }
      
      if (state.direction == Up) {
        onIndex = MAX(minSegmentId, distanceState.value / (endOfStairsDistance / strip.getSegmentsNum()) - 4);
      } else {
        offIndex = MIN(maxSegmentId + 1, distanceState.value / (endOfStairsDistance / strip.getSegmentsNum()) + 3);
      }
    }

    void animateFinishState() {
      if (finishSwipeTimer.isEarly()) {
        return;
      }

      if (state.direction == Up) {
        offIndex--;
        onIndex = 0;
      } else {
        onIndex++;
      }

      if (onIndex == offIndex) {
        if (state.direction == Up) {
          publishMqtt(WentUp, true);
        } else {
          publishMqtt(WentDown, true); 
        }
        coolDownTimer.reset();
        state.set(CoolDown);
      }
    }

    void animate() {
      byte oldOn  = onIndex;
      byte oldOff = offIndex;
        
      switch (state.animation()) {
      case None:
        animateNoneState();
        break;
      case Enter:
        animateEnterState();
        break;
      case FollowDistance:
        animateFollowDistanceState();
        break;
      case Finish:
        animateFinishState();
        break;
      case CoolDown:
        if (!coolDownTimer.isEarly()) {
          publishMqtt(WentUp, false);
          publishMqtt(WentDown, false);
          state.set(Reset);
        }
        break;
      case Reset:
        distanceState.reset();
        onIndex  = 0;
        offIndex = 0;
        state.set(None);
      }

      if (oldOn != onIndex || oldOff != offIndex) {
        updateSegments();
      }
    }

    void enable(bool enable) {
      manageSegments();
      updateSegments();

      if (!enable) {        
        state.set(Reset);
      } else {
        pinMode(topPIRPin, INPUT_PULLUP);
        pinMode(bottomPIRPin, INPUT_PULLUP);
        pinMode(triggerPin, OUTPUT);
        pinMode(echoPin, INPUT);
      }
      enabled = enable;
    }

    uint8_t getFirstActiveSegmentId(void) {
      for (size_t i = 0; i < strip.getSegmentsNum(); i++) {
        if (strip.getSegment(i).isActive()) return i;
      }
      return 0;
    }

    void manageSegments() {
      minSegmentId = getFirstActiveSegmentId();
      maxSegmentId = strip.getLastActiveSegmentId();
    }

  public:
    void setup() {
      // standardize invalid pin numbers to -1
      if (triggerPin    < 0) triggerPin    = -1;
      if (echoPin       < 0) echoPin       = -1;
      if (topPIRPin     < 0) topPIRPin     = -1;
      if (bottomPIRPin  < 0) bottomPIRPin  = -1;
      // allocate pins
      PinManagerPinType pins[4] = {
        { triggerPin, true },
        { topPIRPin, false },
        { bottomPIRPin, false },
        { echoPin, false }
      };
      // NOTE: this *WILL* return TRUE if all the pins are set to -1.
      //       this is *BY DESIGN*.
      if (!pinManager.allocateMultiplePins(pins, 4, PinOwner::UM_DistanceStaircase)) {
        triggerPin = -1;
        echoPin = -1;
        topPIRPin = -1;
        bottomPIRPin = -1;
        enabled = false;
      }
      enable(enabled);
      initDone = true;
    }

    void loop() {
      if (!enabled || strip.isUpdating()) return;
      
      manageSegments();
      checkSensors();
      animate();
    }

    uint16_t getId() { return USERMOD_ID_DISTANCE_STAIRCASE; }

    void addToJsonState(JsonObject& root) {
      JsonObject staircase = root[FPSTR(_name)];
      if (staircase.isNull()) {
        staircase = root.createNestedObject(FPSTR(_name));
      }
      staircase[F("top-sensor")]    = topSensorState;
      staircase[F("bottom-sensor")] = bottomSensorState;
    }

    /*
    * Reads configuration settings from the json API.
    * See void addToJsonState(JsonObject& root)
    */
    void readFromJsonState(JsonObject& root) {
      if (!initDone) {
        return;  // prevent crash on boot applyPreset()
      }
      bool en = enabled;
      JsonObject staircase = root[FPSTR(_name)];
      if (!staircase.isNull()) {
        if (staircase[FPSTR(_enabled)].is<bool>()) {
          en = staircase[FPSTR(_enabled)].as<bool>();
        } else {
          String str = staircase[FPSTR(_enabled)]; // checkbox -> off or on
          en = (bool)(str != "off"); // off is guaranteed to be present
        }
        if (en != enabled) {
          enable(en);
        }
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
      staircase[FPSTR(_enabled)]               = enabled;
      staircase[FPSTR(_sonarTriggerPin)]       = triggerPin;
      staircase[FPSTR(_sonarEchoPin)]          = echoPin;
      staircase[FPSTR(_topPIRPin)]             = topPIRPin;
      staircase[FPSTR(_bottomPIRPin)]          = bottomPIRPin;
      staircase[FPSTR(_HAautodiscovery)]       = HAautodiscovery;
      DEBUG_PRINTLN(F("Staircase config saved."));
    }

    /*
    * Reads the configuration to internal flash memory before setup() is called.
    * 
    * The function should return true if configuration was successfully loaded or false if there was no configuration.
    */
    bool readFromConfig(JsonObject& root) {
      int8_t oldTriggerPin = triggerPin;
      int8_t oldEchoPin = echoPin;
      int8_t oldTopPirPin = topPIRPin;
      int8_t oldBottomPirPin = bottomPIRPin;
      
      JsonObject top = root[FPSTR(_name)];
      if (top.isNull()) {
        DEBUG_PRINT(FPSTR(_name));
        DEBUG_PRINTLN(F(": No config found. (Using defaults.)"));
        return false;
      }

      enabled = top[FPSTR(_enabled)] | enabled;

      HAautodiscovery = top[FPSTR(_HAautodiscovery)] | HAautodiscovery;

      endOfStairsDistance = top[FPSTR(_endOfStairsDistance)] | endOfStairsDistance;
      
      triggerPin = top[FPSTR(_sonarTriggerPin)] | triggerPin;
      echoPin    = top[FPSTR(_sonarEchoPin)] | echoPin;

      topPIRPin    = top[FPSTR(_topPIRPin)] | topPIRPin;
      bottomPIRPin    = top[FPSTR(_bottomPIRPin)] | bottomPIRPin;

      DEBUG_PRINT(FPSTR(_name));
      if (!initDone) {
        // first run: reading from cfg.json
        DEBUG_PRINTLN(F(" config loaded."));
      } else {
        // changing parameters from settings page
        DEBUG_PRINTLN(F(" config (re)loaded."));
        bool changed = false;
        if ((oldTriggerPin != triggerPin) || 
            (oldTopPirPin != topPIRPin) || 
            (oldBottomPirPin != bottomPIRPin) || 
            (oldEchoPin != echoPin)) {
          changed = true;
          pinManager.deallocatePin(oldTriggerPin, PinOwner::UM_DistanceStaircase);
          pinManager.deallocatePin(oldEchoPin, PinOwner::UM_DistanceStaircase);
          pinManager.deallocatePin(oldTopPirPin, PinOwner::UM_DistanceStaircase);
          pinManager.deallocatePin(oldBottomPirPin, PinOwner::UM_DistanceStaircase);
        }
        if (changed) setup();
      }
      return true;
    }

    void addToJsonInfo(JsonObject& root) {
      JsonObject user = root["u"];
      if (user.isNull()) {
        user = root.createNestedObject("u");
      }

      JsonArray infoArr = user.createNestedArray(FPSTR(_name));  // name

      String uiDomString  = F(" <button class=\"btn btn-xs\" onclick=\"requestJson({");
      uiDomString += FPSTR(_name);
      uiDomString += F(":{");
      uiDomString += FPSTR(_enabled);
      if (enabled) {
        uiDomString += F(":false}});\">");
        uiDomString += F("<i class=\"icons on\">&#xe325;</i>");
      } else {
        uiDomString += F(":true}});\">");
        uiDomString += F("<i class=\"icons off\">&#xe08f;</i>");
      }
      uiDomString += F("</button>");
      infoArr.add(uiDomString);
      infoArr.add(distanceState.value);
    }
};

// strings to reduce flash memory usage (used more than twice)
const char Distance_Staircase::_name[]                      PROGMEM = "staircase";
const char Distance_Staircase::_onTime[]                    PROGMEM = "maxTimeSeconds";
const char Distance_Staircase::_enabled[]                   PROGMEM = "enabled";
const char Distance_Staircase::_endOfStairsDistance[]       PROGMEM = "endOfStairsDistanceCM";
const char Distance_Staircase::_sonarTriggerPin[]           PROGMEM = "SonarTriggerPin";
const char Distance_Staircase::_sonarEchoPin[]              PROGMEM = "SonarEchoPin";
const char Distance_Staircase::_topPIRPin[]                 PROGMEM = "TopPIRPin";
const char Distance_Staircase::_bottomPIRPin[]              PROGMEM = "BottomPIRPin";
const char Distance_Staircase::_HAautodiscovery[]           PROGMEM = "HA-autodiscovery";

