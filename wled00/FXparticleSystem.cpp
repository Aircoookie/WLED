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

/*
	Note on ESP32: using 32bit integer is faster than 16bit or 8bit, each operation takes on less instruction, can be testen on https://godbolt.org/
	it does not matter if using int, unsigned int, uint32_t or int32_t, the compiler will make int into 32bit
	this should be used to optimize speed but not if memory is affected much
*/

/*
  TODO:
  -add function to 'update sources' so FX does not have to take care of that. FX can still implement its own version if so desired. config should be optional, if not set, use default config.
  -add possiblity to emit more than one particle, just pass a source and the amount to emit or even add several sources and the amount, function decides if it should do it fair or not
  -add an x/y struct, do particle rendering using that, much easier to read
  -extend rendering to more than 2x2, 3x2 (fire) should be easy, 3x3 maybe also doable without using much math (need to see if it looks good)
  

*/
// sources need to be updatable by the FX, so functions are needed to apply it to a single particle that are public
#include "FXparticleSystem.h"
#include "wled.h"
#include "FastLED.h"
#include "FX.h"

ParticleSystem::ParticleSystem(uint16_t width, uint16_t height, uint16_t numberofparticles, uint16_t numberofsources)
{
	//Serial.println("PS Constructor");
	numSources = numberofsources;
	numParticles = numberofparticles; // set number of particles in the array
	usedParticles = numberofparticles; // use all particles by default
	//particlesettings = {false, false, false, false, false, false, false, false}; // all settings off by default
	updatePSpointers(); // set the particle and sources pointer (call this before accessing sprays or particles)
	setMatrixSize(width, height);
	setWallHardness(255); // set default wall hardness to max
	particlesize = 0; //minimum size
	motionBlur = 0; //no fading by default
	emitIndex = 0;
	/*
	Serial.println("alive particles: ");
	for (int i = 0; i < numParticles; i++)
	{
		//particles[i].ttl = 0; //initialize all particles to dead
		//if (particles[i].ttl)
		{
			Serial.print("x:");
			Serial.print(particles[i].x);
			Serial.print(" y:");
			Serial.println(particles[i].y);
		}
	}*/
	//Serial.println("PS Constructor done");
}

//update function applies gravity, moves the particles, handles collisions and renders the particles
void ParticleSystem::update(void)
{
	//apply gravity globally if enabled
	if (particlesettings.useGravity)
		applyGravity(particles, usedParticles, gforce, &gforcecounter);
	
	// handle collisions (can push particles, must be done before updating particles or they can render out of bounds, causing a crash if using local buffer for speed)
	if (particlesettings.useCollisions)
		handleCollisions();

	//move all particles
	for (int i = 0; i < usedParticles; i++)
	{
		particleMoveUpdate(particles[i], particlesettings);
	}	

	ParticleSys_render();
}

//update function for fire animation
void ParticleSystem::updateFire(uint32_t intensity)
{
	fireParticleupdate();
	ParticleSys_render(true, intensity);
}

void ParticleSystem::setUsedParticles(uint16_t num)
{
	usedParticles = min(num, numParticles); //limit to max particles
}

void ParticleSystem::setWallHardness(uint8_t hardness)
{
	wallHardness = hardness;
}

void ParticleSystem::setCollisionHardness(uint8_t hardness)
{	
	collisionHardness = hardness;  
}

void ParticleSystem::setMatrixSize(uint16_t x, uint16_t y)
{
	maxXpixel = x - 1; // last physical pixel that can be drawn to
	maxYpixel = y - 1;
	maxX = x * PS_P_RADIUS - 1;	// particle system boundary for movements
	maxY = y * PS_P_RADIUS - 1;	// this value is often needed by FX to calculate positions
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
	motionBlur = bluramount;
}

// render size using smearing
void ParticleSystem::setParticleSize(uint8_t size)
{
	particlesize = size;
}
// enable/disable gravity, optionally, set the force (force=8 is default) can be 1-255, 0 is also disable
// if enabled, gravity is applied to all particles in ParticleSystemUpdate()
void ParticleSystem::enableGravity(bool enable, uint8_t force) 
{
	particlesettings.useGravity = enable;
	if (force > 0)
		gforce = force;
	else 
		particlesettings.useGravity = false;	
}

void ParticleSystem::enableParticleCollisions(bool enable, uint8_t hardness) // enable/disable gravity, optionally, set the force (force=8 is default) can be 1-255, 0 is also disable
{
	particlesettings.useCollisions = enable;
	collisionHardness = hardness + 1;
}

// emit one particle with variation
void ParticleSystem::sprayEmit(PSsource &emitter)
{
	for (uint32_t i = 0; i < usedParticles; i++)
	{
		emitIndex++;
		if (emitIndex >= usedParticles)
			emitIndex = 0;
		if (particles[emitIndex].ttl == 0) // find a dead particle
		{
			particles[emitIndex].x = emitter.source.x; // + random16(emitter.var) - (emitter.var >> 1); //randomness uses cpu cycles and is almost invisible, removed for now.
			particles[emitIndex].y = emitter.source.y; // + random16(emitter.var) - (emitter.var >> 1);
			particles[emitIndex].vx = emitter.vx + random(emitter.var) - (emitter.var>>1);
			particles[emitIndex].vy = emitter.vy + random(emitter.var) - (emitter.var>>1);
			particles[emitIndex].ttl = random16(emitter.maxLife - emitter.minLife) + emitter.minLife;
			particles[emitIndex].hue = emitter.source.hue;
			particles[emitIndex].sat = emitter.source.sat;
			particles[emitIndex].collide = emitter.source.collide;
			break;
		}
		/*	
		if (emitIndex < 2)
		{
		Serial.print(" ");
		Serial.print(particles[emitIndex].ttl);
		Serial.print(" ");
		Serial.print(particles[emitIndex].x);
		Serial.print(" ");
		Serial.print(particles[emitIndex].y);
		}*/
	}
	//Serial.println("**");
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
			particles[emitIndex].vx = emitter.vx + random16(emitter.var) - (emitter.var >> 1); //random16 is good enough for fire and much faster
			particles[emitIndex].vy = emitter.vy + random16(emitter.var) - (emitter.var >> 1);
			particles[emitIndex].ttl = random16(emitter.maxLife - emitter.minLife) + emitter.minLife + emitter.source.ttl; // flame intensity dies down with emitter TTL
			// fire uses ttl and not hue for heat, so no need to set the hue
			break; // done
		}
	}
}

//todo: idee: man könnte einen emitter machen, wo die anzahl emittierten partikel von seinem alter abhängt. benötigt aber einen counter
//idee2: source einen counter hinzufügen, dann setting für emitstärke, dann müsste man das nicht immer in den FX animationen handeln

