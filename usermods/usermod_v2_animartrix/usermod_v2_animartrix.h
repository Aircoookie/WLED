#pragma once

#include "wled.h"

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

class AnimartrixCore {
  private:

  public:
    float runtime;                          // elapse ms since startup
    float newdist, newangle;                // parameters for image reconstruction
    float z;                                // 3rd dimension for the 3d noise function
    float offset_x, offset_y;               // wanna shift the cartesians during runtime?
    float scale_x, scale_y;                 // cartesian scaling in 2 dimensions
    float dist, angle;                      // the actual polar coordinates

    int x, y;                               // the cartesian coordiantes
    int num_x;// = WIDTH;                      // horizontal pixel count
    int num_y;// = HEIGHT;                     // vertical pixel count

    float center_x;// = (num_x / 2) - 0.5;     // the reference point for polar coordinates
    float center_y;// = (num_y / 2) - 0.5;     // (can also be outside of the actual xy matrix)
    //float center_x = 20;                  // the reference point for polar coordinates
    //float center_y = 20;                

    //WLEDMM: assign 32x32 fixed for the time being
    float theta   [60] [32];          // look-up table for all angles WLEDMM: 60x32 to support WLED Effects ledmaps
    float distance[60] [32];          // look-up table for all distances

    // std::vector<std::vector<float>> theta;          // look-up table for all angles
    // std::vector<std::vector<float>> distance;          // look-up table for all distances
    // std::vector<std::vector<float>> vignette;
    // std::vector<std::vector<float>> inverse_vignette;

    float spd;                            // can be used for animation speed manipulation during runtime

    float show1, show2, show3, show4, show5; // to save the rendered values of all animation layers
    float red, green, blue;                  // for the final RGB results after the colormapping

    float c, d, e, f;                                                   // factors for oscillators
    float linear_c, linear_d, linear_e, linear_f;                       // linear offsets
    float angle_c, angle_d, angle_e, angle_f;                           // angle offsets
    float noise_angle_c, noise_angle_d, noise_angle_e, noise_angle_f;   // angles based on linear noise travel
    float dir_c, dir_d, dir_e, dir_f;                                   // direction multiplicators

  AnimartrixCore() {
    USER_PRINTLN("AnimartrixCore constructor");
  }
  ~AnimartrixCore() {
    USER_PRINTLN("AnimartrixCore destructor");
  }

  void init() {
    num_x = SEGMENT.virtualWidth(); // horizontal pixel count
    num_y = SEGMENT.virtualHeight(); // vertical pixel count
    center_x = (num_x / 2) - 0.5;     // the reference point for polar coordinates
    center_y = (num_y / 2) - 0.5;     // (can also be outside of the actual xy matrix)

    //allocate memory for the 2D arrays
    // theta.resize(num_x, std::vector<float>(num_y, 0));
    // distance.resize(num_x, std::vector<float>(num_y, 0));
    // vignette.resize(num_x, std::vector<float>(num_y, 0));
    // inverse_vignette.resize(num_x, std::vector<float>(num_y, 0));

    render_polar_lookup_table();          // precalculate all polar coordinates 
                                        // to improve the framerate
  }

  void calculate_oscillators() {
    
    runtime = millis();                          // save elapsed ms since start up

    runtime = runtime * spd;                     // global anaimation speed

    linear_c = runtime * c;                      // some linear rising offsets 0 to max
    linear_d = runtime * d;
    linear_e = runtime * e;
    linear_f = runtime * f;

    angle_c = fmodf(linear_c, 2 * PI);           // some cyclic angle offsets  0 to 2*PI
    angle_d = fmodf(linear_d, 2 * PI);
    angle_e = fmodf(linear_e, 2 * PI);
    angle_f = fmodf(linear_f, 2 * PI);

    dir_c = sinf(angle_c);                       // some direction oscillators -1 to 1
    dir_d = sinf(angle_d);
    dir_e = sinf(angle_e);
    dir_f = sinf(angle_f);
  }

