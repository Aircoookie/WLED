/**
 * @file       BlynkHandlers.cpp
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief      Virtual pin utilities
 */

#include "BlynkConfig.h"
#include "BlynkHandlers.h"
#include "BlynkDebug.h"

void BlynkNoOpCbk()
{}

void BlynkWidgetRead(BlynkReq BLYNK_UNUSED &request)
{
    BLYNK_LOG2(BLYNK_F("No handler for reading from pin "), request.pin);
}

void BlynkWidgetWrite(BlynkReq BLYNK_UNUSED &request, const BlynkParam BLYNK_UNUSED &param)
{
    BLYNK_LOG2(BLYNK_F("No handler for writing to pin "), request.pin);
}

#define BLYNK_ON_READ_IMPL(pin)  void BlynkWidgetRead  ## pin (BlynkReq BLYNK_UNUSED &req) \
          __attribute__((weak, alias("BlynkWidgetRead")))

#define BLYNK_ON_WRITE_IMPL(pin) void BlynkWidgetWrite ## pin (BlynkReq BLYNK_UNUSED &req, const BlynkParam BLYNK_UNUSED &param) \
          __attribute__((weak, alias("BlynkWidgetWrite")))

BLYNK_CONNECTED() __attribute__((weak, alias("BlynkNoOpCbk")));
BLYNK_DISCONNECTED() __attribute__((weak, alias("BlynkNoOpCbk")));

// Internal Virtual Pins
BLYNK_ON_WRITE_IMPL(InternalPinACON);
BLYNK_ON_WRITE_IMPL(InternalPinADIS);
BLYNK_ON_WRITE_IMPL(InternalPinRTC);
BLYNK_ON_WRITE_IMPL(InternalPinOTA);

// Regular Virtual Pins
BLYNK_ON_READ_IMPL(Default);
BLYNK_ON_WRITE_IMPL(Default);

