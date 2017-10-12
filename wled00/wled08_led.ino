/*
 * LED methods
 */

void setAllLeds() {
  double d = bri_t*bri_n;
  int val = d/100;
  if (val > 255) val = 255;
  if (useGammaCorrectionBri)
  {
    strip.setBrightness(gamma8[val]);
  } else {
    strip.setBrightness(val);
  }
  #ifdef RGBW
  if (useGammaCorrectionRGB)
  {
    strip.setColor(gamma8[col_t[0]], gamma8[col_t[1]], gamma8[col_t[2]], gamma8[white_t]);
  } else {
    strip.setColor(col_t[0], col_t[1], col_t[2], white_t);
  }
  #else
  if (useGammaCorrectionRGB)
  {
    strip.setColor(gamma8[col_t[0]], gamma8[col_t[1]], gamma8[col_t[2]]);
  } else {
    strip.setColor(col_t[0], col_t[1], col_t[2]);
  }
  #endif
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

void colorUpdated(int callMode)
{
  //call for notifier -> 0: init 1: direct change 2: button 3: notification 4: nightlight 5: other (no not.) (NN)6: fx changed
  if (col[0] == col_it[0] && col[1] == col_it[1] && col[2] == col_it[2] && white == white_it && bri == bri_it)
  {
    if (callMode == 6) notify(6);
    return; //no change
  }
  if (callMode != 5 && nightlightActive && nightlightFade)
  {
    bri_nls = bri;
    nightlightDelayMs -= (millis() - nightlightStartTime);
    nightlightStartTime = millis();
  }
  col_it[0] = col[0];
  col_it[1] = col[1];
  col_it[2] = col[2];
  white_it = white;
  bri_it = bri;
  if (bri > 0) bri_last = bri;
  notify(callMode);
  if (fadeTransition)
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
  } else
  {
    setLedsStandard();
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
      setLedsStandard();
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
    setAllLeds();
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
      bri_nls = bri;
    }
    float nper = (millis() - nightlightStartTime)/((float)nightlightDelayMs);
    if (nightlightFade)
    {
      bri = bri_nls+((bri_nl - bri_nls)*nper);
      colorUpdated(5);
    }
    if (nper >= 1)
    {
      nightlightActive = false;
      if (!nightlightFade)
      {
        bri = bri_nl;
        colorUpdated(5);
      }
      if (bri == 0) bri_last = bri_nls;
    }
  } else if (nightlightActive_old) //early de-init
  {
    nightlightActive_old = false;
  }
}