  // given a static polar origin we can precalculate 
  // all the (expensive) polar coordinates

  void render_polar_lookup_table() {

    for (int xx = 0; xx < num_x; xx++) {
      for (int yy = 0; yy < num_y; yy++) {

          float dx = xx - center_x;
          float dy = yy - center_y;

        distance[xx] [yy] = hypotf(dx, dy);
        theta[xx] [yy]    = atan2f(dy, dx);
        
      }
    }
  }

  // float mapping maintaining 32 bit precision
  // we keep values with high resolution for potential later usage

  float map_float(float x, float in_min, float in_max, float out_min, float out_max) { 
    
    float result = (x-in_min) * (out_max-out_min) / (in_max-in_min) + out_min;
    if (result < out_min) result = out_min;
    if( result > out_max) result = out_max;

    return result; 
  }

  // Avoid any possible color flicker by forcing the raw RGB values to be 0-255.
  // This enables to play freely with random equations for the colormapping
  // without causing flicker by accidentally missing the valid target range.

  void rgb_sanity_check() {
    // rescue data if possible: when negative return absolute value
    if (red < 0)     red = abs(red);
    if (green < 0) green = abs(green);
    if (blue < 0)   blue = abs(blue);
    
    // discard everything above the valid 0-255 range
    if (red   > 255)   red = 255;
    if (green > 255) green = 255;
    if (blue  > 255)  blue = 255;
  }

  void write_pixel_to_framebuffer() {
    // the final color values shall not exceed 255 (to avoid flickering pixels caused by >255 = black...)
    // negative values * -1 

    rgb_sanity_check();

    CRGB finalcolor = CRGB(red, green, blue);
  
    // write the rendered pixel into the framebutter
    SEGMENT.setPixelColorXY(x,y,finalcolor);
  }

  // Show the current framerate & rendered pixels per second in the serial monitor.

  void report_performance() {
    
    int fps = FastLED.getFPS();                 // frames per second
    int kpps = (fps * SEGMENT.virtualLength()) / 1000;   // kilopixel per second

    USER_PRINT(kpps); USER_PRINT(" kpps ... ");
    USER_PRINT(fps); USER_PRINT(" fps @ ");
    USER_PRINT(SEGMENT.virtualLength()); USER_PRINTLN(" LEDs ");
  
  }
};

class PolarBasics:public AnimartrixCore {
  private:

  public:
    // Background for setting the following 2 numbers: the FastLED inoise16() function returns
    // raw values ranging from 0-65535. In order to improve contrast we filter this output and
    // stretch the remains. In histogram (photography) terms this means setting a blackpoint and
    // a whitepoint. low_limit MUST be smaller than high_limit.

    uint16_t low_limit  = 30000;            // everything lower drawns in black
                                            // higher numer = more black & more contrast present
    uint16_t high_limit = 50000;            // everything higher gets maximum brightness & bleeds out
                                            // lower number = the result will be more bright & shiny

    // float vignette[60] [32];
    // float inverse_vignette[60] [32];

  PolarBasics() {
    USER_PRINTLN("constructor");
  }
  ~PolarBasics() {
    USER_PRINTLN("destructor");
  }

  void speedratiosAndOscillators() {
    // set speedratios for the offsets & oscillators
  
    spd = 0.05  ;
    c   = 0.013  ;
    d   = 0.017   ;
    e   = 0.2  ;
    f   = 0.007  ;

    low_limit  = 30000;
    high_limit = 50000;

    calculate_oscillators();     // get linear offsets and oscillators going
  }

