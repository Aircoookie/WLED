#pragma once

#include "wled.h"
#include <ANIMartRIX.h>

#warning WLEDMM usermod: CC BY-NC 3.0 licensed effects by Stefan Petrick, include this usermod only if you accept the terms!
//========================================================================================================================
//========================================================================================================================
//========================================================================================================================


// Polar basics demo for the 
// FastLED Podcast #2
// https://www.youtube.com/watch?v=KKjFRZFBUrQ
//
// VO.1 preview version
// by Stefan Petrick 2023
// This code is licenced under a 
// Creative Commons Attribution 
// License CC BY-NC 3.0

//based on: https://gist.github.com/StefanPetrick/9c091d9a28a902af5a7b540e40442c64

// class AnimartrixCore:public ANIMartRIX {
//   private:

//   public:
//     float runtime;                          // elapse ms since startup
//     // float newdist, newangle;                // parameters for image reconstruction
//     // float z;                                // 3rd dimension for the 3d noise function
//     // float offset_x, offset_y;               // wanna shift the cartesians during runtime?
//     // float scale_x, scale_y;                 // cartesian scaling in 2 dimensions
//     // float dist, angle;                      // the actual polar coordinates

//     // int x, y;                               // the cartesian coordiantes
//     // int num_x;// = WIDTH;                      // horizontal pixel count
//     // int num_y;// = HEIGHT;                     // vertical pixel count

//     float center_x;// = (num_x / 2) - 0.5;     // the reference point for polar coordinates
//     float center_y;// = (num_y / 2) - 0.5;     // (can also be outside of the actual xy matrix)
//     // //float center_x = 20;                  // the reference point for polar coordinates
//     // //float center_y = 20;                

//     // //WLEDMM: assign 32x32 fixed for the time being
//     // float theta   [60] [32];          // look-up table for all angles WLEDMM: 60x32 to support WLED Effects ledmaps
//     // float distance[60] [32];          // look-up table for all distances

//     // // std::vector<std::vector<float>> theta;          // look-up table for all angles
//     // // std::vector<std::vector<float>> distance;          // look-up table for all distances
//     // // std::vector<std::vector<float>> vignette;
//     // // std::vector<std::vector<float>> inverse_vignette;

//     // float spd;                            // can be used for animation speed manipulation during runtime

//     // float show1, show2, show3, show4, show5; // to save the rendered values of all animation layers
//     // float red, green, blue;                  // for the final RGB results after the colormapping

//     // float c, d, e, f;                                                   // factors for oscillators
//     // float linear_c, linear_d, linear_e, linear_f;                       // linear offsets
//     // float angle_c, angle_d, angle_e, angle_f;                           // angle offsets
//     // float noise_angle_c, noise_angle_d, noise_angle_e, noise_angle_f;   // angles based on linear noise travel
//     // float dir_c, dir_d, dir_e, dir_f;                                   // direction multiplicators

//   AnimartrixCore() {
//     USER_PRINTLN("AnimartrixCore constructor");
//   }
//   ~AnimartrixCore() {
//     USER_PRINTLN("AnimartrixCore destructor");
//   }

//   void init() {
//     num_x = SEGMENT.virtualWidth(); // horizontal pixel count
//     num_y = SEGMENT.virtualHeight(); // vertical pixel count
//     center_x = (num_x / 2) - 0.5;     // the reference point for polar coordinates
//     center_y = (num_y / 2) - 0.5;     // (can also be outside of the actual xy matrix)

//     //allocate memory for the 2D arrays
//     // theta.resize(num_x, std::vector<float>(num_y, 0));
//     // distance.resize(num_x, std::vector<float>(num_y, 0));
//     // vignette.resize(num_x, std::vector<float>(num_y, 0));
//     // inverse_vignette.resize(num_x, std::vector<float>(num_y, 0));

//     render_polar_lookup_table(center_x, center_y);          // precalculate all polar coordinates 
//                                         // to improve the framerate
//   }

//   void write_pixel_to_framebuffer(int x, int y, rgb &pixel) {
//     // the final color values shall not exceed 255 (to avoid flickering pixels caused by >255 = black...)
//     // negative values * -1 

