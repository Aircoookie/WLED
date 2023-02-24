#pragma once

#include <nvs.h>
#include <nvs_flash.h>
#include <sodium/crypto_sign_ed25519.h>

#include "HKAccessory.h"
#include "HKTLV.h"
#include "HKSRP.h"

#define MAX_CLIENTS 8
#define HK_ACCESSORY_ID_STR_SIZE 18
#define HK_IOS_ID_LEN 64

typedef enum {
  pairState_M0 = 0,
  pairState_M1 = 1,
  pairState_M2 = 2,
  pairState_M3 = 3,
  pairState_M4 = 4,
  pairState_M5 = 5,
  pairState_M6 = 6
} PairState;

typedef enum {
  tagError_Unknown = 0x01,
  tagError_Authentication = 0x02,
  tagError_Backoff = 0x03,
  tagError_MaxPeers = 0x04,
  tagError_MaxTries = 0x05,
  tagError_Unavailable = 0x06,
  tagError_Busy = 0x07
} TagError;

struct pairing_info {
  bool allocated = false;
  bool is_admin = false;
  char device_pair_id[HK_IOS_ID_LEN];
  uint8_t device_ltpk[crypto_sign_ed25519_PUBLICKEYBYTES];
};

struct accessory_keys {
  uint8_t ltpk[crypto_sign_ed25519_PUBLICKEYBYTES];
  uint8_t ltsk[crypto_sign_ed25519_SECRETKEYBYTES];
};

class HKStore final {
private:
  uint8_t acc_id[6];

  // NVS Store
  nvs_handle hk_store_handle;

  // Pairings
  static const int MAX_PAIRINGS = 16;
  pairing_info pairings[MAX_PAIRINGS];
  PairState pair_status = pairState_M0;
  pairing_info * get_free_pairing();

  // We will only need one accessory for WLED, which is the primary.
  HKAccessory * accessory = nullptr;
  HAPCharacteristic * get_characteristic(int iid);

  HKStore();

public:
  // Attributes
  bool refresh_connections = false;
  int ignore_connection = -1;
  // These keys are generated in the first call to pair-verify and used in the second call to pair-verify so must persist for a short period
  HKSRP srp;                                    // manages public and private salt of accessory


  ~HKStore() = default;
  void operator=(const HKStore &) = delete;
  HKStore &operator=(HKStore &&) noexcept = default;
  static HKStore& get_instance();

  // LightBulb
  void set_lightbulb_values(bool on, uint16_t hue, uint8_t sat, uint8_t bri);
  void get_lightbulb_values(bool * on, int * hue, int * sat, int * bri);

  // Accessory
  void get_accessory_id(uint8_t * data);
  void get_serialized_accessory_id(char * accessory_id);
  int get_accessory_keys(accessory_keys * keys);

  // Pairings
  int add_pairing(const char * device_id, size_t dev_id_len, uint8_t * device_ltpk, bool is_admin);
  pairing_info * find_pairing(const char * device_id);
  bool has_admin_pairings();
  void remove_pairing(const char * pairing_id);
  void remove_pairings();
  bool set_pair_status(PairState new_state) {
    pair_status = new_state;
    return true;
  }
  PairState get_pair_status() { return pair_status; }

  // JSON
  char * serialize_accessory(int * json_len);
  char * serialize_characteristics(int * json_len, const int * ids, size_t ids_size);
  int deserialize_characteristics(const char * json_buf, size_t json_buf_len, char * output_buf);
  char * serialize_events(int * json_len);
};