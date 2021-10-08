#pragma once

#include "wled.h"
#include <U8x8lib.h> // from https://github.com/olikraus/u8g2/

//
// Insired by the usermod_v2_four_line_display
//
// v2 usermod for using 128x32 or 128x64 i2c
// OLED displays to provide a four line display
// for WLED.
//
// Dependencies
// * This usermod REQURES the ModeSortUsermod
// * This Usermod works best, by far, when coupled 
//   with RotaryEncoderUIUsermod.
//
// Make sure to enable NTP and set your time zone in WLED Config | Time.
//
// REQUIREMENT: You must add the following requirements to
// REQUIREMENT: "lib_deps" within platformio.ini / platformio_override.ini
// REQUIREMENT: *  U8g2  (the version already in platformio.ini is fine)
// REQUIREMENT: *  Wire
//

//The SCL and SDA pins are defined here. 
#ifdef ARDUINO_ARCH_ESP32
  #ifndef FLD_PIN_SCL
    #define FLD_PIN_SCL 22
  #endif
  #ifndef FLD_PIN_SDA
    #define FLD_PIN_SDA 21
  #endif
  #ifndef FLD_PIN_CLOCKSPI
    #define FLD_PIN_CLOCKSPI 18
  #endif
   #ifndef FLD_PIN_DATASPI
    #define FLD_PIN_DATASPI 23
  #endif   
  #ifndef FLD_PIN_DC
    #define FLD_PIN_DC 19
  #endif
  #ifndef FLD_PIN_CS
    #define FLD_PIN_CS 5
  #endif
  #ifndef FLD_PIN_RESET
    #define FLD_PIN_RESET 26
  #endif
#else
  #ifndef FLD_PIN_SCL
    #define FLD_PIN_SCL 5
  #endif
  #ifndef FLD_PIN_SDA
    #define FLD_PIN_SDA 4
  #endif
  #ifndef FLD_PIN_CLOCKSPI
    #define FLD_PIN_CLOCKSPI 14
  #endif
   #ifndef FLD_PIN_DATASPI
    #define FLD_PIN_DATASPI 13
  #endif   
  #ifndef FLD_PIN_DC
    #define FLD_PIN_DC 12
  #endif
    #ifndef FLD_PIN_CS
    #define FLD_PIN_CS 15
  #endif
  #ifndef FLD_PIN_RESET
    #define FLD_PIN_RESET 16
  #endif
#endif

// When to time out to the clock or blank the screen
// if SLEEP_MODE_ENABLED.
#define SCREEN_TIMEOUT_MS  60*1000    // 1 min

#define TIME_INDENT        0
#define DATE_INDENT        2

// Minimum time between redrawing screen in ms
#define USER_LOOP_REFRESH_RATE_MS 100

// Extra char (+1) for null
#define LINE_BUFFER_SIZE            16+1
#define MAX_JSON_CHARS              19+1
#define MAX_MODE_LINE_SPACE         13+1

typedef enum {
  NONE = 0,
  SSD1306,      // U8X8_SSD1306_128X32_UNIVISION_HW_I2C
  SH1106,       // U8X8_SH1106_128X64_WINSTAR_HW_I2C
  SSD1306_64,   // U8X8_SSD1306_128X64_NONAME_HW_I2C
  SSD1305,      // U8X8_SSD1305_128X32_ADAFRUIT_HW_I2C
  SSD1305_64,   // U8X8_SSD1305_128X64_ADAFRUIT_HW_I2C
  SSD1306_SPI,  // U8X8_SSD1306_128X32_NONAME_HW_SPI
  SSD1306_SPI64 // U8X8_SSD1306_128X64_NONAME_HW_SPI
} DisplayType;

/*
  Fontname: benji_custom_icons_1x
  Copyright:
  Glyphs: 1/1
  BBX Build Mode: 3
  * 4 = custom palette
*/
const uint8_t u8x8_font_benji_custom_icons_1x1[13] U8X8_FONT_SECTION("u8x8_font_benji_custom_icons_1x1") = 
 "\4\4\1\1<n\372\377\275\277\26\34";

/*
  Fontname: benji_custom_icons_2x
  Copyright: 
  Glyphs: 8/8
  BBX Build Mode: 3
  // all the icons uses are consolidated into a single library to simplify code
  // these are just the required icons stripped from the U8x8 libraries in addition to a few new custom icons
  * 1 = sun
  * 2 = skip forward
  * 3 = fire
  * 4 = custom palette
  * 5 = puzzle piece
  * 6 = moon
  * 7 = brush
  * 8 = custom saturation
*/
const uint8_t u8x8_font_benji_custom_icons_2x2[261] U8X8_FONT_SECTION("u8x8_font_benji_custom_icons_2x2") = 
  "\1\10\2\2\200\200\14\14\300\340\360\363\363\360\340\300\14\14\200\200\1\1\60\60\3\7\17\317\317\17\7\3"
  "\60\60\1\1\374\370\360\340\340\300\200\0\374\370\360\340\340\300\200\0\77\37\17\7\7\3\1\0\77\37\17\7"
  "\7\3\1\0\0\200\340\360\377\376\374\360\0\0\300\200\0\0\0\0\17\77\177\377\17\7\301\340\370\374\377\377"
  "\377|\0\0\360\370\234\236\376\363\363\377\377\363\363\376><\370\360\3\17\77yy\377\377\377\377\317\17\17"
  "\17\17\7\3\360\360\360\360\366\377\377\366\360\360\360\360\0\0\0\0\377\377\377\377\237\17\17\237\377\377\377\377"
  "\6\17\17\6\340\370\374\376\377\340\200\0\0\0\0\0\0\0\0\0\3\17\37\77\177\177\177\377\376|||"
  "\70\30\14\0\0\0\0\0\0\0\0``\360\370|<\36\7\2\0\300\360\376\377\177\77\36\0\1\1\0"
  "\0\0\0\0\200\200\14\14\300\340\360\363\363\360\340\300\14\14\200\200\1\1\60\60\3\4\10\310\310\10\4\3"
  "\60\60\1\1";

