#pragma once
#include "wled.h"

class PIRLDRMod : public Usermod {
  private:
    // PIR sensor parameters
    bool PIRenabled = true;      // Enable/disable PIR sensor
    int PIRpin = 12;             // Default pin for PIR sensor
    unsigned long lastPirTrigger = 0; // Timestamp of last PIR trigger
    bool pirState = false;

    // LDR parameters
    unsigned long lastLdrUpdate = 0;       // timestamp of last ldr update
    float LDRdelta = 0.0;                   // LDR Delta
    bool LDRenable = true;                  // Enable/disable LDR
    int LDRpin = 33;                        // Default pin for LDR
    float LDRReferenceVoltage = 3.3;        // Reference voltage (in volts)
    float LDRAdcPrecision = 4096.0;         // ADC precision (as a float)
    int LDRResistorValue = 10000;           // Resistor value (in ohms)
    float LDRLuxOffset = 0.0;               // Offset value for lux measurements
    uint16_t lastLux = 0;                   // Last lux measurement

    // Helper method to read luminance from the LDR
    uint16_t getLuminance() {
      // LDR luminance logic using the updated float precision
      float volts = analogRead(LDRpin) * (LDRReferenceVoltage / LDRAdcPrecision);
      float amps = volts / LDRResistorValue;
      float lux = amps * 1000000 * 2.0; // Conversion factor for lux
      
      return uint16_t(lux + LDRLuxOffset); // Apply the offset to lux
    }

    // // MQTT discovery
    // const char* discoveryTopic = "homeassistant/sensor/wled_%06X/%s/config";

    // Helper method to send sensor data via MQTT
    void sendSensorData(const char* sensorType, int value) {
      if (WLED_MQTT_CONNECTED) {
        char subTopic[64];
        snprintf(subTopic, 64, "wled/%s", sensorType);
        char payload[16];
        snprintf(payload, 16, "%d", value);
        mqtt->publish(subTopic, 0, false, payload);
      }
    }

  public:
    void setup() {
      // Initialize PIR sensor pin if enabled
      if (PIRenabled) {
        pinMode(PIRpin, INPUT);
      }

      // Initialize LDR pin if enabled
      if (LDRenable) {
        pinMode(LDRpin, INPUT);
      }
    }

    void loop() {
      unsigned long now = millis();

      // Handle PIR sensor logic if enabled
      if (PIRenabled && now - lastPirTrigger >= 1000) {
        lastPirTrigger = now;
        bool pirReading = digitalRead(PIRpin);
        if (pirReading != pirState) {
          pirState = pirReading;
          sendSensorData("pir_sensor", pirState);
        }
      }


      // Get the current luminance from the LDR if enabled
      if (LDRenable && now - lastLdrUpdate >= 1000) {
        lastLdrUpdate = now;
        uint16_t lastLux = getLuminance();
        if (lastLux <= lastLux - LDRdelta || lastLux >= lastLux + LDRdelta) {
        sendSensorData("light_level", lastLux);
      }
    }

    // Define the options for the Usermod tab in WLED settings
    void addToConfig(JsonObject &root) {
      JsonObject top = root.createNestedObject("PIRLDRMod");
      top["PIRenabled"] = PIRenabled;
      top["PIRpin"] = PIRpin;
      top["LDRenable"] = LDRenable; // Add LDR enable to config
      top["LDRpin"] = LDRpin;
      top["LDRReferenceVoltage"] = LDRReferenceVoltage;
      top["LDRAdcPrecision"] = LDRAdcPrecision;
      top["LDRResistorValue"] = LDRResistorValue;
      top["LDRLuxOffset"] = LDRLuxOffset; // Add offset to config
    }

    bool readFromConfig(JsonObject &root) {
      JsonObject top = root["PIRLDRMod"];
      if (top.isNull()) return false;

      PIRenabled = top["PIRenabled"] | PIRenabled;
      PIRpin = top["PIRpin"] | PIRpin;
      LDRenable = top["LDRenable"] | LDRenable; // Read LDR enable from config
      LDRpin = top["LDRpin"] | LDRpin;
      LDRReferenceVoltage = top["LDRReferenceVoltage"] | LDRReferenceVoltage;
      LDRAdcPrecision = top["LDRAdcPrecision"] | LDRAdcPrecision;
      LDRResistorValue = top["LDRResistorValue"] | LDRResistorValue;
      LDRLuxOffset = top["LDRLuxOffset"] | LDRLuxOffset; // Read offset from config

      return true;
    }

    // Provide unique ID for this Usermod
    uint16_t getId() {
      return USERMOD_ID_EXAMPLE;
    }

    // Provide a description for the Usermod settings
    void getUsermodConfig(JsonObject &root) {
      JsonObject top = root.createNestedObject("PIRLDRMod");
      top["PIRenabled"] = PIRenabled;
      top["PIRpin"] = PIRpin;
      top["LDRenable"] = LDRenable; // Add LDR enable to usermod config
      top["LDRpin"] = LDRpin;
      top["LDRReferenceVoltage"] = LDRReferenceVoltage;
      top["LDRAdcPrecision"] = LDRAdcPrecision;
      top["LDRResistorValue"] = LDRResistorValue;
      top["LDRLuxOffset"] = LDRLuxOffset; // Add offset to usermod config
    }

};
