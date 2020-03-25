#ifndef WLED_COLORS_H
#define WLED_COLORS_H
#include <Arduino.h>
/*
 * Color conversion methods
 */

void colorFromUint32(uint32_t in, bool secondary);
void colorFromUint24(uint32_t in, bool secondary = false);
void relativeChangeWhite(int8_t amount, byte lowerBoundary = 0);
void colorHStoRGB(uint16_t hue, byte sat, byte* rgb); //hue, sat to rgb
void colorCTtoRGB(uint16_t mired, byte* rgb); //white spectrum to rgb

void colorXYtoRGB(float x, float y, byte* rgb); // only defined if huesync disabled TODO
void colorRGBtoXY(byte* rgb, float* xy); // only defined if huesync disabled TODO

void colorFromDecOrHexString(byte* rgb, char* in);
void colorRGBtoRGBW(byte* rgb); //rgb to rgbw (http://codewelt.com/rgbw). (RGBW_MODE_LEGACY)

#endif //WLED_COLORS_H