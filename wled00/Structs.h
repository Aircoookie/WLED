/*
 * These are some data object structs, currently unused
 */
#ifndef WLED_Structs_H
#define WLED_Structs_H

struct Color {
  byte white;
  byte red;
  byte green;
  byte blue;
}

struct Palette {
  Color* paletteColors;
  byte mainColor = 0;
}

struct Segment {
  uint16_t minLed = 0, maxLed = 10;
  Palette segmentPalette;
  byte segmentEffect = 0;
  byte segmentEffectSpeed = 128;
  byte segmentEffectIntensity = 128;
}

struct Preset {
  byte mainSegment;
  Segment* segments;
  String presetName;
}

#endif
