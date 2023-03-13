#ifndef PalettesLedClock_h
#define PalettesLedClock_h

#define GRADIENT_PALETTE_COUNT 58

const byte ib_jul01_gp[] PROGMEM = {
    0, 194,  1,  1,
   94,   0,107, 64,
  132,  57,131, 28,
  255, 148,  0,  0};

// Gradient palette "es_vintage_57_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/vintage/tn/es_vintage_57.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

const byte es_vintage_57_gp[] PROGMEM = {
    0,  89, 36,  8,
   52, 129, 59,  8,
  153, 167,135, 10,
  255,  80, 97, 10};


// Gradient palette "es_vintage_01_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/vintage/tn/es_vintage_01.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 32 bytes of program space.

const byte es_vintage_01_gp[] PROGMEM = {
    0, 128, 57,  0,
   36, 140,  3, 12,
   76, 173,138, 11,
  101, 255,131, 19,
  127, 163, 65,  0,
  152, 177, 72,  2,
  255, 126,  1,  1};


// Gradient palette "es_rivendell_15_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/rivendell/tn/es_rivendell_15.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

const byte es_rivendell_15_gp[] PROGMEM = {
    0,  14, 93, 52,
  101,  43, 83, 40,
  165,  87,106, 47,
  242, 216,223,154,
  255, 223,230,158};


// Gradient palette "rgi_15_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ds/rgi/tn/rgi_15.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 36 bytes of program space.
// Edited to be brighter

const byte rgi_15_gp[] PROGMEM = {
    0,  58, 52,162,
   31, 123, 20, 75,
   63, 255,  4,  7,
   95, 142, 26, 80,
  127,  42, 23,135,
  159,  88, 20,133,
  191, 159, 35, 64,
  223, 174, 41, 96,
  255,  68, 23,130};


// Gradient palette "retro2_16_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ma/retro2/tn/retro2_16.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 8 bytes of program space.

const byte retro2_16_gp[] PROGMEM = {
    0, 188,135,  1,
  255, 133, 53,  0};


// Gradient palette "Analogous_1_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/red/tn/Analogous_1.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

const byte Analogous_1_gp[] PROGMEM = {
    0,   3,  0,255,
   63,  23,  0,255,
  127,  67,  0,255,
  191, 142,  0, 45,
  255, 255,  0,  0};


// Gradient palette "es_pinksplash_08_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/pink_splash/tn/es_pinksplash_08.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

const byte es_pinksplash_08_gp[] PROGMEM = {
    0, 126, 11,255,
  127, 197,  1, 22,
  175, 210,157,172,
  221, 157,  3,112,
  255, 157,  3,112};


// Gradient palette "es_ocean_breeze_036_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/ocean_breeze/tn/es_ocean_breeze_036.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 16 bytes of program space.

const byte es_ocean_breeze_036_gp[] PROGMEM = {
    0,  27,131,152,
   87,  23,158,176,
  153, 173,221,255,
  255,   6,103,116};


// Gradient palette "departure_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/mjf/tn/departure.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 88 bytes of program space.

const byte departure_gp[] PROGMEM = {
    0, 117, 51, 11,
   42,  94, 41, 18,
   64, 119, 63, 14,
   84, 169, 99, 38,
  106, 213,169,119,
  116, 255,255,255,
  138, 135,255,138,
  148,  22,255, 24,
  170,   0,255,  0,
  191,   0,136,  0,
  212,  16,117, 16,
  255,  10,113, 10};


// Gradient palette "es_landscape_64_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/landscape/tn/es_landscape_64.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 36 bytes of program space.

const byte es_landscape_64_gp[] PROGMEM = {
    0,  21, 86, 22,
   54,  15,115,  5,
  127,  79,213,  1,
  128, 126,211, 47,
  130, 188,209,247,
  153, 144,182,205,
  204,  59,117,250,
  255,   1, 37,192};


// Gradient palette "es_landscape_33_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/landscape/tn/es_landscape_33.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

const byte es_landscape_33_gp[] PROGMEM = {
    0,  96, 47, 21,
   38, 161, 55,  1,
   63, 229,144,  1,
   66,  39,142, 74,
  255,  34, 83, 34};


