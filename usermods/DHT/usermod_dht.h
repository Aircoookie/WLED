#pragma once

#include "wled.h"

// DHT Library
// lib_deps = adafruit/DHT sensor library@^1.4.1
#include <DHT.h>

#ifndef USERMOD_DHT_PIN
  #ifdef ARDUINO_ARCH_ESP32
    #define USERMOD_DHT_PIN 18
  #else //ESP8266 boards
    #define USERMOD_DHT_PIN 14
  #endif
#endif

#ifndef USERMOD_DHT_TYPE
  #define USERMOD_DHT_TYPE DHT22 // Default to DHT22  (AM2302)
#endif

#if BTNPIN == USERMOD_DHT_PIN || BTNPIN == USERMOD_DHT_PIN
  #undef BTNPIN   // Deactivate button pin if it conflicts with one of the DHT pin.
#endif

#ifdef USERMOD_DHT_DEBUG
  #define USERMOD_DHT_DEBUG_PRINT(...)                                                       \
    {DEBUG_PRINT(__VA_ARGS__); }
  #define USERMOD_DHT_DEBUG_PRINTLN(...)                                                     \
    { DEBUG_PRINTLN(__VA_ARGS__); }
#else
  #define USERMOD_DHT_DEBUG_PRINT(...)                                                       \
    {} /**< Debug Print Placeholder if Debug is disabled */
  #define USERMOD_DHT_DEBUG_PRINTLN(...)                                                     \
    {} /**< Debug Print Line Placeholder if Debug is disabled */
#endif

// the frequency to check temperature, 1 minute
#ifndef USERMOD_DHT_MEASUREMENT_INTERVAL
  #define USERMOD_DHT_MEASUREMENT_INTERVAL 60000
#endif

// how many seconds after boot to take first measurement, 20 seconds
#ifndef USERMOD_DHT_FIRST_MEASUREMENT_AT
  #define USERMOD_DHT_FIRST_MEASUREMENT_AT 20000
#endif

// the frequency to check temperature, 1 minute
#ifndef USERMOD_DHT_FAHRENHEIT
  #define USERMOD_DHT_FAHRENHEIT false
#else
  #define USERMOD_DHT_FAHRENHEIT true
#endif

// Initialize DHT sensor
DHT sensor(USERMOD_DHT_PIN, USERMOD_DHT_TYPE);

class UsermodDht : public Usermod {
  private:
    // set last reading as "40 sec before boot", so first reading is taken after 20 sec
    unsigned long lastMeasurement = UINT32_MAX - (USERMOD_DHT_MEASUREMENT_INTERVAL - USERMOD_DHT_FIRST_MEASUREMENT_AT);

    unsigned long lastHumidityMeasurement = UINT32_MAX - (USERMOD_DHT_MEASUREMENT_INTERVAL - USERMOD_DHT_FIRST_MEASUREMENT_AT);

    float temperature = -100; // default to -100, DHT22 only goes down to -40C

    float humidity = 0;

    void readDht(){
        getTemperature();
        getHumidity();
    }

    void getTemperature() {
      temperature = sensor.readTemperature(USERMOD_DHT_FAHRENHEIT);

      USERMOD_DHT_DEBUG_PRINT("Temperature ");
      USERMOD_DHT_DEBUG_PRINTLN(temperature);
      lastMeasurement = millis();
    }

    void getHumidity() {
      humidity = sensor.readHumidity();

      USERMOD_DHT_DEBUG_PRINT("Humidity ");
      USERMOD_DHT_DEBUG_PRINTLN(humidity);
      lastHumidityMeasurement = millis();
    }

  public:

    void setup() {
      sensor.begin();
    }

    void loop() {
      unsigned long now = millis();

      // check to see if we are due for taking a measurement
      if (now - lastMeasurement > USERMOD_DHT_MEASUREMENT_INTERVAL)
      {
        getTemperature();

        if (WLED_MQTT_CONNECTED) {
          char subuf[38];
          strcpy(subuf, mqttDeviceTopic);
          if(!isnan(temperature) || temperature < -50){
            // dont publish super low temperature as the graph will get messed up
            // and the DHT22 library returns nan when problem
            // reading the sensor
            strcat(subuf, "/temperature");
            mqtt->publish(subuf, 0, true, String(temperature).c_str());
          } else {
            // publish something else to indicate status?
          }
        }
      }

      if (now - lastHumidityMeasurement > USERMOD_DHT_MEASUREMENT_INTERVAL)
      {
        getHumidity();

        if (WLED_MQTT_CONNECTED) {
          char subuf[38];
          strcpy(subuf, mqttDeviceTopic);
          if(!isnan(humidity) || humidity < 0){
            // dont publish negative humidity
            // and the DHT22 library returns nan when problem
            // reading the sensor
            strcat(subuf, "/humidity");
            mqtt->publish(subuf, 0, true, String(humidity).c_str());
          } else {
            // publish something else to indicate status?
          }
        }
      }


    }

    void addToJsonInfo(JsonObject& root) {
      JsonObject user = root["u"];
      if (user.isNull()) user = root.createNestedObject("u");

      JsonArray temp = user.createNestedArray("Temperature");
      JsonArray hum = user.createNestedArray("Humidity");

      if(!isnan(temperature) || temperature < -50){
        temp.add(temperature);        
        if(USERMOD_DHT_FAHRENHEIT){
          temp.add("°F");
        }else{
          temp.add("°C");
        }
      }else{
        temp.add(0);
        temp.add(" Sensor Error!");
      }

      if(!isnan(humidity) || humidity < 0){
        hum.add(humidity);
        hum.add("%");
      }else{
        hum.add(0);
        hum.add(" Sensor Error!");
      }

    }

    uint16_t getId()
    {
      return USERMOD_ID_TEMPERATURE;
    }
};