// Emits a particle at given angle and speed, angle is from 0-65535 (=0-360deg), speed is also affected by emitter->var
// angle = 0 means in positive x-direction (i.e. to the right)
void ParticleSystem::angleEmit(PSsource &emitter, uint16_t angle, int8_t speed)
{
	emitter.vx = ((int32_t)cos16(angle) * (int32_t)speed) / (int32_t)32600; // cos16() and sin16() return signed 16bit, division should be 32767 but 32600 gives slightly better rounding 
	emitter.vy = ((int32_t)sin16(angle) * (int32_t)speed) / (int32_t)32600; // note: cannot use bit shifts as bit shifting is asymmetrical for positive and negative numbers and this needs to be accurate!
	sprayEmit(emitter);
	Serial.print(" x: ");
	Serial.print(emitter.vx);
	Serial.print(" y: ");
	Serial.println(emitter.vy);
}

// particle moves, decays and dies, if killoutofbounds is set, out of bounds particles are set to ttl=0
// uses passed settings to set bounce or wrap, if useGravity is set, it will never bounce at the top and killoutofbounds is not applied over the top
void ParticleSystem::particleMoveUpdate(PSparticle &part, PSsettings &options)
{
	if (part.ttl > 0)
	{
		// age
		part.ttl--;
		if (particlesettings.colorByAge)
			part.hue = part.ttl > 255 ? 255 : part.ttl; //set color to ttl

		int32_t newX = part.x + (int16_t)part.vx;
		int32_t newY = part.y + (int16_t)part.vy;
		part.outofbounds = 0; // reset out of bounds (in case particle was created outside the matrix and is now moving into view)

		//if wall collisions are enabled, bounce them before they reach the edge, it looks much nicer if the particle is not half out of vew
		if (options.bounceX) 
		{
			if ((newX < PS_P_RADIUS) || (newX > maxX - PS_P_RADIUS)) // reached a wall
			{
				part.vx = -part.vx;					  // invert speed
				part.vx = (part.vx * wallHardness) / 255; // reduce speed as energy is lost on non-hard surface
				if (newX < PS_P_RADIUS)
					newX = PS_P_RADIUS; // fast particles will never reach the edge if position is inverted
				else
					newX = maxX - PS_P_RADIUS;
			}
		}
		
		if ((newX < 0) || (newX > maxX)) // check if particle reached an edge
		{			
			if (options.wrapX)
			{
				newX = wraparound(newX, maxX);
			}
			else if (((newX <= -PS_P_HALFRADIUS) || (newX > maxX + PS_P_HALFRADIUS))) // particle is leaving, set out of bounds if it has fully left
			{
				part.outofbounds = 1;
				if (options.killoutofbounds)
					part.ttl = 0;
			}
		}

		if (options.bounceY) 
		{
			if ((newY < PS_P_RADIUS) || (newY > maxY - PS_P_RADIUS)) // reached floor / ceiling
			{
				if (newY < PS_P_RADIUS) // bounce at bottom
				{
						part.vy = -part.vy; // invert speed
						part.vy = ((int32_t)part.vy * wallHardness) / 255; // reduce speed as energy is lost on non-hard surface
						newY = PS_P_RADIUS;		
				}
				else
				{
					if (options.useGravity) // do not bounce on top if using gravity (open container) if this is needed implement it in the FX
					{
						if (newY > maxY + PS_P_HALFRADIUS)
							part.outofbounds = 1; // set out of bounds, kill out of bounds over the top does not apply if gravity is used (user can implement it in FX if needed)
					}
					else
					{
						part.vy = -part.vy;	// invert speed
						part.vy = ((int32_t)part.vy * wallHardness) / 255; // reduce speed as energy is lost on non-hard surface
						newY = maxY - PS_P_RADIUS;	
					}
				}		
			}
		}
		
		if (((newY < 0) || (newY > maxY))) // check if particle reached an edge
		{
			 if (options.wrapY)
			{
				newY = wraparound(newY, maxY);
			}
			else if (((newY <= -PS_P_HALFRADIUS) || (newY > maxY + PS_P_HALFRADIUS))) // particle is leaving, set out of bounds if it has fully left
			{
				part.outofbounds = 1;
				if (options.killoutofbounds)
				{
					if (newY < 0) // if gravity is enabled, only kill particles below ground
						part.ttl = 0;
					else if (!options.useGravity)
						part.ttl = 0;
				}
			}

		}
		
		part.x = (int16_t)newX; // set new position
		part.y = (int16_t)newY; // set new position
	}
}

// apply a force in x,y direction to particles
// caller needs to provide a 8bit counter that holds its value between calls for each group (numparticles can be 1 for single particle)
// force is in 3.4 fixed point notation so force=16 means apply v+1 each frame default of 8 is every other frame (gives good results)
void ParticleSystem::applyForce(PSparticle *part, uint32_t numparticles, int8_t xforce, int8_t yforce, uint8_t *counter)
{
	// for small forces, need to use a delay counter
	uint8_t xcounter = (*counter) & 0x0F; // lower four bits
	uint8_t ycounter = (*counter) >> 4;	  // upper four bits

	// velocity increase
	int32_t dvx = calcForce_dv(xforce, &xcounter);
	int32_t dvy = calcForce_dv(yforce, &ycounter);

	// save counter values back
	*counter |= xcounter & 0x0F;		// write lower four bits, make sure not to write more than 4 bits
	*counter |= (ycounter << 4) & 0xF0; // write upper four bits

	// apply the force to particle:
	int32_t i = 0;
	if (dvx != 0)
	{
		for (i = 0; i < numparticles; i++)
		{
			// note: not checking if particle is dead is faster as most are usually alive and if few are alive, rendering is faster so no speed penalty			
			part[i].vx = limitSpeed((int32_t)particles[i].vx + dvx);
		}
	}
	if (dvy != 0)
	{
		for (i = 0; i < numparticles; i++)
		{
			part[i].vy = limitSpeed((int32_t)particles[i].vy + dvy);
		}	
	}
}

// apply a force in x,y direction to particles directly (no counter required but no 'sub 1' force supported)
void ParticleSystem::applyForce(PSparticle *part, uint32_t numparticles, int8_t xforce, int8_t yforce)
{
	//note: could make this faster for single particles by adding an if statement, but it is fast enough as is
	for (uint i = 0; i < numparticles; i++)
	{
		// note: not checking if particle is dead is faster as most are usually alive and if few are alive, rendering is faster so no speed penalty
		part[i].vx = limitSpeed((int32_t)part[i].vx + (int32_t)xforce);
		part[i].vy = limitSpeed((int32_t)part[i].vy + (int32_t)yforce);
	}	
}

// apply a force in angular direction to group of particles //TODO: actually test if this works as expected, this is untested code
// caller needs to provide a 8bit counter that holds its value between calls for each group (numparticles can be 1 for single particle)
// angle is from 0-65535 (=0-360deg) angle = 0 means in positive x-direction (i.e. to the right)
void ParticleSystem::applyAngleForce(PSparticle *part, uint32_t numparticles, uint8_t force, uint16_t angle, uint8_t *counter)
{
	int8_t xforce = ((int32_t)force * cos16(angle)) / 32767; // force is +/- 127
	int8_t yforce = ((int32_t)force * sin16(angle)) / 32767; // note: cannot use bit shifts as bit shifting is asymmetrical for positive and negative numbers and this needs to be accurate!
	// note: sin16 is 10% faster than sin8() on ESP32 but on ESP8266 it is 9% slower
	// force is in 3.4 fixed point notation so force=16 means apply v+1 each frame (useful force range is +/- 127) 
	applyForce(part, numparticles, xforce, yforce, counter);
}

