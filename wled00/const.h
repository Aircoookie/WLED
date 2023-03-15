#ifndef WLED_CONST_H
#define WLED_CONST_H

#include "const_ledclock.h"

/*
 * Readability defines and their associated numerical values + compile-time constants
 */

#define GRADIENT_PALETTE_COUNT 58

//Defaults
#define DEFAULT_CLIENT_SSID "Wi-Fi SSID"
#define DEFAULT_AP_SSID     "LED Clock AP"
#define DEFAULT_AP_PASS     ""
#define DEFAULT_OTA_PASS    "wledota"

//increase if you need more
#ifndef WLED_MAX_USERMODS
  #ifdef ESP8266
    #define WLED_MAX_USERMODS 4
  #else
    #define WLED_MAX_USERMODS 6
  #endif
#endif

#ifndef WLED_MAX_BUSSES
  #ifdef ESP8266
    #define WLED_MAX_BUSSES 3
    #define WLED_MIN_VIRTUAL_BUSSES 2
  #else
    #if defined(CONFIG_IDF_TARGET_ESP32C3)    // 2 RMT, 6 LEDC, only has 1 I2S but NPB does not support it ATM
      #define WLED_MAX_BUSSES 3               // will allow 2 digital & 1 analog (or the other way around)
      #define WLED_MIN_VIRTUAL_BUSSES 3
    #elif defined(CONFIG_IDF_TARGET_ESP32S2)  // 4 RMT, 8 LEDC, only has 1 I2S bus, supported in NPB
      #if defined(USERMOD_AUDIOREACTIVE)      // requested by @softhack007 https://github.com/blazoncek/WLED/issues/33
        #define WLED_MAX_BUSSES 6             // will allow 4 digital & 2 analog
        #define WLED_MIN_VIRTUAL_BUSSES 4
      #else
        #define WLED_MAX_BUSSES 7             // will allow 5 digital & 2 analog
        #define WLED_MIN_VIRTUAL_BUSSES 3
      #endif
    #elif defined(CONFIG_IDF_TARGET_ESP32S3)  // 4 RMT, 8 LEDC, has 2 I2S but NPB does not support them ATM
      #define WLED_MAX_BUSSES 6               // will allow 4 digital & 2 analog
      #define WLED_MIN_VIRTUAL_BUSSES 4
    #else
      #if defined(USERMOD_AUDIOREACTIVE)      // requested by @softhack007 https://github.com/blazoncek/WLED/issues/33
        #define WLED_MAX_BUSSES 8
        #define WLED_MIN_VIRTUAL_BUSSES 2
      #else
        #define WLED_MAX_BUSSES 10
        #define WLED_MIN_VIRTUAL_BUSSES 0
      #endif
    #endif
  #endif
#else
  #ifdef ESP8266
    #if WLED_MAX_BUSES > 5
      #error Maximum number of buses is 5.
    #endif
    #define WLED_MIN_VIRTUAL_BUSSES (5-WLED_MAX_BUSSES)
  #else
    #if WLED_MAX_BUSES > 10
      #error Maximum number of buses is 10.
    #endif
    #define WLED_MIN_VIRTUAL_BUSSES (10-WLED_MAX_BUSSES)
  #endif
#endif

#ifndef WLED_MAX_BUTTONS
  #ifdef ESP8266
    #define WLED_MAX_BUTTONS 2
  #else
    #define WLED_MAX_BUTTONS 4
  #endif
#endif

#ifdef ESP8266
#define WLED_MAX_COLOR_ORDER_MAPPINGS 5
#else
#define WLED_MAX_COLOR_ORDER_MAPPINGS 10
#endif

#if defined(WLED_MAX_LEDMAPS) && (WLED_MAX_LEDMAPS > 32 || WLED_MAX_LEDMAPS < 10)
  #undef WLED_MAX_LEDMAPS
#endif
#ifndef WLED_MAX_LEDMAPS
  #ifdef ESP8266
    #define WLED_MAX_LEDMAPS 10
  #else
    #define WLED_MAX_LEDMAPS 16
  #endif
#endif

