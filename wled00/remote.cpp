#include "wled.h"
#ifndef WLED_DISABLE_ESPNOW

#define NIGHT_MODE_DEACTIVATED     -1
#define NIGHT_MODE_BRIGHTNESS      5

#define WIZMOTE_BUTTON_ON          1
#define WIZMOTE_BUTTON_OFF         2
#define WIZMOTE_BUTTON_NIGHT       3
#define WIZMOTE_BUTTON_ONE         16
#define WIZMOTE_BUTTON_TWO         17
#define WIZMOTE_BUTTON_THREE       18
#define WIZMOTE_BUTTON_FOUR        19
#define WIZMOTE_BUTTON_BRIGHT_UP   9
#define WIZMOTE_BUTTON_BRIGHT_DOWN 8

#define WIZ_SMART_BUTTON_ON          100
#define WIZ_SMART_BUTTON_OFF         101
#define WIZ_SMART_BUTTON_BRIGHT_UP   102
#define WIZ_SMART_BUTTON_BRIGHT_DOWN 103

// This is kind of an esoteric strucure because it's pulled from the "Wizmote"
// product spec. That remote is used as the baseline for behavior and availability
// since it's broadly commercially available and works out of the box as a drop-in
typedef struct WizMoteMessageStructure {
  uint8_t program;  // 0x91 for ON button, 0x81 for all others
  uint8_t seq[4];   // Incremetal sequence number 32 bit unsigned integer LSB first
  uint8_t byte5;    // Unknown (seen 0x20)
  uint8_t button;   // Identifies which button is being pressed
  uint8_t byte8;    // Unknown, but always 0x01
  uint8_t byte9;    // Unnkown, but always 0x64

  uint8_t byte10;   // Unknown, maybe checksum
  uint8_t byte11;   // Unknown, maybe checksum
  uint8_t byte12;   // Unknown, maybe checksum
  uint8_t byte13;   // Unknown, maybe checksum
} message_structure_t;

static uint32_t last_seq = UINT32_MAX;
static int brightnessBeforeNightMode = NIGHT_MODE_DEACTIVATED;

// Pulled from the IR Remote logic but reduced to 10 steps with a constant of 3
static const byte brightnessSteps[] = {
  6, 9, 14, 22, 33, 50, 75, 113, 170, 255
};
static const size_t numBrightnessSteps = sizeof(brightnessSteps) / sizeof(byte);

inline bool nightModeActive() {
  return brightnessBeforeNightMode != NIGHT_MODE_DEACTIVATED;
}

static void activateNightMode() {
  if (nightModeActive()) return;
  brightnessBeforeNightMode = bri;
  bri = NIGHT_MODE_BRIGHTNESS;
  stateUpdated(CALL_MODE_BUTTON);
}

static bool resetNightMode() {
  if (!nightModeActive()) return false;
  bri = brightnessBeforeNightMode;
  brightnessBeforeNightMode = NIGHT_MODE_DEACTIVATED;
  stateUpdated(CALL_MODE_BUTTON);
  return true;
}

// increment `bri` to the next `brightnessSteps` value
static void brightnessUp() {
  if (nightModeActive()) return;
  // dumb incremental search is efficient enough for so few items
  for (uint8_t index = 0; index < numBrightnessSteps; ++index) {
    if (brightnessSteps[index] > bri) {
      bri = brightnessSteps[index];
      break;
    }
  }
  stateUpdated(CALL_MODE_BUTTON);
}

// decrement `bri` to the next `brightnessSteps` value
static void brightnessDown() {
  if (nightModeActive()) return;
  // dumb incremental search is efficient enough for so few items
  for (int index = numBrightnessSteps - 1; index >= 0; --index) {
    if (brightnessSteps[index] < bri) {
      bri = brightnessSteps[index];
      break;
    }
  }
  stateUpdated(CALL_MODE_BUTTON);
}

static void setOn() {
  resetNightMode();
  if (!bri) {
    toggleOnOff();
    stateUpdated(CALL_MODE_BUTTON);
  }
}

static void setOff() {
  resetNightMode();
  if (bri) {
    toggleOnOff();
    stateUpdated(CALL_MODE_BUTTON);
  }
}

void presetWithFallback(uint8_t presetID, uint8_t effectID, uint8_t paletteID) {
  resetNightMode();
  unloadPlaylist();
  applyPresetWithFallback(presetID, CALL_MODE_BUTTON_PRESET, effectID, paletteID);
}

