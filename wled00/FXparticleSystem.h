/*
  FXparticleSystem.cpp

  Particle system with functions for particle generation, particle movement and particle rendering to RGB matrix.
  by DedeHai (Damian Schneider) 2013-2024

  LICENSE
  The MIT License (MIT)
  Copyright (c) 2024  Damian Schneider 
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#if !defined(WLED_DISABLE_PARTICLESYSTEM2D) || !defined(WLED_DISABLE_PARTICLESYSTEM1D)  

#include <stdint.h>
#include "FastLED.h"

#define PS_P_MAXSPEED 120 // maximum speed a particle can have (vx/vy is int8)


//shared functions (used both in 1D and 2D system)
int32_t calcForce_dv(int8_t force, uint8_t *counter); //TODO: same as 2D function, could share
int32_t limitSpeed(int32_t speed);  //TODO: same as 2D function, could share
void fast_color_add(CRGB &c1, CRGB &c2, uint32_t scale = 255); // fast and accurate color adding with scaling (scales c2 before adding)
void fast_color_scale(CRGB &c, uint32_t scale); // fast scaling function using 32bit variable and pointer. note: keep 'scale' within 0-255

#endif

#ifndef WLED_DISABLE_PARTICLESYSTEM2D
// memory allocation
#define ESP8266_MAXPARTICLES 180 // enough for one 16x16 segment with transitions
#define ESP8266_MAXSOURCES 16
#define ESP32S2_MAXPARTICLES 840 // enough for four 16x16 segments
#define ESP32S2_MAXSOURCES 48
#define ESP32_MAXPARTICLES 1024 // enough for four 16x16 segments TODO: not enough for one 64x64 panel...
#define ESP32_MAXSOURCES 64

// particle dimensions (subpixel division)
#define PS_P_RADIUS 64 // subpixel size, each pixel is divided by this for particle movement, if this value is changed, also change the shift defines (next two lines)
#define PS_P_HALFRADIUS 32
#define PS_P_RADIUS_SHIFT 6 // shift for RADIUS
#define PS_P_SURFACE 12 // shift: 2^PS_P_SURFACE = (PS_P_RADIUS)^2
#define PS_P_MINHARDRADIUS 70 // minimum hard surface radius
#define PS_P_MINSURFACEHARDNESS 128 // minimum hardness used in collision impulse calculation, below this hardness, particles become sticky

// struct for PS settings (shared for 1D and 2D class)
typedef union
{
  struct{
  // one byte bit field for 2D settings
  bool wrapX : 1; 
  bool wrapY : 1;
  bool bounceX : 1;
  bool bounceY : 1;
  bool killoutofbounds : 1; // if set, out of bound particles are killed immediately
  bool useGravity : 1; // set to 1 if gravity is used, disables bounceY at the top
  bool useCollisions : 1;
  bool colorByAge : 1; // if set, particle hue is set by ttl value in render function
  };
  byte asByte; // access as a byte, order is: LSB is first entry in the list above
} PSsettings2D;

//struct for a single particle (10 bytes)
typedef struct {
    int16_t x;  // x position in particle system
    int16_t y;  // y position in particle system 
    int8_t vx;  // horizontal velocity
    int8_t vy;  // vertical velocity
    uint8_t hue;  // color hue
    uint8_t sat; // particle color saturation
    // two byte bit field:
    uint16_t ttl : 12; // time to live, 12 bit or 4095 max (which is 50s at 80FPS)
    bool outofbounds : 1; // out of bounds flag, set to true if particle is outside of display area
    bool collide : 1; // if set, particle takes part in collisions
    bool perpetual : 1; // if set, particle does not age (TTL is not decremented in move function, it still dies from killoutofbounds)
    bool state : 1; //can be used by FX to track state, not used in PS
} PSparticle;

// struct for additional particle settings (optional)
typedef struct
{  
  uint8_t size; // particle size, 255 means 10 pixels in diameter
  uint8_t forcecounter; // counter for applying forces to individual particles
} PSadvancedParticle;

// struct for advanced particle size control (optional) 
typedef struct
{
  uint8_t asymmetry; // asymmetrical size (0=symmetrical, 255 fully asymmetric)
  uint8_t asymdir; // direction of asymmetry, 64 is x, 192 is y (0 and 128 is symmetrical)
  uint8_t maxsize; // target size for growing 
  uint8_t minsize; // target size for shrinking
  uint8_t sizecounter : 4; // counters used for size contol (grow/shrink/wobble)
  uint8_t wobblecounter : 4; 
  uint8_t growspeed : 4;
  uint8_t shrinkspeed : 4;
  uint8_t wobblespeed : 4;
  bool grow : 1; // flags
  bool shrink : 1;
  bool pulsate : 1; // grows & shrinks & grows & ...
  bool wobble : 1; // alternate x and y size
} PSsizeControl;


//struct for a particle source (17 bytes)
typedef struct {
  uint16_t minLife; // minimum ttl of emittet particles
  uint16_t maxLife; // maximum ttl of emitted particles
  PSparticle source; // use a particle as the emitter source (speed, position, color)
  int8_t var; // variation of emitted speed (adds random(+/- var) to speed)
  int8_t vx; // emitting speed
  int8_t vy; 
  uint8_t size; // particle size (advanced property)
} PSsource; 

// class uses approximately 60 bytes
class ParticleSystem
{
public:
  ParticleSystem(uint16_t width, uint16_t height, uint16_t numberofparticles, uint16_t numberofsources, bool isadvanced = false,  bool sizecontrol = false); // constructor
  // note: memory is allcated in the FX function, no deconstructor needed
  void update(void); //update the particles according to set options and render to the matrix
  void updateFire(uint32_t intensity, bool renderonly = false); // update function for fire, if renderonly is set, particles are not updated (required to fix transitions with frameskips)
  void updateSystem(void); // call at the beginning of every FX, updates pointers and dimensions
  void particleMoveUpdate(PSparticle &part, PSsettings2D *options = NULL, PSadvancedParticle *advancedproperties = NULL); // move function
  
  // particle emitters
  int32_t sprayEmit(PSsource &emitter, uint32_t amount = 1);
  void flameEmit(PSsource &emitter);
  void angleEmit(PSsource& emitter, uint16_t angle, int8_t speed, uint32_t amount = 1);


  //particle physics
  void applyGravity(PSparticle *part); // applies gravity to single particle (use this for sources)
  void applyForce(PSparticle *part, int8_t xforce, int8_t yforce, uint8_t *counter);
  void applyForce(uint16_t particleindex, int8_t xforce, int8_t yforce); // use this for advanced property particles
  void applyForce(int8_t xforce, int8_t yforce); // apply a force to all particles
  void applyAngleForce(PSparticle *part, int8_t force, uint16_t angle, uint8_t *counter);
  void applyAngleForce(uint16_t particleindex, int8_t force, uint16_t angle); // use this for advanced property particles
  void applyAngleForce(int8_t force, uint16_t angle); // apply angular force to all particles
  void applyFriction(PSparticle *part, int32_t coefficient); // apply friction to specific particle
  void applyFriction(int32_t coefficient); // apply friction to all used particles
  void pointAttractor(uint16_t particleindex, PSparticle *attractor, uint8_t strength, bool swallow);
  void lineAttractor(uint16_t particleindex, PSparticle *attractorcenter, uint16_t attractorangle, uint8_t strength);

  // set options
  void setUsedParticles(uint32_t num);
  void setCollisionHardness(uint8_t hardness); // hardness for particle collisions (255 means full hard)
  void setWallHardness(uint8_t hardness); // hardness for bouncing on the wall if bounceXY is set
  void setWallRoughness(uint8_t roughness); // wall roughness randomizes wall collisions
  void setMatrixSize(uint16_t x, uint16_t y);
  void setWrapX(bool enable);
  void setWrapY(bool enable);
  void setBounceX(bool enable);
  void setBounceY(bool enable);
  void setKillOutOfBounds(bool enable); // if enabled, particles outside of matrix instantly die
  void setSaturation(uint8_t sat); // set global color saturation
  void setColorByAge(bool enable);
  void setMotionBlur(uint8_t bluramount); // note: motion blur can only be used if 'particlesize' is set to zero 
  void setParticleSize(uint8_t size);
  void setGravity(int8_t force = 8);
  void enableParticleCollisions(bool enable, uint8_t hardness = 255);
    
  PSparticle *particles; // pointer to particle array
  PSsource *sources; // pointer to sources
  PSadvancedParticle *advPartProps; // pointer to advanced particle properties (can be NULL)
  PSsizeControl *advPartSize; // pointer to advanced particle size control (can be NULL)
  uint8_t* PSdataEnd; // points to first available byte after the PSmemory, is set in setPointers(). use this for FX custom data
  int32_t maxX, maxY; // particle system size i.e. width-1 / height-1 in subpixels
  int32_t maxXpixel, maxYpixel; // last physical pixel that can be drawn to (FX can read this to read segment size if required), equal to width-1 / height-1
  uint32_t numSources; // number of sources
  uint32_t numParticles;  // number of particles available in this system
  uint32_t usedParticles; // number of particles used in animation (can be smaller then numParticles)  

private: 
  //rendering functions
  void ParticleSys_render(bool firemode = false, uint32_t fireintensity = 128);
  void renderParticle(CRGB **framebuffer, uint32_t particleindex, uint32_t brightness, CRGB color, CRGB **renderbuffer);
  
  //paricle physics applied by system if flags are set
  void applyGravity(); // applies gravity to all particles
  void handleCollisions();
  void collideParticles(PSparticle *particle1, PSparticle *particle2);
  void fireParticleupdate();

  //utility functions
  void updatePSpointers(bool isadvanced, bool sizecontrol); // update the data pointers to current segment data space
  void updateSize(PSadvancedParticle *advprops, PSsizeControl *advsize); // advanced size control
  void getParticleXYsize(PSadvancedParticle *advprops, PSsizeControl *advsize, uint32_t &xsize, uint32_t &ysize);
  void bounce(int8_t &incomingspeed, int8_t &parallelspeed, int32_t &position, uint16_t maxposition); // bounce on a wall
  int16_t wraparound(uint16_t p, uint32_t maxvalue);
  CRGB **allocate2Dbuffer(uint32_t cols, uint32_t rows);

  // note: variables that are accessed often are 32bit for speed
  PSsettings2D particlesettings; // settings used when updating particles (can also used by FX to move sources), do not edit properties directly, use functions above
  uint32_t emitIndex; // index to count through particles to emit so searching for dead pixels is faster
  int32_t collisionHardness;
  uint32_t wallHardness;
  uint32_t wallRoughness;
  uint8_t gforcecounter; // counter for global gravity
  int8_t gforce; // gravity strength, default is 8 (negative is allowed, positive is downwards)
  uint32_t collisioncounter; // counter to handle collisions TODO: could use the SEGMENT.call?
  uint8_t forcecounter; // counter for globally applied forces
  // global particle properties for basic particles
  uint8_t particlesize; // global particle size, 0 = 2 pixels, 255 = 10 pixels (note: this is also added to individual sized particles)
  int32_t particleHardRadius; // hard surface radius of a particle, used for collision detection
  uint8_t motionBlur; // enable motion blur, values > 100 gives smoother animations. Note: motion blurring does not work if particlesize is > 0
};

void blur2D(CRGB **colorbuffer, uint32_t xsize, uint32_t ysize, uint32_t xblur, uint32_t yblur, bool smear = true, uint32_t xstart = 0, uint32_t ystart = 0, bool isparticle = false);
// initialization functions (not part of class)
bool initParticleSystem2D(ParticleSystem *&PartSys, uint8_t requestedsources, uint16_t additionalbytes = 0, bool advanced = false, bool sizecontrol = false);
uint32_t calculateNumberOfParticles2D(bool advanced, bool sizecontrol);
uint32_t calculateNumberOfSources2D(uint8_t requestedsources);
bool allocateParticleSystemMemory2D(uint16_t numparticles, uint16_t numsources, bool advanced, bool sizecontrol, uint16_t additionalbytes);

#endif // WLED_DISABLE_PARTICLESYSTEM2D

////////////////////////
// 1D Particle System //
////////////////////////
#ifndef WLED_DISABLE_PARTICLESYSTEM1D
// memory allocation
//MAX_SEGMENT_DATA
#define ESP8266_MAXPARTICLES_1D 400 
#define ESP8266_MAXSOURCES_1D 8
#define ESP32S2_MAXPARTICLES_1D 1900
#define ESP32S2_MAXSOURCES_1D 16
#define ESP32_MAXPARTICLES_1D 6000
#define ESP32_MAXSOURCES_1D 32

// particle dimensions (subpixel division)
#define PS_P_RADIUS_1D 32 // subpixel size, each pixel is divided by this for particle movement, if this value is changed, also change the shift defines (next two lines)
#define PS_P_HALFRADIUS_1D 16
#define PS_P_RADIUS_SHIFT_1D 5 //TODO: may need to adjust
#define PS_P_SURFACE_1D 5 // shift: 2^PS_P_SURFACE = PS_P_RADIUS_1D
#define PS_P_MINHARDRADIUS_1D 32 // minimum hard surface radius 
#define PS_P_MINSURFACEHARDNESS_1D 50 // minimum hardness used in collision impulse calculation

// struct for PS settings (shared for 1D and 2D class)
typedef union
{  
  struct{
  // one byte bit field for 1D settings
  bool wrapX : 1; 
  bool bounceX : 1;  
  bool killoutofbounds : 1; // if set, out of bound particles are killed immediately
  bool useGravity : 1; // set to 1 if gravity is used, disables bounceY at the top
  bool useCollisions : 1;
  bool colorByAge : 1; // if set, particle hue is set by ttl value in render function
  bool colorByPosition : 1; // if set, particle hue is set by its position in the strip segment
  bool unused : 1;
  };
  byte asByte; // access as a byte, order is: LSB is first entry in the list above
} PSsettings1D;

//struct for a single particle (6 bytes)
typedef struct {
    int16_t x;  // x position in particle system
    int8_t vx;  // horizontal velocity    
    uint8_t hue;  // color hue
    // two byte bit field:
    uint16_t ttl : 11; // time to live, 11 bit or 2047 max (which is 25s at 80FPS)
    bool outofbounds : 1; // out of bounds flag, set to true if particle is outside of display area
    bool collide : 1; // if set, particle takes part in collisions
    bool perpetual : 1; // if set, particle does not age (TTL is not decremented in move function, it still dies from killoutofbounds)
    bool reversegrav : 1; // if set, gravity is reversed on this particle
    bool fixed : 1; // if set, particle does not move (and collisions make other particles revert direction), 
} PSparticle1D;

// struct for additional particle settings (optional)
typedef struct
{  
  uint8_t sat; //color saturation
  uint8_t size; // particle size, 255 means 10 pixels in diameter  
  uint8_t forcecounter;
} PSadvancedParticle1D;

//struct for a particle source (17 bytes)
typedef struct {
  uint16_t minLife; // minimum ttl of emittet particles
  uint16_t maxLife; // maximum ttl of emitted particles
  PSparticle1D source; // use a particle as the emitter source (speed, position, color)
  int8_t var; // variation of emitted speed (adds random(+/- var) to speed)
  int8_t v; // emitting speed
  uint8_t sat; // color saturation (advanced property)
  uint8_t size; // particle size (advanced property) 
} PSsource1D; 


class ParticleSystem1D
{
public:
  ParticleSystem1D(uint16_t length, uint16_t numberofparticles, uint16_t numberofsources, bool isadvanced = false); // constructor
  // note: memory is allcated in the FX function, no deconstructor needed
  void update(void); //update the particles according to set options and render to the matrix  
  void updateSystem(void); // call at the beginning of every FX, updates pointers and dimensions

  // particle emitters
  int32_t sprayEmit(PSsource1D &emitter);  
  void particleMoveUpdate(PSparticle1D &part, PSsettings1D *options = NULL, PSadvancedParticle1D *advancedproperties = NULL); // move function
  //particle physics
  void applyForce(PSparticle1D *part, int8_t xforce, uint8_t *counter); //apply a force to a single particle
  void applyForce(int8_t xforce); // apply a force to all particles
  void applyGravity(PSparticle1D *part); // applies gravity to single particle (use this for sources)  
  void applyFriction(int32_t coefficient); // apply friction to all used particles
  
  // set options
  void setUsedParticles(uint32_t num);  
  void setWallHardness(uint8_t hardness); // hardness for bouncing on the wall if bounceXY is set  
  void setSize(uint16_t x); //set particle system size (= strip length)
  void setWrap(bool enable);
  void setBounce(bool enable);  
  void setKillOutOfBounds(bool enable); // if enabled, particles outside of matrix instantly die
 // void setSaturation(uint8_t sat); // set global color saturation
  void setColorByAge(bool enable);
  void setColorByPosition(bool enable);
  void setMotionBlur(uint8_t bluramount); // note: motion blur can only be used if 'particlesize' is set to zero 
  void setParticleSize(uint8_t size); //size 0 = 1 pixel, size 1 = 2 pixels, is overruled by advanced particle size
  void setGravity(int8_t force = 8);
  void enableParticleCollisions(bool enable, uint8_t hardness = 255);
    
  PSparticle1D *particles; // pointer to particle array
  PSsource1D *sources; // pointer to sources
  PSadvancedParticle1D *advPartProps; // pointer to advanced particle properties (can be NULL)
  //PSsizeControl *advPartSize; // pointer to advanced particle size control (can be NULL)
  uint8_t* PSdataEnd; // points to first available byte after the PSmemory, is set in setPointers(). use this for FX custom data
  int32_t maxX; // particle system size i.e. width-1 
  int32_t maxXpixel; // last physical pixel that can be drawn to (FX can read this to read segment size if required), equal to width-1 
  uint32_t numSources; // number of sources
  uint32_t numParticles;  // number of particles available in this system
  uint32_t usedParticles; // number of particles used in animation (can be smaller then numParticles)  

private: 
  //rendering functions
  void ParticleSys_render(void);
  void renderParticle(CRGB *framebuffer, uint32_t particleindex, uint32_t brightness, CRGB color, CRGB *renderbuffer);

  //paricle physics applied by system if flags are set
  void applyGravity(); // applies gravity to all particles
  void handleCollisions();
  void collideParticles(PSparticle1D *particle1, PSparticle1D *particle2, int32_t dx, int32_t relativeV, uint32_t collisiondistance); 

  //utility functions
  void updatePSpointers(bool isadvanced); // update the data pointers to current segment data space
  //void updateSize(PSadvancedParticle *advprops, PSsizeControl *advsize); // advanced size control
  //void getParticleXYsize(PSadvancedParticle *advprops, PSsizeControl *advsize, uint32_t &xsize, uint32_t &ysize);
  void bounce(int8_t &incomingspeed, int8_t &parallelspeed, int32_t &position, uint16_t maxposition); // bounce on a wall
  CRGB *allocate1Dbuffer(uint32_t length);  
  // note: variables that are accessed often are 32bit for speed
  PSsettings1D particlesettings; // settings used when updating particles 
  uint32_t emitIndex; // index to count through particles to emit so searching for dead pixels is faster
  int32_t collisionHardness;
  uint32_t wallHardness;  
  uint8_t gforcecounter; // counter for global gravity
  int8_t gforce; // gravity strength, default is 8 (negative is allowed, positive is downwards)  
  uint8_t forcecounter; // counter for globally applied forces
  //uint8_t collisioncounter; // counter to handle collisions TODO: could use the SEGMENT.call? -> currently unused
  // global particle properties for basic particles
  uint8_t particlesize; // global particle size, 0 = 1 pixel, 1 = 2 pixels, larger sizez TBD (TODO: need larger sizes?)
  int32_t particleHardRadius; // hard surface radius of a particle, used for collision detection
  uint8_t motionBlur; // enable motion blur, values > 100 gives smoother animations. Note: motion blurring does not work if particlesize is > 0
};

bool initParticleSystem1D(ParticleSystem1D *&PartSys, uint8_t requestedsources, uint16_t additionalbytes = 0, bool advanced = false);
uint32_t calculateNumberOfParticles1D(bool isadvanced);
uint32_t calculateNumberOfSources1D(uint8_t requestedsources);
bool allocateParticleSystemMemory1D(uint16_t numparticles, uint16_t numsources, bool isadvanced, uint16_t additionalbytes);
void blur1D(CRGB *colorbuffer, uint32_t size, uint32_t blur, bool smear = true, uint32_t start = 0);
#endif // WLED_DISABLE_PARTICLESYSTEM1D
