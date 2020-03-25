#ifndef WLED_NTP_H
#define WLED_NTP_H
#include <Arduino.h>
#include "timezone/Timezone.h"

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

Timezone* timezones[] = {&tzUTC, &tzUK, &tzEUCentral, &tzEUEastern, &tzUSEastern, &tzUSCentral, &tzUSMountain, &tzUSArizona, &tzUSPacific, &tzChina, &tzJapan, &tzAUEastern, &tzNZ, &tzNK, &tzIndia, &tzCASaskatchewan};  

void handleNetworkTime();
void sendNTPPacket();
bool checkNTPResponse();    
void updateLocalTime();
void getTimeString(char* out);
bool checkCountdown();
void setCountdown();
byte weekdayMondayFirst();
void checkTimers();

#endif // WLED_NTP_H