//     rgb_sanity_check(pixel);

//     CRGB finalcolor = CRGB(pixel.red, pixel.green, pixel.blue);
  
//     // write the rendered pixel into the framebutter
//     SEGMENT.setPixelColorXY(x,y,finalcolor);
//   }

//   // Show the current framerate & rendered pixels per second in the serial monitor.

//   void report_performance() {
    
//     int fps = FastLED.getFPS();                 // frames per second
//     int kpps = (fps * SEGMENT.virtualLength()) / 1000;   // kilopixel per second

//     USER_PRINT(kpps); USER_PRINT(" kpps ... ");
//     USER_PRINT(fps); USER_PRINT(" fps @ ");
//     USER_PRINT(SEGMENT.virtualLength()); USER_PRINTLN(" LEDs ");
  
//   }
// };

// class PolarBasics:public AnimartrixCore {
//   private:

//   public:
//     // Background for setting the following 2 numbers: the FastLED inoise16() function returns
//     // raw values ranging from 0-65535. In order to improve contrast we filter this output and
//     // stretch the remains. In histogram (photography) terms this means setting a blackpoint and
//     // a whitepoint. low_limit MUST be smaller than high_limit.

//     uint16_t low_limit  = 30000;            // everything lower drawns in black
//                                             // higher numer = more black & more contrast present
//     uint16_t high_limit = 50000;            // everything higher gets maximum brightness & bleeds out
//                                             // lower number = the result will be more bright & shiny

//     // float vignette[60] [32];
//     // float inverse_vignette[60] [32];

//   PolarBasics() {
//     USER_PRINTLN("constructor");
//   }
//   ~PolarBasics() {
//     USER_PRINTLN("destructor");
//   }

//   // void speedratiosAndOscillators() {
//   //   // set speedratios for the offsets & oscillators
  
//   //   spd = 0.05  ;
//   //   c   = 0.013  ;
//   //   d   = 0.017   ;
//   //   e   = 0.2  ;
//   //   f   = 0.007  ;

//   //   low_limit  = 30000;
//   //   high_limit = 50000;

//   //   calculate_oscillators();     // get linear offsets and oscillators going
//   // }

//   void forLoop() {
//     // ...and now let's generate a frame 

//     for (int x = 0; x < num_x; x++) {
//       for (int y = 0; y < num_y; y++) {
//         // pick polar coordinates from look the up table 

//         dist  = distance [x] [y];
//         angle = theta    [y] [x];

//         // Generation of one layer. Explore the parameters and what they do.
    
//         scale_x  = 10000;                       // smaller value = zoom in, bigger structures, less detail
//         scale_y  = 10000;                       // higher = zoom out, more pixelated, more detail
//         z        = linear_c * SEGMENT.custom3;                           // must be >= 0
//         newangle = 5*SEGMENT.intensity/255 * angle + angle_c - 3 * SEGMENT.speed/255 * (dist/10*dir_c);
//         newdist  = dist;
//         offset_x = SEGMENT.custom1;                        // must be >=0
//         offset_y = SEGMENT.custom2;                        // must be >=0
        
//         show1 = render_pixel();

//         // newangle = 5*SEGMENT.intensity/255 * angle + angle_d - 3 * SEGMENT.speed/255 * (dist/10*dir_d);
//         // z        = linear_d * SEGMENT.custom3;                           // must be >= 0
//         // show2 = render_pixel();
                
//         // newangle = 5*SEGMENT.intensity/255 * angle + angle_e - 3 * SEGMENT.speed/255 * (dist/10*dir_e);
//         // z        = linear_e * SEGMENT.custom3;                           // must be >= 0
//         // show3 = render_pixel();

//         // Colormapping - Assign rendered values to colors 
        
//         rgb pixel;
//         pixel.red = show1;
//         pixel.green = show2;
//         pixel.blue = show3;
        
//         // Check the final results.
//         // Discard faulty RGB values & write the valid results into the framebuffer.
        
//         write_pixel_to_framebuffer(x, y, pixel);
//       }
//     }
//   }

// };