// apply a force in angular direction to particles directly (no counter required but no 'sub 1' force supported)
// angle is from 0-65535 (=0-360deg) angle = 0 means in positive x-direction (i.e. to the right)
void ParticleSystem::applyAngleForce(PSparticle *part, uint32_t numparticles, uint8_t force, uint16_t angle)
{
	int8_t xforce = ((int32_t)force * cos16(angle)) / 32767; // force is +/- 127
	int8_t yforce = ((int32_t)force * sin16(angle)) / 32767; // note: cannot use bit shifts as bit shifting is asymmetrical for positive and negative numbers and this needs to be accurate!
	applyForce(part, numparticles, xforce, yforce);
}

// apply gravity to a group of particles
// faster than apply force since direction is always down and counter is fixed for all particles
// caller needs to provide a 8bit counter that holds its value between calls
// force is in 3.4 fixed point notation so force=16 means apply v+1 each frame default of 8 is every other frame (gives good results)
// positive force means down
void ParticleSystem::applyGravity(PSparticle *part, uint32_t numarticles, int8_t force, uint8_t *counter)
{
	int32_t dv = calcForce_dv(force, counter);
	for (uint32_t i = 0; i < numarticles; i++)
	{		
		// note: not checking if particle is dead is faster as most are usually alive and if few are alive, rendering is fast anyways
		part[i].vy = limitSpeed((int32_t)particles[i].vy - dv);
	}
	
}

//apply gravity using PS global gforce
void ParticleSystem::applyGravity(PSparticle *part, uint32_t numarticles, uint8_t *counter)
{
	applyGravity(part, numarticles, gforce, counter);
}

//apply gravity to single particle using system settings (use this for sources)
//function does not increment gravity counter, if gravity setting is disabled, this cannot be used
void ParticleSystem::applyGravity(PSparticle *part)
{
	int32_t dv; // velocity increase
	if (gforce > 15)
		dv = (gforce >> 4); // apply the 4 MSBs
	else
		dv = 1;

	if (gforcecounter + gforce > 15) //counter is updated in global update when applying gravity
	{		
		part->vy = limitSpeed((int32_t)part->vy - dv);
	}
}

// slow down particles by friction, the higher the speed, the higher the friction. a high friction coefficient slows them more (255 means instant stop)
void ParticleSystem::applyFriction(PSparticle *part, uint8_t coefficient)
{
	int32_t friction = 255 - coefficient;
	// note: not checking if particle is dead is faster as most are usually alive and if few are alive, rendering is faster
	// note2: cannot use right shifts as bit shifting in right direction is asymmetrical for positive and negative numbers and this needs to be accurate or things start to go to the left side.
	part->vx = ((int16_t)part->vx * friction) / 255; 
	part->vy = ((int16_t)part->vy * friction) / 255; 
}

// apply friction to all particles
void ParticleSystem::applyFriction(uint8_t coefficient)
{
	int32_t friction = 255 - coefficient;	
	for (uint32_t i = 0; i < usedParticles; i++)
	{
		// note: not checking if particle is dead is faster as most are usually alive and if few are alive, rendering is faster
		// note2: cannot use right shifts as bit shifting in right direction is asymmetrical for positive and negative numbers and this needs to be accurate or things start to go to the left side.
		particles[i].vx = ((int16_t)particles[i].vx * friction) / 255; 
		particles[i].vy = ((int16_t)particles[i].vy * friction) / 255; 
	}
}

// attracts a particle to an attractor particle using the inverse square-law
void ParticleSystem::pointAttractor(PSparticle *part, PSparticle *attractor, uint8_t *counter, uint8_t strength, bool swallow)
{
	// Calculate the distance between the particle and the attractor
	int32_t dx = attractor->x - part->x;
	int32_t dy = attractor->y - part->y;

	// Calculate the force based on inverse square law
	int32_t distanceSquared = dx * dx + dy * dy;
	if (distanceSquared < 8192)
	{
		if (swallow) // particle is close, age it fast so it fades out, do not attract further
		{			
			if (part->ttl > 7)
				part->ttl -= 8; 
			else
			{
				part->ttl = 0;
				return;
			}
		}
		distanceSquared = 2 * PS_P_RADIUS * PS_P_RADIUS; // limit the distance to avoid very high forces
	}

	int32_t force = ((int32_t)strength << 16) / distanceSquared;
	int8_t xforce = (force * dx) / 1024; // scale to a lower value, found by experimenting
	int8_t yforce = (force * dy) / 1024; // note: cannot use bit shifts as bit shifting is asymmetrical for positive and negative numbers and this needs to be accurate!

	applyForce(part, 1, xforce, yforce, counter);
}

