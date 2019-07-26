/*
 * UDP notifier
 */

#define WLEDPACKETSIZE 24
#define UDP_IN_MAXSIZE 1472


void notify(byte callMode, bool followUp=false)
{
  if (!udpConnected) return;
  switch (callMode)
  {
    case 0: return;
    case 1: if (!notifyDirect) return; break;
    case 2: if (!notifyButton) return; break;
    case 4: if (!notifyDirect) return; break;
    case 6: if (!notifyDirect) return; break; //fx change
    case 7: if (!notifyHue)    return; break;
    case 8: if (!notifyDirect) return; break;
    case 9: if (!notifyDirect) return; break;
    case 10: if (!notifyAlexa) return; break;
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
  //6: supports tertiary color
  udpOut[11] = 5; 
  udpOut[12] = colSec[0];
  udpOut[13] = colSec[1];
  udpOut[14] = colSec[2];
  udpOut[15] = colSec[3];
  udpOut[16] = effectIntensity;
  udpOut[17] = (transitionDelay >> 0) & 0xFF;
  udpOut[18] = (transitionDelay >> 8) & 0xFF;
  udpOut[19] = effectPalette;
  /*udpOut[20] = colTer[0];
  udpOut[21] = colTer[1];
  udpOut[22] = colTer[2];
  udpOut[23] = colTer[3];*/
  
  IPAddress broadcastIp;
  broadcastIp = ~uint32_t(WiFi.subnetMask()) | uint32_t(WiFi.gatewayIP());

  notifierUdp.beginPacket(broadcastIp, udpPort);
  notifierUdp.write(udpOut, WLEDPACKETSIZE);
  notifierUdp.endPacket();
  notificationSentCallMode = callMode;
  notificationSentTime = millis();
  notificationTwoRequired = (followUp)? false:notifyTwice;
}


void arlsLock(uint32_t timeoutMs)
{
  if (!realtimeActive){
    for (uint16_t i = 0; i < ledCount; i++)
    {
      strip.setPixelColor(i,0,0,0,0);
    }
    strip.unlockAll();
    realtimeActive = true;
  }
  realtimeTimeout = millis() + timeoutMs;
  if (timeoutMs == 255001 || timeoutMs == 65000) realtimeTimeout = UINT32_MAX;
  if (arlsForceMaxBri) strip.setBrightness(255);
}


void initE131(){
  if (WiFi.status() == WL_CONNECTED && e131Enabled)
  {
    e131 = new E131();
    e131->begin((e131Multicast) ? E131_MULTICAST : E131_UNICAST , e131Universe);
  } else {
    e131Enabled = false;
  }
}

void initTPM2net() {
	if (WiFi.status() == WL_CONNECTED && tpm2netEnabled)
	{
		tpm2net = new TPM2NET();
		tpm2net->begin();
	}
	else {
		tpm2netEnabled = false;
	}
}

void handleE131(){
  //E1.31 protocol support
  if(e131Enabled) {
    uint16_t len = tpm2net->parsePacket();
    if (!len || e131->universe < e131Universe || e131->universe > e131Universe +4) return;
    len /= 3; //one LED is 3 DMX channels
    
    uint16_t multipacketOffset = (e131->universe - e131Universe)*170; //if more than 170 LEDs (510 channels), client will send in next higher universe 
    if (ledCount <= multipacketOffset) return;

    arlsLock(realtimeTimeoutMs);
    if (len + multipacketOffset > ledCount) len = ledCount - multipacketOffset;
    
    for (uint16_t i = 0; i < len; i++) {
      int j = i * 3;
      setRealtimePixel(i + multipacketOffset, e131->data[j], e131->data[j+1], e131->data[j+2], 0);
    }
    strip.show();
  }
}

void handletpm2net() {
	//tpm2net protocol support
	if (tpm2netEnabled) {
		uint16_t countofData = tpm2net->parsePacket();
		if (countofData <=1 ) return;
		countofData /= 3; //one LED is 3 DMX channels

		arlsLock(realtimeTimeoutMs);

		uint16_t currentLed = tpm2net->packetNumber * ledCount;
		int dataposition = TPM2NET_HEADER_SIZE;
		for (byte i = 0; i < tpm2net->frameSize; i++) {
			setRealtimePixel(currentLed++, tpm2net->tpm2packet[dataposition], tpm2net->tpm2packet[dataposition + 1], tpm2net->tpm2packet[dataposition + 2], 0);
			dataposition += 3;
		}
		strip.show();
	}
}

void handleNotifications()
{
  //send second notification if enabled
  if(udpConnected && notificationTwoRequired && millis()-notificationSentTime > 250){
    notify(notificationSentCallMode,true);
  }

  handleE131();

  handletpm2net();

  //unlock strip when realtime UDP times out
  if (realtimeActive && millis() > realtimeTimeout)
  {
    //strip.unlockAll();
    strip.setBrightness(bri);
    realtimeActive = false;
    //strip.setMode(effectCurrent);
    realtimeIP[0] = 0;
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
    arlsLock(realtimeTimeoutMs);
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
    if (udpIn[0] == 0 && !realtimeActive && receiveNotifications)
    {
      bool someSel = (receiveNotificationBrightness || receiveNotificationColor || receiveNotificationEffects);
      //apply colors from notification
      if (receiveNotificationColor || !someSel)
      {
        col[0] = udpIn[3];
        col[1] = udpIn[4];
        col[2] = udpIn[5];
        if (udpIn[11] > 0) //check if sending modules white val is inteded
        {
          col[3] = udpIn[10];
          if (udpIn[11] > 1)
          {
            colSec[0] = udpIn[12];
            colSec[1] = udpIn[13];
            colSec[2] = udpIn[14];
            colSec[3] = udpIn[15];
          }
          /*if (udpIn[11] > 5)
          {
            colTer[0] = udpIn[20];
            colTer[1] = udpIn[21];
            colTer[2] = udpIn[22];
            colSec[3] = udpIn[23];
          }*/
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
      colorUpdated(3);
      
    }  else if (udpIn[0] > 0 && udpIn[0] < 4 && receiveDirect) //1 warls //2 drgb //3 drgbw
    {
      realtimeIP = notifierUdp.remoteIP();
      DEBUG_PRINTLN(notifierUdp.remoteIP());
      if (packetSize > 1) {
        if (udpIn[1] == 0)
        {
          realtimeTimeout = 0;
          return;
        } else {
          arlsLock(udpIn[1]*1000 +1);
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
            setRealtimePixel(id, udpIn[i], udpIn[i+1], udpIn[i+2], udpIn[i+3]);
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
