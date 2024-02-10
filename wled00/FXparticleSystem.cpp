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

#include "FXparticleSystem.h"
#include "wled.h"
#include "FastLED.h"
#include "FX.h"

// Fountain style emitter for particles used for flames (particle TTL depends on source TTL)
void Emitter_Flame_emit(PSpointsource *emitter, PSparticle *part)
{
	part->x = emitter->source.x + random8(emitter->var) - (emitter->var >> 1);
	part->y = emitter->source.y + random8(emitter->var) - (emitter->var >> 1);
	part->vx = emitter->vx + random8(emitter->var) - (emitter->var >> 1);
	part->vy = emitter->vy + random8(emitter->var) - (emitter->var >> 1);
	part->ttl = (uint8_t)((rand() % (emitter->maxLife - emitter->minLife)) + emitter->minLife + emitter->source.ttl); // flame intensity dies down with emitter TTL
	part->hue = emitter->source.hue;
	part->sat = emitter->source.sat;
}

// fountain style emitter
void Emitter_Fountain_emit(PSpointsource *emitter, PSparticle *part)
{
	part->x = emitter->source.x; // + random8(emitter->var) - (emitter->var >> 1); //randomness uses cpu cycles and is almost invisible, removed for now.
	part->y = emitter->source.y; // + random8(emitter->var) - (emitter->var >> 1);
	part->vx = emitter->vx + random8(emitter->var) - (emitter->var >> 1);
	part->vy = emitter->vy + random8(emitter->var) - (emitter->var >> 1);
	part->ttl = (rand() % (emitter->maxLife - emitter->minLife)) + emitter->minLife;
	part->hue = emitter->source.hue;
	part->sat = emitter->source.sat;
}

// Emits a particle at given angle and speed, angle is from 0-255 (=0-360deg), speed is also affected by emitter->var
void Emitter_Angle_emit(PSpointsource *emitter, PSparticle *part, uint8_t angle, uint8_t speed)
{
	emitter->vx = (((int16_t)cos8(angle)-127) * speed) >> 7; //cos is signed 8bit, so 1 is 127, -1 is -127, shift by 7
	emitter->vy = (((int16_t)sin8(angle)-127) * speed) >> 7;
	Serial.print(angle);
	Serial.print(" ");
	Serial.print(emitter->vx);
	Serial.print(" ");
	Serial.print(emitter->vy);
	Serial.print(" ");
	Emitter_Fountain_emit(emitter, part);
}
// attracts a particle to an attractor particle using the inverse square-law
void Particle_attractor(PSparticle *particle, PSparticle *attractor, uint8_t *counter, uint8_t strength, bool swallow) // todo: add a parameter 'swallow' so the attractor can 'suck up' particles that are very close, also could use hue of attractor particle for strength
{
	// Calculate the distance between the particle and the attractor
	int16_t dx = attractor->x - particle->x;
	int16_t dy = attractor->y - particle->y;

	// Calculate the force based on inverse square law
	int32_t distanceSquared = dx * dx + dy * dy + 1;
	if (distanceSquared < 4096)
	{
		if (swallow) // particle is close, kill it
		{
			particle->ttl = 0;
			return;
		}
		distanceSquared = 4096; // limit the distance to 64 (=size of a particle) to avoid very high forces.TODO: could make this depending on the #define for particle size
	}
	// check if distance is small enough to even cause a force (for that strength<<10 must be bigger than the distance squared)
	int32_t shiftedstrength = (int32_t)strength << 16;
	int32_t force;
	int32_t xforce;
	int32_t yforce;
	int32_t xforce_abs; // absolute value
	int32_t yforce_abs;

	if (shiftedstrength < distanceSquared) // if far away, set the force to 1 so it still attracts and does not leave particles just sitting outside its influence radius
	{
		// force calculation above is zero
		//give some force in both directions (x and y) to avoid further calculations as this is just to get things moving a little		
		xforce_abs = 1;
		yforce_abs = 1;
	}
	else
	{
		force = shiftedstrength / distanceSquared;
		xforce = (force * dx) >> 10; // scale to a lower value, found by experimenting
		yforce = (force * dy) >> 10;
		xforce_abs = abs(xforce); // absolute value
		yforce_abs = abs(yforce);
	}
	uint8_t xcounter = (*counter) & 0x0F; // lower four bits
	uint8_t ycounter = (*counter) >> 4;	  // upper four bits

	*counter = 0; // reset counter, is set back to correct values below

	// for small forces, need to use a delay timer (counter)
	if (xforce_abs < 16)
	{
		xcounter += xforce_abs;
		if (xcounter > 15)
		{
			xcounter -= 15;
			*counter |= xcounter & 0x0F; // write lower four bits, make sure not to write more than 4 bits
										 // apply force in x direction
			if (dx < 0)
			{
				particle->vx -= 1;
			}
			else
			{
				particle->vx += 1;
			}
		}
	}
	else
	{
		particle->vx += xforce >> 4; // divide by 16
	}

	if (yforce_abs < 16)
	{
		ycounter += yforce_abs;

		if (ycounter > 15)
		{

			ycounter -= 15;
			*counter |= (ycounter << 4) & 0xF0; // write upper four bits

			if (dy < 0)
			{
				particle->vy -= 1;
			}
			else
			{
				particle->vy += 1;
			}
		}
	}
	else
	{
		particle->vy += yforce >> 4; // divide by 16
	}
	// TODO: need to limit the max speed?
}