/*
  Fontname: benji_custom_icons_6x
  Copyright: 
  Glyphs: 8/8
  BBX Build Mode: 3
  // 6x6 icons libraries take up a lot of memory thus all the icons uses are consolidated into a single library
  // these are just the required icons stripped from the U8x8 libraries in addition to a few new custom icons
  * 1 = sun
  * 2 = skip forward
  * 3 = fire
  * 4 = custom palette
  * 5 = puzzle piece
  * 6 = moon
  * 7 = brush
  * 8 = custom saturation
*/
const uint8_t u8x8_font_benji_custom_icons_6x6[2308] U8X8_FONT_SECTION("u8x8_font_benji_custom_icons_6x6") = 
  "\1\10\6\6\0\0\0\0\0\0\200\300\300\300\300\200\0\0\0\0\0\0\0\0\0\36\77\77\77\77\36\0"
  "\0\0\0\0\0\0\0\0\200\300\300\300\300\200\0\0\0\0\0\0\0\0\0\0\0\0\7\17\17\17\17\7"
  "\0\0\0\0\200\300\340\340\340\360\360\360\360\360\360\340\340\340\300\200\0\0\0\0\7\17\17\17\17\7\0\0"
  "\0\0\0\0\300\340\340\340\340\300\0\0\0\0\0\0\340\374\376\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\376\374\340\0\0\0\0\0\0\300\340\340\340\340\300\3\7\7\7\7\3\0\0\0\0\0\0"
  "\7\77\177\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\177\77\7\0\0\0\0\0\0\3\7"
  "\7\7\7\3\0\0\0\0\0\0\340\360\360\360\360\340\0\0\0\0\1\3\7\7\7\17\17\17\17\17\17\7"
  "\7\7\3\1\0\0\0\0\340\360\360\360\360\340\0\0\0\0\0\0\0\0\0\0\0\0\1\3\3\3\3\1"
  "\0\0\0\0\0\0\0\0\0x\374\374\374\374x\0\0\0\0\0\0\0\0\0\1\3\3\3\3\1\0\0"
  "\0\0\0\0\300\200\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\300\200\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\377\377\377\376\376\374\370\360\360\340\300\200"
  "\200\0\0\0\0\0\0\0\0\0\0\0\377\377\377\376\376\374\370\360\360\340\300\200\200\0\0\0\0\0\0\0"
  "\0\0\0\0\377\377\377\377\377\377\377\377\377\377\377\377\377\377\376\374\374\370\360\340\340\300\200\0\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\376\374\374\370\360\340\340\300\200\0\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\177\77\77\37\17\7\7\3\1\0\377\377\377\377\377\377\377\377\377\377\377\377\377\377\177\77\77\37\17\7"
  "\7\3\1\0\377\377\377\177\177\77\37\17\17\7\3\1\1\0\0\0\0\0\0\0\0\0\0\0\377\377\377\177"
  "\177\77\37\17\17\7\3\1\1\0\0\0\0\0\0\0\0\0\0\0\3\1\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\3\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1\376\374\374\370\360\340\300\200\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\200\340\360\374"
  "\377\377\377\377\377\377\377\377\377\376\370\300\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\300\340\360\374\376\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\37\0\0\0\0"
  "\0\0\4\370\360\360\340\300\200\0\0\0\0\0\0\0\0\0\0\0\370\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\177\77\37\7\3\0\0\0\0\0\200\300\360\374\377\377\377\377\377\377\377\376\370\340\0\0\0"
  "\0\0\0\0\3\37\177\377\377\377\377\377\377\377\377\377\77\17\7\1\0\0\0\0\0\200\300\360\370\374\376\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\0\0\0\0\0\0\0\0\0\1\3\7\17\37\77\77\177\200"
  "\0\0\0\0\0\0\340\374\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\177\77\17\1\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\200\300\340\340\360\360\370|<>>>~\377\377\377\377\377\377\377\177"
  "\77\36\36\36\36<|\370\370\360\360\340\340\200\0\0\0\0\0\0\0\0\300\360\374\376\377\377\377\377\377\377"
  "\377\360\340\300\300\300\300\340\360\377\377\377\377\377\377\370\360\340\340\340\340\360\370\377\377\377\377\377\377\377\377\377"
  "\374\360\340\200\360\377\377\377\377\377\207\3\1\1\1\1\3\207\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\207\3\1\1\1\1\3\207\377\377\377\377\377\17\377\377\377\377\377\377\377\376~>>"
  "\77\77\177\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\376\376\376\376\377\377\377"
  "\177\77\37\7\0\0\3\17\77\177\377\377\360\340\300\300\300\300\340\360\377\377\377\377\377\377\377\377\377\377\77\17"
  "\17\7\7\7\7\7\7\7\7\7\3\3\3\3\1\0\0\0\0\0\0\0\0\0\0\0\0\1\3\7\17\37"
  "\37\77\77\177\177\177\377\377\377\377\377\377\377\377\377~\30\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\370\374\376\377\377\377\377\377\377\376\374\360\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\360\360\360\360\360\360\360\360\360\360\360\360"
  "\360\363\377\377\377\377\377\377\377\377\363\360\360\360\360\360\360\360\360\360\360\360\360\360\0\0\0\0\0\0\0\0"
  "\0\0\0\0\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\0\0\0\0\0\0\0\0\0\0\0\0\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\374\374\376\376\377\377\377\377"
  "\377\376\374\360\377\377\377\377\377\377\377\377\377\377\377\377\177\77\37\17\17\17\17\17\17\37\77\177\377\377\377\377"
  "\377\377\377\377\377\377\377\377\3\3\7\7\17\17\17\17\7\7\3\0\377\377\377\377\377\377\377\377\377\377\377\377"
  "\360\300\0\0\0\0\0\0\0\0\300\360\377\377\377\377\377\377\377\377\377\377\377\377\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\200\300\340\360\360\370\374\374\376\376\7\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\300\360\374\376\377\377\377\377\377\377\377"
  "\377\377\377\340\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\374\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\374\360\300\200\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\17\177\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\376\374\370\360\360\340\340\300\300\300\200\200\200\200\0\0\0\0\0\0\200\200"
  "\200\200\0\0\0\0\1\7\37\77\177\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\177\77\37\7\1\0\0\0\0\0\0\0\0\0\0\1\3\3\7"
  "\17\17\37\37\37\77\77\77\77\177\177\177\177\177\177\77\77\77\77\37\37\37\17\17\7\3\3\1\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\200\200\300\340\360\360\370\374\374\376\377~\34\10\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\200\300\300\340\360\360\370\374\376\376\377\377\377\377\377\377\177\77\17\7\3"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\4\6\17\17\37\77\177\377"
  "\377\377\377\377\377\377\77\37\7\3\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\300\370\374\376"
  "\376\377\377\377\377\377\377\376\376\374\370\340\0\0\0\0\3\17\7\3\1\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\200\360\377\377\377\377\377\377\377\377\377\377\377\377\377\377\177\17\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0`px\374\376\377\377\377\377\377\377"
  "\177\177\177\77\77\37\17\7\3\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\200\300\300\200\0\0\0\0\0\0\0\0\0\14\36\77\77\36\14\0\0"
  "\0\0\0\0\0\0\0\200\300\300\200\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\3\7\17\17\7\3"
  "\0\200\300\340\360\360\370\370\370\374\374\374\374\370\370\370\360\360\340\300\200\0\3\7\17\17\7\3\0\0\0\0"
  "\0\0\0\0\300\340\360\360\340\300\0\0\0\0\340\374\377\177\177\177\177\177\177\177\177\177\177\177\177\177\177\177"
  "\177\177\177\177\177\377\374\340\0\0\0\0\300\340\360\360\340\300\0\0\0\1\3\3\1\0\0\0\0\0\1\17"
  "\77\177\370\340\300\200\200\0\0\0\0\0\0\0\0\200\200\300\340\370\177\77\17\1\0\0\0\0\0\1\3\3"
  "\1\0\0\0\0\0\0\0\0\0\60x\374\374x\60\0\0\0\1\3\3\7\7\7\16\16\16\16\7\7\7"
  "\3\3\1\0\0\0\60x\374\374x\60\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\14\36\77\77\36\14\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0";

