#include "wled.h"

#define ESP_NOW_STATE_UNINIT       0
#define ESP_NOW_STATE_ON           1
#define ESP_NOW_STATE_ERROR        2

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

#ifdef WLED_DISABLE_ESPNOW
void handleRemote(){}
#else

typedef struct message_structure {
  uint8_t program;      
  uint8_t seq[4];       
  uint8_t byte5 = 32;   
  uint8_t button;       
  uint8_t byte8 = 1; 
  uint8_t byte9 = 100; 
  uint8_t byte10;  
  uint8_t byte11;  
  uint8_t byte12; 
  uint8_t byte13; 
} message_structure;

static int esp_now_state = ESP_NOW_STATE_UNINIT;
static uint32_t last_seq = UINT32_MAX;
static int brightnessBeforeNightMode = NIGHT_MODE_DEACTIVATED;
static message_structure incoming;

static const byte brightnessSteps[] = {
  6, 9, 14, 22, 33, 50, 75, 113, 170, 255
};
static const size_t numBrightnessSteps = sizeof(brightnessSteps) / sizeof(uint8_t);

static bool nightModeActive() {
  return brightnessBeforeNightMode != NIGHT_MODE_DEACTIVATED;
}

static void activateNightMode() {
  brightnessBeforeNightMode = bri;
  bri = NIGHT_MODE_BRIGHTNESS;
}

static bool resetNightMode() {
  if (!nightModeActive()) {
    return false;
  }
  bri = brightnessBeforeNightMode;
  brightnessBeforeNightMode = NIGHT_MODE_DEACTIVATED;
  return true;
}

static void brightnessUp() {
  if (nightModeActive()) { return; }
  for (uint8_t index = 0; index < numBrightnessSteps; ++index) {
    if (brightnessSteps[index] > bri) {
      bri = brightnessSteps[index];
      break;
    }
  }
}

static void brightnessDown() {
  if (nightModeActive()) { return; }
  for (int index = numBrightnessSteps - 1; index >= 0; --index) {
    if (brightnessSteps[index] < bri) {
      bri = brightnessSteps[index];
      break;
    }
  }
}

static void setOn() {
  if (resetNightMode()) {
    stateUpdated(CALL_MODE_BUTTON);
  }
  if (!bri) {
    toggleOnOff(); 
  }
}

static void setOff() {
  if (resetNightMode()) {
    stateUpdated(CALL_MODE_BUTTON);
  }
  if (bri) {
    toggleOnOff(); 
  }
}

static void presetWithFallback(uint8_t presetID, uint8_t effectID, uint8_t paletteID) {
  resetNightMode();
  unloadPlaylist();
  applyPresetWithFallback(presetID, CALL_MODE_BUTTON_PRESET, effectID, paletteID);
}

bool parsePayload(const char* jsonStr) {
  // Use WLED's built-in JSON handling functions
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, jsonStr);
  if (error) {
    return false;
  }
  JsonObject root = doc.as<JsonObject>();
  return deserializeState(root, CALL_MODE_BUTTON);
}

// Callback function that will be executed when data is received
#ifdef ESP8266
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
#else
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
#endif

  sprintf (last_signal_src, "%02x%02x%02x%02x%02x%02x",
    mac [0], mac [1], mac [2], mac [3], mac [4], mac [5]);

  if (strcmp(last_signal_src, linked_remote) != 0) {
    DEBUG_PRINT(F("ESP Now Message Received from Unlinked Sender: "));
    DEBUG_PRINTLN(last_signal_src);
    return;
  }

  if (len != sizeof(incoming)) {
    DEBUG_PRINT(F("Unknown incoming ESP Now message received of length "));
    DEBUG_PRINTLN(len);
    
    char jsonStr[len + 1];
    memcpy(jsonStr, incomingData, len);
    jsonStr[len] = '\0';

    if (!parsePayload(jsonStr)) {
      DEBUG_PRINTLN(F("Failed to parse JSON"));
      return;
    }

    stateUpdated(CALL_MODE_BUTTON);
    return;
  }

  memcpy(&(incoming.program), incomingData, sizeof(incoming));
  uint32_t cur_seq = incoming.seq[0] | (incoming.seq[1] << 8) | (incoming.seq[2] << 16) | (incoming.seq[3] << 24);

  if (cur_seq == last_seq) {
    return;
  }

  DEBUG_PRINT(F("Incoming ESP Now Packet["));
  DEBUG_PRINT(cur_seq);
  DEBUG_PRINT(F("] from sender["));
  DEBUG_PRINT(last_signal_src);
  DEBUG_PRINT(F("] button: "));
  DEBUG_PRINTLN(incoming.button);
  switch (incoming.button) {
    case WIZMOTE_BUTTON_ON             : setOn();                                         stateUpdated(CALL_MODE_BUTTON); break;
    case WIZMOTE_BUTTON_OFF            : setOff();                                        stateUpdated(CALL_MODE_BUTTON); break;
    case WIZMOTE_BUTTON_ONE            : presetWithFallback(1, FX_MODE_STATIC,        0); resetNightMode(); break;
    case WIZMOTE_BUTTON_TWO            : presetWithFallback(2, FX_MODE_BREATH,        0); resetNightMode(); break;
    case WIZMOTE_BUTTON_THREE          : presetWithFallback(3, FX_MODE_FIRE_FLICKER,  0); resetNightMode(); break;
    case WIZMOTE_BUTTON_FOUR           : presetWithFallback(4, FX_MODE_RAINBOW,       0); resetNightMode(); break;
    case WIZMOTE_BUTTON_NIGHT          : activateNightMode();                             stateUpdated(CALL_MODE_BUTTON); break;
    case WIZMOTE_BUTTON_BRIGHT_UP      : brightnessUp();                                  stateUpdated(CALL_MODE_BUTTON); break;
    case WIZMOTE_BUTTON_BRIGHT_DOWN    : brightnessDown();                                stateUpdated(CALL_MODE_BUTTON); break;
    default: break;
  }

  last_seq = cur_seq;
}

void handleRemote() {
  if (enable_espnow_remote) {
    if ((esp_now_state == ESP_NOW_STATE_UNINIT) && (interfacesInited || apActive)) {
      DEBUG_PRINTLN(F("Initializing ESP_NOW listener"));
      // Init ESP-NOW
      if (esp_now_init() != 0) {
        DEBUG_PRINTLN(F("Error initializing ESP-NOW"));
        esp_now_state = ESP_NOW_STATE_ERROR;
      }

      #ifdef ESP8266
      esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
      #endif
      
      esp_now_register_recv_cb(OnDataRecv);
      esp_now_state = ESP_NOW_STATE_ON;
    }
  } else {
    if (esp_now_state == ESP_NOW_STATE_ON) {
      DEBUG_PRINTLN(F("Disabling ESP-NOW Remote Listener"));
      if (esp_now_deinit() != 0) {
        DEBUG_PRINTLN(F("Error de-initializing ESP-NOW"));
      }
      esp_now_state = ESP_NOW_STATE_UNINIT;
    } else if (esp_now_state == ESP_NOW_STATE_ERROR) {
      //Clear any error states (allows retrying by cycling)
      esp_now_state = ESP_NOW_STATE_UNINIT;
    }
  }
}

#endif
