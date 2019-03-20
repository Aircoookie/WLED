/*
 * MQTT communication protocol for home automation
 */

#define WLED_MQTT_PORT 1883

void parseMQTTBriPayload(char* payload)
{
  if      (strcmp(payload, "ON") == 0 || strcmp(payload, "on") == 0) {bri = briLast; colorUpdated(1);}
  else if (strcmp(payload, "T" ) == 0 || strcmp(payload, "t" ) == 0) {toggleOnOff(); colorUpdated(1);}
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
  strcpy(subuf, mqttDeviceTopic);
  
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

  sendHADiscoveryMQTT();
  publishMqtt();
}


void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {

  DEBUG_PRINT("MQTT callb rec: ");
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
  if (mqtt == NULL) return;
  if (!mqtt->connected()) return;
  DEBUG_PRINTLN("Publish MQTT");

  char s[10];
  char subuf[38];
  
  sprintf(s, "%ld", bri);
  strcpy(subuf, mqttDeviceTopic);
  strcat(subuf, "/g");
  mqtt->publish(subuf, 0, true, s);

  sprintf(s, "#%X", col[3]*16777216 + col[0]*65536 + col[1]*256 + col[2]);
  strcpy(subuf, mqttDeviceTopic);
  strcat(subuf, "/c");
  mqtt->publish(subuf, 0, true, s);

  char apires[1024];
  XML_response(nullptr, false, apires);
  strcpy(subuf, mqttDeviceTopic);
  strcat(subuf, "/v");
  mqtt->publish(subuf, 0, true, apires);
}

