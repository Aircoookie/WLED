#pragma once

#include "wled.h"

#define WLED_DEBOUNCE_THRESHOLD 50 // only consider button input of at least 50ms as valid (debouncing)

// v2 usermod that allows to change brightness and color/color-temp using a rotary encoder,
// change between modes by pressing a button
// change between RGB and white by double-pressing a button (many encoders have one included)
class RotaryEncoderBCT : public Usermod
{
private:
  // Private class members. You can declare variables and functions only accessible to your usermod here
  unsigned long lastTime = 0;
  unsigned long currentTime;
  unsigned long loopTime;

  unsigned char mode = 0;         // 0 = RGB 1 = white
  unsigned char select_state = 0; // 0 = brightness 1 = color/temperature
  CRGB fastled_col;
  CHSV prim_hsv;
  int16_t new_val;
  int16_t old_r;
  int16_t old_g;
  int16_t old_b;
  unsigned int whiteTemp = 6000;

  unsigned char Enc_A;
  unsigned char Enc_B;
  unsigned char Enc_A_prev = 0;

  // private class memebers configurable by Usermod Settings (defaults set inside readFromConfig())
  int8_t DTPin = -1;
  int8_t CLKPin = -1;
  int8_t SWPin = -1;
  int fadeAmount = 5; // how many points to fade the Neopixel with each step
  int fadeAmountTemp = 100;

  static const char _ConfRoot[];
  static const char _CLKPin[];
  static const char _DTPin[];
  static const char _SWPin[];
  static const char _fadeAmount[];
  static const char _fadeAmountTemp[];
public:
  // Functions called by WLED

  /*
   * setup() is called once at boot. WiFi is not yet connected at this point.
   * You can use it to initialize variables, sensors or similar.
   */
  void setup()
  {
    pinMode(DTPin, INPUT_PULLUP);
    pinMode(CLKPin, INPUT_PULLUP);

    currentTime = millis();
    loopTime = currentTime;
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

    if (currentTime >= (loopTime + 2)) // 2ms since last check of encoder = 500Hz
    {
      int Enc_A = digitalRead(DTPin); // Read encoder pins
      int Enc_B = digitalRead(CLKPin);
      if ((!Enc_A) && (Enc_A_prev))
      { // A has gone from high to low
        if (Enc_B == HIGH)
        { // B is high so clockwise
          if (select_state == 0)
          {
            if (bri + fadeAmount <= 255)
              bri += fadeAmount; // increase the brightness, dont go over 255
          }
          else if (select_state == 1)
          {
            if (mode == 0)
            {
              fastled_col.red = col[0];
              fastled_col.green = col[1];
              fastled_col.blue = col[2];
              prim_hsv = rgb2hsv_approximate(fastled_col);
              new_val = (int16_t)prim_hsv.h + fadeAmount;
              if (new_val > 255)
                new_val -= 255; // roll-over if  bigger than 255
              if (new_val < 0)
                new_val += 255; // roll-over if smaller than 0
              prim_hsv.h = (byte)new_val;
              hsv2rgb_rainbow(prim_hsv, fastled_col);
              col[0] = fastled_col.red;
              col[1] = fastled_col.green;
              col[2] = fastled_col.blue;
            }
            else
            {
              if (whiteTemp + fadeAmountTemp < 10000)
                whiteTemp += fadeAmountTemp;

              colorKtoRGB(whiteTemp, col);
            }
          }
        }
        else if (Enc_B == LOW)
        { // B is low so counter-clockwise
          if (select_state == 0)
          {
            if (bri - fadeAmount >= 0)
              bri -= fadeAmount; // decrease the brightness, dont go below 0
          }
          else if (select_state == 1)
          {
            if (mode == 0)
            {
              fastled_col.red = col[0];
              fastled_col.green = col[1];
              fastled_col.blue = col[2];
              prim_hsv = rgb2hsv_approximate(fastled_col);
              new_val = (int16_t)prim_hsv.h - fadeAmount;
              if (new_val > 255)
                new_val -= 255; // roll-over if  bigger than 255
              if (new_val < 0)
                new_val += 255; // roll-over if smaller than 0
              prim_hsv.h = (byte)new_val;
              hsv2rgb_rainbow(prim_hsv, fastled_col);
              col[0] = fastled_col.red;
              col[1] = fastled_col.green;
              col[2] = fastled_col.blue;
            }
            else
            {
              if (whiteTemp - fadeAmountTemp > 1900)
                whiteTemp -= fadeAmountTemp;

              colorKtoRGB(whiteTemp, col);
            }
          }
        }
        // call for notifier -> 0: init 1: direct change 2: button 3: notification 4: nightlight 5: other (No notification)
        //  6: fx changed 7: hue 8: preset cycle 9: blynk 10: alexa
        colorUpdated(CALL_MODE_BUTTON);
        updateInterfaces(CALL_MODE_BUTTON);
      }
      Enc_A_prev = Enc_A;     // Store value of A for next time
      loopTime = currentTime; // Updates loopTime
    }
  }

