#include "wled.h"

bool parseLx(int lxValue, int rgbw[4])
{
  bool ok = false;
  float lxRed = 0;
  float lxGreen = 0;
  float lxBlue = 0;

  if (lxValue < 200000000) { 
    // Loxone RGB
    ok = true;
    lxRed = round((lxValue % 1000) * 2.55);
    lxGreen = round(((lxValue / 1000) % 1000) * 2.55);
    lxBlue = round(((lxValue / 1000000) % 1000) * 2.55);
  } else if ((lxValue >= 200000000) && (lxValue <= 201006500)) { 
    // Loxone Lumitech
    ok = true;
    float tmpBri = floor((lxValue - 200000000) / 10000); ;
    uint16_t ct = (lxValue - 200000000) - (((uint8_t)tmpBri) * 10000);
    float temp = 0;

    tmpBri *= 2.55;
    if (tmpBri < 0) {
      tmpBri = 0;
    } else if (tmpBri > 255) {
      tmpBri = 255;
    }
    if (ct < 2700) {
      ct = 2700;
    } else if (ct > 6500) {
      ct = 6500;
    }

    temp = ct / 100;
    if (temp <= 66) {
      lxRed = 255;
      lxGreen = round(99.4708025861 * log(temp) - 161.1195681661);
      if (temp <= 19) {
        lxBlue = 0;
      } else {
        lxBlue = round(138.5177312231 * log((temp - 10)) - 305.0447927307);
      }
    } else {
      lxRed = round(329.698727446 * pow((temp - 60), -0.1332047592));
      lxGreen = round(288.1221695283 * pow((temp - 60), -0.0755148492));
      lxBlue = 255;
    } 
    lxRed *= (tmpBri/255);
    lxGreen *= (tmpBri/255);
    lxBlue *= (tmpBri/255);
  }

  if (ok) {
    if (lxRed > 255) {
      lxRed = 255;
    } else if (lxRed < 0) {
      lxRed = 0;
    }
    if (lxGreen > 255) {
      lxGreen = 255;
    } else if (lxGreen < 0) {
      lxGreen = 0;
    }
    if (lxBlue > 255) {
      lxBlue = 255;
    } else if (lxBlue < 0) {
      lxBlue = 0;
    }
    rgbw[0] = (uint8_t)lxRed;
    rgbw[1] = (uint8_t)lxGreen;
    rgbw[2] = (uint8_t)lxBlue;
    rgbw[3] = 0;
    return true;
  }
  return false;
}

