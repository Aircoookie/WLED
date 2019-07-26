/*
 *  TPM2.net Protocoll Support to control the strip over udp packages
 *
 *	KeyFeatures: 
 *		- ASYNC UDP Interface
 *		- ...
 *
 *	
 *  Protocoll Description: https://gist.github.com/jblang/89e24e2655be6c463c56
 *  Control Software Example: http://www.live-leds.de/jinx-usermanual-0.95a
 */

#if not defined(#define WLED_DISABLE_TPM2NET)

//define some tpm constants
#define TPM2NET_LISTENING_PORT 65506
#define TPM2NET_HEADER_SIZE 5
#define TPM2NET_HEADER_IDENT 0x9c
#define TPM2NET_CMD_DATAFRAME 0xda
#define TPM2NET_CMD_COMMAND 0xc0
#define TPM2NET_CMD_ANSWER 0xaa
#define TPM2NET_FOOTER_IDENT 0x36

/* to replace with code */ 
#define NR_OF_PANELS 1
#define PIXELS_PER_PANEL 150

//3 byte per pixel or 24bit (RGB)
#define BPP 3


//as the arduino ethernet has only 2kb ram
//we must limit the maximal udp packet size
//a 64 pixel matrix needs 192 bytes data
#define UDP_PACKET_SIZE 500

//package size we expect. the footer byte is not included here!
#define EXPECTED_PACKED_SIZE (PIXELS_PER_PANEL*BPP+TPM2NET_HEADER_SIZE)

//some santiy checks here
#if EXPECTED_PACKED_SIZE > UDP_PACKET_SIZE
#error EXPECTED PACKED SIZE is bigger than UDP BUFFER! increase the buffer
#endif

byte packetBuffer[ UDP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

/*
  Serial.println("Starting UDP");
  udp.begin(TPM2NET_LISTENING_PORT);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
  Serial.print("Expected packagesize:");
  Serial.println(EXPECTED_PACKED_SIZE);


  // if there's data available, read a packet
  int packetSize = udp.parsePacket();
  if (packetSize > 0)
  {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);

    //tpm2 header size is 5 bytes
       if (packetSize > EXPECTED_PACKED_SIZE) {

    // read the packet into packetBufffer
    udp.read(packetBuffer, UDP_PACKET_SIZE);

    // -- Header check

    //check header byte
    if (packetBuffer[0] != TPM2NET_HEADER_IDENT) {
      Serial.print("Invalid header ident ");
      Serial.println(packetBuffer[0], HEX);
      return;
    }

    //check command
    if (packetBuffer[1] != TPM2NET_CMD_DATAFRAME) {
      Serial.print("Invalid block type ");
      Serial.println(packetBuffer[1], HEX);
      return;
    }

    uint16_t frameSize = packetBuffer[2];
    frameSize = (frameSize << 8) + packetBuffer[3];
    Serial.print("Framesize ");
    Serial.println(frameSize, HEX);

    //use packetNumber to calculate offset
    uint8_t packetNumber = packetBuffer[4];
    Serial.print("packetNumber ");
    Serial.println(packetNumber, HEX);

    //check footer
    if (packetBuffer[frameSize + TPM2NET_HEADER_SIZE] != TPM2NET_FOOTER_IDENT) {
      Serial.print("Invalid footer ident ");
      Serial.println(packetBuffer[frameSize + TPM2NET_HEADER_SIZE], HEX);
     // return;
    }

    //calculate offset
    uint16_t currentLed = packetNumber * PIXELS_PER_PANEL;
    int x = TPM2NET_HEADER_SIZE;
    for (byte i = 0; i < frameSize; i++) {
       strip.SetPixelColor(currentLed++, packetBuffer[x], packetBuffer[x + 1], packetBuffer[x + 2]);
      x += 3;
    }

    //TODO maybe update leds only if we got all pixeldata?
    strip.Show();   // write all the pixels out
       }
  }
*/
#endif
