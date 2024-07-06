/*
  FXparticleSystem.cpp

  Particle system with functions for particle generation, particle movement and particle rendering to RGB matrix.
  by DedeHai (Damian Schneider) 2013-2024

  LICENSE
  The MIT License (MIT)
  Copyright (c) 2024 Damian Schneider
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

/*
  Note on ESP32: using 32bit integer is faster than 16bit or 8bit, each operation takes on less instruction, can be testen on https://godbolt.org/
  it does not matter if using int, unsigned int, uint32_t or int32_t, the compiler will make int into 32bit
  this should be used to optimize speed but not if memory is affected much
*/

/*
  TODO:
  -add function to 'update sources' so FX does not have to take care of that. FX can still implement its own version if so desired. 
  -add an x/y struct, do particle rendering using that, much easier to read
*/


#if !defined(WLED_DISABLE_PARTICLESYSTEM2D) || !defined(WLED_DISABLE_PARTICLESYSTEM1D)  
#include "FXparticleSystem.h"
#include "wled.h"
#include "FastLED.h"
#include "FX.h"
#endif

#ifndef WLED_DISABLE_PARTICLESYSTEM2D

ParticleSystem::ParticleSystem(uint16_t width, uint16_t height, uint16_t numberofparticles, uint16_t numberofsources, bool isadvanced, bool sizecontrol)
{
  //Serial.println("PS Constructor");
  numSources = numberofsources;
  numParticles = numberofparticles; // set number of particles in the array
  usedParticles = numberofparticles; // use all particles by default
  advPartProps = NULL; //make sure we start out with null pointers (just in case memory was not cleared)
  advPartSize = NULL;
  updatePSpointers(isadvanced, sizecontrol); // set the particle and sources pointer (call this before accessing sprays or particles)
  setMatrixSize(width, height);
  setWallHardness(255); // set default wall hardness to max
  setWallRoughness(0); // smooth walls by default
  setGravity(0); //gravity disabled by default
  setParticleSize(0); // minimum size by default
  motionBlur = 0; //no fading by default
  emitIndex = 0;

  //initialize some default non-zero values most FX use
  for (uint32_t i = 0; i < numSources; i++)
  {
    sources[i].source.sat = 255; //set saturation to max by default
    sources[i].source.ttl = 1; //set source alive
  }
  for (uint32_t i = 0; i < numParticles; i++)
  {
     particles[i].sat = 255; // full saturation
  }
  //Serial.println("PS Constructor done");
}

// update function applies gravity, moves the particles, handles collisions and renders the particles
void ParticleSystem::update(void)
{
  PSadvancedParticle *advprop = NULL; 
  //apply gravity globally if enabled
  if (particlesettings.useGravity)
    applyGravity();

  //update size settings before handling collisions
  if (advPartSize)
  {
    for (uint32_t i = 0; i < usedParticles; i++)
    {      
      updateSize(&advPartProps[i], &advPartSize[i]);
    }
  }
  
  // handle collisions (can push particles, must be done before updating particles or they can render out of bounds, causing a crash if using local buffer for speed)
  if (particlesettings.useCollisions)
    handleCollisions();

  //move all particles
  for (uint32_t i = 0; i < usedParticles; i++)
  {
    if (advPartProps)
    {
      advprop = &advPartProps[i];
    }
    particleMoveUpdate(particles[i], &particlesettings, advprop);
  }

  /*TODO remove this
  Serial.print("alive particles: ");
  uint32_t aliveparticles = 0;
  for (int i = 0; i < numParticles; i++)
  {
    if (particles[i].ttl)
    aliveparticles++;
  }
  Serial.println(aliveparticles);
  */
  ParticleSys_render();
}

// update function for fire animation
void ParticleSystem::updateFire(uint32_t intensity, bool renderonly)
{
  if (!renderonly)
    fireParticleupdate();
  ParticleSys_render(true, intensity);
}

void ParticleSystem::setUsedParticles(uint32_t num)
{
  usedParticles = min(num, numParticles); //limit to max particles
}

void ParticleSystem::setWallHardness(uint8_t hardness)
{
  wallHardness = hardness;
}

void ParticleSystem::setWallRoughness(uint8_t roughness)
{
  wallRoughness = roughness;
}

void ParticleSystem::setCollisionHardness(uint8_t hardness)
{  
  collisionHardness = (int)hardness + 1;
}

void ParticleSystem::setMatrixSize(uint16_t x, uint16_t y)
{
  maxXpixel = x - 1; // last physical pixel that can be drawn to
  maxYpixel = y - 1;
  maxX = x * PS_P_RADIUS - 1;  // particle system boundary for movements
  maxY = y * PS_P_RADIUS - 1;  // this value is often needed (also by FX) to calculate positions
}

void ParticleSystem::setWrapX(bool enable)
{
  particlesettings.wrapX = enable;
}

void ParticleSystem::setWrapY(bool enable)
{
  particlesettings.wrapY = enable;
}

void ParticleSystem::setBounceX(bool enable)
{
  particlesettings.bounceX = enable;
}

void ParticleSystem::setBounceY(bool enable)
{
  particlesettings.bounceY = enable;
}

void ParticleSystem::setKillOutOfBounds(bool enable)
{
  particlesettings.killoutofbounds = enable;
}

void ParticleSystem::setColorByAge(bool enable)
{
  particlesettings.colorByAge = enable;
}

void ParticleSystem::setMotionBlur(uint8_t bluramount)
{
  if (particlesize == 0) // only allwo motion blurring on default particle size or advanced size(cannot combine motion blur with normal blurring used for particlesize, would require another buffer)
    motionBlur = bluramount;
}

// render size using smearing (see blur function)
void ParticleSystem::setParticleSize(uint8_t size)
{
  particlesize = size;
  particleHardRadius = PS_P_MINHARDRADIUS + particlesize; // note: this sets size if not using advanced props
  motionBlur = 0; // disable motion blur if particle size is set
}
// enable/disable gravity, optionally, set the force (force=8 is default) can be -127 to +127, 0 is disable
// if enabled, gravity is applied to all particles in ParticleSystemUpdate()
// force is in 3.4 fixed point notation so force=16 means apply v+1 each frame default of 8 is every other frame (gives good results)
void ParticleSystem::setGravity(int8_t force) 
{  
  if (force)
  {
    gforce = force;
    particlesettings.useGravity = true;
  }
  else 
    particlesettings.useGravity = false;
}

void ParticleSystem::enableParticleCollisions(bool enable, uint8_t hardness) // enable/disable gravity, optionally, set the force (force=8 is default) can be 1-255, 0 is also disable
{
  particlesettings.useCollisions = enable;
  collisionHardness = (int)hardness + 1;
}

// emit one particle with variation, returns index of last emitted particle (or -1 if no particle emitted)
int32_t ParticleSystem::sprayEmit(PSsource &emitter, uint32_t amount)
{
 for (uint32_t a = 0; a < amount; a++)
  {
    for (uint32_t i = 0; i < usedParticles; i++)
    {
      emitIndex++;
      if (emitIndex >= usedParticles)
        emitIndex = 0;
      if (particles[emitIndex].ttl == 0) // find a dead particle
      {
        particles[emitIndex].vx = emitter.vx + random(-emitter.var, emitter.var); 
        particles[emitIndex].vy = emitter.vy + random(-emitter.var, emitter.var);
        particles[emitIndex].x = emitter.source.x; 
        particles[emitIndex].y = emitter.source.y; 
        particles[emitIndex].hue = emitter.source.hue;
        particles[emitIndex].sat = emitter.source.sat;
        particles[emitIndex].collide = emitter.source.collide;
        particles[emitIndex].ttl = random(emitter.minLife, emitter.maxLife);
        if (advPartProps)
          advPartProps[emitIndex].size = emitter.size;
        return i;
      }
    }
  }
  return -1;
}

// Spray emitter for particles used for flames (particle TTL depends on source TTL)
void ParticleSystem::flameEmit(PSsource &emitter)
{
  for (uint32_t i = 0; i < usedParticles; i++)
  {
    emitIndex++;
    if (emitIndex >= usedParticles)
      emitIndex = 0;
    if (particles[emitIndex].ttl == 0) // find a dead particle
    { 
      particles[emitIndex].x = emitter.source.x + random16(PS_P_RADIUS<<1) - PS_P_RADIUS; // jitter the flame by one pixel to make the flames wider at the base
      particles[emitIndex].y = emitter.source.y;
      particles[emitIndex].vx = emitter.vx + random16(emitter.var) - (emitter.var >> 1); // random16 is good enough for fire and much faster
      particles[emitIndex].vy = emitter.vy + random16(emitter.var) - (emitter.var >> 1);
      particles[emitIndex].ttl = random(emitter.minLife, emitter.maxLife) + emitter.source.ttl; 
      // fire uses ttl and not hue for heat, so no need to set the hue
      break; // done
    }
  }
  /*
  // note: this attemt to save on code size turns out to be much slower as fire uses a lot of particle emits, this must be efficient. also emitter.var would need adjustment
  uint32_t partidx = sprayEmit(emitter); //emit one particle
  // adjust properties
  particles[partidx].x += random16(PS_P_RADIUS<<1) - PS_P_RADIUS; // jitter the flame by one pixel to make the flames wider at the base
  particles[partidx].ttl += emitter.source.ttl; // flame intensity dies down with emitter TTL
  */
}

// Emits a particle at given angle and speed, angle is from 0-65535 (=0-360deg), speed is also affected by emitter->var
// angle = 0 means in positive x-direction (i.e. to the right)
void ParticleSystem::angleEmit(PSsource &emitter, uint16_t angle, int8_t speed, uint32_t amount)
{
  emitter.vx = ((int32_t)cos16(angle) * (int32_t)speed) / (int32_t)32600; // cos16() and sin16() return signed 16bit, division should be 32767 but 32600 gives slightly better rounding 
  emitter.vy = ((int32_t)sin16(angle) * (int32_t)speed) / (int32_t)32600; // note: cannot use bit shifts as bit shifting is asymmetrical for positive and negative numbers and this needs to be accurate!
  sprayEmit(emitter, amount);
}

// particle moves, decays and dies, if killoutofbounds is set, out of bounds particles are set to ttl=0
// uses passed settings to set bounce or wrap, if useGravity is set, it will never bounce at the top and killoutofbounds is not applied over the top
void ParticleSystem::particleMoveUpdate(PSparticle &part, PSsettings2D *options, PSadvancedParticle *advancedproperties)
{
  if (options == NULL)
    options = &particlesettings; //use PS system settings by default
  if (part.ttl > 0)
  {
    if (!part.perpetual) 
      part.ttl--; // age
    if (particlesettings.colorByAge)
      part.hue = part.ttl > 255 ? 255 : part.ttl; //set color to ttl

    bool usesize = false; // particle uses individual size rendering
    int32_t newX = part.x + (int16_t)part.vx;
    int32_t newY = part.y + (int16_t)part.vy;
    part.outofbounds = 0; // reset out of bounds (in case particle was created outside the matrix and is now moving into view)

    if (advancedproperties) //using individual particle size?
    {    
      if (advancedproperties->size > 0)
        usesize = true; // note: variable eases out of frame checking below
      particleHardRadius = max(PS_P_MINHARDRADIUS, (int)particlesize + (advancedproperties->size));
    }
    // if wall collisions are enabled, bounce them before they reach the edge, it looks much nicer if the particle is not half out of view
    if (options->bounceX) 
    {
      if ((newX < particleHardRadius) || (newX > maxX - particleHardRadius)) // reached a wall
        bounce(part.vx, part.vy, newX, maxX);
    }
    
    if ((newX < 0) || (newX > maxX)) // check if particle reached an edge (note: this also checks out of bounds and must not be skipped, even if bounce is enabled)
    {      
      if (options->wrapX)
      {        
        newX = newX % (maxX + 1); 
        if (newX < 0)
          newX += maxX + 1;  
      }
      else if (((newX <= -PS_P_HALFRADIUS) || (newX > maxX + PS_P_HALFRADIUS))) // particle is leaving, set out of bounds if it has fully left
      {
        bool isleaving = true;
        if (usesize) // using individual particle size
        {
          if (((newX > -particleHardRadius) && (newX < maxX + particleHardRadius))) // large particle is not yet leaving the view - note: this is not pixel perfect but good enough
            isleaving = false; 
        }
        
        if (isleaving)
        {
          part.outofbounds = 1;
          if (options->killoutofbounds)
            part.ttl = 0;
        }
      }
    }

    if (options->bounceY) 
    {
      if ((newY < particleHardRadius) || ((newY > maxY - particleHardRadius) && !options->useGravity)) // reached floor / ceiling
      {        
         bounce(part.vy, part.vx, newY, maxY);        
      }
    }
    
    if (((newY < 0) || (newY > maxY))) // check if particle reached an edge (makes sure particles are within frame for rendering)
    {
      if (options->wrapY)
      {
        newY = newY % (maxY + 1); 
        if (newY < 0)
          newY += maxY + 1; 
      }
      else if (((newY <= -PS_P_HALFRADIUS) || (newY > maxY + PS_P_HALFRADIUS))) // particle is leaving, set out of bounds if it has fully left
      {
        bool isleaving = true;
        if (usesize) // using individual particle size
        {
          if (((newY > -particleHardRadius) && (newY < maxY + particleHardRadius))) // still withing rendering reach
            isleaving = false; 
        }
        if (isleaving)
        {
          part.outofbounds = 1;
          if (options->killoutofbounds)
          {
            if (newY < 0) // if gravity is enabled, only kill particles below ground
              part.ttl = 0;
            else if (!options->useGravity)
              part.ttl = 0;
          }
        }
      }
    }
    part.x = (int16_t)newX; // set new position
    part.y = (int16_t)newY; // set new position
  }
}

