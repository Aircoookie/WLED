#include "wled.h"

/*
 * LED methods
 */

void setValuesFromMainSeg()          { setValuesFromSegment(strip.getMainSegmentId()); }
void setValuesFromFirstSelectedSeg() { setValuesFromSegment(strip.getFirstSelectedSegId()); }
void setValuesFromSegment(uint8_t s)
{
  Segment& seg = strip.getSegment(s);
  col[0] = R(seg.colors[0]);
  col[1] = G(seg.colors[0]);
  col[2] = B(seg.colors[0]);
  col[3] = W(seg.colors[0]);
  colSec[0] = R(seg.colors[1]);
  colSec[1] = G(seg.colors[1]);
  colSec[2] = B(seg.colors[1]);
  colSec[3] = W(seg.colors[1]);
  effectCurrent   = seg.mode;
  effectSpeed     = seg.speed;
  effectIntensity = seg.intensity;
  effectPalette   = seg.palette;
}


// applies global legacy values (col, colSec, effectCurrent...)
// problem: if the first selected segment already has the value to be set, other selected segments are not updated
void applyValuesToSelectedSegs()
{
  // copy of first selected segment to tell if value was updated
  uint8_t firstSel = strip.getFirstSelectedSegId();
  Segment selsegPrev = strip.getSegment(firstSel);
  for (uint8_t i = 0; i < strip.getSegmentsNum(); i++) {
    Segment& seg = strip.getSegment(i);
    if (i != firstSel && (!seg.isActive() || !seg.isSelected())) continue;

    if (effectSpeed     != selsegPrev.speed)     {seg.speed     = effectSpeed;     stateChanged = true;}
    if (effectIntensity != selsegPrev.intensity) {seg.intensity = effectIntensity; stateChanged = true;}
    if (effectPalette   != selsegPrev.palette)   {seg.palette   = effectPalette;   stateChanged = true;}
    if (effectCurrent   != selsegPrev.mode)      {strip.setMode(i, effectCurrent); stateChanged = true;}
    uint32_t col0 = RGBW32(   col[0],    col[1],    col[2],    col[3]);
    uint32_t col1 = RGBW32(colSec[0], colSec[1], colSec[2], colSec[3]);
    if (col0 != selsegPrev.colors[0])            {seg.setColor(0, col0);           stateChanged = true;}
    if (col1 != selsegPrev.colors[1])            {seg.setColor(1, col1);           stateChanged = true;}
  }
}


void resetTimebase()
{
  strip.timebase = 0 - millis();
}


void toggleOnOff()
{
  if (bri == 0)
  {
    bri = briLast;
  } else
  {
    briLast = bri;
    bri = 0;
  }
  stateChanged = true;
}


//scales the brightness with the briMultiplier factor
byte scaledBri(byte in)
{
  uint16_t val = ((uint16_t)in*briMultiplier)/100;
  if (val > 255) val = 255;
  return (byte)val;
}


//applies global brightness
void applyBri() {
  if (!realtimeMode || !arlsForceMaxBri)
  {
    strip.setBrightness(scaledBri(briT));
  }
}


//applies global brightness and sets it as the "current" brightness (no transition)
void applyFinalBri() {
  briOld = bri;
  briT = bri;
  applyBri();
}


//called after every state changes, schedules interface updates, handles brightness transition and nightlight activation
//unlike colorUpdated(), does NOT apply any colors or FX to segments
void stateUpdated(byte callMode) {
  //call for notifier -> 0: init 1: direct change 2: button 3: notification 4: nightlight 5: other (No notification)
  //                     6: fx changed 7: hue 8: preset cycle 9: blynk 10: alexa 11: ws send only 12: button preset
  setValuesFromFirstSelectedSeg();

  if (bri != briOld || stateChanged) {
    if (stateChanged) currentPreset = 0; //something changed, so we are no longer in the preset

    if (callMode != CALL_MODE_NOTIFICATION && callMode != CALL_MODE_NO_NOTIFY) notify(callMode);

    //set flag to update ws and mqtt
    interfaceUpdateCallMode = callMode;
    stateChanged = false;
  } else {
    if (nightlightActive && !nightlightActiveOld && callMode != CALL_MODE_NOTIFICATION && callMode != CALL_MODE_NO_NOTIFY) {
      notify(CALL_MODE_NIGHTLIGHT);
      interfaceUpdateCallMode = CALL_MODE_NIGHTLIGHT;
    }
  }

  if (callMode != CALL_MODE_NO_NOTIFY && nightlightActive && (nightlightMode == NL_MODE_FADE || nightlightMode == NL_MODE_COLORFADE)) {
    briNlT = bri;
    nightlightDelayMs -= (millis() - nightlightStartTime);
    nightlightStartTime = millis();
  }
  if (briT == 0) {
    if (callMode != CALL_MODE_NOTIFICATION) resetTimebase(); //effect start from beginning
  }

  if (bri > 0) briLast = bri;

  //deactivate nightlight if target brightness is reached
  if (bri == nightlightTargetBri && callMode != CALL_MODE_NO_NOTIFY && nightlightMode != NL_MODE_SUN) nightlightActive = false;

  // notify usermods of state change
  usermods.onStateChange(callMode);

  if (fadeTransition) {
    //set correct delay if not using notification delay
    if (callMode != CALL_MODE_NOTIFICATION && !jsonTransitionOnce) transitionDelayTemp = transitionDelay; // load actual transition duration
    jsonTransitionOnce = false;
    strip.setTransition(transitionDelayTemp);
    if (transitionDelayTemp == 0) {
      applyFinalBri();
      strip.trigger();
      return;
    }

    if (transitionActive) {
      briOld = briT;
      tperLast = 0;
    }
    strip.setTransitionMode(true); // force all segments to transition mode
    transitionActive = true;
    transitionStartTime = millis();
  } else {
    strip.setTransition(0);
    applyFinalBri();
    strip.trigger();
  }
}


