#include "wled.h"
#include <HTTPClient.h>

/*
 * MQTT communication protocol for home automation
 */

#ifdef WLED_ENABLE_MQTT
#define MQTT_KEEP_ALIVE_TIME 60 // contact the MQTT broker every 60 seconds

// declaring this function up here
void otaUpdate(String url);

void parseMQTTBriPayload(char *payload)
{
  if (strstr(payload, "ON") || strstr(payload, "on") || strstr(payload, "true"))
  {
    bri = briLast;
    stateUpdated(CALL_MODE_DIRECT_CHANGE);
  }
  else if (strstr(payload, "T") || strstr(payload, "t"))
  {
    toggleOnOff();
    stateUpdated(CALL_MODE_DIRECT_CHANGE);
  }
  else
  {
    uint8_t in = strtoul(payload, NULL, 10);
    if (in == 0 && bri > 0)
      briLast = bri;
    bri = in;
    stateUpdated(CALL_MODE_DIRECT_CHANGE);
  }
}

void onMqttConnect(bool sessionPresent)
{
  //(re)subscribe to required topics
  char subuf[38];

  if (mqttDeviceTopic[0] != 0)
  {
    strlcpy(subuf, mqttDeviceTopic, 33);
    mqtt->subscribe(subuf, 0);
    strcat_P(subuf, PSTR("/col"));
    mqtt->subscribe(subuf, 0);
    strlcpy(subuf, mqttDeviceTopic, 33);
    strcat_P(subuf, PSTR("/api"));
    mqtt->subscribe(subuf, 0);
    strlcpy(subuf, mqttDeviceTopic, 33);
    strcat_P(subuf, PSTR("/update"));
    mqtt->subscribe(subuf, 0);
  }

  if (mqttGroupTopic[0] != 0)
  {
    strlcpy(subuf, mqttGroupTopic, 33);
    mqtt->subscribe(subuf, 0);
    strcat_P(subuf, PSTR("/col"));
    mqtt->subscribe(subuf, 0);
    strlcpy(subuf, mqttGroupTopic, 33);
    strcat_P(subuf, PSTR("/api"));
    mqtt->subscribe(subuf, 0);
  }

  usermods.onMqttConnect(sessionPresent);

  doPublishMqtt = true;
  DEBUG_PRINTLN(F("MQTT ready"));
}

void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
  static char *payloadStr;

  DEBUG_PRINT(F("MQTT msg: "));
  DEBUG_PRINTLN(topic);

  // set connected to true. If we got a message, we must be connected (this fixes a lot of issues with AsyncMqttClient)
  mqtt->setConnected(true); // note that setConnected is a function that I added to AsyncMqttClient

  // paranoia check to avoid npe if no payload
  if (payload == nullptr)
  {
    DEBUG_PRINTLN(F("no payload -> leave"));
    return;
  }

  if (index == 0)
  { // start (1st partial packet or the only packet)
    if (payloadStr)
      delete[] payloadStr;            // fail-safe: release buffer
    payloadStr = new char[total + 1]; // allocate new buffer
  }
  if (payloadStr == nullptr)
    return; // buffer not allocated

  // copy (partial) packet to buffer and 0-terminate it if it is last packet
  char *buff = payloadStr + index;
  memcpy(buff, payload, len);
  if (index + len >= total)
  {                           // at end
    payloadStr[total] = '\0'; // terminate c style string
  }
  else
  {
    DEBUG_PRINTLN(F("Partial packet received."));
    return; // process next packet
  }
  DEBUG_PRINTLN(payloadStr);

  size_t topicPrefixLen = strlen(mqttDeviceTopic);
  if (strncmp(topic, mqttDeviceTopic, topicPrefixLen) == 0)
  {
    topic += topicPrefixLen;
  }
  else
  {
    topicPrefixLen = strlen(mqttGroupTopic);
    if (strncmp(topic, mqttGroupTopic, topicPrefixLen) == 0)
    {
      topic += topicPrefixLen;
    }
    else
    {
      // Non-Wled Topic used here. Probably a usermod subscribed to this topic.
      usermods.onMqttMessage(topic, payloadStr);
      delete[] payloadStr;
      payloadStr = nullptr;
      return;
    }
  }

  // Prefix is stripped from the topic at this point

  if (strcmp_P(topic, PSTR("/col")) == 0)
  {
    colorFromDecOrHexString(col, payloadStr);
    colorUpdated(CALL_MODE_DIRECT_CHANGE);
  }
  else if (strcmp_P(topic, PSTR("/api")) == 0)
  {
    if (!requestJSONBufferLock(15))
    {
      delete[] payloadStr;
      payloadStr = nullptr;
      return;
    }
    if (payloadStr[0] == '{')
    { // JSON API
      deserializeJson(doc, payloadStr);
      deserializeState(doc.as<JsonObject>());
    }
    else
    { // HTTP API
      String apireq = "win";
      apireq += '&'; // reduce flash string usage
      apireq += payloadStr;
      handleSet(nullptr, apireq);
    }
    releaseJSONBufferLock();
  }
  else if (strcmp_P(topic, PSTR("/update")) == 0)
  {
    // get the json buffer lock
    if (!requestJSONBufferLock(15))
    {
      delete[] payloadStr;
      payloadStr = nullptr;
      return;
    }

    // deserialize the request
    deserializeJson(doc, payloadStr);
    JsonObject obj = doc.as<JsonObject>();

    // make sure the request has a url
    if (!obj.containsKey("url") || !obj["url"].is<String>())
    {
      DEBUG_PRINTLN("No url in request, won't update. Returning.");
      // release the json buffer lock
      releaseJSONBufferLock();
      // clear out the payload string
      delete[] payloadStr;
      payloadStr = nullptr;
      return;
    }

    // get the url
    String url = obj["url"].as<String>();

    // request the response buffer lock
    if (!requestResponseBufferLock())
    {
      DEBUG_PRINTLN("Failed to get response buffer lock, returning.");
      // release the json buffer lock
      releaseJSONBufferLock();
      // clear out the payload string
      delete[] payloadStr;
      payloadStr = nullptr;
      return;
    }

    // make the response
    mqttResponseDoc["id"] = obj["id"];
    mqttResponseDoc["update"] = "Starting update, do not power off the device.";
    serializeJson(mqttResponseDoc, mqttResponseBuffer);

    // release the json buffer lock
    releaseJSONBufferLock();

    // send the response
    mqtt->publish(mqttResponseTopic, 1, false, mqttResponseBuffer);

    // release the response buffer lock
    releaseResponseBufferLock();

    // do the update
    return otaUpdate(url);
  }
  else if (strlen(topic) != 0)
  {
    // non standard topic, check with usermods
    usermods.onMqttMessage(topic, payloadStr);
  }
  else
  {
    // topmost topic (just wled/MAC)
    parseMQTTBriPayload(payloadStr);
  }
  delete[] payloadStr;
  payloadStr = nullptr;
}

