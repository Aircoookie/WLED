/*
 * Main sketch
 */
/*
 * @title WLED project sketch
 * @version 0.6.5
 * @author Christian Schwinne
 */

#include <Arduino.h>
#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#include <ESPmDNS.h>
#include "src/dependencies/webserver/WebServer.h"
#include <HTTPClient.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#endif
#include <EEPROM.h>
#include <ArduinoOTA.h>
#include <WiFiUDP.h>
#include <DNSServer.h>
#include "src/dependencies/webserver/ESP8266HTTPUpdateServer.h"
#include "src/dependencies/time/Time.h"
#include "src/dependencies/time/TimeLib.h"
#include "src/dependencies/timezone/Timezone.h"
#include "htmls00.h"
#include "htmls01.h"
#include "htmls02.h"
#include "WS2812FX.h"

//version in format yymmddb (b = daily build)
#define VERSION 1805222
const String versionString = "0.6.5";

//AP and OTA default passwords (change them!)
String apPass = "wled1234";
String otaPass = "wledota";

//spiffs FS only useful for debug (only ESP8266)
//#define USEFS

//to toggle usb serial debug (un)comment following line(s)
//#define DEBUG

//Hardware-settings (only changeble via code)
#define PIN 2 //strip pin. Only change for ESP32
byte buttonPin = 0; //needs pull-up
byte auxPin = 15; //use e.g. for external relay
byte auxDefaultState = 0; //0: input 1: high 2: low
byte auxTriggeredState = 0; //0: input 1: high 2: low

//Default CONFIG
String serverDescription = "WLED Light";
byte currentTheme = 0;
byte uiConfiguration = 0; //0: auto 1: classic 2: mobile
String clientSSID = "Your_Network";
String clientPass = "";
String cmDNS = "led";
uint16_t ledCount = 10; //lowered to prevent accidental overcurrent
String apSSID = ""; //AP off by default (unless setup)
byte apChannel = 1;
byte apHide = 0;
byte apWaitTimeSecs = 32;
bool recoveryAPDisabled = false;
IPAddress staticIP(0, 0, 0, 0);
IPAddress staticGateway(0, 0, 0, 0);
IPAddress staticSubnet(255, 255, 255, 0);
IPAddress staticDNS(8, 8, 8, 8); //only for NTP
bool useHSB = true, useHSBDefault = true, useRGBW = false;
bool turnOnAtBoot = true;
bool initLedsLast = false;
byte bootPreset = 0;
byte colS[]{255, 159, 0};
byte colSecS[]{0, 0, 0};
byte whiteS = 0;
byte whiteSecS = 0;
byte briS = 127;
byte nightlightTargetBri = 0;
bool fadeTransition = true;
bool sweepTransition = false, sweepDirection = true;
bool disableSecTransition = true;
uint16_t transitionDelay = 1200, transitionDelayDefault = transitionDelay;
bool reverseMode = false;
bool otaLock = false, wifiLock = false;
bool aOtaEnabled = true;
bool buttonEnabled = true;
bool notifyDirect = true, notifyButton = true, notifyDirectDefault = true, alexaNotify = false, macroNotify = false, notifyTwice = false;
bool receiveNotifications = true, receiveNotificationBrightness = true, receiveNotificationColor = true, receiveNotificationEffects = true;
byte briMultiplier = 100;
byte nightlightDelayMins = 60;
bool nightlightFade = true;
uint16_t udpPort = 21324, udpRgbPort = 19446;
byte effectDefault = 0;
byte effectSpeedDefault = 75;
byte effectIntensityDefault = 128;
//NTP stuff
bool ntpEnabled = false;
String ntpServerName = "0.wled.pool.ntp.org";
//custom chase
byte ccNumPrimary = 2;
byte ccNumSecondary = 4;
byte ccIndex1 = 0;
uint16_t ccIndex2 = ledCount -1;
bool ccFromStart = true, ccFromEnd = false;
byte ccStep = 1;
byte ccStart = 0;

//alexa
bool alexaEnabled = true;
String alexaInvocationName = "Light";

