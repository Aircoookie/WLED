/**
 * @file       BlynkConfig.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief      Configuration of different aspects of library
 *
 */

#ifndef BlynkConfig_h
#define BlynkConfig_h

#include "BlynkDetectDevice.h"

/***************************************************
 * Change these settings to match your need
 ***************************************************/

#define BLYNK_DEFAULT_DOMAIN     "blynk-cloud.com"
#define BLYNK_DEFAULT_PORT       80
#define BLYNK_DEFAULT_PORT_SSL   8441

/***************************************************
 * Professional settings
 ***************************************************/
// Library version.
#define BLYNK_VERSION        "0.5.3"

// Heartbeat period in seconds.
#ifndef BLYNK_HEARTBEAT
#define BLYNK_HEARTBEAT      10
#endif

// Network timeout in milliseconds.
#ifndef BLYNK_TIMEOUT_MS
#define BLYNK_TIMEOUT_MS     2000UL
#endif

// Limit the amount of outgoing commands per second.
#ifndef BLYNK_MSG_LIMIT
#define BLYNK_MSG_LIMIT      15
#endif

// Limit the incoming command length.
#ifndef BLYNK_MAX_READBYTES
#define BLYNK_MAX_READBYTES  256
#endif

// Limit the outgoing command length.
#ifndef BLYNK_MAX_SENDBYTES
#define BLYNK_MAX_SENDBYTES  128
#endif

// Uncomment to use Let's Encrypt Root CA
//#define BLYNK_SSL_USE_LETSENCRYPT

// Uncomment to disable built-in analog and digital operations.
//#define BLYNK_NO_BUILTIN

// Uncomment to disable providing info about device to the server.
//#define BLYNK_NO_INFO

// Uncomment to enable debug prints.
//#define BLYNK_DEBUG

// Uncomment to force-enable 128 virtual pins
//#define BLYNK_USE_128_VPINS

// Uncomment to disable fancy logo
//#define BLYNK_NO_FANCY_LOGO

// Uncomment to enable 3D fancy logo
//#define BLYNK_FANCY_LOGO_3D

// Uncomment to enable experimental functions.
//#define BLYNK_EXPERIMENTAL

// Uncomment to disable all float/double usage
//#define BLYNK_NO_FLOAT

// Uncomment to switch to direct-connect mode
//#define BLYNK_USE_DIRECT_CONNECT


// Uncomment to append command body to header (uses more RAM)
//#define BLYNK_SEND_ATOMIC

// Split whole command into chunks (in bytes)
//#define BLYNK_SEND_CHUNK 64

// Wait after sending each chunk (in milliseconds)
//#define BLYNK_SEND_THROTTLE 10

#endif
