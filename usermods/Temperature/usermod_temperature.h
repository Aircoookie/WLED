#pragma once

#include "wled.h"

#include <DallasTemperature.h> //DS18B20

//Pin defaults for QuinLed Dig-Uno
#ifdef ARDUINO_ARCH_ESP32
#define TEMPERATURE_PIN 18
#else //ESP8266 boards
#define TEMPERATURE_PIN 14
#endif

#define TEMP_CELSIUS // Comment out for Fahrenheit

#define MEASUREMENT_INTERVAL 60000 //1 Minute

OneWire oneWire(TEMPERATURE_PIN);
DallasTemperature sensor(&oneWire);

class UsermodTemperature : public Usermod {
  private:
    //set last reading as "40 sec before boot", so first reading is taken after 20 sec
    unsigned long lastMeasurement = UINT32_MAX - 40000;
    float temperature = 0.0f;
  public:
    void getReading() {
      sensor.requestTemperatures();
      #ifdef TEMP_CELSIUS
      temperature = sensor.getTempCByIndex(0);
      #else
      temperature = sensor.getTempFByIndex(0);
      #endif
    }

    void setup() {
      sensor.begin();
      sensor.setResolution(9);
    }

    void loop() {
      if (millis() - lastMeasurement > MEASUREMENT_INTERVAL)
      {
        getReading();

        if (WLED_MQTT_CONNECTED) {
          char subuf[38];
          strcpy(subuf, mqttDeviceTopic);
          strcat(subuf, "/temperature");
          mqtt->publish(subuf, 0, true, String(temperature).c_str());
        }
        lastMeasurement = millis();
      }
    }

    void addToJsonInfo(JsonObject& root) {
      JsonObject user = root["u"];
      if (user.isNull()) user = root.createNestedObject("u");

      JsonArray temp = user.createNestedArray("Temperature");
      if (temperature == DEVICE_DISCONNECTED_C) {
        temp.add(0);
        temp.add(" Sensor Error!");
        return;
      }

      temp.add(temperature);
      #ifdef TEMP_CELSIUS
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