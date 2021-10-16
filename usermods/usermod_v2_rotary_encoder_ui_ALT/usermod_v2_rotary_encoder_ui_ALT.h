#pragma once

#include "wled.h"

//
// Inspired by the original v2 usermods
// * usermod_v2_rotaty_encoder_ui
//
// v2 usermod that provides a rotary encoder-based UI.
//
// This usermod allows you to control:
// 
// * Brightness
// * Selected Effect
// * Effect Speed
// * Effect Intensity
// * Palette
//
// Change between modes by pressing a button.
//
// Dependencies
// * This usermod REQURES the ModeSortUsermod
// * This Usermod works best coupled with 
//   FourLineDisplayUsermod.
//
// If FourLineDisplayUsermod is used the folowing options are also inabled
//
// * main color
// * saturation of main color
// * display network (long press buttion)
//

#ifndef ENCODER_DT_PIN
#define ENCODER_DT_PIN 18
#endif

#ifndef ENCODER_CLK_PIN
#define ENCODER_CLK_PIN 5
#endif

#ifndef ENCODER_SW_PIN
#define ENCODER_SW_PIN 19
#endif

// The last UI state, remove color and saturation option if diplay not active(too many options)
#ifdef USERMOD_FOUR_LINE_DISPLAY
 #define LAST_UI_STATE 6
#else
 #define LAST_UI_STATE 4
#endif


class RotaryEncoderUIUsermod : public Usermod {
private:
  int fadeAmount = 5;             // Amount to change every step (brightness)
  unsigned long currentTime;
  unsigned long loopTime;
  unsigned long buttonHoldTIme;
  int8_t pinA = ENCODER_DT_PIN;       // DT from encoder
  int8_t pinB = ENCODER_CLK_PIN;      // CLK from encoder
  int8_t pinC = ENCODER_SW_PIN;       // SW from encoder
  unsigned char select_state = 0;     // 0: brightness, 1: effect, 2: effect speed
  unsigned char button_state = HIGH;
  unsigned char prev_button_state = HIGH;
  bool networkShown = false;
  uint16_t currentHue1 = 6425; // default reboot color
  byte currentSat1 = 255; 
  
#ifdef USERMOD_FOUR_LINE_DISPLAY
  FourLineDisplayUsermod *display;
#else
  void* display = nullptr;
#endif

  byte *modes_alpha_indexes = nullptr;
  byte *palettes_alpha_indexes = nullptr;

  unsigned char Enc_A;
  unsigned char Enc_B;
  unsigned char Enc_A_prev = 0;

  bool currentEffectAndPaletteInitialized = false;
  uint8_t effectCurrentIndex = 0;
  uint8_t effectPaletteIndex = 0;
  uint8_t knownMode = 0;
  uint8_t knownPalette = 0;

  bool initDone = false;
  bool enabled = true;

  // strings to reduce flash memory usage (used more than twice)
  static const char _name[];
  static const char _enabled[];
  static const char _DT_pin[];
  static const char _CLK_pin[];
  static const char _SW_pin[];

public:
  /*
     * setup() is called once at boot. WiFi is not yet connected at this point.
     * You can use it to initialize variables, sensors or similar.
     */
  void setup()
  {
    PinManagerPinType pins[3] = { { pinA, false }, { pinB, false }, { pinC, false } };
    if (!pinManager.allocateMultiplePins(pins, 3, PinOwner::UM_RotaryEncoderUI)) {
      // BUG: configuring this usermod with conflicting pins
      //      will cause it to de-allocate pins it does not own
      //      (at second config)
      //      This is the exact type of bug solved by pinManager
      //      tracking the owner tags....
      pinA = pinB = pinC = -1;
      enabled = false;
      return;
    }

    pinMode(pinA, INPUT_PULLUP);
    pinMode(pinB, INPUT_PULLUP);
    pinMode(pinC, INPUT_PULLUP);
    currentTime = millis();
    loopTime = currentTime;

    ModeSortUsermod *modeSortUsermod = (ModeSortUsermod*) usermods.lookup(USERMOD_ID_MODE_SORT);
    modes_alpha_indexes = modeSortUsermod->getModesAlphaIndexes();
    palettes_alpha_indexes = modeSortUsermod->getPalettesAlphaIndexes();

#ifdef USERMOD_FOUR_LINE_DISPLAY    
    // This Usermod uses FourLineDisplayUsermod for the best experience.
    // But it's optional. But you want it.
    display = (FourLineDisplayUsermod*) usermods.lookup(USERMOD_ID_FOUR_LINE_DISP);
    if (display != nullptr) {
      display->setMarkLine(1, 0);
    }
#endif

    initDone = true;
    Enc_A = digitalRead(pinA); // Read encoder pins
    Enc_B = digitalRead(pinB);
    Enc_A_prev = Enc_A;
  }

