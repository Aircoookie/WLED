/*
 * Support for DMX via MAX485.
 * Needs the espdmx library. You might have to change the output pin within the library. Sketchy, i know.
 * https://github.com/Rickgg/ESP-Dmx
 */
#ifdef WLED_ENABLE_DMX

#ifdef ESP8266
  #include <LXESP8266UARTDMX.h>
#else
  #include <LXESP32DMX.h>
#endif

static uint8_t level;
static uint8_t dmxbuffer[DMX_MAX_FRAME];
static bool changed = true;

void copyDMXToOutput(void) {
  #ifndef ESP8266
    xSemaphoreTake( ESP32DMX.lxDataLock, portMAX_DELAY );
  #endif
	for (int i=1; i<DMX_MAX_FRAME; i++) {
    #ifdef ESP8266
      ESP8266DMX.setSlot(i, dmxbuffer[i]);
    #else
    	ESP32DMX.setSlot(i , dmxbuffer[i]);
    #endif
   }
   changed = false;
  #ifndef ESP8266
    xSemaphoreGive( ESP32DMX.lxDataLock );
  #endif
}

void setDMX(uint16_t DMXAddr, byte value) {
  if (DMXAddr < DMX_MAX_FRAME && dmxbuffer[DMXAddr] != value) {
    dmxbuffer[DMXAddr] = value;
    changed = true;
  }
}

void handleDMX() {
  // TODO: calculate brightness manually if no shutter channel is set
  
  uint8_t brightness = strip.getBrightness();
  bool calc_brightness = true;

  // check if no shutter channel is set
  for (byte i = 0; i < DMXChannels; i++)
  {
    if (DMXFixtureMap[i] == 5) calc_brightness = false;
  }

  for (int i = 0; i < ledCount; i++) { // uses the amount of LEDs as fixture count

    uint32_t in = strip.getPixelColor(i); // time to get the colors for the individual fixtures as suggested by AirCookie at issue #462
    byte w = in >> 24 & 0xFF;
    byte r = in >> 16 & 0xFF;
    byte g = in >> 8  & 0xFF;
    byte b = in       & 0xFF;

    int DMXFixtureStart = DMXStart + (DMXGap * i);
    for (int j = 0; j < DMXChannels; j++) {
      int DMXAddr = DMXFixtureStart + j;
      switch (DMXFixtureMap[j]) {
        case 0: // Set this channel to 0. Good way to tell strobe- and fade-functions to fuck right off.
          setDMX(DMXAddr, 0);
          break;
        case 1: // Red
          setDMX(DMXAddr, calc_brightness ? (r * brightness) / 255 : r);
          break;
        case 2: // Green
          setDMX(DMXAddr, calc_brightness ? (g * brightness) / 255 : g);
          break;
        case 3: // Blue
          setDMX(DMXAddr, calc_brightness ? (b * brightness) / 255 : b);
          break;
        case 4: // White
          setDMX(DMXAddr, calc_brightness ? (w * brightness) / 255 : w);
          break;
        case 5: // Shutter channel. Controls the brightness.
          setDMX(DMXAddr, brightness);
          break;
        case 6:// Sets this channel to 255. Like 0, but more wholesome.
          setDMX(DMXAddr, 255);
          break;
      }
    }
  }

  if (changed) {
    copyDMXToOutput();           // update the DMX bus
    DEBUG_PRINTLN("Sent colors via DMX");
  }
}

#else
void handleDMX() {}
#endif
