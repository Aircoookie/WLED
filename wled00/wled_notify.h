#ifndef WLED_NOTIFY_H
#define WLED_NOTIFY_H
#include <Arduino.h>
#include "const.h"
/*
 * UDP notifier
 */
//union e131_packet_t;    // Will this compile?
class IPAddress;

void notify(byte callMode, bool followUp=false);
void arlsLock(uint32_t timeoutMs, byte md = REALTIME_MODE_GENERIC);
void handleE131Packet(e131_packet_t* p, IPAddress clientIP);
void handleNotifications();
void setRealtimePixel(uint16_t i, byte r, byte g, byte b, byte w);

#endif // WLED_NOTIFY_H