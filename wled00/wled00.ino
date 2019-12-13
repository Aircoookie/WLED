/*
 * Main sketch, global variable declarations
 */
/*
 * @title WLED project sketch
 * @version 0.9.0-b1
 * @author Christian Schwinne
 */


//ESP8266-01 (blue) got too little storage space to work with all features of WLED. To use it, you must use ESP8266 Arduino Core v2.4.2 and the setting 512K(No SPIFFS).

//ESP8266-01 (black) has 1MB flash and can thus fit the whole program. Use 1M(64K SPIFFS).
//Uncomment some of the following lines to disable features to compile for ESP8266-01 (max flash size 434kB):

//You are required to disable over-the-air updates:
//#define WLED_DISABLE_OTA         //saves 14kb

//You need to choose some of these features to disable:
//#define WLED_DISABLE_ALEXA       //saves 11kb
//#define WLED_DISABLE_BLYNK       //saves 6kb
//#define WLED_DISABLE_CRONIXIE    //saves 3kb
//#define WLED_DISABLE_HUESYNC     //saves 4kb
//#define WLED_DISABLE_INFRARED    //there is no pin left for this on ESP8266-01, saves 25kb (!)
#define WLED_ENABLE_MQTT           //saves 12kb
#define WLED_ENABLE_ADALIGHT       //saves 500b only

#define WLED_DISABLE_FILESYSTEM    //SPIFFS is not used by any WLED feature yet
//#define WLED_ENABLE_FS_SERVING   //Enable sending html file from SPIFFS before serving progmem version
//#define WLED_ENABLE_FS_EDITOR    //enable /edit page for editing SPIFFS content. Will also be disabled with OTA lock

//to toggle usb serial debug (un)comment the following line
//#define WLED_DEBUG


//library inclusions
#include <Arduino.h>
#ifdef ESP8266
 #include <ESP8266WiFi.h>
 #include <ESP8266mDNS.h>
 #include <ESPAsyncTCP.h>
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
 //#define ESPALEXA_DEBUG
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


#if IR_PIN < 0
 #ifndef WLED_DISABLE_INFRARED
  #define WLED_DISABLE_INFRARED
 #endif
#endif

#ifdef ARDUINO_ARCH_ESP32
 /*#ifndef WLED_DISABLE_INFRARED
  #include <IRremote.h>
 #endif*/ //there are issues with ESP32 infrared, so it is disabled for now
#else
 #ifndef WLED_DISABLE_INFRARED
  #include <IRremoteESP8266.h>
  #include <IRrecv.h>
  #include <IRutils.h>
 #endif
#endif


//version code in format yymmddb (b = daily build)
#define VERSION 1912121
char versionString[] = "0.9.0-b1";


//AP and OTA default passwords (for maximum change them!)
char apPass[65] = "wled1234";
char otaPass[33] = "wledota";


//Hardware CONFIG (only changeble HERE, not at runtime)
//LED strip pin, button pin and IR pin changeable in NpbWrapper.h!

byte auxDefaultState   = 0;                   //0: input 1: high 2: low
byte auxTriggeredState = 0;                   //0: input 1: high 2: low
char ntpServerName[33] = "0.wled.pool.ntp.org";//NTP server to use


//WiFi CONFIG (all these can be changed via web UI, no need to set them here)
char clientSSID[33] = "Your_Network";
char clientPass[65] = "";
char cmDNS[33] = "x";                         //mDNS address (placeholder, will be replaced by wledXXXXXXXXXXXX.local)
char apSSID[33] = "";                         //AP off by default (unless setup)
byte apChannel = 1;                           //2.4GHz WiFi AP channel (1-13)
byte apHide = 0;                              //hidden AP SSID
//byte apWaitTimeSecs = 32;                   //time to wait for connection before opening AP
byte apBehavior = 0;                          //0: Open AP when no connection after boot 1: Open when no connection 2: Always open 3: Only when button pressed for 6 sec
//bool recoveryAPDisabled = false;            //never open AP (not recommended)
IPAddress staticIP(0, 0, 0, 0);               //static IP of ESP
IPAddress staticGateway(0, 0, 0, 0);          //gateway (router) IP
IPAddress staticSubnet(255, 255, 255, 0);     //most common subnet in home networks

//LED CONFIG
uint16_t ledCount = 30;                       //overcurrent prevented by ABL
bool useRGBW = false;                         //SK6812 strips can contain an extra White channel
bool autoRGBtoRGBW = false;                   //if RGBW enabled, calculate White channel from RGB
#define ABL_MILLIAMPS_DEFAULT 850;            //auto lower brightness to stay close to milliampere limit
bool turnOnAtBoot  = true;                    //turn on LEDs at power-up
byte bootPreset = 0;                          //save preset to load after power-up

