#ifndef WLED_H
#define WLED_H
/*
   Main sketch, global variable declarations
   @title WLED project sketch
   @version 0.9.1
   @author Christian Schwinne
 */

// ESP8266-01 (blue) got too little storage space to work with all features of WLED. To use it, you must use ESP8266 Arduino Core v2.4.2 and the setting 512K(No SPIFFS).

// ESP8266-01 (black) has 1MB flash and can thus fit the whole program. Use 1M(64K SPIFFS).
// Uncomment some of the following lines to disable features to compile for ESP8266-01 (max flash size 434kB):
// Alternatively, with platformio pass your chosen flags to your custom build target in platformio.ini.override

// You are required to disable over-the-air updates:
//#define WLED_DISABLE_OTA         //saves 14kb

// You need to choose some of these features to disable:
//#define WLED_DISABLE_ALEXA       //saves 11kb
//#define WLED_DISABLE_BLYNK       //saves 6kb
//#define WLED_DISABLE_CRONIXIE    //saves 3kb
//#define WLED_DISABLE_HUESYNC     //saves 4kb
//#define WLED_DISABLE_INFRARED    //there is no pin left for this on ESP8266-01, saves 12kb
#define WLED_ENABLE_MQTT         //saves 12kb
#define WLED_ENABLE_ADALIGHT     //saves 500b only
//#define WLED_ENABLE_DMX          //uses 3.5kb

#define WLED_DISABLE_FILESYSTEM //SPIFFS is not used by any WLED feature yet
//#define WLED_ENABLE_FS_SERVING   //Enable sending html file from SPIFFS before serving progmem version
//#define WLED_ENABLE_FS_EDITOR    //enable /edit page for editing SPIFFS content. Will also be disabled with OTA lock

// to toggle usb serial debug (un)comment the following line
//#define WLED_DEBUG

// Library inclusions. 
#include <Arduino.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>
extern "C"
{
#include <user_interface.h>
}
#else //ESP32
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
// #define ESPALEXA_DEBUG
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

// remove flicker because PWM signal of RGB channels can become out of phase
#if defined(WLED_USE_ANALOG_LEDS) && defined(ESP8266)
#include "src/dependencies/arduino/core_esp8266_waveform.h"
#endif

// enable additional debug output
#ifdef WLED_DEBUG
#ifndef ESP8266
#include <rom/rtc.h>
#endif
#endif

// version code in format yymmddb (b = daily build)
#define VERSION 2003301

