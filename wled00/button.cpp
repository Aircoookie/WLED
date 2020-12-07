#include "wled.h"

#define DEBUG_INPUT_TAG() \
  DEBUG_PRINT(F("Input ")); \
  DEBUG_PRINT(idx_); \
  DEBUG_PRINT(F(": "));

/*
 * Physical IO
 */
class Input {
public:
  Input(InputConfig& cfg, byte idx) :
    cfg_(cfg),
    idx_(idx)
  {
#ifdef WLED_DEBUG
    DEBUG_INPUT_TAG();
    DEBUG_PRINT(F("pin="));
    DEBUG_PRINT(cfg_.pin);
    DEBUG_PRINT(F(" type="));
    DEBUG_PRINT(cfg_.inputType);
    DEBUG_PRINT(F(" presets=["));
    for (int i = 0; i <= MAX_INPUT_IDX; i++) {
      if (i != 0) {
        DEBUG_PRINT(F(", "));
      }
      DEBUG_PRINT(cfg_.preset[i]);
    }
    DEBUG_PRINTLN(F("]"));
#endif
    if (cfg_.pin != -1) {
      pinManager.allocatePin(cfg_.pin, false);
      pinMode(cfg_.pin, INPUT_PULLUP);

      // disable button if it is "pressed" unintentionally
      if (isButtonPressed()) {
        DEBUG_INPUT_TAG();
        DEBUG_PRINT(F(" input stuck, disabling"));
        inputConfigs[0].inputType = BTN_TYPE_NONE;
      }
    } else {
      inputConfigs[0].inputType = BTN_TYPE_NONE;
    }
  }

  bool isButtonPressed() {
    if (cfg_.pin != -1 && digitalRead(cfg_.pin) == LOW) return true;
#ifdef TOUCHPIN
    if (idx_ == TOUCH_MUX_INPUT && touchRead(TOUCHPIN) <= TOUCH_THRESHOLD) {
      return true;
    }
#endif
    return false;
  }

  void handle() {
    if (cfg_.pin == -1 && idx_ != TOUCH_MUX_INPUT) return;
    if (cfg_.inputType != BTN_TYPE_PUSH) return;

    if (isButtonPressed()) //pressed
    {
      if (!buttonPressedBefore_) buttonPressedTime_ = millis();
      buttonPressedBefore_ = true;

      if (millis() - buttonPressedTime_ > 600) //long press
      {
        if (!buttonLongPressed_) {
          DEBUG_INPUT_TAG();
          if (cfg_.preset[PUSH_LONG]) {
            DEBUG_PRINT(F("LONG TAP, preset "));
            DEBUG_PRINTLN(cfg_.preset[PUSH_LONG]);
            applyPreset(cfg_.preset[PUSH_LONG]);
          } else {
            DEBUG_PRINTLN(F("LONG TAP, random color"));
            _setRandomColor(false,true);
          }
          buttonLongPressed_ = true;
        }
      }
    }
    else if (!isButtonPressed() && buttonPressedBefore_) //released
    {
      long dur = millis() - buttonPressedTime_;
      if (dur < 50) {buttonPressedBefore_ = false; return;} //too short "press", debounce
      bool doublePress = buttonWaitTime_;
      buttonWaitTime_ = 0;

      if (dur > 6000) //long press
      {
        DEBUG_INPUT_TAG();
        DEBUG_PRINTLN(F("HELD DOWN, INITAP"));
        WLED::instance().initAP(true);
      }
      else if (!buttonLongPressed_) //short press
      {
        if (cfg_.preset[PUSH_DOUBLE]) {
          if (doublePress) {
            DEBUG_INPUT_TAG();
            DEBUG_PRINT(F("DOUBLE TAP, preset "));
            DEBUG_PRINTLN(cfg_.preset[PUSH_DOUBLE]);
            applyPreset(cfg_.preset[PUSH_DOUBLE]);
          } else {
            buttonWaitTime_ = millis();
          }
        } else {
          shortPressAction();
        }
      }
      buttonPressedBefore_ = false;
      buttonLongPressed_ = false;
    }
    if (buttonWaitTime_ && millis() - buttonWaitTime_ > 450 && !buttonPressedBefore_)
    {
      buttonWaitTime_ = 0;
      shortPressAction();
    }
  }

