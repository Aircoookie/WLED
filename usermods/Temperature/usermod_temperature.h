#pragma once

#include "wled.h"

//#include <DallasTemperature.h> //DS18B20
#include "OneWire.h"

//Pin defaults for QuinLed Dig-Uno if not overriden
#ifndef TEMPERATURE_PIN
  #ifdef ARDUINO_ARCH_ESP32
    #define TEMPERATURE_PIN 18
  #else //ESP8266 boards
    #define TEMPERATURE_PIN 14
  #endif
#endif

// the frequency to check temperature, 1 minute
#ifndef USERMOD_DALLASTEMPERATURE_MEASUREMENT_INTERVAL
#define USERMOD_DALLASTEMPERATURE_MEASUREMENT_INTERVAL 60000
#endif

// how many seconds after boot to take first measurement, 20 seconds
#ifndef USERMOD_DALLASTEMPERATURE_FIRST_MEASUREMENT_AT
#define USERMOD_DALLASTEMPERATURE_FIRST_MEASUREMENT_AT 20000
#endif

class UsermodTemperature : public Usermod {

  private:

    bool initDone = false;
    OneWire *oneWire;
    // GPIO pin used for sensor (with a default compile-time fallback)
    int8_t temperaturePin = TEMPERATURE_PIN;
    // measurement unit (true==°C, false==°F)
    bool degC = true;
    unsigned long readingInterval = USERMOD_DALLASTEMPERATURE_MEASUREMENT_INTERVAL;
    // set last reading as "40 sec before boot", so first reading is taken after 20 sec
    unsigned long lastMeasurement = UINT32_MAX - (USERMOD_DALLASTEMPERATURE_MEASUREMENT_INTERVAL - USERMOD_DALLASTEMPERATURE_FIRST_MEASUREMENT_AT);
    // last time requestTemperatures was called
    // used to determine when we can read the sensors temperature
    // we have to wait at least 93.75 ms after requestTemperatures() is called
    unsigned long lastTemperaturesRequest;
    float temperature = -100; // default to -100, DS18B20 only goes down to -50C
    // indicates requestTemperatures has been called but the sensor measurement is not complete
    bool waitingForConversion = false;
    // flag to indicate we have finished the first getTemperature call
    // allows this library to report to the user how long until the first
    // measurement
    bool getTemperatureComplete = false;
    // flag set at startup if DS18B20 sensor not found, avoids trying to keep getting
    // temperature if flashed to a board without a sensor attached
    bool disabled = false;

    // strings to reduce flash memory usage (used more than twice)
    static const char _name[];
    static const char _enabled[];
    static const char _readInterval[];

    //Dallas sensor quick reading. Credit to - Author: Peter Scargill, August 17th, 2013
    int16_t readDallas() {
      byte i;
      byte data[2];
      int16_t result;
      oneWire->reset();
      oneWire->write(0xCC);
      oneWire->write(0xBE);
      for (i=0; i < 2; i++) data[i] = oneWire->read();
      result = (data[1]<<8) | data[0];
      result >>= 4;
      if (data[1]&0x80) result |= 61440;
      if (data[0]&0x08) ++result;
      oneWire->reset();
      oneWire->write(0xCC);
      oneWire->write(0x44,1);
      return result*10;
    }

    void requestTemperatures() {
      readDallas();
      lastTemperaturesRequest = millis();
      waitingForConversion = true;
      DEBUG_PRINTLN(F("Requested temperature."));
    }

    void getTemperature() {
      temperature = readDallas()/10.0f;
      if (!degC) temperature = temperature * 1.8f + 32;
      lastMeasurement = millis();
      waitingForConversion = false;
      getTemperatureComplete = true;
      DEBUG_PRINTF("Read temperature %2.1f.\n", temperature);
    }

  public:

    void setup() {
      //bool sensorFound = false;

      // pin retrieved from cfg.json (readFromConfig()) prior to running setup()
      if (!pinManager.allocatePin(temperaturePin,false)) {
        temperaturePin = -1;  // allocation failed
        DEBUG_PRINTLN(F("Temperature pin allocation failed."));
      } else {
        //DeviceAddress deviceAddress;
        oneWire = new OneWire(temperaturePin);
        oneWire->reset();
/*
        // find out if we have DS18xxx sensor attached
        oneWire->reset_search();
        while (oneWire->search(deviceAddress)) {
          if (oneWire->crc8(deviceAddress, 7) == deviceAddress[7]) {
            switch (deviceAddress[0]) {
              case 0x10:  // DS18S20
              case 0x22:  // DS18B20
              case 0x28:  // DS1822
              case 0x3B:  // DS1825
              case 0x42:  // DS28EA00
                sensorFound = true; // sensor found;
                DEBUG_PRINTLN(F("Sensor found."));
                break;
            }
          }
        }
*/
      }
      disabled = disabled || (temperaturePin==-1);

      if (!disabled) {
        DEBUG_PRINTLN(F("Dallas Temperature found"));
      } else {
        DEBUG_PRINTLN(F("Dallas Temperature not found"));
      }
      initDone = true;
    }

