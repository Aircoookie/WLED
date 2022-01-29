#include "wled.h"

/*
 * Adalight and TPM2 handler
 */

enum class AdaState {
  Header_A,
  Header_d,
  Header_a,
  Header_CountHi,
  Header_CountLo,
  Header_CountCheck,
  Data_Red,
  Data_Green,
  Data_Blue,
  TPM2_Header_Type,
  TPM2_Header_CountHi,
  TPM2_Header_CountLo,
};

void update_baud_rate(int rate){
  if (!pinManager.isPinAllocated(1)){
    Serial.print("ATTENTION! Baud rate is changing to "); Serial.println(rate);
    delay(100);
    Serial.end();
    Serial.begin(rate);    
    }
  }
  
void handleSerial()
{
  if (pinManager.isPinAllocated(3)) return;
  
  #ifdef WLED_ENABLE_ADALIGHT
  static auto state = AdaState::Header_A;
  static uint16_t count = 0;
  static uint16_t pixel = 0;
  static byte check = 0x00;
  static byte red   = 0x00;
  static byte green = 0x00;

  uint16_t nBytes = 0;
  
  while (Serial.available() > 0)
  {
    yield();
    byte next = Serial.peek();
    switch (state) {
      case AdaState::Header_A:
        if (next == 'A') state = AdaState::Header_d;
        else if (next == 0xC9) { //TPM2 start byte
          state = AdaState::TPM2_Header_Type;
        }
        else if (next == 'I') {
          handleImprovPacket();
          return;
        } else if (next == 'v') {
          Serial.print("WLED"); Serial.write(' '); Serial.println(VERSION);
     
        } else if ( next == 0xB0 ){ update_baud_rate( 115200 );
        } else if ( next == 0xB1 ){ update_baud_rate( 230400 );
        } else if ( next == 0xB2 ){ update_baud_rate( 460800 );
        } else if ( next == 0xB3 ){ update_baud_rate( 500000 );
        } else if ( next == 0xB4 ){ update_baud_rate( 576000 );
        } else if ( next == 0xB5 ){ update_baud_rate( 921600 );
        } else if ( next == 0xB6 ){ update_baud_rate( 1000000 );
        } else if ( next == 0xB7 ){ update_baud_rate( 1500000 );
        
        } else if (next == 'l'){ // LED Data return in JSON blob. Slow, but easy to use on the other end.
          if (!pinManager.isPinAllocated(1)){
            uint16_t used = strip.getLengthTotal();
            Serial.print("[");
            for (uint16_t i=0; i<used; i+=1){
              Serial.print(strip.getPixelColor(i));
              if (i != used-1) {Serial.print(",");}}
            Serial.println("]");}
            
        } else if (next == 'L'){ // LED Data returned as bytes. Faster, and slightly less easy to use on the other end.
          if (!pinManager.isPinAllocated(1)){
            // first byte sent back denotes number of color bytes. 0x00 RGB, 0x01 RGBW, 0x02 ??? etc
            if (strip.isRgbw){ // alternate idea, merge the white channel down into RGB like recent websocket update. or perhaps 0x02 should be merged white chanel
              Serial.write(0x01);
              nBytes = 4;}
            else{
              Serial.write(0x00);
              nBytes = 3;}
            uint16_t used = strip.getLengthTotal();
            for (uint16_t i=0; i<used; i+=1){
              uint32_t thing = strip.getPixelColor(i);
              Serial.write((byte *) &thing, nBytes);}
            Serial.println();}
            
        } else if (next == '{') { //JSON API
          bool verboseResponse = false;
          #ifdef WLED_USE_DYNAMIC_JSON
          DynamicJsonDocument doc(JSON_BUFFER_SIZE);
          #else
          if (!requestJSONBufferLock(16)) return;
          #endif
          Serial.setTimeout(100);
          DeserializationError error = deserializeJson(doc, Serial);
          if (error) {
            releaseJSONBufferLock();
            return;
          }
          verboseResponse = deserializeState(doc.as<JsonObject>());
          //only send response if TX pin is unused for other purposes
          if (verboseResponse && !pinManager.isPinAllocated(1)) {
            doc.clear();
            JsonObject state = doc.createNestedObject("state");
            serializeState(state);
            JsonObject info  = doc.createNestedObject("info");
            serializeInfo(info);

            serializeJson(doc, Serial);
            Serial.println();
          }
          releaseJSONBufferLock();
        }
        break;
      case AdaState::Header_d:
        if (next == 'd') state = AdaState::Header_a;
        else             state = AdaState::Header_A;
        break;
      case AdaState::Header_a:
        if (next == 'a') state = AdaState::Header_CountHi;
        else             state = AdaState::Header_A;
        break;
      case AdaState::Header_CountHi:
        pixel = 0;
        count = next * 0x100;
        check = next;
        state = AdaState::Header_CountLo;
        break;
      case AdaState::Header_CountLo:
        count += next + 1;
        check = check ^ next ^ 0x55;
        state = AdaState::Header_CountCheck;
        break;
      case AdaState::Header_CountCheck:
        if (check == next) state = AdaState::Data_Red;
        else               state = AdaState::Header_A;
        break;
      case AdaState::TPM2_Header_Type:
        state = AdaState::Header_A; //(unsupported) TPM2 command or invalid type
        if (next == 0xDA) state = AdaState::TPM2_Header_CountHi; //TPM2 data
        else if (next == 0xAA) Serial.write(0xAC); //TPM2 ping
        break;
      case AdaState::TPM2_Header_CountHi:
        pixel = 0;
        count = (next * 0x100) /3;
        state = AdaState::TPM2_Header_CountLo;
        break;
      case AdaState::TPM2_Header_CountLo:
        count += next /3;
        state = AdaState::Data_Red;
        break;
      case AdaState::Data_Red:
        red   = next;
        state = AdaState::Data_Green;
        break;
      case AdaState::Data_Green:
        green = next;
        state = AdaState::Data_Blue;
        break;
      case AdaState::Data_Blue:
        byte blue  = next;
        if (!realtimeOverride) setRealtimePixel(pixel++, red, green, blue, 0);
        if (--count > 0) state = AdaState::Data_Red;
        else {
          if (!realtimeMode && bri == 0) strip.setBrightness(briLast);
          realtimeLock(realtimeTimeoutMs, REALTIME_MODE_ADALIGHT);

          if (!realtimeOverride) strip.show();
          state = AdaState::Header_A;
        }
        break;
    }
    Serial.read(); //discard the byte
  }
  #endif
}
