# 1 "C:\\Users\\RALFBR~1\\AppData\\Local\\Temp\\tmpgbs0_f6y"
#include <Arduino.h>
# 1 "C:/Users/RalfBruechmann/Documents/Arduino/code/WLED/wled00/wled00.ino"
# 25 "C:/Users/RalfBruechmann/Documents/Arduino/code/WLED/wled00/wled00.ino"
#define WLED_ENABLE_MQTT 
#define WLED_ENABLE_ADALIGHT 


#define WLED_DISABLE_FILESYSTEM 







#include <Arduino.h>
#ifdef WLED_ENABLE_DMX
  #include <ESPDMX.h>
  DMXESPSerial dmx;
#endif
#ifdef ESP8266
  #include <ESP8266WiFi.h>
  #include <ESP8266mDNS.h>
  #include <ESPAsyncTCP.h>
  extern "C" {
  #include <user_interface.h>
  }
#else
  #include <WiFi.h>
  #include "esp_wifi.h"
  #include <ESPmDNS.h>
  #include <AsyncTCP.h>
  #include "SPIFFS.h"
#endif

#include <ESPAsyncWebServer.h>
#include <EEPROM.h>
#include <WiFiUdp.h>
#include <DNSServer.h>
#ifndef WLED_DISABLE_OTA
  #include <ArduinoOTA.h>
#endif
#include <SPIFFSEditor.h>
#include "src/dependencies/time/TimeLib.h"
#include "src/dependencies/timezone/Timezone.h"
#ifndef WLED_DISABLE_ALEXA
  #define ESPALEXA_ASYNC
  #define ESPALEXA_NO_SUBPAGE
  #define ESPALEXA_MAXDEVICES 1

  #include "src/dependencies/espalexa/Espalexa.h"
#endif
#ifndef WLED_DISABLE_BLYNK
  #include "src/dependencies/blynk/BlynkSimpleEsp.h"
#endif
#include "src/dependencies/e131/ESPAsyncE131.h"
#include "src/dependencies/async-mqtt-client/AsyncMqttClient.h"
#include "src/dependencies/json/AsyncJson-v6.h"
#include "src/dependencies/json/ArduinoJson-v6.h"
#include "html_ui.h"
#include "html_settings.h"
#include "html_other.h"
#include "FX.h"
#include "ir_codes.h"
#include "const.h"

#ifndef CLIENT_SSID
#define CLIENT_SSID DEFAULT_CLIENT_SSID
#endif

#ifndef CLIENT_PASS
#define CLIENT_PASS ""
#endif


#if IR_PIN < 0
  #ifndef WLED_DISABLE_INFRARED
    #define WLED_DISABLE_INFRARED
  #endif
#endif

#ifndef WLED_DISABLE_INFRARED
  #include <IRremoteESP8266.h>
  #include <IRrecv.h>
  #include <IRutils.h>
#endif


#if defined(WLED_USE_ANALOG_LEDS) && defined(ESP8266)
  #include "src/dependencies/arduino/core_esp8266_waveform.h"
#endif


#ifdef WLED_DEBUG
  #ifndef ESP8266
    #include <rom/rtc.h>
  #endif
#endif


#define VERSION 2002252

char versionString[] = "0.9.1";



char apPass[65] = DEFAULT_AP_PASS;
char otaPass[33] = DEFAULT_OTA_PASS;





byte auxDefaultState = 0;
byte auxTriggeredState = 0;
char ntpServerName[33] = "0.wled.pool.ntp.org";



char clientSSID[33] = CLIENT_SSID;
char clientPass[65] = CLIENT_PASS;
char cmDNS[33] = "x";
char apSSID[33] = "";
byte apChannel = 1;
byte apHide = 0;
byte apBehavior = AP_BEHAVIOR_BOOT_NO_CONN;
IPAddress staticIP(0, 0, 0, 0);
IPAddress staticGateway(0, 0, 0, 0);
IPAddress staticSubnet(255, 255, 255, 0);
bool noWifiSleep = false;



uint16_t ledCount = 30;
bool useRGBW = false;
#define ABL_MILLIAMPS_DEFAULT 850;
bool turnOnAtBoot = true;
byte bootPreset = 0;

byte col[] {255, 160, 0, 0};
byte colSec[] {0, 0, 0, 0};
byte briS = 128;

byte nightlightTargetBri = 0;
byte nightlightDelayMins = 60;
bool nightlightFade = true;
bool nightlightColorFade = false;
bool fadeTransition = true;
uint16_t transitionDelay = 750;

bool skipFirstLed = false;
byte briMultiplier = 100;



char serverDescription[33] = "WLED";
bool syncToggleReceive = false;



bool buttonEnabled = true;
byte irEnabled = 0;

uint16_t udpPort = 21324;
uint16_t udpRgbPort = 19446;

bool receiveNotificationBrightness = true;
bool receiveNotificationColor = true;
bool receiveNotificationEffects = true;
bool notifyDirect = false;
bool notifyButton = false;
bool notifyAlexa = false;
bool notifyMacro = false;
bool notifyHue = true;
bool notifyTwice = false;

bool alexaEnabled = true;
char alexaInvocationName[33] = "Light";

char blynkApiKey[36] = "";

uint16_t realtimeTimeoutMs = 2500;
int arlsOffset = 0;
bool receiveDirect = true;
bool arlsDisableGammaCorrection = true;
bool arlsForceMaxBri = false;

uint16_t e131Universe = 1;
uint8_t DMXMode = DMX_MODE_MULTIPLE_RGB;
uint16_t DMXAddress = 1;
uint8_t DMXOldDimmer = 0;
uint8_t e131LastSequenceNumber = 0;
bool e131Multicast = false;

bool mqttEnabled = false;
char mqttDeviceTopic[33] = "";
char mqttGroupTopic[33] = "wled/all";
char mqttServer[33] = "";
char mqttUser[41] = "";
char mqttPass[41] = "";
char mqttClientID[41] = "";
uint16_t mqttPort = 1883;

bool huePollingEnabled = false;
uint16_t huePollIntervalMs = 2500;
char hueApiKey[47] = "api";
byte huePollLightId = 1;
IPAddress hueIP = (0, 0, 0, 0);
bool hueApplyOnOff = true;
bool hueApplyBri = true;
bool hueApplyColor = true;



bool ntpEnabled = false;
bool useAMPM = false;
byte currentTimezone = 0;
int utcOffsetSecs = 0;

byte overlayDefault = 0;
byte overlayMin = 0, overlayMax = ledCount - 1;

byte analogClock12pixel = 0;
bool analogClockSecondsTrail = false;
bool analogClock5MinuteMarks = false;

char cronixieDisplay[7] = "HHMMSS";
bool cronixieBacklight = true;

bool countdownMode = false;
byte countdownYear = 20, countdownMonth = 1;
byte countdownDay = 1, countdownHour = 0;
byte countdownMin = 0, countdownSec = 0;


byte macroBoot = 0;
byte macroNl = 0;
byte macroCountdown = 0;
byte macroAlexaOn = 0, macroAlexaOff = 0;
byte macroButton = 0, macroLongPress = 0, macroDoublePress = 0;



bool otaLock = false;
bool wifiLock = false;
bool aOtaEnabled = true;


uint16_t userVar0 = 0, userVar1 = 0;

#ifdef WLED_ENABLE_DMX

  byte DMXChannels = 7;
  byte DMXFixtureMap[15] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  uint16_t DMXGap = 10;
  uint16_t DMXStart = 10;
#endif




bool apActive = false;
bool forceReconnect = false;
uint32_t lastReconnectAttempt = 0;
bool interfacesInited = false;
bool wasConnected = false;


byte colOld[] {0, 0, 0, 0};
byte colT[] {0, 0, 0, 0};
byte colIT[] {0, 0, 0, 0};
byte colSecT[] {0, 0, 0, 0};
byte colSecOld[] {0, 0, 0, 0};
byte colSecIT[] {0, 0, 0, 0};

byte lastRandomIndex = 0;


bool transitionActive = false;
uint16_t transitionDelayDefault = transitionDelay;
uint16_t transitionDelayTemp = transitionDelay;
unsigned long transitionStartTime;
float tperLast = 0;
bool jsonTransitionOnce = false;


bool nightlightActive = false;
bool nightlightActiveOld = false;
uint32_t nightlightDelayMs = 10;
uint8_t nightlightDelayMinsDefault = nightlightDelayMins;
unsigned long nightlightStartTime;
byte briNlT = 0;
byte colNlT[] {0, 0, 0, 0};


unsigned long lastOnTime = 0;
bool offMode = !turnOnAtBoot;
byte bri = briS;
byte briOld = 0;
byte briT = 0;
byte briIT = 0;
byte briLast = 128;
byte whiteLast = 128;


bool buttonPressedBefore = false;
bool buttonLongPressed = false;
unsigned long buttonPressedTime = 0;
unsigned long buttonWaitTime = 0;


bool notifyDirectDefault = notifyDirect;
bool receiveNotifications = true;
unsigned long notificationSentTime = 0;
byte notificationSentCallMode = NOTIFIER_CALL_MODE_INIT;
bool notificationTwoRequired = false;


byte effectCurrent = 0;
byte effectSpeed = 128;
byte effectIntensity = 128;
byte effectPalette = 0;


bool udpConnected = false, udpRgbConnected = false;


bool showWelcomePage = false;


byte hueError = HUE_ERROR_INACTIVE;

float hueXLast = 0, hueYLast = 0;
uint16_t hueHueLast = 0, hueCtLast = 0;
byte hueSatLast = 0, hueBriLast = 0;
unsigned long hueLastRequestSent = 0;
bool hueAuthRequired = false;
bool hueReceived = false;
bool hueStoreAllowed = false, hueNewKey = false;


byte overlayCurrent = overlayDefault;
byte overlaySpeed = 200;
unsigned long overlayRefreshMs = 200;
unsigned long overlayRefreshedTime;


byte dP[] {0, 0, 0, 0, 0, 0};
bool cronixieInit = false;


unsigned long countdownTime = 1514764800L;
bool countdownOverTriggered = true;


byte lastTimerMinute = 0;
byte timerHours[] = {0, 0, 0, 0, 0, 0, 0, 0};
byte timerMinutes[] = {0, 0, 0, 0, 0, 0, 0, 0};
byte timerMacro[] = {0, 0, 0, 0, 0, 0, 0, 0};
byte timerWeekday[] = {255, 255, 255, 255, 255, 255, 255, 255};



bool blynkEnabled = false;


bool presetCyclingEnabled = false;
byte presetCycleMin = 1, presetCycleMax = 5;
uint16_t presetCycleTime = 1250;
unsigned long presetCycledTime = 0; byte presetCycCurr = presetCycleMin;
bool presetApplyBri = false, presetApplyCol = true, presetApplyFx = true;
bool saveCurrPresetCycConf = false;


byte realtimeMode = REALTIME_MODE_INACTIVE;
IPAddress realtimeIP = (0,0,0,0);
unsigned long realtimeTimeout = 0;


long lastMqttReconnectAttempt = 0;
long lastInterfaceUpdate = 0;
byte interfaceUpdateCallMode = NOTIFIER_CALL_MODE_INIT;
char mqttStatusTopic[40] = "";

#if AUXPIN >= 0

  byte auxTime = 0;
  unsigned long auxStartTime = 0;
  bool auxActive = false, auxActiveBefore = false;
#endif


String escapedMac;
#ifndef WLED_DISABLE_ALEXA
  Espalexa espalexa;
  EspalexaDevice* espalexaDevice;
#endif


DNSServer dnsServer;


bool ntpConnected = false;
time_t local = 0;
unsigned long ntpLastSyncTime = 999000000L;
unsigned long ntpPacketSentTime = 999000000L;
IPAddress ntpServerIP;
uint16_t ntpLocalPort = 2390;
#define NTP_PACKET_SIZE 48


#define MAX_LEDS 1500
#define MAX_LEDS_DMA 500


#define OMAX 2048
char* obuf;
uint16_t olen = 0;


uint16_t savedPresets = 0;
int8_t currentPreset = -1;
bool isPreset = false;

byte errorFlag = 0;

String messageHead, messageSub;
byte optionType;

bool doReboot = false;
bool doPublishMqtt = false;


AsyncWebServer server(80);
AsyncClient* hueClient = NULL;
AsyncMqttClient* mqtt = NULL;


void colorFromUint32(uint32_t, bool = false);
void serveMessage(AsyncWebServerRequest*, uint16_t, String, String, byte);
void handleE131Packet(e131_packet_t*, IPAddress);
void arlsLock(uint32_t,byte);
void handleOverlayDraw();

#define E131_MAX_UNIVERSE_COUNT 9


WiFiUDP notifierUdp, rgbUdp;
WiFiUDP ntpUdp;
ESPAsyncE131 e131(handleE131Packet);
bool e131NewData = false;


WS2812FX strip = WS2812FX();

#define WLED_CONNECTED (WiFi.status() == WL_CONNECTED)
#define WLED_WIFI_CONFIGURED (strlen(clientSSID) >= 1 && strcmp(clientSSID,DEFAULT_CLIENT_SSID) != 0)


#ifdef WLED_DEBUG
  #define DEBUG_PRINT(x) Serial.print (x)
  #define DEBUG_PRINTLN(x) Serial.println (x)
  #define DEBUG_PRINTF(x) Serial.printf (x)
  unsigned long debugTime = 0;
  int lastWifiState = 3;
  unsigned long wifiStateChangedTime = 0;
  int loops = 0;
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(x)
#endif


#ifndef WLED_DISABLE_FILESYSTEM
  #include <FS.h>
  #ifdef ARDUINO_ARCH_ESP32
    #include "SPIFFS.h"
  #endif
  #include "SPIFFSEditor.h"
#endif
void reset();
bool oappend(const char* txt);
bool oappendi(int i);
void setup();
void loop();
void commit();
void clearEEPROM();
void writeStringToEEPROM(uint16_t pos, char* str, uint16_t len);
void readStringFromEEPROM(uint16_t pos, char* str, uint16_t len);
void saveSettingsToEEPROM();
void loadSettingsFromEEPROM(bool first);
void savedToPresets();
void savePreset(byte index);
void loadMacro(byte index, char* m);
void applyMacro(byte index);
char* URL_response(AsyncWebServerRequest *request);
void sappend(char stype, const char* key, int val);
void sappends(char stype, const char* key, char* val);
void getSettingsJS(byte subPage, char* dest);
bool isAsterisksOnly(const char* str, byte maxLen);
void handleSettingsSet(AsyncWebServerRequest *request, byte subPage);
int getNumVal(const String* req, uint16_t pos);
bool handleSet(AsyncWebServerRequest *request, const String& req);
void handleSerial();
String getContentType(AsyncWebServerRequest* request, String filename);
bool handleFileRead(AsyncWebServerRequest* request, String path);
bool handleFileRead(AsyncWebServerRequest*, String path);
void wledInit();
void beginStrip();
void initConnection();
void initInterfaces();
void handleConnection();
int getSignalQuality(int rssi);
void userSetup();
void userConnected();
void userLoop();
void handleE131Packet(e131_packet_t* p, IPAddress clientIP);
void handleNotifications();
void setRealtimePixel(uint16_t i, byte r, byte g, byte b, byte w);
void setValuesFromMainSeg();
void resetTimebase();
void toggleOnOff();
void setAllLeds();
bool colorChanged();
void colorUpdated(int callMode);
void updateInterfaces(uint8_t callMode);
void handleTransitions();
void handleNightlight();
void shortPressAction();
void handleButton();
void handleIO();
void handleNetworkTime();
void sendNTPPacket();
bool checkNTPResponse();
void updateLocalTime();
void getTimeString(char* out);
void setCountdown();
bool checkCountdown();
byte weekdayMondayFirst();
void checkTimers();
void initCronixie();
void handleOverlays();
void _overlayAnalogClock();
void _overlayAnalogCountdown();
void alexaInit();
void handleAlexa();
byte getSameCodeLength(char code, int index, char const cronixieDisplay[]);
void setCronixie();
void _overlayCronixie();
void colorFromUint32(uint32_t in, bool secondary);
void colorHStoRGB(uint16_t hue, byte sat, byte* rgb);
void colorCTtoRGB(uint16_t mired, byte* rgb);
void colorXYtoRGB(float x, float y, byte* rgb);
void colorRGBtoXY(byte* rgb, float* xy);
void colorFromDecOrHexString(byte* rgb, char* in);
float minf (float v, float w);
float maxf (float v, float w);
void colorRGBtoRGBW(byte* rgb);
void handleHue();
void reconnectHue();
void onHueError(void* arg, AsyncClient* client, int8_t error);
void onHueConnect(void* arg, AsyncClient* client);
void sendHuePoll();
void onHueData(void* arg, AsyncClient* client, void *data, size_t len);
void handleHue();
void reconnectHue();
void initBlynk(const char* auth);
void handleBlynk();
void updateBlynk();
void parseMQTTBriPayload(char* payload);
void onMqttConnect(bool sessionPresent);
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
void publishMqtt();
bool initMqtt();
bool initMqtt();
void publishMqtt();
bool isIp(String str);
bool captivePortal(AsyncWebServerRequest *request);
void initServer();
void serveIndexOrWelcome(AsyncWebServerRequest *request);
void serveIndex(AsyncWebServerRequest* request);
String msgProcessor(const String& var);
String settingsProcessor(const String& var);
String dmxProcessor(const String& var);
void serveSettings(AsyncWebServerRequest* request);
void deserializeSegment(JsonObject elem, byte it);
bool deserializeState(JsonObject root);
void serializeState(JsonObject root);
void serializeInfo(JsonObject root);
void serveJson(AsyncWebServerRequest* request);
void serveLiveLeds(AsyncWebServerRequest* request);
void handleIR();
bool decodeIRCustom(uint32_t code);
void decodeIR(uint32_t code);
void decodeIR24(uint32_t code);
void decodeIR24OLD(uint32_t code);
void decodeIR24CT(uint32_t code);
void decodeIR40(uint32_t code);
void decodeIR44(uint32_t code);
void decodeIR21(uint32_t code);
void decodeIR6(uint32_t code);
void initIR();
void handleIR();
void handleDMX();
void handleDMX();
#line 508 "C:/Users/RalfBruechmann/Documents/Arduino/code/WLED/wled00/wled00.ino"
void reset()
{
  briT = 0;
  long dly = millis();
  while (millis() - dly < 250)
  {
    yield();
  }
  setAllLeds();
  DEBUG_PRINTLN("MODULE RESET");
  ESP.restart();
}



bool oappend(const char* txt)
{
  uint16_t len = strlen(txt);
  if (olen + len >= OMAX) return false;
  strcpy(obuf + olen, txt);
  olen += len;
  return true;
}



bool oappendi(int i)
{
  char s[11];
  sprintf(s, "%ld", i);
  return oappend(s);
}



void setup() {


  wledInit();
}



void loop() {
  handleIR();
  handleConnection();
  handleSerial();
  handleNotifications();
  handleTransitions();
  handleDMX();
  userLoop();

  yield();
  handleIO();
  handleIR();
  handleNetworkTime();
  handleAlexa();

  handleOverlays();
  yield();
#ifdef WLED_USE_ANALOG_LEDS
  strip.setRgbwPwm();
#endif

  if (doReboot) reset();

  if (!realtimeMode)
  {
    if (apActive) dnsServer.processNextRequest();
#ifndef WLED_DISABLE_OTA
    if (WLED_CONNECTED && aOtaEnabled) ArduinoOTA.handle();
#endif
    handleNightlight();
    yield();

    handleHue();
    handleBlynk();

    yield();
    if (!offMode) strip.service();
  }
  yield();
#ifdef ESP8266
  MDNS.update();
#endif
  if (millis() - lastMqttReconnectAttempt > 30000) initMqtt();


#ifdef WLED_DEBUG
  if (millis() - debugTime > 9999)
  {
    DEBUG_PRINTLN("---DEBUG INFO---");
    DEBUG_PRINT("Runtime: "); DEBUG_PRINTLN(millis());
    DEBUG_PRINT("Unix time: "); DEBUG_PRINTLN(now());
    DEBUG_PRINT("Free heap: "); DEBUG_PRINTLN(ESP.getFreeHeap());
    DEBUG_PRINT("Wifi state: "); DEBUG_PRINTLN(WiFi.status());
    if (WiFi.status() != lastWifiState)
    {
      wifiStateChangedTime = millis();
    }
    lastWifiState = WiFi.status();
    DEBUG_PRINT("State time: "); DEBUG_PRINTLN(wifiStateChangedTime);
    DEBUG_PRINT("NTP last sync: "); DEBUG_PRINTLN(ntpLastSyncTime);
    DEBUG_PRINT("Client IP: "); DEBUG_PRINTLN(WiFi.localIP());
    DEBUG_PRINT("Loops/sec: "); DEBUG_PRINTLN(loops / 10);
    loops = 0;
    debugTime = millis();
  }
  loops++;
#endif
}
# 1 "C:/Users/RalfBruechmann/Documents/Arduino/code/WLED/wled00/wled01_eeprom.ino"





#define EEPSIZE 2560


#define EEPVER 17
# 29 "C:/Users/RalfBruechmann/Documents/Arduino/code/WLED/wled00/wled01_eeprom.ino"
void commit()
{
  if (!EEPROM.commit()) errorFlag = 2;
}




void clearEEPROM()
{
  for (int i = 0; i < EEPSIZE; i++)
  {
    EEPROM.write(i, 0);
  }
  commit();
}


void writeStringToEEPROM(uint16_t pos, char* str, uint16_t len)
{
  for (int i = 0; i < len; ++i)
  {
    EEPROM.write(pos + i, str[i]);
    if (str[i] == 0) return;
  }
}


void readStringFromEEPROM(uint16_t pos, char* str, uint16_t len)
{
  for (int i = 0; i < len; ++i)
  {
    str[i] = EEPROM.read(pos + i);
    if (str[i] == 0) return;
  }
  str[len] = 0;
}




void saveSettingsToEEPROM()
{
  if (EEPROM.read(233) != 233)
  {
    clearEEPROM();
    EEPROM.write(233, 233);
  }

  writeStringToEEPROM( 0, clientSSID, 32);
  writeStringToEEPROM( 32, clientPass, 64);
  writeStringToEEPROM( 96, cmDNS, 32);
  writeStringToEEPROM(128, apSSID, 32);
  writeStringToEEPROM(160, apPass, 64);

  EEPROM.write(224, nightlightDelayMinsDefault);
  EEPROM.write(225, nightlightFade);
  EEPROM.write(226, notifyDirectDefault);
  EEPROM.write(227, apChannel);
  EEPROM.write(228, apHide);
  EEPROM.write(229, ledCount & 0xFF);
  EEPROM.write(230, notifyButton);
  EEPROM.write(231, notifyTwice);
  EEPROM.write(232, buttonEnabled);


  for (int i = 0; i<4; i++)
  {
    EEPROM.write(234+i, staticIP[i]);
    EEPROM.write(238+i, staticGateway[i]);
    EEPROM.write(242+i, staticSubnet[i]);
  }

  EEPROM.write(249, briS);

  EEPROM.write(250, receiveNotificationBrightness);
  EEPROM.write(251, fadeTransition);
  EEPROM.write(252, strip.reverseMode);
  EEPROM.write(253, transitionDelayDefault & 0xFF);
  EEPROM.write(254, (transitionDelayDefault >> 8) & 0xFF);
  EEPROM.write(255, briMultiplier);


  writeStringToEEPROM(256, otaPass, 32);

  EEPROM.write(288, nightlightTargetBri);
  EEPROM.write(289, otaLock);
  EEPROM.write(290, udpPort & 0xFF);
  EEPROM.write(291, (udpPort >> 8) & 0xFF);
  writeStringToEEPROM(292, serverDescription, 32);

  EEPROM.write(327, ntpEnabled);
  EEPROM.write(328, currentTimezone);
  EEPROM.write(329, useAMPM);
  EEPROM.write(330, strip.gammaCorrectBri);
  EEPROM.write(331, strip.gammaCorrectCol);
  EEPROM.write(332, overlayDefault);

  EEPROM.write(333, alexaEnabled);
  writeStringToEEPROM(334, alexaInvocationName, 32);
  EEPROM.write(366, notifyAlexa);

  EEPROM.write(367, (arlsOffset>=0));
  EEPROM.write(368, abs(arlsOffset));
  EEPROM.write(369, turnOnAtBoot);

  EEPROM.write(370, noWifiSleep);

  EEPROM.write(372, useRGBW);
  EEPROM.write(374, strip.paletteFade);
  EEPROM.write(375, strip.milliampsPerLed);
  EEPROM.write(376, apBehavior);

  EEPROM.write(377, EEPVER);

  EEPROM.write(382, strip.paletteBlend);
  EEPROM.write(383, strip.colorOrder);

  EEPROM.write(385, irEnabled);

  EEPROM.write(387, strip.ablMilliampsMax & 0xFF);
  EEPROM.write(388, (strip.ablMilliampsMax >> 8) & 0xFF);
  EEPROM.write(389, bootPreset);
  EEPROM.write(390, aOtaEnabled);
  EEPROM.write(391, receiveNotificationColor);
  EEPROM.write(392, receiveNotificationEffects);
  EEPROM.write(393, wifiLock);

  EEPROM.write(394, abs(utcOffsetSecs) & 0xFF);
  EEPROM.write(395, (abs(utcOffsetSecs) >> 8) & 0xFF);
  EEPROM.write(396, (utcOffsetSecs<0));
  EEPROM.write(397, syncToggleReceive);
  EEPROM.write(398, (ledCount >> 8) & 0xFF);




  writeStringToEEPROM(990, ntpServerName, 32);

  EEPROM.write(2048, huePollingEnabled);

  for (int i = 2050; i < 2054; ++i)
  {
    EEPROM.write(i, hueIP[i-2050]);
  }
  writeStringToEEPROM(2054, hueApiKey, 46);
  EEPROM.write(2100, huePollIntervalMs & 0xFF);
  EEPROM.write(2101, (huePollIntervalMs >> 8) & 0xFF);
  EEPROM.write(2102, notifyHue);
  EEPROM.write(2103, hueApplyOnOff);
  EEPROM.write(2104, hueApplyBri);
  EEPROM.write(2105, hueApplyColor);
  EEPROM.write(2106, huePollLightId);

  EEPROM.write(2150, overlayMin);
  EEPROM.write(2151, overlayMax);
  EEPROM.write(2152, analogClock12pixel);
  EEPROM.write(2153, analogClock5MinuteMarks);
  EEPROM.write(2154, analogClockSecondsTrail);

  EEPROM.write(2155, countdownMode);
  EEPROM.write(2156, countdownYear);
  EEPROM.write(2157, countdownMonth);
  EEPROM.write(2158, countdownDay);
  EEPROM.write(2159, countdownHour);
  EEPROM.write(2160, countdownMin);
  EEPROM.write(2161, countdownSec);
  setCountdown();

  writeStringToEEPROM(2165, cronixieDisplay, 6);
  EEPROM.write(2171, cronixieBacklight);
  setCronixie();

  EEPROM.write(2175, macroBoot);
  EEPROM.write(2176, macroAlexaOn);
  EEPROM.write(2177, macroAlexaOff);
  EEPROM.write(2178, macroButton);
  EEPROM.write(2179, macroLongPress);
  EEPROM.write(2180, macroCountdown);
  EEPROM.write(2181, macroNl);
  EEPROM.write(2182, macroDoublePress);

  EEPROM.write(2190, e131Universe & 0xFF);
  EEPROM.write(2191, (e131Universe >> 8) & 0xFF);
  EEPROM.write(2192, e131Multicast);
  EEPROM.write(2193, realtimeTimeoutMs & 0xFF);
  EEPROM.write(2194, (realtimeTimeoutMs >> 8) & 0xFF);
  EEPROM.write(2195, arlsForceMaxBri);
  EEPROM.write(2196, arlsDisableGammaCorrection);
  EEPROM.write(2197, DMXAddress & 0xFF);
  EEPROM.write(2198, (DMXAddress >> 8) & 0xFF);
  EEPROM.write(2199, DMXMode);

  EEPROM.write(2200, !receiveDirect);
  EEPROM.write(2201, notifyMacro);
  EEPROM.write(2203, strip.rgbwMode);
  EEPROM.write(2204, skipFirstLed);

  if (saveCurrPresetCycConf)
  {
    EEPROM.write(2205, presetCyclingEnabled);
    EEPROM.write(2206, presetCycleTime & 0xFF);
    EEPROM.write(2207, (presetCycleTime >> 8) & 0xFF);
    EEPROM.write(2208, presetCycleMin);
    EEPROM.write(2209, presetCycleMax);
    EEPROM.write(2210, presetApplyBri);
    EEPROM.write(2211, presetApplyCol);
    EEPROM.write(2212, presetApplyFx);
    saveCurrPresetCycConf = false;
  }

  writeStringToEEPROM(2220, blynkApiKey, 35);

  for (int i = 0; i < 8; ++i)
  {
    EEPROM.write(2260 + i, timerHours[i] );
    EEPROM.write(2270 + i, timerMinutes[i]);
    EEPROM.write(2280 + i, timerWeekday[i]);
    EEPROM.write(2290 + i, timerMacro[i] );
  }

  EEPROM.write(2299, mqttEnabled);
  writeStringToEEPROM(2300, mqttServer, 32);
  writeStringToEEPROM(2333, mqttDeviceTopic, 32);
  writeStringToEEPROM(2366, mqttGroupTopic, 32);
  writeStringToEEPROM(2399, mqttUser, 40);
  writeStringToEEPROM(2440, mqttPass, 40);
  writeStringToEEPROM(2481, mqttClientID, 40);
  EEPROM.write(2522, mqttPort & 0xFF);
  EEPROM.write(2523, (mqttPort >> 8) & 0xFF);


  #ifdef WLED_ENABLE_DMX
  EEPROM.write(2530, DMXChannels);
  EEPROM.write(2531, DMXGap & 0xFF);
  EEPROM.write(2532, (DMXGap >> 8) & 0xFF);
  EEPROM.write(2533, DMXStart & 0xFF);
  EEPROM.write(2534, (DMXStart >> 8) & 0xFF);

  for (int i=0; i<15; i++) {
    EEPROM.write(2535+i, DMXFixtureMap[i]);
  }
  #endif

  commit();
}





