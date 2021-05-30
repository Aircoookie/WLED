#include "wled.h"

/*
 * Physical IO
 */

#define WLED_DEBOUNCE_THRESHOLD 50 //only consider button input of at least 50ms as valid (debouncing)

void shortPressAction(uint8_t b)
{
  if (!macroButton[b])
  {
    toggleOnOff();
    colorUpdated(NOTIFIER_CALL_MODE_BUTTON);
  } else {
    applyPreset(macroButton[b]);
  }
}

bool isButtonPressed(uint8_t i)
{
  if (btnPin[i]<0) return false;
  switch (buttonType[i]) {
    case BTN_TYPE_NONE:
    case BTN_TYPE_RESERVED:
      break;
    case BTN_TYPE_PUSH:
    case BTN_TYPE_SWITCH:
      if (digitalRead(btnPin[i]) == LOW) return true;
      break;
    case BTN_TYPE_PUSH_ACT_HIGH:
    case BTN_TYPE_SWITCH_ACT_HIGH:
      if (digitalRead(btnPin[i]) == HIGH) return true;
      break;
    case BTN_TYPE_TOUCH:
      #ifdef ARDUINO_ARCH_ESP32
      if (touchRead(btnPin[i]) <= touchThreshold) return true;
      #endif
      break;
  }
  return false;
}

void handleSwitch(uint8_t b)
{
  if (buttonPressedBefore[b] != isButtonPressed(b)) {
    buttonPressedTime[b] = millis();
    buttonPressedBefore[b] = !buttonPressedBefore[b];
  }

  if (buttonLongPressed[b] == buttonPressedBefore[b]) return;
    
  if (millis() - buttonPressedTime[b] > WLED_DEBOUNCE_THRESHOLD) { //fire edge event only after 50ms without change (debounce)
    if (buttonPressedBefore[b]) { //LOW, falling edge, switch closed
      if (macroButton[b]) applyPreset(macroButton[b]);
      else { //turn on
        if (!bri) {toggleOnOff(); colorUpdated(NOTIFIER_CALL_MODE_BUTTON);}
      } 
    } else { //HIGH, rising edge, switch opened
      if (macroLongPress[b]) applyPreset(macroLongPress[b]);
      else { //turn off
        if (bri) {toggleOnOff(); colorUpdated(NOTIFIER_CALL_MODE_BUTTON);}
      } 
    }
    buttonLongPressed[b] = buttonPressedBefore[b]; //save the last "long term" switch state
  }
}

void handleAnalog(uint8_t b)
{
  static uint8_t oldRead[WLED_MAX_BUTTONS];
  #ifdef ESP8266
  uint16_t aRead = analogRead(A0) >> 2; // convert 10bit read to 8bit
  #else
  uint16_t aRead = analogRead(btnPin[b]) >> 4; // convert 12bit read to 8bit
  #endif
  // remove noise & reduce frequency of UI updates
  aRead &= 0xFC;

  if (oldRead[b] == aRead) return;  // no change in reading
  oldRead[b] = aRead;

  // if no macro for "short press" and "long press" is defined use brightness control
  if (!macroButton[b] && !macroLongPress[b]) {
    // if "double press" macro defines which option to change
    if (macroDoublePress[b] >= 250) {
      // global brightness
      if (aRead == 0) {
        briLast = bri;
        bri = 0;
      } else{
        bri = aRead;
      }
    } else if (macroDoublePress[b] == 249) {
      // effect speed
      effectSpeed = aRead;
      effectChanged = true;
      for (uint8_t i = 0; i < strip.getMaxSegments(); i++) {
        WS2812FX::Segment& seg = strip.getSegment(i);
        if (!seg.isSelected()) continue;
        seg.speed = effectSpeed;
      }
    } else if (macroDoublePress[b] == 248) {
      // effect intensity
      effectIntensity = aRead;
      effectChanged = true;
      for (uint8_t i = 0; i < strip.getMaxSegments(); i++) {
        WS2812FX::Segment& seg = strip.getSegment(i);
        if (!seg.isSelected()) continue;
        seg.intensity = effectIntensity;
      }
    } else if (macroDoublePress[b] == 247) {
      // selected palette
      effectPalette = map(aRead, 0, 252, 0, strip.getPaletteCount()-1);
      effectChanged = true;
      for (uint8_t i = 0; i < strip.getMaxSegments(); i++) {
        WS2812FX::Segment& seg = strip.getSegment(i);
        if (!seg.isSelected()) continue;
        seg.palette = effectPalette;
      }
    } else if (macroDoublePress[b] == 200) {
      // primary color, hue, full saturation
      colorHStoRGB(aRead*256,255,col);
    } else {
      // otherwise use "double press" for segment selection
      //uint8_t mainSeg = strip.getMainSegmentId();
      WS2812FX::Segment& seg = strip.getSegment(macroDoublePress[b]);
      if (aRead == 0) {
        seg.setOption(SEG_OPTION_ON, 0); // off
      } else {
        seg.setOpacity(aRead, macroDoublePress[b]);
        seg.setOption(SEG_OPTION_ON, 1);
      }
      // this will notify clients of update (websockets,mqtt,etc)
      //call for notifier -> 0: init 1: direct change 2: button 3: notification 4: nightlight 5: other (No notification)
      // 6: fx changed 7: hue 8: preset cycle 9: blynk 10: alexa
      updateInterfaces(NOTIFIER_CALL_MODE_BUTTON);
    }
  } else {
    //TODO:
    // we can either trigger a preset depending on the level (between short and long entries)
    // or use it for RGBW direct control
  }
  //call for notifier -> 0: init 1: direct change 2: button 3: notification 4: nightlight 5: other (No notification)
  // 6: fx changed 7: hue 8: preset cycle 9: blynk 10: alexa
  colorUpdated(NOTIFIER_CALL_MODE_BUTTON);
}

