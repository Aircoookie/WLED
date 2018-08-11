/**
 * @file       BlynkProtocol.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief      Blynk protocol implementation
 *
 */

#ifndef BlynkProtocol_h
#define BlynkProtocol_h

#include <string.h>
#include <stdlib.h>
#include "BlynkDebug.h"
#include "BlynkProtocolDefs.h"
#include "BlynkApi.h"
#include "BlynkUtility.h"

template <class Transp>
class BlynkProtocol
    : public BlynkApi< BlynkProtocol<Transp> >
{
    friend class BlynkApi< BlynkProtocol<Transp> >;
public:
    enum BlynkState {
        CONNECTING,
        CONNECTED,
        DISCONNECTED,
    };

    BlynkProtocol(Transp& transp)
        : conn(transp)
        , authkey(NULL)
        , redir_serv(NULL)
        , lastActivityIn(0)
        , lastActivityOut(0)
        , lastHeartbeat(0)
        , msgIdOut(0)
        , msgIdOutOverride(0)
        , nesting(0)
        , state(CONNECTING)
    {}

    bool connected() { return state == CONNECTED; }

    bool connect(uint32_t timeout = BLYNK_TIMEOUT_MS*3) {
        conn.disconnect();
        state = CONNECTING;
        millis_time_t started = BlynkMillis();
        while ((state != CONNECTED) &&
               (BlynkMillis() - started < timeout))
        {
            run();
        }
        return state == CONNECTED;
    }

    void disconnect() {
        conn.disconnect();
        state = DISCONNECTED;
        BLYNK_LOG1(BLYNK_F("Disconnected"));
    }

    bool run(bool avail = false);

    // TODO: Fixme
    void startSession() {
        conn.connect();
        state = CONNECTING;
        msgIdOut = 0;
        lastHeartbeat = lastActivityIn = lastActivityOut = (BlynkMillis() - 5000UL);
    }

    void sendCmd(uint8_t cmd, uint16_t id = 0, const void* data = NULL, size_t length = 0, const void* data2 = NULL, size_t length2 = 0);

private:

    void internalReconnect() {
        state = CONNECTING;
        conn.disconnect();
        BlynkOnDisconnected();
    }

    int readHeader(BlynkHeader& hdr);
    uint16_t getNextMsgId();

protected:
    void begin(const char* auth) {
        this->authkey = auth;
        lastHeartbeat = lastActivityIn = lastActivityOut = (BlynkMillis() - 5000UL);

#if defined(BLYNK_NO_FANCY_LOGO)
        BLYNK_LOG1(BLYNK_F("Blynk v" BLYNK_VERSION " on " BLYNK_INFO_DEVICE));
#else
        BLYNK_LOG1(BLYNK_F(BLYNK_NEWLINE
            "    ___  __          __" BLYNK_NEWLINE
            "   / _ )/ /_ _____  / /__" BLYNK_NEWLINE
            "  / _  / / // / _ \\/  '_/" BLYNK_NEWLINE
            " /____/_/\\_, /_//_/_/\\_\\" BLYNK_NEWLINE
            "        /___/ v" BLYNK_VERSION " on " BLYNK_INFO_DEVICE BLYNK_NEWLINE
            BLYNK_NEWLINE
            "  Give Blynk a Github star! => https://github.com/blynkkk/blynk-library" BLYNK_NEWLINE
        ));
#endif
    }
    bool processInput(void);

    Transp& conn;

private:
    const char* authkey;
    char*       redir_serv;
    millis_time_t lastActivityIn;
    millis_time_t lastActivityOut;
    union {
        millis_time_t lastHeartbeat;
        millis_time_t lastLogin;
    };
    uint16_t msgIdOut;
    uint16_t msgIdOutOverride;
    uint8_t  nesting;
protected:
    BlynkState state;
};

