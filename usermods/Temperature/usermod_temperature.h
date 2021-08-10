#pragma once

#include "wled.h"
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

class UsermodTemperature : public Usermod {

  private:

    bool initDone = false;
    OneWire *oneWire;
    // GPIO pin used for sensor (with a default compile-time fallback)
    int8_t temperaturePin = TEMPERATURE_PIN;
    // measurement unit (true==°C, false==°F)
    bool degC = true;
    // using parasite power on the sensor
    bool parasite = false;
    // how often do we read from sensor?
    unsigned long readingInterval = USERMOD_DALLASTEMPERATURE_MEASUREMENT_INTERVAL;
    // set last reading as "40 sec before boot", so first reading is taken after 20 sec
    unsigned long lastMeasurement = UINT32_MAX - USERMOD_DALLASTEMPERATURE_MEASUREMENT_INTERVAL;
    // last time requestTemperatures was called
    // used to determine when we can read the sensors temperature
    // we have to wait at least 93.75 ms after requestTemperatures() is called
    unsigned long lastTemperaturesRequest;
    float temperature = -100; // default to -100, DS18B20 only goes down to -50C
    // indicates requestTemperatures has been called but the sensor measurement is not complete
    bool waitingForConversion = false;
    // flag set at startup if DS18B20 sensor not found, avoids trying to keep getting
    // temperature if flashed to a board without a sensor attached
    bool sensorFound = false;

    bool enabled = true;

    // strings to reduce flash memory usage (used more than twice)
    static const char _name[];
    static const char _enabled[];
    static const char _readInterval[];
    static const char _parasite[];

    //Dallas sensor quick (& dirty) reading. Credit to - Author: Peter Scargill, August 17th, 2013
    float readDallas() {
      byte i;
      byte data[2];
      int16_t result;                         // raw data from sensor
      if (!oneWire->reset()) return -127.0f;  // send reset command and fail fast
      oneWire->skip();                        // skip ROM
      oneWire->write(0xBE);                   // read (temperature) from EEPROM
      for (i=0; i < 2; i++) data[i] = oneWire->read();  // first 2 bytes contain temperature
      for (i=2; i < 8; i++) oneWire->read();  // read unused bytes  
      result = (data[1]<<4) | (data[0]>>4);   // we only need whole part, we will add fraction when returning
      if (data[1]&0x80) result |= 0xFF00;     // fix negative value
      oneWire->reset();
      oneWire->skip();                        // skip ROM
      oneWire->write(0x44,parasite);          // request new temperature reading (without parasite power)
      return (float)result + ((data[0]&0x0008) ? 0.5f : 0.0f);
    }

    void requestTemperatures() {
      readDallas();
      lastTemperaturesRequest = millis();
      waitingForConversion = true;
      DEBUG_PRINTLN(F("Requested temperature."));
    }

    void readTemperature() {
      temperature = readDallas();
      lastMeasurement = millis();
      waitingForConversion = false;
      DEBUG_PRINTF("Read temperature %2.1f.\n", temperature);
    }

    bool findSensor() {
      DEBUG_PRINTLN(F("Searching for sensor..."));
      uint8_t deviceAddress[8] = {0,0,0,0,0,0,0,0};
      // find out if we have DS18xxx sensor attached
      oneWire->reset_search();
      delay(10);
      while (oneWire->search(deviceAddress)) {
        DEBUG_PRINTLN(F("Found something..."));
        if (oneWire->crc8(deviceAddress, 7) == deviceAddress[7]) {
          switch (deviceAddress[0]) {
            case 0x10:  // DS18S20
            case 0x22:  // DS18B20
            case 0x28:  // DS1822
            case 0x3B:  // DS1825
            case 0x42:  // DS28EA00
              DEBUG_PRINTLN(F("Sensor found."));
              return true;
          }
        }
      }
      return false;
    }

  public:

    void setup() {
      int retries = 10;
      if (enabled) {
        // config says we are enabled
        DEBUG_PRINTLN(F("Allocating temperature pin..."));
        // pin retrieved from cfg.json (readFromConfig()) prior to running setup()
        if (temperaturePin >= 0 && pinManager.allocatePin(temperaturePin, true, PinOwner::UM_Temperature)) {
          oneWire = new OneWire(temperaturePin);
          if (!oneWire->reset()) {
            sensorFound = false;   // resetting 1-Wire bus yielded an error
          } else {
            while ((sensorFound=findSensor()) && retries--) {
              delay(25); // try to find sensor
            }
          }
        } else {
          if (temperaturePin >= 0) {
            DEBUG_PRINTLN(F("Temperature pin allocation failed."));
          }
          temperaturePin = -1;  // allocation failed
          sensorFound = false;
        }
      }
      lastMeasurement = millis() - readingInterval + 10000;
      initDone = true;
    }

