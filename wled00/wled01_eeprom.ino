/*
 * Methods to handle saving and loading to non-volatile memory
 */

#define EEPSIZE 2048

void clearEEPROM()
{
  for (int i = 0; i < EEPSIZE; i++)
  {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
}

void saveSettingsToEEPROM()
{
  if (EEPROM.read(233) != 233) //set no first boot flag
  {
    clearEEPROM();
    EEPROM.write(233, 233);
    showWelcomePage = true;
  } else
  {
    showWelcomePage = false;
  }
  
  for (int i = 0; i < 32; ++i)
  {
    EEPROM.write(i, clientssid.charAt(i));
  }
  for (int i = 32; i < 96; ++i)
  {
    EEPROM.write(i, clientpass.charAt(i-32));
  }
  for (int i = 96; i < 128; ++i)
  {
    EEPROM.write(i, cmdns.charAt(i-96));
  }
  for (int i = 128; i < 160; ++i)
  {
    EEPROM.write(i, apssid.charAt(i-128));
  }
  for (int i = 160; i < 224; ++i)
  {
    EEPROM.write(i, appass.charAt(i-160));
  }
  EEPROM.write(224, nightlightDelayMins);
  EEPROM.write(225, nightlightFade);
  EEPROM.write(226, notifyDirectDefault);
  EEPROM.write(227, apchannel);
  EEPROM.write(228, aphide);
  EEPROM.write(229, ledcount);
  EEPROM.write(230, notifyButton);
  //231 was notifyNightlight
  EEPROM.write(232, buttonEnabled);
  //233 reserved for first boot flag
  EEPROM.write(234, staticip[0]);
  EEPROM.write(235, staticip[1]);
  EEPROM.write(236, staticip[2]);
  EEPROM.write(237, staticip[3]);
  EEPROM.write(238, staticgateway[0]);
  EEPROM.write(239, staticgateway[1]);
  EEPROM.write(240, staticgateway[2]);
  EEPROM.write(241, staticgateway[3]);
  EEPROM.write(242, staticsubnet[0]);
  EEPROM.write(243, staticsubnet[1]);
  EEPROM.write(244, staticsubnet[2]);
  EEPROM.write(245, staticsubnet[3]);
  EEPROM.write(246, col_s[0]);
  EEPROM.write(247, col_s[1]);
  EEPROM.write(248, col_s[2]);
  EEPROM.write(249, bri_s);
  EEPROM.write(250, receiveNotificationBrightness);
  EEPROM.write(251, fadeTransition);
  EEPROM.write(253, (transitionDelay >> 0) & 0xFF);
  EEPROM.write(254, (transitionDelay >> 8) & 0xFF);
  EEPROM.write(255, briMultiplier);
  //255,250,231,230,226 notifier bytes
  for (int i = 256; i < 288; ++i)
  {
    EEPROM.write(i, otapass.charAt(i-256));
  }
  EEPROM.write(288, nightlightTargetBri);
  EEPROM.write(289, otaLock);
  EEPROM.write(290, (udpPort >> 0) & 0xFF);
  EEPROM.write(291, (udpPort >> 8) & 0xFF);
  for (int i = 292; i < 324; ++i)
  {
    EEPROM.write(i, serverDescription.charAt(i-292));
  }
  EEPROM.write(324, effectDefault);
  EEPROM.write(325, effectSpeedDefault);
  EEPROM.write(326, effectIntensityDefault);
  EEPROM.write(327, ntpEnabled);
  //328 reserved for timezone setting
  //329 reserved for dst setting
  EEPROM.write(330, useGammaCorrectionBri);
  EEPROM.write(331, useGammaCorrectionRGB);
  EEPROM.write(332, overlayDefault);
  EEPROM.write(333, alexaEnabled);
  for (int i = 334; i < 366; ++i)
  {
    EEPROM.write(i, alexaInvocationName.charAt(i-334));
  }
  EEPROM.write(366, alexaNotify);
  EEPROM.write(367, arlsSign);
  EEPROM.write(368, abs(arlsOffset));
  EEPROM.write(369, turnOnAtBoot);
  EEPROM.write(370, useHSBDefault);
  EEPROM.write(371, white_s);
  EEPROM.write(372, useRGBW);
  EEPROM.write(373, sweepTransition);
  EEPROM.write(374, sweepDirection);
  EEPROM.write(375, apWaitTimeSecs);
  EEPROM.write(376, recoveryAPDisabled);
  EEPROM.write(377, EEPVER); //eeprom was updated to latest
  EEPROM.write(378, col_sec_s[0]);
  EEPROM.write(379, col_sec_s[1]);
  EEPROM.write(380, col_sec_s[2]);
  EEPROM.write(381, white_sec_s);
  EEPROM.write(382, cc_index1);
  EEPROM.write(383, cc_index2);
  EEPROM.write(384, cc_numPrimary);
  EEPROM.write(385, cc_numSecondary);
  EEPROM.write(386, cc_fromStart);
  EEPROM.write(387, cc_fromEnd);
  EEPROM.write(388, cc_step);
  EEPROM.write(389, bootPreset);
  EEPROM.write(390, aOtaEnabled);
  EEPROM.write(391, receiveNotificationColor);
  EEPROM.write(392, receiveNotificationEffects);
  if (currentTheme == 15)
  {
    for (int k=0;k<6;k++){
    for (int i = 900+k*8; i < (908+k*8); ++i)
    {
      EEPROM.write(i, cssCol[k].charAt(i-900));
    }}
  }
  EEPROM.write(948,currentTheme);
  
  EEPROM.commit();
}

void loadSettingsFromEEPROM(bool first)
{
  if (EEPROM.read(233) != 233) //first boot/reset to default
  {
    saveSettingsToEEPROM();
    return;
  }
  int lastEEPROMversion = EEPROM.read(377); //last EEPROM version before update
  
  clientssid = "";
  for (int i = 0; i < 32; ++i)
  {
    if (EEPROM.read(i) == 0) break;
    clientssid += char(EEPROM.read(i));
  }
  clientpass = "";
  for (int i = 32; i < 96; ++i)
  {
    if (EEPROM.read(i) == 0) break;
    clientpass += char(EEPROM.read(i));
  }
  cmdns = "";
  for (int i = 96; i < 128; ++i)
  {
    if (EEPROM.read(i) == 0) break;
    cmdns += char(EEPROM.read(i));
  }
  apssid = "";
  for (int i = 128; i < 160; ++i)
  {
    if (EEPROM.read(i) == 0) break;
    apssid += char(EEPROM.read(i));
  }
  appass = "";
  for (int i = 160; i < 224; ++i)
  {
    if (EEPROM.read(i) == 0) break;
    appass += char(EEPROM.read(i));
  }
  nightlightDelayMins = EEPROM.read(224);
  nightlightFade = EEPROM.read(225);
  notifyDirectDefault = EEPROM.read(226);
  notifyDirect = notifyDirectDefault;
  apchannel = EEPROM.read(227);
  if (apchannel > 13 || apchannel < 1) apchannel = 1;
  aphide = EEPROM.read(228);
  if (aphide > 1) aphide = 1;
  ledcount = EEPROM.read(229); if (ledcount > LEDCOUNT) ledcount = LEDCOUNT;
  notifyButton = EEPROM.read(230);
  //231 was notifyNightlight
  buttonEnabled = EEPROM.read(232);
  staticip[0] = EEPROM.read(234);
  staticip[1] = EEPROM.read(235);
  staticip[2] = EEPROM.read(236);
  staticip[3] = EEPROM.read(237);
  staticgateway[0] = EEPROM.read(238);
  staticgateway[1] = EEPROM.read(239);
  staticgateway[2] = EEPROM.read(240);
  staticgateway[3] = EEPROM.read(241);
  staticsubnet[0] = EEPROM.read(242);
  staticsubnet[1] = EEPROM.read(243);
  staticsubnet[2] = EEPROM.read(244);
  staticsubnet[3] = EEPROM.read(245);
  col_s[0] = EEPROM.read(246); col[0] = col_s[0];
  col_s[1] = EEPROM.read(247); col[1] = col_s[1];
  col_s[2] = EEPROM.read(248); col[2] = col_s[2];
  bri_s = EEPROM.read(249); bri = bri_s;
  if (!EEPROM.read(369) && first)
  {
    bri = 0; bri_last = bri_s;
  }
  receiveNotificationBrightness = EEPROM.read(250);
  fadeTransition = EEPROM.read(251);
  transitionDelay = ((EEPROM.read(253) << 0) & 0xFF) + ((EEPROM.read(254) << 8) & 0xFF00);
  briMultiplier = EEPROM.read(255);
  otapass = "";
  for (int i = 256; i < 288; ++i)
  {
    if (EEPROM.read(i) == 0) break;
    otapass += char(EEPROM.read(i));
  }
  nightlightTargetBri = EEPROM.read(288);
  otaLock = EEPROM.read(289);
  udpPort = ((EEPROM.read(290) << 0) & 0xFF) + ((EEPROM.read(291) << 8) & 0xFF00);
  serverDescription = "";
  for (int i = 292; i < 324; ++i)
  {
    if (EEPROM.read(i) == 0) break;
    serverDescription += char(EEPROM.read(i));
  }
  effectDefault = EEPROM.read(324); effectCurrent = effectDefault;
  effectSpeedDefault = EEPROM.read(325); effectSpeed = effectSpeedDefault;
  ntpEnabled = EEPROM.read(327);
  useGammaCorrectionBri = EEPROM.read(330);
  useGammaCorrectionRGB = EEPROM.read(331);
  overlayDefault = EEPROM.read(332);
  alexaEnabled = EEPROM.read(333);
  alexaInvocationName = "";
  for (int i = 334; i < 366; ++i)
  {
    if (EEPROM.read(i) == 0) break;
    alexaInvocationName += char(EEPROM.read(i));
  }
  alexaNotify = EEPROM.read(366);
  arlsSign = EEPROM.read(367);
  arlsOffset = EEPROM.read(368);
  if (!arlsSign) arlsOffset = -arlsOffset;
  turnOnAtBoot = EEPROM.read(369);
  useHSBDefault = EEPROM.read(370);
  white_s = EEPROM.read(371); white = white_s;
  useRGBW = EEPROM.read(372);
  sweepTransition = EEPROM.read(373);
  sweepDirection = EEPROM.read(374);
  if (lastEEPROMversion > 0) { 
    apWaitTimeSecs = EEPROM.read(375);
    recoveryAPDisabled = EEPROM.read(376);
  }
  //377 = lastEEPROMversion
  if (lastEEPROMversion > 1) {
    col_sec_s[0] = EEPROM.read(378); col_sec[0] = col_sec_s[0];
    col_sec_s[1] = EEPROM.read(379); col_sec[1] = col_sec_s[1];
    col_sec_s[2] = EEPROM.read(380); col_sec[2] = col_sec_s[2];
    white_sec_s = EEPROM.read(381); white_sec = white_sec_s; 
    cc_index1 = EEPROM.read(382);
    cc_index2 = EEPROM.read(383);
    cc_numPrimary = EEPROM.read(384);
    cc_numSecondary = EEPROM.read(385);
    cc_fromStart = EEPROM.read(386);
    cc_fromEnd = EEPROM.read(387);
    cc_step = EEPROM.read(388);
    strip.setCustomChase(cc_index1, cc_index2, cc_start, cc_numPrimary, cc_numSecondary, cc_step, cc_fromStart, cc_fromEnd);
  }
  if (lastEEPROMversion > 3) {
    effectIntensityDefault = EEPROM.read(326); effectIntensity = effectIntensityDefault; 
    aOtaEnabled = EEPROM.read(390);
    receiveNotificationColor = EEPROM.read(391);
    receiveNotificationEffects = EEPROM.read(392);
  }
  receiveNotifications = (receiveNotificationBrightness || receiveNotificationColor || receiveNotificationEffects);
  
  bootPreset = EEPROM.read(389);

  //favorite setting memory (25 slots/ each 20byte)
  //400 - 899 reserved

  currentTheme = EEPROM.read(948);
  if (currentTheme == 15)
  {
    for (int k=0;k<6;k++){
    for (int i = 900+k*8; i < (908+k*8); ++i)
    {
      if (EEPROM.read(i) == 0) break;
      cssCol[k] += char(EEPROM.read(i));
    }}
  }

  //custom macro memory (16 slots/ each 64byte)
  //1024-2047 reserved
  
  useHSB = useHSBDefault;

  strip.setMode(effectCurrent);
  strip.setSpeed(effectSpeed);
  strip.setIntensity(effectIntensity);
  overlayCurrent = overlayDefault;
}

//PRESET PROTOCOL 20 bytes
//0: preset purpose byte 0:invalid 1:valid preset 1.0
//1:a 2:r 3:g 4:b 5:w 6:er 7:eg 8:eb 9:ew 10:fx 11:sx | custom chase 12:numP 13:numS 14:(0:fs 1:both 2:fe) 15:step 16:ix 17-19:Zeros

void applyPreset(uint8_t index, bool loadBri, bool loadCol, bool loadFX)
{
  if (index == 255 || index == 0) loadSettingsFromEEPROM(false);//load boot defaults
  if (index > 25 || index < 1) return;
  uint16_t i = 380 + index*20;
  if (EEPROM.read(i) == 0) return;
  if (loadBri) bri = EEPROM.read(i+1);
  if (loadCol)
  {
    col[0] = EEPROM.read(i+2);
    col[1] = EEPROM.read(i+3);
    col[2] = EEPROM.read(i+4);
    white = EEPROM.read(i+5);
    col_sec[0] = EEPROM.read(i+6);
    col_sec[1] = EEPROM.read(i+7);
    col_sec[2] = EEPROM.read(i+8);
    white_sec = EEPROM.read(i+9);
  }
  if (loadFX)
  {
    effectCurrent = EEPROM.read(i+10);
    effectSpeed = EEPROM.read(i+11);
    effectIntensity = EEPROM.read(i+16);
    cc_numPrimary = EEPROM.read(i+12);
    cc_numSecondary = EEPROM.read(i+13);
    cc_fromEnd = EEPROM.read(i+14);
    cc_fromStart = (EEPROM.read(i+14)<2);
    cc_step = EEPROM.read(i+15);
    strip.setCustomChase(cc_index1, cc_index2, cc_start, cc_numPrimary, cc_numSecondary, cc_step, cc_fromStart, cc_fromEnd);
    strip.setMode(effectCurrent);
    strip.setSpeed(effectSpeed);
    strip.setIntensity(effectIntensity);
  }
}

void savePreset(uint8_t index)
{
  if (index > 25) return;
  if (index < 1) {saveSettingsToEEPROM();return;}
  uint16_t i = 380 + index*20;//min400
  EEPROM.write(i, 1);
  EEPROM.write(i+1, bri);
  EEPROM.write(i+2, col[0]);
  EEPROM.write(i+3, col[1]);
  EEPROM.write(i+4, col[2]);
  EEPROM.write(i+5, white);
  EEPROM.write(i+6, col_sec[0]);
  EEPROM.write(i+7, col_sec[1]);
  EEPROM.write(i+8, col_sec[2]);
  EEPROM.write(i+9, white_sec);
  EEPROM.write(i+10, effectCurrent);
  EEPROM.write(i+11, effectSpeed);
  EEPROM.write(i+12, cc_numPrimary);
  EEPROM.write(i+13, cc_numSecondary);
  uint8_t m = 1;
  if (!cc_fromStart) m = 2;
  if (!cc_fromEnd) m = 0;
  EEPROM.write(i+14, m);
  EEPROM.write(i+15, cc_step);
  EEPROM.write(i+16, effectIntensity);
  EEPROM.commit();
}

void applyMacro(uint8_t index)
{
  if (index > 15) return;
  String mc="win&";
  for (int i = 1024+64*index; i < 1088+64*index; i++)
  {
    if (EEPROM.read(i) == 0) break;
    mc += char(EEPROM.read(i));
  }
  mc += "&IN"; //internal, no XML response
  if (!macroNotify) mc += "&NN";
  String forbidden = "&M="; //dont apply if called by the macro itself to prevent loop
  /*
   * NOTE: loop is still possible if you call a different macro from a macro, which then calls the first macro again. 
   * To prevent that, but also disable calling macros within macros, comment the next line out.
   */
  forbidden = forbidden + index;
  if (mc.indexOf(forbidden) >= 0) return;
  handleSet(mc);
}

void saveMacro(uint8_t index, String mc)
{
  if (index > 15) return;
  int s = 1024+index*64;
  for (int i = s; i < s+64; i++)
  {
    EEPROM.write(i, mc.charAt(i-s));
  }
  EEPROM.commit();
}

