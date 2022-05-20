#include "wled.h"

/*
 * WebSockets server for bidirectional communication
 */
#ifdef WLED_ENABLE_WEBSOCKETS

uint16_t wsLiveClientId = 0;
unsigned long wsLastLiveTime = 0;
//uint8_t* wsFrameBuffer = nullptr;

#define WS_LIVE_INTERVAL 40

void wsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
  if(type == WS_EVT_CONNECT){
    //client connected
    sendDataWs(client);
    DEBUG_PRINTLN(F("WS client connected."));
  } else if(type == WS_EVT_DISCONNECT){
    //client disconnected
    if (client->id() == wsLiveClientId) wsLiveClientId = 0;
    DEBUG_PRINTLN(F("WS client disconnected."));
  } else if(type == WS_EVT_DATA){
    //data packet
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    if(info->final && info->index == 0 && info->len == len){
      //the whole message is in a single frame and we got all of its data (max. 1450byte)
      if(info->opcode == WS_TEXT)
      {
        if (len > 0 && len < 10 && data[0] == 'p') {
          //application layer ping/pong heartbeat.
          //client-side socket layer ping packets are unresponded (investigate)
          client->text(F("pong"));
          return;
        }

        bool verboseResponse = false;
        if (!requestJSONBufferLock(11)) return;

        DeserializationError error = deserializeJson(doc, data, len);
        JsonObject root = doc.as<JsonObject>();
        if (error || root.isNull()) {
          releaseJSONBufferLock();
          return;
        }
        if (root["v"] && root.size() == 1) {
          //if the received value is just "{"v":true}", send only to this client
          verboseResponse = true;
        } else if (root.containsKey("lv")) {
          wsLiveClientId = root["lv"] ? client->id() : 0;
        } else {
          verboseResponse = deserializeState(root);
        }
        releaseJSONBufferLock(); // will clean fileDoc

        // force broadcast in 500ms after upadting client
        if (verboseResponse) {
          sendDataWs(client);
          lastInterfaceUpdate = millis() - (INTERFACE_UPDATE_COOLDOWN -500);
        }
      }
    } else {
      //message is comprised of multiple frames or the frame is split into multiple packets
      //if(info->index == 0){
        //if (!wsFrameBuffer && len < 4096) wsFrameBuffer = new uint8_t[4096];
      //}

      //if (wsFrameBuffer && len < 4096 && info->index + info->)
      //{

      //}

      if((info->index + len) == info->len){
        if(info->final){
          if(info->message_opcode == WS_TEXT) {
            client->text(F("{\"error\":9}")); //we do not handle split packets right now
          }
        }
      }
      DEBUG_PRINTLN(F("WS multipart message."));
    }
  } else if(type == WS_EVT_ERROR){
    //error was received from the other end
    DEBUG_PRINTLN(F("WS error."));

  } else if(type == WS_EVT_PONG){
    //pong message was received (in response to a ping request maybe)
    DEBUG_PRINTLN(F("WS pong."));

  }
}

void sendDataWs(AsyncWebSocketClient * client)
{
  if (!ws.count()) return;
  AsyncWebSocketMessageBuffer * buffer;

  while (strip.isUpdating()) yield();
  
  if (!requestJSONBufferLock(12)) return;

  JsonObject state = doc.createNestedObject("state");
  serializeState(state);
  JsonObject info  = doc.createNestedObject("info");
  serializeInfo(info);

  DEBUG_PRINTF("JSON buffer size: %u for WS request.\n", doc.memoryUsage());
  size_t len = measureJson(doc);

  size_t heap1 = ESP.getFreeHeap();
  buffer = ws.makeBuffer(len); // will not allocate correct memory sometimes
  size_t heap2 = ESP.getFreeHeap();
  if (!buffer || heap1-heap2<len) {
    releaseJSONBufferLock();
    ws.closeAll(1013); //code 1013 = temporary overload, try again later
    ws.cleanupClients(0); //disconnect all clients to release memory
    return; //out of memory
  }

  serializeJson(doc, (char *)buffer->get(), len +1);
  releaseJSONBufferLock();

  DEBUG_PRINT(F("Sending WS data "));
  if (client) {
    client->text(buffer);
    DEBUG_PRINTLN(F("to a single client."));
  } else {
    ws.textAll(buffer);
    DEBUG_PRINTLN(F("to multiple clients."));
  }
}

#define MAX_LIVE_LEDS_WS 256

bool sendLiveLedsWs(uint32_t wsClient)
{
  AsyncWebSocketClient * wsc = ws.client(wsClient);
  if (!wsc || wsc->queueLength() > 0) return false; //only send if queue free

  uint16_t used = strip.getLengthTotal();
  uint16_t n = ((used -1)/MAX_LIVE_LEDS_WS) +1; //only serve every n'th LED if count over MAX_LIVE_LEDS_WS
  uint16_t bufSize = 2 + (used/n)*3;
  AsyncWebSocketMessageBuffer * wsBuf = ws.makeBuffer(bufSize);
  if (!wsBuf) return false; //out of memory
  uint8_t* buffer = wsBuf->get();
  buffer[0] = 'L';
  buffer[1] = 1; //version

  uint16_t pos = 2;
  for (uint16_t i= 0; pos < bufSize -2; i += n)
  {
    uint32_t c = strip.getPixelColor(i);
    buffer[pos++] = qadd8(W(c), R(c)); //R, add white channel to RGB channels as a simple RGBW -> RGB map
    buffer[pos++] = qadd8(W(c), G(c)); //G
    buffer[pos++] = qadd8(W(c), B(c)); //B
  }

  wsc->binary(wsBuf);
  return true;
}

void handleWs()
{
  if (millis() - wsLastLiveTime > WS_LIVE_INTERVAL)
  {
    #ifdef ESP8266
    ws.cleanupClients(3);
    #else
    ws.cleanupClients();
    #endif
    bool success = true;
    if (wsLiveClientId) success = sendLiveLedsWs(wsLiveClientId);
    wsLastLiveTime = millis();
    if (!success) wsLastLiveTime -= 20; //try again in 20ms if failed due to non-empty WS queue
  }
}

#else
void handleWs() {}
void sendDataWs(AsyncWebSocketClient * client) {}
#endif