  /**
   * handleButton() can be used to override default button behaviour. Returning true
   * will prevent button working in a default way.
   * Replicating button.cpp
   */
  bool handleButton(uint8_t b)
  {
    yield();
    if (buttonType[b] == BTN_TYPE_NONE || buttonType[b] == BTN_TYPE_SWITCH || buttonType[b] == BTN_TYPE_RESERVED || buttonType[b] == BTN_TYPE_PIR_SENSOR || buttonType[b] == BTN_TYPE_ANALOG || buttonType[b] == BTN_TYPE_ANALOG_INVERTED)
      return false;

    if (SWPin != btnPin[b])
      return false;

    unsigned long now = millis();
    bool handled = true;

    // momentary button logic
    if (isButtonPressed(b))
    { // pressed
      DEBUG_PRINTLN(F("Rotary button pressed"));
      if (!buttonPressedBefore[b])
        buttonPressedTime[b] = now;
      buttonPressedBefore[b] = true;

      if (now - buttonPressedTime[b] > 600)
      { // long press
        buttonLongPressed[b] = true;
      }
    }
    else if (!isButtonPressed(b) && buttonPressedBefore[b])
    { // released
      long dur = now - buttonPressedTime[b];
      if (dur < WLED_DEBOUNCE_THRESHOLD)
      {
        buttonPressedBefore[b] = false;
        return handled;
      }                                     // too short "press", debounce
      bool doublePress = buttonWaitTime[b]; // did we have short press before?
      buttonWaitTime[b] = 0;

      if (!buttonLongPressed[b])
      { // short press
        // if this is second release within 350ms it is a double press (buttonWaitTime!=0)
        if (doublePress)
        {
          DEBUG_PRINTLN(F("Doubleclick detected -> changing mode"));
          if (mode == 0)
          {
            old_r = col[0];
            old_g = col[1];
            old_b = col[2];
            colorKtoRGB(whiteTemp, col);
            colorUpdated(CALL_MODE_BUTTON);
            updateInterfaces(CALL_MODE_BUTTON);
            mode = 1;
          }
          else
          {
            col[0] = old_r;
            col[1] = old_g;
            col[2] = old_b;
            colorUpdated(CALL_MODE_BUTTON);
            updateInterfaces(CALL_MODE_BUTTON);
            mode = 0;
          }
          handled = true;
        }
        else
        {
          buttonWaitTime[b] = now;
        }
      }
      buttonPressedBefore[b] = false;
      buttonLongPressed[b] = false;
    }
    // if 450ms elapsed since last press/release it is a short press
    if (buttonWaitTime[b] && now - buttonWaitTime[b] > 350 && !buttonPressedBefore[b])
    {
      DEBUG_PRINTLN(F("short press detected -> changing state"));
      buttonWaitTime[b] = 0;
      if (select_state == 0)
      {
        select_state = 1;
      }
      else
      {
        select_state = 0;
      }
    }
    return handled;
  }

  void addToConfig(JsonObject &root)
  {
    JsonObject top = root.createNestedObject(FPSTR(_ConfRoot));
    top[FPSTR(_fadeAmount)] = fadeAmount;
    top[FPSTR(_fadeAmountTemp)] = fadeAmountTemp;
    top[FPSTR(_CLKPin)] = CLKPin;
    top[FPSTR(_DTPin)] = DTPin;
    top[FPSTR(_SWPin)] = SWPin;
  }

  /*
   * This example uses a more robust method of checking for missing values in the config, and setting back to defaults:
   * - The getJsonValue() function copies the value to the variable only if the key requested is present, returning false with no copy if the value isn't present
   * - configComplete is used to return false if any value is missing, not just if the main object is missing
   * - The defaults are loaded every time readFromConfig() is run, not just once after boot
   *
   * This ensures that missing values are added to the config, with their default values, in the rare but plauible cases of:
   * - a single value being missing at boot, e.g. if the Usermod was upgraded and a new setting was added
   * - a single value being missing after boot (e.g. if the cfg.json was manually edited and a value was removed)
   *
   * If configComplete is false, the default values are already set, and by returning false, WLED now knows it needs to save the defaults by calling addToConfig()
   */
  bool readFromConfig(JsonObject &root)
  {
    // set defaults here, they will be set before setup() is called, and if any values parsed from ArduinoJson below are missing, the default will be used instead
    JsonObject top = root[FPSTR(_ConfRoot)];

    bool configComplete = !top.isNull();
    configComplete &= getJsonValue(top[FPSTR(_fadeAmount)], fadeAmount);
    configComplete &= getJsonValue(top[FPSTR(_fadeAmountTemp)], fadeAmountTemp);
    configComplete &= getJsonValue(top[FPSTR(_CLKPin)], CLKPin);
    configComplete &= getJsonValue(top[FPSTR(_DTPin)], DTPin);
    configComplete &= getJsonValue(top[FPSTR(_SWPin)], SWPin);

    return configComplete;
  }
};

// strings to reduce flash memory usage (used more than twice)
const char RotaryEncoderBCT::_ConfRoot[] PROGMEM = "rotaryEncoderColorBrightnessTemp";
const char RotaryEncoderBCT::_CLKPin[] PROGMEM = "CLKPin";
const char RotaryEncoderBCT::_DTPin[] PROGMEM = "DTPin";
const char RotaryEncoderBCT::_SWPin[] PROGMEM = "SWPin";
const char RotaryEncoderBCT::_fadeAmount[] PROGMEM = "fadeAmount";
const char RotaryEncoderBCT::_fadeAmountTemp[] PROGMEM = "fadeAmountWhite";