// // Circular Blobs
// //
// // VO.2 preview version
// // by Stefan Petrick 2023
// // This code is licenced under a 
// // Creative Commons Attribution 
// // License CC BY-NC 3.0
// //
// // In order to run this on your own setup you might want to check and change
// // line 22 & 23 according to your matrix size and 
// // line 75 to suit your LED interface type.
// //
// // In case you want to run this code on a different LED driver library
// // (like SmartMatrix, OctoWS2812, ESP32 16x parallel output) you will need to change
// // line 52 to your own framebuffer and line 276+279 to your own setcolor function.
// // In line 154 the framebuffer gets pushed to the LEDs.
// // The whole report_performance function you can just comment out. It gets called
// // in line 157.
// //
// // With this adaptions it should be easy to use this code with 
// // any given LED driver & interface you might prefer.

// //based on https://gist.github.com/StefanPetrick/35ffd8467df22a77067545cfb889aa4f
// //and Fastled podcast nr 3: https://www.youtube.com/watch?v=3tfjP7GJnZo

// class CircularBlobs:public AnimartrixCore {
//   private:

//     float fade(float t){ return t * t * t * (t * (t * 6 - 15) + 10); }
//     float lerp(float t, float a, float b){ return a + t * (b - a); }
//     float grad(int hash, float x, float y, float z)
//     {
//       int    h = hash & 15;          /* CONVERT LO 4 BITS OF HASH CODE */
//       float  u = h < 8 ? x : y,      /* INTO 12 GRADIENT DIRECTIONS.   */
//                 v = h < 4 ? y : h==12||h==14 ? x : z;
//       return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
//     }

//     #define P(x) p[(x) & 255]

//     float pnoise(float x, float y, float z) {
//       int   X = (int)floorf(x) & 255,             /* FIND UNIT CUBE THAT */
//             Y = (int)floorf(y) & 255,             /* CONTAINS POINT.     */
//             Z = (int)floorf(z) & 255;
//       x -= floorf(x);                             /* FIND RELATIVE X,Y,Z */
//       y -= floorf(y);                             /* OF POINT IN CUBE.   */
//       z -= floorf(z);
//       float  u = fade(x),                         /* COMPUTE FADE CURVES */
//             v = fade(y),                         /* FOR EACH OF X,Y,Z.  */
//             w = fade(z);
//       int  A = P(X)+Y, 
//           AA = P(A)+Z, 
//           AB = P(A+1)+Z,                         /* HASH COORDINATES OF */
//           B = P(X+1)+Y, 
//           BA = P(B)+Z, 
//           BB = P(B+1)+Z;                         /* THE 8 CUBE CORNERS, */

//       return lerp(w,lerp(v,lerp(u, grad(P(AA  ), x, y, z),    /* AND ADD */
//                                 grad(P(BA  ), x-1, y, z)),    /* BLENDED */
//                     lerp(u, grad(P(AB  ), x, y-1, z),         /* RESULTS */
//                         grad(P(BB  ), x-1, y-1, z))),        /* FROM  8 */
//                   lerp(v, lerp(u, grad(P(AA+1), x, y, z-1),   /* CORNERS */
//                       grad(P(BA+1), x-1, y, z-1)),           /* OF CUBE */
//                     lerp(u, grad(P(AB+1), x, y-1, z-1),
//                         grad(P(BB+1), x-1, y-1, z-1))));
//     }


//   public:

//   // Background for setting the following 2 numbers: the pnoise() function returns
//   // raw values ranging from -1 to +1. In order to improve contrast we filter this output and
//   // stretch the remains. In histogram (photography) terms this means setting a blackpoint and
//   // a whitepoint. low_limit MUST be smaller than high_limit.
//   float low_limit  = 0;            // everything lower drawns in black
//                                   // higher numer = more black & more contrast present
//   float high_limit = 0.5;          // everything higher gets maximum brightness & bleeds out
//                                   // lower number = the result will be more bright & shiny
//   float offset_z;     // wanna shift the cartesians during runtime?
//   float scale_z;        // cartesian scaling in 3 dimensions

//   // void speedratiosAndOscillators() {

//   //   // set speedratios for the offsets & oscillators
  
//   //   spd = 0.001  ;               // higher = faster
//   //   c   = 0.05  ;
//   //   d   = 0.07   ;
//   //   e   = 0.09  ;
//   //   f   = 0.01  ;

