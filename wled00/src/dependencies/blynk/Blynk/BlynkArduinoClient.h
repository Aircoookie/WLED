/**
 * @file       BlynkParam.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief
 *
 */

#ifndef BlynkArduinoClient_h
#define BlynkArduinoClient_h

#include "BlynkApiArduino.h"
#include "BlynkDebug.h"
#include <Client.h>

#if defined(ESP8266) && !defined(BLYNK_NO_YIELD)
    #define YIELD_FIX() BLYNK_RUN_YIELD();
#else
    #define YIELD_FIX()
#endif

template <typename Client>
class BlynkArduinoClientGen
{
public:
    BlynkArduinoClientGen(Client& c)
        : client(NULL), domain(NULL), port(0), isConn(false)
    {
        setClient(&c);
    }

    BlynkArduinoClientGen()
        : client(NULL), domain(NULL), port(0), isConn(false)
    {}

    void setClient(Client* c) {
        client = c;
        client->setTimeout(BLYNK_TIMEOUT_MS);
    }

    void begin(IPAddress a, uint16_t p) {
        domain = NULL;
        port = p;
        addr = a;
    }

    void begin(const char* d, uint16_t p) {
        domain = d;
        port = p;
    }

    bool connect() {
        if (domain) {
            BLYNK_LOG4(BLYNK_F("Connecting to "), domain, ':', port);

            isConn = (1 == client->connect(domain, port));
            return isConn;
        } else { //if (uint32_t(addr) != 0) {
            BLYNK_LOG_IP("Connecting to ", addr);
            isConn = (1 == client->connect(addr, port));
            return isConn;
        }
        return false;
    }

    void disconnect() { isConn = false; client->stop(); }

#ifdef BLYNK_ENC28J60_FIX
    size_t read(void* buf, size_t len) {
        while (client->available() < len) { BLYNK_RUN_YIELD(); }
        return client->read((uint8_t*)buf, len);
    }
#else
    size_t read(void* buf, size_t len) {
        size_t res = client->readBytes((char*)buf, len);
        YIELD_FIX();
        return res;
    }
#endif

#ifdef BLYNK_RETRY_SEND
    size_t write(const void* buf, size_t len) {
        size_t sent = 0;
        int retry = 0;
        while (sent < len && ++retry < 10) {
            size_t w = client->write((const uint8_t*)buf+sent, len-sent);
            if (w != 0 && w != -1) {
                sent += w;
            } else {
                BlynkDelay(50);
#if defined(BLYNK_DEBUG) && defined(BLYNK_PRINT)
                BLYNK_PRINT_TIME();
                BLYNK_PRINT.print(BLYNK_F("Retry "));
                BLYNK_PRINT.print(retry);
                BLYNK_PRINT.print(BLYNK_F(" send: "));
                BLYNK_PRINT.print(sent);
                BLYNK_PRINT.print('/');
                BLYNK_PRINT.println(len);
#endif
            }
        }
        return sent;
    }
#else
    size_t write(const void* buf, size_t len) {
        YIELD_FIX();
        size_t res = client->write((const uint8_t*)buf, len);
        YIELD_FIX();
        return res;
    }
#endif

    bool connected() { YIELD_FIX(); return isConn && client->connected(); }
    int available() {  YIELD_FIX(); return client->available(); }

protected:
    Client*     client;
    IPAddress   addr;
    const char* domain;
    uint16_t    port;
    bool        isConn;
};

typedef BlynkArduinoClientGen<Client> BlynkArduinoClient;

#endif
