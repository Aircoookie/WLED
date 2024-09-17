#include "wled.h"

/*
 * Color conversion & utility methods
 */

/*
 * color blend function
 */
uint32_t color_blend(uint32_t color1, uint32_t color2, uint16_t blend, bool b16) {
  if (blend == 0) return color1;
  unsigned blendmax = b16 ? 0xFFFF : 0xFF;
  if (blend == blendmax) return color2;
  unsigned shift = b16 ? 16 : 8;

  uint32_t w1 = W(color1);
  uint32_t r1 = R(color1);
  uint32_t g1 = G(color1);
  uint32_t b1 = B(color1);

  uint32_t w2 = W(color2);
  uint32_t r2 = R(color2);
  uint32_t g2 = G(color2);
  uint32_t b2 = B(color2);

  uint32_t w3 = ((w2 * blend) + (w1 * (blendmax - blend))) >> shift;
  uint32_t r3 = ((r2 * blend) + (r1 * (blendmax - blend))) >> shift;
  uint32_t g3 = ((g2 * blend) + (g1 * (blendmax - blend))) >> shift;
  uint32_t b3 = ((b2 * blend) + (b1 * (blendmax - blend))) >> shift;

  return RGBW32(r3, g3, b3, w3);
}

/*
 * color add function that preserves ratio
 * idea: https://github.com/Aircoookie/WLED/pull/2465 by https://github.com/Proto-molecule
 */
uint32_t color_add(uint32_t c1, uint32_t c2, bool fast)
{
  if (c1 == BLACK) return c2;
  if (c2 == BLACK) return c1;
  if (fast) {
    uint8_t r = R(c1);
    uint8_t g = G(c1);
    uint8_t b = B(c1);
    uint8_t w = W(c1);
    r = qadd8(r, R(c2));
    g = qadd8(g, G(c2));
    b = qadd8(b, B(c2));
    w = qadd8(w, W(c2));
    return RGBW32(r,g,b,w);
  } else {
    uint32_t r = R(c1) + R(c2);
    uint32_t g = G(c1) + G(c2);
    uint32_t b = B(c1) + B(c2);
    uint32_t w = W(c1) + W(c2);
    unsigned max = r;
    if (g > max) max = g;
    if (b > max) max = b;
    if (w > max) max = w;
    if (max < 256) return RGBW32(r, g, b, w);
    else           return RGBW32(r * 255 / max, g * 255 / max, b * 255 / max, w * 255 / max);
  }
}

/*
 * fades color toward black
 * if using "video" method the resulting color will never become black unless it is already black
 */

uint32_t color_fade(uint32_t c1, uint8_t amount, bool video)
{
  if (c1 == BLACK || amount + video == 0) return BLACK;
  uint32_t scaledcolor; // color order is: W R G B from MSB to LSB
  uint32_t r = R(c1);
  uint32_t g = G(c1);
  uint32_t b = B(c1);
  uint32_t w = W(c1);
  uint32_t scale = amount; // 32bit for faster calculation
  if (video) {
    scaledcolor  = (((r * scale) >> 8) + ((r && scale) ? 1 : 0)) << 16;
    scaledcolor |= (((g * scale) >> 8) + ((g && scale) ? 1 : 0)) << 8;
    scaledcolor |=  ((b * scale) >> 8) + ((b && scale) ? 1 : 0);
    scaledcolor |= (((w * scale) >> 8) + ((w && scale) ? 1 : 0)) << 24;
  } else {
    scaledcolor  = ((r * scale) >> 8) << 16;
    scaledcolor |= ((g * scale) >> 8) << 8;
    scaledcolor |=  (b * scale) >> 8;
    scaledcolor |= ((w * scale) >> 8) << 24;
  }
  return scaledcolor;
}

void setRandomColor(byte* rgb)
{
  lastRandomIndex = get_random_wheel_index(lastRandomIndex);
  colorHStoRGB(lastRandomIndex*256,255,rgb);
}

/*
 * generates a random palette based on harmonic color theory
 * takes a base palette as the input, it will choose one color of the base palette and keep it
 */