//   //   low_limit = 0;
//   //   high_limit = 0.5;

//   //   calculate_oscillators();     // get linear offsets and oscillators going
//   // }

//   void forLoop() {
//     // ...and now let's generate a frame 

//     for (int x = 0; x < num_x; x++) {
//       for (int y = 0; y < num_y; y++) {

//         dist     = distance[x][y];  // pick precalculated polar data     
//         angle    =    theta[x][y];
        
//         // define first animation layer
//         scale_x  = 0.11;            // smaller value = zoom in
//         scale_y  = 0.1;             // higher = zoom out
//         scale_z  = 0.1;
      
//         newangle = angle + 5*SEGMENT.speed/255 * noise_angle_c + 5*SEGMENT.speed/255 * noise_angle_f;
//         newdist  = 5*SEGMENT.intensity/255 * dist;
//         offset_z = linear_c * 100;
//         z        = -5 * sqrtf(dist) ;
//         show1    = render_pixel_faster();

//         // repeat for the 2nd layer, every parameter you don't change stays as it was set 
//         // in the previous layer.

//         offset_z = linear_d * 100;
//         newangle = angle + 5*SEGMENT.speed/255 * noise_angle_d + 5*SEGMENT.speed/255 * noise_angle_f;
//         show2    = render_pixel_faster();

//         // 3d layer

//         offset_z = linear_e*100;
//         newangle = angle + 5*SEGMENT.speed/255 * noise_angle_e + 5*SEGMENT.speed/255 * noise_angle_f;
//         show3 = render_pixel_faster();

//         // create some interference between the layers

//         show3 = show3-show2-show1;
//         if (show3 < 0) show3 = 0;
                
//         // Colormapping - Assign rendered values to colors 
        
//         rgb pixel;
//         pixel.red   = show1-show2/2;
//         if (pixel.red < 0) pixel.red=0;
//         pixel.green = (show1-show2)/2;
//         if (pixel.green < 0) pixel.green=0;
//         pixel.blue  = show3-show1/2;
//         if (pixel.blue < 0) pixel.blue=0;
        
//         // Check the final results and store them.
//         // Discard faulty RGB values & write the remaining valid results into the framebuffer.
        
//         write_pixel_to_framebuffer(x, y, pixel);
//       }
//     }
//   }

  
// };

// //effect functions
// uint16_t mode_PolarBasics(void) { 

//   PolarBasics* spe;


//   if(!SEGENV.allocateData(sizeof(PolarBasics))) {SEGMENT.fill(SEGCOLOR(0)); return 350;} //mode_static(); //allocation failed

//   spe = reinterpret_cast<PolarBasics*>(SEGENV.data);

//   //first time init
//   if (SEGENV.call == 0) {

//     USER_PRINTF("mode_PolarBasics %d\n", sizeof(PolarBasics));
//     //  if (SEGENV.call == 0) SEGMENT.setUpLeds();

//     spe->init();

//     // spe->render_vignette_table(9.5);           // the number is the desired radius in pixel
//                                         // WIDTH/2 generates a circle
//   }

//   // spe->speedratiosAndOscillators();

//   spe->forLoop();

//   // FastLED.show();

//   // EVERY_N_MILLIS(500) spe->report_performance();

//   return FRAMETIME;
// }
// static const char _data_FX_mode_PolarBasics[] PROGMEM = "💡Polar Basics ☾@AngleDist,AngleMult;;!;2;sx=0,ix=51,c1=0,c2=0,c3=0";


// uint16_t mode_CircularBlobs(void) { 
//   CircularBlobs* spe;


//   if(!SEGENV.allocateData(sizeof(CircularBlobs))) {SEGMENT.fill(SEGCOLOR(0)); return 350;} //mode_static(); //allocation failed

//   spe = reinterpret_cast<CircularBlobs*>(SEGENV.data);

//   //first time init
//   if (SEGENV.call == 0) {

//     USER_PRINTF("mode_CircularBlobs %d\n", sizeof(CircularBlobs));
//     //  if (SEGENV.call == 0) SEGMENT.setUpLeds();

