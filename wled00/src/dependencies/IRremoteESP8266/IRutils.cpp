// Copyright 2017 David Conran

#include "IRutils.h"
#ifndef UNIT_TEST
#include <Arduino.h>
#endif

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <string.h>
#include <algorithm>
#ifndef ARDUINO
#include <string>
#endif
#include "IRrecv.h"
#include "IRremoteESP8266.h"

// Reverse the order of the requested least significant nr. of bits.
// Args:
//   input: Bit pattern/integer to reverse.
//   nbits: Nr. of bits to reverse.
// Returns:
//   The reversed bit pattern.
uint64_t reverseBits(uint64_t input, uint16_t nbits) {
  if (nbits <= 1) return input;  // Reversing <= 1 bits makes no change at all.
  // Cap the nr. of bits to rotate to the max nr. of bits in the input.
  nbits = std::min(nbits, (uint16_t)(sizeof(input) * 8));
  uint64_t output = 0;
  for (uint16_t i = 0; i < nbits; i++) {
    output <<= 1;
    output |= (input & 1);
    input >>= 1;
  }
  // Merge any remaining unreversed bits back to the top of the reversed bits.
  return (input << nbits) | output;
}

// Convert a uint64_t (unsigned long long) to a string.
// Arduino String/toInt/Serial.print() can't handle printing 64 bit values.
//
// Args:
//   input: The value to print
//   base:  The output base.
// Returns:
//   A string representation of the integer.
// Note: Based on Arduino's Print::printNumber()
String uint64ToString(uint64_t input, uint8_t base) {
  String result = "";
  // prevent issues if called with base <= 1
  if (base < 2) base = 10;
  // Check we have a base that we can actually print.
  // i.e. [0-9A-Z] == 36
  if (base > 36) base = 10;

  // Reserve some string space to reduce fragmentation.
  // 16 bytes should store a uint64 in hex text which is the likely worst case.
  // 64 bytes would be the worst case (base 2).
  result.reserve(16);

  do {
    char c = input % base;
    input /= base;

    if (c < 10)
      c += '0';
    else
      c += 'A' - 10;
    result = c + result;
  } while (input);
  return result;
}

#ifdef ARDUINO
// Print a uint64_t/unsigned long long to the Serial port
// Serial.print() can't handle printing long longs. (uint64_t)
//
// Args:
//   input: The value to print
//   base: The output base.
void serialPrintUint64(uint64_t input, uint8_t base) {
  Serial.print(uint64ToString(input, base));
}
#endif

// Convert a C-style str to a decode_type_t
//
// Args:
//   str:  A C-style string containing a protocol name or number.
// Returns:
//  A decode_type_t enum.
decode_type_t strToDecodeType(const char * const str) {
  return decode_type_t::UNKNOWN;
}


// Does the given protocol use a complex state as part of the decode?
bool hasACState(const decode_type_t protocol) {
  return false;
}

// Return the corrected length of a 'raw' format array structure
// after over-large values are converted into multiple entries.
// Args:
//   results: A ptr to a decode result.
// Returns:
//   A uint16_t containing the length.
uint16_t getCorrectedRawLength(const decode_results * const results) {
  uint16_t extended_length = results->rawlen - 1;
  for (uint16_t i = 0; i < results->rawlen - 1; i++) {
    uint32_t usecs = results->rawbuf[i] * kRawTick;
    // Add two extra entries for multiple larger than UINT16_MAX it is.
    extended_length += (usecs / (UINT16_MAX + 1)) * 2;
  }
  return extended_length;
}

// Convert a decode_results into an array suitable for `sendRaw()`.
// Args:
//   decode:  A pointer to an IR decode_results structure that contains a mesg.
// Returns:
//   A pointer to a dynamically allocated uint16_t sendRaw compatible array.
// Note:
//   Result needs to be delete[]'ed/free()'ed (deallocated) after use by caller.
uint16_t* resultToRawArray(const decode_results * const decode) {
  uint16_t *result = new uint16_t[getCorrectedRawLength(decode)];
  if (result != NULL) {  // The memory was allocated successfully.
    // Convert the decode data.
    uint16_t pos = 0;
    for (uint16_t i = 1; i < decode->rawlen; i++) {
      uint32_t usecs = decode->rawbuf[i] * kRawTick;
      while (usecs > UINT16_MAX) {  // Keep truncating till it fits.
        result[pos++] = UINT16_MAX;
        result[pos++] = 0;  // A 0 in a sendRaw() array basically means skip.
        usecs -= UINT16_MAX;
      }
      result[pos++] = usecs;
    }
  }
  return result;
}

