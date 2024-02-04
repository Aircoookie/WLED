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
void Emitter_Flame_emit(PSpointsource *emitter, PSparticle *part) {
	part->x = emitter->source.x + random8(emitter->var)	- (emitter->var >> 1);
	part->y = emitter->source.y + random8(emitter->var)	- (emitter->var >> 1);
	part->vx = emitter->vx + random8(emitter->var)	- (emitter->var >> 1);
	part->vy = emitter->vy + random8(emitter->var)	- (emitter->var >> 1);
	part->ttl = (uint8_t)((rand() % (emitter->maxLife - emitter->minLife)) + emitter->minLife + emitter->source.ttl); //flame intensity dies down with emitter TTL
	part->hue = emitter->source.hue;
}

//fountain style emitter
void Emitter_Fountain_emit(PSpointsource *emitter, PSparticle *part) {
	part->x = emitter->source.x + random8(emitter->var)	- (emitter->var >> 1);
	part->y = emitter->source.y + random8(emitter->var)	- (emitter->var >> 1);
	part->vx = emitter->vx + random8(emitter->var)	- (emitter->var >> 1);
	part->vy = emitter->vy + random8(emitter->var)	- (emitter->var >> 1);
	part->ttl = (rand() % (emitter->maxLife - emitter->minLife)) + emitter->minLife;
	part->hue = emitter->source.hue;
}

//TODO: could solve all update functions in a single function with parameters and handle gravity acceleration in a separte function (uses more cpu time but that is not a huge issue)

void Particle_Move_update(PSparticle *part) //particle moves, decays and dies (age or out of matrix)
{
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
		part->x += (int16_t) part->vx;
		part->y += (int16_t) part->vy;

		//check if particle is out of bounds 
		if ((part->y <= 0) || (part->y >= PS_MAX_Y)) {
			part->ttl = 0;
		}
		if ((part->x <= 0) || (part->x >= PS_MAX_X)) {
			part->ttl = 0;
		}
		if (part->vx == 0 && part->vy == 0) {
			part->ttl = 0;
		}
	}
}


void Particle_Bounce_update(PSparticle *part, const uint8_t hardness) //bounces a particle on the matrix edges, if surface 'hardness' is <255 some energy will be lost in collision (127 means 50% lost)
{
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
		int16_t newX, newY;
		
		//apply velocity
		newX = part->x + (int16_t) part->vx;
		newY = part->y + (int16_t) part->vy;
			
		if ((newX <= 0) || (newX >= PS_MAX_X)) { //reached an edge
			part->vx = -part->vx; //invert speed
			part->vx = (((int16_t)part->vx)*(int16_t)hardness)>>8; //reduce speed as energy is lost on non-hard surface				
		}

		if ((newY <= 0) || (newY >= PS_MAX_Y)) { //reached an edge							
			part->vy = -part->vy; //invert speed
			part->vy = (((int16_t)part->vy)*(int16_t)hardness)>>8; //reduce speed as energy is lost on non-hard surface					
		}
	
		newX = max(newX, (int16_t)0); //limit to positive
		newY = max(newY, (int16_t)0);
		part->x = min(newX, (int16_t)PS_MAX_X); //limit to matrix boundaries
		part->y = min(newY, (int16_t)PS_MAX_Y);
	}
}



void Particle_Gravity_update(PSparticle *part, bool wrapX, bool bounceX, bool bounceY, const uint8_t hardness) //particle moves, decays and dies (age or out of matrix), if wrapX is set, pixels leaving in x direction reappear on other side, hardness is surface hardness for bouncing (127 means 50% speed lost each bounce)
{

	//Matrix dimension
    const uint16_t cols = strip.isMatrix ? SEGMENT.virtualWidth() : 1;
    const uint16_t rows = strip.isMatrix ? SEGMENT.virtualHeight() : SEGMENT.virtualLength();

    //particle box dimensions
    const uint16_t PS_MAX_X (cols*PS_P_RADIUS-1); 
    const uint16_t PS_MAX_Y (rows*PS_P_RADIUS-1); 

	if (part->ttl > 0) {
		//age
		part->ttl--;

		//check if particle is out of bounds or died
		if ((part->y < -PS_P_RADIUS) || (part->y >= PS_MAX_Y << 1)) { //if it moves more than 1 pixel below y=0, it will not come back. also remove particles that too far above
			part->ttl = 0;
			return; //particle died, we are done
		}
		if(wrapX==false){	
			if ((part->x < -PS_MAX_X) || (part->x >= PS_MAX_X << 1)) { //left and right: keep it alive as long as its not too far out (if adding more effects like wind, it may come back)
				part->ttl = 0;
				return; //particle died, we are done
			}
		}

		//apply acceleration (gravity) every other frame, doing it every frame is too strong
		if(SEGMENT.call % 2 == 0)
		{
			if (part->vy > -MAXGRAVITYSPEED)
				part->vy = part->vy - 1;
		}		

		//apply velocity
		int16_t newX, newY;
		
		newX = part->x + (int16_t) part->vx;
		newY = part->y + (int16_t) part->vy;

		part->outofbounds = 0; 
		//check if particle is outside of displayable matrix

		//x direction, handle wraparound (will overrule bounce x) and bounceX
		if(wrapX)
		{
			newX = newX % (PS_MAX_X+1);
			if (newX < 0) 
				newX = PS_MAX_X-newX;		    
		}
		else {			
			if (newX < 0 || newX > PS_MAX_X){ //reached an edge 
				if(bounceX){
					part->vx = -part->vx; //invert speed
					part->vx = (((int16_t)part->vx)*(int16_t)hardness)>>8; //reduce speed as energy is lost on non-hard surface	
					newX = max(newX, (int16_t)0); //limit to positive			
					newX = min(newX, (int16_t)PS_MAX_X); //limit to matrix boundaries			
				}
				else //not bouncing and out of matrix
					part->outofbounds = 1;						
			}
		}
		
		part->x = newX; //set new position

		//y direction, handle bounceY (bounces at ground only)
		if(newY < 0){// || newY > PS_MAX_Y)	{ //reached an edge
			if(bounceY){
				part->vy = -part->vy; //invert speed
				part->vy = (((int16_t)part->vy)*(int16_t)hardness)>>8; //reduce speed as energy is lost on non-hard surface
				part->y += (int16_t) part->vy;//move particle back to within boundaries so it does not disappear for one frame
				newY = max(newY, (int16_t)0); //limit to positive			
				//newY = min(newY, (int16_t)PS_MAX_Y); //limit to matrix boundaries
			}
			else //not bouncing and out of matrix
				part->outofbounds = 1;
		} 

		part->y = newY; //set new position						
	}
}

