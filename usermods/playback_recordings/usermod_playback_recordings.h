#pragma once

#include "wled.h"

// This usermod can play recorded animations
//
// What formats are supported (so far):
//   - *.TPM2
//
// What does it mean:
//   You can now store short recorded animations on the ESP32 in the ROM.
//   Bigger animations can be stored on a connected SD card if mod activated
//
// How to transfer the animation:
//   WLED offers a web file manager under <IP_OF_WLED>/edit here you can upload a recorded file to ROM
//   WLED offers no web interface (yet) for SD upload, use your computer to move the animation there
//
// How to create a recording:
//   You can record with tools like Jinx
//
// How to load the animation:
//   You have to specify a preset to playback this recording with an API command
//   {"playback":{"file":"/record.tpm2"}}
//
//   You can specify a preset to repeat this recording several times, e.g. 2
//   {"playback":{"file":"/record.tpm2","repeat":2}}
//
//   You can specify a preset to repeat this recording forever
//   {"playback":{"file":"/record.tpm2","repeat":true}}
//
//   You can specify a preset to playback this recording on a specific segment
//   {"playback":{"file":"/record.tpm2","seg":2}}
//   {"playback":{"file":"/record.tpm2","seg":{"id":2}}
//
// How to trigger the animation:
//   Presets can be triggered multiple interfaces e.g. via the json API, via the web interface or with a connected IR remote
//
// What next:
//   - Playback and Recording of RGBW animations, as right now only RGB recordings are supported by WLED

// PREDEFS

void file_handlePlayPlayback();
void file_loadPlayback(const char *filepath, uint16_t startLed, uint16_t stopLed);
void file_playFrame();
void tpm2_playNextPlaybackFrame();

// MACROS

// infinite loop of animation
#define PLAYBACK_REPEAT_LOOP -1

// Default repeat count, when not specified by preset (-1=loop, 0=play once, 2=repeat two times)
#define PLAYBACK_REPEAT_NEVER 0

// Set default framerate to 25 FPS
#define PLAYBACK_FRAME_DELAY 40

#ifndef USED_STORAGE_FILESYSTEMS
  #define USED_STORAGE_FILESYSTEMS "LittleFS (SD_CARD mod not active)"
#endif

//TODO: maybe add as custom RT_MODE in `const.h` and `json.cpp`
#define REALTIME_MODE_PLAYBACK REALTIME_MODE_GENERIC

enum PLAYBACK_FORMAT {
  TPM2=0,
  FSEQ,
  FORMAT_UNKNOWN,
  COUNT_PLAYBACK_FORMATS
  };

enum PLAYBACK_FORMAT currentPlaybackFormat = PLAYBACK_FORMAT::FORMAT_UNKNOWN;

int32_t playbackRepeats = PLAYBACK_REPEAT_LOOP;
uint32_t msFrameDelay   = PLAYBACK_FRAME_DELAY; // time between frames
static const String playback_formats[] = {"tpm2",/*, "fseq"*/"   "};


//      ##     ##  ######  ######## ########  ##     ##  #######  ########
//      ##     ## ##    ## ##       ##     ## ###   ### ##     ## ##     ##
//      ##     ## ##       ##       ##     ## #### #### ##     ## ##     ##
//      ##     ##  ######  ######   ########  ## ### ## ##     ## ##     ##
//      ##     ##       ## ##       ##   ##   ##     ## ##     ## ##     ##
//      ##     ## ##    ## ##       ##    ##  ##     ## ##     ## ##     ##
//       #######   ######  ######## ##     ## ##     ##  #######  ########

class PlaybackRecordings : public Usermod
{
private:
  String jsonKeyPlayback = "playback";
  String jsonKeyFilePath = "file";
  String jsonKeyPlaybackSegment = "seg";
  String jsonKeyPlaybackSegmentId = "id";
  String jsonKeyPlaybackRepeats = "repeat";
  String jsonKeyFramesPerSecond = "fps";
  String formatTpm2 = "tpm2";

public:
  static const char _name[];

  void setup()
  {
    DEBUG_PRINTF("[%s] usermod loaded (storage: %s)\n", _name, USED_STORAGE_FILESYSTEMS);
  }

  void loop()
  {
    file_handlePlayPlayback();
  }

