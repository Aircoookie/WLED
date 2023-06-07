#include "wled.h"
#include "remote_codes.h"

#ifdef WLED_DISABLE_REMOTE
void handleRemote(){}
#else

// This is kind of an esoteric strucure because it's pulled from the "Wizmote"
// product spec. That remote is used as the baseline for behavior and availability
// since it's broadly commercially available and works out of the box as a drop-in
typedef struct message_structure {
  uint8_t program;      // 0x91 for ON button, 0x81 for all others
  uint8_t seq[4];       // Incremetal sequence number 32 bit unsigned integer LSB first
  uint8_t byte5 = 32;   // Unknown
  uint8_t button;       // Identifies which button is being pressed
  uint8_t byte8 = 1;    // Unknown, but always 0x01
  uint8_t byte9 = 100;  // Unnkown, but always 0x64

  uint8_t byte10;  // Unknown, maybe checksum
  uint8_t byte11;  // Unknown, maybe checksum
  uint8_t byte12;  // Unknown, maybe checksum
  uint8_t byte13;  // Unknown, maybe checksum
} message_structure;

int esp_now_state = ESP_NOW_STATE_UNINIT;
uint32_t last_seq = -1;

int brightnessBeforeNightMode = NIGHT_MODE_DEACTIVATED;

message_structure incoming;

// Pulled from the IR Remote logic but reduced to 10 steps with a constant of 3
const byte brightnessSteps[] = {
  6, 9, 14, 22, 33, 50, 75, 113, 170, 255
};
const size_t numBrightnessSteps = sizeof(brightnessSteps) / sizeof(uint8_t);

bool nightModeActive() {
  return brightnessBeforeNightMode != NIGHT_MODE_DEACTIVATED;
}

void activateNightMode() {
  brightnessBeforeNightMode = bri;
  bri = NIGHT_MODE_BRIGHTNESS;
}

bool resetNightMode() {
  if (!nightModeActive()) {
    return false;
  }
  bri = brightnessBeforeNightMode;
  brightnessBeforeNightMode = NIGHT_MODE_DEACTIVATED;
  return true;
}

// increment `bri` to the next `brightnessSteps` value
void brightnessUp() {
  if (nightModeActive()) { return; }
  // dumb incremental search is efficient enough for so few items
  for (uint8_t index = 0; index < numBrightnessSteps; ++index) {
    if (brightnessSteps[index] > bri) {
      bri = brightnessSteps[index];
      break;
    }
  }
}

// decrement `bri` to the next `brightnessSteps` value
void brightnessDown() {
  if (nightModeActive()) { return; }
  // dumb incremental search is efficient enough for so few items
  for (int index = numBrightnessSteps - 1; index >= 0; --index) {
    if (brightnessSteps[index] < bri) {
      bri = brightnessSteps[index];
      break;
    }
  }
}

void setOn() {
  if (resetNightMode()) {
    stateUpdated(CALL_MODE_BUTTON);
  }
  if (!bri) {
    toggleOnOff(); 
  }
}

void setOff() {
  if (resetNightMode()) {
    stateUpdated(CALL_MODE_BUTTON);
  }
  if (bri) {
    toggleOnOff(); 
  }
}
 
// Callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {

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
    case WIZMOTE_BUTTON_ON             : setOn();                                     stateUpdated(CALL_MODE_BUTTON); break;
    case WIZMOTE_BUTTON_OFF            : setOff();                                    stateUpdated(CALL_MODE_BUTTON); break;
    case WIZMOTE_BUTTON_ONE            : presetFallback(1, FX_MODE_STATIC,        0); resetNightMode(); break;
    case WIZMOTE_BUTTON_TWO            : presetFallback(2, FX_MODE_BREATH,        0); resetNightMode(); break;
    case WIZMOTE_BUTTON_THREE          : presetFallback(3, FX_MODE_FIRE_FLICKER,  0); resetNightMode(); break;
    case WIZMOTE_BUTTON_FOUR           : presetFallback(4, FX_MODE_RAINBOW,       0); resetNightMode(); break;
    case WIZMOTE_BUTTON_NIGHT          : activateNightMode();                         stateUpdated(CALL_MODE_BUTTON); break;
    case WIZMOTE_BUTTON_BRIGHT_UP      : brightnessUp();                              stateUpdated(CALL_MODE_BUTTON); break;
    case WIZMOTE_BUTTON_BRIGHT_DOWN    : brightnessDown();                            stateUpdated(CALL_MODE_BUTTON); break;
    default: break;

  }

  last_seq = cur_seq;
}

void handleRemote() {
  if (enable_espnow_remote) {
    if (esp_now_state == ESP_NOW_STATE_UNINIT) {
      DEBUG_PRINTLN(F("Initializing ESP_NOW listener"));
      // Init ESP-NOW
      if (esp_now_init() != 0) {
        DEBUG_PRINTLN(F("Error initializing ESP-NOW"));
        esp_now_state = ESP_NOW_STATE_ERROR;
      }
      esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
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