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

//Fountain style emitter for simple particles used for flames (particle TTL depends on source TTL)
void Emitter_Flame_emit(PSpointsource *emitter, PSsimpleparticle *part) {
	part->x = emitter->source.x + (rand() % emitter->var) - ((emitter->var) >> 1);
	part->y = emitter->source.y + (rand() % emitter->var) - ((emitter->var) >> 1);
	part->vx = emitter->vx + (rand() % emitter->var) - ((emitter->var) >> 1);
	part->vy = emitter->vy + (rand() % emitter->var) - ((emitter->var) >> 1);
	part->ttl = (rand() % (emitter->maxLife - emitter->minLife)) + emitter->minLife + emitter->source.ttl; //flame intensity dies down with emitter TTL
	part->hue = emitter->source.hue;
}

//fountain style emitter
void Emitter_Fountain_emit(PSpointsource *emitter, PSparticle *part) {
	part->x = emitter->source.x + (rand() % emitter->var)
			- ((emitter->var) >> 1);
	part->y = emitter->source.y + (rand() % emitter->var)
			- ((emitter->var) >> 1);
	part->vx = emitter->vx + (rand() % emitter->var) - ((emitter->var) >> 1);
	part->vy = emitter->vy + (rand() % emitter->var) - ((emitter->var) >> 1);
	part->ttl = (rand() % (emitter->maxLife - emitter->minLife)) + emitter->minLife;
	part->hue = emitter->source.hue;
//part->isAlive = 1;
}

void Particle_Move_update(PSparticle *part) //particle moves, decays and dies (age or out of matrix)
{
	//Matrix dimension
    const uint16_t cols = strip.isMatrix ? SEGMENT.virtualWidth() : 1;
    const uint16_t rows = strip.isMatrix ? SEGMENT.virtualHeight() : SEGMENT.virtualLength();

    //particle box dimensions
    const uint16_t PS_MAX_X (cols*PS_P_RADIUS-1); 
    const uint16_t PS_MAX_Y (rows*PS_P_RADIUS-1); 

	if (part->ttl) {
		//age
		part->ttl--;

		int16_t newX, newY;
		//check if particle is out of bounds or dead
		if ((part->y == 0) || (part->y >= PS_MAX_Y)) {
			part->ttl = 0;
		}
		if ((part->x == 0) || (part->x >= PS_MAX_X)) {
			part->ttl = 0;
		}
		if (part->vx == 0 && part->vy == 0) {
			part->ttl = 0;
		}
		//apply velocity

		newX = part->x + (int16_t) part->vx;
		newY = part->y + (int16_t) part->vy;
		newX = max((int16_t)newX, (int16_t)0); //limit to positive
		newY = max((int16_t)newY, (int16_t)0);
		part->x = min((int16_t)newX, (int16_t)PS_MAX_X); //limit to matrix boundaries
		part->y = min((int16_t)newY, (int16_t)PS_MAX_Y);
	}
}


void Particle_Bounce_update(PSparticle *part) //bounces a particle on the matrix edges
{
	//Matrix dimension
    const uint16_t cols = strip.isMatrix ? SEGMENT.virtualWidth() : 1;
    const uint16_t rows = strip.isMatrix ? SEGMENT.virtualHeight() : SEGMENT.virtualLength();

    //particle box dimensions
    const uint16_t PS_MAX_X (cols*PS_P_RADIUS-1); 
    const uint16_t PS_MAX_Y (rows*PS_P_RADIUS-1); 

	if (part->ttl) {
		//age
		part->ttl--;

		//apply acceleration
		//  vx = min(vx + ax, PS_MAX_X);
		//  vy = min(vy + ay, PS_MAX_Y);

		//apply velocity
		int16_t newX, newY;
		
		if ((part->vx == 0 && part->vy == 0)) { //stopped moving, make it die
			part->ttl = 0;
		} else {
			if ((part->y == 0) || (part->y >= PS_MAX_Y)) { //reached an edge
				part->vy = -part->vy;
			}
			if ((part->x == 0) || (part->x >= PS_MAX_X)) { //reached an edge
				part->vx = -part->vx;
			}

			newX = part->x + (int16_t) part->vx;
			newY = part->y + (int16_t) part->vy;
			newX = max(newX, (int16_t)0); //limit to positive
			newY = max(newY, (int16_t)0);
			part->x = min(newX, (int16_t)PS_MAX_X); //limit to matrix boundaries
			part->y = min(newY, (int16_t)PS_MAX_Y);
		}
	}
}



