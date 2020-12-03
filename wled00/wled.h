#ifndef WLED_H
#define WLED_H
/*
   Main sketch, global variable declarations
   @title WLED project sketch
   @version 0.11.0
   @author Christian Schwinne
 */

// version code in format yymmddb (b = daily build)
#define VERSION 2012020

//uncomment this if you have a "my_config.h" file you'd like to use
//#define WLED_USE_MY_CONFIG

// ESP8266-01 (blue) got too little storage space to work with WLED. 0.10.2 is the last release supporting this unit.

// ESP8266-01 (black) has 1MB flash and can thus fit the whole program, although OTA update is not possible. Use 1M(128K SPIFFS).
// Uncomment some of the following lines to disable features to compile for ESP8266-01 (max flash size 434kB):
// Alternatively, with platformio pass your chosen flags to your custom build target in platformio.ini.override

// You are required to disable over-the-air updates:
//#define WLED_DISABLE_OTA         // saves 14kb

// You need to choose some of these features to disable:
//#define WLED_DISABLE_ALEXA       // saves 11kb
//#define WLED_DISABLE_BLYNK       // saves 6kb
//#define WLED_DISABLE_CRONIXIE    // saves 3kb
//#define WLED_DISABLE_HUESYNC     // saves 4kb
//#define WLED_DISABLE_INFRARED    // there is no pin left for this on ESP8266-01, saves 12kb
#ifndef WLED_DISABLE_MQTT
  #define WLED_ENABLE_MQTT         // saves 12kb
#endif
#define WLED_ENABLE_ADALIGHT       // saves 500b only
//#define WLED_ENABLE_DMX          // uses 3.5kb (use LEDPIN other than 2)
#define WLED_ENABLE_LOXONE         // uses 1.2kb
#ifndef WLED_DISABLE_WEBSOCKETS
  #define WLED_ENABLE_WEBSOCKETS
#endif

#define WLED_ENABLE_FS_EDITOR      // enable /edit page for editing FS content. Will also be disabled with OTA lock

// to toggle usb serial debug (un)comment the following line
//#define WLED_DEBUG

// filesystem specific debugging
//#define WLED_DEBUG_FS

// Library inclusions. 
#include <Arduino.h>
#ifdef ESP8266
  #include <ESP8266WiFi.h>
  #include <ESP8266mDNS.h>
  #include <ESPAsyncTCP.h>
  #include <LittleFS.h>
  extern "C"
  {
  #include <user_interface.h>
  }
#else // ESP32
  #include <WiFi.h>
  #include <ETH.h>
  #include "esp_wifi.h"
  #include <ESPmDNS.h>
  #include <AsyncTCP.h>
  //#include "SPIFFS.h"
  #define CONFIG_LITTLEFS_FOR_IDF_3_2
  #include <LITTLEFS.h>
#endif

#include "src/dependencies/network/Network.h"

#ifdef WLED_USE_MY_CONFIG
  #include "my_config.h"
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

#ifdef WLED_ENABLE_DMX
  #include "src/dependencies/dmx/ESPDMX.h"
#endif

#include "src/dependencies/e131/ESPAsyncE131.h"
#include "src/dependencies/async-mqtt-client/AsyncMqttClient.h"

#define ARDUINOJSON_DECODE_UNICODE 0
#include "src/dependencies/json/AsyncJson-v6.h"
#include "src/dependencies/json/ArduinoJson-v6.h"

#include "fcn_declare.h"
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

#ifndef SPIFFS_EDITOR_AIRCOOOKIE
  #error You are not using the Aircoookie fork of the ESPAsyncWebserver library.\
  Using upstream puts your WiFi password at risk of being served by the filesystem.\
  Comment out this error message to build regardless.
#endif

#if IRPIN < 0
  #ifndef WLED_DISABLE_INFRARED
    #define WLED_DISABLE_INFRARED
  #endif
#endif

#ifndef WLED_DISABLE_INFRARED
  #include <IRremoteESP8266.h>
  #include <IRrecv.h>
  #include <IRutils.h>
