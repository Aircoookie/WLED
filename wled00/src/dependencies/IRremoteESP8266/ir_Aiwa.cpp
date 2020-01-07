// Copyright 2017 David Conran

#include "IRrecv.h"
#include "IRsend.h"

//                         AAA   IIIII  W   W   AAA
//                        A   A    I    W   W  A   A
//                        AAAAA    I    W W W  AAAAA
//                        A   A    I    W W W  A   A
//                        A   A  IIIII   WWW   A   A

// Based off the RC-T501 RCU
// Added by David Conran. (Inspired by IRremoteESP8266's implementation:
//                         https://github.com/z3t0/Arduino-IRremote)
// Supports:
//   Brand: Aiwa,  Model: RC-T501 RCU

const uint16_t kAiwaRcT501PreBits = 26;
const uint16_t kAiwaRcT501PostBits = 1;
// NOTE: These are the compliment (inverted) of lirc values as
//       lirc uses a '0' for a mark, and a '1' for a space.
const uint64_t kAiwaRcT501PreData = 0x1D8113FULL;  // 26-bits
const uint64_t kAiwaRcT501PostData = 1ULL;

#if SEND_AIWA_RC_T501
// Send an Aiwa RC T501 formatted message.
//
// Args:
//   data:   The message to be sent.
//   nbits:  The number of bits of the message to be sent.
//           Typically kAiwaRcT501Bits. Max is 37 = (64 - 27)
//   repeat: The number of times the command is to be repeated.
//
// Status: BETA / Should work.
//
// Ref:
//  http://lirc.sourceforge.net/remotes/aiwa/RC-T501
void IRsend::sendAiwaRCT501(uint64_t data, uint16_t nbits, uint16_t repeat) {
  // Appears to be an extended NEC1 protocol. i.e. 42 bits instead of 32 bits.
  // So use sendNEC instead, however the twist is it has a fixed 26 bit
  // prefix, and a fixed postfix bit.
  uint64_t new_data = ((kAiwaRcT501PreData << (nbits + kAiwaRcT501PostBits)) |
                       (data << kAiwaRcT501PostBits) | kAiwaRcT501PostData);
  nbits += kAiwaRcT501PreBits + kAiwaRcT501PostBits;
  if (nbits > sizeof(new_data) * 8)
    return;  // We are overflowing. Abort, and don't send.
  sendNEC(new_data, nbits, repeat);
}
#endif

#if DECODE_AIWA_RC_T501
// Decode the supplied Aiwa RC T501 message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   The number of data bits to expect. Typically kAiwaRcT501Bits.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: BETA / Should work.
//
// Notes:
//   Aiwa RC T501 appears to be a 42 bit variant of the NEC1 protocol.
//   However, we historically (original Arduino IRremote project) treats it as
//   a 15 bit (data) protocol. So, we expect nbits to typically be 15, and we
//   will remove the prefix and postfix from the raw data, and use that as
//   the result.
//
// Ref:
//   http://www.sbprojects.com/knowledge/ir/nec.php
bool IRrecv::decodeAiwaRCT501(decode_results *results, uint16_t nbits,
                              bool strict) {
  // Compliance
  if (strict && nbits != kAiwaRcT501Bits)
    return false;  // Doesn't match our protocol defn.

  // Add on the pre & post bits to our requested bit length.
  uint16_t expected_nbits = nbits + kAiwaRcT501PreBits + kAiwaRcT501PostBits;
  uint64_t new_data;
  if (expected_nbits > sizeof(new_data) * 8)
    return false;  // We can't possibly match something that big.
  // Decode it as a much bigger (non-standard) NEC message, so we have to turn
  // off strict mode checking for NEC.
  if (!decodeNEC(results, expected_nbits, false))
    return false;  // The NEC decode had a problem, so we should too.
  uint16_t actual_bits = results->bits;
  new_data = results->value;
  if (actual_bits < expected_nbits)
    return false;  // The data we caught was undersized. Throw it back.
  if ((new_data & 0x1ULL) != kAiwaRcT501PostData)
    return false;  // The post data doesn't match, so it can't be this protocol.
  // Trim off the post data bit.
  new_data >>= kAiwaRcT501PostBits;
  actual_bits -= kAiwaRcT501PostBits;

  // Extract out our likely new value and put it back in the results.
  actual_bits -= kAiwaRcT501PreBits;
  results->value = new_data & ((1ULL << actual_bits) - 1);

  // Check the prefix data matches.
  new_data >>= actual_bits;  // Trim off the new data to expose the prefix.
  if (new_data != kAiwaRcT501PreData)  // Check the prefix.
    return false;

  // Compliance
  if (strict && results->bits != expected_nbits) return false;

  // Success
  results->decode_type = AIWA_RC_T501;
  results->bits = actual_bits;
  results->address = 0;
  results->command = 0;
  return true;
}
#endif