void handleButton()
{
  static unsigned long lastRead = 0UL;

  for (uint8_t b=0; b<WLED_MAX_BUTTONS; b++) {
    #ifdef ESP8266
    if ((btnPin[b]<0 && buttonType[b] != BTN_TYPE_ANALOG) || buttonType[b] == BTN_TYPE_NONE) continue;
    #else
    if (btnPin[b]<0 || buttonType[b] == BTN_TYPE_NONE) continue;
    #endif

    if (buttonType[b] == BTN_TYPE_ANALOG && millis() - lastRead > 250) {   // button is not a button but a potentiometer
      if (b+1 == WLED_MAX_BUTTONS) lastRead = millis();
      handleAnalog(b); continue;
    }

    if (buttonType[b] == BTN_TYPE_SWITCH || buttonType[b] == BTN_TYPE_SWITCH_ACT_HIGH) { //button is not momentary, but switch. This is only suitable on pins whose on-boot state does not matter (NOT gpio0)
      handleSwitch(b); continue;
    }

    //momentary button logic
    if (isButtonPressed(b)) //pressed
    {
      if (!buttonPressedBefore[b]) buttonPressedTime[b] = millis();
      buttonPressedBefore[b] = true;

      if (millis() - buttonPressedTime[b] > 600) //long press
      {
        if (!buttonLongPressed[b]) 
        {
          if (macroLongPress[b]) {applyPreset(macroLongPress[b]);}
          else _setRandomColor(false,true);

          buttonLongPressed[b] = true;
        }
      }
    }
    else if (!isButtonPressed(b) && buttonPressedBefore[b]) //released
    {
      long dur = millis() - buttonPressedTime[b];
      if (dur < WLED_DEBOUNCE_THRESHOLD) {buttonPressedBefore[b] = false; continue;} //too short "press", debounce
      bool doublePress = buttonWaitTime[b];
      buttonWaitTime[b] = 0;

      if (dur > 6000 && b==0) //long press on button 0
      {
        WLED::instance().initAP(true);
      }
      else if (!buttonLongPressed[b]) { //short press
        if (macroDoublePress[b])
        {
          if (doublePress) applyPreset(macroDoublePress[b]);
          else buttonWaitTime[b] = millis();
        } else shortPressAction(b);
      }
      buttonPressedBefore[b] = false;
      buttonLongPressed[b] = false;
    }

    if (buttonWaitTime[b] && millis() - buttonWaitTime[b] > 450 && !buttonPressedBefore[b])
    {
      buttonWaitTime[b] = 0;
      shortPressAction(b);
    }
  }
}

void handleIO()
{
  handleButton();
  
  //set relay when LEDs turn on
  if (strip.getBrightness())
  {
    lastOnTime = millis();
    if (offMode)
    {
      if (rlyPin>=0) {
        pinMode(rlyPin, OUTPUT);
        digitalWrite(rlyPin, rlyMde);
      }
      offMode = false;
    }
  } else if (millis() - lastOnTime > 600)
  {
    if (!offMode) {
      #ifdef ESP8266
      // turn off built-in LED if strip is turned off
      // this will break digital bus so will need to be reinitialised on On
      pinMode(LED_BUILTIN, OUTPUT);
      digitalWrite(LED_BUILTIN, HIGH);
      #endif
      if (rlyPin>=0) {
        pinMode(rlyPin, OUTPUT);
        digitalWrite(rlyPin, !rlyMde);
      }
    }
    offMode = true;
  }
}
