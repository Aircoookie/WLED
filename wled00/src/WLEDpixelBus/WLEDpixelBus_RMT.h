/*-------------------------------------------------------------------------

WLEDpixelBus - RMT output driver implementation

written by Damian Schneider @dedehai 2026

-------------------------------------------------------------------------*/

#pragma once

#include "WLEDpixelBus.h"
#include "RmtHIDriver.h"  // high interrupt priority driver (self-gating, defines WPB_USE_RMTHI on supported targets)

namespace WLEDpixelBus {

//=======================================
// RMT Bus
//=======================================

class RmtBus : public PixelBus {
public:
  /**
   * Create RMT bus
   * @param pin GPIO pin
   * @param timing LED timing
   * @param order Color order
   */
  RmtBus(int8_t pin, const LedTiming& timing, uint8_t colorOrder, uint8_t numChannels, uint8_t ledType = 0);
  ~RmtBus() override;

  bool begin() override;
  void end() override;

  bool show() override;
  bool canShow() const override;
#ifdef WLED_DEBUG_BUS
  const char* getTypeStr() const override { return "RMT"; }
#endif

  // Reset the auto-allocation counter (call before re-creating buses)
  static void setExpectedChannels(uint8_t expected) { expectedChannels = (expected > 0) ? expected : 1; }
  static void resetAutoChannel() {
    allocatedCount = 0;
    currentChannelIndex = 0;
    usedBlocks = 0;
  }

private:
  int8_t _pin;
  int8_t _channel;
  bool _initialized;
  LedTiming _timing;

  // _encodeBuffer and _encodeBufferSize are in PixelBus base
  static uint8_t expectedChannels; // TODO: make none static? would save a few bytes of ram but use more heap
  static uint8_t allocatedCount;
  static uint8_t currentChannelIndex;
  static uint8_t usedBlocks;
};

} // namespace WLEDpixelBus
