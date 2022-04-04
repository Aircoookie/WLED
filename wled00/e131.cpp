#include "wled.h"

#define MAX_3_CH_LEDS_PER_UNIVERSE 170
#define MAX_4_CH_LEDS_PER_UNIVERSE 128
#define MAX_CHANNELS_PER_UNIVERSE 512

/*
 * E1.31 handler
 */

//DDP protocol support, called by handleE131Packet
//handles RGB data only
void handleDDPPacket(e131_packet_t* p) {
  int lastPushSeq = e131LastSequenceNumber[0];
  
  //reject late packets belonging to previous frame (assuming 4 packets max. before push)
  if (e131SkipOutOfSequence && lastPushSeq) {
    int sn = p->sequenceNum & 0xF;
    if (sn) {
      if (lastPushSeq > 5) {
        if (sn > (lastPushSeq -5) && sn < lastPushSeq) return;
      } else {
        if (sn > (10 + lastPushSeq) || sn < lastPushSeq) return;
      }
    }
  }

  uint32_t start = htonl(p->channelOffset) /3;
  start += DMXAddress /3;
  uint16_t stop = start + htons(p->dataLen) /3;
  uint8_t* data = p->data;
  uint16_t c = 0;
  if (p->flags & DDP_TIMECODE_FLAG) c = 4; //packet has timecode flag, we do not support it, but data starts 4 bytes later

  realtimeLock(realtimeTimeoutMs, REALTIME_MODE_DDP);
  
  if (!realtimeOverride) {
    for (uint16_t i = start; i < stop; i++) {
      setRealtimePixel(i, data[c], data[c+1], data[c+2], 0);
      c+=3;
    }
  }

  bool push = p->flags & DDP_PUSH_FLAG;
  if (push) {
    e131NewData = true;
    byte sn = p->sequenceNum & 0xF;
    if (sn) e131LastSequenceNumber[0] = sn;
  }
}