//     spe->init();

//   }

//   // spe->speedratiosAndOscillators();

//   spe->forLoop();

//   // FastLED.show();

//   // EVERY_N_MILLIS(500) spe->report_performance();

//   return FRAMETIME;
// }
static const char _data_FX_mode_CircularBlobs[] PROGMEM = "💡CircularBlobs ☾@AngleDist,AngleMult;;!;2;sx=51,ix=51,c1=0,c2=0,c3=0";

static const char _data_FX_mode_Module_Experiment10[] PROGMEM = "💡Module_Experiment10 ☾";
static const char _data_FX_mode_Module_Experiment9[] PROGMEM = "💡Module_Experiment9 ☾";
static const char _data_FX_mode_Module_Experiment8[] PROGMEM = "💡Module_Experiment8 ☾";
static const char _data_FX_mode_Module_Experiment7[] PROGMEM = "💡Module_Experiment7 ☾";
static const char _data_FX_mode_Module_Experiment6[] PROGMEM = "💡Module_Experiment6 ☾";
static const char _data_FX_mode_Module_Experiment5[] PROGMEM = "💡Module_Experiment5 ☾";
static const char _data_FX_mode_Module_Experiment4[] PROGMEM = "💡Module_Experiment4 ☾";
static const char _data_FX_mode_Zoom2[] PROGMEM = "💡Zoom2 ☾";
static const char _data_FX_mode_Module_Experiment3[] PROGMEM = "💡Module_Experiment3 ☾";
static const char _data_FX_mode_Module_Experiment2[] PROGMEM = "💡Module_Experiment2 ☾";
static const char _data_FX_mode_Module_Experiment1[] PROGMEM = "💡Module_Experiment1 ☾";
static const char _data_FX_mode_Parametric_Water[] PROGMEM = "💡Parametric_Water ☾";
static const char _data_FX_mode_Water[] PROGMEM = "💡Water ☾";
static const char _data_FX_mode_Complex_Kaleido_6[] PROGMEM = "💡Complex_Kaleido_6 ☾";
static const char _data_FX_mode_Complex_Kaleido_5[] PROGMEM = "💡Complex_Kaleido_5 ☾";
static const char _data_FX_mode_Complex_Kaleido_4[] PROGMEM = "💡Complex_Kaleido_4 ☾";
static const char _data_FX_mode_Complex_Kaleido_3[] PROGMEM = "💡Complex_Kaleido_3 ☾";
static const char _data_FX_mode_Complex_Kaleido_2[] PROGMEM = "💡Complex_Kaleido_2 ☾";
static const char _data_FX_mode_Complex_Kaleido[] PROGMEM = "💡Complex_Kaleido ☾";
static const char _data_FX_mode_SM10[] PROGMEM = "💡SM10 ☾";
static const char _data_FX_mode_SM9[] PROGMEM = "💡SM9 ☾";
static const char _data_FX_mode_SM8[] PROGMEM = "💡SM8 ☾";
static const char _data_FX_mode_SM7[] PROGMEM = "💡SM7 ☾";
static const char _data_FX_mode_SM6[] PROGMEM = "💡SM6 ☾";
static const char _data_FX_mode_SM5[] PROGMEM = "💡SM5 ☾";
static const char _data_FX_mode_SM4[] PROGMEM = "💡SM4 ☾";
static const char _data_FX_mode_SM3[] PROGMEM = "💡SM3 ☾";
static const char _data_FX_mode_SM2[] PROGMEM = "💡SM2 ☾";
static const char _data_FX_mode_SM1[] PROGMEM = "💡SM1 ☾";
static const char _data_FX_mode_Big_Caleido[] PROGMEM = "💡Big_Caleido (Direct leds) ☾";
static const char _data_FX_mode_RGB_Blobs5[] PROGMEM = "💡RGB_Blobs5 ☾";
static const char _data_FX_mode_RGB_Blobs4[] PROGMEM = "💡RGB_Blobs4 ☾";
static const char _data_FX_mode_RGB_Blobs3[] PROGMEM = "💡RGB_Blobs3 ☾";
static const char _data_FX_mode_RGB_Blobs2[] PROGMEM = "💡RGB_Blobs2 ☾";
static const char _data_FX_mode_RGB_Blobs[] PROGMEM = "💡RGB_Blobs ☾";
static const char _data_FX_mode_Polar_Waves[] PROGMEM = "💡Polar_Waves ☾";
static const char _data_FX_mode_Slow_Fade[] PROGMEM = "💡Slow_Fade ☾";
static const char _data_FX_mode_Zoom[] PROGMEM = "💡Zoom ☾";
static const char _data_FX_mode_Hot_Blob[] PROGMEM = "💡Hot_Blob ☾";
static const char _data_FX_mode_Spiralus2[] PROGMEM = "💡Spiralus2 ☾";
static const char _data_FX_mode_Spiralus[] PROGMEM = "💡Spiralus ☾";
static const char _data_FX_mode_Yves[] PROGMEM = "💡Yves ☾";
static const char _data_FX_mode_Scaledemo1[] PROGMEM = "💡Scaledemo1 ☾";
static const char _data_FX_mode_Lava1[] PROGMEM = "💡Lava1 ☾";
static const char _data_FX_mode_Caleido3[] PROGMEM = "💡Caleido3 ☾";
static const char _data_FX_mode_Caleido2[] PROGMEM = "💡Caleido2 ☾";
static const char _data_FX_mode_Caleido1[] PROGMEM = "💡Caleido1 ☾";
static const char _data_FX_mode_Distance_Experiment[] PROGMEM = "💡Distance_Experiment ☾";
static const char _data_FX_mode_Center_Field[] PROGMEM = "💡Center_Field ☾";
static const char _data_FX_mode_Waves[] PROGMEM = "💡Waves ☾";
static const char _data_FX_mode_Chasing_Spirals[] PROGMEM = "💡Chasing_Spirals ☾";
static const char _data_FX_mode_Rotating_Blob[] PROGMEM = "💡Rotating_Blob ☾";


