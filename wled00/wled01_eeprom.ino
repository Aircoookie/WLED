/*
 * Methods to handle saving and loading to non-volatile memory
 * EEPROM Map: https://github.com/Aircoookie/WLED/wiki/EEPROM-Map
 */

#define EEPSIZE 3072

//eeprom Version code, enables default settings instead of 0 init on update
#define EEPVER 6
//0 -> old version, default
//1 -> 0.4p 1711272 and up
//2 -> 0.4p 1711302 and up
//3 -> 0.4  1712121 and up
//4 -> 0.5.0 and up
//5 -> 0.5.1 and up
//6 -> 0.6.0 and up

//todo add settings
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
  } else
  {
    showWelcomePage = false;
  }
  
  for (int i = 0; i < 32; ++i)
  {
    EEPROM.write(i, clientSSID.charAt(i));
  }
  for (int i = 32; i < 96; ++i)
  {
    EEPROM.write(i, clientPass.charAt(i-32));
  }
  for (int i = 96; i < 128; ++i)
  {
    EEPROM.write(i, cmDNS.charAt(i-96));
  }
  for (int i = 128; i < 160; ++i)
  {
    EEPROM.write(i, apSSID.charAt(i-128));
  }
  for (int i = 160; i < 224; ++i)
  {
    EEPROM.write(i, apPass.charAt(i-160));
  }
  EEPROM.write(224, nightlightDelayMins);
  EEPROM.write(225, nightlightFade);
  EEPROM.write(226, notifyDirectDefault);
  EEPROM.write(227, apChannel);
  EEPROM.write(228, apHide);
  EEPROM.write(229, ledCount);
  EEPROM.write(230, notifyButton);
  EEPROM.write(231, notifyTwice);
  EEPROM.write(232, buttonEnabled);
  //233 reserved for first boot flag
  EEPROM.write(234, staticIP[0]);
  EEPROM.write(235, staticIP[1]);
  EEPROM.write(236, staticIP[2]);
  EEPROM.write(237, staticIP[3]);
  EEPROM.write(238, staticGateway[0]);
  EEPROM.write(239, staticGateway[1]);
  EEPROM.write(240, staticGateway[2]);
  EEPROM.write(241, staticGateway[3]);
  EEPROM.write(242, staticSubnet[0]);
  EEPROM.write(243, staticSubnet[1]);
  EEPROM.write(244, staticSubnet[2]);
  EEPROM.write(245, staticSubnet[3]);
  EEPROM.write(246, colS[0]);
  EEPROM.write(247, colS[1]);
  EEPROM.write(248, colS[2]);
  EEPROM.write(249, briS);
  EEPROM.write(250, receiveNotificationBrightness);
  EEPROM.write(251, fadeTransition);
  EEPROM.write(252, reverseMode);
  EEPROM.write(253, (transitionDelayDefault >> 0) & 0xFF);
  EEPROM.write(254, (transitionDelayDefault >> 8) & 0xFF);
  EEPROM.write(255, briMultiplier);
  //255,250,231,230,226 notifier bytes
  for (int i = 256; i < 288; ++i)
  {
    EEPROM.write(i, otaPass.charAt(i-256));
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
  EEPROM.write(328, currentTimezone);
  EEPROM.write(329, useAMPM);
  EEPROM.write(330, useGammaCorrectionBri);
  EEPROM.write(331, useGammaCorrectionRGB);
  EEPROM.write(332, overlayDefault);
  EEPROM.write(333, alexaEnabled);
  for (int i = 334; i < 366; ++i)
  {
    EEPROM.write(i, alexaInvocationName.charAt(i-334));
  }
  EEPROM.write(366, alexaNotify);
  EEPROM.write(367, (arlsOffset>=0));
  EEPROM.write(368, abs(arlsOffset));
  EEPROM.write(369, turnOnAtBoot);
  EEPROM.write(370, useHSBDefault);
  EEPROM.write(371, whiteS);
  EEPROM.write(372, useRGBW);
  EEPROM.write(373, sweepTransition);
  EEPROM.write(374, sweepDirection);
  EEPROM.write(375, apWaitTimeSecs);
  EEPROM.write(376, recoveryAPDisabled);
  EEPROM.write(377, EEPVER); //eeprom was updated to latest
  EEPROM.write(378, colSecS[0]);
  EEPROM.write(379, colSecS[1]);
  EEPROM.write(380, colSecS[2]);
  EEPROM.write(381, whiteSecS);
  EEPROM.write(382, ccIndex1);
  EEPROM.write(383, ccIndex2);
  EEPROM.write(384, ccNumPrimary);
  EEPROM.write(385, ccNumSecondary);
  EEPROM.write(386, ccFromStart);
  EEPROM.write(387, ccFromEnd);
  EEPROM.write(388, ccStep);
  EEPROM.write(389, bootPreset);
  EEPROM.write(390, aOtaEnabled);
  EEPROM.write(391, receiveNotificationColor);
  EEPROM.write(392, receiveNotificationEffects);
  EEPROM.write(393, wifiLock);
  EEPROM.write(394, (abs(utcOffsetSecs) >> 0) & 0xFF);
  EEPROM.write(395, (abs(utcOffsetSecs) >> 8) & 0xFF);
  EEPROM.write(396, (utcOffsetSecs<0)); //is negative
  EEPROM.write(397, initLedsLast);

  for (int k=0;k<6;k++){
    int in = 900+k*8;
    for (int i=in; i < in+8; ++i)
    {
      EEPROM.write(i, cssCol[k].charAt(i-in));
    }}

  EEPROM.write(948,currentTheme);
  for (int i = 950; i < 982; ++i)
  {
    EEPROM.write(i, cssFont.charAt(i-950));
  }

  EEPROM.write(2048, huePollingEnabled);
  //EEPROM.write(2049, hueUpdatingEnabled);
  for (int i = 2050; i < 2054; ++i)
  {
    EEPROM.write(i, hueIP[i-2050]);
  }
  for (int i = 2054; i < 2100; ++i)
  {
    EEPROM.write(i, hueApiKey.charAt(i-2054));
  }
  EEPROM.write(2100, (huePollIntervalMs >> 0) & 0xFF);
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

  for (int i = 2165; i < 2171; ++i)
  {
    EEPROM.write(i, cronixieDisplay.charAt(i-2165));
  }
  EEPROM.write(2171, cronixieBacklight);
  setCronixie();
  
  EEPROM.write(2175, macroBoot);
  EEPROM.write(2176, macroAlexaOn);
  EEPROM.write(2177, macroAlexaOff);
  EEPROM.write(2178, macroButton);
  EEPROM.write(2179, macroLongPress);
  EEPROM.write(2180, macroCountdown);
  EEPROM.write(2181, macroNl);
  
  EEPROM.commit();
}

