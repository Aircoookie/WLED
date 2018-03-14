/*
 * LED methods
 */

void setAllLeds() {
  double d = briT*briMultiplier;
  int val = d/100;
  if (val > 255) val = 255;
  if (useGammaCorrectionBri)
  {
    strip.setBrightness(gamma8[val]);
  } else {
    strip.setBrightness(val);
  }
  if (useGammaCorrectionRGB)
  {
    strip.setColor(gamma8[colT[0]], gamma8[colT[1]], gamma8[colT[2]], gamma8[whiteT]);
    strip.setSecondaryColor(gamma8[colSec[0]], gamma8[colSec[1]], gamma8[colSec[2]], gamma8[whiteSec]);
  } else {
    strip.setColor(colT[0], colT[1], colT[2], whiteT);
    strip.setSecondaryColor(colSec[0], colSec[1], colSec[2], whiteSec);
  }
}

void setLedsStandard()
{
  colOld[0] = col[0];
  colOld[1] = col[1];
  colOld[2] = col[2];
  whiteOld = white;
  briOld = bri;
  colT[0] = col[0];
  colT[1] = col[1];
  colT[2] = col[2];
  whiteT = white;
  briT = bri;
  setAllLeds();
}

bool colorChanged()
{
  for (int i = 0; i < 3; i++)
  {
    if (col[i] != colIT[i]) return true;
    if (colSec[i] != colSecIT[i]) return true;
  }
  if (white != whiteIT || whiteSec != whiteSecIT) return true;
  if (bri != briIT) return true;
  return false;
}

void colorUpdated(int callMode)
{
  //call for notifier -> 0: init 1: direct change 2: button 3: notification 4: nightlight 5: other (NN)6: fx changed 7: hue
  if (!colorChanged())
  {
    if (callMode == 6) notify(6);
    return; //no change
  }
  if (callMode != 5 && nightlightActive && nightlightFade)
  {
    briNlT = bri;
    nightlightDelayMs -= (millis() - nightlightStartTime);
    nightlightStartTime = millis();
  }
  colIT[0] = col[0];
  colIT[1] = col[1];
  colIT[2] = col[2];
  colSecIT[0] = colSec[0];
  colSecIT[1] = colSec[1];
  colSecIT[2] = colSec[2];
  whiteIT = white;
  whiteSecIT = whiteSec;
  briIT = bri;
  if (bri > 0) briLast = bri;
  notify(callMode);
  if (fadeTransition || sweepTransition)
  {
    if (transitionActive)
    {
      colOld[0] = colT[0];
      colOld[1] = colT[1];
      colOld[2] = colT[2];
      whiteOld = whiteT;
      briOld = briT;
      tperLast = 0;
    }
    transitionActive = true;
    transitionStartTime = millis();
    strip.setFastUpdateMode(true);
  } else
  {
    setLedsStandard();
    strip.trigger();
  }
}

void handleTransitions()
{
  if (transitionActive && transitionDelay > 0)
  {
    float tper = (millis() - transitionStartTime)/(float)transitionDelay;
    if (tper >= 1.0)
    {
      transitionActive = false;
      tperLast = 0;
      if (sweepTransition) strip.unlockAll();
      setLedsStandard();
      strip.setFastUpdateMode(false);
      return;
    }
    if (tper - tperLast < transitionResolution)
    {
      return;
    }
    tperLast = tper;
    if (fadeTransition)
    {
      colT[0] = colOld[0]+((col[0] - colOld[0])*tper);
      colT[1] = colOld[1]+((col[1] - colOld[1])*tper);
      colT[2] = colOld[2]+((col[2] - colOld[2])*tper);
      whiteT  = whiteOld +((white  - whiteOld )*tper);
      briT    = briOld   +((bri    - briOld   )*tper);
    }
    if (sweepTransition)
    {
      strip.lockAll();
      if (sweepDirection)
      {
        strip.unlockRange(0, (int)(tper*(double)ledCount));
      } else
      {
        strip.unlockRange(ledCount - (int)(tper*(double)ledCount), ledCount);
      }
      if (!fadeTransition)
      {
        setLedsStandard();
      }
    }
    if (fadeTransition) setAllLeds();
  }
}

void handleNightlight()
{
  if (nightlightActive)
  {
    if (!nightlightActiveOld) //init
    {
      nightlightStartTime = millis();
      notify(4);
      nightlightDelayMs = (int)(nightlightDelayMins*60000);
      nightlightActiveOld = true;
      briNlT = bri;
    }
    float nper = (millis() - nightlightStartTime)/((float)nightlightDelayMs);
    if (nightlightFade)
    {
      bri = briNlT+((nightlightTargetBri - briNlT)*nper);
      colorUpdated(5);
    }
    if (nper >= 1)
    {
      nightlightActive = false;
      if (!nightlightFade)
      {
        bri = nightlightTargetBri;
        colorUpdated(5);
      }
      if (bri == 0) briLast = briNlT;
    }
  } else if (nightlightActiveOld) //early de-init
  {
    nightlightActiveOld = false;
  }
}