void ParticleSystem::lineAttractor(PSparticle *part, PSparticle *attractorcenter, uint16_t attractorangle, uint8_t *counter, uint8_t strength)
{
	// Calculate the distance between the particle and the attractor

	//calculate a second point on the line
	int32_t x1 = attractorcenter->x + (cos16(attractorangle) >> 5);
	int32_t y1 = attractorcenter->y + (sin16(attractorangle) >> 5);
	//calculate squared distance from particle to the line:
	int32_t dx = (x1 - attractorcenter->x) >> 4;
	int32_t dy = (y1 - attractorcenter->y) >> 4;
	int32_t d = ((dx * (part->y - attractorcenter->y)) - (dy * (part->x - attractorcenter->x))) >> 8;
	int32_t distanceSquared = (d * d) / (dx * dx + dy * dy);


	// Calculate the force based on inverse square law
	if (distanceSquared < 2)
	{
		distanceSquared = 1;
	//	distanceSquared = 4 * PS_P_RADIUS * PS_P_RADIUS; // limit the distance to avoid very high forces
	}

	int32_t force = (((int32_t)strength << 16) / distanceSquared)>>10;
	//apply force in a 90° angle to the line
	int8_t xforce = (d > 0 ? 1 : -1) * (force * dy) / 100; // scale to a lower value, found by experimenting
	int8_t yforce = (d > 0 ? -1 : 1) * (force * dx) / 100; // note: cannot use bit shifts as bit shifting is asymmetrical for positive and negative numbers and this needs to be accurate!	
/*
	Serial.print(" partx: ");
	Serial.print(part->x);
	Serial.print(" party ");
	Serial.print(part->y);
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
	Serial.println(yforce);*/
	applyForce(part, 1, xforce, yforce, counter);
}
// render particles to the LED buffer (uses palette to render the 8bit particle color value)
// if wrap is set, particles half out of bounds are rendered to the other side of the matrix
// warning: do not render out of bounds particles or system will crash! rendering does not check if particle is out of bounds
// fireintensity and firemode are optional arguments (fireintensity is only used in firemode)
void ParticleSystem::ParticleSys_render(bool firemode, uint32_t fireintensity)
{
	int32_t pixco[4][2]; //physical pixel coordinates of the four pixels a particle is rendered to. x,y pairs
	CRGB baseRGB;
	bool useLocalBuffer = true; //use local rendering buffer, gives huge speed boost (at least 30% more FPS)
	CRGB **colorbuffer;
	uint32_t i;
	uint32_t brightness; // particle brightness, fades if dying	
	// CRGB colorbuffer[maxXpixel+1][maxYpixel+1] = {0}; //put buffer on stack (not a good idea, can cause crashes on large segments if other function run the stack into the heap)
	if (useLocalBuffer)
	{
		//  allocate memory for the local renderbuffer
		colorbuffer = allocate2Dbuffer(maxXpixel + 1, maxYpixel + 1);
		if (colorbuffer == NULL)
			useLocalBuffer = false; //render to segment pixels directly if not enough memory	

		if (motionBlur > 0) // using SEGMENT.fadeToBlackBy is much slower, this approximately doubles the speed of fade calculation
		{
			uint32_t residual = motionBlur; //32bit for faster calculation
			uint32_t yflipped;
			for (int y = 0; y <= maxYpixel; y++)
			{
				yflipped = maxYpixel - y;
				for (int x = 0; x <= maxXpixel; x++)
				{
					colorbuffer[x][y] = SEGMENT.getPixelColorXY(x, yflipped);
					colorbuffer[x][y].r = (colorbuffer[x][y].r * residual) >> 8;
					colorbuffer[x][y].g = (colorbuffer[x][y].g * residual) >> 8;
					colorbuffer[x][y].b = (colorbuffer[x][y].b * residual) >> 8;
				}
			}
		}
	}
	else
	{
		if (motionBlur > 0)
			SEGMENT.fadeToBlackBy(256 - motionBlur);
		else
			SEGMENT.fill(BLACK); //clear the buffer before rendering to it 
	}
	// go over particles and render them to the buffer
	for (i = 0; i < usedParticles; i++)
	{
		if (particles[i].outofbounds || particles[i].ttl == 0)
			continue;

		// generate RGB values for particle
		if(firemode)
		{
			//brightness = (uint32_t)particles[i].ttl * (1 + (fireintensity >> 4)) + (fireintensity >> 2); //this is good
			//brightness = (uint32_t)particles[i].ttl * (fireintensity >> 3) + (fireintensity >> 1); // this is experimental, also works, flamecolor is more even, does not look as good (but less puffy at lower speeds)
			//brightness = (((uint32_t)particles[i].ttl * (maxY + PS_P_RADIUS - particles[i].y)) >> 7) + (uint32_t)particles[i].ttl * (fireintensity >> 4) + (fireintensity >> 1); // this is experimental //multiplikation mit weniger als >>4 macht noch mehr puffs bei low speed
			//brightness = (((uint32_t)particles[i].ttl * (maxY + PS_P_RADIUS - particles[i].y)) >> 7) + particles[i].ttl + (fireintensity>>1); // this is experimental
			//brightness = (((uint32_t)particles[i].ttl * (maxY + PS_P_RADIUS - particles[i].y)) >> 7) + ((particles[i].ttl * fireintensity) >> 5); // this is experimental TODO: test this -> testing... ok but not the best, bit sparky
			brightness = (((uint32_t)particles[i].ttl * (maxY + PS_P_RADIUS - particles[i].y)) >> 7) + (fireintensity >> 1); // this is experimental TODO: test this -> testing... does not look too bad!
			brightness > 255 ? 255 : brightness; // faster then using min()
			baseRGB = ColorFromPalette(SEGPALETTE, brightness, 255, LINEARBLEND);
		}
		else{
			brightness = particles[i].ttl > 255 ? 255 : particles[i].ttl; //faster then using min()
			baseRGB = ColorFromPalette(SEGPALETTE, particles[i].hue, 255, LINEARBLEND);
			if (particles[i].sat < 255)
			{
				CHSV baseHSV = rgb2hsv_approximate(baseRGB); //convert to hsv
				baseHSV.s = particles[i].sat; //desaturate
				baseRGB = (CRGB)baseHSV; //convert back to RGB
			}
		}
		int32_t pxlbrightness[4] = {0}; //note: pxlbrightness needs to be set to 0 or checking in rendering function does not work (if values persist), this is faster then setting it to 0 there

		// calculate brightness values for all four pixels representing a particle using linear interpolation and calculate the coordinates of the phyiscal pixels to add the color to
		renderParticle(&particles[i], brightness, pxlbrightness, pixco);
		/*
		//debug: check coordinates if out of buffer boundaries print out some info
		for(uint32_t d; d<4; d++)
		{
			if (pixco[d][0] < 0 || pixco[d][0] > maxXpixel)
			{
				pxlbrightness[d] = -1; //do not render
				Serial.print("uncought out of bounds: x=");
				Serial.print(pixco[d][0]);
				Serial.print("particle x=");
				Serial.print(particles[i].x);
				Serial.print(" y=");
				Serial.println(particles[i].y);
				useLocalBuffer = false;
				free(colorbuffer); // free buffer memory
			}
			if (pixco[d][1] < 0 || pixco[d][1] > maxYpixel)
			{
				pxlbrightness[d] = -1; // do not render
				Serial.print("uncought out of bounds: y=");
				Serial.print(pixco[d][1]);
				Serial.print("particle x=");
				Serial.print(particles[i].x);
				Serial.print(" y=");
				Serial.println(particles[i].y);
				useLocalBuffer = false;
				free(colorbuffer); // free buffer memory
			}
		}*/
		if (useLocalBuffer)
		{
			if (pxlbrightness[0] > 0)
				colorbuffer[pixco[0][0]][pixco[0][1]] = fast_color_add(colorbuffer[pixco[0][0]][pixco[0][1]], baseRGB, pxlbrightness[0]); // bottom left			
			if (pxlbrightness[1] > 0)
				colorbuffer[pixco[1][0]][pixco[1][1]] = fast_color_add(colorbuffer[pixco[1][0]][pixco[1][1]], baseRGB, pxlbrightness[1]); // bottom right
			if (pxlbrightness[2] > 0)
				colorbuffer[pixco[2][0]][pixco[2][1]] = fast_color_add(colorbuffer[pixco[2][0]][pixco[2][1]], baseRGB, pxlbrightness[2]); // top right			
			if (pxlbrightness[3] > 0)
				colorbuffer[pixco[3][0]][pixco[3][1]] = fast_color_add(colorbuffer[pixco[3][0]][pixco[3][1]], baseRGB, pxlbrightness[3]); // top left																																			
		}
		else
		{			
			if (pxlbrightness[0] > 0)
				SEGMENT.addPixelColorXY(pixco[0][0], maxYpixel - pixco[0][1], baseRGB.scale8((uint8_t)pxlbrightness[0])); // bottom left
			if (pxlbrightness[1] > 0)
				SEGMENT.addPixelColorXY(pixco[1][0], maxYpixel - pixco[1][1], baseRGB.scale8((uint8_t)pxlbrightness[1])); // bottom right
			if (pxlbrightness[2] > 0)
				SEGMENT.addPixelColorXY(pixco[2][0], maxYpixel - pixco[2][1], baseRGB.scale8((uint8_t)pxlbrightness[2])); // top right
			if (pxlbrightness[3] > 0)			
				SEGMENT.addPixelColorXY(pixco[3][0], maxYpixel - pixco[3][1], baseRGB.scale8((uint8_t)pxlbrightness[3])); // top left
			/*
			uint32_t color = RGBW32(baseRGB.r, baseRGB.g, baseRGB.b, 0);
			if (pxlbrightness[0] > 0)
				SEGMENT.addPixelColorXY(pixco[0][0], maxYpixel - pixco[0][1], color_scale(color, pxlbrightness[0])); // bottom left
			if (pxlbrightness[1] > 0)
				SEGMENT.addPixelColorXY(pixco[1][0], maxYpixel - pixco[1][1], color_scale(color, pxlbrightness[1])); // bottom right
			if (pxlbrightness[2] > 0)
				SEGMENT.addPixelColorXY(pixco[2][0], maxYpixel - pixco[2][1], color_scale(color, pxlbrightness[2])); // top right
			if (pxlbrightness[3] > 0)
				SEGMENT.addPixelColorXY(pixco[3][0], maxYpixel - pixco[3][1], color_scale(color, pxlbrightness[3])); // top left
			*/



			// test to render larger pixels with minimal effort (not working yet, need to calculate coordinate from actual dx position but brightness seems right), could probably be extended to 3x3
			//	SEGMENT.addPixelColorXY(pixco[1][0] + 1, maxYpixel - pixco[1][1], baseRGB.scale8((uint8_t)((brightness>>1) - pxlbrightness[0])), fastcoloradd);
			//	SEGMENT.addPixelColorXY(pixco[2][0] + 1, maxYpixel - pixco[2][1], baseRGB.scale8((uint8_t)((brightness>>1) -pxlbrightness[3])), fastcoloradd);
		}
	}

	if(particlesize > 0)
	{
		if (useLocalBuffer) 
		{
		//TODO: come up with a good and short 2D smearing function derived from blur
		//put it in a function taking width, height and buffer pointer, so it can be used to blur individual particles for different sizes (will be slow) -> or maybe not, there is little use
		// and would need individual sizes for each particle, 
		}
		else
			SEGMENT.blur(particlesize, true); //todo: come up with good algorithm for size
	}

	if (useLocalBuffer) //transfer local buffer back to segment
	{
		uint32_t yflipped;
		for (int y = 0; y <= maxYpixel; y++)
		{
			yflipped = maxYpixel - y;
			for (int x = 0; x <= maxXpixel; x++)
			{
				SEGMENT.setPixelColorXY(x, yflipped, colorbuffer[x][y]);
			}
		}
		free(colorbuffer); // free buffer memory
	}
}

