#pragma once

#include "wled.h"
#include <AHT10.h>

#define AHT10_SUCCESS 1

class UsermodAHT10 : public Usermod
{
private:
  static const char _name[];

  unsigned long _lastLoopCheck = 0;

  struct
  {
    bool settingEnabled : 1;    // Enable the usermod
    bool mqttPublish : 1;       // Publish mqtt values
    bool mqttPublishAlways : 1; // Publish always, regardless if there is a change
    bool mqttHomeAssistant : 1; // Enable Home Assistant docs
    bool initDone : 1;          // Initialization is done
    unsigned : 3;
  } _stateFlags;

  // Settings. Some of these are stored in a different format than they're user settings - so we don't have to convert at runtime
  uint8_t _i2cAddress = AHT10_ADDRESS_0X38;
  ASAIR_I2C_SENSOR _ahtType = AHT10_SENSOR;
  uint16_t _checkInterval = 60000; // milliseconds, user settings is in seconds
  float _decimalFactor = 100;      // a power of 10 factor. 1 would be no change, 10 is one decimal, 100 is two etc. User sees a power of 10 (0, 1, 2, ..)

  uint8_t _lastStatus = 0;
  float _lastHumidity = 0;
  float _lastTemperature = 0;

  AHT10 *_aht = nullptr;

  float truncateDecimals(float val)
  {
    return roundf(val * _decimalFactor) / _decimalFactor;
  }

  void initializeAht()
  {
    if (_aht != nullptr)
    {
      delete _aht;
    }

    _aht = new AHT10(_i2cAddress, _ahtType);

    _lastStatus = 0;
    _lastHumidity = 0;
    _lastTemperature = 0;
  }

  ~UsermodAHT10()
  {
    delete _aht;
    _aht = nullptr;
  }

#ifndef WLED_DISABLE_MQTT
  void mqttInitialize()
  {
    // This is a generic "setup mqtt" function, So we must abort if we're not to do mqtt
    if (!WLED_MQTT_CONNECTED || !_stateFlags.mqttPublish || !_stateFlags.mqttHomeAssistant)
      return;

    char topic[128];
    snprintf_P(topic, 127, "%s/temperature", mqttDeviceTopic);
    mqttCreateHassSensor(F("Temperature"), topic, F("temperature"), F("°C"));

    snprintf_P(topic, 127, "%s/humidity", mqttDeviceTopic);
    mqttCreateHassSensor(F("Humidity"), topic, F("humidity"), F("%"));
  }

  void mqttPublishIfChanged(const __FlashStringHelper *topic, float lastState, float state)
  {
    // Check if MQTT Connected, otherwise it will crash the 8266
    if (WLED_MQTT_CONNECTED && _stateFlags.mqttPublish && (_stateFlags.mqttPublishAlways || lastState != state))
    {
      char subuf[128];
      snprintf_P(subuf, 127, PSTR("%s/%s"), mqttDeviceTopic, (const char *)topic);
      mqtt->publish(subuf, 0, false, String(state).c_str());
    }
  }

  // Create an MQTT Sensor for Home Assistant Discovery purposes, this includes a pointer to the topic that is published to in the Loop.
  void mqttCreateHassSensor(const String &name, const String &topic, const String &deviceClass, const String &unitOfMeasurement)
  {
    String t = String(F("homeassistant/sensor/")) + mqttClientID + "/" + name + F("/config");

    StaticJsonDocument<600> doc;

    doc[F("name")] = name;
    doc[F("state_topic")] = topic;
    doc[F("unique_id")] = String(mqttClientID) + name;
    if (unitOfMeasurement != "")
      doc[F("unit_of_measurement")] = unitOfMeasurement;
    if (deviceClass != "")
      doc[F("device_class")] = deviceClass;
    doc[F("expire_after")] = 1800;

    JsonObject device = doc.createNestedObject(F("device")); // attach the sensor to the same device
    device[F("name")] = serverDescription;
    device[F("identifiers")] = "wled-sensor-" + String(mqttClientID);
    device[F("manufacturer")] = F(WLED_BRAND);
    device[F("model")] = F(WLED_PRODUCT_NAME);
    device[F("sw_version")] = versionString;

    String temp;
    serializeJson(doc, temp);
    DEBUG_PRINTLN(t);
    DEBUG_PRINTLN(temp);

    mqtt->publish(t.c_str(), 0, true, temp.c_str());
  }
#endif

public:
  void setup()
  {
    initializeAht();
  }

  void loop()
  {
    // if usermod is disabled or called during strip updating just exit
    // NOTE: on very long strips strip.isUpdating() may always return true so update accordingly
    if (!_stateFlags.settingEnabled || strip.isUpdating())
      return;

    // do your magic here
    unsigned long currentTime = millis();

    if (currentTime - _lastLoopCheck < _checkInterval)
      return;
    _lastLoopCheck = currentTime;

    _lastStatus = _aht->readRawData();

    if (_lastStatus == AHT10_ERROR)
    {
      // Perform softReset and retry
      DEBUG_PRINTLN(F("AHTxx returned error, doing softReset"));
      if (!_aht->softReset())
      {
        DEBUG_PRINTLN(F("softReset failed"));
        return;
      }

      _lastStatus = _aht->readRawData();
    }

    if (_lastStatus == AHT10_SUCCESS)
    {
      float temperature = truncateDecimals(_aht->readTemperature(AHT10_USE_READ_DATA));
      float humidity = truncateDecimals(_aht->readHumidity(AHT10_USE_READ_DATA));

#ifndef WLED_DISABLE_MQTT
      // Push to MQTT
      mqttPublishIfChanged(F("temperature"), _lastTemperature, temperature);
      mqttPublishIfChanged(F("humidity"), _lastHumidity, humidity);
#endif

      // Store
      _lastTemperature = temperature;
      _lastHumidity = humidity;
    }
  }

#ifndef WLED_DISABLE_MQTT
  void onMqttConnect(bool sessionPresent)
  {
    mqttInitialize();
  }
#endif

