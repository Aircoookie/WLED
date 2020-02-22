#ifndef wled_const_h
#define wled_const_h

//Access point behavior
#define AP_BEHAVIOR_BOOT_NO_CONN  0            //Open AP when no connection after boot
#define AP_BEHAVIOR_NO_CONN       1            //Open when no connection (either after boot or if connection is lost)
#define AP_BEHAVIOR_ALWAYS        2            //Always open
#define AP_BEHAVIOR_BUTTON_ONLY   3            //Only when button pressed for 6 sec

//Notifier callMode 
#define NOTIFIER_CALL_MODE_INIT           0    // no updates on init, can be used to disable updates
#define NOTIFIER_CALL_MODE_DIRECT_CHANGE  1
#define NOTIFIER_CALL_MODE_BUTTON         2
#define NOTIFIER_CALL_MODE_NOTIFICATION   3
#define NOTIFIER_CALL_MODE_NIGHTLIGHT     4
#define NOTIFIER_CALL_MODE_NO_NOTIFY      5
#define NOTIFIER_CALL_MODE_FX_CHANGED     6
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

//E1.31 DMX modes
#define  DMX_MODE_DISABLED        0             //not used
#define  DMX_MODE_SINGLE_RGB      1             //all LEDs same RGB color (3 channels)
#define  DMX_MODE_SINGLE_DRGB     2             //all LEDs same RGB color and master dimmer (4 channels)
#define  DMX_MODE_EFFECT          3             //trigger standalone effects of WLED (11 channels)
#define  DMX_MODE_MULTIPLE_RGB    4             //every LED is addressed with its own RGB (ledCount * 3 channels)
#define  DMX_MODE_MULTIPLE_DRGB   5             //every LED is addressed with its own RGB and share a master dimmer (ledCount * 3 + 1 channels)

#endif
