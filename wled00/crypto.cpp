#include <Crypto.h>
#include "wled.h"

#define HMAC_KEY_SIZE 32

#define SESSION_ID_SIZE 16
#define MAX_SESSION_IDS 8

void getNonce(byte* nonce) {
  RNG::fill(nonce, SESSION_ID_SIZE);
}

struct Nonce {
  byte sessionId[SESSION_ID_SIZE];
  uint32_t counter;
};

Nonce knownSessions[MAX_SESSION_IDS] = {};

void moveToFirst(uint32_t i) {
  if (i >= MAX_SESSION_IDS) return;

  Nonce tmp = knownSessions[i];
  for (int j = i; j > 0; j--) {
    knownSessions[j] = knownSessions[j - 1];
  }
  knownSessions[0] = tmp;
}

bool verifyNonce(const byte* sid, uint32_t counter) {
  for (int i = 0; i < MAX_SESSION_IDS; i++) {
    if (memcmp(knownSessions[i].sessionId, sid, SESSION_ID_SIZE) == 0) {
      if (counter <= knownSessions[i].counter) {
        Serial.println(F("Retransmission detected!"));
        return false;
      }
      knownSessions[i].counter = counter;
      // nonce good, move this entry to the first position of knownSessions
      moveToFirst(i);
      return true;
    }
  }
  Serial.println(F("Unknown session ID!"));
  return false;
}

void addSession(const char* sid) {
  byte sid_new[SESSION_ID_SIZE];
  RNG::fill(sid_new, SESSION_ID_SIZE);

  // first, try to find a completely unused slot
  for (int i = 0; i < MAX_SESSION_IDS; i++) {
    // this is not perfect, but it is extremely unlikely that the first 32 bit of a random session ID are all zeroes
    if ((uint32_t)(knownSessions[i].sessionId) == 0 && knownSessions[i].counter == 0) {
      memcpy(knownSessions[i].sessionId, sid, SESSION_ID_SIZE);
      moveToFirst(i);
      return;
    }
  }
  // next, find oldest slot that has counter 0 (not used before)
  // but leave the two most recent slots alone
  for (int i = MAX_SESSION_IDS - 1; i > 1; i--) {
    if (knownSessions[i].counter == 0) {
      memcpy(knownSessions[i].sessionId, sid, SESSION_ID_SIZE);
      moveToFirst(i);
      return;
    }
  }
  // if all else fails, overwrite the oldest slot
  memcpy(knownSessions[MAX_SESSION_IDS - 1].sessionId, sid, SESSION_ID_SIZE);
  moveToFirst(MAX_SESSION_IDS - 1);
}

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
  //Serial.print(F("Calculated: "));
  //printByteArray(sigCalculated, SHA256HMAC_SIZE);
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

uint8_t verifyHmacFromJsonString0Term(byte* jsonStr, size_t len) {
  // Zero-terminate the JSON string (replace the last character, usually '}', with a null terminator temporarily)
  byte lastChar = jsonStr[len-1];
  jsonStr[len-1] = '\0';
  uint8_t result = verifyHmacFromJsonStr((const char*)jsonStr, len);
  jsonStr[len-1] = lastChar;
  return result;
}

