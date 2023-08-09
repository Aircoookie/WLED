/*
REQUIRED SETUP

TFT_eSPI Library Adjustments (board selection)
You need to modify a file in the 'TFT_eSPI' library to select the correct board.  
Locate the 'User_Setup_Select.h' file can be found in the '/.pio/libdeps/YOUR-BOARD/TFT_eSPI_ID1559' folder.

Modify the 'User_Setup_Select.h'
Comment out the following line
//#include <User_Setup.h> // Default setup is root library folder

Uncomment the following line
#include <User_Setups/Setup25_TTGO_T_Display.h> // Setup file for ESP32 and TTGO T-Display ST7789V SPI bus TFT
*/

#pragma once

#include "wled.h"
#include <TFT_eSPI.h>
#include <SPI.h>
#include "WiFi.h"
#include <Wire.h>

#define PRESET_NAME_BUFFER_SIZE 25

#define TFT_MOSI  19
#define TFT_SCLK  18
#define TFT_CS    5
#define TFT_DC    16
#define TFT_RST   23
#define TFT_BL    4   // Display backlight control pin
#define ADC_EN    14  // Used for enabling battery voltage measurements - not used in this program
#define TFT_CH    6

TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library

class UsermodBackgroundTDisplay : public Usermod {

  private:
    #ifdef TFT_BRIGHTNESS
      uint16_t tftBrightness = TFT_BRIGHTNESS;
    #else
      // 0=OFF; 255=MAX
      uint16_t tftBrightness = 50;
    #endif

    #ifdef TFT_TIMEOUT
      uint16_t tftTimeout = TFT_TIMEOUT;
    #else
      uint16_t tftTimeout = 10000;
    #endif

    bool initDone = false;
    bool needRedraw = true;
  
    // TTGO T-Display
    String knownSsid = "";
    IPAddress knownIp;
    uint8_t knownBrightness = 0;
    uint8_t knownMode = 0;
    uint8_t knownPalette = 0;
    uint8_t tftcharwidth = 19;  // Number of chars that fit on screen with text size set to 2
    unsigned long tftNextTimeout = 0;

    // strings to reduce flash memory usage (used more than twice)
    static const char _strTag[];
    static const char _strBrightness[];
    static const char _strName[];
    static const char _strTimeout[];
 
  public:
    
    void setBrightness(uint32_t newBrightness) {
      ledcWrite(TFT_CH, newBrightness); // 0-15, 0-255 (with 8 bit resolution);  0=totally dark;255=totally shiny
    }

    void initDisplay(int rotation=3) {
      pinMode(TFT_BL, OUTPUT);
      ledcSetup(TFT_CH, 5000, 8); // 0-15, 5000, 8
      ledcAttachPin(TFT_BL, TFT_CH); // TFT_BL, 0 - 15

      tft.init();
      tft.setRotation(rotation);  //Rotation here is set up for the text to be readable with the port on the left. Use 1 to flip.
      tft.fillScreen(TFT_BLACK);
      tft.setTextSize(2);
      tft.setTextColor(TFT_WHITE);
      tft.setCursor(1, 10);
      tft.setTextDatum(MC_DATUM);
      tft.setTextSize(2);
      tft.print("Init...");
    }
    
