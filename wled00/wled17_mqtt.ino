/*
 * MQTT communication protocol for home automation
 */

#define WLED_MQTT_PORT 1883

void parseMQTTBriPayload(char* payload)
{
  if      (strcmp(payload, "ON") == 0) {bri = briLast; colorUpdated(1);}
  else if (strcmp(payload, "T" ) == 0) {toggleOnOff(); colorUpdated(1);}
  else {
    uint8_t in = strtoul(payload, NULL, 10);
    if (in == 0 && bri > 0) briLast = bri;
    bri = in;
    colorUpdated(1);
  }
}


void callbackMQTT(char* topic, byte* payload, unsigned int length) {

  DEBUG_PRINT("MQTT callb rec: ");
  DEBUG_PRINTLN(topic);
  DEBUG_PRINTLN((char*)payload);

  //no need to check the topic because we only get topics we are subscribed to

  if (strstr(topic, "/col"))
  {
    colorFromDecOrHexString(col, (char*)payload);
    colorUpdated(1);
  } else if (strstr(topic, "/api"))
  {
    String apireq = "win&";
    apireq += (char*)payload;
    handleSet(apireq);
  } else
  {
    parseMQTTBriPayload((char*)payload);
  }
}


void publishMQTT()
{
  if (mqtt == NULL) return;
  if (!mqtt->connected()) return;
  DEBUG_PRINTLN("Publish MQTT");

  char s[10];
  char subuf[38];
  
  sprintf(s, "%ld", bri);
  strcpy(subuf, mqttDeviceTopic);
  strcat(subuf, "/g");
  mqtt->publish(subuf, s);

  sprintf(s, "#%X", col[3]*16777216 + col[0]*65536 + col[1]*256 + col[2]);
  strcpy(subuf, mqttDeviceTopic);
  strcat(subuf, "/c");
  mqtt->publish(subuf, s);

  //if you want to use this, increase the MQTT buffer in PubSubClient.h to 350+
  //it will publish the API response to MQTT
  /*XML_response(false, false);
  strcpy(subuf, mqttDeviceTopic);
  strcat(subuf, "/v");
  mqtt->publish(subuf, obuf);*/
}


bool reconnectMQTT()
{
  if (mqtt->connect(escapedMac.c_str()))
  {
    //re-subscribe to required topics
    char subuf[38];
    strcpy(subuf, mqttDeviceTopic);
    
    if (mqttDeviceTopic[0] != 0)
    {
      strcpy(subuf, mqttDeviceTopic);
      mqtt->subscribe(subuf);
      strcat(subuf, "/col");
      mqtt->subscribe(subuf);
      strcpy(subuf, mqttDeviceTopic);
      strcat(subuf, "/api");
      mqtt->subscribe(subuf);
    }

    if (mqttGroupTopic[0] != 0)
    {
      strcpy(subuf, mqttGroupTopic);
      mqtt->subscribe(subuf);
      strcat(subuf, "/col");
      mqtt->subscribe(subuf);
      strcpy(subuf, mqttGroupTopic);
      strcat(subuf, "/api");
      mqtt->subscribe(subuf);
    }

    publishMQTT();
  }
  return mqtt->connected();
}


bool initMQTT()
{
  if (WiFi.status() != WL_CONNECTED) return false;
  if (mqttServer[0] == 0) return false;
  
  IPAddress mqttIP;
  if (mqttIP.fromString(mqttServer)) //see if server is IP or domain
  {
    mqtt->setServer(mqttIP, WLED_MQTT_PORT);
  } else {
    mqtt->setServer(mqttServer, WLED_MQTT_PORT);
  }
  mqtt->setCallback(callbackMQTT);
  DEBUG_PRINTLN("MQTT ready.");
  return true;
}


void handleMQTT()
{
  if (WiFi.status() != WL_CONNECTED || !mqttInit) return;
  
  //every time connection is unsuccessful, the attempt interval is increased, since attempt will block program for 7 sec each time
  if (!mqtt->connected() && millis() - lastMQTTReconnectAttempt > 5000 + (5000 * mqttFailedConAttempts * mqttFailedConAttempts))
  {
    DEBUG_PRINTLN("Attempting to connect MQTT...");
    lastMQTTReconnectAttempt = millis();
    if (!reconnectMQTT())
    {
      //still attempt reconnect about once daily
      if (mqttFailedConAttempts < 120) mqttFailedConAttempts++;
      return;
    }
    DEBUG_PRINTLN("MQTT con!");
    mqttFailedConAttempts = 0;
  }
  mqtt->loop();
}
