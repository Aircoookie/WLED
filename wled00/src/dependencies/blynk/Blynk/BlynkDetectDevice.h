/**
 * @file       BlynkDetectDevice.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       May 2016
 * @brief
 *
 */

#ifndef BlynkDetectDevice_h
#define BlynkDetectDevice_h

// General defines

#define BLYNK_NEWLINE "\r\n"

#define BLYNK_CONCAT(a, b) a ## b
#define BLYNK_CONCAT2(a, b) BLYNK_CONCAT(a, b)

#define BLYNK_STRINGIFY(x) #x
#define BLYNK_TOSTRING(x) BLYNK_STRINGIFY(x)

#define BLYNK_COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#define BLYNK_ATTR_PACKED __attribute__ ((__packed__))
#define BLYNK_NORETURN __attribute__ ((noreturn))
#define BLYNK_UNUSED __attribute__((__unused__))
#define BLYNK_DEPRECATED __attribute__ ((deprecated))
#define BLYNK_CONSTRUCTOR __attribute__((constructor))

// Causes problems on some platforms
#define BLYNK_FORCE_INLINE inline //__attribute__((always_inline))

#ifndef BLYNK_INFO_CPU
    #if   defined(__AVR_ATmega168__)
    #define BLYNK_INFO_CPU      "ATmega168"
    #endif
#endif

