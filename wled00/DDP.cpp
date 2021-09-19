
/*
** Expected interface:
** 
**     realtimeBrodacast(IPAddress client, uint16_t busLength, byte rgbwData[busLength][4]);
**
*/

#include <Arduino.h>
#include <WiFiUdp.h>
#include "DDP.h"

#define DDP_HEADER_LEN 10
#define DDP_SYNCPACKET_LEN 10

#define DDP_FLAGS1_VER 0xc0  // version mask
#define DDP_FLAGS1_VER1 0x40 // version=1
#define DDP_FLAGS1_PUSH 0x01
#define DDP_FLAGS1_QUERY 0x02
#define DDP_FLAGS1_REPLY 0x04
#define DDP_FLAGS1_STORAGE 0x08
#define DDP_FLAGS1_TIME 0x10

#define DDP_ID_DISPLAY 1
#define DDP_ID_CONFIG 250
#define DDP_ID_STATUS 251

//1440 channels per packet
#define DDP_CHANNELS_PER_PACKET 1440 // 480 leds


// 
// copies a 4 byte rgbw buffer to a 3 byte rgb buffer (skipping the w channel)
// 
// destination - the buffer to write to
// source - the buffer to read from
// length - the number of 4 byte channels in the source buffer
// 
void copyRgbwToRgb(byte *destination, byte *source, uint16_t length) {

    uint16_t destinationOffset = 0;
    uint16_t sourceOffset = 0;
    
    for (uint16_t offset = 0; offset < length; offset++)
    {
        destination[destinationOffset+0] = source[sourceOffset+0];
        destination[destinationOffset+1] = source[sourceOffset+1];
        destination[destinationOffset+2] = source[sourceOffset+2];

        destinationOffset += 3;
        sourceOffset += 4;
    }
}

//
// Send real time DDP UDP updates to the specified client
//
// client - the IP address to send to
// busLength - the number of pixels
// rgbwData - a buffer of at least busLength*4 bytes long
//
uint8_t realtimeBrodacast(IPAddress client, uint16_t busLength, byte *rgbwData) {

    WiFiUDP ddpUdp;
    
    // calclate the number of UDP packets we need to send
    uint16_t channelCount = busLength * 3;
    uint16_t packetCount = channelCount / DDP_CHANNELS_PER_PACKET;
    if (channelCount % DDP_CHANNELS_PER_PACKET) {
        packetCount++;
    }

    // allocate a buffer for the UDP packet
    size_t bufferSize = DDP_HEADER_LEN + DDP_CHANNELS_PER_PACKET ;
    byte* buffer = (byte*)malloc(bufferSize);
    if (!buffer) {
        return 1;
    }

    memset(buffer, 0, bufferSize);

    // set common header values
    buffer[0] = DDP_FLAGS1_VER1;
    buffer[2] = 1;
    buffer[3] = DDP_ID_DISPLAY;

    // there are 3 channels per RGB pixel
    int channel = 0; // TODO: allow specifying the start channel

    for (int packetIndex = 0; packetIndex < packetCount; packetIndex++) {

        // how much data is after the header
        uint16_t packetSize = DDP_CHANNELS_PER_PACKET;

        if (packetIndex == (packetCount -1)) {
            // last packet, set the push flag
            buffer[0] = DDP_FLAGS1_VER1 | DDP_FLAGS1_PUSH;

            if (channelCount % DDP_CHANNELS_PER_PACKET) {
                packetSize = channelCount % DDP_CHANNELS_PER_PACKET;
            }
        }

        //offset
        buffer[4] = (channel & 0xFF000000) >> 24;
        buffer[5] = (channel & 0xFF0000) >> 16;
        buffer[6] = (channel & 0xFF00) >> 8;
        buffer[7] = (channel & 0xFF);

        //size
        buffer[8] = (packetSize & 0xFF00) >> 8;
        buffer[9] = packetSize & 0xFF;

        // copy the data into our buffer
        copyRgbwToRgb(&buffer[DDP_HEADER_LEN], rgbwData, busLength);

        ddpUdp.beginPacket(client, DDP_PORT);
        ddpUdp.write(buffer, packetSize);
        ddpUdp.endPacket();

        channel += packetSize;
    }
}
