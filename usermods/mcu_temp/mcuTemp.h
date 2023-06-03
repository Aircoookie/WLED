#pragma once

#include "wled.h"

// class name. Use something descriptive and leave the ": public Usermod" part :)
class mcuTemp : public Usermod
{

private:
  float mcutemp = 0;

  // any private methods should go here (non-inline methosd should be defined out of class)
  void publishMqtt(const char *state, bool retain = false); // example for publishing MQTT message

public:
  mcuTemp(const char *name, bool enabled) : Usermod(name, enabled) {} // WLEDMM

  void setup()
  {
  }

  void connected()
  {
  }

  void loop()
  {
    // if usermod is disabled or called during strip updating just exit
    // NOTE: on very long strips strip.isUpdating() may always return true so update accordingly
    if (!enabled || strip.isUpdating())
      return;

#ifdef ESP8266 // ESP8266
    // does not seem possible
    mcutemp = -1;
#elif defined(CONFIG_IDF_TARGET_ESP32S2) // ESP32S2
    mcutemp = -1;
#else                                    // ESP32 ESP32S3 and ESP32C3
    mcutemp = roundf(temperatureRead() * 100) / 100;
#endif

    if (millis() - lastTime > 10000)
    {
      char array[10];
      snprintf(array, sizeof(array), "%f", mcutemp);
      publishMqtt(array);
      lastTime = millis();
    }
  }
  /*
   * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
   * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
   * Below it is shown how this could be used for e.g. a light sensor
   */
  void addToJsonInfo(JsonObject &root)
  {
    // if "u" object does not exist yet wee need to create it
    JsonObject user = root["u"];
    if (user.isNull())
      user = root.createNestedObject("u");

    // this code adds "u":{"ExampleUsermod":[20," lux"]} to the info object
    // int reading = 20;
    JsonArray lightArr = user.createNestedArray(FPSTR(_name)); // name
    lightArr.add(mcutemp);                                     // value
    lightArr.add(F(" °C"));                                    // unit

    // if you are implementing a sensor usermod, you may publish sensor data
    // JsonObject sensor = root[F("sensor")];
    // if (sensor.isNull()) sensor = root.createNestedObject(F("sensor"));
    // temp = sensor.createNestedArray(F("light"));
    // temp.add(reading);
    // temp.add(F("lux"));
  }

  void addToJsonState(JsonObject &root)
  {
  }
  void readFromJsonState(JsonObject &root)
  {
  }

  void addToConfig(JsonObject &root)
  {
  }

  bool readFromConfig(JsonObject &root)
  {
    return true;
  }

  void appendConfigData()
  {
  }

  void handleOverlayDraw()
  {
  }

  /*
   * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
   * This could be used in the future for the system to determine whether your usermod is installed.
   */
  uint16_t getId()
  {
    return USERMOD_ID_MCUTEMP;
  }
};

void mcuTemp::publishMqtt(const char *state, bool retain)
{
#ifndef WLED_DISABLE_MQTT
  // Check if MQTT Connected, otherwise it will crash the 8266
  if (WLED_MQTT_CONNECTED)
  {
    char subuf[64];
    strcpy(subuf, mqttDeviceTopic);
    strcat_P(subuf, PSTR("/mcutemp"));
    mqtt->publish(subuf, 0, retain, state);
  }
#endif
}
