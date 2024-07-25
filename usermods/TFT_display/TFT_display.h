// Credits to @mrVanboy, @gwaland and my dearest friend @westward
// Also for @spiff72 for usermod TTGO-T-Display
#pragma once

#include "wled.h"
#include <TFT_eSPI.h>

#ifndef USER_SETUP_LOADED
  #ifndef TFT_WIDTH
    #error Please define TFT_WIDTH
  #endif
  #ifndef TFT_HEIGHT
    #error Please define TFT_HEIGHT
  #endif
  #ifndef TFT_DC
    #error Please define TFT_DC
  #endif
  #ifndef TFT_RST
    #error Please define TFT_RST
  #endif
  #ifndef LOAD_GLCD
    #error Please define LOAD_GLCD
  #endif
#endif
#ifndef TFT_BL
  #define TFT_BL -1
#endif
// Set this parameter to rotate the display. 1-3 rotate by 90,180,270 degrees.
#ifndef USERMOD_TFT_DISPLAY_ROTATION
  #define USERMOD_TFT_DISPLAY_ROTATION 0
#endif
// Ideally different sized displays would have their own layouts.
// This mod is only tested with 240x240 and 128x128 displays, so this
// simple rescaling is sufficient.
#ifndef USERMOD_TFT_DISPLAY_SCALE
  #if TFT_WIDTH < 200
    #define USERMOD_TFT_DISPLAY_SCALE 1
  #else
    #define USERMOD_TFT_DISPLAY_SCALE 2
  #endif
#endif

// How often we are redrawing screen
#ifndef USERMOD_TFT_DISPLAY_REFRESH_RATE_MS
  #define USERMOD_TFT_DISPLAY_REFRESH_RATE_MS 1000
#endif

// Time with no updates before screen turns off (-1 to disable)
#ifndef USERMOD_TFT_DISPLAY_TIMEOUT_MS
  #define USERMOD_TFT_DISPLAY_TIMEOUT_MS 5*60*1000
#endif

extern int getSignalQuality(int rssi);

TFT_eSPI tft = TFT_eSPI(TFT_WIDTH, TFT_HEIGHT);  // Invoke custom library

class TFTDisplayUsermod : public Usermod {
 private:
  unsigned long lastTime = 0;
  bool enabled = true;

  long lastRedraw = 0;
  // Next variables hold the previous known values to determine if redraw is
  // required.
  String knownSsid = "";
  IPAddress knownIp;
  uint8_t knownBrightness = 0;
  uint8_t knownMode = 0;
  uint8_t knownPalette = 0;
  uint8_t knownEffectSpeed = 0;
  uint8_t knownEffectIntensity = 0;
  int knownMinutes = 0;

  // Settings
  unsigned font_size = USERMOD_TFT_DISPLAY_SCALE;
  unsigned rotation = USERMOD_TFT_DISPLAY_ROTATION;

  // Number of chars that fit on screen with text size set to `font_size`
  static constexpr size_t TFT_CHAR_WIDTH = 19;
  // Extra char (+1) for null
  static constexpr size_t LINE_BUFFER_SIZE = TFT_CHAR_WIDTH + 1;  

  long lastUpdate = 0;

  // Set the pin to turn the backlight on or off if available.
  static void EnableBacklight(bool enable) {
#if TFT_BL > 0
  #if USERMOD_TFT_DISPLAY_BL_ACTIVE_LOW
    enable = !enable;
  #endif
    digitalWrite(TFT_BL, enable);
#endif
  }

  static void center(String& line, uint8_t width) {
    int len = line.length();
    if (len < width)
      for (byte i = (width - len) / 2; i > 0; i--) line = ' ' + line;
    for (byte i = line.length(); i < width; i++) line += ' ';
  }

  // Make sure the next update redraws the screen.
  void ForceRedraw() {
    knownSsid = "";
    lastUpdate = 0;
  }

  // NOTE: THIS MOD DOES NOT SUPPORT CHANGING THE SPI PINS FROM THE UI! The
  // TFT_eSPI library requires that they are compiled in.
  static void SetSPIPinsFromMacros() {
    spi_mosi = TFT_MOSI;
    // Done in TFT library.
    if (TFT_MISO == TFT_MOSI) {
      spi_miso = -1;
    }
    spi_sclk = TFT_SCLK;
  }

