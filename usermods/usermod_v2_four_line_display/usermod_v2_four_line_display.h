#pragma once

#include "wled.h"
#include <U8x8lib.h> // from https://github.com/olikraus/u8g2/

//
// Insired by the v1 usermod: ssd1306_i2c_oled_u8g2
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
#else
  #ifndef FLD_PIN_SCL
    #define FLD_PIN_SCL 5
  #endif
  #ifndef FLD_PIN_SDA
    #define FLD_PIN_SDA 4
  #endif
#endif

// When to time out to the clock or blank the screen
// if SLEEP_MODE_ENABLED.
#define SCREEN_TIMEOUT_MS  60*1000    // 1 min

#define TIME_INDENT        0
#define DATE_INDENT        2

// Minimum time between redrawing screen in ms
#define USER_LOOP_REFRESH_RATE_MS 1000

// Extra char (+1) for null
#define LINE_BUFFER_SIZE            16+1

typedef enum {
  FLD_LINE_BRIGHTNESS = 0,
  FLD_LINE_EFFECT_SPEED,
  FLD_LINE_EFFECT_INTENSITY,
  FLD_LINE_MODE,
  FLD_LINE_PALETTE,
  FLD_LINE_TIME
} Line4Type;

typedef enum {
  NONE = 0,
  SSD1306,    // U8X8_SSD1306_128X32_UNIVISION_HW_I2C
  SH1106,     // U8X8_SH1106_128X64_WINSTAR_HW_I2C
  SSD1306_64, // U8X8_SSD1306_128X64_NONAME_HW_I2C
  SSD1305,    // U8X8_SSD1305_128X32_ADAFRUIT_HW_I2C
  SSD1305_64  // U8X8_SSD1305_128X64_ADAFRUIT_HW_I2C
} DisplayType;

class FourLineDisplayUsermod : public Usermod {

  private:

    bool initDone = false;
    unsigned long lastTime = 0;

    // HW interface & configuration
    U8X8 *u8x8 = nullptr;           // pointer to U8X8 display object
    int8_t sclPin=FLD_PIN_SCL, sdaPin=FLD_PIN_SDA;    // I2C pins for interfacing, get initialised in readFromConfig()
    DisplayType type = SSD1306;     // display type
    bool flip = false;              // flip display 180°
    uint8_t contrast = 10;          // screen contrast
    uint8_t lineHeight = 1;         // 1 row or 2 rows
    uint32_t refreshRate = USER_LOOP_REFRESH_RATE_MS; // in ms
    uint32_t screenTimeout = SCREEN_TIMEOUT_MS;       // in ms
    bool sleepMode = true;          // allow screen sleep?
    bool clockMode = false;         // display clock

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
    uint8_t knownHour = 99;

    bool displayTurnedOff = false;
    unsigned long lastUpdate = 0;
    unsigned long lastRedraw = 0;
    unsigned long overlayUntil = 0;
    Line4Type lineType = FLD_LINE_BRIGHTNESS;
    // Set to 2 or 3 to mark lines 2 or 3. Other values ignored.
    byte markLineNum = 0;

