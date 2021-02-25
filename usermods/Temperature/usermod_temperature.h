#pragma once

#include "wled.h"

#include <DallasTemperature.h> //DS18B20

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
    OneWire *oneWire;
    DallasTemperature *sensor;
    // The device's unique 64-bit serial code stored in on-board ROM.
    // Reading directly from the sensor device address is faster than
    // reading from index. When reading by index, DallasTemperature
    // must first look up the device address at the specified index.
    DeviceAddress sensorDeviceAddress;
    // GPIO pin used for sensor (with a default compile-time fallback)
    int8_t temperaturePin = TEMPERATURE_PIN;
    // measurement unit (true==°C, false==°F)
    bool degC = true;
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

    void requestTemperatures() {
        // there is requestTemperaturesByAddress however it
        // appears to do more work,
        // TODO: measure exection time difference
        sensor->requestTemperatures();
        lastTemperaturesRequest = millis();
        waitingForConversion = true;
    }

    void getTemperature() {
      if (strip.isUpdating()) return;

      if (degC) temperature = sensor->getTempC(sensorDeviceAddress);
      else      temperature = sensor->getTempF(sensorDeviceAddress);

      lastMeasurement = millis();
      waitingForConversion = false;
      getTemperatureComplete = true;
    }

  public:


    void setup() {

      // pin retrieved from cfg.json (readFromConfig()) prior to running setup()
      if (!pinManager.allocatePin(temperaturePin,false)) {
        temperaturePin = -1;  // allocation failed
        DEBUG_PRINTLN(F("Temperature pin allocation failed."));
      } else {
        oneWire = new OneWire(temperaturePin);
        sensor  = new DallasTemperature(oneWire);
        if (sensor) sensor->begin();
        else
          DEBUG_PRINTLN(F("Temperature sensor allocation failed."));
      }

      // get the unique 64-bit serial code stored in on-board ROM
      // if getAddress returns false, the sensor was not found
      disabled = (temperaturePin==-1) || !sensor->getAddress(sensorDeviceAddress, 0);

      if (!disabled) {
        DEBUG_PRINTLN(F("Dallas Temperature found"));
        // set the resolution for this specific device
        sensor->setResolution(sensorDeviceAddress, 9, true);
        // do not block waiting for reading
        sensor->setWaitForConversion(false);
      } else {
        DEBUG_PRINTLN(F("Dallas Temperature not found"));
      }
    }

    void loop() {
      if (disabled || strip.isUpdating()) return;

      unsigned long now = millis();

      // check to see if we are due for taking a measurement
      // lastMeasurement will not be updated until the conversion
      // is complete the the reading is finished
      if (now - lastMeasurement < USERMOD_DALLASTEMPERATURE_MEASUREMENT_INTERVAL) return;

      // we are due for a measurement, if we are not already waiting
      // for a conversion to complete, then make a new request for temps
      if (!waitingForConversion) {
        requestTemperatures();
        return;
      }

      // we were waiting for a conversion to complete, have we waited log enough?
      if (now - lastTemperaturesRequest >= 94 /* 93.75ms per the datasheet */) {
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

    void addToJsonInfo(JsonObject& root) {
      // dont add temperature to info if we are disabled
      if (disabled) return;

      JsonObject user = root[F("u")];
      if (user.isNull()) user = root.createNestedObject(F("u"));

      JsonArray temp = user.createNestedArray(F("Temperature"));

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
     * Add "pin_Temperature" to json state. This can be used to check which GPIO pin usermod uses.
     */
    void addToJsonState(JsonObject &root)
    {
      root[F("pin_Temperature")] = temperaturePin;
      root[F("mode_Temperature")] = degC ? ("C") : ("F");
    }

    /**
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     * Read "pin_Temperature" from json state and and change GPIO pin used.
     */
    void readFromJsonState(JsonObject &root) {
      if (root[F("pin_Temperature")] != nullptr) {
        int8_t pin = (int)root[F("pin_Temperature")];
        // deallocate pin and release memory
        pinManager.deallocatePin(temperaturePin);
        delete sensor;
        delete oneWire;
        // disable usermod
        temperaturePin = -1;
        disabled = true;
        // check if pin is OK
        if (pin>=0 && pinManager.allocatePin(pin,false)) {
          // allocat memory
          oneWire = new OneWire(pin);
          sensor  = new DallasTemperature(oneWire);
          if (sensor) {
            temperaturePin = pin;
            sensor->begin();
            disabled = !sensor->getAddress(sensorDeviceAddress, 0);
          } else {
            pinManager.deallocatePin(pin);
          }
        }
      }
      if (root[F("mode_Temperature")] != nullptr) {
        degC = (root[F("mode_Temperature")]==String(PSTR("C")));
      }
    }

    /**
     * addToConfig() (called from set.cpp) stores persistent properties to cfg.json
     */
    void addToConfig(JsonObject &root) {
      // we add JSON object: {"Temperature": {"pin": 0, "degC": true}}
      JsonObject top = root.createNestedObject(F("Temperature"));
      top[F("pin")]  = temperaturePin;
      top[F("degC")] = degC;
    }

    /**
     * readFromConfig() is called before setup() to populate properties from values stored in cfg.json
     */
    void readFromConfig(JsonObject &root) {
      // we look for JSON object: {"Temperature": {"pin": 0, "degC": true}}
      JsonObject top = root[F("Temperature")];
      if (!top.isNull() && top[F("pin")] != nullptr) {
        temperaturePin = (int)top[F("pin")];
        degC = top[F("degC")] != nullptr ? top[F("degC")] : true;
      } else {
        DEBUG_PRINTLN(F("No Temperature sensor config found. (Using defaults.)"));
      }
    }

    uint16_t getId()
    {
      return USERMOD_ID_TEMPERATURE;
    }
};
