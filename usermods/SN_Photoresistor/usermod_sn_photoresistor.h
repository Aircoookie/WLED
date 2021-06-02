#pragma once

#include "wled.h"

//Pin defaults for QuinLed Dig-Uno
#define PHOTORESISTOR_PIN A0

// the frequency to check photoresistor, 10 seconds
#ifndef USERMOD_SN_PHOTORESISTOR_MEASUREMENT_INTERVAL
#define USERMOD_SN_PHOTORESISTOR_MEASUREMENT_INTERVAL 10000
#endif

// how many seconds after boot to take first measurement, 10 seconds
#ifndef USERMOD_SN_PHOTORESISTOR_FIRST_MEASUREMENT_AT
#define USERMOD_SN_PHOTORESISTOR_FIRST_MEASUREMENT_AT 10000
#endif

// supplied voltage
#ifndef USERMOD_SN_PHOTORESISTOR_REFERENCE_VOLTAGE
#define USERMOD_SN_PHOTORESISTOR_REFERENCE_VOLTAGE 5
#endif

// 10 bits
#ifndef USERMOD_SN_PHOTORESISTOR_ADC_PRECISION
#define USERMOD_SN_PHOTORESISTOR_ADC_PRECISION 1024.0
#endif

// resistor size 10K hms
#ifndef USERMOD_SN_PHOTORESISTOR_RESISTOR_VALUE
#define USERMOD_SN_PHOTORESISTOR_RESISTOR_VALUE 10000.0
#endif

// only report if differance grater than offset value
#ifndef USERMOD_SN_PHOTORESISTOR_OFFSET_VALUE
#define USERMOD_SN_PHOTORESISTOR_OFFSET_VALUE 5
#endif

class Usermod_SN_Photoresistor : public Usermod
{
private:
  // set last reading as "40 sec before boot", so first reading is taken after 20 sec
  unsigned long lastMeasurement = UINT32_MAX - (USERMOD_SN_PHOTORESISTOR_MEASUREMENT_INTERVAL - USERMOD_SN_PHOTORESISTOR_FIRST_MEASUREMENT_AT);
  // flag to indicate we have finished the first getTemperature call
  // allows this library to report to the user how long until the first
  // measurement
  bool getLuminanceComplete = false;
  uint16_t lastLDRValue = -1000;

  bool checkBoundSensor(float newValue, float prevValue, float maxDiff)
  {
    return isnan(prevValue) || newValue <= prevValue - maxDiff || newValue >= prevValue + maxDiff;
  }

  uint16_t getLuminance()
  {
    // http://forum.arduino.cc/index.php?topic=37555.0
    // https://forum.arduino.cc/index.php?topic=185158.0
    float volts = analogRead(PHOTORESISTOR_PIN) * (USERMOD_SN_PHOTORESISTOR_REFERENCE_VOLTAGE / USERMOD_SN_PHOTORESISTOR_ADC_PRECISION);
    float amps = volts / USERMOD_SN_PHOTORESISTOR_RESISTOR_VALUE;
    float lux = amps * 1000000 * 2.0;

    lastMeasurement = millis();
    getLuminanceComplete = true;
    return uint16_t(lux);
  }

public:
  void setup()
  {
    pinMode(PHOTORESISTOR_PIN, INPUT);
  }

  void loop()
  {
    unsigned long now = millis();

    // check to see if we are due for taking a measurement
    // lastMeasurement will not be updated until the conversion
    // is complete the the reading is finished
    if (now - lastMeasurement < USERMOD_SN_PHOTORESISTOR_MEASUREMENT_INTERVAL)
    {
      return;
    }

    uint16_t currentLDRValue = getLuminance();
    if (checkBoundSensor(currentLDRValue, lastLDRValue, USERMOD_SN_PHOTORESISTOR_OFFSET_VALUE))
    {
      lastLDRValue = currentLDRValue;

      if (WLED_MQTT_CONNECTED)
      {
        char subuf[45];
        strcpy(subuf, mqttDeviceTopic);
        strcat_P(subuf, PSTR("/luminance"));
        mqtt->publish(subuf, 0, true, String(lastLDRValue).c_str());
      }
      else
      {
        DEBUG_PRINTLN("Missing MQTT connection. Not publishing data");
      }
    }
  }

  void addToJsonInfo(JsonObject &root)
  {
    JsonObject user = root[F("u")];
    if (user.isNull()) user = root.createNestedObject(F("u"));

    JsonArray lux = user.createNestedArray(F("Luminance"));

    if (!getLuminanceComplete)
    {
      // if we haven't read the sensor yet, let the user know
      // that we are still waiting for the first measurement
      lux.add((USERMOD_SN_PHOTORESISTOR_FIRST_MEASUREMENT_AT - millis()) / 1000);
      lux.add(F(" sec until read"));
      return;
    }

    lux.add(lastLDRValue);
    lux.add(F(" lux"));
  }

  uint16_t getId()
  {
    return USERMOD_ID_SN_PHOTORESISTOR;
  }
};
