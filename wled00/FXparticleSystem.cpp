/*
  FXparticleSystem.cpp

  Particle system with functions for particle generation, particle movement and particle rendering to RGB matrix.
  by DedeHai (Damian Schneider) 2013-2024

  Copyright (c) 2024  Damian Schneider
  Licensed under the EUPL v. 1.2 or later
*/

/*
  TODO:
  - change passing particle pointers to references (if possible)
  -add underscore to private variables
*/

#if !defined(WLED_DISABLE_PARTICLESYSTEM2D) || !defined(WLED_DISABLE_PARTICLESYSTEM1D)
#include "FXparticleSystem.h"
#endif

#ifndef WLED_DISABLE_PARTICLESYSTEM2D

ParticleSystem2D::ParticleSystem2D(uint16_t width, uint16_t height, uint16_t numberofparticles, uint16_t numberofsources, bool isadvanced, bool sizecontrol) {
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
  for (uint32_t i = 0; i < numSources; i++) {
    sources[i].source.sat = 255; //set saturation to max by default
    sources[i].source.ttl = 1; //set source alive
  }
  for (uint32_t i = 0; i < numParticles; i++) {
     particles[i].sat = 255; // full saturation
  }
}

// update function applies gravity, moves the particles, handles collisions and renders the particles
void ParticleSystem2D::update(void) {
  PSadvancedParticle *advprop = NULL;
  //apply gravity globally if enabled
  if (particlesettings.useGravity)
    applyGravity();

  //update size settings before handling collisions
  if (advPartSize) {
    for (uint32_t i = 0; i < usedParticles; i++) {
      updateSize(&advPartProps[i], &advPartSize[i]);
    }
  }

  // handle collisions (can push particles, must be done before updating particles or they can render out of bounds, causing a crash if using local buffer for speed)
  if (particlesettings.useCollisions)
    handleCollisions();

  //move all particles
  for (uint32_t i = 0; i < usedParticles; i++) {
    if (advPartProps) {
      advprop = &advPartProps[i];
    }
    particleMoveUpdate(particles[i], &particlesettings, advprop);
  }

  ParticleSys_render();
}

// update function for fire animation
void ParticleSystem2D::updateFire(uint32_t intensity, bool renderonly) {
  if (!renderonly)
    fireParticleupdate();
  ParticleSys_render(true, intensity);
}

void ParticleSystem2D::setUsedParticles(uint32_t num) {
  usedParticles = min(num, numParticles); //limit to max particles
}

void ParticleSystem2D::setWallHardness(uint8_t hardness) {
  wallHardness = hardness;
}

void ParticleSystem2D::setWallRoughness(uint8_t roughness) {
  wallRoughness = roughness;
}

void ParticleSystem2D::setCollisionHardness(uint8_t hardness) {
  collisionHardness = (int)hardness + 1;
}

void ParticleSystem2D::setMatrixSize(uint16_t x, uint16_t y) {
  maxXpixel = x - 1; // last physical pixel that can be drawn to
  maxYpixel = y - 1;
  maxX = x * PS_P_RADIUS - 1;  // particle system boundary for movements
  maxY = y * PS_P_RADIUS - 1;  // this value is often needed (also by FX) to calculate positions
}

void ParticleSystem2D::setWrapX(bool enable) {
  particlesettings.wrapX = enable;
}

void ParticleSystem2D::setWrapY(bool enable) {
  particlesettings.wrapY = enable;
}

void ParticleSystem2D::setBounceX(bool enable) {
  particlesettings.bounceX = enable;
}

void ParticleSystem2D::setBounceY(bool enable) {
  particlesettings.bounceY = enable;
}

void ParticleSystem2D::setKillOutOfBounds(bool enable) {
  particlesettings.killoutofbounds = enable;
}

void ParticleSystem2D::setColorByAge(bool enable) {
  particlesettings.colorByAge = enable;
}

void ParticleSystem2D::setMotionBlur(uint8_t bluramount) {
  if (particlesize == 0) // only allow motion blurring on default particle size or advanced size (cannot combine motion blur with normal blurring used for particlesize, would require another buffer)
    motionBlur = bluramount;
}

// render size using smearing (see blur function)
void ParticleSystem2D::setParticleSize(uint8_t size) {
  particlesize = size;
  particleHardRadius = PS_P_MINHARDRADIUS + (particlesize >> 1); // radius used for wall collisions & particle collisions
  motionBlur = 0; // disable motion blur if particle size is set
}

// enable/disable gravity, optionally, set the force (force=8 is default) can be -127 to +127, 0 is disable
// if enabled, gravity is applied to all particles in ParticleSystemUpdate()
// force is in 3.4 fixed point notation so force=16 means apply v+1 each frame default of 8 is every other frame (gives good results)
void ParticleSystem2D::setGravity(int8_t force) {
  if (force) {
    gforce = force;
    particlesettings.useGravity = true;
  } else {
    particlesettings.useGravity = false;
  }
}

void ParticleSystem2D::enableParticleCollisions(bool enable, uint8_t hardness) { // enable/disable gravity, optionally, set the force (force=8 is default) can be 1-255, 0 is also disable
  particlesettings.useCollisions = enable;
  collisionHardness = (int)hardness + 1;
}

// emit one particle with variation, returns index of last emitted particle (or -1 if no particle emitted)
int32_t ParticleSystem2D::sprayEmit(PSsource &emitter, uint32_t amount) {
  bool success = false;
  for (uint32_t a = 0; a < amount; a++) {
    for (uint32_t i = 0; i < usedParticles; i++) {
      emitIndex++;
      if (emitIndex >= usedParticles)
        emitIndex = 0;
      if (particles[emitIndex].ttl == 0) { // find a dead particle
        success = true;
        particles[emitIndex].vx = emitter.vx + random16(emitter.var << 1) - emitter.var; // random(-var, var)
        particles[emitIndex].vy = emitter.vy + random16(emitter.var << 1) - emitter.var; // random(-var, var)
        particles[emitIndex].x = emitter.source.x;
        particles[emitIndex].y = emitter.source.y;
        particles[emitIndex].hue = emitter.source.hue;
        particles[emitIndex].sat = emitter.source.sat;
        particles[emitIndex].collide = emitter.source.collide;
        particles[emitIndex].ttl = random16(emitter.minLife, emitter.maxLife);
        if (advPartProps)
          advPartProps[emitIndex].size = emitter.size;
        break;
      }
    }
  }
  if (success)
    return emitIndex;
  else
    return -1;
}

// Spray emitter for particles used for flames (particle TTL depends on source TTL)
void ParticleSystem2D::flameEmit(PSsource &emitter) {
  int emitIndex = sprayEmit(emitter);
  if(emitIndex > 0)  particles[emitIndex].ttl +=  emitter.source.ttl;
}

// Emits a particle at given angle and speed, angle is from 0-65535 (=0-360deg), speed is also affected by emitter->var
// angle = 0 means in positive x-direction (i.e. to the right)
int32_t ParticleSystem2D::angleEmit(PSsource &emitter, uint16_t angle, int32_t speed, uint32_t amount) {
  emitter.vx = ((int32_t)cos16(angle) * speed) / (int32_t)32600; // cos16() and sin16() return signed 16bit, division should be 32767 but 32600 gives slightly better rounding
  emitter.vy = ((int32_t)sin16(angle) * speed) / (int32_t)32600; // note: cannot use bit shifts as bit shifting is asymmetrical for positive and negative numbers and this needs to be accurate!
  return sprayEmit(emitter, amount);
}

// particle moves, decays and dies, if killoutofbounds is set, out of bounds particles are set to ttl=0
// uses passed settings to set bounce or wrap, if useGravity is enabled, it will never bounce at the top and killoutofbounds is not applied over the top
void ParticleSystem2D::particleMoveUpdate(PSparticle &part, PSsettings2D *options, PSadvancedParticle *advancedproperties) {
  if (options == NULL)
    options = &particlesettings; //use PS system settings by default

  if (part.ttl > 0) {
    if (!part.perpetual)
      part.ttl--; // age
    if (options->colorByAge)
      part.hue = min(part.ttl, (uint16_t)255); //set color to ttl

    int32_t renderradius = PS_P_HALFRADIUS; // used to check out of bounds
    int32_t newX = part.x + (int32_t)part.vx;
    int32_t newY = part.y + (int32_t)part.vy;
    part.outofbounds = false; // reset out of bounds (in case particle was created outside the matrix and is now moving into view)

    if (advancedproperties) { //using individual particle size?
      if (advancedproperties->size > 0) {
        particleHardRadius = max(PS_P_MINHARDRADIUS, (int)particlesize + (advancedproperties->size)); // update radius
        renderradius = particleHardRadius;
      }
    }
    // note: if wall collisions are enabled, bounce them before they reach the edge, it looks much nicer if the particle does not go half out of view
    if (options->bounceY) {
      if ((newY < (int32_t)particleHardRadius) || ((newY > (int32_t)(maxY - particleHardRadius)) && !options->useGravity)) { // reached floor / ceiling
         bounce(part.vy, part.vx, newY, maxY);
      }
    }

    if(!checkBoundsAndWrap(newY, maxY, renderradius, options->wrapY)) { // check out of bounds  note: this must not be skipped, if gravity is enabled, particles will never bounce at the top
      part.outofbounds = true;
      if (options->killoutofbounds) {
        if (newY < 0) // if gravity is enabled, only kill particles below ground
          part.ttl = 0;
        else if (!options->useGravity)
          part.ttl = 0;
      }
    }

    if(part.ttl) { //check x direction only if still alive
      if (options->bounceX) {
        if ((newX < (int32_t)particleHardRadius) || (newX > (int32_t)(maxX - particleHardRadius))) // reached a wall
          bounce(part.vx, part.vy, newX, maxX);
      }
      else if(!checkBoundsAndWrap(newX, maxX, renderradius, options->wrapX)) { // check out of bounds  TODO: not checking out of bounds when bounce is enabled used to lead to crashes, seems fixed now. test more.
        part.outofbounds = true;
        if (options->killoutofbounds)
          part.ttl = 0;
      }
    }

    part.x = (int16_t)newX; // set new position
    part.y = (int16_t)newY; // set new position
  }
}