void loadSettingsFromEEPROM(bool first)
{
  if (EEPROM.read(233) != 233)
  {
    DEBUG_PRINT("Settings invalid, restoring defaults...");
    saveSettingsToEEPROM();
    DEBUG_PRINTLN("done");
    return;
  }
  int lastEEPROMversion = EEPROM.read(377);


  readStringFromEEPROM( 0, clientSSID, 32);
  readStringFromEEPROM( 32, clientPass, 64);
  readStringFromEEPROM( 96, cmDNS, 32);
  readStringFromEEPROM(128, apSSID, 32);
  readStringFromEEPROM(160, apPass, 64);

  nightlightDelayMinsDefault = EEPROM.read(224);
  nightlightDelayMins = nightlightDelayMinsDefault;
  nightlightFade = EEPROM.read(225);
  notifyDirectDefault = EEPROM.read(226);
  notifyDirect = notifyDirectDefault;

  apChannel = EEPROM.read(227);
  if (apChannel > 13 || apChannel < 1) apChannel = 1;
  apHide = EEPROM.read(228);
  if (apHide > 1) apHide = 1;
  ledCount = EEPROM.read(229) + ((EEPROM.read(398) << 8) & 0xFF00); if (ledCount > MAX_LEDS || ledCount == 0) ledCount = 30;

  notifyButton = EEPROM.read(230);
  notifyTwice = EEPROM.read(231);
  buttonEnabled = EEPROM.read(232);

  staticIP[0] = EEPROM.read(234);
  staticIP[1] = EEPROM.read(235);
  staticIP[2] = EEPROM.read(236);
  staticIP[3] = EEPROM.read(237);
  staticGateway[0] = EEPROM.read(238);
  staticGateway[1] = EEPROM.read(239);
  staticGateway[2] = EEPROM.read(240);
  staticGateway[3] = EEPROM.read(241);
  staticSubnet[0] = EEPROM.read(242);
  staticSubnet[1] = EEPROM.read(243);
  staticSubnet[2] = EEPROM.read(244);
  staticSubnet[3] = EEPROM.read(245);

  briS = EEPROM.read(249); bri = briS;
  if (!EEPROM.read(369) && first)
  {
    bri = 0; briLast = briS;
  }
  receiveNotificationBrightness = EEPROM.read(250);
  fadeTransition = EEPROM.read(251);
  strip.reverseMode = EEPROM.read(252);
  transitionDelayDefault = EEPROM.read(253) + ((EEPROM.read(254) << 8) & 0xFF00);
  transitionDelay = transitionDelayDefault;
  briMultiplier = EEPROM.read(255);

  readStringFromEEPROM(256, otaPass, 32);

  nightlightTargetBri = EEPROM.read(288);
  otaLock = EEPROM.read(289);
  udpPort = EEPROM.read(290) + ((EEPROM.read(291) << 8) & 0xFF00);

  readStringFromEEPROM(292, serverDescription, 32);

  ntpEnabled = EEPROM.read(327);
  currentTimezone = EEPROM.read(328);
  useAMPM = EEPROM.read(329);
  strip.gammaCorrectBri = EEPROM.read(330);
  strip.gammaCorrectCol = EEPROM.read(331);
  overlayDefault = EEPROM.read(332);
  if (lastEEPROMversion < 8 && overlayDefault > 0) overlayDefault--;

  alexaEnabled = EEPROM.read(333);

  readStringFromEEPROM(334, alexaInvocationName, 32);

  notifyAlexa = EEPROM.read(366);
  arlsOffset = EEPROM.read(368);
  if (!EEPROM.read(367)) arlsOffset = -arlsOffset;
  turnOnAtBoot = EEPROM.read(369);
  useRGBW = EEPROM.read(372);


  apBehavior = EEPROM.read(376);


  if (lastEEPROMversion > 3) {
    aOtaEnabled = EEPROM.read(390);
    receiveNotificationColor = EEPROM.read(391);
    receiveNotificationEffects = EEPROM.read(392);
  }
  receiveNotifications = (receiveNotificationBrightness || receiveNotificationColor || receiveNotificationEffects);

  if (lastEEPROMversion > 4) {
    huePollingEnabled = EEPROM.read(2048);

    for (int i = 2050; i < 2054; ++i)
    {
      hueIP[i-2050] = EEPROM.read(i);
    }

    readStringFromEEPROM(2054, hueApiKey, 46);

    huePollIntervalMs = EEPROM.read(2100) + ((EEPROM.read(2101) << 8) & 0xFF00);
    notifyHue = EEPROM.read(2102);
    hueApplyOnOff = EEPROM.read(2103);
    hueApplyBri = EEPROM.read(2104);
    hueApplyColor = EEPROM.read(2105);
    huePollLightId = EEPROM.read(2106);
  }
  if (lastEEPROMversion > 5) {
    overlayMin = EEPROM.read(2150);
    overlayMax = EEPROM.read(2151);
    analogClock12pixel = EEPROM.read(2152);
    analogClock5MinuteMarks = EEPROM.read(2153);
    analogClockSecondsTrail = EEPROM.read(2154);
    countdownMode = EEPROM.read(2155);
    countdownYear = EEPROM.read(2156);
    countdownMonth = EEPROM.read(2157);
    countdownDay = EEPROM.read(2158);
    countdownHour = EEPROM.read(2159);
    countdownMin = EEPROM.read(2160);
    countdownSec = EEPROM.read(2161);
    setCountdown();

    readStringFromEEPROM(2165, cronixieDisplay, 6);
    cronixieBacklight = EEPROM.read(2171);

    macroBoot = EEPROM.read(2175);
    macroAlexaOn = EEPROM.read(2176);
    macroAlexaOff = EEPROM.read(2177);
    macroButton = EEPROM.read(2178);
    macroLongPress = EEPROM.read(2179);
    macroCountdown = EEPROM.read(2180);
    macroNl = EEPROM.read(2181);
    macroDoublePress = EEPROM.read(2182);
    if (macroDoublePress > 16) macroDoublePress = 0;
  }

  if (lastEEPROMversion > 6)
  {
    e131Universe = EEPROM.read(2190) + ((EEPROM.read(2191) << 8) & 0xFF00);
    e131Multicast = EEPROM.read(2192);
    realtimeTimeoutMs = EEPROM.read(2193) + ((EEPROM.read(2194) << 8) & 0xFF00);
    arlsForceMaxBri = EEPROM.read(2195);
    arlsDisableGammaCorrection = EEPROM.read(2196);
  }

  if (lastEEPROMversion > 7)
  {
    strip.paletteFade = EEPROM.read(374);
    strip.paletteBlend = EEPROM.read(382);

    for (int i = 0; i < 8; ++i)
    {
      timerHours[i] = EEPROM.read(2260 + i);
      timerMinutes[i] = EEPROM.read(2270 + i);
      timerWeekday[i] = EEPROM.read(2280 + i);
      timerMacro[i] = EEPROM.read(2290 + i);
      if (timerWeekday[i] == 0) timerWeekday[i] = 255;
    }
  }

  if (lastEEPROMversion > 8)
  {
    readStringFromEEPROM(2300, mqttServer, 32);
    readStringFromEEPROM(2333, mqttDeviceTopic, 32);
    readStringFromEEPROM(2366, mqttGroupTopic, 32);
  }

  if (lastEEPROMversion > 9)
  {
    strip.colorOrder = EEPROM.read(383);
    irEnabled = EEPROM.read(385);
    strip.ablMilliampsMax = EEPROM.read(387) + ((EEPROM.read(388) << 8) & 0xFF00);
  } else if (lastEEPROMversion > 1)
  {
    strip.ablMilliampsMax = 65000;
  } else {
    strip.ablMilliampsMax = ABL_MILLIAMPS_DEFAULT;
  }

  if (lastEEPROMversion > 10)
  {
    readStringFromEEPROM(2399, mqttUser, 40);
    readStringFromEEPROM(2440, mqttPass, 40);
    readStringFromEEPROM(2481, mqttClientID, 40);
    mqttPort = EEPROM.read(2522) + ((EEPROM.read(2523) << 8) & 0xFF00);
  }

  if (lastEEPROMversion > 11)
  {
    strip.milliampsPerLed = EEPROM.read(375);
  } else if (strip.ablMilliampsMax == 65000)
  {
    strip.ablMilliampsMax = ABL_MILLIAMPS_DEFAULT;
    strip.milliampsPerLed = 0;
  }
  if (lastEEPROMversion > 12)
  {
    readStringFromEEPROM(990, ntpServerName, 32);
  }
  if (lastEEPROMversion > 13)
  {
    mqttEnabled = EEPROM.read(2299);
    syncToggleReceive = EEPROM.read(397);
  } else {
    mqttEnabled = true;
    syncToggleReceive = false;
  }

  if (lastEEPROMversion > 14)
  {
    DMXAddress = EEPROM.read(2197) + ((EEPROM.read(2198) << 8) & 0xFF00);
    DMXMode = EEPROM.read(2199);
  } else {
    DMXAddress = 1;
    DMXMode = DMX_MODE_MULTIPLE_RGB;
  }



    noWifiSleep = EEPROM.read(370);



  receiveDirect = !EEPROM.read(2200);
  notifyMacro = EEPROM.read(2201);

  strip.rgbwMode = EEPROM.read(2203);
  skipFirstLed = EEPROM.read(2204);

  if (EEPROM.read(2210) || EEPROM.read(2211) || EEPROM.read(2212))
  {
    presetCyclingEnabled = EEPROM.read(2205);
    presetCycleTime = EEPROM.read(2206) + ((EEPROM.read(2207) << 8) & 0xFF00);
    presetCycleMin = EEPROM.read(2208);
    presetCycleMax = EEPROM.read(2209);
    presetApplyBri = EEPROM.read(2210);
    presetApplyCol = EEPROM.read(2211);
    presetApplyFx = EEPROM.read(2212);
  }

  bootPreset = EEPROM.read(389);
  wifiLock = EEPROM.read(393);
  utcOffsetSecs = EEPROM.read(394) + ((EEPROM.read(395) << 8) & 0xFF00);
  if (EEPROM.read(396)) utcOffsetSecs = -utcOffsetSecs;
# 538 "C:/Users/RalfBruechmann/Documents/Arduino/code/WLED/wled00/wled01_eeprom.ino"
  readStringFromEEPROM(2220, blynkApiKey, 35);
  if (strlen(blynkApiKey) < 25) blynkApiKey[0] = 0;

  #ifdef WLED_ENABLE_DMX

  DMXChannels = EEPROM.read(2530);
  DMXGap = EEPROM.read(2531) + ((EEPROM.read(2532) << 8) & 0xFF00);
  DMXStart = EEPROM.read(2533) + ((EEPROM.read(2534) << 8) & 0xFF00);

  for (int i=0;i<15;i++) {
    DMXFixtureMap[i] = EEPROM.read(2535+i);
  }
  #endif




  overlayCurrent = overlayDefault;

  savedToPresets();
}






void savedToPresets()
{
  for (byte index = 1; index < 16; index++)
  {
    uint16_t i = 380 + index*20;

    if (EEPROM.read(i) == 1) {
      savedPresets |= 0x01 << (index-1);
    } else
    {
      savedPresets &= ~(0x01 << (index-1));
    }
  }
  if (EEPROM.read(700) == 2) {
    savedPresets |= 0x01 << 15;
  } else
  {
    savedPresets &= ~(0x01 << 15);
  }
}

bool applyPreset(byte index, bool loadBri = true, bool loadCol = true, bool loadFX = true)
{
  if (index == 255 || index == 0)
  {
    loadSettingsFromEEPROM(false);
    return true;
  }
  if (index > 16 || index < 1) return false;
  uint16_t i = 380 + index*20;
  if (index < 16) {
    if (EEPROM.read(i) != 1) return false;
    strip.applyToAllSelected = true;
    if (loadBri) bri = EEPROM.read(i+1);
    if (loadCol)
    {
      for (byte j=0; j<4; j++)
      {
        col[j] = EEPROM.read(i+j+2);
        colSec[j] = EEPROM.read(i+j+6);
      }
      strip.setColor(2, EEPROM.read(i+12), EEPROM.read(i+13), EEPROM.read(i+14), EEPROM.read(i+15));
    }
    if (loadFX)
    {
      effectCurrent = EEPROM.read(i+10);
      effectSpeed = EEPROM.read(i+11);
      effectIntensity = EEPROM.read(i+16);
      effectPalette = EEPROM.read(i+17);
    }
  } else {
    if (EEPROM.read(i) != 2) return false;
    strip.applyToAllSelected = false;
    if (loadBri) bri = EEPROM.read(i+1);
    WS2812FX::Segment* seg = strip.getSegments();
    memcpy(seg, EEPROM.getDataPtr() +i+2, 240);
    setValuesFromMainSeg();
  }
  currentPreset = index;
  isPreset = true;
  return true;
}

void savePreset(byte index)
{
  if (index > 16) return;
  if (index < 1) {saveSettingsToEEPROM();return;}
  uint16_t i = 380 + index*20;

  if (index < 16) {
    EEPROM.write(i, 1);
    EEPROM.write(i+1, bri);
    for (uint16_t j=0; j<4; j++)
    {
      EEPROM.write(i+j+2, col[j]);
      EEPROM.write(i+j+6, colSec[j]);
    }
    EEPROM.write(i+10, effectCurrent);
    EEPROM.write(i+11, effectSpeed);

    uint32_t colTer = strip.getSegment(strip.getMainSegmentId()).colors[2];
    EEPROM.write(i+12, (colTer >> 16) & 0xFF);
    EEPROM.write(i+13, (colTer >> 8) & 0xFF);
    EEPROM.write(i+14, (colTer >> 0) & 0xFF);
    EEPROM.write(i+15, (colTer >> 24) & 0xFF);

    EEPROM.write(i+16, effectIntensity);
    EEPROM.write(i+17, effectPalette);
  } else {
    EEPROM.write(i, 2);
    EEPROM.write(i+1, bri);
    WS2812FX::Segment* seg = strip.getSegments();
    memcpy(EEPROM.getDataPtr() +i+2, seg, 240);
  }

  commit();
  savedToPresets();
  currentPreset = index;
  isPreset = true;
}


void loadMacro(byte index, char* m)
{
  index-=1;
  if (index > 15) return;
  readStringFromEEPROM(1024+64*index, m, 64);
}


void applyMacro(byte index)
{
  index-=1;
  if (index > 15) return;
  String mc="win&";
  char m[65];
  loadMacro(index+1, m);
  mc += m;
  mc += "&IN";
  if (!notifyMacro) mc += "&NN";
  String forbidden = "&M=";




  forbidden = forbidden + index;
  if (mc.indexOf(forbidden) >= 0) return;
  handleSet(nullptr, mc);
}


void saveMacro(byte index, String mc, bool sing=true)
{
  index-=1;
  if (index > 15) return;
  int s = 1024+index*64;
  for (int i = s; i < s+64; i++)
  {
    EEPROM.write(i, mc.charAt(i-s));
  }
  if (sing) commit();
}
# 1 "C:/Users/RalfBruechmann/Documents/Arduino/code/WLED/wled00/wled02_xml.ino"





char* XML_response(AsyncWebServerRequest *request, char* dest = nullptr)
{
  char sbuf[(dest == nullptr)?1024:1];
  obuf = (dest == nullptr)? sbuf:dest;

  olen = 0;
  oappend((const char*)F("<?xml version=\"1.0\" ?><vs><ac>"));
  oappendi((nightlightActive && nightlightFade) ? briT : bri);
  oappend("</ac>");

  for (int i = 0; i < 3; i++)
  {
   oappend("<cl>");
   oappendi(col[i]);
   oappend("</cl>");
  }
  for (int i = 0; i < 3; i++)
  {
   oappend("<cs>");
   oappendi(colSec[i]);
   oappend("</cs>");
  }
  oappend("<ns>");
  oappendi(notifyDirect);
  oappend("</ns><nr>");
  oappendi(receiveNotifications);
  oappend("</nr><nl>");
  oappendi(nightlightActive);
  oappend("</nl><nf>");
  oappendi(nightlightFade);
  oappend("</nf><nd>");
  oappendi(nightlightDelayMins);
  oappend("</nd><nt>");
  oappendi(nightlightTargetBri);
  oappend("</nt><fx>");
  oappendi(effectCurrent);
  oappend("</fx><sx>");
  oappendi(effectSpeed);
  oappend("</sx><ix>");
  oappendi(effectIntensity);
  oappend("</ix><fp>");
  oappendi(effectPalette);
  oappend("</fp><wv>");
  if (strip.rgbwMode) {
   oappendi(col[3]);
  } else {
   oappend("-1");
  }
  oappend("</wv><ws>");
  oappendi(colSec[3]);
  oappend("</ws><ps>");
  oappendi((currentPreset < 1) ? 0:currentPreset);
  oappend("</ps><cy>");
  oappendi(presetCyclingEnabled);
  oappend("</cy><ds>");
  if (realtimeMode)
  {
    String mesg = "Live ";
    if (realtimeMode == REALTIME_MODE_E131)
    {
      mesg += "E1.31 mode ";
      mesg += DMXMode;
      mesg += F(" at DMX Address ");
      mesg += DMXAddress;
      mesg += " from ";
      mesg += realtimeIP[0];
      for (int i = 1; i < 4; i++)
      {
        mesg += ".";
        mesg += realtimeIP[i];
      }
      mesg += " seq=";
      mesg += e131LastSequenceNumber;
    } else if (realtimeMode == REALTIME_MODE_UDP || realtimeMode == REALTIME_MODE_HYPERION) {
      mesg += "UDP from ";
      mesg += realtimeIP[0];
      for (int i = 1; i < 4; i++)
      {
        mesg += ".";
        mesg += realtimeIP[i];
      }
    } else if (realtimeMode == REALTIME_MODE_ADALIGHT) {
      mesg += F("USB Adalight");
    } else {
      mesg += "data";
    }
    oappend((char*)mesg.c_str());
  } else {
    oappend(serverDescription);
  }
  oappend("</ds><ss>");
  oappendi(strip.getMainSegmentId());
  oappend("</ss></vs>");
  if (request != nullptr) request->send(200, "text/xml", obuf);
}

char* URL_response(AsyncWebServerRequest *request)
{
  char sbuf[256];
  char s2buf[100];
  obuf = s2buf;
  olen = 0;

  char s[16];
  oappend("http://");
  IPAddress localIP = WiFi.localIP();
  sprintf(s, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);
  oappend(s);
  oappend("/win&A=");
  oappendi(bri);
  oappend("&CL=h");
  for (int i = 0; i < 3; i++)
  {
   sprintf(s,"%02X", col[i]);
   oappend(s);
  }
  oappend("&C2=h");
  for (int i = 0; i < 3; i++)
  {
   sprintf(s,"%02X", colSec[i]);
   oappend(s);
  }
  oappend("&FX=");
  oappendi(effectCurrent);
  oappend("&SX=");
  oappendi(effectSpeed);
  oappend("&IX=");
  oappendi(effectIntensity);
  oappend("&FP=");
  oappendi(effectPalette);

  obuf = sbuf;
  olen = 0;

  oappend((const char*)F("<html><body><a href=\""));
  oappend(s2buf);
  oappend((const char*)F("\" target=\"_blank\">"));
  oappend(s2buf);
  oappend((const char*)F("</a></body></html>"));

  if (request != nullptr) request->send(200, "text/html", obuf);
}


void sappend(char stype, const char* key, int val)
{
  char ds[] = "d.Sf.";

  switch(stype)
  {
    case 'c':
      oappend(ds);
      oappend(key);
      oappend(".checked=");
      oappendi(val);
      oappend(";");
      break;
    case 'v':
      oappend(ds);
      oappend(key);
      oappend(".value=");
      oappendi(val);
      oappend(";");
      break;
    case 'i':
      oappend(ds);
      oappend(key);
      oappend(".selectedIndex=");
      oappendi(val);
      oappend(";");
      break;
  }
}


void sappends(char stype, const char* key, char* val)
{
  switch(stype)
  {
    case 's':
      oappend("d.Sf.");
      oappend(key);
      oappend(".value=\"");
      oappend(val);
      oappend("\";");
      break;
    case 'm':
      oappend("d.getElementsByClassName");
      oappend(key);
      oappend(".innerHTML=\"");
      oappend(val);
      oappend("\";");
      break;
  }
}