//Usermod IDs
#define USERMOD_ID_RESERVED               0     //Unused. Might indicate no usermod present
#define USERMOD_ID_UNSPECIFIED            1     //Default value for a general user mod that does not specify a custom ID
#define USERMOD_ID_EXAMPLE                2     //Usermod "usermod_v2_example.h"
#define USERMOD_ID_TEMPERATURE            3     //Usermod "usermod_temperature.h"
#define USERMOD_ID_FIXNETSERVICES         4     //Usermod "usermod_Fix_unreachable_netservices.h"
#define USERMOD_ID_PIRSWITCH              5     //Usermod "usermod_PIR_sensor_switch.h"
#define USERMOD_ID_IMU                    6     //Usermod "usermod_mpu6050_imu.h"
#define USERMOD_ID_FOUR_LINE_DISP         7     //Usermod "usermod_v2_four_line_display.h
#define USERMOD_ID_ROTARY_ENC_UI          8     //Usermod "usermod_v2_rotary_encoder_ui.h"
#define USERMOD_ID_AUTO_SAVE              9     //Usermod "usermod_v2_auto_save.h"
#define USERMOD_ID_DHT                   10     //Usermod "usermod_dht.h"
#define USERMOD_ID_MODE_SORT             11     //Usermod "usermod_v2_mode_sort.h"
#define USERMOD_ID_VL53L0X               12     //Usermod "usermod_vl53l0x_gestures.h"
#define USERMOD_ID_MULTI_RELAY           13     //Usermod "usermod_multi_relay.h"
#define USERMOD_ID_ANIMATED_STAIRCASE    14     //Usermod "Animated_Staircase.h"
#define USERMOD_ID_RTC                   15     //Usermod "usermod_rtc.h"
#define USERMOD_ID_ELEKSTUBE_IPS         16     //Usermod "usermod_elekstube_ips.h"
#define USERMOD_ID_SN_PHOTORESISTOR      17     //Usermod "usermod_sn_photoresistor.h"
#define USERMOD_ID_BATTERY               18     //Usermod "usermod_v2_battery.h"
#define USERMOD_ID_PWM_FAN               19     //Usermod "usermod_PWM_fan.h"
#define USERMOD_ID_BH1750                20     //Usermod "usermod_bh1750.h"
#define USERMOD_ID_SEVEN_SEGMENT_DISPLAY 21     //Usermod "usermod_v2_seven_segment_display.h"
#define USERMOD_RGB_ROTARY_ENCODER       22     //Usermod "rgb-rotary-encoder.h"
#define USERMOD_ID_QUINLED_AN_PENTA      23     //Usermod "quinled-an-penta.h"
#define USERMOD_ID_SSDR                  24     //Usermod "usermod_v2_seven_segment_display_reloaded.h"
#define USERMOD_ID_CRONIXIE              25     //Usermod "usermod_cronixie.h"
#define USERMOD_ID_WIZLIGHTS             26     //Usermod "wizlights.h"
#define USERMOD_ID_WORDCLOCK             27     //Usermod "usermod_v2_word_clock.h"
#define USERMOD_ID_MY9291                28     //Usermod "usermod_MY9291.h"
#define USERMOD_ID_SI7021_MQTT_HA        29     //Usermod "usermod_si7021_mqtt_ha.h"
#define USERMOD_ID_BME280                30     //Usermod "usermod_bme280.h
#define USERMOD_ID_SMARTNEST             31     //Usermod "usermod_smartnest.h"
#define USERMOD_ID_AUDIOREACTIVE         32     //Usermod "audioreactive.h"
#define USERMOD_ID_ANALOG_CLOCK          33     //Usermod "Analog_Clock.h"
#define USERMOD_ID_PING_PONG_CLOCK       34     //Usermod "usermod_v2_ping_pong_clock.h"
#define USERMOD_ID_ADS1115               35     //Usermod "usermod_ads1115.h"
#define USERMOD_ID_BOBLIGHT              36     //Usermod "boblight.h"
#define USERMOD_ID_SD_CARD               37     //Usermod "usermod_sd_card.h"
#define USERMOD_ID_PWM_OUTPUTS           38     //Usermod "usermod_pwm_outputs.h
#define USERMOD_ID_SHT                   39     //Usermod "usermod_sht.h
#define USERMOD_ID_KLIPPER               40     // Usermod Klipper percentage

