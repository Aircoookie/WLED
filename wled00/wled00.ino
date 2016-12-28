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

/*
 * @title WLED project sketch
 * @version 0.3pd
 * @author Christian Schwinne
 */
//Hardware-settings (only changeble via code)
uint8_t led_amount = 9;
uint8_t buttonPin = 0; //needs pull-up

TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       //Central European Standard Time
Timezone CE(CEST, CET);
TimeChangeRule *tcr;        //pointer to the time change rule, use to get the TZ abbrev
time_t local;

//Default CONFIG
String serverDescription = "WLED 0.3pd";
String clientssid = "Your_Network_Here";
String clientpass = "Dummy_Pass";
String cmdns = "led";
String apssid = "WLED-AP";
String appass = "wled1234";
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
String otapass = "wledota";
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
boolean ntpEnabled = true;
const char* ntpServerName = "time.nist.gov";
long ntpRetryMs = 20000;
long ntpResyncMs = 72000000;

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
byte udpIn[16];
IPAddress ntpIp;
byte ntpBuffer[48];
boolean ntpConnected = false;
boolean ntpSyncNeeded = true;
boolean ntpPacketSent = false;
long ntpPacketSentTime, ntpSyncTime;

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
WiFiUDP notifierUdp;
WiFiUDP ntpUdp;

WS2812FX strip = WS2812FX(led_amount, 2, NEO_GRB + NEO_KHZ800);

File fsUploadFile;

void down()
{
  bri_t = 0;
  setAllLeds();
  Serial.println("MODULE TERMINATED");
  while (1) {delay(1000);}
}

void reset()
{
  bri_t = 0;
  setAllLeds();
  Serial.println("MODULE RESET");
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
    strip.service();
}


