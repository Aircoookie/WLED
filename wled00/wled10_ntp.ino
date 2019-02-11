/*
 * Acquires time from NTP server
 */

TimeChangeRule UTCr = {Last, Sun, Mar, 1, 0};     // UTC
Timezone tzUTC(UTCr, UTCr);

TimeChangeRule BST = {Last, Sun, Mar, 1, 60};        // British Summer Time
TimeChangeRule GMT = {Last, Sun, Oct, 2, 0};         // Standard Time
Timezone tzUK(BST, GMT);

TimeChangeRule CEST = {Last, Sun, Mar, 2, 120};     //Central European Summer Time
TimeChangeRule CET = {Last, Sun, Oct, 3, 60};       //Central European Standard Time
Timezone tzEUCentral(CEST, CET);

TimeChangeRule EEST = {Last, Sun, Mar, 3, 180};     //Central European Summer Time
TimeChangeRule EET = {Last, Sun, Oct, 4, 120};       //Central European Standard Time
Timezone tzEUEastern(EEST, EET);

TimeChangeRule EDT = {Second, Sun, Mar, 2, -240 };    //Daylight time = UTC - 4 hours
TimeChangeRule EST = {First, Sun, Nov, 2, -300 };     //Standard time = UTC - 5 hours
Timezone tzUSEastern(EDT, EST);

TimeChangeRule CDT = {Second, Sun, Mar, 2, -300 };    //Daylight time = UTC - 5 hours
TimeChangeRule CST = {First, Sun, Nov, 2, -360 };     //Standard time = UTC - 6 hours
Timezone tzUSCentral(CDT, CST);

TimeChangeRule MDT = {Second, Sun, Mar, 2, -360 };    //Daylight time = UTC - 6 hours
TimeChangeRule MST = {First, Sun, Nov, 2, -420 };     //Standard time = UTC - 7 hours
Timezone tzUSMountain(MDT, MST);

Timezone tzUSArizona(MST, MST); //Mountain without DST

TimeChangeRule PDT = {Second, Sun, Mar, 2, -420 };    //Daylight time = UTC - 7 hours
TimeChangeRule PST = {First, Sun, Nov, 2, -480 };     //Standard time = UTC - 8 hours
Timezone tzUSPacific(PDT, PST);

TimeChangeRule ChST = {Last, Sun, Mar, 1, 480};     // China Standard Time = UTC + 8 hours
Timezone tzChina(ChST, ChST);

TimeChangeRule JST = {Last, Sun, Mar, 1, 540};     // Japan Standard Time = UTC + 9 hours
Timezone tzJapan(JST, JST);

TimeChangeRule AEDT = {Second, Sun, Oct, 2, 660 };    //Daylight time = UTC + 11 hours
TimeChangeRule AEST = {First, Sun, Apr, 3, 600 };     //Standard time = UTC + 10 hours
Timezone tzAUEastern(AEDT, AEST);

TimeChangeRule NZDT = {Second, Sun, Sep, 2, 780 };    //Daylight time = UTC + 13 hours
TimeChangeRule NZST = {First, Sun, Apr, 3, 720 };     //Standard time = UTC + 12 hours
Timezone tzNZ(NZDT, NZST);

TimeChangeRule NKST = {Last, Sun, Mar, 1, 510};     //Pyongyang Time = UTC + 8.5 hours
Timezone tzNK(NKST, NKST);

Timezone* timezones[] = {&tzUTC, &tzUK, &tzEUCentral, &tzEUEastern, &tzUSEastern, &tzUSCentral, &tzUSMountain, &tzUSArizona, &tzUSPacific, &tzChina, &tzJapan, &tzAUEastern, &tzNZ, &tzNK};  

void handleNetworkTime()
{
  if (ntpEnabled && ntpConnected && millis() - ntpLastSyncTime > 50000000L && WiFi.status() == WL_CONNECTED)
  {
    if (millis() - ntpPacketSentTime > 10000)
    {
      sendNTPPacket();
      ntpPacketSentTime = millis();
    }
    if (checkNTPResponse())
    {
      ntpLastSyncTime = millis();
    }
  }
}