byte col[]{255, 160, 0, 0};                   //default RGB(W) color
byte colSec[]{0, 0, 0, 0};                    //default RGB(W) secondary color
byte briS = 128;                              //default brightness

byte nightlightTargetBri = 0;                 //brightness after nightlight is over
byte nightlightDelayMins = 60;
bool nightlightFade = true;                   //if enabled, light will gradually dim towards the target bri. Otherwise, it will instantly set after delay over
bool fadeTransition = true;                   //enable crossfading color transition
bool enableSecTransition = true;              //also enable transition for secondary color
uint16_t transitionDelay = 750;               //default crossfade duration in ms

bool skipFirstLed = false;                    //ignore first LED in strip (useful if you need the LED as signal repeater)
uint8_t disableNLeds = 0;                     //disables N LEDs between active nodes. (Useful for spacing out lights for more traditional christmas light look)
byte briMultiplier =  100;                    //% of brightness to set (to limit power, if you set it to 50 and set bri to 255, actual brightness will be 127)


//User Interface CONFIG
char serverDescription[33] = "WLED";          //Name of module
bool syncToggleReceive = false;               //UIs which only have a single button for sync should toggle send+receive if this is true, only send otherwise


//Sync CONFIG
bool buttonEnabled =  true;
bool irEnabled     = false;                   //Infrared receiver

uint16_t udpPort    = 21324;                  //WLED notifier default port
uint16_t udpRgbPort = 19446;                  //Hyperion port

bool receiveNotificationBrightness = true;    //apply brightness from incoming notifications
bool receiveNotificationColor      = true;    //apply color
bool receiveNotificationEffects    = true;    //apply effects setup
bool notifyDirect = false;                    //send notification if change via UI or HTTP API
bool notifyButton = false;                    //send if updated by button or infrared remote
bool notifyAlexa  = false;                    //send notification if updated via Alexa
bool notifyMacro  = false;                    //send notification for macro
bool notifyHue    =  true;                    //send notification if Hue light changes
bool notifyTwice  = false;                    //notifications use UDP: enable if devices don't sync reliably

bool alexaEnabled = true;                     //enable device discovery by Amazon Echo
char alexaInvocationName[33] = "Light";       //speech control name of device. Choose something voice-to-text can understand

char blynkApiKey[36] = "";                    //Auth token for Blynk server. If empty, no connection will be made

uint16_t realtimeTimeoutMs = 2500;            //ms timeout of realtime mode before returning to normal mode
int  arlsOffset = 0;                          //realtime LED offset
bool receiveDirect    =  true;                //receive UDP realtime
bool arlsDisableGammaCorrection = true;       //activate if gamma correction is handled by the source
bool arlsForceMaxBri = false;                 //enable to force max brightness if source has very dark colors that would be black

uint16_t e131Universe = 1;                    //settings for E1.31 (sACN) protocol
bool e131Multicast = false;

bool mqttEnabled = false;
char mqttDeviceTopic[33] = "";                //main MQTT topic (individual per device, default is wled/mac)
char mqttGroupTopic[33] = "wled/all";         //second MQTT topic (for example to group devices)
char mqttServer[33] = "";                     //both domains and IPs should work (no SSL)
char mqttUser[41] = "";                       //optional: username for MQTT auth
char mqttPass[41] = "";                       //optional: password for MQTT auth
char mqttClientID[41] = "";                   //override the client ID
uint16_t mqttPort = 1883;

bool huePollingEnabled = false;               //poll hue bridge for light state
uint16_t huePollIntervalMs = 2500;            //low values (< 1sec) may cause lag but offer quicker response
char hueApiKey[47] = "api";                   //key token will be obtained from bridge
byte huePollLightId = 1;                      //ID of hue lamp to sync to. Find the ID in the hue app ("about" section)
IPAddress hueIP = (0,0,0,0);                  //IP address of the bridge
bool hueApplyOnOff = true;
bool hueApplyBri   = true;
bool hueApplyColor = true;


//Time CONFIG
bool ntpEnabled = false;                      //get internet time. Only required if you use clock overlays or time-activated macros
bool useAMPM = false;                         //12h/24h clock format
byte currentTimezone = 0;                     //Timezone ID. Refer to timezones array in wled10_ntp.ino
int  utcOffsetSecs   = 0;                     //Seconds to offset from UTC before timzone calculation

byte overlayDefault = 0;                      //0: no overlay 1: analog clock 2: single-digit clocl 3: cronixie
byte overlayMin = 0, overlayMax = ledCount-1; //boundaries of overlay mode

byte analogClock12pixel = 0;                  //The pixel in your strip where "midnight" would be
bool analogClockSecondsTrail = false;         //Display seconds as trail of LEDs instead of a single pixel
bool analogClock5MinuteMarks = false;         //Light pixels at every 5-minute position

