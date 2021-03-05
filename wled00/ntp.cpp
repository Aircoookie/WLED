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
  sprintf(out,"%i-%i-%i, %i:%s%i:%s%i",year(localTime), month(localTime), day(localTime), 
                                       hr,(minute(localTime)<10)?"0":"",minute(localTime),
                                       (second(localTime)<10)?"0":"",second(localTime));
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
    daytime = isDayTime();
    if (prevDaytime != daytime) {
      // sunrise or sunset
      DEBUG_PRINTLN(daytime?F("Sunrise"):F("Sunset"));
    }
    lastTimerMinute = minute(localTime);
    for (uint8_t i = 0; i < 8; i++)
    {
      if (timerMacro[i] != 0
          && (timerHours[i] == hour(localTime) || timerHours[i] == 24) //if hour is set to 24, activate every hour 
          && timerMinutes[i] == minute(localTime)
          && (timerWeekday[i] & 0x01) //timer is enabled
          && timerWeekday[i] >> weekdayMondayFirst() & 0x01) //timer should activate at current day of week
      {
        applyPreset(timerMacro[i]);
      }
    }
  }
}

/*
 * This program calculates solar positions as a function of location, date, and time.
 * The equations are from Jean Meeus, Astronomical Algorithms, Willmann-Bell, Inc., Richmond, VA
 * (C) 2015, David Brooks, Institute for Earth Science Research and Education.
 * http://www.instesre.org/ArduinoUnoSolarCalculations.pdf
 */
//#define DEG_TO_RAD 0.01745329
//#define PI 3.141592654
#define TWOPI 6.28318531

long JulianDate(int year, int month, int day) {
	if (month<=2) {
		year--; month+=12;
	}
	int A=year/100;
	int B=2-A+A/4;
	return (long)(365.25*(year + 4716)) + (int)(30.6001*(month + 1)) + day + B - 1524;
}

bool isDayTime() {
	float JD_frac,T,L0,M,C,L_true,GrHrAngle,Obl,RA,Decl,HrAngle,elev;
	long JD_whole,JDx;

	float Lon = longitude*DEG_TO_RAD;
	float Lat = latitude*DEG_TO_RAD;

  // calculate elevation of the sun (>0 daytime, <0 nighttime)
	JD_whole  = JulianDate(year(localTime), month(localTime), day(localTime));
	JD_frac   = (hour(localTime) + minute(localTime)/60. + second(localTime)/3600.)/24. - .5;
	JDx       = JD_whole - 2451545;
	T         = (JDx + JD_frac)/36525.;
	L0        = DEG_TO_RAD*fmod(280.46645 + 36000.76983*T, 360);
	M         = DEG_TO_RAD*fmod(357.5291 + 35999.0503*T, 360);
	C         = DEG_TO_RAD*((1.9146-0.004847*T)*sin(M) + (0.019993-0.000101*T)*sin(2*M) + 0.00029*sin(3*M));
	Obl       = DEG_TO_RAD*(23 + 26/60. + 21.448/3600. - 46.815/3600*T);
	GrHrAngle = 280.46061837 + (360*JDx)%360 + .98564736629*JDx + 360.98564736629*JD_frac;
	GrHrAngle = fmod(GrHrAngle, 360.);
	L_true    = fmod(C + L0, TWOPI);
	RA        = atan2(sin(L_true)*cos(Obl), cos(L_true));
	Decl      = asin(sin(Obl)*sin(L_true));
	HrAngle   = DEG_TO_RAD*GrHrAngle + Lon - RA;

	elev = asin(sin(Lat)*sin(Decl) + cos(Lat)*(cos(Decl)*cos(HrAngle)));
	// Azimuth measured eastward from north.
	// azimuth = PI+atan2(sin(HrAngle),cos(HrAngle)*sin(Lat)-tan(Decl)*cos(Lat));
	
	return elev > 0.; // if elevation is gt 0 then it is a day
}
