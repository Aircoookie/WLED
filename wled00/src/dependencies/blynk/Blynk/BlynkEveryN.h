
#ifndef BLYNKEVERYN_H
#define BLYNKEVERYN_H

#include "BlynkDebug.h"

millis_time_t blynk_count_millis() {
    const millis_time_t ms = BlynkMillis();
    return ms;
}

uint16_t blynk_count_seconds16() {
    const millis_time_t ms = BlynkMillis();
    return (ms / 1000);
}

uint16_t blynk_count_minutes16()
{
    const millis_time_t ms = BlynkMillis();
    return (ms / (60000L)) & 0xFFFF;
}

uint8_t blynk_count_hours8()
{
    const millis_time_t ms = BlynkMillis();
    return (ms / (3600000L)) & 0xFF;
}

template<typename T, T (*timeGetter)()>
class BlynkPeriodic {
public:
    T mPrev;
    T mPeriod;

    BlynkPeriodic()           { reset(); mPeriod = 1; };
    BlynkPeriodic(T period)   { reset(); setPeriod(period); };
    void setPeriod( T period) { mPeriod = period; };
    T getTime()               { return (T)(timeGetter()); };
    T getPeriod()             { return mPeriod; };
    T getElapsed()            { return getTime() - mPrev; }
    T getRemaining()          { return mPeriod - getElapsed(); }
    T getLastTriggerTime()    { return mPrev; }
    bool ready() {
        bool isReady = (getElapsed() >= mPeriod);
        if( isReady ) { reset(); }
        return isReady;
    }
    void reset()              { mPrev = getTime(); };
    void trigger()            { mPrev = getTime() - mPeriod; };

    operator bool()           { return ready(); }
};

typedef BlynkPeriodic<millis_time_t,blynk_count_millis> BlynkEveryNMillis;
typedef BlynkPeriodic<uint16_t,blynk_count_seconds16>   BlynkEveryNSeconds;
typedef BlynkPeriodic<uint16_t,blynk_count_minutes16>   BlynkEveryNMinutes;
typedef BlynkPeriodic<uint8_t,blynk_count_hours8>       BlynkEveryNHours;

#define BLYNK_EVERY_N_MILLIS_I(NAME,N)  static BlynkEveryNMillis NAME(N); if(NAME)
#define BLYNK_EVERY_N_SECONDS_I(NAME,N) static BlynkEveryNSeconds NAME(N); if(NAME)
#define BLYNK_EVERY_N_MINUTES_I(NAME,N) static BlynkEveryNMinutes NAME(N); if(NAME)
#define BLYNK_EVERY_N_HOURS_I(NAME,N)   static BlynkEveryNHours NAME(N); if(NAME)

#define BLYNK_EVERY_N_MILLIS(N)  BLYNK_EVERY_N_MILLIS_I(BLYNK_CONCAT2(PER, __COUNTER__),N)
#define BLYNK_EVERY_N_SECONDS(N) BLYNK_EVERY_N_SECONDS_I(BLYNK_CONCAT2(PER, __COUNTER__),N)
#define BLYNK_EVERY_N_MINUTES(N) BLYNK_EVERY_N_MINUTES_I(BLYNK_CONCAT2(PER, __COUNTER__),N)
#define BLYNK_EVERY_N_HOURS(N)   BLYNK_EVERY_N_HOURS_I(BLYNK_CONCAT2(PER, __COUNTER__),N)

#endif
