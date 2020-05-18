#ifndef WLED_CONST_H
#define WLED_CONST_H

/*
 * Readability defines and their associated numerical values + compile-time constants
 */

//Defaults
#define DEFAULT_CLIENT_SSID "Your_Network"
#define DEFAULT_AP_PASS     "wled1234"
#define DEFAULT_OTA_PASS    "wledota"

//Access point behavior
#define AP_BEHAVIOR_BOOT_NO_CONN  0            //Open AP when no connection after boot
#define AP_BEHAVIOR_NO_CONN       1            //Open when no connection (either after boot or if connection is lost)
#define AP_BEHAVIOR_ALWAYS        2            //Always open
#define AP_BEHAVIOR_BUTTON_ONLY   3            //Only when button pressed for 6 sec

//Notifier callMode 
#define NOTIFIER_CALL_MODE_INIT           0    //no updates on init, can be used to disable updates
#define NOTIFIER_CALL_MODE_DIRECT_CHANGE  1
#define NOTIFIER_CALL_MODE_BUTTON         2
#define NOTIFIER_CALL_MODE_NOTIFICATION   3
#define NOTIFIER_CALL_MODE_NIGHTLIGHT     4
#define NOTIFIER_CALL_MODE_NO_NOTIFY      5
#define NOTIFIER_CALL_MODE_FX_CHANGED     6    //no longer used
#define NOTIFIER_CALL_MODE_HUE            7
#define NOTIFIER_CALL_MODE_PRESET_CYCLE   8
#define NOTIFIER_CALL_MODE_BLYNK          9
#define NOTIFIER_CALL_MODE_ALEXA         10

//RGB to RGBW conversion mode
#define RGBW_MODE_MANUAL_ONLY     0            //No automatic white channel calculation. Manual white channel slider
#define RGBW_MODE_AUTO_BRIGHTER   1            //New algorithm. Adds as much white as the darkest RGBW channel
#define RGBW_MODE_AUTO_ACCURATE   2            //New algorithm. Adds as much white as the darkest RGBW channel and subtracts this amount from each RGB channel
#define RGBW_MODE_DUAL            3            //Manual slider + auto calculation. Automatically calculates only if manual slider is set to off (0)  
#define RGBW_MODE_LEGACY          4            //Old floating algorithm. Too slow for realtime and palette support

//realtime modes
#define REALTIME_MODE_INACTIVE    0
#define REALTIME_MODE_GENERIC     1
#define REALTIME_MODE_UDP         2
#define REALTIME_MODE_HYPERION    3
#define REALTIME_MODE_E131        4
#define REALTIME_MODE_ADALIGHT    5
#define REALTIME_MODE_ARTNET      6
#define REALTIME_MODE_TPM2NET     7

//realtime override modes
#define REALTIME_OVERRIDE_NONE    0
#define REALTIME_OVERRIDE_ONCE    1
#define REALTIME_OVERRIDE_ALWAYS  2

//E1.31 DMX modes
#define DMX_MODE_DISABLED         0            //not used
#define DMX_MODE_SINGLE_RGB       1            //all LEDs same RGB color (3 channels)
#define DMX_MODE_SINGLE_DRGB      2            //all LEDs same RGB color and master dimmer (4 channels)
#define DMX_MODE_EFFECT           3            //trigger standalone effects of WLED (11 channels)
#define DMX_MODE_MULTIPLE_RGB     4            //every LED is addressed with its own RGB (ledCount * 3 channels)
#define DMX_MODE_MULTIPLE_DRGB    5            //every LED is addressed with its own RGB and share a master dimmer (ledCount * 3 + 1 channels)

//Light capability byte (unused)
#define TYPE_NONE                 0            //light is not configured
#define TYPE_RESERVED             1            //unused. Might indicate a "virtual" light
#define TYPE_WS2812_RGB           2
#define TYPE_SK6812_RGBW          3
#define TYPE_WS2812_WWA           4            //amber + warm + cold white
#define TYPE_WS2801               5
#define TYPE_ANALOG_1CH           6            //single channel PWM. Uses value of brightest RGBW channel
#define TYPE_ANALOG_2CH           7            //analog WW + CW
#define TYPE_ANALOG_3CH           8            //analog RGB
#define TYPE_ANALOG_4CH           9            //analog RGBW
#define TYPE_ANALOG_5CH          10            //analog RGB + WW + CW
#define TYPE_APA102              11
#define TYPE_LPD8806             12

//Hue error codes
#define HUE_ERROR_INACTIVE        0
#define HUE_ERROR_UNAUTHORIZED    1
#define HUE_ERROR_LIGHTID         3
#define HUE_ERROR_PUSHLINK      101
#define HUE_ERROR_JSON_PARSING  250
#define HUE_ERROR_TIMEOUT       251
#define HUE_ERROR_ACTIVE        255

//Segment option byte bits
#define SEG_OPTION_SELECTED       0
#define SEG_OPTION_REVERSED       1
#define SEG_OPTION_ON             2
#define SEG_OPTION_PAUSED         3            //unused
#define SEG_OPTION_NONUNITY       4            //Indicates that the effect does not use FRAMETIME or needs getPixelColor
#define SEG_OPTION_TRANSITIONAL   7

//EEPROM size
#define EEPSIZE 2560  //Maximum is 4096

#define NTP_PACKET_SIZE 48

// maximum number of LEDs - MAX_LEDS is coming from the JSON response getting too big, MAX_LEDS_DMA will become a timing issue
#define MAX_LEDS 1500
#define MAX_LEDS_DMA 500

// string temp buffer (now stored in stack locally)
#define OMAX 2048

#define E131_MAX_UNIVERSE_COUNT 9

#define ABL_MILLIAMPS_DEFAULT 850; // auto lower brightness to stay close to milliampere limit

#endif