BLYNK_ON_READ_IMPL(0 );
BLYNK_ON_READ_IMPL(1 );
BLYNK_ON_READ_IMPL(2 );
BLYNK_ON_READ_IMPL(3 );
BLYNK_ON_READ_IMPL(4 );
BLYNK_ON_READ_IMPL(5 );
BLYNK_ON_READ_IMPL(6 );
BLYNK_ON_READ_IMPL(7 );
BLYNK_ON_READ_IMPL(8 );
BLYNK_ON_READ_IMPL(9 );
BLYNK_ON_READ_IMPL(10);
BLYNK_ON_READ_IMPL(11);
BLYNK_ON_READ_IMPL(12);
BLYNK_ON_READ_IMPL(13);
BLYNK_ON_READ_IMPL(14);
BLYNK_ON_READ_IMPL(15);
BLYNK_ON_READ_IMPL(16);
BLYNK_ON_READ_IMPL(17);
BLYNK_ON_READ_IMPL(18);
BLYNK_ON_READ_IMPL(19);
BLYNK_ON_READ_IMPL(20);
BLYNK_ON_READ_IMPL(21);
BLYNK_ON_READ_IMPL(22);
BLYNK_ON_READ_IMPL(23);
BLYNK_ON_READ_IMPL(24);
BLYNK_ON_READ_IMPL(25);
BLYNK_ON_READ_IMPL(26);
BLYNK_ON_READ_IMPL(27);
BLYNK_ON_READ_IMPL(28);
BLYNK_ON_READ_IMPL(29);
BLYNK_ON_READ_IMPL(30);
BLYNK_ON_READ_IMPL(31);
#ifdef BLYNK_USE_128_VPINS
  BLYNK_ON_READ_IMPL(32);
  BLYNK_ON_READ_IMPL(33);
  BLYNK_ON_READ_IMPL(34);
  BLYNK_ON_READ_IMPL(35);
  BLYNK_ON_READ_IMPL(36);
  BLYNK_ON_READ_IMPL(37);
  BLYNK_ON_READ_IMPL(38);
  BLYNK_ON_READ_IMPL(39);
  BLYNK_ON_READ_IMPL(40);
  BLYNK_ON_READ_IMPL(41);
  BLYNK_ON_READ_IMPL(42);
  BLYNK_ON_READ_IMPL(43);
  BLYNK_ON_READ_IMPL(44);
  BLYNK_ON_READ_IMPL(45);
  BLYNK_ON_READ_IMPL(46);
  BLYNK_ON_READ_IMPL(47);
  BLYNK_ON_READ_IMPL(48);
  BLYNK_ON_READ_IMPL(49);
  BLYNK_ON_READ_IMPL(50);
  BLYNK_ON_READ_IMPL(51);
  BLYNK_ON_READ_IMPL(52);
  BLYNK_ON_READ_IMPL(53);
  BLYNK_ON_READ_IMPL(54);
  BLYNK_ON_READ_IMPL(55);
  BLYNK_ON_READ_IMPL(56);
  BLYNK_ON_READ_IMPL(57);
  BLYNK_ON_READ_IMPL(58);
  BLYNK_ON_READ_IMPL(59);
  BLYNK_ON_READ_IMPL(60);
  BLYNK_ON_READ_IMPL(61);
  BLYNK_ON_READ_IMPL(62);
  BLYNK_ON_READ_IMPL(63);
  BLYNK_ON_READ_IMPL(64);
  BLYNK_ON_READ_IMPL(65);
  BLYNK_ON_READ_IMPL(66);
  BLYNK_ON_READ_IMPL(67);
  BLYNK_ON_READ_IMPL(68);
  BLYNK_ON_READ_IMPL(69);
  BLYNK_ON_READ_IMPL(70);
  BLYNK_ON_READ_IMPL(71);
  BLYNK_ON_READ_IMPL(72);
  BLYNK_ON_READ_IMPL(73);
  BLYNK_ON_READ_IMPL(74);
  BLYNK_ON_READ_IMPL(75);
  BLYNK_ON_READ_IMPL(76);
  BLYNK_ON_READ_IMPL(77);
  BLYNK_ON_READ_IMPL(78);
  BLYNK_ON_READ_IMPL(79);
  BLYNK_ON_READ_IMPL(80);
  BLYNK_ON_READ_IMPL(81);
  BLYNK_ON_READ_IMPL(82);
  BLYNK_ON_READ_IMPL(83);
  BLYNK_ON_READ_IMPL(84);
  BLYNK_ON_READ_IMPL(85);
  BLYNK_ON_READ_IMPL(86);
  BLYNK_ON_READ_IMPL(87);
  BLYNK_ON_READ_IMPL(88);
  BLYNK_ON_READ_IMPL(89);
  BLYNK_ON_READ_IMPL(90);
  BLYNK_ON_READ_IMPL(91);
  BLYNK_ON_READ_IMPL(92);
  BLYNK_ON_READ_IMPL(93);
  BLYNK_ON_READ_IMPL(94);
  BLYNK_ON_READ_IMPL(95);
  BLYNK_ON_READ_IMPL(96);
  BLYNK_ON_READ_IMPL(97);
  BLYNK_ON_READ_IMPL(98);
  BLYNK_ON_READ_IMPL(99);
  BLYNK_ON_READ_IMPL(100);
  BLYNK_ON_READ_IMPL(101);
  BLYNK_ON_READ_IMPL(102);
  BLYNK_ON_READ_IMPL(103);
  BLYNK_ON_READ_IMPL(104);
  BLYNK_ON_READ_IMPL(105);
  BLYNK_ON_READ_IMPL(106);
  BLYNK_ON_READ_IMPL(107);
  BLYNK_ON_READ_IMPL(108);
  BLYNK_ON_READ_IMPL(109);
  BLYNK_ON_READ_IMPL(110);
  BLYNK_ON_READ_IMPL(111);
  BLYNK_ON_READ_IMPL(112);
  BLYNK_ON_READ_IMPL(113);
  BLYNK_ON_READ_IMPL(114);
  BLYNK_ON_READ_IMPL(115);
  BLYNK_ON_READ_IMPL(116);
  BLYNK_ON_READ_IMPL(117);
  BLYNK_ON_READ_IMPL(118);
  BLYNK_ON_READ_IMPL(119);
  BLYNK_ON_READ_IMPL(120);
  BLYNK_ON_READ_IMPL(121);
  BLYNK_ON_READ_IMPL(122);
  BLYNK_ON_READ_IMPL(123);
  BLYNK_ON_READ_IMPL(124);
  BLYNK_ON_READ_IMPL(125);
  BLYNK_ON_READ_IMPL(126);
  BLYNK_ON_READ_IMPL(127);
#endif