// move function for fire particles
void ParticleSystem2D::fireParticleupdate() {
  for (uint32_t i = 0; i < usedParticles; i++) {
    if (particles[i].ttl > 0)
    {
      particles[i].ttl--; // age
      int32_t newY = particles[i].y + (int32_t)particles[i].vy + (particles[i].ttl >> 2); // younger particles move faster upward as they are hotter
      particles[i].outofbounds = 0; // reset out of bounds flag
      // check if particle is out of bounds, wrap x around to other side if wrapping is enabled
      // as fire particles start below the frame, lots of particles are out of bounds in y direction. to improve speed, only check x direction if y is not out of bounds
      if (newY < -PS_P_HALFRADIUS)
        particles[i].outofbounds = 1;
      else if (newY > int32_t(maxY + PS_P_HALFRADIUS)) // particle moved out at the top
        particles[i].ttl = 0;
      else // particle is in frame in y direction, also check x direction now Note: using checkBoundsAndWrap() is slower, only saves a few bytes
      {
        int32_t newX = particles[i].x + (int32_t)particles[i].vx;
        if ((newX < 0) || (newX > (int32_t)maxX)) { // handle out of bounds & wrap
          if (particlesettings.wrapX) {
            newX = newX % (maxX + 1);
            if (newX < 0) // handle negative modulo
              newX += maxX + 1;
          }
          else if ((newX < -PS_P_HALFRADIUS) || (newX > int32_t(maxX + PS_P_HALFRADIUS))) { //if fully out of view
            particles[i].ttl = 0;
          }
        }
        particles[i].x = newX;
      }
      particles[i].y = newY;
    }
  }
/*
  // this loop saves 150 bytes of flash but is 5% slower
  for (uint32_t i = 0; i < usedParticles; i++) {
    if (particles[i].ttl > 0) {
       particles[i].y += (particles[i].ttl >> 2); // younger particles move faster upward as they are hotter
       particleMoveUpdate(particles[i]);
    }
  }
*/
}

// update advanced particle size control
void ParticleSystem2D::updateSize(PSadvancedParticle *advprops, PSsizeControl *advsize) {
  if (advsize == NULL) // safety check
    return;
  // grow/shrink particle
  int32_t newsize = advprops->size;
  uint32_t counter = advsize->sizecounter;
  uint32_t increment = 0;
  // calculate grow speed using 0-8 for low speeds and 9-15 for higher speeds
  if (advsize->grow) increment = advsize->growspeed;
  else if (advsize->shrink) increment = advsize->shrinkspeed;
  if (increment < 9) { // 8 means +1 every frame
    counter += increment;
    if (counter > 7) {
      counter -= 8;
      increment = 1;
    } else
      increment = 0;
    advsize->sizecounter = counter;
  } else {
    increment = (increment - 8) << 1; // 9 means +2, 10 means +4 etc. 15 means +14
  }

  if (advsize->grow) {
    if (newsize < advsize->maxsize) {
      newsize += increment;
      if (newsize >= advsize->maxsize) {
        advsize->grow = false; // stop growing, shrink from now on if enabled
        newsize = advsize->maxsize; // limit
        if (advsize->pulsate) advsize->shrink = true;
      }
    }
  } else if (advsize->shrink) {
    if (newsize > advsize->minsize) {
      newsize -= increment;
      if (newsize <= advsize->minsize) {
        //if (advsize->minsize == 0) part.ttl = 0; //TODO: need to pass particle or return kill instruction
        advsize->shrink = false; // disable shrinking
        newsize = advsize->minsize; // limit
        if (advsize->pulsate) advsize->grow = true;
      }
    }
  }
  advprops->size = newsize;
  // handle wobbling
  if (advsize->wobble) {
    advsize->asymdir += advsize->wobblespeed; // todo: need better wobblespeed control? counter is already in the struct...
  }
}

// calculate x and y size for asymmetrical particles (advanced size control)
void ParticleSystem2D::getParticleXYsize(PSadvancedParticle *advprops, PSsizeControl *advsize, uint32_t &xsize, uint32_t &ysize) {
  if (advsize == NULL) // if advsize is valid, also advanced properties pointer is valid (handled by updatePSpointers())
    return;
  int32_t size = advprops->size;
  int32_t asymdir = advsize->asymdir;
  int32_t deviation = ((uint32_t)size * (uint32_t)advsize->asymmetry) / 255; // deviation from symmetrical size
  // Calculate x and y size based on deviation and direction (0 is symmetrical, 64 is x, 128 is symmetrical, 192 is y)
  if (asymdir < 64) {
    deviation = (asymdir * deviation) / 64;
  } else if (asymdir < 192) {
    deviation = ((128 - asymdir) * deviation) / 64;
  } else {
    deviation = ((asymdir - 255) * deviation) / 64;
  }
  // Calculate x and y size based on deviation, limit to 255 (rendering function cannot handle larger sizes)
  xsize = min((size - deviation), (int32_t)255);
  ysize = min((size + deviation), (int32_t)255);;
}

// function to bounce a particle from a wall using set parameters (wallHardness and wallRoughness)
void ParticleSystem2D::bounce(int8_t &incomingspeed, int8_t &parallelspeed, int32_t &position, uint16_t maxposition) {
  incomingspeed = -incomingspeed;
  incomingspeed = (incomingspeed * wallHardness) / 255; // reduce speed as energy is lost on non-hard surface
  if (position < particleHardRadius)
    position = particleHardRadius; // fast particles will never reach the edge if position is inverted, this looks better
  else
    position = maxposition - particleHardRadius;
  if (wallRoughness) {
    int32_t incomingspeed_abs = abs((int32_t)incomingspeed);
    int32_t totalspeed = incomingspeed_abs + abs((int32_t)parallelspeed);
    // transfer an amount of incomingspeed speed to parallel speed
    int32_t donatespeed = ((random16(incomingspeed_abs << 1) - incomingspeed_abs) * (int32_t)wallRoughness) / (int32_t)255; // take random portion of + or - perpendicular speed, scaled by roughness
    parallelspeed = limitSpeed((int32_t)parallelspeed + donatespeed);
    // give the remainder of the speed to perpendicular speed
    donatespeed = int8_t(totalspeed - abs(parallelspeed)); // keep total speed the same
    incomingspeed = incomingspeed > 0 ? donatespeed : -donatespeed;
  }
}

