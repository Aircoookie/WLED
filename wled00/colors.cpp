#include "wled.h"

/*
 * Color conversion methods
 */

void colorFromUint32(uint32_t in, bool secondary)
{
  if (secondary) {
    colSec[3] = in >> 24 & 0xFF;
    colSec[0] = in >> 16 & 0xFF;
    colSec[1] = in >> 8  & 0xFF;
    colSec[2] = in       & 0xFF;
  } else {
    col[3] = in >> 24 & 0xFF;
    col[0] = in >> 16 & 0xFF;
    col[1] = in >> 8  & 0xFF;
    col[2] = in       & 0xFF;
  }
}

//load a color without affecting the white channel
void colorFromUint24(uint32_t in, bool secondary)
{
  if (secondary) {
    colSec[0] = in >> 16 & 0xFF;
    colSec[1] = in >> 8  & 0xFF;
    colSec[2] = in       & 0xFF;
  } else {
    col[0] = in >> 16 & 0xFF;
    col[1] = in >> 8  & 0xFF;
    col[2] = in       & 0xFF;
  }
}

//store color components in uint32_t
uint32_t colorFromRgbw(byte* rgbw) {
  return (rgbw[0] << 16) + (rgbw[1] << 8) + rgbw[2] + (rgbw[3] << 24);
}

//relatively change white brightness, minumum A=5
void relativeChangeWhite(int8_t amount, byte lowerBoundary)
{
  int16_t new_val = (int16_t) col[3] + amount;
  if (new_val > 0xFF) new_val = 0xFF;
  else if (new_val < lowerBoundary) new_val = lowerBoundary;
  col[3] = new_val;
}

void colorHSVtoRGB(float hue, float saturation, float value, byte& red, byte& green, byte& blue)
{
  float r, g, b;

  auto i = static_cast<int>(hue * 6);
  auto f = hue * 6 - i;
  auto p = value * (1 - saturation);
  auto q = value * (1 - f * saturation);
  auto t = value * (1 - (1 - f) * saturation);

  switch (i % 6)
  {
  case 0: r = value, g = t, b = p;
    break;
  case 1: r = q, g = value, b = p;
    break;
  case 2: r = p, g = value, b = t;
    break;
  case 3: r = p, g = q, b = value;
    break;
  case 4: r = t, g = p, b = value;
    break;
  case 5: r = value, g = p, b = q;
    break;
  }

  red = static_cast<uint8_t>(r * 255);
  green = static_cast<uint8_t>(g * 255);
  blue = static_cast<uint8_t>(b * 255);
}

void colorRGBtoHSV(byte red, byte green, byte blue, float& hue, float& saturation, float& value)
{
  auto rd = static_cast<float>(red) / 255;
  auto gd = static_cast<float>(green) / 255;
  auto bd = static_cast<float>(blue) / 255;
  auto max = std::max({ rd, gd, bd }), min = std::min({ rd, gd, bd });

  value = max;

  auto d = max - min;
  saturation = max == 0 ? 0 : d / max;

  hue = 0;
  if (max != min)
  {
    if      (max == rd) hue = (gd - bd) / d + (gd < bd ? 6 : 0);
    else if (max == gd) hue = (bd - rd) / d + 2;
    else if (max == bd) hue = (rd - gd) / d + 4;
    hue /= 6;
  }
}

#define SATURATION_THRESHOLD 0.1
#define MAX_HSV_VALUE 1
#define MAX_HSV_SATURATION  1

//corrects the realtime colors. 10 is the unchanged saturation/value
//this feature might cause slowdowns with large LED counts
void correctColors(byte r, byte g, byte b, byte* rgb) {
  float hsv[3] = { 0,0,0 };
  colorRGBtoHSV(r, g,b , hsv[0], hsv[1], hsv[2]);
  float saturated = hsv[1] > SATURATION_THRESHOLD ?
    hsv[1] * ((float)liveHSVSaturation / 10) : hsv[1];
  float saturation = saturated < MAX_HSV_SATURATION ? saturated : MAX_HSV_SATURATION;

  float valued = hsv[2] * ((float)liveHSVValue/10);
  float value = valued < MAX_HSV_VALUE ? valued : MAX_HSV_VALUE;
  colorHSVtoRGB(hsv[0], saturation, value, rgb[0], rgb[1], rgb[2]);
}