char cronixieDisplay[7] = "HHMMSS";           //Cronixie Display mask. See wled13_cronixie.ino
bool cronixieBacklight = true;                //Allow digits to be back-illuminated

bool countdownMode = false;                   //Clock will count down towards date
byte countdownYear = 20, countdownMonth = 1;  //Countdown target date, year is last two digits
byte countdownDay  =  1, countdownHour  = 0;
byte countdownMin  =  0, countdownSec   = 0;


byte macroBoot = 0;                           //macro loaded after startup
byte macroNl = 0;                             //after nightlight delay over
byte macroCountdown = 0;
byte macroAlexaOn = 0, macroAlexaOff = 0;
byte macroButton = 0, macroLongPress = 0, macroDoublePress = 0;


//Security CONFIG
bool otaLock = false;                         //prevents OTA firmware updates without password. ALWAYS enable if system exposed to any public networks
bool wifiLock = false;                        //prevents access to WiFi settings when OTA lock is enabled
bool aOtaEnabled = true;                      //ArduinoOTA allows easy updates directly from the IDE. Careful, it does not auto-disable when OTA lock is on


uint16_t userVar0 = 0, userVar1 = 0;


//internal global variable declarations
//wifi
bool apActive = false;
bool forceReconnect = false;
uint32_t lastReconnectAttempt = 0;
bool interfacesInited = false;
bool wasConnected = false;

//color
byte colOld[]{0, 0, 0, 0};                    //color before transition
byte colT[]{0, 0, 0, 0};                      //current color
byte colIT[]{0, 0, 0, 0};                     //color that was last sent to LEDs
byte colSecT[]{0, 0, 0, 0};
byte colSecOld[]{0, 0, 0, 0};
byte colSecIT[]{0, 0, 0, 0};

byte lastRandomIndex = 0;                     //used to save last random color so the new one is not the same

//transitions
bool transitionActive = false;
uint16_t transitionDelayDefault = transitionDelay;
uint16_t transitionDelayTemp = transitionDelay;
unsigned long transitionStartTime;
float tperLast = 0;                           //crossfade transition progress, 0.0f - 1.0f
bool jsonTransitionOnce = false;

//nightlight
bool nightlightActive = false;
bool nightlightActiveOld = false;
uint32_t nightlightDelayMs = 10;
uint8_t nightlightDelayMinsDefault = nightlightDelayMins;
unsigned long nightlightStartTime;
byte briNlT = 0;                              //current nightlight brightness

//brightness
unsigned long lastOnTime = 0;
bool offMode = !turnOnAtBoot;
byte bri = briS;
byte briOld = 0;
byte briT = 0;
byte briIT = 0;
byte briLast = 128;                           //brightness before turned off. Used for toggle function

//button
bool buttonPressedBefore = false;
bool buttonLongPressed = false;
unsigned long buttonPressedTime = 0;
unsigned long buttonWaitTime = 0;

//notifications
bool notifyDirectDefault = notifyDirect;
bool receiveNotifications = true;
unsigned long notificationSentTime = 0;
byte notificationSentCallMode = 0;
bool notificationTwoRequired = false;

//effects
byte effectCurrent = 0;
byte effectSpeed = 128;
byte effectIntensity = 128;
byte effectPalette = 0;

//network
bool udpConnected = false, udpRgbConnected = false;

//ui style
bool showWelcomePage = false;

//hue
char hueError[25] = "Inactive";
//uint16_t hueFailCount = 0;
float hueXLast=0, hueYLast=0;
uint16_t hueHueLast=0, hueCtLast=0;
byte hueSatLast=0, hueBriLast=0;
unsigned long hueLastRequestSent = 0;
bool hueAuthRequired = false;
bool hueReceived = false;
bool hueStoreAllowed = false, hueNewKey = false;

//overlays
byte overlayCurrent = overlayDefault;
byte overlaySpeed = 200;
unsigned long overlayRefreshMs = 200;
unsigned long overlayRefreshedTime;

//cronixie
byte dP[]{0,0,0,0,0,0};
bool cronixieInit = false;

//countdown
unsigned long countdownTime = 1514764800L;
bool countdownOverTriggered = true;

//timer
byte lastTimerMinute = 0;
byte timerHours[]   = {0,0,0,0,0,0,0,0};
byte timerMinutes[] = {0,0,0,0,0,0,0,0};
byte timerMacro[]   = {0,0,0,0,0,0,0,0};
byte timerWeekday[] = {255,255,255,255,255,255,255,255}; //weekdays to activate on
//bit pattern of arr elem: 0b11111111: sun,sat,fri,thu,wed,tue,mon,validity