// Gradient palette "rainbowsherbet_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ma/icecream/tn/rainbowsherbet.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

const byte rainbowsherbet_gp[] PROGMEM = {
    0, 255, 33,  4,
   43, 255, 68, 25,
   86, 255,  7, 25,
  127, 255, 82,103,
  170, 255,255,242,
  209,  42,255, 22,
  255,  87,255, 65};


// Gradient palette "gr65_hult_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/hult/tn/gr65_hult.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

const byte gr65_hult_gp[] PROGMEM = {
    0, 247,176,247,
   48, 255,136,255,
   89, 220, 29,226,
  160,   7, 82,178,
  216,   1,124,109,
  255,   1,124,109};


// Gradient palette "gr64_hult_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/hult/tn/gr64_hult.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 32 bytes of program space.

const byte gr64_hult_gp[] PROGMEM = {
    0,  15,143,128,
   66,  11,127,109,
  104,  88,105, 17,
  130, 115,127,  1,
  150,  80, 98,  9,
  201,  24,129,111,
  239,   6,132,109,
  255,   0,133,108};


// Gradient palette "GMT_drywet_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/gmt/tn/GMT_drywet.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

const byte GMT_drywet_gp[] PROGMEM = {
    0, 143, 96, 20,
   42, 213,147, 24,
   84, 103,219, 52,
  127,   3,219,207,
  184,   1, 48,214,
  255,   1, 28,147};


// Gradient palette "ib15_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ing/general/tn/ib15.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

const byte ib15_gp[] PROGMEM = {
    0, 138, 89,212,
   72, 180,111,100,
   89, 208, 85, 33,
  107, 255, 29, 11,
  141, 178, 46, 57,
  255, 113, 55,179};


// Gradient palette "Tertiary_01_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/vermillion/tn/Tertiary_01.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

const byte Tertiary_01_gp[] PROGMEM = {
    0,   0,  1,255,
   63,  19,144,100,
  127,  23,255,  0,
  191, 196,143, 28,
  255, 255,  1,  4};


// Gradient palette "lava_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/neota/elem/tn/lava.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 52 bytes of program space.

const byte lava_gp[] PROGMEM = {
    0, 179,  0,  0,
   42, 175, 17,  1,
   98, 213, 44,  2,
  119, 255, 82,  4,
  136, 255,115,  4,
  157, 255,156,  4,
  176, 255,203,  4,
  195, 255,255,  4,
  212, 255,255, 71,
  255, 255,255,255};


// Gradient palette "fierce_ice_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/neota/elem/tn/fierce-ice.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

const byte fierce_ice_gp[] PROGMEM = {
    0,   0,  9,133,
  119,   0, 38,255,
  149,   3,100,255,
  180,  23,199,255,
  217, 100,235,255,
  255, 255,255,255};


// Gradient palette "Colorfull_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/atmospheric/tn/Colorfull.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 44 bytes of program space.

const byte Colorfull_gp[] PROGMEM = {
    0,  10, 85,  5,
   25,  29,109, 18,
   60,  59,138, 42,
   93,  83, 99, 52,
  106, 110, 66, 64,
  109, 123, 49, 65,
  113, 139, 35, 66,
  116, 192,117, 98,
  124, 255,255,137,
  168, 100,180,155,
  255,  22,121,174};


// Gradient palette "Pink_Purple_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/atmospheric/tn/Pink_Purple.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 44 bytes of program space.

const byte Pink_Purple_gp[] PROGMEM = {
    0,  90,  0,194,
   25,  82,  0,153,
   51, 134, 16,218,
   76,  64, 49,196,
  102, 118,187,240,
  109, 163,215,247,
  114, 217,244,255,
  122, 159,149,221,
  149, 113, 78,188,
  183, 128, 57,155,
  255, 171, 23,139};


// Gradient palette "Sunset_Real_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/atmospheric/tn/Sunset_Real.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

const byte Sunset_Real_gp[] PROGMEM = {
    0, 163,  0,  0,
   22, 179, 22,  0,
   51, 255,104,  0,
   85, 167, 22, 18,
  135, 171,  2,177,
  198,  16,  0,130,
  255,  66, 66,255};


