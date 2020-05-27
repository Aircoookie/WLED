
/*
 * This file allows you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * EEPROM bytes 2750+ are reserved for your custom use case. (if you extend #define EEPSIZE in const.h)
 * bytes 2400+ are currently ununsed, but might be used for future wled features
 */

//Use userVar0 and userVar1 (API calls &U0=,&U1=, uint16_t)

#include "wled.h"
#include <TFT_eSPI.h>
#include <SPI.h>
#include "WiFi.h"
#include <Wire.h>

#ifndef TFT_DISPOFF
#define TFT_DISPOFF 0x28
#endif

#ifndef TFT_SLPIN
#define TFT_SLPIN   0x10
#endif

#define TFT_MOSI            19
#define TFT_SCLK            18
#define TFT_CS              5
#define TFT_DC              16
#define TFT_RST             23

#define TFT_BL          4  // Display backlight control pin
#define ADC_EN          14
#define ADC_PIN         34
#define BUTTON_1        35
#define BUTTON_2        0

TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library

//gets called once at boot. Do all initialization that doesn't depend on network here
void userSetup() {
    Serial.begin(115200);
    Serial.println("Start");
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(0, 0);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(2);
    tft.print("Loading...");

    if (TFT_BL > 0) { // TFT_BL has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
         pinMode(TFT_BL, OUTPUT); // Set backlight pin to output mode
         digitalWrite(TFT_BL, HIGH); // Turn backlight on. 
    }

    tft.setRotation(1);
}

// gets called every time WiFi is (re-)connected. Initialize own network
// interfaces here
void userConnected() {}

// needRedraw marks if redraw is required to prevent often redrawing.
bool needRedraw = true;

// Next variables hold the previous known values to determine if redraw is
// required.
String knownSsid = "";
IPAddress knownIp;
uint8_t knownBrightness = 0;
uint8_t knownMode = 0;
uint8_t knownPalette = 0;

long lastUpdate = 0;
long lastRedraw = 0;
bool displayTurnedOff = false;
// How often we are redrawing screen
#define USER_LOOP_REFRESH_RATE_MS 5000

void userLoop() {

  // Check if we time interval for redrawing passes.
  if (millis() - lastUpdate < USER_LOOP_REFRESH_RATE_MS) {
    return;
  }
  lastUpdate = millis();
  
  // Turn off display after 3 minutes with no change.
   if(!displayTurnedOff && millis() - lastRedraw > 3*60*1000) {
    digitalWrite(TFT_BL, LOW); // Turn backlight off. 
    displayTurnedOff = true;
  } 

  // Check if values which are shown on display changed from the last time.
  if (((apActive) ? String(apSSID) : WiFi.SSID()) != knownSsid) {
    needRedraw = true;
  } else if (knownIp != (apActive ? IPAddress(4, 3, 2, 1) : WiFi.localIP())) {
    needRedraw = true;
  } else if (knownBrightness != bri) {
    needRedraw = true;
  } else if (knownMode != strip.getMode()) {
    needRedraw = true;
  } else if (knownPalette != strip.getSegment(0).palette) {
    needRedraw = true;
  }

  if (!needRedraw) {
    return;
  }
  needRedraw = false;
  
  if (displayTurnedOff)
  {
    digitalWrite(TFT_BL, TFT_BACKLIGHT_ON); // Turn backlight on.
    displayTurnedOff = false;
  }
  lastRedraw = millis();

  // Update last known values.
  #if defined(ESP8266)
  knownSsid = apActive ? WiFi.softAPSSID() : WiFi.SSID();
  #else
  knownSsid = WiFi.SSID();
  #endif
  knownIp = apActive ? IPAddress(4, 3, 2, 1) : WiFi.localIP();
  knownBrightness = bri;
  knownMode = strip.getMode();
  knownPalette = strip.getSegment(0).palette;

  tft.fillScreen(TFT_BLACK);
  //u8x8.setFont(u8x8_font_chroma48medium8_r);
  tft.setTextSize(2);
  // First row with Wifi name
  //u8x8.setCursor(1, 0);
  tft.setCursor(1, 0);
  //tft.print(knownSsid.substring(0, tft.width() > 1 ? tft.width() - 2 : 0));
  tft.print(knownSsid.substring(0, 25 > 1 ? 25 : 0));
  //u8x8.print(knownSsid.substring(0, u8x8.getCols() > 1 ? u8x8.getCols() - 2 : 0));
  // Print `~` char to indicate that SSID is longer, than owr dicplay
  if (knownSsid.length() > 25)
  //if (knownSsid.length() > tft.width())
    tft.print("~");

  // Second row with IP or Psssword
  tft.setCursor(1, 30);
  // Print password in AP mode and if led is OFF.
  if (apActive && bri == 0)
    tft.print(apPass);
  else
    tft.print(knownIp);

  // Third row with mode name
  tft.setCursor(1, 60);
  uint8_t qComma = 0;
  bool insideQuotes = false;
  uint8_t printedChars = 0;
  char singleJsonSymbol;
  // Find the mode name in JSON
  for (size_t i = 0; i < strlen_P(JSON_mode_names); i++) {
    singleJsonSymbol = pgm_read_byte_near(JSON_mode_names + i);
    switch (singleJsonSymbol) {
    case '"':
      insideQuotes = !insideQuotes;
      break;
    case '[':
    case ']':
      break;
    case ',':
      qComma++;
    default:
      if (!insideQuotes || (qComma != knownMode))
        break;
      tft.print(singleJsonSymbol);
      printedChars++;
    }
    if ((qComma > knownMode) || (printedChars > 25 - 2))
      break;
  }
  // Fourth row with palette name
  tft.setCursor(1, 90);
  qComma = 0;
  insideQuotes = false;
  printedChars = 0;
  // Looking for palette name in JSON.
  for (size_t i = 0; i < strlen_P(JSON_palette_names); i++) {
    singleJsonSymbol = pgm_read_byte_near(JSON_palette_names + i);
    switch (singleJsonSymbol) {
    case '"':
      insideQuotes = !insideQuotes;
      break;
    case '[':
    case ']':
      break;
    case ',':
      qComma++;
    default:
      if (!insideQuotes || (qComma != knownPalette))
        break;
      tft.print(singleJsonSymbol);
      printedChars++;
    }
    if ((qComma > knownPalette) || (printedChars > 25 - 2))
      break;
  }


}