//blynk
bool blynkEnabled = false;

//preset cycling
bool presetCyclingEnabled = false;
byte presetCycleMin = 1, presetCycleMax = 5;
uint16_t presetCycleTime = 1250;
unsigned long presetCycledTime = 0; byte presetCycCurr = presetCycleMin;
bool presetApplyBri = false, presetApplyCol = true, presetApplyFx = true;
bool saveCurrPresetCycConf = false;

//realtime
bool realtimeActive = false;
IPAddress realtimeIP = (0,0,0,0);
unsigned long realtimeTimeout = 0;

//mqtt
long lastMqttReconnectAttempt = 0;
long lastInterfaceUpdate = 0;
byte interfaceUpdateCallMode = 0;
char mqttStatusTopic[40] = ""; //this must be global because of async handlers

#if AUXPIN >= 0
//auxiliary debug pin
byte auxTime = 0;
unsigned long auxStartTime = 0;
bool auxActive = false, auxActiveBefore = false;
#endif

//alexa udp
String escapedMac;
#ifndef WLED_DISABLE_ALEXA
Espalexa espalexa;
EspalexaDevice* espalexaDevice;
#endif

//dns server
DNSServer dnsServer;

//network time
bool ntpConnected = false;
time_t local = 0;
unsigned long ntpLastSyncTime = 999000000L;
unsigned long ntpPacketSentTime = 999000000L;
IPAddress ntpServerIP;
uint16_t ntpLocalPort = 2390;
#define NTP_PACKET_SIZE 48

#define MAX_LEDS 1500
#define MAX_LEDS_DMA 500

//string temp buffer (now stored in stack locally)
#define OMAX 2048
char* obuf;
uint16_t olen = 0;

uint16_t savedPresets = 0;
int8_t currentPreset = -1;
bool isPreset = false;

byte errorFlag = 0;

String messageHead, messageSub;
byte optionType;

bool doReboot = false; //flag to initiate reboot from async handlers
bool doPublishMqtt = false;

//server library objects
AsyncWebServer server(80);
AsyncClient* hueClient = NULL;
AsyncMqttClient* mqtt = NULL;

//function prototypes
void colorFromUint32(uint32_t,bool=false);
void serveMessage(AsyncWebServerRequest*,uint16_t,String,String,byte);
void handleE131Packet(e131_packet_t*, IPAddress);

#define E131_MAX_UNIVERSE_COUNT 9

//udp interface objects
WiFiUDP notifierUdp, rgbUdp;
WiFiUDP ntpUdp;
ESPAsyncE131 e131(handleE131Packet);
bool e131NewData = false;

//led fx library object
WS2812FX strip = WS2812FX();

#define WLED_CONNECTED (WiFi.status() == WL_CONNECTED)
#define WLED_WIFI_CONFIGURED (strlen(clientSSID) >= 1 && strcmp(clientSSID,"Your_Network") != 0)

//debug macros
#ifdef WLED_DEBUG
 #define DEBUG_PRINT(x)  Serial.print (x)
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

//filesystem
#ifndef WLED_DISABLE_FILESYSTEM
 #include <FS.h>
 #ifdef ARDUINO_ARCH_ESP32
  #include "SPIFFS.h"
 #endif
 #include "SPIFFSEditor.h"
#endif

//turns all LEDs off and restarts ESP
void reset()
{
  briT = 0;
  long dly = millis();
  while(millis() - dly < 250)
  {
    yield(); //enough time to send response to client
  }
  setAllLeds();
  DEBUG_PRINTLN("MODULE RESET");
  ESP.restart();
}


//append new c string to temp buffer efficiently
bool oappend(const char* txt)
{
  uint16_t len = strlen(txt);
  if (olen + len >= OMAX) return false; //buffer full
  strcpy(obuf + olen, txt);
  olen += len;
  return true;
}


//append new number to temp buffer efficiently
bool oappendi(int i)
{
  char s[11];
  sprintf(s,"%ld", i);
  return oappend(s);
}


//boot starts here
void setup() {
  wledInit();
}


//main program loop
void loop() {
  handleConnection();
  handleSerial();
  handleNotifications();
  handleTransitions();
  userLoop();

  yield();
  handleIO();
  handleIR();
  handleNetworkTime();
  handleAlexa();

  handleOverlays();
  yield();
  if (doReboot) reset();

  if (!realtimeActive) //block stuff if WARLS/Adalight is enabled
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
  if (millis() - lastMqttReconnectAttempt > 30000) initMqtt();

  //DEBUG serial logging
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
     DEBUG_PRINT("Loops/sec: "); DEBUG_PRINTLN(loops/10);
     loops = 0;
     debugTime = millis();
   }
   loops++;
  #endif
}