void Particle_Gravity_update(PSparticle *part, bool wrapX) //particle moves, decays and dies (age or out of matrix), if wrapX is set, pixels leaving in x direction reappear on other side
{

	//Matrix dimension
    const uint16_t cols = strip.isMatrix ? SEGMENT.virtualWidth() : 1;
    const uint16_t rows = strip.isMatrix ? SEGMENT.virtualHeight() : SEGMENT.virtualLength();

    //particle box dimensions
    const uint16_t PS_MAX_X (cols*PS_P_RADIUS-1); 
    const uint16_t PS_MAX_Y (rows*PS_P_RADIUS-1); 

	int16_t newX, newY;

	if (part->ttl > 0) {
		//age
		part->ttl--;
		//check if particle is out of bounds or died
		if ((part->y < -PS_P_RADIUS) || (part->y >= PS_MAX_Y << 1)) { //if it moves more than 1 pixel below y=0, it will not come back
			part->ttl = 0;
			return; //particle died, we are done
		}
		if(wrapX==false){	
			if ((part->x < -PS_MAX_X) || (part->x >= PS_MAX_X << 1)) { //left and right: keep it alive as long as its not too far out (if adding more effects like wind, it may come back)
				part->ttl = 0;
				return; //particle died, we are done
			}
		}
		if (part->vx == 0 && part->vy == 0) {	
			part->ttl = 0;
			return; //particle died, we are done
		}

		//apply acceleration (gravity)
		if (part->gravitycounterx == 0) //to reduce gravity below 1
				{
			part->vy = part->vy - 1;
			if (part->vy < -MAXGRAVITYSPEED)
				part->vy = part->vy + 1; //revert speed to previous 
			part->gravitycounterx = GRAVITYCOUNTER;
		} else {
			part->gravitycounterx--;
		}
		//apply velocity
		newX = part->x + (int16_t) part->vx;
		newY = part->y + (int16_t) part->vy;

		//check if particle is outside of displayable matrix

		//x direction, handles wraparound
		if(wrapX)
		{
			newX = newX % (PS_MAX_X+1);
			if (newX < 0) 
				newX = PS_MAX_X-newX;			
		}
		else {
			if (newX < 0 || newX > PS_MAX_X)
				part->outofbounds = 1;
			else
				part->outofbounds = 0;
		}
		

		//y direction
		if(newY < 0 || newY > PS_MAX_Y) 
			part->outofbounds = 1;
		else
			part->outofbounds = 0;

		
		//newX = max(newX, (int16_t)-PS_MAX_X); //limit to double matrix size
		//newY = max(newY, (int16_t)-PS_MAX_Y);
		//part->x = min(newX, (int16_t)(PS_MAX_X<<1)); //limit to double the space boundaries
		//part->y = min(newY, (int16_t)(PS_MAX_Y<<1));
		part->x = newX;
		part->y = newY;
	}
}

