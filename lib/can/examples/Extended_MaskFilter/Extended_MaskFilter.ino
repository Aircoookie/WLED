
// MCP2515 Mask and Filter example for extended CAN message frames.
// Written by Cory J. Fowler (20140717)

/***********************************************************************************
If you send the following extended IDs below to an Arduino loaded with this sketch
you will find that 0x00FFCC00 and 0x00FF9900 will not get in.

   ID in Hex is the same as the Filter in Hex.
   0x00FFEE00
   0x00FFDD00
   0x00FFCC00  This example will NOT be receiving this ID
   0x00FFBB00
   0x00FFAA00
   0x00FF9900  This example will NOT be receiving this ID
   0x00FF8800
   0x00FF7700

   This mask will check the filters against ID bits 23 through 8.
   (Those familiar with J1939 might see why I used this mask.)
    MASK = 0x00FFFF00
   If there is an explicit filter match to those bits, the message will be passed to the
   receive buffer and the interrupt pin will be set.
   
   This example will NOT be exclusive to ONLY the above message IDs, for that a mask such
   as the below would be used: 
    MASK = 0x1FFFFFFF
   
   At the moment, to disable a filter or mask, copy the value of a used filter or mask.
   
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

  CAN0.init_Mask(0,1,0x00FFFF00);                // Init first mask...
  CAN0.init_Filt(0,1,0x00FFEE00);                // Init first filter...
  CAN0.init_Filt(1,1,0x00FFDD00);                // Init second filter...
  
  CAN0.init_Mask(1,1,0x00FFFF00);                // Init second mask... 
  CAN0.init_Filt(2,1,0x00FFBB00);                // Init third filter...
  CAN0.init_Filt(3,1,0x00FFAA00);                // Init fourth filter...
  CAN0.init_Filt(4,1,0x00FF8800);                // Init fifth filter...
  CAN0.init_Filt(5,1,0x00FF7700);                // Init sixth filter...
  
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
