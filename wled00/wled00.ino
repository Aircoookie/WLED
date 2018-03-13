/*
 * Main sketch
 */
/*
 * @title WLED project sketch
 * @version 0.6.0_dev
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
#include "src/dependencies/webserver/ESP8266HTTPUpdateServer.h"
#include "src/dependencies/time/Time.h"
#include "src/dependencies/time/TimeLib.h"
#include "src/dependencies/timezone/Timezone.h"
#include "htmls00.h"
#include "htmls01.h"
#include "htmls02.h"
#include "WS2812FX.h"

//version in format yymmddb (b = daily build)
#define VERSION 1803141
const String versionString = "0.6.0_dev";

//AP and OTA default passwords (change them!)
String appass = "wled1234";
String otapass = "wledota";

//If you have an RGBW strip, also uncomment first line in WS2812FX.h!
bool useRGBW = false;

//spiffs FS only useful for debug (only ESP8266)
//#define USEFS

//to toggle usb serial debug (un)comment following line(s)
//#define DEBUG

//Hardware-settings (only changeble via code)
#define LEDCOUNT 255 //maximum, exact count set-able via settings
uint8_t buttonPin = 0; //needs pull-up
uint8_t auxPin = 15; //use e.g. for external relay
uint8_t auxDefaultState = 0; //0: input 1: high 2: low
uint8_t auxTriggeredState = 0; //0: input 1: high 2: low

//Default CONFIG
String serverDescription = versionString;
uint8_t currentTheme = 0;
String clientssid = "Your_Network_Here";
String clientpass = "Dummy_Pass";
String cmdns = "led";
uint8_t ledcount = 10; //lowered to prevent accidental overcurrent
String apssid = ""; //AP off by default (unless setup)
uint8_t apchannel = 1;
uint8_t aphide = 0;
uint8_t apWaitTimeSecs = 32;
boolean recoveryAPDisabled = false;
IPAddress staticip(0, 0, 0, 0);
IPAddress staticgateway(0, 0, 0, 0);
IPAddress staticsubnet(255, 255, 255, 0);
IPAddress staticdns(8, 8, 8, 8); //only for NTP
bool useHSB = false, useHSBDefault = false;
bool turnOnAtBoot = true;
uint8_t bootPreset = 0;
byte col_s[]{255, 159, 0};
byte col_sec_s[]{0, 0, 0};
byte white_s = 0;
byte white_sec_s = 0;
byte bri_s = 127;
uint8_t nightlightTargetBri = 0, bri_nl_t;
bool fadeTransition = true;
bool sweepTransition = false, sweepDirection = true;
uint16_t transitionDelay = 1200;
bool reverseMode = false;
bool otaLock = false, wifiLock = false;
bool aOtaEnabled = true;
bool onlyAP = false;
bool buttonEnabled = true;
bool notifyDirect = true, notifyButton = true, notifyDirectDefault = true, alexaNotify = false, macroNotify = false, notifyTwice = false;
bool receiveNotifications = true, receiveNotificationBrightness = true, receiveNotificationColor = true, receiveNotificationEffects = true;
uint8_t briMultiplier = 100;
uint8_t nightlightDelayMins = 60;
bool nightlightFade = true;
uint16_t udpPort = 21324;
uint8_t effectDefault = 0;
uint8_t effectSpeedDefault = 75;
uint8_t effectIntensityDefault = 128;
//NTP stuff
boolean ntpEnabled = false;
IPAddress ntpServerIP;
const char* ntpServerName = "0.wled.pool.ntp.org";
//custom chase
uint8_t cc_numPrimary = 2;
uint8_t cc_numSecondary = 4;
uint8_t cc_index1 = 0;
uint8_t cc_index2 = ledcount -1;
bool cc_fromStart = true, cc_fromEnd = false;
uint8_t cc_step = 1;
uint8_t cc_start = 0;

//alexa
boolean alexaEnabled = true;
String alexaInvocationName = "Light";

uint8_t macroBoot = 0, macroNl = 0;
uint8_t macroAlexaOn = 0, macroAlexaOff = 0;
uint8_t macroButton = 0, macroCountdown = 0, macroLongPress = 0;

unsigned long countdownTime = 1514764800L;

double transitionResolution = 0.011;

//hue
long hueLastRequestSent = 0;
bool huePollingEnabled = false, hueAttempt = false;
uint16_t huePollIntervalMs = 2500;
uint32_t huePollIntervalMsTemp = 2500;
String hueApiKey = "api";
uint8_t huePollLightId = 1;
IPAddress hueIP = (0,0,0,0);
bool notifyHue = true;
bool hueApplyOnOff = true, hueApplyBri = true, hueApplyColor = true;
String hueError = "Inactive";
uint16_t hueFailCount = 0;
float hueXLast=0, hueYLast=0;
uint16_t hueHueLast=0, hueCtLast=0;
uint8_t hueSatLast=0, hueBriLast=0;

//Internal vars
byte col[]{0, 0, 0};
byte col_old[]{0, 0, 0};
byte col_t[]{0, 0, 0};
byte col_it[]{0, 0, 0};
byte col_sec[]{0, 0, 0};
byte col_sec_it[]{0, 0, 0};
byte white, white_old, white_t, white_it;
byte white_sec, white_sec_it;
uint8_t lastRandomIndex = 0;
unsigned long transitionStartTime;
unsigned long nightlightStartTime;
float tper_last = 0;
byte bri = 0;
byte bri_old = 0;
byte bri_t = 0;
byte bri_it = 0;
byte bri_last = 127;
boolean transitionActive = false;
boolean buttonPressedBefore = false;
long buttonPressedTime = 0;
long notificationSentTime = 0;
uint8_t notificationSentCallMode = 0;
bool notificationTwoRequired = false;
boolean nightlightActive = false;
boolean nightlightActive_old = false;
int nightlightDelayMs;
uint8_t effectCurrent = 0;
uint8_t effectSpeed = 75;
uint8_t effectIntensity = 128;
boolean udpConnected = false;
byte udpIn[1026];
String cssCol[]={"","","","","",""};
String cssFont="Verdana";
String cssColorString="";
//NTP stuff
boolean ntpConnected = false;
unsigned int ntpLocalPort = 2390;
const int NTP_PACKET_SIZE = 48; 
byte ntpPacketBuffer[NTP_PACKET_SIZE];
unsigned long ntpLastSyncTime = 999000000L;
unsigned long ntpPacketSentTime = 999000000L;
uint8_t currentTimezone = 0;
time_t local;
int utcOffsetSecs = 0;

//overlay stuff
uint8_t overlayDefault = 0;
uint8_t overlayCurrent = 0;
uint8_t overlayMin = 0, overlayMax = ledcount-1;
uint8_t analogClock12pixel = 0;
boolean analogClockSecondsTrail = false;
boolean analogClock5MinuteMarks = false;
uint8_t overlaySpeed = 200;
long overlayRefreshMs = 200;
unsigned long overlayRefreshedTime;
int overlayArr[6];
uint16_t overlayDur[6];
uint16_t overlayPauseDur[6];
int nixieClockI = -1;
boolean nixiePause;
uint8_t countdownYear=19, countdownMonth=1, countdownDay=1, countdownHour=0, countdownMin=0, countdownSec=0; //year is actual year -2000
bool countdownOverTriggered = true;
//cronixie
String cronixieDisplay = "HHMMSS";
byte dP[]{0,0,0,0,0,0};
bool useAMPM = false;
bool cronixieBacklight = true;
bool countdownMode = false;
bool cronixieInit = false;

int arlsTimeoutMillis = 2500;
boolean arlsTimeout = false;
long arlsTimeoutTime;
boolean arlsSign = true;
uint8_t auxTime = 0;
unsigned long auxStartTime;
boolean auxActive, auxActiveBefore;
boolean showWelcomePage = false;

boolean useGammaCorrectionBri = false;
boolean useGammaCorrectionRGB = true;
int arlsOffset = -22; //10: -22 assuming arls52

//alexa udp
WiFiUDP UDP;
IPAddress ipMulti(239, 255, 255, 250);
unsigned int portMulti = 1900;
char packetBuffer[255];
String escapedMac;

#ifdef ARDUINO_ARCH_ESP32
WebServer server(80);
#else
ESP8266WebServer server(80);
#endif
HTTPClient hueClient;
ESP8266HTTPUpdateServer httpUpdater;
WiFiUDP notifierUdp;
WiFiUDP ntpUdp;

WS2812FX strip = WS2812FX(LEDCOUNT);

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

const uint8_t gamma8[] = {
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
  bri_t = 0;
  setAllLeds();
  DEBUG_PRINTLN("MODULE TERMINATED");
  while (1) {delay(1000);}
}

void reset()
{
  bri_t = 0;
  setAllLeds();
  DEBUG_PRINTLN("MODULE RESET");
  ESP.restart();
}

void setup() {
    wledInit();
}

void loop() {
    server.handleClient();
    handleNotifications();
    handleTransitions();
    userLoop();
    yield();
    handleButton();
    handleNetworkTime();
    if (!otaLock && aOtaEnabled) ArduinoOTA.handle();
    handleAlexa();
    handleOverlays();
    if (!arlsTimeout) //block stuff if WARLS is enabled
    {
      handleHue();
      handleNightlight();
      if (bri_t) strip.service(); //do not update strip if off, prevents flicker on ESP32
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