#define USERMOD_ID_LEDCLOCK              99

//Access point behavior
#define AP_BEHAVIOR_BOOT_NO_CONN          0     //Open AP when no connection after boot
#define AP_BEHAVIOR_NO_CONN               1     //Open when no connection (either after boot or if connection is lost)
#define AP_BEHAVIOR_ALWAYS                2     //Always open
#define AP_BEHAVIOR_BUTTON_ONLY           3     //Only when button pressed for 6 sec

//Notifier callMode
#define CALL_MODE_INIT           0     //no updates on init, can be used to disable updates
#define CALL_MODE_DIRECT_CHANGE  1
#define CALL_MODE_BUTTON         2     //default button actions applied to selected segments
#define CALL_MODE_NOTIFICATION   3
#define CALL_MODE_NIGHTLIGHT     4
#define CALL_MODE_NO_NOTIFY      5
#define CALL_MODE_FX_CHANGED     6     //no longer used
#define CALL_MODE_HUE            7
#define CALL_MODE_PRESET_CYCLE   8
#define CALL_MODE_BLYNK          9     //no longer used
#define CALL_MODE_ALEXA         10
#define CALL_MODE_WS_SEND       11     //special call mode, not for notifier, updates websocket only
#define CALL_MODE_BUTTON_PRESET 12     //button/IR JSON preset/macro

//RGB to RGBW conversion mode
#define RGBW_MODE_MANUAL_ONLY     0    // No automatic white channel calculation. Manual white channel slider
#define RGBW_MODE_AUTO_BRIGHTER   1    // New algorithm. Adds as much white as the darkest RGBW channel
#define RGBW_MODE_AUTO_ACCURATE   2    // New algorithm. Adds as much white as the darkest RGBW channel and subtracts this amount from each RGB channel
#define RGBW_MODE_DUAL            3    // Manual slider + auto calculation. Automatically calculates only if manual slider is set to off (0)
#define RGBW_MODE_MAX             4    // Sets white to the value of the brightest RGB channel (good for white-only LEDs without any RGB)
//#define RGBW_MODE_LEGACY        4    // Old floating algorithm. Too slow for realtime and palette support (unused)
#define AW_GLOBAL_DISABLED      255    // Global auto white mode override disabled. Per-bus setting is used

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
#define DMX_MODE_EFFECT           3            //trigger standalone effects of WLED (15 channels)
#define DMX_MODE_EFFECT_W         7            //trigger standalone effects of WLED (18 channels)
#define DMX_MODE_MULTIPLE_RGB     4            //every LED is addressed with its own RGB (ledCount * 3 channels)
#define DMX_MODE_MULTIPLE_DRGB    5            //every LED is addressed with its own RGB and share a master dimmer (ledCount * 3 + 1 channels)
#define DMX_MODE_MULTIPLE_RGBW    6            //every LED is addressed with its own RGBW (ledCount * 4 channels)
#define DMX_MODE_EFFECT_SEGMENT   8            //trigger standalone effects of WLED (15 channels per segement)
#define DMX_MODE_EFFECT_SEGMENT_W 9            //trigger standalone effects of WLED (18 channels per segement)
#define DMX_MODE_PRESET           10           //apply presets (1 channel)

//Light capability byte (unused) 0bRCCCTTTT
//bits 0/1/2/3: specifies a type of LED driver. A single "driver" may have different chip models but must have the same protocol/behavior
//bits 4/5/6: specifies the class of LED driver - 0b000 (dec. 0-15)  unconfigured/reserved
//                                              - 0b001 (dec. 16-31) digital (data pin only)
//                                              - 0b010 (dec. 32-47) analog (PWM)
//                                              - 0b011 (dec. 48-63) digital (data + clock / SPI)
//                                              - 0b100 (dec. 64-79) unused/reserved
//                                              - 0b101 (dec. 80-95) virtual network busses
//                                              - 0b110 (dec. 96-111) unused/reserved
//                                              - 0b111 (dec. 112-127) unused/reserved
//bit 7 is reserved and set to 0

