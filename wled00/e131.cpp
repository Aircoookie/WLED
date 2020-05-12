#include "wled.h"

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
  uint16_t possibleLEDsInCurrentUniverse;

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
        col[3]          = e131_data[DMXAddress+11]; //white
        colSec[3]       = e131_data[DMXAddress+12];
      }
      transitionDelayTemp = 0;                        // act fast
      colorUpdated(NOTIFIER_CALL_MODE_NOTIFICATION);  // don't send UDP
      return;                                         // don't activate realtime live mode
      break;

    case DMX_MODE_MULTIPLE_RGB:
      realtimeLock(realtimeTimeoutMs, mde);
      if (realtimeOverride) return;
      if (previousUniverses == 0) {
        // first universe of this fixture
        possibleLEDsInCurrentUniverse = (dmxChannels - DMXAddress + 1) / 3;
        for (uint16_t i = 0; i < ledCount; i++) {
          if (i >= possibleLEDsInCurrentUniverse) break;  // more LEDs will follow in next universe(s)
          setRealtimePixel(i, e131_data[DMXAddress+i*3+0], e131_data[DMXAddress+i*3+1], e131_data[DMXAddress+i*3+2], 0);
        }
      } else if (previousUniverses > 0 && uni < (e131Universe + E131_MAX_UNIVERSE_COUNT)) {
        // additional universe(s) of this fixture
        uint16_t numberOfLEDsInPreviousUniverses = ((512 - DMXAddress + 1) / 3);                            // first universe
        if (previousUniverses > 1) numberOfLEDsInPreviousUniverses += (512 / 3) * (previousUniverses - 1);  // extended universe(s) before current
        possibleLEDsInCurrentUniverse = dmxChannels / 3;
        for (uint16_t i = numberOfLEDsInPreviousUniverses; i < ledCount; i++) {
          uint8_t j = i - numberOfLEDsInPreviousUniverses;
          if (j >= possibleLEDsInCurrentUniverse) break;   // more LEDs will follow in next universe(s)
          setRealtimePixel(i, e131_data[j*3+1], e131_data[j*3+2], e131_data[j*3+3], 0);
        }
      }
      break;

    case DMX_MODE_MULTIPLE_DRGB:
      realtimeLock(realtimeTimeoutMs, mde);
      if (realtimeOverride) return;
      if (previousUniverses == 0) {
        // first universe of this fixture
        if (DMXOldDimmer != e131_data[DMXAddress+0]) {
          DMXOldDimmer = e131_data[DMXAddress+0];
          bri = e131_data[DMXAddress+0];
          strip.setBrightness(bri);
        }
        possibleLEDsInCurrentUniverse = (dmxChannels - DMXAddress) / 3;
        for (uint16_t i = 0; i < ledCount; i++) {
          if (i >= possibleLEDsInCurrentUniverse) break;  // more LEDs will follow in next universe(s)
          setRealtimePixel(i, e131_data[DMXAddress+i*3+1], e131_data[DMXAddress+i*3+2], e131_data[DMXAddress+i*3+3], 0);
        }
      } else if (previousUniverses > 0 && uni < (e131Universe + E131_MAX_UNIVERSE_COUNT)) {
        // additional universe(s) of this fixture
        uint16_t numberOfLEDsInPreviousUniverses = ((512 - DMXAddress + 1) / 3);                            // first universe
        if (previousUniverses > 1) numberOfLEDsInPreviousUniverses += (512 / 3) * (previousUniverses - 1);  // extended universe(s) before current
        possibleLEDsInCurrentUniverse = dmxChannels / 3;
        for (uint16_t i = numberOfLEDsInPreviousUniverses; i < ledCount; i++) {
          uint8_t j = i - numberOfLEDsInPreviousUniverses;
          if (j >= possibleLEDsInCurrentUniverse) break;   // more LEDs will follow in next universe(s)
          setRealtimePixel(i, e131_data[j*3+1], e131_data[j*3+2], e131_data[j*3+3], 0);
        }
      }
      break;

    default:
      DEBUG_PRINTLN("unknown E1.31 DMX mode");
      return;  // nothing to do
      break;
  }

  e131NewData = true;
}