void getSettingsJS(byte subPage, char* dest)
{

  DEBUG_PRINT("settings resp");
  DEBUG_PRINTLN(subPage);
  obuf = dest;
  olen = 0;

  if (subPage <1 || subPage >7) return;

  if (subPage == 1) {
    sappends('s',"CS",clientSSID);

    byte l = strlen(clientPass);
    char fpass[l+1];
    fpass[l] = 0;
    memset(fpass,'*',l);
    sappends('s',"CP",fpass);

    char k[3]; k[2] = 0;
    for (int i = 0; i<4; i++)
    {
      k[1] = 48+i;
      k[0] = 'I'; sappend('v',k,staticIP[i]);
      k[0] = 'G'; sappend('v',k,staticGateway[i]);
      k[0] = 'S'; sappend('v',k,staticSubnet[i]);
    }

    sappends('s',"CM",cmDNS);
    sappend('i',"AB",apBehavior);
    sappends('s',"AS",apSSID);
    sappend('c',"AH",apHide);

    l = strlen(apPass);
    char fapass[l+1];
    fapass[l] = 0;
    memset(fapass,'*',l);
    sappends('s',"AP",fapass);

    sappend('v',"AC",apChannel);
    sappend('c',"WS",noWifiSleep);


    if (WiFi.localIP()[0] != 0)
    {
      char s[16];
      IPAddress localIP = WiFi.localIP();
      sprintf(s, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);
      sappends('m',"(\"sip\")[0]",s);
    } else
    {
      sappends('m',"(\"sip\")[0]","Not connected");
    }

    if (WiFi.softAPIP()[0] != 0)
    {
      char s[16];
      IPAddress apIP = WiFi.softAPIP();
      sprintf(s, "%d.%d.%d.%d", apIP[0], apIP[1], apIP[2], apIP[3]);
      sappends('m',"(\"sip\")[1]",s);
    } else
    {
      sappends('m',"(\"sip\")[1]","Not active");
    }
  }

  if (subPage == 2) {
    #ifdef ESP8266
    #if LEDPIN == 3
    oappend("d.Sf.LC.max=500;");
    #endif
    #endif
    sappend('v',"LC",ledCount);
    sappend('v',"MA",strip.ablMilliampsMax);
    sappend('v',"LA",strip.milliampsPerLed);
    if (strip.currentMilliamps)
    {
      sappends('m',"(\"pow\")[0]","");
      olen -= 2;
      oappendi(strip.currentMilliamps);
      oappend("mA\";");
    }

    sappend('v',"CA",briS);
    sappend('c',"EW",useRGBW);
    sappend('i',"CO",strip.colorOrder);
    sappend('v',"AW",strip.rgbwMode);

    sappend('c',"BO",turnOnAtBoot);
    sappend('v',"BP",bootPreset);

    sappend('c',"GB",strip.gammaCorrectBri);
    sappend('c',"GC",strip.gammaCorrectCol);
    sappend('c',"TF",fadeTransition);
    sappend('v',"TD",transitionDelayDefault);
    sappend('c',"PF",strip.paletteFade);
    sappend('v',"BF",briMultiplier);
    sappend('v',"TB",nightlightTargetBri);
    sappend('v',"TL",nightlightDelayMinsDefault);
    sappend('c',"TW",nightlightFade);
    sappend('i',"PB",strip.paletteBlend);
    sappend('c',"RV",strip.reverseMode);
    sappend('c',"SL",skipFirstLed);
  }

  if (subPage == 3)
  {
    sappends('s',"DS",serverDescription);
    sappend('c',"ST",syncToggleReceive);
  }

  if (subPage == 4)
  {
    sappend('c',"BT",buttonEnabled);
    sappend('v',"IR",irEnabled);
    sappend('v',"UP",udpPort);
    sappend('c',"RB",receiveNotificationBrightness);
    sappend('c',"RC",receiveNotificationColor);
    sappend('c',"RX",receiveNotificationEffects);
    sappend('c',"SD",notifyDirectDefault);
    sappend('c',"SB",notifyButton);
    sappend('c',"SH",notifyHue);
    sappend('c',"SM",notifyMacro);
    sappend('c',"S2",notifyTwice);
    sappend('c',"RD",receiveDirect);
    sappend('c',"EM",e131Multicast);
    sappend('v',"EU",e131Universe);
    sappend('v',"DA",DMXAddress);
    sappend('v',"DM",DMXMode);
    sappend('v',"ET",realtimeTimeoutMs);
    sappend('c',"FB",arlsForceMaxBri);
    sappend('c',"RG",arlsDisableGammaCorrection);
    sappend('v',"WO",arlsOffset);
    sappend('c',"AL",alexaEnabled);
    sappends('s',"AI",alexaInvocationName);
    sappend('c',"SA",notifyAlexa);
    sappends('s',"BK",(char*)((blynkEnabled)?"Hidden":""));

    #ifdef WLED_ENABLE_MQTT
    sappend('c',"MQ",mqttEnabled);
    sappends('s',"MS",mqttServer);
    sappend('v',"MQPORT",mqttPort);
    sappends('s',"MQUSER",mqttUser);
    sappends('s',"MQPASS",mqttPass);
    byte l = strlen(mqttPass);
    char fpass[l+1];
    fpass[l] = 0;
    memset(fpass,'*',l);
    sappends('s',"MQPASS",fpass);
    sappends('s',"MQCID",mqttClientID);
    sappends('s',"MD",mqttDeviceTopic);
    sappends('s',"MG",mqttGroupTopic);
    #endif

    #ifndef WLED_DISABLE_HUESYNC
    sappend('v',"H0",hueIP[0]);
    sappend('v',"H1",hueIP[1]);
    sappend('v',"H2",hueIP[2]);
    sappend('v',"H3",hueIP[3]);
    sappend('v',"HL",huePollLightId);
    sappend('v',"HI",huePollIntervalMs);
    sappend('c',"HP",huePollingEnabled);
    sappend('c',"HO",hueApplyOnOff);
    sappend('c',"HB",hueApplyBri);
    sappend('c',"HC",hueApplyColor);
    char hueErrorString[25];
    switch (hueError)
    {
      case HUE_ERROR_INACTIVE : strcpy(hueErrorString,(char*)F("Inactive")); break;
      case HUE_ERROR_ACTIVE : strcpy(hueErrorString,(char*)F("Active")); break;
      case HUE_ERROR_UNAUTHORIZED : strcpy(hueErrorString,(char*)F("Unauthorized")); break;
      case HUE_ERROR_LIGHTID : strcpy(hueErrorString,(char*)F("Invalid light ID")); break;
      case HUE_ERROR_PUSHLINK : strcpy(hueErrorString,(char*)F("Link button not pressed")); break;
      case HUE_ERROR_JSON_PARSING : strcpy(hueErrorString,(char*)F("JSON parsing error")); break;
      case HUE_ERROR_TIMEOUT : strcpy(hueErrorString,(char*)F("Timeout")); break;
      default: sprintf(hueErrorString,"Bridge Error %i",hueError);
    }

    sappends('m',"(\"hms\")[0]",hueErrorString);
    #endif
  }

  if (subPage == 5)
  {
    sappend('c',"NT",ntpEnabled);
    sappends('s',"NS",ntpServerName);
    sappend('c',"CF",!useAMPM);
    sappend('i',"TZ",currentTimezone);
    sappend('v',"UO",utcOffsetSecs);
    char tm[32];
    getTimeString(tm);
    sappends('m',"(\"times\")[0]",tm);
    sappend('i',"OL",overlayCurrent);
    sappend('v',"O1",overlayMin);
    sappend('v',"O2",overlayMax);
    sappend('v',"OM",analogClock12pixel);
    sappend('c',"OS",analogClockSecondsTrail);
    sappend('c',"O5",analogClock5MinuteMarks);
    sappends('s',"CX",cronixieDisplay);
    sappend('c',"CB",cronixieBacklight);
    sappend('c',"CE",countdownMode);
    sappend('v',"CY",countdownYear);
    sappend('v',"CI",countdownMonth);
    sappend('v',"CD",countdownDay);
    sappend('v',"CH",countdownHour);
    sappend('v',"CM",countdownMin);
    sappend('v',"CS",countdownSec);
    char k[4]; k[0]= 'M';
    for (int i=1;i<17;i++)
    {
      char m[65];
      loadMacro(i, m);
      sprintf(k+1,"%i",i);
      sappends('s',k,m);
    }

    sappend('v',"MB",macroBoot);
    sappend('v',"A0",macroAlexaOn);
    sappend('v',"A1",macroAlexaOff);
    sappend('v',"MP",macroButton);
    sappend('v',"ML",macroLongPress);
    sappend('v',"MC",macroCountdown);
    sappend('v',"MN",macroNl);
    sappend('v',"MD",macroDoublePress);

    k[2] = 0;
    for (int i = 0; i<8; i++)
    {
      k[1] = 48+i;
      k[0] = 'H'; sappend('v',k,timerHours[i]);
      k[0] = 'N'; sappend('v',k,timerMinutes[i]);
      k[0] = 'T'; sappend('v',k,timerMacro[i]);
      k[0] = 'W'; sappend('v',k,timerWeekday[i]);
    }
  }

  if (subPage == 6)
  {
    sappend('c',"NO",otaLock);
    sappend('c',"OW",wifiLock);
    sappend('c',"AO",aOtaEnabled);
    sappends('m',"(\"msg\")[0]","WLED ");
    olen -= 2;
    oappend(versionString);
    oappend(" (build ");
    oappendi(VERSION);
    oappend(") OK\";");
  }

  #ifdef WLED_ENABLE_DMX
  if (subPage == 7)
  {
    sappend('v',"CN",DMXChannels);
    sappend('v',"CG",DMXGap);
    sappend('v',"CS",DMXStart);

    sappend('i',"CH1",DMXFixtureMap[0]);
    sappend('i',"CH2",DMXFixtureMap[1]);
    sappend('i',"CH3",DMXFixtureMap[2]);
    sappend('i',"CH4",DMXFixtureMap[3]);
    sappend('i',"CH5",DMXFixtureMap[4]);
    sappend('i',"CH6",DMXFixtureMap[5]);
    sappend('i',"CH7",DMXFixtureMap[6]);
    sappend('i',"CH8",DMXFixtureMap[7]);
    sappend('i',"CH9",DMXFixtureMap[8]);
    sappend('i',"CH10",DMXFixtureMap[9]);
    sappend('i',"CH11",DMXFixtureMap[10]);
    sappend('i',"CH12",DMXFixtureMap[11]);
    sappend('i',"CH13",DMXFixtureMap[12]);
    sappend('i',"CH14",DMXFixtureMap[13]);
    sappend('i',"CH15",DMXFixtureMap[14]);
    }
  #endif
  oappend("}</script>");
}
# 1 "C:/Users/RalfBruechmann/Documents/Arduino/code/WLED/wled00/wled03_set.ino"




void _setRandomColor(bool _sec,bool fromButton=false)
{
  lastRandomIndex = strip.get_random_wheel_index(lastRandomIndex);
  if (_sec){
    colorHStoRGB(lastRandomIndex*256,255,colSec);
  } else {
    colorHStoRGB(lastRandomIndex*256,255,col);
  }
  if (fromButton) colorUpdated(2);
}


bool isAsterisksOnly(const char* str, byte maxLen)
{
  for (byte i = 0; i < maxLen; i++) {
    if (str[i] == 0) break;
    if (str[i] != '*') return false;
  }
  return true;
}



void handleSettingsSet(AsyncWebServerRequest *request, byte subPage)
{


  if (subPage <1 || subPage >7) return;


  if (subPage == 1)
  {
    strlcpy(clientSSID,request->arg("CS").c_str(), 33);

    if (!isAsterisksOnly(request->arg("CP").c_str(), 65)) strlcpy(clientPass, request->arg("CP").c_str(), 65);

    strlcpy(cmDNS, request->arg("CM").c_str(), 33);

    apBehavior = request->arg("AB").toInt();
    strlcpy(apSSID, request->arg("AS").c_str(), 33);
    apHide = request->hasArg("AH");
    int passlen = request->arg("AP").length();
    if (passlen == 0 || (passlen > 7 && !isAsterisksOnly(request->arg("AP").c_str(), 65))) strlcpy(apPass, request->arg("AP").c_str(), 65);
    int t = request->arg("AC").toInt(); if (t > 0 && t < 14) apChannel = t;

    noWifiSleep = request->hasArg("WS");

    char k[3]; k[2] = 0;
    for (int i = 0; i<4; i++)
    {
      k[1] = i+48;

      k[0] = 'I';
      staticIP[i] = request->arg(k).toInt();

      k[0] = 'G';
      staticGateway[i] = request->arg(k).toInt();

      k[0] = 'S';
      staticSubnet[i] = request->arg(k).toInt();
    }
  }


  if (subPage == 2)
  {
    int t = request->arg("LC").toInt();
    if (t > 0 && t <= MAX_LEDS) ledCount = t;
    #ifdef ESP8266
    #if LEDPIN == 3
    if (ledCount > MAX_LEDS_DMA) ledCount = MAX_LEDS_DMA;
    #endif
    #endif
    strip.ablMilliampsMax = request->arg("MA").toInt();
    strip.milliampsPerLed = request->arg("LA").toInt();

    useRGBW = request->hasArg("EW");
    strip.colorOrder = request->arg("CO").toInt();
    strip.rgbwMode = request->arg("AW").toInt();

    briS = request->arg("CA").toInt();

    saveCurrPresetCycConf = request->hasArg("PC");
    turnOnAtBoot = request->hasArg("BO");
    t = request->arg("BP").toInt();
    if (t <= 25) bootPreset = t;
    strip.gammaCorrectBri = request->hasArg("GB");
    strip.gammaCorrectCol = request->hasArg("GC");

    fadeTransition = request->hasArg("TF");
    t = request->arg("TD").toInt();
    if (t > 0) transitionDelay = t;
    transitionDelayDefault = t;
    strip.paletteFade = request->hasArg("PF");

    nightlightTargetBri = request->arg("TB").toInt();
    t = request->arg("TL").toInt();
    if (t > 0) nightlightDelayMinsDefault = t;
    nightlightDelayMins = nightlightDelayMinsDefault;
    nightlightFade = request->hasArg("TW");

    t = request->arg("PB").toInt();
    if (t >= 0 && t < 4) strip.paletteBlend = t;
    strip.reverseMode = request->hasArg("RV");
    skipFirstLed = request->hasArg("SL");
    t = request->arg("BF").toInt();
    if (t > 0) briMultiplier = t;
  }


  if (subPage == 3)
  {
    strlcpy(serverDescription, request->arg("DS").c_str(), 33);
    syncToggleReceive = request->hasArg("ST");
  }


  if (subPage == 4)
  {
    buttonEnabled = request->hasArg("BT");
    irEnabled = request->arg("IR").toInt();
    int t = request->arg("UP").toInt();
    if (t > 0) udpPort = t;
    receiveNotificationBrightness = request->hasArg("RB");
    receiveNotificationColor = request->hasArg("RC");
    receiveNotificationEffects = request->hasArg("RX");
    receiveNotifications = (receiveNotificationBrightness || receiveNotificationColor || receiveNotificationEffects);
    notifyDirectDefault = request->hasArg("SD");
    notifyDirect = notifyDirectDefault;
    notifyButton = request->hasArg("SB");
    notifyAlexa = request->hasArg("SA");
    notifyHue = request->hasArg("SH");
    notifyMacro = request->hasArg("SM");
    notifyTwice = request->hasArg("S2");

    receiveDirect = request->hasArg("RD");
    e131Multicast = request->hasArg("EM");
    t = request->arg("EU").toInt();
    if (t > 0 && t <= 63999) e131Universe = t;
    t = request->arg("DA").toInt();
    if (t > 0 && t <= 510) DMXAddress = t;
    t = request->arg("DM").toInt();
    if (t >= DMX_MODE_DISABLED && t <= DMX_MODE_MULTIPLE_DRGB) DMXMode = t;
    t = request->arg("ET").toInt();
    if (t > 99 && t <= 65000) realtimeTimeoutMs = t;
    arlsForceMaxBri = request->hasArg("FB");
    arlsDisableGammaCorrection = request->hasArg("RG");
    t = request->arg("WO").toInt();
    if (t >= -255 && t <= 255) arlsOffset = t;

    alexaEnabled = request->hasArg("AL");
    strlcpy(alexaInvocationName, request->arg("AI").c_str(), 33);

    if (request->hasArg("BK") && !request->arg("BK").equals("Hidden")) {
      strlcpy(blynkApiKey, request->arg("BK").c_str(), 36); initBlynk(blynkApiKey);
    }

    #ifdef WLED_ENABLE_MQTT
    mqttEnabled = request->hasArg("MQ");
    strlcpy(mqttServer, request->arg("MS").c_str(), 33);
    t = request->arg("MQPORT").toInt();
    if (t > 0) mqttPort = t;
    strlcpy(mqttUser, request->arg("MQUSER").c_str(), 41);
    if (!isAsterisksOnly(request->arg("MQPASS").c_str(), 41)) strlcpy(mqttPass, request->arg("MQPASS").c_str(), 41);
    strlcpy(mqttClientID, request->arg("MQCID").c_str(), 41);
    strlcpy(mqttDeviceTopic, request->arg("MD").c_str(), 33);
    strlcpy(mqttGroupTopic, request->arg("MG").c_str(), 33);
    #endif

    #ifndef WLED_DISABLE_HUESYNC
    for (int i=0;i<4;i++){
      String a = "H"+String(i);
      hueIP[i] = request->arg(a).toInt();
    }

    t = request->arg("HL").toInt();
    if (t > 0) huePollLightId = t;

    t = request->arg("HI").toInt();
    if (t > 50) huePollIntervalMs = t;

    hueApplyOnOff = request->hasArg("HO");
    hueApplyBri = request->hasArg("HB");
    hueApplyColor = request->hasArg("HC");
    huePollingEnabled = request->hasArg("HP");
    hueStoreAllowed = true;
    reconnectHue();
    #endif
  }


  if (subPage == 5)
  {
    ntpEnabled = request->hasArg("NT");
    strlcpy(ntpServerName, request->arg("NS").c_str(), 33);
    useAMPM = !request->hasArg("CF");
    currentTimezone = request->arg("TZ").toInt();
    utcOffsetSecs = request->arg("UO").toInt();


    if (ntpEnabled && WLED_CONNECTED && !ntpConnected) ntpConnected = ntpUdp.begin(ntpLocalPort);

    if (request->hasArg("OL")){
      overlayDefault = request->arg("OL").toInt();
      overlayCurrent = overlayDefault;
    }

    overlayMin = request->arg("O1").toInt();
    overlayMax = request->arg("O2").toInt();
    analogClock12pixel = request->arg("OM").toInt();
    analogClock5MinuteMarks = request->hasArg("O5");
    analogClockSecondsTrail = request->hasArg("OS");

    strcpy(cronixieDisplay,request->arg("CX").c_str());
    bool cbOld = cronixieBacklight;
    cronixieBacklight = request->hasArg("CB");
    if (cbOld != cronixieBacklight && overlayCurrent == 3)
    {
      strip.setCronixieBacklight(cronixieBacklight); overlayRefreshedTime = 0;
    }
    countdownMode = request->hasArg("CE");
    countdownYear = request->arg("CY").toInt();
    countdownMonth = request->arg("CI").toInt();
    countdownDay = request->arg("CD").toInt();
    countdownHour = request->arg("CH").toInt();
    countdownMin = request->arg("CM").toInt();
    countdownSec = request->arg("CS").toInt();

    for (int i=1;i<17;i++)
    {
      String a = "M"+String(i);
      if (request->hasArg(a.c_str())) saveMacro(i,request->arg(a),false);
    }

    macroBoot = request->arg("MB").toInt();
    macroAlexaOn = request->arg("A0").toInt();
    macroAlexaOff = request->arg("A1").toInt();
    macroButton = request->arg("MP").toInt();
    macroLongPress = request->arg("ML").toInt();
    macroCountdown = request->arg("MC").toInt();
    macroNl = request->arg("MN").toInt();
    macroDoublePress = request->arg("MD").toInt();

    char k[3]; k[2] = 0;
    for (int i = 0; i<8; i++)
    {
      k[1] = i+48;

      k[0] = 'H';
      timerHours[i] = request->arg(k).toInt();

      k[0] = 'N';
      timerMinutes[i] = request->arg(k).toInt();

      k[0] = 'T';
      timerMacro[i] = request->arg(k).toInt();

      k[0] = 'W';
      timerWeekday[i] = request->arg(k).toInt();
    }
  }


  if (subPage == 6)
  {
    if (request->hasArg("RS"))
    {
      clearEEPROM();
      serveMessage(request, 200, "All Settings erased.", "Connect to WLED-AP to setup again",255);
      doReboot = true;
    }

    bool pwdCorrect = !otaLock;
    if (request->hasArg("OP"))
    {
      if (otaLock && strcmp(otaPass,request->arg("OP").c_str()) == 0)
      {
        pwdCorrect = true;
      }
      if (!otaLock && request->arg("OP").length() > 0)
      {
        strlcpy(otaPass,request->arg("OP").c_str(), 33);
      }
    }

    if (pwdCorrect)
    {
      otaLock = request->hasArg("NO");
      wifiLock = request->hasArg("OW");
      aOtaEnabled = request->hasArg("AO");
    }
  }
  #ifdef WLED_ENABLE_DMX
  if (subPage == 7)
  {
    int t = request->arg("CN").toInt();
    if (t>0 && t<16) {
      DMXChannels = t;
    }
    t = request->arg("CS").toInt();
    if (t>0 && t<513) {
      DMXStart = t;
    }
    t = request->arg("CG").toInt();
    if (t>0 && t<513) {
      DMXGap = t;
    }
    for (int i=0; i<15; i++) {
      String argname = "CH" + String((i+1));
      t = request->arg(argname).toInt();
      DMXFixtureMap[i] = t;
    }
  }

  #endif
  if (subPage != 6 || !doReboot) saveSettingsToEEPROM();
  if (subPage == 2) {
    strip.init(useRGBW,ledCount,skipFirstLed);
  }
  if (subPage == 4) alexaInit();
}




int getNumVal(const String* req, uint16_t pos)
{
  return req->substring(pos+3).toInt();
}



bool updateVal(const String* req, const char* key, byte* val, byte minv=0, byte maxv=255)
{
  int pos = req->indexOf(key);
  if (pos < 1) return false;

  if (req->charAt(pos+3) == '~') {
    int out = getNumVal(req, pos+1);
    if (out == 0)
    {
      if (req->charAt(pos+4) == '-')
      {
        *val = (*val <= minv)? maxv : *val -1;
      } else {
        *val = (*val >= maxv)? minv : *val +1;
      }
    } else {
      out += *val;
      if (out > maxv) out = maxv;
      if (out < minv) out = minv;
      *val = out;
    }
  } else
  {
    *val = getNumVal(req, pos);
  }
  return true;
}



bool handleSet(AsyncWebServerRequest *request, const String& req)
{
  if (!(req.indexOf("win") >= 0)) return false;

  int pos = 0;
  DEBUG_PRINT("API req: ");
  DEBUG_PRINTLN(req);


  pos = req.indexOf("&MS=");
  if (pos > 0) {
    int i = req.substring(pos + 4).toInt();
    pos = req.indexOf('(') +1;
    if (pos > 0) {
      int en = req.indexOf(')');
      String mc = req.substring(pos);
      if (en > 0) mc = req.substring(pos, en);
      saveMacro(i, mc);
    }

    pos = req.indexOf("IN");
    if (pos < 1) XML_response(request);
    return true;

  }

  strip.applyToAllSelected = true;


  byte prevMain = strip.getMainSegmentId();
  pos = req.indexOf("SM=");
  if (pos > 0) {
    strip.mainSegment = getNumVal(&req, pos);
  }
  byte main = strip.getMainSegmentId();
  if (main != prevMain) setValuesFromMainSeg();

  pos = req.indexOf("SS=");
  if (pos > 0) {
    byte t = getNumVal(&req, pos);
    if (t < strip.getMaxSegments()) main = t;
  }

  WS2812FX::Segment& mainseg = strip.getSegment(main);
  pos = req.indexOf("SV=");
  if (pos > 0) mainseg.setOption(0, (req.charAt(pos+3) != '0'));

  uint16_t startI = mainseg.start;
  uint16_t stopI = mainseg.stop;
  uint8_t grpI = mainseg.grouping;
  uint16_t spcI = mainseg.spacing;
  pos = req.indexOf("&S=");
  if (pos > 0) {
    startI = getNumVal(&req, pos);
  }
  pos = req.indexOf("S2=");
  if (pos > 0) {
    stopI = getNumVal(&req, pos);
  }
  pos = req.indexOf("GP=");
  if (pos > 0) {
    grpI = getNumVal(&req, pos);
    if (grpI == 0) grpI = 1;
  }
  pos = req.indexOf("SP=");
  if (pos > 0) {
    spcI = getNumVal(&req, pos);
  }
  strip.setSegment(main, startI, stopI, grpI, spcI);

  main = strip.getMainSegmentId();


  pos = req.indexOf("P1=");
  if (pos > 0) presetCycleMin = getNumVal(&req, pos);

  pos = req.indexOf("P2=");
  if (pos > 0) presetCycleMax = getNumVal(&req, pos);


  pos = req.indexOf("CY=");
  if (pos > 0)
  {
    presetCyclingEnabled = (req.charAt(pos+3) != '0');
    presetCycCurr = presetCycleMin;
  }

  pos = req.indexOf("PT=");
  if (pos > 0) {
    int v = getNumVal(&req, pos);
    if (v > 49) presetCycleTime = v;
  }

  pos = req.indexOf("PA=");
  if (pos > 0) presetApplyBri = (req.charAt(pos+3) != '0');

  pos = req.indexOf("PC=");
  if (pos > 0) presetApplyCol = (req.charAt(pos+3) != '0');

  pos = req.indexOf("PX=");
  if (pos > 0) presetApplyFx = (req.charAt(pos+3) != '0');

  pos = req.indexOf("PS=");
  if (pos > 0) savePreset(getNumVal(&req, pos));


  if (updateVal(&req, "PL=", &presetCycCurr, presetCycleMin, presetCycleMax)) {
    applyPreset(presetCycCurr, presetApplyBri, presetApplyCol, presetApplyFx);
  }


  updateVal(&req, "&A=", &bri);


  updateVal(&req, "&R=", &col[0]);
  updateVal(&req, "&G=", &col[1]);
  updateVal(&req, "&B=", &col[2]);
  updateVal(&req, "&W=", &col[3]);
  updateVal(&req, "R2=", &colSec[0]);
  updateVal(&req, "G2=", &colSec[1]);
  updateVal(&req, "B2=", &colSec[2]);
  updateVal(&req, "W2=", &colSec[3]);


  pos = req.indexOf("HU=");
  if (pos > 0) {
    uint16_t temphue = getNumVal(&req, pos);
    byte tempsat = 255;
    pos = req.indexOf("SA=");
    if (pos > 0) {
      tempsat = getNumVal(&req, pos);
    }
    colorHStoRGB(temphue,tempsat,(req.indexOf("H2")>0)? colSec:col);
  }


  pos = req.indexOf("CL=");
  if (pos > 0) {
    colorFromDecOrHexString(col, (char*)req.substring(pos + 3).c_str());
  }
  pos = req.indexOf("C2=");
  if (pos > 0) {
    colorFromDecOrHexString(colSec, (char*)req.substring(pos + 3).c_str());
  }


  pos = req.indexOf("SR");
  if (pos > 0) {
    _setRandomColor(getNumVal(&req, pos));
  }


  pos = req.indexOf("SC");
  if (pos > 0) {
    byte temp;
    for (uint8_t i=0; i<4; i++)
    {
      temp = col[i];
      col[i] = colSec[i];
      colSec[i] = temp;
    }
  }


  if (updateVal(&req, "FX=", &effectCurrent, 0, strip.getModeCount()-1)) presetCyclingEnabled = false;
  updateVal(&req, "SX=", &effectSpeed);
  updateVal(&req, "IX=", &effectIntensity);
  updateVal(&req, "FP=", &effectPalette, 0, strip.getPaletteCount()-1);


  pos = req.indexOf("OL=");
  if (pos > 0) {
    overlayCurrent = getNumVal(&req, pos);
  }


  pos = req.indexOf("&M=");
  if (pos > 0) {
    applyMacro(getNumVal(&req, pos));
  }


  pos = req.indexOf("SN=");
  if (pos > 0) notifyDirect = (req.charAt(pos+3) != '0');


  pos = req.indexOf("RN=");
  if (pos > 0) receiveNotifications = (req.charAt(pos+3) != '0');


  pos = req.indexOf("RD=");
  if (pos > 0) receiveDirect = (req.charAt(pos+3) != '0');


  bool aNlDef = false;
  if (req.indexOf("&ND") > 0) aNlDef = true;
  pos = req.indexOf("NL=");
  if (pos > 0)
  {
    if (req.charAt(pos+3) == '0')
    {
      nightlightActive = false;
      bri = briT;
    } else {
      nightlightActive = true;
      if (!aNlDef) nightlightDelayMins = getNumVal(&req, pos);
      nightlightStartTime = millis();
    }
  } else if (aNlDef)
  {
    nightlightActive = true;
    nightlightStartTime = millis();
  }


  pos = req.indexOf("NT=");
  if (pos > 0) {
    nightlightTargetBri = getNumVal(&req, pos);
    nightlightActiveOld = false;
  }


  pos = req.indexOf("NF=");
  if (pos > 0)
  {
    nightlightFade = (req.charAt(pos+3) != '0');
    nightlightColorFade = (req.charAt(pos+3) == '2');
    nightlightActiveOld = false;
  }

  #if AUXPIN >= 0

  pos = req.indexOf("AX=");
  if (pos > 0) {
    auxTime = getNumVal(&req, pos);
    auxActive = true;
    if (auxTime == 0) auxActive = false;
  }
  #endif

  pos = req.indexOf("TT=");
  if (pos > 0) transitionDelay = getNumVal(&req, pos);


  pos = req.indexOf("&T=");
  if (pos > 0) {
    nightlightActive = false;
    switch (getNumVal(&req, pos))
    {
      case 0: if (bri != 0){briLast = bri; bri = 0;} break;
      case 1: bri = briLast; break;
      default: toggleOnOff();
    }
  }


  pos = req.indexOf("RV=");
  if (pos > 0) strip.getSegment(main).setOption(1, req.charAt(pos+3) != '0');


  if (bri == nightlightTargetBri) nightlightActive = false;

  pos = req.indexOf("ST=");
  if (pos > 0) {
    setTime(getNumVal(&req, pos));
  }


  pos = req.indexOf("CT=");
  if (pos > 0) {
    countdownTime = getNumVal(&req, pos);
    if (countdownTime - now() > 0) countdownOverTriggered = false;
  }


  #ifndef WLED_DISABLE_CRONIXIE

  pos = req.indexOf("NM=");
  if (pos > 0) countdownMode = (req.charAt(pos+3) != '0');

  pos = req.indexOf("NX=");
  if (pos > 0) {
    strlcpy(cronixieDisplay, req.substring(pos + 3, pos + 9).c_str(), 6);
    setCronixie();
  }

  pos = req.indexOf("NB=");
  if (pos > 0)
  {
    presetApplyFx = (req.charAt(pos+3) != '0');
    if (overlayCurrent == 3) strip.setCronixieBacklight(cronixieBacklight);
    overlayRefreshedTime = 0;
  }
  #endif

  pos = req.indexOf("U0=");
  if (pos > 0) {
    userVar0 = getNumVal(&req, pos);
  }

  pos = req.indexOf("U1=");
  if (pos > 0) {
    userVar1 = getNumVal(&req, pos);
  }



  pos = req.indexOf("IN");
  if (pos < 1) XML_response(request);

  pos = req.indexOf("&NN");
  colorUpdated((pos > 0) ? NOTIFIER_CALL_MODE_NO_NOTIFY : NOTIFIER_CALL_MODE_DIRECT_CHANGE);

  return true;
}
# 1 "C:/Users/RalfBruechmann/Documents/Arduino/code/WLED/wled00/wled04_file.ino"



