/**
 * @file       BlynkParam.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief      Container for handler parameters
 *
 */

#ifndef BlynkParam_h
#define BlynkParam_h

#include <string.h>
#include <stdlib.h>
#include "BlynkConfig.h"
#include "BlynkDebug.h"

#define BLYNK_PARAM_KV(k, v) k "\0" v "\0"

class BlynkParam
{
public:
    class iterator
    {
    public:
        iterator(const char* c, const char* l) : ptr(c), limit(l) {}
        static iterator invalid() { return iterator(NULL, NULL); }

        operator const char* () const   { return asStr(); }
        operator int () const           { return asInt(); }
        const char* asStr() const       { return ptr; }
        const char* asString() const    { return ptr; }
        int         asInt() const       { if(!isValid()) return 0; return atoi(ptr); }
        long        asLong() const      { if(!isValid()) return 0; return atol(ptr); }
        //long long   asLongLong() const  { return atoll(ptr); }
#ifndef BLYNK_NO_FLOAT
        double      asDouble() const    { if(!isValid()) return 0; return atof(ptr); }
        float       asFloat() const     { if(!isValid()) return 0; return atof(ptr); }
#endif
        bool isValid() const            { return ptr != NULL && ptr < limit; }
        bool isEmpty() const            { if(!isValid()) return true; return *ptr == '\0'; }

        bool operator <  (const iterator& it) const { return ptr < it.ptr; }
        bool operator >= (const iterator& it) const { return ptr >= it.ptr; }

        iterator& operator ++() {
            if(isValid()) {
                ptr += strlen(ptr) + 1;
            }
            return *this;
        }
    private:
        const char* ptr;
        const char* limit;
    };

public:
    explicit
    BlynkParam(void* addr, size_t length)
        : buff((char*)addr), len(length), buff_size(length)
    {}

    explicit
    BlynkParam(void* addr, size_t length, size_t buffsize)
        : buff((char*)addr), len(length), buff_size(buffsize)
    {}

    const char* asStr() const       { return buff; }
    const char* asString() const    { return buff; }
    int         asInt() const       { return atoi(buff); }
    long        asLong() const      { return atol(buff); }
    //long long   asLongLong() const  { return atoll(buff); }
#ifndef BLYNK_NO_FLOAT
    double      asDouble() const    { return atof(buff); }
    float       asFloat() const     { return atof(buff); }
#endif
    bool isEmpty() const            { return *buff == '\0'; }

    iterator begin() const { return iterator(buff, buff+len); }
    iterator end() const   { return iterator(buff+len, buff+len); }

    iterator operator[](int index) const;
    iterator operator[](const char* key) const;

    void*  getBuffer() const { return (void*)buff; }
    size_t getLength() const { return len; }

    // Modification
    void add(int value);
    void add(unsigned int value);
    void add(long value);
    void add(unsigned long value);
    void add(long long value);
    void add(unsigned long long value);
#ifndef BLYNK_NO_FLOAT
    void add(float value);
    void add(double value);
#endif
    void add(const char* str);
    void add(const void* b, size_t l);
#if defined(ARDUINO) || defined(SPARK) || defined(PARTICLE)
    void add(const String& str);
#if defined(BLYNK_HAS_PROGMEM)
    void add(const __FlashStringHelper* str);
#endif
#endif

    template<typename T, typename... Args>
    void add_multi(T last) {
        add(last);
    }

    template<typename T, typename... Args>
    void add_multi(T head, Args... tail) {
        add(head);
        add_multi(tail...);
    }

    template <typename TV>
    void add_key(const char* key, const TV& val) {
        add(key);
        add(val);
    }

protected:
    char*    buff;
    size_t   len;
    size_t   buff_size;
};


class BlynkParamAllocated
    : public BlynkParam
{
public:
    BlynkParamAllocated(size_t size)
        : BlynkParam(malloc(size), 0, size)
    {}
    ~BlynkParamAllocated() {
        free(buff);
    }
};

inline
BlynkParam::iterator BlynkParam::operator[](int index) const
{
    const iterator e = end();
    for (iterator it = begin(); it < e; ++it) {
        if (!index--) {
            return it;
        }
    }
    return iterator::invalid();
}

inline
BlynkParam::iterator BlynkParam::operator[](const char* key) const
{
    const iterator e = end();
    for (iterator it = begin(); it < e; ++it) {
        if (!strcmp(it.asStr(), key)) {
            return ++it;
        }
        ++it;
        if (it >= e) break;
    }
    return iterator::invalid();
}

