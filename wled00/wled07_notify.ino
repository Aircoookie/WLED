/*
 * UDP notifier
 */

#define WLEDPACKETSIZE 24

void notify(byte callMode, bool followUp=false)
{
  if (!udpConnected) return;
  switch (callMode)
  {
    case 1: if (!notifyDirect) return; break;
    case 2: if (!notifyButton) return; break;
    case 4: if (!notifyDirect) return; break;
    case 6: if (!notifyDirect) return; break; //fx change
    case 7: if (!notifyHue) return; break;
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
  udpOut[10] = white;
  udpOut[11] = 3; //compatibilityVersionByte: 0: old 1: supports white 2: supports secondary color 3: supports FX intensity, 24 byte packet
  udpOut[12] = colSec[0];
  udpOut[13] = colSec[1];
  udpOut[14] = colSec[2];
  udpOut[15] = whiteSec;
  udpOut[16] = effectIntensity;
  
  IPAddress broadcastIp;
  broadcastIp = ~WiFi.subnetMask() | WiFi.gatewayIP();

  notifierUdp.beginPacket(broadcastIp, udpPort);
  notifierUdp.write(udpOut, WLEDPACKETSIZE);
  notifierUdp.endPacket();
  notificationSentCallMode = callMode;
  notificationSentTime = millis();
  notificationTwoRequired = (followUp)? false:notifyTwice;
}

void handleNotifications()
{
  if(udpConnected && notificationTwoRequired && millis()-notificationSentTime > 250){
    notify(notificationSentCallMode,true);
  }
  
  if(udpConnected && receiveNotifications){
    uint16_t packetSize = notifierUdp.parsePacket();
    if (packetSize > 1026) return;
    if(packetSize && notifierUdp.remoteIP() != WiFi.localIP()) //don't process broadcasts we send ourselves
    {
      byte udpIn[packetSize];
      notifierUdp.read(udpIn, packetSize);
      if (udpIn[0] == 0 && !arlsTimeout) //wled notifier, block if realtime packets active
      {
        if (receiveNotificationColor)
        {
        col[0] = udpIn[3];
        col[1] = udpIn[4];
        col[2] = udpIn[5];
        }
        if (udpIn[11] > 0 && receiveNotificationColor) //check if sending modules white val is inteded
        {
          white = udpIn[10];
          if (udpIn[11] > 1 )
          {
            colSec[0] = udpIn[12];
            colSec[1] = udpIn[13];
            colSec[2] = udpIn[14];
            whiteSec = udpIn[15];
          }
        }
        if (udpIn[8] != effectCurrent && receiveNotificationEffects)
        {
          effectCurrent = udpIn[8];
          strip.setMode(effectCurrent);
        }
        if (udpIn[9] != effectSpeed && receiveNotificationEffects)
        {
          effectSpeed = udpIn[9];
          strip.setSpeed(effectSpeed);
        }
        if (udpIn[11] > 2 && udpIn[16] != effectIntensity && receiveNotificationEffects)
        {
          effectIntensity = udpIn[16];
          strip.setIntensity(effectIntensity);
        }
        nightlightActive = udpIn[6];
        if (!nightlightActive)
        {
          if (receiveNotificationBrightness) bri = udpIn[2];
          colorUpdated(3);
        }
      }  else if (udpIn[0] == 1) //warls
      {
        if (packetSize > 1) {
          if (udpIn[1] == 0)
          {
            arlsTimeout = false;
          } else {
            if (!arlsTimeout){
              strip.setRange(0, ledCount-1, 0);
              strip.setMode(0);
            }
            arlsTimeout = true;
            arlsTimeoutTime = millis() + 1000*udpIn[1];
          }
          for (int i = 2; i < packetSize -3; i += 4)
          {
            if (udpIn[i] + arlsOffset < ledCount && udpIn[i] + arlsOffset >= 0)
            if (useGammaCorrectionRGB)
            {
              strip.setPixelColor(udpIn[i] + arlsOffset, gamma8[udpIn[i+1]], gamma8[udpIn[i+2]], gamma8[udpIn[i+3]]);
            } else {
              strip.setPixelColor(udpIn[i] + arlsOffset, udpIn[i+1], udpIn[i+2], udpIn[i+3]);
            }
          }
          strip.show();
        }
      }
    }
    if (arlsTimeout && millis() > arlsTimeoutTime)
    {
      strip.unlockAll();
      arlsTimeout = false;
      strip.setMode(effectCurrent);
    }
  }
}


