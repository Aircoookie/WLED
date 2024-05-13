#pragma once

#include "wled.h"
#include <AHT10.h>

#define AHT10_SUCCESS 1

class UsermodAHT10 : public Usermod {
  private:
    static const char _name[];

    unsigned long _lastLoopCheck = 0;
    bool _initDone = false;

    // Settings. Some of these are stored in a different format than they're user settings - so we don't have to convert at runtime
    bool _enabled = false;
    uint8_t _i2cAddress = AHT10_ADDRESS_0X38;
    ASAIR_I2C_SENSOR _ahtType = AHT10_SENSOR;
    uint16_t _checkInterval = 60000;    // milliseconds, user settings is in seconds
    float _decimalFactor = 100;         // a power of 10 factor. 1 would be no change, 10 is one decimal, 100 is two etc. User sees a power of 10 (0, 1, 2, ..)

    uint8_t _lastStatus = 0;
    float _lastHumidity = 0;
    float _lastTemperature = 0;

    AHT10* _aht = nullptr;

    float truncateDecimals(float val) {
      return roundf(val * _decimalFactor) / _decimalFactor;
    }

    void initializeAht() {
      if (_aht != nullptr) {
        delete _aht;
      }

      _aht = new AHT10(_i2cAddress, _ahtType);

      _lastStatus = 0;
      _lastHumidity = 0;
      _lastTemperature = 0;
    }

    ~UsermodAHT10() {
      delete _aht;
      _aht = nullptr;
    }

  public:
    void setup() {
      initializeAht();
    }

    void loop() {
      // if usermod is disabled or called during strip updating just exit
      // NOTE: on very long strips strip.isUpdating() may always return true so update accordingly
      if (!_enabled || strip.isUpdating())
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
        DEBUG_PRINTLN("AHTxx returned error, doing softReset");
        if (!_aht->softReset())
        {
          DEBUG_PRINTLN("softReset failed");
          return;
        }

        _lastStatus = _aht->readRawData();
      }

      if (_lastStatus == AHT10_SUCCESS)
      {
        float temperature = truncateDecimals(_aht->readTemperature(AHT10_USE_READ_DATA));
        float humidity = truncateDecimals(_aht->readHumidity(AHT10_USE_READ_DATA));

        // Push to MQTT

        // Store
        _lastHumidity = humidity;
        _lastTemperature = temperature;
      }
    }

    void addToJsonInfo(JsonObject& root) override
    {
      // if "u" object does not exist yet wee need to create it
      JsonObject user = root["u"];
      if (user.isNull()) 
        user = root.createNestedObject("u");

#ifdef USERMOD_AHT10_DEBUG
      JsonArray temp = user.createNestedArray(F("status"));
      temp.add(_lastLoopCheck);
      temp.add(F(" / "));
      temp.add(_lastStatus);
#endif

      JsonArray jsonTemp = user.createNestedArray(F("AHT Temperature"));
      JsonArray jsonHumidity = user.createNestedArray(F("AHT Humidity"));
      
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

    void addToConfig(JsonObject& root)
    {
      JsonObject top = root.createNestedObject(FPSTR(_name));
      top[F("Enabled")] = _enabled;
      top[F("I2CAddress")] = static_cast<uint8_t>(_i2cAddress);
      top[F("SensorType")] = _ahtType;
      top[F("CheckInterval")] = _checkInterval / 1000;
      top[F("Decimals")] = log10f(_decimalFactor);

      DEBUG_PRINTLN(F("AHT10 config saved."));
    }

    bool readFromConfig(JsonObject& root) override
    {
      // default settings values could be set here (or below using the 3-argument getJsonValue()) instead of in the class definition or constructor
      // setting them inside readFromConfig() is slightly more robust, handling the rare but plausible use case of single value being missing after boot (e.g. if the cfg.json was manually edited and a value was removed)

      JsonObject top = root[FPSTR(_name)];

      bool configComplete = !top.isNull();
      if (!configComplete)
        return false;

      configComplete &= getJsonValue(top["Enabled"], _enabled);
      configComplete &= getJsonValue(top["I2CAddress"], _i2cAddress);
      configComplete &= getJsonValue(top["CheckInterval"], _checkInterval);
      if (configComplete)
      {
        if (1 <= _checkInterval && _checkInterval <= 600)
          _checkInterval *= 1000;
        else
          // Invalid input
          _checkInterval = 60000;
      }

      configComplete &= getJsonValue(top["Decimals"], _decimalFactor);
      if (configComplete)
      {
        if (0 <= _decimalFactor && _decimalFactor <= 5)
          _decimalFactor = pow10f(_decimalFactor);
        else
          // Invalid input
          _decimalFactor = 100;
      }

      uint8_t tmpAhtType;
      configComplete &= getJsonValue(top["SensorType"], tmpAhtType);
       if (configComplete)
      {
        if (0 <= tmpAhtType && tmpAhtType <= 2)
          _ahtType = static_cast<ASAIR_I2C_SENSOR>(tmpAhtType);
        else
          // Invalid input
          _ahtType = 0;
      }

      if (_initDone)
      {
        // Reloading config
        initializeAht();
      }

      _initDone = true;
      return configComplete;
    }
};

const char UsermodAHT10::_name[]    PROGMEM = "AHTxx";