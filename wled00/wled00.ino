/*
 * Main sketch
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include "WS2812FX.h"
#include <WiFiUDP.h>
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>
#include "htmls00.h"
#include "htmls01.h"
#include "switch.h"
#include "UpnpBroadcastResponder.h"
#include "CallbackFunction.h"

//version in format yymmddb (b = daily build)
#define VERSION 1710120

//uncomment if you have an RGBW strip
#define RGBW

//to toggle usb serial debug (un)comment following line
//#define DEBUG

//spiffs FS only useful for debug
//#define USEFS

//overlays, needed for clocks etc.
//#define USEOVERLAYS

#ifdef USEFS
#include <FS.h>
#endif

#ifdef DEBUG
 #define DEBUG_PRINT(x)  Serial.print (x)
 #define DEBUG_PRINTLN(x) Serial.println (x)
 #define DEBUG_PRINTF(x) Serial.printf (x)
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTLN(x)
 #define DEBUG_PRINTF(x)
#endif

/*
 * @title WLED project sketch
 * @version 0.3
 * @author Christian Schwinne
 */
//Hardware-settings (only changeble via code)
#define LEDCOUNT 93 //maximum, exact count set-able via settings
#define MAXDIRECT 93 //for direct access like arls, should be >= LEDCOUNT
uint8_t buttonPin = 0; //needs pull-up
uint8_t auxPin = 15; //use e.g. for external relay
uint8_t auxDefaultState = 0; //0: input 1: high 2: low
uint8_t auxTriggeredState = 0; //0: input 1: high 2: low

//AP and OTA default passwords (change them!)
String appass = "wled1234";
String otapass = "wledota";

TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       //Central European Standard Time
Timezone TZ(CEST, CET);
TimeChangeRule *tcr;        //pointer to the time change rule, use to get the TZ abbrev
time_t local;

//Default CONFIG
uint8_t ledcount = 255;
String serverDescription = "WLED 0.3pd";
String clientssid = "Your_Network_Here";
String clientpass = "Dummy_Pass";
String cmdns = "led";
String apssid = "WLED-AP";
uint8_t apchannel = 1;
uint8_t aphide = 0;
boolean useap = true;
IPAddress staticip(0, 0, 0, 0);
IPAddress staticgateway(0, 0, 0, 0);
IPAddress staticsubnet(255, 255, 255, 0);
boolean useHSB = false, useHSBDefault = false;
boolean turnOnAtBoot = true;
byte col_s[]{255, 159, 0};
byte white_s = 0;
byte bri_s = 127;
uint8_t bri_nl = 0, bri_nls;
boolean fadeTransition = true;
uint16_t transitionDelay = 1200;
boolean ota_lock = true;
boolean only_ap = false;
boolean buttonEnabled = true;
boolean notifyDirect = true, notifyButton = true, notifyDirectDefault = true;
boolean receiveNotifications = true, receiveNotificationsDefault = true;
uint8_t bri_n = 100;
uint8_t nightlightDelayMins = 60;
boolean nightlightFade = true;
uint16_t udpPort = 21324;
uint8_t effectDefault = 0;
uint8_t effectSpeedDefault = 75;
//NTP stuff
boolean ntpEnabled = false;
IPAddress ntpServerIP;
const char* ntpServerName = "time.nist.gov";

//alexa
boolean alexaEnabled = true;
String alexaInvocationName = "Light";
boolean alexaNotify = false;

double transitionResolution = 0.011;

//Internal vars
byte col[]{0, 0, 0};
byte col_old[]{0, 0, 0};
byte col_t[]{0, 0, 0};
byte col_it[]{0, 0, 0};
byte white, white_old, white_t, white_it;
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
boolean nightlightActive = false;
boolean nightlightActive_old = false;
int nightlightDelayMs;
uint8_t effectCurrent = 0;
uint8_t effectSpeed = 75;
boolean udpConnected = false;
byte udpIn[MAXDIRECT*4+2];
//NTP stuff
boolean ntpConnected = false;
unsigned int ntpLocalPort = 2390;
const int NTP_PACKET_SIZE = 48; 
byte ntpPacketBuffer[NTP_PACKET_SIZE];
unsigned long ntpLastSyncTime = 999000000L;
unsigned long ntpPacketSentTime = 999000000L;
const unsigned long seventyYears = 2208988800UL;

//overlay stuff
uint8_t overlayDefault = 0;
uint8_t overlayCurrent = 0;
#ifdef USEOVERLAYS
int overlayMin = 0, overlayMax = 9;
int analogClock12pixel = 25;
boolean analogClockSecondsTrail = false;
boolean analogClock5MinuteMarks = true;
boolean nixieClockDisplaySeconds = false;
boolean nixieClock12HourFormat = false;
boolean overlayReverse = true;
uint8_t overlaySpeed = 200;
long overlayRefreshMs = 200;
unsigned long overlayRefreshedTime;
int overlayArr[6];
int overlayDur[6];
int overlayPauseDur[6];
int nixieClockI = -1;
boolean nixiePause;
unsigned long countdownTime = 1483225200L;
#endif

int arlsTimeoutMillis = 2500;
boolean arlsTimeout = false;
long arlsTimeoutTime;
boolean arlsSign = true;
uint8_t auxTime = 0;
unsigned long auxStartTime;
boolean auxActive, auxActiveBefore;

boolean useGammaCorrectionBri = true;
boolean useGammaCorrectionRGB = true;
int arlsOffset = -22; //10: -22 assuming arls52
boolean realtimeEnabled = true;

//alexa
Switch *alexa = NULL;
UpnpBroadcastResponder upnpBroadcastResponder;

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
WiFiUDP notifierUdp;
WiFiUDP ntpUdp;

WS2812FX strip = WS2812FX(LEDCOUNT);

#ifdef USEFS
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
  ESP.reset();
}

void setup() {
    wledInit();
}

void loop() {
    server.handleClient();
    handleNotifications();
    handleTransitions();
    handleNightlight();
    yield();
    handleButton();
    handleNetworkTime();
    #ifdef USEOVERLAYS
    handleOverlays();
    #endif
    handleAlexa();
    strip.service();

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


