#pragma once

#include "wled.h"

#include <DHT.h>

// USERMOD_DHT_DHTTYPE:
//   DHT11   // DHT 11
//   DHT22   // DHT 22  (AM2302), AM2321 *** default
#ifdef USERMOD_DHT_DHTTYPE
#define DHTTYPE USERMOD_DHT_DHTTYPE
#else
#define DHTTYPE DHT22
#endif

// Connect pin 1 (on the left) of the sensor to +5V
//   NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
//   to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
//   NOTE: Pin defaults below are for QuinLed Dig-Uno's Q4 on the board
// Connect pin 4 (on the right) of the sensor to GROUND
//   NOTE: If using a bare sensor (AM*), Connect a 10K resistor from pin 2
//   (data) to pin 1 (power) of the sensor. DHT* boards have the pullup already

#ifdef USERMOD_DHT_PIN
#define DHTPIN USERMOD_DHT_PIN
#else
#ifdef ARDUINO_ARCH_ESP32
#define DHTPIN 23
#else //ESP8266 boards
#define DHTPIN 13
#endif
#endif

// the frequency to check sensor, 1 minute
#ifndef USERMOD_DHT_MEASUREMENT_INTERVAL
#define USERMOD_DHT_MEASUREMENT_INTERVAL 60000
#endif

// how many seconds after boot to take first measurement, 20 seconds
#ifndef USERMOD_DHT_FIRST_MEASUREMENT_AT
#define USERMOD_DHT_FIRST_MEASUREMENT_AT 20000 
#endif

volatile float sensor_humidity = 0;
volatile float sensor_temperature = 0;
volatile uint8_t sensor_error = 0;
volatile int8_t sensor_result = 0;
volatile uint16_t sensor_error_count = 0;

void ICACHE_RAM_ATTR handleData(float h, float t) {
  sensor_humidity = h;
  sensor_temperature = t;
  sensor_result = 1;
}

void ICACHE_RAM_ATTR handleError(uint8_t e) {
  sensor_error = e;
  sensor_result = -1;
  sensor_error_count++;
}

class UsermodDHT : public Usermod {
  private:
    unsigned long nextReadTime = 0;
    unsigned long lastReadTime = 0;
    float humidity, temperature = 0;
    DHTTYPE sensor;
    bool initializing = true;
    bool disabled = false;
    bool error_retried = false;
  public:
    void setup() {
      // non-blocking sensor with callbacks
      sensor.setPin(DHTPIN);
      sensor.onData(handleData);
      sensor.onError(handleError);
      nextReadTime = millis() + USERMOD_DHT_FIRST_MEASUREMENT_AT;
      lastReadTime = nextReadTime;
    }


    void loop() {
      if (disabled) {
        return;
      }
      if (sensor_result == 1) {
        sensor_result = 0;
        lastReadTime = millis();
        initializing = false;
        error_retried = false;
        humidity = sensor_humidity;
        temperature = sensor_temperature;
        #ifndef USERMOD_DHT_CELSIUS
        temperature = (temperature * 9 / 5) + 32;
        #endif
      } else if (sensor_result == -1) {
        // retry right away on error - but only once
        if (!error_retried) {
          nextReadTime = 0;
          error_retried = true;
        }
      }
      if (millis() >= nextReadTime) {
        if ((millis() - lastReadTime)
            > 10*USERMOD_DHT_MEASUREMENT_INTERVAL) {
          disabled = true;
        } else {
          nextReadTime = millis() + USERMOD_DHT_MEASUREMENT_INTERVAL;
          sensor.read();
        }
      } 
    }


    void addToJsonInfo(JsonObject& root) {
      if (disabled) {
        return;
      }
      JsonObject user = root["u"];
      if (user.isNull()) user = root.createNestedObject("u");

      JsonArray temp = user.createNestedArray("Temperature");
      JsonArray hum = user.createNestedArray("Humidity");
      if (sensor_error_count > 0) {
        JsonArray err = user.createNestedArray("DHTErrors");
        err.add(sensor_error_count);
        err.add("Errors");
      }

      if (sensor_result == -1) {
        temp.add(sensor_error);
        temp.add(" DHT Sensor Error!");
        hum.add(sensor_error);
        hum.add(" DHT Sensor Error!");
        return;
      }

      if (initializing) {
        // if we haven't read the sensor yet, let the user know
        // that we are still waiting for the first measurement
        temp.add((nextReadTime - millis()) / 1000);
        temp.add(" sec until read");
        hum.add((nextReadTime - millis()) / 1000);
        hum.add(" sec until read");
        return;
      }


      hum.add(humidity);
      hum.add("%");

      temp.add(temperature);
      #ifdef USERMOD_DHT_CELSIUS
      temp.add("°C");
      #else
      temp.add("°F");
      #endif
    }
   
      uint16_t getId()
    {
      return USERMOD_ID_DHT;
    }

};
