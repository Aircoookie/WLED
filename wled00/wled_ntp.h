#ifndef WLED_NTP_H
#define WLED_NTP_H
#include <Arduino.h>
#include "timezone/Timezone.h"
/*
 * Acquires time from NTP server
 */

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