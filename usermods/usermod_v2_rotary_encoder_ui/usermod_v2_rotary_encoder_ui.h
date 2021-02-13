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

// Number of modes at the start of the list to not sort
#define MODE_SORT_SKIP_COUNT 1

// Pointers the start of the mode names within JSON_mode_names
char **modes_qstrings = nullptr;

// Array of mode indexes in alphabetical order.
byte *modes_alpha_indexes = nullptr;

// Pointers the start of the palette names within JSON_palette_names
char **palettes_qstrings = nullptr;

// Which list is being sorted
char **listBeingSorted = nullptr;

// Array of palette indexes in alphabetical order.
byte *palettes_alpha_indexes = nullptr;

/**
 * Modes and palettes are stored as strings that
 * end in a quote character. Compare two of them.
 * We are comparing directly within either
 * JSON_mode_names or JSON_palette_names.
 */
int re_qstringCmp(const void *ap, const void *bp) {
    char *a = listBeingSorted[*((byte*)ap)];
    char *b  = listBeingSorted[*((byte*)bp)];
    int i = 0;
    do {
        char aVal = pgm_read_byte_near(a + i);
        if (aVal >= 97 && aVal <= 122) {
          // Lowercase
          aVal -= 32;
        }
        char bVal = pgm_read_byte_near(b + i);
        if (bVal >= 97 && bVal <= 122) {
          // Lowercase
          bVal -= 32;
        }
        // Relly we shouldn't ever get to '\0'
        if (aVal == '"' || bVal == '"' || aVal == '\0' || bVal == '\0') {
          // We're done. one is a substring of the other
          // or something happenend and the quote didn't stop us.
          if (aVal == bVal) {
            // Same value, probably shouldn't happen
            // with this dataset
            return 0;
          }
          else if (aVal == '"' || aVal == '\0' ) {
            return -1;
          }
          else {
            return 1;
          }
        }
        if (aVal == bVal) {
          // Same characters. Move to the next.
          i++;
          continue;
        }
        // We're done
        if (aVal < bVal) {
          return -1;
        }
        else {
          return 1;
        }
    } while (true);
    // We shouldn't get here.
    return 0;
}

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
    /**
     * Sort the modes and palettes on startup
     * as they are guarantted to change.
     */
    sortModesAndPalettes();

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

/**
 * Sort the modes and palettes to the index arrays
 * modes_alpha_indexes and palettes_alpha_indexes.
 */
void sortModesAndPalettes() {
    modes_qstrings = re_findModeStrings(JSON_mode_names, strip.getModeCount());    
    modes_alpha_indexes = re_initIndexArray(strip.getModeCount());
    re_sortModes(modes_qstrings, modes_alpha_indexes, strip.getModeCount(), MODE_SORT_SKIP_COUNT);
    free(modes_qstrings);
    modes_qstrings = nullptr;

    palettes_qstrings = re_findModeStrings(JSON_palette_names, strip.getPaletteCount());
    palettes_alpha_indexes = re_initIndexArray(strip.getPaletteCount());

    int skipPaletteCount = 1;
    while (true) {
      // How many palette names start with '*' and should not be sorted?
      // (Also skipping the first one, 'Default').
      if (pgm_read_byte_near(palettes_qstrings[skipPaletteCount]) == '*') {
        skipPaletteCount++;
      }
      else {
        break;  
      }
    }

    re_sortModes(palettes_qstrings, palettes_alpha_indexes, strip.getPaletteCount(), skipPaletteCount);
    free(palettes_qstrings);
    palettes_qstrings = nullptr;
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
      byte value = modes_alpha_indexes[i];
      if (modes_alpha_indexes[i] == effectCurrent) {
        effectCurrentIndex = i;
        break;
      }
    }

    for (uint8_t i = 0; i < strip.getPaletteCount(); i++) {
      byte value = palettes_alpha_indexes[i];
      if (palettes_alpha_indexes[i] == strip.getSegment(0).palette) {
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
    effectCurrent = modes_alpha_indexes[effectCurrentIndex];
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
    effectPalette = palettes_alpha_indexes[effectPaletteIndex];
    lampUdated();
  }

  byte *re_initIndexArray(int numModes) {
    byte *indexes = (byte *)malloc(sizeof (byte) * numModes);  
    for (byte i = 0; i < numModes; i++) {
      indexes[i] = i;
    }
    return indexes;
  }

  /**
   * Return an array of mode or palette names from the JSON string.
   * They don't end in '\0', they end in '"'. 
   */
  char **re_findModeStrings(const char json[], int numModes) {
    char **modeStrings = (char **)malloc(sizeof (char *) * numModes);
    uint8_t modeIndex = 0;
    bool insideQuotes = false;
    // advance past the mark for markLineNum that may exist.
    char singleJsonSymbol;

    // Find the mode name in JSON
    bool complete = false;
    for (size_t i = 0; i < strlen_P(json); i++) {
      singleJsonSymbol = pgm_read_byte_near(json + i);
      switch (singleJsonSymbol) {
        case '"':
          insideQuotes = !insideQuotes;
          if (insideQuotes) {
            // We have a new mode or palette
            modeStrings[modeIndex] = (char*)(json + i + 1);
          }
          break;
        case '[':
          break;
        case ']':
          complete = true;
          break;
        case ',':
          modeIndex++;
        default:
          if (!insideQuotes) {
            break;
          }
      }
      if (complete) {
        break;
      }
    }
    return modeStrings;
  }

  /**
   * Sort either the modes or the palettes using quicksort.
   */
  void re_sortModes(char **modeNames, byte *indexes, int count, int numSkip) {
    listBeingSorted = modeNames;
    qsort(indexes + numSkip, count - numSkip, sizeof(byte), re_qstringCmp);
    listBeingSorted = nullptr;
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