  void forLoop() {
    // ...and now let's generate a frame 

    for (x = 0; x < num_x; x++) {
      for (y = 0; y < num_y; y++) {
        // pick polar coordinates from look the up table 

        dist  = distance [x] [y];
        angle = theta    [y] [x];

        // Generation of one layer. Explore the parameters and what they do.
    
        scale_x  = 10000;                       // smaller value = zoom in, bigger structures, less detail
        scale_y  = 10000;                       // higher = zoom out, more pixelated, more detail
        z        = linear_c * SEGMENT.custom3;                           // must be >= 0
        newangle = 5*SEGMENT.intensity/255 * angle + angle_c - 3 * SEGMENT.speed/255 * (dist/10*dir_c);
        newdist  = dist;
        offset_x = SEGMENT.custom1;                        // must be >=0
        offset_y = SEGMENT.custom2;                        // must be >=0
        
        show1 = render_pixel();

        // newangle = 5*SEGMENT.intensity/255 * angle + angle_d - 3 * SEGMENT.speed/255 * (dist/10*dir_d);
        // z        = linear_d * SEGMENT.custom3;                           // must be >= 0
        // show2 = render_pixel();
                
        // newangle = 5*SEGMENT.intensity/255 * angle + angle_e - 3 * SEGMENT.speed/255 * (dist/10*dir_e);
        // z        = linear_e * SEGMENT.custom3;                           // must be >= 0
        // show3 = render_pixel();

        // Colormapping - Assign rendered values to colors 
        
        red   = show1;
        green = show2;
        blue  = show3;

        // Check the final results.
        // Discard faulty RGB values & write the valid results into the framebuffer.
        
        write_pixel_to_framebuffer();
      }
    }
  }

  void calculate_oscillators() {

    AnimartrixCore::calculate_oscillators();    

    uint16_t noi;
    noi =  inoise16(10000 + linear_c * 100000);    // some noise controlled angular offsets
    noise_angle_c = map_float(noi, 0, 65535 , 0, 4*PI);
    noi =  inoise16(20000 + linear_d * 100000);
    noise_angle_d = map_float(noi, 0, 65535 , 0, 4*PI);
    noi =  inoise16(30000 + linear_e * 100000);
    noise_angle_e = map_float(noi, 0, 65535 , 0, 4*PI);
    noi =  inoise16(40000 + linear_f * 100000);
    noise_angle_f = map_float(noi, 0, 65535 , 0, 4*PI);
  }


  // convert polar coordinates back to cartesian
  // & render noise value there

  float render_pixel() {

    // convert polar coordinates back to cartesian ones

    float newx = (offset_x + center_x - (cosf(newangle) * newdist)) * scale_x;
    float newy = (offset_y + center_y - (sinf(newangle) * newdist)) * scale_y;

    // render noisevalue at this new cartesian point

    uint16_t raw_noise_field_value = inoise16(newx, newy, z);

    // a lot is happening here, namely
    // A) enhance histogram (improve contrast) by setting the black and white point
    // B) scale the result to a 0-255 range
    // it's the contrast boosting & the "colormapping" (technically brightness mapping)

    if (raw_noise_field_value < low_limit)  raw_noise_field_value =  low_limit;
    if (raw_noise_field_value > high_limit) raw_noise_field_value = high_limit;

    float scaled_noise_value = map_float(raw_noise_field_value, low_limit, high_limit, 0, 255);

    return scaled_noise_value;

    // done, we've just rendered one color value for one single pixel
  }

  // // precalculate a radial brightness mask

  // void render_vignette_table(float filter_radius) {

  //   for (int xx = 0; xx < num_x; xx++) {
  //     for (int yy = 0; yy < num_y; yy++) {

