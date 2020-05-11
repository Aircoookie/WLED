#include "src/dependencies/timezone/Timezone.h"
#include "wled.h"

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

Timezone tzCASaskatchewan(CST, CST); //Central without DST

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

TimeChangeRule IST = {Last, Sun, Mar, 1, 330};     // India Standard Time = UTC + 5.5 hours
Timezone tzIndia(IST, IST);

TimeChangeRule ACST = {First, Sun, Apr, 3, 570};   //Australian Central Standard = UTC + 9.5 hours
TimeChangeRule ACDT = {First, Sun, Oct, 2, 630};   //Australian Central Daylight = UTC + 10.5 hours
Timezone tzAUNorthern(ACST, ACST);
Timezone tzAUSouthern(ACDT, ACST);

// Pick your timezone from here.
Timezone* timezones[] = {&tzUTC, &tzUK, &tzEUCentral, &tzEUEastern, &tzUSEastern, &tzUSCentral, &tzUSMountain, &tzUSArizona, &tzUSPacific, &tzChina, &tzJapan, &tzAUEastern, &tzNZ, &tzNK, &tzIndia, &tzCASaskatchewan, &tzAUNorthern, &tzAUSouthern};  

void handleNetworkTime()
{
  if (ntpEnabled && ntpConnected && millis() - ntpLastSyncTime > 50000000L && WLED_CONNECTED)
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
  if (!ntpServerIP.fromString(ntpServerName)) //see if server is IP or domain
  {
    #ifdef ESP8266
    WiFi.hostByName(ntpServerName, ntpServerIP, 750);
    #else
    WiFi.hostByName(ntpServerName, ntpServerIP);
    #endif
  }

  DEBUG_PRINTLN("send NTP");
  byte pbuf[NTP_PACKET_SIZE];
  memset(pbuf, 0, NTP_PACKET_SIZE);

  pbuf[0] = 0b11100011;   // LI, Version, Mode
  pbuf[1] = 0;     // Stratum, or type of clock
  pbuf[2] = 6;     // Polling Interval
  pbuf[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  pbuf[12]  = 49;
  pbuf[13]  = 0x4E;
  pbuf[14]  = 49;
  pbuf[15]  = 52;

  ntpUdp.beginPacket(ntpServerIP, 123); //NTP requests are to port 123
  ntpUdp.write(pbuf, NTP_PACKET_SIZE);
  ntpUdp.endPacket();
}

bool checkNTPResponse()
{
  int cb = ntpUdp.parsePacket();
  if (cb) {
    DEBUG_PRINT("NTP recv, l=");
    DEBUG_PRINTLN(cb);
    byte pbuf[NTP_PACKET_SIZE];
    ntpUdp.read(pbuf, NTP_PACKET_SIZE); // read the packet into the buffer

    unsigned long highWord = word(pbuf[40], pbuf[41]);
    unsigned long lowWord = word(pbuf[42], pbuf[43]);
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

void getTimeString(char* out)
{
  updateLocalTime();
  byte hr = hour(local);
  if (useAMPM)
  {
    if (hr > 11) hr -= 12;
    if (hr == 0) hr  = 12;
  }
  sprintf(out,"%i-%i-%i, %i:%s%i:%s%i",year(local), month(local), day(local), 
                                       hr,(minute(local)<10)?"0":"",minute(local),
                                       (second(local)<10)?"0":"",second(local));
  if (useAMPM)
  {
    strcat(out,(hour(local) > 11)? " PM":" AM");
  }
}

void setCountdown()
{
  countdownTime = timezones[currentTimezone]->toUTC(getUnixTime(countdownHour, countdownMin, countdownSec, countdownDay, countdownMonth, countdownYear));
  if (countdownTime - now() > 0) countdownOverTriggered = false;
}

//returns true if countdown just over
bool checkCountdown()
{
  unsigned long n = now();
  if (countdownMode) local = countdownTime - n + utcOffsetSecs;
  if (n > countdownTime) {
    if (countdownMode) local = n - countdownTime + utcOffsetSecs;
    if (!countdownOverTriggered)
    {
      if (macroCountdown != 0) applyMacro(macroCountdown);
      countdownOverTriggered = true;
      return true;
    }
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
