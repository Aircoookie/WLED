/**
 * @file       BlynkApiArduino.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Mar 2015
 * @brief
 *
 */

#ifndef BlynkApiArduino_h
#define BlynkApiArduino_h

#include "BlynkApi.h"
#include <Arduino.h>

#ifdef BLYNK_NO_INFO

template<class Proto>
BLYNK_FORCE_INLINE
void BlynkApi<Proto>::sendInfo() {}

#else

template<class Proto>
BLYNK_FORCE_INLINE
void BlynkApi<Proto>::sendInfo()
{
    static const char profile[] BLYNK_PROGMEM = "blnkinf\0"
#ifdef BOARD_FIRMWARE_VERSION
        BLYNK_PARAM_KV("ver"    , BOARD_FIRMWARE_VERSION)
        BLYNK_PARAM_KV("blynk"  , BLYNK_VERSION)
#else
        BLYNK_PARAM_KV("ver"    , BLYNK_VERSION)
#endif
#ifdef BOARD_TEMPLATE_ID
        BLYNK_PARAM_KV("tmpl"   , BOARD_TEMPLATE_ID)
#endif
        BLYNK_PARAM_KV("h-beat" , BLYNK_TOSTRING(BLYNK_HEARTBEAT))
        BLYNK_PARAM_KV("buff-in", BLYNK_TOSTRING(BLYNK_MAX_READBYTES))
#ifdef BLYNK_INFO_DEVICE
        BLYNK_PARAM_KV("dev"    , BLYNK_INFO_DEVICE)
#endif
#ifdef BLYNK_INFO_CPU
        BLYNK_PARAM_KV("cpu"    , BLYNK_INFO_CPU)
#endif
#ifdef BLYNK_INFO_CONNECTION
        BLYNK_PARAM_KV("con"    , BLYNK_INFO_CONNECTION)
#endif
        BLYNK_PARAM_KV("build"  , __DATE__ " " __TIME__)
        "\0"
    ;
    const size_t profile_len = sizeof(profile)-8-2;

#ifdef BLYNK_HAS_PROGMEM
    char mem[profile_len];
    memcpy_P(mem, profile+8, profile_len);
    static_cast<Proto*>(this)->sendCmd(BLYNK_CMD_INTERNAL, 0, mem, profile_len);
#else
    static_cast<Proto*>(this)->sendCmd(BLYNK_CMD_INTERNAL, 0, profile+8, profile_len);
#endif
    return;
}

#endif


// Check if analog pins can be referenced by name on this device
#if defined(analogInputToDigitalPin)
    #define BLYNK_DECODE_PIN(it) (((it).asStr()[0] == 'A') ? analogInputToDigitalPin(atoi((it).asStr()+1)) : (it).asInt())
#else
    #define BLYNK_DECODE_PIN(it) ((it).asInt())

    #if defined(BLYNK_DEBUG_ALL)
        #pragma message "analogInputToDigitalPin not defined"
    #endif
#endif

template<class Proto>
BLYNK_FORCE_INLINE
void BlynkApi<Proto>::processCmd(const void* buff, size_t len)
{
    BlynkParam param((void*)buff, len);
    BlynkParam::iterator it = param.begin();
    if (it >= param.end())
        return;
    const char* cmd = it.asStr();
    uint16_t cmd16;
    memcpy(&cmd16, cmd, sizeof(cmd16));
    if (++it >= param.end())
        return;

    uint8_t pin = BLYNK_DECODE_PIN(it);

    switch(cmd16) {

#ifndef BLYNK_NO_BUILTIN

    case BLYNK_HW_PM: {
        while (it < param.end()) {
            pin = BLYNK_DECODE_PIN(it);
            ++it;
            if (!strcmp(it.asStr(), "in")) {
                pinMode(pin, INPUT);
            } else if (!strcmp(it.asStr(), "out") || !strcmp(it.asStr(), "pwm")) {
                pinMode(pin, OUTPUT);
#ifdef INPUT_PULLUP
            } else if (!strcmp(it.asStr(), "pu")) {
                pinMode(pin, INPUT_PULLUP);
#endif
#ifdef INPUT_PULLDOWN
            } else if (!strcmp(it.asStr(), "pd")) {
                pinMode(pin, INPUT_PULLDOWN);
#endif
            } else {
#ifdef BLYNK_DEBUG
                BLYNK_LOG4(BLYNK_F("Invalid pin "), pin, BLYNK_F(" mode "), it.asStr());
#endif
            }
            ++it;
        }
    } break;
    case BLYNK_HW_DR: {
        char mem[16];
        BlynkParam rsp(mem, 0, sizeof(mem));
        rsp.add("dw");
        rsp.add(pin);
        rsp.add(digitalRead(pin));
        static_cast<Proto*>(this)->sendCmd(BLYNK_CMD_HARDWARE, 0, rsp.getBuffer(), rsp.getLength()-1);
    } break;
    case BLYNK_HW_DW: {
        // Should be 1 parameter (value)
        if (++it >= param.end())
            return;

#ifdef ESP8266
        // Disable PWM...
        analogWrite(pin, 0);
#endif
#ifndef BLYNK_MINIMIZE_PINMODE_USAGE
        pinMode(pin, OUTPUT);
#endif
        digitalWrite(pin, it.asInt() ? HIGH : LOW);
    } break;
    case BLYNK_HW_AR: {
        char mem[16];
        BlynkParam rsp(mem, 0, sizeof(mem));
        rsp.add("aw");
        rsp.add(pin);
        rsp.add(analogRead(pin));
        static_cast<Proto*>(this)->sendCmd(BLYNK_CMD_HARDWARE, 0, rsp.getBuffer(), rsp.getLength()-1);
    } break;

// TODO: Remove workaround for ESP32
#if !defined(ESP32)

    case BLYNK_HW_AW: {
        // Should be 1 parameter (value)
        if (++it >= param.end())
            return;

#ifndef BLYNK_MINIMIZE_PINMODE_USAGE
        pinMode(pin, OUTPUT);
#endif
        analogWrite(pin, it.asInt());
    } break;

#endif // TODO: Remove workaround for ESP32

#endif

    case BLYNK_HW_VR: {
        BlynkReq req = { pin };
        WidgetReadHandler handler = GetReadHandler(pin);
        if (handler && (handler != BlynkWidgetRead)) {
            handler(req);
        } else {
            BlynkWidgetReadDefault(req);
        }
    } break;
    case BLYNK_HW_VW: {
        ++it;
        char* start = (char*)it.asStr();
        BlynkParam param2(start, len - (start - (char*)buff));
        BlynkReq req = { pin };
        WidgetWriteHandler handler = GetWriteHandler(pin);
        if (handler && (handler != BlynkWidgetWrite)) {
            handler(req, param2);
        } else {
            BlynkWidgetWriteDefault(req, param2);
        }
    } break;
    default:
        BLYNK_LOG2(BLYNK_F("Invalid HW cmd: "), cmd);
        static_cast<Proto*>(this)->sendCmd(BLYNK_CMD_RESPONSE, static_cast<Proto*>(this)->msgIdOutOverride, NULL, BLYNK_ILLEGAL_COMMAND);
    }
}

#endif
