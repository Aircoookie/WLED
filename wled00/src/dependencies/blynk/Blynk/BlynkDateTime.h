/**
 * @file       BlynkDateTime.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Aug 2016
 * @brief      DateTime implementation
 *
 */

#ifndef BlynkDateTime_h
#define BlynkDateTime_h

typedef long blynk_time_t;

struct blynk_tm *blynk_gmtime_r(const blynk_time_t *time, struct blynk_tm *tm);
blynk_time_t blynk_mk_gmtime(struct blynk_tm *tm);

struct blynk_tm {
    int8_t          tm_sec;
    int8_t          tm_min;
    int8_t          tm_hour;
    int8_t          tm_mday;
    int8_t          tm_wday;
    int8_t          tm_mon;
    int16_t         tm_year;
    int16_t         tm_yday;
    int16_t         tm_isdst;
};

class BlynkTime {

public:
    static const uint32_t MAX_TIME = 86400L;

    BlynkTime() : mTime(-1) {}

    BlynkTime(const BlynkTime& t) : mTime(t.mTime) {}

    BlynkTime(long seconds) : mTime(seconds % MAX_TIME) {}

    BlynkTime(int hour, int minute, int second)
    {
        mTime = (hour * 3600 + minute * 60 + second) % MAX_TIME;
    }

    int second() const { return mTime % 60; }
    int minute() const { return (mTime / 60) % 60; }
    int hour()   const { return mTime / 3600; }

    int hour12() const {
        int h = hour();
        if (h == 0)
            return 12; // 12 midnight
        else if (h > 12)
            return h - 12;
        return h;
    }

    bool isAM() const { return !isPM(); }
    bool isPM() const { return (hour() >= 12); }

    void adjustSeconds(int sec) {
        if (isValid()) {
            mTime = (mTime + sec) % MAX_TIME;
        }
    }

    blynk_time_t getUnixOffset() const { return mTime; }

    bool isValid()  const { return mTime < MAX_TIME; }
    operator bool() const { return isValid(); }

    bool operator == (const BlynkTime& t) const { return mTime == t.mTime; }
    bool operator >= (const BlynkTime& t) const { return mTime >= t.mTime; }
    bool operator <= (const BlynkTime& t) const { return mTime <= t.mTime; }
    bool operator >  (const BlynkTime& t) const { return mTime >  t.mTime; }
    bool operator <  (const BlynkTime& t) const { return mTime <  t.mTime; }

private:
    uint32_t mTime;
};

class BlynkDateTime {

public:
    BlynkDateTime() : mTime(0) {}

    BlynkDateTime(const BlynkDateTime& t)
    {
        mTime = t.mTime;
        blynk_gmtime_r(&mTime, &mTm);
    }

    BlynkDateTime(blynk_time_t t)
    {
        mTime = t;
        blynk_gmtime_r(&mTime, &mTm);
    }

    BlynkDateTime(int hour, int minute, int second, int day, int month, int year)
    {
        mTm.tm_hour = hour;
        mTm.tm_min  = minute;
        mTm.tm_sec  = second;

        mTm.tm_mday = day;
        mTm.tm_mon  = month - 1;
        mTm.tm_year = year - 1900;

        mTm.tm_isdst = 0;

        mTime = blynk_mk_gmtime(&mTm);
    }

    int second() const { return mTm.tm_sec; }
    int minute() const { return mTm.tm_min; }
    int hour()   const { return mTm.tm_hour; }
    int day()    const { return mTm.tm_mday; }
    int month()  const { return 1 + mTm.tm_mon; }
    int year()   const { return 1900 + mTm.tm_year; }

    int day_of_year() const { return 1 + mTm.tm_yday; }
    int day_of_week() const { return mTm.tm_wday == 0 ? 7 : mTm.tm_wday; }

    /*int weak_of_year() const {
        int julian = day_of_year();
        int dow = day_of_week();
        int dowJan1 = BlynkDateTime(0,0,0, 1,1,year()).day_of_week();
        int weekNum = ((julian + 6) / 7);
        if (dow < dowJan1)
            ++weekNum;
        return (weekNum);
    }*/

    int hour12() const {
        int h = hour();
        if (h == 0)
            return 12; // 12 midnight
        else if (h > 12)
            return h - 12;
        return h;
    }

    bool isAM() const { return !isPM(); }
    bool isPM() const { return (hour() >= 12); }

    void adjustSeconds(int sec) {
        if (isValid()) {
            mTime += sec;
            blynk_gmtime_r(&mTime, &mTm);
        }
    }

    //tm& getTm() { return mTm; }
    blynk_time_t getUnix() const { return mTime; }

    bool isValid()  const { return mTime != 0; }
    operator bool() const { return isValid(); }

    bool operator == (const BlynkDateTime& t) const { return mTime == t.mTime; }
    bool operator >= (const BlynkDateTime& t) const { return mTime >= t.mTime; }
    bool operator <= (const BlynkDateTime& t) const { return mTime <= t.mTime; }
    bool operator >  (const BlynkDateTime& t) const { return mTime >  t.mTime; }
    bool operator <  (const BlynkDateTime& t) const { return mTime <  t.mTime; }

private:
    blynk_tm mTm;
    blynk_time_t mTime;
};


#endif