// this function follows the same principle as decodeIRJson()
static bool remoteJson(int button)
{
  char objKey[10];
  bool parsed = false;

  if (!requestJSONBufferLock(22)) return false;

  sprintf_P(objKey, PSTR("\"%d\":"), button);

  // attempt to read command from remote.json
  readObjectFromFile("/remote.json", objKey, pDoc);
  JsonObject fdo = pDoc->as<JsonObject>();
  if (fdo.isNull()) {
    // the received button does not exist
    if (!WLED_FS.exists("/remote.json")) errorFlag = ERR_FS_RMLOAD; //warn if file itself doesn't exist
    releaseJSONBufferLock();
    return parsed;
  }

  String cmdStr = fdo["cmd"].as<String>();
  JsonObject jsonCmdObj = fdo["cmd"]; //object

  if (jsonCmdObj.isNull())  // we could also use: fdo["cmd"].is<String>()
  {
    if (cmdStr.startsWith("!")) {
      // call limited set of C functions
      if (cmdStr.startsWith(F("!incBri"))) {
        brightnessUp();
        parsed = true;
      } else if (cmdStr.startsWith(F("!decBri"))) {
        brightnessDown();
        parsed = true;
      } else if (cmdStr.startsWith(F("!presetF"))) { //!presetFallback
        uint8_t p1 = fdo["PL"] | 1;
        uint8_t p2 = fdo["FX"] | random8(strip.getModeCount() -1);
        uint8_t p3 = fdo["FP"] | 0;
        presetWithFallback(p1, p2, p3);
        parsed = true;
      }
    } else {
      // HTTP API command
      String apireq = "win"; apireq += '&';                        // reduce flash string usage
      //if (cmdStr.indexOf("~") || fdo["rpt"]) lastValidCode = code; // repeatable action
      if (!cmdStr.startsWith(apireq)) cmdStr = apireq + cmdStr;    // if no "win&" prefix
      if (!irApplyToAllSelected && cmdStr.indexOf(F("SS="))<0) {
        char tmp[10];
        sprintf_P(tmp, PSTR("&SS=%d"), strip.getMainSegmentId());
        cmdStr += tmp;
      }
      fdo.clear();                                                 // clear JSON buffer (it is no longer needed)
      handleSet(nullptr, cmdStr, false);                           // no stateUpdated() call here
      stateUpdated(CALL_MODE_BUTTON);
      parsed = true;
    }
  } else {
    // command is JSON object (TODO: currently will not handle irApplyToAllSelected correctly)
    deserializeState(jsonCmdObj, CALL_MODE_BUTTON);
    parsed = true;
  }
  releaseJSONBufferLock();
  return parsed;
}

// Callback function that will be executed when data is received
void handleRemote(uint8_t *incomingData, size_t len) {
  message_structure_t *incoming = reinterpret_cast<message_structure_t *>(incomingData);

  if (strcmp(last_signal_src, linked_remote) != 0) {
    DEBUG_PRINT(F("ESP Now Message Received from Unlinked Sender: "));
    DEBUG_PRINTLN(last_signal_src);
    return;
  }

  if (len != sizeof(message_structure_t)) {
    DEBUG_PRINT(F("Unknown incoming ESP Now message received of length "));
    DEBUG_PRINTLN(len);
    return;
  }

  uint32_t cur_seq = incoming->seq[0] | (incoming->seq[1] << 8) | (incoming->seq[2] << 16) | (incoming->seq[3] << 24);
  if (cur_seq == last_seq) {
    return;
  }

  DEBUG_PRINT(F("Incoming ESP Now Packet ["));
  DEBUG_PRINT(cur_seq);
  DEBUG_PRINT(F("] from sender ["));
  DEBUG_PRINT(last_signal_src);
  DEBUG_PRINT(F("] button: "));
  DEBUG_PRINTLN(incoming->button);

  if (!remoteJson(incoming->button))
    switch (incoming->button) {
      case WIZMOTE_BUTTON_ON             : setOn();                                         break;
      case WIZMOTE_BUTTON_OFF            : setOff();                                        break;
      case WIZMOTE_BUTTON_ONE            : presetWithFallback(1, FX_MODE_STATIC,        0); break;
      case WIZMOTE_BUTTON_TWO            : presetWithFallback(2, FX_MODE_BREATH,        0); break;
      case WIZMOTE_BUTTON_THREE          : presetWithFallback(3, FX_MODE_FIRE_FLICKER,  0); break;
      case WIZMOTE_BUTTON_FOUR           : presetWithFallback(4, FX_MODE_RAINBOW,       0); break;
      case WIZMOTE_BUTTON_NIGHT          : activateNightMode();                             break;
      case WIZMOTE_BUTTON_BRIGHT_UP      : brightnessUp();                                  break;
      case WIZMOTE_BUTTON_BRIGHT_DOWN    : brightnessDown();                                break;
      case WIZ_SMART_BUTTON_ON           : setOn();                                         break;
      case WIZ_SMART_BUTTON_OFF          : setOff();                                        break;
      case WIZ_SMART_BUTTON_BRIGHT_UP    : brightnessUp();                                  break;
      case WIZ_SMART_BUTTON_BRIGHT_DOWN  : brightnessDown();                                break;
      default: break;
    }
  last_seq = cur_seq;
}

#else
void handleRemote(uint8_t *incomingData, size_t len) {}
#endif