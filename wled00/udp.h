#ifndef UDP_H
#define UDP_H

// expected to be included from wled.h where other dependencies are loaded first

void notify(byte callMode, bool followUp);
void realtimeLock(uint32_t timeoutMs, byte md);
void sendTPM2Ack();
void handleNotifications();
void setRealtimePixel(uint16_t i, byte r, byte g, byte b, byte w);

/*********************************************************************************************\
   Refresh aging for remote units, drop if too old...
\*********************************************************************************************/
void refreshNodeList();

/*********************************************************************************************\
   Broadcast system info to other nodes. (to update node lists)
\*********************************************************************************************/
void sendSysInfoUDP();

/*********************************************************************************************\
 * Art-Net, DDP, E131 output - work in progress
\*********************************************************************************************/

// Send real time DDP UDP updates to the specified client
//
//   client - the IP address to send to
//   rgbwData - a buffer of at least length*4 bytes long
//   length - the number of pixels
//
// Returns
//    0 - Ok
//    1 - could not allocate buffer
//
uint8_t realtimeBrodacast(IPAddress client, uint8_t *rgbwData, uint16_t length);

#define DDP_PORT 4048

#define DDP_PUSH_FLAG 0x01
#define DDP_TIMECODE_FLAG 0x10

#ifdef UPD_OUTPUT // just disable out for now
// Base class for all UDP output types. 
class UDPOutputData {
public:
    UDPOutputData(const JsonDocument& config);
    virtual ~UDPOutputData();

    virtual bool IsPingable() = 0;

    virtual void PrepareData(unsigned char* channelData /*,UDPOutputMessages& msgs*/) = 0;
    virtual void PostPrepareData(unsigned char* channelData /*,UDPOutputMessages& msgs*/) { }

    int startChannel;
    int channelCount;
    IPAddress ipAddress;

    UDPOutputData(UDPOutputData const&) = delete;
    void operator=(UDPOutputData const& x) = delete;

protected:
    // functions and settings to detect duplicate frames to avoid sending the same data as last time
    void SaveFrame(unsigned char* channelData, int len);
    bool NeedToOutputFrame(unsigned char* channelData, int startChannel, int savedIdx, int count);
    bool deDuplicate = false;
    int skippedFrames;
    unsigned char* lastData;
};

// Art-Net - https://en.wikipedia.org/wiki/Art-Net
class ArtNetOutputData : public UDPOutputData {
  // TODO
};

// Distributed Display Protocol (DDP)
class DDPOutputData : public UDPOutputData {
public:
    explicit DDPOutputData(const JsonDocument& config);
    virtual ~DDPOutputData();

    virtual bool IsPingable() override { return true; }
    virtual void PrepareData(unsigned char* channelData /*,UDPOutputMessages& msgs*/) override;
};

// E1.31 (Streaming-ACN) Protocol
class E131OutputData : public UDPOutputData {
  // TODO
};

class UDPOutput {
public:
    void AddOutput(UDPOutputData*);
};
#endif // UPD_OUTPUT

#endif