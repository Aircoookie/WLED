/*
 * LED methods
 */

void setAllLeds() {
  double d = bri_t*briMultiplier;
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
    strip.setColor(gamma8[col_t[0]], gamma8[col_t[1]], gamma8[col_t[2]], gamma8[white_t]);
    strip.setSecondaryColor(gamma8[col_sec[0]], gamma8[col_sec[1]], gamma8[col_sec[2]], gamma8[white_sec]);
  } else {
    strip.setColor(col_t[0], col_t[1], col_t[2], white_t);
    strip.setSecondaryColor(col_sec[0], col_sec[1], col_sec[2], white_sec);
  }
}

void setLedsStandard()
{
  col_old[0] = col[0];
  col_old[1] = col[1];
  col_old[2] = col[2];
  white_old = white;
  bri_old = bri;
  col_t[0] = col[0];
  col_t[1] = col[1];
  col_t[2] = col[2];
  white_t = white;
  bri_t = bri;
  setAllLeds();
}

bool colorChanged()
{
  for (int i = 0; i < 3; i++)
  {
    if (col[i] != col_it[i]) return true;
    if (col_sec[i] != col_sec_it[i]) return true;
  }
  if (white != white_it || white_sec != white_sec_it) return true;
  if (bri != bri_it) return true;
  return false;
}

void colorUpdated(int callMode)
{
  //call for notifier -> 0: init 1: direct change 2: button 3: notification 4: nightlight 5: other (no not.) (NN)6: fx changed
  if (!colorChanged())
  {
    if (callMode == 6) notify(6);
    return; //no change
  }
  if (callMode != 5 && nightlightActive && nightlightFade)
  {
    bri_nl_t = bri;
    nightlightDelayMs -= (millis() - nightlightStartTime);
    nightlightStartTime = millis();
  }
  col_it[0] = col[0];
  col_it[1] = col[1];
  col_it[2] = col[2];
  col_sec_it[0] = col_sec[0];
  col_sec_it[1] = col_sec[1];
  col_sec_it[2] = col_sec[2];
  white_it = white;
  white_sec_it = white_sec;
  bri_it = bri;
  if (bri > 0) bri_last = bri;
  notify(callMode);
  if (fadeTransition || sweepTransition)
  {
    if (transitionActive)
    {
      col_old[0] = col_t[0];
      col_old[1] = col_t[1];
      col_old[2] = col_t[2];
      white_old = white_t;
      bri_old = bri_t;
      tper_last = 0;
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
      tper_last = 0;
      if (sweepTransition) strip.unlockAll();
      setLedsStandard();
      strip.setFastUpdateMode(false);
      return;
    }
    if (tper - tper_last < transitionResolution)
    {
      return;
    }
    tper_last = tper;
    if (fadeTransition)
    {
      col_t[0] = col_old[0]+((col[0] - col_old[0])*tper);
      col_t[1] = col_old[1]+((col[1] - col_old[1])*tper);
      col_t[2] = col_old[2]+((col[2] - col_old[2])*tper);
      white_t  = white_old +((white  - white_old )*tper);
      bri_t    = bri_old   +((bri    - bri_old   )*tper);
    }
    if (sweepTransition)
    {
      strip.lockAll();
      if (sweepDirection)
      {
        strip.unlockRange(0, (int)(tper*(double)ledcount));
      } else
      {
        strip.unlockRange(ledcount - (int)(tper*(double)ledcount), ledcount);
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
    if (!nightlightActive_old) //init
    {
      nightlightStartTime = millis();
      notify(4);
      nightlightDelayMs = (int)(nightlightDelayMins*60000);
      nightlightActive_old = true;
      bri_nl_t = bri;
    }
    float nper = (millis() - nightlightStartTime)/((float)nightlightDelayMs);
    if (nightlightFade)
    {
      bri = bri_nl_t+((nightlightTargetBri - bri_nl_t)*nper);
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
      if (bri == 0) bri_last = bri_nl_t;
    }
  } else if (nightlightActive_old) //early de-init
  {
    nightlightActive_old = false;
  }
}