  //       vignette[xx] [yy] = (filter_radius - distance[xx] [yy]) / filter_radius; 
  //       if (vignette[xx] [yy] < 0) vignette[xx] [yy] = 0;  
  //     }
  //   }
  // }

};

    /*
    Ken Perlins improved noise   -  http://mrl.nyu.edu/~perlin/noise/
    C-port:  http://www.fundza.com/c4serious/noise/perlin/perlin.html
    by Malcolm Kesson;   arduino port by Peter Chiochetti, Sep 2007 :
    -  make permutation constant byte, obsoletes init(), lookup % 256
    */

    static const byte p[] = {   151,160,137,91,90, 15,131, 13,201,95,96,
    53,194,233, 7,225,140,36,103,30,69,142, 8,99,37,240,21,10,23,190, 6,
    148,247,120,234,75, 0,26,197,62,94,252,219,203,117, 35,11,32,57,177,
    33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,134,139,
    48,27,166, 77,146,158,231,83,111,229,122, 60,211,133,230,220,105,92,
    41,55,46,245,40,244,102,143,54,65,25,63,161, 1,216,80,73,209,76,132,
    187,208, 89, 18,169,200,196,135,130,116,188,159, 86,164,100,109,198,
    173,186, 3,64,52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,
    212,207,206, 59,227, 47,16,58,17,182,189, 28,42,223,183,170,213,119,
    248,152,2,44,154,163,70,221,153,101,155,167,43,172, 9,129,22,39,253,
    19,98,108,110,79,113,224,232,178,185,112,104,218,246, 97,228,251,34,
    242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,
    150,254,138,236,205, 93,222,114, 67,29,24, 72,243,141,128,195,78,66,
    215,61,156,180
    };

// Circular Blobs
//
// VO.2 preview version
// by Stefan Petrick 2023
// This code is licenced under a 
// Creative Commons Attribution 
// License CC BY-NC 3.0
//
// In order to run this on your own setup you might want to check and change
// line 22 & 23 according to your matrix size and 
// line 75 to suit your LED interface type.
//
// In case you want to run this code on a different LED driver library
// (like SmartMatrix, OctoWS2812, ESP32 16x parallel output) you will need to change
// line 52 to your own framebuffer and line 276+279 to your own setcolor function.
// In line 154 the framebuffer gets pushed to the LEDs.
// The whole report_performance function you can just comment out. It gets called
// in line 157.
//
// With this adaptions it should be easy to use this code with 
// any given LED driver & interface you might prefer.

//based on https://gist.github.com/StefanPetrick/35ffd8467df22a77067545cfb889aa4f
//and Fastled podcast nr 3: https://www.youtube.com/watch?v=3tfjP7GJnZo

class CircularBlobs:public AnimartrixCore {
  private:

    float fade(float t){ return t * t * t * (t * (t * 6 - 15) + 10); }
    float lerp(float t, float a, float b){ return a + t * (b - a); }
    float grad(int hash, float x, float y, float z)
    {
      int    h = hash & 15;          /* CONVERT LO 4 BITS OF HASH CODE */
      float  u = h < 8 ? x : y,      /* INTO 12 GRADIENT DIRECTIONS.   */
                v = h < 4 ? y : h==12||h==14 ? x : z;
      return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
    }

    #define P(x) p[(x) & 255]

    float pnoise(float x, float y, float z) {
      int   X = (int)floorf(x) & 255,             /* FIND UNIT CUBE THAT */
            Y = (int)floorf(y) & 255,             /* CONTAINS POINT.     */
            Z = (int)floorf(z) & 255;
      x -= floorf(x);                             /* FIND RELATIVE X,Y,Z */
      y -= floorf(y);                             /* OF POINT IN CUBE.   */
      z -= floorf(z);
      float  u = fade(x),                         /* COMPUTE FADE CURVES */
            v = fade(y),                         /* FOR EACH OF X,Y,Z.  */
            w = fade(z);
      int  A = P(X)+Y, 
          AA = P(A)+Z, 
          AB = P(A+1)+Z,                         /* HASH COORDINATES OF */
          B = P(X+1)+Y, 
          BA = P(B)+Z, 
          BB = P(B+1)+Z;                         /* THE 8 CUBE CORNERS, */

      return lerp(w,lerp(v,lerp(u, grad(P(AA  ), x, y, z),    /* AND ADD */
                                grad(P(BA  ), x-1, y, z)),    /* BLENDED */
                    lerp(u, grad(P(AB  ), x, y-1, z),         /* RESULTS */
                        grad(P(BB  ), x-1, y-1, z))),        /* FROM  8 */
                  lerp(v, lerp(u, grad(P(AA+1), x, y, z-1),   /* CORNERS */
                      grad(P(BA+1), x-1, y, z-1)),           /* OF CUBE */
                    lerp(u, grad(P(AB+1), x, y-1, z-1),
                        grad(P(BB+1), x-1, y-1, z-1))));
    }