  /*
     * connected() is called every time the WiFi is (re)connected
     * Use it to initialize network interfaces
     */
  void connected()
  {
    //Serial.println("Connected to WiFi!");
  }

  /*
     * loop() is called continuously. Here you can check for events, read sensors, etc.
     * 
     * Tips:
     * 1. You can use "if (WLED_CONNECTED)" to check for a successful network connection.
     *    Additionally, "if (WLED_MQTT_CONNECTED)" is available to check for a connection to an MQTT broker.
     * 
     * 2. Try to avoid using the delay() function. NEVER use delays longer than 10 milliseconds.
     *    Instead, use a timer check as shown here.
     */
  void loop()
  {
    currentTime = millis(); // get the current elapsed time

    // Initialize effectCurrentIndex and effectPaletteIndex to
    // current state. We do it here as (at least) effectCurrent
    // is not yet initialized when setup is called.
    
    if (!currentEffectAndPaletteInitialized) {
      findCurrentEffectAndPalette();}

    if(modes_alpha_indexes[effectCurrentIndex] != effectCurrent 
    || palettes_alpha_indexes[effectPaletteIndex] != effectPalette){
      currentEffectAndPaletteInitialized = false;
      }

    if (currentTime >= (loopTime + 2)) // 2ms since last check of encoder = 500Hz
    {
      button_state = digitalRead(pinC);
      if (prev_button_state != button_state)
      {
        if (button_state == HIGH && (millis()-buttonHoldTIme < 3000))
        {
          prev_button_state = button_state;

          char newState = select_state + 1;
          if (newState > LAST_UI_STATE) newState = 0;
          
          bool changedState = true;
          if (display != nullptr) {
            switch(newState) {
              case 0:
                changedState = changeState("   Brightness", 1, 0, 1);
                break;
              case 1:
                changedState = changeState("     Speed", 1, 4, 2);
                break;
              case 2:
                changedState = changeState("    Intensity", 1 ,8, 3);
                break;
              case 3:
                changedState = changeState("  Color Palette", 2, 0, 4);
                break;
              case 4:
                changedState = changeState("     Effect", 3, 0, 5);
                break;
              case 5:
                changedState = changeState("   Main Color", 255, 255, 7);
                break;
              case 6:
                changedState = changeState("   Saturation", 255, 255, 8);
                break;
            }
          }
          if (changedState) {
            select_state = newState;
          }
        }
        else
        {
          prev_button_state = button_state;
          networkShown = false;
          if(!prev_button_state)buttonHoldTIme = millis();
        }
      }
      
      if (!prev_button_state && (millis()-buttonHoldTIme > 3000) && !networkShown) displayNetworkInfo(); //long press for network info

      Enc_A = digitalRead(pinA); // Read encoder pins
      Enc_B = digitalRead(pinB);
      if ((Enc_A) && (!Enc_A_prev))
      { // A has gone from high to low
        if (Enc_B == LOW)    //changes to LOW so that then encoder registers a change at the very end of a pulse
        { // B is high so clockwise
          switch(select_state) {
            case 0:
              changeBrightness(true);
              break;
            case 1:
              changeEffectSpeed(true);
              break;
            case 2:
              changeEffectIntensity(true);
              break;
            case 3:
              changePalette(true);
              break;
            case 4:
              changeEffect(true);
              break;
            case 5:
              changeHue(true);
              break;
            case 6:
              changeSat(true);
              break;
          }
        }
        else if (Enc_B == HIGH)
        { // B is low so counter-clockwise
          switch(select_state) {
            case 0:
              changeBrightness(false);
              break;
            case 1:
              changeEffectSpeed(false);
              break;
            case 2:
              changeEffectIntensity(false);
              break;
            case 3:
              changePalette(false);
              break;
            case 4:
              changeEffect(false);
              break;
            case 5:
              changeHue(false);
              break;
            case 6:
              changeSat(false);
              break;
          }
        }
      }
      Enc_A_prev = Enc_A;     // Store value of A for next time
      loopTime = currentTime; // Updates loopTime
    }
  }

  void displayNetworkInfo(){
    #ifdef USERMOD_FOUR_LINE_DISPLAY
    display->networkOverlay("  NETWORK INFO", 15000);
    networkShown = true;
    #endif
  }

  void findCurrentEffectAndPalette() {
    currentEffectAndPaletteInitialized = true;
    for (uint8_t i = 0; i < strip.getModeCount(); i++) {
      if (modes_alpha_indexes[i] == effectCurrent) {
        effectCurrentIndex = i;
        break;
      }
    }

    for (uint8_t i = 0; i < strip.getPaletteCount(); i++) {
      if (palettes_alpha_indexes[i] == effectPalette) {
        effectPaletteIndex = i;
        break;
      }
    }
  }

