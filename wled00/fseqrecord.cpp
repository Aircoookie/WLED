#include "fseqrecord.h"

// This adds FSEQ-file storing and playback capabilities to WLED.
//
// >> Credit goes to @constant-flow for the original idea and structure created for TPM2 playback! <<
// https://github.com/Aircoookie/WLED/pull/2292
// 
// What does it mean:
//   You can now store short recorded animations on the ESP32 (in the ROM: no SD required) with a connected LED stripe.
//
// How to transfer the animation:
//   WLED offers a web file manager under <IP_OF_WLED>/edit here you can upload a recorded *.FSEQ file
//
// How to create a recording:
//   You can record with tools like xLights or Vixen
//
// How to load the animation:
//   You can specify a preset to playback this recording with the following API command
//   {"fseq":{"file":"/record.fseq"}}
//
//   You can specify a preset to playback this recording on a specific segment
//   {"fseq":{"file":"/record.fseq", "seg":{"id":2}}
//   {"fseq":{"file":"/record.fseq", "seg":2}
//
// How to trigger the animation:
//   Presets can be triggered multiple interfaces e.g. via the json API, via the web interface or with a connected IR remote
//
// How to configure SD card:
//   Arduino only supports up to SDHC 32gb, and the card must be formatted using FAT32. To optimize read efficiency, pixel
//   data is parsed in "chunks". This may need to be adjusted if the default "chunk" size is either too large (out of memory),
//   or is too small (too slow).
//
//   Most devices will work with the SD_MCC library, however some devices such as LilyGO / TTGO may require the SPI interface
//   to be used instead. This can be configured by using the WLED_USE_SD_SPI parameter. SPI PIN configuration will use the
//   default PINs as defined in Arduino, but can be overridden if needed using WLED_PIN_(SCK|MISO|MOSI|SS) defines.
//
//   For example, TTGO T8 can be configured in platformio(_override).ini as follows.
//     -D WLED_USE_SD_SPI
//     -D WLED_PIN_SCK=14
//     -D WLED_PIN_MISO=2
//     -D WLED_PIN_MOSI=15
//     -D WLED_PIN_SS=13
//
// What next:
//   - Add support for compressed FSEQ files, not explored yet.
//   - Add support for "sparse ranges", did not seem to be used by xLights.
//   - Add support for complete file header, including variable length header, however not required for any scenario yet

// reference spec of FSEQ: https://github.com/Cryptkeeper/fseq-file-format
// first-party FPP file format guide: https://github.com/FalconChristmas/fpp/blob/master/docs/FSEQ_Sequence_File_Format.txt

// --- Recording playback related ---
File     FSEQFile::recordingFile;
uint8_t  FSEQFile::colorChannels = 3;
int32_t  FSEQFile::recordingRepeats = RECORDING_REPEAT_LOOP;
uint32_t FSEQFile::now = 0;
uint32_t FSEQFile::next_time = 0;
uint16_t FSEQFile::playbackLedStart = 0; // first led to play animation on
uint16_t FSEQFile::playbackLedStop = 0; // led after the last led to play animation on
uint32_t FSEQFile::frame = 0; // current frame
uint16_t FSEQFile::buffer_size = 48; // data buffer size for file read operations (1 byte buffer == ~4 fps, 3 byte buffer == ~20 fps, etc...)
FSEQFile::file_header_t FSEQFile::file_header;

// --- File reading functions ---
inline uint32_t FSEQFile::readUInt32() {
  uint32_t len = 4;
  char buffer[len];
  if (recordingFile.readBytes(buffer, len) < len) return 0;
  uint32_t u32 = (((uint32_t)buffer[0]) + (((uint32_t)buffer[1]) << 8) +
    (((uint32_t)buffer[2]) << 16) + (((uint32_t)buffer[3]) << 24));
  return u32;
}
inline uint32_t FSEQFile::readUInt24() {
  uint32_t len = 3;
  char buffer[len];
  if (recordingFile.readBytes(buffer, len) < len) return 0;
  uint32_t u24 =
    (((uint32_t)buffer[0]) + (((uint32_t)buffer[1]) << 8) + (((uint32_t)buffer[2]) << 16));
  return u24;
}
inline uint16_t FSEQFile::readUInt16() {
  uint32_t len = 2;
  char buffer[len];
  if (recordingFile.readBytes(buffer, len) < len) return 0;
  uint16_t u16 = (((uint16_t)buffer[0]) + (((uint16_t)buffer[1]) << 8));
  return u16;
}
inline uint8_t FSEQFile::readUInt8() {
  uint32_t len = 1;
  char buffer[len];
  if (recordingFile.readBytes(buffer, len) < len) return 0;
  uint8_t u8 = (((uint8_t)buffer[0]));
  return u8;
}