#define TYPE_NONE                 0            //light is not configured
#define TYPE_RESERVED             1            //unused. Might indicate a "virtual" light
//Digital types (data pin only) (16-31)
#define TYPE_WS2812_1CH          18            //white-only chips (1 channel per IC) (unused)
#define TYPE_WS2812_1CH_X3       19            //white-only chips (3 channels per IC)
#define TYPE_WS2812_2CH_X3       20            //CCT chips (1st IC controls WW + CW of 1st zone and CW of 2nd zone, 2nd IC controls WW of 2nd zone and WW + CW of 3rd zone)
#define TYPE_WS2812_WWA          21            //amber + warm + cold white
#define TYPE_WS2812_RGB          22
#define TYPE_GS8608              23            //same driver as WS2812, but will require signal 2x per second (else displays test pattern)
#define TYPE_WS2811_400KHZ       24            //half-speed WS2812 protocol, used by very old WS2811 units
#define TYPE_TM1829              25
#define TYPE_SK6812_RGBW         30
#define TYPE_TM1814              31
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
#define TYPE_LPD6803             54
//Network types (master broadcast) (80-95)
#define TYPE_NET_DDP_RGB         80            //network DDP RGB bus (master broadcast bus)
#define TYPE_NET_E131_RGB        81            //network E131 RGB bus (master broadcast bus, unused)
#define TYPE_NET_ARTNET_RGB      82            //network ArtNet RGB bus (master broadcast bus, unused)
#define TYPE_NET_DDP_RGBW        88            //network DDP RGBW bus (master broadcast bus)

#define IS_DIGITAL(t) ((t) & 0x10) //digital are 16-31 and 48-63
#define IS_PWM(t)     ((t) > 40 && (t) < 46)
#define NUM_PWM_PINS(t) ((t) - 40) //for analog PWM 41-45 only
#define IS_2PIN(t)      ((t) > 47)

//Color orders
#define COL_ORDER_GRB             0           //GRB(w),defaut
#define COL_ORDER_RGB             1           //common for WS2811
#define COL_ORDER_BRG             2
#define COL_ORDER_RBG             3
#define COL_ORDER_BGR             4
#define COL_ORDER_GBR             5
#define COL_ORDER_MAX             5


//Button type
#define BTN_TYPE_NONE             0
#define BTN_TYPE_RESERVED         1
#define BTN_TYPE_PUSH             2
#define BTN_TYPE_PUSH_ACT_HIGH    3
#define BTN_TYPE_SWITCH           4
#define BTN_TYPE_PIR_SENSOR       5
#define BTN_TYPE_TOUCH            6
#define BTN_TYPE_ANALOG           7
#define BTN_TYPE_ANALOG_INVERTED  8

//Ethernet board types
#define WLED_NUM_ETH_TYPES        9

#define WLED_ETH_NONE             0
#define WLED_ETH_WT32_ETH01       1
#define WLED_ETH_ESP32_POE        2
#define WLED_ETH_WESP32           3
#define WLED_ETH_QUINLED          4
#define WLED_ETH_TWILIGHTLORD     5
#define WLED_ETH_ESP32DEUX        6

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
#define SEG_OPTION_FREEZE         4            //Segment contents will not be refreshed
#define SEG_OPTION_RESET          5            //Segment runtime requires reset
#define SEG_OPTION_TRANSITIONAL   6
#define SEG_OPTION_REVERSED_Y     7
#define SEG_OPTION_MIRROR_Y       8
#define SEG_OPTION_TRANSPOSED     9

//Segment differs return byte
#define SEG_DIFFERS_BRI        0x01 // opacity
#define SEG_DIFFERS_OPT        0x02 // all segment options except: selected, reset & transitional
#define SEG_DIFFERS_COL        0x04 // colors
#define SEG_DIFFERS_FX         0x08 // effect/mode parameters
#define SEG_DIFFERS_BOUNDS     0x10 // segment start/stop ounds
#define SEG_DIFFERS_GSO        0x20 // grouping, spacing & offset
#define SEG_DIFFERS_SEL        0x80 // selected