// update advanced particle size control 
void ParticleSystem::updateSize(PSadvancedParticle *advprops, PSsizeControl *advsize)
{
  if (advsize == NULL) // just a safety check
    return;
  // grow/shrink particle
  int32_t newsize = advprops->size;
  uint32_t counter = advsize->sizecounter;
  uint32_t increment = 0;
  // calculate grow speed using 0-8 for low speeds and 9-15 for higher speeds
  if (advsize->grow) increment = advsize->growspeed;
  else if (advsize->shrink) increment = advsize->shrinkspeed;
  if (increment < 9) // 8 means +1 every frame
  {
    counter += increment;
    if (counter > 7)
    {
      counter -= 8;
      increment = 1;
    }
    else
      increment = 0;
    advsize->sizecounter = counter; 
  }
  else{
    increment = (increment - 8) << 1; // 9 means +2, 10 means +4 etc. 15 means +14
  }
  if (advsize->grow) 
  {
    if (newsize < advsize->maxsize)
    {
      newsize += increment; 
      if (newsize >= advsize->maxsize)
      {
        advsize->grow = false; // stop growing, shrink from now on if enabled
        newsize = advsize->maxsize; // limit
        if (advsize->pulsate) advsize->shrink = true;
      }
    }
  }
  else if (advsize->shrink)
  {
    if (newsize > advsize->minsize)
    {
      newsize -= increment; 
      if (newsize <= advsize->minsize)
      {
        //if (advsize->minsize == 0) part.ttl = 0; //TODO: need to pass particle or return kill instruction 
        advsize->shrink = false; // disable shrinking
        newsize = advsize->minsize; // limit 
        if (advsize->pulsate) advsize->grow = true;
      }
    }
  }
  advprops->size = newsize;
  // handle wobbling
  if (advsize->wobble) 
  {
    advsize->asymdir += advsize->wobblespeed; // todo: need better wobblespeed control? counter is already in the struct...
  }
}

// calculate x and y size for asymmetrical particles (advanced size control)
void ParticleSystem::getParticleXYsize(PSadvancedParticle *advprops, PSsizeControl *advsize, uint32_t &xsize, uint32_t &ysize)
{
  if (advsize == NULL) // if advanced size is valid, also advced properties pointer is valid (handled by pointer assignment function)
    return;
    int32_t deviation = ((uint32_t)advprops->size * (uint32_t)advsize->asymmetry) / 255; // deviation from symmetrical size
    // Calculate x and y size based on deviation and direction (0 is symmetrical, 64 is x, 128 is symmetrical, 192 is y)
  if (advsize->asymdir < 64) {
        deviation = ((int32_t)advsize->asymdir * deviation) / 64;
    } else if (advsize->asymdir < 192) {
        deviation = ((128 - (int32_t)advsize->asymdir) * deviation) / 64;
    } else {
        deviation = (((int32_t)advsize->asymdir - 255) * deviation) / 64;
    }
    // Calculate x and y size based on deviation, limit to 255 (rendering function cannot handle lareger sizes)
    xsize = ((int32_t)advprops->size - deviation) > 255 ? 255 : advprops->size - deviation;
    ysize = ((int32_t)advprops->size + deviation) > 255 ? 255 : advprops->size + deviation;
}

// function to bounce a particle from a wall using set parameters (wallHardness and wallRoughness)
void ParticleSystem::bounce(int8_t &incomingspeed, int8_t &parallelspeed, int32_t &position, uint16_t maxposition)
{
  incomingspeed = -incomingspeed;
  incomingspeed = (incomingspeed * wallHardness) / 255; // reduce speed as energy is lost on non-hard surface
  if (position < particleHardRadius)
    position = particleHardRadius; // fast particles will never reach the edge if position is inverted, this looks better
  else
    position = maxposition - particleHardRadius;
  if (wallRoughness)
  {
    int32_t incomingspeed_abs =  abs((int32_t)incomingspeed);
    int32_t totalspeed = incomingspeed_abs + abs((int32_t)parallelspeed);
    // transfer an amount of incomingspeed speed to parallel speed
    int32_t donatespeed = (random(-incomingspeed_abs, incomingspeed_abs) * (int32_t)wallRoughness) / (int32_t)255; //take random portion of + or - perpendicular speed, scaled by roughness 
    parallelspeed = limitSpeed((int32_t)parallelspeed + donatespeed);
    //give the remainder of the speed to perpendicular speed
    donatespeed = int8_t(totalspeed - abs(parallelspeed)); // keep total speed the same
    incomingspeed = incomingspeed > 0 ? donatespeed : -donatespeed;
  }
}

// apply a force in x,y direction to individual particle
// caller needs to provide a 8bit counter (for each paticle) that holds its value between calls
// force is in 3.4 fixed point notation so force=16 means apply v+1 each frame default of 8 is every other frame (gives good results)
void ParticleSystem::applyForce(PSparticle *part, int8_t xforce, int8_t yforce, uint8_t *counter)
{
  // for small forces, need to use a delay counter
  uint8_t xcounter = (*counter) & 0x0F; // lower four bits
  uint8_t ycounter = (*counter) >> 4;   // upper four bits

  // velocity increase
  int32_t dvx = calcForce_dv(xforce, &xcounter);
  int32_t dvy = calcForce_dv(yforce, &ycounter);

  // save counter values back
  *counter = xcounter & 0x0F; // write lower four bits, make sure not to write more than 4 bits
  *counter |= (ycounter << 4) & 0xF0; // write upper four bits

  // apply the force to particle
  part->vx = limitSpeed((int32_t)part->vx + dvx);
  part->vy = limitSpeed((int32_t)part->vy + dvy);
}

// apply a force in x,y direction to individual particle using advanced particle properties 
void ParticleSystem::applyForce(uint16_t particleindex, int8_t xforce, int8_t yforce)
{
  if (advPartProps == NULL)
    return; // no advanced properties available
  applyForce(&particles[particleindex], xforce, yforce, &advPartProps[particleindex].forcecounter);
}

// apply a force in x,y direction to all particles
// force is in 3.4 fixed point notation (see above)
void ParticleSystem::applyForce(int8_t xforce, int8_t yforce)
{
  // for small forces, need to use a delay counter
  uint8_t tempcounter;
  // note: this is not the most compuatationally efficient way to do this, but it saves on duplacte code and is fast enough
  for (uint i = 0; i < usedParticles; i++)
  {
    tempcounter = forcecounter;
    applyForce(&particles[i], xforce, yforce, &tempcounter);
  }  
  forcecounter = tempcounter; //save value back
}

// apply a force in angular direction to single particle
// caller needs to provide a 8bit counter that holds its value between calls (if using single particles, a counter for each particle is needed)
// angle is from 0-65535 (=0-360deg) angle = 0 means in positive x-direction (i.e. to the right)
// force is in 3.4 fixed point notation so force=16 means apply v+1 each frame (useful force range is +/- 127)
void ParticleSystem::applyAngleForce(PSparticle *part, int8_t force, uint16_t angle, uint8_t *counter)
{
  int8_t xforce = ((int32_t)force * cos16(angle)) / 32767; // force is +/- 127
  int8_t yforce = ((int32_t)force * sin16(angle)) / 32767; // note: cannot use bit shifts as bit shifting is asymmetrical for positive and negative numbers and this needs to be accurate!
  // note: sin16 is 10% faster than sin8() on ESP32 but on ESP8266 it is 9% slower
  applyForce(part, xforce, yforce, counter);
}

void ParticleSystem::applyAngleForce(uint16_t particleindex, int8_t force, uint16_t angle)
{
  if (advPartProps == NULL)
    return; // no advanced properties available
  applyAngleForce(&particles[particleindex], force, angle, &advPartProps[particleindex].forcecounter);
}

// apply a force in angular direction to all particles
// angle is from 0-65535 (=0-360deg) angle = 0 means in positive x-direction (i.e. to the right)
void ParticleSystem::applyAngleForce(int8_t force, uint16_t angle)
{
  int8_t xforce = ((int32_t)force * cos16(angle)) / 32767; // force is +/- 127
  int8_t yforce = ((int32_t)force * sin16(angle)) / 32767; // note: cannot use bit shifts as bit shifting is asymmetrical for positive and negative numbers and this needs to be accurate!
  applyForce(xforce, yforce);
}


// apply gravity to all particles using PS global gforce setting
// force is in 3.4 fixed point notation, see note above
// note: faster than apply force since direction is always down and counter is fixed for all particles
void ParticleSystem::applyGravity()
{
  int32_t dv = calcForce_dv(gforce, &gforcecounter);
  for (uint32_t i = 0; i < usedParticles; i++)
  {
    // note: not checking if particle is dead is faster as most are usually alive and if few are alive, rendering is fast anyways
    particles[i].vy = limitSpeed((int32_t)particles[i].vy - dv);
  }  
}

// apply gravity to single particle using system settings (use this for sources)
// function does not increment gravity counter, if gravity setting is disabled, this cannot be used
void ParticleSystem::applyGravity(PSparticle *part)
{
  uint32_t counterbkp = gforcecounter;
  int32_t dv = calcForce_dv(gforce, &gforcecounter);
  gforcecounter = counterbkp; //save it back 
  part->vy = limitSpeed((int32_t)part->vy - dv);
}

// slow down particle by friction, the higher the speed, the higher the friction. a high friction coefficient slows them more (255 means instant stop)
// note: a coefficient smaller than 0 will speed them up (this is a feature, not a bug), coefficient larger than 255 inverts the speed, so don't do that
void ParticleSystem::applyFriction(PSparticle *part, int32_t coefficient)
{
  int32_t friction = 255 - coefficient;
  // note: not checking if particle is dead can be done by caller (or can be omitted)
  // note2: cannot use right shifts as bit shifting in right direction is asymmetrical for positive and negative numbers and this needs to be accurate or things start to go to the left side.
  part->vx = ((int16_t)part->vx * friction) / 255; 
  part->vy = ((int16_t)part->vy * friction) / 255; 
}

// apply friction to all particles
void ParticleSystem::applyFriction(int32_t coefficient)
{
  for (uint32_t i = 0; i < usedParticles; i++)
  {
    if (particles[i].ttl)
      applyFriction(&particles[i], coefficient);
  }
}

// attracts a particle to an attractor particle using the inverse square-law
void ParticleSystem::pointAttractor(uint16_t particleindex, PSparticle *attractor, uint8_t strength, bool swallow) 
{
  if (advPartProps == NULL)
    return; // no advanced properties available

  // Calculate the distance between the particle and the attractor
  int32_t dx = attractor->x - particles[particleindex].x;
  int32_t dy = attractor->y - particles[particleindex].y;

  // Calculate the force based on inverse square law
  int32_t distanceSquared = dx * dx + dy * dy;
  if (distanceSquared < 8192)
  {
    if (swallow) // particle is close, age it fast so it fades out, do not attract further
    {      
      if (particles[particleindex].ttl > 7)
        particles[particleindex].ttl -= 8; 
      else
      {
        particles[particleindex].ttl = 0;
        return;
      }
    }
    distanceSquared = 2 * PS_P_RADIUS * PS_P_RADIUS; // limit the distance to avoid very high forces
  }

  int32_t force = ((int32_t)strength << 16) / distanceSquared;
  int8_t xforce = (force * dx) / 1024; // scale to a lower value, found by experimenting
  int8_t yforce = (force * dy) / 1024; // note: cannot use bit shifts as bit shifting is asymmetrical for positive and negative numbers and this needs to be accurate!

  applyForce(particleindex, xforce, yforce);
}

