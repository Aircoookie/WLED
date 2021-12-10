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

#define DS18S20_CHIPID       0x10  // +/-0.5C 9-bit
#define DS1822_CHIPID        0x22  // +/-2C 12-bit
#define DS18B20_CHIPID       0x28  // +/-0.5C 12-bit
#define MAX31850_CHIPID      0x3B  // +/-0.25C 14-bit

#define W1_SKIP_ROM          0xCC
#define W1_CONVERT_TEMP      0x44
#define W1_READ_SCRATCHPAD   0xBE

#define DS18X20_MAX_SENSORS  8


// the frequency to check temperature, 1 minute
#ifndef USERMOD_DALLASTEMPERATURE_MEASUREMENT_INTERVAL
#define USERMOD_DALLASTEMPERATURE_MEASUREMENT_INTERVAL 60000
#endif

const char kDs18x20Types[] PROGMEM = "DS18x20|DS18S20|DS1822|DS18B20|MAX31850";

uint8_t ds18x20_chipids[] = { 0, DS18S20_CHIPID, DS1822_CHIPID, DS18B20_CHIPID, MAX31850_CHIPID };

struct {
  float temp;
  uint16_t numread;
  uint8_t address[8];
  uint8_t index;
  uint8_t valid;
} ds18x20_sensor[DS18X20_MAX_SENSORS];

struct {
  char name[17];
  uint8_t sensors = 0;
} DS18X20Data;


class UsermodTemperature : public Usermod {

  private:

    bool initDone = false;
    OneWire *ds = nullptr;
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

    void Ds18x20Init(void) {
      ds = new OneWire(temperaturePin);

      Ds18x20Search();
    }
    
    void Ds18x20Search(void) {
      uint8_t num_sensors=0;

      ds->reset_search();
      for (num_sensors = 0; num_sensors < DS18X20_MAX_SENSORS;) {
        if (!ds->search(ds18x20_sensor[num_sensors].address)) {
          ds->reset_search();
          break;
        }
        // If CRC Ok and Type DS18S20, DS1822, DS18B20 or MAX31850
        if ((OneWire::crc8(ds18x20_sensor[num_sensors].address, 7) == ds18x20_sensor[num_sensors].address[7]) &&
           ((ds18x20_sensor[num_sensors].address[0] == DS18S20_CHIPID) ||
            (ds18x20_sensor[num_sensors].address[0] == DS1822_CHIPID) ||
            (ds18x20_sensor[num_sensors].address[0] == DS18B20_CHIPID) ||
            (ds18x20_sensor[num_sensors].address[0] == MAX31850_CHIPID))) {
          num_sensors++;
        }
      }
      for (uint32_t i = 0; i < num_sensors; i++) {
        ds18x20_sensor[i].index = i;
      }
      for (uint32_t i = 0; i < num_sensors; i++) {
        for (uint32_t j = i + 1; j < num_sensors; j++) {
          if (uint32_t(ds18x20_sensor[ds18x20_sensor[i].index].address) > uint32_t(ds18x20_sensor[ds18x20_sensor[j].index].address)) {
            std::swap(ds18x20_sensor[i].index, ds18x20_sensor[j].index);
          }
        }
      }
      DS18X20Data.sensors = num_sensors;
    }
    
    bool Ds18x20Read(uint8_t sensor, float &t) {
      uint8_t data[12];
      int8_t sign = 1;

      t = NAN;

      uint8_t index = ds18x20_sensor[sensor].index;
      if (ds18x20_sensor[index].valid) { ds18x20_sensor[index].valid--; }

      ds->reset();
      ds->select(ds18x20_sensor[index].address);
      ds->write(W1_READ_SCRATCHPAD); // Read Scratchpad

      for (uint32_t i = 0; i < 9; i++) {
        data[i] = ds->read();
      }
      if (OneWire::crc8(data, 8) == data[8]) {
        switch(ds18x20_sensor[index].address[0]) {
          case DS18S20_CHIPID: {
            int16_t tempS = (((data[1] << 8) | (data[0] & 0xFE)) << 3) | ((0x10 - data[6]) & 0x0F);
            t = tempS * 0.0625 - 0.250;
            ds18x20_sensor[index].valid = 5;
            return true;
          }
          case DS1822_CHIPID:
          case DS18B20_CHIPID: {
            uint16_t temp12 = (data[1] << 8) + data[0];
            if (temp12 > 2047) {
              temp12 = (~temp12) +1;
              sign = -1;
            }
            t = sign * temp12 * 0.0625;  // Divide by 16
            ds18x20_sensor[index].valid = 5;
            return true;
          }
          case MAX31850_CHIPID: {
            int16_t temp14 = (data[1] << 8) + (data[0] & 0xFC);
            t = temp14 * 0.0625;  // Divide by 16
            ds18x20_sensor[index].valid = 5;
            return true;
          }
        }
      }
      return false;
    }


    void Ds18x20Convert(void) {
      ds->reset();
      ds->write(W1_SKIP_ROM);        // Address all Sensors on Bus
      ds->write(W1_CONVERT_TEMP);    // start conversion, no parasite power on at the end
    //  delay(750);                   // 750ms should be enough for 12bit conv
      lastTemperaturesRequest = millis();
      waitingForConversion = true;
    }



  public:

    void setup() {
      if (enabled) {
        // config says we are enabled
        DEBUG_PRINTLN(F("Allocating temperature pin..."));
        // pin retrieved from cfg.json (readFromConfig()) prior to running setup()
        if (temperaturePin >= 0 && pinManager.allocatePin(temperaturePin, true, PinOwner::UM_Temperature)) {
          Ds18x20Init();
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
        Ds18x20Convert();
        return;
      }

      // we were waiting for a conversion to complete, have we waited log enough?
      if (now - lastTemperaturesRequest >= 750 /* 93.75ms per the datasheet but can be up to 750ms */) {
        float t;
        for (uint8_t i = 0; i < DS18X20Data.sensors; i++) {
          // 12mS per device
          if (Ds18x20Read(i, t)) {   // Read temperature
            ds18x20_sensor[i].temp = t;
            lastMeasurement = millis();
            waitingForConversion = false;
          }
        }

        if (WLED_MQTT_CONNECTED) {
          char subuf[64];
          String uiTempString;
          for (uint8_t i = 0; i < DS18X20Data.sensors; i++) {
            if (-100 <= ds18x20_sensor[i].temp) {
              uiTempString = F("/temperature");
              uiTempString += i;
              strcpy(subuf, mqttDeviceTopic);
              strcat(subuf, uiTempString.c_str());
              if (degC) {              
                mqtt->publish(subuf, 0, false, String(ds18x20_sensor[i].temp).c_str());
              } else {
                mqtt->publish(subuf, 0, false, String((float)ds18x20_sensor[i].temp * 1.8f + 32).c_str());
              } 
            }
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
        String uiTempString;
        for (uint8_t i = 0; i < DS18X20Data.sensors; i++) {
          uiTempString = F("Temp ");
          uiTempString += i;
          JsonArray temp = user.createNestedArray(uiTempString); // Temperature number
          temp.add(degC ? ds18x20_sensor[i].temp : (float)ds18x20_sensor[i].temp * 1.8f + 32);
          if (degC) 
            temp.add(F("°C"));
          else
            temp.add(F("°F"));
      }

     
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
          delete ds;
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