// calculate pixel positions and brightness distribution for rendering function
// pixelpositions are the physical positions in the matrix that the particle renders to (4x2 array for the four positions)
void ParticleSystem::renderParticle(PSparticle* particle, uint32_t brightess, int32_t *pixelvalues, int32_t (*pixelpositions)[2])
{
	// subtract half a radius as the rendering algorithm always starts at the bottom left, this makes calculations more efficient
	int32_t xoffset = particle->x - PS_P_HALFRADIUS;
	int32_t yoffset = particle->y - PS_P_HALFRADIUS;
	int32_t dx = xoffset % PS_P_RADIUS; //relativ particle position in subpixel space
	int32_t dy = yoffset % PS_P_RADIUS;
	int32_t x = xoffset >> PS_P_RADIUS_SHIFT; // divide by PS_P_RADIUS which is 64, so can bitshift (compiler may not optimize automatically)
	int32_t y = yoffset >> PS_P_RADIUS_SHIFT;

	// set the four raw pixel coordinates, the order is bottom left [0], bottom right[1], top right [2], top left [3]
	pixelpositions[0][0] = pixelpositions[3][0] = x;	 // bottom left & top left
	pixelpositions[0][1] = pixelpositions[1][1] = y;	 // bottom left & bottom right
	pixelpositions[1][0] = pixelpositions[2][0] = x + 1; // bottom right & top right
	pixelpositions[2][1] = pixelpositions[3][1] = y + 1; // top right & top left

	// now check if any are out of frame. set values to -1 if they are so they can be easily checked after (no value calculation, no setting of pixelcolor if value < 0)	
	
	if (x < 0) // left pixels out of frame
	{
		dx = PS_P_RADIUS + dx;		// if x<0, xoffset becomes negative (and so does dx), must adjust dx as modulo will flip its value (really old bug now finally fixed)
		//note: due to inverted shift math, a particel at position -32 (xoffset = -64, dx = 64) is rendered at the wrong pixel position (it should be out of frame)
		//checking this above makes this algorithm slower (in frame pixels do not have to be checked), so just correct for it here:
		if (dx == PS_P_RADIUS)
		{
			pixelvalues[1] = pixelvalues[2] = -1; // pixel is actually out of matrix boundaries, do not render
		}
		if (particlesettings.wrapX) // wrap x to the other side if required
			pixelpositions[0][0] = pixelpositions[3][0] = maxXpixel;
		else
			pixelvalues[0] = pixelvalues[3] = -1; // pixel is out of matrix boundaries, do not render
	}
	else if (pixelpositions[1][0] > maxXpixel) // right pixels, only has to be checkt if left pixels did not overflow
	{
		if (particlesettings.wrapX) // wrap y to the other side if required
			pixelpositions[1][0] = pixelpositions[2][0] = 0;
		else
			pixelvalues[1] = pixelvalues[2] = -1;
	}

	if (y < 0) // bottom pixels out of frame
	{
		dy = PS_P_RADIUS + dy; //see note above
		if (dy == PS_P_RADIUS)
		{
			pixelvalues[2] = pixelvalues[3] = -1; // pixel is actually out of matrix boundaries, do not render
		}
		if (particlesettings.wrapY) // wrap y to the other side if required
			pixelpositions[0][1] = pixelpositions[1][1] = maxYpixel;
		else
			pixelvalues[0] = pixelvalues[1] = -1;
	}
	else if (pixelpositions[2][1] > maxYpixel) // top pixels
	{
		if (particlesettings.wrapY) // wrap y to the other side if required
			pixelpositions[2][1] = pixelpositions[3][1] = 0;
		else
			pixelvalues[2] = pixelvalues[3] = -1;
	}

	// calculate brightness values for all four pixels representing a particle using linear interpolation
	// precalculate values for speed optimization
	int32_t precal1 = (int32_t)PS_P_RADIUS - dx;
	int32_t precal2 = ((int32_t)PS_P_RADIUS - dy) * brightess;
	int32_t precal3 = dy * brightess;

	//calculate the values for pixels that are in frame
	if (pixelvalues[0] >= 0)
		pixelvalues[0] = (precal1 * precal2) >> PS_P_SURFACE; // bottom left value equal to ((PS_P_RADIUS - dx) * (PS_P_RADIUS-dy) * brightess) >> PS_P_SURFACE
	if (pixelvalues[1] >= 0)
		pixelvalues[1] = (dx * precal2) >> PS_P_SURFACE; // bottom right value equal to (dx * (PS_P_RADIUS-dy) * brightess) >> PS_P_SURFACE
	if (pixelvalues[2] >= 0)
		pixelvalues[2] = (dx * precal3) >> PS_P_SURFACE; // top right value equal to (dx * dy * brightess) >> PS_P_SURFACE
	if (pixelvalues[3] >= 0)
		pixelvalues[3] = (precal1 * precal3) >> PS_P_SURFACE; // top left value equal to ((PS_P_RADIUS-dx) * dy * brightess) >> PS_P_SURFACE
/*
	Serial.print("x:");
	Serial.print(particle->x);
	Serial.print(" y:");
	Serial.print(particle->y);
	//Serial.print(" xo");
	//Serial.print(xoffset);
	//Serial.print(" dx");
	//Serial.print(dx);
	//Serial.print(" ");
	for(uint8_t t = 0; t<4; t++)
	{
		Serial.print(" v");
		Serial.print(pixelvalues[t]);
		Serial.print(" x");
		Serial.print(pixelpositions[t][0]);
		Serial.print(" y");
		Serial.print(pixelpositions[t][1]);

		Serial.print(" ");
	}
	Serial.println(" ");
*/
/*
	// debug: check coordinates if out of buffer boundaries print out some info
	for (uint32_t d = 0; d < 4; d++)
	{
		if (pixelpositions[d][0] < 0 || pixelpositions[d][0] > maxXpixel)
		{
			//Serial.print("<");
			if (pixelvalues[d] >= 0)
			{				
				Serial.print("uncought out of bounds: x:");
				Serial.print(pixelpositions[d][0]);
				Serial.print(" y:");
				Serial.print(pixelpositions[d][1]);
				Serial.print("particle x=");
				Serial.print(particle->x);
				Serial.print(" y=");
				Serial.println(particle->y);
				pixelvalues[d] = -1; // do not render
			}
		}
		if (pixelpositions[d][1] < 0 || pixelpositions[d][1] > maxYpixel)
		{
			//Serial.print("^");
			if (pixelvalues[d] >= 0)
			{		
				Serial.print("uncought out of bounds: x:");
				Serial.print(pixelpositions[d][0]);
				Serial.print(" y:");
				Serial.print(pixelpositions[d][1]);
				Serial.print("particle x=");
				Serial.print(particle->x);
				Serial.print(" y=");
				Serial.println(particle->y);				
				pixelvalues[d] = -1; // do not render
			}
		}
	}
*/
}

