#include "wled.h"

#define MAX_LEDS_PER_UNIVERSE 170
#define MAX_CHANNELS_PER_UNIVERSE 512

/*
 * E1.31 handler
 */

void handleE131Packet(e131_packet_t* p, IPAddress clientIP, bool isArtnet){
  //E1.31 protocol support

  uint16_t uni = 0, dmxChannels = 0;
  uint8_t* e131_data = nullptr;
  uint8_t seq = 0, mde = REALTIME_MODE_E131;

  if (isArtnet)
  {
    uni = p->art_universe;
    dmxChannels = htons(p->art_length);
    e131_data = p->art_data;
    seq = p->art_sequence_number;
    mde = REALTIME_MODE_ARTNET;
  } else {
    uni = htons(p->universe);
    dmxChannels = htons(p->property_value_count) -1;
    e131_data = p->property_values;
    seq = p->sequence_number;
  }

  #ifdef WLED_ENABLE_DMX
  // does not act on out-of-order packets yet
  if (e131ProxyUniverse > 0 && uni == e131ProxyUniverse) {
    for (uint16_t i = 1; i <= dmxChannels; i++)
      dmx.write(i, e131_data[i]);
    dmx.update();
  }
  #endif

  // only listen for universes we're handling & allocated memory
  if (uni >= (e131Universe + E131_MAX_UNIVERSE_COUNT)) return;

  uint8_t previousUniverses = uni - e131Universe;

  if (e131SkipOutOfSequence)
    if (seq < e131LastSequenceNumber[uni-e131Universe] && seq > 20 && e131LastSequenceNumber[uni-e131Universe] < 250){
      DEBUG_PRINT("skipping E1.31 frame (last seq=");
      DEBUG_PRINT(e131LastSequenceNumber[uni-e131Universe]);
      DEBUG_PRINT(", current seq=");
      DEBUG_PRINT(seq);
      DEBUG_PRINT(", universe=");
      DEBUG_PRINT(uni);
      DEBUG_PRINTLN(")");
      return;
    }
  e131LastSequenceNumber[uni-e131Universe] = seq;

  // update status info
  realtimeIP = clientIP;
  byte wChannel = 0;

  switch (DMXMode) {
    case DMX_MODE_DISABLED:
      return;  // nothing to do
      break;

    case DMX_MODE_SINGLE_RGB:
      if (uni != e131Universe) return;
      if (dmxChannels-DMXAddress+1 < 3) return;
      realtimeLock(realtimeTimeoutMs, mde);
      if (realtimeOverride) return;
      wChannel = (dmxChannels-DMXAddress+1 > 3) ? e131_data[DMXAddress+3] : 0;
      for (uint16_t i = 0; i < ledCount; i++)
        setRealtimePixel(i, e131_data[DMXAddress+0], e131_data[DMXAddress+1], e131_data[DMXAddress+2], wChannel);
      break;

    case DMX_MODE_SINGLE_DRGB:
      if (uni != e131Universe) return;
      if (dmxChannels-DMXAddress+1 < 4) return;
      realtimeLock(realtimeTimeoutMs, mde);
      if (realtimeOverride) return;
      wChannel = (dmxChannels-DMXAddress+1 > 4) ? e131_data[DMXAddress+4] : 0;
      if (DMXOldDimmer != e131_data[DMXAddress+0]) {
        DMXOldDimmer = e131_data[DMXAddress+0];
        bri = e131_data[DMXAddress+0];
        strip.setBrightness(bri);
      }
      for (uint16_t i = 0; i < ledCount; i++)
        setRealtimePixel(i, e131_data[DMXAddress+1], e131_data[DMXAddress+2], e131_data[DMXAddress+3], wChannel);
      break;

    case DMX_MODE_EFFECT:
      if (uni != e131Universe) return;
      if (dmxChannels-DMXAddress+1 < 11) return;
      if (DMXOldDimmer != e131_data[DMXAddress+0]) {
        DMXOldDimmer = e131_data[DMXAddress+0];
        bri = e131_data[DMXAddress+0];
      }
      if (e131_data[DMXAddress+1] < MODE_COUNT)
        effectCurrent = e131_data[DMXAddress+ 1];
      effectSpeed     = e131_data[DMXAddress+ 2];  // flickers
      effectIntensity = e131_data[DMXAddress+ 3];
      effectPalette   = e131_data[DMXAddress+ 4];
      col[0]          = e131_data[DMXAddress+ 5];
      col[1]          = e131_data[DMXAddress+ 6];
      col[2]          = e131_data[DMXAddress+ 7];
      colSec[0]       = e131_data[DMXAddress+ 8];
      colSec[1]       = e131_data[DMXAddress+ 9];
      colSec[2]       = e131_data[DMXAddress+10];
      if (dmxChannels-DMXAddress+1 > 11)
      {
        col[3]        = e131_data[DMXAddress+11]; //white
        colSec[3]     = e131_data[DMXAddress+12];
      }
      transitionDelayTemp = 0;                        // act fast
      colorUpdated(NOTIFIER_CALL_MODE_NOTIFICATION);  // don't send UDP
      return;                                         // don't activate realtime live mode
      break;

    case DMX_MODE_MULTIPLE_DRGB:
    case DMX_MODE_MULTIPLE_RGB:
      {
        realtimeLock(realtimeTimeoutMs, mde);
        if (realtimeOverride) return;
        uint16_t previousLeds, dmxOffset;
        if (previousUniverses == 0) {
          if (dmxChannels-DMXAddress < 1) return;
          dmxOffset = DMXAddress;
          previousLeds = 0;
          // First DMX address is dimmer in DMX_MODE_MULTIPLE_DRGB mode.
          if (DMXMode == DMX_MODE_MULTIPLE_DRGB) {
            strip.setBrightness(e131_data[dmxOffset++]);
          }
        } else {
          // All subsequent universes start at the first channel.
          dmxOffset = isArtnet ? 0 : 1;
          uint16_t ledsInFirstUniverse = (MAX_CHANNELS_PER_UNIVERSE - DMXAddress) / 3;
          previousLeds = ledsInFirstUniverse + (previousUniverses - 1) * MAX_LEDS_PER_UNIVERSE;
        }
        uint16_t ledsTotal = previousLeds + (dmxChannels - dmxOffset) / 3;
        for (uint16_t i = previousLeds; i < ledsTotal; i++) {
          setRealtimePixel(i, e131_data[dmxOffset++], e131_data[dmxOffset++], e131_data[dmxOffset++], 0);
        }
        break;
      }
    default:
      DEBUG_PRINTLN("unknown E1.31 DMX mode");
      return;  // nothing to do
      break;
  }

  e131NewData = true;
}
