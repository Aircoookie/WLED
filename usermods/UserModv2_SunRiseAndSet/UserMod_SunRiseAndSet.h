#pragma once

#include "wled.h"
#include <Dusk2Dawn.h>

/*
 * 
 * REQUIREMENTS:
 *      The Dusk2Dawn library must be installed.  This can be found at https://github.com/dmkishi/Dusk2Dawn.  The 1.0.1 version of this library found via
 *      Arduino or platformio library managers is buggy and won't compile.  The latest version from github should be used.
 * 
 *      NTP must be enabled and functional.  It simply makes no sense to have events on sunrise/sunset when an accurate time isn't available.
 * 
 *      The user's geographical latitude and longitude must be configured (in decimal, not degrees/minutes/etc) using m_fLatitude and m_fLongitude
 * 
 *      if desired, an offset of up to +/- 2 hours can be specified for each of sunrise/sunset using m_sunriseOffset and m_sunsetOffset (defaults to 0)
 * 
 *      The specific macro to run at sunrise and/or sunset can be changed using m_sunriseMacro and m_sunsetMacro. (defaults to 15 and 16)
 * 
 *      From the Dusk2Dawn library:
 *          HINT: An easy way to find the longitude and latitude for any location is
 *          to find the spot in Google Maps, right click the place on the map, and
 *          select "What's here?". At the bottom, you’ll see a card with the
 *          coordinates.
 * 
 *      Once configured, copy UserMod_SunRiseAndSet.h to the sketch file (the same folder as wled00.ino exists), 
 *      and then edit "usermods_list.cpp":
 *          Add '#include "UserMod_SunRiseAndSet.h"' in the 'includes' area
 *          Add 'usermods.add(new UserMod_SunRiseAndSet());' in the registerUsermods() area
 * 
 */

class UserMod_SunRiseAndSet : public Usermod 
{
private:    

    /**** USER SETTINGS ****/

    float m_fLatitude = 40.6;       // latitude where sunrise/set are calculated
    float m_fLongitude = -79.80;    // longitude where sunrise/set are calculated
    int8_t m_sunriseOffset = 0;     // offset from sunrise, in minutes, when macro should be run (negative for before sunrise, positive for after sunrise)
    int8_t m_sunsetOffset = 0;      // offset from sunset, in minutes, when macro should be run (negative for before sunset, positive for after sunset)
    uint8_t m_sunriseMacro = 15;    // macro number to run at sunrise
    uint8_t m_sunsetMacro = 16;     // macro number to run at sunset

    /**** END OF USER SETTINGS.  DO NOT EDIT BELOW THIS LINE! ****/
    

    Dusk2Dawn *m_pD2D = NULL;         // this must be dynamically allocated in order for parameters to be loaded from EEPROM
    
    int m_nUserSunrise = -1;        // time, in minutes from midnight, of sunrise    
    int m_nUserSunset = -1;         // time, in minutes from midnight, of sunset

    byte m_nLastRunMinute = -1;     // indicates what minute the userloop was last run - used so that the code only runs once per minute

public:

    virtual void setup(void)
    {
        /* TODO:  From EEPROM, load the following variables:
        *
        *   int16_t latitude16 = 4060;      // user provided latitude, multiplied by 100 and rounded
        *   int16_t longitude16 = -7980;    // user provided longitude, multiplied by 100 and rounded.
        *   int8_t  sunrise_offset = 0;     // number of minutes to offset the sunrise macro trigger (positive for minutes after sunrise, negative for minutes before)
        *   int8_t  sunset_offset = 0;     // number of minutes to offset the sunset macro trigger (positive for minutes after sunset, negative for minutes before)
        * 
        *   then:
        *       m_fLatitude = (float)latitude / 100.0;
        *       m_fLongitude = (float)longitude / 100.0;
        *       m_sunriseOffset = sunrise_offset;
        *       m_sunsetOffset = sunset_offset;
        */

        if ((0.0 != m_fLatitude) || (0.0 != m_fLongitude))
        {
            m_pD2D = new Dusk2Dawn (m_fLatitude, m_fLongitude, 0 /* UTC */);
            // can't really check for failures.  if the alloc fails, the mod just doesn't work.
        }        
    }        