void publishMqtt()
{
  doPublishMqtt = false;
  if (!WLED_MQTT_CONNECTED)
    return;
  DEBUG_PRINTLN(F("Publish MQTT"));

#ifndef USERMOD_SMARTNEST
  char s[10];
  char subuf[38];

  sprintf_P(s, PSTR("%u"), bri);
  strlcpy(subuf, mqttDeviceTopic, 33);
  strcat_P(subuf, PSTR("/g"));
  mqtt->publish(subuf, 0, retainMqttMsg, s); // optionally retain message (#2263)

  sprintf_P(s, PSTR("#%06X"), (col[3] << 24) | (col[0] << 16) | (col[1] << 8) | (col[2]));
  strlcpy(subuf, mqttDeviceTopic, 33);
  strcat_P(subuf, PSTR("/c"));
  mqtt->publish(subuf, 0, retainMqttMsg, s); // optionally retain message (#2263)

  strlcpy(subuf, mqttDeviceTopic, 33);
  strcat_P(subuf, PSTR("/status"));
  mqtt->publish(subuf, 0, true, "online"); // retain message for a LWT

  char apires[1024]; // allocating 1024 bytes from stack can be risky
  XML_response(nullptr, apires);
  strlcpy(subuf, mqttDeviceTopic, 33);
  strcat_P(subuf, PSTR("/v"));
  mqtt->publish(subuf, 0, retainMqttMsg, apires); // optionally retain message (#2263)
#endif
}

// HA autodiscovery was removed in favor of the native integration in HA v0.102.0

bool initMqtt()
{
  DEBUG_PRINTLN(F("Initializing MQTT"));
  // set the important variables
  mqttEnabled = true;
  strlcpy(mqttServer, "my-server-name-here", MQTT_MAX_SERVER_LEN + 1); // put actual mqtt server here, I use emqx
  mqttPort = 0;                                                        // put actual port here
  strlcpy(mqttUser, "username", 41);                                   // put actual username here
  strlcpy(mqttPass, "password", 65);                                   // put actual password here

  if (!mqttEnabled || mqttServer[0] == 0 || !WLED_CONNECTED)
    return false;

  if (mqtt == nullptr)
  {
    mqtt = new AsyncMqttClient();
    mqtt->onMessage(onMqttMessage);
    mqtt->onConnect(onMqttConnect);
  }
  if (mqtt->connected())
    return true;

  DEBUG_PRINTLN(F("Reconnecting MQTT with info:"));
  DEBUG_PRINTLN(mqttServer);
  DEBUG_PRINTLN(mqttPort);
  DEBUG_PRINTLN(mqttUser);
  DEBUG_PRINTLN(mqttPass);
  IPAddress mqttIP;
  if (mqttIP.fromString(mqttServer)) // see if server is IP or domain
  {
    mqtt->setServer(mqttIP, mqttPort);
  }
  else
  {
    mqtt->setServer(mqttServer, mqttPort);
  }
  mqtt->setClientId(mqttClientID);
  if (mqttUser[0] && mqttPass[0])
    mqtt->setCredentials(mqttUser, mqttPass);

#ifndef USERMOD_SMARTNEST
  strlcpy(mqttStatusTopic, mqttDeviceTopic, 33);
  strcat_P(mqttStatusTopic, PSTR("/status"));
  mqtt->setWill(mqttStatusTopic, 0, true, "offline"); // LWT message
#endif
  mqtt->setKeepAlive(MQTT_KEEP_ALIVE_TIME);
  mqtt->connect();
  return true;
}

