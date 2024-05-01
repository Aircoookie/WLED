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


#include <stdint.h>
#include "FastLED.h"

//memory allocation
#define ESP8266_MAXPARTICLES 180// // enough for one 16x16 segment with transitions
#define ESP8266_MAXSOURCES 16
#define ESP32S2_MAXPARTICLES 840 // enough for four 16x16 segments
#define ESP32S2_MAXSOURCES 48
#define ESP32_MAXPARTICLES 1024 // enough for four 16x16 segments  TODO: not enough for one 64x64 panel...
#define ESP32_MAXSOURCES 64

//particle dimensions (subpixel division)
#define PS_P_RADIUS 64 //subpixel size, each pixel is divided by this for particle movement, if this value is changed, also change the shift defines (next two lines)
#define PS_P_HALFRADIUS 32  
#define PS_P_RADIUS_SHIFT 6 // shift for RADIUS
#define PS_P_SURFACE 12 // shift: 2^PS_P_SURFACE = (PS_P_RADIUS)^2
#define PS_P_MINHARDRADIUS 70 // minimum hard surface radius
#define PS_P_MINSURFACEHARDNESS 128 //minimum hardness used in collision impulse calculation, below this hardness, particles become sticky
#define PS_P_MAXSPEED 120 //maximum speed a particle can have (vx/vy is int8)

//struct for a single particle (10 bytes)
typedef struct {
    int16_t x;   //x position in particle system
    int16_t y;   //y position in particle system    
    int8_t vx;  //horizontal velocity
    int8_t vy;  //vertical velocity
    uint8_t hue;  // color hue
    uint8_t sat; //particle color saturation
    //two byte bit field:
    uint16_t ttl : 12; // time to live, 12 bit or 4095 max (which is 50s at 80FPS)
    bool outofbounds : 1; //out of bounds flag, set to true if particle is outside of display area
    bool collide : 1; //if set, particle takes part in collisions
    bool perpetural : 1; //if set, particle does not age (TTL is not decremented in move function, it still dies from killoutofbounds)
    bool flag4 : 1;   // unused flag      
} PSparticle;

// struct for additional particle settings (optional)
typedef struct
{  
  uint8_t size; //particle size, 255 means 10 pixels in diameter
  uint8_t forcecounter; //counter for applying forces to individual particles

  //bool flag1 : 1; // unused flags... for now.
  //bool flag2 : 1;    
  //bool flag3 : 1;       
  //bool flag4 : 1;
} PSadvancedParticle;

// struct for advanced particle size control (optional) TODO: this is currently just an idea, may not make it into final code if too slow / complex 
typedef struct
{
  uint8_t sizeasymmetry; // asymmetrical size TODO: need something better to define this?
  uint8_t targetsize;  // target size for growing / shrinking
  uint8_t growspeed : 4;   
  uint8_t shrinkspeed : 4; 
  uint8_t sizecounter; // counter that can be used for size contol TODO: need more than one?
  //ideas:
  //wobbleamount, rotation angle for asymmetic particles
  //a flag 'usegravity' that can be set to false for selective gravity application

  bool grow : 1; // flags
  bool shrink : 1;
  bool wobble : 1;
  bool flag4 : 1;
} PSsizeControl;


//struct for a particle source (17 bytes)
typedef struct {
	uint16_t minLife; // minimum ttl of emittet particles
	uint16_t maxLife; // maximum ttl of emitted particles
  PSparticle source; // use a particle as the emitter source (speed, position, color)
  uint8_t var; // variation of emitted speed (use odd numbers for good symmetry)
  int8_t vx; // emitting speed
  int8_t vy; 
  uint8_t size; // particle size (advanced property)
} PSsource; 

// struct for PS settings
typedef union
{
  struct{
  // add a one byte bit field:  
  bool wrapX : 1; 
  bool wrapY : 1;
  bool bounceX : 1;
  bool bounceY : 1;
  bool killoutofbounds : 1; // if set, out of bound particles are killed immediately
  bool useGravity : 1; //set to 1 if gravity is used, disables bounceY at the top
  bool useCollisions : 1;
  bool colorByAge : 1; // if set, particle hue is set by ttl value in render function
  };
  byte asByte; //order is: LSB is first entry in the list above
} PSsettings;

//class uses approximately 60 bytes
class ParticleSystem
{
public:
  ParticleSystem(uint16_t width, uint16_t height, uint16_t numberofparticles, uint16_t numberofsources, bool isadvanced = false); // constructor
  // note: memory is allcated in the FX function, no deconstructor needed
  void update(void); //update the particles according to set options and render to the matrix
  void updateFire(uint32_t intensity, bool renderonly = false); // update function for fire, if renderonly is set, particles are not updated (required to fix transitions with frameskips)
  void updateSystem(void); // call at the beginning of every FX, updates pointers and dimensions

  // particle emitters
  void flameEmit(PSsource &emitter);
  void sprayEmit(PSsource &emitter);
  void angleEmit(PSsource& emitter, uint16_t angle, int8_t speed);  