//Playlist option byte
#define PL_OPTION_SHUFFLE      0x01

// Segment capability byte
#define SEG_CAPABILITY_RGB     0x01
#define SEG_CAPABILITY_W       0x02
#define SEG_CAPABILITY_CCT     0x04

// WLED Error modes
#define ERR_NONE         0  // All good :)
#define ERR_EEP_COMMIT   2  // Could not commit to EEPROM (wrong flash layout?)
#define ERR_NOBUF        3  // JSON buffer was not released in time, request cannot be handled at this time
#define ERR_JSON         9  // JSON parsing failed (input too large?)
#define ERR_FS_BEGIN    10  // Could not init filesystem (no partition?)
#define ERR_FS_QUOTA    11  // The FS is full or the maximum file size is reached
#define ERR_FS_PLOAD    12  // It was attempted to load a preset that does not exist
#define ERR_FS_IRLOAD   13  // It was attempted to load an IR JSON cmd, but the "ir.json" file does not exist
#define ERR_FS_GENERAL  19  // A general unspecified filesystem error occured
#define ERR_OVERTEMP    30  // An attached temperature sensor has measured above threshold temperature (not implemented)
#define ERR_OVERCURRENT 31  // An attached current sensor has measured a current above the threshold (not implemented)
#define ERR_UNDERVOLT   32  // An attached voltmeter has measured a voltage below the threshold (not implemented)

//Timer mode types
#define NL_MODE_SET               0            //After nightlight time elapsed, set to target brightness
#define NL_MODE_FADE              1            //Fade to target brightness gradually
#define NL_MODE_COLORFADE         2            //Fade to target brightness and secondary color gradually
#define NL_MODE_SUN               3            //Sunrise/sunset. Target brightness is set immediately, then Sunrise effect is started. Max 60 min.


#define NTP_PACKET_SIZE 48

//maximum number of rendered LEDs - this does not have to match max. physical LEDs, e.g. if there are virtual busses
#ifndef MAX_LEDS
#ifdef ESP8266
#define MAX_LEDS 1664 //can't rely on memory limit to limit this to 1600 LEDs
#else
#define MAX_LEDS 8192
#endif
#endif

#ifndef MAX_LED_MEMORY
  #ifdef ESP8266
    #define MAX_LED_MEMORY 4000
  #else
    #if defined(ARDUINO_ARCH_ESP32S2) || defined(ARDUINO_ARCH_ESP32C3)
      #define MAX_LED_MEMORY 32000
    #else
      #define MAX_LED_MEMORY 64000
    #endif
  #endif
#endif

#ifndef MAX_LEDS_PER_BUS
#define MAX_LEDS_PER_BUS 2048   // may not be enough for fast LEDs (i.e. APA102)
#endif

// string temp buffer (now stored in stack locally)
#ifdef ESP8266
#define SETTINGS_STACK_BUF_SIZE 2048
#else
#define SETTINGS_STACK_BUF_SIZE 3096
#endif

#ifdef WLED_USE_ETHERNET
  #define E131_MAX_UNIVERSE_COUNT 20
#else
  #ifdef ESP8266
    #define E131_MAX_UNIVERSE_COUNT 9
  #else
    #define E131_MAX_UNIVERSE_COUNT 12
  #endif
#endif

#ifndef ABL_MILLIAMPS_DEFAULT
  #define ABL_MILLIAMPS_DEFAULT 850   // auto lower brightness to stay close to milliampere limit
#else
  #if ABL_MILLIAMPS_DEFAULT == 0      // disable ABL
  #elif ABL_MILLIAMPS_DEFAULT < 250   // make sure value is at least 250
   #warning "make sure value is at least 250"
   #define ABL_MILLIAMPS_DEFAULT 250
  #endif
#endif

// PWM settings
#ifndef WLED_PWM_FREQ
#ifdef ESP8266
  #define WLED_PWM_FREQ    880 //PWM frequency proven as good for LEDs
#else
  #define WLED_PWM_FREQ  19531
#endif
#endif

#define TOUCH_THRESHOLD 32 // limit to recognize a touch, higher value means more sensitive