#endif

//Filesystem to use for preset and config files. SPIFFS or LittleFS on ESP8266, SPIFFS only on ESP32 (now using LITTLEFS port by lorol)
#ifdef ESP8266
  #define WLED_FS LittleFS
#else
  #define WLED_FS LITTLEFS
#endif

// remove flicker because PWM signal of RGB channels can become out of phase (part of core as of Arduino core v2.7.0)
//#if defined(WLED_USE_ANALOG_LEDS) && defined(ESP8266)
//  #include "src/dependencies/arduino/core_esp8266_waveform.h"
//#endif

// GLOBAL VARIABLES
// both declared and defined in header (solution from http://www.keil.com/support/docs/1868.htm)
//
//e.g. byte test = 2 becomes WLED_GLOBAL byte test _INIT(2);
//     int arr[]{0,1,2} becomes WLED_GLOBAL int arr[] _INIT_N(({0,1,2}));

#ifndef WLED_DEFINE_GLOBAL_VARS
# define WLED_GLOBAL extern
# define _INIT(x)
# define _INIT_N(x)
#else
# define WLED_GLOBAL
# define _INIT(x) = x

//needed to ignore commas in array definitions
#define UNPACK( ... ) __VA_ARGS__
# define _INIT_N(x) UNPACK x
#endif

// Global Variable definitions
WLED_GLOBAL char versionString[] _INIT("0.11.0");
#define WLED_CODENAME "Mirai"

// AP and OTA default passwords (for maximum security change them!)
WLED_GLOBAL char apPass[65]  _INIT(DEFAULT_AP_PASS);
WLED_GLOBAL char otaPass[33] _INIT(DEFAULT_OTA_PASS);

// Hardware CONFIG (only changeble HERE, not at runtime)
// LED strip pin, button pin and IR pin changeable in NpbWrapper.h!

//WLED_GLOBAL byte presetToApply _INIT(0); 

#if AUXPIN >= 0
WLED_GLOBAL byte auxDefaultState _INIT(0);                         // 0: input 1: high 2: low
WLED_GLOBAL byte auxTriggeredState _INIT(0);                       // 0: input 1: high 2: low
#endif
WLED_GLOBAL char ntpServerName[33] _INIT("0.wled.pool.ntp.org");   // NTP server to use

// WiFi CONFIG (all these can be changed via web UI, no need to set them here)
WLED_GLOBAL char clientSSID[33] _INIT(CLIENT_SSID);
WLED_GLOBAL char clientPass[65] _INIT(CLIENT_PASS);
WLED_GLOBAL char cmDNS[33] _INIT("x");                             // mDNS address (placeholder, is replaced by wledXXXXXX.local)
WLED_GLOBAL char apSSID[33] _INIT("");                             // AP off by default (unless setup)
WLED_GLOBAL byte apChannel _INIT(1);                               // 2.4GHz WiFi AP channel (1-13)
WLED_GLOBAL byte apHide    _INIT(0);                               // hidden AP SSID
WLED_GLOBAL byte apBehavior _INIT(AP_BEHAVIOR_BOOT_NO_CONN);       // access point opens when no connection after boot by default
WLED_GLOBAL IPAddress staticIP      _INIT_N(((  0,   0,  0,  0))); // static IP of ESP
WLED_GLOBAL IPAddress staticGateway _INIT_N(((  0,   0,  0,  0))); // gateway (router) IP
WLED_GLOBAL IPAddress staticSubnet  _INIT_N(((255, 255, 255, 0))); // most common subnet in home networks
WLED_GLOBAL bool noWifiSleep _INIT(false);                         // disabling modem sleep modes will increase heat output and power usage, but may help with connection issues

// LED CONFIG
WLED_GLOBAL uint16_t ledCount _INIT(30);          // overcurrent prevented by ABL
WLED_GLOBAL bool useRGBW      _INIT(false);       // SK6812 strips can contain an extra White channel
WLED_GLOBAL bool turnOnAtBoot _INIT(true);        // turn on LEDs at power-up
WLED_GLOBAL byte bootPreset   _INIT(0);           // save preset to load after power-up

