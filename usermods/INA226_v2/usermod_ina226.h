#pragma once

#include "wled.h"
#include <INA226_WE.h>

#define INA226_ADDRESS 0x40 // Default I2C address for INA226

class UsermodINA226 : public Usermod
{
private:
    static const char _name[];

    unsigned long _lastLoopCheck = 0;

    bool _settingEnabled : 1;    // Enable the usermod
    bool _mqttPublish : 1;       // Publish MQTT values
    bool _mqttPublishAlways : 1; // Publish always, regardless if there is a change
    bool _mqttHomeAssistant : 1; // Enable Home Assistant docs
    bool _initDone : 1;          // Initialization is done

    uint8_t _i2cAddress = INA226_ADDRESS;
    uint16_t _checkInterval = 60000; // milliseconds, user settings is in seconds
    float _decimalFactor = 100;      // a power of 10 factor. 1 would be no change, 10 is one decimal, 100 is two etc. User sees a power of 10 (0, 1, 2, ..)
    uint16_t _shuntResistor = 1000;  // Shunt resistor value in milliohms
    uint16_t _currentRange = 1000;   // Expected maximum current in milliamps

    uint8_t _lastStatus = 0;
    float _lastCurrent = 0;
    float _lastVoltage = 0;
    float _lastPower = 0;
    float _lastShuntVoltage = 0;
    bool _lastOverflow = false;

#ifndef WLED_MQTT_DISABLE
    float _lastCurrentSent = 0;
    float _lastVoltageSent = 0;
    float _lastPowerSent = 0;
    float _lastShuntVoltageSent = 0;
    bool _lastOverflowSent = false;
#endif

    INA226_WE *_ina226 = nullptr;

    float truncateDecimals(float val)
    {
        return roundf(val * _decimalFactor) / _decimalFactor;
    }

    void setOptimalSettings()
    {
        INA226_AVERAGES avg;
        INA226_CONV_TIME conversionTime;

        // Identify the combination of samples and conversion times that will provide us with a measurement within our specified check interval.
        // The two values will define how stable a measurement is (number of samples) and how much time can be used to calculate on it
        // (conversion time). The calculation is:
        //    `Samples * ConversionTime * 2`
        //
        // This table shows all possible combinations and the time it'll take.
        // | Conversion Time (μs) | 1 Sample | 4 Samples | 16 Samples | 64 Samples | 128 Samples | 256 Samples | 512 Samples | 1024 Samples |
        // |----------------------|----------|-----------|------------|------------|-------------|-------------|-------------|--------------|
        // | 140                  | 0.28 ms  | 1.12 ms   | 4.48 ms    | 17.92 ms   | 35.84 ms    | 71.68 ms    | 143.36 ms   | 286.72 ms    |
        // | 204                  | 0.408 ms | 1.632 ms  | 6.528 ms   | 26.112 ms  | 52.224 ms   | 104.448 ms  | 208.896 ms  | 417.792 ms   |
        // | 332                  | 0.664 ms | 2.656 ms  | 10.624 ms  | 42.496 ms  | 84.992 ms   | 169.984 ms  | 339.968 ms  | 679.936 ms   |
        // | 588                  | 1.176 ms | 4.704 ms  | 18.816 ms  | 75.264 ms  | 150.528 ms  | 301.056 ms  | 602.112 ms  | 1204.224 ms  |
        // | 1100                 | 2.2 ms   | 8.8 ms    | 35.2 ms    | 140.8 ms   | 281.6 ms    | 563.2 ms    | 1126.4 ms   | 2252.8 ms    |
        // | 2116                 | 4.232 ms | 16.928 ms | 67.712 ms  | 270.848 ms | 541.696 ms  | 1083.392 ms | 2166.784 ms | 4333.568 ms  |
        // | 4156                 | 8.312 ms | 33.248 ms | 132.992 ms | 531.968 ms | 1063.936 ms | 2127.872 ms | 4255.744 ms | 8511.488 ms  |
        // | 8244                 | 16.488 ms| 65.952 ms | 263.808 ms | 1055.232 ms| 2110.464 ms | 4220.928 ms | 8441.856 ms | 16883.712 ms |

        // The below determines which number of average samples to use, because this number is likely most important, and then finds the max conversion time.
        if (_checkInterval >= 5000)
        {
            avg = AVERAGE_1024;
            if (_checkInterval > 17000)
                conversionTime = CONV_TIME_8244;
            else
                conversionTime = CONV_TIME_4156;
        }
        else if (_checkInterval >= 2000)
        {
            avg = AVERAGE_512;
            if (_checkInterval > 3000)
                conversionTime = CONV_TIME_2116;
            else
                conversionTime = CONV_TIME_1100;
        }
        else
        {
            // Always 1 second or more

            avg = AVERAGE_256;
            if (_checkInterval >= 3000)
                conversionTime = CONV_TIME_4156;
            else if (_checkInterval >= 2000)
                conversionTime = CONV_TIME_2116;
            else
                conversionTime = CONV_TIME_1100;
        }

        _ina226->setAverage(avg);
        _ina226->setConversionTime(conversionTime);
    }

