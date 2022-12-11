#pragma once

#include "SHT85.h"


class ShtUsermod : public Usermod
{
  private:
    bool enabled = false;
    bool firstRunDone = false;
    bool initDone = false;
    bool haMqttDiscovery = false;
    SHT *shtTempHumidSensor;

    // SHT vars
    bool shtInitDone = false;
    bool shtReadDataSuccess = false;
    byte shtI2cAddress = 0x44;
    unsigned long shtLastTimeUpdated = 0;
    bool shtDataRequested = false;
    float shtCurrentTemp = 0;
    float shtLastKnownTemp = 0;
    float shtCurrentHumidity = 0;
    float shtLastKnownHumidity = 0;


    void initShtTempHumiditySensor()
    {
      PinManagerPinType pins[2] = { { i2c_sda, true }, { i2c_scl, true } };
      if (!pinManager.allocateMultiplePins(pins, 2, PinOwner::HW_I2C)) {
        DEBUG_PRINTF("[%s] SHT pin allocation failed!\n", _name);
        shtInitDone = false;
        cleanupShtTempHumiditySensor();
        cleanup();
        return;
      }

      #ifdef USERMOD_SHT_TYPE_SHT31
        shtTempHumidSensor = (SHT *) new SHT31();
      #else
        #ifdef USERMOD_SHT_TYPE_SHT35
          shtTempHumidSensor = (SHT *) new SHT35();
        #else
          #ifdef USERMOD_SHT_TYPE_SHT85
            shtTempHumidSensor = (SHT *) new SHT85();
          #else
            shtTempHumidSensor = (SHT *) new SHT30();
          #endif
        #endif
      #endif

      shtTempHumidSensor->begin(shtI2cAddress, i2c_sda, i2c_scl);
      if (shtTempHumidSensor->readStatus() == 0xFFFF) {
        DEBUG_PRINTF("[%s] SHT init failed!\n", _name);
        shtInitDone = false;
        cleanupShtTempHumiditySensor();
        cleanup();
        return;
      }

      shtInitDone = true;
    }

    void cleanupShtTempHumiditySensor()
    {
      if (shtInitDone) {
        shtTempHumidSensor->reset();
      }

      pinManager.deallocatePin(i2c_sda, PinOwner::HW_I2C);
      pinManager.deallocatePin(i2c_scl, PinOwner::HW_I2C);

      delete shtTempHumidSensor;

      shtInitDone = false;
    }

    void cleanup()
    {
      if (isShtReady()) {
        cleanupShtTempHumiditySensor();
      }

      enabled = false;
    }

    bool isShtReady()
    {
      return shtInitDone;
    }


  public:
    // strings to reduce flash memory usage (used more than twice)
    static const char _name[];
    static const char _enabled[];
    static const char _haMqttDiscovery[];

    /*
      * setup() is called once at boot. WiFi is not yet connected at this point.
      * You can use it to initialize variables, sensors or similar.
      */
    void setup()
    {
      if (enabled) {
        initShtTempHumiditySensor();

        initDone = true;
      }

      firstRunDone = true;
    }

    /*
      * loop() is called continuously. Here you can check for events, read sensors, etc.
      *
      * Tips:
      * 1. You can use "if (WLED_CONNECTED)" to check for a successful network connection.
      *    Additionally, "if (WLED_MQTT_CONNECTED)" is available to check for a connection to an MQTT broker.
      *
      * 2. Try to avoid using the delay() function. NEVER use delays longer than 10 milliseconds.
      *    Instead, use a timer check as shown here.
      */
    void loop()
    {
      if (!enabled || !initDone || strip.isUpdating()) return;

      if (isShtReady()) {
        if (millis() - shtLastTimeUpdated > 30000 && !shtDataRequested) {
          shtTempHumidSensor->requestData();
          shtDataRequested = true;

          shtLastTimeUpdated = millis();
        }

        if (shtDataRequested) {
          if (shtTempHumidSensor->dataReady()) {
            if (shtTempHumidSensor->readData(false)) {
              shtCurrentTemp = shtTempHumidSensor->getTemperature();
              shtCurrentHumidity = shtTempHumidSensor->getHumidity();

              publishTemperatureAndHumidityViaMqtt();
              shtReadDataSuccess = true;
            }
            else {
              shtReadDataSuccess = false;
            }

            shtDataRequested = false;
          }
        }
      }
    }

    void onMqttConnect(bool sessionPresent) {
      if (haMqttDiscovery) publishHomeAssistantAutodiscovery();
    }

    void addToConfig(JsonObject &root)
    {
      JsonObject top = root.createNestedObject(FPSTR(_name)); // usermodname

      top[FPSTR(_enabled)] = enabled;
      top[FPSTR(_haMqttDiscovery)] = haMqttDiscovery;
    }