// update & move particle, wraps around left/right if settings.wrapX is true, wrap around up/down if settings.wrapY is true
// particles move upwards faster if ttl is high (i.e. they are hotter)
void ParticleSystem::fireParticleupdate()
{
	//TODO: cleanup this function? check if normal move is much slower, change move function to check y first and check again
	//todo: kill out of bounds funktioniert nicht?
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
			//particles[i].y = particles[i].y + (int32_t)particles[i].vy;// + (particles[i].ttl >> 3); // younger particles move faster upward as they are hotter //!! shift ttl by 2 is the original value, this is experimental
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
						particles[i].x = wraparound(particles[i].x, maxX);						
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
	// if m ore accurate collisions are needed, just call it twice in a row
	if (collisioncounter & 0x01) 
	{ 
		startparticle = endparticle;
		endparticle = usedParticles;
	}	
	collisioncounter++;

	//startparticle = 0;//!!! test: do all collisions every frame, FPS goes from about 52 to 
	//endparticle = usedParticles;

	for (i = startparticle; i < endparticle; i++)
	{
		// go though all 'higher number' particles and see if any of those are in close proximity and if they are, make them collide		
		if (particles[i].ttl > 0 && particles[i].outofbounds == 0  && particles[i].collide) // if particle is alive and does collide and is not out of view
		{
			int32_t dx, dy; // distance to other particles
			for (j = i + 1; j < usedParticles; j++)
			{							  // check against higher number particles
				if (particles[j].ttl > 0) // if target particle is alive
				{
					dx = particles[i].x - particles[j].x;
					if (dx < PS_P_HARDRADIUS && dx > -PS_P_HARDRADIUS) // check x direction, if close, check y direction
					{
						dy = particles[i].y - particles[j].y;
						if (dy < PS_P_HARDRADIUS && dy > -PS_P_HARDRADIUS) // particles are close
							collideParticles(&particles[i], &particles[j]);
					}
				}
			}
		}
	}
}



