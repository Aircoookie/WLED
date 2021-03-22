#include "src/dependencies/timezone/Timezone.h"
#include "wled.h"

/*
 * Acquires time from NTP server
 */
Timezone* tz;

#define TZ_UTC                  0
#define TZ_UK                   1
#define TZ_EUROPE_CENTRAL       2
#define TZ_EUROPE_EASTERN       3
#define TZ_US_EASTERN           4
#define TZ_US_CENTRAL           5
#define TZ_US_MOUNTAIN          6
#define TZ_US_ARIZONA           7
#define TZ_US_PACIFIC           8
#define TZ_CHINA                9
#define TZ_JAPAN               10
#define TZ_AUSTRALIA_EASTERN   11
#define TZ_NEW_ZEALAND         12
#define TZ_NORTH_KOREA         13
#define TZ_INDIA               14
#define TZ_SASKACHEWAN         15
#define TZ_AUSTRALIA_NORTHERN  16
#define TZ_AUSTRALIA_SOUTHERN  17
#define TZ_HAWAII              18
#define TZ_INIT               255

byte tzCurrent = TZ_INIT; //uninitialized

void updateTimezone() {
  delete tz;
  TimeChangeRule tcrDaylight = {Last, Sun, Mar, 1, 0}; //UTC
  TimeChangeRule tcrStandard = tcrDaylight;            //UTC

  switch (currentTimezone) {
    case TZ_UK : {
      tcrDaylight = {Last, Sun, Mar, 1, 60};      //British Summer Time
      tcrStandard = {Last, Sun, Oct, 2, 0};       //Standard Time
      break;
    }
    case TZ_EUROPE_CENTRAL : {
      tcrDaylight = {Last, Sun, Mar, 2, 120};     //Central European Summer Time
      tcrStandard = {Last, Sun, Oct, 3, 60};      //Central European Standard Time
      break;
    }
    case TZ_EUROPE_EASTERN : {
      tcrDaylight = {Last, Sun, Mar, 3, 180};     //East European Summer Time
      tcrStandard = {Last, Sun, Oct, 4, 120};     //East European Standard Time
      break;
    }
    case TZ_US_EASTERN : {
      tcrDaylight = {Second, Sun, Mar, 2, -240};  //EDT = UTC - 4 hours
      tcrStandard = {First,  Sun, Nov, 2, -300};  //EST = UTC - 5 hours
      break;
    }
    case TZ_US_CENTRAL : {
      tcrDaylight = {Second, Sun, Mar, 2, -300};  //CDT = UTC - 5 hours
      tcrStandard = {First,  Sun, Nov, 2, -360};  //CST = UTC - 6 hours
      break;
    }
    case TZ_US_MOUNTAIN : {
      tcrDaylight = {Second, Sun, Mar, 2, -360};  //MDT = UTC - 6 hours
      tcrStandard = {First,  Sun, Nov, 2, -420};  //MST = UTC - 7 hours
      break;
    }
    case TZ_US_ARIZONA : {
      tcrDaylight = {First,  Sun, Nov, 2, -420};  //MST = UTC - 7 hours
      tcrStandard = {First,  Sun, Nov, 2, -420};  //MST = UTC - 7 hours
      break;
    }
    case TZ_US_PACIFIC : {
      tcrDaylight = {Second, Sun, Mar, 2, -420};  //PDT = UTC - 7 hours
      tcrStandard = {First,  Sun, Nov, 2, -480};  //PST = UTC - 8 hours
      break;
    }
    case TZ_CHINA : {
      tcrDaylight = {Last, Sun, Mar, 1, 480};     //CST = UTC + 8 hours
      tcrStandard = tcrDaylight;
      break;
    }
    case TZ_JAPAN : {
      tcrDaylight = {Last, Sun, Mar, 1, 540};     //JST = UTC + 9 hours
      tcrStandard = tcrDaylight;
      break;
    }
    case TZ_AUSTRALIA_EASTERN : {
      tcrDaylight = {Second, Sun, Oct, 2, 660};   //AEDT = UTC + 11 hours
      tcrStandard = {First,  Sun, Apr, 3, 600};   //AEST = UTC + 10 hours
      break;
    }
    case TZ_NEW_ZEALAND : {
      tcrDaylight = {Second, Sun, Sep, 2, 780};   //NZDT = UTC + 13 hours
      tcrStandard = {First,  Sun, Apr, 3, 720};   //NZST = UTC + 12 hours
      break;
    }
    case TZ_NORTH_KOREA : {
      tcrDaylight = {Last, Sun, Mar, 1, 510};     //Pyongyang Time = UTC + 8.5 hours
      tcrStandard = tcrDaylight;
      break;
    }
    case TZ_INDIA : {
      tcrDaylight = {Last, Sun, Mar, 1, 330};     //India Standard Time = UTC + 5.5 hours
      tcrStandard = tcrDaylight;
      break;
    }
    case TZ_SASKACHEWAN : {
      tcrDaylight = {First,  Sun, Nov, 2, -360};  //CST = UTC - 6 hours
      tcrStandard = tcrDaylight;
      break;
    }
    case TZ_AUSTRALIA_NORTHERN : {
      tcrStandard = {First, Sun, Apr, 3, 570};   //ACST = UTC + 9.5 hours
      tcrStandard = tcrDaylight;
      break;
    }
    case TZ_AUSTRALIA_SOUTHERN : {
      tcrDaylight = {First, Sun, Oct, 2, 630};   //ACDT = UTC + 10.5 hours
      tcrStandard = {First, Sun, Apr, 3, 570};   //ACST = UTC + 9.5 hours
      break;
    }
    case TZ_HAWAII : {
      tcrDaylight = {Last, Sun, Mar, 1, -600};   //HST =  UTC - 10 hours
      tcrStandard = tcrDaylight;
      break;
    }
  }

  tzCurrent = currentTimezone;

  tz = new Timezone(tcrDaylight, tcrStandard);
}

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

  DEBUG_PRINTLN(F("send NTP"));
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
    DEBUG_PRINT(F("NTP recv, l="));
    DEBUG_PRINTLN(cb);
    byte pbuf[NTP_PACKET_SIZE];
    ntpUdp.read(pbuf, NTP_PACKET_SIZE); // read the packet into the buffer

    unsigned long highWord = word(pbuf[40], pbuf[41]);
    unsigned long lowWord = word(pbuf[42], pbuf[43]);
    if (highWord == 0 && lowWord == 0) return false;
    
    unsigned long secsSince1900 = highWord << 16 | lowWord;
 
    DEBUG_PRINT(F("Unix time = "));
    unsigned long epoch = secsSince1900 - 2208988799UL; //subtract 70 years -1sec (on avg. more precision)
    setTime(epoch);
    DEBUG_PRINTLN(epoch);
    if (countdownTime - now() > 0) countdownOverTriggered = false;
    // if time changed re-calculate sunrise/sunset
    updateLocalTime();
    calculateSunriseAndSunset();
    return true;
  }
  return false;
}

