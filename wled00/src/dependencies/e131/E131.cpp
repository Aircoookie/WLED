/*
* E131.cpp
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

#include "E131.h"
#include <string.h>

/* E1.17 ACN Packet Identifier */
const byte E131::ACN_ID[12] = { 0x41, 0x53, 0x43, 0x2d, 0x45, 0x31, 0x2e, 0x31, 0x37, 0x00, 0x00, 0x00 };

/* Constructor */
E131::E131() {
#ifdef NO_DOUBLE_BUFFER
    memset(pbuff1.raw, 0, sizeof(pbuff1.raw));
    packet = &pbuff1;
    pwbuff = packet;
#else
    memset(pbuff1.raw, 0, sizeof(pbuff1.raw));
    memset(pbuff2.raw, 0, sizeof(pbuff2.raw));
    packet = &pbuff1;
    pwbuff = &pbuff2;
#endif

    stats.num_packets = 0;
    stats.packet_errors = 0;
}

void E131::initUnicast() {
    udp.begin(E131_DEFAULT_PORT);
}

void E131::initMulticast(uint16_t universe, uint8_t n) {
    IPAddress address = IPAddress(239, 255, ((universe >> 8) & 0xff),
            ((universe >> 0) & 0xff));
	#ifdef ARDUINO_ARCH_ESP32
		ip4_addr_t ifaddr;
		ip4_addr_t multicast_addr;

		ifaddr.addr = static_cast<uint32_t>(WiFi.localIP());
		for (uint8_t i = 1; i < n; i++) {
			multicast_addr.addr = static_cast<uint32_t>(IPAddress(239, 255,
					(((universe + i) >> 8) & 0xff), (((universe + i) >> 0)
					& 0xff)));
			igmp_joingroup(&ifaddr, &multicast_addr);
		}
		udp.beginMulticast(address, E131_DEFAULT_PORT);
	#else
		ip_addr_t ifaddr;
		ip_addr_t multicast_addr;

		ifaddr.addr = static_cast<uint32_t>(WiFi.localIP());
		for (uint8_t i = 1; i < n; i++) {
			multicast_addr.addr = static_cast<uint32_t>(IPAddress(239, 255,
					(((universe + i) >> 8) & 0xff), (((universe + i) >> 0)
					& 0xff)));
			igmp_joingroup(&ifaddr, &multicast_addr);
		}
	  udp.beginMulticast(WiFi.localIP(), address, E131_DEFAULT_PORT);
	#endif
}

void E131::begin(e131_listen_t type, uint16_t universe, uint8_t n) {
    if (type == E131_UNICAST)
        initUnicast();
    if (type == E131_MULTICAST)
        initMulticast(universe, n);
}
