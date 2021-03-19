#include "wled.h"

/*
 * UDP sync notifier / Realtime / Hyperion / TPM2.NET
 */

#define WLEDPACKETSIZE 29
#define UDP_IN_MAXSIZE 1472

#ifdef WLED_USE_REALTIME_SMOOTHING
#include "realtime_smooth.h"
void setRealtimeFrame(RealtimeSmooth::rts_frame*);
#endif

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

  // Turn on if it was off
  if (realtimeMode == REALTIME_MODE_INACTIVE && md != realtimeMode) {
    if (arlsForceMaxBri && !realtimeOverride)
      strip.setBrightness(scaledBri(255));
    else if (bri == 0)
      strip.setBrightness(scaledBri(briLast));
  }
  realtimeMode = md;
  
  if (md == REALTIME_MODE_GENERIC) strip.show();
}


#define TMP2NET_OUT_PORT 65442

void sendTPM2Ack() {
  notifierUdp.beginPacket(notifierUdp.remoteIP(), TMP2NET_OUT_PORT);
  uint8_t response_ack = 0xac;
  notifierUdp.write(&response_ack, 1);
  notifierUdp.endPacket();
}

#ifdef WLED_USE_REALTIME_SMOOTHING
inline void setRealtimeFrame(RealtimeSmooth::rts_frame* frame) {
  for(uint16_t i=0; i < RTS_MAX_PACKETS_PER_UPDATE; i++) {
    setRealtime(frame->packets[i], frame->len[i]);
#ifdef DEBUG_RTS
    if(frame->len[i] == 0) rts.missed++;
#endif
  }
}
#endif

void setRealtime(uint8_t *udpIn, uint16_t packetSize) {
  if (packetSize < 2) return;
  uint16_t i, id=0;
  switch (udpIn[0]) {
  case 1: //warls
    for (i = 2; i < packetSize -3; i += 4) 
      setRealtimePixel(udpIn[i], udpIn[i+1], udpIn[i+2], udpIn[i+3], 0);
    break;

  case 2: //drgb
    for (i = 2; i < packetSize -2; i += 3) {
	  setRealtimePixel(id, udpIn[i], udpIn[i+1], udpIn[i+2], 0);
	  id++; if (id >= ledCount) break;
    }
    break;

  case 3: //drgbw
    for (i = 2; i < packetSize -3; i += 4) {
	  setRealtimePixel(id, udpIn[i], udpIn[i+1], udpIn[i+2], udpIn[i+3]);
	  id++; if (id >= ledCount) break;
    }
    break;

  case 4: //dnrgb
    id = ((udpIn[3] << 0) & 0xFF) + ((udpIn[2] << 8) & 0xFF00);
    for (i = 4; i < packetSize -2; i += 3) {
      if (id >= ledCount) break;
      setRealtimePixel(id, udpIn[i], udpIn[i+1], udpIn[i+2], 0);
      id++;
    }
    break;

  case 5: //dnrgbw
    id = ((udpIn[3] << 0) & 0xFF) + ((udpIn[2] << 8) & 0xFF00);
    for (i = 4; i < packetSize -2; i += 4) {
      if (id >= ledCount) break;
      setRealtimePixel(id, udpIn[i], udpIn[i+1], udpIn[i+2], udpIn[i+3]);
      id++;
    }
    break;
    
  case 6: //irgbw (=warlsw), with 2byte index
    for (i = 2; i < packetSize - 5; i += 6) {
      id = ((udpIn[i+1] << 0) & 0xFF) + ((udpIn[i] << 8) & 0xFF00);
      setRealtimePixel(id, udpIn[i+2], udpIn[i+3], udpIn[i+4], udpIn[i+5]);
    }
    break;
  }
}

