/*
 * UDP notifier
 */

void notify(uint8_t callMode)
{
  if (!udpConnected || !notifyMaster) return;
  switch (callMode)
  {
    case 1: if (!notifyDirect) return; break;
    case 2: if (!notifyButton) return; break;
    case 3: return;
    case 4: if (!notifyNightlight) return; break;
    case 6: if (!notifyDirect) return; break; //fx change
    default: return;
  }
  byte udpOut[16];
  udpOut[0] = 233; //233: wled notifier protocol 0-7: ARLS8 protocol 253: IRGB protocol 254: RGB protocol
  udpOut[1] = callMode;
  udpOut[2] = bri;
  udpOut[3] = col[0];
  udpOut[4] = col[1];
  udpOut[5] = col[2];
  udpOut[6] = nightlightActive;
  udpOut[7] = nightlightDelayMins;
  udpOut[8] = effectCurrent;
  udpOut[9] = effectSpeed;
  
  IPAddress broadcastIp;
  broadcastIp = ~WiFi.subnetMask() | WiFi.gatewayIP();

  notifierUdp.beginPacket(broadcastIp, udpPort);
  notifierUdp.write(udpOut, 16);
  notifierUdp.endPacket();
}

void handleNotifications()
{
  if(udpConnected && receiveNotifications){
    int packetSize = notifierUdp.parsePacket();
    if(packetSize && notifierUdp.remoteIP() != WiFi.localIP())
    {
      notifierUdp.read(udpIn, packetSize);
      if (udpIn[0] == 233) //wled notifier
      {
        col[0] = udpIn[3];
        col[1] = udpIn[4];
        col[2] = udpIn[5];
        if (udpIn[8] != effectCurrent)
        {
          effectCurrent = udpIn[8];
          strip.setMode(effectCurrent);
        }
        if (udpIn[9] != effectSpeed)
        {
          effectSpeed = udpIn[9];
          strip.setSpeed(effectSpeed);
        }
        nightlightActive = udpIn[6];
        if (!udpIn[6])
        {
          bri = udpIn[2];
          colorUpdated(3);
        }
      }  else if (udpIn[0] == 253) //irgb
      {
        arlsTimeout = true;
        arlsTimeoutTime = millis() + arlsTimeoutMillis;
        for (int i = 1; i < packetSize -3; i += 4)
        {
          if (udpIn[i] < LEDCOUNT)
          if (useGammaCorrectionRGB)
          {
            strip.setIndividual(udpIn[i], ((uint32_t)gamma8[udpIn[i+1]] << 16) | ((uint32_t)gamma8[udpIn[i+2]] << 8) | gamma8[udpIn[i+3]]);
          } else {
            strip.setIndividual(udpIn[i], ((uint32_t)udpIn[i+1] << 16) | ((uint32_t)udpIn[i+2] << 8) | udpIn[i+3]);
          }
        }
      } else if (udpIn[0] == 254) //rgb
      {
        arlsTimeout = true;
        arlsTimeoutTime = millis() + arlsTimeoutMillis;
        for (int i = 1; i < packetSize -3; i += 3)
        {
          strip.setIndividual(udpIn[i/3], ((uint32_t)gamma8[udpIn[i]] << 16) | ((uint32_t)gamma8[udpIn[i]] << 8) | gamma8[udpIn[i]]);
        }
      } else //ARLS8 for now
      {
        if (useGammaCorrectionRGB)
        {
          strip.setColor(gamma8[udpIn[13]], gamma8[udpIn[14]], gamma8[udpIn[15]]);
        } else
        {
          strip.setColor(udpIn[13], udpIn[14], udpIn[15]);
        }
        strip.trigger();
      }
    }
    if (arlsTimeout && millis() > arlsTimeoutTime)
    {
      strip.unlockAll();
      arlsTimeout = false;
    }
  }
}