// TODO: could solve all update functions in a single function with parameters and handle gravity acceleration in a separte function (uses more cpu time but that is not a huge issue) or maybe not, like this, different preferences can be set

void Particle_Move_update(PSparticle *part) // particle moves, decays and dies  
{
	// Matrix dimension
	const uint16_t cols = strip.isMatrix ? SEGMENT.virtualWidth() : 1;
	const uint16_t rows = strip.isMatrix ? SEGMENT.virtualHeight() : SEGMENT.virtualLength();

	// particle box dimensions
	const uint16_t PS_MAX_X(cols * PS_P_RADIUS - 1);
	const uint16_t PS_MAX_Y(rows * PS_P_RADIUS - 1);

	if (part->ttl > 0)
	{
			// age
			part->ttl--;
			
			// apply velocity
			part->x += (int16_t)part->vx;
			part->y += (int16_t)part->vy;

			part->outofbounds = 0; // reset out of bounds (in case particle was created outside the matrix and is now moving into view)

			// check if particle is out of bounds
			if ((part->y <= 0) || (part->y >= PS_MAX_Y))
			{
				part->outofbounds = 1;
			}
			if ((part->x <= 0) || (part->x >= PS_MAX_X))
			{
				part->outofbounds = 1;
			}
	}
}

void Particle_Bounce_update(PSparticle *part, const uint8_t hardness) // bounces a particle on the matrix edges, if surface 'hardness' is <255 some energy will be lost in collision (127 means 50% lost)
{
	// Matrix dimension
	const uint16_t cols = strip.isMatrix ? SEGMENT.virtualWidth() : 1;
	const uint16_t rows = strip.isMatrix ? SEGMENT.virtualHeight() : SEGMENT.virtualLength();

	// particle box dimensions
	const uint16_t PS_MAX_X(cols * PS_P_RADIUS - 1);
	const uint16_t PS_MAX_Y(rows * PS_P_RADIUS - 1);

	if (part->ttl > 0)
	{
		// age
		part->ttl--;

		// apply velocity
		int16_t newX, newY;

		// apply velocity
		newX = part->x + (int16_t)part->vx;
		newY = part->y + (int16_t)part->vy;

		if ((newX <= 0) || (newX >= PS_MAX_X))
		{															   // reached an edge
			part->vx = -part->vx;									   // invert speed
			part->vx = (((int16_t)part->vx) * ((int16_t)hardness+1)) >> 8; // reduce speed as energy is lost on non-hard surface
		}

		if ((newY <= 0) || (newY >= PS_MAX_Y))
		{															   // reached an edge
			part->vy = -part->vy;									   // invert speed
			part->vy = (((int16_t)part->vy) * ((int16_t)hardness+1)) >> 8; // reduce speed as energy is lost on non-hard surface
		}

		newX = max(newX, (int16_t)0); // limit to positive
		newY = max(newY, (int16_t)0);
		part->x = min(newX, (int16_t)PS_MAX_X); // limit to matrix boundaries
		part->y = min(newY, (int16_t)PS_MAX_Y);
	}
}

