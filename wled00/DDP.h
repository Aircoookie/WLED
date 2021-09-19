#define DDP_PORT 4048

#define DDP_PUSH_FLAG 0x01
#define DDP_TIMECODE_FLAG 0x10

//
// Send real time DDP UDP updates to the specified client
//
//   client - the IP address to send to
//   busLength - the number of pixels
//   rgbwData - a buffer of at least busLength*4 bytes long
//
// Returns
//    0 - Ok
//    1 - could not allocate buffer
uint8_t realtimeBrodacast(IPAddress client, uint16_t busLength, byte *rgbwData);
