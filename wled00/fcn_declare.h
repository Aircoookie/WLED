#ifndef WLED_FCN_DECLARE_H
#define WLED_FCN_DECLARE_H
#include <Arduino.h>
#include "src/dependencies/espalexa/EspalexaDevice.h"
#include "src/dependencies/e131/ESPAsyncE131.h"

/*
 * All globally accessible functions are declared here
 */

//alexa.cpp
void onAlexaChange(EspalexaDevice* dev);
void alexaInit();
void handleAlexa();
void onAlexaChange(EspalexaDevice* dev);

//blynk.cpp
void initBlynk(const char* auth, const char* host, uint16_t port);
void handleBlynk();
void updateBlynk();

//button.cpp
void shortPressAction();
bool isButtonPressed();
void handleButton();
void handleIO();

//cfg.cpp
void deserializeConfig();
bool deserializeConfigSec();
void serializeConfig();
void serializeConfigSec();

//colors.cpp
void colorFromUint32(uint32_t in, bool secondary = false);
void colorFromUint24(uint32_t in, bool secondary = false);
uint32_t colorFromRgbw(byte* rgbw);
void relativeChangeWhite(int8_t amount, byte lowerBoundary = 0);
void colorHStoRGB(uint16_t hue, byte sat, byte* rgb); //hue, sat to rgb
void colorKtoRGB(uint16_t kelvin, byte* rgb);
void colorCTtoRGB(uint16_t mired, byte* rgb); //white spectrum to rgb

void colorXYtoRGB(float x, float y, byte* rgb); // only defined if huesync disabled TODO
void colorRGBtoXY(byte* rgb, float* xy); // only defined if huesync disabled TODO

void colorFromDecOrHexString(byte* rgb, char* in);
bool colorFromHexString(byte* rgb, const char* in);
void colorRGBtoRGBW(byte* rgb); //rgb to rgbw (http://codewelt.com/rgbw). (RGBW_MODE_LEGACY)

//dmx.cpp
void initDMX();
void handleDMX();

//e131.cpp
void handleE131Packet(e131_packet_t* p, IPAddress clientIP, byte protocol);

//file.cpp
bool handleFileRead(AsyncWebServerRequest*, String path);
bool writeObjectToFileUsingId(const char* file, uint16_t id, JsonDocument* content);
bool writeObjectToFile(const char* file, const char* key, JsonDocument* content);
bool readObjectFromFileUsingId(const char* file, uint16_t id, JsonDocument* dest);
bool readObjectFromFile(const char* file, const char* key, JsonDocument* dest);
void updateFSInfo();
void closeFile();

//hue.cpp
void handleHue();
void reconnectHue();
void onHueError(void* arg, AsyncClient* client, int8_t error);
void onHueConnect(void* arg, AsyncClient* client);
void sendHuePoll();
void onHueData(void* arg, AsyncClient* client, void *data, size_t len);

//ir.cpp
bool decodeIRCustom(uint32_t code);
void applyRepeatActions();
void relativeChange(byte* property, int8_t amount, byte lowerBoundary = 0, byte higherBoundary = 0xFF);
void changeEffectSpeed(int8_t amount);
void changeBrightness(int8_t amount);
void changeEffectIntensity(int8_t amount);
void decodeIR(uint32_t code);
void decodeIR24(uint32_t code);
void decodeIR24OLD(uint32_t code);
void decodeIR24CT(uint32_t code);
void decodeIR40(uint32_t code);
void decodeIR44(uint32_t code);
void decodeIR21(uint32_t code);
void decodeIR6(uint32_t code);
void decodeIR9(uint32_t code);

void initIR();
void handleIR();

//json.cpp
#include "ESPAsyncWebServer.h"
#include "src/dependencies/json/ArduinoJson-v6.h"
#include "src/dependencies/json/AsyncJson-v6.h"
#include "FX.h"

void deserializeSegment(JsonObject elem, byte it);
bool deserializeState(JsonObject root);
void serializeSegment(JsonObject& root, WS2812FX::Segment& seg, byte id, bool forPreset = false, bool segmentBounds = true);
void serializeState(JsonObject root, bool forPreset = false, bool includeBri = true, bool segmentBounds = true);
void serializeInfo(JsonObject root);
void serveJson(AsyncWebServerRequest* request);
bool serveLiveLeds(AsyncWebServerRequest* request, uint32_t wsClient = 0);

//led.cpp
void setValuesFromMainSeg();
void resetTimebase();
void toggleOnOff();
void setAllLeds();
void setLedsStandard(bool justColors = false);
bool colorChanged();
void colorUpdated(int callMode);
void updateInterfaces(uint8_t callMode);
void handleTransitions();
void handleNightlight();
byte scaledBri(byte in);