/*
//attract to a line (TODO: this is not yet working)
void ParticleSystem::lineAttractor(uint16_t particleindex, PSparticle *attractorcenter, uint16_t attractorangle, uint8_t strength)
{
  // Calculate the distance between the particle and the attractor
  if (advPartProps == NULL)
    return; // no advanced properties available

  // calculate a second point on the line
  int32_t x1 = attractorcenter->x + (cos16(attractorangle) >> 5);
  int32_t y1 = attractorcenter->y + (sin16(attractorangle) >> 5);
  // calculate squared distance from particle to the line:
  int32_t dx = (x1 - attractorcenter->x) >> 4;
  int32_t dy = (y1 - attractorcenter->y) >> 4;
  int32_t d = ((dx * (particles[particleindex].y - attractorcenter->y)) - (dy * (particles[particleindex].x - attractorcenter->x))) >> 8;
  int32_t distanceSquared = (d * d) / (dx * dx + dy * dy);


  // Calculate the force based on inverse square law
  if (distanceSquared < 2)
  {
    distanceSquared = 1;
  //  distanceSquared = 4 * PS_P_RADIUS * PS_P_RADIUS; // limit the distance to avoid very high forces
  }

  int32_t force = (((int32_t)strength << 16) / distanceSquared)>>10;
  //apply force in a 90° angle to the line
  int8_t xforce = (d > 0 ? 1 : -1) * (force * dy) / 100; // scale to a lower value, found by experimenting
  int8_t yforce = (d > 0 ? -1 : 1) * (force * dx) / 100; // note: cannot use bit shifts as bit shifting is asymmetrical for positive and negative numbers and this needs to be accurate!  

  Serial.print(" partx: ");
  Serial.print(particles[particleindex].x);
  Serial.print(" party ");
  Serial.print(particles[particleindex].y);
  Serial.print(" x1 ");
  Serial.print(x1);
  Serial.print(" y1 ");
  Serial.print(y1);
  Serial.print(" dx ");
  Serial.print(dx);
  Serial.print(" dy ");
  Serial.print(dy);
  Serial.print(" d: ");
  Serial.print(d);
  Serial.print(" dsq: ");
  Serial.print(distanceSquared);
  Serial.print(" force: ");
  Serial.print(force);
  Serial.print(" fx: ");
  Serial.print(xforce);
  Serial.print(" fy: ");
  Serial.println(yforce);

  applyForce(particleindex, xforce, yforce);
}*/

// render particles to the LED buffer (uses palette to render the 8bit particle color value)
// if wrap is set, particles half out of bounds are rendered to the other side of the matrix
// warning: do not render out of bounds particles or system will crash! rendering does not check if particle is out of bounds
// fireintensity and firemode are optional arguments (fireintensity is only used in firemode)
void ParticleSystem::ParticleSys_render(bool firemode, uint32_t fireintensity)
{
  
  CRGB baseRGB;
  bool useLocalBuffer = true; //use local rendering buffer, gives huge speed boost (at least 30% more FPS)
  CRGB **framebuffer = NULL; //local frame buffer
  CRGB **renderbuffer = NULL; //local particle render buffer for advanced particles
  uint32_t i;
  uint32_t brightness; // particle brightness, fades if dying

  if (useLocalBuffer)
  {    
    /*
    //memory fragmentation check:
    Serial.print("heap: ");
    Serial.print(heap_caps_get_free_size(MALLOC_CAP_8BIT));
    Serial.print(" block: ");
    Serial.println(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
    */

    // allocate empty memory for the local renderbuffer
    framebuffer = allocate2Dbuffer(maxXpixel + 1, maxYpixel + 1);
    if (framebuffer == NULL)
    {
      //Serial.println("Frame buffer alloc failed");
      useLocalBuffer = false; //render to segment pixels directly if not enough memory
    }
    else{
      if (advPartProps)
      {  
        renderbuffer = allocate2Dbuffer(10, 10); //buffer to render individual particles to if size > 0. note: null checking is done when accessing it
      }
      if (motionBlur > 0) // using SEGMENT.fadeToBlackBy is much slower, this approximately doubles the speed of fade calculation
      {
        uint32_t yflipped;
        for (int32_t y = 0; y <= maxYpixel; y++)
        {
          yflipped = maxYpixel - y;
          for (int32_t x = 0; x <= maxXpixel; x++)
          {
            framebuffer[x][y] = SEGMENT.getPixelColorXY(x, yflipped); //copy to local buffer
            fast_color_scale(framebuffer[x][y], motionBlur);
          }
        }
      }

    }

  }
  
  if (!useLocalBuffer) //disabled or allocation above failed
  {
    //Serial.println("NOT using local buffer!");
    if (motionBlur > 0)
      SEGMENT.fadeToBlackBy(255 - motionBlur);
    else
      SEGMENT.fill(BLACK); //clear the buffer before rendering to it 
  }
  // go over particles and render them to the buffer
  for (i = 0; i < usedParticles; i++)
  {
    if (particles[i].outofbounds || particles[i].ttl == 0)
      continue;

    // generate RGB values for particle
    if (firemode)
    {
      //TODO: decide on a final version...
      //brightness = (uint32_t)particles[i].ttl * (1 + (fireintensity >> 4)) + (fireintensity >> 2); //this is good
      //brightness = (uint32_t)particles[i].ttl * (fireintensity >> 3) + (fireintensity >> 1); // this is experimental, also works, flamecolor is more even, does not look as good (but less puffy at lower speeds)
      //brightness = (((uint32_t)particles[i].ttl * (maxY + PS_P_RADIUS - particles[i].y)) >> 7) + (uint32_t)particles[i].ttl * (fireintensity >> 4) + (fireintensity >> 1); // this is experimental //multiplikation mit weniger als >>4 macht noch mehr puffs bei low speed
      //brightness = (((uint32_t)particles[i].ttl * (maxY + PS_P_RADIUS - particles[i].y)) >> 7) + particles[i].ttl + (fireintensity>>1); // this is experimental
      //brightness = (((uint32_t)particles[i].ttl * (maxY + PS_P_RADIUS - particles[i].y)) >> 7) + ((particles[i].ttl * fireintensity) >> 5); // this is experimental TODO: test this -> testing... ok but not the best, bit sparky
      brightness = (((uint32_t)particles[i].ttl * (maxY + PS_P_RADIUS - particles[i].y)) >> 7) + (fireintensity >> 1); // this is experimental TODO: test this -> testing... does not look too bad!
      brightness = brightness > 255 ? 255 : brightness; // faster then using min()
      baseRGB = ColorFromPalette(SEGPALETTE, brightness, 255, LINEARBLEND);
    }
    else{
      brightness = particles[i].ttl > 255 ? 255 : particles[i].ttl; //faster then using min()
      baseRGB = ColorFromPalette(SEGPALETTE, particles[i].hue, 255, LINEARBLEND);
      if (particles[i].sat < 255) 
      {
        CHSV baseHSV = rgb2hsv_approximate(baseRGB); //convert to HSV
        baseHSV.s = particles[i].sat; //set the saturation
        baseRGB = (CRGB)baseHSV; // convert back to RGB
      }
    }
    
    renderParticle(framebuffer, i, brightness, baseRGB, renderbuffer);
  }

  if (particlesize > 0)
  {
    uint32_t passes = particlesize/64 + 1; // number of blur passes, four passes max
    uint32_t bluramount = particlesize; 
    uint32_t bitshift = 0;
  
    for(uint32_t i = 0; i < passes; i++)
    {
      if (i == 2) // for the last two passes, use higher amount of blur (results in a nicer brightness gradient with soft edges)
        bitshift = 1;
      
      if (useLocalBuffer)
        blur2D(framebuffer, maxXpixel + 1, maxYpixel + 1, bluramount << bitshift, bluramount << bitshift);
      else
        SEGMENT.blur(bluramount << bitshift, true);
      bluramount -= 64;
    }
  }

  if (useLocalBuffer) // transfer local buffer back to segment
  {
    int32_t yflipped;
    for (int32_t y = 0; y <= maxYpixel; y++)
    {
      yflipped = maxYpixel - y;
      for (int32_t x = 0; x <= maxXpixel; x++)
      {
        SEGMENT.setPixelColorXY((int)x, (int)yflipped, framebuffer[x][y]);
      }
    }
    free(framebuffer); 
  }
  if (renderbuffer)
    free(renderbuffer); 
}

// calculate pixel positions and brightness distribution and render the particle to local buffer or global buffer
void ParticleSystem::renderParticle(CRGB **framebuffer, uint32_t particleindex, uint32_t brightness, CRGB color, CRGB **renderbuffer)
{
  int32_t pxlbrightness[4] = {0}; // note: pxlbrightness needs to be set to 0 or checking does not work
  int32_t pixco[4][2]; // physical pixel coordinates of the four pixels a particle is rendered to. x,y pairs
  bool advancedrender = false; // rendering for advanced particles

  // subtract half a radius as the rendering algorithm always starts at the bottom left, this makes calculations more efficient
  int32_t xoffset = particles[particleindex].x - PS_P_HALFRADIUS;
  int32_t yoffset = particles[particleindex].y - PS_P_HALFRADIUS;
  int32_t dx = xoffset % PS_P_RADIUS; //relativ particle position in subpixel space
  int32_t dy = yoffset % PS_P_RADIUS;
  int32_t x = xoffset >> PS_P_RADIUS_SHIFT; // divide by PS_P_RADIUS which is 64, so can bitshift (compiler may not optimize automatically)
  int32_t y = yoffset >> PS_P_RADIUS_SHIFT;

  // check if particle has advanced size properties and buffer is available
  if (advPartProps)
  {
    if (advPartProps[particleindex].size > 0)
    {
      if (renderbuffer && framebuffer)
      {
        advancedrender = true;
        memset(renderbuffer[0], 0, 100 * sizeof(CRGB)); // clear the buffer, renderbuffer is 10x10 pixels
      }
      else
        return; // cannot render without buffers
    }      
  }

  // set the four raw pixel coordinates, the order is bottom left [0], bottom right[1], top right [2], top left [3]
  pixco[0][0] = pixco[3][0] = x;      // bottom left & top left
  pixco[0][1] = pixco[1][1] = y;      // bottom left & bottom right
  pixco[1][0] = pixco[2][0] = x + 1;  // bottom right & top right
  pixco[2][1] = pixco[3][1] = y + 1;  // top right & top left

  // now check if any are out of frame. set values to -1 if they are so they can be easily checked after (no value calculation, no setting of pixelcolor if value < 0)  
  if (x < 0) // left pixels out of frame
  {
    dx = PS_P_RADIUS + dx; // if x<0, xoffset becomes negative (and so does dx), must adjust dx as modulo will flip its value (really old bug now finally fixed)
    // note: due to inverted shift math, a particel at position -32 (xoffset = -64, dx = 64) is rendered at the wrong pixel position (it should be out of frame)
    // checking this above makes this algorithm slower (in frame pixels do not have to be checked), so just correct for it here:
    if (dx == PS_P_RADIUS)
    {
      pxlbrightness[1] = pxlbrightness[2] = -1; // pixel is actually out of matrix boundaries, do not render
    }
    if (particlesettings.wrapX) // wrap x to the other side if required
      pixco[0][0] = pixco[3][0] = maxXpixel;
    else
      pxlbrightness[0] = pxlbrightness[3] = -1; // pixel is out of matrix boundaries, do not render
  }
  else if (pixco[1][0] > maxXpixel) // right pixels, only has to be checkt if left pixels did not overflow
  {
    if (particlesettings.wrapX) // wrap y to the other side if required
      pixco[1][0] = pixco[2][0] = 0;
    else
      pxlbrightness[1] = pxlbrightness[2] = -1;
  }

  if (y < 0) // bottom pixels out of frame
  {
    dy = PS_P_RADIUS + dy; //see note above
    if (dy == PS_P_RADIUS)
    {
      pxlbrightness[2] = pxlbrightness[3] = -1; // pixel is actually out of matrix boundaries, do not render
    }
    if (particlesettings.wrapY) // wrap y to the other side if required
      pixco[0][1] = pixco[1][1] = maxYpixel;
    else
      pxlbrightness[0] = pxlbrightness[1] = -1;
  }
  else if (pixco[2][1] > maxYpixel) // top pixels
  {
    if (particlesettings.wrapY) // wrap y to the other side if required
      pixco[2][1] = pixco[3][1] = 0;
    else
      pxlbrightness[2] = pxlbrightness[3] = -1;
  }
  
  if (advancedrender) // always render full particles in advanced rendering, undo out of frame marking (faster than checking each time in code above)
  {
    for(uint32_t i = 0; i < 4; i++)
      pxlbrightness[i] = 0;
  }

  // calculate brightness values for all four pixels representing a particle using linear interpolation
  // precalculate values for speed optimization
  int32_t precal1 = (int32_t)PS_P_RADIUS - dx;
  int32_t precal2 = ((int32_t)PS_P_RADIUS - dy) * brightness;
  int32_t precal3 = dy * brightness;

  //calculate the values for pixels that are in frame
  if (pxlbrightness[0] >= 0)
    pxlbrightness[0] = (precal1 * precal2) >> PS_P_SURFACE; // bottom left value equal to ((PS_P_RADIUS - dx) * (PS_P_RADIUS-dy) * brightness) >> PS_P_SURFACE
  if (pxlbrightness[1] >= 0)
    pxlbrightness[1] = (dx * precal2) >> PS_P_SURFACE; // bottom right value equal to (dx * (PS_P_RADIUS-dy) * brightness) >> PS_P_SURFACE
  if (pxlbrightness[2] >= 0)
    pxlbrightness[2] = (dx * precal3) >> PS_P_SURFACE; // top right value equal to (dx * dy * brightness) >> PS_P_SURFACE
  if (pxlbrightness[3] >= 0)
    pxlbrightness[3] = (precal1 * precal3) >> PS_P_SURFACE; // top left value equal to ((PS_P_RADIUS-dx) * dy * brightness) >> PS_P_SURFACE

  if (advancedrender)
  {
    //render particle to a bigger size
    //particle size to pixels: < 64 is 4x4, < 128 is 6x6, < 192 is 8x8, bigger is 10x10 
    //first, render the pixel to the center of the renderbuffer, then apply 2D blurring
    fast_color_add(renderbuffer[4][4], color, pxlbrightness[0]); // order is: bottom left, bottom right, top right, top left
    fast_color_add(renderbuffer[5][4], color, pxlbrightness[1]);
    fast_color_add(renderbuffer[5][5], color, pxlbrightness[2]);
    fast_color_add(renderbuffer[4][5], color, pxlbrightness[3]); //TODO: make this a loop somehow? needs better coordinate handling...
    uint32_t rendersize = 2; // initialize render size, minimum is 4x4 pixels, it is incremented int he loop below to start with 4
    uint32_t offset = 4; // offset to zero coordinate to write/read data in renderbuffer (actually needs to be 3, is decremented in the loop below)
    uint32_t maxsize = advPartProps[particleindex].size;
    uint32_t xsize = maxsize;
    uint32_t ysize = maxsize;
    if (advPartSize) // use advanced size control
    {
      if (advPartSize[particleindex].asymmetry > 0)
        getParticleXYsize(&advPartProps[particleindex], &advPartSize[particleindex], xsize, ysize);
      maxsize = xsize;
      if (ysize > maxsize) maxsize = ysize; //maxsize is now the bigger of the two
    }
    maxsize = maxsize/64 + 1; // number of blur passes depends on maxsize, four passes max
    uint32_t bitshift = 0;
    for(uint32_t i = 0; i < maxsize; i++)
    {
      if (i == 2) //for the last two passes, use higher amount of blur (results in a nicer brightness gradient with soft edges)
        bitshift = 1;
      rendersize += 2;
      offset--;
      blur2D(renderbuffer, rendersize, rendersize, xsize << bitshift, ysize << bitshift, true, offset, offset, true);
      xsize = xsize > 64 ? xsize - 64 : 0; 
      ysize = ysize > 64 ? ysize - 64 : 0;
    }
    
    // calculate origin coordinates to render the particle to in the framebuffer
    uint32_t xfb_orig = x - (rendersize>>1) + 1 - offset;
    uint32_t yfb_orig = y - (rendersize>>1) + 1 - offset;
    uint32_t xfb, yfb; // coordinates in frame buffer to write to note: by making this uint, only overflow has to be checked (spits a warning though)

    // transfer particle renderbuffer to framebuffer
    for(uint32_t xrb = offset; xrb < rendersize+offset; xrb++)
    {
      xfb = xfb_orig + xrb;
      if (xfb > maxXpixel)
      {
        if (particlesettings.wrapX) // wrap x to the other side if required
          xfb = xfb % (maxXpixel + 1); //TODO: this did not work in 1D system but appears to work in 2D (wrapped pixels were offset) under which conditions does this not work?
        else
          continue;
      }

      for(uint32_t yrb = offset; yrb < rendersize+offset; yrb++)
      {
        yfb = yfb_orig + yrb;
        if (yfb > maxYpixel)
        {
          if (particlesettings.wrapY) // wrap y to the other side if required
            yfb = yfb % (maxYpixel + 1);
          else
            continue;
        }
        fast_color_add(framebuffer[xfb][yfb], renderbuffer[xrb][yrb]); 
      }
    }
  }    
  else // standard rendering
  {
    if (framebuffer)
    {
      for(uint32_t i = 0; i < 4; i++)
      {
        if (pxlbrightness[i] > 0)
          fast_color_add(framebuffer[pixco[i][0]][pixco[i][1]], color, pxlbrightness[i]); // order is: bottom left, bottom right, top right, top left
      }
    }
    else
    {  
      for(uint32_t i = 0; i < 4; i++)
      {
        if (pxlbrightness[i] > 0)
          SEGMENT.addPixelColorXY(pixco[i][0], maxYpixel - pixco[i][1], color.scale8((uint8_t)pxlbrightness[i])); 
      }
    }
  }


/*
  // debug: check coordinates if out of buffer boundaries print out some info (rendering out of bounds particle causes crash!)
  for (uint32_t d = 0; d < 4; d++)
  {
    if (pixco[d][0] < 0 || pixco[d][0] > maxXpixel)
    {
      //Serial.print("<");
      if (pxlbrightness[d] >= 0)
      {
        Serial.print("uncought out of bounds: x:");
        Serial.print(pixco[d][0]);
        Serial.print(" y:");
        Serial.print(pixco[d][1]);
        Serial.print("particle x=");
        Serial.print(particles[particleindex].x);
        Serial.print(" y=");
        Serial.println(particles[particleindex].y);
        pxlbrightness[d] = -1; // do not render
      }
    }
    if (pixco[d][1] < 0 || pixco[d][1] > maxYpixel)
    {
      //Serial.print("^");
      if (pxlbrightness[d] >= 0)
      {    
        Serial.print("uncought out of bounds: y:");
        Serial.print(pixco[d][0]);
        Serial.print(" y:");
        Serial.print(pixco[d][1]);
        Serial.print("particle x=");
        Serial.print(particles[particleindex].x);
        Serial.print(" y=");
        Serial.println(particles[particleindex].y);
        pxlbrightness[d] = -1; // do not render
      }
    }
  }
*/

}