    void loop(void) 
    {
        // without NTP, or a configured lat/long, none of this stuff is going to work...  
        // As an alternative, need to figure out how to determine if the user has manually set the clock or not.
        if (m_pD2D && (999000000L != ntpLastSyncTime))
        {
            // to prevent needing to import all the timezone stuff from other modules, work completely in UTC
            time_t timeUTC = toki.second();
            tmElements_t tmNow;
            breakTime(timeUTC, tmNow);
            int nCurMinute = tmNow.Minute;

            if (m_nLastRunMinute != nCurMinute) //only check once a new minute begins
            {
                m_nLastRunMinute = nCurMinute;
                int numMinutes = (60 * tmNow.Hour) + m_nLastRunMinute; // how many minutes into the day are we?

                // check to see if sunrise/sunset should be re-determined.  Only do this if neither sunrise nor sunset 
                // are set.  That happens when the device has just stated, and after both sunrise/sunset have already run.
                if ((-1 == m_nUserSunrise) && (-1 == m_nUserSunset))
                {                
                    m_nUserSunrise = m_pD2D->sunrise(tmNow.Year + 1970, tmNow.Month, tmNow.Day, false) % 1440;
                    m_nUserSunset = m_pD2D->sunset(tmNow.Year + 1970, tmNow.Month, tmNow.Day, false) % 1440;
                    if (m_nUserSunrise > numMinutes) // has sunrise already passed?  if so, recompute for tomorrow
                    {
                        breakTime(timeUTC + (60*60*24), tmNow);
                        m_nUserSunrise = m_pD2D->sunrise(tmNow.Year + 1970, tmNow.Month, tmNow.Day, false) % 1440;
                        if (m_nUserSunset > numMinutes) // if sunset has also passed, recompute that as well
                        {
                            m_nUserSunset = m_pD2D->sunset(tmNow.Year + 1970, tmNow.Month, tmNow.Day, false) % 1440;
                        }                    
                    }
                    // offset by user provided values.  becuase the offsets are signed bytes, the max offset is just over 2 hours.
                    m_nUserSunrise += m_sunriseOffset;
                    m_nUserSunset += m_sunsetOffset;
                }

                if (numMinutes == m_nUserSunrise)     // Good Morning!
                {
                    if (m_sunriseMacro)
                        applyMacro(m_sunriseMacro); // run macro 15
                    m_nUserSunrise = -1;
                }
                else if (numMinutes == m_nUserSunset)  // Good Night!
                {
                    if (m_sunsetMacro)                    
                        applyMacro(m_sunsetMacro); // run macro 16
                    m_nUserSunset = -1;
                }
            } // if (m_nLastRunMinute != nCurMinute)
        }  // if (m_pD2D && (999000000L != ntpLastSyncTime))
    }

   void addToJsonState(JsonObject& root)
    {
        JsonObject user = root["SunRiseAndSet"];
        if (user.isNull()) user = root.createNestedObject("SunRiseAndSet");

        char buf[10];
        if (-1 != m_nUserSunrise)
        {
            snprintf(buf, 10, "%02d:%02d UTC", m_nUserSunrise / 60, m_nUserSunrise % 60);
            user["rise"] = buf;
        }
        if (-1 != m_nUserSunset)
        {
            snprintf(buf, 10, "%02d:%02d UTC", m_nUserSunset / 60, m_nUserSunset % 60);
            user["set"] = buf;
        }
        JsonObject vars = user.createNestedObject("vars");
        vars["lat"] = m_fLatitude;
        vars["long"] = m_fLongitude;
        vars["rise_mac"] = m_sunriseMacro;
        vars["set_mac"] = m_sunsetMacro;
        vars["rise_off"] = m_sunriseOffset;
        vars["set_off"] = m_sunsetOffset;
    }

    ~UserMod_SunRiseAndSet(void)
    {
        if (m_pD2D) delete m_pD2D;
    }
};



