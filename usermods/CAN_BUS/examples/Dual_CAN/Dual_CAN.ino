// Demo: Dual CAN-BUS Shields, Data Pass-through
// Written by: Cory J. Fowler
// January 31st 2014
// This examples the ability of this library to support more than one MCP2515 based CAN interface.


#include <mcp_can.h>
#include <SPI.h>

unsigned long rxId;
byte len;
byte rxBuf[8];

byte txBuf0[] = {0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55};
byte txBuf1[] = {0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA};
char msgString[128];                        // Array to store serial string

MCP_CAN CAN0(2);                              // CAN0 interface usins CS on digital pin 2
MCP_CAN CAN1(3);                               // CAN1 interface using CS on digital pin 3

#define CAN0_INT 4    //define interrupt pin for CAN0 recieve buffer
#define CAN1_INT 5    //define interrupt pin for CAN1 recieve buffer

void setup()
{
  Serial.begin(115200);
  
  pinMode(CAN0_INT, INPUT_PULLUP);
  pinMode(CAN1_INT, INPUT_PULLUP);
  
  // init CAN0 bus, baudrate: 250k@16MHz
  if(CAN0.begin(MCP_ANY, CAN_250KBPS, MCP_16MHZ) == CAN_OK){
  Serial.print("CAN0: Init OK!\r\n");
  CAN0.setMode(MCP_NORMAL);
  } else Serial.print("CAN0: Init Fail!!!\r\n");
  
  // init CAN1 bus, baudrate: 250k@16MHz
  if(CAN1.begin(MCP_ANY, CAN_250KBPS, MCP_16MHZ) == CAN_OK){
  Serial.print("CAN1: Init OK!\r\n");
  CAN1.setMode(MCP_NORMAL);
  } else Serial.print("CAN1: Init Fail!!!\r\n");
  
  SPI.setClockDivider(SPI_CLOCK_DIV2);         // Set SPI to run at 8MHz (16MHz / 2 = 8 MHz)
  
  CAN0.sendMsgBuf(0x1000000, 1, 8, txBuf0);
  CAN1.sendMsgBuf(0x1000001, 1, 8, txBuf1);
}

void loop(){  
  if(!digitalRead(CAN0_INT)){                  // If interrupt pin is low, read CAN0 receive buffer
    Serial.println("CAN0 receive buffer:");
    CAN0.readMsgBuf(&rxId, &len, rxBuf);       // Read data: len = data length, buf = data byte(s)
    CAN1.sendMsgBuf(rxId, 1, len, rxBuf);      // Immediately send message out CAN1 interface 
    
    if((rxId & 0x80000000) == 0x80000000)      // Determine if ID is standard (11 bits) or extended (29 bits)
      sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (rxId & 0x1FFFFFFF), len);
    else
      sprintf(msgString, "Standard ID: 0x%.3lX       DLC: %1d  Data:", rxId, len);
  
    Serial.print(msgString);
  
    if((rxId & 0x40000000) == 0x40000000){    // Determine if message is a remote request frame.
      sprintf(msgString, " REMOTE REQUEST FRAME");
      Serial.print(msgString);
    } else {
      for(byte i = 0; i<len; i++){
        sprintf(msgString, " 0x%.2X", rxBuf[i]);
        Serial.print(msgString);
      }
    }
        
    Serial.println();
  }
  
  if(!digitalRead(CAN1_INT)){                         // If interrupt pin is low, read CAN1 receive buffer
    Serial.println("CAN1 receive buffer:");
    CAN1.readMsgBuf(&rxId, &len, rxBuf);       // Read data: len = data length, buf = data byte(s)
    CAN0.sendMsgBuf(rxId, 1, len, rxBuf);      // Immediately send message out CAN0 interface
    
    if((rxId & 0x80000000) == 0x80000000)     // Determine if ID is standard (11 bits) or extended (29 bits)
      sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (rxId & 0x1FFFFFFF), len);
    else
      sprintf(msgString, "Standard ID: 0x%.3lX       DLC: %1d  Data:", rxId, len);
  
    Serial.print(msgString);
  
    if((rxId & 0x40000000) == 0x40000000){    // Determine if message is a remote request frame.
      sprintf(msgString, " REMOTE REQUEST FRAME");
      Serial.print(msgString);
    } else {
      for(byte i = 0; i<len; i++){
        sprintf(msgString, " 0x%.2X", rxBuf[i]);
        Serial.print(msgString);
      }
    }
        
    Serial.println();
  }

  delay(1000);
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