enum class AdaState {
  Header_A,
  Header_d,
  Header_a,
  Header_CountHi,
  Header_CountLo,
  Header_CountCheck,
  Data_Red,
  Data_Green,
  Data_Blue
};

void handleSerial()
{
  #ifdef WLED_ENABLE_ADALIGHT
  static auto state = AdaState::Header_A;
  static uint16_t count = 0;
  static uint16_t pixel = 0;
  static byte check = 0x00;
  static byte red = 0x00;
  static byte green = 0x00;

  while (Serial.available() > 0)
  {
    yield();
    byte next = Serial.read();
    switch (state) {
      case AdaState::Header_A:
        if (next == 'A') state = AdaState::Header_d;
        break;
      case AdaState::Header_d:
        if (next == 'd') state = AdaState::Header_a;
        else state = AdaState::Header_A;
        break;
      case AdaState::Header_a:
        if (next == 'a') state = AdaState::Header_CountHi;
        else state = AdaState::Header_A;
        break;
      case AdaState::Header_CountHi:
        pixel = 0;
        count = next * 0x100;
        check = next;
        state = AdaState::Header_CountLo;
        break;
      case AdaState::Header_CountLo:
        count += next + 1;
        check = check ^ next ^ 0x55;
        state = AdaState::Header_CountCheck;
        break;
      case AdaState::Header_CountCheck:
        if (check == next) state = AdaState::Data_Red;
        else state = AdaState::Header_A;
        break;
      case AdaState::Data_Red:
        red = next;
        state = AdaState::Data_Green;
        break;
      case AdaState::Data_Green:
        green = next;
        state = AdaState::Data_Blue;
        break;
      case AdaState::Data_Blue:
        byte blue = next;
        setRealtimePixel(pixel++, red, green, blue, 0);
        if (--count > 0) state = AdaState::Data_Red;
        else {
          if (!realtimeMode && bri == 0) strip.setBrightness(briLast);
          arlsLock(realtimeTimeoutMs, REALTIME_MODE_ADALIGHT);

          strip.show();
          state = AdaState::Header_A;
        }
        break;
    }
  }
  #endif
}


#if !defined WLED_DISABLE_FILESYSTEM && defined WLED_ENABLE_FS_SERVING

String getContentType(AsyncWebServerRequest* request, String filename){
  if(request->hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";


  else if(filename.endsWith(".json")) return "application/json";
  else if(filename.endsWith(".png")) return "image/png";

  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";




  return "text/plain";
}

bool handleFileRead(AsyncWebServerRequest* request, String path){
  DEBUG_PRINTLN("FileRead: " + path);
  if(path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(request, path);
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz)){
    request->send(SPIFFS, pathWithGz, contentType);
    return true;
  }
  if(SPIFFS.exists(path)) {
    request->send(SPIFFS, path, contentType);
    return true;
  }
  return false;
}

#else
bool handleFileRead(AsyncWebServerRequest*, String path){return false;}
#endif
# 1 "C:/Users/RalfBruechmann/Documents/Arduino/code/WLED/wled00/wled05_init.ino"




void wledInit()
{
  EEPROM.begin(EEPSIZE);
  ledCount = EEPROM.read(229) + ((EEPROM.read(398) << 8) & 0xFF00);
  if (ledCount > MAX_LEDS || ledCount == 0) ledCount = 30;

  #ifdef ESP8266
  #if LEDPIN == 3
  if (ledCount > MAX_LEDS_DMA) ledCount = MAX_LEDS_DMA;
  #endif
  #endif
  Serial.begin(115200);
  Serial.setTimeout(50);
  DEBUG_PRINTLN();
  DEBUG_PRINT("---WLED "); DEBUG_PRINT(versionString); DEBUG_PRINT(" "); DEBUG_PRINT(VERSION); DEBUG_PRINTLN(" INIT---");
  #ifdef ARDUINO_ARCH_ESP32
  DEBUG_PRINT("esp32 "); DEBUG_PRINTLN(ESP.getSdkVersion());
  #else
  DEBUG_PRINT("esp8266 "); DEBUG_PRINTLN(ESP.getCoreVersion());
  #endif
  int heapPreAlloc = ESP.getFreeHeap();
  DEBUG_PRINT("heap ");
  DEBUG_PRINTLN(ESP.getFreeHeap());

  strip.init(EEPROM.read(372),ledCount,EEPROM.read(2204));
  strip.setBrightness(0);

  DEBUG_PRINT("LEDs inited. heap usage ~");
  DEBUG_PRINTLN(heapPreAlloc - ESP.getFreeHeap());

  #ifndef WLED_DISABLE_FILESYSTEM
   #ifdef ARDUINO_ARCH_ESP32
    SPIFFS.begin(true);
   #endif
    SPIFFS.begin();
  #endif

  DEBUG_PRINTLN("Load EEPROM");
  loadSettingsFromEEPROM(true);
  beginStrip();
  userSetup();
  if (strcmp(clientSSID,DEFAULT_CLIENT_SSID) == 0) showWelcomePage = true;
  WiFi.persistent(false);

  if (macroBoot>0) applyMacro(macroBoot);
  Serial.println("Ada");


  escapedMac = WiFi.macAddress();
  escapedMac.replace(":", "");
  escapedMac.toLowerCase();
  if (strcmp(cmDNS,"x") == 0)
  {
    strcpy(cmDNS, "wled-");
    sprintf(cmDNS+5, "%*s", 6, escapedMac.c_str()+6);
  }
  if (mqttDeviceTopic[0] == 0)
  {
    strcpy(mqttDeviceTopic, "wled/");
    sprintf(mqttDeviceTopic+5, "%*s", 6, escapedMac.c_str()+6);
  }
  if (mqttClientID[0] == 0)
  {
    strcpy(mqttClientID, "WLED-");
    sprintf(mqttClientID+5, "%*s", 6, escapedMac.c_str()+6);
  }

  strip.service();

  #ifndef WLED_DISABLE_OTA
    if (aOtaEnabled)
    {
      ArduinoOTA.onStart([]() {
        #ifdef ESP8266
        wifi_set_sleep_type(NONE_SLEEP_T);
        #endif
        DEBUG_PRINTLN("Start ArduinoOTA");
      });
      if (strlen(cmDNS) > 0) ArduinoOTA.setHostname(cmDNS);
    }
  #endif
  #ifdef WLED_ENABLE_DMX
    dmx.init(512);
  #endif

  initServer();
}


void beginStrip()
{

  strip.setShowCallback(handleOverlayDraw);

#ifdef BTNPIN
  pinMode(BTNPIN, INPUT_PULLUP);
#endif

  if (bootPreset>0) applyPreset(bootPreset, turnOnAtBoot, true, true);
  colorUpdated(NOTIFIER_CALL_MODE_INIT);


  #if RLYPIN >= 0
    pinMode(RLYPIN, OUTPUT);
    #if RLYMDE
      digitalWrite(RLYPIN, bri);
    #else
      digitalWrite(RLYPIN, !bri);
    #endif
  #endif


#ifdef BTNPIN
  if(digitalRead(BTNPIN) == LOW) buttonEnabled = false;
#else
  buttonEnabled = false;
#endif
}


void initAP(bool resetAP=false){
  if (apBehavior == AP_BEHAVIOR_BUTTON_ONLY && !resetAP) return;

  if (!apSSID[0] || resetAP) strcpy(apSSID, "WLED-AP");
  if (resetAP) strcpy(apPass,DEFAULT_AP_PASS);
  DEBUG_PRINT("Opening access point ");
  DEBUG_PRINTLN(apSSID);
  WiFi.softAPConfig(IPAddress(4, 3, 2, 1), IPAddress(4, 3, 2, 1), IPAddress(255,255,255,0));
  WiFi.softAP(apSSID, apPass, apChannel, apHide);

  if (!apActive)
  {
    DEBUG_PRINTLN("Init AP interfaces");
    server.begin();
    if (udpPort > 0 && udpPort != ntpLocalPort)
    {
      udpConnected = notifierUdp.begin(udpPort);
    }
    if (udpRgbPort > 0 && udpRgbPort != ntpLocalPort && udpRgbPort != udpPort)
    {
      udpRgbConnected = rgbUdp.begin(udpRgbPort);
    }

    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, "*", WiFi.softAPIP());
  }
  apActive = true;
}

void initConnection()
{
  WiFi.disconnect();
  #ifdef ESP8266
  WiFi.setPhyMode(WIFI_PHY_MODE_11N);
  #endif

  if (staticIP[0] != 0 && staticGateway[0] != 0)
  {
    WiFi.config(staticIP, staticGateway, staticSubnet, IPAddress(8,8,8,8));
  } else
  {
    WiFi.config(0U, 0U, 0U);
  }

  lastReconnectAttempt = millis();

  if (!WLED_WIFI_CONFIGURED)
  {
    DEBUG_PRINT("No connection configured. ");
    if (!apActive) initAP();
    return;
  } else if (!apActive) {
    if (apBehavior == AP_BEHAVIOR_ALWAYS)
    {
      initAP();
    } else
    {
      DEBUG_PRINTLN("Access point disabled.");
      WiFi.softAPdisconnect(true);
    }
  }
  showWelcomePage = false;

  DEBUG_PRINT("Connecting to ");
  DEBUG_PRINT(clientSSID);
  DEBUG_PRINTLN("...");

  #ifdef ESP8266
   WiFi.hostname(serverDescription);
  #endif

   WiFi.begin(clientSSID, clientPass);

  #ifdef ARDUINO_ARCH_ESP32
   WiFi.setSleep(!noWifiSleep);
   WiFi.setHostname(serverDescription);
  #else
   wifi_set_sleep_type((noWifiSleep) ? NONE_SLEEP_T : MODEM_SLEEP_T);
  #endif
}

void initInterfaces() {
  DEBUG_PRINTLN("Init STA interfaces");

  if (hueIP[0] == 0)
  {
    hueIP[0] = WiFi.localIP()[0];
    hueIP[1] = WiFi.localIP()[1];
    hueIP[2] = WiFi.localIP()[2];
  }


  if (alexaEnabled) alexaInit();

  #ifndef WLED_DISABLE_OTA
   if (aOtaEnabled) ArduinoOTA.begin();
  #endif

  strip.service();

  if (strlen(cmDNS) > 0)
  {
    if (!aOtaEnabled) MDNS.begin(cmDNS);

    DEBUG_PRINTLN("mDNS started");
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("wled", "tcp", 80);
    MDNS.addServiceTxt("wled", "tcp", "mac", escapedMac.c_str());
  }
  server.begin();

  if (udpPort > 0 && udpPort != ntpLocalPort)
  {
    udpConnected = notifierUdp.begin(udpPort);
    if (udpConnected && udpRgbPort != udpPort) udpRgbConnected = rgbUdp.begin(udpRgbPort);
  }
  if (ntpEnabled) ntpConnected = ntpUdp.begin(ntpLocalPort);

  initBlynk(blynkApiKey);
  e131.begin((e131Multicast) ? E131_MULTICAST : E131_UNICAST , e131Universe, E131_MAX_UNIVERSE_COUNT);
  reconnectHue();
  initMqtt();
  interfacesInited = true;
  wasConnected = true;
}

byte stacO = 0;
uint32_t lastHeap;
unsigned long heapTime = 0;

void handleConnection() {
  if (millis() < 2000 && (!WLED_WIFI_CONFIGURED || apBehavior == AP_BEHAVIOR_ALWAYS)) return;
  if (lastReconnectAttempt == 0) initConnection();


  if (millis() - heapTime > 5000)
  {
    uint32_t heap = ESP.getFreeHeap();
    if (heap < 9000 && lastHeap < 9000) {
      DEBUG_PRINT("Heap too low! ");
      DEBUG_PRINTLN(heap);
      forceReconnect = true;
    }
    lastHeap = heap;
    heapTime = millis();
  }

  byte stac = 0;
  if (apActive) {
    #ifdef ESP8266
    stac = wifi_softap_get_station_num();
    #else
    wifi_sta_list_t stationList;
    esp_wifi_ap_get_sta_list(&stationList);
    stac = stationList.num;
    #endif
    if (stac != stacO)
    {
      stacO = stac;
      DEBUG_PRINT("Connected AP clients: ");
      DEBUG_PRINTLN(stac);
      if (!WLED_CONNECTED && WLED_WIFI_CONFIGURED) {
        if (stac) WiFi.disconnect();
        else initConnection();
      }
    }
  }
  if (forceReconnect) {
    DEBUG_PRINTLN("Forcing reconnect.");
    initConnection();
    interfacesInited = false;
    forceReconnect = false;
    wasConnected = false;
    return;
  }
  if (!WLED_CONNECTED) {
    if (interfacesInited) {
      DEBUG_PRINTLN("Disconnected!");
      interfacesInited = false;
      initConnection();
    }
    if (millis() - lastReconnectAttempt > ((stac) ? 300000 : 20000) && WLED_WIFI_CONFIGURED) initConnection();
    if (!apActive && millis() - lastReconnectAttempt > 12000 && (!wasConnected || apBehavior == AP_BEHAVIOR_NO_CONN)) initAP();
  } else if (!interfacesInited) {
    DEBUG_PRINTLN("");
    DEBUG_PRINT("Connected! IP address: ");
    DEBUG_PRINTLN(WiFi.localIP());
    initInterfaces();
    userConnected();


    if (apBehavior != AP_BEHAVIOR_ALWAYS && apActive)
    {
      dnsServer.stop();
      WiFi.softAPdisconnect(true);
      apActive = false;
      DEBUG_PRINTLN("Access point disabled.");
    }
  }
}


int getSignalQuality(int rssi)
{
  int quality = 0;

  if (rssi <= -100) {
    quality = 0;
  } else if (rssi >= -50) {
    quality = 100;
  } else {
    quality = 2 * (rssi + 100);
  }
  return quality;
}
# 1 "C:/Users/RalfBruechmann/Documents/Arduino/code/WLED/wled00/wled06_usermod.ino"
# 11 "C:/Users/RalfBruechmann/Documents/Arduino/code/WLED/wled00/wled06_usermod.ino"
void userSetup()
{

}


void userConnected()
{

}


void userLoop()
{

}
# 1 "C:/Users/RalfBruechmann/Documents/Arduino/code/WLED/wled00/wled07_notify.ino"




#define WLEDPACKETSIZE 29
#define UDP_IN_MAXSIZE 1472


void notify(byte callMode, bool followUp=false)
{
  if (!udpConnected) return;
  switch (callMode)
  {
    case NOTIFIER_CALL_MODE_INIT: return;
    case NOTIFIER_CALL_MODE_DIRECT_CHANGE: if (!notifyDirect) return; break;
    case NOTIFIER_CALL_MODE_BUTTON: if (!notifyButton) return; break;
    case NOTIFIER_CALL_MODE_NIGHTLIGHT: if (!notifyDirect) return; break;
    case NOTIFIER_CALL_MODE_HUE: if (!notifyHue) return; break;
    case NOTIFIER_CALL_MODE_PRESET_CYCLE: if (!notifyDirect) return; break;
    case NOTIFIER_CALL_MODE_BLYNK: if (!notifyDirect) return; break;
    case NOTIFIER_CALL_MODE_ALEXA: if (!notifyAlexa) return; break;
    default: return;
  }
  byte udpOut[WLEDPACKETSIZE];
  udpOut[0] = 0;
  udpOut[1] = callMode;
  udpOut[2] = bri;
  udpOut[3] = col[0];
  udpOut[4] = col[1];
  udpOut[5] = col[2];
  udpOut[6] = nightlightActive;
  udpOut[7] = nightlightDelayMins;
  udpOut[8] = effectCurrent;
  udpOut[9] = effectSpeed;
  udpOut[10] = col[3];




  udpOut[11] = 7;
  udpOut[12] = colSec[0];
  udpOut[13] = colSec[1];
  udpOut[14] = colSec[2];
  udpOut[15] = colSec[3];
  udpOut[16] = effectIntensity;
  udpOut[17] = (transitionDelay >> 0) & 0xFF;
  udpOut[18] = (transitionDelay >> 8) & 0xFF;
  udpOut[19] = effectPalette;
  uint32_t colTer = strip.getSegment(strip.getMainSegmentId()).colors[2];
  udpOut[20] = (colTer >> 16) & 0xFF;
  udpOut[21] = (colTer >> 8) & 0xFF;
  udpOut[22] = (colTer >> 0) & 0xFF;
  udpOut[23] = (colTer >> 24) & 0xFF;

  udpOut[24] = followUp;
  uint32_t t = millis() + strip.timebase;
  udpOut[25] = (t >> 24) & 0xFF;
  udpOut[26] = (t >> 16) & 0xFF;
  udpOut[27] = (t >> 8) & 0xFF;
  udpOut[28] = (t >> 0) & 0xFF;

  IPAddress broadcastIp;
  broadcastIp = ~uint32_t(WiFi.subnetMask()) | uint32_t(WiFi.gatewayIP());

  notifierUdp.beginPacket(broadcastIp, udpPort);
  notifierUdp.write(udpOut, WLEDPACKETSIZE);
  notifierUdp.endPacket();
  notificationSentCallMode = callMode;
  notificationSentTime = millis();
  notificationTwoRequired = (followUp)? false:notifyTwice;
}


void arlsLock(uint32_t timeoutMs, byte md = REALTIME_MODE_GENERIC)
{
  if (!realtimeMode){
    for (uint16_t i = 0; i < ledCount; i++)
    {
      strip.setPixelColor(i,0,0,0,0);
    }
    realtimeMode = md;
  }
  realtimeTimeout = millis() + timeoutMs;
  if (timeoutMs == 255001 || timeoutMs == 65000) realtimeTimeout = UINT32_MAX;
  if (arlsForceMaxBri) strip.setBrightness(255);
}


void handleE131Packet(e131_packet_t* p, IPAddress clientIP){



  if (p->sequence_number < e131LastSequenceNumber && p->sequence_number > 20 && e131LastSequenceNumber < 250){
    DEBUG_PRINT("skipping E1.31 frame (last seq=");
    DEBUG_PRINT(e131LastSequenceNumber);
    DEBUG_PRINT(", current seq=");
    DEBUG_PRINT(p->sequence_number);
    DEBUG_PRINTLN(")");
    return;
  }
  e131LastSequenceNumber = p->sequence_number;


  realtimeIP = clientIP;

  uint16_t uni = htons(p->universe);
  uint8_t previousUniverses = uni - e131Universe;
  uint16_t possibleLEDsInCurrentUniverse;
  uint16_t dmxChannels = htons(p->property_value_count) -1;

  switch (DMXMode) {
    case DMX_MODE_DISABLED:
      return;
      break;

    case DMX_MODE_SINGLE_RGB:
      if (uni != e131Universe) return;
      if (dmxChannels-DMXAddress+1 < 3) return;
      for (uint16_t i = 0; i < ledCount; i++)
        setRealtimePixel(i, p->property_values[DMXAddress+0], p->property_values[DMXAddress+1], p->property_values[DMXAddress+2], 0);
      break;

    case DMX_MODE_SINGLE_DRGB:
      if (uni != e131Universe) return;
      if (dmxChannels-DMXAddress+1 < 4) return;
      if (DMXOldDimmer != p->property_values[DMXAddress+0]) {
        DMXOldDimmer = p->property_values[DMXAddress+0];
        bri = p->property_values[DMXAddress+0];
        strip.setBrightness(bri);
      }
      for (uint16_t i = 0; i < ledCount; i++)
        setRealtimePixel(i, p->property_values[DMXAddress+1], p->property_values[DMXAddress+2], p->property_values[DMXAddress+3], 0);
      break;

    case DMX_MODE_EFFECT:
      if (uni != e131Universe) return;
      if (dmxChannels-DMXAddress+1 < 11) return;
      if (DMXOldDimmer != p->property_values[DMXAddress+0]) {
        DMXOldDimmer = p->property_values[DMXAddress+0];
        bri = p->property_values[DMXAddress+0];
      }
      if (p->property_values[DMXAddress+1] < MODE_COUNT)
        effectCurrent = p->property_values[DMXAddress+ 1];
      effectSpeed = p->property_values[DMXAddress+ 2];
      effectIntensity = p->property_values[DMXAddress+ 3];
      effectPalette = p->property_values[DMXAddress+ 4];
      col[0] = p->property_values[DMXAddress+ 5];
      col[1] = p->property_values[DMXAddress+ 6];
      col[2] = p->property_values[DMXAddress+ 7];
      colSec[0] = p->property_values[DMXAddress+ 8];
      colSec[1] = p->property_values[DMXAddress+ 9];
      colSec[2] = p->property_values[DMXAddress+10];
      if (dmxChannels-DMXAddress+1 > 11)
      {
        col[3] = p->property_values[DMXAddress+11];
        colSec[3] = p->property_values[DMXAddress+12];
      }
      transitionDelayTemp = 0;
      colorUpdated(NOTIFIER_CALL_MODE_NOTIFICATION);
      return;
      break;

    case DMX_MODE_MULTIPLE_RGB:
      if (previousUniverses == 0) {

        possibleLEDsInCurrentUniverse = (dmxChannels - DMXAddress + 1) / 3;
        for (uint16_t i = 0; i < ledCount; i++) {
          if (i >= possibleLEDsInCurrentUniverse) break;
          setRealtimePixel(i, p->property_values[DMXAddress+i*3+0], p->property_values[DMXAddress+i*3+1], p->property_values[DMXAddress+i*3+2], 0);
        }
      } else if (previousUniverses > 0 && uni < (e131Universe + E131_MAX_UNIVERSE_COUNT)) {

        uint16_t numberOfLEDsInPreviousUniverses = ((512 - DMXAddress + 1) / 3);
        if (previousUniverses > 1) numberOfLEDsInPreviousUniverses += (512 / 3) * (previousUniverses - 1);
        possibleLEDsInCurrentUniverse = dmxChannels / 3;
        for (uint16_t i = numberOfLEDsInPreviousUniverses; i < ledCount; i++) {
          uint8_t j = i - numberOfLEDsInPreviousUniverses;
          if (j >= possibleLEDsInCurrentUniverse) break;
          setRealtimePixel(i, p->property_values[j*3+1], p->property_values[j*3+2], p->property_values[j*3+3], 0);
        }
      }
      break;

    case DMX_MODE_MULTIPLE_DRGB:
      if (previousUniverses == 0) {

        if (DMXOldDimmer != p->property_values[DMXAddress+0]) {
          DMXOldDimmer = p->property_values[DMXAddress+0];
          bri = p->property_values[DMXAddress+0];
          strip.setBrightness(bri);
        }
        possibleLEDsInCurrentUniverse = (dmxChannels - DMXAddress) / 3;
        for (uint16_t i = 0; i < ledCount; i++) {
          if (i >= possibleLEDsInCurrentUniverse) break;
          setRealtimePixel(i, p->property_values[DMXAddress+i*3+1], p->property_values[DMXAddress+i*3+2], p->property_values[DMXAddress+i*3+3], 0);
        }
      } else if (previousUniverses > 0 && uni < (e131Universe + E131_MAX_UNIVERSE_COUNT)) {

        uint16_t numberOfLEDsInPreviousUniverses = ((512 - DMXAddress + 1) / 3);
        if (previousUniverses > 1) numberOfLEDsInPreviousUniverses += (512 / 3) * (previousUniverses - 1);
        possibleLEDsInCurrentUniverse = dmxChannels / 3;
        for (uint16_t i = numberOfLEDsInPreviousUniverses; i < ledCount; i++) {
          uint8_t j = i - numberOfLEDsInPreviousUniverses;
          if (j >= possibleLEDsInCurrentUniverse) break;
          setRealtimePixel(i, p->property_values[j*3+1], p->property_values[j*3+2], p->property_values[j*3+3], 0);
        }
      }
      break;

    default:
      DEBUG_PRINTLN("unknown E1.31 DMX mode");
      return;
      break;
  }

  arlsLock(realtimeTimeoutMs, REALTIME_MODE_E131);
  e131NewData = true;
}


