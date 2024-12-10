#pragma once

#include "wled.h"

#ifndef USERMOD_PRESET_NAVIGATION_MAX_PRESETS
#define USERMOD_PRESET_NAVIGATION_MAX_PRESETS 64
#endif

#ifndef USERMOD_PRESET_NAVIGATION_MAX_BANKS
#define USERMOD_PRESET_NAVIGATION_MAX_BANKS 23
#endif

class UsermodPresetNavigation : public Usermod {
  private:
    bool enabled = false;
    uint8_t presets[USERMOD_PRESET_NAVIGATION_MAX_PRESETS];
    uint8_t bankOffset[USERMOD_PRESET_NAVIGATION_MAX_BANKS+1];
    String presetArray = "1";
    String bootupPresetNavigation;
    static const char *sName, *sEnabled, *sArray;

    int x = 0; // x is current bank
    int y = 0; // y is current sub-bank preset
    int xMax = 0;

    // One day might return false on a parsing error
    bool readArray();
    void printArray();
    inline void applyPresetNavigation(){
      applyPreset(presets[bankOffset[x]+y]);
    }

  public:
    void incBank(bool wrap=true);
    void decBank(bool wrap=true);
    void incPreset(bool wrap=true);
    void decPreset(bool wrap=true);

    inline void enable(bool enable) { enabled = enable; }
    inline bool isEnabled() { return enabled; }

    void setup() override {
      x = y = 0;
      return;
    }

    void loop() override {
      return;
    }

    void readFromJsonState(JsonObject& root) override {
      String cmd;
      bool cmdExists = getJsonValue(root["pn"], cmd);
      if (cmdExists && enabled) {
        if (cmd.equals("b~")) incBank();
        else if (cmd.equals("p~")) incPreset();
        else if (cmd.equals("b~-")) decBank();
        else if (cmd.equals("p~-")) decPreset();
        // no wrap versions
        else if (cmd.equals("b+")) incBank(false);
        else if (cmd.equals("p+")) incPreset(false);
        else if (cmd.equals("b-")) decBank(false);
        else if (cmd.equals("p-")) decPreset(false);
      }
    }

    void addToConfig(JsonObject& root) override {
      JsonObject top = root.createNestedObject(FPSTR(sName));
      top[FPSTR(sEnabled)] = enabled;
      top[FPSTR(sArray)] = presetArray;
    }

    bool readFromConfig(JsonObject& root) override
    {
      JsonObject top = root[FPSTR(sName)];
      bool configComplete = !top.isNull();

      configComplete &= getJsonValue(top[FPSTR(sEnabled)], enabled, false);
      configComplete &= getJsonValue(top[FPSTR(sArray)], presetArray, "1");

      // Remove any carriage returns (\r) from presetArray, leave only newlines (\n)
      int i=0;
      int lastI;
      while(true) {
        lastI = i;
        i = presetArray.indexOf('\r',lastI);
        if (i < 0) break;
        presetArray.remove(i,1);
      }

      readArray();
      return configComplete;
    }

};

const char* UsermodPresetNavigation::sName = "Preset Navigation";
const char* UsermodPresetNavigation::sEnabled = "Enable";
const char* UsermodPresetNavigation::sArray = "Preset banks>"; // Trailing > makes this a textarea in settings

bool UsermodPresetNavigation::readArray() {
  int readX = 0;
  int i = 0;
  int cursor = 0;
  int currentNewline = presetArray.indexOf("\n");
  int currentComma;
  bool finished = false;
  bool bankFinished;

  while (!finished) {
    // Iterate over each bank (line of text) in presetArray (string representation of array)
    if (currentNewline == -1){
      finished = true;
      currentNewline = presetArray.length();
    }
    if (readX >= USERMOD_PRESET_NAVIGATION_MAX_BANKS) {
      DEBUG_PRINTLN("Preset navigation: exceeded max banks");
      break;
    }
    currentComma = presetArray.indexOf(",", cursor);
    bankFinished = false;
    bankOffset[readX]=i;
    while (!bankFinished) {
      // Iterate through each PRESET (comma separated value) in this bank
      if (i >= USERMOD_PRESET_NAVIGATION_MAX_PRESETS) {
        DEBUG_PRINTLN("Preset navigation: exceeded max presets");
        break;
      }
      if (currentComma == -1 || currentComma > currentNewline) {
        bankFinished = true;
        currentComma = currentNewline;
      }
      String preset = presetArray.substring(cursor,currentComma);
      presets[i] = preset.toInt();
      i++;
      cursor = currentComma + 1;
      currentComma = presetArray.indexOf(",", cursor);
    } // end PRESET loop
    readX++;
    // Find our next newline
    cursor = currentNewline + 1;
    currentNewline = presetArray.indexOf("\n", cursor);
  } // end bank loop

  xMax=readX-1;
  bankOffset[readX]=i;
  return true;
}

void UsermodPresetNavigation::printArray() {

}

void UsermodPresetNavigation::incBank(bool wrap){
  if (x == xMax && wrap) {
    x = 0; y = 0;
    applyPresetNavigation();
  } else if (x < xMax) {
    x++; y = 0;
    applyPresetNavigation();
  }
}

void UsermodPresetNavigation::decBank(bool wrap){
  if (x == 0 && wrap) {
    x = xMax; y = 0;
    applyPresetNavigation();
  } else if (x > 0) {
    x--; y = 0;
    applyPresetNavigation();
  }
}

void UsermodPresetNavigation::incPreset(bool wrap){
  int yMax = bankOffset[x+1] - bankOffset[x] - 1;
  if (yMax == 0) {
    // We don't have anywhere to cycle
  } else if (y == yMax && wrap) {
    y=0;
    applyPresetNavigation();
  } else if (y < yMax) {
    y++;
    applyPresetNavigation();
  }
}

void UsermodPresetNavigation::decPreset(bool wrap){
  int yMax = bankOffset[x+1] - bankOffset[x] - 1;
  if (yMax == 0) {
    // We don't have anywhere to cycle
  } else if (y == 0 && wrap) {
    y = yMax;
    applyPresetNavigation();
  } else if (y > 0) {
    y--;
    applyPresetNavigation();
  }
}
