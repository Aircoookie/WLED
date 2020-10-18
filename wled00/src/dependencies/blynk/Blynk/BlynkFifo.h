/**
 * @file       BlynkFifo.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Feb 2015
 * @brief      FIFO implementation
 *
 */

#ifndef BlynkFifo_h
#define BlynkFifo_h

#include "BlynkUtility.h"

template <class T, unsigned N>
class BlynkFifo
{
public:
    BlynkFifo()
    {
        clear();
    }

    void clear()
    {
        _r = 0;
        _w = 0;
    }

    ~BlynkFifo(void)
    {}

    // writing thread/context API
    //-------------------------------------------------------------

    bool writeable(void)
    {
        return free() > 0;
    }

    int free(void)
    {
        int s = _r - _w;
        if (s <= 0)
            s += N;
        return s - 1;
    }

    T put(const T& c)
    {
        int i = _w;
        int j = i;
        i = _inc(i);
        while (i == _r) // = !writeable()
            /* nothing / just wait */;
        _b[j] = c;
        _w = i;
        return c;
    }

    int put(const T* p, int n, bool blocking = false)
    {
        int c = n;
        while (c)
        {
            int f;
            while ((f = free()) == 0) // wait for space
            {
                if (!blocking) return n - c; // no more space and not blocking
                /* nothing / just wait */;
            }
            // check free space
            if (c < f) f = c;
            int w = _w;
            int m = N - w;
            // check wrap
            if (f > m) f = m;
            memcpy(&_b[w], p, f);
            _w = _inc(w, f);
            c -= f;
            p += f;
        }
        return n - c;
    }

    // reading thread/context API
    // --------------------------------------------------------

    bool readable(void)
    {
        return (_r != _w);
    }

    size_t size(void)
    {
        int s = _w - _r;
        if (s < 0)
            s += N;
        return s;
    }

    T get(void)
    {
        int r = _r;
        while (r == _w) // = !readable()
            /* nothing / just wait */;
        T t = _b[r];
        _r = _inc(r);
        return t;
    }

    T peek(void)
    {
        int r = _r;
        while (r == _w);
        return _b[r];
    }

    int get(T* p, int n, bool blocking = false)
    {
        int c = n;
        while (c)
        {
            int f;
            for (;;) // wait for data
            {
                f = size();
                if (f)  break;        // free space
                if (!blocking) return n - c; // no space and not blocking
                /* nothing / just wait */;
            }
            // check available data
            if (c < f) f = c;
            int r = _r;
            int m = N - r;
            // check wrap
            if (f > m) f = m;
            memcpy(p, &_b[r], f);
            _r = _inc(r, f);
            c -= f;
            p += f;
        }
        return n - c;
    }

private:
    int _inc(int i, int n = 1)
    {
        return (i + n) % N;
    }

    T             _b[N];
    volatile int  _w;
    volatile int  _r;
};

#endif