void handleNotifications()
{

  if(udpConnected && notificationTwoRequired && millis()-notificationSentTime > 250){
    notify(notificationSentCallMode,true);
  }

  if (e131NewData && millis() - strip.getLastShow() > 15)
  {
    e131NewData = false;
    strip.show();
  }


  if (realtimeMode && millis() > realtimeTimeout)
  {
    strip.setBrightness(bri);
    realtimeMode = REALTIME_MODE_INACTIVE;
  }


  if (!udpConnected || !(receiveNotifications || receiveDirect)) return;

  uint16_t packetSize = notifierUdp.parsePacket();


  if (!packetSize && udpRgbConnected) {
    packetSize = rgbUdp.parsePacket();
    if (!receiveDirect) return;
    if (packetSize > UDP_IN_MAXSIZE || packetSize < 3) return;
    realtimeIP = rgbUdp.remoteIP();
    DEBUG_PRINTLN(rgbUdp.remoteIP());
    uint8_t lbuf[packetSize];
    rgbUdp.read(lbuf, packetSize);
    arlsLock(realtimeTimeoutMs, REALTIME_MODE_HYPERION);
    uint16_t id = 0;
    for (uint16_t i = 0; i < packetSize -2; i += 3)
    {
      setRealtimePixel(id, lbuf[i], lbuf[i+1], lbuf[i+2], 0);

      id++; if (id >= ledCount) break;
    }
    strip.show();
    return;
  }


  if (packetSize > UDP_IN_MAXSIZE) return;
  if(packetSize && notifierUdp.remoteIP() != WiFi.localIP())
  {
    uint8_t udpIn[packetSize];
    notifierUdp.read(udpIn, packetSize);


    if (udpIn[0] == 0 && !realtimeMode && receiveNotifications)
    {

      if (millis() - notificationSentTime < 1000) return;
      if (udpIn[1] > 199) return;

      bool someSel = (receiveNotificationBrightness || receiveNotificationColor || receiveNotificationEffects);

      if (receiveNotificationColor || !someSel)
      {
        col[0] = udpIn[3];
        col[1] = udpIn[4];
        col[2] = udpIn[5];
        if (udpIn[11] > 0)
        {
          col[3] = udpIn[10];
          if (udpIn[11] > 1)
          {
            colSec[0] = udpIn[12];
            colSec[1] = udpIn[13];
            colSec[2] = udpIn[14];
            colSec[3] = udpIn[15];
          }
          if (udpIn[11] > 5)
          {
            uint32_t t = (udpIn[25] << 24) | (udpIn[26] << 16) | (udpIn[27] << 8) | (udpIn[28]);
            t += 2;
            t -= millis();
            strip.timebase = t;
          }
          if (udpIn[11] > 6)
          {
            strip.setColor(2, udpIn[20], udpIn[21], udpIn[22], udpIn[23]);
          }
        }
      }


      if (udpIn[11] < 200 && (receiveNotificationEffects || !someSel))
      {
        if (udpIn[8] < strip.getModeCount()) effectCurrent = udpIn[8];
        effectSpeed = udpIn[9];
        if (udpIn[11] > 2) effectIntensity = udpIn[16];
        if (udpIn[11] > 4 && udpIn[19] < strip.getPaletteCount()) effectPalette = udpIn[19];
      }

      if (udpIn[11] > 3)
      {
        transitionDelayTemp = ((udpIn[17] << 0) & 0xFF) + ((udpIn[18] << 8) & 0xFF00);
      }

      nightlightActive = udpIn[6];
      if (nightlightActive) nightlightDelayMins = udpIn[7];

      if (receiveNotificationBrightness || !someSel) bri = udpIn[2];
      colorUpdated(NOTIFIER_CALL_MODE_NOTIFICATION);

    } else if (udpIn[0] > 0 && udpIn[0] < 5 && receiveDirect)
    {
      realtimeIP = notifierUdp.remoteIP();
      DEBUG_PRINTLN(notifierUdp.remoteIP());
      if (packetSize > 1) {
        if (udpIn[1] == 0)
        {
          realtimeTimeout = 0;
          return;
        } else {
          arlsLock(udpIn[1]*1000 +1, REALTIME_MODE_UDP);
        }
        if (udpIn[0] == 1)
        {
          for (uint16_t i = 2; i < packetSize -3; i += 4)
          {
            setRealtimePixel(udpIn[i], udpIn[i+1], udpIn[i+2], udpIn[i+3], 0);
          }
        } else if (udpIn[0] == 2)
        {
          uint16_t id = 0;
          for (uint16_t i = 2; i < packetSize -2; i += 3)
          {
            setRealtimePixel(id, udpIn[i], udpIn[i+1], udpIn[i+2], 0);

            id++; if (id >= ledCount) break;
          }
        } else if (udpIn[0] == 3)
        {
          uint16_t id = 0;
          for (uint16_t i = 2; i < packetSize -3; i += 4)
          {
            setRealtimePixel(id, udpIn[i], udpIn[i+1], udpIn[i+2], udpIn[i+3]);

            id++; if (id >= ledCount) break;
          }
        } else if (udpIn[0] == 4)
        {
          uint16_t id = ((udpIn[3] << 0) & 0xFF) + ((udpIn[2] << 8) & 0xFF00);
          for (uint16_t i = 4; i < packetSize -2; i += 3)
          {
             if (id >= ledCount) break;
            setRealtimePixel(id, udpIn[i], udpIn[i+1], udpIn[i+2], 0);
            id++;
          }
        }
        strip.show();
      }
    }
  }
}


void setRealtimePixel(uint16_t i, byte r, byte g, byte b, byte w)
{
  uint16_t pix = i + arlsOffset;
  if (pix < ledCount)
  {
    if (!arlsDisableGammaCorrection && strip.gammaCorrectCol)
    {
      strip.setPixelColor(pix, strip.gamma8(r), strip.gamma8(g), strip.gamma8(b), strip.gamma8(w));
    } else {
      strip.setPixelColor(pix, r, g, b, w);
    }
  }
}
# 1 "C:/Users/RalfBruechmann/Documents/Arduino/code/WLED/wled00/wled08_led.ino"



void setValuesFromMainSeg()
{
  WS2812FX::Segment& seg = strip.getSegment(strip.getMainSegmentId());
  colorFromUint32(seg.colors[0]);
  colorFromUint32(seg.colors[1], true);
  effectCurrent = seg.mode;
  effectSpeed = seg.speed;
  effectIntensity = seg.intensity;
  effectPalette = seg.palette;
}


void resetTimebase()
{
  strip.timebase = 0 - millis();
}


void toggleOnOff()
{
  if (bri == 0)
  {
    bri = briLast;
  } else
  {
    briLast = bri;
    bri = 0;
  }
}


void setAllLeds() {
  if (!realtimeMode || !arlsForceMaxBri)
  {
    double d = briT*briMultiplier;
    int val = d/100;
    if (val > 255) val = 255;
    strip.setBrightness(val);
  }
  if (useRGBW && strip.rgbwMode == RGBW_MODE_LEGACY)
  {
    colorRGBtoRGBW(colT);
    colorRGBtoRGBW(colSecT);
  }
  strip.setColor(0, colT[0], colT[1], colT[2], colT[3]);
  strip.setColor(1, colSecT[0], colSecT[1], colSecT[2], colSecT[3]);
}


void setLedsStandard(bool justColors = false)
{
  for (byte i=0; i<4; i++)
  {
    colOld[i] = col[i];
    colT[i] = col[i];
    colSecOld[i] = colSec[i];
    colSecT[i] = colSec[i];
  }
  if (justColors) return;
  briOld = bri;
  briT = bri;
  setAllLeds();
}


bool colorChanged()
{
  for (byte i=0; i<4; i++)
  {
    if (col[i] != colIT[i]) return true;
    if (colSec[i] != colSecIT[i]) return true;

  }
  if (bri != briIT) return true;
  return false;
}


void colorUpdated(int callMode)
{


  if (callMode != NOTIFIER_CALL_MODE_INIT &&
      callMode != NOTIFIER_CALL_MODE_DIRECT_CHANGE &&
      callMode != NOTIFIER_CALL_MODE_NO_NOTIFY) strip.applyToAllSelected = true;

  bool fxChanged = strip.setEffectConfig(effectCurrent, effectSpeed, effectIntensity, effectPalette);
  bool colChanged = colorChanged();

  if (fxChanged || colChanged)
  {
    if (realtimeTimeout == UINT32_MAX) realtimeTimeout = 0;
    if (isPreset) {isPreset = false;}
        else {currentPreset = -1;}

    notify(callMode);


    if (callMode != NOTIFIER_CALL_MODE_PRESET_CYCLE) interfaceUpdateCallMode = callMode;
  } else {
    if (nightlightActive && !nightlightActiveOld &&
        callMode != NOTIFIER_CALL_MODE_NOTIFICATION &&
        callMode != NOTIFIER_CALL_MODE_NO_NOTIFY)
    {
      notify(NOTIFIER_CALL_MODE_NIGHTLIGHT);
      interfaceUpdateCallMode = NOTIFIER_CALL_MODE_NIGHTLIGHT;
    }
  }

  if (!colChanged) return;

  if (callMode != NOTIFIER_CALL_MODE_NO_NOTIFY && nightlightActive && nightlightFade)
  {
    briNlT = bri;
    nightlightDelayMs -= (millis() - nightlightStartTime);
    nightlightStartTime = millis();
  }
  for (byte i=0; i<4; i++)
  {
    colIT[i] = col[i];
    colSecIT[i] = colSec[i];
  }
  if (briT == 0)
  {
    setLedsStandard(true);
    if (callMode != NOTIFIER_CALL_MODE_NOTIFICATION) resetTimebase();
  }

  briIT = bri;
  if (bri > 0) briLast = bri;

  if (fadeTransition)
  {

    if (callMode != NOTIFIER_CALL_MODE_NOTIFICATION && !jsonTransitionOnce) transitionDelayTemp = transitionDelay;
    jsonTransitionOnce = false;
    if (transitionDelayTemp == 0) {setLedsStandard(); strip.trigger(); return;}

    if (transitionActive)
    {
      for (byte i=0; i<4; i++)
      {
        colOld[i] = colT[i];
        colSecOld[i] = colSecT[i];
      }
      briOld = briT;
      tperLast = 0;
    }
    strip.setTransitionMode(true);
    transitionActive = true;
    transitionStartTime = millis();
  } else
  {
    setLedsStandard();
    strip.trigger();
  }
}


void updateInterfaces(uint8_t callMode)
{
  #ifndef WLED_DISABLE_ALEXA
  if (espalexaDevice != nullptr && callMode != NOTIFIER_CALL_MODE_ALEXA) {
    espalexaDevice->setValue(bri);
    espalexaDevice->setColor(col[0], col[1], col[2]);
  }
  #endif
  if (callMode != NOTIFIER_CALL_MODE_BLYNK &&
      callMode != NOTIFIER_CALL_MODE_NO_NOTIFY) updateBlynk();
  doPublishMqtt = true;
  lastInterfaceUpdate = millis();
}


void handleTransitions()
{

  if (interfaceUpdateCallMode && millis() - lastInterfaceUpdate > 2000)
  {
    updateInterfaces(interfaceUpdateCallMode);
    interfaceUpdateCallMode = 0;
  }
  if (doPublishMqtt) publishMqtt();

  if (transitionActive && transitionDelayTemp > 0)
  {
    float tper = (millis() - transitionStartTime)/(float)transitionDelayTemp;
    if (tper >= 1.0)
    {
      strip.setTransitionMode(false);
      transitionActive = false;
      tperLast = 0;
      setLedsStandard();
      return;
    }
    if (tper - tperLast < 0.004) return;
    tperLast = tper;
    for (byte i=0; i<4; i++)
    {
      colT[i] = colOld[i]+((col[i] - colOld[i])*tper);
      colSecT[i] = colSecOld[i]+((colSec[i] - colSecOld[i])*tper);
    }
    briT = briOld +((bri - briOld )*tper);

    setAllLeds();
  }
}


void handleNightlight()
{
  if (nightlightActive)
  {
    if (!nightlightActiveOld)
    {
      nightlightStartTime = millis();
      nightlightDelayMs = (int)(nightlightDelayMins*60000);
      nightlightActiveOld = true;
      briNlT = bri;
      for (byte i=0; i<4; i++) colNlT[i] = col[i];
    }
    float nper = (millis() - nightlightStartTime)/((float)nightlightDelayMs);
    if (nightlightFade)
    {
      bri = briNlT + ((nightlightTargetBri - briNlT)*nper);
      if (nightlightColorFade)
      {
        for (byte i=0; i<4; i++) col[i] = colNlT[i]+ ((colSec[i] - colNlT[i])*nper);
      }
      colorUpdated(NOTIFIER_CALL_MODE_NO_NOTIFY);
    }
    if (nper >= 1)
    {
      nightlightActive = false;
      if (!nightlightFade)
      {
        bri = nightlightTargetBri;
        colorUpdated(NOTIFIER_CALL_MODE_NO_NOTIFY);
      }
      updateBlynk();
      if (bri == 0) briLast = briNlT;
    }
  } else if (nightlightActiveOld)
  {
    nightlightActiveOld = false;
  }


  if (presetCyclingEnabled && (millis() - presetCycledTime > presetCycleTime))
  {
    applyPreset(presetCycCurr,presetApplyBri,presetApplyCol,presetApplyFx);
    presetCycCurr++; if (presetCycCurr > presetCycleMax) presetCycCurr = presetCycleMin;
    if (presetCycCurr > 25) presetCycCurr = 1;
    colorUpdated(NOTIFIER_CALL_MODE_PRESET_CYCLE);
    presetCycledTime = millis();
  }
}
# 1 "C:/Users/RalfBruechmann/Documents/Arduino/code/WLED/wled00/wled09_button.ino"




void shortPressAction()
{
  if (!macroButton)
  {
    toggleOnOff();
    colorUpdated(NOTIFIER_CALL_MODE_BUTTON);
  } else {
    applyMacro(macroButton);
  }
}


void handleButton()
{
#ifdef BTNPIN
  if (!buttonEnabled) return;

  if (digitalRead(BTNPIN) == LOW)
  {
    if (!buttonPressedBefore) buttonPressedTime = millis();
    buttonPressedBefore = true;

    if (millis() - buttonPressedTime > 600)
    {
      if (!buttonLongPressed)
      {
        if (macroLongPress) {applyMacro(macroLongPress);}
        else _setRandomColor(false,true);

        buttonLongPressed = true;
      }
    }
  }
  else if (digitalRead(BTNPIN) == HIGH && buttonPressedBefore)
  {
    long dur = millis() - buttonPressedTime;
    if (dur < 50) {buttonPressedBefore = false; return;}
    bool doublePress = buttonWaitTime;
    buttonWaitTime = 0;

    if (dur > 6000)
    {
      initAP(true);
    }
    else if (!buttonLongPressed) {
      if (macroDoublePress)
      {
        if (doublePress) applyMacro(macroDoublePress);
        else buttonWaitTime = millis();
      } else shortPressAction();
    }
    buttonPressedBefore = false;
    buttonLongPressed = false;
  }

  if (buttonWaitTime && millis() - buttonWaitTime > 450 && !buttonPressedBefore)
  {
    buttonWaitTime = 0;
    shortPressAction();
  }
#endif
}

void handleIO()
{
  handleButton();


  if (strip.getBrightness())
  {
    lastOnTime = millis();
    if (offMode)
    {
      #if RLYPIN >= 0
       digitalWrite(RLYPIN, RLYMDE);
      #endif
      offMode = false;
    }
  } else if (millis() - lastOnTime > 600)
  {
    #if RLYPIN >= 0
     if (!offMode) digitalWrite(RLYPIN, !RLYMDE);
    #endif
    offMode = true;
  }

  #if AUXPIN >= 0

  if (auxActive || auxActiveBefore)
  {
    if (!auxActiveBefore)
    {
      auxActiveBefore = true;
      switch (auxTriggeredState)
      {
        case 0: pinMode(AUXPIN, INPUT); break;
        case 1: pinMode(AUXPIN, OUTPUT); digitalWrite(AUXPIN, HIGH); break;
        case 2: pinMode(AUXPIN, OUTPUT); digitalWrite(AUXPIN, LOW); break;
      }
      auxStartTime = millis();
    }
    if ((millis() - auxStartTime > auxTime*1000 && auxTime != 255) || !auxActive)
    {
      auxActive = false;
      auxActiveBefore = false;
      switch (auxDefaultState)
      {
        case 0: pinMode(AUXPIN, INPUT); break;
        case 1: pinMode(AUXPIN, OUTPUT); digitalWrite(AUXPIN, HIGH); break;
        case 2: pinMode(AUXPIN, OUTPUT); digitalWrite(AUXPIN, LOW); break;
      }
    }
  }
  #endif
}
# 1 "C:/Users/RalfBruechmann/Documents/Arduino/code/WLED/wled00/wled10_ntp.ino"




TimeChangeRule UTCr = {Last, Sun, Mar, 1, 0};
Timezone tzUTC(UTCr, UTCr);

TimeChangeRule BST = {Last, Sun, Mar, 1, 60};
TimeChangeRule GMT = {Last, Sun, Oct, 2, 0};
Timezone tzUK(BST, GMT);

TimeChangeRule CEST = {Last, Sun, Mar, 2, 120};
TimeChangeRule CET = {Last, Sun, Oct, 3, 60};
Timezone tzEUCentral(CEST, CET);

TimeChangeRule EEST = {Last, Sun, Mar, 3, 180};
TimeChangeRule EET = {Last, Sun, Oct, 4, 120};
Timezone tzEUEastern(EEST, EET);

TimeChangeRule EDT = {Second, Sun, Mar, 2, -240 };
TimeChangeRule EST = {First, Sun, Nov, 2, -300 };
Timezone tzUSEastern(EDT, EST);

TimeChangeRule CDT = {Second, Sun, Mar, 2, -300 };
TimeChangeRule CST = {First, Sun, Nov, 2, -360 };
Timezone tzUSCentral(CDT, CST);

Timezone tzCASaskatchewan(CST, CST);

TimeChangeRule MDT = {Second, Sun, Mar, 2, -360 };
TimeChangeRule MST = {First, Sun, Nov, 2, -420 };
Timezone tzUSMountain(MDT, MST);

Timezone tzUSArizona(MST, MST);

TimeChangeRule PDT = {Second, Sun, Mar, 2, -420 };
TimeChangeRule PST = {First, Sun, Nov, 2, -480 };
Timezone tzUSPacific(PDT, PST);

TimeChangeRule ChST = {Last, Sun, Mar, 1, 480};
Timezone tzChina(ChST, ChST);

TimeChangeRule JST = {Last, Sun, Mar, 1, 540};
Timezone tzJapan(JST, JST);

TimeChangeRule AEDT = {Second, Sun, Oct, 2, 660 };
TimeChangeRule AEST = {First, Sun, Apr, 3, 600 };
Timezone tzAUEastern(AEDT, AEST);

TimeChangeRule NZDT = {Second, Sun, Sep, 2, 780 };
TimeChangeRule NZST = {First, Sun, Apr, 3, 720 };
Timezone tzNZ(NZDT, NZST);

TimeChangeRule NKST = {Last, Sun, Mar, 1, 510};
Timezone tzNK(NKST, NKST);

TimeChangeRule IST = {Last, Sun, Mar, 1, 330};
Timezone tzIndia(IST, IST);

Timezone* timezones[] = {&tzUTC, &tzUK, &tzEUCentral, &tzEUEastern, &tzUSEastern, &tzUSCentral, &tzUSMountain, &tzUSArizona, &tzUSPacific, &tzChina, &tzJapan, &tzAUEastern, &tzNZ, &tzNK, &tzIndia, &tzCASaskatchewan};

void handleNetworkTime()
{
  if (ntpEnabled && ntpConnected && millis() - ntpLastSyncTime > 50000000L && WLED_CONNECTED)
  {
    if (millis() - ntpPacketSentTime > 10000)
    {
      sendNTPPacket();
      ntpPacketSentTime = millis();
    }
    if (checkNTPResponse())
    {
      ntpLastSyncTime = millis();
    }
  }
}

void sendNTPPacket()
{
  if (!ntpServerIP.fromString(ntpServerName))
  {
    #ifdef ESP8266
    WiFi.hostByName(ntpServerName, ntpServerIP, 750);
    #else
    WiFi.hostByName(ntpServerName, ntpServerIP);
    #endif
  }

  DEBUG_PRINTLN("send NTP");
  byte pbuf[NTP_PACKET_SIZE];
  memset(pbuf, 0, NTP_PACKET_SIZE);

  pbuf[0] = 0b11100011;
  pbuf[1] = 0;
  pbuf[2] = 6;
  pbuf[3] = 0xEC;

  pbuf[12] = 49;
  pbuf[13] = 0x4E;
  pbuf[14] = 49;
  pbuf[15] = 52;

  ntpUdp.beginPacket(ntpServerIP, 123);
  ntpUdp.write(pbuf, NTP_PACKET_SIZE);
  ntpUdp.endPacket();
}

bool checkNTPResponse()
{
  int cb = ntpUdp.parsePacket();
  if (cb) {
    DEBUG_PRINT("NTP recv, l=");
    DEBUG_PRINTLN(cb);
    byte pbuf[NTP_PACKET_SIZE];
    ntpUdp.read(pbuf, NTP_PACKET_SIZE);

    unsigned long highWord = word(pbuf[40], pbuf[41]);
    unsigned long lowWord = word(pbuf[42], pbuf[43]);
    if (highWord == 0 && lowWord == 0) return false;

    unsigned long secsSince1900 = highWord << 16 | lowWord;

    DEBUG_PRINT("Unix time = ");
    unsigned long epoch = secsSince1900 - 2208988799UL;
    setTime(epoch);
    DEBUG_PRINTLN(epoch);
    if (countdownTime - now() > 0) countdownOverTriggered = false;
    return true;
  }
  return false;
}

void updateLocalTime()
{
  unsigned long tmc = now()+ utcOffsetSecs;
  local = timezones[currentTimezone]->toLocal(tmc);
}

void getTimeString(char* out)
{
  updateLocalTime();
  byte hr = hour(local);
  if (useAMPM)
  {
    if (hr > 11) hr -= 12;
    if (hr == 0) hr = 12;
  }
  sprintf(out,"%i-%i-%i, %i:%s%i:%s%i",year(local), month(local), day(local),
                                       hr,(minute(local)<10)?"0":"",minute(local),
                                       (second(local)<10)?"0":"",second(local));
  if (useAMPM)
  {
    strcat(out,(hour(local) > 11)? " PM":" AM");
  }
}

void setCountdown()
{
  countdownTime = timezones[currentTimezone]->toUTC(getUnixTime(countdownHour, countdownMin, countdownSec, countdownDay, countdownMonth, countdownYear));
  if (countdownTime - now() > 0) countdownOverTriggered = false;
}


bool checkCountdown()
{
  unsigned long n = now();
  if (countdownMode) local = countdownTime - n + utcOffsetSecs;
  if (n > countdownTime) {
    if (countdownMode) local = n - countdownTime + utcOffsetSecs;
    if (!countdownOverTriggered)
    {
      if (macroCountdown != 0) applyMacro(macroCountdown);
      countdownOverTriggered = true;
      return true;
    }
  }
  return false;
}

byte weekdayMondayFirst()
{
  byte wd = weekday(local) -1;
  if (wd == 0) wd = 7;
  return wd;
}

void checkTimers()
{
  if (lastTimerMinute != minute(local))
  {
    lastTimerMinute = minute(local);
    for (uint8_t i = 0; i < 8; i++)
    {
      if (timerMacro[i] != 0
          && (timerHours[i] == hour(local) || timerHours[i] == 24)
          && timerMinutes[i] == minute(local)
          && (timerWeekday[i] & 0x01)
          && timerWeekday[i] >> weekdayMondayFirst() & 0x01)
      {
        applyMacro(timerMacro[i]);
      }
    }
  }
}
# 1 "C:/Users/RalfBruechmann/Documents/Arduino/code/WLED/wled00/wled11_ol.ino"




void initCronixie()
{
  if (overlayCurrent == 3 && !cronixieInit)
  {
    strip.driverModeCronixie(true);
    strip.setCronixieBacklight(cronixieBacklight);
    setCronixie();
    cronixieInit = true;
  } else if (cronixieInit && overlayCurrent != 3)
  {
    strip.driverModeCronixie(false);
    cronixieInit = false;
  }
}


void handleOverlays()
{
  if (millis() - overlayRefreshedTime > overlayRefreshMs)
  {
    initCronixie();
    updateLocalTime();
    checkTimers();
    checkCountdown();
    if (overlayCurrent == 3) _overlayCronixie();
    overlayRefreshedTime = millis();
  }
}