template <class Transp>
bool BlynkProtocol<Transp>::run(bool avail)
{
    BLYNK_RUN_YIELD();

    if (state == DISCONNECTED) {
        return false;
    }

    // Detect nesting
    BlynkHelperAutoInc guard(nesting);
    if (msgIdOutOverride || nesting > 2) {
      //BLYNK_LOG1(BLYNK_F("Nested run() skipped"));
      return true;
    }

    if (conn.connected()) {
        while (avail || conn.available() > 0) {
            //BLYNK_LOG2(BLYNK_F("Available: "), conn.available());
            //const unsigned long t = micros();
            if (!processInput()) {
                conn.disconnect();
// TODO: Only when in direct mode?
#ifdef BLYNK_USE_DIRECT_CONNECT
                state = CONNECTING;
#endif
                BlynkOnDisconnected();
                return false;
            }
            avail = false;
            //BLYNK_LOG2(BLYNK_F("Proc time: "), micros() - t);
        }
    }

    const millis_time_t t = BlynkMillis();

    // Update connection status after running commands
    const bool tconn = conn.connected();

    if (state == CONNECTED) {
        if (!tconn) {
            lastHeartbeat = t;
            internalReconnect();
            return false;
        }

        if (t - lastActivityIn > (1000UL * BLYNK_HEARTBEAT + BLYNK_TIMEOUT_MS*3)) {
#ifdef BLYNK_DEBUG
            BLYNK_LOG6(BLYNK_F("Heartbeat timeout: "), t, BLYNK_F(", "), lastActivityIn, BLYNK_F(", "), lastHeartbeat);
#else
            BLYNK_LOG1(BLYNK_F("Heartbeat timeout"));
#endif
            internalReconnect();
            return false;
        } else if ((t - lastActivityIn  > 1000UL * BLYNK_HEARTBEAT ||
                    t - lastActivityOut > 1000UL * BLYNK_HEARTBEAT) &&
                    t - lastHeartbeat   > BLYNK_TIMEOUT_MS)
        {
            // Send ping if we didn't either send or receive something
            // for BLYNK_HEARTBEAT seconds
            sendCmd(BLYNK_CMD_PING);
            lastHeartbeat = t;
        }
    } else if (state == CONNECTING) {
#ifdef BLYNK_USE_DIRECT_CONNECT
        if (!tconn)
            conn.connect();
#else
        if (tconn && (t - lastLogin > BLYNK_TIMEOUT_MS)) {
            BLYNK_LOG1(BLYNK_F("Login timeout"));
            conn.disconnect();
            state = CONNECTING;
            return false;
        } else if (!tconn && (t - lastLogin > 5000UL)) {
            conn.disconnect();
            if (!conn.connect()) {
                lastLogin = t;
                return false;
            }

            msgIdOut = 1;
            sendCmd(BLYNK_CMD_LOGIN, 1, authkey, strlen(authkey));
            lastLogin = lastActivityOut;
            return true;
        }
#endif
    }
    return true;
}