WLED_GLOBAL byte col[]    _INIT_N(({ 255, 160, 0, 0 }));  // current RGB(W) primary color. col[] should be updated if you want to change the color.
WLED_GLOBAL byte colSec[] _INIT_N(({ 0, 0, 0, 0 }));      // current RGB(W) secondary color
WLED_GLOBAL byte briS     _INIT(128);                     // default brightness

WLED_GLOBAL byte nightlightTargetBri _INIT(0);      // brightness after nightlight is over
WLED_GLOBAL byte nightlightDelayMins _INIT(60);
WLED_GLOBAL byte nightlightMode      _INIT(NL_MODE_FADE); // See const.h for available modes. Was nightlightFade
WLED_GLOBAL bool fadeTransition      _INIT(true);   // enable crossfading color transition
WLED_GLOBAL uint16_t transitionDelay _INIT(750);    // default crossfade duration in ms

WLED_GLOBAL bool skipFirstLed  _INIT(false);        // ignore first LED in strip (useful if you need the LED as signal repeater)
WLED_GLOBAL byte briMultiplier _INIT(100);          // % of brightness to set (to limit power, if you set it to 50 and set bri to 255, actual brightness will be 127)

// User Interface CONFIG
WLED_GLOBAL char serverDescription[33] _INIT("WLED");  // Name of module
WLED_GLOBAL bool syncToggleReceive     _INIT(false);   // UIs which only have a single button for sync should toggle send+receive if this is true, only send otherwise

// Sync CONFIG
WLED_GLOBAL bool buttonEnabled  _INIT(true);
WLED_GLOBAL byte irEnabled      _INIT(0);     // Infrared receiver

WLED_GLOBAL uint16_t udpPort    _INIT(21324); // WLED notifier default port
WLED_GLOBAL uint16_t udpPort2   _INIT(65506); // WLED notifier supplemental port
WLED_GLOBAL uint16_t udpRgbPort _INIT(19446); // Hyperion port

WLED_GLOBAL bool receiveNotificationBrightness _INIT(true);       // apply brightness from incoming notifications
WLED_GLOBAL bool receiveNotificationColor      _INIT(true);       // apply color
WLED_GLOBAL bool receiveNotificationEffects    _INIT(true);       // apply effects setup
WLED_GLOBAL bool notifyDirect _INIT(false);                       // send notification if change via UI or HTTP API
WLED_GLOBAL bool notifyButton _INIT(false);                       // send if updated by button or infrared remote
WLED_GLOBAL bool notifyAlexa  _INIT(false);                       // send notification if updated via Alexa
WLED_GLOBAL bool notifyMacro  _INIT(false);                       // send notification for macro
WLED_GLOBAL bool notifyHue    _INIT(true);                        // send notification if Hue light changes
WLED_GLOBAL bool notifyTwice  _INIT(false);                       // notifications use UDP: enable if devices don't sync reliably

WLED_GLOBAL bool alexaEnabled _INIT(false);                       // enable device discovery by Amazon Echo
WLED_GLOBAL char alexaInvocationName[33] _INIT("Light");          // speech control name of device. Choose something voice-to-text can understand

WLED_GLOBAL char blynkApiKey[36] _INIT("");                       // Auth token for Blynk server. If empty, no connection will be made

WLED_GLOBAL uint16_t realtimeTimeoutMs _INIT(2500);               // ms timeout of realtime mode before returning to normal mode
WLED_GLOBAL int arlsOffset _INIT(0);                              // realtime LED offset
WLED_GLOBAL bool receiveDirect _INIT(true);                       // receive UDP realtime
WLED_GLOBAL bool arlsDisableGammaCorrection _INIT(true);          // activate if gamma correction is handled by the source
WLED_GLOBAL bool arlsForceMaxBri _INIT(false);                    // enable to force max brightness if source has very dark colors that would be black

