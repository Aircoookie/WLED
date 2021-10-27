#include "wled.h"

#ifdef WLED_USE_SD
#define USED_STORAGE_FILESYSTEMS "SD, LittleFS"
#include "SD_MMC.h"
#else
#define USED_STORAGE_FILESYSTEMS "LittleFS"
#endif

// This adds TPM2-file storing and playback capabilities to WLED.
// 
// What does it mean:
//   You can now store short recorded animations on the ESP32 (in the ROM: no SD required) with a connected LED stripe.
//
// How to transfer the animation:
//   WLED offers a web file manager under <IP_OF_WLED>/edit here you can upload a recorded *.TPM2 file
//
// How to create a recording:
//   You can record with tools like Jinx
//
// How to load the animation:
//   You can specify a preset to playback this recording with the following API command
//   {"tpm2":{"file":"/record.tpm2"}}
//
//   You can specify a preset to playback this recording on a specific segment
//   {"tpm2":{"file":"/record.tpm2", "seg":{"id":2}}
//   {"tpm2":{"file":"/record.tpm2", "seg":2}
//
// How to trigger the animation:
//   Presets can be triggered multiple interfaces e.g. via the json API, via the web interface or with a connected IR remote
//
// What next:
//   - Playback from SD card is the next plan, here the length of the animation is less of a problem.
//   - Playback and Recording of RGBW animations, as right now only RGB recordings are supported by WLED

// reference spec of TPM2: https://gist.github.com/jblang/89e24e2655be6c463c56
// - A packet contains any data of the TPM2 protocol, it
//     starts with `TPM2_START` and ends with `TPM2_END`
// - A frame contains the visual data (the LEDs color's) of one moment

// --- CONSTANTS ---
#define TPM2_START      0xC9
#define TPM2_DATA_FRAME 0xDA
#define TPM2_COMMAND    0xC0
#define TPM2_END        0x36
#define TPM2_RESPONSE   0xAA

// infinite loop of animation
#define RECORDING_REPEAT_LOOP -1

// Default repeat count, when not specified by preset (-1=loop, 0=play once, 2=repeat two times)
#define RECORDING_REPEAT_DEFAULT 0

// --- Recording playback related ---
File     recordingFile;
uint8_t  colorData[4];
uint8_t  colorChannels    = 3;
uint32_t msFrameDelay     = 33; // time between frames
int32_t  recordingRepeats = RECORDING_REPEAT_LOOP;
unsigned long lastFrame   = 0;
uint16_t playbackLedStart = 0; // first led to play animation on
uint16_t playbackLedStop  = 0; // led after the last led to play animation on

// skips until a specific byte comes up
void skipUntil(uint8_t byteToStopAt)
{
  uint8_t rb = 0;
  do { rb = recordingFile.read(); }
  while (recordingFile.available() && rb != byteToStopAt);
}

void skipUntilNextPacket()  { skipUntil(TPM2_START); }

void skipUntilEndOfPacket() { skipUntil(TPM2_END); }

void getNextColorData(uint8_t data[])
{
  data[0] = recordingFile.read();
  data[1] = recordingFile.read();
  data[2] = recordingFile.read();
  data[3] = 0; // TODO add RGBW mode to TPM2
}

uint16_t getNextPacketLength()
{
  if (!recordingFile.available()) { return 0; }
  uint8_t highbyte_size = recordingFile.read();
  uint8_t lowbyte_size = recordingFile.read();
  uint16_t size = highbyte_size << 8 | lowbyte_size;
  return size;
}

void processCommandData()
{
  DEBUG_PRINTLN("processCommandData: not implemented yet");
  skipUntilNextPacket();
}

void processResponseData()
{
  DEBUG_PRINTLN("processResponseData: not implemented yet");
  skipUntilNextPacket();
}

void processFrameData()
{
  uint16_t packetLength = getNextPacketLength(); // opt-TODO maybe stretch recording to available leds
  uint16_t lastLed = min(playbackLedStop, uint16_t(playbackLedStart + packetLength));

  for (uint16_t i = playbackLedStart; i < lastLed; i++)
  {
    getNextColorData(colorData);
    setRealtimePixel(i, colorData[0], colorData[1], colorData[2], colorData[3]);
  }

  skipUntilEndOfPacket();

  strip.show();
  // tell ui we are playing the recording right now
  uint8_t mode = REALTIME_MODE_TPM2RECORD;
  realtimeLock(realtimeTimeoutMs, mode);

  lastFrame = millis();
}

void processUnknownData(uint8_t data)
{
  DEBUG_PRINT("processUnknownData - received:");
  DEBUG_PRINTLN(data);
  skipUntilNextPacket();
}