void colorHStoRGB(uint16_t hue, byte sat, byte* rgb) //hue, sat to rgb
{
  float h = ((float)hue)/65535.0;
  float s = ((float)sat)/255.0;
  byte i = floor(h*6);
  float f = h * 6-i;
  float p = 255 * (1-s);
  float q = 255 * (1-f*s);
  float t = 255 * (1-(1-f)*s);
  switch (i%6) {
    case 0: rgb[0]=255,rgb[1]=t,rgb[2]=p;break;
    case 1: rgb[0]=q,rgb[1]=255,rgb[2]=p;break;
    case 2: rgb[0]=p,rgb[1]=255,rgb[2]=t;break;
    case 3: rgb[0]=p,rgb[1]=q,rgb[2]=255;break;
    case 4: rgb[0]=t,rgb[1]=p,rgb[2]=255;break;
    case 5: rgb[0]=255,rgb[1]=p,rgb[2]=q;
  }
  if (strip.isRgbw && strip.rgbwMode == RGBW_MODE_LEGACY) colorRGBtoRGBW(col);
}

void colorKtoRGB(uint16_t kelvin, byte* rgb) //white spectrum to rgb, calc
{
  float r = 0, g = 0, b = 0;
  float temp = kelvin / 100;
  if (temp <= 66) {
    r = 255;
    g = round(99.4708025861 * log(temp) - 161.1195681661);
    if (temp <= 19) {
      b = 0;
    } else {
      b = round(138.5177312231 * log((temp - 10)) - 305.0447927307);
    }
  } else {
    r = round(329.698727446 * pow((temp - 60), -0.1332047592));
    g = round(288.1221695283 * pow((temp - 60), -0.0755148492));
    b = 255;
  } 
  g += 15; //mod by Aircoookie, a bit less accurate but visibly less pinkish
  rgb[0] = (uint8_t) constrain(r, 0, 255);
  rgb[1] = (uint8_t) constrain(g, 0, 255);
  rgb[2] = (uint8_t) constrain(b, 0, 255);
  rgb[3] = 0;
}

void colorCTtoRGB(uint16_t mired, byte* rgb) //white spectrum to rgb, bins
{
  //this is only an approximation using WS2812B with gamma correction enabled
  if (mired > 475) {
    rgb[0]=255;rgb[1]=199;rgb[2]=92;//500
  } else if (mired > 425) {
    rgb[0]=255;rgb[1]=213;rgb[2]=118;//450
  } else if (mired > 375) {
    rgb[0]=255;rgb[1]=216;rgb[2]=118;//400
  } else if (mired > 325) {
    rgb[0]=255;rgb[1]=234;rgb[2]=140;//350
  } else if (mired > 275) {
    rgb[0]=255;rgb[1]=243;rgb[2]=160;//300
  } else if (mired > 225) {
    rgb[0]=250;rgb[1]=255;rgb[2]=188;//250
  } else if (mired > 175) {
    rgb[0]=247;rgb[1]=255;rgb[2]=215;//200
  } else {
    rgb[0]=237;rgb[1]=255;rgb[2]=239;//150
  }
  if (strip.isRgbw && strip.rgbwMode == RGBW_MODE_LEGACY) colorRGBtoRGBW(col);
}