// Gradient palette "Sunset_Yellow_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/atmospheric/tn/Sunset_Yellow.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 44 bytes of program space.

const byte Sunset_Yellow_gp[] PROGMEM = {
    0,   0, 95,204,
   36,  56,130,103,
   87, 153,225, 85,
  100, 199,217, 68,
  107, 255,207, 54,
  115, 247,152, 57,
  120, 239,107, 61,
  128, 247,152, 57,
  180, 255,207, 54,
  223, 255,227, 48,
  255, 255,248, 42};


// Gradient palette "Beech_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/atmospheric/tn/Beech.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 60 bytes of program space.

const byte Beech_gp[] PROGMEM = {
    0, 255,252,214,
   12, 255,252,214,
   22, 255,252,214,
   31, 112,255,205,
   50,  51,246,214,
   71,  17,235,226,
   93,   2,193,199,
  120,   0,156,174,
  139,   0,156,204,
  208, 122,202,255,
  255,   0,106,255};


// Gradient palette "Another_Sunset_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/atmospheric/tn/Another_Sunset.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 32 bytes of program space.

const byte Another_Sunset_gp[] PROGMEM = {
    0, 163, 63,  0,
   68, 255,102,  0,
   73, 239,124,  8,
   97, 220,156, 27,
  124, 203,193, 61,
  178,  79,221,243,
  255,   0,  3,194};





// Gradient palette "es_autumn_19_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/autumn/tn/es_autumn_19.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 52 bytes of program space.

const byte es_autumn_19_gp[] PROGMEM = {
    0, 175, 97,  8,
   51, 166, 35, 28,
   79, 160, 35, 19,
  104, 193,148, 51,
  112, 171,105, 18,
  124, 162,157, 17,
  135, 158, 96, 16,
  144, 181,139, 23,
  169, 170, 32, 14,
  204, 144, 19, 14,
  255, 210, 63, 15};


// Gradient palette "BlacK_Blue_Magenta_White_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/basic/tn/BlacK_Blue_Magenta_White.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

const byte BlacK_Blue_Magenta_White_gp[] PROGMEM = {
    0,   0, 10,153,
   32, 166,  0,255,
   74, 255,107,201,
  127,  42,  0,255,
  170, 255,  0,255,
  212, 255, 55,255,
  255, 255,255,255};


// Gradient palette "BlacK_Magenta_Red_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/basic/tn/BlacK_Magenta_Red.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

const byte BlacK_Magenta_Red_gp[] PROGMEM = {
    0, 112,  0,153,
   63, 145,  9,154,
  127, 255,  0,255,
  191, 255,  0, 45,
  255, 255,  0,  0};


// Gradient palette "BlacK_Red_Magenta_Yellow_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/basic/tn/BlacK_Red_Magenta_Yellow.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

const byte BlacK_Red_Magenta_Yellow_gp[] PROGMEM = {
    0, 168,  0,  0,
   41, 251,132, 35,
   84, 255,  0,  0,
  127, 255,  0, 45,
  170, 255,  0,255,
  212, 255, 55, 45,
  255, 255,255,  0};


// Gradient palette "Blue_Cyan_Yellow_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/basic/tn/Blue_Cyan_Yellow.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

const byte Blue_Cyan_Yellow_gp[] PROGMEM = {
    0,   0,  0,255,
   63,   0, 55,255,
  127,   0,255,255,
  191,  42,255, 45,
  255, 255,255,  0};


//Custom palette by Aircoookie

const byte Orange_Teal_gp[] PROGMEM = {
    0,   0,150, 92,
   55,   0,150, 92,
  200, 255, 72,  0,
  255, 255, 72,  0};

//Custom palette by Aircoookie

const byte Tiamat_gp[] PROGMEM = {
    0,   0, 12,143,
   33,  60, 69,175,
  100,  13,135, 92,
  120,  43,255,193,
  140, 247,  7,249,
  160, 193, 17,208,
  180,  39,255,154,
  200,   4,213,236,
  220,  39,252,135,
  240, 193,213,253,
  255, 255,249,255};
  
//Custom palette by Aircoookie

