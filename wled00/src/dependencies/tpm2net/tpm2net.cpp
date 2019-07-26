/*
* tpm2net.cpp
*
* Project: E131 - E.131 (sACN) library for Arduino
* Copyright (c) 2015 Shelby Merrick
* http://www.forkineye.com
*
*
*/

#include "tpm2net.h"
#include <string.h>
#include <stdio.h> 

int TPM2NET::TPM2NETInit(uint16_t inledCount){

	ledCount = inledCount;

	#ifndef TPM2NET_DEBUG
	Serial.printf("TPM2NET -- Intialize Module VERSION: 1.0");
	#endif 

	/* Set the Array Size, we got 3 Bytes per LED! */
	uint16_t sizeofTPM2Paket = ledCount * TPM2NET_BYTEPERLED + TPM2NET_PROTOCOL_OVERHERAD;

#ifndef TPM2NET_DEBUG
	Serial.printf("TPM2NET -- : LED %d - BPL %d - SizeOf Array", ledCount, TPM2NET_BYTEPERLED, sizeofTPM2Paket);
#endif 

	tpm2packet = new byte[sizeofTPM2Paket]; /* create an array with the rigth length for the Data */

	if (tpm2packet != NULL) {  /* allocation succeeded */
#ifndef TPM2NET_DEBUG
		Serial.printf("TPM2NET -- : Array Init Done - Size: ", sizeof(tpm2packet));
#endif 

		memset(tpm2packet, 0x00, sizeof(tpm2packet));
		return 0;
	}

	/* If the allocation not succeeded return a error!*/
	return -1;

}

void TPM2NET::begin() {
	tpm2udp.begin(TPM2NET_PORT);
 }
