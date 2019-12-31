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
  if (useRGBW) colorRGBtoRGBW(col);
}

#ifndef WLED_DISABLE_HUESYNC
void colorCTtoRGB(uint16_t mired, byte* rgb) //white spectrum to rgb
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
  if (useRGBW) colorRGBtoRGBW(col);
}

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
  if (useRGBW) colorRGBtoRGBW(col);
}

void colorRGBtoXY(byte* rgb, float* xy) //rgb to coordinates (https://www.developers.meethue.com/documentation/color-conversions-rgb-xy)
{
  float X = rgb[0] * 0.664511f + rgb[1] * 0.154324f + rgb[2] * 0.162028f;
  float Y = rgb[0] * 0.283881f + rgb[1] * 0.668433f + rgb[2] * 0.047685f;
  float Z = rgb[0] * 0.000088f + rgb[1] * 0.072310f + rgb[2] * 0.986039f;
  xy[0] = X / (X + Y + Z);
  xy[1] = Y / (X + Y + Z);
}
#endif

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

void colorRGBtoRGBW(byte* rgb) //rgb to rgbw (http://codewelt.com/rgbw)
{
  float low = minf(rgb[0],minf(rgb[1],rgb[2]));
  float high = maxf(rgb[0],maxf(rgb[1],rgb[2]));
  if (high < 0.1f) return;
  float sat = 100.0f * ((high - low) / high);;   // maximum saturation is 100  (corrected from 255)
  rgb[3] = (byte)((255.0f - sat) / 255.0f * (rgb[0] + rgb[1] + rgb[2]) / 3);
}