  void readFromJsonState(JsonObject &root)
  {
    DEBUG_PRINTF("[%s] load json\n", _name);

    // check if playback keyword is contained in API-command (json)
    // when a preset is fired it's normal to receive first the preset-firing ("ps":"<nr>]","time":"<UTC>")
    // followed by the specified API-Command of this preset
    JsonVariant jsonPlaybackEntry = root[jsonKeyPlayback];
    if (!jsonPlaybackEntry.is<JsonObject>()) {
      String debugOut;
      serializeJson(root, debugOut);
      DEBUG_PRINTF("[%s] no '%s' key or wrong format: \"%s\"\n", _name, jsonKeyPlayback.c_str(), debugOut.c_str());
      return;
    }

    // check if a mandatory path to the playback file exists in the API-command
    const char *playbackPath = jsonPlaybackEntry[jsonKeyFilePath].as<const char *>();
    String pathToPlayback = playbackPath;
    if (!playbackPath)
    {
      DEBUG_PRINTF("[%s] '%s' not defined\n", _name, jsonKeyFilePath.c_str());
      return;
    }

    // retrieve the segment(id) to play the recording on
    int id = -1; // segment id
    JsonVariant jsonPlaybackSegment = jsonPlaybackEntry[jsonKeyPlaybackSegment];
    if (jsonPlaybackSegment)
    { // playback on segments
      if      (jsonPlaybackSegment.is<JsonObject>())  { id = jsonPlaybackSegment[jsonKeyPlaybackSegmentId] | -1; }
      else if (jsonPlaybackSegment.is<JsonInteger>()) { id = jsonPlaybackSegment; }
      else { DEBUG_PRINTF("[%s] '%s' either as integer or as json with 'id':'integer'\n", _name, jsonKeyPlaybackSegment.c_str());};
    }

    // retrieve the recording format from the file extension
    for(int i=0; i<PLAYBACK_FORMAT::COUNT_PLAYBACK_FORMATS; i++){
      if(pathToPlayback.endsWith(playback_formats[i])) {
        currentPlaybackFormat = (PLAYBACK_FORMAT) i;
        break;
      }
    }

    // check how often the playback should play
    playbackRepeats = PLAYBACK_REPEAT_NEVER;
    JsonVariant jsonPlaybackRepeats = jsonPlaybackEntry[jsonKeyPlaybackRepeats];
    if (jsonPlaybackRepeats) {
      if(jsonPlaybackRepeats.is<bool>()) {
        bool doesLoop = jsonPlaybackRepeats;
        DEBUG_PRINTF("[%s] repeats found as boolean: loop %d \n", _name, doesLoop);
        playbackRepeats = doesLoop ? PLAYBACK_REPEAT_LOOP : PLAYBACK_REPEAT_NEVER;
      } else if(jsonPlaybackRepeats.is<JsonInteger>()) {
        int repeatCountInJson = jsonPlaybackRepeats;
        DEBUG_PRINTF("[%s] repeats found as integer: repeat count %d\n", _name, repeatCountInJson);
        playbackRepeats = repeatCountInJson;
      } else {
        DEBUG_PRINTF("[%s] %s either as true (loops forever) or as integer to specify count\n", _name, jsonKeyPlaybackRepeats.c_str());
      }
    }

    //adjust the framerate if defined
    msFrameDelay = PLAYBACK_FRAME_DELAY;
    JsonVariant jsonPlaybackFps = jsonPlaybackEntry[jsonKeyFramesPerSecond];
    if (jsonPlaybackFps) {
      if(jsonPlaybackFps.is<JsonInteger>() || jsonPlaybackFps.is<JsonFloat>()) {
        float fps = jsonPlaybackFps;
        uint32_t newMsDelay = round(1000.f / fps);
        DEBUG_PRINTF("[%s] framerate %d -> delay between frames: %d\n", _name, fps, newMsDelay);
        msFrameDelay = newMsDelay;
      } else {
        DEBUG_PRINTF("[%s] %s either as integer or float, though delay will be in ms and integer always\n", _name, jsonKeyFramesPerSecond.c_str());
      }
    }

    // stop here if the format is unknown
    if(currentPlaybackFormat == PLAYBACK_FORMAT::FORMAT_UNKNOWN) {
      DEBUG_PRINTF("[%s] unknown format ... if you read that, you can code the format you need XD\n", _name);
      return;
    }

    // load playback to defined segment on strip (file_loadPlayback handles the different formats within (file_playFrame))
    Segment sg = strip.getSegment(id);
    file_loadPlayback(playbackPath, sg.start, sg.stop);
    DEBUG_PRINTF("[%s] start playback\n", _name);
  }