void loadSettingsFromEEPROM(bool first)
{
  if (EEPROM.read(233) != 233) //first boot/reset to default
  {
    showWelcomePage=true;
    saveSettingsToEEPROM();
    return;
  }
  int lastEEPROMversion = EEPROM.read(377); //last EEPROM version before update
  
  clientSSID = "";
  for (int i = 0; i < 32; ++i)
  {
    if (EEPROM.read(i) == 0) break;
    clientSSID += char(EEPROM.read(i));
  }
  clientPass = "";
  for (int i = 32; i < 96; ++i)
  {
    if (EEPROM.read(i) == 0) break;
    clientPass += char(EEPROM.read(i));
  }
  cmDNS = "";
  for (int i = 96; i < 128; ++i)
  {
    if (EEPROM.read(i) == 0) break;
    cmDNS += char(EEPROM.read(i));
  }
  apSSID = "";
  for (int i = 128; i < 160; ++i)
  {
    if (EEPROM.read(i) == 0) break;
    apSSID += char(EEPROM.read(i));
  }
  apPass = "";
  for (int i = 160; i < 224; ++i)
  {
    if (EEPROM.read(i) == 0) break;
    apPass += char(EEPROM.read(i));
  }
  nightlightDelayMins = EEPROM.read(224);
  nightlightFade = EEPROM.read(225);
  notifyDirectDefault = EEPROM.read(226);
  notifyDirect = notifyDirectDefault;
  apChannel = EEPROM.read(227);
  if (apChannel > 13 || apChannel < 1) apChannel = 1;
  apHide = EEPROM.read(228);
  if (apHide > 1) apHide = 1;
  ledCount = EEPROM.read(229); if (ledCount > LEDCOUNT) ledCount = LEDCOUNT;
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
  transitionDelayDefault = ((EEPROM.read(253) << 0) & 0xFF) + ((EEPROM.read(254) << 8) & 0xFF00);
  transitionDelay = transitionDelayDefault;
  briMultiplier = EEPROM.read(255);
  otaPass = "";
  for (int i = 256; i < 288; ++i)
  {
    if (EEPROM.read(i) == 0) break;
    otaPass += char(EEPROM.read(i));
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
  currentTimezone = EEPROM.read(328);
  useAMPM = EEPROM.read(329);
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
  arlsOffset = EEPROM.read(368);
  if (!EEPROM.read(367)) arlsOffset = -arlsOffset;
  turnOnAtBoot = EEPROM.read(369);
  useHSBDefault = EEPROM.read(370);
  whiteS = EEPROM.read(371); white = whiteS;
  useRGBW = EEPROM.read(372);
  sweepTransition = EEPROM.read(373);
  sweepDirection = EEPROM.read(374);
  if (lastEEPROMversion > 0) { 
    apWaitTimeSecs = EEPROM.read(375);
    recoveryAPDisabled = EEPROM.read(376);
  }
  //377 = lastEEPROMversion
  if (lastEEPROMversion > 1) {
    colSecS[0] = EEPROM.read(378); colSec[0] = colSecS[0];
    colSecS[1] = EEPROM.read(379); colSec[1] = colSecS[1];
    colSecS[2] = EEPROM.read(380); colSec[2] = colSecS[2];
    whiteSecS = EEPROM.read(381); whiteSec = whiteSecS; 
    ccIndex1 = EEPROM.read(382);
    ccIndex2 = EEPROM.read(383);
    ccNumPrimary = EEPROM.read(384);
    ccNumSecondary = EEPROM.read(385);
    ccFromStart = EEPROM.read(386);
    ccFromEnd = EEPROM.read(387);
    ccStep = EEPROM.read(388);
    strip.setCustomChase(ccIndex1, ccIndex2, ccStart, ccNumPrimary, ccNumSecondary, ccStep, ccFromStart, ccFromEnd);
  }
  if (lastEEPROMversion > 3) {
    effectIntensityDefault = EEPROM.read(326); effectIntensity = effectIntensityDefault; 
    aOtaEnabled = EEPROM.read(390);
    receiveNotificationColor = EEPROM.read(391);
    receiveNotificationEffects = EEPROM.read(392);
    cssFont = "";
    for (int i = 950; i < 982; ++i)
    {
      if (EEPROM.read(i) == 0) break;
      cssFont += char(EEPROM.read(i));
    }
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
    hueApiKey = "";
    for (int i = 2054; i < 2100; ++i)
    {
      if (EEPROM.read(i) == 0) break;
      hueApiKey += char(EEPROM.read(i));
    }
    huePollIntervalMs = ((EEPROM.read(2100) << 0) & 0xFF) + ((EEPROM.read(2101) << 8) & 0xFF00);
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

    cronixieDisplay = "";
    for (int i = 2165; i < 2171; ++i)
    {
      if (EEPROM.read(i) == 0) break;
      cronixieDisplay += char(EEPROM.read(i));
    }
    cronixieBacklight = EEPROM.read(2171);
    
    macroBoot = EEPROM.read(2175);
    macroAlexaOn = EEPROM.read(2176);
    macroAlexaOff = EEPROM.read(2177);
    macroButton = EEPROM.read(2178);
    macroLongPress = EEPROM.read(2179);
    macroCountdown = EEPROM.read(2180);
    macroNl = EEPROM.read(2181);
  }
  
  bootPreset = EEPROM.read(389);
  wifiLock = EEPROM.read(393);
  utcOffsetSecs = ((EEPROM.read(394) << 0) & 0xFF) + ((EEPROM.read(395) << 8) & 0xFF00);
  if (EEPROM.read(396)) utcOffsetSecs = -utcOffsetSecs; //negative
  initLedsLast = EEPROM.read(397);

  //favorite setting memory (25 slots/ each 20byte)
  //400 - 899 reserved

  currentTheme = EEPROM.read(948);
  for (int k=0;k<6;k++){
    int in=900+k*8;
    for (int i=in; i < in+8; ++i)
    {
      if (EEPROM.read(i) == 0) break;
      cssCol[k] += char(EEPROM.read(i));
    }}

  //custom macro memory (16 slots/ each 64byte)
  //1024-2047 reserved

  //user MOD memory
  //2944 - 3071 reserved
  
  useHSB = useHSBDefault;

  strip.setMode(effectCurrent);
  strip.setSpeed(effectSpeed);
  strip.setIntensity(effectIntensity);
  overlayCurrent = overlayDefault;
}

//PRESET PROTOCOL 20 bytes
//0: preset purpose byte 0:invalid 1:valid preset 1.0
//1:a 2:r 3:g 4:b 5:w 6:er 7:eg 8:eb 9:ew 10:fx 11:sx | custom chase 12:numP 13:numS 14:(0:fs 1:both 2:fe) 15:step 16:ix 17-19:Zeros

void applyPreset(byte index, bool loadBri, bool loadCol, bool loadFX)
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
    colSec[0] = EEPROM.read(i+6);
    colSec[1] = EEPROM.read(i+7);
    colSec[2] = EEPROM.read(i+8);
    whiteSec = EEPROM.read(i+9);
  }
  if (loadFX)
  {
    byte lastfx = effectCurrent;
    effectCurrent = EEPROM.read(i+10);
    effectSpeed = EEPROM.read(i+11);
    effectIntensity = EEPROM.read(i+16);
    ccNumPrimary = EEPROM.read(i+12);
    ccNumSecondary = EEPROM.read(i+13);
    ccFromEnd = EEPROM.read(i+14);
    ccFromStart = (EEPROM.read(i+14)<2);
    ccStep = EEPROM.read(i+15);
    strip.setCustomChase(ccIndex1, ccIndex2, ccStart, ccNumPrimary, ccNumSecondary, ccStep, ccFromStart, ccFromEnd);
    if (lastfx != effectCurrent) strip.setMode(effectCurrent);
    strip.setSpeed(effectSpeed);
    strip.setIntensity(effectIntensity);
  }
}

