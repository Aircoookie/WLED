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
  #ifdef WLED_ENABLE_HOMEASSISTANT_AUTODISCOVERY
  sendHADiscoveryMQTT();
  #endif
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

#ifdef WLED_ENABLE_HOMEASSISTANT_AUTODISCOVERY
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
    "[FX=00]  Static",
    "[FX=01]  Blink",
    "[FX=02]  Breath",
    "[FX=03]  Color Wipe",
    "[FX=04]  Color Wipe Random",
    "[FX=05]  Random Color",
    "[FX=06]  Color Sweep",
    "[FX=07]  Dynamic",
    "[FX=08]  Rainbow",
    "[FX=09]  Rainbow Cycle",
    "[FX=10]  Scan",
    "[FX=11]  Dual Scan",
    "[FX=12]  Fade",
    "[FX=13]  Theater Chase",
    "[FX=14]  Theater Chase Rainbow",
    "[FX=15]  Running Lights",
    "[FX=16]  Saw",
    "[FX=17]  Twinkle",
    "[FX=18]  Dissolve",
    "[FX=19]  Dissolve Random",
    "[FX=20]  Sparkle",
    "[FX=21]  Flash Sparkle",
    "[FX=22]  Hyper Sparkle",
    "[FX=23]  Strobe",
    "[FX=24]  Strobe Rainbow",
    "[FX=25]  Multi Strobe",
    "[FX=26]  Blink Rainbow",
    "[FX=27]  Android",
    "[FX=28]  Chase Color",
    "[FX=29]  Chase Random",
    "[FX=30]  Chase Rainbow",
    "[FX=31]  Chase Flash",
    "[FX=32]  Chase Flash Random",
    "[FX=33]  Chase Rainbow White",
    "[FX=34]  Colorful",
    "[FX=35]  Traffic Light",
    "[FX=36]  Color Sweep Random",
    "[FX=37]  Running Color",
    "[FX=38]  Running Red Blue",
    "[FX=39]  Running Random",
    "[FX=40]  Larson Scanner",
    "[FX=41]  Comet",
    "[FX=42]  Fireworks",
    "[FX=43]  Rain",
    "[FX=44]  Merry Christmas",
    "[FX=45]  Fire Flicker",
    "[FX=46]  Gradient",
    "[FX=47]  Loading",
    "[FX=48]  Dual Color Wipe In Out",
    "[FX=49]  Dual Color Wipe In In",
    "[FX=50]  Dual Color Wipe Out Out",
    "[FX=51]  Dual Color Wipe Out In",
    "[FX=52]  Circus Combustus",
    "[FX=53]  Halloween",
    "[FX=54]  Tricolor Chase",
    "[FX=55]  Tricolor Wipe",
    "[FX=56]  Tricolor Fade",
    "[FX=57]  Lightning",
    "[FX=58]  ICU",
    "[FX=59]  Multi Comet",
    "[FX=60]  Dual Larson Scanner",
    "[FX=61]  Random Chase",
    "[FX=62]  Oscillate",
    "[FX=63]  Pride 2015",
    "[FX=64]  Juggle",
    "[FX=65]  Pallette",
    "[FX=66]  Fire 2012",
    "[FX=67]  Colorwaves",
    "[FX=68]  BPM",
    "[FX=69]  Fill Noise 8",
    "[FX=70]  Noise 16 1",
    "[FX=71]  Noise 16 2",
    "[FX=72]  Noise 16 3",
    "[FX=73]  Noise 16 4",
    "[FX=74]  Color Twinkle",
    "[FX=75]  Lake",
    "[FX=76]  Meteor",
    "[FX=77]  Meteor Smooth",
    "[FX=78]  Railway",
    "[FX=79]  Ripple"
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
  #if defined(__CORE_ESP8266_VERSION_H) // only defined in esp8266-aruduino core > 2.5.0, recommended by maintainers to check for this movinf forward
    for(uint8_t i=0; i<strip.getModeCount(); i++)
    {
      char effect_tmp_name[35];
        snprintf(effect_tmp_name, 35, "[FX=%02d] %s", i, strip.getModeName(i)); // for esp8266-aruduino core > 2.5.0
        fx_list.add(effect_tmp_name);
    }
  #else // for esp8266-aruduino core < 2.5.0
    fx_list.add("[FX=00]  Static");
    fx_list.add("[FX=01]  Blink");
    fx_list.add("[FX=02]  Breath");
    fx_list.add("[FX=03]  Color Wipe");
    fx_list.add("[FX=04]  Color Wipe Random");
    fx_list.add("[FX=05]  Random Color");
    fx_list.add("[FX=06]  Color Sweep");
    fx_list.add("[FX=07]  Dynamic");
    fx_list.add("[FX=08]  Rainbow");
    fx_list.add("[FX=09]  Rainbow Cycle");
    fx_list.add("[FX=10]  Scan");
    fx_list.add("[FX=11]  Dual Scan");
    fx_list.add("[FX=12]  Fade");
    fx_list.add("[FX=13]  Theater Chase");
    fx_list.add("[FX=14]  Theater Chase Rainbow");
    fx_list.add("[FX=15]  Running Lights");
    fx_list.add("[FX=16]  Saw");
    fx_list.add("[FX=17]  Twinkle");
    fx_list.add("[FX=18]  Dissolve");
    fx_list.add("[FX=19]  Dissolve Random");
    fx_list.add("[FX=20]  Sparkle");
    fx_list.add("[FX=21]  Flash Sparkle");
    fx_list.add("[FX=22]  Hyper Sparkle");
    fx_list.add("[FX=23]  Strobe");
    fx_list.add("[FX=24]  Strobe Rainbow");
    fx_list.add("[FX=25]  Multi Strobe");
    fx_list.add("[FX=26]  Blink Rainbow");
    fx_list.add("[FX=27]  Android");
    fx_list.add("[FX=28]  Chase Color");
    fx_list.add("[FX=29]  Chase Random");
    fx_list.add("[FX=30]  Chase Rainbow");
    fx_list.add("[FX=31]  Chase Flash");
    fx_list.add("[FX=32]  Chase Flash Random");
    fx_list.add("[FX=33]  Chase Rainbow White");
    fx_list.add("[FX=34]  Colorful");
    fx_list.add("[FX=35]  Traffic Light");
    fx_list.add("[FX=36]  Color Sweep Random");
    fx_list.add("[FX=37]  Running Color");
    fx_list.add("[FX=38]  Running Red Blue");
    fx_list.add("[FX=39]  Running Random");
    fx_list.add("[FX=40]  Larson Scanner");
    fx_list.add("[FX=41]  Comet");
    fx_list.add("[FX=42]  Fireworks");
    fx_list.add("[FX=43]  Rain");
    fx_list.add("[FX=44]  Merry Christmas");
    fx_list.add("[FX=45]  Fire Flicker");
    fx_list.add("[FX=46]  Gradient");
    fx_list.add("[FX=47]  Loading");
    fx_list.add("[FX=48]  Dual Color Wipe In Out");
    fx_list.add("[FX=49]  Dual Color Wipe In In");
    fx_list.add("[FX=50]  Dual Color Wipe Out Out");
    fx_list.add("[FX=51]  Dual Color Wipe Out In");
    fx_list.add("[FX=52]  Circus Combustus");
    fx_list.add("[FX=53]  Halloween");
    fx_list.add("[FX=54]  Tricolor Chase");
    fx_list.add("[FX=55]  Tricolor Wipe");
    fx_list.add("[FX=56]  Tricolor Fade");
    fx_list.add("[FX=57]  Lightning");
    fx_list.add("[FX=58]  ICU");
    fx_list.add("[FX=59]  Multi Comet");
    fx_list.add("[FX=60]  Dual Larson Scanner");
    fx_list.add("[FX=61]  Random Chase");
    fx_list.add("[FX=62]  Oscillate");
    fx_list.add("[FX=63]  Pride 2015");
    fx_list.add("[FX=64]  Juggle");
    fx_list.add("[FX=65]  Pallette");
    fx_list.add("[FX=66]  Fire 2012");
    fx_list.add("[FX=67]  Colorwaves");
    fx_list.add("[FX=68]  BPM");
    fx_list.add("[FX=69]  Fill Noise 8");
    fx_list.add("[FX=70]  Noise 16 1");
    fx_list.add("[FX=71]  Noise 16 2");
    fx_list.add("[FX=72]  Noise 16 3");
    fx_list.add("[FX=73]  Noise 16 4");
    fx_list.add("[FX=74]  Color Twinkle");
    fx_list.add("[FX=75]  Lake");
    fx_list.add("[FX=76]  Meteor");
    fx_list.add("[FX=77]  Meteor Smooth");
    fx_list.add("[FX=78]  Railway");
    fx_list.add("[FX=79]  Ripple");
  #endif

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
#endif

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
