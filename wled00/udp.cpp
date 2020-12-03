#include "wled.h"

/*
 * UDP sync notifier / Realtime / Hyperion / TPM2.NET
 */

#define WLEDPACKETSIZE 29
#define UDP_IN_MAXSIZE 1472

void notify(byte callMode, bool followUp)
{
  if (!udpConnected) return;
  switch (callMode)
  {
    case NOTIFIER_CALL_MODE_INIT:          return;
    case NOTIFIER_CALL_MODE_DIRECT_CHANGE: if (!notifyDirect) return; break;
    case NOTIFIER_CALL_MODE_BUTTON:        if (!notifyButton) return; break;
    case NOTIFIER_CALL_MODE_NIGHTLIGHT:    if (!notifyDirect) return; break;
    case NOTIFIER_CALL_MODE_HUE:           if (!notifyHue)    return; break;
    case NOTIFIER_CALL_MODE_PRESET_CYCLE:  if (!notifyDirect) return; break;
    case NOTIFIER_CALL_MODE_BLYNK:         if (!notifyDirect) return; break;
    case NOTIFIER_CALL_MODE_ALEXA:         if (!notifyAlexa)  return; break;
    default: return;
  }
  byte udpOut[WLEDPACKETSIZE];
  udpOut[0] = 0; //0: wled notifier protocol 1: WARLS protocol
  udpOut[1] = callMode;
  udpOut[2] = bri;
  udpOut[3] = col[0];
  udpOut[4] = col[1];
  udpOut[5] = col[2];
  udpOut[6] = nightlightActive;
  udpOut[7] = nightlightDelayMins;
  udpOut[8] = effectCurrent;
  udpOut[9] = effectSpeed;
  udpOut[10] = col[3];
  //compatibilityVersionByte: 
  //0: old 1: supports white 2: supports secondary color
  //3: supports FX intensity, 24 byte packet 4: supports transitionDelay 5: sup palette
  //6: supports timebase syncing, 29 byte packet 7: supports tertiary color 
  udpOut[11] = 7; 
  udpOut[12] = colSec[0];
  udpOut[13] = colSec[1];
  udpOut[14] = colSec[2];
  udpOut[15] = colSec[3];
  udpOut[16] = effectIntensity;
  udpOut[17] = (transitionDelay >> 0) & 0xFF;
  udpOut[18] = (transitionDelay >> 8) & 0xFF;
  udpOut[19] = effectPalette;
  uint32_t colTer = strip.getSegment(strip.getMainSegmentId()).colors[2];
  udpOut[20] = (colTer >> 16) & 0xFF;
  udpOut[21] = (colTer >>  8) & 0xFF;
  udpOut[22] = (colTer >>  0) & 0xFF;
  udpOut[23] = (colTer >> 24) & 0xFF;
  
  udpOut[24] = followUp;
  uint32_t t = millis() + strip.timebase;
  udpOut[25] = (t >> 24) & 0xFF;
  udpOut[26] = (t >> 16) & 0xFF;
  udpOut[27] = (t >>  8) & 0xFF;
  udpOut[28] = (t >>  0) & 0xFF;
  
  IPAddress broadcastIp;
  broadcastIp = ~uint32_t(Network.subnetMask()) | uint32_t(Network.gatewayIP());

  notifierUdp.beginPacket(broadcastIp, udpPort);
  notifierUdp.write(udpOut, WLEDPACKETSIZE);
  notifierUdp.endPacket();
  notificationSentCallMode = callMode;
  notificationSentTime = millis();
  notificationTwoRequired = (followUp)? false:notifyTwice;
}


void realtimeLock(uint32_t timeoutMs, byte md)
{
  if (!realtimeMode && !realtimeOverride){
    for (uint16_t i = 0; i < ledCount; i++)
    {
      strip.setPixelColor(i,0,0,0,0);
    }
  }

  realtimeTimeout = millis() + timeoutMs;
  if (timeoutMs == 255001 || timeoutMs == 65000) realtimeTimeout = UINT32_MAX;
  realtimeMode = md;

  if (arlsForceMaxBri && !realtimeOverride) strip.setBrightness(scaledBri(255));
  if (md == REALTIME_MODE_GENERIC) strip.show();
}


#define TMP2NET_OUT_PORT 65442

void sendTPM2Ack() {
  notifierUdp.beginPacket(notifierUdp.remoteIP(), TMP2NET_OUT_PORT);
  uint8_t response_ack = 0xac;
  notifierUdp.write(&response_ack, 1);
  notifierUdp.endPacket();
}