//render particles to the LED buffer (uses palette to render the 8bit particle color value)
//if wrap is set, particles half out of bounds are rendered to the other side of the matrix
//saturation is color saturation, if not set to 255, hsv instead of palette is used (palette does not support saturation)
void ParticleSys_render(PSparticle *particles, uint16_t numParticles, uint8_t saturation, bool wrapX, bool wrapY) {
	
    const uint16_t cols = strip.isMatrix ? SEGMENT.virtualWidth() : 1; 
    const uint16_t rows = strip.isMatrix ? SEGMENT.virtualHeight() : SEGMENT.virtualLength();

    //particle box dimensions
    const uint16_t PS_MAX_X (cols*PS_P_RADIUS-1); 
    const uint16_t PS_MAX_Y (rows*PS_P_RADIUS-1); 
    
    int16_t x, y;
	uint8_t dx, dy;
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
		
		if(saturation<255){
			CHSV baseHSV = rgb2hsv_approximate(ColorFromPalette(SEGPALETTE, particles[i].hue , 255, LINEARBLEND));
			baseHSV.s = saturation;
			baseRGB = (CRGB)baseHSV;		
		}
		else
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

		if(wrapX){ //wrap it to the other side if required
			if(x<0){  //left half of particle render is out of frame, wrap it
				x = cols-1;
			}
		}
		if(wrapY){ //wrap it to the other side if required
			if(y<0){  //left half of particle render is out of frame, wrap it
				y = rows-1;
			}
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
		if(wrapX){ //wrap it to the other side if required
			if(x>=cols)
				x = x % cols; //in case the right half of particle render is out of frame, wrap it (note: on microcontrollers with hardware division, the if statement is not really needed)			
		}
		if (x < cols && y < rows) {
			intensity = ((uint32_t) dx * ((PS_P_RADIUS) - dy)* (uint32_t) brightess) >> PS_P_SURFACE; //divide by PS_P_SURFACE to distribute the energy
			SEGMENT.addPixelColorXY(x, rows-y-1, baseRGB.scale8(intensity));
		}
		//top right
		y++;
		if(wrapY){ //wrap it to the other side if required
			if(y>=rows)
				y = y % rows; //in case the right half of particle render is out of frame, wrap it (note: on microcontrollers with hardware division, the if statement is not really needed)			
		}
		if (x < cols && y < rows) {
			intensity = ((uint32_t) dx * dy * (uint32_t) brightess)	>> PS_P_SURFACE; //divide by PS_P_SURFACE to distribute the energy
			SEGMENT.addPixelColorXY(x, rows-y-1, baseRGB.scale8(intensity));
		}
		//top left
		x--;
		if(wrapX){ //wrap it to the other side if required
			if(x<0){  //left half of particle render is out of frame, wrap it
				x=cols-1;
			}
		}
		if (x < cols && y < rows) {
			intensity = ((uint32_t) ((PS_P_RADIUS) - dx) * dy* (uint32_t) brightess) >> PS_P_SURFACE;//divide by PS_P_SURFACE to distribute the energy
			SEGMENT.addPixelColorXY(x, rows-y-1, baseRGB.scale8(intensity));
		}
	}
}


