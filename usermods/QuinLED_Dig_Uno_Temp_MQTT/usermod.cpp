#include <Arduino.h>
#include "wled.h"
//Intiating code for QuinLED Dig-Uno temp sensor
//Uncomment Celsius if that is your prefered temperature scale
#include <DallasTemperature.h> //Dallastemperature sensor
#ifdef ARDUINO_ARCH_ESP32 //ESP32 boards
OneWire oneWire(18);
#else //ESP8266 boards
OneWire oneWire(14);
#endif
DallasTemperature sensor(&oneWire);
long temptimer = millis();
long lastMeasure = 0;
#define Celsius // Show temperature mesaurement in Celcius otherwise is in Fahrenheit
void userSetup()
{
// Start the DS18B20 sensor
  sensor.begin();
}
    
//gets called every time WiFi is (re-)connected. Initialize own network interfaces here
void userConnected()
{

}

void userLoop()
{
  temptimer = millis();
  
// Timer to publishe new temperature every 60 seconds
  if (temptimer - lastMeasure > 60000) {
    lastMeasure = temptimer;
    
//Check if MQTT Connected, otherwise it will crash the 8266
    if (mqtt != nullptr){
      sensor.requestTemperatures();

//Gets prefered temperature scale based on selection in definitions section
      #ifdef Celsius
      float board_temperature = sensor.getTempCByIndex(0);
      #else
      float board_temperature = sensors.getTempFByIndex(0);
      #endif

//Create character string populated with user defined device topic from the UI, and the read temperature. Then publish to MQTT server.
      char subuf[38];
      strcpy(subuf, mqttDeviceTopic);
      strcat(subuf, "/temperature");
      mqtt->publish(subuf, 0, true, String(board_temperature).c_str());
    return;}
  return;}
return;
}
