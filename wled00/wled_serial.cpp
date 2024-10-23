#include "wled.h"

enum class AwaProtocol {
    HEADER_A,
    HEADER_w,
    HEADER_a,
    HEADER_HI,
    HEADER_LO,
    HEADER_CRC,
    DATA_RED,
    DATA_GREEN,
    DATA_BLUE,
    FLETCHER1,
    FLETCHER2
};

// static data buffer for the loop
#define MAX_BUFFER 2048

void handleSerial()
{
    static uint8_t     buffer[MAX_BUFFER];
    static AwaProtocol state = AwaProtocol::HEADER_A;
    static uint8_t     CRC = 0;
    static uint16_t    count = 0;
    static uint16_t    currentPixel = 0;
    static uint16_t    fletcher1 = 0;
    static uint16_t    fletcher2 = 0;
    static uint8_t     inputColorR = 0;
    static uint8_t     inputColorG = 0;
    static uint8_t     inputColorB = 0;

    uint16_t bufferPointer = 0;
    uint16_t internalIndex = min(Serial.available(), MAX_BUFFER);

    if (internalIndex > 0)
        internalIndex = Serial.readBytes(buffer, internalIndex);
        
    yield();

    while (bufferPointer < internalIndex)
    {
        byte input = buffer[bufferPointer++];
        switch (state)
        {
        case AwaProtocol::HEADER_A:
            if (input == 'A') state = AwaProtocol::HEADER_w;
            break;

        case AwaProtocol::HEADER_w:
            if (input == 'w') state = AwaProtocol::HEADER_a;
            else              state = AwaProtocol::HEADER_A;
            break;

        case AwaProtocol::HEADER_a:
            if (input == 'a') state = AwaProtocol::HEADER_HI;
            else              state = AwaProtocol::HEADER_A;
            break;

        case AwaProtocol::HEADER_HI:
            currentPixel = 0;
            count = input * 0x100;
            CRC = input;
            fletcher1 = 0;
            fletcher2 = 0;
            state = AwaProtocol::HEADER_LO;
            break;

        case AwaProtocol::HEADER_LO:
            count += input;
            CRC = CRC ^ input ^ 0x55;
            state = AwaProtocol::HEADER_CRC;
            break;

        case AwaProtocol::HEADER_CRC:
            if (CRC == input)
            {               
                state = AwaProtocol::DATA_RED;
            }
            else
                state = AwaProtocol::HEADER_A;
            break;

        case AwaProtocol::DATA_RED:
            inputColorR = input;
            fletcher1 = (fletcher1 + (uint16_t)input) % 255;
            fletcher2 = (fletcher2 + fletcher1) % 255;

            state = AwaProtocol::DATA_GREEN;
            break;

        case AwaProtocol::DATA_GREEN:
            inputColorG = input;            
            fletcher1 = (fletcher1 + (uint16_t)input) % 255;
            fletcher2 = (fletcher2 + fletcher1) % 255;

            state = AwaProtocol::DATA_BLUE;
            break;

        case AwaProtocol::DATA_BLUE:
            inputColorB = input;              
            fletcher1 = (fletcher1 + (uint16_t)input) % 255;
            fletcher2 = (fletcher2 + fletcher1) % 255;

            if (!realtimeOverride) setRealtimePixel(currentPixel++, inputColorR, inputColorG, inputColorB, 0);            

            if (count-- > 0) state = AwaProtocol::DATA_RED;
            else state = AwaProtocol::FLETCHER1;
            break;

        case AwaProtocol::FLETCHER1:
            if (input != fletcher1) state = AwaProtocol::HEADER_A;
            else state = AwaProtocol::FLETCHER2;
            break;

        case AwaProtocol::FLETCHER2:
            if (input == fletcher2) 
            {
				realtimeLock(realtimeTimeoutMs, REALTIME_MODE_ADALIGHT);

				if (!realtimeOverride) strip.show();
            }
            state = AwaProtocol::HEADER_A;
            break;
        }
    }
}



/*
 * Adalight and TPM2 handler
 */
/*
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
};*/

uint16_t currentBaud = 20000; //default baudrate 2000000 (divided by 100)
bool continuousSendLED = false;
uint32_t lastUpdate = 0;

void updateBaudRate(uint32_t rate){
  uint16_t rate100 = rate/100;
  if (rate100 == currentBaud || rate100 < 96) return;
  currentBaud = rate100;

  if (!pinManager.isPinAllocated(hardwareTX) || pinManager.getPinOwner(hardwareTX) == PinOwner::DebugOut){
    Serial.print(F("Baud is now ")); Serial.println(rate);
  }

  Serial.flush();
  Serial.begin(rate);
}
  
/*void handleSerial()
{
  if (pinManager.isPinAllocated(hardwareRX)) return;
  if (!Serial) return;              // arduino docs: `if (Serial)` indicates whether or not the USB CDC serial connection is open. For all non-USB CDC ports, this will always return true

  #ifdef WLED_ENABLE_ADALIGHT
  static auto state = AdaState::Header_A;
  static uint16_t count = 0;
  static uint16_t pixel = 0;
  static byte check = 0x00;
  static byte red   = 0x00;
  static byte green = 0x00;

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

        } else if (next == 0xB0) {updateBaudRate( 115200);
        } else if (next == 0xB1) {updateBaudRate( 230400);
        } else if (next == 0xB2) {updateBaudRate( 460800);
        } else if (next == 0xB3) {updateBaudRate( 500000);
        } else if (next == 0xB4) {updateBaudRate( 576000);
        } else if (next == 0xB5) {updateBaudRate( 921600);
        } else if (next == 0xB6) {updateBaudRate(1000000);
        } else if (next == 0xB7) {updateBaudRate(1500000);

        } else if (next == 'l') {sendJSON(); // Send LED data as JSON Array
        } else if (next == 'L') {sendBytes(); // Send LED data as TPM2 Data Packet

        } else if (next == 'o') {continuousSendLED = false; // Disable Continuous Serial Streaming
        } else if (next == 'O') {continuousSendLED = true; // Enable Continuous Serial Streaming

        } else if (next == '{') { //JSON API
          bool verboseResponse = false;
          if (!requestJSONBufferLock(16)) return;
          Serial.setTimeout(100);
          DeserializationError error = deserializeJson(doc, Serial);
          if (error) {
            releaseJSONBufferLock();
            return;
          }
          verboseResponse = deserializeState(doc.as<JsonObject>());
          //only send response if TX pin is unused for other purposes
          if (verboseResponse && (!pinManager.isPinAllocated(hardwareTX) || pinManager.getPinOwner(hardwareTX) == PinOwner::DebugOut)) {
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
          realtimeLock(realtimeTimeoutMs, REALTIME_MODE_ADALIGHT);

          if (!realtimeOverride) strip.show();
          state = AdaState::Header_A;
        }
        break;
    }

    // All other received bytes will disable Continuous Serial Streaming
    if (continuousSendLED && next != 'O'){
      continuousSendLED = false;
      }

    Serial.read(); //discard the byte
  }
  #endif

  // If Continuous Serial Streaming is enabled, send new LED data as bytes
  if (continuousSendLED && (lastUpdate != strip.getLastShow())){
    sendBytes();
    lastUpdate = strip.getLastShow();
  }
}
*/