void _overlayAnalogClock()
{
  int overlaySize = overlayMax - overlayMin +1;
  if (countdownMode)
  {
    _overlayAnalogCountdown(); return;
  }
  double hourP = ((double)(hour(local)%12))/12;
  double minuteP = ((double)minute(local))/60;
  hourP = hourP + minuteP/12;
  double secondP = ((double)second(local))/60;
  int hourPixel = floor(analogClock12pixel + overlaySize*hourP);
  if (hourPixel > overlayMax) hourPixel = overlayMin -1 + hourPixel - overlayMax;
  int minutePixel = floor(analogClock12pixel + overlaySize*minuteP);
  if (minutePixel > overlayMax) minutePixel = overlayMin -1 + minutePixel - overlayMax;
  int secondPixel = floor(analogClock12pixel + overlaySize*secondP);
  if (secondPixel > overlayMax) secondPixel = overlayMin -1 + secondPixel - overlayMax;
  if (analogClockSecondsTrail)
  {
    if (secondPixel < analogClock12pixel)
    {
      strip.setRange(analogClock12pixel, overlayMax, 0xFF0000);
      strip.setRange(overlayMin, secondPixel, 0xFF0000);
    } else
    {
      strip.setRange(analogClock12pixel, secondPixel, 0xFF0000);
    }
  }
  if (analogClock5MinuteMarks)
  {
    int pix;
    for (int i = 0; i <= 12; i++)
    {
      pix = analogClock12pixel + round((overlaySize / 12.0) *i);
      if (pix > overlayMax) pix -= overlaySize;
      strip.setPixelColor(pix, 0x00FFAA);
    }
  }
  if (!analogClockSecondsTrail) strip.setPixelColor(secondPixel, 0xFF0000);
  strip.setPixelColor(minutePixel, 0x00FF00);
  strip.setPixelColor(hourPixel, 0x0000FF);
  overlayRefreshMs = 998;
}


void _overlayAnalogCountdown()
{
  if (now() < countdownTime)
  {
    long diff = countdownTime - now();
    double pval = 60;
    if (diff > 31557600L)
    {
      pval = 315576000L;
    } else if (diff > 2592000L)
    {
      pval = 31557600L;
    } else if (diff > 604800)
    {
      pval = 2592000L;
    } else if (diff > 86400)
    {
      pval = 604800;
    } else if (diff > 3600)
    {
      pval = 86400;
    } else if (diff > 60)
    {
      pval = 3600;
    }
    int overlaySize = overlayMax - overlayMin +1;
    double perc = (pval-(double)diff)/pval;
    if (perc > 1.0) perc = 1.0;
    byte pixelCnt = perc*overlaySize;
    if (analogClock12pixel + pixelCnt > overlayMax)
    {
      strip.setRange(analogClock12pixel, overlayMax, ((uint32_t)colSec[3] << 24)| ((uint32_t)colSec[0] << 16) | ((uint32_t)colSec[1] << 8) | colSec[2]);
      strip.setRange(overlayMin, overlayMin +pixelCnt -(1+ overlayMax -analogClock12pixel), ((uint32_t)colSec[3] << 24)| ((uint32_t)colSec[0] << 16) | ((uint32_t)colSec[1] << 8) | colSec[2]);
    } else
    {
      strip.setRange(analogClock12pixel, analogClock12pixel + pixelCnt, ((uint32_t)colSec[3] << 24)| ((uint32_t)colSec[0] << 16) | ((uint32_t)colSec[1] << 8) | colSec[2]);
    }
  }
  overlayRefreshMs = 998;
}


void handleOverlayDraw() {
  if (overlayCurrent != 1) return;
  _overlayAnalogClock();
}
# 1 "C:/Users/RalfBruechmann/Documents/Arduino/code/WLED/wled00/wled12_alexa.ino"
# 9 "C:/Users/RalfBruechmann/Documents/Arduino/code/WLED/wled00/wled12_alexa.ino"
#ifndef WLED_DISABLE_ALEXA
void onAlexaChange(EspalexaDevice* dev);

void alexaInit()
{
  if (alexaEnabled && WLED_CONNECTED)
  {
    if (espalexaDevice == nullptr)
    {
      espalexaDevice = new EspalexaDevice(alexaInvocationName, onAlexaChange, EspalexaDeviceType::extendedcolor);
      espalexa.addDevice(espalexaDevice);
      espalexa.begin(&server);
    } else {
      espalexaDevice->setName(alexaInvocationName);
    }
  }
}

void handleAlexa()
{
  if (!alexaEnabled || !WLED_CONNECTED) return;
  espalexa.loop();
}

void onAlexaChange(EspalexaDevice* dev)
{
  EspalexaDeviceProperty m = espalexaDevice->getLastChangedProperty();

  if (m == EspalexaDeviceProperty::on)
  {
    if (!macroAlexaOn)
    {
      if (bri == 0)
      {
        bri = briLast;
        colorUpdated(NOTIFIER_CALL_MODE_ALEXA);
      }
    } else applyMacro(macroAlexaOn);
  } else if (m == EspalexaDeviceProperty::off)
  {
    if (!macroAlexaOff)
    {
      if (bri > 0)
      {
        briLast = bri;
        bri = 0;
        colorUpdated(NOTIFIER_CALL_MODE_ALEXA);
      }
    } else applyMacro(macroAlexaOff);
  } else if (m == EspalexaDeviceProperty::bri)
  {
    bri = espalexaDevice->getValue();
    colorUpdated(NOTIFIER_CALL_MODE_ALEXA);
  } else
  {
    uint32_t color = espalexaDevice->getRGB();
    col[3] = ((color >> 24) & 0xFF);
    col[0] = ((color >> 16) & 0xFF);
    col[1] = ((color >> 8) & 0xFF);
    col[2] = (color & 0xFF);
    if (useRGBW && col[3] == 0) colorRGBtoRGBW(col);
    colorUpdated(NOTIFIER_CALL_MODE_ALEXA);
  }
}


#else
 void alexaInit(){}
 void handleAlexa(){}
#endif
# 1 "C:/Users/RalfBruechmann/Documents/Arduino/code/WLED/wled00/wled13_cronixie.ino"



byte getSameCodeLength(char code, int index, char const cronixieDisplay[])
{
  byte counter = 0;

  for (int i = index+1; i < 6; i++)
  {
    if (cronixieDisplay[i] == code)
    {
      counter++;
    } else {
      return counter;
    }
  }
  return counter;
}

void setCronixie()
{
  #ifndef WLED_DISABLE_CRONIXIE
# 86 "C:/Users/RalfBruechmann/Documents/Arduino/code/WLED/wled00/wled13_cronixie.ino"
  DEBUG_PRINT("cset ");
  DEBUG_PRINTLN(cronixieDisplay);

  overlayRefreshMs = 1997;

  for (int i = 0; i < 6; i++)
  {
    dP[i] = 10;
    switch (cronixieDisplay[i])
    {
      case '_': dP[i] = 10; break;
      case '-': dP[i] = 11; break;
      case 'r': dP[i] = random(1,7); break;
      case 'R': dP[i] = random(0,10); break;
      case 't': break;
      case 'T': break;
      case 'b': dP[i] = 14 + getSameCodeLength('b',i,cronixieDisplay); i = i+dP[i]-14; break;
      case 'B': dP[i] = 14 + getSameCodeLength('B',i,cronixieDisplay); i = i+dP[i]-14; break;
      case 'h': dP[i] = 70 + getSameCodeLength('h',i,cronixieDisplay); i = i+dP[i]-70; break;
      case 'H': dP[i] = 20 + getSameCodeLength('H',i,cronixieDisplay); i = i+dP[i]-20; break;
      case 'A': dP[i] = 108; i++; break;
      case 'a': dP[i] = 58; i++; break;
      case 'm': dP[i] = 74 + getSameCodeLength('m',i,cronixieDisplay); i = i+dP[i]-74; break;
      case 'M': dP[i] = 24 + getSameCodeLength('M',i,cronixieDisplay); i = i+dP[i]-24; break;
      case 's': dP[i] = 80 + getSameCodeLength('s',i,cronixieDisplay); i = i+dP[i]-80; overlayRefreshMs = 497; break;
      case 'S': dP[i] = 30 + getSameCodeLength('S',i,cronixieDisplay); i = i+dP[i]-30; overlayRefreshMs = 497; break;
      case 'Y': dP[i] = 36 + getSameCodeLength('Y',i,cronixieDisplay); i = i+dP[i]-36; break;
      case 'y': dP[i] = 86 + getSameCodeLength('y',i,cronixieDisplay); i = i+dP[i]-86; break;
      case 'I': dP[i] = 39 + getSameCodeLength('I',i,cronixieDisplay); i = i+dP[i]-39; break;
      case 'i': dP[i] = 89 + getSameCodeLength('i',i,cronixieDisplay); i = i+dP[i]-89; break;
      case 'W': break;
      case 'w': break;
      case 'D': dP[i] = 43 + getSameCodeLength('D',i,cronixieDisplay); i = i+dP[i]-43; break;
      case 'd': dP[i] = 93 + getSameCodeLength('d',i,cronixieDisplay); i = i+dP[i]-93; break;
      case '0': dP[i] = 0; break;
      case '1': dP[i] = 1; break;
      case '2': dP[i] = 2; break;
      case '3': dP[i] = 3; break;
      case '4': dP[i] = 4; break;
      case '5': dP[i] = 5; break;
      case '6': dP[i] = 6; break;
      case '7': dP[i] = 7; break;
      case '8': dP[i] = 8; break;
      case '9': dP[i] = 9; break;
      case 'V': break;
      case 'v': break;
    }
  }
  DEBUG_PRINT("result ");
  for (int i = 0; i < 5; i++)
  {
    DEBUG_PRINT((int)dP[i]);
    DEBUG_PRINT(" ");
  }
  DEBUG_PRINTLN((int)dP[5]);

  _overlayCronixie();
  #endif
}

void _overlayCronixie()
{
  #ifndef WLED_DISABLE_CRONIXIE
  byte h = hour(local);
  byte h0 = h;
  byte m = minute(local);
  byte s = second(local);
  byte d = day(local);
  byte mi = month(local);
  int y = year(local);

  y -= 2000; if (y<0) y += 30;

  if (useAMPM && !countdownMode)
  {
    if (h>12) h-=12;
    else if (h==0) h+=12;
  }
  byte _digitOut[]{10,10,10,10,10,10};
  for (int i = 0; i < 6; i++)
  {
    if (dP[i] < 12) _digitOut[i] = dP[i];
    else {
      if (dP[i] < 65)
      {
        switch(dP[i])
        {
          case 21: _digitOut[i] = h/10; _digitOut[i+1] = h- _digitOut[i]*10; i++; break;
          case 25: _digitOut[i] = m/10; _digitOut[i+1] = m- _digitOut[i]*10; i++; break;
          case 31: _digitOut[i] = s/10; _digitOut[i+1] = s- _digitOut[i]*10; i++; break;

          case 20: _digitOut[i] = h- (h/10)*10; break;
          case 24: _digitOut[i] = m/10; break;
          case 30: _digitOut[i] = s/10; break;

          case 43: _digitOut[i] = weekday(local); _digitOut[i]--; if (_digitOut[i]<1) _digitOut[i]= 7; break;
          case 44: _digitOut[i] = d/10; _digitOut[i+1] = d- _digitOut[i]*10; i++; break;
          case 40: _digitOut[i] = mi/10; _digitOut[i+1] = mi- _digitOut[i]*10; i++; break;
          case 37: _digitOut[i] = y/10; _digitOut[i+1] = y- _digitOut[i]*10; i++; break;
          case 39: _digitOut[i] = 2; _digitOut[i+1] = 0; _digitOut[i+2] = y/10; _digitOut[i+3] = y- _digitOut[i+2]*10; i+=3; break;

          case 16: _digitOut[i+2] = ((h0/3)&1)?1:0; i++;
          case 15: _digitOut[i+1] = (h0>17 || (h0>5 && h0<12))?1:0; i++;
          case 14: _digitOut[i] = (h0>11)?1:0; break;
        }
      } else
      {
        switch(dP[i])
        {
          case 71: _digitOut[i] = h/10; _digitOut[i+1] = h- _digitOut[i]*10; if(_digitOut[i] == 0) _digitOut[i]=10; i++; break;
          case 75: _digitOut[i] = m/10; _digitOut[i+1] = m- _digitOut[i]*10; if(_digitOut[i] == 0) _digitOut[i]=10; i++; break;
          case 81: _digitOut[i] = s/10; _digitOut[i+1] = s- _digitOut[i]*10; if(_digitOut[i] == 0) _digitOut[i]=10; i++; break;
          case 66: _digitOut[i+2] = ((h0/3)&1)?1:10; i++;
          case 65: _digitOut[i+1] = (h0>17 || (h0>5 && h0<12))?1:10; i++;
          case 64: _digitOut[i] = (h0>11)?1:10; break;

          case 93: _digitOut[i] = weekday(local); _digitOut[i]--; if (_digitOut[i]<1) _digitOut[i]= 7; break;
          case 94: _digitOut[i] = d/10; _digitOut[i+1] = d- _digitOut[i]*10; if(_digitOut[i] == 0) _digitOut[i]=10; i++; break;
          case 90: _digitOut[i] = mi/10; _digitOut[i+1] = mi- _digitOut[i]*10; if(_digitOut[i] == 0) _digitOut[i]=10; i++; break;
          case 87: _digitOut[i] = y/10; _digitOut[i+1] = y- _digitOut[i]*10; i++; break;
          case 89: _digitOut[i] = 2; _digitOut[i+1] = 0; _digitOut[i+2] = y/10; _digitOut[i+3] = y- _digitOut[i+2]*10; i+=3; break;
        }
      }
    }
  }
  strip.setCronixieDigits(_digitOut);

  #endif
}
# 1 "C:/Users/RalfBruechmann/Documents/Arduino/code/WLED/wled00/wled14_colors.ino"




void colorFromUint32(uint32_t in, bool secondary)
{
  if (secondary) {
    colSec[3] = in >> 24 & 0xFF;
    colSec[0] = in >> 16 & 0xFF;
    colSec[1] = in >> 8 & 0xFF;
    colSec[2] = in & 0xFF;
  } else {
    col[3] = in >> 24 & 0xFF;
    col[0] = in >> 16 & 0xFF;
    col[1] = in >> 8 & 0xFF;
    col[2] = in & 0xFF;
  }
}


void colorFromUint24(uint32_t in, bool secondary = false)
{
  if (secondary) {
    colSec[0] = in >> 16 & 0xFF;
    colSec[1] = in >> 8 & 0xFF;
    colSec[2] = in & 0xFF;
  } else {
    col[0] = in >> 16 & 0xFF;
    col[1] = in >> 8 & 0xFF;
    col[2] = in & 0xFF;
  }
}


void relativeChangeWhite(int8_t amount, byte lowerBoundary =0)
{
  int16_t new_val = (int16_t) col[3] + amount;
  if (new_val > 0xFF) new_val = 0xFF;
  else if (new_val < lowerBoundary) new_val = lowerBoundary;
  col[3] = new_val;
}

void colorHStoRGB(uint16_t hue, byte sat, byte* rgb)
{
  float h = ((float)hue)/65535.0;
  float s = ((float)sat)/255.0;
  byte i = floor(h*6);
  float f = h * 6-i;
  float p = 255 * (1-s);
  float q = 255 * (1-f*s);
  float t = 255 * (1-(1-f)*s);
  switch (i%6) {
    case 0: rgb[0]=255,rgb[1]=t,rgb[2]=p;break;
    case 1: rgb[0]=q,rgb[1]=255,rgb[2]=p;break;
    case 2: rgb[0]=p,rgb[1]=255,rgb[2]=t;break;
    case 3: rgb[0]=p,rgb[1]=q,rgb[2]=255;break;
    case 4: rgb[0]=t,rgb[1]=p,rgb[2]=255;break;
    case 5: rgb[0]=255,rgb[1]=p,rgb[2]=q;
  }
  if (useRGBW) colorRGBtoRGBW(col);
}

#ifndef WLED_DISABLE_HUESYNC
void colorCTtoRGB(uint16_t mired, byte* rgb)
{

  if (mired > 475) {
    rgb[0]=255;rgb[1]=199;rgb[2]=92;
  } else if (mired > 425) {
    rgb[0]=255;rgb[1]=213;rgb[2]=118;
  } else if (mired > 375) {
    rgb[0]=255;rgb[1]=216;rgb[2]=118;
  } else if (mired > 325) {
    rgb[0]=255;rgb[1]=234;rgb[2]=140;
  } else if (mired > 275) {
    rgb[0]=255;rgb[1]=243;rgb[2]=160;
  } else if (mired > 225) {
    rgb[0]=250;rgb[1]=255;rgb[2]=188;
  } else if (mired > 175) {
    rgb[0]=247;rgb[1]=255;rgb[2]=215;
  } else {
    rgb[0]=237;rgb[1]=255;rgb[2]=239;
  }
  if (useRGBW) colorRGBtoRGBW(col);
}

void colorXYtoRGB(float x, float y, byte* rgb)
{
  float z = 1.0f - x - y;
  float X = (1.0f / y) * x;
  float Z = (1.0f / y) * z;
  float r = (int)255*(X * 1.656492f - 0.354851f - Z * 0.255038f);
  float g = (int)255*(-X * 0.707196f + 1.655397f + Z * 0.036152f);
  float b = (int)255*(X * 0.051713f - 0.121364f + Z * 1.011530f);
  if (r > b && r > g && r > 1.0f) {

    g = g / r;
    b = b / r;
    r = 1.0f;
  } else if (g > b && g > r && g > 1.0f) {

    r = r / g;
    b = b / g;
    g = 1.0f;
  } else if (b > r && b > g && b > 1.0f) {

    r = r / b;
    g = g / b;
    b = 1.0f;
  }

  r = r <= 0.0031308f ? 12.92f * r : (1.0f + 0.055f) * pow(r, (1.0f / 2.4f)) - 0.055f;
  g = g <= 0.0031308f ? 12.92f * g : (1.0f + 0.055f) * pow(g, (1.0f / 2.4f)) - 0.055f;
  b = b <= 0.0031308f ? 12.92f * b : (1.0f + 0.055f) * pow(b, (1.0f / 2.4f)) - 0.055f;

  if (r > b && r > g) {

    if (r > 1.0f) {
      g = g / r;
      b = b / r;
      r = 1.0f;
    }
  } else if (g > b && g > r) {

    if (g > 1.0f) {
      r = r / g;
      b = b / g;
      g = 1.0f;
    }
  } else if (b > r && b > g) {

    if (b > 1.0f) {
      r = r / b;
      g = g / b;
      b = 1.0f;
    }
  }
  rgb[0] = 255.0*r;
  rgb[1] = 255.0*g;
  rgb[2] = 255.0*b;
  if (useRGBW) colorRGBtoRGBW(col);
}

void colorRGBtoXY(byte* rgb, float* xy)
{
  float X = rgb[0] * 0.664511f + rgb[1] * 0.154324f + rgb[2] * 0.162028f;
  float Y = rgb[0] * 0.283881f + rgb[1] * 0.668433f + rgb[2] * 0.047685f;
  float Z = rgb[0] * 0.000088f + rgb[1] * 0.072310f + rgb[2] * 0.986039f;
  xy[0] = X / (X + Y + Z);
  xy[1] = Y / (X + Y + Z);
}
#endif

void colorFromDecOrHexString(byte* rgb, char* in)
{
  if (in[0] == 0) return;
  char first = in[0];
  uint32_t c = 0;

  if (first == '#' || first == 'h' || first == 'H')
  {
    c = strtoul(in +1, NULL, 16);
  } else
  {
    c = strtoul(in, NULL, 10);
  }

  rgb[3] = (c >> 24) & 0xFF;
  rgb[0] = (c >> 16) & 0xFF;
  rgb[1] = (c >> 8) & 0xFF;
  rgb[2] = c & 0xFF;
}

float minf (float v, float w)
{
  if (w > v) return v;
  return w;
}

float maxf (float v, float w)
{
  if (w > v) return w;
  return v;
}

void colorRGBtoRGBW(byte* rgb)
{
  float low = minf(rgb[0],minf(rgb[1],rgb[2]));
  float high = maxf(rgb[0],maxf(rgb[1],rgb[2]));
  if (high < 0.1f) return;
  float sat = 100.0f * ((high - low) / high);;
  rgb[3] = (byte)((255.0f - sat) / 255.0f * (rgb[0] + rgb[1] + rgb[2]) / 3);
}
# 1 "C:/Users/RalfBruechmann/Documents/Arduino/code/WLED/wled00/wled15_hue.ino"



#ifndef WLED_DISABLE_HUESYNC
void handleHue()
{
  if (hueReceived)
  {
    colorUpdated(NOTIFIER_CALL_MODE_HUE); hueReceived = false;
    if (hueStoreAllowed && hueNewKey)
    {
      saveSettingsToEEPROM();
      hueStoreAllowed = false;
      hueNewKey = false;
    }
  }

  if (!WLED_CONNECTED || hueClient == nullptr || millis() - hueLastRequestSent < huePollIntervalMs) return;

  hueLastRequestSent = millis();
  if (huePollingEnabled)
  {
    reconnectHue();
  } else {
    hueClient->close();
    if (hueError == HUE_ERROR_ACTIVE) hueError = HUE_ERROR_INACTIVE;
  }
}

void reconnectHue()
{
  if (!WLED_CONNECTED || !huePollingEnabled) return;
  DEBUG_PRINTLN("Hue reconnect");
  if (hueClient == nullptr) {
    hueClient = new AsyncClient();
    hueClient->onConnect(&onHueConnect, hueClient);
    hueClient->onData(&onHueData, hueClient);
    hueClient->onError(&onHueError, hueClient);
    hueAuthRequired = (strlen(hueApiKey)<20);
  }
  hueClient->connect(hueIP, 80);
}

void onHueError(void* arg, AsyncClient* client, int8_t error)
{
  DEBUG_PRINTLN("Hue err");
  hueError = HUE_ERROR_TIMEOUT;
}

void onHueConnect(void* arg, AsyncClient* client)
{
  DEBUG_PRINTLN("Hue connect");
  sendHuePoll();
}

void sendHuePoll()
{
  if (hueClient == nullptr || !hueClient->connected()) return;
  String req = "";
  if (hueAuthRequired)
  {
    req += F("POST /api HTTP/1.1\r\nHost: ");
    req += hueIP.toString();
    req += F("\r\nContent-Length: 25\r\n\r\n{\"devicetype\":\"wled#esp\"}");
  } else
  {
    req += "GET /api/";
    req += hueApiKey;
    req += "/lights/" + String(huePollLightId);
    req += F(" HTTP/1.1\r\nHost: ");
    req += hueIP.toString();
    req += "\r\n\r\n";
  }
  hueClient->add(req.c_str(), req.length());
  hueClient->send();
  hueLastRequestSent = millis();
}

void onHueData(void* arg, AsyncClient* client, void *data, size_t len)
{
  if (!len) return;
  char* str = (char*)data;
  DEBUG_PRINTLN(hueApiKey);
  DEBUG_PRINTLN(str);

  str = strstr(str,"\r\n\r\n");
  if (str == nullptr) return;
  str += 4;

  StaticJsonDocument<512> root;
  if (str[0] == '[')
  {
    auto error = deserializeJson(root, str);
    if (error)
    {
      hueError = HUE_ERROR_JSON_PARSING; return;
    }

    int hueErrorCode = root[0]["error"]["type"];
    if (hueErrorCode)
    {
      hueError = hueErrorCode;
      switch (hueErrorCode)
      {
        case 1: hueAuthRequired = true; break;
        case 3: huePollingEnabled = false; break;
        case 101: hueAuthRequired = true; break;
      }
      return;
    }

    if (hueAuthRequired)
    {
      const char* apikey = root[0]["success"]["username"];
      if (apikey != nullptr && strlen(apikey) < sizeof(hueApiKey))
      {
        strcpy(hueApiKey, apikey);
        hueAuthRequired = false;
        hueNewKey = true;
      }
    }
    return;
  }


  str = strstr(str,"state");
  if (str == nullptr) return;
  str = strstr(str,"{");

  auto error = deserializeJson(root, str);
  if (error)
  {
    hueError = HUE_ERROR_JSON_PARSING; return;
  }

  float hueX=0, hueY=0;
  uint16_t hueHue=0, hueCt=0;
  byte hueBri=0, hueSat=0, hueColormode=0;

  if (root["on"]) {
    if (root.containsKey("bri"))
    {
      hueBri = root["bri"];
      hueBri++;
      const char* cm =root["colormode"];
      if (cm != nullptr)
      {
        if (strstr(cm,"ct") != nullptr)
        {
          hueCt = root["ct"];
          hueColormode = 3;
        } else if (strstr(cm,"xy") != nullptr)
        {
          hueX = root["xy"][0];
          hueY = root["xy"][1];
          hueColormode = 1;
        } else
        {
          hueHue = root["hue"];
          hueSat = root["sat"];
          hueColormode = 2;
        }
      }
    } else
    {
      hueBri = briLast;
    }
  } else
  {
    hueBri = 0;
  }

  hueError = HUE_ERROR_ACTIVE;


  if (hueBri != hueBriLast)
  {
    if (hueApplyOnOff)
    {
      if (hueBri==0) {bri = 0;}
      else if (bri==0 && hueBri>0) bri = briLast;
    }
    if (hueApplyBri)
    {
      if (hueBri>0) bri = hueBri;
    }
    hueBriLast = hueBri;
  }
  if (hueApplyColor)
  {
    switch(hueColormode)
    {
      case 1: if (hueX != hueXLast || hueY != hueYLast) colorXYtoRGB(hueX,hueY,col); hueXLast = hueX; hueYLast = hueY; break;
      case 2: if (hueHue != hueHueLast || hueSat != hueSatLast) colorHStoRGB(hueHue,hueSat,col); hueHueLast = hueHue; hueSatLast = hueSat; break;
      case 3: if (hueCt != hueCtLast) colorCTtoRGB(hueCt,col); hueCtLast = hueCt; break;
    }
  }
  hueReceived = true;
}
#else
void handleHue(){}
void reconnectHue(){}
#endif
# 1 "C:/Users/RalfBruechmann/Documents/Arduino/code/WLED/wled00/wled16_blynk.ino"




uint16_t blHue = 0;
byte blSat = 255;

void initBlynk(const char* auth)
{
  #ifndef WLED_DISABLE_BLYNK
  if (!WLED_CONNECTED) return;
  blynkEnabled = (auth[0] != 0);
  if (blynkEnabled) Blynk.config(auth);
  #endif
}

void handleBlynk()
{
  #ifndef WLED_DISABLE_BLYNK
  if (WLED_CONNECTED && blynkEnabled)
  Blynk.run();
  #endif
}

void updateBlynk()
{
  #ifndef WLED_DISABLE_BLYNK
  if (!WLED_CONNECTED) return;
  Blynk.virtualWrite(V0, bri);

  Blynk.virtualWrite(V3, bri? 1:0);
  Blynk.virtualWrite(V4, effectCurrent);
  Blynk.virtualWrite(V5, effectSpeed);
  Blynk.virtualWrite(V6, effectIntensity);
  Blynk.virtualWrite(V7, nightlightActive);
  Blynk.virtualWrite(V8, notifyDirect);
  #endif
}

