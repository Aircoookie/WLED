/*
 *  TPM2.net Protocoll Support to control the strip over udp packages
 *
 *	KeyFeatures:
 *		- ASYNC UDP Interface
 *		- ...
 *	Options:
 *		- User ASYNC UDP Interface Interrupt driven
 *
 *  Protocoll Description:		https://gist.github.com/jblang/89e24e2655be6c463c56
 *  Control Software Example:	http://www.live-leds.de/jinx-usermanual-0.95a
 */

#ifndef TPM2NET_H_
#define TPM2NET_H_

#include "Arduino.h"

/* Network interface detection.  WiFi for ESP32 */
#if defined (ARDUINO_ARCH_ESP32)
#   include <WiFi.h>
#endif

#   include <WiFiUdp.h>
//#	include <ASYNCUDP.h>

/* Activate the ASYNC UDP Handler for Interrupt handled Mode*/
#if defined (ARDUINO_ARCH_ESP32) && defined (TPM2NET_ASYNC)
#define _UDP AsyncUDP
#else
#define _UDP WiFiUDP
#endif


/* Defaults */
#define TPM2NET_PORT 5568
/* Define the maximum Packet sieze * Header + User Data */
/* User Data = Count of LEDs * Bytes = 150 LED´s * 3 Byte = 450Bytes + Header (5) + End Byte (1) = 456 Bytes*/
#define TPM2NET_MAX_PACKETSIZE		456 
#define TPM2NET_BYTEPERLED			3
//char TPM2NET_VERSION[] = "1.0.0.0";


/* Header Definition:
Byte:		Description			 Value(s)
	0		Packed Start Byte    0x9C
	1		Packed CMD / Type	 0xDA for Data, 0xC0 for Command, 0xAA for Reuests
	2 - 3   Frame Size			 16bit Size Value
	4		Packet Number		 1 - 255
	6 - x   User Data			 LED Data with size of Frame Size
	last	Packet End Byte		 0x36	
*/


/* TPM2.NET Constants Offsets / Commands */
#define TPM2NET_HEADER_SIZE		5
#define TPM2NET_PROTOCOL_OVERHERAD 6

#define TPM2NET_HEADER_IDENT	0x9C

#define TPM2NET_CMD_DATAFRAME	0xDA
#define TPM2NET_CMD_COMMAND		0xC0
#define TPM2NET_CMD_REQEST		0xAA

#define TPM2NET_FOOTER_IDENT	0x36

/* Error Types */
typedef enum {
	ERROR_TPM_NONE = 0,
	ERROR_TPM_PACKET_SIZE = -1,
	ERROR_TPM_WRONG_FOOTER = -2,
	ERROR_TPM_WRONG_HEADER = -3,
	ERROR_TPM_WRONG_COMMAND = -4,
} tpm2net_error_t;


/* Header structure */
typedef struct {
	uint8_t  startbyte; // Must be 0x9C
	uint8_t  packedtype;
	uint16_t framesize; //1 ist High Byte 2. is Low Byte
	uint8_t  packetnumber; // Possible Value 1-255
} tpm2_header_t;

class TPM2NET {
 private:
 	 _UDP           tpm2udp;				/* UDP handle */
	 uint8_t		ledCount;
	 uint16_t		currentLed;
 public:
    uint8_t         *data;         
	uint8_t		    packetNumber;
	uint16_t		frameSize;
	uint8_t		    *tpm2packet;		 /* define the package holding Buffer*/


	int TPM2NETInit(uint16_t ledcount);

	/* Generic */
	void begin();

    /*  Main packet parser */
    inline uint16_t parsePacket() {
        uint8_t retval = 0;

        int size = tpm2udp.parsePacket();
        if (size >= 1) {
			tpm2udp.readBytes(tpm2packet, size);
			tpm2_header_t tpm2Header;
			memcpy(&tpm2Header, 0x00, sizeof(tpm2Header));

			//check header byte
			if (tpm2Header.startbyte != TPM2NET_HEADER_IDENT) {
#ifndef TPM2NET_DEBUG
				Serial.printf("TPM2NET -- Invalid header ident  %#010x", tpm2Header.startbyte);
#endif
				return 0;
			}

			//check the Comand Type
			if (tpm2Header.packedtype == TPM2NET_CMD_DATAFRAME) {
#ifndef TPM2NET_DEBUG
				Serial.print("TPM2NET -- Got CMD Type: REQUEST ");
#endif
			}else if(tpm2Header.packedtype == TPM2NET_CMD_COMMAND) {
#ifndef TPM2NET_DEBUG
				Serial.print("TPM2NET -- Got CMD Type: REQUEST ");
				Serial.print("TPM2NET -- Not Implemented yet");
#endif
				return 0;
			}else if (tpm2Header.packedtype == TPM2NET_CMD_REQEST){
#ifndef TPM2NET_DEBUG
				Serial.print("TPM2NET -- Got CMD Type: REQUEST ");
				Serial.print("TPM2NET -- Not Implemented yet");
#endif
				return 0;
			}else {
#ifndef TPM2NET_DEBUG
				Serial.printf("TPM2NET -- Invalid Comand type %#010x", tpm2Header.packedtype);
#endif
				return 0;
			}

			frameSize = tpm2packet[2];
			frameSize = (frameSize << 8) + tpm2packet[3];
			tpm2Header.framesize = frameSize;
#ifndef TPM2NET_DEBUG
			Serial.printf("TPM2NET -- Frame Size: %d", frameSize);
			Serial.printf("TPM2NET -- Packet Number: %d", tpm2Header.packetnumber);
#endif
			//check footer
			if (tpm2packet[frameSize + TPM2NET_HEADER_SIZE] != TPM2NET_FOOTER_IDENT) {
#ifndef TPM2NET_DEBUG
				Serial.printf ("Invalid footer ident tpm2Header.packedtype", tpm2packet[frameSize + TPM2NET_HEADER_SIZE], HEX);
#endif	
				return 0;
			}

			return tpm2Header.framesize;

		}
	}

};

#endif /* E131_H_ */