#ifdef WLED_ENABLE_DMX
WLED_GLOBAL DMXESPSerial dmx;
WLED_GLOBAL uint16_t e131ProxyUniverse _INIT(0);                  // output this E1.31 (sACN) / ArtNet universe via MAX485 (0 = disabled)
#endif
WLED_GLOBAL uint16_t e131Universe _INIT(1);                       // settings for E1.31 (sACN) protocol (only DMX_MODE_MULTIPLE_* can span over consequtive universes)
WLED_GLOBAL uint16_t e131Port _INIT(5568);                        // DMX in port. E1.31 default is 5568, Art-Net is 6454
WLED_GLOBAL byte DMXMode _INIT(DMX_MODE_MULTIPLE_RGB);            // DMX mode (s.a.)
WLED_GLOBAL uint16_t DMXAddress _INIT(1);                         // DMX start address of fixture, a.k.a. first Channel [for E1.31 (sACN) protocol]
WLED_GLOBAL byte DMXOldDimmer _INIT(0);                           // only update brightness on change
WLED_GLOBAL byte e131LastSequenceNumber[E131_MAX_UNIVERSE_COUNT]; // to detect packet loss
WLED_GLOBAL bool e131Multicast _INIT(false);                      // multicast or unicast
WLED_GLOBAL bool e131SkipOutOfSequence _INIT(false);              // freeze instead of flickering

WLED_GLOBAL bool mqttEnabled _INIT(false);
WLED_GLOBAL char mqttDeviceTopic[33] _INIT("");            // main MQTT topic (individual per device, default is wled/mac)
WLED_GLOBAL char mqttGroupTopic[33] _INIT("wled/all");     // second MQTT topic (for example to group devices)
WLED_GLOBAL char mqttServer[33] _INIT("");                 // both domains and IPs should work (no SSL)
WLED_GLOBAL char mqttUser[41] _INIT("");                   // optional: username for MQTT auth
WLED_GLOBAL char mqttPass[41] _INIT("");                   // optional: password for MQTT auth
WLED_GLOBAL char mqttClientID[41] _INIT("");               // override the client ID
WLED_GLOBAL uint16_t mqttPort _INIT(1883);

WLED_GLOBAL bool huePollingEnabled _INIT(false);           // poll hue bridge for light state
WLED_GLOBAL uint16_t huePollIntervalMs _INIT(2500);        // low values (< 1sec) may cause lag but offer quicker response
WLED_GLOBAL char hueApiKey[47] _INIT("api");               // key token will be obtained from bridge
WLED_GLOBAL byte huePollLightId _INIT(1);                  // ID of hue lamp to sync to. Find the ID in the hue app ("about" section)
WLED_GLOBAL IPAddress hueIP _INIT((0, 0, 0, 0));           // IP address of the bridge
WLED_GLOBAL bool hueApplyOnOff _INIT(true);
WLED_GLOBAL bool hueApplyBri _INIT(true);
WLED_GLOBAL bool hueApplyColor _INIT(true);

// Time CONFIG
WLED_GLOBAL bool ntpEnabled _INIT(false);         // get internet time. Only required if you use clock overlays or time-activated macros
WLED_GLOBAL bool useAMPM _INIT(false);            // 12h/24h clock format
WLED_GLOBAL byte currentTimezone _INIT(0);        // Timezone ID. Refer to timezones array in wled10_ntp.ino
WLED_GLOBAL int utcOffsetSecs _INIT(0);           // Seconds to offset from UTC before timzone calculation

WLED_GLOBAL byte overlayDefault _INIT(0);                               // 0: no overlay 1: analog clock 2: single-digit clocl 3: cronixie
WLED_GLOBAL byte overlayMin _INIT(0), overlayMax _INIT(ledCount - 1);   // boundaries of overlay mode

WLED_GLOBAL byte analogClock12pixel _INIT(0);               // The pixel in your strip where "midnight" would be
WLED_GLOBAL bool analogClockSecondsTrail _INIT(false);      // Display seconds as trail of LEDs instead of a single pixel
WLED_GLOBAL bool analogClock5MinuteMarks _INIT(false);      // Light pixels at every 5-minute position