class FourLineDisplayUsermod : public Usermod {

  private:

    bool initDone = false;
    unsigned long lastTime = 0;

    // HW interface & configuration
    U8X8 *u8x8 = nullptr;           // pointer to U8X8 display object
    #ifndef FLD_SPI_DEFAULT
    int8_t ioPin[5] = {FLD_PIN_SCL, FLD_PIN_SDA, -1, -1, -1};        // I2C pins: SCL, SDA
    uint32_t ioFrequency = 400000;  // in Hz (minimum is 100000, baseline is 400000 and maximum should be 3400000)
    DisplayType type = SSD1306_64;     // display type
    #else
    int8_t ioPin[5] = {FLD_PIN_CLOCKSPI, FLD_PIN_DATASPI, FLD_PIN_CS, FLD_PIN_DC, FLD_PIN_RESET}; // SPI pins: CLK, MOSI, CS, DC, RST
    DisplayType type = SSD1306_SPI; // display type
    #endif
    bool flip = false;              // flip display 180°
    uint8_t contrast = 10;          // screen contrast
    uint8_t lineHeight = 1;         // 1 row or 2 rows
    uint32_t refreshRate = USER_LOOP_REFRESH_RATE_MS; // in ms
    uint32_t screenTimeout = SCREEN_TIMEOUT_MS;       // in ms
    bool sleepMode = true;          // allow screen sleep?
    bool clockMode = false;         // display clock

    // needRedraw marks if redraw is required to prevent often redrawing.
    bool needRedraw = true;

    // Next variables hold the previous known values to determine if redraw is
    // required.
    String knownSsid = "";
    IPAddress knownIp;
    uint8_t knownBrightness = 0;
    uint8_t knownEffectSpeed = 0;
    uint8_t knownEffectIntensity = 0;
    uint8_t knownMode = 0;
    uint8_t knownPalette = 0;
    uint8_t knownMinute = 99;
    byte brightness100;
    byte fxspeed100;
    byte fxintensity100;
    bool knownnightlight = nightlightActive;
    bool wificonnected = interfacesInited;
    bool powerON = true;

    bool displayTurnedOff = false;
    unsigned long lastUpdate = 0;
    unsigned long lastRedraw = 0;
    unsigned long overlayUntil = 0;
    // Set to 2 or 3 to mark lines 2 or 3. Other values ignored.
    byte markLineNum = 0;
    byte markColNum = 0;

    // strings to reduce flash memory usage (used more than twice)
    static const char _name[];
    static const char _contrast[];
    static const char _refreshRate[];
    static const char _screenTimeOut[];
    static const char _flip[];
    static const char _sleepMode[];
    static const char _clockMode[];
    static const char _busClkFrequency[];

    // If display does not work or looks corrupted check the
    // constructor reference:
    // https://github.com/olikraus/u8g2/wiki/u8x8setupcpp
    // or check the gallery:
    // https://github.com/olikraus/u8g2/wiki/gallery

  public:

    // gets called once at boot. Do all initialization that doesn't depend on
    // network here
    void setup() {
      if (type == NONE) return;
      if (type == SSD1306_SPI || type == SSD1306_SPI64) {
        PinManagerPinType pins[5] = { { ioPin[0], true }, { ioPin[1], true}, { ioPin[2], true }, { ioPin[3], true}, { ioPin[4], true }};
        if (!pinManager.allocateMultiplePins(pins, 5, PinOwner::UM_FourLineDisplay)) { type=NONE; return; }
      } else {
        PinManagerPinType pins[2] = { { ioPin[0], true }, { ioPin[1], true} };
        if (!pinManager.allocateMultiplePins(pins, 2, PinOwner::UM_FourLineDisplay)) { type=NONE; return; }
      }
      DEBUG_PRINTLN(F("Allocating display."));
      switch (type) {
        case SSD1306:
          #ifdef ESP8266
          if (!(ioPin[0]==5 && ioPin[1]==4))
            u8x8 = (U8X8 *) new U8X8_SSD1306_128X32_UNIVISION_SW_I2C(ioPin[0], ioPin[1]); // SCL, SDA, reset
          else
          #endif
            u8x8 = (U8X8 *) new U8X8_SSD1306_128X32_UNIVISION_HW_I2C(U8X8_PIN_NONE, ioPin[0], ioPin[1]); // Pins are Reset, SCL, SDA
          lineHeight = 1;
          break;
        case SH1106:
          #ifdef ESP8266
          if (!(ioPin[0]==5 && ioPin[1]==4))
            u8x8 = (U8X8 *) new U8X8_SH1106_128X64_WINSTAR_SW_I2C(ioPin[0], ioPin[1]); // SCL, SDA, reset
          else
          #endif
            u8x8 = (U8X8 *) new U8X8_SH1106_128X64_WINSTAR_HW_I2C(U8X8_PIN_NONE, ioPin[0], ioPin[1]); // Pins are Reset, SCL, SDA
          lineHeight = 2;
          break;
        case SSD1306_64:
          #ifdef ESP8266
          if (!(ioPin[0]==5 && ioPin[1]==4))
            u8x8 = (U8X8 *) new U8X8_SSD1306_128X64_NONAME_SW_I2C(ioPin[0], ioPin[1]); // SCL, SDA, reset
          else
          #endif
            u8x8 = (U8X8 *) new U8X8_SSD1306_128X64_NONAME_HW_I2C(U8X8_PIN_NONE, ioPin[0], ioPin[1]); // Pins are Reset, SCL, SDA
          lineHeight = 2;
          break;
        case SSD1305:
          #ifdef ESP8266
          if (!(ioPin[0]==5 && ioPin[1]==4))
            u8x8 = (U8X8 *) new U8X8_SSD1305_128X32_NONAME_SW_I2C(ioPin[0], ioPin[1]); // SCL, SDA, reset
          else
          #endif
            u8x8 = (U8X8 *) new U8X8_SSD1305_128X32_ADAFRUIT_HW_I2C(U8X8_PIN_NONE, ioPin[0], ioPin[1]); // Pins are Reset, SCL, SDA
          lineHeight = 1;
          break;
        case SSD1305_64:
          #ifdef ESP8266
          if (!(ioPin[0]==5 && ioPin[1]==4))
            u8x8 = (U8X8 *) new U8X8_SSD1305_128X64_ADAFRUIT_SW_I2C(ioPin[0], ioPin[1]); // SCL, SDA, reset
          else
          #endif
            u8x8 = (U8X8 *) new U8X8_SSD1305_128X64_ADAFRUIT_HW_I2C(U8X8_PIN_NONE, ioPin[0], ioPin[1]); // Pins are Reset, SCL, SDA
          lineHeight = 2;
          break;
        case SSD1306_SPI:
          if (!(ioPin[0]==FLD_PIN_CLOCKSPI && ioPin[1]==FLD_PIN_DATASPI)) // if not overridden these sould be HW accellerated
            u8x8 = (U8X8 *) new U8X8_SSD1306_128X32_UNIVISION_4W_SW_SPI(ioPin[0], ioPin[1], ioPin[2], ioPin[3], ioPin[4]);
          else
            u8x8 = (U8X8 *) new U8X8_SSD1306_128X32_UNIVISION_4W_HW_SPI(ioPin[2], ioPin[3], ioPin[4]); // Pins are cs, dc, reset
          lineHeight = 1;
          break;
        case SSD1306_SPI64:
          if (!(ioPin[0]==FLD_PIN_CLOCKSPI && ioPin[1]==FLD_PIN_DATASPI)) // if not overridden these sould be HW accellerated
            u8x8 = (U8X8 *) new U8X8_SSD1306_128X64_NONAME_4W_SW_SPI(ioPin[0], ioPin[1], ioPin[2], ioPin[3], ioPin[4]);
          else
            u8x8 = (U8X8 *) new U8X8_SSD1306_128X64_NONAME_4W_HW_SPI(ioPin[2], ioPin[3], ioPin[4]); // Pins are cs, dc, reset
          lineHeight = 2;
          break;
        default:
          u8x8 = nullptr;
      }
      if (nullptr == u8x8) {
          DEBUG_PRINTLN(F("Display init failed."));
          for (byte i=0; i<5 && ioPin[i]>=0; i++) pinManager.deallocatePin(ioPin[i], PinOwner::UM_FourLineDisplay);
          type = NONE;
          return;
      }

      initDone = true;
      DEBUG_PRINTLN(F("Starting display."));
      if (!(type == SSD1306_SPI || type == SSD1306_SPI64)) u8x8->setBusClock(ioFrequency);  // can be used for SPI too
      u8x8->begin();
      setFlipMode(flip);
      setContrast(contrast); //Contrast setup will help to preserve OLED lifetime. In case OLED need to be brighter increase number up to 255
      setPowerSave(0);
      drawString(0, 0, "Loading...");
    }

    // gets called every time WiFi is (re-)connected. Initialize own network
    // interfaces here
    void connected() {}

