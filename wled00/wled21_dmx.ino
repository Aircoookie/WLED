/*
 * Support for DMX via MAX485.
 * Needs the espdmx library. You might have to change the output pin within the library. Sketchy, i know.
 * https://github.com/Rickgg/ESP-Dmx
 */
#ifdef WLED_ENABLE_DMX

void handleDMX() {
  // TODO: calculate brightness manually if no shutter channel is set
  
  uint8_t brightness = strip.getBrightness();

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
          dmx.write(DMXAddr, 0);
          break;
        case 1: // Red
          dmx.write(DMXAddr, r);
          break;
        case 2: // Green
          dmx.write(DMXAddr, g);
          break;
        case 3: // Blue
          dmx.write(DMXAddr, b);
          break;
        case 4: // White
          dmx.write(DMXAddr, w);
          break;
        case 5: // Shutter channel. Controls the brightness.
          dmx.write(DMXAddr, brightness);
          break;
        case 6:// Sets this channel to 255. Like 0, but more wholesome.
          dmx.write(DMXAddr, 255);
          break;
      }
    }
  }

  dmx.update();           // update the DMX bus
}

#else
void handleDMX() {}
#endif
