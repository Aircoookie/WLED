#pragma once

#include "wled.h"

// Usermod for conditional triggering of presets based on a sensor/switch state
class UsermodConditionalTriggering : public Usermod
  {
  private:
    // Sensor pin configuration
    int8_t pin = -1;                // GPIO pin number where sensor is connected
    bool sensorNormallyOpen = true; // Sensor type: true for normally open, false for normally closed

    // Preset configuration
    uint8_t presetA = 1;  // Preset to trigger when sensor is ON
    uint8_t presetB = 2;  // Preset to trigger when sensor is OFF
    uint8_t presetX = 99; // Preset that triggers the conditional check

    // Dynamic toggling between the two presets configuration
    bool togglePresetsWithSensor = false; // Variable to store user preference

    // Debounce configuration
    bool sensorDebounceEnabled = true;        // Enable or disable debounce
    const unsigned long debounceDelay = 100;  // Debounce delay (milliseconds)
    unsigned long lastDebounceTime = 0;       // Time when the last debounce event occurred
    bool lastSensorState = false;             // Last read state of the sensor
    bool sensorState = false;                 // Current debounced state of the sensor
    bool sensorEnabled = false;               // Indicates if the sensor is enabled

    // Reads the state of the sensor with debounce logic
    bool readSensor()
    {
      bool currentReading = digitalRead(pin);
      bool isOff = sensorNormallyOpen ? (currentReading == HIGH) : (currentReading == LOW);
      if (!sensorDebounceEnabled)
      {
        return !isOff;
      }
      if ((isOff && lastSensorState) || (!isOff && !lastSensorState))
      {
        lastDebounceTime = millis();
      }
      if ((millis() - lastDebounceTime) > debounceDelay)
      {
        sensorState = !isOff;
      }
      lastSensorState = !isOff;
      return sensorState;
    }

    // Checks the sensor state and applies the appropriate preset
    void checkAndApplyPreset()
    {
      bool sensorState = readSensor();
      applyPreset(sensorState ? presetA : presetB);
    }

  public:
    // Setup method, initializes sensor pin
    void setup()
    {
      if (pin >= 0)
      {
        pinMode(pin, INPUT_PULLUP);
        sensorEnabled = true;
      }
    }

    // Main loop method, checks sensor state and applies preset
    void loop()
    {
      if (!sensorEnabled)
        return;
      // Check if Preset X, A, or B is currently active
      if (currentPreset == presetX || currentPreset == presetA || currentPreset == presetB)
      {
        // If active, read the current state of the sensor
        bool sensorState = readSensor();
        if (currentPreset == presetX)
        {
          // If the triggering preset is active, apply the appropriate preset based on sensor state
          checkAndApplyPreset();
        }
        else if (togglePresetsWithSensor && (currentPreset == presetA || currentPreset == presetB))
        {
          // If auto toggling is enabled and either preset A or B is active, determine if the preset should change
          uint8_t targetPreset = sensorState ? presetA : presetB;
          if (currentPreset != targetPreset)
          {
            // Apply the new preset if the current one doesn't match the target
            applyPreset(targetPreset);
          }
        }
      }
    }

    // Adds usermod configuration to JSON object
    void addToConfig(JsonObject &root) override
    {
      JsonObject top = root.createNestedObject(F("Conditional Triggering"));
      top["pin"] = pin;
      top["sensorNormallyOpen"] = sensorNormallyOpen;
      top["sensorDebounceEnabled"] = sensorDebounceEnabled;
      top["togglePresetsWithSensor"] = togglePresetsWithSensor;
      top["presetX"] = presetX;
      top["presetA"] = presetA;
      top["presetB"] = presetB;
    }

    // Reads usermod configuration from JSON object
    bool readFromConfig(JsonObject &root)
    {
      JsonObject top = root[F("Conditional Triggering")];
      if (top.isNull())
        return false;
      pin = top["pin"] | pin;
      sensorNormallyOpen = top["sensorNormallyOpen"] | sensorNormallyOpen;
      sensorDebounceEnabled = top["sensorDebounceEnabled"] | sensorDebounceEnabled;
      togglePresetsWithSensor = top["togglePresetsWithSensor"] | togglePresetsWithSensor;
      presetA = top["presetA"] | presetA;
      presetB = top["presetB"] | presetB;
      presetX = top["presetX"] | presetX;
      return true;
    }

    // Returns the ID of the usermod
    uint16_t getId()
    {
      return USERMOD_ID_CONDITIONAL_TRIGGER;
    }
  };
