
/**
 * This is a stand-alone C test.
 * I don't intend to submit to Github
 * (not in this state, anyway).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NATIVE_TESTING

#define byte unsigned char
#define pgm_read_byte_near (char*)
#define strlen_P strlen

#include "../../../usermods/usermod_v2_rotary_encoder_ui/usermod_v2_rotary_encoder_ui.h"

#define MODE_COUNT  118
#define GRADIENT_PALETTE_COUNT 43 
#define PALETTE_COOUNT 13 + GRADIENT_PALETTE_COUNT

//10 names per line
const char JSON_mode_names[] = R"=====([
"Solid","Blink","Breathe","Wipe","Wipe Random","Random Colors","Sweep","Dynamic","Colorloop","Rainbow",
"Scan","Scan Dual","Fade","Theater","Theater Rainbow","Running","Saw","Twinkle","Dissolve","Dissolve Rnd",
"Sparkle","Sparkle Dark","Sparkle+","Strobe","Strobe Rainbow","Strobe Mega","Blink Rainbow","Android","Chase","Chase Random",
"Chase Rainbow","Chase Flash","Chase Flash Rnd","Rainbow Runner","Colorful","Traffic Light","Sweep Random","Running 2","Aurora","Stream",
"Scanner","Lighthouse","Fireworks","Rain","Tetrix","Fire Flicker","Gradient","Loading","Police","Police All",
"Two Dots","Two Areas","Circus","Halloween","Tri Chase","Tri Wipe","Tri Fade","Lightning","ICU","Multi Comet",
"Scanner Dual","Stream 2","Oscillate","Pride 2015","Juggle","Palette","Fire 2012","Colorwaves","Bpm","Fill Noise",
"Noise 1","Noise 2","Noise 3","Noise 4","Colortwinkles","Lake","Meteor","Meteor Smooth","Railway","Ripple",
"Twinklefox","Twinklecat","Halloween Eyes","Solid Pattern","Solid Pattern Tri","Spots","Spots Fade","Glitter","Candle","Fireworks Starburst",
"Fireworks 1D","Bouncing Balls","Sinelon","Sinelon Dual","Sinelon Rainbow","Popcorn","Drip","Plasma","Percent","Ripple Rainbow",
"Heartbeat","Pacifica","Candle Multi", "Solid Glitter","Sunrise","Phased","Twinkleup","Noise Pal", "Sine","Phased Noise",
"Flow","Chunchun","Dancing Shadows","Washing Machine","Candy Cane","Blends","TV Simulator","Dynamic Smooth"
])=====";

const char JSON_palette_names[]  = R"=====([
"Default","* Random Cycle","* Color 1","* Colors 1&2","* Color Gradient","* Colors Only","Party","Cloud","Lava","Ocean",
"Forest","Rainbow","Rainbow Bands","Sunset","Rivendell","Breeze","Red & Blue","Yellowout","Analogous","Splash",
"Pastel","Sunset 2","Beech","Vintage","Departure","Landscape","Beach","Sherbet","Hult","Hult 64",
"Drywet","Jul","Grintage","Rewhi","Tertiary","Fire","Icefire","Cyane","Light Pink","Autumn",
"Magenta","Magred","Yelmag","Yelblu","Orange & Teal","Tiamat","April Night","Orangery","C9","Sakura",
"Aurora","Atlantica","C9 2","C9 New","Temperature","Aurora 2"
])=====";


int main(int argc, char **argv) {
    printf("I was here.\n");

    modes_qstrings = re_findModeStrings(JSON_mode_names, MODE_COUNT);
    palettes_qstrings = re_findModeStrings(JSON_palette_names, PALETTE_COOUNT);
    
    printf("Done.\n");
}

