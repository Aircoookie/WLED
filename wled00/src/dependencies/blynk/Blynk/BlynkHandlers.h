/**
 * @file       BlynkHandlers.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief      Handlers for virtual pin operations
 *
 */

#ifndef BlynkHandlers_h
#define BlynkHandlers_h

#include "BlynkConfig.h"
#include "BlynkParam.h"

// Helper macro

#define V0  0
#define V1  1
#define V2  2
#define V3  3
#define V4  4
#define V5  5
#define V6  6
#define V7  7
#define V8  8
#define V9  9
#define V10 10
#define V11 11
#define V12 12
#define V13 13
#define V14 14
#define V15 15
#define V16 16
#define V17 17
#define V18 18
#define V19 19
#define V20 20
#define V21 21
#define V22 22
#define V23 23
#define V24 24
#define V25 25
#define V26 26
#define V27 27
#define V28 28
#define V29 29
#define V30 30
#define V31 31
#ifdef BLYNK_USE_128_VPINS
  #define V32 32
  #define V33 33
  #define V34 34
  #define V35 35
  #define V36 36
  #define V37 37
  #define V38 38
  #define V39 39
  #define V40 40
  #define V41 41
  #define V42 42
  #define V43 43
  #define V44 44
  #define V45 45
  #define V46 46
  #define V47 47
  #define V48 48
  #define V49 49
  #define V50 50
  #define V51 51
  #define V52 52
  #define V53 53
  #define V54 54
  #define V55 55
  #define V56 56
  #define V57 57
  #define V58 58
  #define V59 59
  #define V60 60
  #define V61 61
  #define V62 62
  #define V63 63
  #define V64 64
  #define V65 65
  #define V66 66
  #define V67 67
  #define V68 68
  #define V69 69
  #define V70 70
  #define V71 71
  #define V72 72
  #define V73 73
  #define V74 74
  #define V75 75
  #define V76 76
  #define V77 77
  #define V78 78
  #define V79 79
  #define V80 80
  #define V81 81
  #define V82 82
  #define V83 83
  #define V84 84
  #define V85 85
  #define V86 86
  #define V87 87
  #define V88 88
  #define V89 89
  #define V90 90
  #define V91 91
  #define V92 92
  #define V93 93
  #define V94 94
  #define V95 95
  #define V96 96
  #define V97 97
  #define V98 98
  #define V99 99
  #define V100 100
  #define V101 101
  #define V102 102
  #define V103 103
  #define V104 104
  #define V105 105
  #define V106 106
  #define V107 107
  #define V108 108
  #define V109 109
  #define V110 110
  #define V111 111
  #define V112 112
  #define V113 113
  #define V114 114
  #define V115 115
  #define V116 116
  #define V117 117
  #define V118 118
  #define V119 119
  #define V120 120
  #define V121 121
  #define V122 122
  #define V123 123
  #define V124 124
  #define V125 125
  #define V126 126
  #define V127 127
#endif

// Initial syntax:
#define BLYNK_WRITE_2(pin) \
    void BlynkWidgetWrite ## pin (BlynkReq BLYNK_UNUSED &request, const BlynkParam BLYNK_UNUSED &param)

#define BLYNK_READ_2(pin)  \
    void BlynkWidgetRead ## pin  (BlynkReq BLYNK_UNUSED &request)

#define BLYNK_WRITE_DEFAULT() BLYNK_WRITE_2(Default)
#define BLYNK_READ_DEFAULT()  BLYNK_READ_2(Default)

#define BLYNK_WRITE(pin)      BLYNK_WRITE_2(pin)
#define BLYNK_READ(pin)       BLYNK_READ_2(pin)

// New, more readable syntax:
#define BLYNK_IN_2(pin)  \
    void BlynkWidgetWrite ## pin (BlynkReq BLYNK_UNUSED &request, const BlynkParam BLYNK_UNUSED &getValue)

#define BLYNK_OUT_2(pin) \
    void BlynkWidgetRead ## pin  (BlynkReq BLYNK_UNUSED &request)