#ifndef BLYNK_INFO_DEVICE

    #if   defined(ENERGIA)

        #define BLYNK_NO_YIELD
        #define BLYNK_USE_128_VPINS

        #if   defined(ENERGIA_ARCH_MSP430)
            #define BLYNK_INFO_DEVICE  "LaunchPad MSP430"
            #define BLYNK_INFO_CPU     "MSP430"
            #define BLYNK_NO_FLOAT
        #elif defined(ENERGIA_ARCH_MSP432)
            #define BLYNK_INFO_DEVICE  "LaunchPad MSP432"
            #define BLYNK_INFO_CPU     "MSP432"
        #elif defined(ENERGIA_ARCH_TIVAC)
            #define BLYNK_INFO_DEVICE  "LaunchPad"

        #elif defined(ENERGIA_ARCH_CC3200EMT) || defined(ENERGIA_ARCH_CC3200)
            #define BLYNK_INFO_CONNECTION  "CC3200"
            #define BLYNK_SEND_CHUNK 64
            #define BLYNK_BUFFERS_SIZE 1024

            #if   defined(ENERGIA_CC3200_LAUNCHXL) //TODO: This is a bug in Energia IDE
            #define BLYNK_INFO_DEVICE  "CC3200 LaunchXL"
            #elif defined(ENERGIA_RedBearLab_CC3200)
            #define BLYNK_INFO_DEVICE  "RBL CC3200"
            #elif defined(ENERGIA_RedBearLab_WiFiMini)
            #define BLYNK_INFO_DEVICE  "RBL WiFi Mini"
            #elif defined(ENERGIA_RedBearLab_WiFiMicro)
            #define BLYNK_INFO_DEVICE  "RBL WiFi Micro"
            #endif
        #elif defined(ENERGIA_ARCH_CC3220EMT) || defined(ENERGIA_ARCH_CC3220)
            #define BLYNK_INFO_CONNECTION  "CC3220"
            #define BLYNK_SEND_CHUNK 64
            #define BLYNK_BUFFERS_SIZE 1024

            #define BLYNK_USE_INTERNAL_DTOSTRF

            #define BLYNK_INFO_DEVICE  "CC3220"
            #define BLYNK_INFO_CPU     "CC3220"
        #endif

        #if !defined(BLYNK_INFO_DEVICE)
        #define BLYNK_INFO_DEVICE  "Energia"
        #endif

    #elif defined(LINUX)

        #define BLYNK_INFO_DEVICE  "Linux"
        #define BLYNK_USE_128_VPINS
        #define BLYNK_BUFFERS_SIZE 4096

    #elif defined(SPARK) || defined(PARTICLE)

        #define BLYNK_USE_128_VPINS
        #define BLYNK_BUFFERS_SIZE 1024

        #if PLATFORM_ID==0
        #define BLYNK_INFO_DEVICE  "Particle Core"
        #undef BLYNK_BUFFERS_SIZE // Use default on Core
        #elif PLATFORM_ID==6
        #define BLYNK_INFO_DEVICE  "Particle Photon"
        #elif PLATFORM_ID==8
        #define BLYNK_INFO_DEVICE  "Particle P1"
        #elif PLATFORM_ID==9
        #define BLYNK_INFO_DEVICE  "Particle Ethernet"
        #elif PLATFORM_ID==10
        #define BLYNK_INFO_DEVICE  "Particle Electron"
        #elif PLATFORM_ID==31
        #define BLYNK_INFO_DEVICE  "Particle RPi"
        #elif PLATFORM_ID==82
        #define BLYNK_INFO_DEVICE  "Digistump Oak"
        #elif PLATFORM_ID==88
        #define BLYNK_INFO_DEVICE  "RedBear Duo"
        #elif PLATFORM_ID==103
        #define BLYNK_INFO_DEVICE  "Bluz"
        #else
        #if defined(BLYNK_DEBUG_ALL)
            #warning "Cannot detect board type"
        #endif
        #define BLYNK_INFO_DEVICE  "Particle"
        #endif

    #elif defined(__MBED__)

        #define BLYNK_INFO_DEVICE  "MBED"
        #define BLYNK_USE_128_VPINS
        #define BLYNK_BUFFERS_SIZE 512
        #define noInterrupts() __disable_irq()
        #define interrupts()   __enable_irq()

    #elif defined(ARDUINO) && defined(MPIDE)
        #define BLYNK_NO_YIELD

        #if   defined(_BOARD_UNO_)
        #define BLYNK_INFO_DEVICE  "chipKIT Uno32"
        #else
        #define BLYNK_INFO_DEVICE  "chipKIT"
        #endif

    #elif defined(ARDUINO) && defined(ARDUINO_AMEBA)
        #if defined(BOARD_RTL8710)
        #define BLYNK_INFO_DEVICE  "RTL8710"
        #define BLYNK_USE_128_VPINS
        #define BLYNK_BUFFERS_SIZE 1024
        #elif defined(BOARD_RTL8711AM)
        #define BLYNK_INFO_DEVICE  "RTL8711AM"
        #define BLYNK_USE_128_VPINS
        #define BLYNK_BUFFERS_SIZE 1024
        #elif defined(BOARD_RTL8195A)
        #define BLYNK_INFO_DEVICE  "RTL8195A"
        #define BLYNK_USE_128_VPINS
        #define BLYNK_BUFFERS_SIZE 1024
        #else
        #define BLYNK_INFO_DEVICE  "Ameba"
        #endif

    #elif defined(ARDUINO) && defined(TEENSYDUINO)

        #if   defined(__MK66FX1M0__)
        #define BLYNK_INFO_DEVICE  "Teensy 3.6"
        #define BLYNK_USE_128_VPINS
        #define BLYNK_BUFFERS_SIZE 1024
        #elif defined(__MK64FX512__)
        #define BLYNK_INFO_DEVICE  "Teensy 3.5"
        #define BLYNK_USE_128_VPINS
        #define BLYNK_BUFFERS_SIZE 1024
        #elif defined(__MK20DX256__)
        #define BLYNK_INFO_DEVICE  "Teensy 3.2/3.1"
        #define BLYNK_USE_128_VPINS
        #define BLYNK_BUFFERS_SIZE 1024
        #elif   defined(__MK20DX128__)
        #define BLYNK_INFO_DEVICE  "Teensy 3.0"
        #define BLYNK_USE_128_VPINS
        #define BLYNK_BUFFERS_SIZE 1024
        #elif   defined(__MKL26Z64__)
        #define BLYNK_INFO_DEVICE  "Teensy LC"
        #define BLYNK_BUFFERS_SIZE 512
        #elif   defined(ARDUINO_ARCH_AVR)
        #define BLYNK_INFO_DEVICE  "Teensy 2.0"
        #else
        #define BLYNK_INFO_DEVICE  "Teensy"
        #endif

    #elif defined(ARDUINO)

        #if defined(ARDUINO_ARCH_SAMD) || defined(ESP32) || defined(ESP8266)
            #define BLYNK_USE_128_VPINS
            #define BLYNK_BUFFERS_SIZE 1024
        #endif

        /* Arduino AVR */
        #if   defined(ARDUINO_AVR_NANO)
        #define BLYNK_INFO_DEVICE  "Arduino Nano"

        /* ESP8266 */
        #elif defined(ARDUINO_ESP8266_ESP01)
        #define BLYNK_INFO_DEVICE  "ESP8266"
        #elif defined(ARDUINO_ESP8266_ESP12)
        #define BLYNK_INFO_DEVICE  "ESP-12"
        #elif defined(ARDUINO_ESP8266_NODEMCU)
        #define BLYNK_INFO_DEVICE  "NodeMCU"
        #elif defined(ARDUINO_ESP8266_THING)
        #define BLYNK_INFO_DEVICE  "Esp Thing"
        #elif defined(ARDUINO_ESP8266_THING_DEV)
        #define BLYNK_INFO_DEVICE  "Esp Thing Dev"

        /* ESP32 */
        #elif defined(ARDUINO_ESP32_DEV)
        #define BLYNK_INFO_DEVICE  "ESP32"
        #elif defined(ARDUINO_ESP320)
        #define BLYNK_INFO_DEVICE  "SweetPeas ESP320"
        #elif defined(ARDUINO_NANO32)
        #define BLYNK_INFO_DEVICE  "ESP32 Nano32"
        #elif defined(ARDUINO_LoLin32)
        #define BLYNK_INFO_DEVICE  "LoLin32"
        #elif defined(ARDUINO_ESPea32)
        #define BLYNK_INFO_DEVICE  "ESPea32"
        #elif defined(ARDUINO_QUANTUM)
        #define BLYNK_INFO_DEVICE  "Noduino Quantum"

        #else
        #if defined(BLYNK_DEBUG_ALL)
            #warning "Cannot detect board type"
        #endif
        #define BLYNK_INFO_DEVICE  "Arduino"
        #endif

    #elif defined(TI_CC3220)
        #define BLYNK_INFO_DEVICE  "TI CC3220"
        #define BLYNK_USE_128_VPINS
        #define BLYNK_BUFFERS_SIZE 1024

        #define BLYNK_USE_INTERNAL_DTOSTRF

    #else

        #define BLYNK_INFO_DEVICE  "Custom platform"

    #endif

    #if !defined(BLYNK_MAX_READBYTES) && defined(BLYNK_BUFFERS_SIZE)
    #define BLYNK_MAX_READBYTES  BLYNK_BUFFERS_SIZE
    #endif

    #if !defined(BLYNK_MAX_SENDBYTES) && defined(BLYNK_BUFFERS_SIZE)
    #define BLYNK_MAX_SENDBYTES  BLYNK_BUFFERS_SIZE
    #endif

    // Print diagnostics

    #if defined(BLYNK_DEBUG_ALL)
        #if defined(BLYNK_INFO_DEVICE)
        #pragma message ("BLYNK_INFO_DEVICE=" BLYNK_TOSTRING(BLYNK_INFO_DEVICE))
        #endif

        #if defined(BLYNK_INFO_CPU)
        #pragma message ("BLYNK_INFO_CPU="    BLYNK_TOSTRING(BLYNK_INFO_CPU))
        #endif

        #if defined(BLYNK_BUFFERS_SIZE)
        #pragma message ("BLYNK_BUFFERS_SIZE=" BLYNK_TOSTRING(BLYNK_BUFFERS_SIZE))
        #endif
    #endif

#endif

#endif
