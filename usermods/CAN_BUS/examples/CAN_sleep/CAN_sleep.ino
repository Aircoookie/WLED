// CAN Sleep Example
// By Zak Kemble, based on the CAN receive example

// If you only have 2 devices on the CAN bus (the device running this sketch and some other device sending messages), then you may find that duplicate messages are received when waking up.
// This is because when the MCP2515 wakes up it enters LISTENONLY mode where it does not send ACKs to messages, so the transmitter will retransmit the same message a few times.

#include <mcp_can.h>
#include <SPI.h>
#include <avr/sleep.h>

#define CAN0_INT 2 // Set INT to pin 2
MCP_CAN CAN0(10); // Set CS to pin 10

void setup()
{
	Serial.begin(115200);

	// Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
	if(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) == CAN_OK)
		Serial.println(F("MCP2515 Initialized Successfully!"));
	else
		Serial.println(F("Error Initializing MCP2515..."));

	CAN0.setMode(MCP_NORMAL); // Set operation mode to normal so the MCP2515 sends acks to received data.
	CAN0.setSleepWakeup(1); // Enable wake up interrupt when in sleep mode

	pinMode(CAN0_INT, INPUT); // Configuring pin for /INT input

	// Enable interrupts for the CAN0_INT pin (should be pin 2 or 3 for Uno and other ATmega328P based boards)
	attachInterrupt(digitalPinToInterrupt(CAN0_INT), ISR_CAN, FALLING);

	set_sleep_mode(SLEEP_MODE_PWR_DOWN);

	Serial.println(F("MCP2515 Library Sleep Example..."));
}

void loop()
{
	if(!digitalRead(CAN0_INT)) // If CAN0_INT pin is low, read receive buffer
	{
		unsigned long rxId;
		byte len;
		byte rxBuf[8];

		if(CAN0.readMsgBuf(&rxId, &len, rxBuf) == CAN_OK) // Read data: len = data length, buf = data byte(s)
		{
			char msgString[128]; // Array to store serial string
			if(rxId & CAN_IS_EXTENDED) // Determine if ID is standard (11 bits) or extended (29 bits)
				sprintf_P(msgString, PSTR("Extended ID: 0x%.8lX  DLC: %1d  Data:"), (rxId & CAN_EXTENDED_ID), len);
			else
				sprintf_P(msgString, PSTR("Standard ID: 0x%.3lX       DLC: %1d  Data:"), rxId, len);

			Serial.print(msgString);

			if(rxId & CAN_IS_REMOTE_REQUEST) // Determine if message is a remote request frame.
				Serial.print(F(" REMOTE REQUEST FRAME"));
			else
			{
				for(byte i=0;i<len;i++)
				{
					sprintf_P(msgString, PSTR(" 0x%.2X"), rxBuf[i]);
					Serial.print(msgString);
				}
			}
			
			Serial.println();
		}
		else
			Serial.println(F("Interrupt is asserted, but there were no messages to process..."));
	}
	
	Serial.println(F("SLEEPING..."));

	// Put MCP2515 into sleep mode
	// This can sometimes take up to around 100ms depending on what the chip is currently doing
	CAN0.setMode(MCP_SLEEP);

	Serial.println(F("SLEEP"));

	// Clear serial buffers before sleeping
	Serial.flush();

	// Put the microcontroller to sleep
	cli(); // Disable interrupts
	if(digitalRead(CAN0_INT)) // Make sure we haven't missed an interrupt between the digitalRead() above and now. If an interrupt happens between now and sei()/sleep_cpu() then sleep_cpu() will immediately wake up again
	{
		sleep_enable();
		sleep_bod_disable();
		sei();
		sleep_cpu();
		sleep_disable();
	}
	sei();

	Serial.println(F("WAKE"));

	CAN0.setMode(MCP_NORMAL); // When the MCP2515 wakes up it will be in LISTENONLY mode, here we put it into NORMAL mode
}

static void ISR_CAN()
{
	// We don't do anything here, this is just for waking up the microcontroller
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