WLED_GLOBAL char cronixieDisplay[7] _INIT("HHMMSS");        // Cronixie Display mask. See wled13_cronixie.ino
WLED_GLOBAL bool cronixieBacklight _INIT(true);             // Allow digits to be back-illuminated

WLED_GLOBAL bool countdownMode _INIT(false);                         // Clock will count down towards date
WLED_GLOBAL byte countdownYear _INIT(20), countdownMonth _INIT(1);   // Countdown target date, year is last two digits
WLED_GLOBAL byte countdownDay  _INIT(1) , countdownHour  _INIT(0);
WLED_GLOBAL byte countdownMin  _INIT(0) , countdownSec   _INIT(0);

WLED_GLOBAL byte macroNl   _INIT(0);        // after nightlight delay over
WLED_GLOBAL byte macroCountdown _INIT(0);
WLED_GLOBAL byte macroAlexaOn _INIT(0), macroAlexaOff _INIT(0);
WLED_GLOBAL byte macroButton _INIT(0), macroLongPress _INIT(0), macroDoublePress _INIT(0);

// Security CONFIG
WLED_GLOBAL bool otaLock     _INIT(false);  // prevents OTA firmware updates without password. ALWAYS enable if system exposed to any public networks
WLED_GLOBAL bool wifiLock    _INIT(false);  // prevents access to WiFi settings when OTA lock is enabled
WLED_GLOBAL bool aOtaEnabled _INIT(true);   // ArduinoOTA allows easy updates directly from the IDE. Careful, it does not auto-disable when OTA lock is on

WLED_GLOBAL uint16_t userVar0 _INIT(0), userVar1 _INIT(0); //available for use in usermod

#ifdef WLED_ENABLE_DMX
  // dmx CONFIG
  WLED_GLOBAL byte DMXChannels _INIT(7);        // number of channels per fixture
  WLED_GLOBAL byte DMXFixtureMap[15] _INIT_N(({ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }));
  // assigns the different channels to different functions. See wled21_dmx.ino for more information.
  WLED_GLOBAL uint16_t DMXGap _INIT(10);          // gap between the fixtures. makes addressing easier because you don't have to memorize odd numbers when climbing up onto a rig.
  WLED_GLOBAL uint16_t DMXStart _INIT(10);        // start address of the first fixture
  WLED_GLOBAL uint16_t DMXStartLED _INIT(0);      // LED from which DMX fixtures start
#endif

// internal global variable declarations
// wifi
WLED_GLOBAL bool apActive _INIT(false);
WLED_GLOBAL bool forceReconnect _INIT(false);
WLED_GLOBAL uint32_t lastReconnectAttempt _INIT(0);
WLED_GLOBAL bool interfacesInited _INIT(false);
WLED_GLOBAL bool wasConnected _INIT(false);

// color
WLED_GLOBAL byte colOld[]    _INIT_N(({ 0, 0, 0, 0 }));        // color before transition
WLED_GLOBAL byte colT[]      _INIT_N(({ 0, 0, 0, 0 }));          // color that is currently displayed on the LEDs
WLED_GLOBAL byte colIT[]     _INIT_N(({ 0, 0, 0, 0 }));         // color that was last sent to LEDs
WLED_GLOBAL byte colSecT[]   _INIT_N(({ 0, 0, 0, 0 }));
WLED_GLOBAL byte colSecOld[] _INIT_N(({ 0, 0, 0, 0 }));
WLED_GLOBAL byte colSecIT[]  _INIT_N(({ 0, 0, 0, 0 }));

WLED_GLOBAL byte lastRandomIndex _INIT(0);        // used to save last random color so the new one is not the same

// transitions
WLED_GLOBAL bool transitionActive _INIT(false);
WLED_GLOBAL uint16_t transitionDelayDefault _INIT(transitionDelay);
WLED_GLOBAL uint16_t transitionDelayTemp _INIT(transitionDelay);
WLED_GLOBAL unsigned long transitionStartTime;
WLED_GLOBAL float tperLast _INIT(0);        // crossfade transition progress, 0.0f - 1.0f
WLED_GLOBAL bool jsonTransitionOnce _INIT(false);