// update & move particle, wraps around left/right if settings.wrapX is true, wrap around up/down if settings.wrapY is true
// particles move upwards faster if ttl is high (i.e. they are hotter)
void ParticleSystem::fireParticleupdate()
{
  //TODO: cleanup this function? check if normal move is much slower, change move function to check y first then this function just needs to add ttl to y befor calling normal move function  (this function uses 274bytes of flash)
  uint32_t i = 0;

  for (i = 0; i < usedParticles; i++)
  {
    if (particles[i].ttl > 0)
    {
      // age
      particles[i].ttl--;
      // apply velocity
      particles[i].x = particles[i].x + (int32_t)particles[i].vx;
      particles[i].y = particles[i].y + (int32_t)particles[i].vy + (particles[i].ttl >> 2); // younger particles move faster upward as they are hotter 
      //particles[i].y = particles[i].y + (int32_t)particles[i].vy;// + (particles[i].ttl >> 3); // younger particles move faster upward as they are hotter //this is experimental, different shifting
      particles[i].outofbounds = 0;
      // check if particle is out of bounds, wrap x around to other side if wrapping is enabled
      // as fire particles start below the frame, lots of particles are out of bounds in y direction. to improve speed, only check x direction if y is not out of bounds
      // y-direction
      if (particles[i].y < -PS_P_HALFRADIUS)
        particles[i].outofbounds = 1;
      else if (particles[i].y > maxY + PS_P_HALFRADIUS) // particle moved out at the top
        particles[i].ttl = 0;
      else // particle is in frame in y direction, also check x direction now
      {
        if ((particles[i].x < 0) || (particles[i].x > maxX))
        {
          if (particlesettings.wrapX)
          {
            particles[i].x = (uint16_t)particles[i].x % (maxX + 1); 
          }
          else if ((particles[i].x < -PS_P_HALFRADIUS) || (particles[i].x > maxX + PS_P_HALFRADIUS)) //if fully out of view
          {
            particles[i].ttl = 0;
          }
        }
      }
    }
  }
}

// detect collisions in an array of particles and handle them
void ParticleSystem::handleCollisions()
{
  // detect and handle collisions
  uint32_t i, j;
  uint32_t startparticle = 0;
  uint32_t endparticle = usedParticles >> 1; // do half the particles, significantly speeds things up
  // every second frame, do other half of particles (helps to speed things up as not all collisions are handled each frame, less accurate but good enough)
  // if more accurate collisions are needed, just call it twice in a row
  if (collisioncounter & 0x01) 
  { 
    startparticle = endparticle;
    endparticle = usedParticles;
  }  
  collisioncounter++;

  for (i = startparticle; i < endparticle; i++)
  {
    // go though all 'higher number' particles and see if any of those are in close proximity and if they are, make them collide
    if (particles[i].ttl > 0 && particles[i].outofbounds == 0  && particles[i].collide) // if particle is alive and does collide and is not out of view
    {
      int32_t dx, dy; // distance to other particles
      for (j = i + 1; j < usedParticles; j++) // check against higher number particles
      {                
        if (particles[j].ttl > 0 && particles[j].collide) // if target particle is alive
        {
          dx = particles[i].x - particles[j].x;
          if (advPartProps) //may be using individual particle size
          {                
              particleHardRadius = PS_P_MINHARDRADIUS + particlesize + (((uint32_t)advPartProps[i].size + (uint32_t)advPartProps[j].size)>>1); // collision distance
          }      
          if (dx < particleHardRadius && dx > -particleHardRadius) // check x direction, if close, check y direction
          {
            dy = particles[i].y - particles[j].y;
            if (dy < particleHardRadius && dy > -particleHardRadius) // particles are close
              collideParticles(&particles[i], &particles[j]);
          }
        }
      }
    }
  }
}

// handle a collision if close proximity is detected, i.e. dx and/or dy smaller than 2*PS_P_RADIUS
// takes two pointers to the particles to collide and the particle hardness (softer means more energy lost in collision, 255 means full hard)
void ParticleSystem::collideParticles(PSparticle *particle1, PSparticle *particle2) // TODO: dx,dy is calculated just above, can pass it over here to save a few CPU cycles?
{
  int32_t dx = particle2->x - particle1->x;
  int32_t dy = particle2->y - particle1->y; 
  int32_t distanceSquared = dx * dx + dy * dy;
  // Calculate relative velocity (if it is zero, could exit but extra check does not overall speed but deminish it)
  int32_t relativeVx = (int16_t)particle2->vx - (int16_t)particle1->vx;
  int32_t relativeVy = (int16_t)particle2->vy - (int16_t)particle1->vy;

  // if dx and dy are zero (i.e. they meet at the center) give them an offset, if speeds are also zero, also offset them (pushes them apart if they are clumped before enabling collisions)
  if (distanceSquared == 0) 
  {
    // Adjust positions based on relative velocity direction 
    dx = -1; 
    if (relativeVx < 0) // if true, particle2 is on the right side
      dx = 1;
    else if (relativeVx == 0) 
      relativeVx = 1;

    dy = -1;
    if (relativeVy < 0)
      dy = 1;
    else if (relativeVy == 0)
      relativeVy = 1;

    distanceSquared = 2; //1 + 1
  }

  // Calculate dot product of relative velocity and relative distance
  int32_t dotProduct = (dx * relativeVx + dy * relativeVy); // is always negative if moving towards each other
  int32_t notsorandom = dotProduct & 0x01; //dotprouct LSB should be somewhat random, so no need to calculate a random number

  if (dotProduct < 0) // particles are moving towards each other
  {
    // integer math used to avoid floats. 
    // overflow check: dx/dy are 7bit, relativV are 8bit -> dotproduct is 15bit, dotproduct/distsquared ist 8b, multiplied by collisionhardness of 8bit. so a 16bit shift is ok, make it 15 to be sure no overflows happen
    // note: cannot use right shifts as bit shifting in right direction is asymmetrical for positive and negative numbers and this needs to be accurate! the trick is: only shift positive numers
    // Calculate new velocities after collision
    int32_t surfacehardness = collisionHardness < PS_P_MINSURFACEHARDNESS ? PS_P_MINSURFACEHARDNESS : collisionHardness; // if particles are soft, the impulse must stay above a limit or collisions slip through at higher speeds, 170 seems to be a good value
    int32_t impulse = -(((((-dotProduct) << 15) / distanceSquared) * surfacehardness) >> 8); // note: inverting before bitshift corrects for asymmetry in right-shifts (and is slightly faster)
    int32_t ximpulse = ((impulse) * dx) / 32767; // cannot use bit shifts here, it can be negative, use division by 2^bitshift
    int32_t yimpulse = ((impulse) * dy) / 32767;
    particle1->vx += ximpulse;
    particle1->vy += yimpulse;
    particle2->vx -= ximpulse;
    particle2->vy -= yimpulse;

    if (collisionHardness < surfacehardness) // if particles are soft, they become 'sticky' i.e. apply some friction (they do pile more nicely and stop sloshing around)
    {
      const uint32_t coeff = collisionHardness + (255 - PS_P_MINSURFACEHARDNESS);
      particle1->vx = ((int32_t)particle1->vx * coeff) / 255; 
      particle1->vy = ((int32_t)particle1->vy * coeff) / 255;

      particle2->vx = ((int32_t)particle2->vx * coeff) / 255;
      particle2->vy = ((int32_t)particle2->vy * coeff) / 255;
/*
      if (collisionHardness < 10) // if they are very soft, stop slow particles completely to make them stick to each other
      {
        particle1->vx = (particle1->vx < 3 && particle1->vx > -3) ? 0 : particle1->vx;
        particle1->vy = (particle1->vy < 3 && particle1->vy > -3) ? 0 : particle1->vy;

        particle2->vx = (particle2->vx < 3 && particle2->vx > -3) ? 0 : particle2->vx;
        particle2->vy = (particle2->vy < 3 && particle2->vy > -3) ? 0 : particle2->vy;
      }*/
    }
    
    // particles have volume, push particles apart if they are too close
    // tried lots of configurations, it works best if not moved but given a little velocity, it tends to oscillate less this way
    // a problem with giving velocity is, that on harder collisions, this adds up as it is not dampened enough, so add friction in the FX if required
    if (dotProduct > -250) //this means particles are slow (or really really close) so push them apart.
    {      
      int32_t pushamount = 1 + ((250 + dotProduct) >> 6); // the closer dotproduct is to zero, the closer the particles are
      int32_t push = 0;
      if (dx < 0)  // particle 1 is on the right
        push = pushamount; 
      else if (dx > 0)
        push = -pushamount; 
      else // on the same x coordinate, shift it a little so they do not stack
      {
        if (notsorandom)
          particle1->x++; // move it so pile collapses
        else
          particle1->x--;
      }
      particle1->vx += push; //TODO: what happens if particle2 is also pushed? in 1D it stacks better, maybe also just reverse the comparison order so they flip roles?
      push = 0;
      if (dy < 0)
        push = pushamount; 
      else if (dy > 0)
        push = -pushamount; 
      else // dy==0
      {
        if (notsorandom)
          particle1->y++; // move it so pile collapses
        else
          particle1->y--;
      }
      particle1->vy += push;
      // note: pushing may push particles out of frame, if bounce is active, it will move it back as position will be limited to within frame, if bounce is disabled: bye bye
      if (collisionHardness < 16) // if they are very soft, stop slow particles completely to make them stick to each other
      {
        particle1->vx = 0;
        particle1->vy = 0;
        particle2->vx = 0; 
        particle2->vy = 0;
        //push them apart
        particle1->x += push;
        particle1->y += push;
      }
    }
  }
}