    void initializeINA226()
    {
        if (_ina226 != nullptr)
        {
            delete _ina226;
        }

        _ina226 = new INA226_WE(_i2cAddress);
        if (!_ina226->init())
        {
            DEBUG_PRINTLN(F("INA226 initialization failed!"));
            return;
        }
        _ina226->setCorrectionFactor(1.0);
        setOptimalSettings();
        _ina226->setMeasureMode(CONTINUOUS);
        _ina226->setResistorRange(static_cast<float>(_shuntResistor) / 1000.0, static_cast<float>(_currentRange) / 1000.0);
    }

    ~UsermodINA226()
    {
        delete _ina226;
        _ina226 = nullptr;
    }

#ifndef WLED_DISABLE_MQTT
    void mqttInitialize()
    {
        if (!WLED_MQTT_CONNECTED || !_mqttPublish || !_mqttHomeAssistant)
            return;

        char topic[128];
        snprintf_P(topic, 127, "%s/current", mqttDeviceTopic);
        mqttCreateHassSensor(F("Current"), topic, F("current"), F("A"));

        snprintf_P(topic, 127, "%s/voltage", mqttDeviceTopic);
        mqttCreateHassSensor(F("Voltage"), topic, F("voltage"), F("V"));

        snprintf_P(topic, 127, "%s/power", mqttDeviceTopic);
        mqttCreateHassSensor(F("Power"), topic, F("power"), F("W"));

        snprintf_P(topic, 127, "%s/shunt_voltage", mqttDeviceTopic);
        mqttCreateHassSensor(F("Shunt Voltage"), topic, F("voltage"), F("V"));

        snprintf_P(topic, 127, "%s/overflow", mqttDeviceTopic);
        mqttCreateHassBinarySensor(F("Overflow"), topic);
    }

    void mqttPublishIfChanged(const __FlashStringHelper *topic, float &lastState, float state, float minChange)
    {
        if (WLED_MQTT_CONNECTED && _mqttPublish && (_mqttPublishAlways || fabsf(lastState - state) > minChange))
        {
            char subuf[128];
            snprintf_P(subuf, 127, PSTR("%s/%s"), mqttDeviceTopic, (const char *)topic);
            mqtt->publish(subuf, 0, false, String(state).c_str());

            lastState = state;
        }
    }

    void mqttPublishIfChanged(const __FlashStringHelper *topic, bool &lastState, bool state)
    {
        if (WLED_MQTT_CONNECTED && _mqttPublish && (_mqttPublishAlways || lastState != state))
        {
            char subuf[128];
            snprintf_P(subuf, 127, PSTR("%s/%s"), mqttDeviceTopic, (const char *)topic);
            mqtt->publish(subuf, 0, false, state ? "true" : "false");

            lastState = state;
        }
    }

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

        JsonObject device = doc.createNestedObject(F("device"));
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

    void mqttCreateHassBinarySensor(const String &name, const String &topic)
    {
        String t = String(F("homeassistant/binary_sensor/")) + mqttClientID + "/" + name + F("/config");

        StaticJsonDocument<600> doc;

        doc[F("name")] = name;
        doc[F("state_topic")] = topic;
        doc[F("unique_id")] = String(mqttClientID) + name;

        JsonObject device = doc.createNestedObject(F("device"));
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
        initializeINA226();
    }

