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
// This Usermod works best, by far, when coupled with RotaryEncoderUIUsermod.
//
// Make sure to enable NTP and set your time zone in WLED Config | Time.
//
// REQUIREMENT: You must add the following requirements to
// REQUIREMENT: "lib_deps" within platformio.ini / platformio_override.ini
// REQUIREMENT: *  U8g2  (the version already in platformio.ini is fine)
// REQUIREMENT: *  Wire
//

//The SCL and SDA pins are defined here. 
#ifndef FLD_PIN_SCL
#define FLD_PIN_SCL 5
#endif

#ifndef FLD_PIN_SDA
#define FLD_PIN_SDA 4
#endif

// U8X8_SSD1306_128X32_UNIVISION_HW_I2C u8x8(
//   U8X8_PIN_NONE, FLD_PIN_SCL, FLD_PIN_SDA); 
U8X8_SH1106_128X64_WINSTAR_HW_I2C u8x8(
  U8X8_PIN_NONE, FLD_PIN_SCL, FLD_PIN_SDA); 

// Screen upside down? Change to 0 or 1
#ifndef FLIP_MODE
#define FLIP_MODE 0
#endif

// LINE_HEIGHT 1 is single height, for 128x32 displays.
// LINE_HEIGHT 2 makes the 128x64 screen display at double height.
#ifndef LINE_HEIGHT
#define LINE_HEIGHT 2
#endif

// If you aren't also including RotaryEncoderUIUsermod
// you probably want to set both
//     SLEEP_MODE_ENABLED false
//     CLOCK_MODE_ENABLED false
// as you will never be able wake the display / disable the clock.
#ifdef USERMOD_ROTARY_ENCODER_UI
#ifndef SLEEP_MODE_ENABLED
#define SLEEP_MODE_ENABLED true
#endif
#ifndef CLOCK_MODE_ENABLED
#define CLOCK_MODE_ENABLED true
#endif
#else
#define SLEEP_MODE_ENABLED false
#define CLOCK_MODE_ENABLED false
#endif

// When to time out to the clock or blank the screen
// if SLEEP_MODE_ENABLED.
#define SCREEN_TIMEOUT_MS  15*1000

#define TIME_INDENT        0
#define DATE_INDENT        2

// Minimum time between redrawing screen in ms
#define USER_LOOP_REFRESH_RATE_MS 1000

#if LINE_HEIGHT == 2
#define DRAW_STRING draw1x2String
#define DRAW_GLYPH draw1x2Glyph
#define DRAW_BIG_STRING draw2x2String
#else
#define DRAW_STRING drawString
#define DRAW_GLYPH drawGlyph
#define DRAW_BIG_STRING draw2x2String
#endif

// Extra char (+1) for null
#define LINE_BUFFER_SIZE            16+1
#define FLD_LINE_3_BRIGHTNESS       0
#define FLD_LINE_3_EFFECT_SPEED     1
#define FLD_LINE_3_EFFECT_INTENSITY 2
#define FLD_LINE_3_PALETTE          3

#if LINE_HEIGHT == 2
#define TIME_LINE  1
#else
#define TIME_LINE  0
#endif

class FourLineDisplayUsermod : public Usermod {
  private:
    unsigned long lastTime = 0;

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
    byte lineThreeType = FLD_LINE_3_BRIGHTNESS;
    // Set to 2 or 3 to mark lines 2 or 3. Other values ignored.
    byte markLineNum = 0;

    char lineBuffer[LINE_BUFFER_SIZE];

    // If display does not work or looks corrupted check the
    // constructor reference:
    // https://github.com/olikraus/u8g2/wiki/u8x8setupcpp
    // or check the gallery:
    // https://github.com/olikraus/u8g2/wiki/gallery
  public:

    // gets called once at boot. Do all initialization that doesn't depend on
    // network here
    void setup() {
      u8x8.begin();
      u8x8.setFlipMode(FLIP_MODE);
      u8x8.setPowerSave(0);
      u8x8.setContrast(10); //Contrast setup will help to preserve OLED lifetime. In case OLED need to be brighter increase number up to 255
      u8x8.setFont(u8x8_font_chroma48medium8_r);
      u8x8.DRAW_STRING(0, 0*LINE_HEIGHT, "Loading...");
    }

