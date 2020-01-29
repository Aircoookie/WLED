#ifdef WLED_ENABLE_DMX

void handleDMX() {
  int DMXchannels = 7;
  int 
  /* channel types:
   *  0            set this channel to 0, good way to tell strobe functions to fuck right off.
   *  1            red
   *  2            green
   *  3            blue
   *  4            white
   *  5            brightness control
   *  255          set this channel to 255
   */
  uint16_t pixel = 0;


  uint32_t in = strip.getPixelColor(pixel);
  byte w = in >> 24 & 0xFF;
  byte r = in >> 16  & 0xFF;
  byte g = in >> 8  & 0xFF;
  byte b = in       & 0xFF;
  uint8_t brightness = strip.getBrightness();

  dmx.write(2, r); // red
  dmx.write(3, g); // green
  dmx.write(4, b); // blue
  dmx.write(5, w); // white

  dmx.write(1, brightness); // shutter

  dmx.write(6, 1);   // automode

  dmx.update();           // update the DMX bus
}

#else
void DMXInit() {}
void handleDMX() {}
#endif