    void loop()
    {
        if (!_settingEnabled || strip.isUpdating())
            return;

        unsigned long currentTime = millis();

        if (currentTime - _lastLoopCheck < _checkInterval)
            return;
        _lastLoopCheck = currentTime;

        _lastStatus = _ina226->getI2cErrorCode();

        if (_lastStatus == 0)
        {
            float current = truncateDecimals(_ina226->getCurrent_mA() / 1000.0);
            float voltage = truncateDecimals(_ina226->getBusVoltage_V());
            float power = truncateDecimals(_ina226->getBusPower() / 1000.0); // Correct power value to W
            float shuntVoltage = truncateDecimals(_ina226->getShuntVoltage_V());
            bool overflow = _ina226->overflow;

#ifndef WLED_DISABLE_MQTT
            mqttPublishIfChanged(F("current"), _lastCurrentSent, current, 0.01f);
            mqttPublishIfChanged(F("voltage"), _lastVoltageSent, voltage, 0.01f);
            mqttPublishIfChanged(F("power"), _lastPowerSent, power, 0.1f);
            mqttPublishIfChanged(F("shunt_voltage"), _lastShuntVoltageSent, shuntVoltage, 0.01f);
            mqttPublishIfChanged(F("overflow"), _lastOverflowSent, overflow);
#endif

            _lastCurrent = current;
            _lastVoltage = voltage;
            _lastPower = power;
            _lastShuntVoltage = shuntVoltage;
            _lastOverflow = overflow;
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
        return USERMOD_ID_INA226;
    }

    void addToJsonInfo(JsonObject &root) override
    {
        JsonObject user = root["u"];
        if (user.isNull())
            user = root.createNestedObject("u");

        JsonArray jsonCurrent = user.createNestedArray(F("Current"));
        JsonArray jsonVoltage = user.createNestedArray(F("Voltage"));
        JsonArray jsonPower = user.createNestedArray(F("Power"));
        JsonArray jsonShuntVoltage = user.createNestedArray(F("Shunt Voltage"));
        JsonArray jsonOverflow = user.createNestedArray(F("Overflow"));

        if (_lastLoopCheck == 0)
        {
            jsonCurrent.add(F("Not read yet"));
            jsonVoltage.add(F("Not read yet"));
            jsonPower.add(F("Not read yet"));
            jsonShuntVoltage.add(F("Not read yet"));
            jsonOverflow.add(F("Not read yet"));
            return;
        }

        if (_lastStatus != 0)
        {
            jsonCurrent.add(F("An error occurred"));
            jsonVoltage.add(F("An error occurred"));
            jsonPower.add(F("An error occurred"));
            jsonShuntVoltage.add(F("An error occurred"));
            jsonOverflow.add(F("An error occurred"));
            return;
        }

        jsonCurrent.add(_lastCurrent);
        jsonCurrent.add(F("A"));

        jsonVoltage.add(_lastVoltage);
        jsonVoltage.add(F("V"));

        jsonPower.add(_lastPower);
        jsonPower.add(F("W"));

        jsonShuntVoltage.add(_lastShuntVoltage);
        jsonShuntVoltage.add(F("V"));

        jsonOverflow.add(_lastOverflow ? F("true") : F("false"));
    }

    void addToConfig(JsonObject &root)
    {
        JsonObject top = root.createNestedObject(FPSTR(_name));
        top[F("Enabled")] = _settingEnabled;
        top[F("I2CAddress")] = static_cast<uint8_t>(_i2cAddress);
        top[F("CheckInterval")] = _checkInterval / 1000;
        top[F("Decimals")] = log10f(_decimalFactor);
        top[F("ShuntResistor")] = _shuntResistor;
        top[F("CurrentRange")] = _currentRange;
#ifndef WLED_DISABLE_MQTT
        top[F("MqttPublish")] = _mqttPublish;
        top[F("MqttPublishAlways")] = _mqttPublishAlways;
        top[F("MqttHomeAssistantDiscovery")] = _mqttHomeAssistant;
#endif

        DEBUG_PRINTLN(F("INA226 config saved."));
    }

    bool readFromConfig(JsonObject &root) override
    {
        JsonObject top = root[FPSTR(_name)];

        bool configComplete = !top.isNull();
        if (!configComplete)
            return false;

        bool tmpBool = false;
        configComplete &= getJsonValue(top[F("Enabled")], tmpBool);
        if (configComplete)
            _settingEnabled = tmpBool;

        configComplete &= getJsonValue(top[F("I2CAddress")], _i2cAddress);
        configComplete &= getJsonValue(top[F("CheckInterval")], _checkInterval);
        if (configComplete)
        {
            if (1 <= _checkInterval && _checkInterval <= 600)
                _checkInterval *= 1000;
            else
                _checkInterval = 60000;
        }

        configComplete &= getJsonValue(top[F("Decimals")], _decimalFactor);
        if (configComplete)
        {
            if (0 <= _decimalFactor && _decimalFactor <= 5)
                _decimalFactor = pow10f(_decimalFactor);
            else
                _decimalFactor = 100;
        }

        configComplete &= getJsonValue(top[F("ShuntResistor")], _shuntResistor);
        configComplete &= getJsonValue(top[F("CurrentRange")], _currentRange);

#ifndef WLED_DISABLE_MQTT
        configComplete &= getJsonValue(top[F("MqttPublish")], tmpBool);
        if (configComplete)
            _mqttPublish = tmpBool;

        configComplete &= getJsonValue(top[F("MqttPublishAlways")], tmpBool);
        if (configComplete)
            _mqttPublishAlways = tmpBool;

        configComplete &= getJsonValue(top[F("MqttHomeAssistantDiscovery")], tmpBool);
        if (configComplete)
            _mqttHomeAssistant = tmpBool;
#endif

        if (_initDone)
        {
            initializeINA226();

#ifndef WLED_DISABLE_MQTT
            mqttInitialize();
#endif
        }

        _initDone = true;
        return configComplete;
    }
};

const char UsermodINA226::_name[] PROGMEM = "INA226";