#define BLYNK_INPUT_DEFAULT()   BLYNK_IN_2(Default)
#define BLYNK_OUTPUT_DEFAULT()  BLYNK_OUT_2(Default)

#define BLYNK_INPUT(pin)        BLYNK_IN_2(pin)
#define BLYNK_OUTPUT(pin)       BLYNK_OUT_2(pin)

// Additional handlers
#define BLYNK_CONNECTED()    void BlynkOnConnected()
#define BLYNK_DISCONNECTED() void BlynkOnDisconnected()

// Advanced functions

#define BLYNK_VAR_INT(name, pin) \
    int name;  \
    BLYNK_WRITE(pin) { name = param.asInt(); } \
    BLYNK_READ(pin)  { Blynk.virtualWrite(pin, name); }

#define BLYNK_VAR_LONG(name, pin) \
    long name;  \
    BLYNK_WRITE(pin) { name = param.asLong(); } \
    BLYNK_READ(pin)  { Blynk.virtualWrite(pin, name); }

#ifndef BLYNK_NO_FLOAT
#define BLYNK_VAR_DOUBLE(name, pin) \
    double name;  \
    BLYNK_WRITE(pin) { name = param.asDouble(); } \
    BLYNK_READ(pin)  { Blynk.virtualWrite(pin, name); }
#endif

#ifdef ARDUINO
#define BLYNK_VAR_STRING(name, pin) \
    String name;  \
    BLYNK_WRITE(pin) { name = param.asStr(); } \
    BLYNK_READ(pin)  { Blynk.virtualWrite(pin, name); }
#endif

