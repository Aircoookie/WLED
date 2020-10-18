/**
 * @file       BlynkDebug.cpp
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief      Debug utilities for Arduino
 */
#include "BlynkDebug.h"

#if defined(ARDUINO) && defined(__AVR__) && defined(BLYNK_USE_AVR_WDT)

    #include <Arduino.h>
    #include <avr/wdt.h>

    BLYNK_CONSTRUCTOR
    static void BlynkSystemInit()
    {
        MCUSR = 0;
        wdt_disable();
    }

    void BlynkReset()
    {
        wdt_enable(WDTO_15MS);
        delay(50);
        void(*resetFunc)(void) = 0;
        resetFunc();
        for(;;) {} // To make compiler happy
    }

    size_t BlynkFreeRam()
    {
        extern int __heap_start, *__brkval;
        int v;
        return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
    }

    #define _BLYNK_USE_DEFAULT_MILLIS
    #define _BLYNK_USE_DEFAULT_DELAY

#elif defined(ARDUINO) && defined(__AVR__)

    #include <Arduino.h>

    void BlynkReset()
    {
        void(*resetFunc)(void) = 0;
        resetFunc();
        for(;;) {}
    }

    size_t BlynkFreeRam()
    {
        extern int __heap_start, *__brkval;
        int v;
        return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
    }

    #define _BLYNK_USE_DEFAULT_MILLIS
    #define _BLYNK_USE_DEFAULT_DELAY

#elif defined(ARDUINO) && defined(ESP8266)

    #include <Arduino.h>

    size_t BlynkFreeRam()
    {
        return ESP.getFreeHeap();
    }

    void BlynkReset()
    {
        ESP.restart();
        for(;;) {}
    }

    #define _BLYNK_USE_DEFAULT_MILLIS
    #define _BLYNK_USE_DEFAULT_DELAY

#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_SAM)

    #include <Arduino.h>

    size_t BlynkFreeRam()
    {
        return 0;
    }

    void BlynkReset()
    {
        NVIC_SystemReset();
        for(;;) {}
    }

    #define _BLYNK_USE_DEFAULT_MILLIS
    #define _BLYNK_USE_DEFAULT_DELAY

#elif defined (ARDUINO_ARCH_ARC32)

    millis_time_t BlynkMillis()
    {
        // TODO: Remove workaround for Intel Curie
        // https://forum.arduino.cc/index.php?topic=391836.0
        noInterrupts();
        uint64_t t = millis();
        interrupts();
        return t;
    }

    #define _BLYNK_USE_DEFAULT_FREE_RAM
    #define _BLYNK_USE_DEFAULT_RESET
    #define _BLYNK_USE_DEFAULT_DELAY

#elif defined(ARDUINO) && (defined(__STM32F1__) || defined(__STM32F3__))

    #include <Arduino.h>
    #include <libmaple/nvic.h>

    void BlynkReset()
    {
        nvic_sys_reset();
        for(;;) {}
    }

    #define _BLYNK_USE_DEFAULT_FREE_RAM
    #define _BLYNK_USE_DEFAULT_MILLIS
    #define _BLYNK_USE_DEFAULT_DELAY

#elif defined (PARTICLE) || defined(SPARK)

    #include "application.h"

    void BlynkReset()
    {
        System.reset();
        for(;;) {} // To make compiler happy
    }

    #define _BLYNK_USE_DEFAULT_FREE_RAM
    #define _BLYNK_USE_DEFAULT_MILLIS
    #define _BLYNK_USE_DEFAULT_DELAY

#elif defined(__MBED__)

    #include "mbed.h"

    static Timer  blynk_millis_timer;
    static Ticker blynk_waker;

    static
    void blynk_wake() {
        //pc.puts("(...)");
    }

    BLYNK_CONSTRUCTOR
    static void BlynkSystemInit()
    {
        blynk_waker.attach(&blynk_wake, 2.0);
        blynk_millis_timer.start();
    }

    void BlynkDelay(millis_time_t ms)
    {
        wait_ms(ms);
    }

    millis_time_t BlynkMillis()
    {
        return blynk_millis_timer.read_ms();
    }

    #define _BLYNK_USE_DEFAULT_FREE_RAM
    #define _BLYNK_USE_DEFAULT_RESET

#elif defined(LINUX) && defined(RASPBERRY)

    #include <stdlib.h>
    #include <wiringPi.h>

    BLYNK_CONSTRUCTOR
    static void BlynkSystemInit()
    {
        wiringPiSetupGpio();
    }

    void BlynkReset()
    {
        exit(1);
        for(;;) {} // To make compiler happy
    }

    #define _BLYNK_USE_DEFAULT_FREE_RAM
    #define _BLYNK_USE_DEFAULT_MILLIS
    #define _BLYNK_USE_DEFAULT_DELAY

#elif defined(LINUX)

    #define _POSIX_C_SOURCE 200809L
    #include <stdlib.h>
    #include <time.h>
    #include <unistd.h>

    static millis_time_t blynk_startup_time = 0;

    BLYNK_CONSTRUCTOR
    static void BlynkSystemInit()
    {
        blynk_startup_time = BlynkMillis();
    }

    void BlynkReset()
    {
        exit(1);
        for(;;) {} // To make compiler happy
    }

    void BlynkDelay(millis_time_t ms)
    {
        usleep(ms * 1000);
    }

    millis_time_t BlynkMillis()
    {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts );
        return ( ts.tv_sec * 1000 + ts.tv_nsec / 1000000L ) - blynk_startup_time;
    }

    #define _BLYNK_USE_DEFAULT_FREE_RAM

#else

    #if defined(BLYNK_DEBUG_ALL)
        #warning "Need to implement board-specific utilities"
    #endif

    #define _BLYNK_USE_DEFAULT_FREE_RAM
    #define _BLYNK_USE_DEFAULT_RESET
    #define _BLYNK_USE_DEFAULT_MILLIS
    #define _BLYNK_USE_DEFAULT_DELAY

#endif

#ifdef _BLYNK_USE_DEFAULT_DELAY
    void BlynkDelay(millis_time_t ms)
    {
        return delay(ms);
    }
#endif

#ifdef _BLYNK_USE_DEFAULT_MILLIS
    millis_time_t BlynkMillis()
    {
        return millis();
    }
#endif

#ifdef _BLYNK_USE_DEFAULT_FREE_RAM
    size_t BlynkFreeRam()
    {
        return 0;
    }
#endif

#ifdef _BLYNK_USE_DEFAULT_RESET
    void BlynkReset()
    {
        for(;;) {} // To make compiler happy
    }
#endif

void BlynkFatal()
{
    BlynkDelay(10000L);
    BlynkReset();
}

