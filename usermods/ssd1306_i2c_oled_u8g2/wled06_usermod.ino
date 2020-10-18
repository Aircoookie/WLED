#include <U8x8lib.h> // from https://github.com/olikraus/u8g2/

//The SCL and SDA pins are defined here. 
//Lolin32 boards use SCL=5 SDA=4 
#define U8X8_PIN_SCL 5
#define U8X8_PIN_SDA 4


// If display does not work or looks corrupted check the
// constructor reference:
// https://github.com/olikraus/u8g2/wiki/u8x8setupcpp
// or check the gallery:
// https://github.com/olikraus/u8g2/wiki/gallery
U8X8_SSD1306_128X32_UNIVISION_HW_I2C u8x8(U8X8_PIN_NONE, U8X8_PIN_SCL,
                                          U8X8_PIN_SDA); // Pins are Reset, SCL, SDA

// gets called once at boot. Do all initialization that doesn't depend on
// network here
void userSetup() {
  u8x8.begin();
  u8x8.setPowerSave(0);
    u8x8.setContrast(10); //Contrast setup will help to preserve OLED lifetime. In case OLED need to be brighter increase number up to 255
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.drawString(0, 0, "Loading...");
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
    u8x8.setPowerSave(1);
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
    u8x8.setPowerSave(0);
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

  u8x8.clear();
  u8x8.setFont(u8x8_font_chroma48medium8_r);

  // First row with Wifi name
  u8x8.setCursor(1, 0);
  u8x8.print(knownSsid.substring(0, u8x8.getCols() > 1 ? u8x8.getCols() - 2 : 0));
  // Print `~` char to indicate that SSID is longer, than owr dicplay
  if (knownSsid.length() > u8x8.getCols())
    u8x8.print("~");

  // Second row with IP or Psssword
  u8x8.setCursor(1, 1);
  // Print password in AP mode and if led is OFF.
  if (apActive && bri == 0)
    u8x8.print(apPass);
  else
    u8x8.print(knownIp);

  // Third row with mode name
  u8x8.setCursor(2, 2);
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
      u8x8.print(singleJsonSymbol);
      printedChars++;
    }
    if ((qComma > knownMode) || (printedChars > u8x8.getCols() - 2))
      break;
  }
  // Fourth row with palette name
  u8x8.setCursor(2, 3);
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
      u8x8.print(singleJsonSymbol);
      printedChars++;
    }
    if ((qComma > knownMode) || (printedChars > u8x8.getCols() - 2))
      break;
  }

  u8x8.setFont(u8x8_font_open_iconic_embedded_1x1);
  u8x8.drawGlyph(0, 0, 80); // wifi icon
  u8x8.drawGlyph(0, 1, 68); // home icon
  u8x8.setFont(u8x8_font_open_iconic_weather_2x2);
  u8x8.drawGlyph(0, 2, 66 + (bri > 0 ? 3 : 0)); // sun/moon icon
}