void sendHADiscoveryMQTT(){
/*

YYYY is discovery tipic
XXXX is device name

Send out HA MQTT Discovery message on MQTT connect (~2.4kB):
{
"name": "XXXX",
"stat_t":"YYYY/c",
"cmd_t":"YYYY",
"rgb_stat_t":"YYYY/c",
"rgb_cmd_t":"YYYY/col",
"bri_cmd_t":"YYYY",
"bri_stat_t":"YYYY/g",
"bri_val_tpl":"{{value}}",
"rgb_cmd_tpl":"{{'#%02x%02x%02x' | format(red, green, blue)}}",
"rgb_val_tpl":"{{value[1:3]|int(base=16)}},{{value[3:5]|int(base=16)}},{{value[5:7]|int(base=16)}}",
"qos": 0,
"opt":true,
"pl_on": "ON",
"pl_off": "OFF",
"fx_cmd_t":"YYYY/api",
"fx_stat_t":"YYYY/api",
"fx_val_tpl":"{{value}}",
"fx_list":[
"[FX=00]  STATIC",
"[FX=01]  BLINK",
"[FX=02]  BREATH",
"[FX=03]  COLOR_WIPE",
"[FX=04]  COLOR_WIPE_RANDOM",
"[FX=05]  RANDOM_COLOR",
"[FX=06]  COLOR_SWEEP",
"[FX=07]  DYNAMIC",
"[FX=08]  RAINBOW",
"[FX=09]  RAINBOW_CYCLE",
"[FX=10]  SCAN",
"[FX=11]  DUAL_SCAN",
"[FX=12]  FADE",
"[FX=13]  THEATER_CHASE",
"[FX=14]  THEATER_C_RAINBOW",
"[FX=15]  RUNNING_LIGHTS",
"[FX=16]  SAW",
"[FX=17]  TWINKLE",
"[FX=18]  DISSOLVE",
"[FX=19]  DISSOLVE_RANDOM",
"[FX=20]  SPARKLE",
"[FX=21]  FLASH_SPARKLE",
"[FX=22]  HYPER_SPARKLE",
"[FX=23]  STROBE",
"[FX=24]  STROBE_RAINBOW",
"[FX=25]  MULTI_STROBE",
"[FX=26]  BLINK_RAINBOW",
"[FX=27]  ANDROID",
"[FX=28]  CHASE_COLOR",
"[FX=29]  CHASE_RANDOM",
"[FX=30]  CHASE_RAINBOW",
"[FX=31]  CHASE_FLASH",
"[FX=32]  CHASE_FLASH_RANDOM",
"[FX=33]  CHASE_RAINBOW_WHITE",
"[FX=34]  COLORFUL",
"[FX=35]  TRAFFIC_LIGHT",
"[FX=36]  COLOR_SWEEP_RANDOM",
"[FX=37]  RUNNING_COLOR",
"[FX=38]  RUNNING_RED_BLUE",
"[FX=39]  RUNNING_RANDOM",
"[FX=40]  LARSON_SCANNER",
"[FX=41]  COMET",
"[FX=42]  FIREWORKS",
"[FX=43]  RAIN",
"[FX=44]  MERRY_CHRISTMAS",
"[FX=45]  FIRE_FLICKER",
"[FX=46]  GRADIENT",
"[FX=47]  LOADING",
"[FX=48]  DUAL_COLOR_WIPE_IN_OUT",
"[FX=49]  DUAL_COLOR_WIPE_IN_IN",
"[FX=50]  DUAL_COLOR_WIPE_OUT_OUT",
"[FX=51]  DUAL_COLOR_WIPE_OUT_IN",
"[FX=52]  CIRCUS_COMBUSTUS",
"[FX=53]  HALLOWEEN",
"[FX=54]  TRICOLOR_CHASE",
"[FX=55]  TRICOLOR_WIPE",
"[FX=56]  TRICOLOR_FADE",
"[FX=57]  LIGHTNING",
"[FX=58]  ICU",
"[FX=59]  MULTI_COMET",
"[FX=60]  DUAL_LARSON_SCANNER",
"[FX=61]  RANDOM_CHASE",
"[FX=62]  OSCILLATE",
"[FX=63]  PRIDE_2015",
"[FX=64]  JUGGLE",
"[FX=65]  PALETTE",
"[FX=66]  FIRE_2012",
"[FX=67]  COLORWAVES",
"[FX=68]  BPM",
"[FX=69]  FILLNOISE8",
"[FX=70]  NOISE16_1",
"[FX=71]  NOISE16_2",
"[FX=72]  NOISE16_3",
"[FX=73]  NOISE16_4",
"[FX=74]  COLORTWINKLE",
"[FX=75]  LAKE",
"[FX=76]  METEOR",
"[FX=77]  METEOR_SMOOTH",
"[FX=78]  RAILWAY",
"[FX=79]  RIPPLE"
]

}

  */
  char bufc[38], bufcol[38], bufg[38], bufapi[38];

  strcpy(bufc, mqttDeviceTopic);
  strcpy(bufcol, mqttDeviceTopic);
  strcpy(bufg, mqttDeviceTopic);
  strcpy(bufapi, mqttDeviceTopic);

  strcat(bufc, "/c");
  strcat(bufcol, "/col");
  strcat(bufg, "/g");
  strcat(bufapi, "/api");


  DynamicJsonBuffer jsonBuffer(JSON_ARRAY_SIZE(strip.getModeCount()) + JSON_OBJECT_SIZE(18));
  JsonObject& root = jsonBuffer.createObject();
  root["name"] = serverDescription;
  root["stat_t"] = bufc;
  root["cmd_t"] = mqttDeviceTopic;
  root["rgb_stat_t"] = bufc;
  root["rgb_cmd_t"] = bufcol;
  root["bri_cmd_t"] = mqttDeviceTopic;
  root["bri_stat_t"] = bufg;
  root["bri_val_tpl"] = "{{value}}";
  root["rgb_cmd_tpl"] = "{{'#%02x%02x%02x'|format(red, green, blue)}}";
  root["rgb_val_tpl"] = "{{value[1:3]|int(base=16)}},{{value[3:5]|int(base=16)}},{{value[5:7]|int(base=16)}}";
  root["qos"] = 0;
  root["opt"] = true;
  root["pl_on"] = "ON";
  root["pl_off"] = "OFF";
  root["fx_cmd_t"] = bufapi;
  root["fx_stat_t"] = bufapi;
  root["fx_val_tpl"] = "{{value}}";

  JsonArray& fx_list = root.createNestedArray("fx_list");
  for(uint8_t i=0; i<strip.getModeCount(); i++)
  {
    char effect_tmp_name[64];
    snprintf(effect_tmp_name, 64, "[FX=%02d] %s", i, strip.getModeName(i));
    fx_list.add(String(effect_tmp_name));
  }

  size_t jlen = root.measureLength() + 1;
  char buffer[jlen], pubt[21 + sizeof(serverDescription) + 8];
  root.printTo(buffer, jlen);

  DEBUG_PRINT("HA Discovery Sending >>");
  DEBUG_PRINTLN(buffer);

  strcpy(pubt, "homeassistant/light/WLED_");
  strcat(pubt, escapedMac.c_str());
  strcat(pubt, "/config");
  mqtt->publish(pubt, 0, true, buffer);
}

bool initMqtt()
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
  mqtt->setClientId(escapedMac.c_str());
  mqtt->onMessage(onMqttMessage);
  mqtt->onConnect(onMqttConnect);
  mqtt->connect();
  DEBUG_PRINTLN("MQTT ready.");
  return true;
}