bool FSEQFile::fileOnSD(const char* filepath)
{
#if defined(WLED_USE_SD) || defined(WLED_USE_SD_SPI)
#ifdef WLED_USE_SD_SPI
  SPI.begin(WLED_PIN_SCK, WLED_PIN_MISO, WLED_PIN_MOSI, WLED_PIN_SS);
  if (!WLED_SD.begin(WLED_PIN_SS)) return false;
#else
  if (!WLED_SD.begin("/sdcard", true)) return false; // mounting the card failed
#endif

  uint8_t cardType = WLED_SD.cardType();
  if (cardType == CARD_NONE) return false; // no SD card attached  
  if (cardType == CARD_MMC || cardType == CARD_SD || cardType == CARD_SDHC)
  {
    return WLED_SD.exists(filepath);
  }
#endif

  return false; // unknown card type
}
bool FSEQFile::fileOnFS(const char* filepath)
{
  return WLED_FS.exists(filepath);
}

// --- Common functions ---
void FSEQFile::handlePlayRecording()
{
  now = millis();
  if (realtimeMode != REALTIME_MODE_FSEQ) return;
  if (now < next_time) return;

  playNextRecordingFrame();
}
void FSEQFile::loadRecording(const char* filepath, uint16_t startLed, uint16_t stopLed)
{
  // close any potentially open file
  if (recordingFile.available()) {
    clearLastPlayback();
    recordingFile.close();
  }

  playbackLedStart = startLed;
  playbackLedStop = stopLed;

  // No start/stop defined
  if (playbackLedStart == uint16_t(-1) || playbackLedStop == uint16_t(-1)) {
    WS2812FX::Segment sg = strip.getSegment(-1);
    playbackLedStart = sg.start;
    playbackLedStop = sg.stop;
  }

  DEBUG_PRINTF("FSEQ load animation on LED %d to %d\r\n", playbackLedStart, playbackLedStop);

#if defined(WLED_USE_SD) || defined(WLED_USE_SD_SPI)
  if (fileOnSD(filepath)) {
    DEBUG_PRINTF("Read file from SD: %s\r\n", filepath);
    recordingFile = WLED_SD.open(filepath, "rb");
  }
  else
#endif
  if (fileOnFS(filepath)) {
    DEBUG_PRINTF("Read file from FS: %s\r\n", filepath);
    recordingFile = WLED_FS.open(filepath, "rb");
  }
  else {
    DEBUG_PRINTF("File %s not found (%s)\r\n", filepath, USED_STORAGE_FILESYSTEMS);
    return;
  }

  // Parse header
  if ((uint64_t)recordingFile.available() < sizeof(file_header)) {
    DEBUG_PRINTF("Invalid file size: %d\r\n", recordingFile.available());
    recordingFile.close();
    return;
  }
  for (int i = 0; i < 4; i++) {
    file_header.identifier[i] = readUInt8();
  }
  file_header.channel_data_offset = readUInt16();
  file_header.minor_version = readUInt8();
  file_header.major_version = readUInt8();
  file_header.header_length = readUInt16();
  file_header.channel_count = readUInt32();
  file_header.frame_count = readUInt32();
  file_header.step_time = readUInt8();
  file_header.flags = readUInt8();

  // Print debug info
  printHeaderInfo();

  // Verify file format
  if (file_header.identifier[0] != 'P' || file_header.identifier[1] != 'S' || file_header.identifier[2] != 'E' || file_header.identifier[3] != 'Q') {
    DEBUG_PRINTF("Error reading FSEQ file %s header, invalid identifier\r\n", filepath);
    recordingFile.close();
    return;
  }
  if ((file_header.minor_version != V1FSEQ_MINOR_VERSION && file_header.major_version != V1FSEQ_MAJOR_VERSION) || (file_header.minor_version != V2FSEQ_MINOR_VERSION && file_header.major_version != V2FSEQ_MAJOR_VERSION)) {
    DEBUG_PRINTF("Error reading FSEQ file %s header, unknown version 0x%" PRIX8 " 0x%" PRIX8 "\r\n", filepath, file_header.minor_version, file_header.major_version);
    recordingFile.close();
    return;
  }
  if (((uint64_t)file_header.channel_count * (uint64_t)file_header.frame_count) + file_header.header_length > UINT32_MAX) {
    DEBUG_PRINTF("Error reading FSEQ file %s header, file too long (max 4gb)\r\n", filepath);
    recordingFile.close();
    return;
  }
  if (file_header.step_time < 1) {
    DEBUG_PRINTF("Invalid step time %d, using default %d instead\r\n", file_header.step_time, FSEQ_DEFAULT_STEP_TIME);
    file_header.step_time = FSEQ_DEFAULT_STEP_TIME;
  }

  if (realtimeOverride == REALTIME_OVERRIDE_ONCE) {
    realtimeOverride = REALTIME_OVERRIDE_NONE;
  }

  recordingRepeats = RECORDING_REPEAT_DEFAULT;
  playNextRecordingFrame();
}