    /**
     * Da loop.
     */
    void loop() {
      if (displayTurnedOff && millis() - lastUpdate < 1000) {
        return;
        }else if (millis() - lastUpdate < refreshRate){
        return;}
      redraw(false);
      lastUpdate = millis();
    }

    /**
     * Wrappers for screen drawing
     */
    void setFlipMode(uint8_t mode) {
      if (type==NONE) return;
      u8x8->setFlipMode(mode);
    }
    void setContrast(uint8_t contrast) {
      if (type==NONE) return;
      u8x8->setContrast(contrast);
    }
    void drawString(uint8_t col, uint8_t row, const char *string, bool ignoreLH=false) {
      if (type==NONE) return;
      u8x8->setFont(u8x8_font_chroma48medium8_r);
      if (!ignoreLH && lineHeight==2) u8x8->draw1x2String(col, row, string);
      else                            u8x8->drawString(col, row, string);
    }
    void draw2x2String(uint8_t col, uint8_t row, const char *string) {
      if (type==NONE) return;
      u8x8->setFont(u8x8_font_chroma48medium8_r);
      u8x8->draw2x2String(col, row, string);
    }
    void drawGlyph(uint8_t col, uint8_t row, char glyph, const uint8_t *font, bool ignoreLH=false) {
      if (type==NONE) return;
      u8x8->setFont(font);
      if (!ignoreLH && lineHeight==2) u8x8->draw1x2Glyph(col, row, glyph);
      else                            u8x8->drawGlyph(col, row, glyph);
    }
    uint8_t getCols() {
      if (type==NONE) return 0;
      return u8x8->getCols();
    }
    void clear() {
      if (type==NONE) return;
      u8x8->clear();
    }
    void setPowerSave(uint8_t save) {
      if (type==NONE) return;
      u8x8->setPowerSave(save);
    }

    void center(String &line, uint8_t width) {
      int len = line.length();
      if (len<width) for (byte i=(width-len)/2; i>0; i--) line = ' ' + line;
      for (byte i=line.length(); i<width; i++) line += ' ';
    }

    //function to update lastredraw
      void updateRedrawTime(){
        lastRedraw = millis();
      }

    /**
     * Redraw the screen (but only if things have changed
     * or if forceRedraw).
     */
    void redraw(bool forceRedraw) {
      if (type==NONE) return;
        if (overlayUntil > 0) {
          if (millis() >= overlayUntil) {
            // Time to display the overlay has elapsed.
            overlayUntil = 0;
            forceRedraw = true;
          } else {
            // We are still displaying the overlay
            // Don't redraw.
            return;
          }
        }

  
      // Check if values which are shown on display changed from the last time.
      if (forceRedraw) {
          needRedraw = true;
      } else if ((bri == 0 && powerON) || (bri > 0 && !powerON)) {   //trigger power icon
          powerON = !powerON;
          drawStatusIcons();
          lastRedraw = millis();
      } else if (knownnightlight != nightlightActive) {   //trigger moon icon 
          knownnightlight = nightlightActive;
          drawStatusIcons();
          if (knownnightlight) overlay("    Timer On", 1000, 6);
          lastRedraw = millis();
      }else if (wificonnected != interfacesInited){   //trigger wifi icon
          wificonnected = interfacesInited;
          drawStatusIcons();
          lastRedraw = millis();
      } else if (knownMode != effectCurrent) {
          knownMode = effectCurrent;
          if(displayTurnedOff)needRedraw = true;
          else showCurrentEffectOrPalette(knownMode, JSON_mode_names, 3);
      } else if (knownPalette != effectPalette) {
           knownPalette = effectPalette;
           if(displayTurnedOff)needRedraw = true;
           else showCurrentEffectOrPalette(knownPalette, JSON_palette_names, 2);
      } else if (knownBrightness != bri) {
          if(displayTurnedOff && nightlightActive){needRedraw = false; knownBrightness = bri;}
          else if(displayTurnedOff)needRedraw = true;
          else updateBrightness();
      } else if (knownEffectSpeed != effectSpeed) {
          if(displayTurnedOff)needRedraw = true;
          else updateSpeed();
      } else if (knownEffectIntensity != effectIntensity) {
          if(displayTurnedOff)needRedraw = true;
          else updateIntensity();
      }
      

      if (!needRedraw) {
        // Nothing to change.
        // Turn off display after 1 minutes with no change.
        if(sleepMode && !displayTurnedOff && (millis() - lastRedraw > screenTimeout)) {
          // We will still check if there is a change in redraw()
          // and turn it back on if it changed.
          sleepOrClock(true);
        } else if (displayTurnedOff && clockMode) {
          showTime();
        }
        return;
      } else {
        clear();
      }

      needRedraw = false;
      lastRedraw = millis();
      
      if (displayTurnedOff) {
        // Turn the display back on
        sleepOrClock(false);
      }

      // Update last known values.
      knownSsid = apActive ? apSSID : WiFi.SSID(); //apActive ? WiFi.softAPSSID() : 
      knownIp = apActive ? IPAddress(4, 3, 2, 1) : WiFi.localIP();
      knownBrightness = bri;
      knownMode = effectCurrent;
      knownPalette = effectPalette;
      knownEffectSpeed = effectSpeed;
      knownEffectIntensity = effectIntensity;
      knownnightlight = nightlightActive;
      wificonnected = interfacesInited;

      // Do the actual drawing
      // First row: Icons
      draw2x2GlyphIcons();
      drawArrow();
      drawStatusIcons();

      // Second row 
      updateBrightness();
      updateSpeed();
      updateIntensity();

      // Third row
      showCurrentEffectOrPalette(knownPalette, JSON_palette_names, 2); //Palette info

      // Fourth row
      showCurrentEffectOrPalette(knownMode, JSON_mode_names, 3); //Effect Mode info
    }

