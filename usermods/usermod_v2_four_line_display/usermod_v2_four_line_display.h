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
  FLD_LINE_3_BRIGHTNESS = 0,
  FLD_LINE_3_EFFECT_SPEED,
  FLD_LINE_3_EFFECT_INTENSITY,
  FLD_LINE_3_PALETTE,
  FLD_LINE_3_TIME
} Line3Type;

typedef enum {
  NONE = 0,
  SSD1306,  // U8X8_SSD1306_128X32_UNIVISION_HW_I2C
  SH1106    // U8X8_SH1106_128X64_WINSTAR_HW_I2C
} DisplayType;

class FourLineDisplayUsermod : public Usermod {
  private:
    unsigned long lastTime = 0;

    // HW interface & configuration
    void *u8x8;               // pointer to U8X8 display object
    int8_t sclPin, sdaPin;    // I2C pins for interfacing, get initialised in readFromConfig()
    DisplayType type = NONE;  // display type
    bool flip;                // flip display 180°
    uint8_t contrast;
    uint8_t lineHeight;       // 1 row or 2 rows
    uint32_t refreshRate;     // in ms
    uint32_t screenTimeout;   // in ms
    bool sleepMode;           // allow screen sleep?
    bool clockMode;           // display clock instead of info?

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
    uint8_t knownHour = 99;

    bool displayTurnedOff = false;
    long lastUpdate = 0;
    long lastRedraw = 0;
    long overlayUntil = 0;
    Line3Type lineThreeType = FLD_LINE_3_BRIGHTNESS;
    // Set to 2 or 3 to mark lines 2 or 3. Other values ignored.
    byte markLineNum = 0;

    char **modes_qstrings = nullptr;
    char **palettes_qstrings = nullptr;

    // If display does not work or looks corrupted check the
    // constructor reference:
    // https://github.com/olikraus/u8g2/wiki/u8x8setupcpp
    // or check the gallery:
    // https://github.com/olikraus/u8g2/wiki/gallery
  public:

    // gets called once at boot. Do all initialization that doesn't depend on
    // network here
    void setup() {
      if (type==NONE) return;
      if (!pinManager.allocatePin(sclPin)) { sclPin = -1; type = NONE; return;}
      if (!pinManager.allocatePin(sdaPin)) { pinManager.deallocatePin(sclPin); sclPin = sdaPin = -1; type = NONE; return; }
      switch (type) {
        case SSD1306:
          u8x8 = (void *) new U8X8_SSD1306_128X32_UNIVISION_HW_I2C(U8X8_PIN_NONE, sclPin, sdaPin); // Pins are Reset, SCL, SDA
          (static_cast<U8X8_SSD1306_128X32_UNIVISION_HW_I2C*>(u8x8))->begin();
          (static_cast<U8X8_SSD1306_128X32_UNIVISION_HW_I2C*>(u8x8))->setFlipMode(flip);
          (static_cast<U8X8_SSD1306_128X32_UNIVISION_HW_I2C*>(u8x8))->setContrast(contrast); //Contrast setup will help to preserve OLED lifetime. In case OLED need to be brighter increase number up to 255
          break;
        case SH1106:
          u8x8 = (void *) new U8X8_SH1106_128X64_WINSTAR_HW_I2C(U8X8_PIN_NONE, sclPin, sdaPin); // Pins are Reset, SCL, SDA
          (static_cast<U8X8_SH1106_128X64_WINSTAR_HW_I2C*>(u8x8))->begin();
          (static_cast<U8X8_SH1106_128X64_WINSTAR_HW_I2C*>(u8x8))->setFlipMode(flip);
          (static_cast<U8X8_SH1106_128X64_WINSTAR_HW_I2C*>(u8x8))->setContrast(contrast); //Contrast setup will help to preserve OLED lifetime. In case OLED need to be brighter increase number up to 255
          break;
        default:
          u8x8 = nullptr;
          type = NONE;
          return;
      }
      setPowerSave(0);
      String loading = String(F("Loading..."));
      drawString(0, 0, loading.c_str());

      ModeSortUsermod *modeSortUsermod = (ModeSortUsermod*) usermods.lookup(USERMOD_ID_MODE_SORT);
      modes_qstrings = modeSortUsermod->getModesQStrings();
      palettes_qstrings = modeSortUsermod->getPalettesQStrings();
    }