BLYNK_ON_WRITE_IMPL(0 );
BLYNK_ON_WRITE_IMPL(1 );
BLYNK_ON_WRITE_IMPL(2 );
BLYNK_ON_WRITE_IMPL(3 );
BLYNK_ON_WRITE_IMPL(4 );
BLYNK_ON_WRITE_IMPL(5 );
BLYNK_ON_WRITE_IMPL(6 );
BLYNK_ON_WRITE_IMPL(7 );
BLYNK_ON_WRITE_IMPL(8 );
BLYNK_ON_WRITE_IMPL(9 );
BLYNK_ON_WRITE_IMPL(10);
BLYNK_ON_WRITE_IMPL(11);
BLYNK_ON_WRITE_IMPL(12);
BLYNK_ON_WRITE_IMPL(13);
BLYNK_ON_WRITE_IMPL(14);
BLYNK_ON_WRITE_IMPL(15);
BLYNK_ON_WRITE_IMPL(16);
BLYNK_ON_WRITE_IMPL(17);
BLYNK_ON_WRITE_IMPL(18);
BLYNK_ON_WRITE_IMPL(19);
BLYNK_ON_WRITE_IMPL(20);
BLYNK_ON_WRITE_IMPL(21);
BLYNK_ON_WRITE_IMPL(22);
BLYNK_ON_WRITE_IMPL(23);
BLYNK_ON_WRITE_IMPL(24);
BLYNK_ON_WRITE_IMPL(25);
BLYNK_ON_WRITE_IMPL(26);
BLYNK_ON_WRITE_IMPL(27);
BLYNK_ON_WRITE_IMPL(28);
BLYNK_ON_WRITE_IMPL(29);
BLYNK_ON_WRITE_IMPL(30);
BLYNK_ON_WRITE_IMPL(31);
#ifdef BLYNK_USE_128_VPINS
  BLYNK_ON_WRITE_IMPL(32);
  BLYNK_ON_WRITE_IMPL(33);
  BLYNK_ON_WRITE_IMPL(34);
  BLYNK_ON_WRITE_IMPL(35);
  BLYNK_ON_WRITE_IMPL(36);
  BLYNK_ON_WRITE_IMPL(37);
  BLYNK_ON_WRITE_IMPL(38);
  BLYNK_ON_WRITE_IMPL(39);
  BLYNK_ON_WRITE_IMPL(40);
  BLYNK_ON_WRITE_IMPL(41);
  BLYNK_ON_WRITE_IMPL(42);
  BLYNK_ON_WRITE_IMPL(43);
  BLYNK_ON_WRITE_IMPL(44);
  BLYNK_ON_WRITE_IMPL(45);
  BLYNK_ON_WRITE_IMPL(46);
  BLYNK_ON_WRITE_IMPL(47);
  BLYNK_ON_WRITE_IMPL(48);
  BLYNK_ON_WRITE_IMPL(49);
  BLYNK_ON_WRITE_IMPL(50);
  BLYNK_ON_WRITE_IMPL(51);
  BLYNK_ON_WRITE_IMPL(52);
  BLYNK_ON_WRITE_IMPL(53);
  BLYNK_ON_WRITE_IMPL(54);
  BLYNK_ON_WRITE_IMPL(55);
  BLYNK_ON_WRITE_IMPL(56);
  BLYNK_ON_WRITE_IMPL(57);
  BLYNK_ON_WRITE_IMPL(58);
  BLYNK_ON_WRITE_IMPL(59);
  BLYNK_ON_WRITE_IMPL(60);
  BLYNK_ON_WRITE_IMPL(61);
  BLYNK_ON_WRITE_IMPL(62);
  BLYNK_ON_WRITE_IMPL(63);
  BLYNK_ON_WRITE_IMPL(64);
  BLYNK_ON_WRITE_IMPL(65);
  BLYNK_ON_WRITE_IMPL(66);
  BLYNK_ON_WRITE_IMPL(67);
  BLYNK_ON_WRITE_IMPL(68);
  BLYNK_ON_WRITE_IMPL(69);
  BLYNK_ON_WRITE_IMPL(70);
  BLYNK_ON_WRITE_IMPL(71);
  BLYNK_ON_WRITE_IMPL(72);
  BLYNK_ON_WRITE_IMPL(73);
  BLYNK_ON_WRITE_IMPL(74);
  BLYNK_ON_WRITE_IMPL(75);
  BLYNK_ON_WRITE_IMPL(76);
  BLYNK_ON_WRITE_IMPL(77);
  BLYNK_ON_WRITE_IMPL(78);
  BLYNK_ON_WRITE_IMPL(79);
  BLYNK_ON_WRITE_IMPL(80);
  BLYNK_ON_WRITE_IMPL(81);
  BLYNK_ON_WRITE_IMPL(82);
  BLYNK_ON_WRITE_IMPL(83);
  BLYNK_ON_WRITE_IMPL(84);
  BLYNK_ON_WRITE_IMPL(85);
  BLYNK_ON_WRITE_IMPL(86);
  BLYNK_ON_WRITE_IMPL(87);
  BLYNK_ON_WRITE_IMPL(88);
  BLYNK_ON_WRITE_IMPL(89);
  BLYNK_ON_WRITE_IMPL(90);
  BLYNK_ON_WRITE_IMPL(91);
  BLYNK_ON_WRITE_IMPL(92);
  BLYNK_ON_WRITE_IMPL(93);
  BLYNK_ON_WRITE_IMPL(94);
  BLYNK_ON_WRITE_IMPL(95);
  BLYNK_ON_WRITE_IMPL(96);
  BLYNK_ON_WRITE_IMPL(97);
  BLYNK_ON_WRITE_IMPL(98);
  BLYNK_ON_WRITE_IMPL(99);
  BLYNK_ON_WRITE_IMPL(100);
  BLYNK_ON_WRITE_IMPL(101);
  BLYNK_ON_WRITE_IMPL(102);
  BLYNK_ON_WRITE_IMPL(103);
  BLYNK_ON_WRITE_IMPL(104);
  BLYNK_ON_WRITE_IMPL(105);
  BLYNK_ON_WRITE_IMPL(106);
  BLYNK_ON_WRITE_IMPL(107);
  BLYNK_ON_WRITE_IMPL(108);
  BLYNK_ON_WRITE_IMPL(109);
  BLYNK_ON_WRITE_IMPL(110);
  BLYNK_ON_WRITE_IMPL(111);
  BLYNK_ON_WRITE_IMPL(112);
  BLYNK_ON_WRITE_IMPL(113);
  BLYNK_ON_WRITE_IMPL(114);
  BLYNK_ON_WRITE_IMPL(115);
  BLYNK_ON_WRITE_IMPL(116);
  BLYNK_ON_WRITE_IMPL(117);
  BLYNK_ON_WRITE_IMPL(118);
  BLYNK_ON_WRITE_IMPL(119);
  BLYNK_ON_WRITE_IMPL(120);
  BLYNK_ON_WRITE_IMPL(121);
  BLYNK_ON_WRITE_IMPL(122);
  BLYNK_ON_WRITE_IMPL(123);
  BLYNK_ON_WRITE_IMPL(124);
  BLYNK_ON_WRITE_IMPL(125);
  BLYNK_ON_WRITE_IMPL(126);
  BLYNK_ON_WRITE_IMPL(127);