void Particle_Gravity_update(PSparticle *part, bool wrapX, bool bounceX, bool bounceY, const uint8_t hardness) // particle moves, decays and dies (age or out of matrix), if wrapX is set, pixels leaving in x direction reappear on other side, hardness is surface hardness for bouncing (127 means 50% speed lost each bounce)
{

	// Matrix dimension
	const uint16_t cols = strip.isMatrix ? SEGMENT.virtualWidth() : 1;
	const uint16_t rows = strip.isMatrix ? SEGMENT.virtualHeight() : SEGMENT.virtualLength();

	// particle box dimensions
	const uint16_t PS_MAX_X(cols * PS_P_RADIUS - 1);
	const uint16_t PS_MAX_Y(rows * PS_P_RADIUS - 1);

	if (part->ttl > 0)
	{
		// age
		part->ttl--;

		// check if particle is out of bounds or died
		if ((part->y < -PS_P_RADIUS) || (part->y >= PS_MAX_Y << 1))
		{ // if it moves more than 1 pixel below y=0, it will not come back. also remove particles that too far above
			part->ttl = 0;
			return; // particle died, we are done
		}
		if (wrapX == false)
		{
			if ((part->x < -PS_MAX_X) || (part->x >= PS_MAX_X << 1))
			{ // left and right: keep it alive as long as its not too far out (if adding more effects like wind, it may come back)
				part->ttl = 0;
				return; // particle died, we are done
			}
		}

		// apply acceleration (gravity) every other frame, doing it every frame is too strong
		if (SEGMENT.call % 2 == 0)
		{
			if (part->vy > -MAXGRAVITYSPEED)
				part->vy = part->vy - 1;
		}

		// apply velocity
		int16_t newX, newY;

		newX = part->x + (int16_t)part->vx;
		newY = part->y + (int16_t)part->vy;

		part->outofbounds = 0;
		// check if particle is outside of displayable matrix

		// x direction, handle wraparound (will overrule bounce x) and bounceX
		if (wrapX)
		{
			newX = newX % (PS_MAX_X + 1);
			if (newX < 0)
				newX = PS_MAX_X - newX;
		}
		else
		{
			if (newX < 0 || newX > PS_MAX_X)
			{ // reached an edge
				if (bounceX)
				{
					part->vx = -part->vx;									   // invert speed
					part->vx = (((int16_t)part->vx) * (int16_t)hardness) >> 8; // reduce speed as energy is lost on non-hard surface
					newX = max(newX, (int16_t)0);							   // limit to positive
					newX = min(newX, (int16_t)PS_MAX_X);					   // limit to matrix boundaries
				}
				else // not bouncing and out of matrix
					part->outofbounds = 1;
			}
		}

		part->x = newX; // set new position

		// y direction, handle bounceY (bounces at ground only)
		if (newY < 0)
		{ // || newY > PS_MAX_Y)	{ //reached an edge
			if (bounceY)
			{
				part->vy = -part->vy;									   // invert speed
				part->vy = (((int16_t)part->vy) * (int16_t)hardness) >> 8; // reduce speed as energy is lost on non-hard surface
				part->y += (int16_t)part->vy;							   // move particle back to within boundaries so it does not disappear for one frame
				newY = max(newY, (int16_t)0);							   // limit to positive
				// newY = min(newY, (int16_t)PS_MAX_Y); //limit to matrix boundaries
			}
			else // not bouncing and out of matrix
				part->outofbounds = 1;
		}

		part->y = newY; // set new position
	}
}

