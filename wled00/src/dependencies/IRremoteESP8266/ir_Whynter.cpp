// Copyright 2009 Ken Shirriff
// Copyright 2017 David Conran

// Whynter A/C ARC-110WD added by Francesco Meschia
// Whynter originally added from https://github.com/shirriff/Arduino-IRremote/

// Supports:
//   Brand: Whynter,  Model: ARC-110WD A/C

#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

// Constants

const uint16_t kWhynterTick = 50;
const uint16_t kWhynterHdrMarkTicks = 57;
const uint16_t kWhynterHdrMark = kWhynterHdrMarkTicks * kWhynterTick;
const uint16_t kWhynterHdrSpaceTicks = 57;
const uint16_t kWhynterHdrSpace = kWhynterHdrSpaceTicks * kWhynterTick;
const uint16_t kWhynterBitMarkTicks = 15;
const uint16_t kWhynterBitMark = kWhynterBitMarkTicks * kWhynterTick;
const uint16_t kWhynterOneSpaceTicks = 43;
const uint16_t kWhynterOneSpace = kWhynterOneSpaceTicks * kWhynterTick;
const uint16_t kWhynterZeroSpaceTicks = 15;
const uint16_t kWhynterZeroSpace = kWhynterZeroSpaceTicks * kWhynterTick;
const uint16_t kWhynterMinCommandLengthTicks = 2160;  // Totally made up value.
const uint32_t kWhynterMinCommandLength =
    kWhynterMinCommandLengthTicks * kWhynterTick;
const uint16_t kWhynterMinGapTicks =
    kWhynterMinCommandLengthTicks -
    (2 * (kWhynterBitMarkTicks + kWhynterZeroSpaceTicks) +
     kWhynterBits * (kWhynterBitMarkTicks + kWhynterOneSpaceTicks));
const uint16_t kWhynterMinGap = kWhynterMinGapTicks * kWhynterTick;

#if SEND_WHYNTER
// Send a Whynter message.
//
// Args:
//   data: message to be sent.
//   nbits: Nr. of bits of the message to be sent.
//   repeat: Nr. of additional times the message is to be sent.
//
// Status: STABLE
//
// Ref:
//   https://github.com/z3t0/Arduino-IRremote/blob/master/ir_Whynter.cpp
void IRsend::sendWhynter(uint64_t data, uint16_t nbits, uint16_t repeat) {
  // Set IR carrier frequency
  enableIROut(38);

  for (uint16_t i = 0; i <= repeat; i++) {
    // (Pre-)Header
    mark(kWhynterBitMark);
    space(kWhynterZeroSpace);
    sendGeneric(
        kWhynterHdrMark, kWhynterHdrSpace, kWhynterBitMark, kWhynterOneSpace,
        kWhynterBitMark, kWhynterZeroSpace, kWhynterBitMark, kWhynterMinGap,
        kWhynterMinCommandLength - (kWhynterBitMark + kWhynterZeroSpace), data,
        nbits, 38, true, 0,  // Repeats are already handled.
        50);
  }
}
#endif

#if DECODE_WHYNTER
// Decode the supplied Whynter message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   Nr. of data bits to expect.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: BETA  Strict mode is ALPHA.
//
// Ref:
//   https://github.com/z3t0/Arduino-IRremote/blob/master/ir_Whynter.cpp
bool IRrecv::decodeWhynter(decode_results *results, uint16_t nbits,
                           bool strict) {
  if (results->rawlen < 2 * nbits + 2 * kHeader + kFooter - 1)
    return false;  // We don't have enough entries to possibly match.

  // Compliance
  if (strict && nbits != kWhynterBits)
    return false;  // Incorrect nr. of bits per spec.

  uint16_t offset = kStartOffset;
  uint64_t data = 0;
  // Pre-Header
  // Sequence begins with a bit mark and a zero space.
  if (!matchMark(results->rawbuf[offset++], kWhynterBitMark)) return false;
  if (!matchSpace(results->rawbuf[offset++], kWhynterZeroSpace)) return false;
  // Match Main Header + Data + Footer
  if (!matchGeneric(results->rawbuf + offset, &data,
                    results->rawlen - offset, nbits,
                    kWhynterHdrMark, kWhynterHdrSpace,
                    kWhynterBitMark, kWhynterOneSpace,
                    kWhynterBitMark, kWhynterZeroSpace,
                    kWhynterBitMark, kWhynterMinGap, true)) return false;
  // Success
  results->decode_type = WHYNTER;
  results->bits = nbits;
  results->value = data;
  results->address = 0;
  results->command = 0;
  return true;
}
#endif