uint8_t sumBytes(const uint8_t * const start, const uint16_t length,
                 const uint8_t init) {
  uint8_t checksum = init;
  const uint8_t *ptr;
  for (ptr = start; ptr - start < length; ptr++) checksum += *ptr;
  return checksum;
}

uint8_t xorBytes(const uint8_t * const start, const uint16_t length,
                 const uint8_t init) {
  uint8_t checksum = init;
  const uint8_t *ptr;
  for (ptr = start; ptr - start < length; ptr++) checksum ^= *ptr;
  return checksum;
}

// Count the number of bits of a certain type.
// Args:
//   start: Ptr to the start of data to count bits in.
//   length: How many bytes to count.
//   ones: Count the binary 1 bits. False for counting the 0 bits.
//   init: Start the counting from this value.
// Returns:
//   Nr. of bits found.
uint16_t countBits(const uint8_t * const start, const uint16_t length,
                   const bool ones, const uint16_t init) {
  uint16_t count = init;
  for (uint16_t offset = 0; offset < length; offset++)
    for (uint8_t currentbyte = *(start + offset);
         currentbyte;
         currentbyte >>= 1)
      if (currentbyte & 1) count++;
  if (ones || length == 0)
    return count;
  else
    return (length * 8) - count;
}

// Count the number of bits of a certain type.
// Args:
//   data: The value you want bits counted for, starting from the LSB.
//   length: How many bits to count.
//   ones: Count the binary 1 bits. False for counting the 0 bits.
//   init: Start the counting from this value.
// Returns:
//   Nr. of bits found.
uint16_t countBits(const uint64_t data, const uint8_t length, const bool ones,
                   const uint16_t init) {
  uint16_t count = init;
  uint8_t bitsSoFar = length;
  for (uint64_t remainder = data; remainder && bitsSoFar;
       remainder >>= 1, bitsSoFar--)
      if (remainder & 1) count++;
  if (ones || length == 0)
    return count;
  else
    return length - count;
}

uint64_t invertBits(const uint64_t data, const uint16_t nbits) {
  // No change if we are asked to invert no bits.
  if (nbits == 0) return data;
  uint64_t result = ~data;
  // If we are asked to invert all the bits or more than we have, it's simple.
  if (nbits >= sizeof(data) * 8) return result;
  // Mask off any unwanted bits and return the result.
  return (result & ((1ULL << nbits) - 1));
}

namespace irutils {
  // Sum all the nibbles together in a series of bytes.
  // Args:
  //   start: PTR to the start of the bytes.
  //   length: Nr of bytes to sum the nibbles of.
  //   init: Starting value of the sum.
  // Returns:
  //   A uint8_t sum of all the nibbles inc the init.
  uint8_t sumNibbles(const uint8_t * const start, const uint16_t length,
                     const uint8_t init) {
    uint8_t sum = init;
    const uint8_t *ptr;
    for (ptr = start; ptr - start < length; ptr++)
      sum += (*ptr >> 4) + (*ptr & 0xF);
    return sum;
  }

  uint8_t bcdToUint8(const uint8_t bcd) {
    if (bcd > 0x99) return 255;  // Too big.
    return (bcd >> 4) * 10 + (bcd & 0xF);
  }

  uint8_t uint8ToBcd(const uint8_t integer) {
    if (integer > 99) return 255;  // Too big.
    return ((integer / 10) << 4) + (integer % 10);
  }

  // Return the value of `position`th bit of `data`.
  // Args:
  //   data: Value to be examined.
  //   position: Nr. of the nth bit to be examined. `0` is the LSB.
  //   size: Nr. of bits in data.
  bool getBit(const uint64_t data, const uint8_t position, const uint8_t size) {
    if (position >= size) return false;  // Outside of range.
    return data & (1ULL << position);
  }

  // Return the value of `position`th bit of `data`.
  // Args:
  //   data: Value to be examined.
  //   position: Nr. of the nth bit to be examined. `0` is the LSB.
  bool getBit(const uint8_t data, const uint8_t position) {
    if (position >= 8) return false;  // Outside of range.
    return data & (1 << position);
  }

  // Return the value of `data` with the `position`th bit changed to `on`
  // Args:
  //   data: Value to be changed.
  //   position: Nr. of the bit to be changed. `0` is the LSB.
  //   on: Value to set the position'th bit to.
  //   size: Nr. of bits in data.
  uint64_t setBit(const uint64_t data, const uint8_t position, const bool on,
                  const uint8_t size) {
    if (position >= size) return data;  // Outside of range.
    uint64_t mask = 1ULL << position;
    if (on)
      return data | mask;
    else
      return data & ~mask;
  }

