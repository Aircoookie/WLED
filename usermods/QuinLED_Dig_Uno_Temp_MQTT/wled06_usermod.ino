//starts Dallas Temp service on boot
void userSetup()
{
// Start the DS18B20 sensor
  sensors.begin(); 
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
      sensors.requestTemperatures();

//Gets prefered temperature scale based on selection in definitions section
      #ifdef Celsius
      float board_temperature = sensors.getTempCByIndex(0);
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