  /**
   * Display the current date and time in large characters
   * on the middle rows. Based 24 or 12 hour depending on
   * the useAMPM configuration.
   */
  void showTime() {
    if (!ntpEnabled) return;
    char lineBuffer[LINE_BUFFER_SIZE];

    int minuteCurrent = minute(localTime);
    int hourCurrent = hour(localTime);
    int currentMonth = month(localTime);
    sprintf_P(lineBuffer, PSTR("%s %2d "), monthShortStr(currentMonth),
              day(localTime));
    tft.setTextColor(TFT_SILVER);
    tft.setCursor(84, 0);
    tft.setTextSize(font_size);
    tft.print(lineBuffer);

    int showHour = hourCurrent;
    boolean isAM = false;
    if (useAMPM) {
      if (showHour == 0) {
        showHour = 12;
        isAM = true;
      } else if (showHour > 12) {
        showHour -= 12;
        isAM = false;
      } else {
        isAM = true;
      }
    }

    sprintf_P(lineBuffer, PSTR("%2d:%02d"), (useAMPM ? showHour : hourCurrent),
              minuteCurrent);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2 * font_size);
    tft.setCursor(60, 24);
    tft.print(lineBuffer);

    tft.setTextSize(font_size);
    tft.setCursor(186, 24);
    if (useAMPM) tft.print(isAM ? "AM" : "PM");
  }

 public:
  // Functions called by WLED

  /*
   * setup() is called once at boot. WiFi is not yet connected at this point.
   * You can use it to initialize variables, sensors or similar.
   */
  void setup() override {
    DEBUG_PRINTLN(F("Usermod TFT Display init"));
    SetSPIPinsFromMacros();
    PinManagerPinType spiPins[] = {
        {spi_mosi, true}, {spi_miso, false}, {spi_sclk, true}};
    if (!pinManager.allocateMultiplePins(spiPins, 3, PinOwner::HW_SPI)) {
      enabled = false;
    } else {
      PinManagerPinType displayPins[] = {
          {TFT_CS, true}, {TFT_DC, true}, {TFT_RST, true}, {TFT_BL, true}};
      if (!pinManager.allocateMultiplePins(
              displayPins, sizeof(displayPins) / sizeof(PinManagerPinType),
              PinOwner::UM_FourLineDisplay)) {
        pinManager.deallocateMultiplePins(spiPins, 3, PinOwner::HW_SPI);
        enabled = false;
      }
    }

    if (!enabled) {
      DEBUG_PRINTLN(F("Usermod TFT Display pin allocations failed."));
      return;
    }

    tft.init();
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_RED);
    tft.setCursor(60, 100);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(2 * font_size);
    tft.print("Loading...");
    EnableBacklight(true);
  }

  /*
   * connected() is called every time the WiFi is (re)connected
   * Use it to initialize network interfaces
   */
  void connected() override {
    // Serial.println("Connected to WiFi!");
  }

  /*
   * loop() is called continuously. Here you can check for events, read sensors,
   * etc.
   *
   * Tips:
   * 1. You can use "if (WLED_CONNECTED)" to check for a successful network
   * connection. Additionally, "if (WLED_MQTT_CONNECTED)" is available to check
   * for a connection to an MQTT broker.
   *
   * 2. Try to avoid using the delay() function. NEVER use delays longer than 10
   * milliseconds. Instead, use a timer check as shown here.
   */
  void loop() override {
    char buff[LINE_BUFFER_SIZE];

    // Check if we time interval for redrawing passes.
    if (millis() - lastUpdate < USERMOD_TFT_DISPLAY_REFRESH_RATE_MS) {
      return;
    }
    lastUpdate = millis();

    // Turn off display after its timeout occurs (5min by default).
    if (USERMOD_TFT_DISPLAY_TIMEOUT_MS > 0 &&
        millis() - lastRedraw > USERMOD_TFT_DISPLAY_TIMEOUT_MS) {
      EnableBacklight(false);
    }

    bool timeChanged = false;
    if (ntpEnabled) {
      updateLocalTime();
      int minuteCurrent = minute(localTime);
      if (minuteCurrent != knownMinutes) {
        timeChanged = true;
        knownMinutes = minuteCurrent;
      }
    }

    // Check if values which are shown on display changed from the last time.
    if (!((((apActive) ? String(apSSID) : WiFi.SSID()) != knownSsid) ||
          (knownIp != (apActive ? IPAddress(4, 3, 2, 1) : Network.localIP())) ||
          (knownBrightness != bri) ||
          (knownEffectSpeed != strip.getMainSegment().speed) ||
          (knownEffectIntensity != strip.getMainSegment().intensity) ||
          (knownMode != strip.getMainSegment().mode) ||
          (knownPalette != strip.getMainSegment().palette) || timeChanged)) {
      return;
    }

    EnableBacklight(true);
    tft.setRotation(rotation);
    lastRedraw = millis();

    knownSsid = apActive ? WiFi.softAPSSID() : WiFi.SSID();
    knownIp = apActive ? IPAddress(4, 3, 2, 1) : WiFi.localIP();
    knownBrightness = bri;
    knownMode = strip.getMainSegment().mode;
    knownPalette = strip.getMainSegment().palette;
    knownEffectSpeed = strip.getMainSegment().speed;
    knownEffectIntensity = strip.getMainSegment().intensity;

    tft.fillScreen(TFT_BLACK);

    showTime();

    tft.setTextSize(font_size);

    // Wifi name
    tft.setTextColor(TFT_GREEN);
    tft.setCursor(0, 60);
    String line = knownSsid.substring(0, TFT_CHAR_WIDTH - 1);
    // Print `~` char to indicate that SSID is longer, than our display
    if (knownSsid.length() > TFT_CHAR_WIDTH)
      line = line.substring(0, TFT_CHAR_WIDTH - 1) + '~';
    center(line, TFT_CHAR_WIDTH);
    tft.print(line.c_str());

    // Print AP IP and password in AP mode or knownIP if AP not active.
    if (apActive) {
      tft.setCursor(0, 84);
      tft.print("AP IP: ");
      tft.print(knownIp);
      tft.setCursor(0, 108);
      tft.print("AP Pass:");
      tft.print(apPass);
    } else {
      tft.setCursor(0, 84);
      line = knownIp.toString();
      center(line, TFT_CHAR_WIDTH);
      tft.print(line.c_str());
      // percent brightness
      tft.setCursor(0, 120);
      tft.setTextColor(TFT_WHITE);
      tft.print("Bri: ");
      tft.print((((int)bri * 100) / 255));
      tft.print("%");
      // signal quality
      tft.setCursor(124, 120);
      tft.print("Sig: ");
      if (getSignalQuality(WiFi.RSSI()) < 10) {
        tft.setTextColor(TFT_RED);
      } else if (getSignalQuality(WiFi.RSSI()) < 25) {
        tft.setTextColor(TFT_ORANGE);
      } else {
        tft.setTextColor(TFT_GREEN);
      }
      tft.print(getSignalQuality(WiFi.RSSI()));
      tft.setTextColor(TFT_WHITE);
      tft.print("%");
    }

    // mode name
    tft.setTextColor(TFT_CYAN);
    tft.setCursor(0, 144);
    char lineBuffer[TFT_CHAR_WIDTH + 1];
    extractModeName(knownMode, JSON_mode_names, lineBuffer, TFT_CHAR_WIDTH);
    tft.print(lineBuffer);

    // palette name
    tft.setTextColor(TFT_YELLOW);
    tft.setCursor(0, 168);
    extractModeName(knownPalette, JSON_palette_names, lineBuffer, TFT_CHAR_WIDTH);
    tft.print(lineBuffer);

    tft.setCursor(0, 192);
    tft.setTextColor(TFT_SILVER);
    sprintf_P(buff, PSTR("FX  Spd:%3d Int:%3d"), effectSpeed, effectIntensity);
    tft.print(buff);

    // Fifth row with estimated mA usage
    tft.setTextColor(TFT_SILVER);
    tft.setCursor(0, 216);
    // Print estimated milliamp usage (must specify the LED type in LED prefs
    // for this to be a reasonable estimate).
    tft.print("Current: ");
    tft.setTextColor(TFT_ORANGE);
    tft.print(BusManager::currentMilliamps());
    tft.print("mA");
  }

  /*
   * addToJsonInfo() can be used to add custom entries to the /json/info part of
   * the JSON API. Creating an "u" object allows you to add custom key/value
   * pairs to the Info section of the WLED web UI. Below it is shown how this
   * could be used for e.g. a light sensor
   */
  void addToJsonInfo(JsonObject& root) override {
    JsonObject user = root["u"];
    if (user.isNull()) user = root.createNestedObject("u");

    JsonArray lightArr = user.createNestedArray("TFT");      // name
    lightArr.add(enabled ? F("installed") : F("disabled"));  // unit
  }

  /*
   * addToJsonState() can be used to add custom entries to the /json/state part
   * of the JSON API (state object). Values in the state object may be modified
   * by connected clients
   */
  void addToJsonState(JsonObject& root) override {
    // root["user0"] = userVar0;
  }

  /*
   * readFromJsonState() can be used to receive data clients send to the
   * /json/state part of the JSON API (state object). Values in the state object
   * may be modified by connected clients
   */
  void readFromJsonState(JsonObject& root) override {
    // userVar0 = root["user0"] | userVar0; //if "user0" key exists in JSON,
    // update, else keep old value if (root["bri"] == 255)
    // Serial.println(F("Don't burn down your garage!"));
  }

  /*
   * addToConfig() can be used to add custom persistent settings to the cfg.json
   * file in the "um" (usermod) object. It will be called by WLED when settings
   * are actually saved (for example, LED settings are saved) If you want to
   * force saving the current state, use serializeConfig() in your loop().
   *
   * CAUTION: serializeConfig() will initiate a filesystem write operation.
   * It might cause the LEDs to stutter and will cause flash wear if called too
   * often. Use it sparingly and always in the loop, never in network callbacks!
   *
   * addToConfig() will also not yet add your setting to one of the settings
   * pages automatically. To make that work you still have to add the setting to
   * the HTML, xml.cpp and set.cpp manually.
   *
   * I highly recommend checking out the basics of ArduinoJson serialization and
   * deserialization in order to use custom settings!
   */
  void addToConfig(JsonObject& root) override {
    JsonObject top = root.createNestedObject("TFT");
    top["rotation"] = rotation;
    top["font_size"] = font_size;
    JsonArray pins = top.createNestedArray("pin");
    pins.add(TFT_CS);
    pins.add(TFT_DC);
    pins.add(TFT_RST);
    pins.add(TFT_BL);
  }

  void appendConfigData() override {
    oappend(SET_F("dd=addDropdown('TFT','rotation');"));
    oappend(SET_F("addOption(dd,'0 deg',0);"));
    oappend(SET_F("addOption(dd,'90 deg',1);"));
    oappend(SET_F("addOption(dd,'180 deg',2);"));
    oappend(SET_F("addOption(dd,'270 deg',3);"));
    oappend(
        SET_F("addInfo('TFT:font_size',1,'<br><i class=\"warn\">DO NOT CHANGE "
              "SPI PINS ABOVE OR BELOW.</i><br><i class=\"warn\">CHANGES ARE "
              "IGNORED EXCEPT FOR CHECKING PIN CONFLICTS.</i>','');"));
    oappend(SET_F("addInfo('TFT:pin[]',0,'','SPI CS');"));
    oappend(SET_F("addInfo('TFT:pin[]',1,'','SPI DC');"));
    oappend(SET_F("addInfo('TFT:pin[]',2,'','SPI RST');"));
    oappend(SET_F("addInfo('TFT:pin[]',3,'','SPI BL');"));
  }

  /*
   * readFromConfig() can be used to read back the custom settings you added
   * with addToConfig(). This is called by WLED when settings are loaded
   * (currently this only happens once immediately after boot)
   *
   * readFromConfig() is called BEFORE setup(). This means you can use your
   * persistent values in setup() (e.g. pin assignments, buffer sizes), but also
   * that if you want to write persistent values to a dynamic buffer, you'd need
   * to allocate it here instead of in setup. If you don't know what that is,
   * don't fret. It most likely doesn't affect your use case :)
   */
  bool readFromConfig(JsonObject& root) override {
    // we look for JSON object:
    // {"TFT":{"rotation":0,"font_size":1}}
    JsonObject top = root["TFT"];
    if (top.isNull()) {
      DEBUG_PRINTLN(F("TFT: No config found. (Using defaults.)"));
      return false;
    }
    unsigned new_rotation = min(top["rotation"] | rotation, 3u);
    unsigned new_font_size = max(top["font_size"] | font_size, 1u);

    // Restore the SPI pins to their compiled in defaults.
    SetSPIPinsFromMacros();

    if (new_rotation != rotation || font_size != new_font_size) {
      rotation = new_rotation;
      font_size = new_font_size;
      ForceRedraw();
    }

    // use "return !top["newestParameter"].isNull();" when updating Usermod with
    // new features
    return !top["TFT"].isNull();
  }

  /*
   * getId() allows you to optionally give your V2 usermod an unique ID (please
   * define it in const.h!). This could be used in the future for the system to
   * determine whether your usermod is installed.
   */
  uint16_t getId() { return USERMOD_ID_TFT_DISPLAY; }

  // More methods can be added in the future, this example will then be
  // extended. Your usermod will remain compatible as it does not need to
  // implement all methods from the Usermod base class!
};