  // move functions
  void particleMoveUpdate(PSparticle &part, PSsettings &options, PSadvancedParticle *advancedproperties = NULL);
  //particle physics  
  void applyGravity(PSparticle *part); // applies gravity to single particle (use this for sources)
  void applyForce(PSparticle *part, int8_t xforce, int8_t yforce, uint8_t *counter);
  void applyForce(uint16_t particleindex, int8_t xforce, int8_t yforce); //use this for advanced property particles
  void applyForce(int8_t xforce, int8_t yforce); //apply a force to all particles
  void applyAngleForce(PSparticle *part, int8_t force, uint16_t angle, uint8_t *counter);
  void applyAngleForce(uint16_t particleindex, int8_t force, uint16_t angle); // use this for advanced property particles
  void applyAngleForce(int8_t force, uint16_t angle); //apply angular force to all particles
  void applyFriction(PSparticle *part, uint8_t coefficient); // apply friction to specific particle
  void applyFriction(uint8_t coefficient); // apply friction to all used particles
  void pointAttractor(uint16_t particleindex, PSparticle *attractor, uint8_t strength, bool swallow);
  void lineAttractor(uint16_t particleindex, PSparticle *attractorcenter, uint16_t attractorangle, uint8_t strength);

  //set options
  void setUsedParticles(uint16_t num);
  void setCollisionHardness(uint8_t hardness); //hardness for particle collisions (255 means full hard)
  void setWallHardness(uint8_t hardness); //hardness for bouncing on the wall if bounceXY is set
  void setWallRoughness(uint8_t roughness); //wall roughness randomizes wall collisions
  void setMatrixSize(uint16_t x, uint16_t y);
  void setWrapX(bool enable);
  void setWrapY(bool enable);
  void setBounceX(bool enable);
  void setBounceY(bool enable);
  void setKillOutOfBounds(bool enable); //if enabled, particles outside of matrix instantly die
  void setSaturation(uint8_t sat); //set global color saturation
  void setColorByAge(bool enable);
  void setMotionBlur(uint8_t bluramount);
  void setParticleSize(uint8_t size);
  void setGravity(int8_t force = 8);
  void enableParticleCollisions(bool enable, uint8_t hardness = 255);  
    
  PSparticle *particles; // pointer to particle array
  PSsource *sources; // pointer to sources
  PSadvancedParticle *advPartProps; // pointer to advanced particle properties (can be NULL)
  uint8_t* PSdataEnd; //points to first available byte after the PSmemory, is set in setPointers(). use this to set pointer to FX custom data  
  uint16_t maxX, maxY; //particle system size i.e. width-1 / height-1 in subpixels
  uint32_t maxXpixel, maxYpixel; // last physical pixel that can be drawn to (FX can read this to read segment size if required), equal to width-1 / height-1
  uint8_t numSources; //number of sources
  uint16_t numParticles;  // number of particles available in this system
  uint16_t usedParticles; // number of particles used in animation (can be smaller then numParticles)
  PSsettings particlesettings; // settings used when updating particles (can also used by FX to move sources), do not edit properties directly, use functions above

private: 
  //rendering functions
  void ParticleSys_render(bool firemode = false, uint32_t fireintensity = 128);
  void renderParticle(CRGB **framebuffer, uint32_t particleindex, uint32_t brightess, CRGB color, CRGB **renderbuffer);
  
  //paricle physics applied by system if flags are set
  void applyGravity(); // applies gravity to all particles
  void handleCollisions();
  void collideParticles(PSparticle *particle1, PSparticle *particle2);
  void fireParticleupdate();

  //utility functions
  void updatePSpointers(bool isadvanced); // update the data pointers to current segment data space
  void bounce(int8_t &incomingspeed, int8_t &parallelspeed, int32_t &position, uint16_t maxposition); //bounce on a wall
  int16_t wraparound(uint16_t p, uint32_t maxvalue);
  int32_t calcForce_dv(int8_t force, uint8_t *counter);
  int32_t limitSpeed(int32_t speed);
  CRGB **allocate2Dbuffer(uint32_t cols, uint32_t rows);

  // note: variables that are accessed often are 32bit for speed
  uint32_t emitIndex; // index to count through particles to emit so searching for dead pixels is faster
  int32_t collisionHardness;
  uint8_t wallHardness;
  uint8_t wallRoughness;
  uint8_t gforcecounter; //counter for global gravity
  int8_t gforce; //gravity strength, default is 8 (negative is allowed, positive is downwards)
  uint8_t collisioncounter; //counter to handle collisions TODO: could use the SEGMENT.call?
  uint8_t forcecounter; //counter for globally applied forces
  //global particle properties for basic particles
  uint8_t particlesize; //global particle size, 0 = 2 pixels, 255 = 10 pixels (note: this is also added to individual sized particles)
  int32_t particleHardRadius; // hard surface radius of a particle, used for collision detection
  uint8_t motionBlur; //enable motion blur, values > 100 gives smoother animations. Note: motion blurring does not work if particlesize is > 0
};

//initialization functions (not part of class)
bool initParticleSystem(ParticleSystem *&PartSys, uint8_t requestedsources, bool advanced = false, uint16_t additionalbytes = 0);
uint32_t calculateNumberOfParticles(bool advanced);
uint32_t calculateNumberOfSources(uint8_t requestedsources);
bool allocateParticleSystemMemory(uint16_t numparticles, uint16_t numsources, bool advanced, uint16_t additionalbytes);
//color add function
void fast_color_add(CRGB &c1, CRGB &c2, uint32_t scale = 255); // fast and accurate color adding with scaling (scales c2 before adding)
void fast_color_scale(CRGB &c, uint32_t scale); //fast scaling function using 32bit factor (keep it 0-255) and pointer
void blur2D(CRGB **colorbuffer, uint32_t xsize, uint32_t ysize, uint32_t xblur, uint32_t yblur, bool smear = true, uint32_t xstart = 0, uint32_t ystart = 0, bool isparticle = false);