    void loop() {
      if (disabled || strip.isUpdating()) return;

      unsigned long now = millis();

      // check to see if we are due for taking a measurement
      // lastMeasurement will not be updated until the conversion
      // is complete the the reading is finished
      if (now - lastMeasurement < readingInterval) return;

      // we are due for a measurement, if we are not already waiting
      // for a conversion to complete, then make a new request for temps
      if (!waitingForConversion) {
        requestTemperatures();
        return;
      }

      // we were waiting for a conversion to complete, have we waited log enough?
      if (now - lastTemperaturesRequest >= 800 /* 93.75ms per the datasheet but can be up to 750ms*/) {
        getTemperature();

        if (WLED_MQTT_CONNECTED) {
          char subuf[64];
          strcpy(subuf, mqttDeviceTopic);
          if (-100 <= temperature) {
            // dont publish super low temperature as the graph will get messed up
            // the DallasTemperature library returns -127C or -196.6F when problem
            // reading the sensor
            strcat_P(subuf, PSTR("/temperature"));
            mqtt->publish(subuf, 0, true, String(temperature).c_str());
          } else {
            // publish something else to indicate status?
          }
        }
      }
    }

    /*
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
     * Below it is shown how this could be used for e.g. a light sensor
     */
    void addToJsonInfo(JsonObject& root) {
      // dont add temperature to info if we are disabled
      if (disabled) return;

      JsonObject user = root["u"];
      if (user.isNull()) user = root.createNestedObject("u");

      JsonArray temp = user.createNestedArray(FPSTR(_name));
      //temp.add(F("Loaded."));

      if (!getTemperatureComplete) {
        // if we haven't read the sensor yet, let the user know
        // that we are still waiting for the first measurement
        temp.add((USERMOD_DALLASTEMPERATURE_FIRST_MEASUREMENT_AT - millis()) / 1000);
        temp.add(F(" sec until read"));
        return;
      }

      if (temperature <= -100) {
        temp.add(0);
        temp.add(F(" Sensor Error!"));
        return;
      }

      temp.add(temperature);
      if (degC) temp.add(F("°C"));
      else      temp.add(F("°F"));
    }

    /**
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    //void addToJsonState(JsonObject &root)
    //{
    //}

    /**
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     * Read "<usermodname>_<usermodparam>" from json state and and change settings (i.e. GPIO pin) used.
     */
    //void readFromJsonState(JsonObject &root) {
    //  if (!initDone) return;  // prevent crash on boot applyPreset()
    //}

    /**
     * addToConfig() (called from set.cpp) stores persistent properties to cfg.json
     */
    void addToConfig(JsonObject &root) {
      // we add JSON object: {"Temperature": {"pin": 0, "degC": true}}
      JsonObject top = root.createNestedObject(FPSTR(_name)); // usermodname
      top[FPSTR(_enabled)] = !disabled;
      top["pin"]  = temperaturePin;     // usermodparam
      top["degC"] = degC;  // usermodparam
      top[FPSTR(_readInterval)] = readingInterval / 1000;
      DEBUG_PRINTLN(F("Temperature config saved."));
    }

    /**
     * readFromConfig() is called before setup() to populate properties from values stored in cfg.json
     */
    void readFromConfig(JsonObject &root) {
      // we look for JSON object: {"Temperature": {"pin": 0, "degC": true}}
      JsonObject top = root[FPSTR(_name)];
      int8_t newTemperaturePin = temperaturePin;

      if (!top.isNull() && top["pin"] != nullptr) {
        if (top[FPSTR(_enabled)].is<bool>()) {
          disabled = !top[FPSTR(_enabled)].as<bool>();
        } else {
          String str = top[FPSTR(_enabled)]; // checkbox -> off or on
          disabled = (bool)(str=="off"); // off is guaranteed to be present
        }
        newTemperaturePin = min(39,max(-1,top["pin"].as<int>()));
        if (top["degC"].is<bool>()) {
          // reading from cfg.json
          degC = top["degC"].as<bool>();
        } else {
          // new configuration from set.cpp
          String str = top["degC"]; // checkbox -> off or on
          degC = (bool)(str!="off"); // off is guaranteed to be present
        }
        readingInterval = min(120,max(10,top[FPSTR(_readInterval)].as<int>())) * 1000;  // convert to ms
        DEBUG_PRINTLN(F("Temperature config (re)loaded."));
      } else {
        DEBUG_PRINTLN(F("No config found. (Using defaults.)"));
      }

      if (!initDone) {
        // first run: reading from cfg.json
        temperaturePin = newTemperaturePin;
      } else {
        // changing paramters from settings page
        if (newTemperaturePin != temperaturePin) {
          // deallocate pin and release memory
          delete oneWire;
          pinManager.deallocatePin(temperaturePin);
          temperaturePin = newTemperaturePin;
          // initialise
          setup();
        }
      }
    }

    uint16_t getId()
    {
      return USERMOD_ID_TEMPERATURE;
    }
};

// strings to reduce flash memory usage (used more than twice)
const char UsermodTemperature::_name[]         PROGMEM = "Temperature";
const char UsermodTemperature::_enabled[]      PROGMEM = "enabled";
const char UsermodTemperature::_readInterval[] PROGMEM = "read-interval-s";