const byte April_Night_gp[] PROGMEM = {
    0,   0, 13,153,
   10,   0, 16,194,
   25,   5,169,175,
   40,   0, 80,184,
   61,   2, 97,  0,
   76,  45,175, 31,
   91,  25,104, 24,
  112, 158,100,  0,
  127, 249,150,  5,
  143, 157, 92,  1,
  162, 188, 76,  1,
  178, 255, 92,  0,
  193, 179, 71,  0,
  214, 163,  0, 41,
  229, 223, 45, 72,
  244, 138,  0, 41,
  255,  92,  0, 63};

const byte Orangery_gp[] PROGMEM = {
    0, 255, 95, 23,
   30, 255, 82,  0,
   60, 223, 13,  8,
   90, 254,120, 62,
  120, 255,110, 17,
  150, 255, 69,  0,
  180, 194,  3,  0,
  210, 241, 82, 17,
  255, 213, 37,  4};

//inspired by Mark Kriegsman https://gist.github.com/kriegsman/756ea6dcae8e30845b5a
const byte C9_gp[] PROGMEM = {
    0, 184,  4,  0,
   60, 184,  4,  0,
   65, 144, 44,  2,
  125, 144, 44,  2,
  130,   4, 96,  2,
  190,   4, 96,  2,
  195,   7,  7, 88,
  255,   7,  7, 88};

const byte Sakura_gp[] PROGMEM = {
    0, 196, 19, 10,
   65, 255, 69, 45,
  130, 223, 45, 72,
  195, 255, 82,103,
  255, 223, 13, 17};

const byte Aurora_gp[] PROGMEM = {
    0,   0, 99,204,
   54,   0,200, 23,
   92,   0,255,157,
  126,  56,252,255,
  167,   0,245,139,
  200,   0,184,  9,
  255,   0, 91,209};//deep blue

const byte Atlantica_gp[] PROGMEM = {
    0,   0, 46,184,
   50,  32, 96,255,
  100,   0,243, 45,
  150,  24,140,122,
  200,  25,190, 95,
  255,  40,170, 80};//#28AA50

  const byte C9_2_gp[] PROGMEM = {
    0,   6,126,  2,
   45,   6,126,  2,
   45,   4, 30,114,
   90,   4, 30,114,
   90, 255,  5,  0,
  135, 255,  5,  0,
  135, 196, 57,  2,
  180, 196, 57,  2,
  180, 137, 85,  2,
  255, 137, 85,  2};

  //C9, but brighter and with a less purple blue
  const byte C9_new_gp[] PROGMEM = {
    0, 255,  5,  0,
   60, 255,  5,  0,
   60, 196, 57,  2,
  120, 196, 57,  2,
  120,   6,126,  2,
  180,   6,126,  2,
  180,   4, 30,114,
  255,   4, 30,114};
  
// Gradient palette "temperature_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/arendal/tn/temperature.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 144 bytes of program space.  

const byte temperature_gp[] PROGMEM = {
    0,   1, 27,105,
   14,   1, 40,127,
   28,   1, 70,168,
   42,   1, 92,197,
   56,   1,119,221,
   70,   3,130,151,
   84,  23,156,149,
   99,  67,182,112,
  119, 121,201, 52,
  134, 142,203, 11,
  149, 224,223,  1,
  167, 252,187,  2,
  186, 247,147,  1,
  202, 237, 87,  1,
  222, 229, 43,  1,
  255, 230,  0,  0};

  const byte Aurora2_gp[] PROGMEM = {
    0,  17,177, 13,
   64, 121,242,  5,
  128,  25,173,121,
  192, 250, 77,127,
  255, 171,101,221};

  // Gradient palette "bhw1_01_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw1/tn/bhw1_01.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 12 bytes of program space.

const byte retro_clown_gp[] PROGMEM = {
    0, 255,111,  0,
  117, 230, 30, 30,
  255, 115,  0,255};

// Gradient palette "bhw1_04_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw1/tn/bhw1_04.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

const byte candy_gp[] PROGMEM = {
    0, 229,227,  1,
   15, 227,101,  3,
  142, 128,  8,247,
  198,  11, 90,193,
  255,   0,  0,204};

// Gradient palette "bhw1_05_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw1/tn/bhw1_05.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 8 bytes of program space.

