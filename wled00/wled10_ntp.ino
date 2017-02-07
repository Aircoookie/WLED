/*
 * Acquires time from NTP server
 */

void handleNetworkTime()
{
  if (ntpEnabled && ntpConnected && millis() - ntpLastSyncTime > 50000000L)
  {
    if (millis() - ntpPacketSentTime > 10000)
    {
      sendNTPPacket();
      ntpPacketSentTime = millis();
    }
    if (checkNTPResponse())
    {
      ntpLastSyncTime = millis();
    }
  }
}

void sendNTPPacket()
{
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.println("sending NTP packet...");

  memset(ntpPacketBuffer, 0, NTP_PACKET_SIZE);

  ntpPacketBuffer[0] = 0b11100011;   // LI, Version, Mode
  ntpPacketBuffer[1] = 0;     // Stratum, or type of clock
  ntpPacketBuffer[2] = 6;     // Polling Interval
  ntpPacketBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  ntpPacketBuffer[12]  = 49;
  ntpPacketBuffer[13]  = 0x4E;
  ntpPacketBuffer[14]  = 49;
  ntpPacketBuffer[15]  = 52;

  ntpUdp.beginPacket(ntpServerIP, 123); //NTP requests are to port 123
  ntpUdp.write(ntpPacketBuffer, NTP_PACKET_SIZE);
  ntpUdp.endPacket();
}

boolean checkNTPResponse()
{
  int cb = ntpUdp.parsePacket();
  if (cb) {
    DEBUG_PRINT("packet received, length=");
    DEBUG_PRINTLN(cb);

    ntpUdp.read(ntpPacketBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    unsigned long highWord = word(ntpPacketBuffer[40], ntpPacketBuffer[41]);
    unsigned long lowWord = word(ntpPacketBuffer[42], ntpPacketBuffer[43]);
    if (highWord == 0 && lowWord == 0) return false;
    
    unsigned long secsSince1900 = highWord << 16 | lowWord;
 
    DEBUG_PRINT("Unix time = ");
    unsigned long epoch = secsSince1900 - seventyYears;
    setTime(epoch);
    DEBUG_PRINTLN(epoch);
    return true;
  }
  return false;
}

String getTimeString()
{
  local = TZ.toLocal(now(), &tcr);
  String ret = monthStr(month(local));
  ret = ret + " ";
  ret = ret + day(local);
  ret = ret + " ";
  ret = ret + year(local);
  ret = ret + ", ";
  ret = ret + hour(local);
  ret = ret + ":";
  if (minute(local) < 10) ret = ret + "0";
  ret = ret + minute(local);
  ret = ret + ":";
  if (second(local) < 10) ret = ret + "0";
  ret = ret + second(local);
  return ret;
}