  uint16_t getId()
  {
    return USERMOD_ID_AHT10;
  }

  void addToJsonInfo(JsonObject &root) override
  {
    // if "u" object does not exist yet wee need to create it
    JsonObject user = root["u"];
    if (user.isNull())
      user = root.createNestedObject("u");

#ifdef USERMOD_AHT10_DEBUG
    JsonArray temp = user.createNestedArray(F("AHT last loop"));
    temp.add(_lastLoopCheck);

    temp = user.createNestedArray(F("AHT last status"));
    temp.add(_lastStatus);
#endif

    JsonArray jsonTemp = user.createNestedArray(F("Temperature"));
    JsonArray jsonHumidity = user.createNestedArray(F("Humidity"));

    if (_lastLoopCheck == 0)
    {
      // Before first run
      jsonTemp.add(F("Not read yet"));
      jsonHumidity.add(F("Not read yet"));
      return;
    }

    if (_lastStatus != AHT10_SUCCESS)
    {
      jsonTemp.add(F("An error occurred"));
      jsonHumidity.add(F("An error occurred"));
      return;
    }

    jsonTemp.add(_lastTemperature);
    jsonTemp.add(F("°C"));

    jsonHumidity.add(_lastHumidity);
    jsonHumidity.add(F("%"));
  }

  void addToConfig(JsonObject &root)
  {
    JsonObject top = root.createNestedObject(FPSTR(_name));
    top[F("Enabled")] = _stateFlags.settingEnabled;
    top[F("I2CAddress")] = static_cast<uint8_t>(_i2cAddress);
    top[F("SensorType")] = _ahtType;
    top[F("CheckInterval")] = _checkInterval / 1000;
    top[F("Decimals")] = log10f(_decimalFactor);
#ifndef WLED_DISABLE_MQTT
    top[F("MqttPublish")] = _stateFlags.mqttPublish;
    top[F("MqttPublishAlways")] = _stateFlags.mqttPublishAlways;
    top[F("MqttHomeAssistantDiscovery")] = _stateFlags.mqttHomeAssistant;
#endif

    DEBUG_PRINTLN(F("AHT10 config saved."));
  }

  bool readFromConfig(JsonObject &root) override
  {
    // default settings values could be set here (or below using the 3-argument getJsonValue()) instead of in the class definition or constructor
    // setting them inside readFromConfig() is slightly more robust, handling the rare but plausible use case of single value being missing after boot (e.g. if the cfg.json was manually edited and a value was removed)

    JsonObject top = root[FPSTR(_name)];

    bool configComplete = !top.isNull();
    if (!configComplete)
      return false;

    bool tmpBool = false;
    configComplete &= getJsonValue(top[F("Enabled")], tmpBool);
    if (configComplete)
      _stateFlags.settingEnabled = tmpBool;

    configComplete &= getJsonValue(top[F("I2CAddress")], _i2cAddress);
    configComplete &= getJsonValue(top[F("CheckInterval")], _checkInterval);
    if (configComplete)
    {
      if (1 <= _checkInterval && _checkInterval <= 600)
        _checkInterval *= 1000;
      else
        // Invalid input
        _checkInterval = 60000;
    }

    configComplete &= getJsonValue(top[F("Decimals")], _decimalFactor);
    if (configComplete)
    {
      if (0 <= _decimalFactor && _decimalFactor <= 5)
        _decimalFactor = pow10f(_decimalFactor);
      else
        // Invalid input
        _decimalFactor = 100;
    }

    uint8_t tmpAhtType;
    configComplete &= getJsonValue(top[F("SensorType")], tmpAhtType);
    if (configComplete)
    {
      if (0 <= tmpAhtType && tmpAhtType <= 2)
        _ahtType = static_cast<ASAIR_I2C_SENSOR>(tmpAhtType);
      else
        // Invalid input
        _ahtType = ASAIR_I2C_SENSOR::AHT10_SENSOR;
    }

#ifndef WLED_DISABLE_MQTT
    configComplete &= getJsonValue(top[F("MqttPublish")], tmpBool);
    if (configComplete)
      _stateFlags.mqttPublish = tmpBool;

    configComplete &= getJsonValue(top[F("MqttPublishAlways")], tmpBool);
    if (configComplete)
      _stateFlags.mqttPublishAlways = tmpBool;

    configComplete &= getJsonValue(top[F("MqttHomeAssistantDiscovery")], tmpBool);
    if (configComplete)
      _stateFlags.mqttHomeAssistant = tmpBool;
#endif

    if (_stateFlags.initDone)
    {
      // Reloading config
      initializeAht();

#ifndef WLED_DISABLE_MQTT
      mqttInitialize();
#endif
    }

    _stateFlags.initDone = true;
    return configComplete;
  }
};

const char UsermodAHT10::_name[] PROGMEM = "AHTxx";