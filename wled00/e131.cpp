#include "wled.h"

/*
 * E1.31 handler
 */

void handleE131Packet(e131_packet_t* p, IPAddress clientIP){
  //E1.31 protocol support

  uint16_t uni = htons(p->universe);
  uint8_t previousUniverses = uni - e131Universe;
  uint16_t possibleLEDsInCurrentUniverse;
  uint16_t dmxChannels = htons(p->property_value_count) -1;

  // only listen for universes we're handling & allocated memory
  if (uni >= (e131Universe + E131_MAX_UNIVERSE_COUNT)) return;

  if (e131SkipOutOfSequence)
    if (p->sequence_number < e131LastSequenceNumber[uni-e131Universe] && p->sequence_number > 20 && e131LastSequenceNumber[uni-e131Universe] < 250){
      DEBUG_PRINT("skipping E1.31 frame (last seq=");
      DEBUG_PRINT(e131LastSequenceNumber[uni-e131Universe]);
      DEBUG_PRINT(", current seq=");
      DEBUG_PRINT(p->sequence_number);
      DEBUG_PRINT(", universe=");
      DEBUG_PRINT(uni);
      DEBUG_PRINTLN(")");
      return;
    }
  e131LastSequenceNumber[uni-e131Universe] = p->sequence_number;

  // update status info
  realtimeIP = clientIP;
  
  switch (DMXMode) {
    case DMX_MODE_DISABLED:
      return;  // nothing to do
      break;

    case DMX_MODE_SINGLE_RGB:
      if (uni != e131Universe) return;
      if (dmxChannels-DMXAddress+1 < 3) return;
      arlsLock(realtimeTimeoutMs, REALTIME_MODE_E131);
      for (uint16_t i = 0; i < ledCount; i++)
        setRealtimePixel(i, p->property_values[DMXAddress+0], p->property_values[DMXAddress+1], p->property_values[DMXAddress+2], 0);
      break;

    case DMX_MODE_SINGLE_DRGB:
      if (uni != e131Universe) return;
      if (dmxChannels-DMXAddress+1 < 4) return;
      arlsLock(realtimeTimeoutMs, REALTIME_MODE_E131);
      if (DMXOldDimmer != p->property_values[DMXAddress+0]) {
        DMXOldDimmer = p->property_values[DMXAddress+0];
        bri = p->property_values[DMXAddress+0];
        strip.setBrightness(bri);
      }
      for (uint16_t i = 0; i < ledCount; i++)
        setRealtimePixel(i, p->property_values[DMXAddress+1], p->property_values[DMXAddress+2], p->property_values[DMXAddress+3], 0);
      break;

    case DMX_MODE_EFFECT:
      if (uni != e131Universe) return;
      if (dmxChannels-DMXAddress+1 < 11) return;
      if (DMXOldDimmer != p->property_values[DMXAddress+0]) {
        DMXOldDimmer = p->property_values[DMXAddress+0];
        bri = p->property_values[DMXAddress+0];
      }
      if (p->property_values[DMXAddress+1] < MODE_COUNT)
        effectCurrent = p->property_values[DMXAddress+ 1];
      effectSpeed     = p->property_values[DMXAddress+ 2];  // flickers
      effectIntensity = p->property_values[DMXAddress+ 3];
      effectPalette   = p->property_values[DMXAddress+ 4];
      col[0]          = p->property_values[DMXAddress+ 5];
      col[1]          = p->property_values[DMXAddress+ 6];
      col[2]          = p->property_values[DMXAddress+ 7];
      colSec[0]       = p->property_values[DMXAddress+ 8];
      colSec[1]       = p->property_values[DMXAddress+ 9];
      colSec[2]       = p->property_values[DMXAddress+10];
      if (dmxChannels-DMXAddress+1 > 11)
      {
        col[3]          = p->property_values[DMXAddress+11]; //white
        colSec[3]       = p->property_values[DMXAddress+12];
      }
      transitionDelayTemp = 0;                        // act fast
      colorUpdated(NOTIFIER_CALL_MODE_NOTIFICATION);  // don't send UDP
      break;

    case DMX_MODE_MULTIPLE_RGB:
      arlsLock(realtimeTimeoutMs, REALTIME_MODE_E131);
      if (previousUniverses == 0) {
        // first universe of this fixture
        possibleLEDsInCurrentUniverse = (dmxChannels - DMXAddress + 1) / 3;
        for (uint16_t i = 0; i < ledCount; i++) {
          if (i >= possibleLEDsInCurrentUniverse) break;  // more LEDs will follow in next universe(s)
          setRealtimePixel(i, p->property_values[DMXAddress+i*3+0], p->property_values[DMXAddress+i*3+1], p->property_values[DMXAddress+i*3+2], 0);
        }
      } else if (previousUniverses > 0 && uni < (e131Universe + E131_MAX_UNIVERSE_COUNT)) {
        // additional universe(s) of this fixture
        uint16_t numberOfLEDsInPreviousUniverses = ((512 - DMXAddress + 1) / 3);                            // first universe
        if (previousUniverses > 1) numberOfLEDsInPreviousUniverses += (512 / 3) * (previousUniverses - 1);  // extended universe(s) before current
        possibleLEDsInCurrentUniverse = dmxChannels / 3;
        for (uint16_t i = numberOfLEDsInPreviousUniverses; i < ledCount; i++) {
          uint8_t j = i - numberOfLEDsInPreviousUniverses;
          if (j >= possibleLEDsInCurrentUniverse) break;   // more LEDs will follow in next universe(s)
          setRealtimePixel(i, p->property_values[j*3+1], p->property_values[j*3+2], p->property_values[j*3+3], 0);
        }
      }
      break;

    case DMX_MODE_MULTIPLE_DRGB:
      arlsLock(realtimeTimeoutMs, REALTIME_MODE_E131);
      if (previousUniverses == 0) {
        // first universe of this fixture
        if (DMXOldDimmer != p->property_values[DMXAddress+0]) {
          DMXOldDimmer = p->property_values[DMXAddress+0];
          bri = p->property_values[DMXAddress+0];
          strip.setBrightness(bri);
        }
        possibleLEDsInCurrentUniverse = (dmxChannels - DMXAddress) / 3;
        for (uint16_t i = 0; i < ledCount; i++) {
          if (i >= possibleLEDsInCurrentUniverse) break;  // more LEDs will follow in next universe(s)
          setRealtimePixel(i, p->property_values[DMXAddress+i*3+1], p->property_values[DMXAddress+i*3+2], p->property_values[DMXAddress+i*3+3], 0);
        }
      } else if (previousUniverses > 0 && uni < (e131Universe + E131_MAX_UNIVERSE_COUNT)) {
        // additional universe(s) of this fixture
        uint16_t numberOfLEDsInPreviousUniverses = ((512 - DMXAddress + 1) / 3);                            // first universe
        if (previousUniverses > 1) numberOfLEDsInPreviousUniverses += (512 / 3) * (previousUniverses - 1);  // extended universe(s) before current
        possibleLEDsInCurrentUniverse = dmxChannels / 3;
        for (uint16_t i = numberOfLEDsInPreviousUniverses; i < ledCount; i++) {
          uint8_t j = i - numberOfLEDsInPreviousUniverses;
          if (j >= possibleLEDsInCurrentUniverse) break;   // more LEDs will follow in next universe(s)
          setRealtimePixel(i, p->property_values[j*3+1], p->property_values[j*3+2], p->property_values[j*3+3], 0);
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
