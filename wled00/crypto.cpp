#include <Crypto.h>
#include "wled.h"

#define HMAC_KEY_SIZE 32

void printByteArray(const byte* arr, size_t len) {
  for (size_t i = 0; i < len; i++) {
    Serial.print(arr[i], HEX);
  }
  Serial.println();
}

void hexStringToByteArray(const char* hexString, unsigned char* byteArray, size_t byteArraySize) {
  for (size_t i = 0; i < byteArraySize; i++) {
    char c[3] = {hexString[2 * i], hexString[2 * i + 1], '\0'};  // Get two characters
    byteArray[i] = (unsigned char)strtoul(c, NULL, 16);          // Convert to byte
  }
}

void hmacSign(const byte* message, size_t msgLen, const char* pskHex, byte* signature) {
  size_t len = strlen(pskHex) / 2; // This will drop the last character if the string has an odd length
  if (len > HMAC_KEY_SIZE) {
    Serial.println(F("PSK too long!"));
    return;
  }
  unsigned char pskByteArray[len];
  hexStringToByteArray(pskHex, pskByteArray, len);

  SHA256HMAC hmac(pskByteArray, len);
  hmac.doUpdate(message, msgLen);
  hmac.doFinal(signature);
}

bool hmacVerify(const byte* message, size_t msgLen, const char* pskHex, const byte* signature) {
  byte sigCalculated[SHA256HMAC_SIZE];
  hmacSign(message, msgLen, pskHex, sigCalculated);
  if (memcmp(sigCalculated, signature, SHA256HMAC_SIZE) != 0) {
    DEBUG_PRINTLN(F("HMAC verification failed!"));
    Serial.print(F("Expected: "));
    printByteArray(signature, SHA256HMAC_SIZE);
    Serial.print(F("Calculated: "));
    printByteArray(sigCalculated, SHA256HMAC_SIZE);
    return false;
  }
  Serial.println(F("HMAC verification successful!"));
  return true;
}

#define WLED_HMAC_TEST_PW "guessihadthekeyafterall"
#define WLED_HMAC_TEST_PSK "a6f8488da62c5888d7f640276676e78da8639faf0495110b43e226b35ac37a4c"

bool verifyHmacFromJsonStr(const char* jsonStr, uint32_t maxLen) {
  // Extract the signature from the JSON string
  size_t jsonLen = strlen(jsonStr);
  if (jsonLen > maxLen) { // memory safety
    Serial.println(F("JSON string too long!"));
    Serial.print(F("Length: "));
    Serial.print(jsonLen);
    Serial.print(F(", max: "));
    Serial.println(maxLen);
    return false;
  }
  Serial.print(F("Received JSON: "));
  Serial.println(jsonStr);
  char* sigPos = strstr(jsonStr, PSTR("\"sig\":\""));
  if (sigPos == nullptr) {
    Serial.println(F("No signature found in JSON."));
    return false;
  }
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, jsonStr +7);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return false;
  }
  const char* sig = doc.as<const char*>();
  if (sig == nullptr) {
    Serial.println(F("Failed signature JSON."));
    return false;
  }
  Serial.print(F("Received signature: "));
  Serial.println(sig);

  // extract the message object from the JSON string
  char* msgPos = strstr(jsonStr, PSTR("\"msg\":\""));
  char* objStart = strchr(msgPos + 7, '{');
  size_t maxObjLen = jsonLen - (objStart - jsonStr);
  uint32_t objDepth = 0;
  char* objEnd = nullptr;

  for (size_t i = 0; i < maxObjLen; i++) {
    if (objStart[i] == '{') objDepth++;
    if (objStart [i] == '}') objDepth--;
    if (objDepth == 0) {
      objEnd = objStart + i;
      break;
    }
    i++;
  }
  if (objEnd == nullptr) {
    Serial.println(F("Couldn't find msg object end."));
    return false;
  }

  // Convert the signature from hex string to byte array
  size_t len = strlen(sig) / 2; // This will drop the last character if the string has an odd length
  if (len != SHA256HMAC_SIZE) {
    Serial.println(F("Received sig not expected size!"));
    return false;
  }
  unsigned char sigByteArray[len];
  hexStringToByteArray(sig, sigByteArray, len);

  // Calculate the HMAC of the message object
  return hmacVerify((const byte*)objStart, objEnd - objStart + 1, WLED_HMAC_TEST_PSK, sigByteArray);
}

bool hmacTest() {
  Serial.println(F("Testing HMAC..."));
  unsigned long start = millis();
  const char message[] = "Hello, World!";
  const char psk[] = "d0c0ffeedeadbeef";
  byte signature[SHA256HMAC_SIZE];
  hmacSign((const byte*)message, strlen(message), psk, signature);
  Serial.print(F("Took "));
  Serial.print(millis() - start);
  Serial.println(F("ms to sign message."));
  Serial.print(F("Signature: "));
  printByteArray(signature, SHA256HMAC_SIZE);
  start = millis();
  bool result = hmacVerify((const byte*)message, strlen(message), psk, signature);
  Serial.print(F("Took "));
  Serial.print(millis() - start);
  Serial.println(F("ms to verify signature."));
  return result;
}