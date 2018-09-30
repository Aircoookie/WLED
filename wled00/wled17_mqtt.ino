/*
 * MQTT communication protocol for home automation
 */

void parseMQTTBriPayload(char* payload)
{
  if      (strcmp(payload, "ON") == 0) {bri = briLast; colorUpdated(1);}
  else if (strcmp(payload, "T" ) == 0) {handleSet("win&T=2");}
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
    colorFromDecOrHexString(col, &white, (char*)payload);
    colorUpdated(1);
  } else if (strstr(topic, "/api"))
  {
    handleSet(String((char*)payload));
  } else
  {
    parseMQTTBriPayload((char*)payload);
  }
}

void publishStatus()
{
  if (!mqtt.connected()) return;
  DEBUG_PRINTLN("Publish MQTT");

  char s[4];
  sprintf(s,"%ld", bri);
  mqtt.publish(strcat(mqttTopic0, "/g") , s);
  XML_response(false);
  mqtt.publish(strcat(mqttTopic0, "/vs"), obuf);
}

bool reconnectMQTT()
{
  if (mqtt.connect(escapedMac.c_str()))
  {
    //re-subscribe to required topics
    char subuf[38];
    strcpy(subuf, mqttTopic0);
    
    if (mqttTopic0[0] != 0)
    {
      strcpy(subuf, mqttTopic0);
      mqtt.subscribe(subuf);
      strcat(subuf, "/col");
      mqtt.subscribe(subuf);
      strcpy(subuf, mqttTopic0);
      strcat(subuf, "/api");
      mqtt.subscribe(subuf);
    }

    if (mqttTopic1[0] != 0)
    {
      strcpy(subuf, mqttTopic1);
      mqtt.subscribe(subuf);
      strcat(subuf, "/col");
      mqtt.subscribe(subuf);
      strcpy(subuf, mqttTopic1);
      strcat(subuf, "/api");
      mqtt.subscribe(subuf);
    }
  }
  return mqtt.connected();
}

bool initMQTT()
{
  if (WiFi.status() != WL_CONNECTED) return false;
  if (mqttServer[0] == 0) return false;
  
  IPAddress mqttIP;
  if (mqttIP.fromString(mqttServer)) //see if server is IP or domain
  {
    mqtt.setServer(mqttIP,1883);
  } else {
    mqtt.setServer(mqttServer,1883);
  }
  mqtt.setCallback(callbackMQTT);
  DEBUG_PRINTLN("MQTT ready.");
  return true;
}

void handleMQTT()
{
  if (WiFi.status() != WL_CONNECTED || !mqttInit) return;
  if (!mqtt.connected() && millis() - lastMQTTReconnectAttempt > 5000)
  {
    DEBUG_PRINTLN("Attempting to connect MQTT...");
    lastMQTTReconnectAttempt = millis();
    if (!reconnectMQTT()) return;
    DEBUG_PRINTLN("MQTT con!");
  }
  mqtt.loop();
}
