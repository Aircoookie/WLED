#pragma once

#include "wled.h"
#include <Arduino.h>
#include <Wire.h>
#include <BME280I2C.h>               // BME280 sensor
#include <EnvironmentCalculations.h> // BME280 extended measurements

class UsermodBME280 : public Usermod
{
private:
// User-defined configuration
#define Celsius               // Show temperature mesaurement in Celcius. Comment out for Fahrenheit
#define TemperatureDecimals 1 // Number of decimal places in published temperaure values
#define HumidityDecimals 0    // Number of decimal places in published humidity values
#define PressureDecimals 2    // Number of decimal places in published pressure values
#define TemperatureInterval 5 // Interval to measure temperature (and humidity, dew point if available) in seconds
#define PressureInterval 300  // Interval to measure pressure in seconds

// Sanity checks
#if !defined(TemperatureDecimals) || TemperatureDecimals < 0
  #define TemperatureDecimals 0
#endif
#if !defined(HumidityDecimals) || HumidityDecimals < 0
  #define HumidityDecimals 0
#endif
#if !defined(PressureDecimals) || PressureDecimals < 0
  #define PressureDecimals 0
#endif
#if !defined(TemperatureInterval) || TemperatureInterval < 0
  #define TemperatureInterval 1
#endif
#if !defined(PressureInterval) || PressureInterval < 0
  #define PressureInterval TemperatureInterval
#endif

#ifdef ARDUINO_ARCH_ESP32 // ESP32 boards
  uint8_t SCL_PIN = 22;
  uint8_t SDA_PIN = 21;
#else // ESP8266 boards
  uint8_t SCL_PIN = 5;
  uint8_t SDA_PIN = 4;
  //uint8_t RST_PIN = 16; // Uncoment for Heltec WiFi-Kit-8
#endif

  // BME280 sensor settings
  BME280I2C::Settings settings{
      BME280::OSR_X16, // Temperature oversampling x16
      BME280::OSR_X16, // Humidity oversampling x16
      BME280::OSR_X16, // Pressure oversampling x16
      // Defaults
      BME280::Mode_Forced,
      BME280::StandbyTime_1000ms,
      BME280::Filter_Off,
      BME280::SpiEnable_False,
      BME280I2C::I2CAddr_0x76 // I2C address. I2C specific. Default 0x76
  };

  BME280I2C bme{settings};

  uint8_t SensorType;

  // Measurement timers
  long timer;
  long lastTemperatureMeasure = 0;
  long lastPressureMeasure = 0;

  // Current sensor values
  float SensorTemperature;
  float SensorHumidity;
  float SensorHeatIndex;
  float SensorDewPoint;
  float SensorPressure;
  // Track previous sensor values
  float lastTemperature;
  float lastHumidity;
  float lastHeatIndex;
  float lastDewPoint;
  float lastPressure;

  // Store packet IDs of MQTT publications
  uint16_t mqttTemperaturePub = 0;
  uint16_t mqttPressurePub = 0;

  void UpdateBME280Data(int SensorType)
  {
    float _temperature, _humidity, _pressure;
    #ifdef Celsius
      BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
      EnvironmentCalculations::TempUnit envTempUnit(EnvironmentCalculations::TempUnit_Celsius);
    #else
      BME280::TempUnit tempUnit(BME280::TempUnit_Fahrenheit);
      EnvironmentCalculations::TempUnit envTempUnit(EnvironmentCalculations::TempUnit_Fahrenheit);
    #endif
    BME280::PresUnit presUnit(BME280::PresUnit_hPa);

    bme.read(_pressure, _temperature, _humidity, tempUnit, presUnit);

    SensorTemperature = _temperature;
    SensorHumidity = _humidity;
    SensorPressure = _pressure;
    if (SensorType == 1)
    {
      SensorHeatIndex = EnvironmentCalculations::HeatIndex(_temperature, _humidity, envTempUnit);
      SensorDewPoint = EnvironmentCalculations::DewPoint(_temperature, _humidity, envTempUnit);
    }
  }

public:
  void setup()
  {
    Wire.begin(SDA_PIN, SCL_PIN);

    if (!bme.begin())
    {
      SensorType = 0;
      Serial.println("Could not find BME280I2C sensor!");
    }
    else
    {
      switch (bme.chipModel())
      {
      case BME280::ChipModel_BME280:
        SensorType = 1;
        Serial.println("Found BME280 sensor! Success.");
        break;
      case BME280::ChipModel_BMP280:
        SensorType = 2;
        Serial.println("Found BMP280 sensor! No Humidity available.");
        break;
      default:
        SensorType = 0;
        Serial.println("Found UNKNOWN sensor! Error!");
      }
    }
  }

  void loop()
  {
    // BME280 sensor MQTT publishing
    // Check if sensor present and MQTT Connected, otherwise it will crash the MCU
    if (SensorType != 0 && mqtt != nullptr)
    {
      // Timer to fetch new temperature, humidity and pressure data at intervals
      timer = millis();

      if (timer - lastTemperatureMeasure >= TemperatureInterval * 1000 || mqttTemperaturePub == 0)
      {
        lastTemperatureMeasure = timer;

        UpdateBME280Data(SensorType);

        float Temperature = roundf(SensorTemperature * pow(10, TemperatureDecimals)) / pow(10, TemperatureDecimals);
        float Humidity, HeatIndex, DewPoint;

        // If temperature has changed since last measure, create string populated with device topic
        // from the UI and values read from sensor, then publish to broker
        if (Temperature != lastTemperature)
        {
          String topic = String(mqttDeviceTopic) + "/temperature";
          mqttTemperaturePub = mqtt->publish(topic.c_str(), 0, false, String(Temperature, TemperatureDecimals).c_str());
        }

        lastTemperature = Temperature; // Update last sensor temperature for next loop

        if (SensorType == 1) // Only if sensor is a BME280
        {
          Humidity = roundf(SensorHumidity * pow(10, HumidityDecimals)) / pow(10, HumidityDecimals);
          HeatIndex = roundf(SensorHeatIndex * pow(10, TemperatureDecimals)) / pow(10, TemperatureDecimals);
          DewPoint = roundf(SensorDewPoint * pow(10, TemperatureDecimals)) / pow(10, TemperatureDecimals);

          if (Humidity != lastHumidity)
          {
            String topic = String(mqttDeviceTopic) + "/humidity";
            mqtt->publish(topic.c_str(), 0, false, String(Humidity, HumidityDecimals).c_str());
          }

          if (HeatIndex != lastHeatIndex)
          {
            String topic = String(mqttDeviceTopic) + "/heat_index";
            mqtt->publish(topic.c_str(), 0, false, String(HeatIndex, TemperatureDecimals).c_str());
          }

          if (DewPoint != lastDewPoint)
          {
            String topic = String(mqttDeviceTopic) + "/dew_point";
            mqtt->publish(topic.c_str(), 0, false, String(DewPoint, TemperatureDecimals).c_str());
          }

          lastHumidity = Humidity;
          lastHeatIndex = HeatIndex;
          lastDewPoint = DewPoint;
        }
      }

      if (timer - lastPressureMeasure >= PressureInterval * 1000 || mqttPressurePub == 0)
      {
        lastPressureMeasure = timer;

        float Pressure = roundf(SensorPressure * pow(10, PressureDecimals)) / pow(10, PressureDecimals);

        if (Pressure != lastPressure)
        {
          String topic = String(mqttDeviceTopic) + "/pressure";
          mqttPressurePub = mqtt->publish(topic.c_str(), 0, true, String(Pressure, PressureDecimals).c_str());
        }

        lastPressure = Pressure;
      }
    }
  }
};