// render particles to the LED buffer (uses palette to render the 8bit particle color value)
// if wrap is set, particles half out of bounds are rendered to the other side of the matrix
// saturation is color saturation, if not set to 255, hsv instead of palette is used (palette does not support saturation)
void ParticleSys_render(PSparticle *particles, uint16_t numParticles, bool wrapX, bool wrapY, bool fastcoloradd)
{

	const uint16_t cols = strip.isMatrix ? SEGMENT.virtualWidth() : 1;
	const uint16_t rows = strip.isMatrix ? SEGMENT.virtualHeight() : SEGMENT.virtualLength();

	// particle box dimensions
	const uint16_t PS_MAX_X(cols * PS_P_RADIUS - 1);
	const uint16_t PS_MAX_Y(rows * PS_P_RADIUS - 1);

	int16_t x, y;
	uint8_t dx, dy;
	uint32_t intensity; // todo: can this be an uint8_t or will it mess things up?
	CRGB baseRGB;
	uint16_t i;
	uint8_t brightess; // particle brightness, fades if dying
	

	// go over particles and update matrix cells on the way
	for (i = 0; i < numParticles; i++)
	{
		if (particles[i].ttl == 0 || particles[i].outofbounds)
		{
			continue;
		}
		// generate RGB values for particle
		brightess = min(particles[i].ttl, (uint16_t)255);

		if (particles[i].sat < 255)
		{
			CHSV baseHSV = rgb2hsv_approximate(ColorFromPalette(SEGPALETTE, particles[i].hue, 255, LINEARBLEND));
			baseHSV.s = particles[i].sat;
			baseRGB = (CRGB)baseHSV;
		}
		else
			baseRGB = ColorFromPalette(SEGPALETTE, particles[i].hue, 255, LINEARBLEND);

		dx = (uint8_t)((uint16_t)particles[i].x % (uint16_t)PS_P_RADIUS);
		dy = (uint8_t)((uint16_t)particles[i].y % (uint16_t)PS_P_RADIUS);

		x = (uint8_t)((uint16_t)particles[i].x / (uint16_t)PS_P_RADIUS);
		y = (uint8_t)((uint16_t)particles[i].y / (uint16_t)PS_P_RADIUS);

		// for vx=1, vy=1: starts out with all four pixels at the same color (32/32)
		// moves to upper right pixel (64/64)
		// then moves one physical pixel up and right(+1/+1), starts out now with
		// lower left pixel fully bright (0/0) and moves to all four pixel at same
		// color (32/32)

		if (dx < (PS_P_RADIUS >> 1)) // jump to next physical pixel if half of virtual pixel size is reached
		{
			x--; // shift x to next pixel left, will overflow to 255 if 0
			dx = dx + (PS_P_RADIUS >> 1);
		}
		else // if jump has ocurred
		{
			dx = dx - (PS_P_RADIUS >> 1); // adjust dx so pixel fades
		}

		if (dy < (PS_P_RADIUS >> 1)) // jump to next physical pixel if half of virtual pixel size is reached
		{
			y--; // shift y to next pixel down, will overflow to 255 if 0
			dy = dy + (PS_P_RADIUS >> 1);
		}
		else
		{
			dy = dy - (PS_P_RADIUS >> 1);
		}

		if (wrapX)
		{ // wrap it to the other side if required
			if (x < 0)
			{ // left half of particle render is out of frame, wrap it
				x = cols - 1;
			}
		}
		if (wrapY)
		{ // wrap it to the other side if required
			if (y < 0)
			{ // left half of particle render is out of frame, wrap it
				y = rows - 1;
			}
		}

		// calculate brightness values for all four pixels representing a particle using linear interpolation,
		// add color to the LEDs.
		// intensity is a scaling value from 0-255 (0-100%)

		// bottom left
		if (x < cols && y < rows)
		{
			// calculate the intensity with linear interpolation
			intensity = ((uint32_t)((PS_P_RADIUS)-dx) * ((PS_P_RADIUS)-dy) * (uint32_t)brightess) >> PS_P_SURFACE; // divide by PS_P_SURFACE to distribute the energy
			// scale the particle base color by the intensity and add it to the pixel
			SEGMENT.addPixelColorXY(x, rows - y - 1, baseRGB.scale8(intensity), fastcoloradd);
		}
		// bottom right;
		x++;
		if (wrapX)
		{ // wrap it to the other side if required
			if (x >= cols)
				x = x % cols; // in case the right half of particle render is out of frame, wrap it (note: on microcontrollers with hardware division, the if statement is not really needed)
		}
		if (x < cols && y < rows)
		{
			intensity = ((uint32_t)dx * ((PS_P_RADIUS)-dy) * (uint32_t)brightess) >> PS_P_SURFACE; // divide by PS_P_SURFACE to distribute the energy
			SEGMENT.addPixelColorXY(x, rows - y - 1, baseRGB.scale8(intensity), fastcoloradd);
		}
		// top right
		y++;
		if (wrapY)
		{ // wrap it to the other side if required
			if (y >= rows)
				y = y % rows; // in case the right half of particle render is out of frame, wrap it (note: on microcontrollers with hardware division, the if statement is not really needed)
		}
		if (x < cols && y < rows)
		{
			intensity = ((uint32_t)dx * dy * (uint32_t)brightess) >> PS_P_SURFACE; // divide by PS_P_SURFACE to distribute the energy
			SEGMENT.addPixelColorXY(x, rows - y - 1, baseRGB.scale8(intensity), fastcoloradd);
		}
		// top left
		x--;
		if (wrapX)
		{ // wrap it to the other side if required
			if (x < 0)
			{ // left half of particle render is out of frame, wrap it
				x = cols - 1;
			}
		}
		if (x < cols && y < rows)
		{
			intensity = ((uint32_t)((PS_P_RADIUS)-dx) * dy * (uint32_t)brightess) >> PS_P_SURFACE; // divide by PS_P_SURFACE to distribute the energy
			SEGMENT.addPixelColorXY(x, rows - y - 1, baseRGB.scale8(intensity), fastcoloradd);
		}
	}
}