inline
void BlynkParam::add(const void* b, size_t l)
{
    if (len + l > buff_size)
        return;
    memcpy(buff+len, b, l);
    len += l;
}

inline
void BlynkParam::add(const char* str)
{
    if (str == NULL) {
        buff[len++] = '\0';
        return;
    }
    add(str, strlen(str)+1);
}

#if defined(ARDUINO) || defined(SPARK) || defined(PARTICLE)
inline
void BlynkParam::add(const String& str)
{
#if defined(ARDUINO_AVR_DIGISPARK) \
    || defined(__ARDUINO_X86__) \
    || defined(__RFduino__)

    size_t len = str.length()+1;
    char buff[len];
    const_cast<String&>(str).toCharArray(buff, len);
    add(buff, len);
#else
    add(str.c_str());
#endif
}

#if defined(BLYNK_HAS_PROGMEM)

inline
void BlynkParam::add(const __FlashStringHelper* ifsh)
{
    PGM_P p = reinterpret_cast<PGM_P>(ifsh);
    size_t l = strlen_P(p) + 1;
    if (len + l > buff_size)
        return;
    memcpy_P(buff+len, p, l);
    len += l;
    buff[len] = '\0';
}

#endif

#endif

#if defined(__AVR__) || defined (ARDUINO_ARCH_ARC32)

    #include <stdlib.h>

    inline
    void BlynkParam::add(int value)
    {
        char str[2 + 8 * sizeof(value)];
        itoa(value, str, 10);
        add(str);
    }

    inline
    void BlynkParam::add(unsigned int value)
    {
        char str[1 + 8 * sizeof(value)];
        utoa(value, str, 10);
        add(str);
    }

    inline
    void BlynkParam::add(long value)
    {
        char str[2 + 8 * sizeof(value)];
        ltoa(value, str, 10);
        add(str);
    }

    inline
    void BlynkParam::add(unsigned long value)
    {
        char str[1 + 8 * sizeof(value)];
        ultoa(value, str, 10);
        add(str);
    }

    inline
    void BlynkParam::add(long long value)  // TODO: this currently adds just a long
    {
        char str[2 + 8 * sizeof(value)];
        ltoa(value, str, 10);
        add(str);
    }

    inline
    void BlynkParam::add(unsigned long long value) // TODO: this currently adds just a long
    {
        char str[1 + 8 * sizeof(value)];
        ultoa(value, str, 10);
        add(str);
    }

#ifndef BLYNK_NO_FLOAT

    inline
    void BlynkParam::add(float value)
    {
        char str[33];
        dtostrf(value, 5, 3, str);
        add(str);
    }

    inline
    void BlynkParam::add(double value)
    {
        char str[33];
        dtostrf(value, 5, 7, str);
        add(str);
    }
#endif

#else

    #include <stdio.h>

    inline
    void BlynkParam::add(int value)
    {
        len += snprintf(buff+len, buff_size-len, "%i", value)+1;
    }

    inline
    void BlynkParam::add(unsigned int value)
    {
        len += snprintf(buff+len, buff_size-len, "%u", value)+1;
    }

    inline
    void BlynkParam::add(long value)
    {
        len += snprintf(buff+len, buff_size-len, "%li", value)+1;
    }

    inline
    void BlynkParam::add(unsigned long value)
    {
        len += snprintf(buff+len, buff_size-len, "%lu", value)+1;
    }

    inline
    void BlynkParam::add(long long value)
    {
        len += snprintf(buff+len, buff_size-len, "%lli", value)+1;
    }

    inline
    void BlynkParam::add(unsigned long long value)
    {
        len += snprintf(buff+len, buff_size-len, "%llu", value)+1;
    }

#ifndef BLYNK_NO_FLOAT

#if defined(BLYNK_USE_INTERNAL_DTOSTRF)

    extern char* dtostrf_internal(double number, signed char width, unsigned char prec, char *s);

    inline
    void BlynkParam::add(float value)
    {
        char str[33];
        dtostrf_internal(value, 5, 3, str);
        add(str);
    }

    inline
    void BlynkParam::add(double value)
    {
        char str[33];
        dtostrf_internal(value, 5, 7, str);
        add(str);
    }

#else

    inline
    void BlynkParam::add(float value)
    {
        len += snprintf(buff+len, buff_size-len, "%2.3f", value)+1;
    }

    inline
    void BlynkParam::add(double value)
    {
        len += snprintf(buff+len, buff_size-len, "%2.7f", value)+1;
    }

#endif

#endif

#endif


#endif