template <class Transp>
BLYNK_FORCE_INLINE
bool BlynkProtocol<Transp>::processInput(void)
{
    BlynkHeader hdr;
    const int ret = readHeader(hdr);

    if (ret == 0) {
        return true; // Considered OK (no data on input)
    }

    if (ret < 0 || hdr.msg_id == 0) {
#ifdef BLYNK_DEBUG
        BLYNK_LOG2(BLYNK_F("Bad hdr len: "), ret);
#endif
        return false;
    }

    if (hdr.type == BLYNK_CMD_RESPONSE) {
        lastActivityIn = BlynkMillis();

#ifndef BLYNK_USE_DIRECT_CONNECT
        if (state == CONNECTING && (1 == hdr.msg_id)) {
            switch (hdr.length) {
            case BLYNK_SUCCESS:
            case BLYNK_ALREADY_REGISTERED:
                BLYNK_LOG3(BLYNK_F("Ready (ping: "), lastActivityIn-lastHeartbeat, BLYNK_F("ms)."));
                lastHeartbeat = lastActivityIn;
                state = CONNECTED;
#ifdef BLYNK_DEBUG
                if (size_t ram = BlynkFreeRam()) {
                    BLYNK_LOG2(BLYNK_F("Free RAM: "), ram);
                }
#endif
                this->sendInfo();
                BLYNK_RUN_YIELD();
                BlynkOnConnected();
                return true;
            case BLYNK_INVALID_TOKEN:
                BLYNK_LOG1(BLYNK_F("Invalid auth token"));
                break;
            default:
                BLYNK_LOG2(BLYNK_F("Connect failed. code: "), hdr.length);
            }
            return false;
        }
        if (BLYNK_NOT_AUTHENTICATED == hdr.length) {
            return false;
        }
#endif
        // TODO: return code may indicate App presence
        return true;
    }

    if (hdr.length > BLYNK_MAX_READBYTES) {
        BLYNK_LOG2(BLYNK_F("Packet too big: "), hdr.length);
        // TODO: Flush
        internalReconnect();
        return true;
    }

    uint8_t inputBuffer[hdr.length+1]; // Add 1 to zero-terminate
    if (hdr.length != conn.read(inputBuffer, hdr.length)) {
#ifdef BLYNK_DEBUG
        BLYNK_LOG1(BLYNK_F("Can't read body"));
#endif
        return false;
    }
    inputBuffer[hdr.length] = '\0';

    BLYNK_DBG_DUMP(">", inputBuffer, hdr.length);

    lastActivityIn = BlynkMillis();

    switch (hdr.type)
    {
    case BLYNK_CMD_LOGIN: {
#ifdef BLYNK_USE_DIRECT_CONNECT
        if (strncmp(authkey, (char*)inputBuffer, 32)) {
            BLYNK_LOG1(BLYNK_F("Invalid token"));
            sendCmd(BLYNK_CMD_RESPONSE, hdr.msg_id, NULL, BLYNK_INVALID_TOKEN);
            break;
        }
#endif
        if (state == CONNECTING) {
            BLYNK_LOG1(BLYNK_F("Ready"));
            state = CONNECTED;
#ifdef BLYNK_DEBUG
            if (size_t ram = BlynkFreeRam()) {
                BLYNK_LOG2(BLYNK_F("Free RAM: "), ram);
            }
#endif
            this->sendInfo();
            BLYNK_RUN_YIELD();
            BlynkOnConnected();
        }
        sendCmd(BLYNK_CMD_RESPONSE, hdr.msg_id, NULL, BLYNK_SUCCESS);
    } break;
    case BLYNK_CMD_PING: {
        sendCmd(BLYNK_CMD_RESPONSE, hdr.msg_id, NULL, BLYNK_SUCCESS);
    } break;
    case BLYNK_CMD_REDIRECT: {
        if (!redir_serv) {
             redir_serv = (char*)malloc(32);
        }
        BlynkParam param(inputBuffer, hdr.length);
        uint16_t redir_port = BLYNK_DEFAULT_PORT; // TODO: Fixit

        BlynkParam::iterator it = param.begin();
        if (it >= param.end())
            return false;
        strncpy(redir_serv, it.asStr(), 32);
        if (++it < param.end())
            redir_port = it.asLong();
        BLYNK_LOG4(BLYNK_F("Redirecting to "), redir_serv, ':', redir_port);
        conn.disconnect();
        conn.begin(redir_serv, redir_port);
        state = CONNECTING;
        lastHeartbeat = lastActivityIn = lastActivityOut = (BlynkMillis() - 5000UL);
    } break;
    case BLYNK_CMD_HARDWARE:
    case BLYNK_CMD_BRIDGE: {
        msgIdOutOverride = hdr.msg_id;
        this->processCmd(inputBuffer, hdr.length);
        msgIdOutOverride = 0;
    } break;
    case BLYNK_CMD_INTERNAL: {
        BlynkReq req = { 0 };
        BlynkParam param(inputBuffer, hdr.length);
        BlynkParam::iterator it = param.begin();
        if (it >= param.end())
            return true;

        uint32_t cmd32;
        memcpy(&cmd32, it.asStr(), sizeof(cmd32));

        ++it;
        char* start = (char*)(it).asStr();
        unsigned length = hdr.length - (start - (char*)inputBuffer);
        BlynkParam param2(start, length);

        switch (cmd32) {
        case BLYNK_INT_RTC:  BlynkWidgetWriteInternalPinRTC(req, param2);    break;
        case BLYNK_INT_OTA:  BlynkWidgetWriteInternalPinOTA(req, param2);    break;
        case BLYNK_INT_ACON: BlynkWidgetWriteInternalPinACON(req, param2);   break;
        case BLYNK_INT_ADIS: BlynkWidgetWriteInternalPinADIS(req, param2);   break;
#ifdef BLYNK_DEBUG
        default:             BLYNK_LOG2(BLYNK_F("Invalid internal cmd:"), param.asStr());
#endif
        }
    } break;
    case BLYNK_CMD_DEBUG_PRINT: {
        if (hdr.length) {
            BLYNK_LOG2(BLYNK_F("Server: "), (char*)inputBuffer);
        }
    } break;
    default: {
#ifdef BLYNK_DEBUG
        BLYNK_LOG2(BLYNK_F("Invalid header type: "), hdr.type);
#endif
        // TODO: Flush
        internalReconnect();
    } break;
    }

    return true;
}

template <class Transp>
int BlynkProtocol<Transp>::readHeader(BlynkHeader& hdr)
{
    size_t rlen = conn.read(&hdr, sizeof(hdr));
    if (rlen == 0) {
        return 0;
    }

    if (sizeof(hdr) != rlen) {
        return -1;
    }

    BLYNK_DBG_DUMP(">", &hdr, sizeof(BlynkHeader));

    hdr.msg_id = ntohs(hdr.msg_id);
    hdr.length = ntohs(hdr.length);

    return rlen;
}