CRGBPalette16 generateHarmonicRandomPalette(CRGBPalette16 &basepalette)
{
  CHSV palettecolors[4]; //array of colors for the new palette
  uint8_t keepcolorposition = random8(4); //color position of current random palette to keep
  palettecolors[keepcolorposition] = rgb2hsv_approximate(basepalette.entries[keepcolorposition*5]); //read one of the base colors of the current palette
  palettecolors[keepcolorposition].hue += random8(10)-5; // +/- 5 randomness of base color
  //generate 4 saturation and brightness value numbers
  //only one saturation is allowed to be below 200 creating mostly vibrant colors
  //only one brightness value number is allowed below 200, creating mostly bright palettes

  for (int i = 0; i < 3; i++) { //generate three high values
    palettecolors[i].saturation = random8(200,255);
    palettecolors[i].value = random8(220,255);
  }
  //allow one to be lower
  palettecolors[3].saturation = random8(20,255);
  palettecolors[3].value = random8(80,255);

  //shuffle the arrays
  for (int i = 3; i > 0; i--) {
    std::swap(palettecolors[i].saturation, palettecolors[random8(i + 1)].saturation);
    std::swap(palettecolors[i].value, palettecolors[random8(i + 1)].value);
  }

  //now generate three new hues based off of the hue of the chosen current color
  uint8_t basehue = palettecolors[keepcolorposition].hue;
  uint8_t harmonics[3]; //hues that are harmonic but still a little random
  uint8_t type = random8(5); //choose a harmony type

  switch (type) {
    case 0: // analogous
      harmonics[0] = basehue + random8(30, 50);
      harmonics[1] = basehue + random8(10, 30);
      harmonics[2] = basehue - random8(10, 30);
      break;

    case 1: // triadic
      harmonics[0] = basehue + 113 + random8(15);
      harmonics[1] = basehue + 233 + random8(15);
      harmonics[2] = basehue -   7 + random8(15);
      break;

    case 2: // split-complementary
      harmonics[0] = basehue + 145 + random8(10);
      harmonics[1] = basehue + 205 + random8(10);
      harmonics[2] = basehue -   5 + random8(10);
      break;
    
    case 3: // square
      harmonics[0] = basehue +  85 + random8(10);
      harmonics[1] = basehue + 175 + random8(10);
      harmonics[2] = basehue + 265 + random8(10);
     break;

    case 4: // tetradic
      harmonics[0] = basehue +  80 + random8(20);
      harmonics[1] = basehue + 170 + random8(20);
      harmonics[2] = basehue -  15 + random8(30);
     break;
  }

  if (random8() < 128) {
    //50:50 chance of shuffling hues or keep the color order
    for (int i = 2; i > 0; i--) {
      std::swap(harmonics[i], harmonics[random8(i + 1)]);
    }
  }

  //now set the hues
  int j = 0;
  for (int i = 0; i < 4; i++) {
    if (i==keepcolorposition) continue; //skip the base color
    palettecolors[i].hue = harmonics[j];
    j++;
  }

  bool makepastelpalette = false;
  if (random8() < 25) { //~10% chance of desaturated 'pastel' colors
    makepastelpalette = true;
  }

  //apply saturation & gamma correction
  CRGB RGBpalettecolors[4];
  for (int i = 0; i < 4; i++) {
    if (makepastelpalette && palettecolors[i].saturation > 180) { 
      palettecolors[i].saturation -= 160; //desaturate all four colors
    }    
    RGBpalettecolors[i] = (CRGB)palettecolors[i]; //convert to RGB
    RGBpalettecolors[i] = gamma32(((uint32_t)RGBpalettecolors[i]) & 0x00FFFFFFU); //strip alpha from CRGB
  }

  return CRGBPalette16(RGBpalettecolors[0],
                       RGBpalettecolors[1],
                       RGBpalettecolors[2],
                       RGBpalettecolors[3]);
}

CRGBPalette16 generateRandomPalette()  //generate fully random palette
{
  return CRGBPalette16(CHSV(random8(), random8(160, 255), random8(128, 255)),
                       CHSV(random8(), random8(160, 255), random8(128, 255)),
                       CHSV(random8(), random8(160, 255), random8(128, 255)),
                       CHSV(random8(), random8(160, 255), random8(128, 255)));
}