//render particles to the LED buffer (uses palette to render the 8bit particle color value)
void ParticleSys_render(PSparticle *particles, uint16_t numParticles) {
	
    const uint16_t cols = strip.isMatrix ? SEGMENT.virtualWidth() : 1; 
    const uint16_t rows = strip.isMatrix ? SEGMENT.virtualHeight() : SEGMENT.virtualLength();

    //particle box dimensions
    const uint16_t PS_MAX_X (cols*PS_P_RADIUS-1); 
    const uint16_t PS_MAX_Y (rows*PS_P_RADIUS-1); 
    
    uint8_t x, y, dx, dy;
	uint32_t intensity; //todo: can this be an uint8_t or will it mess things up?
	CRGB baseRGB;    
	uint16_t i;
	uint8_t brightess; //particle brightness, fades if dying


//go over particles and update matrix cells on the way
	for (i = 0; i < numParticles; i++) {
		if (particles[i].ttl == 0 || particles[i].outofbounds) {
			continue;
		}
		//generate RGB values for particle
		brightess = min(particles[i].ttl, (uint16_t)255);
		//baseRGB = CHSV(particles[i].hue, 255, 255);
        baseRGB = ColorFromPalette(SEGPALETTE, particles[i].hue , 255, LINEARBLEND);

		dx = (uint8_t) ((uint16_t) particles[i].x % (uint16_t) PS_P_RADIUS);
		dy = (uint8_t) ((uint16_t) particles[i].y % (uint16_t) PS_P_RADIUS);

		x = (uint8_t) ((uint16_t) particles[i].x / (uint16_t) PS_P_RADIUS);
		y = (uint8_t) ((uint16_t) particles[i].y / (uint16_t) PS_P_RADIUS);

		//for vx=1, vy=1: starts out with all four pixels at the same color (32/32)
		//moves to upper right pixel (64/64)
		//then moves one physical pixel up and right(+1/+1), starts out now with
		//lower left pixel fully bright (0/0) and moves to all four pixel at same
		//color (32/32)

		if (dx < (PS_P_RADIUS >> 1)) //jump to next physical pixel if half of virtual pixel size is reached
				{
			x--; //shift x to next pixel left, will overflow to 255 if 0
			dx = dx + (PS_P_RADIUS >> 1);
		} else //if jump has ocurred, fade pixel out
		{			
			dx = dx - (PS_P_RADIUS >> 1);	//adjust dx so pixel fades 		
		}

		if (dy < (PS_P_RADIUS >> 1)) //jump to next physical pixel if half of virtual pixel size is reached
				{
			y--; //shift y to next pixel down, will overflow to 255 if 0
			dy = dy + (PS_P_RADIUS >> 1);
		} else {
			//adjust dy so pixel fades
			dy = dy - (PS_P_RADIUS >> 1);
		}

        //calculate brightness values for all four pixels representing a particle using linear interpolation,
        //add color to the LEDs.
        //intensity is a scaling value from 0-255 (0-100%)

		//bottom left
		if (x < cols && y < rows) {
			//calculate the intensity with linear interpolation
            intensity = ((uint32_t) ((PS_P_RADIUS) - dx) * ((PS_P_RADIUS) - dy)	* (uint32_t) brightess) >> PS_P_SURFACE; //divide by PS_P_SURFACE to distribute the energy
            //scale the particle base color by the intensity and add it to the pixel
            SEGMENT.addPixelColorXY(x, rows-y-1, baseRGB.scale8(intensity)); 
		}
		//bottom right;
		x++;
		if (x < cols && y < rows) {
			intensity = ((uint32_t) dx * ((PS_P_RADIUS) - dy)* (uint32_t) brightess) >> PS_P_SURFACE; //divide by PS_P_SURFACE to distribute the energy
			SEGMENT.addPixelColorXY(x, rows-y-1, baseRGB.scale8(intensity));
		}
		//top right
		y++;
		if (x < cols && y < rows) {
			intensity = ((uint32_t) dx * dy * (uint32_t) brightess)	>> PS_P_SURFACE; //divide by PS_P_SURFACE to distribute the energy
			SEGMENT.addPixelColorXY(x, rows-y-1, baseRGB.scale8(intensity));
		}
		//top left
		x--;
		if (x < cols && y < rows) {
			intensity = ((uint32_t) ((PS_P_RADIUS) - dx) * dy* (uint32_t) brightess) >> PS_P_SURFACE;//divide by PS_P_SURFACE to distribute the energy
			SEGMENT.addPixelColorXY(x, rows-y-1, baseRGB.scale8(intensity));
		}
	}
}