#ifndef WLED_DISABLE_HUESYNC
void colorXYtoRGB(float x, float y, byte* rgb) //coordinates to rgb (https://www.developers.meethue.com/documentation/color-conversions-rgb-xy)
{
  float z = 1.0f - x - y;
  float X = (1.0f / y) * x;
  float Z = (1.0f / y) * z;
  float r = (int)255*(X * 1.656492f - 0.354851f - Z * 0.255038f);
  float g = (int)255*(-X * 0.707196f + 1.655397f + Z * 0.036152f);
  float b = (int)255*(X * 0.051713f - 0.121364f + Z * 1.011530f);
  if (r > b && r > g && r > 1.0f) {
    // red is too big
    g = g / r;
    b = b / r;
    r = 1.0f;
  } else if (g > b && g > r && g > 1.0f) {
    // green is too big
    r = r / g;
    b = b / g;
    g = 1.0f;
  } else if (b > r && b > g && b > 1.0f) {
    // blue is too big
    r = r / b;
    g = g / b;
    b = 1.0f;
  }
  // Apply gamma correction
  r = r <= 0.0031308f ? 12.92f * r : (1.0f + 0.055f) * pow(r, (1.0f / 2.4f)) - 0.055f;
  g = g <= 0.0031308f ? 12.92f * g : (1.0f + 0.055f) * pow(g, (1.0f / 2.4f)) - 0.055f;
  b = b <= 0.0031308f ? 12.92f * b : (1.0f + 0.055f) * pow(b, (1.0f / 2.4f)) - 0.055f;

  if (r > b && r > g) {
    // red is biggest
    if (r > 1.0f) {
      g = g / r;
      b = b / r;
      r = 1.0f;
    }
  } else if (g > b && g > r) {
    // green is biggest
    if (g > 1.0f) {
      r = r / g;
      b = b / g;
      g = 1.0f;
    }
  } else if (b > r && b > g) {
    // blue is biggest
    if (b > 1.0f) {
      r = r / b;
      g = g / b;
      b = 1.0f;
    }
  }
  rgb[0] = 255.0*r;
  rgb[1] = 255.0*g;
  rgb[2] = 255.0*b;
  if (strip.isRgbw && strip.rgbwMode == RGBW_MODE_LEGACY) colorRGBtoRGBW(col);
}

void colorRGBtoXY(byte* rgb, float* xy) //rgb to coordinates (https://www.developers.meethue.com/documentation/color-conversions-rgb-xy)
{
  float X = rgb[0] * 0.664511f + rgb[1] * 0.154324f + rgb[2] * 0.162028f;
  float Y = rgb[0] * 0.283881f + rgb[1] * 0.668433f + rgb[2] * 0.047685f;
  float Z = rgb[0] * 0.000088f + rgb[1] * 0.072310f + rgb[2] * 0.986039f;
  xy[0] = X / (X + Y + Z);
  xy[1] = Y / (X + Y + Z);
}
#endif // WLED_DISABLE_HUESYNC

//RRGGBB / WWRRGGBB order for hex
void colorFromDecOrHexString(byte* rgb, char* in)
{
  if (in[0] == 0) return;
  char first = in[0];
  uint32_t c = 0;
  
  if (first == '#' || first == 'h' || first == 'H') //is HEX encoded
  {
    c = strtoul(in +1, NULL, 16);
  } else
  {
    c = strtoul(in, NULL, 10);
  }

  rgb[3] = (c >> 24) & 0xFF;
  rgb[0] = (c >> 16) & 0xFF;
  rgb[1] = (c >>  8) & 0xFF;
  rgb[2] =  c        & 0xFF;
}

//contrary to the colorFromDecOrHexString() function, this uses the more standard RRGGBB / RRGGBBWW order
bool colorFromHexString(byte* rgb, const char* in) {
  if (in == nullptr) return false;
  size_t inputSize = strnlen(in, 9);
  if (inputSize != 6 && inputSize != 8) return false;

  uint32_t c = strtoul(in, NULL, 16);

  if (inputSize == 6) {
    rgb[0] = (c >> 16) & 0xFF;
    rgb[1] = (c >>  8) & 0xFF;
    rgb[2] =  c        & 0xFF;
  } else {
    rgb[0] = (c >> 24) & 0xFF;
    rgb[1] = (c >> 16) & 0xFF;
    rgb[2] = (c >>  8) & 0xFF;
    rgb[3] =  c        & 0xFF;
  }
  return true;
}

float minf (float v, float w)
{
  if (w > v) return v;
  return w;
}

float maxf (float v, float w)
{
  if (w > v) return w;
  return v;
}

void colorRGBtoRGBW(byte* rgb) //rgb to rgbw (http://codewelt.com/rgbw). (RGBW_MODE_LEGACY)
{
  float low = minf(rgb[0],minf(rgb[1],rgb[2]));
  float high = maxf(rgb[0],maxf(rgb[1],rgb[2]));
  if (high < 0.1f) return;
  float sat = 100.0f * ((high - low) / high);;   // maximum saturation is 100  (corrected from 255)
  rgb[3] = (byte)((255.0f - sat) / 255.0f * (rgb[0] + rgb[1] + rgb[2]) / 3);
}
