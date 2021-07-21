#pragma once

#include "wled.h"
#ifndef WLED_ENABLE_MQTT
#error "This user mod requires MQTT to be enabled."
#endif

class UsermodMqttStream: public Usermod
{
private:
    bool mqttInitialized;

public:
    UsermodMqttStream() :
            mqttInitialized(false)
    {
    }

    void setup()
    {
        
    } 

    void loop()
    {
        if (!mqttInitialized) {
            mqttInit();
            return; // Try again in next loop iteration
        }
    }

    void mqttInit()
    {
        char subuf[38];
        if (!mqtt)
            return;
        if (!mqtt->connected())
            return;
        mqtt->onMessage(
                std::bind(&UsermodMqttStream::onMqttMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4,
                          std::placeholders::_5, std::placeholders::_6));
        
       uint16_t ret=0;
        if (mqttDeviceTopic[0] != 0) {
            strcpy(subuf, mqttDeviceTopic);
            strcat_P(subuf, PSTR("/stream"));
            ret = mqtt->subscribe(subuf, 0);
            DEBUG_PRINTF("sub: %s -> %u\n",subuf, ret);
        }
        if (mqttGroupTopic[0] != 0) {
            strcpy(subuf, mqttGroupTopic);
            strcat_P(subuf, PSTR("/stream"));
            ret = mqtt->subscribe(subuf, 0);
            DEBUG_PRINTF("sub: %s -> %u\n",subuf, ret);
        }
        mqttInitialized = true;
    }

    void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);

};

inline void UsermodMqttStream::onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{// compare length of stream with led count
    DEBUG_PRINTLN("mqtt message");
    if (ledCount == len / 4) {
      realtimeLock(realtimeTimeoutMs*5, REALTIME_MODE_GENERIC);
      for(int i=0; i < len / 4; i++){
        if (!arlsDisableGammaCorrection && strip.gammaCorrectCol)
        {
          strip.setPixelColor(i, 
            strip.gamma8(payload[i*4]),
            strip.gamma8(payload[(i*4)+1]),
            strip.gamma8(payload[(i*4)+2]),
            strip.gamma8(payload[(i*4)+3])
          );
        } else {
          strip.setPixelColor(i, payload[i*4], payload[(i*4)+1], payload[(i*4)+2], payload[(i*4)+3]);
        }
      }
      strip.show();
    }
}
