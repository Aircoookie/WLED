/*
 * MQTT communication protocol for home automation
 */

void parseMQTTBriPayload(char* payload)
{
  if      (strstr(payload, "ON") || strstr(payload, "on") || strstr(payload, "true")) {bri = briLast; colorUpdated(1);}
  else if (strstr(payload, "T" ) || strstr(payload, "t" )) {toggleOnOff(); colorUpdated(1);}
  else {
    uint8_t in = strtoul(payload, NULL, 10);
    if (in == 0 && bri > 0) briLast = bri;
    bri = in;
    colorUpdated(1);
  }
}


void onMqttConnect(bool sessionPresent)
{
  //(re)subscribe to required topics
  char subuf[38];

  if (mqttDeviceTopic[0] != 0)
  {
    strcpy(subuf, mqttDeviceTopic);
    mqtt->subscribe(subuf, 0);
    strcat(subuf, "/col");
    mqtt->subscribe(subuf, 0);
    strcpy(subuf, mqttDeviceTopic);
    strcat(subuf, "/api");
    mqtt->subscribe(subuf, 0);
  }

  if (mqttGroupTopic[0] != 0)
  {
    strcpy(subuf, mqttGroupTopic);
    mqtt->subscribe(subuf, 0);
    strcat(subuf, "/col");
    mqtt->subscribe(subuf, 0);
    strcpy(subuf, mqttGroupTopic);
    strcat(subuf, "/api");
    mqtt->subscribe(subuf, 0);
  }

  doPublishMqtt = true;
  DEBUG_PRINTLN("MQTT ready");
}


void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {

  DEBUG_PRINT("MQTT msg: ");
  DEBUG_PRINTLN(topic);
  DEBUG_PRINTLN(payload);

  //no need to check the topic because we only get topics we are subscribed to

  if (strstr(topic, "/col"))
  {
    colorFromDecOrHexString(col, (char*)payload);
    colorUpdated(1);
  } else if (strstr(topic, "/api"))
  {
    String apireq = "win&";
    apireq += (char*)payload;
    handleSet(nullptr, apireq);
  } else parseMQTTBriPayload(payload);
}


void publishMqtt()
{
  doPublishMqtt = false;
  if (mqtt == nullptr || !mqtt->connected()) return;
  DEBUG_PRINTLN("Publish MQTT");

  char s[10];
  char subuf[38];

  sprintf(s, "%ld", bri);
  strcpy(subuf, mqttDeviceTopic);
  strcat(subuf, "/g");
  mqtt->publish(subuf, 0, true, s);

  sprintf(s, "#%06X", (col[3] << 24) | (col[0] << 16) | (col[1] << 8) | (col[2]));
  strcpy(subuf, mqttDeviceTopic);
  strcat(subuf, "/c");
  mqtt->publish(subuf, 0, true, s);

  strcpy(subuf, mqttDeviceTopic);
  strcat(subuf, "/status");
  mqtt->publish(subuf, 0, true, "online");

  char apires[1024];
  XML_response(nullptr, apires);
  strcpy(subuf, mqttDeviceTopic);
  strcat(subuf, "/v");
  mqtt->publish(subuf, 0, true, apires);
}


//HA autodiscovery was removed in favor of the native integration in HA v0.102.0

bool initMqtt()
{
  lastMqttReconnectAttempt = millis();
  if (mqttServer[0] == 0 || !WLED_CONNECTED) return false;

  if (mqtt == nullptr) {
    mqtt = new AsyncMqttClient();
    mqtt->onMessage(onMqttMessage);
    mqtt->onConnect(onMqttConnect);
  }
  if (mqtt->connected()) return true;

  DEBUG_PRINTLN("Reconnecting MQTT");
  IPAddress mqttIP;
  if (mqttIP.fromString(mqttServer)) //see if server is IP or domain
  {
    mqtt->setServer(mqttIP, mqttPort);
  } else {
    mqtt->setServer(mqttServer, mqttPort);
  }
  mqtt->setClientId(mqttClientID);
  if (mqttUser[0] && mqttPass[0]) mqtt->setCredentials(mqttUser, mqttPass);

  strcpy(mqttStatusTopic, mqttDeviceTopic);
  strcat(mqttStatusTopic, "/status");
  mqtt->setWill(mqttStatusTopic, 0, true, "offline");
  mqtt->connect();
  return true;
}
