void notify(uint8_t callMode)
{
  if (!udpConnected || !notifyMaster) return;
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
      notifierUdp.read(udpIn, 16);
      col[0] = udpIn[3];
      col[1] = udpIn[4];
      col[2] = udpIn[5];
      if (true) //always receive effects?
      {
        effectCurrent = udpIn[8];
        effectSpeed = udpIn[9];
      }
      nightlightActive = udpIn[6];
      if (!udpIn[6])
      {
        bri = udpIn[2];
        colorUpdated(3);
      }
    }
  }
}