    void loop() {
      if (!enabled || strip.isUpdating()) return;

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
      if (now - lastTemperaturesRequest >= 100 /* 93.75ms per the datasheet but can be up to 750ms */) {
        readTemperature();

        if (WLED_MQTT_CONNECTED) {
          char subuf[64];
          strcpy(subuf, mqttDeviceTopic);
          if (-100 <= temperature) {
            // dont publish super low temperature as the graph will get messed up
            // the DallasTemperature library returns -127C or -196.6F when problem
            // reading the sensor
            strcat_P(subuf, PSTR("/temperature"));
            mqtt->publish(subuf, 0, false, String(temperature).c_str());
            strcat_P(subuf, PSTR("_f"));
            mqtt->publish(subuf, 0, false, String((float)temperature * 1.8f + 32).c_str());
          } else {
            // publish something else to indicate status?
          }
        }
      }
    }

    /*
     * API calls te enable data exchange between WLED modules
     */
    inline float getTemperatureC() {
      return (float)temperature;
    }
    inline float getTemperatureF() {
      return (float)temperature * 1.8f + 32;
    }

    /*
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
     * Below it is shown how this could be used for e.g. a light sensor
     */
    void addToJsonInfo(JsonObject& root) {
      // dont add temperature to info if we are disabled
      if (!enabled) return;

      JsonObject user = root["u"];
      if (user.isNull()) user = root.createNestedObject("u");

      JsonArray temp = user.createNestedArray(FPSTR(_name));
      //temp.add(F("Loaded."));

      if (temperature <= -100.0 || (!sensorFound && temperature == -1.0)) {
        temp.add(0);
        temp.add(F(" Sensor Error!"));
        return;
      }

      temp.add(degC ? temperature : (float)temperature * 1.8f + 32);
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
      top[FPSTR(_enabled)] = enabled;
      top["pin"]  = temperaturePin;     // usermodparam
      top["degC"] = degC;  // usermodparam
      top[FPSTR(_readInterval)] = readingInterval / 1000;
      top[FPSTR(_parasite)] = parasite;
      DEBUG_PRINTLN(F("Temperature config saved."));
    }

    /**
     * readFromConfig() is called before setup() to populate properties from values stored in cfg.json
     *
     * The function should return true if configuration was successfully loaded or false if there was no configuration.
     */
    bool readFromConfig(JsonObject &root) {
      // we look for JSON object: {"Temperature": {"pin": 0, "degC": true}}
      int8_t newTemperaturePin = temperaturePin;

      JsonObject top = root[FPSTR(_name)];
      if (top.isNull()) {
        DEBUG_PRINT(FPSTR(_name));
        DEBUG_PRINTLN(F(": No config found. (Using defaults.)"));
        return false;
      }

      enabled           = top[FPSTR(_enabled)] | enabled;
      newTemperaturePin = top["pin"] | newTemperaturePin;
//      newTemperaturePin = min(33,max(-1,(int)newTemperaturePin)); // bounds check
      degC              = top["degC"] | degC;
      readingInterval   = top[FPSTR(_readInterval)] | readingInterval/1000;
      readingInterval   = min(120,max(10,(int)readingInterval)) * 1000;  // convert to ms
      parasite          = top[FPSTR(_parasite)] | parasite;

      DEBUG_PRINT(FPSTR(_name));
      if (!initDone) {
        // first run: reading from cfg.json
        temperaturePin = newTemperaturePin;
        DEBUG_PRINTLN(F(" config loaded."));
      } else {
        DEBUG_PRINTLN(F(" config (re)loaded."));
        // changing paramters from settings page
        if (newTemperaturePin != temperaturePin) {
          DEBUG_PRINTLN(F("Re-init temperature."));
          // deallocate pin and release memory
          delete oneWire;
          pinManager.deallocatePin(temperaturePin, PinOwner::UM_Temperature);
          temperaturePin = newTemperaturePin;
          // initialise
          setup();
        }
      }
      // use "return !top["newestParameter"].isNull();" when updating Usermod with new features
      return !top[FPSTR(_parasite)].isNull();
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
const char UsermodTemperature::_parasite[]     PROGMEM = "parasite-pwr";