void FSEQFile::printHeaderInfo() {
  DEBUG_PRINTLN("FSEQ file_header:");
  DEBUG_PRINT(F(" channel_data_offset = ")); DEBUG_PRINTLN(file_header.channel_data_offset);
  DEBUG_PRINT(F(" minor_version       = ")); DEBUG_PRINTLN(file_header.minor_version);
  DEBUG_PRINT(F(" major_version       = ")); DEBUG_PRINTLN(file_header.major_version);
  DEBUG_PRINT(F(" header_length       = ")); DEBUG_PRINTLN(file_header.header_length);
  DEBUG_PRINT(F(" channel_count       = ")); DEBUG_PRINTLN(file_header.channel_count);
  DEBUG_PRINT(F(" frame_count         = ")); DEBUG_PRINTLN(file_header.frame_count);
  DEBUG_PRINT(F(" step_time           = ")); DEBUG_PRINTLN(file_header.step_time);
  DEBUG_PRINT(F(" flags               = ")); DEBUG_PRINTLN(file_header.flags);
}

void FSEQFile::processFrameData()
{
  uint16_t packetLength = file_header.channel_count;
  uint16_t lastLed = min(playbackLedStop, uint16_t(playbackLedStart + (packetLength / 3)));

  // process data in "chunks" to speed up read operation
  char frame_data[buffer_size];
  CRGB* crgb = reinterpret_cast<CRGB*>(frame_data);

  uint16_t bytes_remaining = packetLength;
  uint16_t index = playbackLedStart;
  while (index < lastLed && bytes_remaining > 0) {
    uint16_t length = min(bytes_remaining, buffer_size);
    recordingFile.readBytes(frame_data, length);
    bytes_remaining -= length;

    for (uint16_t offset = 0; offset < length / 3; offset++) {
      setRealtimePixel(index, (byte)crgb[offset].r, (byte)crgb[offset].g, (byte)crgb[offset].b, 0);
      if (++index > lastLed) break; // end of string or data
    }
  }

  strip.show();

  // tell ui we are playing the recording right now
  uint8_t mode = REALTIME_MODE_FSEQ;
  realtimeLock(realtimeTimeoutMs, mode);

  next_time = now + file_header.step_time;
}

void FSEQFile::clearLastPlayback() {

  for (uint16_t i = playbackLedStart; i < playbackLedStop; i++)
  {
    setRealtimePixel(i, 0, 0, 0, 0);
  }

  frame = 0; // reset frame index
}

bool FSEQFile::stopBecauseAtTheEnd()
{
  // if recording reached end loop or stop playback
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
      DEBUG_PRINTF("Repeat recording again for: %" PRId32 "\r\n", recordingRepeats);
    }
    else
    {
      DEBUG_PRINTLN(F("Finished playing recording, disabling realtime mode"));
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
void FSEQFile::playNextRecordingFrame()
{
  if (stopBecauseAtTheEnd()) return;

  // go to next FSEQ frame offset
  uint32_t offset = file_header.channel_count;
  offset *= frame++;
  offset += file_header.header_length;

  if (!recordingFile.seek(offset)) {
    // check position, avoid false error when already at correct offset
    if (recordingFile.position() != offset) {
      DEBUG_PRINTLN(F("Failed to seek to proper offset for channel data!"));
      DEBUG_PRINT(F(" offset:    ")); DEBUG_PRINTLN(offset);
      DEBUG_PRINT(F(" position:  ")); DEBUG_PRINTLN(recordingFile.position());
      DEBUG_PRINT(F(" available: ")); DEBUG_PRINTLN(recordingFile.available());
      return;
    }
  }

  // process everything until the next frame
  processFrameData();
}