uint8_t verifyHmacFromJsonStr(const char* jsonStr, uint32_t maxLen) {
  // Extract the signature from the JSON string
  size_t jsonLen = strlen(jsonStr);
  Serial.print(F("Length: "));
  Serial.println(jsonLen);
  if (jsonLen > maxLen) { // memory safety
    Serial.print(F("JSON string too long!"));
    Serial.print(F(", max: "));
    Serial.println(maxLen);
    return ERR_HMAC_GEN;
  }
  Serial.print(F("Received JSON: "));
  Serial.println(jsonStr);
  
  const char* macPos = strstr(jsonStr, "\"mac\":\"");
  if (macPos == nullptr) {
    Serial.println(F("No MAC found in JSON."));
    return ERR_HMAC_MISS;
  }
  StaticJsonDocument<128> macDoc;
  DeserializationError error = deserializeJson(macDoc, macPos +6);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return ERR_HMAC_GEN;
  }
  const char* mac = macDoc.as<const char*>();
  if (mac == nullptr) {
    Serial.println(F("Failed MAC JSON."));
    return ERR_HMAC_GEN;
  }
  Serial.print(F("Received MAC: "));
  Serial.println(mac);

  // extract the message object from the JSON string
  char* msgPos = strstr(jsonStr, "\"msg\":");
  char* objStart = strchr(msgPos + 6, '{');
  if (objStart == nullptr) {
    Serial.println(F("Couldn't find msg object start."));
    return ERR_HMAC_GEN;
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
    return ERR_HMAC_GEN;
  }

  // get nonce (note: the nonce implementation uses "nc" for the key instead of "n" to avoid conflicts with segment names)
  const char* noncePos = strstr(objStart, "\"nc\":");
  if (noncePos == nullptr || noncePos > objEnd) {
    // note that it is critical to check that the nonce is within the "msg" object and thus authenticated
    Serial.println(F("No nonce found in msg."));
    return ERR_HMAC_GEN;
  }
  // {
  //   StaticJsonDocument<128> nonceDoc;
  //   DeserializationError error = deserializeJson(nonceDoc, noncePos +5);
  //   if (error) {
  //     Serial.print(F("deser nc failed: "));
  //     Serial.println(error.c_str());
  //     return false;
  //   }
  //   JsonObject nonceObj = nonceDoc.as<JsonObject>();
  //   if (nonceObj.isNull()) {
  //     Serial.println(F("Failed nonce JSON."));
  //     return false;
  //   }
  //   const char* sessionId = nonceObj["sid"];
  //   if (sessionId == nullptr) {
  //     Serial.println(F("No session ID found in nonce."));
  //     return false;
  //   }
  //   uint32_t counter = nonceObj["c"] | 0;
  //   if (counter == 0) {
  //     Serial.println(F("No counter found in nonce."));
  //     return false;
  //   }
  //   if (counter > UINT32_MAX - 100) {
  //     Serial.println(F("Counter too large."));
  //     return false;
  //   }
  //   byte sidBytes[SESSION_ID_SIZE];
  //   hexStringToByteArray(sessionId, sidBytes, SESSION_ID_SIZE);
  //   if (!verifyNonce(sidBytes, counter)) {
  //     return false;
  //   }
  // }

  // Convert the MAC from hex string to byte array
  size_t len = strlen(mac) / 2; // This will drop the last character if the string has an odd length
  if (len != SHA256HMAC_SIZE) {
    Serial.println(F("Received MAC not expected size!"));
    return ERR_HMAC_GEN;
  }
  unsigned char macByteArray[len];
  hexStringToByteArray(mac, macByteArray, len);

  // Calculate the HMAC of the message object
  bool hmacOk = hmacVerify((const byte*)objStart, objEnd - objStart + 1, WLED_HMAC_TEST_PSK, macByteArray);
  return hmacOk ? ERR_NONE : ERR_HMAC;
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

void printDuration(unsigned long start) {
  unsigned long end = millis();
  Serial.print(F("Took "));
  Serial.print(end - start);
  Serial.println(F(" ms."));
  yield();
}

#define HMAC_BENCH_ITERATIONS 100

void hmacBenchmark(const char* message) {
  Serial.print(F("Starting HMAC benchmark with message length:"));
  Serial.println(strlen(message));
  Serial.println(F("100 iterations signing message."));
  unsigned long start = millis();
  byte mac[SHA256HMAC_SIZE];
  for (int i = 0; i < HMAC_BENCH_ITERATIONS; i++) {
    hmacSign((const byte*)message, strlen(message), WLED_HMAC_TEST_PSK, mac);
  }
  printDuration(start);

  Serial.println(F("100 iterations verifying message."));
  start = millis();
  for (int i = 0; i < HMAC_BENCH_ITERATIONS; i++) {
    hmacVerify((const byte*)message, strlen(message), WLED_HMAC_TEST_PSK, mac);
  }
  printDuration(start);
}