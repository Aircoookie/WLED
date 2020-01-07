// Copyright 2009 Ken Shirriff
// Copyright 2015 Mark Szabo
// Copyright 2015 Sebastien Warin
// Copyright 2017 David Conran

#ifndef IRRECV_H_
#define IRRECV_H_

#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include <stddef.h>
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include "IRremoteESP8266.h"

// Constants
const uint16_t kHeader = 2;        // Usual nr. of header entries.
const uint16_t kFooter = 2;        // Usual nr. of footer (stop bits) entries.
const uint16_t kStartOffset = 1;   // Usual rawbuf entry to start from.
#define MS_TO_USEC(x) (x * 1000U)  // Convert milli-Seconds to micro-Seconds.
// Marks tend to be 100us too long, and spaces 100us too short
// when received due to sensor lag.
const uint16_t kMarkExcess = 50;
const uint16_t kRawBuf = 100;  // Default length of raw capture buffer
const uint64_t kRepeat = UINT64_MAX;
// Default min size of reported UNKNOWN messages.
const uint16_t kUnknownThreshold = 6;

// receiver states
const uint8_t kIdleState = 2;
const uint8_t kMarkState = 3;
const uint8_t kSpaceState = 4;
const uint8_t kStopState = 5;
const uint8_t kTolerance = 25;   // default percent tolerance in measurements.
const uint8_t kUseDefTol = 255;  // Indicate to use the class default tolerance.
const uint16_t kRawTick = 2;     // Capture tick to uSec factor.
#define RAWTICK kRawTick  // Deprecated. For legacy user code support only.
// How long (ms) before we give up wait for more data?
// Don't exceed kMaxTimeoutMs without a good reason.
// That is the capture buffers maximum value size. (UINT16_MAX / kRawTick)
// Typically messages/protocols tend to repeat around the 100ms timeframe,
// thus we should timeout before that to give us some time to try to decode
// before we need to start capturing a possible new message.
// Typically 15ms suits most applications. However, some protocols demand a
// higher value. e.g. 90ms for XMP-1 and some aircon units.
const uint8_t kTimeoutMs = 15;  // In MilliSeconds.
#define TIMEOUT_MS kTimeoutMs   // For legacy documentation.
const uint16_t kMaxTimeoutMs = kRawTick * (UINT16_MAX / MS_TO_USEC(1));

// Use FNV hash algorithm: http://isthe.com/chongo/tech/comp/fnv/#FNV-param
const uint32_t kFnvPrime32 = 16777619UL;
const uint32_t kFnvBasis32 = 2166136261UL;

// Which of the ESP32 timers to use by default. (0-3)
const uint8_t kDefaultESP32Timer = 3;

#if DECODE_AC
// Hitachi AC is the current largest state size.
const uint16_t kStateSizeMax = kHitachiAc2StateLength;
#else
// Just define something
const uint16_t kStateSizeMax = 0;
#endif

// Types
// information for the interrupt handler
typedef struct {
  uint8_t recvpin;   // pin for IR data from detector
  uint8_t rcvstate;  // state machine
  uint16_t timer;    // state timer, counts 50uS ticks.
  uint16_t bufsize;  // max. nr. of entries in the capture buffer.
  uint16_t *rawbuf;  // raw data
  // uint16_t is used for rawlen as it saves 3 bytes of iram in the interrupt
  // handler. Don't ask why, I don't know. It just does.
  uint16_t rawlen;   // counter of entries in rawbuf.
  uint8_t overflow;  // Buffer overflow indicator.
  uint8_t timeout;   // Nr. of milliSeconds before we give up.
} irparams_t;

// results from a data match
typedef struct {
  bool success;   // Was the match successful?
  uint64_t data;  // The data found.
  uint16_t used;  // How many buffer positions were used.
} match_result_t;

// Classes

// Results returned from the decoder
class decode_results {
 public:
  decode_type_t decode_type;  // NEC, SONY, RC5, UNKNOWN
  // value, address, & command are all mutually exclusive with state.
  // i.e. They MUST NOT be used at the same time as state, so we can use a union
  // structure to save us a handful of valuable bytes of memory.
  union {
    struct {
      uint64_t value;    // Decoded value
      uint32_t address;  // Decoded device address.
      uint32_t command;  // Decoded command.
    };
    uint8_t state[kStateSizeMax];  // Multi-byte results.
  };
  uint16_t bits;              // Number of bits in decoded value
  volatile uint16_t *rawbuf;  // Raw intervals in .5 us ticks
  uint16_t rawlen;            // Number of records in rawbuf.
  bool overflow;
  bool repeat;  // Is the result a repeat code?
};

