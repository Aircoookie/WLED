
/*
** Expected interface:
** 
**     realtimeBrodacast(IPAddress client, uint16_t busLength, byte rgbwData[busLength][4]);
**
** http://www.3waylabs.com/ddp/
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
// Parameters:
//   destination - the buffer to write to must be able to hold length*3 bytes
//   source - the buffer to read from
//   length - the number of 4 byte channels in the source buffer
// Returns:
//   the pointer in the source where we have copied up to
//
uint8_t* copyRgbwToRgb(uint8_t *destination, uint8_t *source, uint16_t length) {
    
    while (length--)
    {
        *(destination++) = *(source++); // R
        *(destination++) = *(source++); // G
        *(destination++) = *(source++); // B
        source++; // W
    }

    return source;
}

//
// Send real time DDP UDP updates to the specified client
//
// client - the IP address to send to
// length - the number of pixels
// rgbwData - a buffer of at least length*4 bytes long
//
uint8_t realtimeBrodacast(IPAddress client, uint8_t *rgbwData, uint16_t length) {

    WiFiUDP ddpUdp;

    // calclate the number of UDP packets we need to send
    uint16_t channelCount = length * 3; // 1 channel for every R,G,B value
    uint16_t packetCount = channelCount / DDP_CHANNELS_PER_PACKET;
    if (channelCount % DDP_CHANNELS_PER_PACKET) {
        packetCount++;
    }

    // allocate a buffer for the UDP packet
    size_t bufferSize = (DDP_HEADER_LEN + DDP_CHANNELS_PER_PACKET) * sizeof(uint8_t);
    uint8_t* buffer = (uint8_t*)malloc(bufferSize);
    if (!buffer) {
        return 1;
    }

    memset(buffer, 0, bufferSize);

    // set common header values
    buffer[0] = DDP_FLAGS1_VER1;
    buffer[2] = 1;
    buffer[3] = DDP_ID_DISPLAY;

    // there are 3 channels per RGB pixel
    uint16_t channel = 0; // TODO: allow specifying the start channel

    for (uint16_t currentPacket = 0; currentPacket < packetCount; currentPacket++) {

        // how much data is after the header
        uint16_t packetSize = DDP_CHANNELS_PER_PACKET;

        if (currentPacket == (packetCount - 1)) {
            // last packet, set the push flag
            // TODO: determine if we want to send an empty push packet to each destination after sending the pixel data
            buffer[0] = DDP_FLAGS1_VER1 | DDP_FLAGS1_PUSH;

            if (channelCount % DDP_CHANNELS_PER_PACKET) {
                packetSize = channelCount % DDP_CHANNELS_PER_PACKET;
            }
        }

        // data offset in bytes, 32-bit number, MSB first
        buffer[4] = (channel & 0xFF000000) >> 24;
        buffer[5] = (channel & 0xFF0000) >> 16;
        buffer[6] = (channel & 0xFF00) >> 8;
        buffer[7] = (channel & 0xFF);

        // data length in bytes, 16-bit number, MSB first
        buffer[8] = (packetSize & 0xFF00) >> 8;
        buffer[9] = packetSize & 0xFF;

        // copy the data from the source buffer into our pack
        rgbwData = copyRgbwToRgb(&buffer[DDP_HEADER_LEN], rgbwData, packetSize);

        ddpUdp.beginPacket(client, DDP_PORT);
        ddpUdp.write(buffer, packetSize);
        ddpUdp.endPacket();

        channel += packetSize;
    }

    free(buffer);
    return 0;
}
