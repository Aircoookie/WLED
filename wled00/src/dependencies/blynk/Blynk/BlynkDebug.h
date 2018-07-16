/**
 * @file       BlynkDebug.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief      Debug utilities
 *
 */

#ifndef BlynkDebug_h
#define BlynkDebug_h

#include "BlynkConfig.h"

#include <stddef.h>
#ifdef ESP8266
    extern "C" {
    #include "ets_sys.h"
    #include "os_type.h"
    #include "mem.h"
    }
#else
    #include <inttypes.h>
#endif

#if defined(ARDUINO_ARCH_ARC32)
    typedef uint64_t millis_time_t;
#else
    typedef uint32_t millis_time_t;
#endif

void            BlynkDelay(millis_time_t ms);
millis_time_t   BlynkMillis();
size_t          BlynkFreeRam();
void            BlynkReset() BLYNK_NORETURN;
void            BlynkFatal() BLYNK_NORETURN;


#if defined(SPARK) || defined(PARTICLE)
    #include "application.h"
#endif

#if defined(ARDUINO)
    #if ARDUINO >= 100
        #include <Arduino.h>
    #else
        #include <WProgram.h>
    #endif
#endif

#if defined(LINUX)
    #if defined(RASPBERRY)
        #include <wiringPi.h>
    #endif
#endif

#if !defined(BLYNK_RUN_YIELD)
    #if defined(BLYNK_NO_YIELD)
        #define BLYNK_RUN_YIELD() {}
    #elif defined(SPARK) || defined(PARTICLE)
        #define BLYNK_RUN_YIELD() { Particle.process(); }
    #elif !defined(ARDUINO) || (ARDUINO < 151)
        #define BLYNK_RUN_YIELD() {}
    #else
        #define BLYNK_RUN_YIELD() { BlynkDelay(0); }
    #endif
#endif

#if defined(__AVR__)
    #include <avr/pgmspace.h>
    #define BLYNK_HAS_PROGMEM
    #define BLYNK_PROGMEM PROGMEM
    #define BLYNK_F(s) F(s)
    #define BLYNK_PSTR(s) PSTR(s)
#else
    #define BLYNK_PROGMEM
    #define BLYNK_F(s) s
    #define BLYNK_PSTR(s) s
#endif

#ifdef ARDUINO_AVR_DIGISPARK
    typedef fstr_t __FlashStringHelper;
#endif

#if defined(BLYNK_DEBUG_ALL) && !(__cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__))
    #warning "Compiler features not enabled -> please contact yor board vendor to enable c++0x"
#endif

// Diagnostic defines

#define BLYNK_FATAL(msg)     { BLYNK_LOG1(msg); BlynkFatal(); }
#define BLYNK_LOG_RAM()      { BLYNK_LOG2(BLYNK_F("Free RAM: "), BlynkFreeRam()); }
#define BLYNK_LOG_FN()       BLYNK_LOG3(BLYNK_F(__FUNCTION__), '@', __LINE__);
#define BLYNK_LOG_TROUBLE(t) BLYNK_LOG2(BLYNK_F("Trouble detected: http://docs.blynk.cc/#troubleshooting-"), t)

#ifndef BLYNK_PRINT
#undef BLYNK_DEBUG
#endif

#ifdef BLYNK_DEBUG_ALL
#define BLYNK_DEBUG
#endif

#ifdef BLYNK_PRINT

    #if defined(ARDUINO) || defined(SPARK) || defined(PARTICLE)

#if defined(ARDUINO_ARCH_ARC32)
        // This will cause error - on purpose
        #define BLYNK_LOG(msg, ...)  BLYNK_LOG_UNAVAILABLE(msg, ##__VA_ARGS__)
#else
        #define BLYNK_LOG(msg, ...)  blynk_dbg_print(BLYNK_PSTR(msg), ##__VA_ARGS__)
