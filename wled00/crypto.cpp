#include <Crypto.h>
#include "wled.h"

#define HMAC_KEY_SIZE 32

void print_byte_array(const byte* arr, size_t len) {
  for (size_t i = 0; i < len; i++) {
    Serial.print(arr[i], HEX);
  }
  Serial.println();
}

void hmac_sign(const char* message, const char* psk, byte* signature) {
  SHA256HMAC hmac((const byte*)psk, strlen(psk));
  hmac.doUpdate(message, strlen(message));
  hmac.doFinal(signature);
}

bool hmac_verify(const char* message, const char* psk, const byte* signature) {
  byte sig_calculated[SHA256HMAC_SIZE];
  hmac_sign(message, psk, sig_calculated);
  if (memcmp(sig_calculated, signature, SHA256HMAC_SIZE) != 0) {
    DEBUG_PRINTLN(F("HMAC verification failed!"));
    Serial.print(F("Expected: "));
    print_byte_array(signature, SHA256HMAC_SIZE);
    Serial.print(F("Calculated: "));
    print_byte_array(sig_calculated, SHA256HMAC_SIZE);
    return false;
  }
  Serial.println(F("HMAC verification successful!"));
  return true;
}

bool verify_json_hmac(JsonObject root) {
  JsonObject msg = root["msg"];
  if (!msg) {
    Serial.println(F("No message object found in JSON."));
    return false;
  }
  const char *sig = msg["sig"];
  if (sig == nullptr) {
    Serial.println(F("No signature found in JSON."));
    return false;
  }
  
}

bool hmac_test() {
  Serial.println(F("Testing HMAC..."));
  unsigned long start = millis();
  char message[] = "Hello, World!";
  char psk[] = "tokyo";
  byte signature[SHA256HMAC_SIZE];
  hmac_sign(message, psk, signature);
  Serial.print(F("Took "));
  Serial.print(millis() - start);
  Serial.println(F("ms to sign message."));
  Serial.print(F("Signature: "));
  print_byte_array(signature, SHA256HMAC_SIZE);
  start = millis();
  bool result = hmac_verify(message, psk, signature);
  Serial.print(F("Took "));
  Serial.print(millis() - start);
  Serial.println(F("ms to verify signature."));
  return result;
}