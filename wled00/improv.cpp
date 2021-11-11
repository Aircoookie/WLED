#include "wled.h"

#ifdef WLED_DEBUG_IMPROV
  #define DIMPROV_PRINT(x) Serial.print(x)
  #define DIMPROV_PRINTLN(x) Serial.println(x)
  #define DIMPROV_PRINTF(x...) Serial.printf(x)
#else
  #define DIMPROV_PRINT(x)
  #define DIMPROV_PRINTLN(x)
  #define DIMPROV_PRINTF(x...)
#endif

#define IMPROV_VERSION 1

enum ImprovPacketType {
  Current_State = 0x01,
  Error_State = 0x02,
  RPC_Command = 0x03,
  RPC_Response = 0x04
};

enum ImprovPacketByte {
  Version = 6,
  PacketType = 7,
  Length = 8
};

enum ImprovRPCType {
  Command_Wifi = 0x01,
  Request_State = 0x02,
  Request_Info = 0x03
};

//blocking function to parse an Improv Serial packet
void handleImprovPacket() {
  uint8_t header[6] = {'I','M','P','R','O','V'};

  bool timeout = false;
  uint8_t waitTime = 25;
  uint16_t packetByte = 0;
  uint8_t packetLen = 9;

  File f = WLED_FS.open("/improvon.log","a");

  bool isProvisioning = false;
  char ssid[40], pass[70];
  uint8_t ssidLen = 0, passLen = 0;

  while (!timeout) {
    if (Serial.available() < 1) {
      delay(1);
      waitTime--;
      if (!waitTime) timeout = true;
      continue;
    }
    byte next = Serial.read();
    DIMPROV_PRINT("Received improv byte: "); DIMPROV_PRINTF("%x\r\n",next);
    f.write(next);
    switch (packetByte) {
      case ImprovPacketByte::Version: {
        if (next != IMPROV_VERSION) {
          DIMPROV_PRINTLN(F("Invalid version"));
          return;
        }
      } break;
      case ImprovPacketByte::PacketType: {
        if (next != ImprovPacketType::RPC_Command) {
          DIMPROV_PRINTF("Non RPC-command improv packet type %i\n",next);
          return;
        }
      } break;
      case ImprovPacketByte::Length: packetLen = 9 + next; break;
      default: {
        if (packetByte >= packetLen -1) {f.close(); return;}
        if (packetByte < 6) { //check header
          if (next != header[packetByte]) {
            DIMPROV_PRINTLN(F("Invalid improv header"));
            return;
          }
        } else if (packetByte == 9) { //RPC command
          switch (next) {
            case ImprovRPCType::Command_Wifi: isProvisioning = true; break;
            case ImprovRPCType::Request_State: {
              uint8_t improvState = 0x02; //authorized
              if (WLED_WIFI_CONFIGURED) improvState = 0x03; //provisioning
              if (Network.isConnected()) improvState = 0x04; //provisioned
              sendImprovStateResponse(improvState, false, &f); break;
            }
            case ImprovRPCType::Request_Info: sendImprovInfoResponse(&f); break;
            default: {
              DIMPROV_PRINTF("Unknown RPC command %i\n",next);
              sendImprovStateResponse(0x02, true, &f);
            }
          }
        } else {

        }
      }
    }

    packetByte++;
  }
  f.close();
}

void sendImprovStateResponse(uint8_t state, bool error, File *f) {
  char out[11] = {'I','M','P','R','O','V'};
  out[6] = IMPROV_VERSION;
  out[7] = error? ImprovPacketType::Error_State : ImprovPacketType::Current_State;
  out[8] = 1;
  out[9] = state;

  uint8_t checksum = 0;
  for (uint8_t i = 0; i < 10; i++) checksum += out[i];
  out[10] = checksum;
  Serial.write(out, 11);
  Serial.write('\n');
  f->print("S-REPLY");
  f->write((uint8_t*)out, 11);
}

void sendImprovRPCResponse() {
  uint8_t packetLen = 12;
  char out[64] = {'I','M','P','R','O','V'};
  out[6] = IMPROV_VERSION;
  out[7] = ImprovPacketType::RPC_Response;
  out[8] = 2; //Length (set below)
  out[9] = ImprovRPCType::Command_Wifi;
  out[10] = 0; //Data len (set below)
  out[11] = '\0'; //URL len (set below)

  if (Network.isConnected())
  {
    IPAddress localIP = Network.localIP();
    uint8_t len = sprintf(out+12, "http://%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);
    if (len > 24) return; //sprintf fail?
    out[11] = len;
    out[10] = 1 + len;
    out[8] = 5 + len; //RPC command type + data len + url len + url + RPC checksum
    packetLen = 13 + len; 
  }

  uint8_t checksum = 0;
  for (uint8_t i = 9; i < packetLen -1; i++) checksum += out[i];
  out[packetLen -1] = checksum;
  Serial.write(out, packetLen);
}

void sendImprovInfoResponse(File *f) {
  uint8_t packetLen = 12;
  char out[128] = {'I','M','P','R','O','V'};
  out[6] = IMPROV_VERSION;
  out[7] = ImprovPacketType::RPC_Response;
  //out[8] = 2; //Length (set below)
  out[9] = ImprovRPCType::Request_Info;
  //out[10] = 0; //Data len (set below)
  out[11] = 4; //Firmware len ("WLED")
  out[12] = 'W'; out[13] = 'L'; out[14] = 'E'; out[15] = 'D';
  uint8_t lengthSum = 17;
  uint8_t vlen = sprintf_P(out+lengthSum,PSTR("0.13.0-b4/%i"),VERSION);
  out[16] = vlen; lengthSum += vlen;
  uint8_t hlen = 7;
  #ifdef ESP8266
  strcpy(out+lengthSum+1,"esp8266");
  #else
  hlen = 5;
  strcpy(out+lengthSum+1,"esp32");
  #endif
  out[lengthSum] = hlen;
  lengthSum += hlen + 1;
  //Use serverDescription if it has been changed from the default "WLED", else mDNS name
  bool useMdnsName = (strcmp(serverDescription, "WLED") == 0 && strlen(cmDNS) > 0);
  strcpy(out+lengthSum+1,useMdnsName ? cmDNS : serverDescription);
  uint8_t nlen = strlen(useMdnsName ? cmDNS : serverDescription);
  out[lengthSum] = nlen;
  lengthSum += nlen + 1;

  packetLen = lengthSum +1;
  out[8] = lengthSum -8;
  out[10] = lengthSum -11;

  uint8_t checksum = 0;
  for (uint8_t i = 9; i < packetLen -1; i++) checksum += out[i];
  out[packetLen -1] = checksum;
  Serial.write(out, packetLen);
  Serial.write('\n');
  f->print("REPLY");
  f->write((uint8_t*)out, packetLen);
}

void improvConnectWiFi() {
  forceReconnect = true;
}