    // gets called every time WiFi is (re-)connected. Initialize own network
    // interfaces here
    void connected() {}

    /**
     * Da loop.
     */
    void loop() {
      if (millis() - lastUpdate < refreshRate) {
        return;
      }
      lastUpdate = millis();

      redraw(false);
    }

    /**
     * Wrappers for screen drawing
     */
    void drawString(uint8_t col, uint8_t row, const char *string) {
      if (type==NONE) return;
      switch (type) {
        case SSD1306:
          (static_cast<U8X8_SSD1306_128X32_UNIVISION_HW_I2C*>(u8x8))->setFont(u8x8_font_chroma48medium8_r);
          if (lineHeight==2) (static_cast<U8X8_SSD1306_128X32_UNIVISION_HW_I2C*>(u8x8))->draw1x2String(col, row, string);
          else               (static_cast<U8X8_SSD1306_128X32_UNIVISION_HW_I2C*>(u8x8))->drawString(col, row, string);
          break;
        case SH1106:
          (static_cast<U8X8_SH1106_128X64_WINSTAR_HW_I2C*>(u8x8))->setFont(u8x8_font_chroma48medium8_r);
          if (lineHeight==2) (static_cast<U8X8_SH1106_128X64_WINSTAR_HW_I2C*>(u8x8))->draw1x2String(col, row, string);
          else               (static_cast<U8X8_SH1106_128X64_WINSTAR_HW_I2C*>(u8x8))->drawString(col, row, string);
          break;
      }
    }
    void draw2x2String(uint8_t col, uint8_t row, const char *string) {
      if (type==NONE) return;
      switch (type) {
        case SSD1306:
          (static_cast<U8X8_SSD1306_128X32_UNIVISION_HW_I2C*>(u8x8))->setFont(u8x8_font_chroma48medium8_r);
          (static_cast<U8X8_SSD1306_128X32_UNIVISION_HW_I2C*>(u8x8))->draw2x2String(col, row, string);
          break;
        case SH1106:
          (static_cast<U8X8_SH1106_128X64_WINSTAR_HW_I2C*>(u8x8))->setFont(u8x8_font_chroma48medium8_r);
          (static_cast<U8X8_SH1106_128X64_WINSTAR_HW_I2C*>(u8x8))->draw2x2String(col, row, string);
          break;
      }
    }
    void drawGlyph(uint8_t col, uint8_t row, char glyph, const uint8_t *font) {
      if (type==NONE) return;
      switch (type) {
        case SSD1306:
          (static_cast<U8X8_SSD1306_128X32_UNIVISION_HW_I2C*>(u8x8))->setFont(font);
          if (lineHeight==2) (static_cast<U8X8_SSD1306_128X32_UNIVISION_HW_I2C*>(u8x8))->draw1x2Glyph(col, row, glyph);
          else               (static_cast<U8X8_SSD1306_128X32_UNIVISION_HW_I2C*>(u8x8))->drawGlyph(col, row, glyph);
          break;
        case SH1106:
          (static_cast<U8X8_SH1106_128X64_WINSTAR_HW_I2C*>(u8x8))->setFont(font);
          if (lineHeight==2) (static_cast<U8X8_SH1106_128X64_WINSTAR_HW_I2C*>(u8x8))->draw1x2Glyph(col, row, glyph);
          else               (static_cast<U8X8_SH1106_128X64_WINSTAR_HW_I2C*>(u8x8))->drawGlyph(col, row, glyph);
          break;
      }
    }
    uint8_t getCols() {
      if (type==NONE) return 255;
      switch (type) {
        case SSD1306:
          return (static_cast<U8X8_SSD1306_128X32_UNIVISION_HW_I2C*>(u8x8))->getCols();
        case SH1106:
          return (static_cast<U8X8_SH1106_128X64_WINSTAR_HW_I2C*>(u8x8))->getCols();
      }
    }
    void clear() {
      if (type==NONE) return;
      switch (type) {
        case SSD1306:
          (static_cast<U8X8_SSD1306_128X32_UNIVISION_HW_I2C*>(u8x8))->clear();
          break;
        case SH1106:
          (static_cast<U8X8_SH1106_128X64_WINSTAR_HW_I2C*>(u8x8))->clear();
          break;
      }
    }
    void setPowerSave(uint8_t save) {
      if (type==NONE) return;
      switch (type) {
        case SSD1306:
          (static_cast<U8X8_SSD1306_128X32_UNIVISION_HW_I2C*>(u8x8))->setPowerSave(save);
          break;
        case SH1106:
          (static_cast<U8X8_SH1106_128X64_WINSTAR_HW_I2C*>(u8x8))->setPowerSave(save);
          break;
      }
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
      } else if (((apActive) ? String(apSSID) : WiFi.SSID()) != knownSsid) {
        needRedraw = true;
      } else if (knownIp != (apActive ? IPAddress(4, 3, 2, 1) : WiFi.localIP())) {
        needRedraw = true;
      } else if (knownBrightness != bri) {
        needRedraw = true;
      } else if (knownEffectSpeed != effectSpeed) {
        needRedraw = true;
      } else if (knownEffectIntensity != effectIntensity) {
        needRedraw = true;
      } else if (knownMode != strip.getMode()) {
        needRedraw = true;
      } else if (knownPalette != strip.getSegment(0).palette) {
        needRedraw = true;
      }

