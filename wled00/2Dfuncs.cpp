/* 2D support functions. Shamelessly stolen from Stefan Petricks most excellent code and adapted to WLED
 *  
 *  XY          translate 2 dimensional coordinates into index
 *  Line        draw a line
 *  Pixel       draw a pixel (not sure if we really need that)
 *  ClearAll    Clear the screenbuffer
 */

/* required external variables, currently defined here until we find a way to integrate into UI
 *  
 *  boolean ZigZig                  is the array set up in a zigzag way or not?
 *  
 */

#include "FX.h"

#define MAX_X 32
#define MAX_Y 32

uint8_t noise[MAX_X][MAX_Y];

// Scale determines how far apart the pixels in our noise matrix are.  Try
// changing these values around to see how it affects the motion of the display.  The
// higher the value of scale, the more "zoomed out" the noise iwll be.  A value
// of 1 will be so zoomed in, you'll mostly see solid colors.
uint16_t scale = 30; // scale is set dynamically once we've started up

uint8_t ZigZag = 0;                 // 0 if we have a ZigZag array, anything else if we have an array where all led strips go in the same direction
uint16_t Height = 32;
uint16_t Width = 8;
uint16_t x;
uint16_t y;
uint16_t z;
uint8_t colorLoop = 1;


// translates from x, y into an index into the LED array and
// finds the right index for a S shaped matrix
//
int XY(int x, int y) { 
  if(y > Height) { 
    y = Height; 
  }
  if(y < 0) { 
    y = 0; 
  }
  if(x > Width) { 
    x = Width;
  } 
  if(x < 0) { 
    x = 0; 
  }
  if (ZigZag) {
    // for a serpentine layout reverse every 2nd row:
    if(x % 2 == 1) {  
      return (x * (Width) + (Height - y -1)); 
    } 
    else { 
      // use that line only, if you have all rows beginning at the same side
      return (x * (Width) + y); 
    }
  }
  return (x * (Width) + y); 
}


void fillnoise8(uint8_t speed) {
  // If we're runing at a low "speed", some 8-bit artifacts become visible
  // from frame-to-frame.  In order to reduce this, we can do some fast data-smoothing.
  // The amount of data smoothing we're doing depends on "speed".
  uint8_t dataSmoothing = 0;
  if( speed < 50) {
    dataSmoothing = 200 - (speed * 4);
  }
  
  for(int i = 0; i < MAX_X; i++) {
    int ioffset = scale * i;
    for(int j = 0; j < MAX_Y; j++) {
      int joffset = scale * j;
      
      uint8_t data = inoise8(x + ioffset,y + joffset,z);

      // The range of the inoise8 function is roughly 16-238.
      // These two operations expand those values out to roughly 0..255
      // You can comment them out if you want the raw noise data.
      data = qsub8(data,16);
      data = qadd8(data,scale8(data,39));

      if( dataSmoothing ) {
        uint8_t olddata = noise[i][j];
        uint8_t newdata = scale8( olddata, dataSmoothing) + scale8( data, 256 - dataSmoothing);
        data = newdata;
      }
      
      noise[i][j] = data;
    }
  }
  
  z += speed;
  
  // apply slow drift to X and Y, just for visual variation.
  x += speed / 8;
  y -= speed / 16;
}

void mapNoiseToLEDsUsingPalette()
{
  static uint8_t ihue=0;
  
  for(int i = 0; i < MAX_X; i++) {
    for(int j = 0; j < MAX_Y; j++) {
      // We use the value at the (i,j) coordinate in the noise
      // array for our brightness, and the flipped value from (j,i)
      // for our pixel's index into the color palette.

      uint8_t index = noise[j][i];
      uint8_t bri =   noise[i][j];

      // if this palette is a 'loop', add a slowly-changing base value
      if( colorLoop) { 
        index += ihue;
      }

      // brighten up, as the color palette itself often contains the 
      // light/dark dynamic range desired
      if( bri > 127 ) {
        bri = 255;
      } else {
        bri = dim8_raw( bri * 2);
      }

      CRGB color = ColorFromPalette( currentPalette, index, bri);
      leds[XY(i,j)] = color;
    }
  }
  
  ihue+=1;
}