//E1.31 and Art-Net protocol support
void handleE131Packet(e131_packet_t* p, IPAddress clientIP, byte protocol){

  uint16_t uni = 0, dmxChannels = 0;
  uint8_t* e131_data = nullptr;
  uint8_t seq = 0, mde = REALTIME_MODE_E131;

  if (protocol == P_ARTNET)
  {
    if (p->art_opcode == ARTNET_OPCODE_OPPOLL) {
      handleArtnetPollReply(clientIP);
      return;
    }
    uni = p->art_universe;
    dmxChannels = htons(p->art_length);
    e131_data = p->art_data;
    seq = p->art_sequence_number;
    mde = REALTIME_MODE_ARTNET;
  } else if (protocol == P_E131) {
    uni = htons(p->universe);
    dmxChannels = htons(p->property_value_count) -1;
    e131_data = p->property_values;
    seq = p->sequence_number;
  } else { //DDP
    realtimeIP = clientIP;
    handleDDPPacket(p);
    return;
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
  uint16_t totalLen = strip.getLengthTotal();

  switch (DMXMode) {
    case DMX_MODE_DISABLED:
      return;  // nothing to do
      break;

    case DMX_MODE_SINGLE_RGB: // RGB only
      if (uni != e131Universe) return;
      if (dmxChannels-DMXAddress+1 < 3) return;
      realtimeLock(realtimeTimeoutMs, mde);
      if (realtimeOverride) return;
      wChannel = (dmxChannels-DMXAddress+1 > 3) ? e131_data[DMXAddress+3] : 0;
      for (uint16_t i = 0; i < totalLen; i++)
        setRealtimePixel(i, e131_data[DMXAddress+0], e131_data[DMXAddress+1], e131_data[DMXAddress+2], wChannel);
      break;

    case DMX_MODE_SINGLE_DRGB: // Dimmer + RGB
      if (uni != e131Universe) return;
      if (dmxChannels-DMXAddress+1 < 4) return;
      realtimeLock(realtimeTimeoutMs, mde);
      if (realtimeOverride) return;
      wChannel = (dmxChannels-DMXAddress+1 > 4) ? e131_data[DMXAddress+4] : 0;
      if (DMXOldDimmer != e131_data[DMXAddress+0]) {
        DMXOldDimmer = e131_data[DMXAddress+0];
        bri = e131_data[DMXAddress+0];
        strip.setBrightness(bri, true);
      }
      for (uint16_t i = 0; i < totalLen; i++)
        setRealtimePixel(i, e131_data[DMXAddress+1], e131_data[DMXAddress+2], e131_data[DMXAddress+3], wChannel);
      break;

    case DMX_MODE_EFFECT: // Length 1: Apply Preset ID, length 11-13: apply effect config
      if (uni != e131Universe) return;
      if (dmxChannels-DMXAddress+1 < 11) {
        if (dmxChannels-DMXAddress+1 > 1) return;
        applyPreset(e131_data[DMXAddress+0], CALL_MODE_NOTIFICATION);
        return;
      }
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
      transitionDelayTemp = 0;               // act fast
      colorUpdated(CALL_MODE_NOTIFICATION);  // don't send UDP
      return;                                // don't activate realtime live mode
      break;

    case DMX_MODE_MULTIPLE_DRGB:
    case DMX_MODE_MULTIPLE_RGB:
    case DMX_MODE_MULTIPLE_RGBW:
      {
        realtimeLock(realtimeTimeoutMs, mde);
        bool is4Chan = (DMXMode == DMX_MODE_MULTIPLE_RGBW);
        const uint16_t dmxChannelsPerLed = is4Chan ? 4 : 3;
        const uint16_t ledsPerUniverse = is4Chan ? MAX_4_CH_LEDS_PER_UNIVERSE : MAX_3_CH_LEDS_PER_UNIVERSE;
        if (realtimeOverride) return;
        uint16_t previousLeds, dmxOffset;
        if (previousUniverses == 0) {
          if (dmxChannels-DMXAddress < 1) return;
          dmxOffset = DMXAddress;
          previousLeds = 0;
          // First DMX address is dimmer in DMX_MODE_MULTIPLE_DRGB mode.
          if (DMXMode == DMX_MODE_MULTIPLE_DRGB) {
            strip.setBrightness(e131_data[dmxOffset++], true);
          }
        } else {
          // All subsequent universes start at the first channel.
          dmxOffset = (protocol == P_ARTNET) ? 0 : 1;
          uint16_t ledsInFirstUniverse = (MAX_CHANNELS_PER_UNIVERSE - DMXAddress) / dmxChannelsPerLed;
          previousLeds = ledsInFirstUniverse + (previousUniverses - 1) * ledsPerUniverse;
        }
        uint16_t ledsTotal = previousLeds + (dmxChannels - dmxOffset +1) / dmxChannelsPerLed;
        if (!is4Chan) {
          for (uint16_t i = previousLeds; i < ledsTotal; i++) {
            setRealtimePixel(i, e131_data[dmxOffset], e131_data[dmxOffset+1], e131_data[dmxOffset+2], 0);
            dmxOffset+=3;
          }
        } else {
          for (uint16_t i = previousLeds; i < ledsTotal; i++) {
            setRealtimePixel(i, e131_data[dmxOffset], e131_data[dmxOffset+1], e131_data[dmxOffset+2], e131_data[dmxOffset+3]);
            dmxOffset+=4;
          }
        }
        break;
      }
    default:
      DEBUG_PRINTLN(F("unknown E1.31 DMX mode"));
      return;  // nothing to do
      break;
  }

  e131NewData = true;
}

void handleArtnetPollReply(IPAddress ipAddress) {
  prepareArtnetPollReply();

  uint16_t startUniverse = e131Universe;
  uint16_t endUniverse = e131Universe;

  switch (DMXMode) {
    case DMX_MODE_DISABLED:
      return;  // nothing to do
      break;

    case DMX_MODE_SINGLE_RGB:
    case DMX_MODE_SINGLE_DRGB:
    case DMX_MODE_EFFECT:
      break;  // 1 universe is enough

    case DMX_MODE_MULTIPLE_DRGB:
    case DMX_MODE_MULTIPLE_RGB:
    case DMX_MODE_MULTIPLE_RGBW:
      {
        bool is4Chan = (DMXMode == DMX_MODE_MULTIPLE_RGBW);
        const uint16_t dmxChannelsPerLed = is4Chan ? 4 : 3;
        const uint16_t dimmerOffset = (DMXMode == DMX_MODE_MULTIPLE_DRGB) ? 1 : 0;
        const uint16_t dmxLenOffset = (DMXAddress == 0) ? 0 : 1; // For legacy DMX start address 0
        const uint16_t ledsInFirstUniverse = (((MAX_CHANNELS_PER_UNIVERSE - DMXAddress) + dmxLenOffset) - dimmerOffset) / dmxChannelsPerLed;
        const uint16_t totalLen = strip.getLengthTotal();

        if (totalLen > ledsInFirstUniverse) {
          const uint16_t ledsPerUniverse = is4Chan ? MAX_4_CH_LEDS_PER_UNIVERSE : MAX_3_CH_LEDS_PER_UNIVERSE;
          const uint16_t remainLED = totalLen - ledsInFirstUniverse;

          endUniverse += (remainLED / ledsPerUniverse);

          if ((remainLED % ledsPerUniverse) > 0) {
            endUniverse++;
          }

          if ((endUniverse - startUniverse) > E131_MAX_UNIVERSE_COUNT) {
            endUniverse = startUniverse + E131_MAX_UNIVERSE_COUNT - 1;
          }
        }
        break;
      }
    default:
      DEBUG_PRINTLN(F("unknown E1.31 DMX mode"));
      return;  // nothing to do
      break;
  }

  for (uint16_t i = startUniverse; i <= endUniverse; ++i) {
    sendArtnetPollReply(ipAddress, i);
    yield();
  }
}

void prepareArtnetPollReply() {
  // Art-Net
  artnetPollReply.reply_id[0] = 0x41;
  artnetPollReply.reply_id[1] = 0x72;
  artnetPollReply.reply_id[2] = 0x74;
  artnetPollReply.reply_id[3] = 0x2d;
  artnetPollReply.reply_id[4] = 0x4e;
  artnetPollReply.reply_id[5] = 0x65;
  artnetPollReply.reply_id[6] = 0x74;
  artnetPollReply.reply_id[7] = 0x00;

  artnetPollReply.reply_opcode = ARTNET_OPCODE_OPPOLLREPLY;

  IPAddress localIP = Network.localIP();
  for (uint8_t i = 0; i < 4; i++) {
    artnetPollReply.reply_ip[i] = localIP[i];
  }

  artnetPollReply.reply_port = ARTNET_DEFAULT_PORT;

  char wledVersion[] = TOSTRING(WLED_VERSION);
  char * numberEnd = wledVersion;
  artnetPollReply.reply_version_h = (uint8_t)strtol(numberEnd, &numberEnd, 10);
  numberEnd++;
  artnetPollReply.reply_version_l = (uint8_t)strtol(numberEnd, &numberEnd, 10);

  // Switch values depend on universe, set before sending
  artnetPollReply.reply_net_sw = 0x00;
  artnetPollReply.reply_sub_sw = 0x00;

  artnetPollReply.reply_oem_h = 0x00;
  artnetPollReply.reply_oem_l = 0x00;

  artnetPollReply.reply_ubea_ver = 0x00;

  // Indicators in Normal Mode
  // All or part of Port-Address programmed by network or Web browser
  artnetPollReply.reply_status_1 = 0xE0;

  artnetPollReply.reply_esta_man = 0x0000;

  strlcpy((char *)(artnetPollReply.reply_short_name), serverDescription, 18);
  strlcpy((char *)(artnetPollReply.reply_long_name), serverDescription, 64);

  artnetPollReply.reply_node_report[0] = '\0';

  artnetPollReply.reply_num_ports_h = 0x00;
  artnetPollReply.reply_num_ports_l = 0x01; // One output port

  artnetPollReply.reply_port_types[0] = 0x80; // Output DMX data
  artnetPollReply.reply_port_types[1] = 0x00;
  artnetPollReply.reply_port_types[2] = 0x00;
  artnetPollReply.reply_port_types[3] = 0x00;

  // No inputs
  artnetPollReply.reply_good_input[0] = 0x00;
  artnetPollReply.reply_good_input[1] = 0x00;
  artnetPollReply.reply_good_input[2] = 0x00;
  artnetPollReply.reply_good_input[3] = 0x00;

  // One output
  artnetPollReply.reply_good_output_a[0] = 0x80; // Data is being transmitted
  artnetPollReply.reply_good_output_a[1] = 0x00;
  artnetPollReply.reply_good_output_a[2] = 0x00;
  artnetPollReply.reply_good_output_a[3] = 0x00;

  // Values depend on universe, set before sending
  artnetPollReply.reply_sw_in[0] = 0x00;
  artnetPollReply.reply_sw_in[1] = 0x00;
  artnetPollReply.reply_sw_in[2] = 0x00;
  artnetPollReply.reply_sw_in[3] = 0x00;

  // Values depend on universe, set before sending
  artnetPollReply.reply_sw_out[0] = 0x00;
  artnetPollReply.reply_sw_out[1] = 0x00;
  artnetPollReply.reply_sw_out[2] = 0x00;
  artnetPollReply.reply_sw_out[3] = 0x00;

  artnetPollReply.reply_sw_video = 0x00;
  artnetPollReply.reply_sw_macro = 0x00;
  artnetPollReply.reply_sw_remote = 0x00;

  artnetPollReply.reply_spare[0] = 0x00;
  artnetPollReply.reply_spare[1] = 0x00;
  artnetPollReply.reply_spare[2] = 0x00;

  // A DMX to / from Art-Net device
  artnetPollReply.reply_style = 0x00;

  Network.localMAC(artnetPollReply.reply_mac);

  for (uint8_t i = 0; i < 4; i++) {
    artnetPollReply.reply_bind_ip[i] = localIP[i];
  }

  artnetPollReply.reply_bind_index = 1;

  // Product supports web browser configuration
  // Node’s IP is DHCP or manually configured
  // Node is DHCP capable
  // Node supports 15 bit Port-Address (Art-Net 3 or 4)
  // Node is able to switch between ArtNet and sACN
  artnetPollReply.reply_status_2 = (staticIP[0] == 0) ? 0x1F : 0x1D;

  // RDM is disabled
  // Output style is continuous
  artnetPollReply.reply_good_output_b[0] = 0xC0;
  artnetPollReply.reply_good_output_b[1] = 0xC0;
  artnetPollReply.reply_good_output_b[2] = 0xC0;
  artnetPollReply.reply_good_output_b[3] = 0xC0;

  // Fail-over state: Hold last state
  // Node does not support fail-over
  artnetPollReply.reply_status_3 = 0x00;

  for (uint8_t i = 0; i < 21; i++) {
    artnetPollReply.reply_filler[i] = 0x00;
  }
}

void sendArtnetPollReply(IPAddress ipAddress, uint16_t portAddress) {
  artnetPollReply.reply_net_sw = (uint8_t)((portAddress >> 8) & 0x007F);
  artnetPollReply.reply_sub_sw = (uint8_t)((portAddress >> 4) & 0x000F);
  artnetPollReply.reply_sw_out[0] = (uint8_t)(portAddress & 0x000F);

  sprintf((char *)artnetPollReply.reply_node_report, "#0001 [%04u] OK - WLED version: " TOSTRING(WLED_VERSION), pollReplyCount);
  
  if (pollReplyCount < 9999) {
    pollReplyCount++;
  } else {
    pollReplyCount = 0;
  }

  pollReplyUDP.beginPacket(ipAddress, ARTNET_DEFAULT_PORT);
  pollReplyUDP.write(artnetPollReply.raw, sizeof(ArtPollReply));
  pollReplyUDP.endPacket();

  artnetPollReply.reply_bind_index++;
}