#endif

        #define BLYNK_LOG1(p1)            { BLYNK_LOG_TIME(); BLYNK_PRINT.println(p1); }
        #define BLYNK_LOG2(p1,p2)         { BLYNK_LOG_TIME(); BLYNK_PRINT.print(p1); BLYNK_PRINT.println(p2); }
        #define BLYNK_LOG3(p1,p2,p3)      { BLYNK_LOG_TIME(); BLYNK_PRINT.print(p1); BLYNK_PRINT.print(p2); BLYNK_PRINT.println(p3); }
        #define BLYNK_LOG4(p1,p2,p3,p4)   { BLYNK_LOG_TIME(); BLYNK_PRINT.print(p1); BLYNK_PRINT.print(p2); BLYNK_PRINT.print(p3); BLYNK_PRINT.println(p4); }
        #define BLYNK_LOG6(p1,p2,p3,p4,p5,p6) { BLYNK_LOG_TIME(); BLYNK_PRINT.print(p1); BLYNK_PRINT.print(p2); BLYNK_PRINT.print(p3); BLYNK_PRINT.print(p4); BLYNK_PRINT.print(p5); BLYNK_PRINT.println(p6); }
        #define BLYNK_LOG_IP(msg, ip)     { BLYNK_LOG_TIME(); BLYNK_PRINT.print(BLYNK_F(msg)); \
                                            BLYNK_PRINT.print(ip[0]); BLYNK_PRINT.print('.');  \
                                            BLYNK_PRINT.print(ip[1]); BLYNK_PRINT.print('.');  \
                                            BLYNK_PRINT.print(ip[2]); BLYNK_PRINT.print('.');  \
                                            BLYNK_PRINT.println(ip[3]); }
        #define BLYNK_LOG_IP_REV(msg, ip) { BLYNK_LOG_TIME(); BLYNK_PRINT.print(BLYNK_F(msg)); \
                                            BLYNK_PRINT.print(ip[3]); BLYNK_PRINT.print('.');  \
                                            BLYNK_PRINT.print(ip[2]); BLYNK_PRINT.print('.');  \
                                            BLYNK_PRINT.print(ip[1]); BLYNK_PRINT.print('.');  \
                                            BLYNK_PRINT.println(ip[0]); }

        static
        void BLYNK_LOG_TIME() {
            BLYNK_PRINT.print('[');
            BLYNK_PRINT.print(BlynkMillis());
            BLYNK_PRINT.print(BLYNK_F("] "));
        }

#ifdef BLYNK_DEBUG
        #include <ctype.h>
        #define BLYNK_DBG_BREAK()    { for(;;); }
        #define BLYNK_ASSERT(expr)   { if(!(expr)) { BLYNK_LOG2(BLYNK_F("Assertion failed: "), BLYNK_F(#expr)); BLYNK_DBG_BREAK() } }

        static
        void BLYNK_DBG_DUMP(const char* msg, const void* addr, size_t len) {
            if (len) {
                BLYNK_LOG_TIME();
                BLYNK_PRINT.print(msg);
                int l2 = len;
                const uint8_t* octets = (const uint8_t*)addr;
                bool prev_print = true;
                while (l2--) {
                    const uint8_t c = *octets++ & 0xFF;
                    if (c >= 32 && c < 127) {
                        if (!prev_print) { BLYNK_PRINT.print(']'); }
                        BLYNK_PRINT.print((char)c);
                        prev_print = true;
                    } else {
                        BLYNK_PRINT.print(prev_print?'[':'|');
                        if (c < 0x10) { BLYNK_PRINT.print('0'); }
                        BLYNK_PRINT.print(c, HEX);
                        prev_print = false;
                    }
                }
                if (!prev_print) {
                    BLYNK_PRINT.print(']');
                }
                BLYNK_PRINT.println();
            }
        }
#endif

        #if !defined(ARDUINO_ARCH_ARC32)
        #include <stdio.h>
        #include <stdarg.h>

        BLYNK_UNUSED
        void blynk_dbg_print(const char* BLYNK_PROGMEM fmt, ...)
        {
            va_list ap;
            va_start(ap, fmt);
            char buff[128];
            BLYNK_PRINT.print('[');
            BLYNK_PRINT.print(BlynkMillis());
            BLYNK_PRINT.print(BLYNK_F("] "));
#if defined(__AVR__)
            vsnprintf_P(buff, sizeof(buff), fmt, ap);
#else
            vsnprintf(buff, sizeof(buff), fmt, ap);
#endif
            BLYNK_PRINT.println(buff);
            va_end(ap);
        }
        #endif // ARDUINO_ARCH_ARC32

    #elif defined(__MBED__)

        #define BLYNK_LOG(msg, ...)       { BLYNK_PRINT.printf("[%ld] " msg "\n", BlynkMillis(), ##__VA_ARGS__); }
        #define BLYNK_LOG1(p1)            { BLYNK_LOG(p1);}
        #define BLYNK_LOG2(p1,p2)         { BLYNK_LOG(p1,p2);}
        #define BLYNK_LOG3(p1,p2,p3)      { BLYNK_LOG(p1,p2,p3);}
        #define BLYNK_LOG4(p1,p2,p3,p4)   { BLYNK_LOG(p1,p2,p3,p4);}
        #define BLYNK_LOG6(p1,p2,p3,p4,p5,p6)   { BLYNK_LOG(p1,p2,p3,p4,p5,p6);}

        #define BLYNK_LOG_TIME() BLYNK_PRINT.printf("[%ld]", BlynkMillis());

