#include "wled_ntp.h"
#include "wled.h"
#include "wled_eeprom.h" 


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
