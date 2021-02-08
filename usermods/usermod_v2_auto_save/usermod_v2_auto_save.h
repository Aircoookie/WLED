#pragma once

#include "wled.h"

//
// v2 Usermod automatically to automatically save settings 
// to preset 1 after a change to any of
//
// * brightness
// * effect speed
// * effect intensity
// * mode (effect)
// * palette
//
// happens, but it will wait for SETTINGS_SETTLE_MS 
// milliseconds "settle" period in case there are other changes.
//
// Functionality is enhanced when paired with the
// Four Line Display Usermod.

//How long to wait after settings change to auto-save
#ifndef SETTINGS_SETTLE_MS
#define SETTINGS_SETTLE_MS 10*1000
#endif

//  "Auto save MM-DD HH:MM:SS"
#define PRESET_NAME_BUFFER_SIZE 25

class AutoSaveUsermod : public Usermod {
  private:
    // If we've detected the need to auto save, this will
    // be non zero.
    unsigned long autoSaveAfter = 0;

    char presetNameBuffer[PRESET_NAME_BUFFER_SIZE];

    // Don't start auto saving for 10 seconds
    unsigned long dontStartUntil = millis() + 10*1000;
    bool firstLoop = true;

    uint8_t knownBrightness = 0;
    uint8_t knownEffectSpeed = 0;
    uint8_t knownEffectIntensity = 0;
    uint8_t knownMode = 0;
    uint8_t knownPalette = 0;

#ifdef USERMOD_FOUR_LINE_DISLAY
    FourLineDisplayUsermod* display;
#endif

  public:
    // gets called once at boot. Do all initialization that doesn't depend on
    // network here
    void setup() {
#ifdef USERMOD_FOUR_LINE_DISLAY    
    // This Usermod has enhanced funcionality if
    // FourLineDisplayUsermod is available.
    display = (FourLineDisplayUsermod*) usermods.lookup(USERMOD_ID_FOUR_LINE_DISP);
#endif
    }

    // gets called every time WiFi is (re-)connected. Initialize own network
    // interfaces here
    void connected() {}

    /**
     * Da loop.
     */
    void loop() {
      unsigned long now = millis();
      if (firstLoop && now < dontStartUntil) {
        return;
      }
      uint8_t currentMode = strip.getMode();
      uint8_t currentPalette = strip.getSegment(0).palette;
      if (firstLoop) {
        firstLoop = false;
        knownBrightness = bri;
        knownEffectSpeed = effectSpeed;
        knownEffectIntensity = effectIntensity;
        knownMode = currentMode;
        knownPalette = currentPalette;
        return;
      }

      unsigned long wouldAutoSaveAfter = now + SETTINGS_SETTLE_MS;
      if (knownBrightness != bri) {
        knownBrightness = bri;
        autoSaveAfter = wouldAutoSaveAfter;
      } else if (knownEffectSpeed != effectSpeed) {
        knownEffectSpeed = effectSpeed;
        autoSaveAfter = wouldAutoSaveAfter;
      } else if (knownEffectIntensity != effectIntensity) {
        knownEffectIntensity = effectIntensity;
        autoSaveAfter = wouldAutoSaveAfter;
      } else if (knownMode != currentMode) {
        knownMode = currentMode;
        autoSaveAfter = wouldAutoSaveAfter;
      } else if (knownPalette != currentPalette) {
        knownPalette = currentPalette;
        autoSaveAfter = wouldAutoSaveAfter;
      }

      if (autoSaveAfter && now > autoSaveAfter) {
        autoSaveAfter = 0;
        // Time to auto save. You may have some flickry?
        saveSettings();
        displayOverlay();
      }
    }

    void saveSettings() {
      updateLocalTime();
      sprintf(presetNameBuffer, 
        "Auto save %02d-%02d %02d:%02d:%02d",
        month(localTime), day(localTime),
        hour(localTime), minute(localTime), second(localTime));
      savePreset(1, true, presetNameBuffer);
    }

    void displayOverlay() {
#ifdef USERMOD_FOUR_LINE_DISLAY
      if (display != nullptr) {
        display->wakeDisplay();
        display->overlay("Settings", "Auto Saved", 1500);
      }
#endif
    }

    /*
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void addToJsonState(JsonObject& root) {
    }

    /*
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void readFromJsonState(JsonObject& root) {
    }

    /*
     * addToConfig() can be used to add custom persistent settings to the cfg.json file in the "um" (usermod) object.
     * It will be called by WLED when settings are actually saved (for example, LED settings are saved)
     * If you want to force saving the current state, use serializeConfig() in your loop().
     * 
     * CAUTION: serializeConfig() will initiate a filesystem write operation.
     * It might cause the LEDs to stutter and will cause flash wear if called too often.
     * Use it sparingly and always in the loop, never in network callbacks!
     * 
     * addToConfig() will also not yet add your setting to one of the settings pages automatically.
     * To make that work you still have to add the setting to the HTML, xml.cpp and set.cpp manually.
     * 
     * I highly recommend checking out the basics of ArduinoJson serialization and deserialization in order to use custom settings!
     */
    void addToConfig(JsonObject& root) {
    }

    /*
     * readFromConfig() can be used to read back the custom settings you added with addToConfig().
     * This is called by WLED when settings are loaded (currently this only happens once immediately after boot)
     * 
     * readFromConfig() is called BEFORE setup(). This means you can use your persistent values in setup() (e.g. pin assignments, buffer sizes),
     * but also that if you want to write persistent values to a dynamic buffer, you'd need to allocate it here instead of in setup.
     * If you don't know what that is, don't fret. It most likely doesn't affect your use case :)
     */
    void readFromConfig(JsonObject& root) {
    }

    /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
    uint16_t getId() {
      return USERMOD_ID_AUTO_SAVE;
    }

};