void notify(uint8_t callMode)
{
  if (!udpConnected) return;
  switch (callMode)
  {
    case 1: if (!notifyDirect) return; break;
    case 2: if (!notifyButton) return; break;
    case 3: return;
    case 4: if (!notifyNightlight) return; break;
    default: return;
  }
  byte udpOut[16];
  udpOut[0] = 0; //reserved
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
      col[0] = notifierBuffer[3];
      col[1] = notifierBuffer[4];
      col[2] = notifierBuffer[5];
      nightlightActive = notifierBuffer[6];
      if (!notifierBuffer[6])
      {
        bri = notifierBuffer[2];
        colorUpdated(3);
      }
    }
  }
}