  boolean changeState(const char *stateName, byte markedLine, byte markedCol, byte glyph) {
        #ifdef USERMOD_FOUR_LINE_DISPLAY
            if (display != nullptr) {
              if (display->wakeDisplay()) {
                // Throw away wake up input
                return false;
              }
              display->overlay(stateName, 750, glyph);
              display->setMarkLine(markedLine, markedCol);
            }
          #endif
            return true;
  }

  void lampUdated() {
    //bool fxChanged = strip.setEffectConfig(effectCurrent, effectSpeed, effectIntensity, effectPalette);
    //call for notifier -> 0: init 1: direct change 2: button 3: notification 4: nightlight 5: other (No notification)
    // 6: fx changed 7: hue 8: preset cycle 9: blynk 10: alexa
    colorUpdated(CALL_MODE_DIRECT_CHANGE);
    updateInterfaces(CALL_MODE_DIRECT_CHANGE);
  }

  void changeBrightness(bool increase) {
        #ifdef USERMOD_FOUR_LINE_DISPLAY
            if (display && display->wakeDisplay()) {
              // Throw away wake up input
              return;
            }
        #endif
            if (increase) bri = (bri + fadeAmount <= 255) ? (bri + fadeAmount) : 255;
            else bri = (bri - fadeAmount >= 0) ? (bri - fadeAmount) : 0;
            lampUdated();
            #ifdef USERMOD_FOUR_LINE_DISPLAY
            display->updateBrightness();
            #endif
  }


  void changeEffect(bool increase) {
        #ifdef USERMOD_FOUR_LINE_DISPLAY
            if (display && display->wakeDisplay()) {
              // Throw away wake up input
              return;
            }
        #endif
            if (increase) effectCurrentIndex = (effectCurrentIndex + 1 >= strip.getModeCount()) ? 0 : (effectCurrentIndex + 1);
            else effectCurrentIndex = (effectCurrentIndex - 1 < 0) ? (strip.getModeCount() - 1) : (effectCurrentIndex - 1);
            effectCurrent = modes_alpha_indexes[effectCurrentIndex];
            lampUdated();
            #ifdef USERMOD_FOUR_LINE_DISPLAY
            display->showCurrentEffectOrPalette(effectCurrent, JSON_mode_names, 3);
            #endif
  }


  void changeEffectSpeed(bool increase) {
        #ifdef USERMOD_FOUR_LINE_DISPLAY
            if (display && display->wakeDisplay()) {
              // Throw away wake up input
              return;
            }
        #endif
            if (increase) effectSpeed = (effectSpeed + fadeAmount <= 255) ? (effectSpeed + fadeAmount) : 255;
            else effectSpeed = (effectSpeed - fadeAmount >= 0) ? (effectSpeed - fadeAmount) : 0;
            lampUdated();
            #ifdef USERMOD_FOUR_LINE_DISPLAY
            display->updateSpeed();
            #endif
  }


  void changeEffectIntensity(bool increase) {
        #ifdef USERMOD_FOUR_LINE_DISPLAY
            if (display && display->wakeDisplay()) {
              // Throw away wake up input
              return;
            }
        #endif
            if (increase) effectIntensity = (effectIntensity + fadeAmount <= 255) ? (effectIntensity + fadeAmount) : 255;
            else effectIntensity = (effectIntensity - fadeAmount >= 0) ? (effectIntensity - fadeAmount) : 0;
            lampUdated();
            #ifdef USERMOD_FOUR_LINE_DISPLAY
            display->updateIntensity();
            #endif
  }


  void changePalette(bool increase) {
        #ifdef USERMOD_FOUR_LINE_DISPLAY
            if (display && display->wakeDisplay()) {
              // Throw away wake up input
              return;
            }
        #endif
            if (increase) effectPaletteIndex = (effectPaletteIndex + 1 >= strip.getPaletteCount()) ? 0 : (effectPaletteIndex + 1);
            else effectPaletteIndex = (effectPaletteIndex - 1 < 0) ? (strip.getPaletteCount() - 1) : (effectPaletteIndex - 1);
            effectPalette = palettes_alpha_indexes[effectPaletteIndex];
            lampUdated();
            #ifdef USERMOD_FOUR_LINE_DISPLAY
            display->showCurrentEffectOrPalette(effectPalette, JSON_palette_names, 2);
            #endif
  }


  void changeHue(bool increase){
        #ifdef USERMOD_FOUR_LINE_DISPLAY
            if (display && display->wakeDisplay()) {
              // Throw away wake up input
              return;
            }
        #endif

        if(increase) currentHue1 += 321;
        else currentHue1 -= 321;
        colorHStoRGB(currentHue1, currentSat1, col);
        lampUdated();
        #ifdef USERMOD_FOUR_LINE_DISPLAY
        display->updateRedrawTime();
        #endif
  }

