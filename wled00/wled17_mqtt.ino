/*
 * MQTT communication protocol for home automation
 */

void callbackMQTT(char* topic, byte* payload, unsigned int length) {
  
  if (strcmp(topic, mqttTopic0) == 0 ||
      strcmp(topic, mqttTopic1) == 0) 
  {
    if      (strcmp((char*)payload, "ON") == 0) {bri = briLast;}
    else if (strcmp((char*)payload, "T" ) == 0) {handleSet("win&T=2");}
    else {
      uint8_t in = strtoul((char*)payload, NULL, 10);
      if (in == 0 && bri > 0) briLast = bri;
      bri = in;
    }
    colorUpdated(1);
    return; 
  }
  
  if (strcmp(topic, strcat(mqttTopic0, "/col")) == 0 ||
      strcmp(topic, strcat(mqttTopic1, "/col")) == 0) 
  {
    colorFromDecOrHexString(col, &white, (char*)payload);
    colorUpdated(1);
    return; 
  }
  
  if (strcmp(topic, strcat(mqttTopic0, "/api")) == 0 ||
      strcmp(topic, strcat(mqttTopic1, "/api")) == 0) 
  {
    handleSet(String((char*)payload));
    return; 
  }
}

void publishStatus()
{
  if (!mqtt.connected()) return;

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
    
    if (mqttTopic0[0] != 0)
    {
      mqtt.subscribe(mqttTopic0);
      mqtt.subscribe(strcat(mqttTopic0, "/col"));
      mqtt.subscribe(strcat(mqttTopic0, "/api"));
    }

    if (mqttTopic1[0] != 0)
    {
      mqtt.subscribe(mqttTopic1);
      mqtt.subscribe(strcat(mqttTopic1, "/col"));
      mqtt.subscribe(strcat(mqttTopic1, "/api"));
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
  return true;
}

void handleMQTT()
{
  if (WiFi.status() != WL_CONNECTED || !mqttInit) return;
  if (!mqtt.connected() && millis() - lastMQTTReconnectAttempt > 5000)
  {
    lastMQTTReconnectAttempt = millis();
    if (!reconnectMQTT()) return;
  }
  mqtt.loop();
}
