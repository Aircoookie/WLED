#pragma once

#include "wled.h"

// TODO: check if pin_manager.h needs this usermod 
// implementation of both buttons pressed:
// https://discord.com/channels/473448917040758787/1144310280751435817/1160697530783387688

class TestModeUsermod : public Usermod {
  private:
    bool enabled = true;
    bool initDone = false;
    bool firstButton = false;
    bool secondButton = false;

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
      return;
    }

    bool handleButton(uint8_t b) override {
      yield();
      // ignore certain button types as they may have other consequences
      if (!enabled
       || buttonType[b] == BTN_TYPE_NONE
       || buttonType[b] == BTN_TYPE_RESERVED
       || buttonType[b] == BTN_TYPE_PIR_SENSOR
       || buttonType[b] == BTN_TYPE_ANALOG
       || buttonType[b] == BTN_TYPE_ANALOG_INVERTED) {
        return false;
      }

      bool handled = false;
      // do your button handling here
      Serial.println("Button has been pressed: " + String(b));
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