void updateLocalTime()
{
  if (currentTimezone != tzCurrent) updateTimezone();
  unsigned long tmc = now()+ utcOffsetSecs;
  localTime = tz->toLocal(tmc);
}

void getTimeString(char* out)
{
  updateLocalTime();
  byte hr = hour(localTime);
  if (useAMPM)
  {
    if (hr > 11) hr -= 12;
    if (hr == 0) hr  = 12;
  }
  sprintf_P(out,PSTR("%i-%i-%i, %02d:%02d:%02d"),year(localTime), month(localTime), day(localTime), hr, minute(localTime), second(localTime));
  if (useAMPM)
  {
    strcat(out,(hour(localTime) > 11)? " PM":" AM");
  }
}

void setCountdown()
{
  if (currentTimezone != tzCurrent) updateTimezone();
  countdownTime = tz->toUTC(getUnixTime(countdownHour, countdownMin, countdownSec, countdownDay, countdownMonth, countdownYear));
  if (countdownTime - now() > 0) countdownOverTriggered = false;
}

//returns true if countdown just over
bool checkCountdown()
{
  unsigned long n = now();
  if (countdownMode) localTime = countdownTime - n + utcOffsetSecs;
  if (n > countdownTime) {
    if (countdownMode) localTime = n - countdownTime + utcOffsetSecs;
    if (!countdownOverTriggered)
    {
      if (macroCountdown != 0) applyPreset(macroCountdown);
      countdownOverTriggered = true;
      return true;
    }
  }
  return false;
}

byte weekdayMondayFirst()
{
  byte wd = weekday(localTime) -1;
  if (wd == 0) wd = 7;
  return wd;
}

void checkTimers()
{
  if (lastTimerMinute != minute(localTime)) //only check once a new minute begins
  {
    lastTimerMinute = minute(localTime);

    // re-calculate sunrise and sunset just after midnight
    if (!hour(localTime) && minute(localTime)==1) calculateSunriseAndSunset();
    if (sunrise && sunset) daytime = difftime(localTime, sunrise) > 0 && difftime(localTime, sunset) < 0;

    DEBUG_PRINTF("Local time: %02d:%02d\n", hour(localTime), minute(localTime));
    for (uint8_t i = 0; i < 8; i++)
    {
      if (timerMacro[i] != 0
          && (timerHours[i] == hour(localTime) || timerHours[i] == 24) //if hour is set to 24, activate every hour 
          && timerMinutes[i] == minute(localTime)
          && (timerWeekday[i] & 0x01) //timer is enabled
          && ((timerWeekday[i] >> weekdayMondayFirst()) & 0x01)) //timer should activate at current day of week
      {
        applyPreset(timerMacro[i]);
      }
    }
    // sunrise macro
    if (sunrise) {
      time_t tmp = sunrise + timerMinutes[8]*60;  // NOTE: may not be ok
      DEBUG_PRINTF("Trigger time: %02d:%02d\n", hour(tmp), minute(tmp));
      if (timerMacro[8] != 0
          && hour(tmp) == hour(localTime)
          && minute(tmp) == minute(localTime)
          && (timerWeekday[8] & 0x01) //timer is enabled
          && ((timerWeekday[8] >> weekdayMondayFirst()) & 0x01)) //timer should activate at current day of week
      {
        applyPreset(timerMacro[8]);
        DEBUG_PRINTF("Sunrise macro %d triggered.",timerMacro[8]);
      }
    }
    // sunset macro
    if (sunset) {
      time_t tmp = sunset + timerMinutes[9]*60;  // NOTE: may not be ok
      DEBUG_PRINTF("Trigger time: %02d:%02d\n", hour(tmp), minute(tmp));
      if (timerMacro[9] != 0
          && hour(tmp) == hour(localTime)
          && minute(tmp) == minute(localTime)
          && (timerWeekday[9] & 0x01) //timer is enabled
          && ((timerWeekday[9] >> weekdayMondayFirst()) & 0x01)) //timer should activate at current day of week
      {
        applyPreset(timerMacro[9]);
        DEBUG_PRINTF("Sunset macro %d triggered.",timerMacro[9]);
      }
    }
  }
}

