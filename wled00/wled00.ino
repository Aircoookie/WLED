/*
 * Main sketch
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <Hash.h>
#include "WS2812FX.h"
#include <FS.h>
#include <WiFiUDP.h>
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>

//to toggle usb serial debug (un)comment following line
//#define DEBUG

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
 * @version 0.3pd
 * @author Christian Schwinne
 */
//Hardware-settings (only changeble via code)
#define LEDCOUNT 84
uint8_t buttonPin = 0; //needs pull-up

//AP and OTA default passwords (change them!)
String appass = "wled1234";
String otapass = "wledota";

TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       //Central European Standard Time
Timezone TZ(CEST, CET);
TimeChangeRule *tcr;        //pointer to the time change rule, use to get the TZ abbrev
time_t local;

//Default CONFIG
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
byte col_s[]{255, 127, 0};
byte bri_s = 127;
uint8_t bri_nl = 0;
boolean fadeTransition = true;
boolean seqTransition = false;
uint16_t transitionDelay = 1500;
boolean ota_lock = true;
boolean only_ap = false;
boolean buttonEnabled = true;
boolean notifyDirect = true, notifyButton = true, notifyNightlight = false, notifyMaster = true;
boolean receiveNotifications = true, receiveNotificationsDefault = true;
uint8_t bri_n = 100;
uint8_t nightlightDelayMins = 60;
boolean nightlightFade = true;
uint16_t udpPort = 21324;
uint8_t effectDefault = 0;
uint8_t effectSpeedDefault = 75;
boolean ntpEnabled = false;
const char* ntpServerName = "time.nist.gov";
long ntpRetryMs = 9600;
long ntpResyncMs = 72000000L;
int overlayMin = 0, overlayMax = 9;
int analogClock12pixel = 25;
boolean analogClockSecondsTrail = false;
boolean analogClock5MinuteMarks = true;
boolean nixieClockDisplaySeconds = false;
boolean nixieClock12HourFormat = false;
boolean overlayReverse = true;
uint8_t overlaySpeed = 200;

double transitionResolution = 0.011;

//Internal vars
byte col[]{0, 0, 0};
byte col_old[]{0, 0, 0};
byte col_t[]{0, 0, 0};
byte col_it[]{0, 0, 0};
long transitionStartTime;
long nightlightStartTime;
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
int transitionDelay_old;
int nightlightDelayMs;
uint8_t effectCurrent = 0;
uint8_t effectSpeed = 75;
boolean udpConnected = false;
byte udpIn[LEDCOUNT*4+2];
IPAddress ntpIp;
IPAddress ntpBackupIp(134,130,5,17);
byte ntpBuffer[48];
boolean ntpConnected = false;
boolean ntpSyncNeeded = true;
boolean ntpPacketSent = false;
long ntpPacketSentTime, ntpSyncTime;
uint8_t overlayCurrent = 0;
long overlayRefreshMs = 200;
long overlayRefreshedTime;
int overlayArr[6];
int overlayDur[6];
int overlayPauseDur[6];
int nixieClockI = -1;
boolean nixiePause;
long countdownTime = 1483225200L;
boolean arlsTimeout = false;
long arlsTimeoutTime;

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
WiFiUDP notifierUdp;
WiFiUDP ntpUdp;

WS2812FX strip = WS2812FX(LEDCOUNT, 2, NEO_GRB + NEO_KHZ800);

File fsUploadFile;

#ifdef DEBUG
int debugIndex = 0;
int lastWifiState = 3;
long wifiStateChangedTime = 0;
#endif

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

uint8_t bool2int(boolean value)
{
  if (value) return 1;
  return 0;
}

void setup() {
    wledInit();
}

void loop() {
    server.handleClient();
    handleNotifications();
    handleTransitions();
    handleNightlight();
    handleButton();
    handleNetworkTime();
    handleOverlays();
    strip.service();

    //DEBUG
    #ifdef DEBUG
    debugIndex ++;
    if (debugIndex > 99999)
    {
      debugIndex = 0;
      DEBUG_PRINTLN("---MODULE DEBUG INFO---");
      DEBUG_PRINT("Runtime: "); DEBUG_PRINTLN(millis());
      DEBUG_PRINT("Unix time: "); DEBUG_PRINTLN(now());
      DEBUG_PRINT("Wifi state: "); DEBUG_PRINTLN(WiFi.status());
      if (WiFi.status() != lastWifiState)
      {
        wifiStateChangedTime = millis();
      }
      lastWifiState = WiFi.status();
      DEBUG_PRINT("Wifi state: "); DEBUG_PRINTLN(wifiStateChangedTime);
      DEBUG_PRINT("NTP sync needed: "); DEBUG_PRINTLN(ntpSyncNeeded);
      DEBUG_PRINT("Client IP: "); DEBUG_PRINTLN(WiFi.localIP()); 
    }
    #endif
}


