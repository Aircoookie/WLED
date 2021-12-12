#ifndef FSEQRECORD_H
#define FSEQRECORD_H

#include "wled.h"

#if defined(WLED_USE_SD) || defined(WLED_USE_SD_SPI)
  #define USED_STORAGE_FILESYSTEMS "SD, LittleFS"
  #ifdef WLED_USE_SD_SPI
    #ifndef WLED_USE_SD
      #define WLED_USE_SD
    #endif

    #include <SPI.h>
    #include <SD.h>

    #ifndef WLED_PIN_SCK
      #define WLED_PIN_SCK SCK
    #endif
    #ifndef WLED_PIN_MISO
      #define WLED_PIN_MISO MISO
    #endif
    #ifndef WLED_PIN_MOSI
      #define WLED_PIN_MOSI MOSI
    #endif
    #ifndef WLED_PIN_SS
      #define WLED_PIN_SS SS
    #endif
    #define WLED_SD SD
  #else
    #include "SD_MMC.h"
    #define WLED_SD SD_MMC
  #endif
#else
  #define USED_STORAGE_FILESYSTEMS "LittleFS"
#endif

// infinite loop of animation
#ifndef RECORDING_REPEAT_LOOP
  #define RECORDING_REPEAT_LOOP -1
#endif

// Default repeat count, when not specified by preset (-1=loop, 0=play once, 2=repeat two times)
#ifndef RECORDING_REPEAT_DEFAULT
  #define RECORDING_REPEAT_DEFAULT 0
#endif

class FSEQFile {
  struct file_header_t
  {
    uint8_t  identifier[4]; // Always PSEQ (older encodings may contain FSEQ)
    uint16_t channel_data_offset; // Byte index of the channel data portion of the file
    uint8_t  minor_version; // Normally 0x00, optionally 0x01 is required to enable support for Extended Compression Blocks (see xLights@e33c065)
    uint8_t  major_version; // Currently 0x02
    uint16_t header_length; // Address of first variable, length of the header (32 bytes) + Compression Block Count * length of a Compression Block (8 bytes) + Sparse Range Count * length of a Sparse Range (12 bytes)
    uint32_t channel_count; // Channel count per frame
    uint32_t frame_count;   // Number of frames
    uint8_t  step_time;     // Timing interval in milliseconds
    uint8_t  flags;         // Unused by the fpp & xLights implementations
  };

public:
  static void handlePlayRecording();
  static void loadRecording(const char* filepath, uint16_t startLed, uint16_t stopLed);

private:
  FSEQFile() {};

  // --- CONSTANTS ---
  static const int V1FSEQ_MINOR_VERSION = 0;
  static const int V1FSEQ_MAJOR_VERSION = 1;
  static const int V2FSEQ_MINOR_VERSION = 0;
  static const int V2FSEQ_MAJOR_VERSION = 2;
  static const int FSEQ_DEFAULT_STEP_TIME = 50;

  // --- Recording playback related ---
  static File     recordingFile;
  static uint8_t  colorChannels;
  static int32_t  recordingRepeats;
  static uint32_t now;
  static uint32_t next_time;
  static uint16_t playbackLedStart; // first led to play animation on
  static uint16_t playbackLedStop; // led after the last led to play animation on
  static uint32_t frame; // current frame
  static uint16_t buffer_size; // data buffer size for file read operations (1 byte buffer == ~4 fps, 3 byte buffer == ~20 fps, etc...)
  static file_header_t file_header;

  // --- File reading functions ---
  static inline uint32_t readUInt32();
  static inline uint32_t readUInt24();
  static inline uint16_t readUInt16();
  static inline uint8_t readUInt8();

  static bool fileOnSD(const char* filepath);
  static bool fileOnFS(const char* filepath);

  // --- Common functions ---
  static void printHeaderInfo();
  static void processFrameData();
  static void clearLastPlayback();
  static bool stopBecauseAtTheEnd();
  static void playNextRecordingFrame();
};

#endif