#ifndef BLYNK_SEND_THROTTLE
#define BLYNK_SEND_THROTTLE 0
#endif

#ifndef BLYNK_SEND_CHUNK
#define BLYNK_SEND_CHUNK 1024 // Just a big number
#endif

template <class Transp>
void BlynkProtocol<Transp>::sendCmd(uint8_t cmd, uint16_t id, const void* data, size_t length, const void* data2, size_t length2)
{
    if (!conn.connected() || (cmd != BLYNK_CMD_RESPONSE && cmd != BLYNK_CMD_PING && cmd != BLYNK_CMD_LOGIN && state != CONNECTED) ) {
#ifdef BLYNK_DEBUG_ALL
        BLYNK_LOG2(BLYNK_F("Cmd skipped:"), cmd);
#endif
        return;
    }

    if (0 == id) {
        id = getNextMsgId();
    }

#if defined(BLYNK_MSG_LIMIT) && BLYNK_MSG_LIMIT > 0
    if (cmd >= BLYNK_CMD_TWEET && cmd <= BLYNK_CMD_HARDWARE) {
        const millis_time_t allowed_time = BlynkMax(lastActivityOut, lastActivityIn) + 1000/BLYNK_MSG_LIMIT;
        int32_t wait_time = allowed_time - BlynkMillis();
        if (wait_time >= 0) {
#ifdef BLYNK_DEBUG_ALL
            BLYNK_LOG2(BLYNK_F("Waiting:"), wait_time);
#endif
            while (wait_time >= 0) {
                run();
                wait_time = allowed_time - BlynkMillis();
            }
        } else if (nesting == 0) {
            run();
        }
    }
#endif

    const size_t full_length = (sizeof(BlynkHeader)) +
                               (data  ? length  : 0) +
                               (data2 ? length2 : 0);

#if defined(BLYNK_SEND_ATOMIC) || defined(ESP8266) || defined(ESP32) || defined(SPARK) || defined(PARTICLE) || defined(ENERGIA)
    // Those have more RAM and like single write at a time...

    uint8_t buff[full_length];

    BlynkHeader* hdr = (BlynkHeader*)buff;
    hdr->type = cmd;
    hdr->msg_id = htons(id);
    hdr->length = htons(length+length2);

    size_t pos = sizeof(BlynkHeader);
    if (data && length) {
        memcpy(buff + pos, data, length);
        pos += length;
    }
    if (data2 && length2) {
        memcpy(buff + pos, data2, length2);
    }

    size_t wlen = 0;
    while (wlen < full_length) {
        const size_t chunk = BlynkMin(size_t(BLYNK_SEND_CHUNK), full_length - wlen);
        BLYNK_DBG_DUMP("<", buff + wlen, chunk);
        const size_t w = conn.write(buff + wlen, chunk);
        BlynkDelay(BLYNK_SEND_THROTTLE);
        if (w == 0) {
#ifdef BLYNK_DEBUG
            BLYNK_LOG1(BLYNK_F("Cmd error"));
#endif
            conn.disconnect();
            state = CONNECTING;
            BlynkOnDisconnected();
            return;
        }
        wlen += w;
    }

#else

    BlynkHeader hdr;
    hdr.type = cmd;
    hdr.msg_id = htons(id);
    hdr.length = htons(length+length2);

    BLYNK_DBG_DUMP("<", &hdr, sizeof(hdr));
    size_t wlen = conn.write(&hdr, sizeof(hdr));
    BlynkDelay(BLYNK_SEND_THROTTLE);

    if (cmd != BLYNK_CMD_RESPONSE) {
        if (length) {
            BLYNK_DBG_DUMP("<", data, length);
            wlen += conn.write(data, length);
            BlynkDelay(BLYNK_SEND_THROTTLE);
        }
        if (length2) {
            BLYNK_DBG_DUMP("<", data2, length2);
            wlen += conn.write(data2, length2);
            BlynkDelay(BLYNK_SEND_THROTTLE);
        }
    }

#endif

    if (wlen != full_length) {
#ifdef BLYNK_DEBUG
        BLYNK_LOG4(BLYNK_F("Sent "), wlen, '/', full_length);
#endif
        internalReconnect();
        return;
    }

    lastActivityOut = BlynkMillis();

}

template <class Transp>
uint16_t BlynkProtocol<Transp>::getNextMsgId()
{
    if (msgIdOutOverride != 0)
        return msgIdOutOverride;
    if (++msgIdOut == 0)
        msgIdOut = 1;
    return msgIdOut;
}

#endif
