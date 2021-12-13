// - - - - -
// ESPDMX - A Arduino library for sending and receiving DMX using the builtin serial hardware port.
// ESPDMX.cpp: Library implementation file
//
// Copyright (C) 2015  Rick <ricardogg95@gmail.com>
// This work is licensed under a GNU style license.
//
// Last change: Marcel Seerig <https://github.com/mseerig>
//
// Documentation and samples are available at https://github.com/Rickgg/ESP-Dmx
// - - - - -

/* ----- LIBRARIES ----- */
#include <Arduino.h>

#include "ESPDMX.h"



#define dmxMaxChannel  512
#define defaultMax 32

#define DMXSPEED       250000
#define DMXFORMAT      SERIAL_8N2
#define BREAKSPEED     83333
#define BREAKFORMAT    SERIAL_8N1

#define RXPIN 4
#define TXPIN 17

HardwareSerial DMXSerial(2);

bool dmxStarted = false;
int sendPin = 4;		//dafault on ESP8266

//DMX value array and size. Entry 0 will hold startbyte
uint8_t dmxData[dmxMaxChannel] = {};
int chanSize;


void DMXESPSerial::init() {
  chanSize = defaultMax;

  DMXSerial.begin(DMXSPEED, SERIAL_8N1, RXPIN, TXPIN);
  pinMode(sendPin, OUTPUT);
  dmxStarted = true;
}

// Set up the DMX-Protocol
void DMXESPSerial::init(int chanQuant) {

  if (chanQuant > dmxMaxChannel || chanQuant <= 0) {
    chanQuant = defaultMax;
  }

  chanSize = chanQuant;

  DMXSerial.begin(DMXSPEED, SERIAL_8N1, RXPIN, TXPIN);
  pinMode(sendPin, OUTPUT);
  dmxStarted = true;
}

// Function to read DMX data
uint8_t DMXESPSerial::read(int Channel) {
  if (dmxStarted == false) init();

  if (Channel < 1) Channel = 1;
  if (Channel > dmxMaxChannel) Channel = dmxMaxChannel;
  return(dmxData[Channel]);
}

// Function to send DMX data
void DMXESPSerial::write(int Channel, uint8_t value) {
  if (dmxStarted == false) init();

  if (Channel < 1) Channel = 1;
  if (Channel > chanSize) Channel = chanSize;
  if (value < 0) value = 0;
  if (value > 255) value = 255;

  dmxData[Channel] = value;
}

void DMXESPSerial::end() {
  chanSize = 0;
  DMXSerial.end();
  dmxStarted = false;
}

void DMXESPSerial::update() {
  if (dmxStarted == false) init();

  //Send break
  digitalWrite(sendPin, HIGH);
  DMXSerial.begin(BREAKSPEED, BREAKFORMAT, RXPIN, TXPIN);
  DMXSerial.write(0);
  DMXSerial.flush();
  delay(10);
  DMXSerial.end();

  //pinMatrixOutDetach(TXPIN, false, false);
  //pinMode(TXPIN, OUTPUT);
  //digitalWrite(TXPIN, LOW); //88 uS break
  //delayMicroseconds(88);  
  //digitalWrite(TXPIN, HIGH); //4 Us Mark After Break
  //delayMicroseconds(1);
  //pinMatrixOutAttach(TXPIN, U1TXD_OUT_IDX, false, false);

  //send data
  DMXSerial.begin(DMXSPEED, DMXFORMAT, RXPIN, TXPIN);
  digitalWrite(sendPin, LOW);
  DMXSerial.write(dmxData, chanSize);
  DMXSerial.flush();
  delay(10);
  DMXSerial.end();
}

// Function to update the DMX bus