const byte toxy_reaf_gp[] PROGMEM = {
    0,   1,221, 53,
  255, 102,  0,255};

// Gradient palette "bhw1_06_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw1/tn/bhw1_06.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 16 bytes of program space.

const byte fairy_reaf_gp[] PROGMEM = {
    0, 184,  1,128,
  160,   1,193,182,
  219, 153,227,190,
  255, 255,255,255};

// Gradient palette "bhw1_14_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw1/tn/bhw1_14.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 36 bytes of program space.

const byte semi_blue_gp[] PROGMEM = {
    0,   0, 64,214,
   65,   0,157,255,
  126,   2, 25,216,
  186,  20,134,255,
  255,   0, 38,189};

// Gradient palette "bhw1_three_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw1/tn/bhw1_three.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 32 bytes of program space.

const byte pink_candy_gp[] PROGMEM = {
    0, 255,255,255,
   45, 201,  5,255,
   98, 227,  1,127,
  131, 254,154,236,
  160, 227,  1,127,
  196, 181, 33,192,
  255, 255,255,255};

// Gradient palette "bhw1_w00t_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw1/tn/bhw1_w00t.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 16 bytes of program space.

const byte red_reaf_gp[] PROGMEM = {
    0,   0, 43,173,
  104,  78,141,240,
  188, 255,  0,  0,
  255, 168,  0,  0};


// Gradient palette "bhw2_23_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw2/tn/bhw2_23.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Red & Flash in SR
// Size: 28 bytes of program space.

const byte aqua_flash_gp[] PROGMEM = {
    0,   0,139,199,
   66,  57,227,233,
   96, 255,255,  8,
  124, 255,255,255,
  153, 255,255,  8,
  188,  57,227,233,
  255,   0,176,235};

// Gradient palette "bhw2_xc_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw2/tn/bhw2_xc.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// YBlue in SR
// Size: 28 bytes of program space.

const byte yelblu_hot_gp[] PROGMEM = {
    0,  56,  0,199,
   58, 122, 74,217,
  158, 190, 23, 14,
  183, 218, 71, 22,
  219, 245,139, 25,
  255, 251,255,  0};

 // Gradient palette "bhw2_45_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw2/tn/bhw2_45.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

const byte lite_light_gp[] PROGMEM = {
    0, 176,176,176,
   26, 222,222,222,
   70,  22,176,254,
  126, 255,229, 82,
  181, 225,144,254,
  227, 222,222,222,
  255, 176,176,176};

// Gradient palette "bhw2_22_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw2/tn/bhw2_22.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Pink Plasma in SR
// Size: 20 bytes of program space.

const byte red_flash_gp[] PROGMEM = {
    0, 148,  0,  0,
   70, 227,  1,  1,
  126, 249,199, 95,
  186, 227,  1,  1,
  255, 153,  0,  0};

// Gradient palette "bhw3_40_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw3/tn/bhw3_40.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 32 bytes of program space.

const byte blink_red_gp[] PROGMEM = {
    0, 168,  0,115,
   43, 130, 77,255,
   76, 151, 53, 74,
  109, 161,  4, 29,
  127, 255, 86,123,
  165, 125, 16,160,
  204,  35, 13,223,
  255, 168,  0,168};

// Gradient palette "bhw3_52_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw3/tn/bhw3_52.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Yellow2Blue in SR
// Size: 28 bytes of program space.

const byte red_shift_gp[] PROGMEM = {
    0, 173,  0,110,
   45, 189,  0, 82,
   99, 199, 35, 41,
  132, 213,128, 10,
  175, 199, 22,  1,
  201, 199,  9,  6,
  255, 184, 89,  0};

// Gradient palette "bhw4_097_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw4/tn/bhw4_097.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Yellow2Red in SR
// Size: 44 bytes of program space.

const byte red_tide_gp[] PROGMEM = {
    0, 247,  5,  0,
   28, 255, 67,  1,
   43, 234, 88, 11,
   58, 234,176, 51,
   84, 229, 28,  1,
  114, 225, 63, 45,
  140, 255,225, 44,
  168, 253, 50, 28,
  196, 244,209, 88,
  216, 255, 28,  1,
  255, 163,  0,  0};

