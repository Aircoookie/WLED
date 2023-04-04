#pragma once

#include "wled.h"

//WLEDMM


// Polar basics demo for the 
// FastLED Podcast #2
// https://www.youtube.com/watch?v=KKjFRZFBUrQ
//
// VO.1 preview version
// by Stefan Petrick 2023
// This code is licenced under a 
// Creative Commons Attribution 
// License CC BY-NC 3.0

class PolarBasics {
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

    // Background for setting the following 2 numbers: the FastLED inoise16() function returns
    // raw values ranging from 0-65535. In order to improve contrast we filter this output and
    // stretch the remains. In histogram (photography) terms this means setting a blackpoint and
    // a whitepoint. low_limit MUST be smaller than high_limit.

    uint16_t low_limit;//  = 30000;            // everything lower drawns in black
                                            // higher numer = more black & more contrast present
    uint16_t high_limit;// = 50000;            // everything higher gets maximum brightness & bleeds out
                                            // lower number = the result will be more bright & shiny

    float center_x;// = (num_x / 2) - 0.5;     // the reference point for polar coordinates
    float center_y;// = (num_y / 2) - 0.5;     // (can also be outside of the actual xy matrix)
    //float center_x = 20;                  // the reference point for polar coordinates
    //float center_y = 20;                

    //WLEDMM: assign 32x32 fixed for the time being
    float theta   [32] [32];          // look-up table for all angles
    float distance[32] [32];          // look-up table for all distances
    float vignette[32] [32];
    float inverse_vignette[32] [32];

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

  void init () {
    num_x = SEGMENT.virtualWidth(); // horizontal pixel count
    num_y = SEGMENT.virtualHeight(); // vertical pixel count
    low_limit  = 30000;            // everything lower drawns in black
                                            // higher numer = more black & more contrast present
    high_limit = 50000;            // everything higher gets maximum brightness & bleeds out
                                            // lower number = the result will be more bright & shiny
    center_x = (num_x / 2) - 0.5;     // the reference point for polar coordinates
    center_y = (num_y / 2) - 0.5;     // (can also be outside of the actual xy matrix)

    //allocate memory for the 2D arrays
    // theta.resize(num_x, std::vector<float>(num_y, 0));
    // distance.resize(num_x, std::vector<float>(num_y, 0));
    // vignette.resize(num_x, std::vector<float>(num_y, 0));
    // inverse_vignette.resize(num_x, std::vector<float>(num_y, 0));
  }

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

    calculate_oscillators();     // get linear offsets and oscillators going
  }

  void forLoop() {
      // pick polar coordinates from look the up table 

      dist  = distance [x] [y];
      angle = theta    [y] [x];

      // Generation of one layer. Explore the parameters and what they do.
   
      scale_x  = 10000;                       // smaller value = zoom in, bigger structures, less detail
      scale_y  = 10000;                       // higher = zoom out, more pixelated, more detail
      z        = 0;                           // must be >= 0
      newangle = angle + angle_c;
      newdist  = dist;
      offset_x = 0;                        // must be >=0
      offset_y = 0;                        // must be >=0
      
      show1 = render_pixel();

              
      // Colormapping - Assign rendered values to colors 
      
      red   = show1;
      green = 0;
      blue  = 0;

      // Check the final results.
      // Discard faulty RGB values & write the valid results into the framebuffer.
      
      write_pixel_to_framebuffer();
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

  // precalculate a radial brightness mask

  void render_vignette_table(float filter_radius) {

    for (int xx = 0; xx < num_x; xx++) {
      for (int yy = 0; yy < num_y; yy++) {

        vignette[xx] [yy] = (filter_radius - distance[xx] [yy]) / filter_radius; 
        if (vignette[xx] [yy] < 0) vignette[xx] [yy] = 0;  
      }
    }
  }

};

//effect functions
uint16_t mode_PolarBasics(void) { 

  PolarBasics* pb;


  if(!SEGENV.allocateData(sizeof(PolarBasics))) {SEGMENT.fill(SEGCOLOR(0)); return 350;} //mode_static(); //allocation failed

  pb = reinterpret_cast<PolarBasics*>(SEGENV.data);

  //first time init
  if (SEGENV.call == 0) {

    USER_PRINTF("mode_PolarBasics %d\n", sizeof(PolarBasics));
    //  if (SEGENV.call == 0) SEGMENT.setUpLeds();

    pb->init();

    pb->render_polar_lookup_table();          // precalculate all polar coordinates 
                                        // to improve the framerate
    pb->render_vignette_table(9.5);           // the number is the desired radius in pixel
                                        // WIDTH/2 generates a circle
  }

  pb->speedratiosAndOscillators();

  // ...and now let's generate a frame 

  for (pb->x = 0; pb->x < pb->num_x; pb->x++) {
    for (pb->y = 0; pb->y < pb->num_y; pb->y++) {

      pb->forLoop();

    }
  }

  // FastLED.show();

  return FRAMETIME;
}

static const char _data_FX_mode_PolarBasics[] PROGMEM = "💡Polar Basics ☾@;;;2";

//class name. Use something descriptive and leave the ": public Usermod" part :)
class FastledUsermod : public Usermod {

  private:


  public:

    FastledUsermod(const char *name, bool enabled):Usermod(name, enabled) {} //WLEDMM

    void setup() {
      strip.addEffect(255, &mode_PolarBasics, _data_FX_mode_PolarBasics);

      initDone = true;
    }


    void connected() {
    }


    void loop() {
      if (!enabled || strip.isUpdating()) return;

      // do your magic here
      if (millis() - lastTime > 1000) {
        //Serial.println("I'm alive!");
        lastTime = millis();
      }
    }


    uint16_t getId()
    {
      return USERMOD_ID_FASTLED;
    }

};