  void changeSat(bool increase){
        #ifdef USERMOD_FOUR_LINE_DISPLAY
            if (display && display->wakeDisplay()) {
              // Throw away wake up input
              return;
            }
        #endif

        if(increase) currentSat1 = (currentSat1 + 5 <= 255 ? (currentSat1 + 5) : 255);
        else currentSat1 = (currentSat1 - 5 >= 0 ? (currentSat1 - 5) : 0);
        colorHStoRGB(currentHue1, currentSat1, col);
        lampUdated();
        #ifdef USERMOD_FOUR_LINE_DISPLAY
        display->updateRedrawTime();
        #endif

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

  /*
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
  void addToJsonState(JsonObject &root)
  {
    //root["user0"] = userVar0;
  }

  /*
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
  void readFromJsonState(JsonObject &root)
  {
    //userVar0 = root["user0"] | userVar0; //if "user0" key exists in JSON, update, else keep old value
    //if (root["bri"] == 255) Serial.println(F("Don't burn down your garage!"));
  }

  /**
   * addToConfig() (called from set.cpp) stores persistent properties to cfg.json
   */
  void addToConfig(JsonObject &root) {
    // we add JSON object: {"Rotary-Encoder":{"DT-pin":12,"CLK-pin":14,"SW-pin":13}}
    JsonObject top = root.createNestedObject(FPSTR(_name)); // usermodname
    top[FPSTR(_enabled)] = enabled;
    top[FPSTR(_DT_pin)]  = pinA;
    top[FPSTR(_CLK_pin)] = pinB;
    top[FPSTR(_SW_pin)]  = pinC;
    DEBUG_PRINTLN(F("Rotary Encoder config saved."));
  }

  /**
   * readFromConfig() is called before setup() to populate properties from values stored in cfg.json
   *
   * The function should return true if configuration was successfully loaded or false if there was no configuration.
   */
  bool readFromConfig(JsonObject &root) {
    // we look for JSON object: {"Rotary-Encoder":{"DT-pin":12,"CLK-pin":14,"SW-pin":13}}
    JsonObject top = root[FPSTR(_name)];
    if (top.isNull()) {
      DEBUG_PRINT(FPSTR(_name));
      DEBUG_PRINTLN(F(": No config found. (Using defaults.)"));
      return false;
    }
    int8_t newDTpin  = pinA;
    int8_t newCLKpin = pinB;
    int8_t newSWpin  = pinC;

    enabled   = top[FPSTR(_enabled)] | enabled;
    newDTpin  = top[FPSTR(_DT_pin)]  | newDTpin;
    newCLKpin = top[FPSTR(_CLK_pin)] | newCLKpin;
    newSWpin  = top[FPSTR(_SW_pin)]  | newSWpin;

    DEBUG_PRINT(FPSTR(_name));
    if (!initDone) {
      // first run: reading from cfg.json
      pinA = newDTpin;
      pinB = newCLKpin;
      pinC = newSWpin;
      DEBUG_PRINTLN(F(" config loaded."));
    } else {
      DEBUG_PRINTLN(F(" config (re)loaded."));
      // changing parameters from settings page
      if (pinA!=newDTpin || pinB!=newCLKpin || pinC!=newSWpin) {
        pinManager.deallocatePin(pinA, PinOwner::UM_RotaryEncoderUI);
        pinManager.deallocatePin(pinB, PinOwner::UM_RotaryEncoderUI);
        pinManager.deallocatePin(pinC, PinOwner::UM_RotaryEncoderUI);
        pinA = newDTpin;
        pinB = newCLKpin;
        pinC = newSWpin;
        if (pinA<0 || pinB<0 || pinC<0) {
          enabled = false;
          return true;
        }
        setup();
      }
    }
    // use "return !top["newestParameter"].isNull();" when updating Usermod with new features
    return !top[FPSTR(_enabled)].isNull();
  }

  /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
  uint16_t getId()
  {
    return USERMOD_ID_ROTARY_ENC_UI;
  }
};

// strings to reduce flash memory usage (used more than twice)
const char RotaryEncoderUIUsermod::_name[]     PROGMEM = "Rotary-Encoder";
const char RotaryEncoderUIUsermod::_enabled[]  PROGMEM = "enabled";
const char RotaryEncoderUIUsermod::_DT_pin[]   PROGMEM = "DT-pin";
const char RotaryEncoderUIUsermod::_CLK_pin[]  PROGMEM = "CLK-pin";
const char RotaryEncoderUIUsermod::_SW_pin[]   PROGMEM = "SW-pin";