void sendNTPPacket()
{
  WiFi.hostByName(ntpServerName, ntpServerIP);
  DEBUG_PRINTLN("send NTP packet");

  memset(obuf, 0, NTP_PACKET_SIZE);

  obuf[0] = 0b11100011;   // LI, Version, Mode
  obuf[1] = 0;     // Stratum, or type of clock
  obuf[2] = 6;     // Polling Interval
  obuf[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  obuf[12]  = 49;
  obuf[13]  = 0x4E;
  obuf[14]  = 49;
  obuf[15]  = 52;

  ntpUdp.beginPacket(ntpServerIP, 123); //NTP requests are to port 123
  ntpUdp.write((byte*)obuf, NTP_PACKET_SIZE);
  ntpUdp.endPacket();
}

bool checkNTPResponse()
{
  int cb = ntpUdp.parsePacket();
  if (cb) {
    DEBUG_PRINT("packet received, l=");
    DEBUG_PRINTLN(cb);

    ntpUdp.read(obuf, NTP_PACKET_SIZE); // read the packet into the buffer

    unsigned long highWord = word(obuf[40], obuf[41]);
    unsigned long lowWord = word(obuf[42], obuf[43]);
    if (highWord == 0 && lowWord == 0) return false;
    
    unsigned long secsSince1900 = highWord << 16 | lowWord;
 
    DEBUG_PRINT("Unix time = ");
    unsigned long epoch = secsSince1900 - 2208988799UL; //subtract 70 years -1sec (on avg. more precision)
    setTime(epoch);
    DEBUG_PRINTLN(epoch);
    if (countdownTime - now() > 0) countdownOverTriggered = false;
    return true;
  }
  return false;
}

void updateLocalTime()
{
  unsigned long tmc = now()+ utcOffsetSecs;
  local = timezones[currentTimezone]->toLocal(tmc);
}

String getTimeString()
{
  updateLocalTime();
  String ret = monthStr(month(local));
  ret = ret + " ";
  ret = ret + day(local);
  ret = ret + " ";
  ret = ret + year(local);
  ret = ret + ", ";
  ret += (useAMPM)? hour(local)%12:hour(local);
  ret = ret + ":";
  if (minute(local) < 10) ret = ret + "0";
  ret = ret + minute(local);
  ret = ret + ":";
  if (second(local) < 10) ret = ret + "0";
  ret = ret + second(local);
  if (useAMPM)
  {
    ret += (hour(local) > 11)? " PM":" AM";
  }
  return ret;
}

void setCountdown()
{
  countdownTime = timezones[currentTimezone]->toUTC(getUnixTime(countdownHour, countdownMin, countdownSec, countdownDay, countdownMonth, countdownYear));
  if (countdownTime - now() > 0) countdownOverTriggered = false;
}

//returns true if countdown just over
bool checkCountdown()
{
  long diff = countdownTime - now();
  local = abs(diff);
  if (diff <0 && !countdownOverTriggered)
  {
    if (macroCountdown != 0) applyMacro(macroCountdown);
    countdownOverTriggered = true;
    return true;
  }
  return false;
}

byte weekdayMondayFirst()
{
  byte wd = weekday(local) -1;
  if (wd == 0) wd = 7;
  return wd;
}

void checkTimers()
{
  if (lastTimerMinute != minute(local)) //only check once a new minute begins
  {
    lastTimerMinute = minute(local);
    for (uint8_t i = 0; i < 8; i++)
    {
      if (timerMacro[i] != 0
          && (timerHours[i] == hour(local) || timerHours[i] == 24) //if hour is set to 24, activate every hour 
          && timerMinutes[i] == minute(local)
          && (timerWeekday[i] & 0x01) //timer is enabled
          && timerWeekday[i] >> weekdayMondayFirst() & 0x01) //timer should activate at current day of week
      {
        applyMacro(timerMacro[i]);
      }
    }
  }
}
