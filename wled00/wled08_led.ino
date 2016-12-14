void setAllLeds() {
  double d = bri_t*bri_n;
  int val = d/100;
  strip.setBrightness(val);
  strip.setColor(col_t[0], col_t[1], col_t[2]);
}

void setLedsStandard()
{
  col_old[0] = col[0];
  col_old[1] = col[1];
  col_old[2] = col[2];
  bri_old = bri;
  col_t[0] = col[0];
  col_t[1] = col[1];
  col_t[2] = col[2];
  bri_t = bri;
  setAllLeds();
}

void colorUpdated(int callMode)
{
  //call for notifier -> 0: init 1: direct change 2: button 3: notification 4: nightlight 5: other (no not.)
  if (col[0] == col_it[0] && col[1] == col_it[1] && col[2] == col_it[2] && bri == bri_it)
  {
    if (callMode == 6) notify(6);
    return; //no change
  }
  col_it[0] = col[0];
  col_it[1] = col[1];
  col_it[2] = col[2];
  bri_it = bri;
  if (bri > 0) bri_last = bri;
  notify(callMode);
  if (fadeTransition || seqTransition)
  {
    if (transitionActive)
    {
      col_old[0] = col_t[0];
      col_old[1] = col_t[1];
      col_old[2] = col_t[2];
      bri_old = bri_t;
    }
    transitionActive = true;
    transitionStartTime = millis();
    transitionDelay = transitionDelay_old;
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
      if (nightlightActive && nightlightFade)
      {
        initNightlightFade();
      }
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
      bri_t = bri_old+((bri - bri_old)*tper);
    }
    if (seqTransition)
    {
      
    } else setAllLeds();
  }
}

void initNightlightFade()
{
  float nper = (millis() - nightlightStartTime)/((float)nightlightDelayMs);
  nightlightDelayMs = nightlightDelayMs*(1-nper);
  if (nper >= 1)
  {
    return;
  }
  bri = bri_nl;
  bri_it = bri_nl;
  transitionDelay = (int)(nightlightDelayMins*60000);
  transitionStartTime = nightlightStartTime;
  transitionActive = true;
  nightlightStartTime = millis();
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
      if (nightlightFade)
      {
        initNightlightFade();
      }
    }
    float nper = (millis() - nightlightStartTime)/((float)nightlightDelayMs);
    if (nper >= 1)
    {
      nightlightActive = false;
      if (!nightlightFade)
      {
        bri = bri_nl;
        colorUpdated(5);
      }
    }
  } else if (nightlightActive_old) //early de-init
  {
    nightlightActive_old = false;
    if (nightlightFade)
    {
      transitionDelay = transitionDelay_old;
      transitionActive = false;
    }
  }
}