// nightlight
WLED_GLOBAL bool nightlightActive _INIT(false);
WLED_GLOBAL bool nightlightActiveOld _INIT(false);
WLED_GLOBAL uint32_t nightlightDelayMs _INIT(10);
WLED_GLOBAL byte nightlightDelayMinsDefault _INIT(nightlightDelayMins);
WLED_GLOBAL unsigned long nightlightStartTime;
WLED_GLOBAL byte briNlT _INIT(0);                     // current nightlight brightness
WLED_GLOBAL byte colNlT[] _INIT_N(({ 0, 0, 0, 0 }));        // current nightlight color

// brightness
WLED_GLOBAL unsigned long lastOnTime _INIT(0);
WLED_GLOBAL bool offMode _INIT(!turnOnAtBoot);
WLED_GLOBAL byte bri _INIT(briS);
WLED_GLOBAL byte briOld _INIT(0);
WLED_GLOBAL byte briT _INIT(0);
WLED_GLOBAL byte briIT _INIT(0);
WLED_GLOBAL byte briLast _INIT(128);          // brightness before turned off. Used for toggle function
WLED_GLOBAL byte whiteLast _INIT(128);        // white channel before turned off. Used for toggle function

// button
WLED_GLOBAL bool buttonPressedBefore _INIT(false);
WLED_GLOBAL bool buttonLongPressed _INIT(false);
WLED_GLOBAL unsigned long buttonPressedTime _INIT(0);
WLED_GLOBAL unsigned long buttonWaitTime _INIT(0);

// notifications
WLED_GLOBAL bool notifyDirectDefault _INIT(notifyDirect);
WLED_GLOBAL bool receiveNotifications _INIT(true);
WLED_GLOBAL unsigned long notificationSentTime _INIT(0);
WLED_GLOBAL byte notificationSentCallMode _INIT(NOTIFIER_CALL_MODE_INIT);
WLED_GLOBAL bool notificationTwoRequired _INIT(false);

// effects
WLED_GLOBAL byte effectCurrent _INIT(0);
WLED_GLOBAL byte effectSpeed _INIT(128);
WLED_GLOBAL byte effectIntensity _INIT(128);
WLED_GLOBAL byte effectPalette _INIT(0);

// network
WLED_GLOBAL bool udpConnected _INIT(false), udp2Connected _INIT(false), udpRgbConnected _INIT(false);

// ui style
WLED_GLOBAL bool showWelcomePage _INIT(false);

// hue
WLED_GLOBAL byte hueError _INIT(HUE_ERROR_INACTIVE);
// WLED_GLOBAL uint16_t hueFailCount _INIT(0);
WLED_GLOBAL float hueXLast _INIT(0), hueYLast _INIT(0);
WLED_GLOBAL uint16_t hueHueLast _INIT(0), hueCtLast _INIT(0);
WLED_GLOBAL byte hueSatLast _INIT(0), hueBriLast _INIT(0);
WLED_GLOBAL unsigned long hueLastRequestSent _INIT(0);
WLED_GLOBAL bool hueAuthRequired _INIT(false);
WLED_GLOBAL bool hueReceived _INIT(false);
WLED_GLOBAL bool hueStoreAllowed _INIT(false), hueNewKey _INIT(false);

// overlays
WLED_GLOBAL byte overlayCurrent _INIT(overlayDefault);
WLED_GLOBAL byte overlaySpeed _INIT(200);
WLED_GLOBAL unsigned long overlayRefreshMs _INIT(200);
WLED_GLOBAL unsigned long overlayRefreshedTime;

// cronixie
WLED_GLOBAL byte dP[] _INIT_N(({ 0, 0, 0, 0, 0, 0 }));
WLED_GLOBAL bool cronixieInit _INIT(false);

// countdown
WLED_GLOBAL unsigned long countdownTime _INIT(1514764800L);
WLED_GLOBAL bool countdownOverTriggered _INIT(true);