    void updateDisplay(){
      // Check if values which are shown on display changed from the last time.
      if (((apActive) ? String(apSSID) : WiFi.SSID()) != knownSsid) {
        needRedraw = true;
      } else if (knownIp != (apActive ? IPAddress(4, 3, 2, 1) : WiFi.localIP())) {
        needRedraw = true;
      } else if (knownBrightness != bri) {
        needRedraw = true;
      } else if (knownMode != strip.getMainSegment().mode) {
        needRedraw = true;
      } else if (knownPalette != strip.getMainSegment().palette) {
        needRedraw = true;
      }

      if(tftNextTimeout < millis())
        setBrightness(0);

      if (!needRedraw) return;
            
      knownSsid = WiFi.SSID();
      knownIp = apActive ? IPAddress(4, 3, 2, 1) : WiFi.localIP();
      knownBrightness = bri;
      knownMode = strip.getMainSegment().mode;
      knownPalette = strip.getMainSegment().palette;

      tft.fillScreen(TFT_BLACK);
      tft.setTextSize(2);
      tft.setTextColor(TFT_YELLOW);

      // First row
      tft.setCursor(1, 1);
      tft.print(apSSID);
      //tft.print(knownSsid.substring(0, tftcharwidth > 1 ? tftcharwidth - 1 : 0));
      //if (knownSsid.length() > tftcharwidth)
      //  tft.print("~");

      // Second row
      tft.setTextSize(2);
      tft.setCursor(1, 24);
      tft.setTextColor(TFT_GREENYELLOW);

      if (apActive) {
        tft.print("AP IP: ");
        tft.print(knownIp);
        tft.setCursor(1,46);
        tft.print("AP Pass:");
        tft.print(apPass);
      }
      else {
        tft.print("IP: ");
        tft.print(knownIp);
        tft.setCursor(1,46);
        tft.setTextColor(TFT_GREEN);
        tft.print("Bright: ");
        tft.print(((float(bri)/255)*100));
        tft.print("%");
      }

      // Third row
      tft.setCursor(1, 68);
      tft.setTextColor(TFT_SKYBLUE);
      char lineBuffer[tftcharwidth+1];
      extractModeName(knownMode, JSON_mode_names, lineBuffer, tftcharwidth);
      tft.print(lineBuffer);

      // Fourth row
      tft.setCursor(1, 90);
      tft.setTextColor(TFT_BLUE);
      tft.print(strip.currentMilliamps);
      tft.print("mA est.");

      // Fifth row
      tft.setCursor(1, 112);
      tft.setTextColor(TFT_VIOLET);
      extractModeName(knownPalette, JSON_palette_names, lineBuffer, tftcharwidth);
      tft.print(lineBuffer);


      needRedraw = false;
      tftNextTimeout = tftTimeout + millis();
      setBrightness(tftBrightness);
    }

    void setup() {
      Serial.println("Setup - UsermodBackgroundTDisplay");
      
      initDisplay();
      setBrightness(tftBrightness); 
      needRedraw = true;
      initDone = true;
    }

    // gets called every time WiFi is (re-)connected. Initialize own network interfaces here
    void connected() {
      needRedraw = true;
    }

    void doWork(){
      if(needRedraw)
        updateDisplay();
      
      EVERY_N_MILLISECONDS(5000){ updateDisplay(); }
    }

    void backgroundLoop(){
      // Calling doWork on the background thread
      // Only call it from one thread
      doWork();
    }

    void loop(){
      // Calling doWork on the main thread
      // Only call it from one thread
      // doWork();
    }

    // fired upon WLED state change
    void onStateChange(uint8_t mode) {
      needRedraw = true;
    }    
    
    // Add JSON entries that go to cfg.json
    void addToConfig(JsonObject &root)
    {
      JsonObject top = root.createNestedObject(FPSTR(_strTag));
      top[FPSTR(_strBrightness)] = tftBrightness;
      top[FPSTR(_strTimeout)] = tftTimeout;
    }

    // Read JSON entries that go to cfg.json
    bool readFromConfig(JsonObject &root)
    {
      JsonObject top = root[FPSTR(_strTag)];
      
      if (top.isNull()) {
        DEBUG_PRINTLN(FPSTR(_strTag));
        return false;
      }

      bool configComplete = !top.isNull();
      configComplete &= getJsonValue(top[FPSTR(_strBrightness)], tftBrightness);
      configComplete &= getJsonValue(top[FPSTR(_strTimeout)], tftTimeout);

      return configComplete;
    }

    /*
     * getId() allows you to optionally give your V2 usermod an unique ID.
     */
    uint16_t getId() {
      return USERMOD_ID_TDISPLAY;
    }
};

// strings to reduce flash memory usage (used more than twice)
const char UsermodBackgroundTDisplay::_strTag[] PROGMEM = "tdesp32";
const char UsermodBackgroundTDisplay::_strBrightness[] PROGMEM = "TFT Brightness";
const char UsermodBackgroundTDisplay::_strTimeout[] PROGMEM = "TFT Timeout";