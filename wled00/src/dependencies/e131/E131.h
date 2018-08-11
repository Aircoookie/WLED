/*
* E131.h
*
* Project: E131 - E.131 (sACN) library for Arduino
* Copyright (c) 2015 Shelby Merrick
* http://www.forkineye.com
*
*  This program is provided free for you to use in any way that you wish,
*  subject to the laws and regulations where you are using it.  Due diligence
*  is strongly suggested before using this code.  Please give credit where due.
*
*  The Author makes no warranty of any kind, express or implied, with regard
*  to this program or the documentation contained in this document.  The
*  Author shall not be liable in any event for incidental or consequential
*  damages in connection with, or arising out of, the furnishing, performance
*  or use of these programs.
*
*/

#ifndef E131_H_
#define E131_H_

#include "Arduino.h"

/* Network interface detection.  WiFi for ESP8266 and Ethernet for AVR */
#if defined (ARDUINO_ARCH_ESP8266)
#   include <ESP8266WiFi.h>
#   define NO_DOUBLE_BUFFER
#elif defined (ARDUINO_ARCH_ESP32)
#   include <WiFi.h>
#endif
#   include <WiFiUdp.h>
#   include <lwip/ip_addr.h>
#   include <lwip/igmp.h>
#   define _UDP WiFiUDP

/* Defaults */
#define E131_DEFAULT_PORT 5568

/* E1.31 Packet Offsets */
#define E131_ROOT_PREAMBLE_SIZE 0
#define E131_ROOT_POSTAMBLE_SIZE 2
#define E131_ROOT_ID 4
#define E131_ROOT_FLENGTH 16
#define E131_ROOT_VECTOR 18
#define E131_ROOT_CID 22

#define E131_FRAME_FLENGTH 38
#define E131_FRAME_VECTOR 40
#define E131_FRAME_SOURCE 44
#define E131_FRAME_PRIORITY 108
#define E131_FRAME_RESERVED 109
#define E131_FRAME_SEQ 111
#define E131_FRAME_OPT 112
#define E131_FRAME_UNIVERSE 113

#define E131_DMP_FLENGTH 115
#define E131_DMP_VECTOR 117
#define E131_DMP_TYPE 118
#define E131_DMP_ADDR_FIRST 119
#define E131_DMP_ADDR_INC 121
#define E131_DMP_COUNT 123
#define E131_DMP_DATA 125

/* E1.31 Packet Structure */
typedef union {
    struct {
        /* Root Layer */
        uint16_t preamble_size;
        uint16_t postamble_size;
        uint8_t  acn_id[12];
        uint16_t root_flength;
        uint32_t root_vector;
        uint8_t  cid[16];

        /* Frame Layer */
        uint16_t frame_flength;
        uint32_t frame_vector;
        uint8_t  source_name[64];
        uint8_t  priority;
        uint16_t reserved;
        uint8_t  sequence_number;
        uint8_t  options;
        uint16_t universe;

        /* DMP Layer */
        uint16_t dmp_flength;
        uint8_t  dmp_vector;
        uint8_t  type;
        uint16_t first_address;
        uint16_t address_increment;
        uint16_t property_value_count;
        uint8_t  property_values[513];
    } __attribute__((packed));

    uint8_t raw[638];
} e131_packet_t;

/* Error Types */
typedef enum {
    ERROR_NONE,
    ERROR_IGNORE,
    ERROR_ACN_ID,
    ERROR_PACKET_SIZE,
    ERROR_VECTOR_ROOT,
    ERROR_VECTOR_FRAME,
    ERROR_VECTOR_DMP
} e131_error_t;

/* E1.31 Listener Types */
typedef enum {
    E131_UNICAST,
    E131_MULTICAST
} e131_listen_t;

/* Status structure */
typedef struct {
    uint32_t    num_packets;
    uint32_t    packet_errors;
    IPAddress   last_clientIP;
    uint16_t    last_clientPort;
} e131_stats_t;

class E131 {
 private:
    /* Constants for packet validation */
    static const uint8_t ACN_ID[];
    static const uint32_t VECTOR_ROOT = 4;
    static const uint32_t VECTOR_FRAME = 2;
    static const uint8_t VECTOR_DMP = 2;

    e131_packet_t   pbuff1;     /* Packet buffer */
#ifndef NO_DOUBLE_BUFFER
    e131_packet_t   pbuff2;     /* Double buffer */
#endif
    e131_packet_t   *pwbuff;    /* Pointer to working packet buffer */
    _UDP            udp;        /* UDP handle */

    /* Internal Initializers */
    void initUnicast();
    void initMulticast(uint16_t universe, uint8_t n = 1);

 public:
    uint8_t       *data;                /* Pointer to DMX channel data */
    uint16_t      universe;             /* DMX Universe of last valid packet */
    e131_packet_t *packet;              /* Pointer to last valid packet */
    e131_stats_t  stats;                /* Statistics tracker */

    E131();

    /* Generic UDP listener, no physical or IP configuration */
    void begin(e131_listen_t type, uint16_t universe = 1, uint8_t n = 1);

    /* Main packet parser */
    inline uint16_t parsePacket() {
        e131_error_t error;
        uint16_t retval = 0;

        int size = udp.parsePacket();
        if (size) {
            udp.readBytes(pwbuff->raw, size);
            error = validate();
            if (!error) {
#ifndef NO_DOUBLE_BUFFER
                e131_packet_t *swap = packet;
                packet = pwbuff;
                pwbuff = swap;
#endif
                universe = htons(packet->universe);
                data = packet->property_values + 1;
                retval = htons(packet->property_value_count) - 1;
                stats.num_packets++;
                stats.last_clientIP = udp.remoteIP();
                stats.last_clientPort = udp.remotePort();
            }
        }
        return retval;
    }

    /* Packet validater */
    inline e131_error_t validate() {
        if (memcmp(pwbuff->acn_id, ACN_ID, sizeof(pwbuff->acn_id)))
            return ERROR_ACN_ID;
        if (htonl(pwbuff->root_vector) != VECTOR_ROOT)
            return ERROR_VECTOR_ROOT;
        if (htonl(pwbuff->frame_vector) != VECTOR_FRAME)
            return ERROR_VECTOR_FRAME;
        if (pwbuff->dmp_vector != VECTOR_DMP)
            return ERROR_VECTOR_DMP;
        if (pwbuff->property_values[0] != 0)
            return ERROR_IGNORE;
        return ERROR_NONE;
    }
};

#endif /* E131_H_ */