// timer
WLED_GLOBAL byte lastTimerMinute _INIT(0);
WLED_GLOBAL byte timerHours[] _INIT_N(({ 0, 0, 0, 0, 0, 0, 0, 0 }));
WLED_GLOBAL byte timerMinutes[] _INIT_N(({ 0, 0, 0, 0, 0, 0, 0, 0 }));
WLED_GLOBAL byte timerMacro[] _INIT_N(({ 0, 0, 0, 0, 0, 0, 0, 0 }));
WLED_GLOBAL byte timerWeekday[] _INIT_N(({ 255, 255, 255, 255, 255, 255, 255, 255 }));        // weekdays to activate on
// bit pattern of arr elem: 0b11111111: sun,sat,fri,thu,wed,tue,mon,validity

// blynk
WLED_GLOBAL bool blynkEnabled _INIT(false);

// preset cycling
WLED_GLOBAL bool presetCyclingEnabled _INIT(false);
WLED_GLOBAL byte presetCycleMin _INIT(1), presetCycleMax _INIT(5);
WLED_GLOBAL uint16_t presetCycleTime _INIT(12);
WLED_GLOBAL unsigned long presetCycledTime _INIT(0);
WLED_GLOBAL byte presetCycCurr _INIT(presetCycleMin);
WLED_GLOBAL bool saveCurrPresetCycConf _INIT(false);

WLED_GLOBAL int16_t currentPlaylist _INIT(0);

// realtime
WLED_GLOBAL byte realtimeMode _INIT(REALTIME_MODE_INACTIVE);
WLED_GLOBAL byte realtimeOverride _INIT(REALTIME_OVERRIDE_NONE);
WLED_GLOBAL IPAddress realtimeIP _INIT((0, 0, 0, 0));
WLED_GLOBAL unsigned long realtimeTimeout _INIT(0);
WLED_GLOBAL uint8_t tpmPacketCount _INIT(0);
WLED_GLOBAL uint16_t tpmPayloadFrameSize _INIT(0);

// mqtt
WLED_GLOBAL long lastMqttReconnectAttempt _INIT(0);
WLED_GLOBAL long lastInterfaceUpdate _INIT(0);
WLED_GLOBAL byte interfaceUpdateCallMode _INIT(NOTIFIER_CALL_MODE_INIT);
WLED_GLOBAL char mqttStatusTopic[40] _INIT("");        // this must be global because of async handlers