  public:

  // Background for setting the following 2 numbers: the pnoise() function returns
  // raw values ranging from -1 to +1. In order to improve contrast we filter this output and
  // stretch the remains. In histogram (photography) terms this means setting a blackpoint and
  // a whitepoint. low_limit MUST be smaller than high_limit.
  float low_limit  = 0;            // everything lower drawns in black
                                  // higher numer = more black & more contrast present
  float high_limit = 0.5;          // everything higher gets maximum brightness & bleeds out
                                  // lower number = the result will be more bright & shiny
  float offset_z;     // wanna shift the cartesians during runtime?
  float scale_z;        // cartesian scaling in 3 dimensions

  void speedratiosAndOscillators() {

    // set speedratios for the offsets & oscillators
  
    spd = 0.001  ;               // higher = faster
    c   = 0.05  ;
    d   = 0.07   ;
    e   = 0.09  ;
    f   = 0.01  ;

    low_limit = 0;
    high_limit = 0.5;

    calculate_oscillators();     // get linear offsets and oscillators going
  }

  void forLoop() {
    // ...and now let's generate a frame 

    for (x = 0; x < num_x; x++) {
      for (y = 0; y < num_y; y++) {

        dist     = distance[x][y];  // pick precalculated polar data     
        angle    =    theta[x][y];
        
        // define first animation layer
        scale_x  = 0.11;            // smaller value = zoom in
        scale_y  = 0.1;             // higher = zoom out
        scale_z  = 0.1;
      
        newangle = angle + 5*SEGMENT.speed/255 * noise_angle_c + 5*SEGMENT.speed/255 * noise_angle_f;
        newdist  = 5*SEGMENT.intensity/255 * dist;
        offset_z = linear_c * 100;
        z        = -5 * sqrtf(dist) ;
        show1    = render_pixel_faster();

        // repeat for the 2nd layer, every parameter you don't change stays as it was set 
        // in the previous layer.

        offset_z = linear_d * 100;
        newangle = angle + 5*SEGMENT.speed/255 * noise_angle_d + 5*SEGMENT.speed/255 * noise_angle_f;
        show2    = render_pixel_faster();

        // 3d layer

        offset_z = linear_e*100;
        newangle = angle + 5*SEGMENT.speed/255 * noise_angle_e + 5*SEGMENT.speed/255 * noise_angle_f;
        show3 = render_pixel_faster();

        // create some interference between the layers

        show3 = show3-show2-show1;
        if (show3 < 0) show3 = 0;
                
        // Colormapping - Assign rendered values to colors 
        
        red   = show1-show2/2;
        if (red < 0) red=0;
        green = (show1-show2)/2;
        if (green < 0) green=0;
        blue  = show3-show1/2;
        if (blue < 0) blue=0;
        
        // Check the final results and store them.
        // Discard faulty RGB values & write the remaining valid results into the framebuffer.
        
        write_pixel_to_framebuffer();
      }
    }
  }

  void calculate_oscillators() {

    AnimartrixCore::calculate_oscillators();    

    float n;
      
    n =  1 + pnoise(linear_c , 10, 10);    // some noise controlled angular offsets 0 to PI
    noise_angle_c = n * PI;
    n =  1 + pnoise(linear_d , 20, 20);    
    noise_angle_d = n * PI;
    n =  1 + pnoise(linear_e , 30, 30);    
    noise_angle_e = n * PI;
    n =  1 + pnoise(linear_f , 40, 40);    
    noise_angle_f = n * PI; 
  }