// Gradient palette "bhw4_017_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw4/tn/bhw4_017.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 40 bytes of program space.

const byte candy2_gp[] PROGMEM = {
    0, 254, 42, 77,
   25, 185, 70, 70,
   48, 255, 98, 41,
   73, 224,173,  1,
   89, 216, 52, 19,
  130, 255,169, 71,
  163, 255,114,  6,
  186, 224,173,  1,
  211, 254, 88,115,
  255, 179,  0,  0};

// Single array of defined cpt-city color palettes.
// This will let us programmatically choose one based on
// a number, rather than having to activate each explicitly
// by name every time.
const byte* const gGradientPalettes[] PROGMEM = {
  Sunset_Real_gp,               //13-00 Sunset
  es_rivendell_15_gp,           //14-01 Rivendell
  es_ocean_breeze_036_gp,       //15-02 Breeze
  rgi_15_gp,                    //16-03 Red & Blue
  retro2_16_gp,                 //17-04 Yellowout
  Analogous_1_gp,               //18-05 Analogous
  es_pinksplash_08_gp,          //19-06 Splash
  Sunset_Yellow_gp,             //20-07 Pastel
  Another_Sunset_gp,            //21-08 Sunset2
  Beech_gp,                     //22-09 Beech
  es_vintage_01_gp,             //23-10 Vintage
  departure_gp,                 //24-11 Departure
  es_landscape_64_gp,           //25-12 Landscape
  es_landscape_33_gp,           //26-13 Beach
  rainbowsherbet_gp,            //27-14 Sherbet
  gr65_hult_gp,                 //28-15 Hult
  gr64_hult_gp,                 //29-16 Hult64
  GMT_drywet_gp,                //30-17 Drywet
  ib_jul01_gp,                  //31-18 Jul
  es_vintage_57_gp,             //32-19 Grintage
  ib15_gp,                      //33-20 Rewhi
  Tertiary_01_gp,               //34-21 Tertiary
  lava_gp,                      //35-22 Fire
  fierce_ice_gp,                //36-23 Icefire
  Colorfull_gp,                 //37-24 Cyane
  Pink_Purple_gp,               //38-25 Light Pink
  es_autumn_19_gp,              //39-26 Autumn
  BlacK_Blue_Magenta_White_gp,  //40-27 Magenta
  BlacK_Magenta_Red_gp,         //41-28 Magred
  BlacK_Red_Magenta_Yellow_gp,  //42-29 Yelmag
  Blue_Cyan_Yellow_gp,          //43-30 Yelblu
  Orange_Teal_gp,               //44-31 Orange & Teal
  Tiamat_gp,                    //45-32 Tiamat
  April_Night_gp,               //46-33 April Night
  Orangery_gp,                  //47-34 Orangery
  C9_gp,                        //48-35 C9
  Sakura_gp,                    //49-36 Sakura
  Aurora_gp,                    //50-37 Aurora
  Atlantica_gp,                 //51-38 Atlantica
  C9_2_gp,                      //52-39 C9 2
  C9_new_gp,                    //53-40 C9 New
  temperature_gp,               //54-41 Temperature
  Aurora2_gp,                   //55-42 Aurora 2
  retro_clown_gp,               //56-43 Retro Clown
  candy_gp,                     //57-44 Candy
  toxy_reaf_gp,                 //58-45 Toxy Reaf
  fairy_reaf_gp,                //59-46 Fairy Reaf
  semi_blue_gp,                 //60-47 Semi Blue
  pink_candy_gp,                //61-48 Pink Candy
  red_reaf_gp,                  //62-49 Red Reaf
  aqua_flash_gp,                //63-50 Aqua Flash
  yelblu_hot_gp,                //64-51 Yelblu Hot
  lite_light_gp,                //65-52 Lite Light
  red_flash_gp,                 //66-53 Red Flash
  blink_red_gp,                 //67-54 Blink Red
  red_shift_gp,                 //68-55 Red Shift
  red_tide_gp,                  //69-56 Red Tide
  candy2_gp                     //70-57 Candy2
};

// Palettes ported from FastLED
// These are not included in the above list,
// but are loaded manually from code