// allocate memory for the 2D array in one contiguous block and set values to zero
CRGB **ParticleSystem::allocate2Dbuffer(uint32_t cols, uint32_t rows)
{  
  CRGB ** array2D = (CRGB **)calloc(cols, sizeof(CRGB *) + rows * sizeof(CRGB));
  if (array2D == NULL)
    DEBUG_PRINT(F("PS buffer alloc failed"));
  else
  {
    // assign pointers of 2D array
    CRGB *start = (CRGB *)(array2D + cols);
    for (uint i = 0; i < cols; i++)
    {
      array2D[i] = start + i * rows;
    }
  }
  return array2D;
}

// update size and pointers (memory location and size can change dynamically)
// note: do not access the PS class in FX befor running this function (or it messes up SEGENV.data)
void ParticleSystem::updateSystem(void)
{
  // update matrix size
  uint32_t cols = strip.isMatrix ? SEGMENT.virtualWidth() : 1;
  uint32_t rows = strip.isMatrix ? SEGMENT.virtualHeight() : SEGMENT.virtualLength();
  setMatrixSize(cols, rows);
  updatePSpointers(advPartProps != NULL, advPartSize != NULL);
}

// set the pointers for the class (this only has to be done once and not on every FX call, only the class pointer needs to be reassigned to SEGENV.data every time)
// function returns the pointer to the next byte available for the FX (if it assigned more memory for other stuff using the above allocate function)
// FX handles the PSsources, need to tell this function how many there are
void ParticleSystem::updatePSpointers(bool isadvanced, bool sizecontrol)
{
  // DEBUG_PRINT(F("*** PS pointers ***"));
  // DEBUG_PRINTF_P(PSTR("this PS %p "), this);
  // Note on memory alignment:
  // a pointer MUST be 4 byte aligned. sizeof() in a struct/class is always aligned to the largest element. if it contains a 32bit, it will be padded to 4 bytes, 16bit is padded to 2byte alignment.
  // The PS is aligned to 4 bytes, a PSparticle is aligned to 2 and a struct containing only byte sized variables is not aligned at all and may need to be padded when dividing the memoryblock.
  // by making sure that the number of sources and particles is a multiple of 4, padding can be skipped here as alignent is ensured, independent of struct sizes.
  particles = reinterpret_cast<PSparticle *>(this + 1); // pointer to particle array at data+sizeof(ParticleSystem)
  sources = reinterpret_cast<PSsource *>(particles + numParticles); // pointer to source(s)
  if (isadvanced)
  {
    advPartProps = reinterpret_cast<PSadvancedParticle *>(sources + numSources);
    PSdataEnd = reinterpret_cast<uint8_t *>(advPartProps + numParticles);
    if (sizecontrol)
    {
      advPartSize = reinterpret_cast<PSsizeControl *>(advPartProps + numParticles);
      PSdataEnd = reinterpret_cast<uint8_t *>(advPartSize + numParticles);
    }     
  }
  else
  {
    PSdataEnd = reinterpret_cast<uint8_t *>(sources + numSources); // pointer to first available byte after the PS for FX additional data
  }
  /*
  DEBUG_PRINTF_P(PSTR(" particles %p "), particles);
  DEBUG_PRINTF_P(PSTR(" sources %p "), sources);
  DEBUG_PRINTF_P(PSTR(" adv. props %p "), advPartProps);
  DEBUG_PRINTF_P(PSTR(" adv. ctrl %p "), advPartSize);
  DEBUG_PRINTF_P(PSTR("end %p\n"), PSdataEnd);
  */
}

// blur a matrix in x and y direction, blur can be asymmetric in x and y
// for speed, 32bit variables are used, make sure to limit them to 8bit (0-255) or result is undefined 
// to blur a subset of the buffer, change the xsize/ysize and set xstart/ystart to the desired starting coordinates (default start is 0/0)
void blur2D(CRGB **colorbuffer, uint32_t xsize, uint32_t ysize, uint32_t xblur, uint32_t yblur, bool smear, uint32_t xstart, uint32_t ystart, bool isparticle)
{
  CRGB seeppart, carryover;
  uint32_t seep = xblur >> 1;  
  if (isparticle) //first and last row are always black in particle rendering
  {
    ystart++;
    ysize--;
  }
  for(uint32_t y = ystart; y < ystart + ysize; y++)
  {
    carryover =  BLACK;
    for(uint32_t x = xstart; x < xstart + xsize; x++)
    {
      seeppart = colorbuffer[x][y]; // create copy of current color
      fast_color_scale(seeppart, seep); // scale it and seep to neighbours
      if (!smear) // fade current pixel if smear is disabled
        fast_color_scale(colorbuffer[x][y], 255 - xblur); 

      if (x > 0)
      {
        fast_color_add(colorbuffer[x-1][y], seeppart);
        fast_color_add(colorbuffer[x][y], carryover); // TODO: could check if carryover is > 0, takes 7 instructions, add takes ~35, with lots of same color pixels (like background), it would be faster
      }
      carryover = seeppart;
    }
    fast_color_add(colorbuffer[xsize-1][y], carryover); // set last pixel
  }

  if (isparticle) // now also do first and last row
  {
    ystart--;
    ysize++;
  }

  seep = yblur >> 1;
  for(uint32_t x = xstart; x < xstart + xsize; x++)
  {
    carryover = BLACK;
    for(uint32_t y = ystart; y < ystart + ysize; y++)
    {
      seeppart = colorbuffer[x][y]; // create copy of current color
      fast_color_scale(seeppart, seep); // scale it and seep to neighbours
      if (!smear) // fade current pixel if smear is disabled
        fast_color_scale(colorbuffer[x][y], 255 - yblur); 

      if (y > 0)
      {
        fast_color_add(colorbuffer[x][y-1], seeppart);
        fast_color_add(colorbuffer[x][y], carryover); // todo: could check if carryover is > 0, takes 7 instructions, add takes ~35, with lots of same color pixels (like background), it would be faster
      }
      carryover = seeppart;
    }
    fast_color_add(colorbuffer[x][ysize-1], carryover); // set last pixel
  }
}


//non class functions to use for initialization
uint32_t calculateNumberOfParticles2D(bool isadvanced, bool sizecontrol)
{
  uint32_t cols = strip.isMatrix ? SEGMENT.virtualWidth() : 1;
  uint32_t rows = strip.isMatrix ? SEGMENT.virtualHeight() : SEGMENT.virtualLength();
#ifdef ESP8266
  uint32_t numberofParticles = (cols * rows * 3) / 4; // 0.75 particle per pixel
  uint32_t particlelimit = ESP8266_MAXPARTICLES; // maximum number of paticles allowed (based on one segment of 16x16 and 4k effect ram)
#elif ARDUINO_ARCH_ESP32S2
  uint32_t numberofParticles = (cols * rows); // 1 particle per pixel
  uint32_t particlelimit = ESP32S2_MAXPARTICLES; // maximum number of paticles allowed (based on one segment of 32x32 and 24k effect ram)
#else
  uint32_t numberofParticles = (cols * rows);  // 1 particle per pixel (for example 512 particles on 32x16)
  uint32_t particlelimit = ESP32_MAXPARTICLES; // maximum number of paticles allowed (based on two segments of 32x32 and 40k effect ram)
#endif
  numberofParticles = max((uint32_t)1, min(numberofParticles, particlelimit));
  if (isadvanced) // advanced property array needs ram, reduce number of particles to use the same amount
    numberofParticles = (numberofParticles * sizeof(PSparticle)) / (sizeof(PSparticle) + sizeof(PSadvancedParticle));
  if (sizecontrol) // advanced property array needs ram, reduce number of particles to use the same amount
    numberofParticles /= 8; // if size control is used, much fewer particles are needed 

  //make sure it is a multiple of 4 for proper memory alignment (easier than using padding bytes)
  numberofParticles = ((numberofParticles+3) >> 2) << 2;
  return numberofParticles;
}

uint32_t calculateNumberOfSources2D(uint8_t requestedsources)
{
  uint32_t cols = strip.isMatrix ? SEGMENT.virtualWidth() : 1;
  uint32_t rows = strip.isMatrix ? SEGMENT.virtualHeight() : SEGMENT.virtualLength();
#ifdef ESP8266
  int numberofSources = min((cols * rows) / 8, (uint32_t)requestedsources);
  numberofSources = max(1, min(numberofSources, ESP8266_MAXSOURCES)); // limit to 1 - 16
#elif ARDUINO_ARCH_ESP32S2
  int numberofSources = min((cols * rows) / 6, (uint32_t)requestedsources);
  numberofSources = max(1, min(numberofSources, ESP32S2_MAXSOURCES)); // limit to 1 - 48
#else
  int numberofSources = min((cols * rows) / 4, (uint32_t)requestedsources);
  numberofSources = max(1, min(numberofSources, ESP32_MAXSOURCES)); // limit to 1 - 64
#endif
  // make sure it is a multiple of 4 for proper memory alignment
  numberofSources = ((numberofSources+3) >> 2) << 2;
  return numberofSources;
}

//allocate memory for particle system class, particles, sprays plus additional memory requested by FX
bool allocateParticleSystemMemory2D(uint16_t numparticles, uint16_t numsources, bool isadvanced, bool sizecontrol, uint16_t additionalbytes)
{
  uint32_t requiredmemory = sizeof(ParticleSystem);
  // functions above make sure these are a multiple of 4 bytes (to avoid alignment issues)
  requiredmemory += sizeof(PSparticle) * numparticles;
  if (isadvanced)
    requiredmemory += sizeof(PSadvancedParticle) * numparticles;
  if (sizecontrol)
    requiredmemory += sizeof(PSsizeControl) * numparticles;
  requiredmemory += sizeof(PSsource) * numsources;
  requiredmemory += additionalbytes;
  //Serial.print("allocating: ");
  //Serial.print(requiredmemory);
  //Serial.println("Bytes");
  //Serial.print("allocating for segment at");
  //Serial.println((uintptr_t)SEGENV.data);
  return(SEGMENT.allocateData(requiredmemory));
}

// initialize Particle System, allocate additional bytes if needed (pointer to those bytes can be read from particle system class: PSdataEnd)
bool initParticleSystem2D(ParticleSystem *&PartSys, uint8_t requestedsources, uint16_t additionalbytes, bool advanced, bool sizecontrol)
{
  //Serial.println("PS init function");
  uint32_t numparticles = calculateNumberOfParticles2D(advanced, sizecontrol);
  uint32_t numsources = calculateNumberOfSources2D(requestedsources);
  //Serial.print("numsources: ");
  //Serial.println(numsources);
  if (!allocateParticleSystemMemory2D(numparticles, numsources, advanced, sizecontrol, additionalbytes))
  {
    DEBUG_PRINT(F("PS init failed: memory depleted"));
    return false;
  }
  //Serial.print("SEGENV.data ptr");
  //Serial.println((uintptr_t)(SEGENV.data));
  uint16_t cols = strip.isMatrix ? SEGMENT.virtualWidth() : 1;
  uint16_t rows = strip.isMatrix ? SEGMENT.virtualHeight() : SEGMENT.virtualLength();
  //Serial.println("calling constructor");
  PartSys = new (SEGENV.data) ParticleSystem(cols, rows, numparticles, numsources, advanced, sizecontrol); // particle system constructor
  //Serial.print("PS pointer at ");
  //Serial.println((uintptr_t)PartSys);
  return true;
}