// update & move particle using simple particles, wraps around left/right if wrapX is true, wrap around up/down if wrapY is true
void FireParticle_update(PSparticle *part, bool wrapX = false, bool wrapY = false)
{
	// Matrix dimension
	const uint16_t cols = strip.isMatrix ? SEGMENT.virtualWidth() : 1;
	const uint16_t rows = strip.isMatrix ? SEGMENT.virtualHeight() : SEGMENT.virtualLength();

	// particle box dimensions
	const uint16_t PS_MAX_X(cols * PS_P_RADIUS - 1);
	const uint16_t PS_MAX_Y(rows * PS_P_RADIUS - 1);

	if (part->ttl > 0)
	{
		// age
		part->ttl--;

		// apply velocity
		part->x = part->x + (int16_t)part->vx;
		part->y = part->y + (int16_t)part->vy + (part->ttl >> 4); // younger particles move faster upward as they are hotter, used for fire //TODO: need to make this optional?

		// check if particle is out of bounds, wrap around to other side if wrapping is enabled
		// x-direction
		if ((part->x < 0) || (part->x > PS_MAX_X))
		{
			if (wrapX)
			{
				part->x = part->x % (PS_MAX_X + 1);
				if (part->x < 0)
					part->x = PS_MAX_X - part->x;
			}
			else
			{
				part->ttl = 0; // todo: for round flame display, particles need to go modulo
			}
		}

		// y-direction
		if ((part->y < -(PS_P_RADIUS << 4)) || (part->y > PS_MAX_Y))
		{ // position up to 8 pixels the matrix is allowed, used in fire for wider flames
			if (wrapY)
			{
				part->y = part->y % (PS_MAX_Y + 1);
				if (part->y < 0)
					part->y = PS_MAX_Y - part->y;
			}
			else
			{
				part->ttl = 0; // todo: for round flame display, particles need to go modulo
			}
		}
	}
}