// handle a collision if close proximity is detected, i.e. dx and/or dy smaller than 2*PS_P_RADIUS
// takes two pointers to the particles to collide and the particle hardness (softer means more energy lost in collision, 255 means full hard)
void ParticleSystem::collideParticles(PSparticle *particle1, PSparticle *particle2) //TODO: dx,dy is calculated just above, can pass it over here to save a few CPU cycles
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
		else if(relativeVx == 0) //if true 
		{
			relativeVx = 1;
		}	
		
		dy = -1;
		if (relativeVy < 0)
			dy = 1;
		else if (relativeVy == 0)
		{
			relativeVy = 1;
		}

		distanceSquared = 2; //1 + 1
	}

	// Calculate dot product of relative velocity and relative distance
	
	int32_t dotProduct = (dx * relativeVx + dy * relativeVy); //is always negative if moving towards each other	
	int32_t notsorandom = dotProduct & 0x01; // random16(2); //dotprouct LSB should be somewhat random, so no need to calculate a random number

	if (dotProduct < 0) // particles are moving towards each other
	{
		// integer math used to avoid floats. 
		// overflow check: dx/dy are 7bit, relativV are 8bit -> dotproduct is 15bit, dotproduct/distsquared ist 8b, multiplied by collisionhardness of 8bit. so a 16bit shift is ok, make it 15 to be sure no overflows happen
		// note: cannot use right shifts as bit shifting in right direction is asymmetrical for positive and negative numbers and this needs to be accurate! the trick is: only shift positive numers		
		// Calculate new velocities after collision
		uint32_t surfacehardness = collisionHardness < PS_P_MINSURFACEHARDNESS ? PS_P_MINSURFACEHARDNESS : collisionHardness; // if particles are soft, the impulse must stay above a limit or collisions slip through at higher speeds, 170 seems to be a good value
		int32_t impulse = -(((((-dotProduct) << 15) / distanceSquared) * surfacehardness) >> 8); // note: inverting before bitshift corrects for asymmetry in right-shifts (and is slightly faster)
		int32_t ximpulse = ((impulse) * dx) / 32767; //cannot use bit shifts here, it can be negative, use division by 2^bitshift
		int32_t yimpulse = ((impulse) * dy) / 32767;
		particle1->vx += ximpulse;
		particle1->vy += yimpulse;
		particle2->vx -= ximpulse;
		particle2->vy -= yimpulse;

		if (collisionHardness < surfacehardness) // if particles are soft, they become 'sticky' i.e. apply some friction
		{
			const uint32_t coeff = collisionHardness + (255 - PS_P_MINSURFACEHARDNESS);
			particle1->vx = ((int32_t)particle1->vx * coeff) / 255; 
			particle1->vy = ((int32_t)particle1->vy * coeff) / 255;

			particle2->vx = ((int32_t)particle2->vx * coeff) / 255;
			particle2->vy = ((int32_t)particle2->vy * coeff) / 255;

			if (collisionHardness < 10) // if they are very soft, stop slow particles completely to make them stick to each other
			{
				particle1->vx = (particle1->vx < 3 && particle1->vx > -3) ? 0 : particle1->vx;
				particle1->vy = (particle1->vy < 3 && particle1->vy > -3) ? 0 : particle1->vy;

				particle2->vx = (particle2->vx < 3 && particle2->vx > -3) ? 0 : particle2->vx;
				particle2->vy = (particle2->vy < 3 && particle2->vy > -3) ? 0 : particle2->vy;
			}
		}

		// this part is for particle piling: slow them down if they are close (they become sticky) and push them so they counteract gravity
		// particles have volume, push particles apart if they are too close
		// tried lots of configurations, it works best if not moved but given a little velocity, it tends to oscillate less this way
		// a problem with giving velocity is, that on harder collisions, this adds up as it is not dampened enough, so add friction in the FX if required		
		if (dotProduct > -250) //this means particles are slow (or really really close) so push them apart.
		{
			/**
			//only apply friction if particles are slow or else fast moving particles (as in explosions) get slowed a lot
			relativeVy *= relativeVy; //square the speed, apply friction if speed is below 10
			if (relativeVy < 100) //particles are slow in y direction -> this works but most animations look much nicer without this friction. add friction in FX if required.
			{
				//now check x as well (no need to check if y speed is high, this saves some computation time)
				relativeVx *= relativeVx; // square the speed, apply friction if speed is below 10
				if (relativeVx < 100)	  // particles are slow in x direction
				{
					particle1->vx = ((int32_t)particle1->vx * 254) / 256;
					particle2->vx = ((int32_t)particle2->vx * 254) / 256;

					particle1->vy = ((int32_t)particle1->vy * 254) / 256;
					particle2->vy = ((int32_t)particle2->vy * 254) / 256;

				}
			}*/

		

			// const int32_t HARDDIAMETER = 2 * PS_P_HARDRADIUS; // push beyond the hard radius, helps with keeping stuff fluffed up -> not really
			//  int32_t push = (2 * PS_P_HARDRADIUS * PS_P_HARDRADIUS - distanceSquared) >> 6; // push a small amount, if pushing too much, it becomse chaotic as waves of pushing run through piles
			int32_t pushamount = 1 + ((250 + dotProduct) >> 6); // the closer dotproduct is to zero, the closer the particles are
			int32_t push;

			// if (dx < HARDDIAMETER && dx > -HARDDIAMETER) //this is always true as it is checked before ntering this function!
			{ // distance is too small, push them apart
				push = 0;
				if (dx < 0)			   // particle 1 is on the right
					push = pushamount; //(HARDDIAMETER + dx) / 4;
				else if (dx > 0)
					push = -pushamount; //-(HARDDIAMETER - dx) / 4;
				else					// on the same x coordinate, shift it a little so they do not stack
				{

					if (notsorandom)
						particle1->x++; // move it so pile collapses
					else
						particle1->x--;
				}

				particle1->vx += push;
			}

			// if (dy < HARDDIAMETER && dy > -HARDDIAMETER) //dito
			{
				push = 0;
				if (dy < 0)
					push = pushamount; //(HARDDIAMETER + dy) / 4;
				else if (dy > 0)
					push = -pushamount; //-(HARDDIAMETER - dy) / 4;
				else					// dy==0
				{
					if (notsorandom)
						particle1->y++; // move it so pile collapses
					else
						particle1->y--;
				}

				particle1->vy += push;
			}
			/*
			if (dx < HARDDIAMETER && dx > -HARDDIAMETER)
			{ // distance is too small, push them apart
				push = 0;
				if (dx < 0)			// particle 1 is on the right
					push = 2; //(HARDDIAMETER + dx) / 4;
				else if (dx > 0)
					push = -2; //-(HARDDIAMETER - dx) / 4;
				else //on the same x coordinate, shift it a little so they do not stack
					particle1->x += 2;
				if (notsorandom) // chose one of the particles to push, avoids oscillations
				{
					if (!particle1->flag3)
					{
						particle1->vx += push;
						particle1->flag3 = 1; // particle was pushed, is reset on next push request
					}
					else
						particle1->flag3 = 0; //reset
				}
				else
				{
					if (!particle2->flag3)
					{
						particle2->vx -= push;
						particle2->flag3 = 1; // particle was pushed, is reset on next push request
					}
					else
						particle2->flag3 = 0; // reset
				}
			}

			if (dy < HARDDIAMETER && dy > -HARDDIAMETER)
			{
				push = 0;
				if (dy < 0)
					push = 2; //(HARDDIAMETER + dy) / 4;
				else if (dy > 0)
					push = -2; //-(HARDDIAMETER - dy) / 4;

				if (!notsorandom)	 // chose one of the particles to push, avoids oscillations
				{
					if (!particle1->flag3)
					{
						particle1->vy += push;
						particle1->flag3 = 1; // particle was pushed, is reset on next push request
					}
					else
						particle1->flag3 = 0; // reset
				}
				else
				{
					if (!particle2->flag3)
					{
						particle2->vy -= push;
						particle2->flag3 = 1; // particle was pushed, is reset on next push request
					}
					else
						particle2->flag3 = 0; // reset
				}
			}*/

			// note: pushing may push particles out of frame, if bounce is active, it will move it back as position will be limited to within frame, if bounce is disabled: bye bye
		}
	}


}

//fast calculation of particle wraparound (modulo version takes 37 instructions, this only takes 28, other variants are slower on ESP8266)
//function assumes that out of bounds is checked before calling it
int32_t ParticleSystem::wraparound(int32_t p, int32_t maxvalue)
{
	if (p < 0)
	{
		p += maxvalue + 1;
	}
	else //if (p > maxvalue) 
	{
		p -= maxvalue + 1;
	}
	return p;
}

//calculate the delta speed (dV) value and update the counter for force calculation (is used several times, function saves on codesize)
//force is in 3.4 fixedpoint notation, +/-127
int32_t ParticleSystem::calcForce_dv(int8_t force, uint8_t* counter)
{
	// for small forces, need to use a delay counter
	int32_t force_abs = abs(force); // absolute value (faster than lots of if's only 7 instructions)
	int32_t dv;
	// for small forces, need to use a delay counter, apply force only if it overflows
	if (force_abs < 16)
	{
		*counter += force_abs;
		if (*counter > 15)
		{
			*counter -= 16;
			dv = (force < 0) ? -1 : ((force > 0) ? 1 : 0); // force is either, 1, 0 or -1 if it is small
		}		
	}
	else
	{
		dv = force >> 4; // MSBs
	}
	return dv;
}

//limit speed to prevent overflows
int32_t ParticleSystem::limitSpeed(int32_t speed)
{
	return speed > PS_P_MAXSPEED ? PS_P_MAXSPEED : (speed < -PS_P_MAXSPEED ? -PS_P_MAXSPEED : speed);
}

