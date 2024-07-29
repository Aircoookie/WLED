#pragma once

#include "wled.h"

#ifndef TEST_BUTTONS_NUMBER
  #define TEST_BUTTONS_NUMBER 0
#endif

#ifndef TEST_BRIGHTNESS
  #define TEST_BRIGHTNESS 60
#endif

#ifndef TEST_EFFECT
  #define TEST_EFFECT FX_MODE_FADE
#endif

// TODO: Move away from inlining all functions

class TestModeUsermod : public Usermod {
  private:
    bool enabled = true;
    bool initDone = false;
    unsigned long lastTime = 0;
    int8_t testButtons[WLED_MAX_BUTTONS];
    int8_t heldButtons = 0;
    bool active = false;

  public:
    // strings to reduce flash memory usage (used more than twice)
    static const char _name[];
    static const char _enabled[];
    inline void enable(bool enable) { enabled = enable; }
    inline bool isEnabled() { return enabled; }
    void setup() {
      initDone = true;
    }

    void loop() {
      if (!enabled) { return; }

      if (millis() - lastTime > 1000) {
        lastTime = millis();
        heldButtons = 0;
        for (unsigned i=0; i<WLED_MAX_BUTTONS; i++) {
          if (testButtons[i] == 1) {
            heldButtons += 1;
          }
        }
        if (TEST_BUTTONS_NUMBER > 0 && heldButtons >= TEST_BUTTONS_NUMBER) {
          DEBUG_PRINT(F("Test Mode activated"));
          active = true;
          bri = TEST_BRIGHTNESS;
          stateUpdated(CALL_MODE_DIRECT_CHANGE);
          effectCurrent = TEST_EFFECT;
          colorUpdated(CALL_MODE_DIRECT_CHANGE);
          enable(false);
          serializeConfig();
        }
      
      }

      return;
    }

    bool handleButton(uint8_t b) override {
      yield();
      // once test mode has been activated keep handling buttons for this boot
      if (!active) {
        if (!enabled
        // ignore certain button types as they may have other consequences
        || buttonType[b] == BTN_TYPE_NONE
        || buttonType[b] == BTN_TYPE_RESERVED
        || buttonType[b] == BTN_TYPE_PIR_SENSOR
        || buttonType[b] == BTN_TYPE_ANALOG
        || buttonType[b] == BTN_TYPE_ANALOG_INVERTED) {
          return false;
        }
      }

      bool handled = false;
      // do your button handling here
      unsigned long now = millis();

      if (isButtonPressed(b)) { //pressed
        if (!buttonPressedBefore[b]) buttonPressedTime[b] = now;
        buttonPressedBefore[b] = true;

        if (now - buttonPressedTime[b] > 600) {
            testButtons[b] = 1;
            // DEBUG_PRINT(F("Button held: ")); DEBUG_PRINTLN(b);
        }
        buttonLongPressed[b] = true;
      }
      else if (!isButtonPressed(b) && buttonPressedBefore[b]) {
        testButtons[b] = 0;
        // DEBUG_PRINT(F("Button released: ")); DEBUG_PRINTLN(b);
        buttonPressedBefore[b] = false;
        buttonLongPressed[b] = false;
      }

      handled = true;
      return handled;
    }

    void addToConfig(JsonObject& root) override
    {
      JsonObject top = root.createNestedObject(FPSTR(_name));
      top[FPSTR(_enabled)] = enabled;
    }

    bool readFromConfig(JsonObject& root) override
    {
      JsonObject top = root[FPSTR(_name)];

      bool configComplete = !top.isNull();

      configComplete &= getJsonValue(top[FPSTR(_enabled)], enabled);

      return configComplete;
    }

    uint16_t getId() override
    {
      return USERMOD_ID_TEST_MODE;
    }
};

// add more strings here to reduce flash memory usage
const char TestModeUsermod::_name[]    PROGMEM = "Test Mode";
const char TestModeUsermod::_enabled[] PROGMEM = "enabled";