#define ZENITH -0.83
// get sunrise (or sunset) time (in minutes) for a given day at a given geo location
int getSunriseUTC(int year, int month, int day, float lat, float lon, bool sunset=false) {
  //1. first calculate the day of the year
  float N1 = floor(275 * month / 9);
  float N2 = floor((month + 9) / 12);
  float N3 = (1 + floor((year - 4 * floor(year / 4) + 2) / 3));
  float N = N1 - (N2 * N3) + day - 30;

  //2. convert the longitude to hour value and calculate an approximate time
  float lngHour = lon / 15.0;      
  float t = N + (((sunset ? 18 : 6) - lngHour) / 24);
  
  //3. calculate the Sun's mean anomaly   
  float M = (0.9856 * t) - 3.289;

  //4. calculate the Sun's true longitude
  float L = fmod(M + (1.916 * sin(DEG_TO_RAD*M)) + (0.020 * sin(2*DEG_TO_RAD*M)) + 282.634, 360.0);

  //5a. calculate the Sun's right ascension      
  float RA = fmod(RAD_TO_DEG*atan(0.91764 * tan(DEG_TO_RAD*L)), 360.0);

  //5b. right ascension value needs to be in the same quadrant as L   
  float Lquadrant  = floor( L/90) * 90;
  float RAquadrant = floor(RA/90) * 90;
  RA = RA + (Lquadrant - RAquadrant);

  //5c. right ascension value needs to be converted into hours   
  RA /= 15.;

  //6. calculate the Sun's declination
  float sinDec = 0.39782 * sin(DEG_TO_RAD*L);
  float cosDec = cos(asin(sinDec));

  //7a. calculate the Sun's local hour angle
  float cosH = (sin(DEG_TO_RAD*ZENITH) - (sinDec * sin(DEG_TO_RAD*lat))) / (cosDec * cos(DEG_TO_RAD*lat));
  if (cosH > 1 && !sunset) return 0;  // the sun never rises on this location (on the specified date)
  if (cosH < -1 && sunset) return 0;  // the sun never sets on this location (on the specified date)

  //7b. finish calculating H and convert into hours
  float H = sunset ? RAD_TO_DEG*acos(cosH) : 360 - RAD_TO_DEG*acos(cosH);
  H /= 15.;

  //8. calculate local mean time of rising/setting      
  float T = H + RA - (0.06571 * t) - 6.622;

  //9. adjust back to UTC
  float UT = fmod(T - lngHour, 24.0);

  // return in minutes from midnight
	return UT*60;
}

// calculate sunrise and sunset (if longitude and latitude are set)
void calculateSunriseAndSunset() {
  if ((int)(longitude*10.) || (int)(latitude*10.)) {
    struct tm tim_0;
    tim_0.tm_year = year(localTime)-1900;
    tim_0.tm_mon = month(localTime)-1;
    tim_0.tm_mday = day(localTime);
    tim_0.tm_sec = 0;
    tim_0.tm_isdst = 0;

    int minUTC = getSunriseUTC(year(localTime), month(localTime), day(localTime), latitude, longitude);
    if (minUTC) {
      // there is a sunrise
      tim_0.tm_hour = minUTC / 60;
      tim_0.tm_min = minUTC % 60;
      sunrise = tz->toLocal(mktime(&tim_0) - utcOffsetSecs);
      DEBUG_PRINTF("Sunrise: %02d:%02d\n", hour(sunrise), minute(sunrise));
    } else {
      sunrise = 0;
    }

    minUTC = getSunriseUTC(year(localTime), month(localTime), day(localTime), latitude, longitude, true);
    if (minUTC) {
      // there is a sunset
      tim_0.tm_hour = minUTC / 60;
      tim_0.tm_min = minUTC % 60;
      sunset = tz->toLocal(mktime(&tim_0) - utcOffsetSecs);
      DEBUG_PRINTF("Sunset: %02d:%02d\n", hour(sunset), minute(sunset));
    } else {
      sunset = 0;
    }
  }
}