      if (!needRedraw) {
        // Nothing to change.
        // Turn off display after 3 minutes with no change.
        if(sleepMode && !displayTurnedOff && (millis() - lastRedraw > screenTimeout)) {
          // We will still check if there is a change in redraw()
          // and turn it back on if it changed.
          sleepOrClock(true);
        } else if (displayTurnedOff && clockMode) {
          showTime();
        } else if ((millis() - lastRedraw)/1000%3 == 0) {
          // change 4th line every 3s
          switch (lineThreeType) {
            case FLD_LINE_3_BRIGHTNESS:
              setLineThreeType(FLD_LINE_3_PALETTE);
              break;
            case FLD_LINE_3_PALETTE:
              setLineThreeType(FLD_LINE_3_EFFECT_SPEED);
              break;
            case FLD_LINE_3_EFFECT_SPEED:
              setLineThreeType(FLD_LINE_3_EFFECT_INTENSITY);
              break;
            case FLD_LINE_3_EFFECT_INTENSITY:
              setLineThreeType(FLD_LINE_3_BRIGHTNESS);
              break;
            default:
              setLineThreeType(FLD_LINE_3_BRIGHTNESS);
              break;
          }
          drawLineThree();
        }
        return;
      } else
        clear();

      needRedraw = false;
      lastRedraw = millis();
      
      if (displayTurnedOff) {
        // Turn the display back on
        sleepOrClock(false);
      }

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
      knownEffectSpeed = effectSpeed;
      knownEffectIntensity = effectIntensity;

      // Do the actual drawing

      // First row with Wifi name
      String ssidString = knownSsid.substring(0, getCols() > 1 ? getCols() - 2 : 0);
      drawString(1, 0, ssidString.c_str());
      // Print `~` char to indicate that SSID is longer, than our display
      if (knownSsid.length() > getCols()) {
        drawString(getCols() - 1, 0, "~");
      }

      // Second row with IP or Psssword
      // Print password in AP mode and if led is OFF.
      if (apActive && bri == 0) {
        drawString(1, lineHeight, apPass);
      } else {
        drawString(1, lineHeight, (knownIp.toString()).c_str());
      }

      // Third row with mode name
      showCurrentEffectOrPalette(modes_qstrings[knownMode], 2);

      // Fourth row
      drawLineThree();