// Global external variable declaration. See wled.cpp for definitions and comments.
extern char versionString[];
extern char apPass[65];
extern char otaPass[33];
extern byte auxDefaultState;                       
extern byte auxTriggeredState;                     
extern char ntpServerName[33]; 
extern char clientSSID[33];
extern char clientPass[65];
extern char cmDNS[33];                       
extern char apSSID[33];                       
extern byte apChannel;                         
extern byte apHide;                            
extern byte apBehavior; 
extern IPAddress staticIP;             
extern IPAddress staticGateway;        
extern IPAddress staticSubnet;   
extern bool noWifiSleep;                   
extern uint16_t ledCount;            
extern bool useRGBW;              
#define ABL_MILLIAMPS_DEFAULT 850; //auto lower brightness to stay close to milliampere limit
extern bool turnOnAtBoot;          
extern byte bootPreset;               
extern byte col[]; 
extern byte colSec[];  
extern byte briS;            
extern byte nightlightTargetBri; 
extern byte nightlightDelayMins;
extern bool nightlightFade;       
extern bool nightlightColorFade; 
extern bool fadeTransition;       
extern uint16_t transitionDelay;   
extern bool skipFirstLed; 
extern byte briMultiplier;  
extern char serverDescription[33]; 
extern bool syncToggleReceive;      
extern bool buttonEnabled;
extern byte irEnabled; 
extern uint16_t udpPort;    
extern uint16_t udpRgbPort; 
extern bool receiveNotificationBrightness; 
extern bool receiveNotificationColor;      
extern bool receiveNotificationEffects;    
extern bool notifyDirect;                 
extern bool notifyButton;                 
extern bool notifyAlexa;                  
extern bool notifyMacro;                  
extern bool notifyHue;                     
extern bool notifyTwice;                  
extern bool alexaEnabled;               
extern char alexaInvocationName[33]; 
extern char blynkApiKey[36]; 
extern uint16_t realtimeTimeoutMs;      
extern int arlsOffset;                     
extern bool receiveDirect;              
extern bool arlsDisableGammaCorrection; 
extern bool arlsForceMaxBri;           
#define E131_MAX_UNIVERSE_COUNT 9
extern uint16_t e131Universe;                               
extern uint8_t DMXMode;                 
extern uint16_t DMXAddress;                                 
extern uint8_t DMXOldDimmer;                                
extern uint8_t e131LastSequenceNumber[E131_MAX_UNIVERSE_COUNT]; 
extern bool e131Multicast;                              
extern bool e131SkipOutOfSequence;                      
extern bool mqttEnabled;
extern char mqttDeviceTopic[33];        
extern char mqttGroupTopic[33]; 
extern char mqttServer[33];             
extern char mqttUser[41];               
extern char mqttPass[41];               
extern char mqttClientID[41];           
extern uint16_t mqttPort;
extern bool huePollingEnabled;    
extern uint16_t huePollIntervalMs; 
extern char hueApiKey[47];        
extern byte huePollLightId;           
extern IPAddress hueIP;    
extern bool hueApplyOnOff;
extern bool hueApplyBri;
extern bool hueApplyColor;
extern bool ntpEnabled;  
extern bool useAMPM;     
extern byte currentTimezone; 
extern int utcOffsetSecs;    
extern byte overlayDefault;                        
extern byte overlayMin; 
extern byte overlayMax;
extern byte analogClock12pixel;          
extern bool analogClockSecondsTrail; 
extern bool analogClock5MinuteMarks; 
extern char cronixieDisplay[7]; 
extern bool cronixieBacklight;      
extern bool countdownMode;                  
extern byte countdownYear, countdownMonth; 
extern byte countdownDay, countdownHour;
extern byte countdownMin, countdownSec;
extern byte macroBoot; 
extern byte macroNl;   
extern byte macroCountdown;
extern byte macroAlexaOn, macroAlexaOff;
extern byte macroButton, macroLongPress, macroDoublePress;
extern bool otaLock;    
extern bool wifiLock;   
extern bool aOtaEnabled; 
extern uint16_t userVar0, userVar1;
#ifdef WLED_ENABLE_DMX
extern byte DMXChannels; 
extern byte DMXFixtureMap[15];
extern 
extern uint16_t DMXGap;   
extern uint16_t DMXStart; 
#endif
extern bool apActive;
extern bool forceReconnect;
extern uint32_t lastReconnectAttempt;
extern bool interfacesInited;
extern bool wasConnected;
extern byte colOld[]; 
extern byte colT[];   
extern byte colIT[];  
extern byte colSecT[];
extern byte colSecOld[];
extern byte colSecIT[];
extern byte lastRandomIndex; 
extern bool transitionActive;
extern uint16_t transitionDelayDefault;
extern uint16_t transitionDelayTemp;
extern unsigned long transitionStartTime;
extern float tperLast; 
extern bool jsonTransitionOnce;
extern bool nightlightActive;
extern bool nightlightActiveOld;
extern uint32_t nightlightDelayMs;
extern uint8_t nightlightDelayMinsDefault;
extern unsigned long nightlightStartTime;
extern byte briNlT;           
extern byte colNlT[]; 
extern unsigned long lastOnTime;
extern bool offMode;
extern byte bri;
extern byte briOld;
extern byte briT;
extern byte briIT;
extern byte briLast;   
extern byte whiteLast; 
extern bool buttonPressedBefore;
extern bool buttonLongPressed;
extern unsigned long buttonPressedTime;
extern unsigned long buttonWaitTime;
extern bool notifyDirectDefault;
extern bool receiveNotifications;
extern unsigned long notificationSentTime;
extern byte notificationSentCallMode;
extern bool notificationTwoRequired;
extern byte effectCurrent;
extern byte effectSpeed;
extern byte effectIntensity;
extern byte effectPalette;
extern bool udpConnected, udpRgbConnected;
extern bool showWelcomePage;
extern byte hueError;
extern float hueXLast, hueYLast;
extern uint16_t hueHueLast, hueCtLast;
extern byte hueSatLast, hueBriLast;
extern unsigned long hueLastRequestSent;
extern bool hueAuthRequired;
extern bool hueReceived;
extern bool hueStoreAllowed, hueNewKey;
extern byte overlayCurrent;
extern byte overlaySpeed;
extern unsigned long overlayRefreshMs;
extern unsigned long overlayRefreshedTime;
extern byte dP[];
extern bool cronixieInit;
extern unsigned long countdownTime;
extern bool countdownOverTriggered;
extern byte lastTimerMinute;
extern byte timerHours[];
extern byte timerMinutes[];
extern byte timerMacro[];
extern byte timerWeekday[]; 
extern bool blynkEnabled;
extern bool presetCyclingEnabled;
extern byte presetCycleMin, presetCycleMax;
extern uint16_t presetCycleTime;
extern unsigned long presetCycledTime;
extern byte presetCycCurr;
extern bool presetApplyBri;
extern bool saveCurrPresetCycConf;
extern byte realtimeMode;
extern IPAddress realtimeIP;
extern unsigned long realtimeTimeout;
extern long lastMqttReconnectAttempt;
extern long lastInterfaceUpdate;
extern byte interfaceUpdateCallMode;
extern char mqttStatusTopic[40]; 
#if AUXPIN >= 0
extern byte auxTime;
extern unsigned long auxStartTime;
extern bool auxActive;
#endif
extern String escapedMac;
#ifndef WLED_DISABLE_ALEXA
extern Espalexa espalexa;
extern EspalexaDevice *espalexaDevice;
#endif
extern DNSServer dnsServer;
extern bool ntpConnected;
extern time_t local;
extern unsigned long ntpLastSyncTime;
extern unsigned long ntpPacketSentTime;
extern IPAddress ntpServerIP;
extern uint16_t ntpLocalPort;
#define NTP_PACKET_SIZE 48

