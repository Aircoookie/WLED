#pragma once

#include "wled.h"

//
// Inspired by the v1 usermods
// * rotary_encoder_change_brightness
// * rotary_encoder_change_effect
//
// v2 usermod that provides a rotary encoder-based UI.
//
// This Usermod works best coupled with FourLineDisplayUsermod.
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

#ifndef ENCODER_DT_PIN
#define ENCODER_DT_PIN 12
#endif

#ifndef ENCODER_CLK_PIN
#define ENCODER_CLK_PIN 14
#endif

#ifndef ENCODER_SW_PIN
#define ENCODER_SW_PIN 13
#endif

#ifndef USERMOD_FOUR_LINE_DISLAY
// These constants won't be defined if we aren't using FourLineDisplay.
#define FLD_LINE_3_BRIGHTNESS       0
#define FLD_LINE_3_EFFECT_SPEED     0
#define FLD_LINE_3_EFFECT_INTENSITY 0
#define FLD_LINE_3_PALETTE          0
#endif

// The last UI state
#define LAST_UI_STATE 4

/**
 * Array of mode indexes in alphabetical order.
 * Should be ordered from JSON_mode_names array in FX.h.
 * 
 * NOTE: If JSON_mode_names changes, this will need to be updated.
 */
const byte modes_alpha_order[] = {
    0, 27, 38, 115, 1, 26, 91, 68, 2, 88, 102, 114, 28, 31, 32,
    30, 29, 111, 52, 34, 8, 74, 67, 112, 18, 19, 96, 7, 117, 12,
    69, 66, 45, 42, 90, 89, 110, 87, 46, 53, 82, 100, 58, 64, 75,
    41, 57, 47, 44, 76, 77, 59, 70, 71, 72, 73, 107, 62, 101, 65,
    98, 105, 109, 97, 48, 49, 95, 63, 78, 43, 9, 33, 5, 79, 99,
    15, 37, 16, 10, 11, 40, 60, 108, 92, 93, 94, 103, 83, 84, 20,
    21, 22, 85, 86, 39, 61, 23, 25, 24, 104, 6, 36, 13, 14, 35,
    54, 56, 55, 116, 17, 81, 80, 106, 51, 50, 113, 3, 4 };

/**
 * Array of palette indexes in alphabetical order.
 * Should be ordered from JSON_palette_names array in FX.h.
 *
 * NOTE: If JSON_palette_names changes, this will need to be updated.
 */
const byte palettes_alpha_order[] = { 
  0, 1, 2, 3, 4, 5, 18, 46, 51, 50, 55, 39, 26, 22, 15,
  48, 52, 53, 7, 37, 24, 30, 35, 10, 32, 28, 29, 36, 31,
  25, 8, 38, 40, 41, 9, 44, 47, 6, 20, 11, 12, 16, 33, 
  14, 49, 27, 19, 13, 21, 54, 34, 45, 23, 43, 17, 42 };

class RotaryEncoderUIUsermod : public Usermod {
private:
  int fadeAmount = 10;             // Amount to change every step (brightness)
  unsigned long currentTime;
  unsigned long loopTime;
  const int pinA = ENCODER_DT_PIN;     // DT from encoder
  const int pinB = ENCODER_CLK_PIN;    // CLK from encoder
  const int pinC = ENCODER_SW_PIN;     // SW from encoder
  unsigned char select_state = 0;      // 0: brightness, 1: effect, 2: effect speed
  unsigned char button_state = HIGH;
  unsigned char prev_button_state = HIGH;
  
#ifdef USERMOD_FOUR_LINE_DISLAY
  FourLineDisplayUsermod* display;
#else
  void* display = nullptr;
#endif
  unsigned char Enc_A;
  unsigned char Enc_B;
  unsigned char Enc_A_prev = 0;

