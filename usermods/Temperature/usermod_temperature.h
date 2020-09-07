#pragma once

#include "wled.h"

#include <DallasTemperature.h> //DS18B20

//Pin defaults for QuinLed Dig-Uno
#ifdef ARDUINO_ARCH_ESP32
#define TEMPERATURE_PIN 18
#else //ESP8266 boards
#define TEMPERATURE_PIN 14
#endif

#define MEASUREMENT_INTERVAL 60000 //1 Minute

#define ELAPSED(current, previous) ((previous) <= (current) ? (current) - (previous) : (UINT32_MAX - previous) + current)

OneWire oneWire(TEMPERATURE_PIN);
DallasTemperature sensor(&oneWire);

class UsermodTemperature : public Usermod {
  private:
    // set last reading as "40 sec before boot", so first reading is taken after 20 sec
    unsigned long lastMeasurement = UINT32_MAX - 40000;
    // last time requestTemperatures was called
    unsigned long lastTempRequest;
    // indicates requestTemperatures has been called but the sensor measurement is not complete
    bool waitingForConversion = false;

    float temperature = DEVICE_DISCONNECTED_C;
	  DeviceAddress deviceAddress;

    void requestTemperatures() {
      sensor.requestTemperatures();
      waitingForConversion = true;
      lastTempRequest = millis();
    }

  public:
    void getReading() {
      #ifdef USERMOD_DALLASTEMPERATURE_CELSIUS
      //temperature = sensor.getTempCByIndex(0);
      temperature = sensor.getTempC((uint8_t*) deviceAddress);
      #else
      temperature = sensor.getTempFByIndex(0);
      #endif
      waitingForConversion = false;
    }

    void setup() {
      sensor.begin();
      sensor.setResolution(9);
      sensor.setWaitForConversion(false); // do not block waiting for reading
      // get the device's address to avoid getTemp(C|F)ByIndex calling this every time
      if (!sensor.getAddress(deviceAddress, 0)) {
        // ...
      }
    }

    void loop() {
      unsigned long now = millis();
      if (!waitingForConversion && ELAPSED(now, lastMeasurement) > MEASUREMENT_INTERVAL)
      {
        requestTemperatures();
      }
      else if (waitingForConversion && ELAPSED(now, lastTempRequest) >= 94 /* 93.75ms per the datasheet */)
      {
        getReading(); // this will reset waitingForConversion = false
 
        if (WLED_MQTT_CONNECTED) {
          char subuf[38];
          strcpy(subuf, mqttDeviceTopic);
          if (temperature != DEVICE_DISCONNECTED_C) {
            // dont publish -127 as the graph will get messed up
            strcat(subuf, "/temperature");
            mqtt->publish(subuf, 0, true, String(temperature).c_str());
          } else {
            // publish something else to indicate status?
          }
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