void handleNotifications()
{
  uint32_t now = millis();

#ifdef WLED_USE_REALTIME_SMOOTHING
#ifdef DEBUG_RTS
  rts.log(now); 
#endif 
  // Play back buffered realtime frames at the calculated interval
  // (or, during training, as soon as target buffer fullness is reached)
  if ((rts.sample_cnt >= RTS_N_TRAIN && now - rts.last_remove > rts.mean_deltaT()) || 
      (rts.sample_cnt <  RTS_N_TRAIN && rts.frame_cnt >= RTS_TARGET)) {
    RealtimeSmooth::rts_frame* send = rts.remove(now);
    if (send != NULL) {
      setRealtimeFrame(send);
      strip.show();
    } // If buffer ran dry, we can just wait...
  }
#endif
  
  //send second notification if enabled
  if(udpConnected && notificationTwoRequired && now-notificationSentTime > 250){
    notify(notificationSentCallMode,true);
  }

  // Show any new ES131 data
  if (e131NewData && now - strip.getLastShow() > 15)
  {
    e131NewData = false;
    strip.show();
  }

  //unlock strip when realtime UDP times out
  if (realtimeMode && now > realtimeTimeout)
  {
    if (realtimeOverride == REALTIME_OVERRIDE_ONCE) realtimeOverride = REALTIME_OVERRIDE_NONE;
    strip.setBrightness(scaledBri(bri));
    realtimeMode = REALTIME_MODE_INACTIVE;
    realtimeIP[0] = 0;
#ifdef WLED_USE_REALTIME_SMOOTHING
    rts.reset();
#endif
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
      //DEBUG_PRINTLN(rgbUdp.remoteIP());
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

  uint16_t len;
#ifdef WLED_USE_REALTIME_SMOOTHING
  uint8_t *udpIn = rts.get_free_buffer();
#else
  uint8_t udpIn[packetSize+1];
#endif
  if (isSupp) {
    realtimeIP = notifier2Udp.remoteIP();
    len = notifier2Udp.read(udpIn, packetSize);
  } else {
    realtimeIP = notifierUdp.remoteIP();
    len = notifierUdp.read(udpIn, packetSize);
  }

  // Realtime or Notifications on main UDP port
  if (receiveDirect && !isSupp) { 
    if (udpIn[0] > 0 && udpIn[0] <= 6) {
      if (udpIn[1] == 0) {
	realtimeTimeout = 0;
	return;
      } else {
	realtimeLock(udpIn[1]*1000 + 1, REALTIME_MODE_UDP);
      }
      if (realtimeOverride || realtimeIP == Network.localIP())
	return;
    
      do { // Read ALL pending realtime packets!
	if(udpIn[0] > 0 && udpIn[0] <= 6) {  // just skip non-realtime
#ifdef WLED_USE_REALTIME_SMOOTHING
	  if (packetSize > RTS_MAX_SIZE) continue;
	  if (!rts.add(udpIn, packetSize, now)) { // ring overflowed: remove & show one
	    RealtimeSmooth::rts_frame* show_now = rts.remove(now); 
	    setRealtimeFrame(show_now);
	    strip.show();
	    rts.add(udpIn, packetSize, now, true); // try again
	  }
#else // If not RealtimeSmoothing, just display each packet immediately
	  setRealtime(udpIn,packetSize); // Show realtime packet immediately
	  strip.show();
#endif
	}
	packetSize = notifierUdp.parsePacket();
	if(packetSize) { // A packet is waiting... read it!
#ifdef WLED_USE_REALTIME_SMOOTHING
	  udpIn = rts.get_free_buffer();
	  #if DEBUG_RTS
	  rts.waiting++;
	  #endif
#endif
	  notifierUdp.read(udpIn, packetSize);
	}
      } while (packetSize);
      return;
    }
  }

  // WLED nodes info notifications
  if (isSupp && udpIn[0] == 255 && udpIn[1] == 1 && len >= 40) {
    if (!nodeListEnabled || realtimeIP == Network.localIP()) return;

    uint8_t unit = udpIn[39];
    NodesMap::iterator it = Nodes.find(unit);
    if (it == Nodes.end() && Nodes.size() < WLED_MAX_NODES) { // Create a new element when not present
      Nodes[unit].age = 0;
      it = Nodes.find(unit);
    }

    if (it != Nodes.end()) {
      for (byte x = 0; x < 4; x++) {
        it->second.ip[x] = udpIn[x + 2];
      }
      it->second.age = 0; // reset 'age counter'
      char tmpNodeName[33] = { 0 };
      memcpy(&tmpNodeName[0], reinterpret_cast<byte *>(&udpIn[6]), 32);
      tmpNodeName[32]     = 0;
      it->second.nodeName = tmpNodeName;
      it->second.nodeName.trim();
      it->second.nodeType = udpIn[38];
      uint32_t build = 0;
      if (len >= 44)
        for (byte i=0; i<sizeof(uint32_t); i++)
          build |= udpIn[40+i]<<(8*i);
      it->second.build = build;
    }
    return;
  }
  
  //WLED Notifier: ignore if realtime packets active
  if (udpIn[0] == 0 && !realtimeMode && receiveNotifications)
  {
    //ignore notification if received within a second after sending a notification ourselves
    if (now - notificationSentTime < 1000) return;
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

/*********************************************************************************************\
   Refresh aging for remote units, drop if too old...
\*********************************************************************************************/
void refreshNodeList()
{
  for (NodesMap::iterator it = Nodes.begin(); it != Nodes.end();) {
    bool mustRemove = true;

    if (it->second.ip[0] != 0) {
      if (it->second.age < 10) {
        it->second.age++;
        mustRemove = false;
        ++it;
      }
    }

    if (mustRemove) {
      it = Nodes.erase(it);
    }
  }
}

/*********************************************************************************************\
   Broadcast system info to other nodes. (to update node lists)
\*********************************************************************************************/
void sendSysInfoUDP()
{
  if (!udp2Connected) return;

  IPAddress ip = Network.localIP();

  // TODO: make a nice struct of it and clean up
  //  0: 1 byte 'binary token 255'
  //  1: 1 byte id '1'
  //  2: 4 byte ip
  //  6: 32 char name
  // 38: 1 byte node type id
  // 39: 1 byte node id
  // 40: 4 byte version ID
  // 44 bytes total

  // send my info to the world...
  uint8_t data[44] = {0};
  data[0] = 255;
  data[1] = 1;
  
  for (byte x = 0; x < 4; x++) {
    data[x + 2] = ip[x];
  }
  memcpy((byte *)data + 6, serverDescription, 32);
  #ifdef ESP8266
  data[38] = NODE_TYPE_ID_ESP8266;
  #elif defined(ARDUINO_ARCH_ESP32)
  data[38] = NODE_TYPE_ID_ESP32;
  #else
  data[38] = NODE_TYPE_ID_UNDEFINED;
  #endif
  data[39] = ip[3]; // unit ID == last IP number

  uint32_t build = VERSION;
  for (byte i=0; i<sizeof(uint32_t); i++)
    data[40+i] = (build>>(8*i)) & 0xFF;

  IPAddress broadcastIP(255, 255, 255, 255);
  notifier2Udp.beginPacket(broadcastIP, udpPort2);
  notifier2Udp.write(data, sizeof(data));
  notifier2Udp.endPacket();
}