#endif

static const WidgetReadHandler BlynkReadHandlerVector[] BLYNK_PROGMEM = {
    BlynkWidgetRead0,   BlynkWidgetRead1,   BlynkWidgetRead2,   BlynkWidgetRead3,
    BlynkWidgetRead4,   BlynkWidgetRead5,   BlynkWidgetRead6,   BlynkWidgetRead7,
    BlynkWidgetRead8,   BlynkWidgetRead9,   BlynkWidgetRead10,  BlynkWidgetRead11,
    BlynkWidgetRead12,  BlynkWidgetRead13,  BlynkWidgetRead14,  BlynkWidgetRead15,
    BlynkWidgetRead16,  BlynkWidgetRead17,  BlynkWidgetRead18,  BlynkWidgetRead19,
    BlynkWidgetRead20,  BlynkWidgetRead21,  BlynkWidgetRead22,  BlynkWidgetRead23,
    BlynkWidgetRead24,  BlynkWidgetRead25,  BlynkWidgetRead26,  BlynkWidgetRead27,
    BlynkWidgetRead28,  BlynkWidgetRead29,  BlynkWidgetRead30,  BlynkWidgetRead31,
#ifdef BLYNK_USE_128_VPINS
    BlynkWidgetRead32,  BlynkWidgetRead33,  BlynkWidgetRead34,  BlynkWidgetRead35,
    BlynkWidgetRead36,  BlynkWidgetRead37,  BlynkWidgetRead38,  BlynkWidgetRead39,
    BlynkWidgetRead40,  BlynkWidgetRead41,  BlynkWidgetRead42,  BlynkWidgetRead43,
    BlynkWidgetRead44,  BlynkWidgetRead45,  BlynkWidgetRead46,  BlynkWidgetRead47,
    BlynkWidgetRead48,  BlynkWidgetRead49,  BlynkWidgetRead50,  BlynkWidgetRead51,
    BlynkWidgetRead52,  BlynkWidgetRead53,  BlynkWidgetRead54,  BlynkWidgetRead55,
    BlynkWidgetRead56,  BlynkWidgetRead57,  BlynkWidgetRead58,  BlynkWidgetRead59,
    BlynkWidgetRead60,  BlynkWidgetRead61,  BlynkWidgetRead62,  BlynkWidgetRead63,
    BlynkWidgetRead64,  BlynkWidgetRead65,  BlynkWidgetRead66,  BlynkWidgetRead67,
    BlynkWidgetRead68,  BlynkWidgetRead69,  BlynkWidgetRead70,  BlynkWidgetRead71,
    BlynkWidgetRead72,  BlynkWidgetRead73,  BlynkWidgetRead74,  BlynkWidgetRead75,
    BlynkWidgetRead76,  BlynkWidgetRead77,  BlynkWidgetRead78,  BlynkWidgetRead79,
    BlynkWidgetRead80,  BlynkWidgetRead81,  BlynkWidgetRead82,  BlynkWidgetRead83,
    BlynkWidgetRead84,  BlynkWidgetRead85,  BlynkWidgetRead86,  BlynkWidgetRead87,
    BlynkWidgetRead88,  BlynkWidgetRead89,  BlynkWidgetRead90,  BlynkWidgetRead91,
    BlynkWidgetRead92,  BlynkWidgetRead93,  BlynkWidgetRead94,  BlynkWidgetRead95,
    BlynkWidgetRead96,  BlynkWidgetRead97,  BlynkWidgetRead98,  BlynkWidgetRead99,
    BlynkWidgetRead100,  BlynkWidgetRead101,  BlynkWidgetRead102,  BlynkWidgetRead103,
    BlynkWidgetRead104,  BlynkWidgetRead105,  BlynkWidgetRead106,  BlynkWidgetRead107,
    BlynkWidgetRead108,  BlynkWidgetRead109,  BlynkWidgetRead110,  BlynkWidgetRead111,
    BlynkWidgetRead112,  BlynkWidgetRead113,  BlynkWidgetRead114,  BlynkWidgetRead115,
    BlynkWidgetRead116,  BlynkWidgetRead117,  BlynkWidgetRead118,  BlynkWidgetRead119,
    BlynkWidgetRead120,  BlynkWidgetRead121,  BlynkWidgetRead122,  BlynkWidgetRead123,
    BlynkWidgetRead124,  BlynkWidgetRead125,  BlynkWidgetRead126,  BlynkWidgetRead127,
#endif
};