    // strings to reduce flash memory usage (used more than twice)
    static const char _name[];
    static const char _contrast[];
    static const char _refreshRate[];
    static const char _screenTimeOut[];
    static const char _flip[];
    static const char _sleepMode[];
    static const char _clockMode[];

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
      if (!pinManager.allocatePin(sclPin)) { sclPin = -1; type = NONE; return;}
      if (!pinManager.allocatePin(sdaPin)) { pinManager.deallocatePin(sclPin); sclPin = sdaPin = -1; type = NONE; return; }
      switch (type) {
        case SSD1306:
          #ifdef ESP8266
          if (!(sclPin==5 && sdaPin==4))
            u8x8 = (U8X8 *) new U8X8_SSD1306_128X32_UNIVISION_SW_I2C(sclPin, sdaPin); // SCL, SDA, reset
          else
          #endif
            u8x8 = (U8X8 *) new U8X8_SSD1306_128X32_UNIVISION_HW_I2C(U8X8_PIN_NONE, sclPin, sdaPin); // Pins are Reset, SCL, SDA
          lineHeight = 1;
          break;
        case SH1106:
          #ifdef ESP8266
          if (!(sclPin==5 && sdaPin==4))
            u8x8 = (U8X8 *) new U8X8_SH1106_128X64_WINSTAR_SW_I2C(sclPin, sdaPin); // SCL, SDA, reset
          else
          #endif
            u8x8 = (U8X8 *) new U8X8_SH1106_128X64_WINSTAR_HW_I2C(U8X8_PIN_NONE, sclPin, sdaPin); // Pins are Reset, SCL, SDA
          lineHeight = 2;
          break;
        case SSD1306_64:
          #ifdef ESP8266
          if (!(sclPin==5 && sdaPin==4))
            u8x8 = (U8X8 *) new U8X8_SSD1306_128X64_NONAME_SW_I2C(sclPin, sdaPin); // SCL, SDA, reset
          else
          #endif
            u8x8 = (U8X8 *) new U8X8_SSD1306_128X64_NONAME_HW_I2C(U8X8_PIN_NONE, sclPin, sdaPin); // Pins are Reset, SCL, SDA
          lineHeight = 2;
          break;
        case SSD1305:
          #ifdef ESP8266
          if (!(sclPin==5 && sdaPin==4))
            u8x8 = (U8X8 *) new U8X8_SSD1305_128X32_NONAME_SW_I2C(sclPin, sdaPin); // SCL, SDA, reset
          else
          #endif
            u8x8 = (U8X8 *) new U8X8_SSD1305_128X32_ADAFRUIT_HW_I2C(U8X8_PIN_NONE, sclPin, sdaPin); // Pins are Reset, SCL, SDA
          lineHeight = 1;
          break;
        case SSD1305_64:
          #ifdef ESP8266
          if (!(sclPin==5 && sdaPin==4))
            u8x8 = (U8X8 *) new U8X8_SSD1305_128X64_ADAFRUIT_SW_I2C(sclPin, sdaPin); // SCL, SDA, reset
          else
          #endif
            u8x8 = (U8X8 *) new U8X8_SSD1305_128X64_ADAFRUIT_HW_I2C(U8X8_PIN_NONE, sclPin, sdaPin); // Pins are Reset, SCL, SDA
          lineHeight = 2;
          break;
        default:
          u8x8 = nullptr;
          type = NONE;
          return;
      }
      (static_cast<U8X8*>(u8x8))->begin();
      setFlipMode(flip);
      setContrast(contrast); //Contrast setup will help to preserve OLED lifetime. In case OLED need to be brighter increase number up to 255
      setPowerSave(0);
      drawString(0, 0, "Loading...");
      initDone = true;
    }

    // gets called every time WiFi is (re-)connected. Initialize own network
    // interfaces here
    void connected() {}

    /**
     * Da loop.
     */
    void loop() {
      if (millis() - lastUpdate < (clockMode?1000:refreshRate) || strip.isUpdating()) return;
      lastUpdate = millis();

      redraw(false);
    }

    /**
     * Wrappers for screen drawing
     */
    void setFlipMode(uint8_t mode) {
      if (type==NONE) return;
      (static_cast<U8X8*>(u8x8))->setFlipMode(mode);
    }
    void setContrast(uint8_t contrast) {
      if (type==NONE) return;
      (static_cast<U8X8*>(u8x8))->setContrast(contrast);
    }
    void drawString(uint8_t col, uint8_t row, const char *string, bool ignoreLH=false) {
      if (type==NONE) return;
      (static_cast<U8X8*>(u8x8))->setFont(u8x8_font_chroma48medium8_r);
      if (!ignoreLH && lineHeight==2) (static_cast<U8X8*>(u8x8))->draw1x2String(col, row, string);
      else                            (static_cast<U8X8*>(u8x8))->drawString(col, row, string);
    }
    void draw2x2String(uint8_t col, uint8_t row, const char *string) {
      if (type==NONE) return;
      (static_cast<U8X8*>(u8x8))->setFont(u8x8_font_chroma48medium8_r);
      (static_cast<U8X8*>(u8x8))->draw2x2String(col, row, string);
    }
    void drawGlyph(uint8_t col, uint8_t row, char glyph, const uint8_t *font, bool ignoreLH=false) {
      if (type==NONE) return;
      (static_cast<U8X8*>(u8x8))->setFont(font);
      if (!ignoreLH && lineHeight==2) (static_cast<U8X8*>(u8x8))->draw1x2Glyph(col, row, glyph);
      else                            (static_cast<U8X8*>(u8x8))->drawGlyph(col, row, glyph);
    }
    uint8_t getCols() {
      if (type==NONE) return 0;
      return (static_cast<U8X8*>(u8x8))->getCols();
    }
    void clear() {
      if (type==NONE) return;
      (static_cast<U8X8*>(u8x8))->clear();
    }
    void setPowerSave(uint8_t save) {
      if (type==NONE) return;
      (static_cast<U8X8*>(u8x8))->setPowerSave(save);
    }

    /**
     * Redraw the screen (but only if things have changed
     * or if forceRedraw).
     */
    void redraw(bool forceRedraw) {
      static bool showName = false;
      unsigned long now = millis();

      if (type==NONE) return;
      if (overlayUntil > 0) {
        if (now >= overlayUntil) {
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
      if (forceRedraw ||
          (((apActive) ? String(apSSID) : WiFi.SSID()) != knownSsid) ||
          (knownIp != (apActive ? IPAddress(4, 3, 2, 1) : Network.localIP())) ||
          (knownBrightness != bri) ||
          (knownEffectSpeed != effectSpeed) ||
          (knownEffectIntensity != effectIntensity) ||
          (knownMode != strip.getMode()) ||
          (knownPalette != strip.getSegment(0).palette)) {
        knownHour = 99; // force time update
        clear();
      } else if (sleepMode && !displayTurnedOff && ((now - lastRedraw)/1000)%5 == 0) {
        // change line every 5s
        showName = !showName;
        switch (lineType) {
          case FLD_LINE_BRIGHTNESS:
            lineType = FLD_LINE_EFFECT_SPEED;
            break;
          case FLD_LINE_MODE:
            lineType = FLD_LINE_BRIGHTNESS;
            break;
          case FLD_LINE_PALETTE:
            lineType = clockMode ? FLD_LINE_MODE : FLD_LINE_BRIGHTNESS;
            break;
          case FLD_LINE_EFFECT_SPEED:
            lineType = FLD_LINE_EFFECT_INTENSITY;
            break;
          case FLD_LINE_EFFECT_INTENSITY:
            lineType = FLD_LINE_PALETTE;
            break;
          default:
            lineType = FLD_LINE_MODE;
            break;
        }
        knownHour = 99; // force time update
      } else {
        // Nothing to change.
        // Turn off display after 3 minutes with no change.
        if(sleepMode && !displayTurnedOff && (millis() - lastRedraw > screenTimeout)) {
          // We will still check if there is a change in redraw()
          // and turn it back on if it changed.
          clear(); // force screen clear
          sleepOrClock(true);
        } else if (displayTurnedOff && clockMode) {
          showTime();
        }
        return;
      }

      // do not update lastRedraw marker if just switching row contenet
      if (((now - lastRedraw)/1000)%5 != 0) lastRedraw = now;
      
      // Turn the display back on
      if (displayTurnedOff) sleepOrClock(false);

      // Update last known values.
      knownSsid = apActive ? WiFi.softAPSSID() : WiFi.SSID();
      knownIp = apActive ? IPAddress(4, 3, 2, 1) : Network.localIP();
      knownBrightness = bri;
      knownMode = strip.getMode();
      knownPalette = strip.getSegment(0).palette;
      knownEffectSpeed = effectSpeed;
      knownEffectIntensity = effectIntensity;

      // Do the actual drawing

      // First row with Wifi name
      drawGlyph(0, 0, 80, u8x8_font_open_iconic_embedded_1x1); // home icon
      String ssidString = knownSsid.substring(0, getCols() > 1 ? getCols() - 2 : 0);
      drawString(1, 0, ssidString.c_str());
      // Print `~` char to indicate that SSID is longer, than our display
      if (knownSsid.length() > getCols()) {
        drawString(getCols() - 1, 0, "~");
      }

      // Second row with IP or Psssword
      drawGlyph(0, lineHeight, 68, u8x8_font_open_iconic_embedded_1x1); // wifi icon
      // Print password in AP mode and if led is OFF.
      if (apActive && bri == 0) {
        drawString(1, lineHeight, apPass);
      } else {
        // alternate IP address and server name
        String secondLine = knownIp.toString();
        if (showName && strcmp(serverDescription, "WLED") != 0) {
          secondLine = serverDescription;
        }
        for (uint8_t i=secondLine.length(); i<getCols()-1; i++) secondLine += ' ';
        drawString(1, lineHeight, secondLine.c_str());
      }

      // draw third and fourth row
      drawLine(2, clockMode ? lineType : FLD_LINE_MODE);
      drawLine(3, clockMode ? FLD_LINE_TIME : lineType);

      drawGlyph(0, 2*lineHeight, 66 + (bri > 0 ? 3 : 0), u8x8_font_open_iconic_weather_2x2); // sun/moon icon
      //if (markLineNum>1) drawGlyph(2, markLineNum*lineHeight, 66, u8x8_font_open_iconic_arrow_1x1); // arrow icon
    }

    void drawLine(uint8_t line, Line4Type lineType) {
      char lineBuffer[LINE_BUFFER_SIZE];
      switch(lineType) {
        case FLD_LINE_BRIGHTNESS:
          sprintf_P(lineBuffer, PSTR("Brightness %3d"), bri);
          drawString(2, line*lineHeight, lineBuffer);
          break;
        case FLD_LINE_EFFECT_SPEED:
          sprintf_P(lineBuffer, PSTR("FX Speed   %3d"), effectSpeed);
          drawString(2, line*lineHeight, lineBuffer);
          break;
        case FLD_LINE_EFFECT_INTENSITY:
          sprintf_P(lineBuffer, PSTR("FX Intens. %3d"), effectIntensity);
          drawString(2, line*lineHeight, lineBuffer);
          break;
        case FLD_LINE_MODE:
          showCurrentEffectOrPalette(knownMode, JSON_mode_names, line);
          break;
        case FLD_LINE_PALETTE:
          showCurrentEffectOrPalette(knownPalette, JSON_palette_names, line);
          break;
        case FLD_LINE_TIME:
          showTime(false);
          break;
        default:
          // unknown type, do nothing
          break;
      }
    }

    /**
     * Display the current effect or palette (desiredEntry) 
     * on the appropriate line (row).
     */
    void showCurrentEffectOrPalette(int knownMode, const char *qstring, uint8_t row) {
      char lineBuffer[LINE_BUFFER_SIZE];
      uint8_t qComma = 0;
      bool insideQuotes = false;
      uint8_t printedChars = 0;
      char singleJsonSymbol;

      // Find the mode name in JSON
      for (size_t i = 0; i < strlen_P(qstring); i++) {
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
            if (!insideQuotes || (qComma != knownMode)) break;
            lineBuffer[printedChars++] = singleJsonSymbol;
        }
        if ((qComma > knownMode) || (printedChars >= getCols()-2) || printedChars >= sizeof(lineBuffer)-2) break;
      }
      for (;printedChars < getCols()-2 && printedChars < sizeof(lineBuffer)-2; printedChars++) lineBuffer[printedChars]=' ';
      lineBuffer[printedChars] = 0;
      drawString(2, row*lineHeight, lineBuffer);
    }

    /**
     * If there screen is off or in clock is displayed,
     * this will return true. This allows us to throw away
     * the first input from the rotary encoder but
     * to wake up the screen.
     */
    bool wakeDisplay() {
      knownHour = 99;
      if (displayTurnedOff) {
        // Turn the display back on
        sleepOrClock(false);
        redraw(true);
        return true;
      }
      return false;
    }

    /**
     * Allows you to show up to two lines as overlay for a
     * period of time.
     * Clears the screen and prints on the middle two lines.
     */
    void overlay(const char* line1, const char *line2, long showHowLong) {
      if (displayTurnedOff) {
        // Turn the display back on
        sleepOrClock(false);
      }

      // Print the overlay
      clear();
      if (line1) drawString(0, 1*lineHeight, line1);
      if (line2) drawString(0, 2*lineHeight, line2);
      overlayUntil = millis() + showHowLong;
    }

    /**
     * Line 3 or 4 (last two lines) can be marked with an
     * arrow in the first column. Pass 2 or 3 to this to
     * specify which line to mark with an arrow.
     * Any other values are ignored.
     */
    void setMarkLine(byte newMarkLineNum) {
      if (newMarkLineNum == 2 || newMarkLineNum == 3) {
        markLineNum = newMarkLineNum;
      }
      else {
        markLineNum = 0;
      }
    }

    /**
     * Enable sleep (turn the display off) or clock mode.
     */
    void sleepOrClock(bool enabled) {
      if (enabled) {
        if (clockMode) showTime();
        else           setPowerSave(1);
        displayTurnedOff = true;
      } else {
        setPowerSave(0);
        displayTurnedOff = false;
      }
    }

    /**
     * Display the current date and time in large characters
     * on the middle rows. Based 24 or 12 hour depending on
     * the useAMPM configuration.
     */
    void showTime(bool fullScreen = true) {
      char lineBuffer[LINE_BUFFER_SIZE];

      updateLocalTime();
      byte minuteCurrent = minute(localTime);
      byte hourCurrent   = hour(localTime);
      byte secondCurrent = second(localTime);
      if (knownMinute == minuteCurrent && knownHour == hourCurrent) {
        // Time hasn't changed.
        if (!fullScreen) return;
      } else {
        //if (fullScreen) clear();
      }
      knownMinute = minuteCurrent;
      knownHour = hourCurrent;

      byte currentMonth = month(localTime);
      sprintf_P(lineBuffer, PSTR("%s %2d "), monthShortStr(currentMonth), day(localTime));
      if (fullScreen)
        draw2x2String(DATE_INDENT, lineHeight==1 ? 0 : lineHeight, lineBuffer); // adjust for 8 line displays
      else
        drawString(2, lineHeight*3, lineBuffer);

      byte showHour = hourCurrent;
      boolean isAM = false;
      if (useAMPM) {
        if (showHour == 0) {
          showHour = 12;
          isAM = true;
        } 
        else if (showHour > 12) {
          showHour -= 12;
          isAM = false;
        }
        else {
          isAM = true;
        }
      }

      sprintf_P(lineBuffer, (secondCurrent%2 || !fullScreen) ? PSTR("%2d:%02d") : PSTR("%2d %02d"), (useAMPM ? showHour : hourCurrent), minuteCurrent);
      // For time, we always use LINE_HEIGHT of 2 since
      // we are printing it big.
      if (fullScreen) {
        draw2x2String(TIME_INDENT+2, lineHeight*2, lineBuffer);
        sprintf_P(lineBuffer, PSTR("%02d"), secondCurrent);
        if (useAMPM) drawString(12+(fullScreen?0:2), lineHeight*2, (isAM ? "AM" : "PM"), true);
        else         drawString(12, lineHeight*2+1, lineBuffer, true); // even with double sized rows print seconds in 1 line
      } else {
        drawString(9+(useAMPM?0:2), lineHeight*3, lineBuffer);
        if (useAMPM) drawString(12+(fullScreen?0:2), lineHeight*3, (isAM ? "AM" : "PM"), true);
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
      JsonObject top    = root.createNestedObject(FPSTR(_name));
      JsonArray i2c_pin = top.createNestedArray("pin");
      i2c_pin.add(sclPin);
      i2c_pin.add(sdaPin);
      top["type"]                = type;
      top[FPSTR(_flip)]          = (bool) flip;
      top[FPSTR(_contrast)]      = contrast;
      top[FPSTR(_refreshRate)]   = refreshRate/1000;
      top[FPSTR(_screenTimeOut)] = screenTimeout/1000;
      top[FPSTR(_sleepMode)]     = (bool) sleepMode;
      top[FPSTR(_clockMode)]     = (bool) clockMode;
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
      int8_t newScl       = sclPin;
      int8_t newSda       = sdaPin;

      JsonObject top = root[FPSTR(_name)];
      if (top.isNull()) {
        DEBUG_PRINT(FPSTR(_name));
        DEBUG_PRINTLN(F(": No config found. (Using defaults.)"));
        return false;
      }

      newScl        = top["pin"][0] | newScl;
      newSda        = top["pin"][1] | newSda;
      newType       = top["type"] | newType;
      flip          = top[FPSTR(_flip)] | flip;
      contrast      = top[FPSTR(_contrast)] | contrast;
      refreshRate   = (top[FPSTR(_refreshRate)] | refreshRate/1000) * 1000;
      screenTimeout = (top[FPSTR(_screenTimeOut)] | screenTimeout/1000) * 1000;
      sleepMode     = top[FPSTR(_sleepMode)] | sleepMode;
      clockMode     = top[FPSTR(_clockMode)] | clockMode;

      DEBUG_PRINT(FPSTR(_name));
      if (!initDone) {
        // first run: reading from cfg.json
        sclPin = newScl;
        sdaPin = newSda;
        type = newType;
        DEBUG_PRINTLN(F(" config loaded."));
      } else {
        // changing parameters from settings page
        if (sclPin!=newScl || sdaPin!=newSda || type!=newType) {
          if (type != NONE) delete (static_cast<U8X8*>(u8x8));
          pinManager.deallocatePin(sclPin);
          pinManager.deallocatePin(sdaPin);
          sclPin = newScl;
          sdaPin = newSda;
          if (newScl<0 || newSda<0) {
            type = NONE;
            return true;
          } else type = newType;
          setup();
          needsRedraw |= true;
        }
        setContrast(contrast);
        setFlipMode(flip);
        if (needsRedraw && !wakeDisplay()) redraw(true);
        DEBUG_PRINTLN(F(" config (re)loaded."));
      }
      // use "return !top["newestParameter"].isNull();" when updating Usermod with new features
      return true;
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
const char FourLineDisplayUsermod::_refreshRate[]   PROGMEM = "refreshRateSec";
const char FourLineDisplayUsermod::_screenTimeOut[] PROGMEM = "screenTimeOutSec";
const char FourLineDisplayUsermod::_flip[]          PROGMEM = "flip";
const char FourLineDisplayUsermod::_sleepMode[]     PROGMEM = "sleepMode";
const char FourLineDisplayUsermod::_clockMode[]     PROGMEM = "clockMode";