void otaUpdate(String url)
{
  DEBUG_PRINT(F("OTA update from URL: "));
  DEBUG_PRINTLN(url);

  // make client for HTTP request
  HTTPClient http;
  http.begin(url);
  http.setTimeout(3000000); // 5 minute timeout, may change

  // do a get request to get the update binary
  int httpCode = http.GET();
  // make sure the request was successful
  if (httpCode != HTTP_CODE_OK)
  {
    DEBUG_PRINT(F("HTTP GET failed, code: "));
    DEBUG_PRINTLN(httpCode);
    http.end();
    return;
  }

  // disable the watchdog
  WLED::instance().disableWatchdog();
  otaInProgress = true; // I've tried both with and without this and neither works

  // get the size of the update
  int len = http.getSize();
  DEBUG_PRINT(F("Update size: "));
  DEBUG_PRINTLN(len);

  // make a buffer for reading
  WiFiClient *stream = http.getStreamPtr();

  DEBUG_PRINTLN("Got stream");

  // Initialize Update
  if (!Update.begin(len))
  {
    DEBUG_PRINTLN(F("Update.begin failed, most likely not enough space"));
    http.end();
    WLED::instance().enableWatchdog();
    otaInProgress = false;
    return;
  }

  DEBUG_PRINTLN("Update.begin succeeded");
  DEBUG_PRINTLN("Is the stream null?");
  DEBUG_PRINTLN(stream == nullptr);
  DEBUG_PRINT(F("Free Heap: "));
  DEBUG_PRINTLN(ESP.getFreeHeap());

  // write the update to the device
  size_t written = 0;
  int bufferSize = 512;
  uint8_t buffer[bufferSize];
  // size_t written = Update.writeStream(*stream);
  while (http.connected() && written < len)
  {
    if (stream->available())
    {
      int bytesRead = stream->readBytes(buffer, bufferSize);
      if (bytesRead == 0)
      {
        DEBUG_PRINTLN("No bytes read");
      }
      written += Update.write(buffer, bytesRead);
      DEBUG_PRINT("Bytes written: ");
      DEBUG_PRINTLN(written);
      if (ESP.getFreeHeap() < 80000)
      {
        DEBUG_PRINT(F("Free Heap below 80000: "));
        DEBUG_PRINTLN(ESP.getFreeHeap());
      }
      if (http.connected() != 1)
      {
        DEBUG_PRINT("http Connection status: ");
        DEBUG_PRINTLN(http.connected());
      }
      if (WiFi.status() != 3)
      {
        DEBUG_PRINT("Wifi status: ");
        DEBUG_PRINTLN(WiFi.status());
      }
    }
    else
    {
      DEBUG_PRINTLN("No bytes available");
    }
    delay(10);
  }

  DEBUG_PRINTLN("Wrote stream");

  // check if the update was successful
  if (written == len)
  {
    DEBUG_PRINTLN(F("Written to flash successfully"));
  }
  else
  {
    DEBUG_PRINT(F("Update failed, only wrote : "));
    DEBUG_PRINT(written);
    DEBUG_PRINTLN(F(" bytes"));
    http.end();
    WLED::instance().enableWatchdog();
    otaInProgress = false;
    return;
  }

  // End the update process
  if (Update.end())
  {
    if (Update.isFinished())
    {
      DEBUG_PRINTLN(F("Update finished successfully, restarting now"));
      http.end();
      delay(1000);
      ESP.restart();
    }
    else
    {
      DEBUG_PRINTLN(F("Update not finished, something went wrong!"));
    }
  }
  else
  {
    DEBUG_PRINT(F("OTA Error Occurred. Error: "));
    DEBUG_PRINT(Update.errorString());
    DEBUG_PRINT(" Code: ");
    DEBUG_PRINTLN(Update.getError());
  }

  // reenable the watchdog
  WLED::instance().enableWatchdog();
  // end the http request
  http.end();
  // set ota in progress to false
  otaInProgress = false;
}

#endif
