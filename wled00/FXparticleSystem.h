/*
  FXparticleSystem.cpp

  Particle system with functions for particle generation, particle movement and particle rendering to RGB matrix.
  by DedeHai (Damian Schneider) 2013-2024
  Rendering is based on algorithm by giladaya, https://github.com/giladaya/arduino-particle-sys

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

//particle dimensions (subpixel division)
#define PS_P_RADIUS 64 //subpixel size, each pixel is divided by this for particle movement
#define PS_P_HARDRADIUS 92 //hard surface radius of a particle, in collisions, this is forbidden to be entered by another particle (for stacking) 
#define PS_P_SURFACE 12  //shift: 2^PS_P_SURFACE = (PS_P_RADIUS)^2


//struct for a single particle with gravity (12 bytes)
typedef struct {
    int16_t x;   //x position in particle system
    int16_t y;   //y position in particle system
    uint16_t ttl; //time to live
    uint8_t outofbounds; //set to 1 if outside of matrix
    uint8_t hue; //hue
    int8_t vx;  //horizontal velocity
    int8_t vy;  //vertical velocity
} PSparticle;

//struct for a particle source
typedef struct {
	uint16_t minLife; //minimum ttl of emittet particles
	uint16_t maxLife; //maximum ttl of emitted particles
    PSparticle source; //use a particle as the emitter source (speed, position, color)
    uint8_t var; //variation of emitted speed
    int8_t vx; //emitting speed
    int8_t vy; //emitting speed
} PSpointsource;

#define GRAVITYCOUNTER 2 //the higher the value the lower the gravity (speed is increased every n'th particle update call), values of 1 to 4 give good results
#define MAXGRAVITYSPEED 40 //particle terminal velocity

/*
//todo: make these local variables
uint8_t vortexspeed; //speed around vortex
uint8_t vortexdirection; //1 or 0
int8_t vortexpull; //if positive, vortex pushes, if negative it pulls
*/

void Emitter_Flame_emit(PSpointsource *emitter, PSparticle *part);
void Emitter_Fountain_emit(PSpointsource *emitter, PSparticle *part);
void Particle_Move_update(PSparticle *part);
void Particle_Bounce_update(PSparticle *part, const uint8_t hardness);
void Particle_Gravity_update(PSparticle *part, bool wrapX, bool bounceX, bool bounceY, const uint8_t hardness);
void ParticleSys_render(PSparticle *particles, uint16_t numParticles, uint8_t saturation, bool wrapX, bool wrapY);
void FireParticle_update(PSparticle *part, bool wrapX, bool WrapY);
void ParticleSys_renderParticleFire(PSparticle *particles, uint16_t numParticles, bool wrapX);
void PartMatrix_addHeat(uint8_t col, uint8_t row, uint16_t heat);
void handleCollision(PSparticle *particle1, PSparticle *particle2, const uint8_t hardness);
void applyFriction(PSparticle *particle, uint8_t coefficient);
