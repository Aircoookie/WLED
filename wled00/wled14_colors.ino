/*
 * Color conversion methods
 */
void colorCTtoRGB(uint16_t mired, uint8_t* rgb) //white spectrum to rgb
{
  
}

void colorHSBtoRGB(uint16_t hue, uint8_t sat, uint8_t bri, uint8_t* rgb) //hue, sat, bri to rgb
{
  
}

void colorXYtoRGB(float x, float y, uint8_t* rgb) //coordinates to rgb (https://www.developers.meethue.com/documentation/color-conversions-rgb-xy)
{
  float z = 1.0f - x - y;
  //float Y = 1.0f; // Brightness, we handle this separately
  float X = (1.0f / y) * x;
  float Z = (1.0f / y) * z;
  rgb[0] = (int)(X * 1.656492f - 0.354851f - Z * 0.255038f);
  rgb[1] = (int)(-X * 0.707196f + 1.655397f + Z * 0.036152f);
  rgb[2] = (int)(X * 0.051713f - 0.121364f + Z * 1.011530f);
}

void colorRGBtoXY(uint8_t* rgb, float* xy){} //rgb to coordinates (https://www.developers.meethue.com/documentation/color-conversions-rgb-xy)

void colorRGBtoRGBW(uint8_t* rgb, uint8_t* rgbw){} //rgb to rgbw, not imlemented yet