#if AUXPIN >= 0
  // auxiliary debug pin
  WLED_GLOBAL byte auxTime _INIT(0);
  WLED_GLOBAL unsigned long auxStartTime _INIT(0);
  WLED_GLOBAL bool auxActive _INIT(false, auxActiveBefore _INIT(false);
#endif

// alexa udp
WLED_GLOBAL String escapedMac;
#ifndef WLED_DISABLE_ALEXA
  WLED_GLOBAL Espalexa espalexa;
  WLED_GLOBAL EspalexaDevice* espalexaDevice;
#endif

// dns server
WLED_GLOBAL DNSServer dnsServer;

// network time
WLED_GLOBAL bool ntpConnected _INIT(false);
WLED_GLOBAL time_t localTime _INIT(0);
WLED_GLOBAL unsigned long ntpLastSyncTime _INIT(999000000L);
WLED_GLOBAL unsigned long ntpPacketSentTime _INIT(999000000L);
WLED_GLOBAL IPAddress ntpServerIP;
WLED_GLOBAL uint16_t ntpLocalPort _INIT(2390);
WLED_GLOBAL uint16_t rolloverMillis _INIT(0);

// Temp buffer
WLED_GLOBAL char* obuf;
WLED_GLOBAL uint16_t olen _INIT(0);

// General filesystem
WLED_GLOBAL size_t fsBytesUsed _INIT(0);
WLED_GLOBAL size_t fsBytesTotal _INIT(0);
WLED_GLOBAL unsigned long presetsModifiedTime _INIT(0L);
WLED_GLOBAL JsonDocument* fileDoc;
WLED_GLOBAL bool doCloseFile _INIT(false);

// presets
WLED_GLOBAL uint16_t savedPresets _INIT(0);
WLED_GLOBAL int16_t currentPreset _INIT(-1);
WLED_GLOBAL bool isPreset _INIT(false);

WLED_GLOBAL byte errorFlag _INIT(0);

WLED_GLOBAL String messageHead, messageSub;
WLED_GLOBAL byte optionType;

WLED_GLOBAL bool doReboot _INIT(false);        // flag to initiate reboot from async handlers
WLED_GLOBAL bool doPublishMqtt _INIT(false);

// server library objects
WLED_GLOBAL AsyncWebServer server _INIT_N(((80)));
#ifdef WLED_ENABLE_WEBSOCKETS
WLED_GLOBAL AsyncWebSocket ws _INIT_N((("/ws")));
#endif
WLED_GLOBAL AsyncClient* hueClient _INIT(NULL);
WLED_GLOBAL AsyncMqttClient* mqtt _INIT(NULL);

// udp interface objects
WLED_GLOBAL WiFiUDP notifierUdp, rgbUdp, notifier2Udp;
WLED_GLOBAL WiFiUDP ntpUdp;
WLED_GLOBAL ESPAsyncE131 e131 _INIT_N(((handleE131Packet)));
WLED_GLOBAL bool e131NewData _INIT(false);

// led fx library object
WLED_GLOBAL WS2812FX strip _INIT(WS2812FX());

// Usermod manager
WLED_GLOBAL UsermodManager usermods _INIT(UsermodManager());

WLED_GLOBAL PinManagerClass pinManager _INIT(PinManagerClass());

// Status LED
#if STATUSLED && STATUSLED != LEDPIN
  WLED_GLOBAL unsigned long ledStatusLastMillis _INIT(0);
  WLED_GLOBAL unsigned short ledStatusType _INIT(0); // current status type - corresponds to number of blinks per second
  WLED_GLOBAL bool ledStatusState _INIT(0); // the current LED state
#endif

// enable additional debug output
#ifdef WLED_DEBUG
  #ifndef ESP8266
  #include <rom/rtc.h>
  #endif
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(x...) Serial.printf(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(x)
#endif

#ifdef WLED_DEBUG_FS
  #define DEBUGFS_PRINT(x) Serial.print(x)
  #define DEBUGFS_PRINTLN(x) Serial.println(x)
  #define DEBUGFS_PRINTF(x...) Serial.printf(x)
#else
  #define DEBUGFS_PRINT(x)
  #define DEBUGFS_PRINTLN(x)
  #define DEBUGFS_PRINTF(x...)
#endif

// debug macro variable definitions
#ifdef WLED_DEBUG
  WLED_GLOBAL unsigned long debugTime _INIT(0);
  WLED_GLOBAL int lastWifiState _INIT(3);
  WLED_GLOBAL unsigned long wifiStateChangedTime _INIT(0);
  WLED_GLOBAL int loops _INIT(0);
#endif

#ifdef ARDUINO_ARCH_ESP32
  #define WLED_CONNECTED (WiFi.status() == WL_CONNECTED || ETH.localIP()[0] != 0)
#else
  #define WLED_CONNECTED (WiFi.status() == WL_CONNECTED)
#endif
#define WLED_WIFI_CONFIGURED (strlen(clientSSID) >= 1 && strcmp(clientSSID, DEFAULT_CLIENT_SSID) != 0)
#define WLED_MQTT_CONNECTED (mqtt != nullptr && mqtt->connected())

// append new c string to temp buffer efficiently
bool oappend(const char* txt);
// append new number to temp buffer efficiently
bool oappendi(int i);

class WLED {
public:
  WLED();
  static WLED& instance()
  {
    static WLED instance;
    return instance;
  }

  // boot starts here
  void setup();

  void loop();
  void reset();

  void beginStrip();
  void handleConnection();
  void initAP(bool resetAP = false);
  void initConnection();
  void initInterfaces();
  void handleStatusLED();
};
#endif        // WLED_H