  uint16_t getId()
  {
    return USERMOD_ID_PLAYBACK_RECORDINGS;
  }
};

const char PlaybackRecordings::_name[] PROGMEM = "Playback Recordings";


//        ######## #### ##       ########
//        ##        ##  ##       ##
//        ##        ##  ##       ##
//        ######    ##  ##       ######
//        ##        ##  ##       ##
//        ##        ##  ##       ##
//        ##       #### ######## ########

// Recording format agnostic functions to load and skim thru a
// recording/playback file and control its related entities

File playbackFile;
uint16_t playbackLedStart = 0; // first led to play animation on
uint16_t playbackLedStop  = 0; // led after the last led to play animation on
uint8_t  colorData[4];
uint8_t  colorChannels    = 3;
unsigned long lastFrame   = 0;

// clear the segment used by the playback
void file_clearLastPlayback() {
  for (uint16_t i = playbackLedStart; i < playbackLedStop; i++)
  {
    // tpm2_GetNextColorData(colorData);
    setRealtimePixel(i, 0,0,0,0);
  }
}

//checks if the file is available on LittleFS
bool file_onFS(const char *filepath)
{
  return WLED_FS.exists(filepath);
}

//checks if an override was defined. If stop the playback
void file_checkRealtimeOverride()
{
  if (realtimeOverride == REALTIME_OVERRIDE_ALWAYS) {
    realtimeOverride = REALTIME_OVERRIDE_ONCE;
  } else if(realtimeOverride == REALTIME_OVERRIDE_ONCE) { 
    exitRealtime();
    playbackFile.close();
    file_clearLastPlayback();
  }
}

void file_loadPlayback(const char *filepath, uint16_t startLed, uint16_t stopLed)
{
  //close any potentially open file
  if(playbackFile.available()) {
    file_clearLastPlayback();
    playbackFile.close();
  }

  playbackLedStart = startLed;
  playbackLedStop = stopLed;

  // No start/stop defined
  if(playbackLedStart == uint16_t(-1) || playbackLedStop == uint16_t(-1)) {
    Segment sg = strip.getSegment(-1);

    playbackLedStart = sg.start;
    playbackLedStop = sg.stop;
  }

  DEBUG_PRINTF("[%s] Load animation on LED %d to %d\n", PlaybackRecordings::_name, playbackLedStart, playbackLedStop);

  #ifdef SD_ADAPTER
  if(file_onSD(filepath)){
    DEBUG_PRINTF("[%s] Read file from SD: %s\n", PlaybackRecordings::_name, filepath);
    playbackFile = SD_ADAPTER.open(filepath, "rb");
  } else
  #endif
  if(file_onFS(filepath)) {
    DEBUG_PRINTF("[%s] Read file from FS: %s\n", PlaybackRecordings::_name, filepath);
    playbackFile = WLED_FS.open(filepath, "rb");
  } else {
    DEBUG_PRINTF("[%s] File %s not found (%s)\n", PlaybackRecordings::_name, filepath, USED_STORAGE_FILESYSTEMS);
    return;
  }


  file_playFrame();
}

// skips until a specific byte comes up
void file_skipUntil(uint8_t byteToStopAt)
{
  uint8_t rb = 0;
  do { rb = playbackFile.read(); }
  while (playbackFile.available() && rb != byteToStopAt);
}

bool file_stopBecauseAtTheEnd()
{
  //If playback reached end loop or stop playback
  if (!playbackFile.available())
  {
    if (playbackRepeats == PLAYBACK_REPEAT_LOOP)
    {
      playbackFile.seek(0); // go back the beginning of the recording
    }
    else if (playbackRepeats > 0)
    {
      playbackFile.seek(0); // go back the beginning of the recording
      playbackRepeats--;
      DEBUG_PRINTF("[%s] Repeat playback again for: %d\n", PlaybackRecordings::_name, playbackRepeats);
    }
    else
    {
      DEBUG_PRINTF("[%s] Stop playback\n", PlaybackRecordings::_name);
      exitRealtime();
      playbackFile.close();
      file_clearLastPlayback();
      return true;
    }
  }

  return false;
}