byte macroBoot = 0, macroNl = 0;
byte macroAlexaOn = 0, macroAlexaOff = 0;
byte macroButton = 0, macroCountdown = 0, macroLongPress = 0;

unsigned long countdownTime = 1514764800L;

//hue
bool huePollingEnabled = false, hueAttempt = false;
uint16_t huePollIntervalMs = 2500;
String hueApiKey = "api";
byte huePollLightId = 1;
IPAddress hueIP = (0,0,0,0);
bool notifyHue = true;
bool hueApplyOnOff = true, hueApplyBri = true, hueApplyColor = true;

uint16_t userVar0 = 0, userVar1 = 0;

//Internal vars
byte col[]{0, 0, 0};
byte colOld[]{0, 0, 0};
byte colT[]{0, 0, 0};
byte colIT[]{0, 0, 0};
byte colSec[]{0, 0, 0};
byte colSecT[]{0, 0, 0};
byte colSecOld[]{0, 0, 0};
byte colSecIT[]{0, 0, 0};
byte white, whiteOld, whiteT, whiteIT;
byte whiteSec, whiteSecOld, whiteSecT, whiteSecIT;
byte lastRandomIndex = 0;
uint16_t transitionDelayTemp = transitionDelay;
unsigned long transitionStartTime;
unsigned long nightlightStartTime;
float tperLast = 0;
byte bri = 0;
byte briOld = 0;
byte briT = 0;
byte briIT = 0;
byte briLast = 127;
bool transitionActive = false;
bool buttonPressedBefore = false;
unsigned long buttonPressedTime = 0;
unsigned long notificationSentTime = 0;
byte notificationSentCallMode = 0;
bool notificationTwoRequired = false;
bool nightlightActive = false;
bool nightlightActiveOld = false;
uint32_t nightlightDelayMs = 10;
byte briNlT = 0;
byte effectCurrent = 0;
byte effectSpeed = 75;
byte effectIntensity = 128;
bool onlyAP = false;
bool udpConnected = false, udpRgbConnected = false;
String cssCol[]={"","","","","",""};
String cssFont="Verdana";
String cssColorString="";
//NTP stuff
bool ntpConnected = false;
byte currentTimezone = 0;
time_t local = 0;
int utcOffsetSecs = 0;

//hue
String hueError = "Inactive";
uint16_t hueFailCount = 0;
float hueXLast=0, hueYLast=0;
uint16_t hueHueLast=0, hueCtLast=0;
byte hueSatLast=0, hueBriLast=0;
long hueLastRequestSent = 0;
uint32_t huePollIntervalMsTemp = huePollIntervalMs;

//overlay stuff
byte overlayDefault = 0;
byte overlayCurrent = 0;
byte overlayMin = 0, overlayMax = ledCount-1;
byte analogClock12pixel = 0;
bool analogClockSecondsTrail = false;
bool analogClock5MinuteMarks = false;
byte overlaySpeed = 200;
unsigned long overlayRefreshMs = 200;
unsigned long overlayRefreshedTime;
int overlayArr[6];
uint16_t overlayDur[6];
uint16_t overlayPauseDur[6];
int nixieClockI = -1;
bool nixiePause;
byte countdownYear=19, countdownMonth=1, countdownDay=1, countdownHour=0, countdownMin=0, countdownSec=0; //year is actual year -2000
bool countdownOverTriggered = true;
//cronixie
String cronixieDisplay = "HHMMSS";
byte dP[]{0,0,0,0,0,0};
bool useAMPM = false;
bool cronixieBacklight = true;
bool countdownMode = false;
bool cronixieInit = false;

bool presetCyclingEnabled = false;
byte presetCycleMin = 1, presetCycleMax = 5;
uint16_t presetCycleTime = 1250;
unsigned long presetCycledTime = 0; byte presetCycCurr = presetCycleMin;
bool presetApplyBri = true, presetApplyCol = true, presetApplyFx = true;