    // gets called every time WiFi is (re-)connected. Initialize own network
    // interfaces here
    void connected() {}

    /**
     * Da loop.
     */
    void loop() {
      if (millis() - lastUpdate < USER_LOOP_REFRESH_RATE_MS) {
        return;
      }
      lastUpdate = millis();

      redraw(false);
    }

    /**
     * Redraw the screen (but only if things have changed
     * or if forceRedraw).
     */
    void redraw(bool forceRedraw) {
      if (overlayUntil > 0) {
        if (millis() >= overlayUntil) {
          // Time to display the overlay has elapsed.
          overlayUntil = 0;
          forceRedraw = true;
        }
        else {
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
        if(SLEEP_MODE_ENABLED && !displayTurnedOff &&
            (millis() - lastRedraw > SCREEN_TIMEOUT_MS)) {
          // We will still check if there is a change in redraw()
          // and turn it back on if it changed.
          sleepOrClock(true);
        }
        else if (displayTurnedOff && CLOCK_MODE_ENABLED) {
          showTime();
        }
        return;
      }
      needRedraw = false;
      lastRedraw = millis();
      
      if (displayTurnedOff)
      {
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
      u8x8.clear();
      u8x8.setFont(u8x8_font_chroma48medium8_r);

      // First row with Wifi name
      String ssidString = knownSsid.substring(0, u8x8.getCols() > 1 ? u8x8.getCols() - 2 : 0);
      u8x8.DRAW_STRING(1, 0*LINE_HEIGHT, ssidString.c_str());
      // Print `~` char to indicate that SSID is longer, than owr dicplay
      if (knownSsid.length() > u8x8.getCols()) {
        u8x8.DRAW_STRING(u8x8.getCols() - 1, 0*LINE_HEIGHT, "~");
      }

      // Second row with IP or Psssword
      // Print password in AP mode and if led is OFF.
      if (apActive && bri == 0) {
        u8x8.DRAW_STRING(1, 1*LINE_HEIGHT, apPass);
      }
      else {
        String ipString = knownIp.toString();
        u8x8.DRAW_STRING(1, 1*LINE_HEIGHT, ipString.c_str());
      }

      // Third row with mode name
      showCurrentEffectOrPalette(JSON_mode_names, 2, knownMode);

      switch(lineThreeType) {
        case FLD_LINE_3_BRIGHTNESS:
          sprintf(lineBuffer, "Brightness %d", bri);
          u8x8.DRAW_STRING(1, 3*LINE_HEIGHT, lineBuffer);
          break;
        case FLD_LINE_3_EFFECT_SPEED:
          sprintf(lineBuffer, "FX Speed %d", effectSpeed);
          u8x8.DRAW_STRING(1, 3*LINE_HEIGHT, lineBuffer);
          break;
        case FLD_LINE_3_EFFECT_INTENSITY:
          sprintf(lineBuffer, "FX Intense %d", effectIntensity);
          u8x8.DRAW_STRING(1, 3*LINE_HEIGHT, lineBuffer);
          break;
        case FLD_LINE_3_PALETTE:
          showCurrentEffectOrPalette(JSON_palette_names, 3, knownPalette);
          break;
      }

      u8x8.setFont(u8x8_font_open_iconic_arrow_1x1);
      u8x8.DRAW_GLYPH(0, markLineNum*LINE_HEIGHT, 66); // arrow icon

      u8x8.setFont(u8x8_font_open_iconic_embedded_1x1);
      u8x8.DRAW_GLYPH(0, 0*LINE_HEIGHT, 80); // wifi icon
      u8x8.DRAW_GLYPH(0, 1*LINE_HEIGHT, 68); // home icon
    }

    /**
     * Display the current effect or palette (desiredEntry) 
     * on the appropriate line (row).
     * 
     * TODO: Should we cache the current effect and 
     * TODO: palette name? This seems expensive.
     */
    void showCurrentEffectOrPalette(const char json[], uint8_t row, uint8_t desiredEntry) {
      uint8_t qComma = 0;
      bool insideQuotes = false;
      // advance past the mark for markLineNum that may exist.
      uint8_t printedChars = 1;
      char singleJsonSymbol;

      // Find the mode name in JSON
      for (size_t i = 0; i < strlen_P(json); i++) {
        singleJsonSymbol = pgm_read_byte_near(json + i);
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
          if (!insideQuotes || (qComma != desiredEntry)) {
            break;
          }
          u8x8.DRAW_GLYPH(printedChars, row * LINE_HEIGHT, singleJsonSymbol);
          printedChars++;
        }
        if ((qComma > desiredEntry) || (printedChars > u8x8.getCols() - 2)) {
          break;
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
      u8x8.clear();
      u8x8.setFont(u8x8_font_chroma48medium8_r);
      if (line1) {
        u8x8.DRAW_STRING(0, 1*LINE_HEIGHT, line1);
      }
      if (line2) {
        u8x8.DRAW_STRING(0, 2*LINE_HEIGHT, line2);
      }
      overlayUntil = millis() + showHowLong;
    }

    /**
     * Specify what data should be defined on line 3
     * (the last line).
     */
    void setLineThreeType(byte newLineThreeType) {
      if (newLineThreeType == FLD_LINE_3_BRIGHTNESS || 
          newLineThreeType == FLD_LINE_3_EFFECT_SPEED || 
          newLineThreeType == FLD_LINE_3_EFFECT_INTENSITY || 
          newLineThreeType == FLD_LINE_3_PALETTE) {
        lineThreeType = newLineThreeType;
      }
      else {
        // Unknown value.
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

    /*
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
     * Below it is shown how this could be used for e.g. a light sensor
     */
    /*
    void addToJsonInfo(JsonObject& root)
    {
      int reading = 20;
      //this code adds "u":{"Light":[20," lux"]} to the info object
      JsonObject user = root["u"];
      if (user.isNull()) user = root.createNestedObject("u");

      JsonArray lightArr = user.createNestedArray("Light"); //name
      lightArr.add(reading); //value
      lightArr.add(" lux"); //unit
    }
    */

    /**
     * Enable sleep (turn the display off) or clock mode.
     */
    void sleepOrClock(bool enabled) {
      if (enabled) {
        if (CLOCK_MODE_ENABLED) {
          showTime();
        }
        else {
          u8x8.setPowerSave(1);
        }
        displayTurnedOff = true;
      }
      else {
        if (!CLOCK_MODE_ENABLED) {
          u8x8.setPowerSave(0);
        }
        displayTurnedOff = false;
      }
    }

    /**
     * Display the current date and time in large characters
     * on the middle rows. Based 24 or 12 hour depending on
     * the useAMPM configuration.
     */
    void showTime() {
      updateLocalTime();
      byte minuteCurrent = minute(localTime);
      byte hourCurrent = hour(localTime);
      if (knownMinute == minuteCurrent && knownHour == hourCurrent) {
        // Time hasn't changed.
        return;
      }
      knownMinute = minuteCurrent;
      knownHour = hourCurrent;

      u8x8.clear();
      u8x8.setFont(u8x8_font_chroma48medium8_r);

      int currentMonth = month(localTime);
      sprintf(lineBuffer, "%s %d", monthShortStr(currentMonth), day(localTime));
      u8x8.DRAW_BIG_STRING(DATE_INDENT, TIME_LINE*LINE_HEIGHT, lineBuffer);

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

      sprintf(lineBuffer, "%02d:%02d %s", showHour, minuteCurrent, useAMPM ? (isAM ? "AM" : "PM") : "");
      // For time, we always use LINE_HEIGHT of 2 since
      // we are printing it big.
      u8x8.DRAW_BIG_STRING(TIME_INDENT + (useAMPM ? 0 : 2), (TIME_LINE + 1) * 2, lineBuffer);
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
    }

    /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
    uint16_t getId() {
      return USERMOD_ID_FOUR_LINE_DISP;
    }

};