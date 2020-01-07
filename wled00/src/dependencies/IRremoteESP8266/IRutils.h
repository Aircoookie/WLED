#ifndef IRUTILS_H_
#define IRUTILS_H_

// Copyright 2017 David Conran

#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#ifndef ARDUINO
#include <string>
#endif
#include "IRremoteESP8266.h"
#include "IRrecv.h"

const uint8_t kNibbleSize = 4;
const uint8_t kLowNibble = 0;
const uint8_t kHighNibble = 4;
const uint8_t kModeBitsSize = 3;
uint64_t reverseBits(uint64_t input, uint16_t nbits);
void serialPrintUint64(uint64_t input, uint8_t base = 10);
bool hasACState(const decode_type_t protocol);
uint16_t getCorrectedRawLength(const decode_results * const results);
uint16_t *resultToRawArray(const decode_results * const decode);
uint8_t sumBytes(const uint8_t * const start, const uint16_t length,
                 const uint8_t init = 0);
uint8_t xorBytes(const uint8_t * const start, const uint16_t length,
                 const uint8_t init = 0);
uint16_t countBits(const uint8_t * const start, const uint16_t length,
                   const bool ones = true, const uint16_t init = 0);
uint16_t countBits(const uint64_t data, const uint8_t length,
                   const bool ones = true, const uint16_t init = 0);
uint64_t invertBits(const uint64_t data, const uint16_t nbits);
decode_type_t strToDecodeType(const char *str);
float celsiusToFahrenheit(const float deg);
float fahrenheitToCelsius(const float deg);
namespace irutils {
  uint8_t sumNibbles(const uint8_t * const start, const uint16_t length,
                     const uint8_t init = 0);
  uint8_t bcdToUint8(const uint8_t bcd);
  uint8_t uint8ToBcd(const uint8_t integer);
  bool getBit(const uint64_t data, const uint8_t position,
              const uint8_t size = 64);
  bool getBit(const uint8_t data, const uint8_t position);
#define GETBIT8(a, b) (a & ((uint8_t)1 << b))
#define GETBIT16(a, b) (a & ((uint16_t)1 << b))
#define GETBIT32(a, b) (a & ((uint32_t)1 << b))
#define GETBIT64(a, b) (a & ((uint64_t)1 << b))
#define GETBITS8(data, offset, size) \
    (((data) & (((uint8_t)UINT8_MAX >> (8 - (size))) << (offset))) >> (offset))
#define GETBITS16(data, offset, size) \
    (((data) & (((uint16_t)UINT16_MAX >> (16 - (size))) << (offset))) >> \
     (offset))
#define GETBITS32(data, offset, size) \
    (((data) & (((uint32_t)UINT32_MAX >> (32 - (size))) << (offset))) >> \
     (offset))
#define GETBITS64(data, offset, size) \
    (((data) & (((uint64_t)UINT64_MAX >> (64 - (size))) << (offset))) >> \
     (offset))
  uint64_t setBit(const uint64_t data, const uint8_t position,
                  const bool on = true, const uint8_t size = 64);
  uint8_t setBit(const uint8_t data, const uint8_t position,
                 const bool on = true);
  void setBit(uint8_t * const data, const uint8_t position,
              const bool on = true);
  void setBit(uint32_t * const data, const uint8_t position,
              const bool on = true);
  void setBit(uint64_t * const data, const uint8_t position,
              const bool on = true);
  void setBits(uint8_t * const dst, const uint8_t offset, const uint8_t nbits,
               const uint8_t data);
  void setBits(uint32_t * const dst, const uint8_t offset, const uint8_t nbits,
               const uint32_t data);
  void setBits(uint64_t * const dst, const uint8_t offset, const uint8_t nbits,
               const uint64_t data);
}  // namespace irutils
#endif  // IRUTILS_H_
