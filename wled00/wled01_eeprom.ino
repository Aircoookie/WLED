/*
 * Methods to handle saving and loading to non-volatile memory
 */

void clearEEPROM()
{
  for (int i = 0; i < 1024; i++)
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
  EEPROM.write(226, notifyDirect);
  EEPROM.write(227, apchannel);
  EEPROM.write(228, aphide);
  EEPROM.write(229, ledcount);
  EEPROM.write(230, notifyButton);
  EEPROM.write(231, notifyNightlight);
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
  EEPROM.write(250, receiveNotificationsDefault);
  EEPROM.write(251, fadeTransition);
  EEPROM.write(253, (transitionDelay >> 0) & 0xFF);
  EEPROM.write(254, (transitionDelay >> 8) & 0xFF);
  EEPROM.write(255, bri_n);
  //255,250,231,230,226 notifier bytes
  for (int i = 256; i < 288; ++i)
  {
    EEPROM.write(i, otapass.charAt(i-256));
  }
  EEPROM.write(288, bri_nl);
  EEPROM.write(289, ota_lock);
  EEPROM.write(290, (udpPort >> 0) & 0xFF);
  EEPROM.write(291, (udpPort >> 8) & 0xFF);
  for (int i = 292; i < 324; ++i)
  {
    EEPROM.write(i, serverDescription.charAt(i-292));
  }
  EEPROM.write(324, effectDefault);
  EEPROM.write(325, effectSpeedDefault);
  //326 reserved for effectIntensity
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
  EEPROM.commit();
}

void loadSettingsFromEEPROM()
{
  if (EEPROM.read(233) != 233) //first boot/reset to default
  {
    saveSettingsToEEPROM();
    return;
  }
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
  notifyDirect = EEPROM.read(226);
  apchannel = EEPROM.read(227);
  if (apchannel > 13 || apchannel < 1) apchannel = 1;
  aphide = EEPROM.read(228);
  if (aphide > 1) aphide = 1;
  ledcount = EEPROM.read(229);
  notifyButton = EEPROM.read(230);
  notifyNightlight = EEPROM.read(231);
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
  if (!EEPROM.read(369))
  {
    bri = 0; bri_last = bri_s;
  }
  receiveNotifications = EEPROM.read(250);
  receiveNotificationsDefault = receiveNotifications;
  fadeTransition = EEPROM.read(251);
  transitionDelay = ((EEPROM.read(253) << 0) & 0xFF) + ((EEPROM.read(254) << 8) & 0xFF00);
  bri_n = EEPROM.read(255);
  otapass = "";
  for (int i = 256; i < 288; ++i)
  {
    if (EEPROM.read(i) == 0) break;
    otapass += char(EEPROM.read(i));
  }
  bri_nl = EEPROM.read(288);
  ota_lock = EEPROM.read(289);
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
}
