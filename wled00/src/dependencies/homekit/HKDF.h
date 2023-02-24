#include <stdint.h>
/////////////////////////////////////////////////
// HKDF-SHA-512
//
// This is a wrapper around mbedtls_hkdf, which is NOT
// included in the normal Arduino-ESP32 library.
// Code was instead sourced directly from MBED GitHub and 
// incorporated under hkdf.cpp, with a wrapper to always
// use SHA-512 with 32 bytes of output as required by HAP.

int create_hkdf(uint8_t *output_key, uint8_t *input_key, int input_len, const char *salt, const char *info);    // output of HKDF is always a 32-byte key derived from an input key, a salt string, and an info string