void file_playFrame() {
  switch (currentPlaybackFormat)
  {
    case PLAYBACK_FORMAT::TPM2:  tpm2_playNextPlaybackFrame(); break;
    // Add case for each format
    default: break;
  }
}

void file_handlePlayPlayback()
{
  if (realtimeMode != REALTIME_MODE_PLAYBACK) return;
  if ( millis() - lastFrame < msFrameDelay)   return;

  file_playFrame();
  file_checkRealtimeOverride();
}

//      ######## ########  ##     ##  #######
//         ##    ##     ## ###   ### ##     ##
//         ##    ##     ## #### ####        ##
//         ##    ########  ## ### ##  #######
//         ##    ##        ##     ## ##
//         ##    ##        ##     ## ##
//         ##    ##        ##     ## #########

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

// --- Recording playback related ---

void tpm2_SkipUntilNextPacket()  { file_skipUntil(TPM2_START); }

void tpm2_SkipUntilEndOfPacket() { file_skipUntil(TPM2_END); }

// reads the color data for one pixel
void tpm2_GetNextColorData(uint8_t data[])
{
  data[0] = playbackFile.read();
  data[1] = playbackFile.read();
  data[2] = playbackFile.read();
  data[3] = 0; // TODO add RGBW mode to TPM2
}

// reads the next TPM2 packet length from two bytes
uint16_t tpm2_getNextPacketLength()
{
  if (!playbackFile.available()) { return 0; }
  uint8_t highbyte_size = playbackFile.read();
  uint8_t lowbyte_size = playbackFile.read();
  uint16_t size = highbyte_size << 8 | lowbyte_size;
  return size;
}

// processes a TPM2 command, could be used for defining/changing framerate/channels in a recording
// not implemented
void tpm2_processCommandData()
{
  DEBUG_PRINTF("[%s] tpm2_processCommandData: not implemented yet\n", PlaybackRecordings::_name);
  tpm2_SkipUntilNextPacket();
}

// processes a TPM2 response, not implented
void tpm2_processResponseData()
{
  DEBUG_PRINTF("[%s] tpm2_processResponseData: not implemented yet\n", PlaybackRecordings::_name);
  tpm2_SkipUntilNextPacket();
}

// processes the actual data frame (color data)
void tpm2_processFrameData()
{
  uint16_t packetLength = tpm2_getNextPacketLength(); // opt-TODO maybe stretch recording to available leds
  uint16_t lastLed = min(playbackLedStop, uint16_t(playbackLedStart + packetLength));

  for (uint16_t i = playbackLedStart; i < lastLed; i++)
  {
    tpm2_GetNextColorData(colorData);
    setRealtimePixel(i, colorData[0], colorData[1], colorData[2], colorData[3]);
  }

  tpm2_SkipUntilEndOfPacket();

  strip.show();
  // tell ui we are playing the recording right now
  realtimeLock(realtimeTimeoutMs, REALTIME_MODE_PLAYBACK);

  lastFrame = millis();
}

void tpm2_processUnknownData(uint8_t data)
{
  DEBUG_PRINTF("[%s] tpm2_processUnknownData - received: %d\n", PlaybackRecordings::_name, data);
  tpm2_SkipUntilNextPacket();
}

// scan until next frame was read (this will process commands)
void tpm2_playNextPlaybackFrame()
{
  if(file_stopBecauseAtTheEnd()) return;

  uint8_t rb = 0; // last read byte from file

  // scan to next TPM2 packet start, should be the first attempt
  do { rb = playbackFile.read(); }
  while (playbackFile.available() && rb != TPM2_START);
  if (!playbackFile.available()) { return; }

  // process everything until (including) the next frame data
  while(true)
  {
    rb = playbackFile.read();
    if     (rb == TPM2_COMMAND)    tpm2_processCommandData();
    else if(rb == TPM2_RESPONSE)   tpm2_processResponseData();
    else if(rb != TPM2_DATA_FRAME) tpm2_processUnknownData(rb);
    else {
      tpm2_processFrameData();
      break;
    }
  }
}

// prints the whole recording to the debug log
void tpm2_printWholePlayback()
{
  while (playbackFile.available())
  {
    uint8_t rb = playbackFile.read();

    switch (rb)
    {
    case TPM2_START:
      DEBUG_PRINTF("\n%02x ", rb);
      break;
    default:
    {
      DEBUG_PRINTF("%02x ", rb);
    }
    }
  }

  playbackFile.close();
}