// apply a force in x,y direction to individual particle
// caller needs to provide a 8bit counter (for each particle) that holds its value between calls
// force is in 3.4 fixed point notation so force=16 means apply v+1 each frame default of 8 is every other frame (gives good results)
void ParticleSystem2D::applyForce(PSparticle *part, int8_t xforce, int8_t yforce, uint8_t *counter) {
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
void ParticleSystem2D::applyForce(uint16_t particleindex, int8_t xforce, int8_t yforce) {
  if (advPartProps == NULL)
    return; // no advanced properties available
  applyForce(&particles[particleindex], xforce, yforce, &advPartProps[particleindex].forcecounter);
}

// apply a force in x,y direction to all particles
// force is in 3.4 fixed point notation (see above)
void ParticleSystem2D::applyForce(int8_t xforce, int8_t yforce) {
  // for small forces, need to use a delay counter
  uint8_t tempcounter;
  // note: this is not the most computationally efficient way to do this, but it saves on duplicate code and is fast enough
  for (uint32_t i = 0; i < usedParticles; i++) {
    tempcounter = forcecounter;
    applyForce(&particles[i], xforce, yforce, &tempcounter);
  }
  forcecounter = tempcounter; // save value back
}

// apply a force in angular direction to single particle
// caller needs to provide a 8bit counter that holds its value between calls (if using single particles, a counter for each particle is needed)
// angle is from 0-65535 (=0-360deg) angle = 0 means in positive x-direction (i.e. to the right)
// force is in 3.4 fixed point notation so force=16 means apply v+1 each frame (useful force range is +/- 127)
void ParticleSystem2D::applyAngleForce(PSparticle *part, int8_t force, uint16_t angle, uint8_t *counter) {
  int8_t xforce = ((int32_t)force * cos16(angle)) / 32767; // force is +/- 127
  int8_t yforce = ((int32_t)force * sin16(angle)) / 32767; // note: cannot use bit shifts as bit shifting is asymmetrical for positive and negative numbers and this needs to be accurate!
  // note: sin16 is 10% faster than sin8() on ESP32 but on ESP8266 it is 9% slower
  applyForce(part, xforce, yforce, counter);
}

void ParticleSystem2D::applyAngleForce(uint16_t particleindex, int8_t force, uint16_t angle) {
  if (advPartProps == NULL)
    return; // no advanced properties available
  applyAngleForce(&particles[particleindex], force, angle, &advPartProps[particleindex].forcecounter);
}

// apply a force in angular direction to all particles
// angle is from 0-65535 (=0-360deg) angle = 0 means in positive x-direction (i.e. to the right)
void ParticleSystem2D::applyAngleForce(int8_t force, uint16_t angle) {
  int8_t xforce = ((int32_t)force * cos16(angle)) / 32767; // force is +/- 127
  int8_t yforce = ((int32_t)force * sin16(angle)) / 32767; // note: cannot use bit shifts as bit shifting is asymmetrical for positive and negative numbers and this needs to be accurate!
  applyForce(xforce, yforce);
}

// apply gravity to all particles using PS global gforce setting
// force is in 3.4 fixed point notation, see note above
// note: faster than apply force since direction is always down and counter is fixed for all particles
void ParticleSystem2D::applyGravity() {
  int32_t dv = calcForce_dv(gforce, &gforcecounter);
  if(dv == 0) return;
  for (uint32_t i = 0; i < usedParticles; i++) {
    // Note: not checking if particle is dead is faster as most are usually alive and if few are alive, rendering is fast anyways
    particles[i].vy = limitSpeed((int32_t)particles[i].vy - dv);
  }
}

// apply gravity to single particle using system settings (use this for sources)
// function does not increment gravity counter, if gravity setting is disabled, this cannot be used
void ParticleSystem2D::applyGravity(PSparticle &part) {
  uint32_t counterbkp = gforcecounter; // backup PS gravity counter
  int32_t dv = calcForce_dv(gforce, &gforcecounter);
  gforcecounter = counterbkp; //save it back
  part.vy = limitSpeed((int32_t)part.vy - dv);
}

// slow down particle by friction, the higher the speed, the higher the friction. a high friction coefficient slows them more (255 means instant stop)
// note: a coefficient smaller than 0 will speed them up (this is a feature, not a bug), coefficient larger than 255 inverts the speed, so don't do that
void ParticleSystem2D::applyFriction(PSparticle *part, int32_t coefficient) {
  int32_t friction = 255 - coefficient;
  // note: not checking if particle is dead can be done by caller (or can be omitted)
  // note2: cannot use right shifts as bit shifting in right direction is asymmetrical for positive and negative numbers and this needs to be accurate
  part->vx = ((int32_t)part->vx * friction) / 255;
  part->vy = ((int32_t)part->vy * friction) / 255;
}

// apply friction to all particles
void ParticleSystem2D::applyFriction(int32_t coefficient) {
  for (uint32_t i = 0; i < usedParticles; i++) {
    if (particles[i].ttl)
      applyFriction(&particles[i], coefficient);
  }
}

// attracts a particle to an attractor particle using the inverse square-law
void ParticleSystem2D::pointAttractor(uint16_t particleindex, PSparticle *attractor, uint8_t strength, bool swallow) {
  if (advPartProps == NULL)
    return; // no advanced properties available

  // Calculate the distance between the particle and the attractor
  int32_t dx = attractor->x - particles[particleindex].x;
  int32_t dy = attractor->y - particles[particleindex].y;

  // Calculate the force based on inverse square law
  int32_t distanceSquared = dx * dx + dy * dy;
  if (distanceSquared < 8192) {
    if (swallow) { // particle is close, age it fast so it fades out, do not attract further
      if (particles[particleindex].ttl > 7)
        particles[particleindex].ttl -= 8;
      else {
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

// render particles to the LED buffer (uses palette to render the 8bit particle color value)
// if wrap is set, particles half out of bounds are rendered to the other side of the matrix
// warning: do not render out of bounds particles or system will crash! rendering does not check if particle is out of bounds
// fireintensity and firemode are optional arguments (fireintensity is only used in firemode)
void ParticleSystem2D::ParticleSys_render(bool firemode, uint32_t fireintensity) {
  CRGB baseRGB;
  bool useLocalBuffer = true; //use local rendering buffer, gives huge speed boost (at least 30% more FPS)
  CRGB *framebuffer = NULL; //local frame buffer, Note: 1D array access is faster, especially when accessing in order
  CRGB *renderbuffer = NULL; //local particle render buffer for advanced particles
  uint32_t i;
  uint32_t brightness; // particle brightness, fades if dying

  /*
  //memory fragmentation check:
  Serial.print("heap: ");
  Serial.print(heap_caps_get_free_size(MALLOC_CAP_8BIT));
  Serial.print(" block: ");
  Serial.println(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
  */

  if (useLocalBuffer) {
    // allocate empty memory for the local renderbuffer
    framebuffer = allocate1Dbuffer((maxXpixel + 1) * (maxYpixel + 1));
    if (framebuffer == NULL) {
      //Serial.println("Frame buffer alloc failed");
      useLocalBuffer = false; //render to segment pixels directly if not enough memory
    }
    else {
      if (motionBlur > 0) { // using SEGMENT.fadeToBlackBy is much slower, this approximately doubles the speed of fade calculation
        uint32_t yflipped;
        for (uint32_t y = 0; y <= maxYpixel; y++) {
          yflipped = maxYpixel - y;
          int index = y * (maxXpixel + 1); // current row index for 1D buffer
          for (uint32_t x = 0; x <= maxXpixel; x++) {
            framebuffer[index] = SEGMENT.getPixelColorXY(x, yflipped); //copy to local buffer
            fast_color_scale(framebuffer[index], motionBlur);
            index++;
          }
        }
      }
    }
  }
  if (advPartProps) {
    renderbuffer = allocate1Dbuffer(10*10); //buffer to render individual particles to if size > 0. note: null checking is done when accessing it
  }

  if (!useLocalBuffer) { //disabled or allocation above failed
    if (motionBlur > 0)
      SEGMENT.fadeToBlackBy(255 - motionBlur);
    else
      SEGMENT.fill(BLACK); //clear the buffer before rendering to it
  }
  bool wrapX = particlesettings.wrapX; // use local variables for faster access
  bool wrapY = particlesettings.wrapY;
  // go over particles and render them to the buffer
  for (i = 0; i < usedParticles; i++) {
    if (particles[i].outofbounds || particles[i].ttl == 0)
      continue;

    // generate RGB values for particle
    if (firemode) {
      brightness = (uint32_t)particles[i].ttl * (3 + (fireintensity >> 5)) + 20;
      brightness = min(brightness, (uint32_t)255);
      baseRGB = ColorFromPalette(SEGPALETTE, brightness, 255);
    }
    else {
      brightness = min(particles[i].ttl, (uint16_t)255);
      baseRGB = ColorFromPalette(SEGPALETTE, particles[i].hue, 255);
      if (particles[i].sat < 255) {
        CHSV baseHSV = rgb2hsv_approximate(baseRGB); //convert to HSV
        baseHSV.s = particles[i].sat; //set the saturation
        baseRGB = (CRGB)baseHSV; // convert back to RGB
      }
    }
    renderParticle(framebuffer, i, brightness, baseRGB, renderbuffer, wrapX, wrapY);
  }

  if (particlesize > 0) {
    uint32_t passes = particlesize / 64 + 1; // number of blur passes, four passes max
    uint32_t bluramount = particlesize;
    uint32_t bitshift = 0;

    for (uint32_t i = 0; i < passes; i++) {
      if (i == 2) // for the last two passes, use higher amount of blur (results in a nicer brightness gradient with soft edges)
        bitshift = 1;

      if (useLocalBuffer)
        blur2D(framebuffer, maxXpixel + 1, maxYpixel + 1, bluramount << bitshift, bluramount << bitshift);
      else {
        SEGMENT.blur(bluramount << bitshift, true);
      }
      bluramount -= 64;
    }
  }

  if (useLocalBuffer) { // transfer local buffer back to segment
    int32_t yflipped;
    for (uint32_t y = 0; y <= maxYpixel; y++) {
      yflipped = maxYpixel - y;
      int index = y * (maxXpixel + 1); // current row index for 1D buffer
      for (uint32_t x = 0; x <= maxXpixel; x++) {
        SEGMENT.setPixelColorXY((int)x, (int)yflipped, framebuffer[index++]);
      }
    }
    free(framebuffer);
  }
  if (renderbuffer)
    free(renderbuffer);
}

// calculate pixel positions and brightness distribution and render the particle to local buffer or global buffer
void ParticleSystem2D::renderParticle(CRGB *framebuffer, const uint32_t particleindex, const uint32_t brightness, const CRGB& color, CRGB *renderbuffer, const bool wrapX, const bool wrapY) {
  int32_t pxlbrightness[4]; // brightness values for the four pixels representing a particle
  int32_t pixco[4][2]; // physical pixel coordinates of the four pixels a particle is rendered to. x,y pairs
  bool pixelvalid[4] = {true, true, true, true}; // is set to false if pixel is out of bounds
  bool advancedrender = false; // rendering for advanced particles
  // check if particle has advanced size properties and buffer is available
  if (advPartProps && advPartProps[particleindex].size > 0) {
      if (renderbuffer) {
        advancedrender = true;
        memset(renderbuffer, 0, 100 * sizeof(CRGB)); // clear the buffer, renderbuffer is 10x10 pixels
      }
      else return; // cannot render without buffers
  }
  // add half a radius as the rendering algorithm always starts at the bottom left, this leaves things positive, so shifts can be used, then shift coordinate by a full pixel (x--/y-- below)
  int32_t xoffset = particles[particleindex].x + PS_P_HALFRADIUS;
  int32_t yoffset = particles[particleindex].y + PS_P_HALFRADIUS;
  int32_t dx = xoffset & (PS_P_RADIUS - 1); // relativ particle position in subpixel space
  int32_t dy = yoffset & (PS_P_RADIUS - 1); // modulo replaced with bitwise AND, as radius is always a power of 2
  int32_t x = (xoffset >> PS_P_RADIUS_SHIFT); // divide by PS_P_RADIUS which is 64, so can bitshift (compiler can not optimize integer)
  int32_t y = (yoffset >> PS_P_RADIUS_SHIFT);

  // set the four raw pixel coordinates, the order is bottom left [0], bottom right[1], top right [2], top left [3]
  pixco[1][0] = pixco[2][0] = x;  // bottom right & top right
  pixco[2][1] = pixco[3][1] = y;  // top right & top left
  x--; // shift by a full pixel here, this is skipped above to not do -1 and then +1
  y--;
  pixco[0][0] = pixco[3][0] = x;      // bottom left & top left
  pixco[0][1] = pixco[1][1] = y;      // bottom left & bottom right

  // calculate brightness values for all four pixels representing a particle using linear interpolation
  // could check for out of frame pixels here but calculating them is faster (very few are out)
  // precalculate values for speed optimization
  int32_t precal1 = (int32_t)PS_P_RADIUS - dx;
  int32_t precal2 = ((int32_t)PS_P_RADIUS - dy) * brightness;
  int32_t precal3 = dy * brightness;
  pxlbrightness[0] = (precal1 * precal2) >> PS_P_SURFACE; // bottom left value equal to ((PS_P_RADIUS - dx) * (PS_P_RADIUS-dy) * brightness) >> PS_P_SURFACE
  pxlbrightness[1] = (dx * precal2) >> PS_P_SURFACE; // bottom right value equal to (dx * (PS_P_RADIUS-dy) * brightness) >> PS_P_SURFACE
  pxlbrightness[2] = (dx * precal3) >> PS_P_SURFACE; // top right value equal to (dx * dy * brightness) >> PS_P_SURFACE
  pxlbrightness[3] = (precal1 * precal3) >> PS_P_SURFACE; // top left value equal to ((PS_P_RADIUS-dx) * dy * brightness) >> PS_P_SURFACE

  if (advancedrender) {
    //render particle to a bigger size
    //particle size to pixels: < 64 is 4x4, < 128 is 6x6, < 192 is 8x8, bigger is 10x10
    //first, render the pixel to the center of the renderbuffer, then apply 2D blurring
    fast_color_add(renderbuffer[4 + (4 * 10)], color, pxlbrightness[0]); // order is: bottom left, bottom right, top right, top left
    fast_color_add(renderbuffer[5 + (4 * 10)], color, pxlbrightness[1]);
    fast_color_add(renderbuffer[5 + (5 * 10)], color, pxlbrightness[2]);
    fast_color_add(renderbuffer[4 + (5 * 10)], color, pxlbrightness[3]);
    uint32_t rendersize = 2; // initialize render size, minimum is 4x4 pixels, it is incremented int he loop below to start with 4
    uint32_t offset = 4; // offset to zero coordinate to write/read data in renderbuffer (actually needs to be 3, is decremented in the loop below)
    uint32_t maxsize = advPartProps[particleindex].size;
    uint32_t xsize = maxsize;
    uint32_t ysize = maxsize;
    if (advPartSize) { // use advanced size control
      if (advPartSize[particleindex].asymmetry > 0)
        getParticleXYsize(&advPartProps[particleindex], &advPartSize[particleindex], xsize, ysize);
      maxsize = (xsize > ysize) ? xsize : ysize; // choose the bigger of the two
    }
    maxsize = maxsize/64 + 1; // number of blur passes depends on maxsize, four passes max
    uint32_t bitshift = 0;
    for(uint32_t i = 0; i < maxsize; i++) {
      if (i == 2) //for the last two passes, use higher amount of blur (results in a nicer brightness gradient with soft edges)
        bitshift = 1;
      rendersize += 2;
      offset--;
      blur2D(renderbuffer, rendersize, rendersize, xsize << bitshift, ysize << bitshift, offset, offset, true);
      xsize = xsize > 64 ? xsize - 64 : 0;
      ysize = ysize > 64 ? ysize - 64 : 0;
    }

    // calculate origin coordinates to render the particle to in the framebuffer
    uint32_t xfb_orig = x - (rendersize>>1) + 1 - offset;
    uint32_t yfb_orig = y - (rendersize>>1) + 1 - offset;
    uint32_t xfb, yfb; // coordinates in frame buffer to write to note: by making this uint, only overflow has to be checked (spits a warning though)

    // transfer particle renderbuffer to framebuffer
    for (uint32_t xrb = offset; xrb < rendersize + offset; xrb++) {
      xfb = xfb_orig + xrb;
      if (xfb > maxXpixel) {
      if (wrapX) // wrap x to the other side if required
        xfb = xfb % (maxXpixel + 1); // TODO: this did not work in 1D system but appears to work in 2D (wrapped pixels were offset) under which conditions does this not work?
      else
        continue;
      }

      for (uint32_t yrb = offset; yrb < rendersize + offset; yrb++) {
      yfb = yfb_orig + yrb;
      if (yfb > maxYpixel) {
        if (wrapY) // wrap y to the other side if required
        yfb = yfb % (maxYpixel + 1);
        else
        continue;
      }
      if (framebuffer)
        fast_color_add(framebuffer[xfb + yfb * (maxXpixel + 1)], renderbuffer[xrb + yrb * 10]);
      else
        SEGMENT.addPixelColorXY(xfb, maxYpixel - yfb, renderbuffer[xrb + yrb * 10]);
      }
    }
    } else { // standard rendering
    // check for out of frame pixels and wrap them if required
    if (x < 0) { // left pixels out of frame
      if (wrapX) { // wrap x to the other side if required
        pixco[0][0] = pixco[3][0] = maxXpixel;
      } else {
        pixelvalid[0] = pixelvalid[3] = false; // out of bounds
      }
    }
    else if (pixco[1][0] > (int32_t)maxXpixel) { // right pixels, only has to be checkt if left pixels did not overflow
      if (wrapX) { // wrap y to the other side if required
        pixco[1][0] = pixco[2][0] = 0;
      } else {
        pixelvalid[1] = pixelvalid[2] = false; // out of bounds
      }
    }

    if (y < 0) { // bottom pixels out of frame
      if (wrapY) { // wrap y to the other side if required
        pixco[0][1] = pixco[1][1] = maxYpixel;
      } else {
        pixelvalid[0] = pixelvalid[1] = false; // out of bounds
      }
    }
    else if (pixco[2][1] > maxYpixel) { // top pixels
      if (wrapY) { // wrap y to the other side if required
        pixco[2][1] = pixco[3][1] = 0;
      } else {
        pixelvalid[2] = pixelvalid[3] = false; // out of bounds
      }
    }
    if (framebuffer) {
      for (uint32_t i = 0; i < 4; i++) {
        if (pixelvalid[i])
          fast_color_add(framebuffer[pixco[i][0] + pixco[i][1] * (maxXpixel + 1)], color, pxlbrightness[i]); // order is: bottom left, bottom right, top right, top left
      }
    }
    else {
      for (uint32_t i = 0; i < 4; i++) {
      if (pixelvalid[i])
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


// detect collisions in an array of particles and handle them
void ParticleSystem2D::handleCollisions() {
  // detect and handle collisions
  uint32_t i, j;
  uint32_t startparticle = 0;
  uint32_t endparticle = usedParticles >> 1; // do half the particles, significantly speeds things up
  int32_t collDistSq = particleHardRadius << 1;
  collDistSq = collDistSq * collDistSq; // square it for faster comparison (square is one operation)
  // every second frame, do other half of particles (helps to speed things up as not all collisions are handled each frame, less accurate but good enough)
  // if more accurate collisions are needed, just call it twice in a row
  if (collisioncounter & 0x01) {
    startparticle = endparticle;
    endparticle = usedParticles;
  }
  collisioncounter++;
  for (i = startparticle; i < endparticle; i++) { // go though all 'higher number' particles and see if any of those are in close proximity and if they are, make them collide
    if (particles[i].ttl > 0 && particles[i].outofbounds == 0 && particles[i].collide) { // if particle is alive and is not out of view and does collide
      int32_t dx, dy; // distance to other particles
      for (j = i + 1; j < usedParticles; j++) { // check against higher number particles
        if (particles[j].ttl > 0 && particles[j].collide) { // if target particle is alive

          if (advPartProps) { //may be using individual particle size
            collDistSq = PS_P_MINHARDRADIUS + particlesize + (((uint32_t)advPartProps[i].size + (uint32_t)advPartProps[j].size)>>1); // collision distance
            collDistSq = collDistSq * collDistSq; // square it for faster comparison
          }

          dx = particles[j].x - particles[i].x;
          if (dx * dx < collDistSq) { // check x direction, if close, check y direction (squaring is faster than abs() or dual compare)
            dy = particles[j].y - particles[i].y;
            if (dy * dy < collDistSq) // particles are close
              collideParticles(&particles[i], &particles[j], dx, dy);
          }
        }
      }
    }
  }
}

// handle a collision if close proximity is detected, i.e. dx and/or dy smaller than 2*PS_P_RADIUS
// takes two pointers to the particles to collide and the particle hardness (softer means more energy lost in collision, 255 means full hard)
void ParticleSystem2D::collideParticles(PSparticle *particle1, PSparticle *particle2, int32_t dx, int32_t dy) { // TODO: dx,dy is calculated just above, can pass it over here to save a few CPU cycles?
  int32_t distanceSquared = dx * dx + dy * dy;
  // Calculate relative velocity (if it is zero, could exit but extra check does not overall speed but deminish it)
  int32_t relativeVx = (int32_t)particle2->vx - (int32_t)particle1->vx;
  int32_t relativeVy = (int32_t)particle2->vy - (int32_t)particle1->vy;

  // if dx and dy are zero (i.e. same position) give them an offset, if speeds are also zero, also offset them (pushes particles apart if they are clumped before enabling collisions)
  if (distanceSquared == 0) {
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

    distanceSquared = 2; // 1 + 1
  }

  // Calculate dot product of relative velocity and relative distance
  int32_t dotProduct = (dx * relativeVx + dy * relativeVy); // is always negative if moving towards each other

  if (dotProduct < 0) {// particles are moving towards each other
    // integer math used to avoid floats.
    // overflow check: dx/dy are 7bit, relativV are 8bit -> dotproduct is 15bit, dotproduct/distsquared ist 8b, multiplied by collisionhardness of 8bit. so a 16bit shift is ok, make it 15 to be sure no overflows happen
    // note: cannot use right shifts as bit shifting in right direction is asymmetrical for positive and negative numbers and this needs to be accurate! the trick is: only shift positive numers
    // Calculate new velocities after collision
    int32_t surfacehardness = max(collisionHardness, (int32_t)PS_P_MINSURFACEHARDNESS); // if particles are soft, the impulse must stay above a limit or collisions slip through at higher speeds, 170 seems to be a good value
    int32_t impulse = -(((((-dotProduct) << 15) / distanceSquared) * surfacehardness) >> 8); // note: inverting before bitshift corrects for asymmetry in right-shifts (and is slightly faster)
    int32_t ximpulse = ((impulse) * dx) / 32767; // cannot use bit shifts here, it can be negative, use division by 2^bitshift
    int32_t yimpulse = ((impulse) * dy) / 32767;
    particle1->vx += ximpulse;
    particle1->vy += yimpulse;
    particle2->vx -= ximpulse;
    particle2->vy -= yimpulse;

    if (collisionHardness < surfacehardness) { // if particles are soft, they become 'sticky' i.e. apply some friction (they do pile more nicely and stop sloshing around)
      const uint32_t coeff = collisionHardness + (255 - PS_P_MINSURFACEHARDNESS);  // Note: could call applyFriction, but this is faster and speed is key here
      particle1->vx = ((int32_t)particle1->vx * coeff) / 255;
      particle1->vy = ((int32_t)particle1->vy * coeff) / 255;

      particle2->vx = ((int32_t)particle2->vx * coeff) / 255;
      particle2->vy = ((int32_t)particle2->vy * coeff) / 255;
    }

    // particles have volume, push particles apart if they are too close
    // tried lots of configurations, it works best if not moved but given a little velocity, it tends to oscillate less this way
    // a problem with giving velocity is, that on harder collisions, this adds up as it is not dampened enough, so add friction in the FX if required
    if (dotProduct > -250) { //this means particles are slow (or really really close) so push them apart.
      int32_t notsorandom = dotProduct & 0x01; //dotprouct LSB should be somewhat random, so no need to calculate a random number
      int32_t pushamount = 1 + ((250 + dotProduct) >> 6); // the closer dotproduct is to zero, the closer the particles are
      int32_t push = 0;
      if (dx < 0)  // particle 1 is on the right
        push = pushamount;
      else if (dx > 0)
        push = -pushamount;
      else { // on the same x coordinate, shift it a little so they do not stack
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
      else { // dy==0
        if (notsorandom)
          particle1->y++; // move it so pile collapses
        else
          particle1->y--;
      }
      particle1->vy += push;
      // note: pushing may push particles out of frame, if bounce is active, it will move it back as position will be limited to within frame, if bounce is disabled: bye bye
      if (collisionHardness < 16) { // if they are very soft, stop slow particles completely to make them stick to each other
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

// update size and pointers (memory location and size can change dynamically)
// note: do not access the PS class in FX befor running this function (or it messes up SEGENV.data)
void ParticleSystem2D::updateSystem(void) {
  // update matrix size
  uint32_t cols = strip.isMatrix ? SEGMENT.virtualWidth() : 1;
  uint32_t rows = strip.isMatrix ? SEGMENT.virtualHeight() : SEGMENT.virtualLength();
  setMatrixSize(cols, rows);
  updatePSpointers(advPartProps != NULL, advPartSize != NULL);
}

// set the pointers for the class (this only has to be done once and not on every FX call, only the class pointer needs to be reassigned to SEGENV.data every time)
// function returns the pointer to the next byte available for the FX (if it assigned more memory for other stuff using the above allocate function)
// FX handles the PSsources, need to tell this function how many there are
void ParticleSystem2D::updatePSpointers(bool isadvanced, bool sizecontrol) {
  // DEBUG_PRINT(F("*** PS pointers ***"));
  // DEBUG_PRINTF_P(PSTR("this PS %p "), this);
  // Note on memory alignment:
  // a pointer MUST be 4 byte aligned. sizeof() in a struct/class is always aligned to the largest element. if it contains a 32bit, it will be padded to 4 bytes, 16bit is padded to 2byte alignment.
  // The PS is aligned to 4 bytes, a PSparticle is aligned to 2 and a struct containing only byte sized variables is not aligned at all and may need to be padded when dividing the memoryblock.
  // by making sure that the number of sources and particles is a multiple of 4, padding can be skipped here as alignent is ensured, independent of struct sizes.
  particles = reinterpret_cast<PSparticle *>(this + 1); // pointer to particle array at data+sizeof(ParticleSystem2D)
  sources = reinterpret_cast<PSsource *>(particles + numParticles); // pointer to source(s)
  if (isadvanced) {
    advPartProps = reinterpret_cast<PSadvancedParticle *>(sources + numSources);
    PSdataEnd = reinterpret_cast<uint8_t *>(advPartProps + numParticles);
    if (sizecontrol) {
      advPartSize = reinterpret_cast<PSsizeControl *>(advPartProps + numParticles);
      PSdataEnd = reinterpret_cast<uint8_t *>(advPartSize + numParticles);
    }
  }
  else {
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
// for speed, 1D array and 32bit variables are used, make sure to limit them to 8bit (0-255) or result is undefined
// to blur a subset of the buffer, change the xsize/ysize and set xstart/ystart to the desired starting coordinates (default start is 0/0)
// subset blurring only works on 10x10 buffer (single particle rendering), if other sizes are needed, buffer width must be passed as parameter
void blur2D(CRGB *colorbuffer, uint32_t xsize, uint32_t ysize, uint32_t xblur, uint32_t yblur, uint32_t xstart, uint32_t ystart, bool isparticle) {
  CRGB seeppart, carryover;
  uint32_t seep = xblur >> 1;
  uint32_t width = xsize; // width of the buffer, used to calculate the index of the pixel

  if (isparticle) { //first and last row are always black in first pass of particle rendering
    ystart++;
    ysize--;
    width = 10; // buffer size is 10x10
  }

  for(uint32_t y = ystart; y < ystart + ysize; y++) {
    carryover =  BLACK;
    uint32_t indexXY = xstart + y * width;
    for(uint32_t x = xstart; x < xstart + xsize; x++) {
      seeppart = colorbuffer[indexXY]; // create copy of current color
      fast_color_scale(seeppart, seep); // scale it and seep to neighbours
      if (x > 0) {
        fast_color_add(colorbuffer[indexXY - 1], seeppart);
        fast_color_add(colorbuffer[indexXY], carryover); // TODO: could check if carryover is > 0, takes 7 instructions, add takes ~35, with lots of same color pixels (like background), it would be faster
      }
      carryover = seeppart;
      indexXY++; // next pixel in x direction
    }
  }

  if (isparticle) { // first and last row are now smeared
    ystart--;
    ysize++;
  }

  seep = yblur >> 1;
  for(uint32_t x = xstart; x < xstart + xsize; x++) {
    carryover = BLACK;
    uint32_t indexXY = x + ystart * width;
    for(uint32_t y = ystart; y < ystart + ysize; y++) {
      seeppart = colorbuffer[indexXY]; // create copy of current color
      fast_color_scale(seeppart, seep); // scale it and seep to neighbours
      if (y > 0) {
        fast_color_add(colorbuffer[indexXY - width], seeppart);
        fast_color_add(colorbuffer[indexXY], carryover); // todo: could check if carryover is > 0, takes 7 instructions, add takes ~35, with lots of same color pixels (like background), it would be faster
      }
      carryover = seeppart;
      indexXY += width; // next pixel in y direction
    }
  }
}


//non class functions to use for initialization
uint32_t calculateNumberOfParticles2D(bool isadvanced, bool sizecontrol) {
  uint32_t cols = SEGMENT.virtualWidth();
  uint32_t rows = SEGMENT.virtualHeight();
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
  numberofParticles = max((uint32_t)4, min(numberofParticles, particlelimit));
  if (isadvanced) // advanced property array needs ram, reduce number of particles to use the same amount
    numberofParticles = (numberofParticles * sizeof(PSparticle)) / (sizeof(PSparticle) + sizeof(PSadvancedParticle));
  if (sizecontrol) // advanced property array needs ram, reduce number of particles to use the same amount
    numberofParticles /= 8; // if size control is used, much fewer particles are needed

  //make sure it is a multiple of 4 for proper memory alignment (easier than using padding bytes)
  numberofParticles = ((numberofParticles+3) >> 2) << 2;
  return numberofParticles;
}

uint32_t calculateNumberOfSources2D(uint8_t requestedsources) {
  uint32_t cols = SEGMENT.virtualWidth();
  uint32_t rows = SEGMENT.virtualHeight();
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
bool allocateParticleSystemMemory2D(uint16_t numparticles, uint16_t numsources, bool isadvanced, bool sizecontrol, uint16_t additionalbytes) {
  uint32_t requiredmemory = sizeof(ParticleSystem2D);
  // functions above make sure these are a multiple of 4 bytes (to avoid alignment issues)
  requiredmemory += sizeof(PSparticle) * numparticles;
  if (isadvanced)
    requiredmemory += sizeof(PSadvancedParticle) * numparticles;
  if (sizecontrol)
    requiredmemory += sizeof(PSsizeControl) * numparticles;
  requiredmemory += sizeof(PSsource) * numsources;
  requiredmemory += additionalbytes;
  return(SEGMENT.allocateData(requiredmemory));
}

// initialize Particle System, allocate additional bytes if needed (pointer to those bytes can be read from particle system class: PSdataEnd)
bool initParticleSystem2D(ParticleSystem2D *&PartSys, uint8_t requestedsources, uint16_t additionalbytes, bool advanced, bool sizecontrol) {
  if(!strip.isMatrix) return false; // only for 2D
  uint32_t numparticles = calculateNumberOfParticles2D(advanced, sizecontrol);
  uint32_t numsources = calculateNumberOfSources2D(requestedsources);
  if (!allocateParticleSystemMemory2D(numparticles, numsources, advanced, sizecontrol, additionalbytes))
  {
    DEBUG_PRINT(F("PS init failed: memory depleted"));
    return false;
  }
  uint16_t cols = strip.isMatrix ? SEGMENT.virtualWidth() : 1;
  uint16_t rows = strip.isMatrix ? SEGMENT.virtualHeight() : SEGMENT.virtualLength();
  PartSys = new (SEGENV.data) ParticleSystem2D(cols, rows, numparticles, numsources, advanced, sizecontrol); // particle system constructor
  return true;
}

#endif // WLED_DISABLE_PARTICLESYSTEM2D


////////////////////////
// 1D Particle System //
////////////////////////
#ifndef WLED_DISABLE_PARTICLESYSTEM1D

ParticleSystem1D::ParticleSystem1D(uint16_t length, uint16_t numberofparticles, uint16_t numberofsources, bool isadvanced) {
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

  // initialize some default non-zero values most FX use
  for (uint32_t i = 0; i < numSources; i++) {
    sources[i].source.ttl = 1; //set source alive
  }
}

// update function applies gravity, moves the particles, handles collisions and renders the particles
void ParticleSystem1D::update(void) {
  PSadvancedParticle1D *advprop = NULL;
  // handle collisions (can push particles, must be done before updating particles or they can render out of bounds, causing a crash if using local buffer for speed)
  if (particlesettings.useCollisions)
    handleCollisions();

  //apply gravity globally if enabled
  if (particlesettings.useGravity) //note: in 1D system, applying gravity after collisions also works TODO: which one is really better for stacking / oscillations?
    applyGravity();

  //move all particles
  for (uint32_t i = 0; i < usedParticles; i++) {
    if (advPartProps)
      advprop = &advPartProps[i];
    particleMoveUpdate(particles[i], &particlesettings, advprop);
  }

  if (particlesettings.colorByPosition) {
    uint32_t scale = (255 << 16) / maxX;  // speed improvement: multiplication is faster than division
    for (uint32_t i = 0; i < usedParticles; i++) {
      particles[i].hue = (scale * (uint32_t)particles[i].x) >> 16;
    }
  }

  ParticleSys_render();

  uint32_t bg_color = SEGCOLOR(1); //background color, set to black to overlay
  if (bg_color > 0) { //if not black
    for(int32_t i = 0; i < maxXpixel + 1; i++) {
      SEGMENT.addPixelColor(i,bg_color); // TODO: can this be done in rendering function using local buffer?
    }
  }
}

void ParticleSystem1D::setUsedParticles(uint32_t num) {
  usedParticles = min(num, numParticles); //limit to max particles
}

void ParticleSystem1D::setWallHardness(uint8_t hardness) {
  wallHardness = hardness;
}

void ParticleSystem1D::setSize(uint16_t x) {
  maxXpixel = x - 1; // last physical pixel that can be drawn to
  maxX = x * PS_P_RADIUS_1D - 1;  // particle system boundary for movements
}

void ParticleSystem1D::setWrap(bool enable) {
  particlesettings.wrap = enable;
}

void ParticleSystem1D::setBounce(bool enable) {
  particlesettings.bounce = enable;
}

void ParticleSystem1D::setKillOutOfBounds(bool enable) {
  particlesettings.killoutofbounds = enable;
}

void ParticleSystem1D::setColorByAge(bool enable) {
  particlesettings.colorByAge = enable;
}

void ParticleSystem1D::setColorByPosition(bool enable) {
  particlesettings.colorByPosition = enable;
}

void ParticleSystem1D::setMotionBlur(uint8_t bluramount) {
  //TODO: currently normal blurring is not used in 1D system. should it be added? advanced rendering is quite fast and allows for motion blurring
  // if (particlesize < 2) // only allow motion blurring on default particle size or advanced size (cannot combine motion blur with normal blurring used for particlesize, would require another buffer)
  motionBlur = bluramount;
}

// render size, 0 = 1 pixel, 1 = 2 pixel (interpolated), bigger sizes require adanced properties
void ParticleSystem1D::setParticleSize(uint8_t size) {
  particlesize = size > 0 ? 1 : 0; // TODO: add support for global sizes? see not abover (motion blur)
  if (particlesize)
    particleHardRadius = PS_P_MINHARDRADIUS_1D; // 2 pixel sized particles
  else
    particleHardRadius = PS_P_MINHARDRADIUS_1D >> 1; // 1 pixel sized particles have half the radius (for bounce, not for collisions)
}

// enable/disable gravity, optionally, set the force (force=8 is default) can be -127 to +127, 0 is disable
// if enabled, gravity is applied to all particles in ParticleSystemUpdate()
// force is in 3.4 fixed point notation so force=16 means apply v+1 each frame default of 8 is every other frame (gives good results)
void ParticleSystem1D::setGravity(int8_t force) {
  if (force) {
    gforce = force;
    particlesettings.useGravity = true;
  }
  else
    particlesettings.useGravity = false;
}

void ParticleSystem1D::enableParticleCollisions(bool enable, uint8_t hardness) {
  particlesettings.useCollisions = enable;
  collisionHardness = hardness;
}

// emit one particle with variation, returns index of last emitted particle (or -1 if no particle emitted)
int32_t ParticleSystem1D::sprayEmit(PSsource1D &emitter) {
  for (uint32_t i = 0; i < usedParticles; i++) {
    emitIndex++;
    if (emitIndex >= usedParticles)
      emitIndex = 0;
    if (particles[emitIndex].ttl == 0) { // find a dead particle
      particles[emitIndex].vx = emitter.v + random16(emitter.var << 1) - emitter.var; // random(-var,var)
      particles[emitIndex].x = emitter.source.x;
      particles[emitIndex].hue = emitter.source.hue;
      particles[emitIndex].collide = emitter.source.collide;
      particles[emitIndex].reversegrav = emitter.source.reversegrav;
      particles[emitIndex].ttl = random16(emitter.minLife, emitter.maxLife);
      if (advPartProps) {
        advPartProps[emitIndex].sat = emitter.sat;
        advPartProps[emitIndex].size = emitter.size;
      }
      return emitIndex;
    }
  }
  return -1;
}

// particle moves, decays and dies, if killoutofbounds is set, out of bounds particles are set to ttl=0
// uses passed settings to set bounce or wrap, if useGravity is set, it will never bounce at the top and killoutofbounds is not applied over the top
void ParticleSystem1D::particleMoveUpdate(PSparticle1D &part, PSsettings1D *options, PSadvancedParticle1D *advancedproperties) {
  if (options == NULL)
    options = &particlesettings; // use PS system settings by default

  if (part.ttl > 0) {
    if (!part.perpetual)
      part.ttl--; // age
    if (options->colorByAge)
      part.hue = min(part.ttl, (uint16_t)255); // set color to ttl

    int32_t renderradius = PS_P_HALFRADIUS_1D; // used to check out of bounds, default for 2 pixel rendering
    int32_t newX = part.x + (int32_t)part.vx;
    part.outofbounds = false; // reset out of bounds (in case particle was created outside the matrix and is now moving into view)

    if (advancedproperties) { // using individual particle size?
      if (advancedproperties->size > 1)
        particleHardRadius = PS_P_MINHARDRADIUS_1D + (advancedproperties->size >> 1); // TODO: this may need optimization, radius and diameter is still a mess in 1D system.
      else // single pixel particles use half the collision distance for walls
        particleHardRadius = PS_P_MINHARDRADIUS_1D >> 1;
      renderradius = particleHardRadius; // note: for single pixel particles, it should be zero, but it does not matter as out of bounds checking is done in rendering function
    }

    // if wall collisions are enabled, bounce them before they reach the edge, it looks much nicer if the particle is not half out of view
    if (options->bounce) {
      if ((newX < (int32_t)particleHardRadius) || ((newX > (int32_t)(maxX - particleHardRadius)))) { // reached a wall
        bool bouncethis = true;
        if (options->useGravity) {
          if (part.reversegrav) { // skip bouncing at x = 0
            if (newX < particleHardRadius)
              bouncethis = false;
          } else if (newX > particleHardRadius) { // skip bouncing at x = max
            bouncethis = false;
          }
        }
        if (bouncethis) {
          part.vx = -part.vx; // invert speed
          part.vx = ((int32_t)part.vx * (int32_t)wallHardness) / 255; // reduce speed as energy is lost on non-hard surface
          if (newX < (int32_t)particleHardRadius)
            newX = particleHardRadius; // fast particles will never reach the edge if position is inverted, this looks better
          else
            newX = maxX - particleHardRadius;
        }
      }
    }

    if (!checkBoundsAndWrap(newX, maxX, renderradius, options->wrap)) { // check out of bounds note: this must not be skipped or it can lead to crashes TODO: is this still true?
      part.outofbounds = true;
      if (options->killoutofbounds) {
        bool killthis = true;
        if (options->useGravity) { // if gravity is used, only kill below 'floor level'
          if (part.reversegrav) { // skip at x = 0
            if (newX < 0)
              killthis = false;
          } else { // skip at x = max
            if (newX > 0)
              killthis = false;
          }
        }
        if (killthis)
          part.ttl = 0;
      }
    }

    if (!part.fixed)
      part.x = (int16_t)newX; // set new position
    else
      part.vx = 0; // set speed to zero. note: particle can get speed in collisions, if unfixed, it should not speed away
  }
}

// apply a force in x direction to individual particle (or source)
// caller needs to provide a 8bit counter (for each paticle) that holds its value between calls
// force is in 3.4 fixed point notation so force=16 means apply v+1 each frame default of 8 is every other frame
void ParticleSystem1D::applyForce(PSparticle1D *part, int8_t xforce, uint8_t *counter) {
  int32_t dv = calcForce_dv(xforce, counter); // velocity increase
  part->vx = limitSpeed((int32_t)part->vx + dv);   // apply the force to particle
}

// apply a force to all particles
// force is in 3.4 fixed point notation (see above)
void ParticleSystem1D::applyForce(int8_t xforce) {
  int32_t dv = calcForce_dv(xforce, &forcecounter); // velocity increase
  for (uint32_t i = 0; i < usedParticles; i++) {
    particles[i].vx = limitSpeed((int32_t)particles[i].vx + dv);
  }
}

// apply gravity to all particles using PS global gforce setting
// gforce is in 3.4 fixed point notation, see note above
void ParticleSystem1D::applyGravity() {
  int32_t dv_raw = calcForce_dv(gforce, &gforcecounter);
  for (uint32_t i = 0; i < usedParticles; i++) {
    int32_t dv = dv_raw;
    if (particles[i].reversegrav) dv = -dv_raw;
    // note: not checking if particle is dead is omitted as most are usually alive and if few are alive, rendering is fast anyways
    particles[i].vx = limitSpeed((int32_t)particles[i].vx - dv);
  }
}

// apply gravity to single particle using system settings (use this for sources)
// function does not increment gravity counter, if gravity setting is disabled, this cannot be used
void ParticleSystem1D::applyGravity(PSparticle1D *part) {
  uint32_t counterbkp = gforcecounter;
  int32_t dv = calcForce_dv(gforce, &gforcecounter);
  if (part->reversegrav) dv = -dv;
  gforcecounter = counterbkp; //save it back
  part->vx = limitSpeed((int32_t)part->vx - dv);
}


// slow down particle by friction, the higher the speed, the higher the friction. a high friction coefficient slows them more (255 means instant stop)
// note: a coefficient smaller than 0 will speed them up (this is a feature, not a bug), coefficient larger than 255 inverts the speed, so don't do that
void ParticleSystem1D::applyFriction(int32_t coefficient) {
  int32_t friction = 255 - coefficient;
  for (uint32_t i = 0; i < usedParticles; i++) {
    if (particles[i].ttl)
      particles[i].vx = ((int32_t)particles[i].vx * friction) / 255; // note: cannot use bitshift as vx can be negative
  }
}


// render particles to the LED buffer (uses palette to render the 8bit particle color value)
// if wrap is set, particles half out of bounds are rendered to the other side of the matrix
// warning: do not render out of bounds particles or system will crash! rendering does not check if particle is out of bounds
void ParticleSystem1D::ParticleSys_render() {
  CRGB baseRGB;
  bool useLocalBuffer = true; //use local rendering buffer, gives huge speed boost (at least 30% more FPS)
  CRGB *framebuffer = NULL; //local frame buffer
  CRGB *renderbuffer = NULL; //local particle render buffer for advanced particles
  uint32_t brightness; // particle brightness, fades if dying

  if (useLocalBuffer) {
    framebuffer = allocate1Dbuffer(maxXpixel + 1); // allocate memory for the local renderbuffer
    if (framebuffer == NULL) {
      DEBUG_PRINT(F("Frame buffer alloc failed"));
      useLocalBuffer = false; // render to segment pixels directly if not enough memory
    }
    else {
      if (advPartProps)
        renderbuffer = allocate1Dbuffer(10); // buffer to render individual particles to if size > 0. note: null checking is done when accessing it
      if (motionBlur > 0) { // using SEGMENT.fadeToBlackBy is much slower, this approximately doubles the speed of fade calculation
        for (uint32_t x = 0; x <= maxXpixel; x++) {
          framebuffer[x] = SEGMENT.getPixelColor(x); // copy to local buffer
          fast_color_scale(framebuffer[x], motionBlur);
        }
      }
    }
  }

  if (!useLocalBuffer) { // disabled or allocation above failed
    if (motionBlur > 0)
      SEGMENT.fadeToBlackBy(255 - motionBlur);
    else
      SEGMENT.fill(BLACK); // clear the buffer before rendering to it
  }
  bool wrap = particlesettings.wrap; // local copy for speed
  // go over particles and render them to the buffer
  for (uint32_t i = 0; i < usedParticles; i++) {
    if (particles[i].outofbounds || particles[i].ttl == 0)
      continue;

    // generate RGB values for particle
    brightness = min(particles[i].ttl, (uint16_t)255);
    baseRGB = ColorFromPalette(SEGPALETTE, particles[i].hue, 255, LINEARBLEND);

    if (advPartProps) { //saturation is advanced property in 1D system
      if (advPartProps[i].sat < 255) {
        CHSV baseHSV = rgb2hsv_approximate(baseRGB); //convert to HSV
        baseHSV.s = advPartProps[i].sat; //set the saturation
        baseRGB = (CRGB)baseHSV; // convert back to RGB
      }
    }
    renderParticle(framebuffer, i, brightness, baseRGB, renderbuffer, wrap);
  }

  if (useLocalBuffer) { // transfer local buffer back to segment
    for (unsigned x = 0; x <= maxXpixel; x++) {
      SEGMENT.setPixelColor(x, framebuffer[x]);
    }
    free(framebuffer);
  }
  if (renderbuffer)
    free(renderbuffer);
}

// calculate pixel positions and brightness distribution and render the particle to local buffer or global buffer
void ParticleSystem1D::renderParticle(CRGB *framebuffer, const uint32_t particleindex, const uint32_t brightness, const CRGB &color, CRGB *renderbuffer, const bool wrap) {
  uint32_t size = particlesize;
  if (advPartProps) {// use advanced size properties
    size = advPartProps[particleindex].size;
  }
  if (size == 0) { //single pixel particle, can be out of bounds as oob checking is made for 2-pixel particles (and updating it uses more code)
    uint32_t x =  particles[particleindex].x >> PS_P_RADIUS_SHIFT_1D;
    if (x <= maxXpixel) { //by making x unsigned there is no need to check < 0 as it will overflow
      if (framebuffer)
        fast_color_add(framebuffer[x], color, brightness);
      else
        SEGMENT.addPixelColor(x, color.scale8((uint8_t)brightness));
    }
  }
  else { //render larger particles
    bool pxlisinframe[2] = {true, true};
    int32_t pxlbrightness[2];
    int32_t pixco[2]; // physical pixel coordinates of the two pixels representing a particle

    // add half a radius as the rendering algorithm always starts at the bottom left, this leaves things positive, so shifts can be used, then shift coordinate by a full pixel (x-- below)
    int32_t xoffset = particles[particleindex].x + PS_P_HALFRADIUS_1D;
    int32_t dx = xoffset & (PS_P_RADIUS_1D - 1); //relativ particle position in subpixel space,  modulo replaced with bitwise AND
    int32_t x = xoffset >> PS_P_RADIUS_SHIFT_1D; // divide by PS_P_RADIUS, bitshift of negative number stays negative -> checking below for x < 0 works (but does not when using division)

    // set the raw pixel coordinates
    pixco[1] = x;  // right pixel
    x--; // shift by a full pixel here, this is skipped above to not do -1 and then +1
    pixco[0] = x;  // left pixel

    // now check if any are out of frame. set values to -1 if they are so they can be easily checked after (no value calculation, no setting of pixelcolor if value < 0)
    if (x < 0) { // left pixels out of frame
      if (wrap) // wrap x to the other side if required
        pixco[0] = maxXpixel;
      else
        pxlisinframe[0] = false; // pixel is out of matrix boundaries, do not render
    }
    else if (pixco[1] > (int32_t)maxXpixel) { // right pixel, only has to be checkt if left pixel did not overflow
      if (wrap) // wrap y to the other side if required
        pixco[1] = 0;
      else
        pxlisinframe[1] = false;
    }

    //calculate the brightness values for both pixels using linear interpolation (note: in standard rendering out of frame pixels could be skipped but if checks add more clock cycles over all)
    pxlbrightness[0] = (((int32_t)PS_P_RADIUS_1D - dx) * brightness) >> PS_P_SURFACE_1D;
    pxlbrightness[1] = (dx * brightness) >> PS_P_SURFACE_1D;

    // check if particle has advanced size properties and buffer is available
    if (advPartProps && advPartProps[particleindex].size > 1) {
      if (renderbuffer && framebuffer) { // TODO: add unbuffered large size rendering like in 2D system
        memset(renderbuffer, 0, 10 * sizeof(CRGB)); // clear the buffer, renderbuffer is 10 pixels
      }
      else
        return; // cannot render advanced particles without buffer

      //render particle to a bigger size
      //particle size to pixels: 2 - 63 is 4 pixels, < 128 is 6pixels, < 192 is 8 pixels, bigger is 10 pixels
      //first, render the pixel to the center of the renderbuffer, then apply 1D blurring
      fast_color_add(renderbuffer[4], color, pxlbrightness[0]);
      fast_color_add(renderbuffer[5], color, pxlbrightness[1]);
      uint32_t rendersize = 2; // initialize render size, minimum is 4 pixels, it is incremented int he loop below to start with 4
      uint32_t offset = 4; // offset to zero coordinate to write/read data in renderbuffer (actually needs to be 3, is decremented in the loop below)
      uint32_t blurpasses = size/64 + 1; // number of blur passes depends on size, four passes max
      uint32_t bitshift = 0;
      for(uint32_t i = 0; i < blurpasses; i++) {
        if (i == 2) //for the last two passes, use higher amount of blur (results in a nicer brightness gradient with soft edges)
          bitshift = 1;
        rendersize += 2;
        offset--;
        blur1D(renderbuffer, rendersize, size << bitshift, offset);
        size = size > 64 ? size - 64 : 0;
      }

      // calculate origin coordinates to render the particle to in the framebuffer
      uint32_t xfb_orig = x - (rendersize>>1) + 1 - offset; //note: using uint is fine
      uint32_t xfb; // coordinates in frame buffer to write to note: by making this uint, only overflow has to be checked

      // transfer particle renderbuffer to framebuffer
      for(uint32_t xrb = offset; xrb < rendersize+offset; xrb++) {
        xfb = xfb_orig + xrb;
        if (xfb > maxXpixel) {
          if (wrap) { // wrap x to the other side if required
            if (xfb > maxXpixel << 1) // xfb is "negative" (note: for some reason, this check is needed in 1D but works without in 2D...)
              xfb = (maxXpixel +1) + (int32_t)xfb; //TODO: remove this again and see if it works now (changed maxxpixel to unsigned)
            else
              xfb = xfb % (maxXpixel + 1); //TODO: can modulo be avoided?
          }
          else
            continue;
        }
        fast_color_add(framebuffer[xfb], renderbuffer[xrb]); // TODO: add unbuffered large size rendering like in 2D system
      }
    }
    else { // standard rendering (2 pixels per particle)
      for(uint32_t i = 0; i < 2; i++) {
        if (pxlisinframe[i]) {
          if (framebuffer)
            fast_color_add(framebuffer[pixco[i]], color, pxlbrightness[i]);
          else
             SEGMENT.addPixelColor(pixco[i], color.scale8((uint8_t)pxlbrightness[i]));
        }
      }
    }
  }
}

// detect collisions in an array of particles and handle them
void ParticleSystem1D::handleCollisions() {
  uint32_t i, j;
  int32_t collisiondistance = PS_P_MINHARDRADIUS_1D;

  for (i = 0; i < usedParticles; i++) {
    // go though all 'higher number' particles and see if any of those are in close proximity and if they are, make them collide
    if (particles[i].ttl > 0 && particles[i].outofbounds == 0 && particles[i].collide) { // if particle is alive and does collide and is not out of view
      int32_t dx; // distance to other particles
      for (j = i + 1; j < usedParticles; j++) { // check against higher number particles
        if (particles[j].ttl > 0 && particles[j].collide) { // if target particle is alive and collides
          if (advPartProps) { // use advanced size properties
            collisiondistance = PS_P_MINHARDRADIUS_1D + (((uint32_t)advPartProps[i].size + (uint32_t)advPartProps[j].size) >> 1);
          }
          dx = particles[j].x - particles[i].x;
          int32_t dv = (int32_t)particles[j].vx - (int32_t)particles[i].vx;
          int32_t proximity = collisiondistance;
          if (dv >= proximity) // particles would go past each other in next move update
            proximity += abs(dv); // add speed difference to catch fast particles
          if (dx < proximity && dx > -proximity) { // check if close
            collideParticles(&particles[i], &particles[j], dx, dv, collisiondistance);
          }
        }
      }
    }
  }
}

// handle a collision if close proximity is detected, i.e. dx and/or dy smaller than 2*PS_P_RADIUS
// takes two pointers to the particles to collide and the particle hardness (softer means more energy lost in collision, 255 means full hard)
void ParticleSystem1D::collideParticles(PSparticle1D *particle1, PSparticle1D *particle2, int32_t dx, int32_t relativeVx, uint32_t collisiondistance) {
  int32_t dotProduct = (dx * relativeVx); // is always negative if moving towards each other
  if (dotProduct < 0) { // particles are moving towards each other
    uint32_t surfacehardness = max(collisionHardness, (int32_t)PS_P_MINSURFACEHARDNESS_1D); // if particles are soft, the impulse must stay above a limit or collisions slip through
    // TODO: if soft collisions are not needed, the above line can be done in set hardness function and skipped here (which is what it currently looks like)
    // Calculate new velocities after collision
    int32_t impulse = relativeVx * surfacehardness / 255;
    particle1->vx += impulse;
    particle2->vx -= impulse;

    // if one of the particles is fixed, transfer the impulse back so it bounces
    if (particle1->fixed)
      particle2->vx = -particle1->vx;
    else if (particle2->fixed)
      particle1->vx = -particle2->vx;

    if (collisionHardness < PS_P_MINSURFACEHARDNESS_1D) { // if particles are soft, they become 'sticky' i.e. apply some friction (they do pile more nicely and correctly)
      const uint32_t coeff = collisionHardness + (255 - PS_P_MINSURFACEHARDNESS_1D);
      particle1->vx = ((int32_t)particle1->vx * coeff) / 255;
      particle2->vx = ((int32_t)particle2->vx * coeff) / 255;
    }
  }

  uint32_t distance = abs(dx);
  // particles have volume, push particles apart if they are too close
  // behaviour is different than in 2D, we need pixel accurate stacking here, push the top particle to full radius (direction is well defined in 1D)
  // also need to give the top particle some speed to counteract gravity or stacks just collapse
  if (distance <  collisiondistance) { //particles are too close, push the upper particle away
    int32_t pushamount = 1 + ((collisiondistance - distance) >> 1); //add half the remaining distance note: this works best, if less or more is added, it gets more chaotic
    //int32_t pushamount = collisiondistance - distance;
    if (particlesettings.useGravity) { //using gravity, push the 'upper' particle only
      if (dx < 0) { // particle2.x < particle1.x
      if (particle2->reversegrav && !particle2->fixed) {
        particle2->x -= pushamount;
        particle2->vx--;
      } else if (!particle1->reversegrav && !particle1->fixed) {
        particle1->x += pushamount;
        particle1->vx++;
      }
      } else {
      if (particle1->reversegrav && !particle1->fixed) {
        particle1->x -= pushamount;
        particle1->vx--;
      } else if (!particle2->reversegrav && !particle2->fixed) {
        particle2->x += pushamount;
        particle2->vx++;
      }
      }
    }
    else { //not using gravity, push both particles by applying a little velocity (like in 2D system), results in much nicer stacking when applying forces
      pushamount = 1;
      if (dx < 0)  // particle2.x < particle1.x
        pushamount = -1;
      particle1->vx -= pushamount;
      particle2->vx += pushamount;
    }
  }
}

// update size and pointers (memory location and size can change dynamically)
// note: do not access the PS class in FX befor running this function (or it messes up SEGENV.data)
void ParticleSystem1D::updateSystem(void) {
  setSize(SEGMENT.virtualLength()); // update size
  updatePSpointers(advPartProps != NULL);
}

// set the pointers for the class (this only has to be done once and not on every FX call, only the class pointer needs to be reassigned to SEGENV.data every time)
// function returns the pointer to the next byte available for the FX (if it assigned more memory for other stuff using the above allocate function)
// FX handles the PSsources, need to tell this function how many there are
void ParticleSystem1D::updatePSpointers(bool isadvanced) {
  // Note on memory alignment:
  // a pointer MUST be 4 byte aligned. sizeof() in a struct/class is always aligned to the largest element. if it contains a 32bit, it will be padded to 4 bytes, 16bit is padded to 2byte alignment.
  // The PS is aligned to 4 bytes, a PSparticle is aligned to 2 and a struct containing only byte sized variables is not aligned at all and may need to be padded when dividing the memoryblock.
  // by making sure that the number of sources and particles is a multiple of 4, padding can be skipped here as alignent is ensured, independent of struct sizes.
  particles = reinterpret_cast<PSparticle1D *>(this + 1); // pointer to particle array at data+sizeof(ParticleSystem)
  sources = reinterpret_cast<PSsource1D *>(particles + numParticles); // pointer to source(s)
  PSdataEnd = reinterpret_cast<uint8_t *>(sources + numSources); // pointer to first available byte after the PS for FX additional data
  if (isadvanced) {
    advPartProps = reinterpret_cast<PSadvancedParticle1D *>(sources + numSources);
    PSdataEnd = reinterpret_cast<uint8_t *>(advPartProps + numParticles);
  }
}

//non class functions to use for initialization
uint32_t calculateNumberOfParticles1D(bool isadvanced) {
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

uint32_t calculateNumberOfSources1D(uint8_t requestedsources) {
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
bool allocateParticleSystemMemory1D(uint16_t numparticles, uint16_t numsources, bool isadvanced, uint16_t additionalbytes) {
  uint32_t requiredmemory = sizeof(ParticleSystem1D);
  // functions above make sure these are a multiple of 4 bytes (to avoid alignment issues)
  requiredmemory += sizeof(PSparticle1D) * numparticles;
  if (isadvanced)
    requiredmemory += sizeof(PSadvancedParticle1D) * numparticles;
  requiredmemory += sizeof(PSsource1D) * numsources;
  requiredmemory += additionalbytes;
  return(SEGMENT.allocateData(requiredmemory));
}

// initialize Particle System, allocate additional bytes if needed (pointer to those bytes can be read from particle system class: PSdataEnd)
// note: requestedparticles is relative, 127 = 50%, 255 = 100% (deafaults to 100% meaning one particle per pixel)
bool initParticleSystem1D(ParticleSystem1D *&PartSys, uint32_t requestedsources, uint32_t requestedparticles, uint16_t additionalbytes, bool advanced) {
  if (SEGLEN == 1) return false; // single pixel not supported
  uint32_t numparticles = (requestedparticles * calculateNumberOfParticles1D(advanced)) / 255;
  uint32_t numsources = calculateNumberOfSources1D(requestedsources);
  if (!allocateParticleSystemMemory1D(numparticles, numsources, advanced, additionalbytes)) {
    DEBUG_PRINT(F("PS init failed: memory depleted"));
    return false;
  }
  PartSys = new (SEGENV.data) ParticleSystem1D(SEGMENT.virtualLength(), numparticles, numsources, advanced); // particle system constructor
  return true;
}

// blur a 1D buffer, sub-size blurring can be done using start and size
// for speed, 32bit variables are used, make sure to limit them to 8bit (0-255) or result is undefined
// to blur a subset of the buffer, change the size and set start to the desired starting coordinates
void blur1D(CRGB *colorbuffer, uint32_t size, uint32_t blur, uint32_t start)
{
  CRGB seeppart, carryover;
  uint32_t seep = blur >> 1;
  carryover =  BLACK;
  for(uint32_t x = start; x < start + size; x++) {
    seeppart = colorbuffer[x]; // create copy of current color
    fast_color_scale(seeppart, seep); // scale it and seep to neighbours
    if (x > 0) {
      fast_color_add(colorbuffer[x-1], seeppart);
      fast_color_add(colorbuffer[x], carryover); // is black on first pass
    }
    carryover = seeppart;
  }
}
#endif // WLED_DISABLE_PARTICLESYSTEM1D

#if !defined(WLED_DISABLE_PARTICLESYSTEM2D) || !defined(WLED_DISABLE_PARTICLESYSTEM1D)

//////////////////////////////
// Shared Utility Functions //
//////////////////////////////

// calculate the delta speed (dV) value and update the counter for force calculation (is used several times, function saves on codesize)
// force is in 3.4 fixedpoint notation, +/-127
static int32_t calcForce_dv(int8_t force, uint8_t* counter) {
  if (force == 0)
    return 0;
  // for small forces, need to use a delay counter
  int32_t force_abs = abs(force); // absolute value (faster than lots of if's only 7 instructions)
  int32_t dv = 0;
  // for small forces, need to use a delay counter, apply force only if it overflows
  if (force_abs < 16) {
    *counter += force_abs;
    if (*counter > 15) {
      *counter -= 16;
      dv = force < 0 ? -1 : 1; // force is either 1 or -1 if it is small (zero force is handled above)
    }
  }
  else
    dv = force / 16; // MSBs, note: cannot use bitshift as dv can be negative

  return dv;
}

// limit speed to prevent overflows
static int32_t limitSpeed(int32_t speed) {
  return min((int32_t)PS_P_MAXSPEED, max((int32_t)-PS_P_MAXSPEED, speed));
  //return speed > PS_P_MAXSPEED ? PS_P_MAXSPEED : (speed < -PS_P_MAXSPEED ? -PS_P_MAXSPEED : speed); // note: this uses more code, not sure due to speed or inlining
}

// check if particle is out of bounds and wrap it around if required
static bool checkBoundsAndWrap(int32_t &position, const int32_t max, const int32_t particleradius, bool wrap) {
  if ((uint32_t)position > max) { // check if particle reached an edge
    if (wrap) {
      position = position % (max + 1); // note: cannot optimize modulo, particles can be far out of bounds when wrap is enabled
      if (position < 0)
        position += max + 1;
    }
    else if (((position < -particleradius) || (position > max + particleradius))) // particle is leaving boundaries, out of bounds if it has fully left
      return false; // out of bounds
  }
  return true; // particle is in bounds
}

// fastled color adding is very inaccurate in color preservation (but it is fast)
// a better color add function is implemented in colors.cpp but it uses 32bit RGBW. to use it colors need to be shifted just to then be shifted back by that function, which is slow
// this is a fast version for RGB (no white channel, PS does not handle white) and with native CRGB including scaling of second color
// note: result is stored in c1, not using a return value is faster as the CRGB struct does not need to be copied upon return
// note2: function is mainly used to add scaled colors, so checking if one color is black is slower
// note3: scale is 255 when using blur, checking for that makes blur faster
static void fast_color_add(CRGB &c1, const CRGB &c2, const uint32_t scale) {
  uint32_t r, g, b;
  if (scale < 255) {
    r = c1.r + ((c2.r * scale) >> 8);
    g = c1.g + ((c2.g * scale) >> 8);
    b = c1.b + ((c2.b * scale) >> 8);
  } else {
    r = c1.r + c2.r;
    g = c1.g + c2.g;
    b = c1.b + c2.b;
  }

  uint32_t max = std::max(r,g); // check for overflow, using max() is faster as the compiler can optimize
  max = std::max(max,b);
  if (max < 256) {
    c1.r = r; // save result to c1
    c1.g = g;
    c1.b = b;
  } else {
    c1.r = (r * 255) / max;  // note: compile optimizes the divisions, no need to manually optimize
    c1.g = (g * 255) / max;
    c1.b = (b * 255) / max;
  }
}

// faster than fastled color scaling as it does in place scaling
static void fast_color_scale(CRGB &c, const uint32_t scale) {
  c.r = ((c.r * scale) >> 8);
  c.g = ((c.g * scale) >> 8);
  c.b = ((c.b * scale) >> 8);
}

// allocate memory for the 1D CRGB array in one contiguous block and set values to zero
static CRGB* allocate1Dbuffer(uint32_t length) {
  CRGB *array = (CRGB *)calloc(length, sizeof(CRGB));
  return array;
}

#endif  // !defined(WLED_DISABLE_PARTICLESYSTEM2D) || !defined(WLED_DISABLE_PARTICLESYSTEM1D)