//update & move particle using simple particles, wraps around left/right if wrapX is true, wrap around up/down if wrapY is true
void FireParticle_update(PSparticle *part, bool wrapX=false, bool wrapY=false) { 
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
void ParticleSys_renderParticleFire(PSparticle *particles, uint16_t numParticles, bool wrapX) {
	
	const uint16_t cols = strip.isMatrix ? SEGMENT.virtualWidth() : 1; 
    const uint16_t rows = strip.isMatrix ? SEGMENT.virtualHeight() : SEGMENT.virtualLength();

    //particle box dimensions
    const uint16_t PS_MAX_X (cols*PS_P_RADIUS-1); 
    const uint16_t PS_MAX_Y (rows*PS_P_RADIUS-1); 
	
	int16_t x, y;
	uint8_t dx, dy;
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
			x--; //shift left
			dx = dx + (PS_P_RADIUS >> 1); //add half a radius
		} else //if jump has ocurred, fade pixel 
		{
			//adjust dx so pixel fades 
			dx = dx - (PS_P_RADIUS >> 1);
		}

		if (dy < (PS_P_RADIUS >> 1)) //jump to next physical pixel if half of virtual pixel size is reached
				{
			y--; //shift row
			dy = dy + (PS_P_RADIUS >> 1);
		} else {
			//adjust dy so pixel fades 
			dy = dy - (PS_P_RADIUS >> 1);
		}

		if(wrapX){ //wrap it to the other side if required
			if(x<0){  //left half of particle render is out of frame, wrap it
				x=cols-1;
			}
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
		if(wrapX){ //wrap it to the other side if required
			if(x>=cols)
				x = x % cols; //in case the right half of particle render is out of frame, wrap it (note: on microcontrollers with hardware division, the if statement is not really needed)			
		}
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
		if(wrapX){ //wrap it to the other side if required
			if(x<0){  //left half of particle render is out of frame, wrap it
				x=cols-1;
			}
		}
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
	uint8_t colormode = map(SEGMENT.custom3, 0, 31, 0, 5); //get color mode from slider (3bit value)

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

//handle a collision if close proximity is detected, i.e. dx and/or dy smaller than 2*PS_P_RADIUS
//takes two pointers to the particles to collide and the particle hardness (softer means more energy lost in collision)
void handleCollision(PSparticle *particle1, PSparticle *particle2, const uint8_t hardness) {

    int16_t dx = particle2->x - particle1->x;
    int16_t dy = particle2->y - particle1->y;
    int32_t distanceSquared = dx * dx + dy * dy + 1; //+1 so it is never zero to avoid division by zero below
	if(distanceSquared == 0) //add 'noise' in case particles exactly meet at center, prevents dotProduct=0 (this can only happen if they move towards each other)
    { 
        dx++; //
        distanceSquared++;
    }
   
	// Calculate relative velocity
	int16_t relativeVx = (int16_t)particle2->vx - (int16_t)particle1->vx;
	int16_t relativeVy = (int16_t)particle2->vy - (int16_t)particle1->vy;

	// Calculate dot product of relative velocity and relative distance
	int32_t dotProduct = (dx * relativeVx + dy * relativeVy);
	
	// If particles are moving towards each other
	if (dotProduct < 0) {
		const uint8_t bitshift = 14; //bitshift used to avoid floats        	

		// Calculate new velocities after collision
		int32_t impulse = (((dotProduct<<(bitshift)) / (distanceSquared))*hardness)>>8;

		particle1->vx += (impulse * dx)>>bitshift;
		particle1->vy += (impulse * dy)>>bitshift;
		particle2->vx -= (impulse * dx)>>bitshift;
		particle2->vy -= (impulse * dy)>>bitshift;

		if(hardness<150) //if particles are soft, they become 'sticky' i.e. no slow movements
		{
			if(particle1->vx<2&&particle1->vx>-2)
				particle1->vx=0;		
			if(particle1->vy<2&&particle1->vy>-2)
				particle1->vy=0;
			if(particle2->vx<2&&particle1->vx>-2)
				particle1->vx=0;		
			if(particle2->vy<2&&particle1->vy>-2)
				particle1->vy=0;
		}
		
		//particles have volume, push particles apart if they are too close by moving each particle by a fixed amount away from the other particle
		//move each particle by half of the amount they are overlapping, assumes square particles
		
		if(dx < 2*PS_P_HARDRADIUS && dx > -2*PS_P_HARDRADIUS){ //distance is too small 
			int8_t push=1;
			if(dx<0) //dx is negative
			{
				push=-push; //invert push direction
			}
				particle1->x -= push;
				particle2->x += push;	
		}
		if(dy < 2*PS_P_HARDRADIUS && dy > -2*PS_P_HARDRADIUS){ //distance is too small (or negative)
			int8_t push=1;
			if(dy<0) //dy is negative
			{
					push=-push; //invert push direction
			}
		
				particle1->y -= push;
				particle2->y += push;			
		}	
	}
	
	//particles are close, apply friction -> makes them slow down in mid air, is not a good idea to apply here
	//const uint8_t frictioncoefficient=4;
	//applyFriction(particle1, frictioncoefficient);
	//applyFriction(particle2, frictioncoefficient);

}

//slow down particle by friction, the higher the speed, the higher the friction
void applyFriction(PSparticle *particle, uint8_t coefficient) {
    particle->vx = ((int16_t)particle->vx * (255 - coefficient)) >> 8;
    particle->vy = ((int16_t)particle->vy * (255 - coefficient)) >> 8;
}
 