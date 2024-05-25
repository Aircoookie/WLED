#ifndef WLED_ENABLE_MQTT
#error "This user mod requires MQTT to be enabled."
#endif

#pragma once

#include "wled.h"

class Smartnest : public Usermod
{
private:
  bool mqttInitialized = false;

  void sendToBroker(const char *const topic, const char *const message)
  {
    if (!WLED_MQTT_CONNECTED)
    {
      return;
    }

    String topic_ = String(mqttClientID) + "/" + String(topic);
    mqtt->publish(topic_.c_str(), 0, true, message);
  }

  void turnOff()
  {
    setBrightness(0);
    turnOnAtBoot = false;
    offMode = true;
    sendToBroker("report/powerState", "OFF");
  }

  void turnOn()
  {
    setBrightness(briLast);
    turnOnAtBoot = true;
    offMode = false;
    sendToBroker("report/powerState", "ON");
  }

  void setBrightness(int value)
  {
    if (value == 0 && bri > 0)
    {
      briLast = bri;
    }
    bri = value;
    stateUpdated(CALL_MODE_DIRECT_CHANGE);
  }

  void setColor(int r, int g, int b)
  {
    strip.setColor(0, r, g, b);
    stateUpdated(CALL_MODE_DIRECT_CHANGE);
    char msg[18] {};
    sprintf(msg, "rgb(%d,%d,%d)", r, g, b);
    sendToBroker("report/color", msg);
  }

  int splitColor(const char *const color, int * const rgb)
  {
    char *color_ = NULL;
    const char delim[] = ",";
    char *cxt = NULL;
    char *token = NULL;
    int position = 0;

    // We need to copy the string in order to keep it read only as strtok_r function requires mutable string
    color_ = (char *)malloc(strlen(color) + 1);
    if (NULL == color_) {
      return -1;
    }

    strcpy(color_, color);
    token = strtok_r(color_, delim, &cxt);

    while (token != NULL)
    {
      rgb[position++] = (int)strtoul(token, NULL, 10);
      token = strtok_r(NULL, delim, &cxt);
    }
    free(color_);

    return position;
  }

  void mqttInit()
  {
    if (!mqtt)
      return;
    mqtt->onMessage(std::bind(&Smartnest::onMqttMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
    mqtt->onConnect(std::bind(&Smartnest::onMqttConnect, this, std::placeholders::_1));
    mqttInitialized = true;
  }

public:
  bool onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
  {
    String topic_{topic};
    String topic_prefix{mqttClientID + String("/directive/")};

    if (!topic_.startsWith(topic_prefix))
    {
      return false;
    }

    String subtopic = topic_.substring(topic_prefix.length());
    String message_(payload, len);

    if (subtopic == "powerState")
    {
      if (message_ == "ON")
      {
        turnOn();
      }
      else if (message_ == "OFF")
      {
        turnOff();
      }
      return true;
    }

    if (subtopic == "percentage")
    {
      int val = message_.toInt();
      if (val >= 0 && val <= 100)
      {
        setBrightness(map(val, 0, 100, 0, 255));
      }
      return true;
    }

    if (subtopic == "color")
    {
      int rgb[3] = {};
      String colors = message_.substring(String("rgb(").length(), message_.lastIndexOf(')'));
      if (3 != splitColor(colors.c_str(), rgb))
      {
        return false;
      }
      setColor(rgb[0], rgb[1], rgb[2]);
      return true;
    }

    return false;
  }

  void onMqttConnect(bool sessionPresent)
  {
    String topic = String(mqttClientID) + "/#";

    mqtt->subscribe(topic.c_str(), 0);
    sendToBroker("report/online", (bri ? "true" : "false")); // Reports that the device is online
    delay(100);
    sendToBroker("report/firmware", versionString); // Reports the firmware version
    delay(100);
    sendToBroker("report/ip", (char *)WiFi.localIP().toString().c_str()); // Reports the IP
    delay(100);
    sendToBroker("report/network", (char *)WiFi.SSID().c_str()); // Reports the network name
    delay(100);

    String signal(WiFi.RSSI(), 10);
    sendToBroker("report/signal", signal.c_str()); // Reports the signal strength
    delay(100);
  }

  void setup()
  {
    Serial.begin(115200);
    mqttInit();
  }

  void loop()
  {
    if (!mqttInitialized)
    {
      mqttInit();
      return; // Try again in next loop iteration
    }
    // Your additional loop code here
  }

  uint16_t getId()
  {
    return USERMOD_ID_SMARTNEST;
  }
};