#ifdef BLYNK_DEBUG
        #define BLYNK_DBG_BREAK()    raise(SIGTRAP);
        #define BLYNK_ASSERT(expr)   assert(expr)

        static
        void BLYNK_DBG_DUMP(const char* msg, const void* addr, size_t len) {
            BLYNK_LOG_TIME();
            BLYNK_PRINT.printf(msg);
            int l2 = len;
            const uint8_t* octets = (const uint8_t*)addr;
            bool prev_print = true;
            while (l2--) {
                const uint8_t c = *octets++ & 0xFF;
                if (c >= 32 && c < 127) {
                    if (!prev_print) { BLYNK_PRINT.putc(']'); }
                    BLYNK_PRINT.putc((char)c);
                    prev_print = true;
                } else {
                    BLYNK_PRINT.putc(prev_print?'[':'|');
                    BLYNK_PRINT.printf("%02x", c);
                    prev_print = false;
                }
            }
            BLYNK_PRINT.printf("%s\n", prev_print?"":"]");
        }
#endif

    #elif defined(LINUX)

        #include <assert.h>
        #include <stdio.h>
        #include <string.h>
        #include <errno.h>
        #include <signal.h>

        #include <iostream>
        using namespace std;
        #define BLYNK_LOG(msg, ...)       { fprintf(BLYNK_PRINT, "[%ld] " msg "\n", BlynkMillis(), ##__VA_ARGS__); }
        #define BLYNK_LOG1(p1)            { BLYNK_LOG_TIME(); cout << p1 << endl; }
        #define BLYNK_LOG2(p1,p2)         { BLYNK_LOG_TIME(); cout << p1 << p2 << endl; }
        #define BLYNK_LOG3(p1,p2,p3)      { BLYNK_LOG_TIME(); cout << p1 << p2 << p3 << endl; }
        #define BLYNK_LOG4(p1,p2,p3,p4)   { BLYNK_LOG_TIME(); cout << p1 << p2 << p3 << p4 << endl; }
        #define BLYNK_LOG6(p1,p2,p3,p4,p5,p6)   { BLYNK_LOG_TIME(); cout << p1 << p2 << p3 << p4 << p5 << p6 << endl; }

        #define BLYNK_LOG_TIME() cout << '[' << BlynkMillis() << "] ";

#ifdef BLYNK_DEBUG
        #define BLYNK_DBG_BREAK()    raise(SIGTRAP);
        #define BLYNK_ASSERT(expr)   assert(expr)

        static
        void BLYNK_DBG_DUMP(const char* msg, const void* addr, size_t len) {
            BLYNK_LOG_TIME();
            fprintf(BLYNK_PRINT, "%s", msg);
            int l2 = len;
            const uint8_t* octets = (const uint8_t*)addr;
            bool prev_print = true;
            while (l2--) {
                const uint8_t c = *octets++ & 0xFF;
                if (c >= 32 && c < 127) {
                    if (!prev_print) { fputc(']', BLYNK_PRINT); }
                    fputc((char)c, BLYNK_PRINT);
                    prev_print = true;
                } else {
                    fputc(prev_print?'[':'|', BLYNK_PRINT);
                    fprintf(BLYNK_PRINT, "%02x", c);
                    prev_print = false;
                }
            }
            fprintf(BLYNK_PRINT, "%s\n", prev_print?"":"]");
        }
#endif

    #else

        #warning "Cannot detect platform"

    #endif

#endif

#ifndef BLYNK_LOG
    #define BLYNK_LOG(...)
    #define BLYNK_LOG1(p1)
    #define BLYNK_LOG2(p1,p2)
    #define BLYNK_LOG3(p1,p2,p3)
    #define BLYNK_LOG4(p1,p2,p3,p4)
    #define BLYNK_LOG6(p1,p2,p3,p4,p5,p6)
    #define BLYNK_LOG_IP(msg, ip)
    #define BLYNK_LOG_IP_REV(msg, ip)
#endif

#ifndef BLYNK_DBG_BREAK
    #define BLYNK_DBG_BREAK()
    #define BLYNK_ASSERT(expr)
    #define BLYNK_DBG_DUMP(msg, addr, len)
#endif

#endif