void savePreset(byte index)
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
  EEPROM.write(i+6, colSec[0]);
  EEPROM.write(i+7, colSec[1]);
  EEPROM.write(i+8, colSec[2]);
  EEPROM.write(i+9, whiteSec);
  EEPROM.write(i+10, effectCurrent);
  EEPROM.write(i+11, effectSpeed);
  EEPROM.write(i+12, ccNumPrimary);
  EEPROM.write(i+13, ccNumSecondary);
  byte m = 1;
  if (!ccFromStart) m = 2;
  if (!ccFromEnd) m = 0;
  EEPROM.write(i+14, m);
  EEPROM.write(i+15, ccStep);
  EEPROM.write(i+16, effectIntensity);
  EEPROM.commit();
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
  if (m.charAt(0) < 65 || m.charAt(0) > 90) return ""; //do simple check if macro is valid (capital first letter)
  return m;
}

void applyMacro(byte index)
{
  index-=1;
  if (index > 15) return;
  String mc="win&";
  mc += loadMacro(index+1);
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

void saveMacro(byte index, String mc, bool sing=true) //only commit on single save, not in settings
{
  index-=1;
  if (index > 15) return;
  int s = 1024+index*64;
  for (int i = s; i < s+64; i++)
  {
    EEPROM.write(i, mc.charAt(i-s));
  }
  if (sing) EEPROM.commit();
}