void updateInterfaces(uint8_t callMode)
{
  sendDataWs();
  lastInterfaceUpdate = millis();
  if (callMode == CALL_MODE_WS_SEND) return;

  #ifndef WLED_DISABLE_ALEXA
  if (espalexaDevice != nullptr && callMode != CALL_MODE_ALEXA) {
    espalexaDevice->setValue(bri);
    espalexaDevice->setColor(col[0], col[1], col[2]);
  }
  #endif
  doPublishMqtt = true;
  interfaceUpdateCallMode = 0; //disable
}


void handleTransitions()
{
  //handle still pending interface update
  if (interfaceUpdateCallMode && millis() - lastInterfaceUpdate > INTERFACE_UPDATE_COOLDOWN) updateInterfaces(interfaceUpdateCallMode);
#ifndef WLED_DISABLE_MQTT
  if (doPublishMqtt) publishMqtt();
#endif

  if (transitionActive && transitionDelayTemp > 0)
  {
    float tper = (millis() - transitionStartTime)/(float)transitionDelayTemp;
    if (tper >= 1.0)
    {
      strip.setTransitionMode(false);
      transitionActive = false;
      tperLast = 0;
      applyFinalBri();
      return;
    }
    if (tper - tperLast < 0.004) return;
    tperLast = tper;
    briT = briOld + ((bri - briOld) * tper);

    applyBri();
  }
}


// legacy method, applies values from col, effectCurrent, ... to selected segments
void colorUpdated(byte callMode){
  applyValuesToSelectedSegs();
  stateUpdated(callMode);
}


void handleNightlight()
{
  static unsigned long lastNlUpdate;
  unsigned long now = millis();
  if (now < 100 && lastNlUpdate > 0) lastNlUpdate = 0; // take care of millis() rollover
  if (now - lastNlUpdate < 100) return; // allow only 10 NL updates per second
  lastNlUpdate = now;

  if (nightlightActive)
  {
    if (!nightlightActiveOld) //init
    {
      nightlightStartTime = millis();
      nightlightDelayMs = (int)(nightlightDelayMins*60000);
      nightlightActiveOld = true;
      briNlT = bri;
      for (byte i=0; i<4; i++) colNlT[i] = col[i]; // remember starting color
      if (nightlightMode == NL_MODE_SUN)
      {
        //save current
        colNlT[0] = effectCurrent;
        colNlT[1] = effectSpeed;
        colNlT[2] = effectPalette;

        strip.setMode(strip.getFirstSelectedSegId(), FX_MODE_STATIC); // make sure seg runtime is reset if it was in sunrise mode
        effectCurrent = FX_MODE_SUNRISE;
        effectSpeed = nightlightDelayMins;
        effectPalette = 0;
        if (effectSpeed > 60) effectSpeed = 60; //currently limited to 60 minutes
        if (bri) effectSpeed += 60; //sunset if currently on
        briNlT = !bri; //true == sunrise, false == sunset
        if (!bri) bri = briLast;
        colorUpdated(CALL_MODE_NO_NOTIFY);
      }
    }
    float nper = (millis() - nightlightStartTime)/((float)nightlightDelayMs);
    if (nightlightMode == NL_MODE_FADE || nightlightMode == NL_MODE_COLORFADE)
    {
      bri = briNlT + ((nightlightTargetBri - briNlT)*nper);
      if (nightlightMode == NL_MODE_COLORFADE)                                         // color fading only is enabled with "NF=2"
      {
        for (byte i=0; i<4; i++) col[i] = colNlT[i]+ ((colSec[i] - colNlT[i])*nper);   // fading from actual color to secondary color
      }
      colorUpdated(CALL_MODE_NO_NOTIFY);
    }
    if (nper >= 1) //nightlight duration over
    {
      nightlightActive = false;
      if (nightlightMode == NL_MODE_SET)
      {
        bri = nightlightTargetBri;
        colorUpdated(CALL_MODE_NO_NOTIFY);
      }
      if (bri == 0) briLast = briNlT;
      if (nightlightMode == NL_MODE_SUN)
      {
        if (!briNlT) { //turn off if sunset
          effectCurrent = colNlT[0];
          effectSpeed = colNlT[1];
          effectPalette = colNlT[2];
          toggleOnOff();
          applyFinalBri();
        }
      }

      if (macroNl > 0)
        applyPreset(macroNl);
      nightlightActiveOld = false;
    }
  } else if (nightlightActiveOld) //early de-init
  {
    if (nightlightMode == NL_MODE_SUN) { //restore previous effect
      effectCurrent = colNlT[0];
      effectSpeed = colNlT[1];
      effectPalette = colNlT[2];
      colorUpdated(CALL_MODE_NO_NOTIFY);
    }
    nightlightActiveOld = false;
  }
}

//utility for FastLED to use our custom timer
uint32_t get_millisecond_timer()
{
  return strip.now;
}