// Size of buffer for API JSON object (increase for more segments)
#ifdef ESP8266
  #define JSON_BUFFER_SIZE 10240
#else
  #define JSON_BUFFER_SIZE 24576
#endif

//#define MIN_HEAP_SIZE (MAX_LED_MEMORY+2048)
#define MIN_HEAP_SIZE (8192)

// Maximum size of node map (list of other WLED instances)
#ifdef ESP8266
  #define WLED_MAX_NODES 24
#else
  #define WLED_MAX_NODES 150
#endif

//this is merely a default now and can be changed at runtime
#ifndef LEDPIN
#if defined(ESP8266) || (defined(ARDUINO_ARCH_ESP32) && defined(WLED_USE_PSRAM)) || defined(CONFIG_IDF_TARGET_ESP32C3)
  #define LEDPIN 2    // GPIO2 (D4) on Wemod D1 mini compatible boards
#else
  #define LEDPIN 16   // aligns with GPIO2 (D4) on Wemos D1 mini32 compatible boards
#endif
#endif

#ifdef WLED_ENABLE_DMX
#if (LEDPIN == 2)
  #undef LEDPIN
  #define LEDPIN 1
  #warning "Pin conflict compiling with DMX and LEDs on pin 2. The default LED pin has been changed to pin 1."
#endif
#endif

#ifndef DEFAULT_LED_COUNT
  #define DEFAULT_LED_COUNT 30
#endif

#define INTERFACE_UPDATE_COOLDOWN 2000 //time in ms to wait between websockets, alexa, and MQTT updates

// HW_PIN_SCL & HW_PIN_SDA are used for information in usermods settings page and usermods themselves
// which GPIO pins are actually used in a hardwarea layout (controller board)
#if defined(I2CSCLPIN) && !defined(HW_PIN_SCL)
  #define HW_PIN_SCL I2CSCLPIN
#endif
#if defined(I2CSDAPIN) && !defined(HW_PIN_SDA)
  #define HW_PIN_SDA I2CSDAPIN
#endif
// you cannot change HW I2C pins on 8266
#if defined(ESP8266) && defined(HW_PIN_SCL)
  #undef HW_PIN_SCL
#endif
#if defined(ESP8266) && defined(HW_PIN_SDA)
  #undef HW_PIN_SDA
#endif
// defaults for 1st I2C on ESP32 (Wire global)
#ifndef HW_PIN_SCL
  #define HW_PIN_SCL SCL
#endif
#ifndef HW_PIN_SDA
  #define HW_PIN_SDA SDA
#endif

// HW_PIN_SCLKSPI & HW_PIN_MOSISPI & HW_PIN_MISOSPI are used for information in usermods settings page and usermods themselves
// which GPIO pins are actually used in a hardwarea layout (controller board)
#if defined(SPISCLKPIN) && !defined(HW_PIN_CLOCKSPI)
  #define HW_PIN_CLOCKSPI SPISCLKPIN
#endif
#if defined(SPIMOSIPIN) && !defined(HW_PIN_MOSISPI)
  #define HW_PIN_MOSISPI SPIMOSIPIN
#endif
#if defined(SPIMISOPIN) && !defined(HW_PIN_MISOSPI)
  #define HW_PIN_MISOSPI SPIMISOPIN
#endif
// you cannot change HW SPI pins on 8266
#if defined(ESP8266) && defined(HW_PIN_CLOCKSPI)
  #undef HW_PIN_CLOCKSPI
#endif
#if defined(ESP8266) && defined(HW_PIN_DATASPI)
  #undef HW_PIN_DATASPI
#endif
#if defined(ESP8266) && defined(HW_PIN_MISOSPI)
  #undef HW_PIN_MISOSPI
#endif
// defaults for VSPI on ESP32 (SPI global, SPI.cpp) as HSPI is used by WLED (bus_wrapper.h)
#ifndef HW_PIN_CLOCKSPI
  #define HW_PIN_CLOCKSPI SCK
#endif
#ifndef HW_PIN_DATASPI
  #define HW_PIN_DATASPI MOSI
#endif
#ifndef HW_PIN_MISOSPI
  #define HW_PIN_MISOSPI MISO
#endif

#endif