static const WidgetWriteHandler BlynkWriteHandlerVector[] BLYNK_PROGMEM = {
    BlynkWidgetWrite0,  BlynkWidgetWrite1,  BlynkWidgetWrite2,  BlynkWidgetWrite3,
    BlynkWidgetWrite4,  BlynkWidgetWrite5,  BlynkWidgetWrite6,  BlynkWidgetWrite7,
    BlynkWidgetWrite8,  BlynkWidgetWrite9,  BlynkWidgetWrite10, BlynkWidgetWrite11,
    BlynkWidgetWrite12, BlynkWidgetWrite13, BlynkWidgetWrite14, BlynkWidgetWrite15,
    BlynkWidgetWrite16, BlynkWidgetWrite17, BlynkWidgetWrite18, BlynkWidgetWrite19,
    BlynkWidgetWrite20, BlynkWidgetWrite21, BlynkWidgetWrite22, BlynkWidgetWrite23,
    BlynkWidgetWrite24, BlynkWidgetWrite25, BlynkWidgetWrite26, BlynkWidgetWrite27,
    BlynkWidgetWrite28, BlynkWidgetWrite29, BlynkWidgetWrite30, BlynkWidgetWrite31,
#ifdef BLYNK_USE_128_VPINS
    BlynkWidgetWrite32,  BlynkWidgetWrite33,  BlynkWidgetWrite34,  BlynkWidgetWrite35,
    BlynkWidgetWrite36,  BlynkWidgetWrite37,  BlynkWidgetWrite38,  BlynkWidgetWrite39,
    BlynkWidgetWrite40,  BlynkWidgetWrite41,  BlynkWidgetWrite42,  BlynkWidgetWrite43,
    BlynkWidgetWrite44,  BlynkWidgetWrite45,  BlynkWidgetWrite46,  BlynkWidgetWrite47,
    BlynkWidgetWrite48,  BlynkWidgetWrite49,  BlynkWidgetWrite50,  BlynkWidgetWrite51,
    BlynkWidgetWrite52,  BlynkWidgetWrite53,  BlynkWidgetWrite54,  BlynkWidgetWrite55,
    BlynkWidgetWrite56,  BlynkWidgetWrite57,  BlynkWidgetWrite58,  BlynkWidgetWrite59,
    BlynkWidgetWrite60,  BlynkWidgetWrite61,  BlynkWidgetWrite62,  BlynkWidgetWrite63,
    BlynkWidgetWrite64,  BlynkWidgetWrite65,  BlynkWidgetWrite66,  BlynkWidgetWrite67,
    BlynkWidgetWrite68,  BlynkWidgetWrite69,  BlynkWidgetWrite70,  BlynkWidgetWrite71,
    BlynkWidgetWrite72,  BlynkWidgetWrite73,  BlynkWidgetWrite74,  BlynkWidgetWrite75,
    BlynkWidgetWrite76,  BlynkWidgetWrite77,  BlynkWidgetWrite78,  BlynkWidgetWrite79,
    BlynkWidgetWrite80,  BlynkWidgetWrite81,  BlynkWidgetWrite82,  BlynkWidgetWrite83,
    BlynkWidgetWrite84,  BlynkWidgetWrite85,  BlynkWidgetWrite86,  BlynkWidgetWrite87,
    BlynkWidgetWrite88,  BlynkWidgetWrite89,  BlynkWidgetWrite90,  BlynkWidgetWrite91,
    BlynkWidgetWrite92,  BlynkWidgetWrite93,  BlynkWidgetWrite94,  BlynkWidgetWrite95,
    BlynkWidgetWrite96,  BlynkWidgetWrite97,  BlynkWidgetWrite98,  BlynkWidgetWrite99,
    BlynkWidgetWrite100,  BlynkWidgetWrite101,  BlynkWidgetWrite102,  BlynkWidgetWrite103,
    BlynkWidgetWrite104,  BlynkWidgetWrite105,  BlynkWidgetWrite106,  BlynkWidgetWrite107,
    BlynkWidgetWrite108,  BlynkWidgetWrite109,  BlynkWidgetWrite110,  BlynkWidgetWrite111,
    BlynkWidgetWrite112,  BlynkWidgetWrite113,  BlynkWidgetWrite114,  BlynkWidgetWrite115,
    BlynkWidgetWrite116,  BlynkWidgetWrite117,  BlynkWidgetWrite118,  BlynkWidgetWrite119,
    BlynkWidgetWrite120,  BlynkWidgetWrite121,  BlynkWidgetWrite122,  BlynkWidgetWrite123,
    BlynkWidgetWrite124,  BlynkWidgetWrite125,  BlynkWidgetWrite126,  BlynkWidgetWrite127,
#endif
};

WidgetReadHandler GetReadHandler(uint8_t pin)
{
    if (pin >= BLYNK_COUNT_OF(BlynkReadHandlerVector))
        return NULL;
#ifdef BLYNK_HAS_PROGMEM
    return (WidgetReadHandler)pgm_read_word(&BlynkReadHandlerVector[pin]);
#else
    return BlynkReadHandlerVector[pin];
#endif
}

WidgetWriteHandler GetWriteHandler(uint8_t pin)
{
    if (pin >= BLYNK_COUNT_OF(BlynkWriteHandlerVector))
        return NULL;
#ifdef BLYNK_HAS_PROGMEM
    return (WidgetWriteHandler)pgm_read_word(&BlynkWriteHandlerVector[pin]);
#else
    return BlynkWriteHandlerVector[pin];
#endif
}
