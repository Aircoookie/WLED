#ifndef WLED_H
#define WLED_H

/*
   Main sketch, global variable declarations
*/
/*
 * @title WLED project sketch
 * @version 0.9.1
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
//#define WLED_DISABLE_INFRARED    //there is no pin left for this on ESP8266-01, saves 12kb
#define WLED_ENABLE_MQTT     //saves 12kb
#define WLED_ENABLE_ADALIGHT //saves 500b only
//#define WLED_ENABLE_DMX          //uses 3.5kb

#define WLED_DISABLE_FILESYSTEM //SPIFFS is not used by any WLED feature yet
//#define WLED_ENABLE_FS_SERVING   //Enable sending html file from SPIFFS before serving progmem version
//#define WLED_ENABLE_FS_EDITOR    //enable /edit page for editing SPIFFS content. Will also be disabled with OTA lock

//to toggle usb serial debug (un)comment the following line
//#define WLED_DEBUG

//library inclusions
#include <Arduino.h>
#ifdef WLED_ENABLE_DMX
#include <ESPDMX.h>
DMXESPSerial dmx;
#endif
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

//version code in format yymmddb (b = daily build)
#define VERSION 2003222

extern char versionString[];

//AP and OTA default passwords (for maximum change them!)
extern char apPass[65];
extern char otaPass[33];

//Hardware CONFIG (only changeble HERE, not at runtime)
//LED strip pin, button pin and IR pin changeable in NpbWrapper.h!

extern byte auxDefaultState;                       //0: input 1: high 2: low
extern byte auxTriggeredState;                     //0: input 1: high 2: low
extern char ntpServerName[33]; //NTP server to use

//WiFi CONFIG (all these can be changed via web UI, no need to set them here)
extern char clientSSID[33];
extern char clientPass[65];
extern char cmDNS[33];                       //mDNS address (placeholder, will be replaced by wledXXXXXXXXXXXX.local)
extern char apSSID[33];                       //AP off by default (unless setup)
extern byte apChannel;                         //2.4GHz WiFi AP channel (1-13)
extern byte apHide;                            //hidden AP SSID
extern byte apBehavior; //access point opens when no connection after boot by default
extern IPAddress staticIP;             //static IP of ESP
extern IPAddress staticGateway;        //gateway (router) IP
extern IPAddress staticSubnet;   //most common subnet in home networks
extern bool noWifiSleep;                   //disabling modem sleep modes will increase heat output and power usage, but may help with connection issues

//LED CONFIG
extern uint16_t ledCount;            //overcurrent prevented by ABL
extern bool useRGBW;              //SK6812 strips can contain an extra White channel
#define ABL_MILLIAMPS_DEFAULT 850; //auto lower brightness to stay close to milliampere limit
extern bool turnOnAtBoot;          //turn on LEDs at power-up
extern byte bootPreset;               //save preset to load after power-up

extern byte col[]; //current RGB(W) primary color. col[] should be updated if you want to change the color.
extern byte colSec[];  //current RGB(W) secondary color
extern byte briS;            //default brightness

extern byte nightlightTargetBri; //brightness after nightlight is over
extern byte nightlightDelayMins;
extern bool nightlightFade;       //if enabled, light will gradually dim towards the target bri. Otherwise, it will instantly set after delay over
extern bool nightlightColorFade; //if enabled, light will gradually fade color from primary to secondary color.
extern bool fadeTransition;       //enable crossfading color transition
extern uint16_t transitionDelay;   //default crossfade duration in ms

extern bool skipFirstLed; //ignore first LED in strip (useful if you need the LED as signal repeater)
extern byte briMultiplier;  //% of brightness to set (to limit power, if you set it to 50 and set bri to 255, actual brightness will be 127)

//User Interface CONFIG
extern char serverDescription[33]; //Name of module
extern bool syncToggleReceive;      //UIs which only have a single button for sync should toggle send+receive if this is true, only send otherwise

//Sync CONFIG
extern bool buttonEnabled;
extern byte irEnabled; //Infrared receiver

extern uint16_t udpPort;    //WLED notifier default port
extern uint16_t udpRgbPort; //Hyperion port

extern bool receiveNotificationBrightness; //apply brightness from incoming notifications
extern bool receiveNotificationColor;      //apply color
extern bool receiveNotificationEffects;    //apply effects setup
extern bool notifyDirect;                 //send notification if change via UI or HTTP API
extern bool notifyButton;                 //send if updated by button or infrared remote
extern bool notifyAlexa;                  //send notification if updated via Alexa
extern bool notifyMacro;                  //send notification for macro
extern bool notifyHue;                     //send notification if Hue light changes
extern bool notifyTwice;                  //notifications use UDP: enable if devices don't sync reliably

extern bool alexaEnabled;               //enable device discovery by Amazon Echo
extern char alexaInvocationName[33]; //speech control name of device. Choose something voice-to-text can understand

extern char blynkApiKey[36]; //Auth token for Blynk server. If empty, no connection will be made

extern uint16_t realtimeTimeoutMs;      //ms timeout of realtime mode before returning to normal mode
extern int arlsOffset;                     //realtime LED offset
extern bool receiveDirect;              //receive UDP realtime
extern bool arlsDisableGammaCorrection; //activate if gamma correction is handled by the source
extern bool arlsForceMaxBri;           //enable to force max brightness if source has very dark colors that would be black

#define E131_MAX_UNIVERSE_COUNT 9
extern uint16_t e131Universe;                               //settings for E1.31 (sACN) protocol (only DMX_MODE_MULTIPLE_* can span over consequtive universes)
extern uint8_t DMXMode;                 //DMX mode (s.a.)
extern uint16_t DMXAddress;                                 //DMX start address of fixture, a.k.a. first Channel [for E1.31 (sACN) protocol]
extern uint8_t DMXOldDimmer;                                //only update brightness on change
extern uint8_t e131LastSequenceNumber[E131_MAX_UNIVERSE_COUNT]; //to detect packet loss
extern bool e131Multicast;                              //multicast or unicast
extern bool e131SkipOutOfSequence;                      //freeze instead of flickering

extern bool mqttEnabled;
extern char mqttDeviceTopic[33];        //main MQTT topic (individual per device, default is wled/mac)
extern char mqttGroupTopic[33]; //second MQTT topic (for example to group devices)
extern char mqttServer[33];             //both domains and IPs should work (no SSL)
extern char mqttUser[41];               //optional: username for MQTT auth
extern char mqttPass[41];               //optional: password for MQTT auth
extern char mqttClientID[41];           //override the client ID
extern uint16_t mqttPort;

extern bool huePollingEnabled;    //poll hue bridge for light state
extern uint16_t huePollIntervalMs; //low values (< 1sec) may cause lag but offer quicker response
extern char hueApiKey[47];        //key token will be obtained from bridge
extern byte huePollLightId;           //ID of hue lamp to sync to. Find the ID in the hue app ("about" section)
extern IPAddress hueIP;    //IP address of the bridge
extern bool hueApplyOnOff;
extern bool hueApplyBri;
extern bool hueApplyColor;

//Time CONFIG
extern bool ntpEnabled;  //get internet time. Only required if you use clock overlays or time-activated macros
extern bool useAMPM;     //12h/24h clock format
extern byte currentTimezone; //Timezone ID. Refer to timezones array in wled_ntp.cpp
extern int utcOffsetSecs;    //Seconds to offset from UTC before timzone calculation

extern byte overlayDefault;                        //0: no overlay 1: analog clock 2: single-digit clocl 3: cronixie
extern byte overlayMin; //boundaries of overlay mode
extern byte overlayMax;

extern byte analogClock12pixel;          //The pixel in your strip where "midnight" would be
extern bool analogClockSecondsTrail; //Display seconds as trail of LEDs instead of a single pixel
extern bool analogClock5MinuteMarks; //Light pixels at every 5-minute position

extern char cronixieDisplay[7]; //Cronixie Display mask. See wled13_cronixie.ino
extern bool cronixieBacklight;      //Allow digits to be back-illuminated

extern bool countdownMode;                  //Clock will count down towards date
extern byte countdownYear, countdownMonth; //Countdown target date, year is last two digits
extern byte countdownDay, countdownHour;
extern byte countdownMin, countdownSec;

extern byte macroBoot; //macro loaded after startup
extern byte macroNl;   //after nightlight delay over
extern byte macroCountdown;
extern byte macroAlexaOn, macroAlexaOff;
extern byte macroButton, macroLongPress, macroDoublePress;

//Security CONFIG
extern bool otaLock;    //prevents OTA firmware updates without password. ALWAYS enable if system exposed to any public networks
extern bool wifiLock;   //prevents access to WiFi settings when OTA lock is enabled
extern bool aOtaEnabled; //ArduinoOTA allows easy updates directly from the IDE. Careful, it does not auto-disable when OTA lock is on

extern uint16_t userVar0, userVar1;

#ifdef WLED_ENABLE_DMX
//dmx CONFIG
extern byte DMXChannels; // number of channels per fixture
extern byte DMXFixtureMap[15];
extern // assigns the different channels to different functions. See wled21_dmx.ino for more information.
extern uint16_t DMXGap;   // gap between the fixtures. makes addressing easier because you don't have to memorize odd numbers when climbing up onto a rig.
extern uint16_t DMXStart; // start address of the first fixture
#endif

//internal global variable declarations
//wifi
extern bool apActive;
extern bool forceReconnect;
extern uint32_t lastReconnectAttempt;
extern bool interfacesInited;
extern bool wasConnected;

//color
extern byte colOld[]; //color before transition
extern byte colT[];   //color that is currently displayed on the LEDs
extern byte colIT[];  //color that was last sent to LEDs
extern byte colSecT[];
extern byte colSecOld[];
extern byte colSecIT[];

extern byte lastRandomIndex; //used to save last random color so the new one is not the same

//transitions
extern bool transitionActive;
extern uint16_t transitionDelayDefault;
extern uint16_t transitionDelayTemp;
extern unsigned long transitionStartTime;
extern float tperLast; //crossfade transition progress, 0.0f - 1.0f
extern bool jsonTransitionOnce;

//nightlight
extern bool nightlightActive;
extern bool nightlightActiveOld;
extern uint32_t nightlightDelayMs;
extern uint8_t nightlightDelayMinsDefault;
extern unsigned long nightlightStartTime;
extern byte briNlT;           //current nightlight brightness
extern byte colNlT[]; //current nightlight color

extern unsigned long lastOnTime;
extern bool offMode;
extern byte bri;
extern byte briOld;
extern byte briT;
extern byte briIT;
extern byte briLast;   //brightness before turned off. Used for toggle function
extern byte whiteLast; //white channel before turned off. Used for toggle function

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
//uint16_t hueFailCount;
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

//countdown
extern unsigned long countdownTime;
extern bool countdownOverTriggered;

//timer
extern byte lastTimerMinute;
extern byte timerHours[];
extern byte timerMinutes[];
extern byte timerMacro[];
extern byte timerWeekday[]; //weekdays to activate on
//bit pattern of arr elem: 0b11111111: sun,sat,fri,thu,wed,tue,mon,validity

//blynk
extern bool blynkEnabled;

//preset cycling
extern bool presetCyclingEnabled;
extern byte presetCycleMin, presetCycleMax;
extern uint16_t presetCycleTime;
extern unsigned long presetCycledTime;
extern byte presetCycCurr;
extern bool presetApplyBri;
extern bool saveCurrPresetCycConf;

//realtime
extern byte realtimeMode;
extern IPAddress realtimeIP;
extern unsigned long realtimeTimeout;

//mqtt
extern long lastMqttReconnectAttempt;
extern long lastInterfaceUpdate;
extern byte interfaceUpdateCallMode;
extern char mqttStatusTopic[40]; //this must be global because of async handlers

#if AUXPIN >= 0
//auxiliary debug pin
extern byte auxTime;
extern unsigned long auxStartTime;
extern bool auxActive;
#endif

//alexa udp
extern String escapedMac;
#ifndef WLED_DISABLE_ALEXA
extern Espalexa espalexa;
extern EspalexaDevice *espalexaDevice;
#endif

//dns server
extern DNSServer dnsServer;

//network time
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

//presets
extern uint16_t savedPresets;
extern int8_t currentPreset;
extern bool isPreset;

extern byte errorFlag;

extern String messageHead, messageSub;
extern byte optionType;

extern bool doReboot; //flag to initiate reboot from async handlers
extern bool doPublishMqtt;

//server library objects
extern AsyncWebServer server;
extern AsyncClient *hueClient;
extern AsyncMqttClient *mqtt;

//function prototypes
extern void colorFromUint32(uint32_t, bool);
extern void serveMessage(AsyncWebServerRequest *, uint16_t, String, String, byte);
extern void handleE131Packet(e131_packet_t *, IPAddress);
extern void arlsLock(uint32_t, byte);
extern void handleOverlayDraw();

//udp interface objects
extern WiFiUDP notifierUdp, rgbUdp;
extern WiFiUDP ntpUdp;
extern ESPAsyncE131 e131;
extern bool e131NewData;

//led fx library object
extern WS2812FX strip;


#define WLED_CONNECTED (WiFi.status() == WL_CONNECTED)
#define WLED_WIFI_CONFIGURED (strlen(clientSSID) >= 1 && strcmp(clientSSID, DEFAULT_CLIENT_SSID) != 0)

//debug macros
#ifdef WLED_DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINTF(x) Serial.printf(x)
unsigned long debugTime = 0;
int lastWifiState = 3;
unsigned long wifiStateChangedTime = 0;
int loops = 0;
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(x)
#endif


// TODO: Inline?
//append new c string to temp buffer efficiently
bool oappend(const char *txt);
//append new number to temp buffer efficiently
bool oappendi(int i);
int getSignalQuality(int rssi);

class WLED
{
public:
    static WLED &instance()
    {
        static WLED instance;
        return instance;
    }

    WLED();

    void reset();
    void loop();


    //boot starts here
    void setup()
    {
        wledInit();
    }

public: // TODO: privacy
    void wledInit();
    void beginStrip();

    void handleConnection();
    void initAP(bool resetAP = false);
    void initConnection();
    void initInterfaces();
};
#endif // WLED_H