    void updateBrightness(){
      knownBrightness = bri;
      if(overlayUntil == 0){
          brightness100 = (((float)(bri)/255)*100);
          char lineBuffer[4];
          sprintf_P(lineBuffer, PSTR("%-3d"), brightness100);
          drawString(1, lineHeight, lineBuffer);
          lastRedraw = millis();}
    }

    void updateSpeed(){
      knownEffectSpeed = effectSpeed;
      if(overlayUntil == 0){
          fxspeed100 = (((float)(effectSpeed)/255)*100);
          char lineBuffer[4];
          sprintf_P(lineBuffer, PSTR("%-3d"), fxspeed100);
          drawString(5, lineHeight, lineBuffer);
          lastRedraw = millis();}
    }

    void updateIntensity(){
      knownEffectIntensity = effectIntensity;
      if(overlayUntil == 0){
          fxintensity100 = (((float)(effectIntensity)/255)*100);
          char lineBuffer[4];
          sprintf_P(lineBuffer, PSTR("%-3d"), fxintensity100);
          drawString(9, lineHeight, lineBuffer);
          lastRedraw = millis();}
    }

    void draw2x2GlyphIcons(){
      if(lineHeight == 2){
        drawGlyph(1, 0, 1,             u8x8_font_benji_custom_icons_2x2, true);//brightness icon
        drawGlyph(5, 0, 2,             u8x8_font_benji_custom_icons_2x2, true);//speed icon
        drawGlyph(9, 0, 3,             u8x8_font_benji_custom_icons_2x2, true);//intensity icon
        drawGlyph(14, 2*lineHeight, 4, u8x8_font_benji_custom_icons_2x2, true);//palette icon
        drawGlyph(14, 3*lineHeight, 5, u8x8_font_benji_custom_icons_2x2, true);//effect icon
      }
      else{
        drawGlyph(2, 0, 69,           u8x8_font_open_iconic_weather_1x1);//brightness icon
        drawGlyph(6, 0, 72,              u8x8_font_open_iconic_play_1x1);//speed icon
        drawGlyph(10, 0, 78,            u8x8_font_open_iconic_thing_1x1);//intensity icon
        drawGlyph(15, 2*lineHeight, 4, u8x8_font_benji_custom_icons_1x1);//palette icon
        drawGlyph(15, 3*lineHeight, 70, u8x8_font_open_iconic_thing_1x1);//effect icon
      }
    }

    void drawStatusIcons(){
      drawGlyph(14, 0, 80 + (wificonnected?0:1),    u8x8_font_open_iconic_embedded_1x1, true); // wifi icon
      drawGlyph(15, 0, 78 + (bri > 0 ? 0 : 3),      u8x8_font_open_iconic_embedded_1x1, true); // power icon
      drawGlyph(13, 0, 66 + (nightlightActive?0:4), u8x8_font_open_iconic_weather_1x1, true); // moon icon for nighlight mode
    }
    
    /**
     * marks the position of the arrow showing
     * the current setting being changed
     * pass line and colum info
     */
    void setMarkLine(byte newMarkLineNum, byte newMarkColNum) {
        markLineNum = newMarkLineNum;
        markColNum = newMarkColNum;
    }

    //Draw the arrow for the current setting beiong changed
    void drawArrow(){
      if(markColNum != 255 && markLineNum !=255)drawGlyph(markColNum, markLineNum*lineHeight, 69, u8x8_font_open_iconic_play_1x1);
    }