#ifndef WLED_DISABLE_BLYNK
BLYNK_WRITE(V0)
{
  bri = param.asInt();
  colorUpdated(NOTIFIER_CALL_MODE_BLYNK);
}

BLYNK_WRITE(V1)
{
  blHue = param.asInt();
  colorHStoRGB(blHue*10,blSat,(false)? colSec:col);
  colorUpdated(NOTIFIER_CALL_MODE_BLYNK);
}

BLYNK_WRITE(V2)
{
  blSat = param.asInt();
  colorHStoRGB(blHue*10,blSat,(false)? colSec:col);
  colorUpdated(NOTIFIER_CALL_MODE_BLYNK);
}

BLYNK_WRITE(V3)
{
  bool on = (param.asInt()>0);
  if (!on != !bri) {toggleOnOff(); colorUpdated(NOTIFIER_CALL_MODE_BLYNK);}
}

BLYNK_WRITE(V4)
{
  effectCurrent = param.asInt()-1;
  colorUpdated(NOTIFIER_CALL_MODE_BLYNK);
}

BLYNK_WRITE(V5)
{
  effectSpeed = param.asInt();
  colorUpdated(NOTIFIER_CALL_MODE_BLYNK);
}

BLYNK_WRITE(V6)
{
  effectIntensity = param.asInt();
  colorUpdated(NOTIFIER_CALL_MODE_BLYNK);
}

BLYNK_WRITE(V7)
{
  nightlightActive = (param.asInt()>0);
}

BLYNK_WRITE(V8)
{
  notifyDirect = (param.asInt()>0);
}
#endif
# 1 "C:/Users/RalfBruechmann/Documents/Arduino/code/WLED/wled00/wled17_mqtt.ino"




#ifdef WLED_ENABLE_MQTT
#define MQTT_KEEP_ALIVE_TIME 60

void parseMQTTBriPayload(char* payload)
{
  if (strstr(payload, "ON") || strstr(payload, "on") || strstr(payload, "true")) {bri = briLast; colorUpdated(1);}
  else if (strstr(payload, "T" ) || strstr(payload, "t" )) {toggleOnOff(); colorUpdated(1);}
  else {
    uint8_t in = strtoul(payload, NULL, 10);
    if (in == 0 && bri > 0) briLast = bri;
    bri = in;
    colorUpdated(NOTIFIER_CALL_MODE_DIRECT_CHANGE);
  }
}


void onMqttConnect(bool sessionPresent)
{

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



  if (strstr(topic, "/col"))
  {
    colorFromDecOrHexString(col, (char*)payload);
    colorUpdated(NOTIFIER_CALL_MODE_DIRECT_CHANGE);
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




bool initMqtt()
{
  lastMqttReconnectAttempt = millis();
  if (!mqttEnabled || mqttServer[0] == 0 || !WLED_CONNECTED) return false;

  if (mqtt == nullptr) {
    mqtt = new AsyncMqttClient();
    mqtt->onMessage(onMqttMessage);
    mqtt->onConnect(onMqttConnect);
  }
  if (mqtt->connected()) return true;

  DEBUG_PRINTLN("Reconnecting MQTT");
  IPAddress mqttIP;
  if (mqttIP.fromString(mqttServer))
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
  mqtt->setKeepAlive(MQTT_KEEP_ALIVE_TIME);
  mqtt->connect();
  return true;
}

#else
bool initMqtt(){return false;}
void publishMqtt(){}
#endif
# 1 "C:/Users/RalfBruechmann/Documents/Arduino/code/WLED/wled00/wled18_server.ino"





bool isIp(String str) {
  for (size_t i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

bool captivePortal(AsyncWebServerRequest *request)
{
  if (ON_STA_FILTER(request)) return false;
  String hostH;
  if (!request->hasHeader("Host")) return false;
  hostH = request->getHeader("Host")->value();

  if (!isIp(hostH) && hostH.indexOf("wled.me") < 0 && hostH.indexOf(cmDNS) < 0) {
    DEBUG_PRINTLN("Captive portal");
    AsyncWebServerResponse *response = request->beginResponse(302);
    response->addHeader("Location", "http://4.3.2.1");
    request->send(response);
    return true;
  }
  return false;
}

void initServer()
{

  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "*");

  server.on("/liveview", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", PAGE_liveview);
  });


  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){
    serveSettings(request);
  });

  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!handleFileRead(request, "/favicon.ico"))
    {
      request->send_P(200, "image/x-icon", favicon, 156);
    }
  });

  server.on("/sliders", HTTP_GET, [](AsyncWebServerRequest *request){
    serveIndex(request);
  });

  server.on("/welcome", HTTP_GET, [](AsyncWebServerRequest *request){
    serveSettings(request);
  });

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
    serveMessage(request, 200,"Rebooting now...",F("Please wait ~10 seconds..."),129);
    doReboot = true;
  });

  server.on("/settings/wifi", HTTP_POST, [](AsyncWebServerRequest *request){
    if (!(wifiLock && otaLock)) handleSettingsSet(request, 1);
    serveMessage(request, 200,F("WiFi settings saved."),F("Please connect to the new IP (if changed)"),129);
    forceReconnect = true;
  });

  server.on("/settings/leds", HTTP_POST, [](AsyncWebServerRequest *request){
    handleSettingsSet(request, 2);
    serveMessage(request, 200,F("LED settings saved."),"Redirecting...",1);
  });

  server.on("/settings/ui", HTTP_POST, [](AsyncWebServerRequest *request){
    handleSettingsSet(request, 3);
    serveMessage(request, 200,F("UI settings saved."),"Redirecting...",1);
  });

  server.on("/settings/dmx", HTTP_POST, [](AsyncWebServerRequest *request){
    handleSettingsSet(request, 7);
    serveMessage(request, 200,F("UI settings saved."),"Redirecting...",1);
  });

  server.on("/settings/sync", HTTP_POST, [](AsyncWebServerRequest *request){
    handleSettingsSet(request, 4);
    serveMessage(request, 200,F("Sync settings saved."),"Redirecting...",1);
  });

  server.on("/settings/time", HTTP_POST, [](AsyncWebServerRequest *request){
    handleSettingsSet(request, 5);
    serveMessage(request, 200,F("Time settings saved."),"Redirecting...",1);
  });

  server.on("/settings/sec", HTTP_POST, [](AsyncWebServerRequest *request){
    handleSettingsSet(request, 6);
    if (!doReboot) serveMessage(request, 200,F("Security settings saved."),F("Rebooting, please wait ~10 seconds..."),129);
    doReboot = true;
  });

  server.on("/json", HTTP_GET, [](AsyncWebServerRequest *request){
    serveJson(request);
  });

  AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/json", [](AsyncWebServerRequest *request) {
    bool verboseResponse = false;
    {
      DynamicJsonDocument jsonBuffer(8192);
      DeserializationError error = deserializeJson(jsonBuffer, (uint8_t*)(request->_tempObject));
      JsonObject root = jsonBuffer.as<JsonObject>();
      if (error || root.isNull()) {
        request->send(400, "application/json", "{\"error\":10}"); return;
      }
      verboseResponse = deserializeState(root);
    }
    if (verboseResponse) {
      serveJson(request); return;
    }
    request->send(200, "application/json", "{\"success\":true}");
  });
  server.addHandler(handler);


  server.on("/version", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", (String)VERSION);
    });

  server.on("/uptime", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", (String)millis());
    });

  server.on("/freeheap", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", (String)ESP.getFreeHeap());
    });


  server.on("/u", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", PAGE_usermod);
    });

  server.on("/url", HTTP_GET, [](AsyncWebServerRequest *request){
    URL_response(request);
    });

  server.on("/teapot", HTTP_GET, [](AsyncWebServerRequest *request){
    serveMessage(request, 418, F("418. I'm a teapot."), F("(Tangible Embedded Advanced Project Of Twinkling)"), 254);
    });


  if (!otaLock){
    #if !defined WLED_DISABLE_FILESYSTEM && defined WLED_ENABLE_FS_EDITOR
     #ifdef ARDUINO_ARCH_ESP32
      server.addHandler(new SPIFFSEditor(SPIFFS));
     #else
      server.addHandler(new SPIFFSEditor());
     #endif
    #else
    server.on("/edit", HTTP_GET, [](AsyncWebServerRequest *request){
      serveMessage(request, 501, "Not implemented", F("The SPIFFS editor is disabled in this build."), 254);
    });
    #endif

    #ifndef WLED_DISABLE_OTA
    server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", PAGE_update);
    });

    server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
      if (Update.hasError())
      {
        serveMessage(request, 500, F("Failed updating firmware!"), F("Please check your file and retry!"), 254); return;
      }
      serveMessage(request, 200, F("Successfully updated firmware!"), F("Please wait while the module reboots..."), 131);
      doReboot = true;
    },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
      if(!index){
        DEBUG_PRINTLN("OTA Update Start");
        #ifdef ESP8266
        Update.runAsync(true);
        #endif
        Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000);
      }
      if(!Update.hasError()) Update.write(data, len);
      if(final){
        if(Update.end(true)){
          DEBUG_PRINTLN("Update Success");
        } else {
          DEBUG_PRINTLN("Update Failed");
        }
      }
    });

    #else
    server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
      serveMessage(request, 501, "Not implemented", F("OTA updates are disabled in this build."), 254);
    });
    #endif
  } else
  {
    server.on("/edit", HTTP_GET, [](AsyncWebServerRequest *request){
      serveMessage(request, 500, "Access Denied", "Please unlock OTA in security settings!", 254);
    });
    server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
      serveMessage(request, 500, "Access Denied", "Please unlock OTA in security settings!", 254);
    });
  }


    #ifdef WLED_ENABLE_DMX
    server.on("/dmxmap", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", PAGE_dmxmap , dmxProcessor);
    });
    #else
    server.on("/dmxmap", HTTP_GET, [](AsyncWebServerRequest *request){
      serveMessage(request, 501, "Not implemented", F("DMX support is not enabled in this build."), 254);
    });
    #endif
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    if (captivePortal(request)) return;
    serveIndexOrWelcome(request);
  });


  server.onNotFound([](AsyncWebServerRequest *request){
    DEBUG_PRINTLN("Not-Found HTTP call:");
    DEBUG_PRINTLN("URI: " + request->url());
    if (captivePortal(request)) return;


    if (request->method() == HTTP_OPTIONS)
    {
      request->send(200); return;
    }

    if(handleSet(request, request->url())) return;
    #ifndef WLED_DISABLE_ALEXA
    if(espalexa.handleAlexaApiCall(request)) return;
    #endif
    #ifdef WLED_ENABLE_FS_SERVING
    if(handleFileRead(request, request->url())) return;
    #endif
    request->send(404, "text/plain", "Not Found");
  });
}


void serveIndexOrWelcome(AsyncWebServerRequest *request)
{
  if (!showWelcomePage){
    serveIndex(request);
  } else {
    serveSettings(request);
  }
}


void serveIndex(AsyncWebServerRequest* request)
{
  #ifdef WLED_ENABLE_FS_SERVING
  if (handleFileRead(request, "/index.htm")) return;
  #endif

  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", PAGE_index, PAGE_index_L);

  response->addHeader("Content-Encoding","gzip");

  request->send(response);
}


String msgProcessor(const String& var)
{
  if (var == "MSG") {
    String messageBody = messageHead;
    messageBody += "</h2>";
    messageBody += messageSub;
    uint32_t optt = optionType;

    if (optt < 60)
    {
      messageBody += "<script>setTimeout(RS," + String(optt*1000) + ")</script>";
    } else if (optt < 120)
    {

    } else if (optt < 180)
    {
      messageBody += "<script>setTimeout(RP," + String((optt-120)*1000) + ")</script>";
    } else if (optt == 253)
    {
      messageBody += F("<br><br><form action=/settings><button class=\"bt\" type=submit>Back</button></form>");
    } else if (optt == 254)
    {
      messageBody += F("<br><br><button type=\"button\" class=\"bt\" onclick=\"B()\">Back</button>");
    }
    return messageBody;
  }
  return String();
}


void serveMessage(AsyncWebServerRequest* request, uint16_t code, String headl, String subl="", byte optionT=255)
{
  messageHead = headl;
  messageSub = subl;
  optionType = optionT;

  request->send_P(code, "text/html", PAGE_msg, msgProcessor);
}


String settingsProcessor(const String& var)
{
  if (var == "CSS") {
    char buf[2048];
    getSettingsJS(optionType, buf);
    return String(buf);
  }

  #ifdef WLED_ENABLE_DMX

  if (var == "DMXMENU") {
    return String(F("<form action=/settings/dmx><button type=submit>DMX Output</button></form>"));
  }

  #endif
  if (var == "SCSS") return String(FPSTR(PAGE_settingsCss));
  return String();
}

String dmxProcessor(const String& var)
{
  String mapJS;
  #ifdef WLED_ENABLE_DMX
    if (var == "DMXVARS") {
      mapJS += "\nCN=" + String(DMXChannels) + ";\n";
      mapJS += "CS=" + String(DMXStart) + ";\n";
      mapJS += "CG=" + String(DMXGap) + ";\n";
      mapJS += "LC=" + String(ledCount) + ";\n";
      mapJS += "var CH=[";
      for (int i=0;i<15;i++) {
        mapJS += String(DMXFixtureMap[i]) + ",";
      }
      mapJS += "0];";
    }
  #endif

  return mapJS;
}


void serveSettings(AsyncWebServerRequest* request)
{
  byte subPage = 0;
  const String& url = request->url();
  if (url.indexOf("sett") >= 0)
  {
    if (url.indexOf("wifi") > 0) subPage = 1;
    else if (url.indexOf("leds") > 0) subPage = 2;
    else if (url.indexOf("ui") > 0) subPage = 3;
    else if (url.indexOf("sync") > 0) subPage = 4;
    else if (url.indexOf("time") > 0) subPage = 5;
    else if (url.indexOf("sec") > 0) subPage = 6;
    #ifdef WLED_ENABLE_DMX
    else if (url.indexOf("dmx") > 0) subPage = 7;
    #endif
  } else subPage = 255;

  if (subPage == 1 && wifiLock && otaLock)
  {
    serveMessage(request, 500, "Access Denied", "Please unlock OTA in security settings!", 254); return;
  }

  #ifdef WLED_DISABLE_MOBILE_UI
   if (subPage == 255) {serveIndex(request); return;}
  #endif

  optionType = subPage;

  switch (subPage)
  {
    case 1: request->send_P(200, "text/html", PAGE_settings_wifi, settingsProcessor); break;
    case 2: request->send_P(200, "text/html", PAGE_settings_leds, settingsProcessor); break;
    case 3: request->send_P(200, "text/html", PAGE_settings_ui , settingsProcessor); break;
    case 4: request->send_P(200, "text/html", PAGE_settings_sync, settingsProcessor); break;
    case 5: request->send_P(200, "text/html", PAGE_settings_time, settingsProcessor); break;
    case 6: request->send_P(200, "text/html", PAGE_settings_sec , settingsProcessor); break;
    case 7: request->send_P(200, "text/html", PAGE_settings_dmx , settingsProcessor); break;
    case 255: request->send_P(200, "text/html", PAGE_welcome); break;
    default: request->send_P(200, "text/html", PAGE_settings , settingsProcessor);
  }
}
# 1 "C:/Users/RalfBruechmann/Documents/Arduino/code/WLED/wled00/wled19_json.ino"




void deserializeSegment(JsonObject elem, byte it)
{
  byte id = elem["id"] | it;
  if (id < strip.getMaxSegments())
  {
    WS2812FX::Segment& seg = strip.getSegment(id);
    uint16_t start = elem["start"] | seg.start;
    int stop = elem["stop"] | -1;

    if (stop < 0) {
      uint16_t len = elem["len"];
      stop = (len > 0) ? start + len : seg.stop;
    }
    uint16_t grp = elem["grp"] | seg.grouping;
    uint16_t spc = elem["spc"] | seg.spacing;
    strip.setSegment(id, start, stop, grp, spc);

    JsonArray colarr = elem["col"];
    if (!colarr.isNull())
    {
      for (uint8_t i = 0; i < 3; i++)
      {
        JsonArray colX = colarr[i];
        if (colX.isNull()) break;
        byte sz = colX.size();
        if (sz > 0 && sz < 5)
        {
          int rgbw[] = {0,0,0,0};
          byte cp = copyArray(colX, rgbw);
          seg.colors[i] = ((rgbw[3] << 24) | ((rgbw[0]&0xFF) << 16) | ((rgbw[1]&0xFF) << 8) | ((rgbw[2]&0xFF)));
          if (cp == 1 && rgbw[0] == 0) seg.colors[i] = 0;
          if (id == strip.getMainSegmentId())
          {
            if (i == 0) {col[0] = rgbw[0]; col[1] = rgbw[1]; col[2] = rgbw[2]; col[3] = rgbw[3];}
            if (i == 1) {colSec[0] = rgbw[0]; colSec[1] = rgbw[1]; colSec[2] = rgbw[2]; colSec[3] = rgbw[3];}
          }
        }
      }
    }


    seg.setOption(0, elem["sel"] | seg.getOption(0));
    seg.setOption(1, elem["rev"] | seg.getOption(1));


    if (id == strip.getMainSegmentId()) {
      effectCurrent = elem["fx"] | effectCurrent;
      effectSpeed = elem["sx"] | effectSpeed;
      effectIntensity = elem["ix"] | effectIntensity;
      effectPalette = elem["pal"] | effectPalette;
    } else {
      byte fx = elem["fx"] | seg.mode;
      if (fx != seg.mode && fx < strip.getModeCount()) strip.setMode(id, fx);
      seg.speed = elem["sx"] | seg.speed;
      seg.intensity = elem["ix"] | seg.intensity;
      seg.palette = elem["pal"] | seg.palette;
    }
  }
}

bool deserializeState(JsonObject root)
{
  strip.applyToAllSelected = false;
  bool stateResponse = root["v"] | false;

  int ps = root["ps"] | -1;
  if (ps >= 0) applyPreset(ps);

  bri = root["bri"] | bri;

  bool on = root["on"] | (bri > 0);
  if (!on != !bri) toggleOnOff();

  int tr = root["transition"] | -1;
  if (tr >= 0)
  {
    transitionDelay = tr;
    transitionDelay *= 100;
  }

  tr = root["tt"] | -1;
  if (tr >= 0)
  {
    transitionDelayTemp = tr;
    transitionDelayTemp *= 100;
    jsonTransitionOnce = true;
  }

  int cy = root["pl"] | -2;
  if (cy > -2) presetCyclingEnabled = (cy >= 0);
  JsonObject ccnf = root["ccnf"];
  presetCycleMin = ccnf["min"] | presetCycleMin;
  presetCycleMax = ccnf["max"] | presetCycleMax;
  tr = ccnf["time"] | -1;
  if (tr >= 2)
  {
    presetCycleTime = tr;
    presetCycleTime *= 100;
  }

  JsonObject nl = root["nl"];
  nightlightActive = nl["on"] | nightlightActive;
  nightlightDelayMins = nl["dur"] | nightlightDelayMins;
  nightlightFade = nl["fade"] | nightlightFade;
  nightlightTargetBri = nl["tbri"] | nightlightTargetBri;

  JsonObject udpn = root["udpn"];
  notifyDirect = udpn["send"] | notifyDirect;
  receiveNotifications = udpn["recv"] | receiveNotifications;
  bool noNotification = udpn["nn"];

  int timein = root["time"] | -1;
  if (timein != -1) setTime(timein);

  byte prevMain = strip.getMainSegmentId();
  strip.mainSegment = root["mainseg"] | prevMain;
  if (strip.getMainSegmentId() != prevMain) setValuesFromMainSeg();

  int it = 0;
  JsonVariant segVar = root["seg"];
  if (segVar.is<JsonObject>())
  {
    int id = segVar["id"] | -1;

    if (id < 0) {
      bool didSet = false;
      byte lowestActive = 99;
      for (byte s = 0; s < strip.getMaxSegments(); s++)
      {
        WS2812FX::Segment sg = strip.getSegment(s);
        if (sg.isActive())
        {
          if (lowestActive == 99) lowestActive = s;
          if (sg.isSelected()) {
            deserializeSegment(segVar, s);
            didSet = true;
          }
        }
      }
      if (!didSet && lowestActive < strip.getMaxSegments()) deserializeSegment(segVar, lowestActive);
    } else {
      deserializeSegment(segVar, it);
    }
  } else {
    JsonArray segs = segVar.as<JsonArray>();
    for (JsonObject elem : segs)
    {
      deserializeSegment(elem, it);
      it++;
    }
  }

  colorUpdated(noNotification ? NOTIFIER_CALL_MODE_NO_NOTIFY : NOTIFIER_CALL_MODE_DIRECT_CHANGE);

  ps = root["psave"] | -1;
  if (ps >= 0) savePreset(ps);

  return stateResponse;
}

void serializeSegment(JsonObject& root, WS2812FX::Segment& seg, byte id)
{
 root["id"] = id;
 root["start"] = seg.start;
 root["stop"] = seg.stop;
 root["len"] = seg.stop - seg.start;
  root["grp"] = seg.grouping;
  root["spc"] = seg.spacing;

 JsonArray colarr = root.createNestedArray("col");

 for (uint8_t i = 0; i < 3; i++)
 {
  JsonArray colX = colarr.createNestedArray();
  colX.add((seg.colors[i] >> 16) & 0xFF);
  colX.add((seg.colors[i] >> 8) & 0xFF);
  colX.add((seg.colors[i]) & 0xFF);
  if (useRGBW)
   colX.add((seg.colors[i] >> 24) & 0xFF);
 }

 root["fx"] = seg.mode;
 root["sx"] = seg.speed;
 root["ix"] = seg.intensity;
 root["pal"] = seg.palette;
 root["sel"] = seg.isSelected();
 root["rev"] = seg.getOption(1);
}


void serializeState(JsonObject root)
{
  if (errorFlag) root["error"] = errorFlag;

  root["on"] = (bri > 0);
  root["bri"] = briLast;
  root["transition"] = transitionDelay/100;

  root["ps"] = currentPreset;
  root["pss"] = savedPresets;
  root["pl"] = (presetCyclingEnabled) ? 0: -1;


  JsonObject ccnf = root.createNestedObject("ccnf");
  ccnf["min"] = presetCycleMin;
  ccnf["max"] = presetCycleMax;
  ccnf["time"] = presetCycleTime/100;

  JsonObject nl = root.createNestedObject("nl");
  nl["on"] = nightlightActive;
  nl["dur"] = nightlightDelayMins;
  nl["fade"] = nightlightFade;
  nl["tbri"] = nightlightTargetBri;

  JsonObject udpn = root.createNestedObject("udpn");
  udpn["send"] = notifyDirect;
  udpn["recv"] = receiveNotifications;

  root["mainseg"] = strip.getMainSegmentId();

  JsonArray seg = root.createNestedArray("seg");
  for (byte s = 0; s < strip.getMaxSegments(); s++)
  {
    WS2812FX::Segment sg = strip.getSegment(s);
    if (sg.isActive())
    {
      JsonObject seg0 = seg.createNestedObject();
      serializeSegment(seg0, sg, s);
    }
  }
}

void serializeInfo(JsonObject root)
{
  root["ver"] = versionString;
  root["vid"] = VERSION;

  JsonObject leds = root.createNestedObject("leds");
  leds["count"] = ledCount;
  leds["rgbw"] = useRGBW;
  leds["wv"] = useRGBW && (strip.rgbwMode == RGBW_MODE_MANUAL_ONLY || strip.rgbwMode == RGBW_MODE_DUAL);
  JsonArray leds_pin = leds.createNestedArray("pin");
  leds_pin.add(LEDPIN);

  leds["pwr"] = strip.currentMilliamps;
  leds["maxpwr"] = (strip.currentMilliamps)? strip.ablMilliampsMax : 0;
  leds["maxseg"] = strip.getMaxSegments();
  leds["seglock"] = false;

  root["str"] = syncToggleReceive;

  root["name"] = serverDescription;
  root["udpport"] = udpPort;
  root["live"] = (bool)realtimeMode;
  root["fxcount"] = strip.getModeCount();
  root["palcount"] = strip.getPaletteCount();

  JsonObject wifi_info = root.createNestedObject("wifi");
  wifi_info["bssid"] = WiFi.BSSIDstr();
  int qrssi = WiFi.RSSI();
  wifi_info["rssi"] = qrssi;
  wifi_info["signal"] = getSignalQuality(qrssi);
  wifi_info["channel"] = WiFi.channel();

  #ifdef ARDUINO_ARCH_ESP32
  #ifdef WLED_DEBUG
    wifi_info["txPower"] = (int) WiFi.getTxPower();
    wifi_info["sleep"] = (bool) WiFi.getSleep();
  #endif
  root["arch"] = "esp32";
  root["core"] = ESP.getSdkVersion();

  #ifdef WLED_DEBUG
    root["resetReason0"] = (int)rtc_get_reset_reason(0);
    root["resetReason1"] = (int)rtc_get_reset_reason(1);
  #endif
  root["lwip"] = 0;
  #else
  root["arch"] = "esp8266";
  root["core"] = ESP.getCoreVersion();

  #ifdef WLED_DEBUG
    root["resetReason"] = (int)ESP.getResetInfoPtr()->reason;
  #endif
  root["lwip"] = LWIP_VERSION_MAJOR;
  #endif

  root["freeheap"] = ESP.getFreeHeap();
  root["uptime"] = millis()/1000;

  byte os = 0;
  #ifdef WLED_DEBUG
  os = 0x80;
  #endif
  #ifndef WLED_DISABLE_ALEXA
  os += 0x40;
  #endif
  #ifndef WLED_DISABLE_BLYNK
  os += 0x20;
  #endif
  #ifndef WLED_DISABLE_CRONIXIE
  os += 0x10;
  #endif
  #ifndef WLED_DISABLE_FILESYSTEM
  os += 0x08;
  #endif
  #ifndef WLED_DISABLE_HUESYNC
  os += 0x04;
  #endif
  #ifdef WLED_ENABLE_ADALIGHT
  os += 0x02;
  #endif
  #ifndef WLED_DISABLE_OTA
  os += 0x01;
  #endif
  root["opt"] = os;

  root["brand"] = "WLED";
  root["product"] = "DIY light";
  root["mac"] = escapedMac;
}