void handleNotifications()
{
  //send second notification if enabled
  if(udpConnected && notificationTwoRequired && millis()-notificationSentTime > 250){
    notify(notificationSentCallMode,true);
  }
  
  if (e131NewData && millis() - strip.getLastShow() > 15)
  {
    e131NewData = false;
    strip.show();
  }

  //unlock strip when realtime UDP times out
  if (realtimeMode && millis() > realtimeTimeout)
  {
    if (realtimeOverride == REALTIME_OVERRIDE_ONCE) realtimeOverride = REALTIME_OVERRIDE_NONE;
    strip.setBrightness(scaledBri(bri));
    realtimeMode = REALTIME_MODE_INACTIVE;
    realtimeIP[0] = 0;
  }

  //receive UDP notifications
  if (!udpConnected) return;
    
  bool isSupp = false;
  uint16_t packetSize = notifierUdp.parsePacket();
  if (!packetSize && udp2Connected) {
    packetSize = notifier2Udp.parsePacket();
    isSupp = true;
  }

  //hyperion / raw RGB
  if (!packetSize && udpRgbConnected) {
    packetSize = rgbUdp.parsePacket();
    if (packetSize) {
      if (!receiveDirect) return;
      if (packetSize > UDP_IN_MAXSIZE || packetSize < 3) return;
      realtimeIP = rgbUdp.remoteIP();
      DEBUG_PRINTLN(rgbUdp.remoteIP());
      uint8_t lbuf[packetSize];
      rgbUdp.read(lbuf, packetSize);
      realtimeLock(realtimeTimeoutMs, REALTIME_MODE_HYPERION);
      if (realtimeOverride) return;
      uint16_t id = 0;
      for (uint16_t i = 0; i < packetSize -2; i += 3)
      {
        setRealtimePixel(id, lbuf[i], lbuf[i+1], lbuf[i+2], 0);
        
        id++; if (id >= ledCount) break;
      }
      strip.show();
      return;
    } 
  }

  if (!(receiveNotifications || receiveDirect)) return;
  
  //notifier and UDP realtime
  if (!packetSize || packetSize > UDP_IN_MAXSIZE) return;
  if (!isSupp && notifierUdp.remoteIP() == Network.localIP()) return; //don't process broadcasts we send ourselves

  uint8_t udpIn[packetSize +1];
  if (isSupp) notifier2Udp.read(udpIn, packetSize);
  else         notifierUdp.read(udpIn, packetSize);

  //wled notifier, ignore if realtime packets active
  if (udpIn[0] == 0 && !realtimeMode && receiveNotifications)
  {
    //ignore notification if received within a second after sending a notification ourselves
    if (millis() - notificationSentTime < 1000) return;
    if (udpIn[1] > 199) return; //do not receive custom versions
    
    bool someSel = (receiveNotificationBrightness || receiveNotificationColor || receiveNotificationEffects);
    //apply colors from notification
    if (receiveNotificationColor || !someSel)
    {
      col[0] = udpIn[3];
      col[1] = udpIn[4];
      col[2] = udpIn[5];
      if (udpIn[11] > 0) //sending module's white val is intended
      {
        col[3] = udpIn[10];
        if (udpIn[11] > 1)
        {
          colSec[0] = udpIn[12];
          colSec[1] = udpIn[13];
          colSec[2] = udpIn[14];
          colSec[3] = udpIn[15];
        }
        if (udpIn[11] > 5)
        {
          uint32_t t = (udpIn[25] << 24) | (udpIn[26] << 16) | (udpIn[27] << 8) | (udpIn[28]);
          t += 2;
          t -= millis();
          strip.timebase = t;
        }
        if (udpIn[11] > 6)
        {
          strip.setColor(2, udpIn[20], udpIn[21], udpIn[22], udpIn[23]); //tertiary color
        }
      }
    }

    //apply effects from notification
    if (udpIn[11] < 200 && (receiveNotificationEffects || !someSel))
    {
      if (udpIn[8] < strip.getModeCount()) effectCurrent = udpIn[8];
      effectSpeed   = udpIn[9];
      if (udpIn[11] > 2) effectIntensity = udpIn[16];
      if (udpIn[11] > 4 && udpIn[19] < strip.getPaletteCount()) effectPalette = udpIn[19];
    }
    
    if (udpIn[11] > 3)
    {
      transitionDelayTemp = ((udpIn[17] << 0) & 0xFF) + ((udpIn[18] << 8) & 0xFF00);
    }

    nightlightActive = udpIn[6];
    if (nightlightActive) nightlightDelayMins = udpIn[7];
    
    if (receiveNotificationBrightness || !someSel) bri = udpIn[2];
    colorUpdated(NOTIFIER_CALL_MODE_NOTIFICATION);
    return;
  }

  if (!receiveDirect) return;
  
  //TPM2.NET
  if (udpIn[0] == 0x9c)
  {
    //WARNING: this code assumes that the final TMP2.NET payload is evenly distributed if using multiple packets (ie. frame size is constant)
    //if the number of LEDs in your installation doesn't allow that, please include padding bytes at the end of the last packet
    byte tpmType = udpIn[1];
    if (tpmType == 0xaa) { //TPM2.NET polling, expect answer
      sendTPM2Ack(); return;
    }
    if (tpmType != 0xda) return; //return if notTPM2.NET data

    realtimeIP = (isSupp) ? notifier2Udp.remoteIP() : notifierUdp.remoteIP();
    realtimeLock(realtimeTimeoutMs, REALTIME_MODE_TPM2NET);
    if (realtimeOverride) return;

    tpmPacketCount++; //increment the packet count
    if (tpmPacketCount == 1) tpmPayloadFrameSize = (udpIn[2] << 8) + udpIn[3]; //save frame size for the whole payload if this is the first packet
    byte packetNum = udpIn[4]; //starts with 1!
    byte numPackets = udpIn[5];

    uint16_t id = (tpmPayloadFrameSize/3)*(packetNum-1); //start LED
    for (uint16_t i = 6; i < tpmPayloadFrameSize + 4; i += 3)
    {
      if (id < ledCount)
      {
        setRealtimePixel(id, udpIn[i], udpIn[i+1], udpIn[i+2], 0);
        id++;
      }
      else break;
    }
    if (tpmPacketCount == numPackets) //reset packet count and show if all packets were received
    {
      tpmPacketCount = 0;
      strip.show();
    }
    return;
  }

  //UDP realtime: 1 warls 2 drgb 3 drgbw
  if (udpIn[0] > 0 && udpIn[0] < 5)
  {
    realtimeIP = (isSupp) ? notifier2Udp.remoteIP() : notifierUdp.remoteIP();
    DEBUG_PRINTLN(realtimeIP);
    if (packetSize < 2) return;

    if (udpIn[1] == 0)
    {
      realtimeTimeout = 0;
      return;
    } else {
      realtimeLock(udpIn[1]*1000 +1, REALTIME_MODE_UDP);
    }
    if (realtimeOverride) return;

    if (udpIn[0] == 1) //warls
    {
      for (uint16_t i = 2; i < packetSize -3; i += 4)
      {
        setRealtimePixel(udpIn[i], udpIn[i+1], udpIn[i+2], udpIn[i+3], 0);
      }
    } else if (udpIn[0] == 2) //drgb
    {
      uint16_t id = 0;
      for (uint16_t i = 2; i < packetSize -2; i += 3)
      {
        setRealtimePixel(id, udpIn[i], udpIn[i+1], udpIn[i+2], 0);

        id++; if (id >= ledCount) break;
      }
    } else if (udpIn[0] == 3) //drgbw
    {
      uint16_t id = 0;
      for (uint16_t i = 2; i < packetSize -3; i += 4)
      {
        setRealtimePixel(id, udpIn[i], udpIn[i+1], udpIn[i+2], udpIn[i+3]);
        
        id++; if (id >= ledCount) break;
      }
    } else if (udpIn[0] == 4) //dnrgb
    {
      uint16_t id = ((udpIn[3] << 0) & 0xFF) + ((udpIn[2] << 8) & 0xFF00);
      for (uint16_t i = 4; i < packetSize -2; i += 3)
      {
          if (id >= ledCount) break;
        setRealtimePixel(id, udpIn[i], udpIn[i+1], udpIn[i+2], 0);
        id++;
      }
    }
    strip.show();
    return;
  }

  // API over UDP
  udpIn[packetSize] = '\0';

  if (udpIn[0] >= 'A' && udpIn[0] <= 'Z') { //HTTP API
    String apireq = "win&";
    apireq += (char*)udpIn;
    handleSet(nullptr, apireq);
  } else if (udpIn[0] == '{') { //JSON API
    DynamicJsonDocument jsonBuffer(2048);
    DeserializationError error = deserializeJson(jsonBuffer, udpIn);
    JsonObject root = jsonBuffer.as<JsonObject>();
    if (!error && !root.isNull()) deserializeState(root);
  }
}


void setRealtimePixel(uint16_t i, byte r, byte g, byte b, byte w)
{
  uint16_t pix = i + arlsOffset;
  if (pix < ledCount)
  {
    if (!arlsDisableGammaCorrection && strip.gammaCorrectCol)
    {
      strip.setPixelColor(pix, strip.gamma8(r), strip.gamma8(g), strip.gamma8(b), strip.gamma8(w));
    } else {
      strip.setPixelColor(pix, r, g, b, w);
    }
  }
}