     //Display the current effect or palette (desiredEntry) 
     // on the appropriate line (row). 
    void showCurrentEffectOrPalette(int inputEffPal, const char *qstring, uint8_t row) {
      knownMode = effectCurrent;
      knownPalette = effectPalette;
      if(overlayUntil == 0){
          char lineBuffer[MAX_JSON_CHARS];
          char smallBuffer1[MAX_MODE_LINE_SPACE];
          char smallBuffer2[MAX_MODE_LINE_SPACE];
          char smallBuffer3[MAX_MODE_LINE_SPACE+1];
          uint8_t qComma = 0;
          bool insideQuotes = false;
          bool spaceHit = false;
          uint8_t printedChars = 0;
          uint8_t smallChars1 = 0;
          uint8_t smallChars2 = 0;
          uint8_t smallChars3 = 0;
          uint8_t totalCount = 0;
          char singleJsonSymbol;

          // Find the mode name in JSON
          for (size_t i = 0; i < strlen_P(qstring); i++) {       //find and get the full text for printing
            singleJsonSymbol = pgm_read_byte_near(qstring + i);
            if (singleJsonSymbol == '\0') break;
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
                if (!insideQuotes || (qComma != inputEffPal)) break;
                lineBuffer[printedChars++] = singleJsonSymbol;
                totalCount++;
            }
            if ((qComma > inputEffPal)) break;
          }
          
          if(lineHeight ==2){                                    // use this code for 8 line display
            if(printedChars < (MAX_MODE_LINE_SPACE)){            // use big font if the text fits
                for (;printedChars < (MAX_MODE_LINE_SPACE-1); printedChars++) {lineBuffer[printedChars]=' '; }
                lineBuffer[printedChars] = 0;
                drawString(1, row*lineHeight, lineBuffer);
                lastRedraw = millis();
            }else{                                               // for long names divide the text into 2 lines and print them small
              for (uint8_t i = 0; i < printedChars; i++){
                  switch (lineBuffer[i]){
                  case ' ':
                    if(i > 4 && !spaceHit) {
                      spaceHit = true;
                      break;}
                    if(!spaceHit) smallBuffer1[smallChars1++] = lineBuffer[i];
                    if (spaceHit) smallBuffer2[smallChars2++] = lineBuffer[i];
                    break;
                  default:
                    if(!spaceHit) smallBuffer1[smallChars1++] = lineBuffer[i];
                    if (spaceHit) smallBuffer2[smallChars2++] = lineBuffer[i];
                    break;
                  }
                }
              for (; smallChars1 < (MAX_MODE_LINE_SPACE-1); smallChars1++) smallBuffer1[smallChars1]=' ';
                smallBuffer1[smallChars1] = 0;
                drawString(1, row*lineHeight, smallBuffer1, true);
              for (; smallChars2 < (MAX_MODE_LINE_SPACE-1); smallChars2++) smallBuffer2[smallChars2]=' '; 
                smallBuffer2[smallChars2] = 0;
                drawString(1, row*lineHeight+1, smallBuffer2, true);
                lastRedraw = millis();
            }
          }
          else{                                             // use this code for 4 ling displays
            if (printedChars > MAX_MODE_LINE_SPACE) printedChars = MAX_MODE_LINE_SPACE;
            for (uint8_t i = 0; i < printedChars; i++){
              smallBuffer3[smallChars3++] = lineBuffer[i];
            }
            
            for (; smallChars3 < (MAX_MODE_LINE_SPACE); smallChars3++) smallBuffer3[smallChars3]=' ';
                smallBuffer3[smallChars3] = 0;
                drawString(1, row*lineHeight, smallBuffer3, true);
                lastRedraw = millis();
          }
         }
    }

    /**
     * If there screen is off or in clock is displayed,
     * this will return true. This allows us to throw away
     * the first input from the rotary encoder but
     * to wake up the screen.
     */
    bool wakeDisplay() {
      //knownHour = 99;
      if (displayTurnedOff) {
        // Turn the display back on
        sleepOrClock(false);
        redraw(true);
        return true;
      }
      return false;
    }

    /**
     * Allows you to show one line and a glyph as overlay for a
     * period of time.
     * Clears the screen and prints.
     */
    void overlay(const char* line1, long showHowLong, byte glyphType) {
          if (displayTurnedOff) {
            // Turn the display back on
            sleepOrClock(false);
          }

          // Print the overlay
          clear();
          if (glyphType > 0){
          if ( lineHeight == 2) drawGlyph(5, 0, glyphType, u8x8_font_benji_custom_icons_6x6, true);
          else drawGlyph(7, lineHeight, glyphType, u8x8_font_benji_custom_icons_2x2, true);
          }
          if (line1) drawString(0, 3*lineHeight, line1);
          overlayUntil = millis() + showHowLong;
    }

    void networkOverlay(const char* line1, long showHowLong) {
          if (displayTurnedOff) {
            // Turn the display back on
            sleepOrClock(false);
          }
      // Print the overlay
          clear();
      // First row string
          if (line1) drawString(0, 0, line1);
      // Second row with Wifi name
          String ssidString = knownSsid.substring(0, getCols() > 1 ? getCols() - 2 : 0); //
          drawString(0, lineHeight, ssidString.c_str());
      // Print `~` char to indicate that SSID is longer, than our display
          if (knownSsid.length() > getCols()) {
            drawString(getCols() - 1, 0, "~");
          }
      // Third row with IP and Psssword in AP Mode
          drawString(0, lineHeight*2, (knownIp.toString()).c_str());
          if (apActive) {
            String appassword = apPass;
            drawString(0, lineHeight*3, appassword.c_str());
          } 
          overlayUntil = millis() + showHowLong;
    }


    /**
     * Enable sleep (turn the display off) or clock mode.
     */
    void sleepOrClock(bool enabled) {
      if (enabled) {
        if (clockMode) {
          clear();
          knownMinute = 99;
          showTime();
        }else           setPowerSave(1);
        displayTurnedOff = true;
      }
      else {
        setPowerSave(0);
        displayTurnedOff = false;
      }
    }

    /**
     * Display the current date and time in large characters
     * on the middle rows. Based 24 or 12 hour depending on
     * the useAMPM configuration.
     */
    void showTime() {
      if(knownMinute != minute(localTime)){  //only redraw clock if it has changed
      char lineBuffer[LINE_BUFFER_SIZE];

      //updateLocalTime();
      byte AmPmHour = hour(localTime);
      boolean isitAM = true;
      if (useAMPM) {
        if (AmPmHour > 11) AmPmHour -= 12;
        if (AmPmHour == 0) AmPmHour  = 12;
        if (hour(localTime) > 11) isitAM = false;
      }
       clear();
       drawStatusIcons(); //icons power, wifi, timer, etc

      sprintf_P(lineBuffer, PSTR("%s %2d "), monthShortStr(month(localTime)), day(localTime)); 
        draw2x2String(DATE_INDENT, lineHeight==1 ? 0 : lineHeight, lineBuffer); // adjust for 8 line displays, draw month and day

      sprintf_P(lineBuffer,PSTR("%2d:%02d"), (useAMPM ? AmPmHour : hour(localTime)), minute(localTime));
        draw2x2String(TIME_INDENT+2, lineHeight*2, lineBuffer); //draw hour, min. blink ":" depending on odd/even seconds

        if (useAMPM) drawString(12, lineHeight*2, (isitAM ? "AM" : "PM"), true); //draw am/pm if using 12 time
      knownMinute = minute(localTime);
      }
    }

    /*
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
     * Below it is shown how this could be used for e.g. a light sensor
     */
    //void addToJsonInfo(JsonObject& root) {
      //JsonObject user = root["u"];
      //if (user.isNull()) user = root.createNestedObject("u");
      //JsonArray data = user.createNestedArray(F("4LineDisplay"));
      //data.add(F("Loaded."));
    //}

    /*
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    //void addToJsonState(JsonObject& root) {
    //}

    /*
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    //void readFromJsonState(JsonObject& root) {
    //  if (!initDone) return;  // prevent crash on boot applyPreset()
    //}

    /*
     * addToConfig() can be used to add custom persistent settings to the cfg.json file in the "um" (usermod) object.
     * It will be called by WLED when settings are actually saved (for example, LED settings are saved)
     * If you want to force saving the current state, use serializeConfig() in your loop().
     * 
     * CAUTION: serializeConfig() will initiate a filesystem write operation.
     * It might cause the LEDs to stutter and will cause flash wear if called too often.
     * Use it sparingly and always in the loop, never in network callbacks!
     * 
     * addToConfig() will also not yet add your setting to one of the settings pages automatically.
     * To make that work you still have to add the setting to the HTML, xml.cpp and set.cpp manually.
     * 
     * I highly recommend checking out the basics of ArduinoJson serialization and deserialization in order to use custom settings!
     */
    void addToConfig(JsonObject& root) {
      JsonObject top   = root.createNestedObject(FPSTR(_name));
      JsonArray io_pin = top.createNestedArray("pin");
      for (byte i=0; i<5; i++) io_pin.add(ioPin[i]);
      top["help4PinTypes"]       = F("Clk,Data,CS,DC,RST"); // help for Settings page
      top["type"]                = type;
      top[FPSTR(_flip)]          = (bool) flip;
      top[FPSTR(_contrast)]      = contrast;
      top[FPSTR(_refreshRate)]   = refreshRate/10;
      top[FPSTR(_screenTimeOut)] = screenTimeout/1000;
      top[FPSTR(_sleepMode)]     = (bool) sleepMode;
      top[FPSTR(_clockMode)]     = (bool) clockMode;
      top[FPSTR(_busClkFrequency)] = ioFrequency/1000;
      DEBUG_PRINTLN(F("4 Line Display config saved."));
    }

    /*
     * readFromConfig() can be used to read back the custom settings you added with addToConfig().
     * This is called by WLED when settings are loaded (currently this only happens once immediately after boot)
     * 
     * readFromConfig() is called BEFORE setup(). This means you can use your persistent values in setup() (e.g. pin assignments, buffer sizes),
     * but also that if you want to write persistent values to a dynamic buffer, you'd need to allocate it here instead of in setup.
     * If you don't know what that is, don't fret. It most likely doesn't affect your use case :)
     */
    bool readFromConfig(JsonObject& root) {
      bool needsRedraw    = false;
      DisplayType newType = type;
      int8_t newPin[5]; for (byte i=0; i<5; i++) newPin[i] = ioPin[i];

      JsonObject top = root[FPSTR(_name)];
      if (top.isNull()) {
        DEBUG_PRINT(FPSTR(_name));
        DEBUG_PRINTLN(F(": No config found. (Using defaults.)"));
        return false;
      }

      newType       = top["type"] | newType;
      for (byte i=0; i<5; i++) newPin[i] = top["pin"][i] | ioPin[i];
      flip          = top[FPSTR(_flip)] | flip;
      contrast      = top[FPSTR(_contrast)] | contrast;
      refreshRate   = (top[FPSTR(_refreshRate)] | refreshRate/10) * 10;
      screenTimeout = (top[FPSTR(_screenTimeOut)] | screenTimeout/1000) * 1000;
      sleepMode     = top[FPSTR(_sleepMode)] | sleepMode;
      clockMode     = top[FPSTR(_clockMode)] | clockMode;
      ioFrequency   = min(3400, max(100, (int)(top[FPSTR(_busClkFrequency)] | ioFrequency/1000))) * 1000;  // limit frequency

      DEBUG_PRINT(FPSTR(_name));
      if (!initDone) {
        // first run: reading from cfg.json
        for (byte i=0; i<5; i++) ioPin[i] = newPin[i];
        type = newType;
        DEBUG_PRINTLN(F(" config loaded."));
      } else {
        DEBUG_PRINTLN(F(" config (re)loaded."));
        // changing parameters from settings page
        bool pinsChanged = false;
        for (byte i=0; i<5; i++) if (ioPin[i] != newPin[i]) { pinsChanged = true; break; }
        if (pinsChanged || type!=newType) {
          if (type != NONE) delete u8x8;
          for (byte i=0; i<5; i++) {
            if (ioPin[i]>=0) pinManager.deallocatePin(ioPin[i], PinOwner::UM_FourLineDisplay);
            ioPin[i] = newPin[i];
          }
          if (ioPin[0]<0 || ioPin[1]<0) { // data & clock must be > -1
            type = NONE;
            return true;
          } else type = newType;
          setup();
          needsRedraw |= true;
        }
        if (!(type == SSD1306_SPI || type == SSD1306_SPI64)) u8x8->setBusClock(ioFrequency); // can be used for SPI too
        setContrast(contrast);
        setFlipMode(flip);
        if (needsRedraw && !wakeDisplay()) redraw(true);
      }
      // use "return !top["newestParameter"].isNull();" when updating Usermod with new features
      return !(top[_busClkFrequency]).isNull();
    }

    /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
    uint16_t getId() {
      return USERMOD_ID_FOUR_LINE_DISP;
    }
};

// strings to reduce flash memory usage (used more than twice)
const char FourLineDisplayUsermod::_name[]          PROGMEM = "4LineDisplay";
const char FourLineDisplayUsermod::_contrast[]      PROGMEM = "contrast";
const char FourLineDisplayUsermod::_refreshRate[]   PROGMEM = "refreshRate0.01Sec";
const char FourLineDisplayUsermod::_screenTimeOut[] PROGMEM = "screenTimeOutSec";
const char FourLineDisplayUsermod::_flip[]          PROGMEM = "flip";
const char FourLineDisplayUsermod::_sleepMode[]     PROGMEM = "sleepMode";
const char FourLineDisplayUsermod::_clockMode[]     PROGMEM = "clockMode";
const char FourLineDisplayUsermod::_busClkFrequency[] PROGMEM = "i2c-freq-kHz";
