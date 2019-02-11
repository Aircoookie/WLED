/*
 * Methods to handle saving and loading to non-volatile memory
 * EEPROM Map: https://github.com/Aircoookie/WLED/wiki/EEPROM-Map
 */

#define EEPSIZE 3072

//eeprom Version code, enables default settings instead of 0 init on update
#define EEPVER 10
//0 -> old version, default
//1 -> 0.4p 1711272 and up
//2 -> 0.4p 1711302 and up
//3 -> 0.4  1712121 and up
//4 -> 0.5.0 and up
//5 -> 0.5.1 and up
//6 -> 0.6.0 and up
//7 -> 0.7.1 and up
//8 -> 0.8.0-a and up
//9 -> 0.8.0
//10-> 0.8.2


/*
 * Erase all configuration data
 */
void clearEEPROM()
{
  for (int i = 0; i < EEPSIZE; i++)
  {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
}


void writeStringToEEPROM(uint16_t pos, char* str, uint16_t len)
{
  for (int i = 0; i < len; ++i)
  {
    EEPROM.write(pos + i, str[i]);
    if (str[i] == 0) return;
  }
}


void readStringFromEEPROM(uint16_t pos, char* str, uint16_t len)
{
  for (int i = 0; i < len; ++i)
  {
    str[i] = EEPROM.read(pos + i);
    if (str[i] == 0) return;
  }
  str[len] = 0; //make sure every string is properly terminated. str must be at least len +1 big.
}


/*
 * Write configuration to flash
 */
void saveSettingsToEEPROM()
{
  if (EEPROM.read(233) != 233) //set no first boot flag
  {
    clearEEPROM();
    EEPROM.write(233, 233);
  }
  
  writeStringToEEPROM(  0, clientSSID, 32);
  writeStringToEEPROM( 32, clientPass, 64);
  writeStringToEEPROM( 96,      cmDNS, 32);
  writeStringToEEPROM(128,     apSSID, 32);
  writeStringToEEPROM(160,     apPass, 64);

  EEPROM.write(224, nightlightDelayMinsDefault);
  EEPROM.write(225, nightlightFade);
  EEPROM.write(226, notifyDirectDefault);
  EEPROM.write(227, apChannel);
  EEPROM.write(228, apHide);
  EEPROM.write(229, ledCount & 0xFF);
  EEPROM.write(230, notifyButton);
  EEPROM.write(231, notifyTwice);
  EEPROM.write(232, buttonEnabled);
  //233 reserved for first boot flag
  
  for (int i = 0; i<4; i++) //ip addresses
  {
    EEPROM.write(234+i, staticIP[i]);
    EEPROM.write(238+i, staticGateway[i]);
    EEPROM.write(242+i, staticSubnet[i]);
  }
  
  EEPROM.write(246, colS[0]);
  EEPROM.write(247, colS[1]);
  EEPROM.write(248, colS[2]);
  EEPROM.write(249, briS);
  
  EEPROM.write(250, receiveNotificationBrightness);
  EEPROM.write(251, fadeTransition);
  EEPROM.write(252, reverseMode);
  EEPROM.write(253, transitionDelayDefault & 0xFF);
  EEPROM.write(254, (transitionDelayDefault >> 8) & 0xFF);
  EEPROM.write(255, briMultiplier);
  
  //255,250,231,230,226 notifier bytes
  writeStringToEEPROM(256, otaPass, 32);
  
  EEPROM.write(288, nightlightTargetBri);
  EEPROM.write(289, otaLock);
  EEPROM.write(290, udpPort & 0xFF);
  EEPROM.write(291, (udpPort >> 8) & 0xFF);
  writeStringToEEPROM(292, serverDescription, 32);
  
  EEPROM.write(324, effectDefault);
  EEPROM.write(325, effectSpeedDefault);
  EEPROM.write(326, effectIntensityDefault);
  
  EEPROM.write(327, ntpEnabled);
  EEPROM.write(328, currentTimezone);
  EEPROM.write(329, useAMPM);
  EEPROM.write(330, useGammaCorrectionBri);
  EEPROM.write(331, useGammaCorrectionRGB);
  EEPROM.write(332, overlayDefault);
  
  EEPROM.write(333, alexaEnabled);
  writeStringToEEPROM(334, alexaInvocationName, 32);
  EEPROM.write(366, notifyAlexa);
  
  EEPROM.write(367, (arlsOffset>=0));
  EEPROM.write(368, abs(arlsOffset));
  EEPROM.write(369, turnOnAtBoot);
  EEPROM.write(370, useHSBDefault);
  EEPROM.write(371, colS[3]); //white default
  EEPROM.write(372, useRGBW);
  EEPROM.write(373, effectPaletteDefault);
  EEPROM.write(374, strip.paletteFade);
  EEPROM.write(375, apWaitTimeSecs);
  EEPROM.write(376, recoveryAPDisabled);
  
  EEPROM.write(377, EEPVER); //eeprom was updated to latest
  
  EEPROM.write(378, colSecS[0]);
  EEPROM.write(379, colSecS[1]);
  EEPROM.write(380, colSecS[2]);
  EEPROM.write(381, colSecS[3]);
  EEPROM.write(382, strip.paletteBlend);
  EEPROM.write(383, strip.colorOrder);

  EEPROM.write(385, irEnabled);

  EEPROM.write(387, strip.ablMilliampsMax & 0xFF);
  EEPROM.write(388, (strip.ablMilliampsMax >> 8) & 0xFF);
  EEPROM.write(389, bootPreset);
  EEPROM.write(390, aOtaEnabled);
  EEPROM.write(391, receiveNotificationColor);
  EEPROM.write(392, receiveNotificationEffects);
  EEPROM.write(393, wifiLock);
  
  EEPROM.write(394, abs(utcOffsetSecs) & 0xFF);
  EEPROM.write(395, (abs(utcOffsetSecs) >> 8) & 0xFF);
  EEPROM.write(396, (utcOffsetSecs<0)); //is negative
  //397 was initLedsLast
  EEPROM.write(398, (ledCount >> 8) & 0xFF);
  EEPROM.write(399, !enableSecTransition);

  //favorite setting (preset) memory (25 slots/ each 20byte)
  //400 - 899 reserved

  for (int k=0;k<6;k++){
    int in = 900+k*8;
    writeStringToEEPROM(in, cssCol[k], 8);
  }

  EEPROM.write(948,currentTheme);
  writeStringToEEPROM(950, cssFont, 32);

  EEPROM.write(2048, huePollingEnabled);
  //EEPROM.write(2049, hueUpdatingEnabled);
  for (int i = 2050; i < 2054; ++i)
  {
    EEPROM.write(i, hueIP[i-2050]);
  }
  writeStringToEEPROM(2054, hueApiKey, 46);
  EEPROM.write(2100, huePollIntervalMs & 0xFF);
  EEPROM.write(2101, (huePollIntervalMs >> 8) & 0xFF);
  EEPROM.write(2102, notifyHue);
  EEPROM.write(2103, hueApplyOnOff);
  EEPROM.write(2104, hueApplyBri);
  EEPROM.write(2105, hueApplyColor);
  EEPROM.write(2106, huePollLightId);

  EEPROM.write(2150, overlayMin);
  EEPROM.write(2151, overlayMax);
  EEPROM.write(2152, analogClock12pixel);
  EEPROM.write(2153, analogClock5MinuteMarks);
  EEPROM.write(2154, analogClockSecondsTrail);
  
  EEPROM.write(2155, countdownMode);
  EEPROM.write(2156, countdownYear);
  EEPROM.write(2157, countdownMonth);
  EEPROM.write(2158, countdownDay);
  EEPROM.write(2159, countdownHour);
  EEPROM.write(2160, countdownMin);
  EEPROM.write(2161, countdownSec);
  setCountdown();

  writeStringToEEPROM(2165, cronixieDisplay, 6);
  EEPROM.write(2171, cronixieBacklight);
  setCronixie();
  
  EEPROM.write(2175, macroBoot);
  EEPROM.write(2176, macroAlexaOn);
  EEPROM.write(2177, macroAlexaOff);
  EEPROM.write(2178, macroButton);
  EEPROM.write(2179, macroLongPress);
  EEPROM.write(2180, macroCountdown);
  EEPROM.write(2181, macroNl);

  EEPROM.write(2190, e131Universe & 0xFF);
  EEPROM.write(2191, (e131Universe >> 8) & 0xFF);
  EEPROM.write(2192, e131Multicast);
  EEPROM.write(2193, realtimeTimeoutMs & 0xFF);
  EEPROM.write(2194, (realtimeTimeoutMs >> 8) & 0xFF);
  EEPROM.write(2195, arlsForceMaxBri);
  EEPROM.write(2196, arlsDisableGammaCorrection);

  EEPROM.write(2200, !receiveDirect);
  EEPROM.write(2201, enableRealtimeUI);
  EEPROM.write(2202, uiConfiguration);
  EEPROM.write(2203, autoRGBtoRGBW);
  EEPROM.write(2204, skipFirstLed);

  if (saveCurrPresetCycConf)
  {
    EEPROM.write(2205, presetCyclingEnabled);
    EEPROM.write(2206, presetCycleTime & 0xFF);
    EEPROM.write(2207, (presetCycleTime >> 8) & 0xFF);
    EEPROM.write(2208, presetCycleMin);
    EEPROM.write(2209, presetCycleMax);
    EEPROM.write(2210, presetApplyBri);
    EEPROM.write(2211, presetApplyCol);
    EEPROM.write(2212, presetApplyFx);
    saveCurrPresetCycConf = false;
  }

  writeStringToEEPROM(2220, blynkApiKey, 35);

  for (int i = 0; i < 8; ++i)
  {
    EEPROM.write(2260 + i, timerHours[i]  );
    EEPROM.write(2270 + i, timerMinutes[i]);
    EEPROM.write(2280 + i, timerWeekday[i]);
    EEPROM.write(2290 + i, timerMacro[i]  );
  }

  writeStringToEEPROM(2300,      mqttServer, 32);
  writeStringToEEPROM(2333, mqttDeviceTopic, 32);
  writeStringToEEPROM(2366,  mqttGroupTopic, 32);
  
  commit();
}


/*
 * Read all configuration from flash
 */
void loadSettingsFromEEPROM(bool first)
{
  if (EEPROM.read(233) != 233) //first boot/reset to default
  {
    DEBUG_PRINT("Settings invalid, restoring defaults...");
    saveSettingsToEEPROM();
    DEBUG_PRINTLN("done");
    return;
  }
  int lastEEPROMversion = EEPROM.read(377); //last EEPROM version before update
  

  readStringFromEEPROM(  0, clientSSID, 32);
  readStringFromEEPROM( 32, clientPass, 64);
  readStringFromEEPROM( 96,      cmDNS, 32);
  readStringFromEEPROM(128,     apSSID, 32);
  readStringFromEEPROM(160,     apPass, 64);

  nightlightDelayMinsDefault = EEPROM.read(224);
  nightlightDelayMins = nightlightDelayMinsDefault;
  nightlightFade = EEPROM.read(225);
  notifyDirectDefault = EEPROM.read(226);
  notifyDirect = notifyDirectDefault;
  
  apChannel = EEPROM.read(227);
  if (apChannel > 13 || apChannel < 1) apChannel = 1;
  apHide = EEPROM.read(228);
  if (apHide > 1) apHide = 1;
  ledCount = EEPROM.read(229) + ((EEPROM.read(398) << 8) & 0xFF00); if (ledCount > 1200 || ledCount == 0) ledCount = 30;
  
  notifyButton = EEPROM.read(230);
  notifyTwice = EEPROM.read(231);
  buttonEnabled = EEPROM.read(232);
  
  staticIP[0] = EEPROM.read(234);
  staticIP[1] = EEPROM.read(235);
  staticIP[2] = EEPROM.read(236);
  staticIP[3] = EEPROM.read(237);
  staticGateway[0] = EEPROM.read(238);
  staticGateway[1] = EEPROM.read(239);
  staticGateway[2] = EEPROM.read(240);
  staticGateway[3] = EEPROM.read(241);
  staticSubnet[0] = EEPROM.read(242);
  staticSubnet[1] = EEPROM.read(243);
  staticSubnet[2] = EEPROM.read(244);
  staticSubnet[3] = EEPROM.read(245);
  
  colS[0] = EEPROM.read(246); col[0] = colS[0];
  colS[1] = EEPROM.read(247); col[1] = colS[1];
  colS[2] = EEPROM.read(248); col[2] = colS[2];
  briS = EEPROM.read(249); bri = briS;
  if (!EEPROM.read(369) && first)
  {
    bri = 0; briLast = briS;
  }
  receiveNotificationBrightness = EEPROM.read(250);
  fadeTransition = EEPROM.read(251);
  reverseMode = EEPROM.read(252);
  transitionDelayDefault = EEPROM.read(253) + ((EEPROM.read(254) << 8) & 0xFF00);
  transitionDelay = transitionDelayDefault;
  briMultiplier = EEPROM.read(255);

  readStringFromEEPROM(256, otaPass, 32);
  
  nightlightTargetBri = EEPROM.read(288);
  otaLock = EEPROM.read(289);
  udpPort = EEPROM.read(290) + ((EEPROM.read(291) << 8) & 0xFF00);

  readStringFromEEPROM(292, serverDescription, 32);
  
  effectDefault = EEPROM.read(324); effectCurrent = effectDefault;
  effectSpeedDefault = EEPROM.read(325); effectSpeed = effectSpeedDefault;
  ntpEnabled = EEPROM.read(327);
  currentTimezone = EEPROM.read(328);
  useAMPM = EEPROM.read(329);
  useGammaCorrectionBri = EEPROM.read(330);
  useGammaCorrectionRGB = EEPROM.read(331);
  overlayDefault = EEPROM.read(332);
  if (lastEEPROMversion < 8 && overlayDefault > 0) overlayDefault--; //overlay mode 1 (solid) was removed
  
  alexaEnabled = EEPROM.read(333);

  readStringFromEEPROM(334, alexaInvocationName, 32);
  
  notifyAlexa = EEPROM.read(366);
  arlsOffset = EEPROM.read(368);
  if (!EEPROM.read(367)) arlsOffset = -arlsOffset;
  turnOnAtBoot = EEPROM.read(369);
  useHSBDefault = EEPROM.read(370);
  colS[3] = EEPROM.read(371); col[3] = colS[3];
  useRGBW = EEPROM.read(372);
  effectPaletteDefault = EEPROM.read(373); effectPalette = effectPaletteDefault;
  //374 - strip.paletteFade

  if (lastEEPROMversion > 0) { 
    apWaitTimeSecs = EEPROM.read(375);
    recoveryAPDisabled = EEPROM.read(376);
  }
  //377 = lastEEPROMversion
  if (lastEEPROMversion > 1) {
    for (byte i=0; i<4; i++)
    {
      colSecS[i] = EEPROM.read(378+i); colSec[i] = colSecS[i];
    }
  }
  if (lastEEPROMversion > 3) {
    effectIntensityDefault = EEPROM.read(326); effectIntensity = effectIntensityDefault; 
    aOtaEnabled = EEPROM.read(390);
    receiveNotificationColor = EEPROM.read(391);
    receiveNotificationEffects = EEPROM.read(392);

    readStringFromEEPROM(950, cssFont, 32);
  } else //keep receiving notification behavior from pre0.5.0 after update
  {
    receiveNotificationColor = receiveNotificationBrightness;
    receiveNotificationEffects = receiveNotificationBrightness;
  }
  receiveNotifications = (receiveNotificationBrightness || receiveNotificationColor || receiveNotificationEffects);
  if (lastEEPROMversion > 4) {
    huePollingEnabled = EEPROM.read(2048);
    //hueUpdatingEnabled = EEPROM.read(2049);
    for (int i = 2050; i < 2054; ++i)
    {
      hueIP[i-2050] = EEPROM.read(i);
    }

    readStringFromEEPROM(2054, hueApiKey, 46);
    
    huePollIntervalMs = EEPROM.read(2100) + ((EEPROM.read(2101) << 8) & 0xFF00);
    notifyHue = EEPROM.read(2102);
    hueApplyOnOff = EEPROM.read(2103);
    hueApplyBri = EEPROM.read(2104);
    hueApplyColor = EEPROM.read(2105);
    huePollLightId = EEPROM.read(2106);
  }
  if (lastEEPROMversion > 5) {
    overlayMin = EEPROM.read(2150);
    overlayMax = EEPROM.read(2151);
    analogClock12pixel = EEPROM.read(2152);
    analogClock5MinuteMarks = EEPROM.read(2153);
    analogClockSecondsTrail = EEPROM.read(2154);
    countdownMode = EEPROM.read(2155);
    countdownYear = EEPROM.read(2156);
    countdownMonth = EEPROM.read(2157);
    countdownDay = EEPROM.read(2158);
    countdownHour = EEPROM.read(2159);
    countdownMin = EEPROM.read(2160);
    countdownSec = EEPROM.read(2161);
    setCountdown();

    readStringFromEEPROM(2165, cronixieDisplay, 6);
    cronixieBacklight = EEPROM.read(2171);
    
    macroBoot = EEPROM.read(2175);
    macroAlexaOn = EEPROM.read(2176);
    macroAlexaOff = EEPROM.read(2177);
    macroButton = EEPROM.read(2178);
    macroLongPress = EEPROM.read(2179);
    macroCountdown = EEPROM.read(2180);
    macroNl = EEPROM.read(2181);
  }

  if (lastEEPROMversion > 6)
  {
    e131Universe = EEPROM.read(2190) + ((EEPROM.read(2191) << 8) & 0xFF00);
    e131Multicast = EEPROM.read(2192);
    realtimeTimeoutMs = EEPROM.read(2193) + ((EEPROM.read(2194) << 8) & 0xFF00);
    arlsForceMaxBri = EEPROM.read(2195);
    arlsDisableGammaCorrection = EEPROM.read(2196);
  }

  if (lastEEPROMversion > 7)
  {
    strip.paletteFade  = EEPROM.read(374);
    strip.paletteBlend = EEPROM.read(382);

    for (int i = 0; i < 8; ++i)
    {
      timerHours[i]   = EEPROM.read(2260 + i);
      timerMinutes[i] = EEPROM.read(2270 + i);
      timerWeekday[i] = EEPROM.read(2280 + i);
      timerMacro[i]   = EEPROM.read(2290 + i);
      if (timerWeekday[i] == 0) timerWeekday[i] = 255;
    }
  }

  if (lastEEPROMversion > 8)
  {
    readStringFromEEPROM(2300,      mqttServer, 32);
    readStringFromEEPROM(2333, mqttDeviceTopic, 32);
    readStringFromEEPROM(2366,  mqttGroupTopic, 32);
  }

  if (lastEEPROMversion > 9)
  {
    strip.colorOrder = EEPROM.read(383);
    irEnabled = EEPROM.read(385);
    strip.ablMilliampsMax = EEPROM.read(387) + ((EEPROM.read(388) << 8) & 0xFF00);
  } else if (lastEEPROMversion > 1) //ABL is off by default when updating from version older than 0.8.2
  {
    strip.ablMilliampsMax = 65000;
  } else {
    strip.ablMilliampsMax = ABL_MILLIAMPS_DEFAULT;
  }
  
  receiveDirect = !EEPROM.read(2200);
  enableRealtimeUI = EEPROM.read(2201);
  uiConfiguration = EEPROM.read(2202);
  
  #ifdef WLED_DISABLE_MOBILE_UI
  uiConfiguration = 1;
  //force default UI since mobile is unavailable
  #endif
  
  autoRGBtoRGBW = EEPROM.read(2203);
  skipFirstLed = EEPROM.read(2204);

  if (EEPROM.read(2210) || EEPROM.read(2211) || EEPROM.read(2212))
  {
    presetCyclingEnabled = EEPROM.read(2205);
    presetCycleTime = EEPROM.read(2206) + ((EEPROM.read(2207) << 8) & 0xFF00);
    presetCycleMin = EEPROM.read(2208);
    presetCycleMax = EEPROM.read(2209);
    presetApplyBri = EEPROM.read(2210);
    presetApplyCol = EEPROM.read(2211);
    presetApplyFx = EEPROM.read(2212);
  }
  
  bootPreset = EEPROM.read(389);
  wifiLock = EEPROM.read(393);
  utcOffsetSecs = EEPROM.read(394) + ((EEPROM.read(395) << 8) & 0xFF00);
  if (EEPROM.read(396)) utcOffsetSecs = -utcOffsetSecs; //negative
  enableSecTransition = !EEPROM.read(399);

  //favorite setting (preset) memory (25 slots/ each 20byte)
  //400 - 899 reserved

  currentTheme = EEPROM.read(948);
  for (int k=0;k<6;k++){
    int in=900+k*8;
    readStringFromEEPROM(in, cssCol[k], 8);
  }

  //custom macro memory (16 slots/ each 64byte)
  //1024-2047 reserved

  readStringFromEEPROM(2220, blynkApiKey, 35);
  
  //user MOD memory
  //2944 - 3071 reserved
  
  useHSB = useHSBDefault;

  overlayCurrent = overlayDefault;
}


//PRESET PROTOCOL 20 bytes
//0: preset purpose byte 0:invalid 1:valid preset 1.0
//1:a 2:r 3:g 4:b 5:w 6:er 7:eg 8:eb 9:ew 10:fx 11:sx | custom chase 12:numP 13:numS 14:(0:fs 1:both 2:fe) 15:step 16:ix 17: fp 18-19:Zeros

bool applyPreset(byte index, bool loadBri = true, bool loadCol = true, bool loadFX = true)
{
  if (index == 255 || index == 0)
  {
    loadSettingsFromEEPROM(false);//load boot defaults
    return true;
  }
  if (index > 25 || index < 1) return false;
  uint16_t i = 380 + index*20;
  if (EEPROM.read(i) == 0) return false;
  if (loadBri) bri = EEPROM.read(i+1);
  if (loadCol)
  {
    for (byte j=0; j<4; j++)
    {
      col[j] = EEPROM.read(i+j+2);
      colSec[j] = EEPROM.read(i+j+6);
    }
  }
  if (loadFX)
  {
    effectCurrent = EEPROM.read(i+10);
    effectSpeed = EEPROM.read(i+11);
    effectIntensity = EEPROM.read(i+16);
    effectPalette = EEPROM.read(i+17);
  }
  return true;
}

void savePreset(byte index)
{
  if (index > 25) return;
  if (index < 1) {saveSettingsToEEPROM();return;}
  uint16_t i = 380 + index*20;//min400
  EEPROM.write(i, 1);
  EEPROM.write(i+1, bri);
  for (uint16_t j=0; j<4; j++)
  {
    EEPROM.write(i+j+2, col[j]);
    EEPROM.write(i+j+6, colSec[j]);
  }
  EEPROM.write(i+10, effectCurrent);
  EEPROM.write(i+11, effectSpeed);
  
  EEPROM.write(i+16, effectIntensity);
  EEPROM.write(i+17, effectPalette);
  commit();
}


String loadMacro(byte index)
{
  index-=1;
  String m="";
  if (index > 15) return m;
  for (int i = 1024+64*index; i < 1088+64*index; i++)
  {
    if (EEPROM.read(i) == 0) break;
    m += char(EEPROM.read(i));
  }
  return m;
}


void applyMacro(byte index)
{
  index-=1;
  if (index > 15) return;
  String mc="win&";
  mc += loadMacro(index+1);
  mc += "&IN"; //internal, no XML response
  if (!notifyMacro) mc += "&NN";
  String forbidden = "&M="; //dont apply if called by the macro itself to prevent loop
  /*
   * NOTE: loop is still possible if you call a different macro from a macro, which then calls the first macro again. 
   * To prevent that, but also disable calling macros within macros, comment the next line out.
   */
  forbidden = forbidden + index;
  if (mc.indexOf(forbidden) >= 0) return;
  handleSet(mc);
}


void saveMacro(byte index, String mc, bool sing=true) //only commit on single save, not in settings
{
  index-=1;
  if (index > 15) return;
  int s = 1024+index*64;
  for (int i = s; i < s+64; i++)
  {
    EEPROM.write(i, mc.charAt(i-s));
  }
  if (sing) commit();
}


void commit()
{
  DEBUG_PRINT("s");
  //this is to support IR on ESP32, needs work
  /*#ifdef ARDUINO_ARCH_ESP32
  portMUX_TYPE mMux = portMUX_INITIALIZER_UNLOCKED;
  portENTER_CRITICAL(&mMux); 
  #endif*/

  EEPROM.commit();
  
  /*#ifdef ARDUINO_ARCH_ESP32
  portEXIT_CRITICAL(&mMux);
  #endif*/
  DEBUG_PRINT(".");
}
