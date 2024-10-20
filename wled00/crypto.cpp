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
  Serial.print(F("Calculated: "));
  printByteArray(sigCalculated, SHA256HMAC_SIZE);
  if (memcmp(sigCalculated, signature, SHA256HMAC_SIZE) != 0) {
    Serial.println(F("HMAC verification failed!"));
    Serial.print(F("Expected: "));
    printByteArray(signature, SHA256HMAC_SIZE);
    return false;
  }
  Serial.println(F("HMAC verification successful!"));
  return true;
}

#define WLED_HMAC_TEST_PW "guessihadthekeyafterall"
#define WLED_HMAC_TEST_PSK "a6f8488da62c5888d7f640276676e78da8639faf0495110b43e226b35ac37a4c"

bool verifyHmacFromJsonString0Term(byte* jsonStr, size_t len) {
  // Zero-terminate the JSON string (replace the last character, usually '}', with a null terminator temporarily)
  char lastChar = jsonStr[len-1];
  jsonStr[len-1] = '\0';
  bool result = verifyHmacFromJsonStr((const char*)jsonStr, len);
  jsonStr[len-1] = lastChar;
  return result;
}

bool verifyHmacFromJsonStr(const char* jsonStr, uint32_t maxLen) {
  // Extract the signature from the JSON string
  size_t jsonLen = strlen(jsonStr);
  Serial.print(F("Length: "));
  Serial.println(jsonLen);
  if (jsonLen > maxLen) { // memory safety
    Serial.print(F("JSON string too long!"));
    Serial.print(F(", max: "));
    Serial.println(maxLen);
    return false;
  }
  Serial.print(F("Received JSON: "));
  Serial.println(jsonStr);
  
  char* macPos = strstr(jsonStr, "\"mac\":\"");
  if (macPos == nullptr) {
    Serial.println(F("No MAC found in JSON."));
    return false;
  }
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, macPos +6);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return false;
  }
  const char* mac = doc.as<const char*>();
  if (mac == nullptr) {
    Serial.println(F("Failed MAC JSON."));
    return false;
  }
  Serial.print(F("Received MAC: "));
  Serial.println(mac);

  // extract the message object from the JSON string
  char* msgPos = strstr(jsonStr, "\"msg\":");
  char* objStart = strchr(msgPos + 6, '{');
  if (objStart == nullptr) {
    Serial.println(F("Couldn't find msg object start."));
    return false;
  }
  size_t maxObjLen = jsonLen - (objStart - jsonStr);
  Serial.print(F("Max object length: ")); Serial.println(maxObjLen);
  int32_t objDepth = 0;
  char* objEnd = nullptr;

  for (size_t i = 0; i < maxObjLen; i++) {
    Serial.write(objStart[i]);
    if (objStart[i] == '{') objDepth++;
    if (objStart[i] == '}') objDepth--;
    if (objDepth == 0) {
      Serial.print(F("Found msg object end: "));
      Serial.println(i);
      objEnd = objStart + i;
      break;
    }
    //i++;
  }
  if (objEnd == nullptr) {
    Serial.println(F("Couldn't find msg object end."));
    return false;
  }

  // Convert the MAC from hex string to byte array
  size_t len = strlen(mac) / 2; // This will drop the last character if the string has an odd length
  if (len != SHA256HMAC_SIZE) {
    Serial.println(F("Received MAC not expected size!"));
    return false;
  }
  unsigned char macByteArray[len];
  hexStringToByteArray(mac, macByteArray, len);

  // Calculate the HMAC of the message object
  return hmacVerify((const byte*)objStart, objEnd - objStart + 1, WLED_HMAC_TEST_PSK, macByteArray);
}

bool hmacTest() {
  Serial.println(F("Testing HMAC..."));
  unsigned long start = millis();
  const char message[] = "Hello, World!";
  const char psk[] = "d0c0ffeedeadbeef";
  byte mac[SHA256HMAC_SIZE];
  hmacSign((const byte*)message, strlen(message), psk, mac);
  Serial.print(F("Took "));
  Serial.print(millis() - start);
  Serial.println(F("ms to sign message."));
  Serial.print(F("MAC: "));
  printByteArray(mac, SHA256HMAC_SIZE);
  start = millis();
  bool result = hmacVerify((const byte*)message, strlen(message), psk, mac);
  Serial.print(F("Took "));
  Serial.print(millis() - start);
  Serial.println(F("ms to verify MAC."));
  return result;
}