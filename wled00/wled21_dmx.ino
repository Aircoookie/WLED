#ifdef WLED_ENABLE_DMX

void handleDMX() {
  int DMXChannels = 7;
  int DMXFixtureMap[] = { 5, 1, 2, 3, 4, 0, 0};
  int DMXGap = 10;
  int DMXStart = 10;
  /* channel types:
      0            set this channel to 0, good way to tell strobe functions to fuck right off.
      1            red
      2            green
      3            blue
      4            white
      5            brightness control
      255          set this channel to 255
  */

  uint8_t brightness = strip.getBrightness();

  for (int i = 0; i < ledCount; i++) {

    uint32_t in = strip.getPixelColor(i);
    byte w = in >> 24 & 0xFF;
    byte r = in >> 16  & 0xFF;
    byte g = in >> 8  & 0xFF;
    byte b = in       & 0xFF;

    int DMXFixtureStart = DMXStart + (DMXGap * i);
    for (int j = 0; j < DMXChannels; j++) {
      int DMXAddr = DMXFixtureStart + j;
      switch (DMXFixtureMap[j]) {
        case 0:
          dmx.write(DMXAddr, 0);
          break;
        case 1:
          dmx.write(DMXAddr, r);
          break;
        case 2:
          dmx.write(DMXAddr, g);
          break;
        case 3:
          dmx.write(DMXAddr, b);
          break;
        case 4:
          dmx.write(DMXAddr, w);
          break;
        case 5:
          dmx.write(DMXAddr, brightness);
          break;
        case 255:
          dmx.write(DMXAddr, 255);
          break;


      }
    }
  }

  dmx.update();           // update the DMX bus
}

#else
void DMXInit() {}
void handleDMX() {}
#endif
