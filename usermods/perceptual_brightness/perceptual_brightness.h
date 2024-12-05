#pragma once

#include "wled.h"

#ifndef USERMOD_PERCEPTUAL_BRIGHTNESS_MAX_STEPS
#define USERMOD_PERCEPTUAL_BRIGHTNESS_MAX_STEPS 32
#endif

#ifndef USERMOD_PERCEPTUAL_BRIGHTNESS_TOGGLE_DIRECTION_DELAY
#define USERMOD_PERCEPTUAL_BRIGHTNESS_TOGGLE_DIRECTION_DELAY (WLED_LONG_REPEATED_ACTION + 100)
#endif

class UsermodPerceptualBrightness : public Usermod {
  private:
    bool enabled = false;
    bool increasing = false; // used for wrapping brightness control
    uint8_t brightness[USERMOD_PERCEPTUAL_BRIGHTNESS_MAX_STEPS];
    uint8_t maxBrightness, computeSteps, skipStart, skipEnd;
    static const float curveFactor;
    uint32_t lastCalled, delta;
    static const char *sName, *sEnabled, *sMaxBrightness, *sComputeSteps, *sSkipStart, *sSkipEnd;

    int b = 0; // b is current brightness step
    int c = 0; // c is last brightness step

    inline void computeLevels();
    inline void applyBrightness() {
      bri = brightness[b];
    }

  public:
    inline void incBrightness();
    inline void decBrightness();
    inline void cycleBrightness(bool wrap=true);

    inline void enable(bool enable) { enabled = enable; }
    inline bool isEnabled() { return enabled; }

    void setup() override {
      return;
    }

    void loop() override {
      return;
    }

    void readFromJsonState(JsonObject& root) override {
      String cmd;
      bool cmdExists = getJsonValue(root["pb"], cmd);
      if (enabled && cmdExists) {
        if (cmd.length() == 1) {
          char direction = cmd[0];
          switch (direction) {
            case '+': incBrightness(); break;
            case '-': decBrightness(); break;
            case '~': cycleBrightness(); break;
            case '/': cycleBrightness(false); break;
            default: DEBUG_PRINTLN(F("Perceptual brightness error: unrecognized command"));
          }
        } else {
          DEBUG_PRINTLN(F("Perceptual brightness error: unrecognized command"));
        }
      }
    }

    void addToConfig(JsonObject& root) override {
      JsonObject top = root.createNestedObject(FPSTR(sName));
      top[FPSTR(sEnabled)] = enabled;
      top[FPSTR(sMaxBrightness)] = maxBrightness;
      top[FPSTR(sComputeSteps)] = computeSteps;
      top[FPSTR(sSkipStart)] = skipStart;
      top[FPSTR(sSkipEnd)] = skipEnd;
    }

    void appendConfigData() override {
      oappend(F("addInfo('")); oappend(String(FPSTR(sName)).c_str()); oappend(':'); oappend(FPSTR(sMaxBrightness));
      oappend(F("',1,'<em>Max: 255</em>');"));

      oappend(F("addInfo('")); oappend(String(FPSTR(sName)).c_str()); oappend(':'); oappend(FPSTR(sComputeSteps));
      oappend(F("',1,'<em>Max: "));
      oappend(USERMOD_PERCEPTUAL_BRIGHTNESS_MAX_STEPS);
      oappend(F("<br/>You may want to compute more brightness steps than you will actually use,<br/>eg skipping a few low-brightness steps at the start.<br/>You can skip any number of steps at the beginning and end of the set.');"));

      String s = "',1,'<p>All computed steps:<br/>";
      for (int i = 0; i < computeSteps; i++) {
        if (i == skipStart) s += "[";
        s += String(brightness[i]);
        if (i == computeSteps - skipEnd - 1) s += "]";
        if (i < computeSteps - 1) s +=", ";
      }
      s += "<br>Only steps in [square brackets] will be used, others skipped</p>');";

      oappend(F("addInfo('")); oappend(String(FPSTR(sName)).c_str()); oappend(':'); oappend(FPSTR(sSkipEnd)); oappend(s.c_str());
    }