uint32_t arlsTimeoutMillis = 2500;
bool arlsTimeout = false;
bool receiveDirect = true, enableRealtimeUI = false;
unsigned long arlsTimeoutTime = 0;
byte auxTime = 0;
unsigned long auxStartTime = 0;
bool auxActive = false, auxActiveBefore = false;
bool showWelcomePage = false;

bool useGammaCorrectionBri = false;
bool useGammaCorrectionRGB = true;
int arlsOffset = -22; //10: -22 assuming arls52

//alexa udp
WiFiUDP UDP;
IPAddress ipMulti(239, 255, 255, 250);
unsigned int portMulti = 1900;
char packetBuffer[255];
String escapedMac;

//dns server
DNSServer dnsServer;
bool dnsActive = false;

#ifdef ARDUINO_ARCH_ESP32
WebServer server(80);
#else
ESP8266WebServer server(80);
#endif
HTTPClient hueClient;
ESP8266HTTPUpdateServer httpUpdater;
WiFiUDP notifierUdp, rgbUdp;
WiFiUDP ntpUdp;
IPAddress ntpServerIP;
unsigned int ntpLocalPort = 2390;
const uint16_t NTP_PACKET_SIZE = 48; 
byte ntpPacketBuffer[NTP_PACKET_SIZE];
unsigned long ntpLastSyncTime = 999000000L;
unsigned long ntpPacketSentTime = 999000000L;

WS2812FX strip = WS2812FX();

#ifdef DEBUG
 #define DEBUG_PRINT(x)  Serial.print (x)
 #define DEBUG_PRINTLN(x) Serial.println (x)
 #define DEBUG_PRINTF(x) Serial.printf (x)
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTLN(x)
 #define DEBUG_PRINTF(x)
#endif

#ifdef USEFS
#include <FS.h>;
File fsUploadFile;
#endif

#ifdef DEBUG
long debugTime = 0;
int lastWifiState = 3;
long wifiStateChangedTime = 0;
#endif

const byte gamma8[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

String txd = "Please disable OTA Lock in security settings!";

void serveMessage(int,String,String,int=255);

void down()
{
  briT = 0;
  setAllLeds();
  DEBUG_PRINTLN("MODULE TERMINATED");
  while (1) {delay(1000);}
}

void reset()
{
  briT = 0;
  setAllLeds();
  DEBUG_PRINTLN("MODULE RESET");
  ESP.restart();
}

void setup() {
    wledInit();
}

void loop() {
    server.handleClient();
    handleSerial();
    handleNotifications();
    handleTransitions();
    userLoop();
    yield();
    handleButton();
    handleNetworkTime();
    if (aOtaEnabled) ArduinoOTA.handle();
    handleAlexa();
    handleOverlays();
    if (!arlsTimeout) //block stuff if WARLS/Adalight is enabled
    {
      if (dnsActive) dnsServer.processNextRequest();
      handleHue();
      handleNightlight();
      if (briT) strip.service(); //do not update strip if off, prevents flicker on ESP32
    }
    
    //DEBUG
    #ifdef DEBUG
    if (millis() - debugTime > 5000)
    {
      DEBUG_PRINTLN("---MODULE DEBUG INFO---");
      DEBUG_PRINT("Runtime: "); DEBUG_PRINTLN(millis());
      DEBUG_PRINT("Unix time: "); DEBUG_PRINTLN(now());
      DEBUG_PRINT("Free heap: "); DEBUG_PRINTLN(ESP.getFreeHeap());
      DEBUG_PRINT("Wifi state: "); DEBUG_PRINTLN(WiFi.status());
      if (WiFi.status() != lastWifiState)
      {
        wifiStateChangedTime = millis();
      }
      lastWifiState = WiFi.status();
      DEBUG_PRINT("Wifi state: "); DEBUG_PRINTLN(wifiStateChangedTime);
      DEBUG_PRINT("NTP last sync: "); DEBUG_PRINTLN(ntpLastSyncTime);
      DEBUG_PRINT("Client IP: "); DEBUG_PRINTLN(WiFi.localIP());
      debugTime = millis(); 
    }
    #endif
}


