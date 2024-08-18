// Filename: Power_Measurement.h
// This code was cocreated by github copilot and created by Tomáš Kuchta
#pragma once

#include "wled.h"
#include "esp_adc_cal.h"

#ifndef CURRENT_PIN
  #define CURRENT_PIN 1
#endif

#ifndef VOLTAGE_PIN
  #define VOLTAGE_PIN 0
#endif

#define NUM_READINGS 10
#define NUM_READINGS_CAL 100
#define ADC_MAX_VALUE (pow(2, ADCResolution) - 1) // For 12-bit ADC, the max value is 4095
#define UPDATE_INTERVAL_MAIN 100
#define UPDATE_INTERVAL_MQTT 60000

class UsermodPower_Measurement : public Usermod {
  private:
    bool initDone = false;
    unsigned long lastTime_slow = 0;
    unsigned long lastTime_main = 0;
    unsigned long lastTime_energy = 0;
    unsigned long lastTime_mqtt = 0;
    boolean enabled = true;
    boolean calibration_enable = false;
    boolean cal_adavnced = false;

    int Voltage_raw = 0;
    float AverageVoltage_raw = 0;
    int Voltage_raw_adj = 0;
    int Voltage_calc = 0;

    int Current_raw = 0;
    float AverageCurrent_raw = 0;
    int Current_calc = 0;

    float voltageReadings_raw[NUM_READINGS];
    float currentReadings_raw[NUM_READINGS];
    int readIndex = 0;
    float totalVoltage_raw = 0;
    float totalCurrent_raw = 0;

    // Low-pass filter variables
    float alpha = 0.1;
    float filtered_Voltage_raw = 0;
    float filtered_Current_raw = 0;

    unsigned long long wattmiliseconds = 0; //energy counter in watt milliseconds


    // calibration variables
    int Num_Readings_Cal = NUM_READINGS_CAL;
    bool Cal_In_Progress = false;
    bool Cal_Zero_Points = false;
    bool Cal_calibrate_Measured_Voltage = false;
    bool Cal_calibrate_Measured_Current = false;
    float Cal_Measured_Voltage = 0;
    float Cal_Measured_Current = 0;

    float Cal_min_Voltage_raw = 17;
    float Cal_min_Current_calc = 718;

    float Cal_Voltage_raw_averaged = 0;
    float Cal_Voltage_calc_averaged = 0;
    float Cal_Current_calc_averaged = 0;

    int Cal_Current_at_x = 1000;
    int Cal_Current_calc_at_x = 775;
    float Cal_Voltage_Coefficient = 22.97;

    // averiging variables
    float Cal_Voltage_raw_Readings_Avg[NUM_READINGS_CAL];
    float Cal_Voltage_calc_Readings_Avg[NUM_READINGS_CAL];
    float Cal_Current_calc_Readings_Avg[NUM_READINGS_CAL];
    int Cal_Read_Index = 0;
    float Cal_Total_Voltage_raw = 0;
    float Cal_Total_Voltage_calc = 0;
    float Cal_Total_Current_calc = 0;

    int8_t VoltagePin = VOLTAGE_PIN;
    int8_t CurrentPin = CURRENT_PIN;

    int Update_Interval_Mqtt = UPDATE_INTERVAL_MQTT;
    int Update_Interval_Main = UPDATE_INTERVAL_MAIN;

    // String used more than once
    static const char _name[] PROGMEM;
    static const char _no_data[] PROGMEM;

  public:
    int ADCResolution = 12;
    int ADCAttenuation = ADC_6db;

    //For usage in other parts of the main code
    float Voltage = 0;
    float Current = 0;
    float Power = 0;
    unsigned long kilowatthours = 0;

