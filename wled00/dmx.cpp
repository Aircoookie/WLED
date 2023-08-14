#include "wled.h"

/*
 * Support for DMX input and output via MAX485.
 * Change the output pin in src/dependencies/ESPDMX.cpp, if needed (ESP8266)
 * Change the output pin in src/dependencies/SparkFunDMX.cpp, if needed (ESP32)
 * ESP8266 Library from:
 * https://github.com/Rickgg/ESP-Dmx
 * ESP32 Library from:
 * https://github.com/sparkfun/SparkFunDMX
 */

#ifdef WLED_ENABLE_DMX

// WLEDMM: seems that DMX output triggers watchdog resets when compiling for IDF 4.4.x
#ifdef ARDUINO_ARCH_ESP32
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 2, 0)
#warning DMX output support might cause watchdog reset when compiling with ESP-IDF V4.4.x
// E (24101) task_wdt: Task watchdog got triggered. The following tasks did not reset the watchdog in time:
// E (24101) task_wdt:  - IDLE (CPU 0)
// E (24101) task_wdt: Tasks currently running:
// E (24101) task_wdt: CPU 0: FFT
// E (24101) task_wdt: CPU 1: IDLE
// E (24101) task_wdt: Aborting.
//abort() was called at PC 0x40143b6c on core 0
#endif
#endif

void handleDMX()
{
  // don't act, when in DMX Proxy mode
  if (e131ProxyUniverse != 0) return;

  uint8_t brightness = strip.getBrightness();

  bool calc_brightness = true;

   // check if no shutter channel is set
   for (byte i = 0; i < DMXChannels; i++)
   {
     if (DMXFixtureMap[i] == 5) calc_brightness = false;
   }

  uint16_t len = strip.getLengthTotal();
  for (int i = DMXStartLED; i < len; i++) {        // uses the amount of LEDs as fixture count

    uint32_t in = strip.getPixelColor(i);     // get the colors for the individual fixtures as suggested by Aircoookie in issue #462
    byte w = W(in);
    byte r = R(in);
    byte g = G(in);
    byte b = B(in);

    int DMXFixtureStart = DMXStart + (DMXGap * (i - DMXStartLED));
    for (int j = 0; j < DMXChannels; j++) {
      int DMXAddr = DMXFixtureStart + j;
      switch (DMXFixtureMap[j]) {
        case 0:        // Set this channel to 0. Good way to tell strobe- and fade-functions to fuck right off.
          dmx.write(DMXAddr, 0);
          break;
        case 1:        // Red
          dmx.write(DMXAddr, calc_brightness ? (r * brightness) / 255 : r);
          break;
        case 2:        // Green
          dmx.write(DMXAddr, calc_brightness ? (g * brightness) / 255 : g);
          break;
        case 3:        // Blue
          dmx.write(DMXAddr, calc_brightness ? (b * brightness) / 255 : b);
          break;
        case 4:        // White
          dmx.write(DMXAddr, calc_brightness ? (w * brightness) / 255 : w);
          break;
        case 5:        // Shutter channel. Controls the brightness.
          dmx.write(DMXAddr, brightness);
          break;
        case 6:        // Sets this channel to 255. Like 0, but more wholesome.
          dmx.write(DMXAddr, 255);
          break;
      }
    }
  }

  dmx.update();        // update the DMX bus
}

void initDMX() {
 #ifdef ESP8266
  dmx.init(512);        // initialize with bus length
 #else
  dmx.initWrite(512);  // initialize with bus length
 #endif
}
#else
void handleDMX() {}
#endif


#ifdef WLED_ENABLE_DMX_INPUT

#include <esp_dmx.h>


static dmx_port_t dmxInputPort = 2; //TODO make this configurable
bool dmxInputInitialized = false; //true once initDmx finished successfully

void initDMX() {


  if(dmxInputReceivePin > 0 && dmxInputEnablePin > 0 && dmxInputTransmitPin > 0) 
  {
    dmx_config_t config{                                             
        255,                          /*alloc_size*/             
        0,                            /*model_id*/               
        RDM_PRODUCT_CATEGORY_FIXTURE, /*product_category*/      
        VERSION,                      /*software_version_id*/    
        "undefined",                  /*software_version_label*/ 
        1,                            /*current_personality*/    
        {{15, "WLED Effect Mode"}},   /*personalities*/          
        1,                            /*personality_count*/      
        1,                            /*dmx_start_address*/      
    };
    const std::string versionString = "WLED_V" + std::to_string(VERSION);
    strncpy(config.software_version_label, versionString.c_str(), 32);
    config.software_version_label[32] = '\0';//zero termination in case our string was longer than 32 chars

    if(!dmx_driver_install(dmxInputPort, &config, DMX_INTR_FLAGS_DEFAULT))
    {
      USER_PRINTF("Error: Failed to install dmx driver\n");
      return;
    }
    
    USER_PRINTF("Listening for DMX on pin %u\n", dmxInputReceivePin);
    USER_PRINTF("Sending DMX on pin %u\n", dmxInputTransmitPin);
    USER_PRINTF("DMX enable pin is: %u\n", dmxInputEnablePin);
    dmx_set_pin(dmxInputPort, dmxInputTransmitPin, dmxInputReceivePin, dmxInputEnablePin);

    dmxInputInitialized = true;
  }
  else 
  {
    USER_PRINTLN("DMX input disabled due to dmxInputReceivePin, dmxInputEnablePin or dmxInputTransmitPin not set");
    return;
  }

}
  
bool dmxIsConnected = false;
unsigned long dmxLastUpdate = 0;

void handleDMXInput() {
  if(!dmxInputInitialized) {
    return;
  }
  byte dmxdata[DMX_PACKET_SIZE];
  dmx_packet_t packet;
  unsigned long now = millis();
  if (dmx_receive(dmxInputPort, &packet, 0)) {

    /* We should check to make sure that there weren't any DMX errors. */
    if (!packet.err) {
      /* If this is the first DMX data we've received, lets log it! */
      if (!dmxIsConnected) {
        USER_PRINTLN("DMX is connected!");
        dmxIsConnected = true;
      }

      dmx_read(dmxInputPort, dmxdata, packet.size);
      handleDMXData(1, 512, dmxdata, REALTIME_MODE_DMX, 0);
      dmxLastUpdate = now;

    } else {
      /* Oops! A DMX error occurred! Don't worry, this can happen when you first
        connect or disconnect your DMX devices. If you are consistently getting
        DMX errors, then something may have gone wrong with your code or
        something is seriously wrong with your DMX transmitter. */
     DEBUG_PRINT("A DMX error occurred - ");
     DEBUG_PRINTLN(packet.err);
    }
  }
  else if (dmxIsConnected && (now - dmxLastUpdate > 5000)) {
    dmxIsConnected = false;
    USER_PRINTLN("DMX was disconnected.");
  }
}
#else
void initDMX();
#endif