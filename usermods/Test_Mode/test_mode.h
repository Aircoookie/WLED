#pragma once

#include "wled.h"

#ifndef TEST_MODE_BUTTONS
  #define TEST_MODE_BUTTONS 2
#endif

#ifndef TEST_MODE_BRIGHTNESS
  #define TEST_MODE_BRIGHTNESS 60
#endif

#ifndef TEST_MODE_COLOR
  #define TEST_MODE_COLOR ULTRAWHITE
#endif

#ifndef TEST_MODE_EFFECT
  #define TEST_MODE_EFFECT FX_MODE_FADE
#endif

#ifndef TEST_MODE_PALETTE
  #define TEST_MODE_PALETTE 0
#endif

class TestModeUsermod : public Usermod {
  private:
    bool enabled = true;
    bool initDone = false;
    unsigned long lastTime = 0;
    int8_t testButtons[WLED_MAX_BUTTONS];
    int8_t heldButtons = 0;
    bool testModeActivated = false;

  public:
    // strings to reduce flash memory usage (used more than twice)
    static const char _name[];
    static const char _enabled[];

    inline void enable(bool enable) { enabled = enable; }
    inline bool isEnabled() { return enabled; }

    void setup() { initDone = true; }
    void loop();

    bool handleButton(uint8_t b) override;
    void addToConfig(JsonObject& root) override;
    bool readFromConfig(JsonObject& root) override;
    uint16_t getId() override;
};

void TestModeUsermod::loop() {
  if (!enabled || TEST_MODE_BUTTONS < 1) { return; }

  if (millis() - lastTime > 1000) {
    lastTime = millis();
    heldButtons = 0;
    for (unsigned i=0; i<WLED_MAX_BUTTONS; i++) {
      if (testButtons[i] == 1) {
        heldButtons += 1;
      }
    }
    if (heldButtons >= TEST_MODE_BUTTONS) {
      DEBUG_PRINTLN(F("Test Mode activated"));
      testModeActivated = true;

      bri = TEST_MODE_BRIGHTNESS;
      Segment& seg = strip.getMainSegment();
      seg.setMode(TEST_MODE_EFFECT, false);
      seg.setColor(0, TEST_MODE_COLOR);
      seg.setPalette(TEST_MODE_PALETTE);
      stateUpdated(CALL_MODE_DIRECT_CHANGE);

      // Disable usermod so next boot is clean
      enable(false);
      serializeConfig();
    }
  }
}

bool TestModeUsermod::handleButton(uint8_t b) {
  yield();
  // once test mode has been activated keep handling buttons for this boot
  if (!testModeActivated) {
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
    // Button held
    if (now - buttonPressedTime[b] > 600) {
        testButtons[b] = 1;
    }
    buttonLongPressed[b] = true;
  }
  // Button released
  else if (!isButtonPressed(b) && buttonPressedBefore[b]) {
    testButtons[b] = 0;
    buttonPressedBefore[b] = false;
    buttonLongPressed[b] = false;
  }

  handled = true;
  return handled;
}

void TestModeUsermod::addToConfig(JsonObject& root) {
  JsonObject top = root.createNestedObject(FPSTR(_name));
  top[FPSTR(_enabled)] = enabled;
}

bool TestModeUsermod::readFromConfig(JsonObject& root) {
  JsonObject top = root[FPSTR(_name)];

  bool configComplete = !top.isNull();

  configComplete &= getJsonValue(top[FPSTR(_enabled)], enabled);

  return configComplete;
}

uint16_t TestModeUsermod::getId() { return USERMOD_ID_TEST_MODE; }

// add more strings here to reduce flash memory usage
const char TestModeUsermod::_name[]    PROGMEM = "Test Mode";
const char TestModeUsermod::_enabled[] PROGMEM = "Enabled";