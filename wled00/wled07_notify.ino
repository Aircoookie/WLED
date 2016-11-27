void notify(uint8_t callMode)
{
  switch (callMode)
  {
    case 1: if (!notifyDirect) return; break;
    case 2: if (!notifyButton) return; break;
    case 3: if (!notifyForward) return; break;
    case 4: if (!notifyNightlight) return; break;
    default: return;
  }
  byte udpOut[16];
  udpOut[0] = 0; //reserved for future "port" feature
  udpOut[1] = callMode;
  udpOut[2] = bri;
  udpOut[3] = col[0];
  udpOut[4] = col[1];
  udpOut[5] = col[2];
  udpOut[6] = nightlightActive;
  
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
      notifierUdp.read(notifierBuffer, 16);
      int bri_r = notifierBuffer[2]*(((float)bri_n)/100);
      if (bri_r < 256)
      {
        bri_n = bri_r;
      } else
      {
        bri_n = 255;
      }
      col[0] = notifierBuffer[3]
      col[1] = notifierBuffer[4];
      col[2] = notifierBuffer[5];
      if (notifierBuffer[6])
      {
        nightlightActive = true;
      } else {
        colorUpdated(3);
      }
    }
  }
}


