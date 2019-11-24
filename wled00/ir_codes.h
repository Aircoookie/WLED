//Infrared codes

//Add your custom codes here
#define IRCUSTOM_ONOFF  0xA55AEA15 //Pioneer RC-975R "+FAV" button (example)
#define IRCUSTOM_MACRO1 0xFFFFFFFF //placeholder, will never be checked for

//Infrared codes for 24-key remote from http://woodsgood.ca/projects/2015/02/13/rgb-led-strip-controllers-ir-codes/
#define IR24_BRIGHTER  0xF700FF
#define IR24_DARKER    0xF7807F
#define IR24_OFF       0xF740BF 
#define IR24_ON        0xF7C03F
#define IR24_RED       0xF720DF
#define IR24_REDDISH   0xF710EF
#define IR24_ORANGE    0xF730CF
#define IR24_YELLOWISH 0xF708F7
#define IR24_YELLOW    0xF728D7
#define IR24_GREEN     0xF7A05F
#define IR24_GREENISH  0xF7906F
#define IR24_TURQUOISE 0xF7B04F 
#define IR24_CYAN      0xF78877
#define IR24_AQUA      0xF7A857
#define IR24_BLUE      0xF7609F
#define IR24_DEEPBLUE  0xF750AF
#define IR24_PURPLE    0xF7708F
#define IR24_MAGENTA   0xF748B7
#define IR24_PINK      0xF76897
#define IR24_WHITE     0xF7E01F
#define IR24_FLASH     0xF7D02F
#define IR24_STROBE    0xF7F00F 
#define IR24_FADE      0xF7C837 
#define IR24_SMOOTH    0xF7E817 

// 40-key defs for blue remote control 
#define IR44_BPLUS        0xFF3AC5  // 
#define IR44_BMINUS       0xFFBA45  // 
#define IR44_OFF          0xFF827D  // 
#define IR44_ON           0xFF02FD  // 
#define IR44_RED          0xFF1AE5  // 
#define IR44_GREEN        0xFF9A65  // 
#define IR44_BLUE         0xFFA25D  // 
#define IR44_WHITE        0xFF22DD  // natural white
#define IR44_REDDISH      0xFF2AD5  // 
#define IR44_GREENISH     0xFFAA55  // 
#define IR44_DEEPBLUE     0xFF926D  // 
#define IR44_WARMWHITE2   0xFF12ED  // warmest white
#define IR44_ORANGE       0xFF0AF5  // 
#define IR44_TURQUOISE    0xFF8A75  // 
#define IR44_PURPLE       0xFFB24D  // 
#define IR44_WARMWHITE    0xFF32CD  // warm white
#define IR44_YELLOWISH    0xFF38C7  // 
#define IR44_CYAN         0xFFB847  // 
#define IR44_MAGENTA      0xFF7887  // 
#define IR44_COLDWHITE    0xFFF807  // cold white
#define IR44_YELLOW       0xFF18E7  // 
#define IR44_AQUA         0xFF9867  // 
#define IR44_PINK         0xFF58A7  // 
#define IR44_COLDWHITE2   0xFFD827  // coldest white
#define IR44_WPLUS        0xFF28D7  // white chanel bright plus
#define IR44_WMINUS       0xFFA857  // white chanel bright minus
#define IR44_WOFF         0xFF6897  // white chanel on
#define IR44_WON          0xFFE817  // white chanel off
#define IR44_W25          0xFF08F7  // white chanel 25%
#define IR44_W50          0xFF8877  // white chanel 50%
#define IR44_W75          0xFF48B7  // white chanel 75%
#define IR44_W100         0xFFC837  // white chanel 100%
#define IR44_JUMP3        0xFF30CF  // JUMP3
#define IR44_FADE3        0xFFB04F  // FADE3
#define IR44_JUMP7        0xFF708F  // JUMP7
#define IR44_QUICK        0xFFF00F  // QUICK
#define IR44_FADE7        0xFF10EF  // FADE7
#define IR44_FLASH        0xFF906F  // FLASH
#define IR44_AUTO         0xFF50AF  // AUTO
#define IR44_SLOW         0xFFD02F  // SLOW

/* 44-key defs, to be done later
#define IR44_BPlus  0xFF3AC5  // 
#define IR44_BMinus 0xFFBA45  // 
#define IR44_ON     0xFF827D  // 
#define IR44_OFF    0xFF02FD  // 
#define IR44_R      0xFF1AE5  // 
#define IR44_G      0xFF9A65  // 
#define IR44_B      0xFFA25D  // 
#define IR44_W      0xFF22DD  // 
#define IR44_B1     0xFF2AD5  // 
#define IR44_B2     0xFFAA55  // 
#define IR44_B3     0xFF926D  // 
#define IR44_B4     0xFF12ED  // 
#define IR44_B5     0xFF0AF5  // 
#define IR44_B6     0xFF8A75  // 
#define IR44_B7     0xFFB24D  // 
#define IR44_B8     0xFF32CD  // 
#define IR44_B9     0xFF38C7  // 
#define IR44_B10    0xFFB847  // 
#define IR44_B11    0xFF7887  // 
#define IR44_B12    0xFFF807  // 
#define IR44_B13    0xFF18E7  // 
#define IR44_B14    0xFF9867  // 
#define IR44_B15    0xFF58A7  // 
#define IR44_B16    0xFFD827  // 
#define IR44_UPR    0xFF28D7  // 
#define IR44_UPG    0xFFA857  // 
#define IR44_UPB    0xFF6897  // 
#define IR44_QUICK  0xFFE817  // 
#define IR44_DOWNR  0xFF08F7  // 
#define IR44_DOWNG  0xFF8877  // 
#define IR44_DOWNB  0xFF48B7  // 
#define IR44_SLOW   0xFFC837  // 
#define IR44_DIY1   0xFF30CF  // 
#define IR44_DIY2   0xFFB04F  // 
#define IR44_DIY3   0xFF708F  // 
#define IR44_AUTO   0xFFF00F  // 
#define IR44_DIY4   0xFF10EF  // 
#define IR44_DIY5   0xFF906F  // 
#define IR44_DIY6   0xFF50AF  // 
#define IR44_FLASH  0xFFD02F  // 
#define IR44_JUMP3  0xFF20DF  // 
#define IR44_JUMP7  0xFFA05F  // 
#define IR44_FADE3  0xFF609F  // 
#define IR44_FADE7  0xFFE01F  // 
*/

#define COLOR_RED           0xFF0000
#define COLOR_REDDISH       0xFF7800
#define COLOR_ORANGE        0xFFA000
#define COLOR_YELLOWISH     0xFFC800
#define COLOR_YELLOW        0xFFFF00
#define COLOR_GREEN         0x00FF00
#define COLOR_GREENISH      0x00FF78
#define COLOR_TURQUOISE     0x00FFA0
#define COLOR_CYAN          0x00FFDC
#define COLOR_AQUA          0x00C8FF
#define COLOR_BLUE          0x00A0FF
#define COLOR_DEEPBLUE      0x0000FF
#define COLOR_PURPLE        0xAA00FF
#define COLOR_MAGENTA       0xFF00DC
#define COLOR_PINK          0xFF00A0
#define COLOR_WHITE         0xFFFFDC
#define COLOR_WARMWHITE2    0xFFFF9900
#define COLOR_WARMWHITE     0xFF825A00 
#define COLOR_NEUTRALWHITE  0xFF000000
#define COLOR_COLDWHITE     0xFF7F7F7F
#define COLOR_COLDWHITE2    0xFFFFFFFF