#define LC_SERIALIZE_FL_PAL(pal) { byte tcp[72]; memcpy_P(tcp, pal, 72); setPaletteColors(curPalette, tcp); }
#define LC_LOAD_FL_PAL(pal) { byte tcp[72]; memcpy_P(tcp, pal, 72); targetPalette.loadDynamicGradientPalette(tcp); }

const byte flCloudColors_gp[] PROGMEM = {
    0,   0,  0,255,
   17,  52, 52,234,
   34,  88,182,254,
   99,   1, 52,254,
  123,   0,  0,255,
  152,   0,129,209,
  170, 135,206,235,
  187, 135,206,235,
  204, 173,216,230,
  221, 255,255,255,
  238, 173,216,230,
  255, 135,206,235};

const byte flLavaColors_gp[] PROGMEM = {
    0, 219,  0,  0,
   18, 255,  0,  0,
   34, 255,162,  0,
   51, 255,255,255,
   66, 255,162,  0,
   82, 255,  0,  0,
  124, 139,  0,  0,
  164, 255,  0,  0,
  182, 255,165,  0,
  200, 255,255,255,
  219, 255,165,  0,
  237, 255,  0,  0,
  255, 139,  0,  0};

const byte flOceanColors_gp[] PROGMEM = {
    0,   9,  9,211,
   17,   6,  6,193,
   34,  39, 39,236,
   51,   0,  0,173,
   68,   0,  0,194,
   85,   0,  0,205,
  102,  46,139, 87,
  119,   0,128,128,
  136,  95,158,160,
  153,   0,  0,255,
  170,   0,139,139,
  187, 100,149,237,
  204, 127,255,212,
  221,  46,139, 87,
  238,   0,255,255,
  255, 135,206,250};

const byte flForestColors_gp[] PROGMEM = {
    0,   0,153,  0,
   17,   0,138,  0,
   34, 119,180, 14,
   51,   0,100,  0,
   68,   0,128,  0,
   85,  34,139, 34,
  103, 134,187, 27,
  119,   0,128,  0,
  136,  65,200,124,
  153, 102,205,170,
  170,  50,205, 50,
  187, 154,205, 50,
  204, 144,238,144,
  221, 124,252,  0,
  238, 102,205,170,
  255,  34,139, 34};

/// HSV Rainbow
const byte flRainbowColors_gp[] PROGMEM = {
    0, 255,  0,  0,
   17, 235, 47,  0,
   34, 255,128,  0,
   51, 240,180,  0,
   69, 214,214,  0,
   85,  92,230,  0,
  102,   0,255,  0,
  119,   0,213, 42,
  137,   0,219,110,
  153,   0,105,209,
  170,   0,  0,255,
  187,  47,  0,235,
  204, 107,  0,214,
  221, 171,  0,173,
  238, 204,  0,102,
  255, 213,  0, 43};

/// HSV Rainbow colors with alternatating stripes of black
const byte flRainbowStripeColors_gp[] PROGMEM = {
    0, 255,  0,  0,
   33, 214, 41,  0,
   34, 171, 85,  0,
   67, 171,128,  0,
   68, 171,171,  0,
  101,  87,212,  0,
  102,   0,255,  0,
  135,   0,214, 41,
  136,   0,171, 85,
  169,   0, 86,169,
  170,   0,  0,255,
  203,  46,  0,210,
  204,  85,  0,171,
  237, 129,  0,127,
  238, 171,  0, 85,
  255, 255,  0,  0};

/// HSV color ramp: blue purple ping red orange yellow (and back)
/// Basically, everything but the greens, which tend to make
/// people's skin look unhealthy.  This palette is good for
/// lighting at a club or party, where it'll be shining on people.
const byte flPartyColors_gp[] PROGMEM = {
    0, 120,  0,240,
   18, 189,  0,176,
   34, 199,  0, 83,
   51, 229,  0, 27,
   68, 232, 23,  0,
   85, 219, 84,  0,
  102, 230,161,  0,
  118, 219,219,  0,
  136, 219,110,  0,
  153, 221, 34,  0,
  170, 242,  0, 14,
  187, 194,  0, 62,
  204, 179,  0,140,
  221, 110,  0,189,
  238,  47,  0,208,
  255,   0,  7,249};

#endif