    bool readFromConfig(JsonObject& root) override {
      JsonObject top = root[FPSTR(sName)];
      bool configComplete = !top.isNull();

      configComplete &= getJsonValue(top[FPSTR(sEnabled)], enabled, false);
      configComplete &= getJsonValue(top[FPSTR(sMaxBrightness)], maxBrightness, 255);
      configComplete &= getJsonValue(top[FPSTR(sComputeSteps)], computeSteps, 16);
      configComplete &= getJsonValue(top[FPSTR(sSkipStart)], skipStart, 3);
      configComplete &= getJsonValue(top[FPSTR(sSkipEnd)], skipEnd, 0);

      // Enforce bounds of those values
      if (configComplete) {
        // pattern is
        // \1 = (\1 > \2) ? \2 : \1;
        maxBrightness = (maxBrightness > 255) ? 255 : maxBrightness;
        computeSteps = (computeSteps > USERMOD_PERCEPTUAL_BRIGHTNESS_MAX_STEPS) ? USERMOD_PERCEPTUAL_BRIGHTNESS_MAX_STEPS : computeSteps;
        skipStart = (skipStart > (computeSteps - 1)) ? computeSteps - 1 : skipStart;
        skipEnd = (skipEnd > (computeSteps - skipStart - 1)) ? (computeSteps - skipStart - 1) : skipEnd;
      }

      computeLevels();
      return configComplete;
    }

};

const float UsermodPerceptualBrightness::curveFactor = 1.0f / 0.33f;
const char* UsermodPerceptualBrightness::sName = "Perceptual Brightness";
const char* UsermodPerceptualBrightness::sEnabled = "Enabled";
const char* UsermodPerceptualBrightness::sMaxBrightness = "Max brightness";
const char* UsermodPerceptualBrightness::sComputeSteps = "Steps to compute";
const char* UsermodPerceptualBrightness::sSkipStart = "Start skip";
const char* UsermodPerceptualBrightness::sSkipEnd = "End skip";

void UsermodPerceptualBrightness::computeLevels(){
  bool exactMatch = false;
  for (int i = 0; i < computeSteps; i++) {
    brightness[i] = (uint8_t)(0.5 + pow((1.0 + i)/computeSteps,curveFactor) * maxBrightness);
    if (!exactMatch && brightness[i] == bri) {
      exactMatch = true;
      b = i;
    }
  }
  if (!exactMatch) {
    // Current brightness is between steps. Set current step to the first one below the current brightness
    // Will still be 0 as initialized if this search finds nothing, ie bri not less than any step.
    for (int i = computeSteps - 1; i >= 0; i++) {
      if (bri < brightness[i]) {
        b = i;
        break;
      }
    }
  }
}

void UsermodPerceptualBrightness::incBrightness(){
  if (b >= (computeSteps - skipEnd - 1)) b = computeSteps - skipEnd - 1;
  else b++;
  applyBrightness();
}

void UsermodPerceptualBrightness::decBrightness(){
  if (b <= skipStart) b = skipStart;
  else b--;
  applyBrightness();
}

void UsermodPerceptualBrightness::cycleBrightness(bool wrap){
  uint32_t nowTime = millis();
  uint32_t interval = nowTime - lastCalled;
  lastCalled = nowTime;

  if (interval > USERMOD_PERCEPTUAL_BRIGHTNESS_TOGGLE_DIRECTION_DELAY) increasing = !increasing;

  if (increasing) {
    if (b >= (computeSteps - skipEnd - 1)) {
      if (wrap) increasing = false;
      b = computeSteps - skipEnd - 1;
    } else b++;
  } else {
    // decreasing brightnesso
    if (b <= skipStart) {
      if (wrap) increasing = true;
      b = skipStart;
    } else b--;
  }
  applyBrightness();
}