//maximum number of LEDs - MAX_LEDS is coming from the JSON response getting too big, MAX_LEDS_DMA will become a timing issue
#define MAX_LEDS 1500
#define MAX_LEDS_DMA 500

//string temp buffer (now stored in stack locally)
#define OMAX 2048
extern char *obuf;
extern uint16_t olen;

extern uint16_t savedPresets;
extern int8_t currentPreset;
extern bool isPreset;
extern byte errorFlag;
extern String messageHead, messageSub;
extern byte optionType;
extern bool doReboot; 
extern bool doPublishMqtt;
extern AsyncWebServer server;
extern AsyncClient *hueClient;
extern AsyncMqttClient *mqtt;
extern WiFiUDP notifierUdp, rgbUdp;
extern WiFiUDP ntpUdp;
extern ESPAsyncE131 e131;
extern bool e131NewData;
extern WS2812FX strip;

#define WLED_CONNECTED (WiFi.status() == WL_CONNECTED)
#define WLED_WIFI_CONFIGURED (strlen(clientSSID) >= 1 && strcmp(clientSSID, DEFAULT_CLIENT_SSID) != 0)

//debug macros
#ifdef WLED_DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINTF(x) Serial.printf(x)
extern unsigned long debugTime;
extern int lastWifiState;
extern unsigned long wifiStateChangedTime;
extern int loops;
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(x)
#endif

// append new c string to temp buffer efficiently
bool oappend(const char *txt);
// append new number to temp buffer efficiently
bool oappendi(int i);

class WLED
{
public:
    WLED(); 
    static WLED &instance()
    {
        static WLED instance;
        return instance;
    }

    //boot starts here
    void setup();

    void loop();
    void reset();

    void beginStrip();
    void handleConnection();
    void initAP(bool resetAP = false);
    void initConnection();
    void initInterfaces();
};
#endif // WLED_H
