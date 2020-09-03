#include "wled.h"
/*
 * This v1 usermod file allows you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * EEPROM bytes 2750+ are reserved for your custom use case. (if you extend #define EEPSIZE in const.h)
 * If you just need 8 bytes, use 2551-2559 (you do not need to increase EEPSIZE)
 * 
 * Consider the v2 usermod API if you need a more advanced feature set!
 */

//Use userVar0 and userVar1 (API calls &U0=,&U1=, uint16_t)

// PIR sensor pin
const int MOTION_PIN = 16;
 // MQTT topic for sensor values
const char MQTT_TOPIC[] = "/motion";

int prevState = LOW;

//gets called once at boot. Do all initialization that doesn't depend on network here
void userSetup()
{
  pinMode(MOTION_PIN, INPUT);
}

//gets called every time WiFi is (re-)connected. Initialize own network interfaces here
void userConnected()
{

}

void publishMqtt(String state)
{
  //Check if MQTT Connected, otherwise it will crash the 8266
  if (mqtt != nullptr){
    char subuf[38];
    strcpy(subuf, mqttDeviceTopic);
    strcat(subuf, MQTT_TOPIC);
    mqtt->publish(subuf, 0, true, state.c_str());
  }
}

//loop. You can use "if (WLED_CONNECTED)" to check for successful connection
void userLoop()
{
  if (digitalRead(MOTION_PIN) == HIGH && prevState == LOW) { // Motion detected
    publishMqtt("ON");
    prevState = HIGH;
  } 
  if (digitalRead(MOTION_PIN) == LOW && prevState == HIGH) {  // Motion stopped
    publishMqtt("OFF");
    prevState = LOW;
  }
}