#endif // WLED_DISABLE_PARTICLESYSTEM2D


////////////////////////
// 1D Particle System //
////////////////////////
#ifndef WLED_DISABLE_PARTICLESYSTEM1D

ParticleSystem1D::ParticleSystem1D(uint16_t length, uint16_t numberofparticles, uint16_t numberofsources, bool isadvanced) 
{
  //Serial.println("PS Constructor");
  numSources = numberofsources;
  numParticles = numberofparticles; // set number of particles in the array
  usedParticles = numberofparticles; // use all particles by default
  advPartProps = NULL; //make sure we start out with null pointers (just in case memory was not cleared)
  //advPartSize = NULL;
  updatePSpointers(isadvanced); // set the particle and sources pointer (call this before accessing sprays or particles)
  setSize(length);
  setWallHardness(255); // set default wall hardness to max  
  setGravity(0); //gravity disabled by default
  setParticleSize(0); // minimum size by default
  motionBlur = 0; //no fading by default
  emitIndex = 0;

  //initialize some default non-zero values most FX use
  for (uint32_t i = 0; i < numSources; i++)
  {    
    sources[i].source.ttl = 1; //set source alive
  }  
  //Serial.println("PS Constructor done");
}

// update function applies gravity, moves the particles, handles collisions and renders the particles
void ParticleSystem1D::update(void)
{
  PSadvancedParticle1D *advprop = NULL; 
  // handle collisions (can push particles, must be done before updating particles or they can render out of bounds, causing a crash if using local buffer for speed)
  if (particlesettings.useCollisions)
    handleCollisions();

  //apply gravity globally if enabled
  if (particlesettings.useGravity) //note: in 1D system, applying gravity after collisions also works TODO: which one is really better for stacking / oscillations?
    applyGravity();

  //move all particles
  for (uint32_t i = 0; i < usedParticles; i++)
  {
    if (advPartProps)
    {
      advprop = &advPartProps[i];
    }
    particleMoveUpdate(particles[i], &particlesettings, advprop);
  }

  if (particlesettings.colorByPosition)
  {
    for (uint32_t i = 0; i < usedParticles; i++)
    {
      particles[i].hue = (255 * (uint32_t)particles[i].x) / maxX;
    }
  }

  ParticleSys_render();

  uint32_t bg_color = SEGCOLOR(1); //background color, set to black to overlay
  if (bg_color > 0) //if not black
  {
    for(int32_t i = 0; i < maxXpixel + 1; i++)
    {    
      SEGMENT.addPixelColor(i,bg_color); 
    }
  }
}


void ParticleSystem1D::setUsedParticles(uint32_t num)
{
  usedParticles = min(num, numParticles); //limit to max particles
}

void ParticleSystem1D::setWallHardness(uint8_t hardness)
{
  wallHardness = hardness;
}

void ParticleSystem1D::setSize(uint16_t x)
{
  maxXpixel = x - 1; // last physical pixel that can be drawn to
  maxX = x * PS_P_RADIUS_1D - 1;  // particle system boundary for movements
}

void ParticleSystem1D::setWrap(bool enable)
{
  particlesettings.wrapX = enable;
}

void ParticleSystem1D::setBounce(bool enable)
{
  particlesettings.bounceX = enable;
}

void ParticleSystem1D::setKillOutOfBounds(bool enable)
{
  particlesettings.killoutofbounds = enable;
}

void ParticleSystem1D::setColorByAge(bool enable)
{
  particlesettings.colorByAge = enable;
}

void ParticleSystem1D::setColorByPosition(bool enable)
{
  particlesettings.colorByPosition = enable;
}

void ParticleSystem1D::setMotionBlur(uint8_t bluramount)
{
  //TODO: currently normal blurring is not used in 1D system. should it be added? advanced rendering is quite fast and allows for motion blurring
 // if (particlesize < 2) // only allwo motion blurring on default particle size or advanced size(cannot combine motion blur with normal blurring used for particlesize, would require another buffer)
    motionBlur = bluramount; 
}

// render size using smearing (see blur function)
void ParticleSystem1D::setParticleSize(uint8_t size)
{
  particlesize = size;
  particleHardRadius = PS_P_MINHARDRADIUS_1D >> 1; // 1 pixel sized particles have half the radius (for bounce, not for collisions)
  if (particlesize)
    particleHardRadius = particleHardRadius << 1; // 2 pixel sized particles 
  //TODO: since global size rendering is always 1 or 2 pixels, this could maybe be made simpler with a bool 'singlepixelsize'
}
// enable/disable gravity, optionally, set the force (force=8 is default) can be -127 to +127, 0 is disable
// if enabled, gravity is applied to all particles in ParticleSystemUpdate()
// force is in 3.4 fixed point notation so force=16 means apply v+1 each frame default of 8 is every other frame (gives good results)
void ParticleSystem1D::setGravity(int8_t force) 
{  
  if (force)
  {
    gforce = force;
    particlesettings.useGravity = true;
  }
  else 
    particlesettings.useGravity = false;
}

void ParticleSystem1D::enableParticleCollisions(bool enable, uint8_t hardness) // enable/disable gravity, optionally, set the force (force=8 is default) can be 1-255, 0 is also disable
{
  particlesettings.useCollisions = enable;
  collisionHardness = hardness;
}

// emit one particle with variation, returns index of last emitted particle (or -1 if no particle emitted)
int32_t ParticleSystem1D::sprayEmit(PSsource1D &emitter)
{
  for (int32_t i = 0; i < usedParticles; i++)
  {
    emitIndex++;
    if (emitIndex >= usedParticles)
      emitIndex = 0;
    if (particles[emitIndex].ttl == 0) // find a dead particle
    {
      particles[emitIndex].vx = emitter.v + random(-emitter.var, emitter.var);         
      particles[emitIndex].x = emitter.source.x;         
      particles[emitIndex].hue = emitter.source.hue;        
      particles[emitIndex].collide = emitter.source.collide;
      particles[emitIndex].reversegrav = emitter.source.reversegrav;  
      particles[emitIndex].ttl = random16(emitter.minLife, emitter.maxLife);
      if (advPartProps)
      {
        advPartProps[emitIndex].sat = emitter.sat;
        advPartProps[emitIndex].size = emitter.size;
      }
      return i;

      return emitIndex;
    }
  }
  return -1;  
}

// particle moves, decays and dies, if killoutofbounds is set, out of bounds particles are set to ttl=0
// uses passed settings to set bounce or wrap, if useGravity is set, it will never bounce at the top and killoutofbounds is not applied over the top
void ParticleSystem1D::particleMoveUpdate(PSparticle1D &part, PSsettings1D *options,  PSadvancedParticle1D *advancedproperties)
{
  if (options == NULL)
    options = &particlesettings; //use PS system settings by default
  if (part.ttl > 0)
  {
    if (!part.perpetual) 
      part.ttl--; // age
    if (particlesettings.colorByAge)
      part.hue = part.ttl > 250 ? 250 : part.ttl; //set color to ttl 
            
    bool usesize = false; // particle uses individual size rendering    
    int32_t newX = part.x + (int16_t)part.vx;
    part.outofbounds = 0; // reset out of bounds (in case particle was created outside the matrix and is now moving into view)
    if (advancedproperties) //using individual particle size?
    {    
      particleHardRadius = PS_P_MINHARDRADIUS_1D + (advancedproperties->size >> 1);
      if (advancedproperties->size > 1)
      {
        usesize = true; // note: variable eases out of frame checking below
      }
      else if (advancedproperties->size == 0) // single pixel particles use half the collision distance for walls
        particleHardRadius = PS_P_MINHARDRADIUS_1D >> 1;
    }

    // if wall collisions are enabled, bounce them before they reach the edge, it looks much nicer if the particle is not half out of view
    if (options->bounceX) 
    {
      if ((newX < particleHardRadius) || ((newX > maxX - particleHardRadius))) // reached a wall
      {
          bool bouncethis = true;
          if (options->useGravity)
          {
            if (part.reversegrav) //skip at x = 0            
            {
              if (newX < particleHardRadius) 
                bouncethis = false;          
            }
            else //skip at x = max                   
            {
              if (newX > particleHardRadius) 
                bouncethis = false;
            }
          }
          
          if (bouncethis)
          {
            part.vx = -part.vx; //invert speed
            part.vx = ((int32_t)part.vx * wallHardness) / 255; // reduce speed as energy is lost on non-hard surface
            if (newX < particleHardRadius)
              newX = particleHardRadius; // fast particles will never reach the edge if position is inverted, this looks better
            else
              newX = maxX - particleHardRadius;
          }
      }
    }
    if ((newX < 0) || (newX > maxX)) // check if particle reached an edge (note: this also checks out of bounds and must not be skipped, even if bounce is enabled)
    {      
      if (options->wrapX)
      {      
        newX = newX % (maxX + 1); 
        if (newX < 0)
          newX += maxX + 1; 
      }
      else if (((newX <= -PS_P_HALFRADIUS_1D) || (newX > maxX + PS_P_HALFRADIUS_1D))) // particle is leaving, set out of bounds if it has fully left
      {
          bool isleaving = true;
          if (usesize) // using individual particle size
          {
            if (((newX > -particleHardRadius) && (newX < maxX + particleHardRadius))) // large particle is not yet leaving the view - note: this is not pixel perfect but good enough
              isleaving = false; 
          }
          if (isleaving)
          {
            part.outofbounds = 1;
            if (options->killoutofbounds)
            {      
              bool killthis = true;
              if (options->useGravity) //if gravity is used, only kill below 'floor level'
              {
                if (part.reversegrav) //skip at x = 0            
                {
                  if (newX < 0) 
                    killthis = false;          
                }
                else //skip at x = max                   
                {
                  if (newX > 0) 
                    killthis = false;
                }
              }
              if (killthis)
                part.ttl = 0;            
            }
          }
      }
    }
    if (!part.fixed)
      part.x = (int16_t)newX; // set new position
    else
      part.vx = 0; //set speed to zero. note: particle can get speed in collisions, if unfixed, it should not speed away
  }
}

// apply a force in x direction to individual particle (or source)
// caller needs to provide a 8bit counter (for each paticle) that holds its value between calls
// force is in 3.4 fixed point notation so force=16 means apply v+1 each frame default of 8 is every other frame
void ParticleSystem1D::applyForce(PSparticle1D *part, int8_t xforce, uint8_t *counter)
{
  // velocity increase
  int32_t dv = calcForce_dv(xforce, counter);
  // apply the force to particle
  part->vx = limitSpeed((int32_t)part->vx + dv);
}

// apply a force to all particles
// force is in 3.4 fixed point notation (see above)
void ParticleSystem1D::applyForce(int8_t xforce)
{
  // for small forces, need to use a delay counter
  uint8_t tempcounter;
  // note: this is not the most compuatationally efficient way to do this, but it saves on duplacte code and is fast enough
  for (uint i = 0; i < usedParticles; i++)
  {
    tempcounter = forcecounter;
    applyForce(&particles[i], xforce, &tempcounter);
  }  
  forcecounter = tempcounter; //save value back
}

// apply gravity to all particles using PS global gforce setting
// gforce is in 3.4 fixed point notation, see note above
void ParticleSystem1D::applyGravity()
{
  int32_t dv_raw = calcForce_dv(gforce, &gforcecounter);
  for (uint32_t i = 0; i < usedParticles; i++)
  {
    int32_t dv = dv_raw;
    if (particles[i].reversegrav) dv = -dv_raw;
    // note: not checking if particle is dead is faster as most are usually alive and if few are alive, rendering is fast anyways
    particles[i].vx = limitSpeed((int32_t)particles[i].vx - dv);
  }  
}

// apply gravity to single particle using system settings (use this for sources)
// function does not increment gravity counter, if gravity setting is disabled, this cannot be used
void ParticleSystem1D::applyGravity(PSparticle1D *part)
{
  uint32_t counterbkp = gforcecounter;
  int32_t dv = calcForce_dv(gforce, &gforcecounter);
  if (part->reversegrav) dv = -dv; 
  gforcecounter = counterbkp; //save it back 
  part->vx = limitSpeed((int32_t)part->vx - dv);
}


// slow down particle by friction, the higher the speed, the higher the friction. a high friction coefficient slows them more (255 means instant stop)
// note: a coefficient smaller than 0 will speed them up (this is a feature, not a bug), coefficient larger than 255 inverts the speed, so don't do that
void ParticleSystem1D::applyFriction(int32_t coefficient)
{
  int32_t friction = 255 - coefficient;
  for (uint32_t i = 0; i < usedParticles; i++)
  {
    if (particles[i].ttl)
       particles[i].vx = ((int16_t)particles[i].vx * friction) / 255; 
  }
}


