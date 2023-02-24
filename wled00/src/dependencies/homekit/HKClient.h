#pragma once

#ifdef ARDUINO_ARCH_ESP32

#include <WiFiClient.h>
#include <sodium/crypto_sign_ed25519.h>
#include <sodium/crypto_scalarmult_curve25519.h>

#include "HKStore.h"
#include "HKTLV.h"

#define HK_CHACHA_RESUME_MSG1 "\x00\x00\x00\x00PR-Msg01"
#define HK_CHACHA_RESUME_MSG2 "\x00\x00\x00\x00PR-Msg02"
#define HK_CHACHA_VERIFY_MSG2 "\x00\x00\x00\x00PV-Msg02"
#define HK_CHACHA_VERIFY_MSG3 "\x00\x00\x00\x00PV-Msg03"
#define HK_CHACHA_SETUP_MSG5  "\x00\x00\x00\x00PS-Msg05"
#define HK_CHACHA_SETUP_MSG6  "\x00\x00\x00\x00PS-Msg06"

#define HKDF_BYTES 32U

struct pair_setup_keys {
  uint8_t session_key[32];
  uint8_t srp_shared_secret[64];
};

struct pair_verify_keys {
  uint8_t session_key[32];
  uint8_t shared_secret[32];

  uint8_t acc_curve_pk[crypto_scalarmult_curve25519_BYTES];
  uint8_t ios_curve_pk[crypto_scalarmult_curve25519_BYTES];
};

struct session_keys {
  uint8_t encrypt_key[HKDF_BYTES];
  uint8_t decrypt_key[HKDF_BYTES];
};

// This class is considered the controller.

class HKClient {
private:
  int client_index = -1;
  WiFiClient client;

  session_keys s_keys;
  pair_setup_keys ps_keys;
  pair_verify_keys pv_keys;

  int encryption_count = 0;
  int decryption_count = 0;

  pairing_info * secured_pairing;

  void send_bad_request();
  void send_unauthorized_request();
  void send_not_found_request();

  void post_pair_setup_request(const uint8_t *data, size_t len);
  void post_pair_verify_request(const uint8_t * data, size_t len);
  void get_accessories_request();
  void put_characteristics_request(const char * data, size_t len);
  void get_characteristics(int * ids, size_t ids_size);
  void post_pairings_request(const uint8_t * data, size_t data_len);

  // void identifyPostRequest(WiFiClient* request);
  // void pairingsPostRequest(WiFiClient* request);
  // void preparePutRequest(WiFiClient* request);
  // void secureMessagePostRequest(WiFiClient* request);
  // void resourcePostRequest(WiFiClient* request);
  void send_tlv_response(TLV* tlv8);

  int decrypt_message(char * http_buf, const char * encrypted, int encrypted_len);

public:
  HKClient();
  ~HKClient();

  bool payload_available();
  void process_request();

  void set_client(WiFiClient new_client, int index);
  bool client_connected();
  void disconnect_client();
  bool session_secured();
  bool session_verified();

  int send_encrypted(const char * header, const uint8_t * body = NULL, int body_len = 0);
};

#endif