// render simple particles to the LED buffer using heat to color
// each particle adds heat according to its 'age' (ttl) which is then rendered to a fire color in the 'add heat' function
void ParticleSys_renderParticleFire(PSparticle *particles, uint16_t numParticles, bool wrapX)
{

	const uint16_t cols = strip.isMatrix ? SEGMENT.virtualWidth() : 1;
	const uint16_t rows = strip.isMatrix ? SEGMENT.virtualHeight() : SEGMENT.virtualLength();

	// particle box dimensions
	const uint16_t PS_MAX_X(cols * PS_P_RADIUS - 1);
	const uint16_t PS_MAX_Y(rows * PS_P_RADIUS - 1);

	int16_t x, y;
	uint8_t dx, dy;
	uint32_t tempVal;
	uint16_t i;

	// go over particles and update matrix cells on the way
	for (i = 0; i < numParticles; i++)
	{
		if (particles[i].ttl == 0)
		{
			continue;
		}

		// simple particles do not have 'out of bound' parameter, need to check if particle is within matrix boundaries
		dx = (uint8_t)((uint16_t)particles[i].x % (uint16_t)PS_P_RADIUS);
		dy = (uint8_t)((uint16_t)particles[i].y % (uint16_t)PS_P_RADIUS);

		x = (uint8_t)((uint16_t)particles[i].x / (uint16_t)PS_P_RADIUS); // compiler should optimize to bit shift
		y = (uint8_t)((uint16_t)particles[i].y / (uint16_t)PS_P_RADIUS);

		// for x=1, y=1: starts out with all four pixels at the same color (32/32)
		// moves to upper right pixel (64/64)
		// then moves one physical pixel up and right(+1/+1), starts out now with
		// lower left pixel fully bright (0/0) and moves to all four pixel at same
		// color (32/32)

		if (dx < (PS_P_RADIUS >> 1)) // jump to next physical pixel if half of virtual pixel size is reached
		{
			x--;						  // shift left
			dx = dx + (PS_P_RADIUS >> 1); // add half a radius
		}
		else // if jump has ocurred, fade pixel
		{
			// adjust dx so pixel fades
			dx = dx - (PS_P_RADIUS >> 1);
		}

		if (dy < (PS_P_RADIUS >> 1)) // jump to next physical pixel if half of virtual pixel size is reached
		{
			y--; // shift row
			dy = dy + (PS_P_RADIUS >> 1);
		}
		else
		{
			// adjust dy so pixel fades
			dy = dy - (PS_P_RADIUS >> 1);
		}

		if (wrapX)
		{ // wrap it to the other side if required
			if (x < 0)
			{ // left half of particle render is out of frame, wrap it
				x = cols - 1;
			}
		}

		// calculate brightness values for all four pixels representing a particle using linear interpolation
		// bottom left
		if (x < cols && y < rows)
		{
			tempVal = (((uint32_t)((PS_P_RADIUS)-dx) * ((PS_P_RADIUS)-dy) * (uint32_t)particles[i].ttl) >> PS_P_SURFACE);
			PartMatrix_addHeat(x, y, tempVal);
			PartMatrix_addHeat(x + 1, y, tempVal); // shift particle by 1 pixel to the right and add heat again (makes flame wider without using more particles)
		}
		// bottom right;
		x++;
		if (wrapX)
		{ // wrap it to the other side if required
			if (x >= cols)
				x = x % cols; // in case the right half of particle render is out of frame, wrap it (note: on microcontrollers with hardware division, the if statement is not really needed)
		}
		if (x < cols && y < rows)
		{
			tempVal = (((uint32_t)dx * ((PS_P_RADIUS)-dy) * (uint32_t)particles[i].ttl) >> PS_P_SURFACE);
			PartMatrix_addHeat(x, y, tempVal);
			PartMatrix_addHeat(x + 1, y, tempVal); // shift particle by 1 pixel to the right and add heat again (makes flame wider without using more particles)
		}
		// top right
		y++;
		if (x < cols && y < rows)
		{
			tempVal = (((uint32_t)dx * dy * (uint32_t)particles[i].ttl) >> PS_P_SURFACE); //
			PartMatrix_addHeat(x, y, tempVal);
			PartMatrix_addHeat(x + 1, y, tempVal); // shift particle by 1 pixel to the right and add heat again (makes flame wider without using more particles)
		}
		// top left
		x--;
		if (wrapX)
		{ // wrap it to the other side if required
			if (x < 0)
			{ // left half of particle render is out of frame, wrap it
				x = cols - 1;
			}
		}
		if (x < cols && y < rows)
		{
			tempVal = (((uint32_t)((PS_P_RADIUS)-dx) * dy * (uint32_t)particles[i].ttl) >> PS_P_SURFACE);
			PartMatrix_addHeat(x, y, tempVal);
			PartMatrix_addHeat(x + 1, y, tempVal); // shift particle by 1 pixel to the right and add heat again (makes flame wider without using more particles)
		}
	}
}