// main class for receiving IR
class IRrecv {
 public:
#if defined(ESP32)
  explicit IRrecv(const uint16_t recvpin, const uint16_t bufsize = kRawBuf,
                  const uint8_t timeout = kTimeoutMs,
                  const bool save_buffer = false,
                  const uint8_t timer_num = kDefaultESP32Timer);  // Constructor
#else  // ESP32
  explicit IRrecv(const uint16_t recvpin, const uint16_t bufsize = kRawBuf,
                  const uint8_t timeout = kTimeoutMs,
                  const bool save_buffer = false);                // Constructor
#endif  // ESP32
  ~IRrecv(void);                                                  // Destructor
  void setTolerance(const uint8_t percent = kTolerance);
  uint8_t getTolerance(void);
  bool decode(decode_results *results, irparams_t *save = NULL);
  void enableIRIn(const bool pullup = false);
  void disableIRIn(void);
  void resume(void);
  uint16_t getBufSize(void);
#if DECODE_HASH
  void setUnknownThreshold(const uint16_t length);
#endif
  bool match(const uint32_t measured, const uint32_t desired,
             const uint8_t tolerance = kUseDefTol,
             const uint16_t delta = 0);
  bool matchMark(const uint32_t measured, const uint32_t desired,
                 const uint8_t tolerance = kUseDefTol,
                 const int16_t excess = kMarkExcess);
  bool matchSpace(const uint32_t measured, const uint32_t desired,
                  const uint8_t tolerance = kUseDefTol,
                  const int16_t excess = kMarkExcess);
#ifndef UNIT_TEST

 private:
#endif
  irparams_t *irparams_save;
  uint8_t _tolerance;
#if defined(ESP32)
  uint8_t _timer_num;
#endif  // defined(ESP32)
#if DECODE_HASH
  uint16_t _unknown_threshold;
#endif
  // These are called by decode
  uint8_t _validTolerance(const uint8_t percentage);
  void copyIrParams(volatile irparams_t *src, irparams_t *dst);
  uint16_t compare(const uint16_t oldval, const uint16_t newval);
  uint32_t ticksLow(const uint32_t usecs,
                    const uint8_t tolerance = kUseDefTol,
                    const uint16_t delta = 0);
  uint32_t ticksHigh(const uint32_t usecs,
                     const uint8_t tolerance = kUseDefTol,
                     const uint16_t delta = 0);
  bool matchAtLeast(const uint32_t measured, const uint32_t desired,
                    const uint8_t tolerance = kUseDefTol,
                    const uint16_t delta = 0);
  uint16_t _matchGeneric(volatile uint16_t *data_ptr,
                         uint64_t *result_bits_ptr,
                         uint8_t *result_ptr,
                         const bool use_bits,
                         const uint16_t remaining,
                         const uint16_t required,
                         const uint16_t hdrmark,
                         const uint32_t hdrspace,
                         const uint16_t onemark,
                         const uint32_t onespace,
                         const uint16_t zeromark,
                         const uint32_t zerospace,
                         const uint16_t footermark,
                         const uint32_t footerspace,
                         const bool atleast = false,
                         const uint8_t tolerance = kUseDefTol,
                         const int16_t excess = kMarkExcess,
                         const bool MSBfirst = true);
  match_result_t matchData(volatile uint16_t *data_ptr, const uint16_t nbits,
                           const uint16_t onemark, const uint32_t onespace,
                           const uint16_t zeromark, const uint32_t zerospace,
                           const uint8_t tolerance = kUseDefTol,
                           const int16_t excess = kMarkExcess,
                           const bool MSBfirst = true);
  uint16_t matchBytes(volatile uint16_t *data_ptr, uint8_t *result_ptr,
                      const uint16_t remaining, const uint16_t nbytes,
                      const uint16_t onemark, const uint32_t onespace,
                      const uint16_t zeromark, const uint32_t zerospace,
                      const uint8_t tolerance = kUseDefTol,
                      const int16_t excess = kMarkExcess,
                      const bool MSBfirst = true);
  uint16_t matchGeneric(volatile uint16_t *data_ptr,
                        uint64_t *result_ptr,
                        const uint16_t remaining, const uint16_t nbits,
                        const uint16_t hdrmark, const uint32_t hdrspace,
                        const uint16_t onemark, const uint32_t onespace,
                        const uint16_t zeromark, const uint32_t zerospace,
                        const uint16_t footermark, const uint32_t footerspace,
                        const bool atleast = false,
                        const uint8_t tolerance = kUseDefTol,
                        const int16_t excess = kMarkExcess,
                        const bool MSBfirst = true);
  uint16_t matchGeneric(volatile uint16_t *data_ptr, uint8_t *result_ptr,
                        const uint16_t remaining, const uint16_t nbits,
                        const uint16_t hdrmark, const uint32_t hdrspace,
                        const uint16_t onemark, const uint32_t onespace,
                        const uint16_t zeromark, const uint32_t zerospace,
                        const uint16_t footermark,
                        const uint32_t footerspace,
                        const bool atleast = false,
                        const uint8_t tolerance = kUseDefTol,
                        const int16_t excess = kMarkExcess,
                        const bool MSBfirst = true);
  bool decodeHash(decode_results *results);
};

#endif  // IRRECV_H_
