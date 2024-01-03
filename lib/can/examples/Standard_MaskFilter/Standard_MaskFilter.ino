
// MCP2515 Mask and Filter example for standard CAN message frames.
// Written by Cory J. Fowler (20140717)

/***********************************************************************************
If you send the following standard IDs below to an Arduino loaded with this sketch
you will find that 0x102 and 0x105 will not get in.

ID in Hex  -   Two Data Bytes!   -  Filter/Mask in HEX
   0x100   + 0000 0000 0000 0000 =   0x01000000
   0x101   + 0000 0000 0000 0000 =   0x01010000
   0x102   + 0000 0000 0000 0000 =   0x01020000  This example will NOT be receiving this ID
   0x103   + 0000 0000 0000 0000 =   0x01030000
   0x104   + 0000 0000 0000 0000 =   0x01040000
   0x105   + 0000 0000 0000 0000 =   0x01050000  This example will NOT be receiving this ID
   0x106   + 0000 0000 0000 0000 =   0x01060000
   0x107   + 0000 0000 0000 0000 =   0x01070000

   This mask will check the filters against ID bit 8 and ID bits 3-0.   
    MASK   + 0000 0000 0000 0000 =   0x010F0000
   
   If there is an explicit filter match to those bits, the message will be passed to the
   receive buffer and the interrupt pin will be set.
   This example will NOT be exclusive to ONLY the above frame IDs, for that a mask such
   as the below would be used: 
    MASK   + 0000 0000 0000 0000 = 0x07FF0000
    
   This mask will check the filters against all ID bits and the first data byte:
    MASK   + 1111 1111 0000 0000 = 0x07FFFF00
   If you use this mask and do not touch the filters below, you will find that your first
   data byte must be 0x00 for the message to enter the receive buffer.
   
   At the moment, to disable a filter or mask, copy the value of a used filter or mask.
   
   Data bytes are ONLY checked when the MCP2515 is in 'MCP_STDEXT' mode via the begin
   function, otherwise ('MCP_STD') only the ID is checked.
***********************************************************************************/


#include <mcp_can.h>
#include <SPI.h>

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];

MCP_CAN CAN0(10);                          // Set CS to pin 10

void setup()
{
  Serial.begin(115200);
  if(CAN0.begin(MCP_STDEXT, CAN_500KBPS, MCP_16MHZ) == CAN_OK) Serial.print("MCP2515 Init Okay!!\r\n");
  else Serial.print("MCP2515 Init Failed!!\r\n");
  pinMode(2, INPUT);                       // Setting pin 2 for /INT input


  CAN0.init_Mask(0,0,0x010F0000);                // Init first mask...
  CAN0.init_Filt(0,0,0x01000000);                // Init first filter...
  CAN0.init_Filt(1,0,0x01010000);                // Init second filter...
  
  CAN0.init_Mask(1,0,0x010F0000);                // Init second mask... 
  CAN0.init_Filt(2,0,0x01030000);                // Init third filter...
  CAN0.init_Filt(3,0,0x01040000);                // Init fourth filter...
  CAN0.init_Filt(4,0,0x01060000);                // Init fifth filter...
  CAN0.init_Filt(5,0,0x01070000);                // Init sixth filter...
  
  Serial.println("MCP2515 Library Mask & Filter Example...");
  CAN0.setMode(MCP_NORMAL);                // Change to normal mode to allow messages to be transmitted
}

void loop()
{
    if(!digitalRead(2))                    // If pin 2 is low, read receive buffer
    {
      CAN0.readMsgBuf(&rxId, &len, rxBuf); // Read data: len = data length, buf = data byte(s)
      Serial.print("ID: ");
      Serial.print(rxId, HEX);
      Serial.print(" Data: ");
      for(int i = 0; i<len; i++)           // Print each byte of the data
      {
        if(rxBuf[i] < 0x10)                // If data byte is less than 0x10, add a leading zero
        {
          Serial.print("0");
        }
        Serial.print(rxBuf[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
    }
}

/*********************************************************************************************************
END FILE
*********************************************************************************************************/