// adds 'heat' to red color channel, if it overflows, add it to green, if that overflows add it to blue
void PartMatrix_addHeat(uint8_t col, uint8_t row, uint16_t heat)
{

	const uint16_t rows = strip.isMatrix ? SEGMENT.virtualHeight() : SEGMENT.virtualLength();

	CRGB currentcolor = SEGMENT.getPixelColorXY(col, rows - row - 1); // read current matrix color (flip y axis)
	uint16_t newcolorvalue;
	uint8_t colormode = map(SEGMENT.custom3, 0, 31, 0, 5); // get color mode from slider (3bit value)

	// define how the particle TTL value (which is the heat given to the function) maps to heat, if lower, fire is more red, if higher, fire is brighter as bright flames travel higher and decay faster

	heat = heat << 3; // need to take a larger value to scale ttl value of particle to a good heat value that decays fast enough

	// i=0 is normal red fire, i=1 is green fire, i=2 is blue fire
	uint8_t i = (colormode & 0x07) >> 1;
	i = i % 3;
	int8_t increment = (colormode & 0x01) + 1; // 0 (or 3) means only one single color for the flame, 1 is normal, 2 is alternate color modes
	if (currentcolor[i] < 255)
	{
		newcolorvalue = (uint16_t)currentcolor[i] + heat;  // add heat, check if it overflows, is 16bit value
		newcolorvalue = min(newcolorvalue, (uint16_t)255); // limit to 8bit value again
		// check if there is heat left over
		if (newcolorvalue == 255)
		{										   // there cannot be a leftover if it is not full
			heat = heat - (255 - currentcolor[i]); // heat added is difference from current value to full value, subtract it from the inital heat value so heat is the remaining heat not added yet
			// this cannot produce an underflow since we never add more than the initial heat value
		}
		else
		{
			heat = 0; // no heat left
		}
		currentcolor[i] = (uint8_t)newcolorvalue;
	}

	if (heat > 0) // there is still heat left to be added
	{
		i += increment;
		i = i % 3;

		if (currentcolor[i] < 255)
		{
			newcolorvalue = (uint16_t)currentcolor[i] + heat;  // add heat, check if it overflows
			newcolorvalue = min(newcolorvalue, (uint16_t)255); // limit to 8bit value again
			// check if there is heat left over
			if (newcolorvalue == 255) // there cannot be a leftover if red is not full
			{
				heat = heat - (255 - currentcolor[i]); // heat added is difference from current red value to full red value, subtract it from the inital heat value so heat is the remaining heat not added yet
				// this cannot produce an underflow since we never add more than the initial heat value
			}
			else
			{
				heat = 0; // no heat left
			}
			currentcolor[i] = (uint8_t)newcolorvalue;
		}
	}
	if (heat > 0) // there is still heat left to be added
	{
		i += increment;
		i = i % 3;
		if (currentcolor[i] < 255)
		{
			newcolorvalue = currentcolor[i] + heat;			  // add heat, check if it overflows
			newcolorvalue = min(newcolorvalue, (uint16_t)50); // limit so it does not go full white
			currentcolor[i] = (uint8_t)newcolorvalue;
		}
	}

	SEGMENT.setPixelColorXY(col, rows - row - 1, currentcolor);
}