// Default read/write handlers (you can redefine them in your code)
#ifdef __cplusplus
extern "C" {
#endif

struct BlynkReq
{
    uint8_t pin;
};

typedef void (*WidgetReadHandler)(BlynkReq BLYNK_UNUSED &request);
typedef void (*WidgetWriteHandler)(BlynkReq BLYNK_UNUSED &request, const BlynkParam BLYNK_UNUSED &param);

WidgetReadHandler GetReadHandler(uint8_t pin);
WidgetWriteHandler GetWriteHandler(uint8_t pin);

// Declare placeholders
BLYNK_READ();
BLYNK_WRITE();
void BlynkNoOpCbk();

// Declare all pin handlers (you can redefine them in your code)
BLYNK_CONNECTED();
BLYNK_DISCONNECTED();

// Internal Virtual Pins
BLYNK_WRITE(InternalPinACON);
BLYNK_WRITE(InternalPinADIS);
BLYNK_WRITE(InternalPinRTC);
BLYNK_WRITE(InternalPinOTA);

// Aliases
#define BLYNK_APP_CONNECTED()    BLYNK_WRITE(InternalPinACON)
#define BLYNK_APP_DISCONNECTED() BLYNK_WRITE(InternalPinADIS)

// Regular Virtual Pins
BLYNK_READ_DEFAULT();
BLYNK_WRITE_DEFAULT();

BLYNK_READ(0 );
BLYNK_READ(1 );
BLYNK_READ(2 );
BLYNK_READ(3 );
BLYNK_READ(4 );
BLYNK_READ(5 );
BLYNK_READ(6 );
BLYNK_READ(7 );
BLYNK_READ(8 );
BLYNK_READ(9 );
BLYNK_READ(10);
BLYNK_READ(11);
BLYNK_READ(12);
BLYNK_READ(13);
BLYNK_READ(14);
BLYNK_READ(15);
BLYNK_READ(16);
BLYNK_READ(17);
BLYNK_READ(18);
BLYNK_READ(19);
BLYNK_READ(20);
BLYNK_READ(21);
BLYNK_READ(22);
BLYNK_READ(23);
BLYNK_READ(24);
BLYNK_READ(25);
BLYNK_READ(26);
BLYNK_READ(27);
BLYNK_READ(28);
BLYNK_READ(29);
BLYNK_READ(30);
BLYNK_READ(31);
#ifdef BLYNK_USE_128_VPINS
  BLYNK_READ(32);
  BLYNK_READ(33);
  BLYNK_READ(34);
  BLYNK_READ(35);
  BLYNK_READ(36);
  BLYNK_READ(37);
  BLYNK_READ(38);
  BLYNK_READ(39);
  BLYNK_READ(40);
  BLYNK_READ(41);
  BLYNK_READ(42);
  BLYNK_READ(43);
  BLYNK_READ(44);
  BLYNK_READ(45);
  BLYNK_READ(46);
  BLYNK_READ(47);
  BLYNK_READ(48);
  BLYNK_READ(49);
  BLYNK_READ(50);
  BLYNK_READ(51);
  BLYNK_READ(52);
  BLYNK_READ(53);
  BLYNK_READ(54);
  BLYNK_READ(55);
  BLYNK_READ(56);
  BLYNK_READ(57);
  BLYNK_READ(58);
  BLYNK_READ(59);
  BLYNK_READ(60);
  BLYNK_READ(61);
  BLYNK_READ(62);
  BLYNK_READ(63);
  BLYNK_READ(64);
  BLYNK_READ(65);
  BLYNK_READ(66);
  BLYNK_READ(67);
  BLYNK_READ(68);
  BLYNK_READ(69);
  BLYNK_READ(70);
  BLYNK_READ(71);
  BLYNK_READ(72);
  BLYNK_READ(73);
  BLYNK_READ(74);
  BLYNK_READ(75);
  BLYNK_READ(76);
  BLYNK_READ(77);
  BLYNK_READ(78);
  BLYNK_READ(79);
  BLYNK_READ(80);
  BLYNK_READ(81);
  BLYNK_READ(82);
  BLYNK_READ(83);
  BLYNK_READ(84);
  BLYNK_READ(85);
  BLYNK_READ(86);
  BLYNK_READ(87);
  BLYNK_READ(88);
  BLYNK_READ(89);
  BLYNK_READ(90);
  BLYNK_READ(91);
  BLYNK_READ(92);
  BLYNK_READ(93);
  BLYNK_READ(94);
  BLYNK_READ(95);
  BLYNK_READ(96);
  BLYNK_READ(97);
  BLYNK_READ(98);
  BLYNK_READ(99);
  BLYNK_READ(100);
  BLYNK_READ(101);
  BLYNK_READ(102);
  BLYNK_READ(103);
  BLYNK_READ(104);
  BLYNK_READ(105);
  BLYNK_READ(106);
  BLYNK_READ(107);
  BLYNK_READ(108);
  BLYNK_READ(109);
  BLYNK_READ(110);
  BLYNK_READ(111);
  BLYNK_READ(112);
  BLYNK_READ(113);
  BLYNK_READ(114);
  BLYNK_READ(115);
  BLYNK_READ(116);
  BLYNK_READ(117);
  BLYNK_READ(118);
  BLYNK_READ(119);
  BLYNK_READ(120);
  BLYNK_READ(121);
  BLYNK_READ(122);
  BLYNK_READ(123);
  BLYNK_READ(124);
  BLYNK_READ(125);
  BLYNK_READ(126);
  BLYNK_READ(127);
#endif

BLYNK_WRITE(0 );
BLYNK_WRITE(1 );
BLYNK_WRITE(2 );
BLYNK_WRITE(3 );
BLYNK_WRITE(4 );
BLYNK_WRITE(5 );
BLYNK_WRITE(6 );
BLYNK_WRITE(7 );
BLYNK_WRITE(8 );
BLYNK_WRITE(9 );
BLYNK_WRITE(10);
BLYNK_WRITE(11);
BLYNK_WRITE(12);
BLYNK_WRITE(13);
BLYNK_WRITE(14);
BLYNK_WRITE(15);
BLYNK_WRITE(16);
BLYNK_WRITE(17);
BLYNK_WRITE(18);
BLYNK_WRITE(19);
BLYNK_WRITE(20);
BLYNK_WRITE(21);
BLYNK_WRITE(22);
BLYNK_WRITE(23);
BLYNK_WRITE(24);
BLYNK_WRITE(25);
BLYNK_WRITE(26);
BLYNK_WRITE(27);
BLYNK_WRITE(28);
BLYNK_WRITE(29);
BLYNK_WRITE(30);
BLYNK_WRITE(31);
#ifdef BLYNK_USE_128_VPINS
  BLYNK_WRITE(32);
  BLYNK_WRITE(33);
  BLYNK_WRITE(34);
  BLYNK_WRITE(35);
  BLYNK_WRITE(36);
  BLYNK_WRITE(37);
  BLYNK_WRITE(38);
  BLYNK_WRITE(39);
  BLYNK_WRITE(40);
  BLYNK_WRITE(41);
  BLYNK_WRITE(42);
  BLYNK_WRITE(43);
  BLYNK_WRITE(44);
  BLYNK_WRITE(45);
  BLYNK_WRITE(46);
  BLYNK_WRITE(47);
  BLYNK_WRITE(48);
  BLYNK_WRITE(49);
  BLYNK_WRITE(50);
  BLYNK_WRITE(51);
  BLYNK_WRITE(52);
  BLYNK_WRITE(53);
  BLYNK_WRITE(54);
  BLYNK_WRITE(55);
  BLYNK_WRITE(56);
  BLYNK_WRITE(57);
  BLYNK_WRITE(58);
  BLYNK_WRITE(59);
  BLYNK_WRITE(60);
  BLYNK_WRITE(61);
  BLYNK_WRITE(62);
  BLYNK_WRITE(63);
  BLYNK_WRITE(64);
  BLYNK_WRITE(65);
  BLYNK_WRITE(66);
  BLYNK_WRITE(67);
  BLYNK_WRITE(68);
  BLYNK_WRITE(69);
  BLYNK_WRITE(70);
  BLYNK_WRITE(71);
  BLYNK_WRITE(72);
  BLYNK_WRITE(73);
  BLYNK_WRITE(74);
  BLYNK_WRITE(75);
  BLYNK_WRITE(76);
  BLYNK_WRITE(77);
  BLYNK_WRITE(78);
  BLYNK_WRITE(79);
  BLYNK_WRITE(80);
  BLYNK_WRITE(81);
  BLYNK_WRITE(82);
  BLYNK_WRITE(83);
  BLYNK_WRITE(84);
  BLYNK_WRITE(85);
  BLYNK_WRITE(86);
  BLYNK_WRITE(87);
  BLYNK_WRITE(88);
  BLYNK_WRITE(89);
  BLYNK_WRITE(90);
  BLYNK_WRITE(91);
  BLYNK_WRITE(92);
  BLYNK_WRITE(93);
  BLYNK_WRITE(94);
  BLYNK_WRITE(95);
  BLYNK_WRITE(96);
  BLYNK_WRITE(97);
  BLYNK_WRITE(98);
  BLYNK_WRITE(99);
  BLYNK_WRITE(100);
  BLYNK_WRITE(101);
  BLYNK_WRITE(102);
  BLYNK_WRITE(103);
  BLYNK_WRITE(104);
  BLYNK_WRITE(105);
  BLYNK_WRITE(106);
  BLYNK_WRITE(107);
  BLYNK_WRITE(108);
  BLYNK_WRITE(109);
  BLYNK_WRITE(110);
  BLYNK_WRITE(111);
  BLYNK_WRITE(112);
  BLYNK_WRITE(113);
  BLYNK_WRITE(114);
  BLYNK_WRITE(115);
  BLYNK_WRITE(116);
  BLYNK_WRITE(117);
  BLYNK_WRITE(118);
  BLYNK_WRITE(119);
  BLYNK_WRITE(120);
  BLYNK_WRITE(121);
  BLYNK_WRITE(122);
  BLYNK_WRITE(123);
  BLYNK_WRITE(124);
  BLYNK_WRITE(125);
  BLYNK_WRITE(126);
  BLYNK_WRITE(127);
#endif

#ifdef __cplusplus
}
#endif

#endif