  bool currentEffectAndPaleeteInitialized = false;
  uint8_t effectCurrentIndex = 0;
  uint8_t effectPaletteIndex = 0;

public:
  /*
     * setup() is called once at boot. WiFi is not yet connected at this point.
     * You can use it to initialize variables, sensors or similar.
     */
  void setup()
  {
    pinMode(pinA, INPUT_PULLUP);
    pinMode(pinB, INPUT_PULLUP);
    pinMode(pinC, INPUT_PULLUP);
    currentTime = millis();
    loopTime = currentTime;

#ifdef USERMOD_FOUR_LINE_DISLAY    
    // This Usermod uses FourLineDisplayUsermod for the best experience.
    // But it's optional. But you want it.
    display = (FourLineDisplayUsermod*) usermods.lookup(USERMOD_ID_FOUR_LINE_DISP);
    if (display != nullptr) {
      display->setLineThreeType(FLD_LINE_3_BRIGHTNESS);
      display->setMarkLine(3);
    }
#endif
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
    if (!currentEffectAndPaleeteInitialized) {
      findCurrentEffectAndPalette();
    }

    if (currentTime >= (loopTime + 2)) // 2ms since last check of encoder = 500Hz
    {
      button_state = digitalRead(pinC);
      if (prev_button_state != button_state)
      {
        if (button_state == LOW)
        {
          prev_button_state = button_state;

          char newState = select_state + 1;
          if (newState > LAST_UI_STATE) newState = 0;
          
          bool changedState = true;
          if (display != nullptr) {
            switch(newState) {
              case 0:
                changedState = changeState("Brightness", FLD_LINE_3_BRIGHTNESS, 3);
                break;
              case 1:
                changedState = changeState("Select FX", FLD_LINE_3_EFFECT_SPEED, 2);
                break;
              case 2:
                changedState = changeState("FX Speed", FLD_LINE_3_EFFECT_SPEED, 3);
                break;
              case 3:
                changedState = changeState("FX Intensity", FLD_LINE_3_EFFECT_INTENSITY, 3);
                break;
              case 4:
                changedState = changeState("Palette", FLD_LINE_3_PALETTE, 3);
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
        }
      }
      int Enc_A = digitalRead(pinA); // Read encoder pins
      int Enc_B = digitalRead(pinB);
      if ((!Enc_A) && (Enc_A_prev))
      { // A has gone from high to low
        if (Enc_B == HIGH)
        { // B is high so clockwise
          switch(select_state) {
            case 0:
              changeBrightness(true);
              break;
            case 1:
              changeEffect(true);
              break;
            case 2:
              changeEffectSpeed(true);
              break;
            case 3:
              changeEffectIntensity(true);
              break;
            case 4:
              changePalette(true);
              break;
          }
        }
        else if (Enc_B == LOW)
        { // B is low so counter-clockwise
          switch(select_state) {
            case 0:
              changeBrightness(false);
              break;
            case 1:
              changeEffect(false);
              break;
            case 2:
              changeEffectSpeed(false);
              break;
            case 3:
              changeEffectIntensity(false);
              break;
            case 4:
              changePalette(false);
              break;
          }
        }
      }
      Enc_A_prev = Enc_A;     // Store value of A for next time
      loopTime = currentTime; // Updates loopTime
    }
  }

  void findCurrentEffectAndPalette() {
    currentEffectAndPaleeteInitialized = true;
    for (uint8_t i = 0; i < strip.getModeCount(); i++) {
      byte value = modes_alpha_order[i];
      if (modes_alpha_order[i] == effectCurrent) {
        effectCurrentIndex = i;
        break;
      }
    }

    for (uint8_t i = 0; i < strip.getPaletteCount(); i++) {
      byte value = palettes_alpha_order[i];
      if (palettes_alpha_order[i] == strip.getSegment(0).palette) {
        effectPaletteIndex = i;
        break;
      }
    }
  }

  boolean changeState(const char *stateName, byte lineThreeMode, byte markedLine) {
#ifdef USERMOD_FOUR_LINE_DISLAY
    if (display != nullptr) {
      if (display->wakeDisplay()) {
        // Throw away wake up input
        return false;
      }
      display->overlay("Mode change", stateName, 1500);
      display->setLineThreeType(lineThreeMode);
      display->setMarkLine(markedLine);
    }
  #endif
    return true;
  }

  void lampUdated() {
    bool fxChanged = strip.setEffectConfig(effectCurrent, effectSpeed, effectIntensity, effectPalette);

    //call for notifier -> 0: init 1: direct change 2: button 3: notification 4: nightlight 5: other (No notification)
    // 6: fx changed 7: hue 8: preset cycle 9: blynk 10: alexa
    colorUpdated(NOTIFIER_CALL_MODE_DIRECT_CHANGE);
    updateInterfaces(NOTIFIER_CALL_MODE_DIRECT_CHANGE);
  }

  void changeBrightness(bool increase) {
#ifdef USERMOD_FOUR_LINE_DISLAY
    if (display && display->wakeDisplay()) {
      // Throw away wake up input
      return;
    }
#endif
    if (increase) {
      bri = (bri + fadeAmount <= 255) ? (bri + fadeAmount) : 255;
    }
    else {
      bri = (bri - fadeAmount >= 0) ? (bri - fadeAmount) : 0;
    }
    lampUdated();
  }

  void changeEffect(bool increase) {
#ifdef USERMOD_FOUR_LINE_DISLAY
    if (display && display->wakeDisplay()) {
      // Throw away wake up input
      return;
    }
#endif
    if (increase) {
      effectCurrentIndex = (effectCurrentIndex + 1 >= strip.getModeCount()) ? 0 : (effectCurrentIndex + 1);
    }
    else {
      effectCurrentIndex = (effectCurrentIndex - 1 < 0) ? (strip.getModeCount() - 1) : (effectCurrentIndex - 1);
    }
    effectCurrent = modes_alpha_order[effectCurrentIndex];
    lampUdated();
  }

  void changeEffectSpeed(bool increase) {
#ifdef USERMOD_FOUR_LINE_DISLAY
    if (display && display->wakeDisplay()) {
      // Throw away wake up input
      return;
    }
#endif
    if (increase) {
      effectSpeed = (effectSpeed + fadeAmount <= 255) ? (effectSpeed + fadeAmount) : 255;
    }
    else {
      effectSpeed = (effectSpeed - fadeAmount >= 0) ? (effectSpeed - fadeAmount) : 0;
    }
    lampUdated();
  }

  void changeEffectIntensity(bool increase) {
#ifdef USERMOD_FOUR_LINE_DISLAY
    if (display && display->wakeDisplay()) {
      // Throw away wake up input
      return;
    }
#endif
    if (increase) {
      effectIntensity = (effectIntensity + fadeAmount <= 255) ? (effectIntensity + fadeAmount) : 255;
    }
    else {
      effectIntensity = (effectIntensity - fadeAmount >= 0) ? (effectIntensity - fadeAmount) : 0;
    }
    lampUdated();
  }

  void changePalette(bool increase) {
#ifdef USERMOD_FOUR_LINE_DISLAY
    if (display && display->wakeDisplay()) {
      // Throw away wake up input
      return;
    }
#endif
    if (increase) {
      effectPaletteIndex = (effectPaletteIndex + 1 >= strip.getPaletteCount()) ? 0 : (effectPaletteIndex + 1);
    }
    else {
      effectPaletteIndex = (effectPaletteIndex - 1 < 0) ? (strip.getPaletteCount() - 1) : (effectPaletteIndex - 1);
    }
    effectPalette = palettes_alpha_order[effectPaletteIndex];
    lampUdated();
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
    userVar0 = root["user0"] | userVar0; //if "user0" key exists in JSON, update, else keep old value
    //if (root["bri"] == 255) Serial.println(F("Don't burn down your garage!"));
  }

  /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
  uint16_t getId()
  {
    return USERMOD_ID_ROTARY_ENC_UI;
  }

  //More methods can be added in the future, this example will then be extended.
  //Your usermod will remain compatible as it does not need to implement all methods from the Usermod base class!
};