  // Convert the polar 2 coordinates back to cartesian ones & also apply all 3d transitions.
  // Calculate the noise value at this point after the 5 dimensional manipulation of 
  // the underlaying coordinates.
  //  
  // Now I use a 32 bit float noise function which is more precise AND also more FPU friendly.
  // This results in a better render qualitiy in edgecases AND also in a 15% better performance.
  // Hurray!

  float render_pixel_faster() {

    // convert polar coordinates back to cartesian ones

    float newx = (offset_x + center_x - (cosf(newangle) * newdist)) * scale_x;
    float newy = (offset_y + center_y - (sinf(newangle) * newdist)) * scale_y;
    float newz = (offset_z + z) * scale_z;

    // render noisevalue at this new cartesian point

    float raw_noise_field_value = pnoise(newx, newy, newz);
    
    
    // a lot is happening here, namely
    // A) enhance histogram (improve contrast) by setting the black and white point
    // B) scale the result to a 0-255 range
    // it's the contrast boosting & the "colormapping" (technically brightness mapping)

    if (raw_noise_field_value < low_limit)  raw_noise_field_value =  low_limit;
    if (raw_noise_field_value > high_limit) raw_noise_field_value = high_limit;

    float scaled_noise_value = map_float(raw_noise_field_value, low_limit, high_limit, 0, 255);

    return scaled_noise_value;

    // done, we've just rendered one color value for one single pixel
  }
};

//effect functions
uint16_t mode_PolarBasics(void) { 

  PolarBasics* spe;


  if(!SEGENV.allocateData(sizeof(PolarBasics))) {SEGMENT.fill(SEGCOLOR(0)); return 350;} //mode_static(); //allocation failed

  spe = reinterpret_cast<PolarBasics*>(SEGENV.data);

  //first time init
  if (SEGENV.call == 0) {

    USER_PRINTF("mode_PolarBasics %d\n", sizeof(PolarBasics));
    //  if (SEGENV.call == 0) SEGMENT.setUpLeds();

    spe->init();

    // spe->render_vignette_table(9.5);           // the number is the desired radius in pixel
                                        // WIDTH/2 generates a circle
  }

  spe->speedratiosAndOscillators();

  spe->forLoop();

  // FastLED.show();

  // EVERY_N_MILLIS(500) spe->report_performance();

  return FRAMETIME;
}
static const char _data_FX_mode_PolarBasics[] PROGMEM = "💡Polar Basics ☾@AngleDist,AngleMult;;!;2;sx=0,ix=51,c1=0,c2=0,c3=0";


uint16_t mode_CircularBlobs(void) { 
  CircularBlobs* spe;


  if(!SEGENV.allocateData(sizeof(CircularBlobs))) {SEGMENT.fill(SEGCOLOR(0)); return 350;} //mode_static(); //allocation failed

  spe = reinterpret_cast<CircularBlobs*>(SEGENV.data);

  //first time init
  if (SEGENV.call == 0) {

    USER_PRINTF("mode_CircularBlobs %d\n", sizeof(CircularBlobs));
    //  if (SEGENV.call == 0) SEGMENT.setUpLeds();

    spe->init();

  }

  spe->speedratiosAndOscillators();

  spe->forLoop();

  // FastLED.show();

  // EVERY_N_MILLIS(500) spe->report_performance();

  return FRAMETIME;
}
static const char _data_FX_mode_CircularBlobs[] PROGMEM = "💡CircularBlobs ☾@AngleDist,AngleMult;;!;2;sx=51,ix=51,c1=0,c2=0,c3=0";


class AnimartrixUsermod : public Usermod {

  public:

    AnimartrixUsermod(const char *name, bool enabled):Usermod(name, enabled) {} //WLEDMM

    void setup() {
      strip.addEffect(255, &mode_PolarBasics, _data_FX_mode_PolarBasics);
      strip.addEffect(255, &mode_CircularBlobs, _data_FX_mode_CircularBlobs);

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

    uint16_t getId()
    {
      return USERMOD_ID_ANIMARTRIX;
    }

};



