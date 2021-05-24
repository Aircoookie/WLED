/*
  Toki.h - Minimal millisecond accurate timekeeping.

  LICENSE
  The MIT License (MIT)
  Copyright (c) 2021 Christian Schwinne
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include <Arduino.h> 

#define YEARS_70 2208988800UL

//3:13 24.05.2012 ==>  2021-4-29, 06:41:37.

class Toki {
  typedef enum {
    inactive, marked, active
  } TickT;

  public: 
  typedef struct {
    uint32_t sec;
    uint16_t ms;
  } Time;

  private:
    uint32_t fullSecondMillis = 0;
    uint32_t unix = 0;
    TickT tick = TickT::inactive;

  public:
    void setTime(Time t) {
      fullSecondMillis = millis() - t.ms;
      unix = t.sec;
    }

    void setTime(uint32_t sec, uint16_t ms) {
      Time t = {sec, ms};
      setTime(t);
    }

    Time fromNTP(byte *timestamp) { //ntp timestamp is 8 bytes, 4 bytes second and 4 bytes sub-second fraction
      unsigned long highWord = word(timestamp[0], timestamp[1]);
      unsigned long lowWord = word(timestamp[2], timestamp[3]);
    
      unsigned long unix = highWord << 16 | lowWord;
      if (!unix) return {0,0};
      unix -= YEARS_70; //NTP begins 1900, Unix 1970

      unsigned long frac = word(timestamp[5], timestamp[6]); //65536ths of a second
      frac = (frac*1000) >> 16; //convert to ms
      return {unix, (uint16_t)frac};
    }

    uint16_t millisecond() {
      uint32_t ms = millis() - fullSecondMillis;
      while (ms > 999) {
        ms -= 1000;
        fullSecondMillis += 1000;
        unix++;
        if (tick == TickT::inactive) tick = TickT::marked; //marked, will be active on next loop
      }
      return ms;
    }

    uint32_t second() {
      millisecond();
      return unix;
    }

    uint32_t msDifference(Time &t0, Time &t1) {
      bool t1BiggerSec = (t1.sec > t0.sec);
      uint32_t secDiff = (t1BiggerSec) ? t1.sec - t0.sec : t0.sec - t1.sec;
      uint32_t t0ms = t0.ms, t1ms = t1.ms;
      if (t1BiggerSec) t1ms += secDiff;
      else t0ms += secDiff;
      uint32_t msDiff = (t1ms > t0ms) ? t1ms - t0ms : t0ms - t1ms;
      return msDiff;
    }

    void adjust(Time&t, int32_t offset) {
      int32_t secs = offset /1000;
      int32_t ms = offset - secs*1000;
      t.sec += offset /1000;
      int32_t nms = t.ms + ms;
      if (nms > 1000) {nms -= 1000; t.sec++;}
      if (nms < 0) {nms += 1000; t.sec--;}
      t.ms += nms;
    }

    Time getTime() {
      Time t;
      t.ms = millisecond();
      t.sec = unix;
      return t;
    }

    void setTick() {
      if (tick == TickT::marked) tick = TickT::active;
    }

    void resetTick() {
      tick = TickT::inactive;
    }

    bool isTick() {
      return (tick == TickT::active);
    }

    void printTime(const Time& t) {
      Serial.printf_P(PSTR("%u,%03u\n"),t.sec,t.ms);
    }
};