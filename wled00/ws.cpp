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
        { //scope JsonDocument so it releases its buffer
        DEBUG_PRINTLN(F("WS JSON receive buffer requested."));
        #ifdef WLED_USE_DYNAMIC_JSON
          DynamicJsonDocument doc(JSON_BUFFER_SIZE);
        #else
          if (!requestJSONBufferLock()) {
            DEBUG_PRINTLN(F("ERROR: Locking JSON buffer failed!"));
            return;
          }
        #endif

          DeserializationError error = deserializeJson(doc, data, len);
          JsonObject root = doc.as<JsonObject>();
          if (error || root.isNull()) {
            releaseJSONBufferLock();
            return;
          }
          /*
          #ifdef WLED_DEBUG
            DEBUG_PRINT(F("Incoming WS: "));
            serializeJson(root,Serial);
            DEBUG_PRINTLN();
          #endif
          */
          if (root["v"] && root.size() == 1) {
            //if the received value is just "{"v":true}", send only to this client
            verboseResponse = true;
          } else if (root.containsKey("lv"))
          {
            wsLiveClientId = root["lv"] ? client->id() : 0;
          } else {
            //fileDoc = &doc; // used for applying presets (presets.cpp)
            verboseResponse = deserializeState(root);
            //fileDoc = nullptr;
            if (!interfaceUpdateCallMode) {
              //special case, only on playlist load, avoid sending twice in rapid succession
              if (millis() - lastInterfaceUpdate > 1700) verboseResponse = false;
            }
          }
          releaseJSONBufferLock();
        }
        //update if it takes longer than 300ms until next "broadcast"
        if (verboseResponse && (millis() - lastInterfaceUpdate < 1700 || !interfaceUpdateCallMode)) sendDataWs(client);
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

  { //scope JsonDocument so it releases its buffer
  DEBUG_PRINTLN(F("WS JSON send buffer requested."));
  #ifdef WLED_USE_DYNAMIC_JSON
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
  #else
    if (!requestJSONBufferLock()) {
      DEBUG_PRINTLN(F("ERROR: Locking JSON buffer failed!"));
      return;
    }
  #endif

    JsonObject state = doc.createNestedObject("state");
    serializeState(state);
    JsonObject info  = doc.createNestedObject("info");
    serializeInfo(info);
    size_t len = measureJson(doc);
    buffer = ws.makeBuffer(len);
    if (!buffer) {
      releaseJSONBufferLock();
      return; //out of memory
    }
/*
    #ifdef WLED_DEBUG
      DEBUG_PRINT(F("Outgoing WS: "));
      serializeJson(doc,Serial);
      DEBUG_PRINTLN();
    #endif
*/
    serializeJson(doc, (char *)buffer->get(), len +1);
    releaseJSONBufferLock();
  } 
  DEBUG_PRINT(F("Sending WS data "));
  if (client) {
    client->text(buffer);
    DEBUG_PRINTLN(F("to a single client."));
  } else {
    ws.textAll(buffer);
    DEBUG_PRINTLN(F("to multiple clients."));
  }
}

void handleWs()
{
  if (millis() - wsLastLiveTime > WS_LIVE_INTERVAL)
  {
    ws.cleanupClients();
    bool success = true;
    if (wsLiveClientId)
      success = serveLiveLeds(nullptr, wsLiveClientId);
    wsLastLiveTime = millis();
    if (!success) wsLastLiveTime -= 20; //try again in 20ms if failed due to non-empty WS queue
  }
}

#else
void handleWs() {}
void sendDataWs(AsyncWebSocketClient * client) {}
#endif