//lx_parser.cpp
bool parseLx(int lxValue, byte* rgbw);
void parseLxJson(int lxValue, byte segId, bool secondary);

//mqtt.cpp
bool initMqtt();
void publishMqtt();

//ntp.cpp
void handleNetworkTime();
void sendNTPPacket();
bool checkNTPResponse();    
void updateLocalTime();
void getTimeString(char* out);
bool checkCountdown();
void setCountdown();
byte weekdayMondayFirst();
void checkTimers();

//overlay.cpp
void initCronixie();
void handleOverlays();
void handleOverlayDraw();
void _overlayAnalogCountdown();
void _overlayAnalogClock();

byte getSameCodeLength(char code, int index, char const cronixieDisplay[]);
void setCronixie();
void _overlayCronixie();    
void _drawOverlayCronixie();

//pin_manager.cpp
class PinManagerClass {
  private:
  #ifdef ESP8266
  uint8_t pinAlloc[3] = {0x00, 0x00, 0x00}; //24bit, 1 bit per pin, we use first 17bits
  #else
  uint8_t pinAlloc[5] = {0x00, 0x00, 0x00, 0x00, 0x00}; //40bit, 1 bit per pin, we use all bits
  #endif

  public:
  void deallocatePin(byte gpio);
  bool allocatePin(byte gpio, bool output = true);
  bool isPinAllocated(byte gpio);
  bool isPinOk(byte gpio, bool output = true);
};

//playlist.cpp
void loadPlaylist(JsonObject playlistObject);
void handlePlaylist();

//presets.cpp
bool applyPreset(byte index);
void savePreset(byte index, bool persist = true, const char* pname = nullptr, JsonObject saveobj = JsonObject());
void deletePreset(byte index);

//set.cpp
void _setRandomColor(bool _sec,bool fromButton=false);
bool isAsterisksOnly(const char* str, byte maxLen);
void handleSettingsSet(AsyncWebServerRequest *request, byte subPage);
bool handleSet(AsyncWebServerRequest *request, const String& req, bool apply=true);
int getNumVal(const String* req, uint16_t pos);
bool updateVal(const String* req, const char* key, byte* val, byte minv=0, byte maxv=255);

//udp.cpp
void notify(byte callMode, bool followUp=false);
void realtimeLock(uint32_t timeoutMs, byte md = REALTIME_MODE_GENERIC);
void handleNotifications();
void setRealtimePixel(uint16_t i, byte r, byte g, byte b, byte w);

//um_manager.cpp
class Usermod {
  public:
    virtual void loop() {}
    virtual void setup() {}
    virtual void connected() {}
    virtual void addToJsonState(JsonObject& obj) {}
    virtual void addToJsonInfo(JsonObject& obj) {}
    virtual void readFromJsonState(JsonObject& obj) {}
    virtual void addToConfig(JsonObject& obj) {}
    virtual void readFromConfig(JsonObject& obj) {}
    virtual uint16_t getId() {return USERMOD_ID_UNSPECIFIED;}
};

class UsermodManager {
  private:
    Usermod* ums[WLED_MAX_USERMODS];
    byte numMods = 0;

  public:
    void loop();

    void setup();
    void connected();

    void addToJsonState(JsonObject& obj);
    void addToJsonInfo(JsonObject& obj);
    void readFromJsonState(JsonObject& obj);

    void addToConfig(JsonObject& obj);
    void readFromConfig(JsonObject& obj);

    bool add(Usermod* um);
    byte getModCount();
};

//usermods_list.cpp
void registerUsermods();

//usermod.cpp
void userSetup();
void userConnected();
void userLoop();

//wled_eeprom.cpp
void applyMacro(byte index);
void deEEP();
void deEEPSettings();
void clearEEPROM();

//wled_serial.cpp
void handleSerial();

//wled_server.cpp
bool isIp(String str);
bool captivePortal(AsyncWebServerRequest *request);
void initServer();
void serveIndexOrWelcome(AsyncWebServerRequest *request);
void serveIndex(AsyncWebServerRequest* request);
String msgProcessor(const String& var);
void serveMessage(AsyncWebServerRequest* request, uint16_t code, const String& headl, const String& subl="", byte optionT=255);
String settingsProcessor(const String& var);
String dmxProcessor(const String& var);
void serveSettings(AsyncWebServerRequest* request, bool post = false);

//ws.cpp
void handleWs();
void wsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
void sendDataWs(AsyncWebSocketClient * client = nullptr);

//xml.cpp
void XML_response(AsyncWebServerRequest *request, char* dest = nullptr);
void URL_response(AsyncWebServerRequest *request);
void sappend(char stype, const char* key, int val);
void sappends(char stype, const char* key, char* val);
void getSettingsJS(byte subPage, char* dest);

#endif
