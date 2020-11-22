#pragma once

#include "wled.h"

#include <DallasTemperature.h> //DS18B20

//Pin defaults for QuinLed Dig-Uno
#ifdef ARDUINO_ARCH_ESP32
#define TEMPERATURE_PIN 18
#else //ESP8266 boards
#define TEMPERATURE_PIN 14
#endif

// the frequency to check temperature, 1 minute
#ifndef USERMOD_DALLASTEMPERATURE_MEASUREMENT_INTERVAL
#define USERMOD_DALLASTEMPERATURE_MEASUREMENT_INTERVAL 60000
#endif

// how many seconds after boot to take first measurement, 20 seconds
#ifndef USERMOD_DALLASTEMPERATURE_FIRST_MEASUREMENT_AT
#define USERMOD_DALLASTEMPERATURE_FIRST_MEASUREMENT_AT 20000 
#endif

OneWire oneWire(TEMPERATURE_PIN);
DallasTemperature sensor(&oneWire);

class UsermodTemperature : public Usermod {
  private:
    // The device's unique 64-bit serial code stored in on-board ROM.
    // Reading directly from the sensor device address is faster than
    // reading from index. When reading by index, DallasTemperature
    // must first look up the device address at the specified index.
    DeviceAddress sensorDeviceAddress;
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
        sensor.requestTemperatures(); 
        lastTemperaturesRequest = millis();
        waitingForConversion = true;
    }

    void getTemperature() {
      #ifdef USERMOD_DALLASTEMPERATURE_CELSIUS
      temperature = sensor.getTempC(sensorDeviceAddress);
      #else
      temperature = sensor.getTempF(sensorDeviceAddress);
      #endif

      lastMeasurement = millis();
      waitingForConversion = false;
      getTemperatureComplete = true;
    }

  public:


    void setup() {
      sensor.begin();

      // get the unique 64-bit serial code stored in on-board ROM
      // if getAddress returns false, the sensor was not found
      disabled = !sensor.getAddress(sensorDeviceAddress, 0);

      if (!disabled) {
        DEBUG_PRINTLN("Dallas Temperature found");
        // set the resolution for this specific device
        sensor.setResolution(sensorDeviceAddress, 9, true);
        // do not block waiting for reading
        sensor.setWaitForConversion(false); 
      } else {
        DEBUG_PRINTLN("Dallas Temperature not found");
      }
    }

    void loop() {
      if (disabled) {
        return;
      }
      
      unsigned long now = millis();

      // check to see if we are due for taking a measurement
      // lastMeasurement will not be updated until the conversion
      // is complete the the reading is finished
      if (now - lastMeasurement < USERMOD_DALLASTEMPERATURE_MEASUREMENT_INTERVAL)
      {
        return;
      }

      // we are due for a measurement, if we are not already waiting 
      // for a conversion to complete, then make a new request for temps
      if (!waitingForConversion)
      {
        requestTemperatures();
        return;
      }

      // we were waiting for a conversion to complete, have we waited log enough?
      if (now - lastTemperaturesRequest >= 94 /* 93.75ms per the datasheet */)
      {
        getTemperature();
 
        if (WLED_MQTT_CONNECTED) {
          char subuf[38];
          strcpy(subuf, mqttDeviceTopic);
          if (-100 <= temperature) {
            // dont publish super low temperature as the graph will get messed up
            // the DallasTemperature library returns -127C or -196.6F when problem
            // reading the sensor
            strcat(subuf, "/temperature");
            mqtt->publish(subuf, 0, true, String(temperature).c_str());
          } else {
            // publish something else to indicate status?
          }
        }
      }
    }

    void addToJsonInfo(JsonObject& root) {
      // dont add temperature to info if we are disabled
      if (disabled) {
        return;
      }

      JsonObject user = root["u"];
      if (user.isNull()) user = root.createNestedObject("u");

      JsonArray temp = user.createNestedArray("Temperature");

      if (!getTemperatureComplete) {
        // if we haven't read the sensor yet, let the user know
        // that we are still waiting for the first measurement
        temp.add((USERMOD_DALLASTEMPERATURE_FIRST_MEASUREMENT_AT - millis()) / 1000);
        temp.add(" sec until read");
        return;
      }

      if (temperature <= -100) {
        temp.add(0);
        temp.add(" Sensor Error!");
        return;
      }

      temp.add(temperature);
      #ifdef USERMOD_DALLASTEMPERATURE_CELSIUS
      temp.add("°C");
      #else
      temp.add("°F");
      #endif
    }

    uint16_t getId()
    {
      return USERMOD_ID_TEMPERATURE;
    }
};