//update & move particle using simple particles, wraps around left/right if wrapX is true, wrap around up/down if wrapY is true
//todo: need to add parameter for fire? there is fire stuff in here, see comments below
void SimpleParticle_update(PSsimpleparticle *part, bool wrapX, bool wrapY) { 
	//Matrix dimension
    const uint16_t cols = strip.isMatrix ? SEGMENT.virtualWidth() : 1;
    const uint16_t rows = strip.isMatrix ? SEGMENT.virtualHeight() : SEGMENT.virtualLength();

    //particle box dimensions
    const uint16_t PS_MAX_X (cols*PS_P_RADIUS-1); 
    const uint16_t PS_MAX_Y (rows*PS_P_RADIUS-1); 


	if (part->ttl>0) {
		//age
		part->ttl--;
	
		//apply velocity
		part->x = part->x + (int16_t) part->vx;
		part->y = part->y + (int16_t) part->vy + (part->ttl >> 4); //younger particles move faster upward as they are hotter, used for fire //TODO: need to make this optional?
		
		//check if particle is out of bounds, wrap around to other side if wrapping is enabled
		//x-direction
		if ((part->x < 0) || (part->x > PS_MAX_X)) {
			if(wrapX)
			{
				part->x = part->x % (PS_MAX_X+1);
				if (part->x < 0) 
					part->x = PS_MAX_X-part->x;	
			}
			else {
				part->ttl = 0; //todo: for round flame display, particles need to go modulo
			}
		}
		
		//y-direction
		if ((part->y < -(PS_P_RADIUS<<4)) || (part->y > PS_MAX_Y)) { //position up to 8 pixels the matrix is allowed, used in fire for wider flames
			if(wrapY)
			{
				part->y = part->y % (PS_MAX_Y+1);
				if (part->y < 0) 
					part->y = PS_MAX_Y-part->y;	
			}
			else {
				part->ttl = 0; //todo: for round flame display, particles need to go modulo
			}
		}
	}
}

//render simple particles to the LED buffer using heat to color 
//each particle adds heat according to its 'age' (ttl) which is then rendered to a fire color in the 'add heat' function
void ParticleSys_renderParticleFire(PSsimpleparticle *particles, uint16_t numParticles) {
	
	const uint16_t cols = strip.isMatrix ? SEGMENT.virtualWidth() : 1; 
    const uint16_t rows = strip.isMatrix ? SEGMENT.virtualHeight() : SEGMENT.virtualLength();

    //particle box dimensions
    const uint16_t PS_MAX_X (cols*PS_P_RADIUS-1); 
    const uint16_t PS_MAX_Y (rows*PS_P_RADIUS-1); 
	
	uint8_t x, y, dx, dy;
	uint32_t tempVal;
	uint16_t i;

	//go over particles and update matrix cells on the way
	for (i = 0; i < numParticles; i++) {
		if (particles[i].ttl == 0) {
			continue;
		}

		//simple particles do not have 'out of bound' parameter, need to check if particle is within matrix boundaries
		dx = (uint8_t) ((uint16_t) particles[i].x % (uint16_t) PS_P_RADIUS);
		dy = (uint8_t) ((uint16_t) particles[i].y % (uint16_t) PS_P_RADIUS);

		x = (uint8_t) ((uint16_t) particles[i].x / (uint16_t) PS_P_RADIUS); //compiler should optimize to bit shift
		y = (uint8_t) ((uint16_t) particles[i].y / (uint16_t) PS_P_RADIUS);

		//for x=1, y=1: starts out with all four pixels at the same color (32/32)
		//moves to upper right pixel (64/64)
		//then moves one physical pixel up and right(+1/+1), starts out now with
		//lower left pixel fully bright (0/0) and moves to all four pixel at same
		//color (32/32)

		if (dx < (PS_P_RADIUS >> 1)) //jump to next physical pixel if half of virtual pixel size is reached
				{
			x--; //shift left, will overflow to 255 if 0
			dx = dx + (PS_P_RADIUS >> 1);
		} else //if jump has ocurred, fade pixel out
		{
			//adjust dx so pixel fades out
			dx = dx - (PS_P_RADIUS >> 1);
		}

		if (dy < (PS_P_RADIUS >> 1)) //jump to next physical pixel if half of virtual pixel size is reached
				{
			y--; //shift row, will overflow to 255 if 0
			dy = dy + (PS_P_RADIUS >> 1);
		} else {
			//adjust dy so pixel fades out
			dy = dy - (PS_P_RADIUS >> 1);
		}

        //calculate brightness values for all four pixels representing a particle using linear interpolation
		//bottom left
		if (x < cols && y < rows) {
			tempVal = (((uint32_t) ((PS_P_RADIUS) - dx) * ((PS_P_RADIUS) - dy)* (uint32_t) particles[i].ttl) >> PS_P_SURFACE);
			PartMatrix_addHeat(x, y, tempVal);			
			PartMatrix_addHeat(x + 1, y, tempVal); //shift particle by 1 pixel to the right and add heat again (makes flame wider without using more particles)
		}
		//bottom right;
		x++;
		if (x < cols && y <rows) {
			tempVal = (((uint32_t) dx * ((PS_P_RADIUS) - dy)* (uint32_t) particles[i].ttl) >> PS_P_SURFACE);
			PartMatrix_addHeat(x, y, tempVal);			
			PartMatrix_addHeat(x + 1, y, tempVal); //shift particle by 1 pixel to the right and add heat again (makes flame wider without using more particles)
		}
		//top right
		y++;
		if (x < cols && y < rows) {
			tempVal = (((uint32_t) dx * dy * (uint32_t) particles[i].ttl)>> PS_P_SURFACE); //
			PartMatrix_addHeat(x, y, tempVal);			
			PartMatrix_addHeat(x + 1, y, tempVal); //shift particle by 1 pixel to the right and add heat again (makes flame wider without using more particles)
		}
		//top left
		x--;
		if (x < cols && y < rows) {
			tempVal = (((uint32_t) ((PS_P_RADIUS) - dx) * dy* (uint32_t) particles[i].ttl) >> PS_P_SURFACE);
			PartMatrix_addHeat(x, y, tempVal);			
			PartMatrix_addHeat(x + 1, y, tempVal); //shift particle by 1 pixel to the right and add heat again (makes flame wider without using more particles)
		}
	}
}