  // Return the value of `data` with the `position`th bit changed to `on`
  // Args:
  //   data: Value to be changed.
  //   position: Nr. of the bit to be changed. `0` is the LSB.
  //   on: Value to set the position'th bit to.
  uint8_t setBit(const uint8_t data, const uint8_t position, const bool on) {
    if (position >= 8) return data;  // Outside of range.
    uint8_t mask = 1 << position;
    if (on)
      return data | mask;
    else
      return data & ~mask;
  }

  // Change the value at the location `data_ptr` with the `position`th bit
  //   changed to `on`
  // Args:
  //   data: Ptr to the data to be changed.
  //   position: Nr. of the bit to be changed. `0` is the LSB.
  //   on: Value to set the position'th bit to.
  void setBit(uint8_t * const data, const uint8_t position, const bool on) {
    uint8_t mask = 1 << position;
    if (on)
      *data |= mask;
    else
      *data &= ~mask;
  }

  // Change the value at the location `data_ptr` with the `position`th bit
  //   changed to `on`
  // Args:
  //   data: Ptr to the data to be changed.
  //   position: Nr. of the bit to be changed. `0` is the LSB.
  //   on: Value to set the position'th bit to.
  void setBit(uint32_t * const data, const uint8_t position, const bool on) {
    uint32_t mask = (uint32_t)1 << position;
    if (on)
      *data |= mask;
    else
      *data &= ~mask;
  }

  // Change the value at the location `data_ptr` with the `position`th bit
  //   changed to `on`
  // Args:
  //   data: Ptr to the data to be changed.
  //   position: Nr. of the bit to be changed. `0` is the LSB.
  //   on: Value to set the position'th bit to.
  void setBit(uint64_t * const data, const uint8_t position, const bool on) {
    uint64_t mask = (uint64_t)1 << position;
    if (on)
      *data |= mask;
    else
      *data &= ~mask;
  }

  // Change the uint8_t pointed to by `dst` starting at the `offset`th bit
  //   and for `nbits` bits, with the contents of `data`.
  // Args:
  //   dst: Ptr to the uint8_t to be changed.
  //   offset: Nr. of bits from the Least Significant Bit to be ignored.
  //   nbits: Nr of bits of `data` to be placed into the destination uint8_t.
  //   data: Value to be placed into dst.
  void setBits(uint8_t * const dst, const uint8_t offset, const uint8_t nbits,
               const uint8_t data) {
    if (offset >= 8 || !nbits) return;  // Short circuit as it won't change.
    // Calculate the mask for the supplied value.
    uint8_t mask = UINT8_MAX >> (8 - ((nbits > 8) ? 8 : nbits));
    // Calculate the mask & clear the space for the data.
    // Clear the destination bits.
    *dst &= ~(uint8_t)(mask << offset);
    // Merge in the data.
    *dst |= ((data & mask) << offset);
  }

  // Change the uint32_t pointed to by `dst` starting at the `offset`th bit
  //   and for `nbits` bits, with the contents of `data`.
  // Args:
  //   dst: Ptr to the uint32_t to be changed.
  //   offset: Nr. of bits from the Least Significant Bit to be ignored.
  //   nbits: Nr of bits of `data` to be placed into the destination uint32_t.
  //   data: Value to be placed into dst.
  void setBits(uint32_t * const dst, const uint8_t offset, const uint8_t nbits,
               const uint32_t data) {
    if (offset >= 32 || !nbits) return;  // Short circuit as it won't change.
    // Calculate the mask for the supplied value.
    uint32_t mask = UINT32_MAX >> (32 - ((nbits > 32) ? 32 : nbits));
    // Calculate the mask & clear the space for the data.
    // Clear the destination bits.
    *dst &= ~(mask << offset);
    // Merge in the data.
    *dst |= ((data & mask) << offset);
  }

  // Change the uint64_t pointed to by `dst` starting at the `offset`th bit
  //   and for `nbits` bits, with the contents of `data`.
  // Args:
  //   dst: Ptr to the uint64_t to be changed.
  //   offset: Nr. of bits from the Least Significant Bit to be ignored.
  //   nbits: Nr of bits of `data` to be placed into the destination uint64_t.
  //   data: Value to be placed into dst.
  void setBits(uint64_t * const dst, const uint8_t offset, const uint8_t nbits,
               const uint64_t data) {
    if (offset >= 64 || !nbits) return;  // Short circuit as it won't change.
    // Calculate the mask for the supplied value.
    uint64_t mask = UINT64_MAX >> (64 - ((nbits > 64) ? 64 : nbits));
    // Calculate the mask & clear the space for the data.
    // Clear the destination bits.
    *dst &= ~(mask << offset);
    // Merge in the data.
    *dst |= ((data & mask) << offset);
  }
}  // namespace irutils
