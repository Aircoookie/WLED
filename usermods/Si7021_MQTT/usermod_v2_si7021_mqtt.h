#pragma once

// this is remixed from usermod_v2_SensorsToMqtt.h (sensors_to_mqtt usermod)

#include "wled.h"
// #include <Arduino.h>
// #include <Wire.h>
// #include <Adafruit_Sensor.h>
// #include <Adafruit_BMP280.h>
// #include <Adafruit_CCS811.h>
#include <Adafruit_Si7021.h>
#include <EnvironmentCalculations.h> // BME280 extended measurements

// Adafruit_BMP280 bmp;
Adafruit_Si7021 si7021;
// Adafruit_CCS811 ccs811;

#ifdef ARDUINO_ARCH_ESP32 //ESP32 boards
uint8_t SCL_PIN = 22;
uint8_t SDA_PIN = 21;
#else //ESP8266 boards
uint8_t SCL_PIN = 5;
uint8_t SDA_PIN = 4;
#endif

class Si7021_MQTT : public Usermod
{
private:
  bool initialized = false;
  bool mqttInitialized = false;
  float sensorTemperature = 0;
  float sensorHumidity = 0;
  float sensorHeatIndex = 0;
  float sensorDewPoint = 0;
  float sensorAbsoluteHumidity= 0;
  String mqttTemperatureTopic = "";
  String mqttHumidityTopic = "";
  String mqttHeatIndexTopic = "";
  String mqttDewPointTopic = "";
  String mqttAbsoluteHumidityTopic = "";
  unsigned long nextMeasure = 0;

  void _initialize()
  {
    initialized = si7021.begin();
    Serial.printf("Si7021_MQTT: initialized = %d\n", initialized);
  }

  void _mqttInitialize()
  {
    mqttTemperatureTopic = String(mqttDeviceTopic) + "/si7021_temperature";
    mqttHumidityTopic = String(mqttDeviceTopic) + "/si7021_humidity";
    mqttHeatIndexTopic = String(mqttDeviceTopic) + "/si7021_heat_index";
    mqttDewPointTopic = String(mqttDeviceTopic) + "/si7021_dew_point";
    mqttAbsoluteHumidityTopic = String(mqttDeviceTopic) + "/si7021_absolute_humidity";

    String t = String("homeassistant/sensor/") + mqttClientID + "/temperature/config";

    _createMqttSensor("temperature", "Temperature", mqttTemperatureTopic, "temperature", "°C");
    _createMqttSensor("humidity", "Humidity", mqttHumidityTopic, "humidity", "%");
    _createMqttSensor("heat_index", "Heat Index", mqttHeatIndexTopic, "", "");
    _createMqttSensor("dew_point", "Dew Point", mqttDewPointTopic, "", "°C");
    _createMqttSensor("absolute_humidity", "Absolute Humidity", mqttAbsoluteHumidityTopic, "", "g/m³");
  }

  void _createMqttSensor(
    const String &name, 
    const String &friendly_name, 
    const String &state_topic, 
    const String &deviceClass, 
    const String &unitOfMeasurement)
  {
    String topic = String("homeassistant/sensor/") + mqttClientID + "/" + name + "/config";

    StaticJsonDocument<300> doc;

    doc["name"] = String(serverDescription) + " " + friendly_name;
    doc["state_topic"] = state_topic;
    doc["unique_id"] = String(mqttClientID) + name;
    if (unitOfMeasurement != "")
      doc["unit_of_measurement"] = unitOfMeasurement;
    if (deviceClass != "")
      doc["device_class"] = deviceClass;
    doc["expire_after"] = 1800;

    JsonObject device = doc.createNestedObject("device"); // attach the sensor to the same device
    device["name"] = String(serverDescription);
    device["model"] = "WLED";
    device["manufacturer"] = "Aircoookie";
    device["identifiers"] = String("wled-") + String(serverDescription);
    device["sw_version"] = VERSION;

    String payload;
    serializeJson(doc, payload);
    // Serial.println("Si7021_MQTT:");
    // Serial.println(t);
    // Serial.println(temp);

    mqtt->publish(topic.c_str(), 0, false, payload.c_str());
  }

  void _updateSensorData()
  {
    sensorTemperature = si7021.readTemperature();
    sensorHumidity = si7021.readHumidity();

    EnvironmentCalculations::TempUnit envTempUnit(EnvironmentCalculations::TempUnit_Celsius);
    sensorHeatIndex = EnvironmentCalculations::HeatIndex(sensorTemperature, sensorHumidity, envTempUnit);
    sensorDewPoint = EnvironmentCalculations::DewPoint(sensorTemperature, sensorHumidity, envTempUnit);
    sensorAbsoluteHumidity = EnvironmentCalculations::AbsoluteHumidity(sensorTemperature, sensorHumidity, envTempUnit);


    // char ch = 248; // "°"
    Serial.print("Si7021_MQTT: Temperature: ");
    Serial.print(sensorTemperature, 2);
    Serial.print("\tHumidity: ");
    Serial.print(sensorHumidity, 2);
    Serial.print("\tHeat Index: ");
    Serial.print(sensorHeatIndex, 2);
    Serial.print("\tDew Point: ");
    Serial.print(sensorDewPoint, 2);
    Serial.print("\tAbsolute Humidity: ");
    Serial.println(sensorAbsoluteHumidity, 2);
  }

public:
  void setup()
  {
    Serial.println("Si7021_MQTT: Starting!");
    Wire.begin(SDA_PIN, SCL_PIN);
    Serial.println("Si7021_MQTT: Initializing sensors.. ");
    _initialize();
  }

  // gets called every time WiFi is (re-)connected.
  void connected()
  {
    nextMeasure = millis() + 5000; // Schedule next measure in 5 seconds
  }

  void loop()
  {
    unsigned long tempTimer = millis();

    if (tempTimer > nextMeasure)
    {
      nextMeasure = tempTimer + 60000; // Schedule next measure in 60 seconds

      if (!initialized)
      {
        Serial.println("Si7021_MQTT: Error! Sensors not initialized in loop()!");
        _initialize();
        return; // lets try again next loop
      }

      if (mqtt != nullptr && mqtt->connected())
      {
        if (!mqttInitialized)
        {
          _mqttInitialize();
          mqttInitialized = true;
        }

        // Update sensor data
        _updateSensorData();

        // Create string populated with user defined device topic from the UI,
        // and the read temperature, humidity and pressure.
        // Then publish to MQTT server.
        mqtt->publish(mqttTemperatureTopic.c_str(), 0, false, String(sensorTemperature).c_str());
        mqtt->publish(mqttHumidityTopic.c_str(), 0, false, String(sensorHumidity).c_str());
        mqtt->publish(mqttHeatIndexTopic.c_str(), 0, false, String(sensorHeatIndex).c_str());
        mqtt->publish(mqttDewPointTopic.c_str(), 0, false, String(sensorDewPoint).c_str());
        mqtt->publish(mqttAbsoluteHumidityTopic.c_str(), 0, false, String(sensorAbsoluteHumidity).c_str());
      }
      else
      {
        Serial.println("Si7021_MQTT: Missing MQTT connection. Not publishing data");
        mqttInitialized = false;
      }
    }
  }
};