void colorHStoRGB(uint16_t hue, byte sat, byte* rgb) //hue, sat to rgb
{
  float h = ((float)hue)/10922.5f; // hue*6/65535
  float s = ((float)sat)/255.0f;
  int   i = int(h);
  float f = h - i;
  int   p = int(255.0f * (1.0f-s));
  int   q = int(255.0f * (1.0f-s*f));
  int   t = int(255.0f * (1.0f-s*(1.0f-f)));
  p = constrain(p, 0, 255);
  q = constrain(q, 0, 255);
  t = constrain(t, 0, 255);
  switch (i%6) {
    case 0: rgb[0]=255,rgb[1]=t,  rgb[2]=p;  break;
    case 1: rgb[0]=q,  rgb[1]=255,rgb[2]=p;  break;
    case 2: rgb[0]=p,  rgb[1]=255,rgb[2]=t;  break;
    case 3: rgb[0]=p,  rgb[1]=q,  rgb[2]=255;break;
    case 4: rgb[0]=t,  rgb[1]=p,  rgb[2]=255;break;
    case 5: rgb[0]=255,rgb[1]=p,  rgb[2]=q;  break;
  }
}

//get RGB values from color temperature in K (https://tannerhelland.com/2012/09/18/convert-temperature-rgb-algorithm-code.html)
void colorKtoRGB(uint16_t kelvin, byte* rgb) //white spectrum to rgb, calc
{
  int r = 0, g = 0, b = 0;
  float temp = kelvin / 100.0f;
  if (temp <= 66.0f) {
    r = 255;
    g = roundf(99.4708025861f * logf(temp) - 161.1195681661f);
    if (temp <= 19.0f) {
      b = 0;
    } else {
      b = roundf(138.5177312231f * logf((temp - 10.0f)) - 305.0447927307f);
    }
  } else {
    r = roundf(329.698727446f * powf((temp - 60.0f), -0.1332047592f));
    g = roundf(288.1221695283f * powf((temp - 60.0f), -0.0755148492f));
    b = 255;
  }
  //g += 12; //mod by Aircoookie, a bit less accurate but visibly less pinkish
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
  r = r <= 0.0031308f ? 12.92f * r : (1.0f + 0.055f) * powf(r, (1.0f / 2.4f)) - 0.055f;
  g = g <= 0.0031308f ? 12.92f * g : (1.0f + 0.055f) * powf(g, (1.0f / 2.4f)) - 0.055f;
  b = b <= 0.0031308f ? 12.92f * b : (1.0f + 0.055f) * powf(b, (1.0f / 2.4f)) - 0.055f;

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
  rgb[0] = byte(255.0f*r);
  rgb[1] = byte(255.0f*g);
  rgb[2] = byte(255.0f*b);
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

  rgb[0] = R(c);
  rgb[1] = G(c);
  rgb[2] = B(c);
  rgb[3] = W(c);
}

//contrary to the colorFromDecOrHexString() function, this uses the more standard RRGGBB / RRGGBBWW order
bool colorFromHexString(byte* rgb, const char* in) {
  if (in == nullptr) return false;
  size_t inputSize = strnlen(in, 9);
  if (inputSize != 6 && inputSize != 8) return false;

  uint32_t c = strtoul(in, NULL, 16);

  if (inputSize == 6) {
    rgb[0] = (c >> 16);
    rgb[1] = (c >>  8);
    rgb[2] =  c       ;
  } else {
    rgb[0] = (c >> 24);
    rgb[1] = (c >> 16);
    rgb[2] = (c >>  8);
    rgb[3] =  c       ;
  }
  return true;
}

static inline float minf(float v, float w)
{
  if (w > v) return v;
  return w;
}

static inline float maxf(float v, float w)
{
  if (w > v) return w;
  return v;
}