void serveJson(AsyncWebServerRequest* request)
{
  byte subJson = 0;
  const String& url = request->url();
  if (url.indexOf("state") > 0) subJson = 1;
  else if (url.indexOf("info") > 0) subJson = 2;
  else if (url.indexOf("live") > 0) {
    serveLiveLeds(request);
    return;
  }
  else if (url.indexOf("eff") > 0) {
    request->send_P(200, "application/json", JSON_mode_names);
    return;
  }
  else if (url.indexOf("pal") > 0) {
    request->send_P(200, "application/json", JSON_palette_names);
    return;
  }
  else if (url.length() > 6) {
    request->send( 501, "application/json", F("{\"error\":\"Not implemented\"}"));
    return;
  }

  AsyncJsonResponse* response = new AsyncJsonResponse();
  JsonObject doc = response->getRoot();

  switch (subJson)
  {
    case 1:
      serializeState(doc); break;
    case 2:
      serializeInfo(doc); break;
    default:
      JsonObject state = doc.createNestedObject("state");
      serializeState(state);
      JsonObject info = doc.createNestedObject("info");
      serializeInfo(info);
      doc["effects"] = serialized((const __FlashStringHelper*)JSON_mode_names);
      doc["palettes"] = serialized((const __FlashStringHelper*)JSON_palette_names);
  }

  response->setLength();
  request->send(response);
}

#define MAX_LIVE_LEDS 180

void serveLiveLeds(AsyncWebServerRequest* request)
{
  byte used = ledCount;
  byte n = (used -1) /MAX_LIVE_LEDS +1;
  char buffer[2000] = "{\"leds\":[";
  olen = 9;
  obuf = buffer;

  for (uint16_t i= 0; i < used; i += n)
  {
    olen += sprintf(buffer + olen, "\"%06X\",", strip.getPixelColor(i));
  }
  olen -= 1;
  oappend("],\"n\":");
  oappendi(n);
  oappend("}");
  request->send(200, "application/json", buffer);
}
# 1 "C:/Users/RalfBruechmann/Documents/Arduino/code/WLED/wled00/wled20_ir.ino"




#if defined(WLED_DISABLE_INFRARED)
void handleIR(){}
#else

IRrecv* irrecv;


decode_results results;

unsigned long irCheckedTime = 0;
uint32_t lastValidCode = 0;
uint16_t irTimesRepeated = 0;
uint8_t lastIR6ColourIdx = 0;




bool decodeIRCustom(uint32_t code)
{
  switch (code)
  {

    case IRCUSTOM_ONOFF : toggleOnOff(); break;
    case IRCUSTOM_MACRO1 : applyMacro(1); break;

    default: return false;
  }
  if (code != IRCUSTOM_MACRO1) colorUpdated(NOTIFIER_CALL_MODE_BUTTON);
  return true;
}



void relativeChange(byte* property, int8_t amount, byte lowerBoundary =0)
{
  int16_t new_val = (int16_t) *property + amount;
  if (new_val > 0xFF) new_val = 0xFF;
  else if (new_val < lowerBoundary) new_val = lowerBoundary;
  *property = new_val;
}


void decodeIR(uint32_t code)
{
  if (code == 0xFFFFFFFF)
  {
    irTimesRepeated++;
    if (lastValidCode == IR24_BRIGHTER || lastValidCode == IR40_BPLUS )
    {
      relativeChange(&bri, 10); colorUpdated(NOTIFIER_CALL_MODE_BUTTON);
    }
    else if (lastValidCode == IR24_DARKER || lastValidCode == IR40_BMINUS )
    {
      relativeChange(&bri, -10, 5); colorUpdated(NOTIFIER_CALL_MODE_BUTTON);
    }
    if (lastValidCode == IR40_WPLUS)
    {
      relativeChangeWhite(10); colorUpdated(NOTIFIER_CALL_MODE_BUTTON);
    }
    else if (lastValidCode == IR40_WMINUS)
    {
      relativeChangeWhite(-10, 5); colorUpdated(NOTIFIER_CALL_MODE_BUTTON);
    }
    else if ((lastValidCode == IR24_ON || lastValidCode == IR40_ON) && irTimesRepeated > 7 )
    {
      nightlightActive = true;
      nightlightStartTime = millis();
      colorUpdated(NOTIFIER_CALL_MODE_BUTTON);
    }
    return;
  }
  lastValidCode = 0; irTimesRepeated = 0;

  if (decodeIRCustom(code)) return;
  if (code > 0xFFFFFF) return;
  else if (code > 0xF70000 && code < 0xF80000) decodeIR24(code);
  else if (code > 0xFF0000) {
    switch (irEnabled) {
      case 1: decodeIR24OLD(code); break;
      case 2: decodeIR24CT(code); break;
      case 3: decodeIR40(code); break;
      case 4: decodeIR44(code); break;
      case 5: decodeIR21(code); break;
      case 6: decodeIR6(code); break;


      default: return;
    }
    colorUpdated(NOTIFIER_CALL_MODE_BUTTON);
  }

}


void decodeIR24(uint32_t code)
{
  switch (code) {
    case IR24_BRIGHTER : relativeChange(&bri, 10); break;
    case IR24_DARKER : relativeChange(&bri, -10, 5); break;
    case IR24_OFF : briLast = bri; bri = 0; break;
    case IR24_ON : bri = briLast; break;
    case IR24_RED : colorFromUint32(COLOR_RED); break;
    case IR24_REDDISH : colorFromUint32(COLOR_REDDISH); break;
    case IR24_ORANGE : colorFromUint32(COLOR_ORANGE); break;
    case IR24_YELLOWISH : colorFromUint32(COLOR_YELLOWISH); break;
    case IR24_YELLOW : colorFromUint32(COLOR_YELLOW); break;
    case IR24_GREEN : colorFromUint32(COLOR_GREEN); break;
    case IR24_GREENISH : colorFromUint32(COLOR_GREENISH); break;
    case IR24_TURQUOISE : colorFromUint32(COLOR_TURQUOISE); break;
    case IR24_CYAN : colorFromUint32(COLOR_CYAN); break;
    case IR24_AQUA : colorFromUint32(COLOR_AQUA); break;
    case IR24_BLUE : colorFromUint32(COLOR_BLUE); break;
    case IR24_DEEPBLUE : colorFromUint32(COLOR_DEEPBLUE); break;
    case IR24_PURPLE : colorFromUint32(COLOR_PURPLE); break;
    case IR24_MAGENTA : colorFromUint32(COLOR_MAGENTA); break;
    case IR24_PINK : colorFromUint32(COLOR_PINK); break;
    case IR24_WHITE : colorFromUint32(COLOR_WHITE); effectCurrent = 0; break;
    case IR24_FLASH : if (!applyPreset(1)) effectCurrent = FX_MODE_COLORTWINKLE; break;
    case IR24_STROBE : if (!applyPreset(2)) effectCurrent = FX_MODE_RAINBOW_CYCLE; break;
    case IR24_FADE : if (!applyPreset(3)) effectCurrent = FX_MODE_BREATH; break;
    case IR24_SMOOTH : if (!applyPreset(4)) effectCurrent = FX_MODE_RAINBOW; break;
    default: return;
  }
  lastValidCode = code;
}

void decodeIR24OLD(uint32_t code)
{
  switch (code) {
    case IR24_OLD_BRIGHTER : relativeChange(&bri, 10); break;
    case IR24_OLD_DARKER : relativeChange(&bri, -10, 5); break;
    case IR24_OLD_OFF : briLast = bri; bri = 0; break;
    case IR24_OLD_ON : bri = briLast; break;
    case IR24_OLD_RED : colorFromUint32(COLOR_RED); break;
    case IR24_OLD_REDDISH : colorFromUint32(COLOR_REDDISH); break;
    case IR24_OLD_ORANGE : colorFromUint32(COLOR_ORANGE); break;
    case IR24_OLD_YELLOWISH : colorFromUint32(COLOR_YELLOWISH); break;
    case IR24_OLD_YELLOW : colorFromUint32(COLOR_YELLOW); break;
    case IR24_OLD_GREEN : colorFromUint32(COLOR_GREEN); break;
    case IR24_OLD_GREENISH : colorFromUint32(COLOR_GREENISH); break;
    case IR24_OLD_TURQUOISE : colorFromUint32(COLOR_TURQUOISE); break;
    case IR24_OLD_CYAN : colorFromUint32(COLOR_CYAN); break;
    case IR24_OLD_AQUA : colorFromUint32(COLOR_AQUA); break;
    case IR24_OLD_BLUE : colorFromUint32(COLOR_BLUE); break;
    case IR24_OLD_DEEPBLUE : colorFromUint32(COLOR_DEEPBLUE); break;
    case IR24_OLD_PURPLE : colorFromUint32(COLOR_PURPLE); break;
    case IR24_OLD_MAGENTA : colorFromUint32(COLOR_MAGENTA); break;
    case IR24_OLD_PINK : colorFromUint32(COLOR_PINK); break;
    case IR24_OLD_WHITE : colorFromUint32(COLOR_WHITE); effectCurrent = 0; break;
    case IR24_OLD_FLASH : if (!applyPreset(1)) { effectCurrent = FX_MODE_COLORTWINKLE; effectPalette = 0; } break;
    case IR24_OLD_STROBE : if (!applyPreset(2)) { effectCurrent = FX_MODE_RAINBOW_CYCLE; effectPalette = 0; } break;
    case IR24_OLD_FADE : if (!applyPreset(3)) { effectCurrent = FX_MODE_BREATH; effectPalette = 0; } break;
    case IR24_OLD_SMOOTH : if (!applyPreset(4)) { effectCurrent = FX_MODE_RAINBOW; effectPalette = 0; } break;
    default: return;
  }
  lastValidCode = code;
}


void decodeIR24CT(uint32_t code)
{
  switch (code) {
    case IR24_CT_BRIGHTER : relativeChange(&bri, 10); break;
    case IR24_CT_DARKER : relativeChange(&bri, -10, 5); break;
    case IR24_CT_OFF : briLast = bri; bri = 0; break;
    case IR24_CT_ON : bri = briLast; break;
    case IR24_CT_RED : colorFromUint32(COLOR_RED); break;
    case IR24_CT_REDDISH : colorFromUint32(COLOR_REDDISH); break;
    case IR24_CT_ORANGE : colorFromUint32(COLOR_ORANGE); break;
    case IR24_CT_YELLOWISH : colorFromUint32(COLOR_YELLOWISH); break;
    case IR24_CT_YELLOW : colorFromUint32(COLOR_YELLOW); break;
    case IR24_CT_GREEN : colorFromUint32(COLOR_GREEN); break;
    case IR24_CT_GREENISH : colorFromUint32(COLOR_GREENISH); break;
    case IR24_CT_TURQUOISE : colorFromUint32(COLOR_TURQUOISE); break;
    case IR24_CT_CYAN : colorFromUint32(COLOR_CYAN); break;
    case IR24_CT_AQUA : colorFromUint32(COLOR_AQUA); break;
    case IR24_CT_BLUE : colorFromUint32(COLOR_BLUE); break;
    case IR24_CT_DEEPBLUE : colorFromUint32(COLOR_DEEPBLUE); break;
    case IR24_CT_PURPLE : colorFromUint32(COLOR_PURPLE); break;
    case IR24_CT_MAGENTA : colorFromUint32(COLOR_MAGENTA); break;
    case IR24_CT_PINK : colorFromUint32(COLOR_PINK); break;
    case IR24_CT_COLDWHITE : colorFromUint32(COLOR2_COLDWHITE); effectCurrent = 0; break;
    case IR24_CT_WARMWHITE : colorFromUint32(COLOR2_WARMWHITE); effectCurrent = 0; break;
    case IR24_CT_CTPLUS : colorFromUint32(COLOR2_COLDWHITE2); effectCurrent = 0; break;
    case IR24_CT_CTMINUS : colorFromUint32(COLOR2_WARMWHITE2); effectCurrent = 0; break;
    case IR24_CT_MEMORY : {
      if (col[3] > 0) col[3] = 0;
      else colorFromUint32(COLOR2_NEUTRALWHITE); effectCurrent = 0; } break;
    default: return;
  }
  lastValidCode = code;
}


void decodeIR40(uint32_t code)
{
  switch (code) {
    case IR40_BPLUS : relativeChange(&bri, 10); break;
    case IR40_BMINUS : relativeChange(&bri, -10, 5); break;
    case IR40_OFF : briLast = bri; bri = 0; break;
    case IR40_ON : bri = briLast; break;
    case IR40_RED : colorFromUint24(COLOR_RED); break;
    case IR40_REDDISH : colorFromUint24(COLOR_REDDISH); break;
    case IR40_ORANGE : colorFromUint24(COLOR_ORANGE); break;
    case IR40_YELLOWISH : colorFromUint24(COLOR_YELLOWISH); break;
    case IR40_YELLOW : colorFromUint24(COLOR_YELLOW); break;
    case IR40_GREEN : colorFromUint24(COLOR_GREEN); break;
    case IR40_GREENISH : colorFromUint24(COLOR_GREENISH); break;
    case IR40_TURQUOISE : colorFromUint24(COLOR_TURQUOISE); break;
    case IR40_CYAN : colorFromUint24(COLOR_CYAN); break;
    case IR40_AQUA : colorFromUint24(COLOR_AQUA); break;
    case IR40_BLUE : colorFromUint24(COLOR_BLUE); break;
    case IR40_DEEPBLUE : colorFromUint24(COLOR_DEEPBLUE); break;
    case IR40_PURPLE : colorFromUint24(COLOR_PURPLE); break;
    case IR40_MAGENTA : colorFromUint24(COLOR_MAGENTA); break;
    case IR40_PINK : colorFromUint24(COLOR_PINK); break;
    case IR40_WARMWHITE2 : {
      if (useRGBW) { colorFromUint32(COLOR2_WARMWHITE2); effectCurrent = 0; }
      else colorFromUint24(COLOR_WARMWHITE2); } break;
    case IR40_WARMWHITE : {
      if (useRGBW) { colorFromUint32(COLOR2_WARMWHITE); effectCurrent = 0; }
      else colorFromUint24(COLOR_WARMWHITE); } break;
    case IR40_WHITE : {
      if (useRGBW) { colorFromUint32(COLOR2_NEUTRALWHITE); effectCurrent = 0; }
      else colorFromUint24(COLOR_NEUTRALWHITE); } break;
    case IR40_COLDWHITE : {
      if (useRGBW) { colorFromUint32(COLOR2_COLDWHITE); effectCurrent = 0; }
      else colorFromUint24(COLOR_COLDWHITE); } break;
    case IR40_COLDWHITE2 : {
      if (useRGBW) { colorFromUint32(COLOR2_COLDWHITE2); effectCurrent = 0; }
      else colorFromUint24(COLOR_COLDWHITE2); } break;
    case IR40_WPLUS : relativeChangeWhite(10); break;
    case IR40_WMINUS : relativeChangeWhite(-10, 5); break;
    case IR40_WOFF : whiteLast = col[3]; col[3] = 0; break;
    case IR40_WON : col[3] = whiteLast; break;
    case IR40_W25 : bri = 63; break;
    case IR40_W50 : bri = 127; break;
    case IR40_W75 : bri = 191; break;
    case IR40_W100 : bri = 255; break;
    case IR40_QUICK : relativeChange(&effectSpeed, 10); break;
    case IR40_SLOW : relativeChange(&effectSpeed, -10, 5); break;
    case IR40_JUMP7 : relativeChange(&effectIntensity, 10); break;
    case IR40_AUTO : relativeChange(&effectIntensity, -10, 5); break;
    case IR40_JUMP3 : if (!applyPreset(1)) { effectCurrent = FX_MODE_STATIC; effectPalette = 0; } break;
    case IR40_FADE3 : if (!applyPreset(2)) { effectCurrent = FX_MODE_BREATH; effectPalette = 0; } break;
    case IR40_FADE7 : if (!applyPreset(3)) { effectCurrent = FX_MODE_FIRE_FLICKER; effectPalette = 0; } break;
    case IR40_FLASH : if (!applyPreset(4)) { effectCurrent = FX_MODE_RAINBOW; effectPalette = 0; } break;
  }
  lastValidCode = code;
}

void decodeIR44(uint32_t code)
{
  switch (code) {
    case IR44_BPLUS : relativeChange(&bri, 10); break;
    case IR44_BMINUS : relativeChange(&bri, -10, 5); break;
    case IR44_OFF : briLast = bri; bri = 0; break;
    case IR44_ON : bri = briLast; break;
    case IR44_RED : colorFromUint24(COLOR_RED); break;
    case IR44_REDDISH : colorFromUint24(COLOR_REDDISH); break;
    case IR44_ORANGE : colorFromUint24(COLOR_ORANGE); break;
    case IR44_YELLOWISH : colorFromUint24(COLOR_YELLOWISH); break;
    case IR44_YELLOW : colorFromUint24(COLOR_YELLOW); break;
    case IR44_GREEN : colorFromUint24(COLOR_GREEN); break;
    case IR44_GREENISH : colorFromUint24(COLOR_GREENISH); break;
    case IR44_TURQUOISE : colorFromUint24(COLOR_TURQUOISE); break;
    case IR44_CYAN : colorFromUint24(COLOR_CYAN); break;
    case IR44_AQUA : colorFromUint24(COLOR_AQUA); break;
    case IR44_BLUE : colorFromUint24(COLOR_BLUE); break;
    case IR44_DEEPBLUE : colorFromUint24(COLOR_DEEPBLUE); break;
    case IR44_PURPLE : colorFromUint24(COLOR_PURPLE); break;
    case IR44_MAGENTA : colorFromUint24(COLOR_MAGENTA); break;
    case IR44_PINK : colorFromUint24(COLOR_PINK); break;
    case IR44_WHITE : {
      if (useRGBW) {
        if (col[3] > 0) col[3] = 0;
        else { colorFromUint32(COLOR2_NEUTRALWHITE); effectCurrent = 0; }
      } else colorFromUint24(COLOR_NEUTRALWHITE); } break;
    case IR44_WARMWHITE2 : {
      if (useRGBW) { colorFromUint32(COLOR2_WARMWHITE2); effectCurrent = 0; }
      else colorFromUint24(COLOR_WARMWHITE2); } break;
    case IR44_WARMWHITE : {
      if (useRGBW) { colorFromUint32(COLOR2_WARMWHITE); effectCurrent = 0; }
      else colorFromUint24(COLOR_WARMWHITE); } break;
    case IR44_COLDWHITE : {
      if (useRGBW) { colorFromUint32(COLOR2_COLDWHITE); effectCurrent = 0; }
      else colorFromUint24(COLOR_COLDWHITE); } break;
    case IR44_COLDWHITE2 : {
      if (useRGBW) { colorFromUint32(COLOR2_COLDWHITE2); effectCurrent = 0; }
      else colorFromUint24(COLOR_COLDWHITE2); } break;
    case IR44_REDPLUS : relativeChange(&effectCurrent, 1); break;
    case IR44_REDMINUS : relativeChange(&effectCurrent, -1, 0); break;
    case IR44_GREENPLUS : relativeChange(&effectPalette, 1); break;
    case IR44_GREENMINUS : relativeChange(&effectPalette, -1, 0); break;
    case IR44_BLUEPLUS : relativeChange(&effectIntensity, 10); break;
    case IR44_BLUEMINUS : relativeChange(&effectIntensity, -10, 5); break;
    case IR44_QUICK : relativeChange(&effectSpeed, 10); break;
    case IR44_SLOW : relativeChange(&effectSpeed, -10, 5); break;
    case IR44_DIY1 : if (!applyPreset(1)) { effectCurrent = FX_MODE_STATIC; effectPalette = 0; } break;
    case IR44_DIY2 : if (!applyPreset(2)) { effectCurrent = FX_MODE_BREATH; effectPalette = 0; } break;
    case IR44_DIY3 : if (!applyPreset(3)) { effectCurrent = FX_MODE_FIRE_FLICKER; effectPalette = 0; } break;
    case IR44_DIY4 : if (!applyPreset(4)) { effectCurrent = FX_MODE_RAINBOW; effectPalette = 0; } break;
    case IR44_DIY5 : if (!applyPreset(5)) { effectCurrent = FX_MODE_METEOR_SMOOTH; effectPalette = 0; } break;
    case IR44_DIY6 : if (!applyPreset(6)) { effectCurrent = FX_MODE_RAIN; effectPalette = 0; } break;
    case IR44_AUTO : effectCurrent = FX_MODE_STATIC; break;
    case IR44_FLASH : effectCurrent = FX_MODE_PALETTE; break;
    case IR44_JUMP3 : bri = 63; break;
    case IR44_JUMP7 : bri = 127; break;
    case IR44_FADE3 : bri = 191; break;
    case IR44_FADE7 : bri = 255; break;
  }
  lastValidCode = code;
}

void decodeIR21(uint32_t code)
{
  switch (code) {
    case IR21_BRIGHTER: relativeChange(&bri, 10); break;
    case IR21_DARKER: relativeChange(&bri, -10, 5); break;
    case IR21_OFF: briLast = bri; bri = 0; break;
    case IR21_ON: bri = briLast; break;
    case IR21_RED: colorFromUint32(COLOR_RED); break;
    case IR21_REDDISH: colorFromUint32(COLOR_REDDISH); break;
    case IR21_ORANGE: colorFromUint32(COLOR_ORANGE); break;
    case IR21_YELLOWISH: colorFromUint32(COLOR_YELLOWISH); break;
    case IR21_GREEN: colorFromUint32(COLOR_GREEN); break;
    case IR21_GREENISH: colorFromUint32(COLOR_GREENISH); break;
    case IR21_TURQUOISE: colorFromUint32(COLOR_TURQUOISE); break;
    case IR21_CYAN: colorFromUint32(COLOR_CYAN); break;
    case IR21_BLUE: colorFromUint32(COLOR_BLUE); break;
    case IR21_DEEPBLUE: colorFromUint32(COLOR_DEEPBLUE); break;
    case IR21_PURPLE: colorFromUint32(COLOR_PURPLE); break;
    case IR21_PINK: colorFromUint32(COLOR_PINK); break;
    case IR21_WHITE: colorFromUint32(COLOR_WHITE); effectCurrent = 0; break;
    case IR21_FLASH: if (!applyPreset(1)) { effectCurrent = FX_MODE_COLORTWINKLE; effectPalette = 0; } break;
    case IR21_STROBE: if (!applyPreset(2)) { effectCurrent = FX_MODE_RAINBOW_CYCLE; effectPalette = 0; } break;
    case IR21_FADE: if (!applyPreset(3)) { effectCurrent = FX_MODE_BREATH; effectPalette = 0; } break;
    case IR21_SMOOTH: if (!applyPreset(4)) { effectCurrent = FX_MODE_RAINBOW; effectPalette = 0; } break;
    default: return;
  }
  lastValidCode = code;
}

void decodeIR6(uint32_t code)
{
  switch (code) {
    case IR6_POWER: toggleOnOff(); break;
    case IR6_CHANNEL_UP: relativeChange(&bri, 10); break;
    case IR6_CHANNEL_DOWN: relativeChange(&bri, -10, 5); break;
    case IR6_VOLUME_UP: relativeChange(&effectCurrent, 1); break;
    case IR6_VOLUME_DOWN:

      relativeChange(&effectPalette, 1);
      switch(lastIR6ColourIdx) {
        case 0: colorFromUint32(COLOR_RED); break;
        case 1: colorFromUint32(COLOR_REDDISH); break;
        case 2:colorFromUint32(COLOR_ORANGE); break;
        case 3:colorFromUint32(COLOR_YELLOWISH); break;
        case 4:colorFromUint32(COLOR_GREEN); break;
        case 5:colorFromUint32(COLOR_GREENISH); break;
        case 6:colorFromUint32(COLOR_TURQUOISE); break;
        case 7: colorFromUint32(COLOR_CYAN); break;
        case 8:colorFromUint32(COLOR_BLUE); break;
        case 9:colorFromUint32(COLOR_DEEPBLUE); break;
        case 10:colorFromUint32(COLOR_PURPLE); break;
        case 11:colorFromUint32(COLOR_PINK); break;
        case 12:colorFromUint32(COLOR_WHITE); break;
        default:break;
      }
      lastIR6ColourIdx++;
      if(lastIR6ColourIdx > 12) lastIR6ColourIdx = 0;
      break;
    case IR6_MUTE: effectCurrent = 0; effectPalette = 0; colorFromUint32(COLOR_WHITE); bri=255; break;
  }
  lastValidCode = code;
}


void initIR()
{
  if (irEnabled > 0)
  {
    irrecv = new IRrecv(IR_PIN);
    irrecv->enableIRIn();
  }
}


void handleIR()
{
  if (irEnabled > 0 && millis() - irCheckedTime > 120)
  {
    irCheckedTime = millis();
    if (irEnabled > 0)
    {
      if (irrecv == NULL)
      {
        initIR(); return;
      }

      if (irrecv->decode(&results))
      {
        if (results.value != 0)
        {
          Serial.print("IR recv\r\n0x");
          Serial.println((uint32_t)results.value, HEX);
          Serial.println();
        }
        decodeIR(results.value);
        irrecv->resume();
      }
    } else if (irrecv != NULL)
    {
      irrecv->disableIRIn();
      delete irrecv; irrecv = NULL;
    }
  }
}

#endif
# 1 "C:/Users/RalfBruechmann/Documents/Arduino/code/WLED/wled00/wled21_dmx.ino"





#ifdef WLED_ENABLE_DMX

void handleDMX() {


  uint8_t brightness = strip.getBrightness();

  for (int i = 0; i < ledCount; i++) {

    uint32_t in = strip.getPixelColor(i);
    byte w = in >> 24 & 0xFF;
    byte r = in >> 16 & 0xFF;
    byte g = in >> 8 & 0xFF;
    byte b = in & 0xFF;

    int DMXFixtureStart = DMXStart + (DMXGap * i);
    for (int j = 0; j < DMXChannels; j++) {
      int DMXAddr = DMXFixtureStart + j;
      switch (DMXFixtureMap[j]) {
        case 0:
          dmx.write(DMXAddr, 0);
          break;
        case 1:
          dmx.write(DMXAddr, r);
          break;
        case 2:
          dmx.write(DMXAddr, g);
          break;
        case 3:
          dmx.write(DMXAddr, b);
          break;
        case 4:
          dmx.write(DMXAddr, w);
          break;
        case 5:
          dmx.write(DMXAddr, brightness);
          break;
        case 6:
          dmx.write(DMXAddr, 255);
          break;
      }
    }
  }

  dmx.update();
}

#else
void handleDMX() {}
#endif