// render particles to the LED buffer (uses palette to render the 8bit particle color value)
// if wrap is set, particles half out of bounds are rendered to the other side of the matrix
// warning: do not render out of bounds particles or system will crash! rendering does not check if particle is out of bounds
void ParticleSystem1D::ParticleSys_render()
{
  
  CRGB baseRGB;
  bool useLocalBuffer = true; //use local rendering buffer, gives huge speed boost (at least 30% more FPS)
  CRGB *framebuffer = NULL; //local frame buffer
  CRGB *renderbuffer = NULL; //local particle render buffer for advanced particles
  uint32_t i;
  uint32_t brightness; // particle brightness, fades if dying
  
  if (useLocalBuffer)
  {    

    // allocate empty memory for the local renderbuffer
    framebuffer = allocate1Dbuffer(maxXpixel + 1);
    if (framebuffer == NULL)
    {
      //Serial.println("Frame buffer alloc failed");
      useLocalBuffer = false; //render to segment pixels directly if not enough memory
    }
    else{
      if (advPartProps)
      {  
        renderbuffer = allocate1Dbuffer(10); //buffer to render individual particles to if size > 0. note: null checking is done when accessing it
      }
      if (motionBlur > 0) // using SEGMENT.fadeToBlackBy is much slower, this approximately doubles the speed of fade calculation
      {        
        for (uint32_t x = 0; x <= maxXpixel; x++)
        {
          framebuffer[x] = SEGMENT.getPixelColor(x); //copy to local buffer
          fast_color_scale(framebuffer[x], motionBlur);
        }
      }
    }
  }
  
  if (!useLocalBuffer) //disabled or allocation above failed
  {
    //Serial.println("NOT using local buffer!");
    if (motionBlur > 0)
      SEGMENT.fadeToBlackBy(255 - motionBlur);
    else
      SEGMENT.fill(BLACK); //clear the buffer before rendering to it 
  }
  // go over particles and render them to the buffer
  for (i = 0; i < usedParticles; i++)
  {
    if (particles[i].outofbounds || particles[i].ttl == 0)
      continue;

    // generate RGB values for particle
    brightness = particles[i].ttl > 255 ? 255 : particles[i].ttl; //faster then using min()
    baseRGB = ColorFromPalette(SEGPALETTE, particles[i].hue, 255, LINEARBLEND);
    
    if (advPartProps) //saturation is advanced property in 1D system
    {
      if (advPartProps[i].sat < 255) 
      {
        CHSV baseHSV = rgb2hsv_approximate(baseRGB); //convert to HSV
        baseHSV.s = advPartProps[i].sat; //set the saturation
        baseRGB = (CRGB)baseHSV; // convert back to RGB
      }
    }
    renderParticle(framebuffer, i, brightness, baseRGB, renderbuffer);
  }

  if (useLocalBuffer) // transfer local buffer back to segment
  {
    for (int x = 0; x <= maxXpixel; x++)
    {
      SEGMENT.setPixelColor(x, framebuffer[x]);
    }    
    free(framebuffer); 
  }
  if (renderbuffer)
    free(renderbuffer); 
}

// calculate pixel positions and brightness distribution and render the particle to local buffer or global buffer
void ParticleSystem1D::renderParticle(CRGB *framebuffer, uint32_t particleindex, uint32_t brightness, CRGB color, CRGB *renderbuffer)
{
  uint32_t size = particlesize;
  if (advPartProps) // use advanced size properties
  {
    size = advPartProps[particleindex].size;
  }
  if (size == 0) //single pixel particle, can be out of bounds as oob checking is made for 2-pixel particles
  {
    uint32_t x =  particles[particleindex].x >> PS_P_RADIUS_SHIFT_1D;
    if (x <= maxXpixel) //by making x unsigned there is no need to check < 0 as it will overflow
    {    
      if (framebuffer)      
        fast_color_add(framebuffer[x], color, brightness);       
      else        
        SEGMENT.addPixelColor(x, color.scale8((uint8_t)brightness));      
    } 
  }
  else { //render larger particles
    int32_t pxlbrightness[2] = {0}; // note: pxlbrightness needs to be set to 0 or checking does not work
    int32_t pixco[2]; // physical pixel coordinates of the two pixels representing a particle    
    // subtract half a radius as the rendering algorithm always starts at the left, this makes calculations more efficient
    int32_t xoffset = particles[particleindex].x - PS_P_HALFRADIUS_1D;
    int32_t dx = xoffset % PS_P_RADIUS_1D; //relativ particle position in subpixel space
    int32_t x = xoffset >> PS_P_RADIUS_SHIFT_1D; // divide by PS_P_RADIUS which is 64, so can bitshift (compiler may not optimize automatically)
    
    // set the raw pixel coordinates
    pixco[0] = x;      // left pixel
    pixco[1] = x + 1;  // right pixel
    
    // now check if any are out of frame. set values to -1 if they are so they can be easily checked after (no value calculation, no setting of pixelcolor if value < 0)  
    if (x < 0) // left pixels out of frame
    {
      dx = PS_P_RADIUS_1D + dx; // if x<0, xoffset becomes negative (and so does dx), must adjust dx as modulo will flip its value 
      // note: due to inverted shift math, a particel at position -32 (xoffset = -64, dx = 64) is rendered at the wrong pixel position (it should be out of frame)
      // checking this above makes this algorithm slower (in frame pixels do not have to be checked), so just correct for it here:
      if (dx == PS_P_RADIUS_1D)
      {
        pxlbrightness[1] = -1; // pixel is actually out of matrix boundaries, do not render
      }
      if (particlesettings.wrapX) // wrap x to the other side if required
        pixco[0] = maxXpixel;
      else
        pxlbrightness[0] = -1; // pixel is out of matrix boundaries, do not render
    }
    else if (pixco[1] > maxXpixel) // right pixel, only has to be checkt if left pixel did not overflow
    {
      if (particlesettings.wrapX) // wrap y to the other side if required
        pixco[1] = 0;
      else
        pxlbrightness[1] = -1;
    }

    // calculate brightness values for the two pixels representing a particle using linear interpolation

    //calculate the values for pixels that are in frame
    if (pxlbrightness[0] >= 0)
      pxlbrightness[0] = (((int32_t)PS_P_RADIUS_1D - dx) * brightness) >> PS_P_SURFACE_1D; 
    if (pxlbrightness[1] >= 0)
      pxlbrightness[1] = (dx * brightness) >> PS_P_SURFACE_1D; 

    // check if particle has advanced size properties and buffer is available
    if (advPartProps && advPartProps[particleindex].size > 1)
    {
      if (renderbuffer && framebuffer)
      {
        memset(renderbuffer, 0, 10 * sizeof(CRGB)); // clear the buffer, renderbuffer is 10 pixels
      }
      else
        return; // cannot render advanced particles without buffer
    

      //render particle to a bigger size
      //particle size to pixels: < 64 is 4 pixels, < 128 is 6pixels, < 192 is 8 pixels, bigger is 10 pixels
      //first, render the pixel to the center of the renderbuffer, then apply 1D blurring
      fast_color_add(renderbuffer[4], color, pxlbrightness[0]); 
      fast_color_add(renderbuffer[5], color, pxlbrightness[1]);      
      uint32_t rendersize = 2; // initialize render size, minimum is 4x4 pixels, it is incremented int he loop below to start with 4
      uint32_t offset = 4; // offset to zero coordinate to write/read data in renderbuffer (actually needs to be 3, is decremented in the loop below)
      uint32_t blurpasses = size/64 + 1; // number of blur passes depends on size, four passes max
      uint32_t bitshift = 0;
      for(int i = 0; i < blurpasses; i++)
      {
        if (i == 2) //for the last two passes, use higher amount of blur (results in a nicer brightness gradient with soft edges)
          bitshift = 1;
        rendersize += 2;
        offset--;
        blur1D(renderbuffer, rendersize, size << bitshift, true, offset); 
        size = size > 64 ? size - 64 : 0; 
      }
      
      // calculate origin coordinates to render the particle to in the framebuffer
      uint32_t xfb_orig = x - (rendersize>>1) + 1 - offset;
      uint32_t xfb; // coordinates in frame buffer to write to note: by making this uint, only overflow has to be checked

      // transfer particle renderbuffer to framebuffer
      for(uint32_t xrb = offset; xrb < rendersize+offset; xrb++)
      {
        xfb = xfb_orig + xrb;
        if (xfb > maxXpixel)
        {
          if (particlesettings.wrapX) // wrap x to the other side if required
          {           
            if (xfb > maxXpixel << 1) // xfb is "negative" (note: for some reason, this check is needed in 1D but works without in 2D...)
              xfb = (maxXpixel +1) + (int32_t)xfb;
            else
              xfb = xfb % (maxXpixel + 1); 
          }
          else
            continue;
        }
        fast_color_add(framebuffer[xfb], renderbuffer[xrb]); 
      }
    }
    else if (framebuffer) // standard rendering (2 pixels per particle)
    {
      for(uint32_t i = 0; i < 2; i++)
      {
        if (pxlbrightness[i] > 0)
          fast_color_add(framebuffer[pixco[i]], color, pxlbrightness[i]); // order is: bottom left, bottom right, top right, top left
      }
    }
    else
    {  
      for(uint32_t i = 0; i < 2; i++)
      {
        if (pxlbrightness[i] > 0)
          SEGMENT.addPixelColor(pixco[i], color.scale8((uint8_t)pxlbrightness[i])); 
      }
    }
  }
}

// detect collisions in an array of particles and handle them
void ParticleSystem1D::handleCollisions()
{
  // detect and handle collisions
  uint32_t i, j;
  uint32_t startparticle = 0;
  uint32_t endparticle = usedParticles;// >> 1; // do half the particles, significantly speeds things up
  // every second frame, do other half of particles (helps to speed things up as not all collisions are handled each frame, less accurate but good enough)
  // if more accurate collisions are needed, just call it twice in a row
  /*if (SEGMENT.call & 0x01)  //every other frame, do the other half
  { 
    startparticle = endparticle;
    endparticle = usedParticles;
  }  */
  int32_t collisiondistance = PS_P_MINHARDRADIUS_1D;
    
  for (i = startparticle; i < endparticle; i++)
  {
    // go though all 'higher number' particles and see if any of those are in close proximity and if they are, make them collide
    if (particles[i].ttl > 0 && particles[i].outofbounds == 0  && particles[i].collide) // if particle is alive and does collide and is not out of view
    {
      int32_t dx; // distance to other particles    
      for (j = i + 1; j < usedParticles; j++) // check against higher number particles
      {                
        if (particles[j].ttl > 0  && particles[j].collide) // if target particle is alive
        {
          if (advPartProps) // use advanced size properties
          {
            collisiondistance = PS_P_MINHARDRADIUS_1D + ((uint32_t)advPartProps[i].size + (uint32_t)advPartProps[j].size)>>1;
          }
          dx = particles[j].x - particles[i].x;  
          int32_t  dv = (int32_t)particles[j].vx - (int32_t)particles[i].vx;        
          int32_t proximity = collisiondistance;
          if (dv >= proximity) //particles would go past each other in next move upate
            proximity += abs(dv); //add speed difference to catch fast particles
          if (dx < proximity && dx > -proximity) // check if close
          {            
              collideParticles(&particles[i], &particles[j], dx, dv, collisiondistance);
          }
        }
      }
    }
  }
}

