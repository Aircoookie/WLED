#pragma once

// this is remixed from usermod_v2_SensorsToMqtt.h (sensors_to_mqtt usermod)

#include "wled.h"
// #include <Arduino.h>
// #include <Wire.h>
// #include <Adafruit_Sensor.h>
// #include <Adafruit_BMP280.h>
// #include <Adafruit_CCS811.h>
#include <Adafruit_Si7021.h>

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
  float SensorTemperature = 0;
  float SensorHumidity = 0;
  String mqttTemperatureTopic = "";
  String mqttHumidityTopic = "";
  unsigned long nextMeasure = 0;

  void _initialize()
  {
    initialized = si7021.begin();
    Serial.printf("Si7021_MQTT: initialized = %d\n", initialized);
  }

  void _mqttInitialize()
  {
    mqttTemperatureTopic = String(mqttDeviceTopic) + "/temperature";
    mqttHumidityTopic = String(mqttDeviceTopic) + "/humidity";

    String t = String("homeassistant/sensor/") + mqttClientID + "/temperature/config";

    _createMqttSensor("temperature", mqttTemperatureTopic, "temperature", "°C");
    _createMqttSensor("humidity", mqttHumidityTopic, "humidity", "%");
  }

  void _createMqttSensor(const String &name, const String &topic, const String &deviceClass, const String &unitOfMeasurement)
  {
    String t = String("homeassistant/sensor/") + mqttClientID + "/" + name + "/config";

    StaticJsonDocument<300> doc;

    doc["name"] = name;
    doc["state_topic"] = topic;
    doc["unique_id"] = String(mqttClientID) + name;
    if (unitOfMeasurement != "")
      doc["unit_of_measurement"] = unitOfMeasurement;
    if (deviceClass != "")
      doc["device_class"] = deviceClass;
    doc["expire_after"] = 1800;

    JsonObject device = doc.createNestedObject("device"); // attach the sensor to the same device
    device["identifiers"] = String("wled-sensor-") + mqttClientID;
    device["manufacturer"] = "Aircoookie";
    device["model"] = "WLED";
    device["sw_version"] = VERSION;
    device["name"] = mqttClientID;

    String temp;
    serializeJson(doc, temp);
    // Serial.println("Si7021_MQTT:");
    // Serial.println(t);
    // Serial.println(temp);

    mqtt->publish(t.c_str(), 0, true, temp.c_str());
  }

  void _updateSensorData()
  {
    SensorTemperature = si7021.readTemperature();
    SensorHumidity = si7021.readHumidity();

    // char ch = 248; // "°"
    Serial.print("Si7021_MQTT: Temperature: ");
    Serial.print(SensorTemperature, 2);
    Serial.print("\tHumidity: ");
    Serial.println(SensorHumidity, 2);
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
      // nextMeasure = tempTimer + 60000; // Schedule next measure in 60 seconds
      nextMeasure = tempTimer + 10000; // debug

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
        mqtt->publish(mqttTemperatureTopic.c_str(), 0, true, String(SensorTemperature).c_str());
        mqtt->publish(mqttHumidityTopic.c_str(), 0, true, String(SensorHumidity).c_str());
      }
      else
      {
        Serial.println("Si7021_MQTT: Missing MQTT connection. Not publishing data");
        mqttInitialized = false;
      }
    }
  }
};
