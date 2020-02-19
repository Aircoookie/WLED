#ifndef wled_const_h
#define wled_const_h

//Access point behavior
#define AP_BEHAVIOR_BOOT_NO_CONN  0            //Open AP when no connection after boot
#define AP_BEHAVIOR_NO_CONN       1            //Open when no connection
#define AP_BEHAVIOR_ALWAYS        2            //Always open
#define AP_BEHAVIOR_BUTTON_ONLY   3            //Only when button pressed for 6 sec

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