    void setup() {
      analogReadResolution(ADCResolution);
      analogSetAttenuation(static_cast<adc_attenuation_t>(ADCAttenuation)); // Set the ADC attenuation (ADC_ATTEN_DB_6 = 0 mV ~ 1300 mV)

      // Initialize all readings to 0:
      for (int i = 0; i < NUM_READINGS; i++) {
        voltageReadings_raw[i] = 0;
        currentReadings_raw[i] = 0;
      }

      Current_raw = 1800;
      filtered_Current_raw = 1800;

      Cal_Zero_Points = false;
      Cal_calibrate_Measured_Voltage = false;
      Cal_calibrate_Measured_Current = false;
      Cal_In_Progress = false;
      Num_Readings_Cal = NUM_READINGS_CAL;


      #ifdef WLED_DEBUG
      esp_adc_cal_characteristics_t adc_chars;
      esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_6, ADC_WIDTH_BIT_12, ESP_ADC_CAL_VAL_DEFAULT_VREF, &adc_chars);

      if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        // eFuse Vref is available
        DEBUG_PRINTLN(F("PM: Using eFuse Vref for ADC calibration_enable"));
      } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        // Two Point calibration_enable is available
        DEBUG_PRINTLN(F("PM: Using Two Point calibration_enable for ADC calibration_enable"));
      } else {
        // Default Vref is used
        DEBUG_PRINTLN(F("PM: Using default Vref for ADC calibration_enable"));
      }
      #endif


      if (enabled) {
        pinAlocation();
      }

      initDone = true;

    }

    void loop() {
      if (!enabled || strip.isUpdating()) return;

      unsigned long currentTime = millis();

      #ifdef WLED_DEBUG
      if (currentTime - lastTime_slow >= 1000) {
        printDebugInfo();
        lastTime_slow = currentTime;
      }
      #endif

      #ifndef WLED_DISABLE_MQTT
      if (currentTime - lastTime_mqtt >= Update_Interval_Mqtt) {
        publishPowerMeasurements();
        lastTime_mqtt = currentTime;
      }
      #endif

      if (currentTime - lastTime_main >= Update_Interval_Main) {
        updateReadings();

        if (Cal_Zero_Points || Cal_calibrate_Measured_Voltage || Cal_calibrate_Measured_Current) calibration();

        lastTime_main = currentTime;
      }


    }

    void pinAlocation() {
      DEBUG_PRINTLN(F("Allocating power pins..."));
      if (VoltagePin >= 0 && pinManager.allocatePin(VoltagePin, true, PinOwner::UM_Power_Measurement)) {
        DEBUG_PRINT(F("Voltage pin allocated: "));
        DEBUG_PRINTLN(VoltagePin);
      } else {
        if (VoltagePin >= 0) {
          DEBUG_PRINTLN(F("Voltage pin allocation failed."));
        }
        VoltagePin = -1;  // allocation failed, disable
      }

      if (CurrentPin >= 0 && pinManager.allocatePin(CurrentPin, true, PinOwner::UM_Power_Measurement)) {
        DEBUG_PRINT(F("Current pin allocated: "));
        DEBUG_PRINTLN(CurrentPin);
      } else {
        if (CurrentPin >= 0) {
          DEBUG_PRINTLN(F("Current pin allocation failed."));
        }
        CurrentPin = -1;  // allocation failed, disable
      }
    }


    void printDebugInfo() {
      DEBUG_PRINT(F("Voltage raw: "));
      DEBUG_PRINTLN(Voltage_raw);
      DEBUG_PRINTLN(AverageVoltage_raw);
      DEBUG_PRINT(F("Calc: "));
      DEBUG_PRINTLN(Voltage_calc);
      DEBUG_PRINT(F("Voltage: "));
      DEBUG_PRINTLN(Voltage);

      DEBUG_PRINT(F("Current raw: "));
      DEBUG_PRINTLN(Current_raw);
      DEBUG_PRINTLN(AverageCurrent_raw);
      DEBUG_PRINT(F("Calc: "));
      DEBUG_PRINTLN(Current_calc);
      DEBUG_PRINT("Current: ");
      DEBUG_PRINTLN(Current);

      DEBUG_PRINT("Power: ");
      DEBUG_PRINTLN(Power);

      DEBUG_PRINT("Energy: ");
      DEBUG_PRINTLN(kilowatthours);
      DEBUG_PRINT("Energy Wms: ");
      DEBUG_PRINTLN(wattmiliseconds);
    }

    void updateReadings() {
      // Measure the voltage and current and store them in the arrays for the moving average and convert via map function:
      totalVoltage_raw -= voltageReadings_raw[readIndex];
      totalCurrent_raw -= currentReadings_raw[readIndex];

      if (VoltagePin == -1) {
        Voltage_raw = 0;
        DEBUG_PRINTLN("Voltage pin not allocated");
      } else {
        Voltage_raw = analogRead(VoltagePin);
      }

      if (CurrentPin == -1) {
        Current_raw = 0;
        DEBUG_PRINTLN("Current pin not allocated");
      } else {
        Current_raw = analogRead(CurrentPin);
      }

      if (millis() > 1000) { // To avoid the initial spike in readings
        filtered_Voltage_raw = (alpha * Voltage_raw) + ((1 - alpha) * filtered_Voltage_raw);
        filtered_Current_raw = (alpha * Current_raw) + ((1 - alpha) * filtered_Current_raw);
      } else {
        filtered_Voltage_raw = Voltage_raw;
        filtered_Current_raw = Current_raw;
      }

      voltageReadings_raw[readIndex] = filtered_Voltage_raw;
      currentReadings_raw[readIndex] = filtered_Current_raw;

      totalVoltage_raw += filtered_Voltage_raw;
      totalCurrent_raw += filtered_Current_raw;

      AverageVoltage_raw = totalVoltage_raw / NUM_READINGS;
      AverageCurrent_raw = totalCurrent_raw / NUM_READINGS;

      readIndex = (readIndex + 1) % NUM_READINGS;

      Voltage_raw_adj = map(AverageVoltage_raw, Cal_min_Voltage_raw, ADC_MAX_VALUE, 0, ADC_MAX_VALUE);
      if (Voltage_raw_adj < 0) Voltage_raw_adj = 0;
      Voltage_calc = readADC_Cal(Voltage_raw_adj);
      Voltage = (Voltage_calc / 1000.0) * Cal_Voltage_Coefficient;
      if (Voltage < 0.05) Voltage = 0;
      Voltage = round(Voltage * 100.0) / 100.0; // Round to 2 decimal places
      if (VoltagePin == -1) Voltage = 0;

      Current_calc = readADC_Cal(AverageCurrent_raw);
      Current = (map(Current_calc, Cal_min_Current_calc, Cal_Current_calc_at_x, 0, Cal_Current_at_x)) / 1000.0;
      if (Current > -0.1 && Current < 0.05) {
        Current = 0;
      }
      Current = round(Current * 100.0) / 100.0;
      if (CurrentPin == -1) Current = 0;

      // Calculate power
      Power = Voltage * Current;
      Power = round(Power * 100.0) / 100.0;

      // Calculate energy - dont do it when led is off
      if (Power > 0) {
        unsigned long elapsedTime = millis() - lastTime_energy;
        wattmiliseconds += Power * elapsedTime;
      }
      lastTime_energy = millis();

      if (wattmiliseconds >= 3600000000) { // 3,600,000 milliseconds = 1 hour
        kilowatthours += wattmiliseconds / 3600000000; // Convert watt-milliseconds to kilowatt-hours (1 watt-millisecond = 1/3,600,000,000 kilowatt-hours)
        wattmiliseconds = 0;
      }
    }

    void calibration() {
      if (Num_Readings_Cal == NUM_READINGS_CAL) {
        DEBUG_PRINTLN("calibration_enable started");
        Cal_In_Progress = true;
        serializeConfig(); // To update the checkboxes in the config
      }
      if (Num_Readings_Cal > 0) {
        Num_Readings_Cal--;
        // Average the readings
        Cal_Total_Voltage_raw -= Cal_Voltage_raw_Readings_Avg[Cal_Read_Index];
        Cal_Total_Voltage_calc -= Cal_Voltage_calc_Readings_Avg[Cal_Read_Index];
        Cal_Total_Current_calc -= Cal_Current_calc_Readings_Avg[Cal_Read_Index];

        Cal_Voltage_raw_Readings_Avg[Cal_Read_Index] = Voltage_raw;
        Cal_Voltage_calc_Readings_Avg[Cal_Read_Index] = Voltage_calc;
        Cal_Current_calc_Readings_Avg[Cal_Read_Index] = Current_calc;

        Cal_Total_Voltage_raw += Voltage_raw;
        Cal_Total_Voltage_calc += Voltage_calc;
        Cal_Total_Current_calc += Current_calc;

        Cal_Read_Index = (Cal_Read_Index + 1) % NUM_READINGS_CAL;

        Cal_Voltage_raw_averaged = Cal_Total_Voltage_raw / NUM_READINGS_CAL;
        Cal_Voltage_calc_averaged = Cal_Total_Voltage_calc / NUM_READINGS_CAL;
        Cal_Current_calc_averaged = Cal_Total_Current_calc / NUM_READINGS_CAL;
      } else {

        DEBUG_PRINTLN("calibration_enable Flags:");
        DEBUG_PRINTLN(Cal_In_Progress);
        DEBUG_PRINTLN(Num_Readings_Cal);
        DEBUG_PRINTLN(Cal_Zero_Points);
        DEBUG_PRINTLN(Cal_calibrate_Measured_Voltage);
        DEBUG_PRINTLN(Cal_calibrate_Measured_Current);
        DEBUG_PRINTLN("the averaged values are:");
        DEBUG_PRINTLN(Cal_Voltage_raw_averaged);
        DEBUG_PRINTLN(Cal_Voltage_calc_averaged);
        DEBUG_PRINTLN(Cal_Current_calc_averaged);
        DEBUG_PRINTLN("Inputed values are:");
        DEBUG_PRINTLN(Cal_Measured_Voltage);
        DEBUG_PRINTLN(Cal_Measured_Current);

        Calibration_calculation();

        Cal_In_Progress = false;
        Cal_Zero_Points = false;
        Cal_calibrate_Measured_Voltage = false;
        Cal_calibrate_Measured_Current = false;
        Num_Readings_Cal = NUM_READINGS_CAL;
        serializeConfig(); // To update the checkboxes in the config

        DEBUG_PRINTLN("calibration_enable finished");
      }
    }

    void Calibration_calculation() {
      DEBUG_PRINTLN("Calculating calibration_enable values");

      if (Cal_calibrate_Measured_Current) {
        Cal_Current_at_x = Cal_Measured_Current * 1000;
        Cal_Current_calc_at_x = Cal_Current_calc_averaged;

      } else if (Cal_calibrate_Measured_Voltage) {
        Cal_Voltage_Coefficient = (Cal_Measured_Voltage * 1000) / Cal_Voltage_calc_averaged;

      } else if (Cal_Zero_Points) {
        Cal_min_Voltage_raw = Cal_Voltage_raw_averaged;
        Cal_min_Current_calc = Cal_Current_calc_averaged;
      } else {
        DEBUG_PRINTLN("No calibration_enable values selected - but that should not happen");
      }

    }

    void addToJsonInfo(JsonObject& root) {
      if (!enabled)return;

      JsonObject user = root["u"];
        if (user.isNull())user = root.createNestedObject("u");

      JsonArray Current_json = user.createNestedArray(FPSTR("Current"));
      if (Current_raw == 0 || CurrentPin == -1) {
        Current_json.add(F(_no_data));
      } else if (Current_raw >= (ADC_MAX_VALUE - 3)) {
        Current_json.add(F("Overrange"));
      } else {
        Current_json.add(Current);
        Current_json.add(F(" A"));
      }

      JsonArray Voltage_json = user.createNestedArray(FPSTR("Voltage"));
      if (Voltage_raw == 0 || VoltagePin == -1) {
        Voltage_json.add(F(_no_data));
      } else if (Voltage_raw >= (ADC_MAX_VALUE - 3)) {
        Voltage_json.add(F(_no_data));
      } else if (Voltage_raw >= (ADC_MAX_VALUE - 3)) {
        Voltage_json.add(F("Overrange"));
      } else {
        Voltage_json.add(Voltage);
        Voltage_json.add(F(" V"));
      }

      if (calibration_enable) {
        JsonArray Current_raw_json = user.createNestedArray(FPSTR("Current raw"));
        Current_raw_json.add(Current_raw);
        Current_raw_json.add(" -> " + String(Current_calc));

        JsonArray Voltage_raw_json = user.createNestedArray(FPSTR("Voltage raw"));
        Voltage_raw_json.add(Voltage_raw);
        Voltage_raw_json.add(" -> " + String(Voltage_calc));
      }

      JsonArray Power_json = user.createNestedArray(FPSTR("Power"));
      Power_json.add(Power);
      Power_json.add(F(" W"));

      JsonArray Energy_json = user.createNestedArray(FPSTR("Energy"));
      Energy_json.add(kilowatthours);
      Energy_json.add(F(" kWh"));
    }

    void addToConfig(JsonObject& root) {
      JsonObject top = root.createNestedObject(FPSTR(_name));
      top[FPSTR("enabled")] = enabled;

      JsonObject power_pins = top.createNestedObject(FPSTR("power_pins"));
      power_pins[FPSTR("Voltage Pin")] = VoltagePin;
      power_pins[FPSTR("Current Pin")] = CurrentPin;

      JsonObject update = top.createNestedObject(FPSTR("update rate in ms"));
      update[FPSTR("update rate of mqtt")] = Update_Interval_Mqtt;
      update[FPSTR("update rate of main")] = Update_Interval_Main;

      JsonObject cal = top.createNestedObject(FPSTR("calibration"));
      cal[FPSTR("calibration Mode")] = calibration_enable;
      if (calibration_enable && !Cal_In_Progress) {
        cal[FPSTR("Advanced")] = cal_adavnced;

        cal["Zero Points"] = Cal_Zero_Points;
        cal["Measured Voltage"] = Cal_Measured_Voltage;
        cal["Calibrate Voltage?"] = Cal_calibrate_Measured_Voltage;
        cal["Measured Current"] = Cal_Measured_Current;
        cal["Calibrate Current?"] = Cal_calibrate_Measured_Current;
      } else if (Cal_In_Progress) {
        cal[FPSTR("calibration_enable is in progress please wait")] = "Non-Essential Data Entry Zone: Just for Kicks and Giggles";
      }

      if (calibration_enable && cal_adavnced && !Cal_In_Progress) {
        cal[FPSTR("Number of samples")] = Num_Readings_Cal;
        cal[FPSTR("Zero Point of Voltage")] = Cal_min_Voltage_raw;
        cal[FPSTR("Zero Point of Current")] = Cal_min_Current_calc;
        cal[FPSTR("Voltage Coefficient")] = Cal_Voltage_Coefficient;
        cal[FPSTR("Current at X (mV at ADC)")] = Cal_Current_calc_at_x;
        cal[FPSTR("Current at X (mA)")] = Cal_Current_at_x;
      }
    }

    bool readFromConfig(JsonObject& root) {
      int8_t tmpVoltagePin = VoltagePin;
      int8_t tmpCurrentPin = CurrentPin;

      JsonObject top = root[FPSTR(_name)];
      if (top.isNull()) {
        DEBUG_PRINTLN(F(": No config found. (Using defaults.)"));
        return false;
      }

      enabled = top[FPSTR("Enabled")] | enabled;

      tmpVoltagePin = top[FPSTR("power_pins")][FPSTR("Voltage Pin")] | tmpVoltagePin;
      tmpCurrentPin = top[FPSTR("power_pins")][FPSTR("Current Pin")] | tmpCurrentPin;

      Update_Interval_Mqtt = top[FPSTR("update rate in ms")][FPSTR("update rate of mqtt")] | Update_Interval_Mqtt;
      Update_Interval_Main = top[FPSTR("update rate in ms")][FPSTR("update rate of main")] | Update_Interval_Main;

      JsonObject cal = top[FPSTR("calibration")];
      calibration_enable = cal[FPSTR("calibration Mode")] | calibration_enable;

      if (calibration_enable && !Cal_In_Progress) {
        cal_adavnced = cal[FPSTR("Advanced")] | cal_adavnced;

        Cal_Zero_Points = cal["Zero Points"] | Cal_Zero_Points;
        Cal_Measured_Voltage = cal["Measured Voltage"] | Cal_Measured_Voltage;
        Cal_calibrate_Measured_Voltage = cal["Calibrate Voltage?"] | Cal_calibrate_Measured_Voltage;
        Cal_Measured_Current = cal["Measured Current"] | Cal_Measured_Current;
        Cal_calibrate_Measured_Current = cal["Calibrate Current?"] | Cal_calibrate_Measured_Current;
      }

      if (calibration_enable && cal_adavnced && !Cal_In_Progress) {
        Num_Readings_Cal = cal[FPSTR("Number of samples")] | Num_Readings_Cal;
        Cal_min_Voltage_raw = cal[FPSTR("Zero Point of Voltage")] | Cal_min_Voltage_raw;
        Cal_min_Current_calc = cal[FPSTR("Zero Point of Current")] | Cal_min_Current_calc;
        Cal_Voltage_Coefficient = cal[FPSTR("Voltage Coefficient")] | Cal_Voltage_Coefficient;
        Cal_Current_calc_at_x = cal[FPSTR("Current at X (mV at ADC)")] | Cal_Current_calc_at_x;
        Cal_Current_at_x = cal[FPSTR("Current at X (mA)")] | Cal_Current_at_x;
      }

      if (!initDone) {
        // first run: reading from cfg.json
        VoltagePin = tmpVoltagePin;
        CurrentPin = tmpCurrentPin;
        DEBUG_PRINTLN(F(" config loaded."));
      } else {
        DEBUG_PRINTLN(F(" config (re)loaded."));
        // changing paramters from settings page
        if (tmpVoltagePin != VoltagePin || tmpCurrentPin != CurrentPin) {
          DEBUG_PRINTLN(F("Re-init Power pins."));
          // deallocate pin and release memory
          pinManager.deallocatePin(VoltagePin, PinOwner::UM_Power_Measurement);
          VoltagePin = tmpVoltagePin;
          pinManager.deallocatePin(CurrentPin, PinOwner::UM_Power_Measurement);
          CurrentPin = tmpCurrentPin;
          // initialise
          pinAlocation();
        }
      }

      return true;
    }

    #ifndef WLED_DISABLE_MQTT
      void onMqttConnect(bool sessionPresent) {
        publishPowerMeasurements();
      }

      void publishPowerMeasurements() {
        if (WLED_MQTT_CONNECTED) {
          char subuf[64];
          char payload[32];

          // Publish Voltage
          strcpy(subuf, mqttDeviceTopic);
          strcat_P(subuf, PSTR("/power_measurement/voltage"));
          dtostrf(Voltage, 6, 2, payload); // Convert float to string
          mqtt->publish(subuf, 0, true, payload);

          // Publish Current
          strcpy(subuf, mqttDeviceTopic);
          strcat_P(subuf, PSTR("/power_measurement/current"));
          dtostrf(Current, 6, 2, payload); // Convert float to string
          mqtt->publish(subuf, 0, true, payload);

          // Publish Power
          strcpy(subuf, mqttDeviceTopic);
          strcat_P(subuf, PSTR("/power_measurement/power"));
          dtostrf(Power, 6, 2, payload); // Convert float to string
          mqtt->publish(subuf, 0, true, payload);

          // Publish kilowatthours
          strcpy(subuf, mqttDeviceTopic);
          strcat_P(subuf, PSTR("/power_measurement/kilowatthours"));
          ultoa(kilowatthours, payload, 10); // Convert unsigned long to string
          mqtt->publish(subuf, 0, true, payload);
        }
      }
    #endif

    uint16_t getId() override {
      return USERMOD_ID_POWER_MEASUREMENT;
    }

    uint32_t readADC_Cal(int ADC_Raw) {
      esp_adc_cal_characteristics_t adc_chars;
      esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_6, ADC_WIDTH_BIT_12, ESP_ADC_CAL_VAL_DEFAULT_VREF, &adc_chars);
      if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        // Handle error if calibration_enable value is not available
        DEBUG_PRINTF("Error: eFuse Vref not available");
        return 0;
      }
      return (esp_adc_cal_raw_to_voltage(ADC_Raw, &adc_chars));
    }
};

// String used more than once
const char UsermodPower_Measurement::_name[]         PROGMEM = "Power Measurement";
const char UsermodPower_Measurement::_no_data[]      PROGMEM = "No data";