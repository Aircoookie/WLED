#include <EEPROM.h>
#include "wled.h"

/*
 * Methods to handle saving and loading to non-volatile memory
 * EEPROM Map: https://github.com/Aircoookie/WLED/wiki/EEPROM-Map
 */

//eeprom Version code, enables default settings instead of 0 init on update
#define EEPVER 22
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
//11-> 0.8.5-dev #mqttauth @TimothyBrown
//12-> 0.8.7-dev
//13-> 0.9.0-dev
//14-> 0.9.0-b1
//15-> 0.9.0-b3
//16-> 0.9.1
//17-> 0.9.1-dmx
//18-> 0.9.1-e131
//19-> 0.9.1n
//20-> 0.9.1p
//21-> 0.10.1p
//22-> 2009260

void commit()
{
  if (!EEPROM.commit()) errorFlag = ERR_EEP_COMMIT;
}

/*
 * Erase all configuration data
 */
void clearEEPROM()
{
  for (int i = 0; i < EEPSIZE; i++)
  {
    EEPROM.write(i, 0);
  }
  commit();
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
  EEPROM.write(225, nightlightMode);
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

  EEPROM.write(249, briS);

  EEPROM.write(250, receiveNotificationBrightness);
  EEPROM.write(251, fadeTransition);
  EEPROM.write(252, strip.reverseMode);
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

  EEPROM.write(327, ntpEnabled);
  EEPROM.write(328, currentTimezone);
  EEPROM.write(329, useAMPM);
  EEPROM.write(330, strip.gammaCorrectBri);
  EEPROM.write(331, strip.gammaCorrectCol);
  EEPROM.write(332, overlayDefault);

  EEPROM.write(333, alexaEnabled);
  writeStringToEEPROM(334, alexaInvocationName, 32);
  EEPROM.write(366, notifyAlexa);

  EEPROM.write(367, (arlsOffset>=0));
  EEPROM.write(368, abs(arlsOffset));
  EEPROM.write(369, turnOnAtBoot);

  EEPROM.write(370, noWifiSleep);

  EEPROM.write(372, useRGBW);
  EEPROM.write(374, strip.paletteFade);
  EEPROM.write(375, strip.milliampsPerLed); //was apWaitTimeSecs up to 0.8.5
  EEPROM.write(376, apBehavior);

  EEPROM.write(377, EEPVER); //eeprom was updated to latest

  EEPROM.write(378, udpPort2 & 0xFF);
  EEPROM.write(379, (udpPort2 >> 8) & 0xFF);

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
  EEPROM.write(397, syncToggleReceive);
  EEPROM.write(398, (ledCount >> 8) & 0xFF);
  //EEPROM.write(399, was !enableSecTransition);

  //favorite setting (preset) memory (25 slots/ each 20byte)
  //400 - 940 reserved
  writeStringToEEPROM(990, ntpServerName, 32);

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
  EEPROM.write(2182, macroDoublePress);

  #ifdef WLED_ENABLE_DMX
  EEPROM.write(2185, e131ProxyUniverse & 0xFF);
  EEPROM.write(2186, (e131ProxyUniverse >> 8) & 0xFF);
  #endif

  EEPROM.write(2187, e131Port & 0xFF);
  EEPROM.write(2188, (e131Port >> 8) & 0xFF);

  EEPROM.write(2189, e131SkipOutOfSequence);
  EEPROM.write(2190, e131Universe & 0xFF);
  EEPROM.write(2191, (e131Universe >> 8) & 0xFF);
  EEPROM.write(2192, e131Multicast);
  EEPROM.write(2193, realtimeTimeoutMs & 0xFF);
  EEPROM.write(2194, (realtimeTimeoutMs >> 8) & 0xFF);
  EEPROM.write(2195, arlsForceMaxBri);
  EEPROM.write(2196, arlsDisableGammaCorrection);
  EEPROM.write(2197, DMXAddress & 0xFF);
  EEPROM.write(2198, (DMXAddress >> 8) & 0xFF);
  EEPROM.write(2199, DMXMode);

  EEPROM.write(2200, !receiveDirect);
  EEPROM.write(2201, notifyMacro); //was enableRealtime
  EEPROM.write(2203, strip.rgbwMode);
  EEPROM.write(2204, skipFirstLed);

  if (saveCurrPresetCycConf)
  {
    EEPROM.write(2205, presetCyclingEnabled);
    EEPROM.write(2206, presetCycleTime & 0xFF);
    EEPROM.write(2207, (presetCycleTime >> 8) & 0xFF);
    EEPROM.write(2208, presetCycleMin);
    EEPROM.write(2209, presetCycleMax);
    // was EEPROM.write(2210, presetApplyBri);
    // was EEPROM.write(2211, presetApplyCol);
    // was EEPROM.write(2212, presetApplyFx);
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

  EEPROM.write(2299, mqttEnabled);
  writeStringToEEPROM(2300, mqttServer, 32);
  writeStringToEEPROM(2333, mqttDeviceTopic, 32);
  writeStringToEEPROM(2366, mqttGroupTopic, 32);
  writeStringToEEPROM(2399, mqttUser, 40);
  writeStringToEEPROM(2440, mqttPass, 40);
  writeStringToEEPROM(2481, mqttClientID, 40);
  EEPROM.write(2522, mqttPort & 0xFF);
  EEPROM.write(2523, (mqttPort >> 8) & 0xFF);

  // DMX (2530 - 2549)
  #ifdef WLED_ENABLE_DMX
  EEPROM.write(2530, DMXChannels);
  EEPROM.write(2531, DMXGap & 0xFF);
  EEPROM.write(2532, (DMXGap >> 8) & 0xFF);
  EEPROM.write(2533, DMXStart & 0xFF);
  EEPROM.write(2534, (DMXStart >> 8) & 0xFF);

  for (int i=0; i<15; i++) {
    EEPROM.write(2535+i, DMXFixtureMap[i]);
  } // last used: 2549. maybe leave a few bytes for future expansion and go on with 2600 kthxbye.
  #endif

  commit();
}


/*
 * Read all configuration from flash
 */
void loadSettingsFromEEPROM()
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
  nightlightMode = EEPROM.read(225);
  notifyDirectDefault = EEPROM.read(226);
  notifyDirect = notifyDirectDefault;

  apChannel = EEPROM.read(227);
  if (apChannel > 13 || apChannel < 1) apChannel = 1;
  apHide = EEPROM.read(228);
  if (apHide > 1) apHide = 1;
  ledCount = EEPROM.read(229) + ((EEPROM.read(398) << 8) & 0xFF00); if (ledCount > MAX_LEDS || ledCount == 0) ledCount = 30;

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

  briS = EEPROM.read(249); bri = briS;
  if (!EEPROM.read(369))
  {
    bri = 0; briLast = briS;
  }
  receiveNotificationBrightness = EEPROM.read(250);
  fadeTransition = EEPROM.read(251);
  strip.reverseMode = EEPROM.read(252);
  transitionDelayDefault = EEPROM.read(253) + ((EEPROM.read(254) << 8) & 0xFF00);
  transitionDelay = transitionDelayDefault;
  briMultiplier = EEPROM.read(255);

  readStringFromEEPROM(256, otaPass, 32);

  nightlightTargetBri = EEPROM.read(288);
  otaLock = EEPROM.read(289);
  udpPort = EEPROM.read(290) + ((EEPROM.read(291) << 8) & 0xFF00);

  readStringFromEEPROM(292, serverDescription, 32);

  ntpEnabled = EEPROM.read(327);
  currentTimezone = EEPROM.read(328);
  useAMPM = EEPROM.read(329);
  strip.gammaCorrectBri = EEPROM.read(330);
  strip.gammaCorrectCol = EEPROM.read(331);
  overlayDefault = EEPROM.read(332);
  if (lastEEPROMversion < 8 && overlayDefault > 0) overlayDefault--; //overlay mode 1 (solid) was removed

  alexaEnabled = EEPROM.read(333);

  readStringFromEEPROM(334, alexaInvocationName, 32);

  notifyAlexa = EEPROM.read(366);
  arlsOffset = EEPROM.read(368);
  if (!EEPROM.read(367)) arlsOffset = -arlsOffset;
  turnOnAtBoot = EEPROM.read(369);
  useRGBW = EEPROM.read(372);
  //374 - strip.paletteFade
  
  apBehavior = EEPROM.read(376);
    
  //377 = lastEEPROMversion
  if (lastEEPROMversion > 3) {
    aOtaEnabled = EEPROM.read(390);
    receiveNotificationColor = EEPROM.read(391);
    receiveNotificationEffects = EEPROM.read(392);
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
    macroDoublePress = EEPROM.read(2182);
    if (macroDoublePress > 16) macroDoublePress = 0;
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
    readStringFromEEPROM(2300, mqttServer, 32);
    readStringFromEEPROM(2333, mqttDeviceTopic, 32);
    readStringFromEEPROM(2366, mqttGroupTopic, 32);
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

  if (lastEEPROMversion > 10)
  {
    readStringFromEEPROM(2399, mqttUser, 40);
    readStringFromEEPROM(2440, mqttPass, 40);
    readStringFromEEPROM(2481, mqttClientID, 40);
    mqttPort = EEPROM.read(2522) + ((EEPROM.read(2523) << 8) & 0xFF00);
  }

  if (lastEEPROMversion > 11)
  {
    strip.milliampsPerLed = EEPROM.read(375);
  } else if (strip.ablMilliampsMax == 65000) //65000 indicates disabled ABL in <0.8.7
  {
    strip.ablMilliampsMax = ABL_MILLIAMPS_DEFAULT;
    strip.milliampsPerLed = 0; //disable ABL
  }
  if (lastEEPROMversion > 12)
  {
    readStringFromEEPROM(990, ntpServerName, 32);
  }
  if (lastEEPROMversion > 13)
  {
    mqttEnabled = EEPROM.read(2299);
    syncToggleReceive = EEPROM.read(397);
  } else {
    mqttEnabled = true;
    syncToggleReceive = false;
  }

  if (lastEEPROMversion > 14)
  {
    DMXAddress = EEPROM.read(2197) + ((EEPROM.read(2198) << 8) & 0xFF00);
    DMXMode = EEPROM.read(2199);
  } else {
    DMXAddress = 1;
    DMXMode = DMX_MODE_MULTIPLE_RGB;
  }

  //if (lastEEPROMversion > 15)
  //{
    noWifiSleep = EEPROM.read(370);
  //}

  if (lastEEPROMversion > 17)
  {
    e131SkipOutOfSequence = EEPROM.read(2189);
  } else {
    e131SkipOutOfSequence = true;
  }

  if (lastEEPROMversion > 18)
  {
    e131Port = EEPROM.read(2187) + ((EEPROM.read(2188) << 8) & 0xFF00);
  }

  #ifdef WLED_ENABLE_DMX
  if (lastEEPROMversion > 19)
  {
    e131ProxyUniverse = EEPROM.read(2185) + ((EEPROM.read(2186) << 8) & 0xFF00);
  }
  #endif

  if (lastEEPROMversion > 21) {
    udpPort2 = EEPROM.read(378) + ((EEPROM.read(379) << 8) & 0xFF00);
  } 
  
  receiveDirect = !EEPROM.read(2200);
  notifyMacro = EEPROM.read(2201);

  strip.rgbwMode = EEPROM.read(2203);
  skipFirstLed = EEPROM.read(2204);

  if (EEPROM.read(2210) || EEPROM.read(2211) || EEPROM.read(2212))
  {
    presetCyclingEnabled = EEPROM.read(2205);
    presetCycleTime = EEPROM.read(2206) + ((EEPROM.read(2207) << 8) & 0xFF00);
    if (lastEEPROMversion < 21) presetCycleTime /= 100; //was stored in ms, now is in tenths of a second
    presetCycleMin = EEPROM.read(2208);
    presetCycleMax = EEPROM.read(2209);
    //was presetApplyBri = EEPROM.read(2210);
    //was presetApplyCol = EEPROM.read(2211);
    //was presetApplyFx = EEPROM.read(2212);
  }

  bootPreset = EEPROM.read(389);
  wifiLock = EEPROM.read(393);
  utcOffsetSecs = EEPROM.read(394) + ((EEPROM.read(395) << 8) & 0xFF00);
  if (EEPROM.read(396)) utcOffsetSecs = -utcOffsetSecs; //negative
  //!EEPROM.read(399); was enableSecTransition

  //favorite setting (preset) memory (25 slots/ each 20byte)
  //400 - 899 reserved

  //custom macro memory (16 slots/ each 64byte)
  //1024-2047 reserved

  readStringFromEEPROM(2220, blynkApiKey, 35);
  if (strlen(blynkApiKey) < 25) blynkApiKey[0] = 0;

  #ifdef WLED_ENABLE_DMX
  // DMX (2530 - 2549)2535
  DMXChannels = EEPROM.read(2530);
  DMXGap = EEPROM.read(2531) + ((EEPROM.read(2532) << 8) & 0xFF00);
  DMXStart = EEPROM.read(2533) + ((EEPROM.read(2534) << 8) & 0xFF00);
  
  for (int i=0;i<15;i++) {
    DMXFixtureMap[i] = EEPROM.read(2535+i);
  } //last used: 2549
  EEPROM.write(2550, DMXStartLED);
  #endif

  //Usermod memory
  //2551 - 2559 reserved for Usermods, usable by default
  //2560 - 2943 usable, NOT reserved (need to increase EEPSIZE accordingly, new WLED core features may override this section)
  //2944 - 3071 reserved for Usermods (need to increase EEPSIZE to 3072 in const.h)

  overlayCurrent = overlayDefault;

  savedToPresets();
}


//PRESET PROTOCOL 20 bytes
//0: preset purpose byte 0:invalid 1:valid preset 2:segment preset 2.0
//1:a 2:r 3:g 4:b 5:w 6:er 7:eg 8:eb 9:ew 10:fx 11:sx | custom chase 12:numP 13:numS 14:(0:fs 1:both 2:fe) 15:step 16:ix 17: fp 18-19:Zeros
//determines which presets already contain save data
void savedToPresets()
{
  for (byte index = 1; index < 16; index++)
  {
    uint16_t i = 380 + index*20;

    if (EEPROM.read(i) == 1) {
      savedPresets |= 0x01 << (index-1);
    } else
    {
      savedPresets &= ~(0x01 << (index-1));
    }
  }
  if (EEPROM.read(700) == 2 || EEPROM.read(700) == 3) {
    savedPresets |= 0x01 << 15;
  } else
  {
    savedPresets &= ~(0x01 << 15);
  }
}

bool applyPreset(byte index)
{
  if (fileDoc) {
    errorFlag = readObjectFromFileUsingId("/presets.json", index, fileDoc) ? ERR_NONE : ERR_FS_PLOAD;
    #ifdef WLED_DEBUG_FS
      serializeJson(*fileDoc, Serial);
    #endif
    deserializeState(fileDoc->as<JsonObject>());
  } else {
    DEBUGFS_PRINTLN(F("Make read buf"));
    DynamicJsonDocument fDoc(JSON_BUFFER_SIZE);
    errorFlag = readObjectFromFileUsingId("/presets.json", index, &fDoc) ? ERR_NONE : ERR_FS_PLOAD;
    #ifdef WLED_DEBUG_FS
      serializeJson(fDoc, Serial);
    #endif
    deserializeState(fDoc.as<JsonObject>());
  }

  if (!errorFlag) {
    currentPreset = index;
    isPreset = true;
    return true;
  }
  return false;
  /*if (index == 255 || index == 0)
  {
    loadSettingsFromEEPROM(false);//load boot defaults
    return true;
  }
  if (index > 16 || index < 1) return false;
  uint16_t i = 380 + index*20;
  byte ver = EEPROM.read(i);

  if (index < 16) {
    if (ver != 1) return false;
    strip.applyToAllSelected = true;
    if (loadBri) bri = EEPROM.read(i+1);
    
    for (byte j=0; j<4; j++)
    {
      col[j] = EEPROM.read(i+j+2);
      colSec[j] = EEPROM.read(i+j+6);
    }
    strip.setColor(2, EEPROM.read(i+12), EEPROM.read(i+13), EEPROM.read(i+14), EEPROM.read(i+15)); //tertiary color

    effectCurrent = EEPROM.read(i+10);
    effectSpeed = EEPROM.read(i+11);
    effectIntensity = EEPROM.read(i+16);
    effectPalette = EEPROM.read(i+17);
  } else {
    if (ver != 2 && ver != 3) return false;
    strip.applyToAllSelected = false;
    if (loadBri) bri = EEPROM.read(i+1);
    WS2812FX::Segment* seg = strip.getSegments();
    memcpy(seg, EEPROM.getDataPtr() +i+2, 240);
    if (ver == 2) { //versions before 2004230 did not have opacity
      for (byte j = 0; j < strip.getMaxSegments(); j++)
      {
        strip.getSegment(j).opacity = 255;
        strip.getSegment(j).setOption(SEG_OPTION_ON, 1);
      }
    }
    setValuesFromMainSeg();
  }
  currentPreset = index;
  isPreset = true;
  return true;*/
}

void savePreset(byte index, bool persist, const char* pname, JsonObject saveobj)
{
  if (index == 0) return;
  bool docAlloc = fileDoc;
  JsonObject sObj = saveobj;

  if (!docAlloc) {
    DEBUGFS_PRINTLN(F("Allocating saving buffer"));
    fileDoc = new DynamicJsonDocument(JSON_BUFFER_SIZE);
    sObj = fileDoc->to<JsonObject>();
    if (pname) sObj["n"] = pname;
  } else {
    DEBUGFS_PRINTLN(F("Reuse recv buffer"));
    sObj.remove(F("psave"));
    sObj.remove(F("v"));
  }

  if (!sObj["o"]) {
    DEBUGFS_PRINTLN(F("Save current state"));
    serializeState(sObj, true, sObj["ib"], sObj["sb"]);
    currentPreset = index;
  }
  sObj.remove("o");
  sObj.remove("ib");
  sObj.remove("sb");
  sObj.remove(F("error"));
  sObj.remove(F("time"));

  writeObjectToFileUsingId("/presets.json", index, fileDoc);
  if (!docAlloc) delete fileDoc;
  presetsModifiedTime = now(); //unix time
  updateFSInfo();
}

void deletePreset(byte index) {
  StaticJsonDocument<24> empty;
  writeObjectToFileUsingId("/presets.json", index, &empty);
  presetsModifiedTime = now(); //unix time
  updateFSInfo();
}


void loadMacro(byte index, char* m)
{
  index-=1;
  if (index > 15) return;
  readStringFromEEPROM(1024+64*index, m, 64);
}


void applyMacro(byte index)
{
  index-=1;
  if (index > 15) return;
  String mc="win&";
  char m[65];
  loadMacro(index+1, m);
  mc += m;
  mc += "&IN"; //internal, no XML response
  if (!notifyMacro) mc += "&NN";
  String forbidden = "&M="; //dont apply if called by the macro itself to prevent loop
  /*
   * NOTE: loop is still possible if you call a different macro from a macro, which then calls the first macro again.
   * To prevent that, but also disable calling macros within macros, comment the next line out.
   */
  forbidden = forbidden + index;
  if (mc.indexOf(forbidden) >= 0) return;
  handleSet(nullptr, mc);
}


void saveMacro(byte index, const String& mc, bool persist) //only commit on single save, not in settings
{
  index-=1;
  if (index > 15) return;
  int s = 1024+index*64;
  for (int i = s; i < s+64; i++)
  {
    EEPROM.write(i, mc.charAt(i-s));
  }
  if (persist) commit();
}


// De-EEPROM routine, upgrade from previous versions to v0.11
void deEEP() {
  if (WLED_FS.exists("/presets.json")) return;
  
  DEBUG_PRINTLN(F("Preset file not found, attempting to load from EEPROM"));
  DEBUGFS_PRINTLN(F("Allocating saving buffer for dEEP"));
  DynamicJsonDocument dDoc(JSON_BUFFER_SIZE);
  JsonObject sObj = dDoc.to<JsonObject>();
  sObj.createNestedObject("0");

  //EEPROM.begin(EEPSIZE);
  if (EEPROM.read(233) == 233) { //valid EEPROM save
    for (uint16_t index = 1; index <= 16; index++) { //copy presets to presets.json
      uint16_t i = 380 + index*20;
      byte ver = EEPROM.read(i);

      if ((index < 16 && ver != 1) || (index == 16 && (ver < 2 || ver > 3))) continue;

      char nbuf[16];
      sprintf(nbuf, "%d", index);

      JsonObject pObj = sObj.createNestedObject(nbuf);

      pObj["q"] = nbuf;
      sprintf_P(nbuf, "Preset %d", index);
      pObj["n"] = nbuf;

      pObj["bri"] = EEPROM.read(i+1);

      if (index < 16) {
        JsonObject segObj = pObj.createNestedObject("seg");

        JsonArray colarr = segObj.createNestedArray("col");

        byte numChannels = (useRGBW)? 4:3;

        for (uint8_t k = 0; k < 3; k++) //k=0 primary (i+2) k=1 secondary (i+6) k=2 tertiary color (i+12)
        {
          JsonArray colX = colarr.createNestedArray();
          uint16_t memloc = i + 6*k;
          if (k == 0) memloc += 2;

          for (byte j = 0; j < numChannels; j++) colX.add(EEPROM.read(memloc + j));
        }
        
        segObj[F("fx")]  = EEPROM.read(i+10);
        segObj[F("sx")]  = EEPROM.read(i+11);
        segObj[F("ix")]  = EEPROM.read(i+16);
        segObj[F("pal")] = EEPROM.read(i+17);
      } else {
        WS2812FX::Segment* seg = strip.getSegments();
        memcpy(seg, EEPROM.getDataPtr() +i+2, 240);
        if (ver == 2) { //versions before 2004230 did not have opacity
          for (byte j = 0; j < strip.getMaxSegments(); j++)
          {
            strip.getSegment(j).opacity = 255;
            strip.getSegment(j).setOption(SEG_OPTION_ON, 1);
          }
        }
        serializeState(pObj, true, false, true);

        strip.resetSegments();
      }
    }

    
    
    for (uint16_t index = 1; index <= 16; index++) { //copy macros to presets.json
      char m[65];
      readStringFromEEPROM(1024+64*(index-1), m, 64);
      if (m[0]) { //macro exists
        char nbuf[16];
        sprintf(nbuf, "%d", index + 16);
        JsonObject pObj = sObj.createNestedObject(nbuf);
        sprintf_P(nbuf, "ZMacro %d", index);
        pObj["n"] = nbuf;
        pObj["win"] = m;
      }
    }
  }

  //EEPROM.end();

  File f = WLED_FS.open("/presets.json", "w");
  if (!f) {
    errorFlag = ERR_FS_GENERAL;
    return;
  }
  serializeJson(dDoc, f);
  f.close();
  DEBUG_PRINTLN(F("deEEP complete!"));
}

//simple macro for ArduinoJSON's or syntax
#define CJSON(a,b) a = b | a

void getStringFromJson(char* dest, const char* src, size_t len) {
  if (src != nullptr) strlcpy(dest, src, len);
}

void deserializeSettings() {
  DynamicJsonDocument doc(JSON_BUFFER_SIZE);

  //deserializeJson(doc, json);

  //int rev_major = doc["rev"][0]; // 1
  //int rev_minor = doc["rev"][1]; // 0

  //long vid = doc["vid"]; // 2010020

  JsonObject id = doc["id"];
  getStringFromJson(cmDNS, id["mdns"], 33);
  getStringFromJson(serverDescription, id["name"], 33);
  getStringFromJson(alexaInvocationName, id["inv"], 33);

  JsonObject nw_ins_0 = doc["nw"]["ins"][0];
  getStringFromJson(clientSSID, nw_ins_0["ssid"], 33);
  //int nw_ins_0_pskl = nw_ins_0["pskl"];
  //The WiFi PSK is normally not contained in the regular file for security reasons.
  //If it is present however, we will use it
  getStringFromJson(clientPass, nw_ins_0["psk"], 65);

  JsonArray nw_ins_0_ip = nw_ins_0["ip"];
  JsonArray nw_ins_0_gw = nw_ins_0["gw"];
  JsonArray nw_ins_0_sn = nw_ins_0["sn"];

  for (int i = 0; i < 4; i++) {
    CJSON(staticIP[i], nw_ins_0_ip[i]);
    CJSON(staticGateway[i], nw_ins_0_gw[i]);
    CJSON(staticSubnet[i], nw_ins_0_sn[i]);
  }

  JsonObject ap = doc["ap"];
  getStringFromJson(apSSID, ap["ssid"], 33);
  getStringFromJson(apPass, ap["psk"] , 65); //normally not present due to security
  //int ap_pskl = ap["pskl"];

  CJSON(apChannel, ap["chan"]);
  if (apChannel > 13 || apChannel < 1) apChannel = 1;

  CJSON(apHide, ap["hide"]);
  if (apHide > 1) apHide = 1;

  CJSON(apBehavior, ap["behav"]);

  /*
  JsonArray ap_ip = ap["ip"];
  for (int i = 0; i < 4; i++) {
    apIP[i] = ap_ip;
  }*/

  noWifiSleep = doc["wifi"]["sleep"] | !noWifiSleep; // inverted
  noWifiSleep = !noWifiSleep;
  //int wifi_phy = doc["wifi"]["phy"]; //force phy mode n?

  JsonObject hw = doc["hw"];

  JsonObject hw_led = hw["led"];
  CJSON(ledCount, hw_led["total"]);
  if (ledCount > MAX_LEDS) ledCount = MAX_LEDS;

  CJSON(strip.ablMilliampsMax, hw_led["maxpwr"]);
  CJSON(strip.milliampsPerLed, hw_led["ledma"]);
  CJSON(strip.reverseMode, hw_led["rev"]);

  /*JsonObject hw_led_ins_0 = hw_led["ins"][0];
  bool hw_led_ins_0_en = hw_led_ins_0["en"]; // true
  int hw_led_ins_0_start = hw_led_ins_0["start"]; // 0
  int hw_led_ins_0_len = hw_led_ins_0["len"]; // 1200

  int hw_led_ins_0_pin_0 = hw_led_ins_0["pin"][0]; // 2

  int hw_led_ins_0_order = hw_led_ins_0["order"]; // 0
  bool hw_led_ins_0_rev = hw_led_ins_0["rev"]; // false
  int hw_led_ins_0_skip = hw_led_ins_0["skip"]; // 0
  int hw_led_ins_0_type = hw_led_ins_0["type"]; // 2*/

  JsonObject hw_btn_ins_0 = hw["btn"]["ins"][0];
  buttonEnabled = hw_btn_ins_0["en"] | buttonEnabled;

  //int hw_btn_ins_0_pin_0 = hw_btn_ins_0["pin"][0]; // 0

  //bool hw_btn_ins_0_rev = hw_btn_ins_0["rev"]; // false

  JsonArray hw_btn_ins_0_macros = hw_btn_ins_0["macros"];
  CJSON(macroButton, hw_btn_ins_0_macros[0]);
  CJSON(macroLongPress,hw_btn_ins_0_macros[1]);
  CJSON(macroDoublePress, hw_btn_ins_0_macros[2]);

  //int hw_btn_ins_0_type = hw_btn_ins_0["type"]; // 0

  //int hw_ir_pin = hw["ir"]["pin"]; // 4
  CJSON(irEnabled, hw["ir"]["type"]); // 0

  //int hw_relay_pin = hw["relay"]["pin"]; // 12
  //bool hw_relay_rev = hw["relay"]["rev"]; // false

  //int hw_status_pin = hw["status"]["pin"]; // -1

  JsonObject light = doc["light"];
  CJSON(briMultiplier, light["scale-bri"]);
  CJSON(strip.paletteBlend, light["pal-mode"]);

  float light_gc_bri = light["gc"]["bri"];
  float light_gc_col = light["gc"]["col"]; // 2.8
  if (light_gc_bri > 1.5) strip.gammaCorrectBri = true;
  else if (light_gc_bri > 0.5) strip.gammaCorrectBri = false;
  if (light_gc_col > 1.5) strip.gammaCorrectCol = true;
  else if (light_gc_col > 0.5) strip.gammaCorrectCol = false;

  JsonObject light_tr = light["tr"];
  CJSON(fadeTransition, light_tr["mode"]);
  CJSON(transitionDelayDefault, light_tr["dur"]); // 700
  CJSON(strip.paletteFade, light_tr["pal"]);

  JsonObject light_nl = light["nl"];
  CJSON(nightlightMode, light_nl["mode"]);
  CJSON(nightlightDelayMinsDefault, light_nl["dur"]);
  nightlightDelayMins = nightlightDelayMinsDefault;

  CJSON(nightlightTargetBri, light_nl["tbri"]);
  CJSON(macroNl, light_nl["macro"]);

  JsonObject def = doc["def"];
  CJSON(bootPreset, def["ps"]);
  CJSON(turnOnAtBoot, def["on"]); // true
  CJSON(briS, def["bri"]); // 128
  if (briS == 0) briS = 255;

  JsonObject def_cy = def["cy"];
  CJSON(presetCyclingEnabled, def_cy["on"]);

  CJSON(presetCycleMin, def_cy["range"][0]);
  CJSON(presetCycleMax, def_cy["range"][1]);

  CJSON(presetCycleTime, def_cy["dur"]);

  JsonObject interfaces = doc["if"];

  JsonObject if_sync = interfaces["sync"];
  CJSON(udpPort, if_sync["port0"]); // 21324
  CJSON(udpPort2, if_sync["port1"]); // 65506

  JsonObject if_sync_recv = if_sync["recv"];
  CJSON(receiveNotificationBrightness, if_sync_recv["bri"]);
  CJSON(receiveNotificationColor, if_sync_recv["col"]);
  CJSON(receiveNotificationEffects, if_sync_recv["fx"]);
  receiveNotifications = (receiveNotificationBrightness || receiveNotificationColor || receiveNotificationEffects);

  JsonObject if_sync_send = if_sync["send"];
  CJSON(notifyDirectDefault, if_sync_send["dir"]);
  notifyDirect = notifyDirectDefault;
  CJSON(notifyButton, if_sync_send["btn"]);
  CJSON(notifyAlexa, if_sync_send["va"]);
  CJSON(notifyHue, if_sync_send["hue"]);
  CJSON(notifyMacro, if_sync_send["macro"]);
  CJSON(notifyTwice, if_sync_send["twice"]);

  JsonObject if_realtime = interfaces["realtime"];
  CJSON(receiveDirect, if_realtime["en"]);
  CJSON(e131Port, if_realtime["port"]); // 5568
  CJSON(e131Multicast, if_realtime["mc"]);

  JsonObject if_realtime_dmx = if_realtime["dmx"];
  CJSON(e131Universe, if_realtime_dmx["uni"]);
  CJSON(e131SkipOutOfSequence, if_realtime_dmx["seqskip"]);
  CJSON(DMXAddress, if_realtime_dmx["addr"]);
  CJSON(DMXMode, if_realtime_dmx["mode"]);

  CJSON(realtimeTimeoutMs, if_realtime["timeout"]);
  CJSON(arlsForceMaxBri, if_realtime["maxbri"]);
  CJSON(arlsDisableGammaCorrection, if_realtime["no-gc"]); // false
  CJSON(arlsOffset, if_realtime["offset"]); // 0

  CJSON(alexaEnabled, interfaces["va"]["alexa"]); // false

  CJSON(macroAlexaOn, interfaces["va"]["macros"][0]);
  CJSON(macroAlexaOff, interfaces["va"]["macros"][1]);

  getStringFromJson(blynkApiKey, interfaces["blynk"]["token"], 36); //normally not present due to security

  JsonObject if_mqtt = interfaces["mqtt"];
  CJSON(mqttEnabled, if_mqtt["en"]);
  getStringFromJson(mqttServer, if_mqtt["broker"], 33);
  CJSON(mqttPort, if_mqtt["port"]); // 1883
  getStringFromJson(mqttUser, if_mqtt["user"], 41);
  getStringFromJson(mqttPass, if_mqtt["psk"], 41); //normally not present due to security
  getStringFromJson(mqttClientID, if_mqtt["cid"], 41);

  getStringFromJson(mqttDeviceTopic, if_mqtt["topics"]["device"], 33); // "wled/test"
  getStringFromJson(mqttGroupTopic, if_mqtt["topics"]["group"], 33); // "" 

  JsonObject if_hue = interfaces["hue"];
  CJSON(huePollingEnabled, if_hue["en"]);
  CJSON(huePollLightId, if_hue["id"]);
  CJSON(huePollIntervalMs, if_hue["iv"]);

  JsonObject if_hue_recv = if_hue["recv"];
  CJSON(hueApplyOnOff, if_hue_recv["on"]);
  CJSON(hueApplyBri, if_hue_recv["bri"]);
  CJSON(hueApplyColor, if_hue_recv["col"]);

  JsonArray if_hue_ip = if_hue["ip"];

  for (int i = 0; i < 4; i++)
    CJSON(hueIP[i], if_hue_ip[i]);

  JsonObject if_ntp = interfaces["ntp"];
  CJSON(ntpEnabled, if_ntp["en"]);
  getStringFromJson(ntpServerName, if_ntp["host"], 33); // "1.wled.pool.ntp.org"
  CJSON(currentTimezone, if_ntp["tz"]);
  CJSON(utcOffsetSecs, if_ntp["offset"]);

  JsonObject ol = doc["ol"];
  CJSON(overlayDefault ,ol["clock"]); // 0
  overlayCurrent = overlayDefault;

  JsonArray ol_cntdwn = ol["cntdwn"]; //[20,12,31,23,59,59]
  CJSON(countdownYear,  ol_cntdwn[0]);
  CJSON(countdownMonth, ol_cntdwn[1]);
  CJSON(countdownDay,   ol_cntdwn[2]);
  CJSON(countdownHour,  ol_cntdwn[3]);
  CJSON(countdownMin,   ol_cntdwn[4]);
  CJSON(countdownSec,   ol_cntdwn[5]);
  CJSON(macroCountdown, ol["macro"]);

  //timed macro rules
  JsonArray timers = doc["timers"]["ins"];
  uint8_t it = 0;
  for (JsonObject timer : timers) {
    if (it > 7) break;
    CJSON(timerHours[it], timer["hour"]);
    CJSON(timerMinutes[it], timer["min"]);
    CJSON(timerMacro[it], timer["macro"]);

    byte dowPrev =  timerWeekday[it];
    bool actPrev = timerWeekday[it] & 0x01;
    CJSON(timerWeekday[it], timer["dow"]);
    if (timerWeekday[it] != dowPrev) { //present in JSON
      timerWeekday[it] <<= 1; //add active bit
      bool act = timer["en"] | actPrev;
      if (act) timerWeekday[it]++;
    }

    it++;
  }

  JsonObject ota = doc["ota"];
  const char* pwd = ota["psk"]; //normally not present due to security

  bool pwdCorrect = !otaLock; //always allow access if ota not locked
  if (pwd != nullptr && strncmp(otaPass, pwd, 33) == 0) pwdCorrect = true;

  if (pwdCorrect) { //only accept these values from cfg.json if ota is unlocked (else from wsec.json)
    CJSON(otaLock, ota["lock"]);
    CJSON(wifiLock, ota["lock-wifi"]);
    CJSON(aOtaEnabled, ota["aota"]);
    getStringFromJson(otaPass, pwd, 33); //normally not present due to security
  }
}

void serializeSettings() {
  DynamicJsonDocument doc(JSON_BUFFER_SIZE);

  JsonArray rev = doc.createNestedArray("rev");
  rev.add(1); //major settings revision
  rev.add(0); //minor settings revision

  doc["vid"] = VERSION;

  JsonObject id = doc.createNestedObject("id");
  id["mdns"] = cmDNS;
  id["name"] = serverDescription;
  id["inv"] = alexaInvocationName;

  JsonObject nw = doc.createNestedObject("nw");

  JsonArray nw_ins = nw.createNestedArray("ins");

  JsonObject nw_ins_0 = nw_ins.createNestedObject();
  nw_ins_0["ssid"] = clientSSID;
  nw_ins_0["pskl"] = strlen(clientPass);

  JsonArray nw_ins_0_ip = nw_ins_0.createNestedArray("ip");
  nw_ins_0_ip.add(10);
  nw_ins_0_ip.add(10);
  nw_ins_0_ip.add(1);
  nw_ins_0_ip.add(27);

  JsonArray nw_ins_0_gw = nw_ins_0.createNestedArray("gw");
  nw_ins_0_gw.add(10);
  nw_ins_0_gw.add(10);
  nw_ins_0_gw.add(1);
  nw_ins_0_gw.add(1);

  JsonArray nw_ins_0_sn = nw_ins_0.createNestedArray("sn");
  nw_ins_0_sn.add(255);
  nw_ins_0_sn.add(255);
  nw_ins_0_sn.add(255);
  nw_ins_0_sn.add(0);

  JsonObject ap = doc.createNestedObject("ap");
  ap["ssid"] = "WLED-AP";
  ap["pskl"] = 8;
  ap["chan"] = 6;
  ap["behav"] = 0;

  JsonArray ap_ip = ap.createNestedArray("ip");
  ap_ip.add(4);
  ap_ip.add(3);
  ap_ip.add(2);
  ap_ip.add(1);

  JsonObject wifi = doc.createNestedObject("wifi");
  wifi["sleep"] = 0;
  wifi["phy"] = 0;

  JsonObject hw = doc.createNestedObject("hw");

  JsonObject hw_led = hw.createNestedObject("led");
  hw_led["total"] = 1200;
  hw_led["maxpwr"] = 0;
  hw_led["ledma"] = 55;
  hw_led["rev"] = false;

  JsonArray hw_led_ins = hw_led.createNestedArray("ins");

  JsonObject hw_led_ins_0 = hw_led_ins.createNestedObject();
  hw_led_ins_0["en"] = true;
  hw_led_ins_0["start"] = 0;
  hw_led_ins_0["len"] = 1200;
  JsonArray hw_led_ins_0_pin = hw_led_ins_0.createNestedArray("pin");
  hw_led_ins_0_pin.add(2);
  hw_led_ins_0["order"] = 0;
  hw_led_ins_0["rev"] = false;
  hw_led_ins_0["skip"] = 0;
  hw_led_ins_0["type"] = 2;

  JsonObject hw_btn = hw.createNestedObject("btn");

  JsonArray hw_btn_ins = hw_btn.createNestedArray("ins");

  JsonObject hw_btn_ins_0 = hw_btn_ins.createNestedObject();
  hw_btn_ins_0["en"] = true;
  JsonArray hw_btn_ins_0_pin = hw_btn_ins_0.createNestedArray("pin");
  hw_btn_ins_0_pin.add(0);
  hw_btn_ins_0["rev"] = false;

  JsonArray hw_btn_ins_0_macros = hw_btn_ins_0.createNestedArray("macros");
  hw_btn_ins_0_macros.add(0);
  hw_btn_ins_0_macros.add(0);
  hw_btn_ins_0_macros.add(0);
  hw_btn_ins_0["type"] = 0;

  JsonObject hw_ir = hw.createNestedObject("ir");
  hw_ir["pin"] = 4;
  hw_ir["type"] = 0;

  JsonObject hw_relay = hw.createNestedObject("relay");
  hw_relay["pin"] = 12;
  hw_relay["rev"] = false;
  JsonObject hw_status = hw.createNestedObject("status");
  hw_status["pin"] = -1;

  JsonObject light = doc.createNestedObject("light");
  light["scale-bri"] = 100;
  light["pal-mode"] = 0;

  JsonObject light_gc = light.createNestedObject("gc");
  light_gc["bri"] = 1;
  light_gc["col"] = 2.2;

  JsonObject light_tr = light.createNestedObject("tr");
  light_tr["mode"] = 1;
  light_tr["dur"] = 700;
  light_tr["pal"] = false;

  JsonObject light_nl = light.createNestedObject("nl");
  light_nl["mode"] = 3;
  light_nl["dur"] = 5;
  light_nl["tbri"] = 255;
  light_nl["macro"] = 0;

  JsonObject def = doc.createNestedObject("def");
  def["ps"] = 1;
  def["on"] = true;
  def["bri"] = 128;

  JsonObject def_cy = def.createNestedObject("cy");
  def_cy["on"] = false;

  JsonArray def_cy_range = def_cy.createNestedArray("range");
  def_cy_range.add(9);
  def_cy_range.add(12);
  def_cy["dur"] = 50;

  JsonObject interfaces = doc.createNestedObject("if");

  JsonObject if_sync = interfaces.createNestedObject("sync");
  if_sync["port0"] = 21324;
  if_sync["port1"] = 65506;

  JsonObject if_sync_recv = if_sync.createNestedObject("recv");
  if_sync_recv["bri"] = true;
  if_sync_recv["col"] = true;
  if_sync_recv["fx"] = true;

  JsonObject if_sync_send = if_sync.createNestedObject("send");
  if_sync_send["dir"] = true;
  if_sync_send["btn"] = true;
  if_sync_send["va"] = false;
  if_sync_send["hue"] = true;
  if_sync_send["macro"] = true;
  if_sync_send["twice"] = false;

  JsonObject if_realtime = interfaces.createNestedObject("realtime");
  if_realtime["en"] = true;
  if_realtime["port"] = 5568;
  if_realtime["mc"] = false;

  JsonObject if_realtime_dmx = if_realtime.createNestedObject("dmx");
  if_realtime_dmx["uni"] = 1;
  if_realtime_dmx["seqskip"] = false;
  if_realtime_dmx["addr"] = 1;
  if_realtime_dmx["mode"] = 4;
  if_realtime["timeout"] = 250;
  if_realtime["maxbri"] = true;
  if_realtime["no-gc"] = false;
  if_realtime["offset"] = 0;

  JsonObject if_va = interfaces.createNestedObject("va");
  if_va["alexa"] = false;

  JsonArray if_va_macros = if_va.createNestedArray("macros");
  if_va_macros.add(0);
  if_va_macros.add(0);
  JsonObject if_blynk = interfaces.createNestedObject("blynk");
  if_blynk["token"] = "";

  JsonObject if_mqtt = interfaces.createNestedObject("mqtt");
  if_mqtt["en"] = false;
  if_mqtt["broker"] = "";
  if_mqtt["port"] = 1883;
  if_mqtt["user"] = "";
  if_mqtt["pskl"] = 0;
  if_mqtt["cid"] = "";

  JsonObject if_mqtt_topics = if_mqtt.createNestedObject("topics");
  if_mqtt_topics["device"] = "wled/test";
  if_mqtt_topics["group"] = "";

  JsonObject if_hue = interfaces.createNestedObject("hue");
  if_hue["en"] = true;
  if_hue["id"] = 1;
  if_hue["iv"] = 250;

  JsonObject if_hue_recv = if_hue.createNestedObject("recv");
  if_hue_recv["on"] = true;
  if_hue_recv["bri"] = true;
  if_hue_recv["col"] = true;

  JsonArray if_hue_ip = if_hue.createNestedArray("ip");
  if_hue_ip.add(10);
  if_hue_ip.add(10);
  if_hue_ip.add(1);
  if_hue_ip.add(0);

  JsonObject if_ntp = interfaces.createNestedObject("ntp");
  if_ntp["en"] = true;
  if_ntp["host"] = "1.wled.pool.ntp.org";
  if_ntp["tz"] = 5;
  if_ntp["offset"] = 0;

  JsonObject ol = doc.createNestedObject("ol");
  ol["clock"] = 0;
  ol["cntdwn"] = 0;
  ol["macro"] = 0;

  JsonObject macros = doc.createNestedObject("macros");

  JsonArray macros_ins = macros.createNestedArray("ins");

  JsonObject macros_ins_0 = macros_ins.createNestedObject();
  macros_ins_0["en"] = true;
  macros_ins_0["hour"] = 0;
  macros_ins_0["min"] = 0;
  macros_ins_0["macro"] = 2;
  macros_ins_0["dow"] = 0;

  JsonObject ota = doc.createNestedObject("ota");
  ota["lock"] = false;
  ota["lock-wifi"] = true;
  ota["pskl"] = 0;
  ota["aota"] = true;

  serializeJson(doc, Serial);
}