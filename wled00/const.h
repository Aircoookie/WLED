#ifndef WLED_CONST_H
#define WLED_CONST_H

/*
 * Readability defines and their associated numerical values + compile-time constants
 */

//Defaults
#define DEFAULT_CLIENT_SSID "Your_Network"
#define DEFAULT_AP_PASS     "wled1234"
#define DEFAULT_OTA_PASS    "wledota"

//increase if you need more
#define WLED_MAX_USERMODS 4

//Usermod IDs
#define USERMOD_ID_RESERVED       0            //Unused. Might indicate no usermod present
#define USERMOD_ID_UNSPECIFIED    1            //Default value for a general user mod that does not specify a custom ID
#define USERMOD_ID_EXAMPLE        2            //Usermod "usermod_v2_example.h"
#define USERMOD_ID_TEMPERATURE    3            //Usermod "usermod_temperature.h"
#define USERMOD_ID_FIXNETSERVICES 4            //Usermod "usermod_Fix_unreachable_netservices.h"
#define USERMOD_ID_PIRSWITCH      5            //Usermod "usermod_PIR_sensor_switch.h"
#define USERMOD_ID_IMU            6            //Usermod "usermod_mpu6050_imu.h"

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
#define REALTIME_MODE_DDP         8

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
#define DMX_MODE_MULTIPLE_RGBW    6            //every LED is addressed with its own RGBW (ledCount * 4 channels)

//Light capability byte (unused) 0bRRCCTTTT
//bits 0/1/2/3: specifies a type of LED driver. A single "driver" may have different chip models but must have the same protocol/behavior
//bits 4/5: specifies the class of LED driver - 0b00 (dec. 0-15)  unconfigured/reserved
//                                            - 0b01 (dec. 16-31) digital (data pin only)
//                                            - 0b10 (dec. 32-47) analog (PWM)
//                                            - 0b11 (dec. 48-63) digital (data + clock / SPI)
//bits 6/7 are reserved and set to 0b00

#define TYPE_NONE                 0            //light is not configured
#define TYPE_RESERVED             1            //unused. Might indicate a "virtual" light
//Digital types (data pin only) (16-31)
#define TYPE_WS2812_1CH          20            //white-only chips
#define TYPE_WS2812_WWA          21            //amber + warm + cold white
#define TYPE_WS2812_RGB          22
#define TYPE_GS8608              23            //same driver as WS2812, but will require signal 2x per second (else displays test pattern)
#define TYPE_WS2811_400KHZ       24            //half-speed WS2812 protocol, used by very old WS2811 units
#define TYPE_SK6812_RGBW         30
//"Analog" types (PWM) (32-47)
#define TYPE_ONOFF               40            //binary output (relays etc.)
#define TYPE_ANALOG_1CH          41            //single channel PWM. Uses value of brightest RGBW channel
#define TYPE_ANALOG_2CH          42            //analog WW + CW
#define TYPE_ANALOG_3CH          43            //analog RGB
#define TYPE_ANALOG_4CH          44            //analog RGBW
#define TYPE_ANALOG_5CH          45            //analog RGB + WW + CW
//Digital types (data + clock / SPI) (48-63)
#define TYPE_WS2801              50
#define TYPE_APA102              51
#define TYPE_LPD8806             52
#define TYPE_P9813               53
#define TYPE_TM1814              54


//Color orders
#define COL_ORDER_GRB             0           //GRB(w),defaut
#define COL_ORDER_RGB             1           //common for WS2811
#define COL_ORDER_BRG             2
#define COL_ORDER_RBG             3
#define COL_ORDER_BGR             4
#define COL_ORDER_GBR             5


//Button type
#define BTN_TYPE_NONE             0
#define BTN_TYPE_RESERVED         1
#define BTN_TYPE_PUSH             2
#define BTN_TYPE_PUSH_ACT_HIGH    3 //not implemented
#define BTN_TYPE_SWITCH           4 //not implemented
#define BTN_TYPE_SWITCH_ACT_HIGH  5 //not implemented


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
#define SEG_OPTION_MIRROR         3            //Indicates that the effect will be mirrored within the segment
#define SEG_OPTION_NONUNITY       4            //Indicates that the effect does not use FRAMETIME or needs getPixelColor
#define SEG_OPTION_FREEZE         5            //Segment contents will not be refreshed
#define SEG_OPTION_TRANSITIONAL   7

// WLED Error modes
#define ERR_NONE         0  // All good :)
#define ERR_EEP_COMMIT   2  // Could not commit to EEPROM (wrong flash layout?)
#define ERR_JSON         9  // JSON parsing failed (input too large?)
#define ERR_FS_BEGIN    10  // Could not init filesystem (no partition?)
#define ERR_FS_QUOTA    11  // The FS is full or the maximum file size is reached
#define ERR_FS_PLOAD    12  // It was attempted to load a preset that does not exist
#define ERR_FS_GENERAL  19  // A general unspecified filesystem error occured

//Timer mode types
#define NL_MODE_SET               0            //After nightlight time elapsed, set to target brightness
#define NL_MODE_FADE              1            //Fade to target brightness gradually
#define NL_MODE_COLORFADE         2            //Fade to target brightness and secondary color gradually
#define NL_MODE_SUN               3            //Sunrise/sunset. Target brightness is set immediately, then Sunrise effect is started. Max 60 min.


#define NTP_PACKET_SIZE 48

// maximum number of LEDs - more than 1500 LEDs (or 500 DMA "LEDPIN 3" driven ones) will cause a low memory condition on ESP8266
#ifndef MAX_LEDS
#define MAX_LEDS 1500
#endif

#define MAX_LEDS_DMA 500

// string temp buffer (now stored in stack locally)
#define OMAX 2048

#define E131_MAX_UNIVERSE_COUNT 9

#define ABL_MILLIAMPS_DEFAULT 850; // auto lower brightness to stay close to milliampere limit


#define TOUCH_THRESHOLD 32 // limit to recognize a touch, higher value means more sensitive

// Size of buffer for API JSON object (increase for more segments)
#ifdef ESP8266
  #define JSON_BUFFER_SIZE 9216
#else
  #define JSON_BUFFER_SIZE 16384
#endif

#endif