void clearLastPlayback() { 

  for (uint16_t i = playbackLedStart; i < playbackLedStop; i++)
  {
    getNextColorData(colorData);
    setRealtimePixel(i, 0,0,0,0);
  }
}

bool stopBecauseAtTheEnd()
{
  //If recording reached end loop or stop playback
  if (!recordingFile.available())
  {
    if (recordingRepeats == RECORDING_REPEAT_LOOP)
    {
      recordingFile.seek(0); // go back the beginning of the recording
    }
    else if (recordingRepeats > 0)
    {
      recordingFile.seek(0); // go back the beginning of the recording
      recordingRepeats--;
      DEBUG_PRINT("Repeat recordind again for:");
      DEBUG_PRINTLN(recordingRepeats);      
    }
    else
    {      
      uint8_t mode = REALTIME_MODE_INACTIVE;
      realtimeLock(10, mode);
      recordingFile.close();
      clearLastPlayback();
      return true;
    }
  }

  return false;
}

// scan and forward until next frame was read (this will process commands)
void playNextRecordingFrame()
{
  if(stopBecauseAtTheEnd()) return;

  uint8_t rb = 0; // last read byte from file

  // scan to next TPM2 packet start, should be the first attempt
  do { rb = recordingFile.read(); } 
  while (recordingFile.available() && rb != TPM2_START);  
  if (!recordingFile.available()) { return; }

  // process everything until (including) the next frame data
  while(true)
  {
    rb = recordingFile.read();
    if     (rb == TPM2_COMMAND)    processCommandData();     
    else if(rb == TPM2_RESPONSE)   processResponseData();
    else if(rb != TPM2_DATA_FRAME) processUnknownData(rb);
    else {
      processFrameData();
      break;      
    }
  }
}

void handlePlayRecording()
{
  if (realtimeMode != REALTIME_MODE_TPM2RECORD) return;
  if ( millis() - lastFrame < msFrameDelay)     return;
  playNextRecordingFrame();
}

void printWholeRecording()
{
  while (recordingFile.available())
  {
    uint8_t rb = recordingFile.read();

    switch (rb)
    {
    case 0xC9:
      DEBUG_PRINTLN("");
      DEBUG_PRINT(rb);
      DEBUG_PRINT(" ");
      break;
    default:
    {
      DEBUG_PRINT(rb);
      DEBUG_PRINT(" ");
    }
    }
  }

  recordingFile.close();
}

#ifdef WLED_USE_SD
//checks if the file is available on SD card
bool fileOnSD(const char *filepath)
{
  if(!SD_MMC.begin()) return false; // mounting the card failed

  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE) return false; // no SD card attached  
  if(cardType == CARD_MMC || cardType == CARD_SD || cardType == CARD_SDHC)
  {
    return SD_MMC.exists(filepath);       
  } 

  return false; // unknown card type
}
#endif

//checks if the file is available on LittleFS
bool fileOnFS(const char *filepath)
{
  return WLED_FS.exists(filepath);
}

void loadRecording(const char *filepath, uint16_t startLed, uint16_t stopLed)
{  
  //close any potentially open file  
  if(recordingFile.available()) {
    clearLastPlayback();
    recordingFile.close();
  }

  playbackLedStart = startLed;
  playbackLedStop = stopLed;

  // No start/stop defined
  if(playbackLedStart == uint16_t(-1) || playbackLedStop == uint16_t(-1)) {
    WS2812FX::Segment sg = strip.getSegment(-1);
    playbackLedStart = sg.start;
    playbackLedStop = sg.stop;
  }

  DEBUG_PRINTF("TPM2 load animation on LED %d to %d\n", playbackLedStart, playbackLedStop);         

  #ifdef WLED_USE_SD
  if(fileOnSD(filepath)){
    DEBUG_PRINTF("Read file from SD: %s\n", filepath);
    recordingFile = SD_MMC.open(filepath, "rb");  
  } else 
  #endif
  if(fileOnFS(filepath)) {
    DEBUG_PRINTF("Read file from FS: %s\n", filepath);
    recordingFile = WLED_FS.open(filepath, "rb");
  } else {
    DEBUG_PRINTF("File %s not found (%s)\n", filepath, USED_STORAGE_FILESYSTEMS);
    return;
  }

  if (realtimeOverride == REALTIME_OVERRIDE_ONCE)
  {
      realtimeOverride = REALTIME_OVERRIDE_NONE;
  }

  recordingRepeats = RECORDING_REPEAT_DEFAULT;
  playNextRecordingFrame();
}