/*detect collisions in an array of particles and handle them*/
void detectCollisions(PSparticle* particles, uint16_t numparticles, uint8_t hardness)
{
	// detect and handle collisions
	uint16_t i,j;
	int16_t startparticle = 0;
	int16_t endparticle = numparticles >> 1; // do half the particles

	if (SEGMENT.call % 2 == 0)
	{ // every second frame, do other half of particles (helps to speed things up as not all collisions are handled each frame which is overkill)
		startparticle = endparticle;
		endparticle = numparticles;
	}
	
	for (i = startparticle; i < endparticle; i++)
	{
		// go though all 'higher number' particles and see if any of those are in close proximity
		// if they are, make them collide
		if (particles[i].ttl > 0) // if particle is alive
		{
			int32_t dx, dy; // distance to other particles
			for (j = i + 1; j < numparticles; j++)
			{							  // check against higher number particles
				if (particles[j].ttl > 0) // if target particle is alive
				{
					dx = particles[i].x - particles[j].x;
					if ((dx < (PS_P_HARDRADIUS)) && (dx > (-PS_P_HARDRADIUS))) //check x direction, if close, check y direction
					{
						dy = particles[i].y - particles[j].y;
						if ((dx < (PS_P_HARDRADIUS)) && (dx > (-PS_P_HARDRADIUS)) && (dy < (PS_P_HARDRADIUS)) && (dy > (-PS_P_HARDRADIUS)))
						{ // particles are close
							handleCollision(&particles[i], &particles[j], hardness);
						}
					}
				}
			}
		}
	}
}
// handle a collision if close proximity is detected, i.e. dx and/or dy smaller than 2*PS_P_RADIUS
// takes two pointers to the particles to collide and the particle hardness (softer means more energy lost in collision)
void handleCollision(PSparticle *particle1, PSparticle *particle2, const uint8_t hardness)
{

	int16_t dx = particle2->x - particle1->x;
	int16_t dy = particle2->y - particle1->y;
	int32_t distanceSquared = dx * dx + dy * dy;

	// Calculate relative velocity
	int16_t relativeVx = (int16_t)particle2->vx - (int16_t)particle1->vx;
	int16_t relativeVy = (int16_t)particle2->vy - (int16_t)particle1->vy;

	if (distanceSquared == 0) // add distance in case particles exactly meet at center, prevents dotProduct=0 (this can only happen if they move towards each other)
	{
		// Adjust positions based on relative velocity direction
	        if (relativeVx < 0) { //if true, particle2 is on the right side
	            particle1->x--;
	            particle2->x++;
	        } else{
	            particle1->x++;
	            particle2->x--;
	        }
	
	        if (relativeVy < 0) {
	            particle1->y--;
	            particle2->y++;
	        } else{
	            particle1->y++;
	            particle2->y--;
	        }
		distanceSquared++;
	}
	// Calculate dot product of relative velocity and relative distance
	int32_t dotProduct = (dx * relativeVx + dy * relativeVy);

	// If particles are moving towards each other
	if (dotProduct < 0)
	{
		const uint8_t bitshift = 14; // bitshift used to avoid floats

		// Calculate new velocities after collision
		int32_t impulse = (((dotProduct << (bitshift)) / (distanceSquared)) * hardness) >> 8;

		particle1->vx += (impulse * dx) >> bitshift;
		particle1->vy += (impulse * dy) >> bitshift;
		particle2->vx -= (impulse * dx) >> bitshift;
		particle2->vy -= (impulse * dy) >> bitshift;

		if (hardness < 150) // if particles are soft, they become 'sticky' i.e. no slow movements
		{
			if (particle1->vx < 2 && particle1->vx > -2)
				particle1->vx = 0;
			if (particle1->vy < 2 && particle1->vy > -2)
				particle1->vy = 0;
			if (particle2->vx < 2 && particle1->vx > -2)
				particle1->vx = 0;
			if (particle2->vy < 2 && particle1->vy > -2)
				particle1->vy = 0;
		}

		// particles have volume, push particles apart if they are too close by moving each particle by a fixed amount away from the other particle
		
		int8_t push;
		
  		if (distanceSquared < (2 * PS_P_HARDRADIUS) * (2 * PS_P_HARDRADIUS)) 
		{
			if (dx < 2 * PS_P_HARDRADIUS && dx > -2 * PS_P_HARDRADIUS)
			{ // distance is too small
				push = 1;
				if (dx < 0) // dx is negative
				{
					push = -push; // invert push direction
				}
				particle1->x -= push;
				particle2->x += push;
			}
			if (dy < 2 * PS_P_HARDRADIUS && dy > -2 * PS_P_HARDRADIUS)
			{ // distance is too small (or negative)
				push = 1;
				if (dy < 0) // dy is negative
				{
					push = -push; // invert push direction
				}
				particle1->y -= push;
				particle2->y += push;
			}
		}
	}

}

// slow down particle by friction, the higher the speed, the higher the friction
void applyFriction(PSparticle *particle, uint8_t coefficient)
{
	if(particle->ttl)
	{
	particle->vx = ((int16_t)particle->vx * (255 - coefficient)) >> 8;
	particle->vy = ((int16_t)particle->vy * (255 - coefficient)) >> 8;
	}
}
