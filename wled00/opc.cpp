/*
  OPC Protocol support
*/

#include "wled.h"
#include "cbuf.h"

#ifdef WLED_ENABLE_OPC

#define OPC_SERVER_PORT 7890
#define OPC_LISTEN_CHANNEL 1
#define OPC_MAX_FRAME_STORAGE 30
#define OPC_HEADER_SIZE 4

void freeOPCClient(AsyncClient* client) {
  client->close(true);
  delete clientServed;
  clientServed = nullptr;
}

void handleOPCError(void* arg, AsyncClient* client, int8_t error) {
	freeOPCClient(client);
}

void handleOPCDisconnect(void* arg, AsyncClient* client) {
	freeOPCClient(client);
}

void handleOPCTimeOut(void* arg, AsyncClient* client, uint32_t time) {
	freeOPCClient(client);
}

void handleOPCData(void* arg, AsyncClient* client, void *data, size_t len) {
  if (len > opcCircularBufPtr->room()) return;
  for (unsigned int i = 0; i < len; i++) { 
    opcCircularBufPtr->write(*((uint8_t*)data + i));
  }
}

void handleOPCClient(void* arg, AsyncClient* client) {
  //add client
  if (clientServed != nullptr) freeOPCClient(client);
	clientServed = client;

  //clean and initialize buffer
  opcDataLen = 0;
  opcCircularBufPtr->flush();
  opcParserWaitingForData = false;
	
	// register client events
	client->onData(&handleOPCData, NULL);
	client->onError(&handleOPCError, NULL);
	client->onDisconnect(&handleOPCDisconnect, NULL);
	client->onTimeout(&handleOPCTimeOut, NULL);
}

void discardInvalidData(uint16_t len) {
  if (len > opcCircularBufPtr->available()) {
    opcCircularBufPtr->flush();
  } else {
    for (uint16_t i = 0; i < len; i++) opcCircularBufPtr->read();
  }
}

void parseOPCLedValues(uint8_t chan, uint8_t cmd) {
  const uint16_t opcDataLenDiv = opcDataLen / 3; 
  uint8_t r, g, b;
  if ((chan == 0 || chan == OPC_LISTEN_CHANNEL) && cmd == 0) {
    realtimeLock(realtimeTimeoutMs, REALTIME_MODE_OPC);
    
    for (uint16_t i = 0; i < opcDataLenDiv && i < ledCount; i++) {
      r = opcCircularBufPtr->read();
      g = opcCircularBufPtr->read();
      b = opcCircularBufPtr->read();
      setRealtimePixel(i, r, g, b, 0);
      
    }
    strip.show();
    if (opcDataLen > ledCount * 3) discardInvalidData(opcDataLen - ledCount * 3);
  } else {
    discardInvalidData(opcDataLen);
  }
}

void handleOPCPacket() {
  // channel and command variables need staticity because of fragmented packet possibility
  static uint8_t chan, cmd;
  
  if (opcCircularBufPtr != nullptr && millis() - strip.getLastShow() > 15) { 
    if (opcCircularBufPtr->available() >= 4 && !realtimeOverride) {
      //there's at least a header and no override
      if(!opcParserWaitingForData) {
        chan = (uint8_t)opcCircularBufPtr->read();
        cmd = (uint8_t)opcCircularBufPtr->read();
        opcDataLen = (uint8_t)opcCircularBufPtr->read() << 8 | (uint8_t)opcCircularBufPtr->read();
        if (opcDataLen > opcCircularBufPtr->available()) {
          opcParserWaitingForData = true;
        } else {
          parseOPCLedValues(chan, cmd);
        }
      } else {
        if (opcDataLen <= opcCircularBufPtr->available()) {
          parseOPCLedValues(chan, cmd);
          opcParserWaitingForData = false;
        }
      }
    } else if (realtimeOverride && opcCircularBufPtr->available()) {
      opcCircularBufPtr->flush();
      opcParserWaitingForData = false;
    }
  }
}

void initOPCServer() {
  if (opcServer != nullptr) {
    opcServer->end();
    delete opcServer;
    opcServer = nullptr;
  }
  if (opcCircularBufPtr != nullptr) {
    opcCircularBufPtr->flush();
    delete opcCircularBufPtr;
    opcCircularBufPtr = nullptr;
  }
  opcCircularBufPtr = new cbuf(OPC_MAX_FRAME_STORAGE * (ledCount + OPC_HEADER_SIZE));
  opcServer = new AsyncServer(OPC_SERVER_PORT); 
  opcServer->onClient(&handleOPCClient, opcServer);
  opcServer->setNoDelay(true);
  opcServer->begin();

  opcDataLen = 0;
  opcParserWaitingForData = false;
}

#else
void initOPCServer(){}
void handleOPCClient() {}
void freeOPCClient(){}
void handleOPCData() {}
void handleOPCTimeOut (){}
void handleOPCError (){}
void handleOPCDisconnect (){}
#endif