// handle a collision if close proximity is detected, i.e. dx and/or dy smaller than 2*PS_P_RADIUS
// takes two pointers to the particles to collide and the particle hardness (softer means more energy lost in collision, 255 means full hard)
void ParticleSystem1D::collideParticles(PSparticle1D *particle1, PSparticle1D *particle2, int32_t dx, int32_t relativeVx, uint32_t collisiondistance) 
{

  // Calculate dot product of relative velocity and relative distance
  int32_t dotProduct = (dx * relativeVx); // is always negative if moving towards each other
 // int32_t notsorandom = dotProduct & 0x01; //dotprouct LSB should be somewhat random, so no need to calculate a random number
 //Serial.print(" dp"); Serial.print(dotProduct); 
  if (dotProduct < 0) // particles are moving towards each other
  {
    // integer math used to avoid floats.     
    // Calculate new velocities after collision
    uint32_t surfacehardness = collisionHardness < PS_P_MINSURFACEHARDNESS_1D ? PS_P_MINSURFACEHARDNESS_1D : collisionHardness; // if particles are soft, the impulse must stay above a limit or collisions slip through
    //TODO: if soft collisions are not needed, the above line can be done in set hardness function and skipped here (which is what it currently looks like)
        
    int32_t impulse = relativeVx * surfacehardness / 255;     
    particle1->vx += impulse; 
    particle2->vx -= impulse;
    
    //if one of the particles is fixed, transfer the impulse back so it bounces
    if (particle1->fixed)
      particle2->vx = -particle1->vx;
    else if (particle2->fixed)
      particle1->vx = -particle2->vx;

    if (collisionHardness < PS_P_MINSURFACEHARDNESS_1D) // if particles are soft, they become 'sticky' i.e. apply some friction (they do pile more nicely and correctly)
    {
      const uint32_t coeff = collisionHardness + (255 - PS_P_MINSURFACEHARDNESS_1D);
      particle1->vx = ((int32_t)particle1->vx * coeff) / 255; 
      particle2->vx = ((int32_t)particle2->vx * coeff) / 255;
    }    
  }

  uint32_t distance = abs(dx);  
  // particles have volume, push particles apart if they are too close 
  // behaviour is different than in 2D, we need pixel accurate stacking here, push the top particle to full radius (direction is well defined in 1D)
  // also need to give the top particle some speed to counteract gravity or stacks just collapse
  if (distance <  collisiondistance) //particles are too close, push the upper particle away
  {      
    int32_t pushamount = 1 + ((collisiondistance - distance) >> 1); //add half the remaining distance note: this works best, if less or more is added, it gets more chaotic
    //int32_t pushamount = collisiondistance - distance;
    if (particlesettings.useGravity) //using gravity, push the 'upper' particle only
    {
      if (dx < 0)  // particle2.x < particle1.x
      {
          if (particle2->reversegrav && !particle2->fixed)
          {
            particle2->x -= pushamount;
            particle2->vx--;
          }
          else if (!particle1->reversegrav && !particle1->fixed)
          {          
            particle1->x += pushamount;
            particle1->vx++;
          } 
      }
      else
      {
        if (particle1->reversegrav && !particle1->fixed)
        {
          particle1->x -= pushamount;
          particle1->vx--;
        }
        else if (!particle2->reversegrav  && !particle2->fixed)
        {
          particle2->x += pushamount;
          particle2->vx++;
        }
      }
    }
    else //not using gravity, push both particles by applying a little velocity (like in 2D system), results in much nicer stacking when applying forces
    {
      pushamount = 1;
      if (dx < 0)  // particle2.x < particle1.x
        pushamount = -1;

      particle1->vx -= pushamount;
      particle2->vx += pushamount;
    }     
  }
}



// allocate memory for the 1D array in one contiguous block and set values to zero
CRGB *ParticleSystem1D::allocate1Dbuffer(uint32_t length)
{  
  CRGB *array = (CRGB *)calloc(length, sizeof(CRGB));
  //if (array == NULL)
  //  DEBUG_PRINT(F("PS 1D buffer alloc failed"));  
  return array;
}

// update size and pointers (memory location and size can change dynamically)
// note: do not access the PS class in FX befor running this function (or it messes up SEGENV.data)
void ParticleSystem1D::updateSystem(void)
{
  // update size
  setSize(SEGMENT.virtualLength());
  updatePSpointers(advPartProps != NULL);
}

// set the pointers for the class (this only has to be done once and not on every FX call, only the class pointer needs to be reassigned to SEGENV.data every time)
// function returns the pointer to the next byte available for the FX (if it assigned more memory for other stuff using the above allocate function)
// FX handles the PSsources, need to tell this function how many there are
void ParticleSystem1D::updatePSpointers(bool isadvanced)
{
  // DEBUG_PRINT(F("*** PS pointers ***"));
  // DEBUG_PRINTF_P(PSTR("this PS %p "), this);
  // Note on memory alignment:
  // a pointer MUST be 4 byte aligned. sizeof() in a struct/class is always aligned to the largest element. if it contains a 32bit, it will be padded to 4 bytes, 16bit is padded to 2byte alignment.
  // The PS is aligned to 4 bytes, a PSparticle is aligned to 2 and a struct containing only byte sized variables is not aligned at all and may need to be padded when dividing the memoryblock.
  // by making sure that the number of sources and particles is a multiple of 4, padding can be skipped here as alignent is ensured, independent of struct sizes.
  particles = reinterpret_cast<PSparticle1D *>(this + 1); // pointer to particle array at data+sizeof(ParticleSystem)
  sources = reinterpret_cast<PSsource1D *>(particles + numParticles); // pointer to source(s)
  PSdataEnd = reinterpret_cast<uint8_t *>(sources + numSources); // pointer to first available byte after the PS for FX additional data
  if (isadvanced)
  {
    advPartProps = reinterpret_cast<PSadvancedParticle1D *>(sources + numSources);
    PSdataEnd = reinterpret_cast<uint8_t *>(advPartProps + numParticles);
    //if (sizecontrol)
    //{
    //  advPartSize = reinterpret_cast<PSsizeControl *>(advPartProps + numParticles);
    //  PSdataEnd = reinterpret_cast<uint8_t *>(advPartSize + numParticles);
    //}     
  }
  
  /*
  DEBUG_PRINTF_P(PSTR(" particles %p "), particles);
  DEBUG_PRINTF_P(PSTR(" sources %p "), sources);
  DEBUG_PRINTF_P(PSTR(" adv. props %p "), advPartProps);
  DEBUG_PRINTF_P(PSTR(" adv. ctrl %p "), advPartSize);
  DEBUG_PRINTF_P(PSTR("end %p\n"), PSdataEnd);
  */
}


//non class functions to use for initialization
uint32_t calculateNumberOfParticles1D(bool isadvanced)
{
   uint32_t numberofParticles = SEGMENT.virtualLength();  // one particle per pixel (if possible)
#ifdef ESP8266
  uint32_t particlelimit = ESP8266_MAXPARTICLES_1D; // maximum number of paticles allowed 
#elif ARDUINO_ARCH_ESP32S2
  uint32_t particlelimit = ESP32S2_MAXPARTICLES_1D; // maximum number of paticles allowed 
#else
  uint32_t particlelimit = ESP32_MAXPARTICLES_1D; // maximum number of paticles allowed 
#endif
  numberofParticles = max((uint32_t)1, min(numberofParticles, particlelimit));
  if (isadvanced) // advanced property array needs ram, reduce number of particles to use the same amount
    numberofParticles = (numberofParticles * sizeof(PSparticle1D)) / (sizeof(PSparticle1D) + sizeof(PSadvancedParticle1D));
  //make sure it is a multiple of 4 for proper memory alignment (easier than using padding bytes)
  numberofParticles = ((numberofParticles+3) >> 2) << 2;
  return numberofParticles;
}

uint32_t calculateNumberOfSources1D(uint8_t requestedsources)
{  
#ifdef ESP8266  
   int numberofSources = max(1, min((int)requestedsources,ESP8266_MAXSOURCES_1D)); // limit to 1 - 8
#elif ARDUINO_ARCH_ESP32S2
  int numberofSources = max(1, min((int)requestedsources, ESP32S2_MAXSOURCES_1D)); // limit to 1 - 16
#else  
  int numberofSources = max(1, min((int)requestedsources, ESP32_MAXSOURCES_1D)); // limit to 1 - 32
#endif
  // make sure it is a multiple of 4 for proper memory alignment
  numberofSources = ((numberofSources+3) >> 2) << 2;
  return numberofSources;
}

//allocate memory for particle system class, particles, sprays plus additional memory requested by FX
bool allocateParticleSystemMemory1D(uint16_t numparticles, uint16_t numsources, bool isadvanced, uint16_t additionalbytes)
{
  uint32_t requiredmemory = sizeof(ParticleSystem1D);
  // functions above make sure these are a multiple of 4 bytes (to avoid alignment issues)
  requiredmemory += sizeof(PSparticle1D) * numparticles;
  if (isadvanced)
    requiredmemory += sizeof(PSadvancedParticle1D) * numparticles;
  requiredmemory += sizeof(PSsource1D) * numsources;
  requiredmemory += additionalbytes;
  //Serial.print("allocating: ");
  //Serial.print(requiredmemory);
  //Serial.println("Bytes");
  //Serial.print("allocating for segment at");
  //Serial.println((uintptr_t)SEGENV.data);
  return(SEGMENT.allocateData(requiredmemory));
}

// initialize Particle System, allocate additional bytes if needed (pointer to those bytes can be read from particle system class: PSdataEnd)
bool initParticleSystem1D(ParticleSystem1D *&PartSys, uint8_t requestedsources, uint16_t additionalbytes, bool advanced)
{
  //Serial.println("PS init function");
  uint32_t numparticles = calculateNumberOfParticles1D(advanced);
  uint32_t numsources = calculateNumberOfSources1D(requestedsources);
  //Serial.print("numsources: ");
  //Serial.println(numsources);
  if (!allocateParticleSystemMemory1D(numparticles, numsources, advanced, additionalbytes))
  {
    DEBUG_PRINT(F("PS init failed: memory depleted"));
    return false;
  }
  //Serial.print("SEGENV.data ptr");
  //Serial.println((uintptr_t)(SEGENV.data));
  //Serial.println("calling constructor");
  PartSys = new (SEGENV.data) ParticleSystem1D(SEGMENT.virtualLength(), numparticles, numsources, advanced); // particle system constructor
  //Serial.print("PS pointer at ");
  //Serial.println((uintptr_t)PartSys);
  return true;
}


// blur a 1D buffer, sub-size blurring can be done using start and size 
// for speed, 32bit variables are used, make sure to limit them to 8bit (0-255) or result is undefined 
// to blur a subset of the buffer, change the size and set start to the desired starting coordinates (default start is 0/0)
void blur1D(CRGB *colorbuffer, uint32_t size, uint32_t blur, bool smear, uint32_t start)
{
  CRGB seeppart, carryover;
  uint32_t seep = blur >> 1;  

    carryover =  BLACK;
    for(uint32_t x = start; x < start + size; x++)
    {
      seeppart = colorbuffer[x]; // create copy of current color
      fast_color_scale(seeppart, seep); // scale it and seep to neighbours
      if (!smear) // fade current pixel if smear is disabled
        fast_color_scale(colorbuffer[x], 255 - blur); 
      if (x > 0)
      {
        fast_color_add(colorbuffer[x-1], seeppart);        
        fast_color_add(colorbuffer[x], carryover); // is black on first pass
      }
      carryover = seeppart;
    }
    fast_color_add(colorbuffer[size-1], carryover); // set last pixel
}

#endif // WLED_DISABLE_PARTICLESYSTEM1D


#if  !defined(WLED_DISABLE_PARTICLESYSTEM2D) || !defined(WLED_DISABLE_PARTICLESYSTEM1D)  

//////////////////////////////
// Shared Utility Functions //
//////////////////////////////

// calculate the delta speed (dV) value and update the counter for force calculation (is used several times, function saves on codesize)
// force is in 3.4 fixedpoint notation, +/-127
int32_t calcForce_dv(int8_t force, uint8_t* counter)
{
  if (force == 0) 
    return 0;
  // for small forces, need to use a delay counter
  int32_t force_abs = abs(force); // absolute value (faster than lots of if's only 7 instructions)  
  int32_t dv = 0;
  // for small forces, need to use a delay counter, apply force only if it overflows
  if (force_abs < 16)
  {
    *counter += force_abs;
    if (*counter > 15)
    {
      *counter -= 16;
      dv = force < 0 ? -1 : 1; // force is either, 1 or -1 if it is small (zero force is handled above)
    }
  }
  else
  {
    dv = force / 16; // MSBs note: cannot use bitshift as dv can be negative
  }
  return dv;
}

// limit speed to prevent overflows
int32_t limitSpeed(int32_t speed)
{
  return speed > PS_P_MAXSPEED ? PS_P_MAXSPEED : (speed < -PS_P_MAXSPEED ? -PS_P_MAXSPEED : speed);
}

// fastled color adding is very inaccurate in color preservation
// a better color add function is implemented in colors.cpp but it uses 32bit RGBW. to use it colors need to be shifted just to then be shifted back by that function, which is slow
// this is a fast version for RGB (no white channel, PS does not handle white) and with native CRGB including scaling of second color (fastled scale8 can be made faster using native 32bit on ESP)
// note: result is stored in c1, so c1 will contain the result. not using a return value is much faster as the struct does not need to be copied upon return
void fast_color_add(CRGB &c1, CRGB &c2, uint32_t scale)
{
  uint32_t r, g, b;
  if (scale < 255) {
    r = c1.r + ((c2.r * scale) >> 8);
    g = c1.g + ((c2.g * scale) >> 8);
    b = c1.b + ((c2.b * scale) >> 8);
  }
  else {
    r = c1.r + c2.r;
    g = c1.g + c2.g;
    b = c1.b + c2.b;
  }
  uint32_t max = r;
  if (g > max) // note: using ? operator would be slower by 2 instructions
    max = g;
  if (b > max)
    max = b;
  if (max < 256)
  {
    c1.r = r; // save result to c1
    c1.g = g;
    c1.b = b;
  }
  else
  {
    c1.r = (r * 255) / max;
    c1.g = (g * 255) / max;
    c1.b = (b * 255) / max;
  }
}

// faster than fastled color scaling as it uses a 32bit scale factor and pointer
void fast_color_scale(CRGB &c, uint32_t scale)
{
  c.r = ((c.r * scale) >> 8);
  c.g = ((c.g * scale) >> 8);
  c.b = ((c.b * scale) >> 8);
}

#endif  // !defined(WLED_DISABLE_PARTICLESYSTEM2D) || !defined(WLED_DISABLE_PARTICLESYSTEM1D)