    /**
     * readFromConfig() is called before setup() to populate properties from values stored in cfg.json
     *
     * The function should return true if configuration was successfully loaded or false if there was no configuration.
     */
    bool readFromConfig(JsonObject &root)
    {
      JsonObject top = root[FPSTR(_name)];
      if (top.isNull()) {
        DEBUG_PRINTF("[%s] No config found. (Using defaults.)\n", _name);
        return false;
      }

      bool oldEnabled = enabled;
      bool oldHaMqttDiscovery = haMqttDiscovery;
      
      getJsonValue(top[FPSTR(_enabled)], enabled);
      getJsonValue(top[FPSTR(_haMqttDiscovery)], haMqttDiscovery);

      // First run: reading from cfg.json, nothing to do here, will be all done in setup()
      if (!firstRunDone) {
        DEBUG_PRINTF("[%s] First run, nothing to do\n", _name);
      }
      // Check if mod has been en-/disabled
      else if (enabled != oldEnabled) {
        enabled ? setup() : cleanup();
        DEBUG_PRINTF("[%s] Usermod has been en-/disabled\n", _name);
      }
      // Config has been changed, so adopt to changes
      else if (enabled) {
        if (oldHaMqttDiscovery != haMqttDiscovery && haMqttDiscovery) {
          publishHomeAssistantAutodiscovery();
        }

        DEBUG_PRINTF("[%s] Config (re)loaded\n", _name);
      }

      return true;
    }

    void addToJsonInfo(JsonObject& root)
    {
      if (!enabled && !isShtReady()) {
        return;
      }

      JsonObject user = root["u"];
      if (user.isNull()) user = root.createNestedObject("u");

      JsonArray jsonTemp = user.createNestedArray("Temperature");
      JsonArray jsonHumidity = user.createNestedArray("Humidity");

      if (shtLastTimeUpdated == 0 || !shtReadDataSuccess) {
        jsonTemp.add(0);
        jsonHumidity.add(0);
        if (shtLastTimeUpdated == 0) {
          jsonTemp.add(" Not read yet");
          jsonHumidity.add(" Not read yet");
        }
        else {
          jsonTemp.add(" Error");
          jsonHumidity.add(" Error");
        }

        return;
      }

      jsonHumidity.add(shtCurrentHumidity);
      jsonHumidity.add(" RH");

      jsonTemp.add(shtCurrentTemp);
      jsonTemp.add(" °C");
    }

    void publishTemperatureAndHumidityViaMqtt() {
      if (!WLED_MQTT_CONNECTED) return;
      char buf[128];

      sprintf_P(buf, PSTR("%s/temperature"), mqttDeviceTopic);
      mqtt->publish(buf, 0, false, String(shtCurrentTemp).c_str());
      sprintf_P(buf, PSTR("%s/humidity"), mqttDeviceTopic);
      mqtt->publish(buf, 0, false, String(shtCurrentHumidity).c_str());
    }

    void publishHomeAssistantAutodiscovery() {
      if (!WLED_MQTT_CONNECTED) return;

      char json_str[1024], buf[128];
      size_t payload_size;
      StaticJsonDocument<1024> json;

      sprintf_P(buf, PSTR("%s Temperature"), serverDescription);
      json[F("name")] = buf;
      sprintf_P(buf, PSTR("%s/temperature"), mqttDeviceTopic);
      json[F("stat_t")] = buf;
      json[F("dev_cla")] = F("temperature");
      json[F("stat_cla")] = F("measurement");
      sprintf_P(buf, PSTR("%s-temperature"), escapedMac.c_str());
      json[F("uniq_id")] = buf;
      json[F("unit_of_meas")] = F("°C");
      appendDeviceToMqttDiscoveryMessage(json);
      payload_size = serializeJson(json, json_str);
      sprintf_P(buf, PSTR("homeassistant/sensor/%s/%s-temperature/config"), escapedMac.c_str(), escapedMac.c_str());
      mqtt->publish(buf, 0, true, json_str, payload_size);

      json.clear();

      sprintf_P(buf, PSTR("%s Humidity"), serverDescription);
      json[F("name")] = buf;
      sprintf_P(buf, PSTR("%s/humidity"), mqttDeviceTopic);
      json[F("stat_t")] = buf;
      json[F("dev_cla")] = F("humidity");
      json[F("stat_cla")] = F("measurement");
      sprintf_P(buf, PSTR("%s-humidity"), escapedMac.c_str());
      json[F("uniq_id")] = buf;
      json[F("unit_of_meas")] = F("%");
      appendDeviceToMqttDiscoveryMessage(json);
      payload_size = serializeJson(json, json_str);
      sprintf_P(buf, PSTR("homeassistant/sensor/%s/%s-humidity/config"), escapedMac.c_str(), escapedMac.c_str());
      mqtt->publish(buf, 0, true, json_str, payload_size);
    }

    void appendDeviceToMqttDiscoveryMessage(JsonDocument& root) {
      JsonObject device = root.createNestedObject("dev");
      device["ids"] = escapedMac.c_str();
      device["name"] = serverDescription;
      device["sw"] = versionString;
      device["mdl"] = ESP.getChipModel();
      device["mf"] = "espressif";
    }

    /*
      * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
      * This could be used in the future for the system to determine whether your usermod is installed.
      */
    uint16_t getId()
    {
      return USERMOD_ID_SHT;
    }
};

// strings to reduce flash memory usage (used more than twice)
// Config settings
const char ShtUsermod::_name[]    PROGMEM = "SHT Temperature & Humidity Sensor";
const char ShtUsermod::_enabled[] PROGMEM = "Enabled";
const char ShtUsermod::_haMqttDiscovery[] PROGMEM = "Add-To-Home-Assistant-MQTT-Discovery";