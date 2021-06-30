#ifndef WLED_MATH_H
#define WLED_MATH_H

/*
 * Contains some trigonometric functions.
 * The ANSI C equivalents are likely faster, but using any sin/cos/tan function incurs a memory penalty of 460 bytes on ESP8266, likely for lookup tables.
 * This implementation has no extra static memory usage.
 * 
 * Source of the cos_t() function: https://web.eecs.utk.edu/~azh/blog/cosine.html (cos_taylor_literal_6terms)
 */

#include <Arduino.h> //PI constant

//#define WLED_DEBUG_MATH

#define modd(x, y) ((x) - (int)((x) / (y)) * (y))

float cos_t(float x)
{
  x = modd(x, TWO_PI);
  char sign = 1;
  if (x > PI)
  {
      x -= PI;
      sign = -1;
  }
  float xx = x * x;

  float res = sign * (1 - ((xx) / (2)) + ((xx * xx) / (24)) - ((xx * xx * xx) / (720)) + ((xx * xx * xx * xx) / (40320)) - ((xx * xx * xx * xx * xx) / (3628800)) + ((xx * xx * xx * xx * xx * xx) / (479001600)));
  #ifdef WLED_DEBUG_MATH
  Serial.printf("cos: %f,%f\n",res,cos(x));
  #endif
  return res;
}

float sin_t(float x) {
  float res =  cos_t(HALF_PI - x);
  #ifdef WLED_DEBUG_MATH
  Serial.printf("sin: %f,%f\n",res,sin(x));
  #endif
  return res;
}

float tan_t(float x) {
  float c = cos_t(x);
  if (c==0.0) return 0;
  float res = sin_t(x) / c;
  #ifdef WLED_DEBUG_MATH
  Serial.printf("tan: %f,%f\n",res,tan(x));
  #endif
  return res;
}

//https://stackoverflow.com/questions/3380628
// Absolute error <= 6.7e-5
float acos_t(float x) {
  float negate = float(x < 0);
  float xabs = std::abs(x);
  float ret = -0.0187293;
  ret = ret * xabs;
  ret = ret + 0.0742610;
  ret = ret * xabs;
  ret = ret - 0.2121144;
  ret = ret * xabs;
  ret = ret + HALF_PI;
  ret = ret * sqrt(1.0-xabs);
  ret = ret - 2 * negate * ret;
  float res = negate * PI + ret;
  #ifdef WLED_DEBUG_MATH
  Serial.printf("acos,%f,%f,%f\n",x,res,acos(x));
  #endif
  return res;
}

float asin_t(float x) {
  float res = HALF_PI - acos_t(x);
  #ifdef WLED_DEBUG_MATH
  Serial.printf("asin,%f,%f,%f\n",x,res,asin(x));
  #endif
  return res;
}

//https://stackoverflow.com/a/42542593
#define A 0.0776509570923569
#define B -0.287434475393028
#define C ((HALF_PI/2) - A - B)

//polynominal factors for approximation between 1 and 5
#define C0 0.089494f
#define C1 0.974207f
#define C2 -0.326175f
#define C3 0.05375f
#define C4 -0.003445f

float atan_t(float x) {
  bool neg = (x < 0);
  #ifdef WLED_DEBUG_MATH
  float xinput = x;
  #endif
  x = std::abs(x);
  float res;
  if (x > 5.0f) { //atan(x) converges to pi/2 - (1/x) for large values
    res = HALF_PI - (1.0f/x);
  }
  else if (x > 1.0f) { //1 < x < 5
    float xx = x * x;
    res = (C4*xx*xx)+(C3*xx*x)+(C2*xx)+(C1*x)+C0;
  } else { //this approximation is only for x <= 1
    float xx = x * x;
    res = ((A*xx + B)*xx + C)*x;
  }
  if (neg) res = -res;
  #ifdef WLED_DEBUG_MATH
  Serial.printf("atan,%f,%f,%f\n",xinput,res,atan(xinput));
  #endif
  return res;
}

float floor_t(float x) {
  bool neg = x < 0;
  int val = x;
  if (neg) val--;
  #ifdef WLED_DEBUG_MATH
  Serial.printf("floor: %f,%f\n",val,floor(x));
  #endif
  return val;
}

float fmod_t(float num, float denom) {
  int tquot = num / denom;
  float res = num - tquot * denom;
  #ifdef WLED_DEBUG_MATH
  Serial.printf("fmod: %f,%f\n",res,fmod(num,denom));
  #endif
  return res;
}

#endif