// adjust RGB values based on color temperature in K (range [2800-10200]) (https://en.wikipedia.org/wiki/Color_balance)
// called from bus manager when color correction is enabled!
uint32_t colorBalanceFromKelvin(uint16_t kelvin, uint32_t rgb)
{
  //remember so that slow colorKtoRGB() doesn't have to run for every setPixelColor()
  static byte correctionRGB[4] = {0,0,0,0};
  static uint16_t lastKelvin = 0;
  if (lastKelvin != kelvin) colorKtoRGB(kelvin, correctionRGB);  // convert Kelvin to RGB
  lastKelvin = kelvin;
  byte rgbw[4];
  rgbw[0] = ((uint16_t) correctionRGB[0] * R(rgb)) /255; // correct R
  rgbw[1] = ((uint16_t) correctionRGB[1] * G(rgb)) /255; // correct G
  rgbw[2] = ((uint16_t) correctionRGB[2] * B(rgb)) /255; // correct B
  rgbw[3] =                                W(rgb);
  return RGBW32(rgbw[0],rgbw[1],rgbw[2],rgbw[3]);
}

//approximates a Kelvin color temperature from an RGB color.
//this does no check for the "whiteness" of the color,
//so should be used combined with a saturation check (as done by auto-white)
//values from http://www.vendian.org/mncharity/dir3/blackbody/UnstableURLs/bbr_color.html (10deg)
//equation spreadsheet at https://bit.ly/30RkHaN
//accuracy +-50K from 1900K up to 8000K
//minimum returned: 1900K, maximum returned: 10091K (range of 8192)
uint16_t approximateKelvinFromRGB(uint32_t rgb) {
  //if not either red or blue is 255, color is dimmed. Scale up
  uint8_t r = R(rgb), b = B(rgb);
  if (r == b) return 6550; //red == blue at about 6600K (also can't go further if both R and B are 0)

  if (r > b) {
    //scale blue up as if red was at 255
    uint16_t scale = 0xFFFF / r; //get scale factor (range 257-65535)
    b = ((uint16_t)b * scale) >> 8;
    //For all temps K<6600 R is bigger than B (for full bri colors R=255)
    //-> Use 9 linear approximations for blackbody radiation blue values from 2000-6600K (blue is always 0 below 2000K)
    if (b < 33)  return 1900 + b       *6;
    if (b < 72)  return 2100 + (b-33)  *10;
    if (b < 101) return 2492 + (b-72)  *14;
    if (b < 132) return 2900 + (b-101) *16;
    if (b < 159) return 3398 + (b-132) *19;
    if (b < 186) return 3906 + (b-159) *22;
    if (b < 210) return 4500 + (b-186) *25;
    if (b < 230) return 5100 + (b-210) *30;
                 return 5700 + (b-230) *34;
  } else {
    //scale red up as if blue was at 255
    uint16_t scale = 0xFFFF / b; //get scale factor (range 257-65535)
    r = ((uint16_t)r * scale) >> 8;
    //For all temps K>6600 B is bigger than R (for full bri colors B=255)
    //-> Use 2 linear approximations for blackbody radiation red values from 6600-10091K (blue is always 0 below 2000K)
    if (r > 225) return 6600 + (254-r) *50;
    uint16_t k = 8080 + (225-r) *86;
    return (k > 10091) ? 10091 : k;
  }
}

//gamma 2.8 lookup table used for color correction
uint8_t NeoGammaWLEDMethod::gammaT[256] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

// re-calculates & fills gamma table
void NeoGammaWLEDMethod::calcGammaTable(float gamma)
{
  for (size_t i = 0; i < 256; i++) {
    gammaT[i] = (int)(powf((float)i / 255.0f, gamma) * 255.0f + 0.5f);
  }
}

uint8_t IRAM_ATTR_YN NeoGammaWLEDMethod::Correct(uint8_t value)
{
  if (!gammaCorrectCol) return value;
  return gammaT[value];
}

// used for color gamma correction
uint32_t IRAM_ATTR_YN NeoGammaWLEDMethod::Correct32(uint32_t color)
{
  if (!gammaCorrectCol) return color;
  uint8_t w = W(color);
  uint8_t r = R(color);
  uint8_t g = G(color);
  uint8_t b = B(color);
  w = gammaT[w];
  r = gammaT[r];
  g = gammaT[g];
  b = gammaT[b];
  return RGBW32(r, g, b, w);
}
