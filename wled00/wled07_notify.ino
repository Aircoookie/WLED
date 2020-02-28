/*
 * UDP notifier
 */

#define WLEDPACKETSIZE 29
#define UDP_IN_MAXSIZE 1472


void notify(byte callMode, bool followUp=false)
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
  broadcastIp = ~uint32_t(WiFi.subnetMask()) | uint32_t(WiFi.gatewayIP());

  notifierUdp.beginPacket(broadcastIp, udpPort);
  notifierUdp.write(udpOut, WLEDPACKETSIZE);
  notifierUdp.endPacket();
  notificationSentCallMode = callMode;
  notificationSentTime = millis();
  notificationTwoRequired = (followUp)? false:notifyTwice;
}


void arlsLock(uint32_t timeoutMs, byte md = REALTIME_MODE_GENERIC)
{
  if (!realtimeMode){
    for (uint16_t i = 0; i < ledCount; i++)
    {
      strip.setPixelColor(i,0,0,0,0);
    }
    realtimeMode = md;
  }
  realtimeTimeout = millis() + timeoutMs;
  if (timeoutMs == 255001 || timeoutMs == 65000) realtimeTimeout = UINT32_MAX;
  if (arlsForceMaxBri) strip.setBrightness(255);
}


void handleE131Packet(e131_packet_t* p, IPAddress clientIP){
  //E1.31 protocol support
  
  // skip out-of-sequence packets
  if (p->sequence_number < e131LastSequenceNumber && p->sequence_number > 20 && e131LastSequenceNumber < 250){
    DEBUG_PRINT("skipping E1.31 frame (last seq=");
    DEBUG_PRINT(e131LastSequenceNumber);
    DEBUG_PRINT(", current seq=");
    DEBUG_PRINT(p->sequence_number);
    DEBUG_PRINTLN(")");
    return;
  }
  e131LastSequenceNumber = p->sequence_number;

  // update status info
  realtimeIP = clientIP;
  
  uint16_t uni = htons(p->universe);
  uint8_t previousUniverses = uni - e131Universe;
  uint16_t possibleLEDsInCurrentUniverse;
  uint16_t dmxChannels = htons(p->property_value_count) -1;

  switch (DMXMode) {
    case DMX_MODE_DISABLED:
      return;  // nothing to do
      break;

    case DMX_MODE_SINGLE_RGB:
      if (uni != e131Universe) return;
      if (dmxChannels-DMXAddress+1 < 3) return;
      for (uint16_t i = 0; i < ledCount; i++)
        setRealtimePixel(i, p->property_values[DMXAddress+0], p->property_values[DMXAddress+1], p->property_values[DMXAddress+2], 0);
      break;

    case DMX_MODE_SINGLE_DRGB:
      if (uni != e131Universe) return;
      if (dmxChannels-DMXAddress+1 < 4) return;
      if (DMXOldDimmer != p->property_values[DMXAddress+0]) {
        DMXOldDimmer = p->property_values[DMXAddress+0];
        bri = p->property_values[DMXAddress+0];
        strip.setBrightness(bri);
      }
      for (uint16_t i = 0; i < ledCount; i++)
        setRealtimePixel(i, p->property_values[DMXAddress+1], p->property_values[DMXAddress+2], p->property_values[DMXAddress+3], 0);
      break;

    case DMX_MODE_EFFECT:
      if (uni != e131Universe) return;
      if (dmxChannels-DMXAddress+1 < 11) return;
      if (DMXOldDimmer != p->property_values[DMXAddress+0]) {
        DMXOldDimmer = p->property_values[DMXAddress+0];
        bri = p->property_values[DMXAddress+0];
      }
      if (p->property_values[DMXAddress+1] < MODE_COUNT)
        effectCurrent = p->property_values[DMXAddress+ 1];
      effectSpeed     = p->property_values[DMXAddress+ 2];  // flickers
      effectIntensity = p->property_values[DMXAddress+ 3];
      effectPalette   = p->property_values[DMXAddress+ 4];
      col[0]          = p->property_values[DMXAddress+ 5];
      col[1]          = p->property_values[DMXAddress+ 6];
      col[2]          = p->property_values[DMXAddress+ 7];
      colSec[0]       = p->property_values[DMXAddress+ 8];
      colSec[1]       = p->property_values[DMXAddress+ 9];
      colSec[2]       = p->property_values[DMXAddress+10];
      if (dmxChannels-DMXAddress+1 > 11)
      {
        col[3]          = p->property_values[DMXAddress+11]; //white
        colSec[3]       = p->property_values[DMXAddress+12];
      }
      transitionDelayTemp = 0;                        // act fast
      colorUpdated(NOTIFIER_CALL_MODE_NOTIFICATION);  // don't send UDP
      return;                                         // don't activate realtime live mode
      break;

    case DMX_MODE_MULTIPLE_RGB:
      if (previousUniverses == 0) {
        // first universe of this fixture
        possibleLEDsInCurrentUniverse = (dmxChannels - DMXAddress + 1) / 3;
        for (uint16_t i = 0; i < ledCount; i++) {
          if (i >= possibleLEDsInCurrentUniverse) break;  // more LEDs will follow in next universe(s)
          setRealtimePixel(i, p->property_values[DMXAddress+i*3+0], p->property_values[DMXAddress+i*3+1], p->property_values[DMXAddress+i*3+2], 0);
        }
      } else if (previousUniverses > 0 && uni < (e131Universe + E131_MAX_UNIVERSE_COUNT)) {
        // additional universe(s) of this fixture
        uint16_t numberOfLEDsInPreviousUniverses = ((512 - DMXAddress + 1) / 3);                            // first universe
        if (previousUniverses > 1) numberOfLEDsInPreviousUniverses += (512 / 3) * (previousUniverses - 1);  // extended universe(s) before current
        possibleLEDsInCurrentUniverse = dmxChannels / 3;
        for (uint16_t i = numberOfLEDsInPreviousUniverses; i < ledCount; i++) {
          uint8_t j = i - numberOfLEDsInPreviousUniverses;
          if (j >= possibleLEDsInCurrentUniverse) break;   // more LEDs will follow in next universe(s)
          setRealtimePixel(i, p->property_values[j*3+1], p->property_values[j*3+2], p->property_values[j*3+3], 0);
        }
      }
      break;

    case DMX_MODE_MULTIPLE_DRGB:
      if (previousUniverses == 0) {
        // first universe of this fixture
        if (DMXOldDimmer != p->property_values[DMXAddress+0]) {
          DMXOldDimmer = p->property_values[DMXAddress+0];
          bri = p->property_values[DMXAddress+0];
          strip.setBrightness(bri);
        }
        possibleLEDsInCurrentUniverse = (dmxChannels - DMXAddress) / 3;
        for (uint16_t i = 0; i < ledCount; i++) {
          if (i >= possibleLEDsInCurrentUniverse) break;  // more LEDs will follow in next universe(s)
          setRealtimePixel(i, p->property_values[DMXAddress+i*3+1], p->property_values[DMXAddress+i*3+2], p->property_values[DMXAddress+i*3+3], 0);
        }
      } else if (previousUniverses > 0 && uni < (e131Universe + E131_MAX_UNIVERSE_COUNT)) {
        // additional universe(s) of this fixture
        uint16_t numberOfLEDsInPreviousUniverses = ((512 - DMXAddress + 1) / 3);                            // first universe
        if (previousUniverses > 1) numberOfLEDsInPreviousUniverses += (512 / 3) * (previousUniverses - 1);  // extended universe(s) before current
        possibleLEDsInCurrentUniverse = dmxChannels / 3;
        for (uint16_t i = numberOfLEDsInPreviousUniverses; i < ledCount; i++) {
          uint8_t j = i - numberOfLEDsInPreviousUniverses;
          if (j >= possibleLEDsInCurrentUniverse) break;   // more LEDs will follow in next universe(s)
          setRealtimePixel(i, p->property_values[j*3+1], p->property_values[j*3+2], p->property_values[j*3+3], 0);
        }
      }
      break;

    default:
      DEBUG_PRINTLN("unknown E1.31 DMX mode");
      return;  // nothing to do
      break;
  }

  arlsLock(realtimeTimeoutMs, REALTIME_MODE_E131);
  e131NewData = true;
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
    strip.setBrightness(bri);
    realtimeMode = REALTIME_MODE_INACTIVE;
  }

  //receive UDP notifications
  if (!udpConnected || !(receiveNotifications || receiveDirect)) return;
    
  uint16_t packetSize = notifierUdp.parsePacket();

  //hyperion / raw RGB
  if (!packetSize && udpRgbConnected) {
    packetSize = rgbUdp.parsePacket();
    if (!receiveDirect) return;
    if (packetSize > UDP_IN_MAXSIZE || packetSize < 3) return;
    realtimeIP = rgbUdp.remoteIP();
    DEBUG_PRINTLN(rgbUdp.remoteIP());
    uint8_t lbuf[packetSize];
    rgbUdp.read(lbuf, packetSize);
    arlsLock(realtimeTimeoutMs, REALTIME_MODE_HYPERION);
    uint16_t id = 0;
    for (uint16_t i = 0; i < packetSize -2; i += 3)
    {
      setRealtimePixel(id, lbuf[i], lbuf[i+1], lbuf[i+2], 0);
      
      id++; if (id >= ledCount) break;
    }
    strip.show();
    return;
  }

  //notifier and UDP realtime
  if (packetSize > UDP_IN_MAXSIZE) return;
  if(packetSize && notifierUdp.remoteIP() != WiFi.localIP()) //don't process broadcasts we send ourselves
  {
    uint8_t udpIn[packetSize];
    notifierUdp.read(udpIn, packetSize);

    //wled notifier, block if realtime packets active
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
      
    }  else if (udpIn[0] > 0 && udpIn[0] < 5 && receiveDirect) //1 warls //2 drgb //3 drgbw
    {
      realtimeIP = notifierUdp.remoteIP();
      DEBUG_PRINTLN(notifierUdp.remoteIP());
      if (packetSize > 1) {
        if (udpIn[1] == 0)
        {
          realtimeTimeout = 0;
          return;
        } else {
          arlsLock(udpIn[1]*1000 +1, REALTIME_MODE_UDP);
        }
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
      }
    }
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