  void shortPressAction()
  {
    if (!cfg_.preset[PUSH_SHORT])
    {
      DEBUG_INPUT_TAG();
      DEBUG_PRINTLN(F("SHORT TAP, toggle"));
      toggleOnOff();
      colorUpdated(NOTIFIER_CALL_MODE_BUTTON);
    }
    else
    {
      DEBUG_INPUT_TAG();
      DEBUG_PRINT(F("SHORT TAP, preset "));
      DEBUG_PRINTLN(cfg_.preset[PUSH_SHORT]);
      applyPreset(cfg_.preset[PUSH_SHORT]);
    }
  }

private:
  InputConfig& cfg_;
  const byte idx_;
  static const byte TOUCH_MUX_INPUT = 0;
  bool buttonPressedBefore_{false};
  bool buttonLongPressed_{false};
  unsigned long buttonPressedTime_{0};
  unsigned long buttonWaitTime_{0};
};

Input* inputs[WLED_INPUTS];

/**
 * Prime the default input configs from compiled in constants
 */
void
inputEarlyConfig()
{
  for (int i = 0; i < WLED_INPUTS; i++) {
    InputConfig& cfg(inputConfigs[i]);
    switch (i) {
    case 0: {
      cfg.pin = BTNPIN;
#if BTNPIN != -1 || defined(TOUHPIN)
      cfg.inputType = BTN_TYPE_PUSH;
#else
      cfg.inputType = BTN_TYPE_NONE;
#endif
      break;
    }
    case 1: {
      cfg.pin = BTNPIN2;
#if BTNPIN2 != -1
      cfg.inputType = BTN_TYPE_PUSH;
#else
      cfg.inputType = BTN_TYPE_NONE;
#endif
      break;
    }
    case 2: {
      cfg.pin = BTNPIN3;
#if BTNPIN3 != -1
      cfg.inputType = BTN_TYPE_PUSH;
#else
      cfg.inputType = BTN_TYPE_NONE;
#endif
      break;
    }
    default: {
      cfg.pin = -1;
      cfg.inputType = BTN_TYPE_NONE;
    }
    }
  }
}

/**
 * This instantiates the input state objects and
 * connects the hardware resources
 */
void inputInit()
{
  for (int i = 0; i < WLED_INPUTS; i++) {
    inputs[i] = new Input(inputConfigs[i], i);
  }
}

bool isButtonPressed(byte idx)
{
  return inputs[0]->isButtonPressed();
}

void handleIO()
{
  for (int i = 0; i < WLED_INPUTS; i++) {
    inputs[i]->handle();
  }

  //set relay when LEDs turn on
  if (strip.getBrightness())
  {
    lastOnTime = millis();
    if (offMode)
    {
      #if RLYPIN >= 0
       digitalWrite(RLYPIN, RLYMDE);
      #endif
      offMode = false;
    }
  } else if (millis() - lastOnTime > 600)
  {
     if (!offMode) {
      #if LEDPIN == LED_BUILTIN
        pinMode(LED_BUILTIN, OUTPUT);
        digitalWrite(LED_BUILTIN, HIGH);
      #endif
      #if RLYPIN >= 0
       digitalWrite(RLYPIN, !RLYMDE);
      #endif
     }
    offMode = true;
  }

  #if AUXPIN >= 0
  //output
  if (auxActive || auxActiveBefore)
  {
    if (!auxActiveBefore)
    {
      auxActiveBefore = true;
      switch (auxTriggeredState)
      {
        case 0: pinMode(AUXPIN, INPUT); break;
        case 1: pinMode(AUXPIN, OUTPUT); digitalWrite(AUXPIN, HIGH); break;
        case 2: pinMode(AUXPIN, OUTPUT); digitalWrite(AUXPIN, LOW); break;
      }
      auxStartTime = millis();
    }
    if ((millis() - auxStartTime > auxTime*1000 && auxTime != 255) || !auxActive)
    {
      auxActive = false;
      auxActiveBefore = false;
      switch (auxDefaultState)
      {
        case 0: pinMode(AUXPIN, INPUT); break;
        case 1: pinMode(AUXPIN, OUTPUT); digitalWrite(AUXPIN, HIGH); break;
        case 2: pinMode(AUXPIN, OUTPUT); digitalWrite(AUXPIN, LOW); break;
      }
    }
  }
  #endif
}
