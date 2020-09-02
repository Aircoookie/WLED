#include <EEPROM.h>
#include "wled.h"

/*
 * Methods to handle saving and loading to non-volatile memory
 * EEPROM Map: https://github.com/Aircoookie/WLED/wiki/EEPROM-Map
 */

//eeprom Version code, enables default settings instead of 0 init on update
#define EEPVER 21
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

#define EEP_AUDIO 3072         // Start of Audio Reactive EEPROM Settings - END 3299

void commit()
{
  if (!EEPROM.commit()) errorFlag = 2;
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
    EEPROM.write(2210, presetApplyBri);
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

// Audio Reactive SEGMENT specific write settings
  EEPROM.write(EEP_AUDIO, soundSquelch);
  Serial.print(audioSyncPort);
  EEPROM.write(EEP_AUDIO+1, audioSyncPort & 0xFF);
  EEPROM.write(EEP_AUDIO+2, (audioSyncPort >> 8) & 0xFF);
  EEPROM.write(EEP_AUDIO+3, audioSyncEnabled);

  EEPROM.write(EEP_AUDIO+4, effectFFT1);
  EEPROM.write(EEP_AUDIO+5, effectFFT2);
  EEPROM.write(EEP_AUDIO+6, effectFFT3);

#ifndef ESP8266
  EEPROM.write(EEP_AUDIO+7, strip.matrixWidth & 0xFF);
  EEPROM.write(EEP_AUDIO+8, (strip.matrixWidth >> 8) & 0xFF);
  EEPROM.write(EEP_AUDIO+9, strip.matrixHeight & 0xFF);
  EEPROM.write(EEP_AUDIO+10, (strip.matrixHeight >> 8) & 0xFF);
  EEPROM.write(EEP_AUDIO+11, strip.matrixSerpentine);
#endif
  EEPROM.write(EEP_AUDIO+12, sampleGain);

// RESERVE 3175-3299 for FFT Preset saves and future expansion
// 3175:      FFT1
// 3176:      FFT2
// 3177:      FFT3
// 3178-3179: ZEROs

// End of Audio Reactive SEGMENT specific write settings

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
  if (!EEPROM.read(369) && first)
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
    presetApplyBri = EEPROM.read(2210);
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


// Audio Reactive specific read settings

  if (lastEEPROMversion > 20) {                                   // Version sanity checking
    soundSquelch =  EEPROM.read(EEP_AUDIO);
    audioSyncPort = EEPROM.read(EEP_AUDIO+1) + ((EEPROM.read(EEP_AUDIO+2) << 8) & 0xFF00);
    audioSyncEnabled = EEPROM.read(EEP_AUDIO + 3);

    effectFFT1 = EEPROM.read(EEP_AUDIO+4);
    effectFFT2 = EEPROM.read(EEP_AUDIO+5);
    effectFFT3 = EEPROM.read(EEP_AUDIO+6);

    #ifndef ESP8266
      strip.matrixWidth = EEPROM.read(EEP_AUDIO+7) + ((EEPROM.read(EEP_AUDIO+8) << 8) & 0xFF00); if (strip.matrixWidth == 0) strip.matrixWidth = 1;
      strip.matrixHeight = EEPROM.read(EEP_AUDIO+9) + ((EEPROM.read(EEP_AUDIO+10) << 10) & 0xFF00); if (strip.matrixHeight == 0) strip.matrixHeight = ledCount;
      strip.matrixSerpentine = EEPROM.read(EEP_AUDIO+11); // > 0;
    #endif

    sampleGain = EEPROM.read(EEP_AUDIO+12);
  }

// FFT Slider Data Preset Protocol 5 bytes, 25 "slots"
// RESERVE 3175-3299 for FFT Preset saves and future expansion
// 3175:      FFT1
// 3176:      FFT2
// 3177:      FFT3
// 3178-3179: ZEROs

// End of Audio Reactive SEGMENT specific read settings

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

bool applyPreset(byte index, bool loadBri)
{
  if (index == 255 || index == 0)
  {
    loadSettingsFromEEPROM(false);    //load boot defaults
    return true;
  }
  if (index > 16 || index < 1) return false;
  uint16_t i = 380 + index*20;        // Begin main WLED preset data - min400
  uint16_t k = 3170 + index*5;        // Begin FFT slider preset data - min3175
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
    effectFFT1 = EEPROM.read(k);      // Read FFT Slider values from EEPROM for presets
    effectFFT2 = EEPROM.read(k+1);    // Read FFT Slider values from EEPROM for presets
    effectFFT3 = EEPROM.read(k+2);    // Read FFT Slider values from EEPROM for presets
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
  return true;
}

void savePreset(byte index, bool persist)
{
  if (index > 16) return;
  if (index < 1) {saveSettingsToEEPROM();return;}
  uint16_t i = 380 + index*20;        // Begin main WLED preset data - min400
  uint16_t k = 3170 + index*5;        // Begin FFT slider preset data - min3175

  if (index < 16) {
    EEPROM.write(i, 1);
    EEPROM.write(i+1, bri);
    for (uint16_t j=0; j<4; j++)
    {
      EEPROM.write(i+j+2, col[j]);
      EEPROM.write(i+j+6, colSec[j]);
    }
    EEPROM.write(i+10, effectCurrent);
    EEPROM.write(i+11, effectSpeed);

    uint32_t colTer = strip.getSegment(strip.getMainSegmentId()).colors[2];
    EEPROM.write(i+12, (colTer >> 16) & 0xFF);
    EEPROM.write(i+13, (colTer >>  8) & 0xFF);
    EEPROM.write(i+14, (colTer >>  0) & 0xFF);
    EEPROM.write(i+15, (colTer >> 24) & 0xFF);

    EEPROM.write(i+16, effectIntensity);
    EEPROM.write(i+17, effectPalette);
    EEPROM.write(k, effectFFT1);      // Save FFT Slider values to EEPROM for presets
    EEPROM.write(k+1, effectFFT2);    // Save FFT Slider values to EEPROM for presets
    EEPROM.write(k+2, effectFFT3);    // Save FFT Slider values to EEPROM for presets

  } else { //segment 16 can save segments
    EEPROM.write(i, 3);
    EEPROM.write(i+1, bri);
    WS2812FX::Segment* seg = strip.getSegments();
    memcpy(EEPROM.getDataPtr() +i+2, seg, 240);
  }

  if (persist) commit();
  savedToPresets();
  currentPreset = index;
  isPreset = true;
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


void saveMacro(byte index, String mc, bool persist) //only commit on single save, not in settings
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