class ANIMartRIXMod:public ANIMartRIX {
	public:
	void output() {
		for(int x = 0; x < num_x; x++) {
			for(int y = 0; y < num_y; y++) {
				SEGMENT.setPixelColorXY(x,y, buffer[xy(x,y)]);
			}
		}
	}
};
ANIMartRIXMod anim;

uint16_t mode_Module_Experiment10() { 
	anim.Module_Experiment10();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Module_Experiment9() { 
	anim.Module_Experiment9();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Module_Experiment8() { 
	anim.Module_Experiment8();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Module_Experiment7() { 
	anim.Module_Experiment7();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Module_Experiment6() { 
	anim.Module_Experiment6();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Module_Experiment5() { 
	anim.Module_Experiment5();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Module_Experiment4() { 
	anim.Module_Experiment4();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Zoom2() { 
	anim.Zoom2();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Module_Experiment3() { 
	anim.Module_Experiment3();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Module_Experiment2() { 
	anim.Module_Experiment2();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Module_Experiment1() { 
	anim.Module_Experiment1();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Parametric_Water() { 
	anim.Parametric_Water();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Water() { 
	anim.Water();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Complex_Kaleido_6() { 
	anim.Complex_Kaleido_6();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Complex_Kaleido_5() { 
	anim.Complex_Kaleido_5();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Complex_Kaleido_4() { 
	anim.Complex_Kaleido_4();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Complex_Kaleido_3() { 
	anim.Complex_Kaleido_3();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Complex_Kaleido_2() { 
	anim.Complex_Kaleido_2();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Complex_Kaleido() { 
	anim.Complex_Kaleido();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_SM10() { 
	anim.SM10();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_SM9() { 
	anim.SM9();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_SM8() { 
	anim.SM8();
	anim.output();
	return FRAMETIME;
}
// uint16_t mode_SM7() { 
// 	art.SM7();
//	art.output();
//	return FRAMETIME;
// }
uint16_t mode_SM6() { 
	anim.SM6();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_SM5() { 
	anim.SM5();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_SM4() { 
	anim.SM4();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_SM3() { 
	anim.SM3();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_SM2() { 
	anim.SM2();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_SM1() { 
	anim.SM1();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Big_Caleido() { 
	if (SEGENV.call == 0) {
		SEGMENT.setUpLeds();  //lossless getPixelColor()
    	SEGMENT.fill(BLACK);
		anim.setBuffer(SEGMENT.leds);
	}
	anim.Big_Caleido();
	SEGMENT.fadeToBlackBy(0);
//	anim.output();
	return FRAMETIME;
}
uint16_t mode_RGB_Blobs5() { 
	anim.RGB_Blobs5();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_RGB_Blobs4() { 
	anim.RGB_Blobs4();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_RGB_Blobs3() { 
	anim.RGB_Blobs3();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_RGB_Blobs2() { 
	anim.RGB_Blobs2();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_RGB_Blobs() { 
	anim.RGB_Blobs();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Polar_Waves() { 
	anim.Polar_Waves();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Slow_Fade() { 
	anim.Slow_Fade();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Zoom() { 
	anim.Zoom();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Hot_Blob() { 
	anim.Hot_Blob();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Spiralus2() { 
	anim.Spiralus2();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Spiralus() { 
	anim.Spiralus();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Yves() { 
	anim.Yves();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Scaledemo1() { 
	anim.Scaledemo1();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Lava1() { 
	anim.Lava1();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Caleido3() { 
	anim.Caleido3();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Caleido2() { 
	anim.Caleido2();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Caleido1() { 
	anim.Caleido1();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Distance_Experiment() { 
	anim.Distance_Experiment();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Center_Field() { 
	anim.Center_Field();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Waves() { 
	anim.Waves();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Chasing_Spirals() { 
	anim.Chasing_Spirals();
	anim.output();
	return FRAMETIME;
}
uint16_t mode_Rotating_Blob() { 
	anim.Rotating_Blob();
	anim.output();
	return FRAMETIME;
}


class AnimartrixUsermod : public Usermod {

  public:

    AnimartrixUsermod(const char *name, bool enabled):Usermod(name, enabled) {} //WLEDMM
	CRGB buffer[256];

    void setup() {

      bool serpentine = false;
      anim.init(SEGMENT.virtualWidth(), SEGMENT.virtualHeight(), buffer, serpentine);

      // strip.addEffect(255, &mode_PolarBasics, _data_FX_mode_PolarBasics);
      // strip.addEffect(255, &mode_CircularBlobs, _data_FX_mode_CircularBlobs);
      strip.addEffect(255, &mode_Module_Experiment10, _data_FX_mode_Module_Experiment10);
      strip.addEffect(255, &mode_Module_Experiment9, _data_FX_mode_Module_Experiment9);
      strip.addEffect(255, &mode_Module_Experiment8, _data_FX_mode_Module_Experiment8);
      strip.addEffect(255, &mode_Module_Experiment7, _data_FX_mode_Module_Experiment7);
      strip.addEffect(255, &mode_Module_Experiment6, _data_FX_mode_Module_Experiment6);
      strip.addEffect(255, &mode_Module_Experiment5, _data_FX_mode_Module_Experiment5);
      strip.addEffect(255, &mode_Module_Experiment4, _data_FX_mode_Module_Experiment4);
      strip.addEffect(255, &mode_Zoom2, _data_FX_mode_Zoom2);
      strip.addEffect(255, &mode_Module_Experiment3, _data_FX_mode_Module_Experiment3);
      strip.addEffect(255, &mode_Module_Experiment2, _data_FX_mode_Module_Experiment2);
      strip.addEffect(255, &mode_Module_Experiment1, _data_FX_mode_Module_Experiment1);
      strip.addEffect(255, &mode_Parametric_Water, _data_FX_mode_Parametric_Water);
      strip.addEffect(255, &mode_Water, _data_FX_mode_Water);
      strip.addEffect(255, &mode_Complex_Kaleido_6, _data_FX_mode_Complex_Kaleido_6);
      strip.addEffect(255, &mode_Complex_Kaleido_5, _data_FX_mode_Complex_Kaleido_5);
      strip.addEffect(255, &mode_Complex_Kaleido_4, _data_FX_mode_Complex_Kaleido_4);
      strip.addEffect(255, &mode_Complex_Kaleido_3, _data_FX_mode_Complex_Kaleido_3);
      strip.addEffect(255, &mode_Complex_Kaleido_2, _data_FX_mode_Complex_Kaleido_2);
      strip.addEffect(255, &mode_Complex_Kaleido, _data_FX_mode_Complex_Kaleido);
      strip.addEffect(255, &mode_SM10, _data_FX_mode_SM10);
      strip.addEffect(255, &mode_SM9, _data_FX_mode_SM9);
      strip.addEffect(255, &mode_SM8, _data_FX_mode_SM8);
      // strip.addEffect(255, &mode_SM7, _data_FX_mode_SM7);
      strip.addEffect(255, &mode_SM6, _data_FX_mode_SM6);
      strip.addEffect(255, &mode_SM5, _data_FX_mode_SM5);
      strip.addEffect(255, &mode_SM4, _data_FX_mode_SM4);
      strip.addEffect(255, &mode_SM3, _data_FX_mode_SM3);
      strip.addEffect(255, &mode_SM2, _data_FX_mode_SM2);
      strip.addEffect(255, &mode_SM1, _data_FX_mode_SM1);
      strip.addEffect(255, &mode_Big_Caleido, _data_FX_mode_Big_Caleido);
      strip.addEffect(255, &mode_RGB_Blobs5, _data_FX_mode_RGB_Blobs5);
      strip.addEffect(255, &mode_RGB_Blobs4, _data_FX_mode_RGB_Blobs4);
      strip.addEffect(255, &mode_RGB_Blobs3, _data_FX_mode_RGB_Blobs3);
      strip.addEffect(255, &mode_RGB_Blobs2, _data_FX_mode_RGB_Blobs2);
      strip.addEffect(255, &mode_RGB_Blobs, _data_FX_mode_RGB_Blobs);
      strip.addEffect(255, &mode_Polar_Waves, _data_FX_mode_Polar_Waves);
      strip.addEffect(255, &mode_Slow_Fade, _data_FX_mode_Slow_Fade);
      strip.addEffect(255, &mode_Zoom, _data_FX_mode_Zoom);
      strip.addEffect(255, &mode_Hot_Blob, _data_FX_mode_Hot_Blob);
      strip.addEffect(255, &mode_Spiralus2, _data_FX_mode_Spiralus2);
      strip.addEffect(255, &mode_Spiralus, _data_FX_mode_Spiralus);
      strip.addEffect(255, &mode_Yves, _data_FX_mode_Yves);
      strip.addEffect(255, &mode_Scaledemo1, _data_FX_mode_Scaledemo1);
      strip.addEffect(255, &mode_Lava1, _data_FX_mode_Lava1);
      strip.addEffect(255, &mode_Caleido3, _data_FX_mode_Caleido3);
      strip.addEffect(255, &mode_Caleido2, _data_FX_mode_Caleido2);
      strip.addEffect(255, &mode_Caleido1, _data_FX_mode_Caleido1);
      strip.addEffect(255, &mode_Distance_Experiment, _data_FX_mode_Distance_Experiment);
      strip.addEffect(255, &mode_Center_Field, _data_FX_mode_Center_Field);
      strip.addEffect(255, &mode_Waves, _data_FX_mode_Waves);
      strip.addEffect(255, &mode_Chasing_Spirals, _data_FX_mode_Chasing_Spirals);
      strip.addEffect(255, &mode_Rotating_Blob, _data_FX_mode_Rotating_Blob);

      initDone = true;
    }

    void loop() {
      if (!enabled || strip.isUpdating()) return;

      // do your magic here
      if (millis() - lastTime > 1000) {
        //USER_PRINTLN("I'm alive!");
        lastTime = millis();
      }
    }

    void addToJsonInfo(JsonObject& root)
    {
      char myStringBuffer[16]; // buffer for snprintf()
      JsonObject user = root["u"];
      if (user.isNull()) user = root.createNestedObject("u");

      JsonArray infoArr = user.createNestedArray(FPSTR(_name));

      String uiDomString = F("Animartrix requires the Creative Commons Attribution License CC BY-NC 3.0");
      infoArr.add(uiDomString);
	}

    uint16_t getId()
    {
      return USERMOD_ID_ANIMARTRIX;
    }

};