// allocate memory for the 2D array in one contiguous block and set values to zero
CRGB **ParticleSystem::allocate2Dbuffer(uint32_t cols, uint32_t rows)
{	
	cli();//!!! test to see if anything messes with the allocation (flicker issues)
	CRGB ** array2D = (CRGB **)malloc(cols * sizeof(CRGB *) + cols * rows * sizeof(CRGB));
	sei();
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
		memset(start, 0, cols * rows * sizeof(CRGB)); // set all values to zero
	}
	return array2D;
}

//update size and pointers (memory location and size can change dynamically)
//note: do not access the PS class in FX befor running this function (or it messes up SEGMENT.data)
void ParticleSystem::updateSystem(void)
{
	// update matrix size
	uint32_t cols = strip.isMatrix ? SEGMENT.virtualWidth() : 1;
	uint32_t rows = strip.isMatrix ? SEGMENT.virtualHeight() : SEGMENT.virtualLength();
	setMatrixSize(cols, rows);
	updatePSpointers();
}

// set the pointers for the class (this only has to be done once and not on every FX call, only the class pointer needs to be reassigned to SEGENV.data every time)
// function returns the pointer to the next byte available for the FX (if it assigned more memory for other stuff using the above allocate function)
// FX handles the PSsources, need to tell this function how many there are
void ParticleSystem::updatePSpointers()
{
	//DEBUG_PRINT(F("*** PS pointers ***"));
	//DEBUG_PRINTF_P(PSTR("this PS %p\n"), this);

	particles = reinterpret_cast<PSparticle *>(this + 1);							  // pointer to particle array at data+sizeof(ParticleSystem)
	sources = reinterpret_cast<PSsource *>(particles + numParticles);				  // pointer to source(s)
	PSdataEnd = reinterpret_cast<uint8_t *>(sources + numSources);					  // pointer to first available byte after the PS
	
	//DEBUG_PRINTF_P(PSTR("particles %p\n"), particles);	
	//DEBUG_PRINTF_P(PSTR("sources %p\n"), sources);	
	//DEBUG_PRINTF_P(PSTR("end %p\n"), PSdataEnd);
}

//non class functions to use for initialization
uint32_t calculateNumberOfParticles()
{
	uint32_t cols = strip.isMatrix ? SEGMENT.virtualWidth() : 1;
	uint32_t rows = strip.isMatrix ? SEGMENT.virtualHeight() : SEGMENT.virtualLength();
#ifdef ESP8266
	uint numberofParticles = (cols * rows * 3) / 4; // 0.75 particle per pixel
	uint particlelimit = ESP8266_MAXPARTICLES; // maximum number of paticles allowed (based on one segment of 16x16 and 4k effect ram)
#elif ARDUINO_ARCH_ESP32S2
	uint numberofParticles = (cols * rows); // 1 particle per pixe
	uint particlelimit = ESP32S2_MAXPARTICLES; // maximum number of paticles allowed (based on one segment of 32x32 and 24k effect ram)
#else
	uint numberofParticles = (cols * rows);		 // 1 particle per pixel (for example 768 particles on 32x16)
	uint particlelimit = ESP32_MAXPARTICLES; // maximum number of paticles allowed (based on two segments of 32x32 and 40k effect ram)
#endif
	numberofParticles = max((uint)1, min(numberofParticles, particlelimit)); 	
	return numberofParticles;
}

uint32_t calculateNumberOfSources()
{
	uint32_t cols = strip.isMatrix ? SEGMENT.virtualWidth() : 1;
	uint32_t rows = strip.isMatrix ? SEGMENT.virtualHeight() : SEGMENT.virtualLength();
#ifdef ESP8266
	int numberofSources = (cols * rows) / 8;
	numberofSources = max(1, min(numberofSources, ESP8266_MAXSOURCES)); // limit to 1 - 16
#elif ARDUINO_ARCH_ESP32S2
	int numberofSources = (cols * rows) / 6;
	numberofSources = max(1, min(numberofSources, ESP32S2_MAXSOURCES)); // limit to 1 - 48
#else
	int numberofSources = (cols * rows) / 4;
	numberofSources = max(1, min(numberofSources, ESP32_MAXSOURCES)); // limit to 1 - 72
#endif
	return numberofSources;
}

//allocate memory for particle system class, particles, sprays plus additional memory requested by FX
bool allocateParticleSystemMemory(uint16_t numparticles, uint16_t numsources, uint16_t additionalbytes)
{
	uint32_t requiredmemory = sizeof(ParticleSystem);
	requiredmemory += sizeof(PSparticle) * numparticles;
	requiredmemory += sizeof(PSsource) * numsources;
	requiredmemory += additionalbytes;
	//Serial.print("allocating: ");
	//Serial.print(requiredmemory);
	//Serial.println("Bytes");
	//Serial.print("allocating for segment at");
	//Serial.println((uintptr_t)SEGMENT.data);
	return(SEGMENT.allocateData(requiredmemory));		
}

// initialize Particle System, allocate additional bytes if needed (pointer to those bytes can be read from particle system class: PSdataEnd)
bool initParticleSystem(ParticleSystem *&PartSys, uint16_t additionalbytes)
{
	//Serial.println("PS init function");
	uint32_t numparticles = calculateNumberOfParticles();
	uint32_t numsources = calculateNumberOfSources();
	if (!allocateParticleSystemMemory(numparticles, numsources, additionalbytes))
	{
		DEBUG_PRINT(F("PS init failed: memory depleted"));
		return false;
	}
	//Serial.print("segment.data ptr");
	//Serial.println((uintptr_t)(SEGMENT.data));
	uint16_t cols = strip.isMatrix ? SEGMENT.virtualWidth() : 1;
	uint16_t rows = strip.isMatrix ? SEGMENT.virtualHeight() : SEGMENT.virtualLength();
	//Serial.println("calling constructor");
	PartSys = new (SEGMENT.data) ParticleSystem(cols, rows, numparticles, numsources); // particle system constructor TODO: why does VS studio thinkt this is bad?
	//Serial.print("PS pointer at ");
	//Serial.println((uintptr_t)PartSys);
	return true;
}

// fastled color adding is very inaccurate in color preservation
// a better color add function is implemented in colors.cpp but it uses 32bit RGBW. to use it colors need to be shifted just to then be shifted back by that function, which is slow
// this is a fast version for RGB (no white channel, PS does not handle white) and with native CRGB including scaling of second color (fastled scale8 can be made faster using native 32bit on ESP)
CRGB fast_color_add(CRGB c1, CRGB c2, uint32_t scale)
{
	CRGB result;
	scale++; //add one to scale so 255 will not scale when shifting
	uint32_t r = c1.r + ((c2.r * (scale)) >> 8);
	uint32_t g = c1.g + ((c2.g * (scale)) >> 8);
	uint32_t b = c1.b + ((c2.b * (scale)) >> 8);
	uint32_t max = r;
	if (g > max) //note: using ? operator would be slower by 2 cpu cycles
		max = g;
	if (b > max)
		max = b;
	if (max < 256)
	{
		result.r = r;
		result.g = g;
		result.b = b;
	}
	else
	{
		result.r = (r * 255) / max;
		result.g = (g * 255) / max;
		result.b = (b * 255) / max;
	}
	return result;
}