      drawGlyph(0, 2, 66 + (bri > 0 ? 3 : 0), u8x8_font_open_iconic_weather_2x2); // sun/moon icon
      drawGlyph(0, 0, 80, u8x8_font_open_iconic_embedded_1x1); // wifi icon
      drawGlyph(0, lineHeight, 68, u8x8_font_open_iconic_embedded_1x1); // home icon
      //if (markLineNum>1) drawGlyph(2, markLineNum*lineHeight, 66, u8x8_font_open_iconic_arrow_1x1); // arrow icon
    }

    void drawLineThree() {
      char lineBuffer[LINE_BUFFER_SIZE];
      switch(lineThreeType) {
        case FLD_LINE_3_BRIGHTNESS:
          sprintf_P(lineBuffer, PSTR("Brightness %3d"), bri);
          drawString(2, 3*lineHeight, lineBuffer);
          break;
        case FLD_LINE_3_EFFECT_SPEED:
          sprintf_P(lineBuffer, PSTR("FX Speed   %3d"), effectSpeed);
          drawString(2, 3*lineHeight, lineBuffer);
          break;
        case FLD_LINE_3_EFFECT_INTENSITY:
          sprintf_P(lineBuffer, PSTR("FX Intens. %3d"), effectIntensity);
          drawString(2, 3*lineHeight, lineBuffer);
          break;
        case FLD_LINE_3_PALETTE:
        default:
          showCurrentEffectOrPalette(palettes_qstrings[knownPalette], 3);
          break;
      }
    }

    /**
     * Display the current effect or palette (desiredEntry) 
     * on the appropriate line (row).
     * 
     * TODO: Should we cache the current effect and 
     * TODO: palette name? This seems expensive.
     */
    void showCurrentEffectOrPalette(char *qstring, uint8_t row) {
      char lineBuffer[LINE_BUFFER_SIZE];

      uint8_t printedChars = 0;
      char singleJsonSymbol;
      while (printedChars < getCols() - 2) {
        singleJsonSymbol = pgm_read_byte_near(qstring + printedChars);
        if (singleJsonSymbol == '"' || singleJsonSymbol == '\0' ) {
          break;
        }
        lineBuffer[printedChars++] = singleJsonSymbol;
      }
      for (;printedChars<getCols()-2; printedChars++) lineBuffer[printedChars]=' ';
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
     * Specify what data should be defined on line 3
     * (the last line).
     */
    void setLineThreeType(Line3Type newLineThreeType) {
      if (newLineThreeType == FLD_LINE_3_BRIGHTNESS || 
          newLineThreeType == FLD_LINE_3_EFFECT_SPEED || 
          newLineThreeType == FLD_LINE_3_EFFECT_INTENSITY || 
          newLineThreeType == FLD_LINE_3_PALETTE ||
          newLineThreeType == FLD_LINE_3_TIME) {
        lineThreeType = newLineThreeType;
      } else {
        // Unknown value
        lineThreeType = FLD_LINE_3_BRIGHTNESS;
      }
    }

    /**
     * Line 2 or 3 (last two lines) can be marked with an
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
        if (clockMode) {
          showTime();
        }
        else {
          setPowerSave(1);
        }
        displayTurnedOff = true;
      }
      else {
        if (!clockMode) {
          setPowerSave(0);
        }
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
      byte hourCurrent = hour(localTime);
      if (knownMinute == minuteCurrent && knownHour == hourCurrent) {
        // Time hasn't changed.
        return;
      }
      knownMinute = minuteCurrent;
      knownHour = hourCurrent;

      if (fullScreen) clear();

      int currentMonth = month(localTime);
      sprintf_P(lineBuffer, PSTR("%s %2d "), monthShortStr(currentMonth), day(localTime));
      if (fullScreen)
        draw2x2String(DATE_INDENT, lineHeight==1 ? 0 : lineHeight, lineBuffer);
      else
        drawString(0, lineHeight, lineBuffer);

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

      sprintf_P(lineBuffer, PSTR("%02d:%02d %s"), showHour, minuteCurrent, useAMPM ? (isAM ? "AM" : "PM") : "");
      // For time, we always use LINE_HEIGHT of 2 since
      // we are printing it big.
      if (fullScreen)
        draw2x2String(TIME_INDENT + (useAMPM ? 0 : 2), lineHeight*2, lineBuffer);
      else
        drawString(8, lineHeight, lineBuffer);
    }

    /*
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
     * Below it is shown how this could be used for e.g. a light sensor
     */
    void addToJsonInfo(JsonObject& root) {
      //JsonObject user = root["u"];
      //if (user.isNull()) user = root.createNestedObject("u");
      //JsonArray data = user.createNestedArray(F("4LineDisplay"));
      //data.add(F("Loaded."));
    }

    /*
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void addToJsonState(JsonObject& root) {
    }

    /*
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void readFromJsonState(JsonObject& root) {
      DisplayType newType = type;
      uint8_t newScl = sclPin;
      uint8_t newSda = sdaPin;
      uint8_t newContrast = contrast;
      bool newFlip = flip;

      if (root[F("4LineDisplay_type")] != nullptr) newType = (DisplayType)root[F("4LineDisplay_type")];
      if (root[F("4LineDisplay_pin")] != nullptr) {
        newScl = (int)root[F("4LineDisplay_pin")][0];
        newSda = (int)root[F("4LineDisplay_pin")][1];
      }
      if (root[F("4LineDisplay_contrast")] != nullptr)      newContrast = (int)root[F("4LineDisplay_contrast")];
      if (root[F("4LineDisplay_refreshRate")] != nullptr)   refreshRate = (int)root[F("4LineDisplay_refreshRate")]*1000;
      if (root[F("4LineDisplay_screenTimeOut")] != nullptr) screenTimeout = (int)root[F("4LineDisplay_screenTimeOut")]*1000;
      if (root[F("4LineDisplay_flip")] != nullptr) {
        String str = root[F("4LineDisplay_flip")]; // checkbox -> off or on
        newFlip = (bool)(str!="off"); // off is guaranteed to be present
      }
      if (root[F("4LineDisplay_sleepMode")] != nullptr) {
        String str = root[F("4LineDisplay_sleepMode")]; // checkbox -> off or on
        sleepMode = (bool)(str!="off"); // off is guaranteed to be present
      }
      if (root[F("4LineDisplay_clockMode")] != nullptr) {
        String str = root[F("4LineDisplay_clockMode")]; // checkbox -> off or on
        clockMode = (bool)(str!="off"); // off is guaranteed to be present
      }

      if (flip!=newFlip || contrast!=newContrast || sclPin!=newScl || sdaPin!=newSda || type!=newType) {
        if (type==SSD1306) delete (static_cast<U8X8_SSD1306_128X32_UNIVISION_HW_I2C*>(u8x8));
        if (type==SH1106)  delete (static_cast<U8X8_SH1106_128X64_WINSTAR_HW_I2C*>(u8x8));
        pinManager.deallocatePin(sclPin);
        pinManager.deallocatePin(sdaPin);
        sclPin = newScl;
        sdaPin = newSda;
        type = newType;
        contrast = newContrast;
        flip = newFlip;
        lineHeight = type==SH1106 ? 2 : 1;
        setup();
      }
    }

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
      JsonObject top = root.createNestedObject(F("4LineDisplay"));
      JsonArray i2c_pin = top.createNestedArray("pin");
      i2c_pin.add(sclPin);
      i2c_pin.add(sdaPin);
      top["type"] = type;
      top[F("flip")] = (bool) flip;
      top[F("contrast")] = contrast;
      top[F("refreshRate")] = refreshRate/1000;
      top[F("screenTimeOut")] = screenTimeout/1000;
      top[F("sleepMode")] = (bool) sleepMode;
      top[F("clockMode")] = (bool) clockMode;
    }

    /*
     * readFromConfig() can be used to read back the custom settings you added with addToConfig().
     * This is called by WLED when settings are loaded (currently this only happens once immediately after boot)
     * 
     * readFromConfig() is called BEFORE setup(). This means you can use your persistent values in setup() (e.g. pin assignments, buffer sizes),
     * but also that if you want to write persistent values to a dynamic buffer, you'd need to allocate it here instead of in setup.
     * If you don't know what that is, don't fret. It most likely doesn't affect your use case :)
     */
    void readFromConfig(JsonObject& root) {
      JsonObject top = root[F("4LineDisplay")];
      sclPin = top["pin"][0] | FLD_PIN_SCL;
      sdaPin = top["pin"][1] | FLD_PIN_SDA;
      type = top["type"] | SSD1306;
      lineHeight = type==SH1106 ? 2 : 1;
      flip = top[F("flip")] | false ;
      contrast = top[F("contrast")] | 10;
      refreshRate = int(top[F("refreshRate")])*1000 | USER_LOOP_REFRESH_RATE_MS;
      screenTimeout = int(top[F("screenTimeOut")])*1000 | SCREEN_TIMEOUT_MS;
      sleepMode = top[F("sleepMode")] | true;
      clockMode = top[F("clockMode")] | false;
    }

    /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
    uint16_t getId() {
      return USERMOD_ID_FOUR_LINE_DISP;
    }

};