//adds 'heat' to red color channel, if it overflows, add it to green, if that overflows add it to blue
void PartMatrix_addHeat(uint8_t col, uint8_t row, uint16_t heat) {
	
	const uint16_t rows = strip.isMatrix ? SEGMENT.virtualHeight() : SEGMENT.virtualLength();	

	CRGB currentcolor =  SEGMENT.getPixelColorXY(col, rows-row-1); //read current matrix color (flip y axis)
	uint16_t newcolorvalue;
	uint8_t colormode = SEGMENT.custom2>>5; //get color mode from slider (3bit value)

	//define how the particle TTL value (which is the heat given to the function) maps to heat, if lower, fire is more red, if higher, fire is brighter as bright flames travel higher and decay faster

	heat = heat << 3; //need to take a larger value to scale ttl value of particle to a good heat value that decays fast enough

//i=0 is normal red fire, i=1 is green fire, i=2 is blue fire
	uint8_t i = (colormode & 0x07) >> 1;
	i = i % 3;
	int8_t increment = (colormode & 0x01) + 1; //0 (or 3) means only one single color for the flame, 1 is normal, 2 is alternate color modes
	if (currentcolor[i] < 255) {
		newcolorvalue = (uint16_t)currentcolor[i] + heat; //add heat, check if it overflows, is 16bit value
		newcolorvalue = min(newcolorvalue, (uint16_t)255); //limit to 8bit value again
		//check if there is heat left over
		if (newcolorvalue == 255) { //there cannot be a leftover if it is not full
			heat = heat - (255 - currentcolor[i]); //heat added is difference from current red value to full red value, subtract it from the inital heat value so heat is the remaining heat not added yet
			//this cannot produce an underflow since we never add more than the initial heat value
		} 
		else 		{
			heat = 0; //no heat left
		}
		currentcolor[i] = (uint8_t)newcolorvalue;
	}

	if (heat > 0) //there is still heat left to be added
	{
		i += increment;
		i = i % 3;

		if (currentcolor[i] < 255) {
			newcolorvalue = (uint16_t)currentcolor[i] + heat; //add heat, check if it overflows
			newcolorvalue = min(newcolorvalue,(uint16_t)255); //limit to 8bit value again
			//check if there is heat left over
			if (newcolorvalue == 255) //there cannot be a leftover if red is not full
					{
				heat = heat - (255 - currentcolor[i]); //heat added is difference from current red value to full red value, subtract it from the inital heat value so heat is the remaining heat not added yet
				//this cannot produce an underflow since we never add more than the initial heat value
			} else {
				heat = 0; //no heat left
			}
			currentcolor[i] = (uint8_t)newcolorvalue;
		}
	}
	if (heat > 0) //there is still heat left to be added
	{
		i += increment;
		i = i % 3;
		if (currentcolor[i] < 255) {
			newcolorvalue = currentcolor[i] + heat; //add heat, check if it overflows
			newcolorvalue = min(newcolorvalue, (uint16_t)50); //limit so it does not go full white
			currentcolor[i] = (uint8_t)newcolorvalue;
		}

	}

	
	SEGMENT.setPixelColorXY(col, rows-row-1, currentcolor);

	
}