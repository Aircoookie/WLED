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
#define HumidityDecimals 2    // Number of decimal places in published humidity values
#define PressureDecimals 2    // Number of decimal places in published pressure values
#define TemperatureInterval 5 // Interval to measure temperature (and humidity, dew point if available) in seconds
#define PressureInterval 300  // Interval to measure pressure in seconds
#define PublishAlways 0       // Publish values even when they have not changed

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
#if !defined(PublishAlways)
  #define PublishAlways 0
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

  uint8_t sensorType;

  // Measurement timers
  long timer;
  long lastTemperatureMeasure = 0;
  long lastPressureMeasure = 0;

  // Current sensor values
  float sensorTemperature;
  float sensorHumidity;
  float sensorHeatIndex;
  float sensorDewPoint;
  float sensorPressure;
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

    sensorTemperature = _temperature;
    sensorHumidity = _humidity;
    sensorPressure = _pressure;
    if (sensorType == 1)
    {
      sensorHeatIndex = EnvironmentCalculations::HeatIndex(_temperature, _humidity, envTempUnit);
      sensorDewPoint = EnvironmentCalculations::DewPoint(_temperature, _humidity, envTempUnit);
    }
  }

public:
  void setup()
  {
    Wire.begin(SDA_PIN, SCL_PIN);

    if (!bme.begin())
    {
      sensorType = 0;
      Serial.println("Could not find BME280I2C sensor!");
    }
    else
    {
      switch (bme.chipModel())
      {
      case BME280::ChipModel_BME280:
        sensorType = 1;
        Serial.println("Found BME280 sensor! Success.");
        break;
      case BME280::ChipModel_BMP280:
        sensorType = 2;
        Serial.println("Found BMP280 sensor! No Humidity available.");
        break;
      default:
        sensorType = 0;
        Serial.println("Found UNKNOWN sensor! Error!");
      }
    }
  }

  void loop()
  {
    // BME280 sensor MQTT publishing
    // Check if sensor present and MQTT Connected, otherwise it will crash the MCU
    if (sensorType != 0 && mqtt != nullptr)
    {
      // Timer to fetch new temperature, humidity and pressure data at intervals
      timer = millis();

      if (timer - lastTemperatureMeasure >= TemperatureInterval * 1000 || mqttTemperaturePub == 0)
      {
        lastTemperatureMeasure = timer;

        UpdateBME280Data(sensorType);

        float temperature = roundf(sensorTemperature * pow(10, TemperatureDecimals)) / pow(10, TemperatureDecimals);
        float humidity, heatIndex, dewPoint;

        // If temperature has changed since last measure, create string populated with device topic
        // from the UI and values read from sensor, then publish to broker
        if (temperature != lastTemperature || PublishAlways)
        {
          String topic = String(mqttDeviceTopic) + "/temperature";
          mqttTemperaturePub = mqtt->publish(topic.c_str(), 0, false, String(temperature, TemperatureDecimals).c_str());
        }

        lastTemperature = temperature; // Update last sensor temperature for next loop

        if (sensorType == 1) // Only if sensor is a BME280
        {
          humidity = roundf(sensorHumidity * pow(10, HumidityDecimals)) / pow(10, HumidityDecimals);
          heatIndex = roundf(sensorHeatIndex * pow(10, TemperatureDecimals)) / pow(10, TemperatureDecimals);
          dewPoint = roundf(sensorDewPoint * pow(10, TemperatureDecimals)) / pow(10, TemperatureDecimals);

          if (humidity != lastHumidity || PublishAlways)
          {
            String topic = String(mqttDeviceTopic) + "/humidity";
            mqtt->publish(topic.c_str(), 0, false, String(humidity, HumidityDecimals).c_str());
          }

          if (heatIndex != lastHeatIndex || PublishAlways)
          {
            String topic = String(mqttDeviceTopic) + "/heat_index";
            mqtt->publish(topic.c_str(), 0, false, String(heatIndex, TemperatureDecimals).c_str());
          }

          if (dewPoint != lastDewPoint || PublishAlways)
          {
            String topic = String(mqttDeviceTopic) + "/dew_point";
            mqtt->publish(topic.c_str(), 0, false, String(dewPoint, TemperatureDecimals).c_str());
          }

          lastHumidity = humidity;
          lastHeatIndex = heatIndex;
          lastDewPoint = dewPoint;
        }
      }

      if (timer - lastPressureMeasure >= PressureInterval * 1000 || mqttPressurePub == 0)
      {
        lastPressureMeasure = timer;

        float pressure = roundf(sensorPressure * pow(10, PressureDecimals)) / pow(10, PressureDecimals);

        if (pressure != lastPressure || PublishAlways)
        {
          String topic = String(mqttDeviceTopic) + "/pressure";
          mqttPressurePub = mqtt->publish(topic.c_str(), 0, true, String(pressure, PressureDecimals).c_str());
        }

        lastPressure = pressure;
      }
    }
  }
};