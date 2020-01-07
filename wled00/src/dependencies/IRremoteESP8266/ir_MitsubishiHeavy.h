// Copyright 2019 David Conran

// Supports:
//   Brand: Mitsubishi Heavy Industries,  Model: RLA502A700B remote
//   Brand: Mitsubishi Heavy Industries,  Model: SRKxxZM-S A/C
//   Brand: Mitsubishi Heavy Industries,  Model: SRKxxZMXA-S A/C
//   Brand: Mitsubishi Heavy Industries,  Model: RKX502A001C remote
//   Brand: Mitsubishi Heavy Industries,  Model: SRKxxZJ-S A/C

#ifndef IR_MITSUBISHIHEAVY_H_
#define IR_MITSUBISHIHEAVY_H_

#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include "IRremoteESP8266.h"
#include "IRsend.h"
#ifdef UNIT_TEST
#include "IRsend_test.h"
#endif

// Ref:
//   https://github.com/crankyoldgit/IRremoteESP8266/issues/660
//   https://github.com/ToniA/Raw-IR-decoder-for-Arduino/blob/master/MitsubishiHeavy.cpp
//   https://github.com/ToniA/arduino-heatpumpir/blob/master/MitsubishiHeavyHeatpumpIR.cpp

// Constants.
const uint8_t kMitsubishiHeavySigLength = 5;


// ZMS (152 bit)
const uint8_t kMitsubishiHeavyZmsSig[kMitsubishiHeavySigLength] = {
    0xAD, 0x51, 0x3C, 0xE5, 0x1A};
// Byte[5]
const uint8_t kMitsubishiHeavyModeOffset = 0;
//                           Mode Mask =      0b00000111;  // Byte 9 on ZJS
const uint8_t kMitsubishiHeavyAuto = 0;         // 0b000
const uint8_t kMitsubishiHeavyCool = 1;         // 0b001
const uint8_t kMitsubishiHeavyDry =  2;         // 0b010
const uint8_t kMitsubishiHeavyFan =  3;         // 0b011
const uint8_t kMitsubishiHeavyHeat = 4;         // 0b100
const uint8_t kMitsubishiHeavyPowerOffset = 3;  // Byte 9 on ZJS
const uint8_t kMitsubishiHeavyCleanOffset = 5;
const uint8_t kMitsubishiHeavyFilterOffset = 6;
// Byte[7]
const uint8_t kMitsubishiHeavyMinTemp = 17;   // 17C
const uint8_t kMitsubishiHeavyMaxTemp = 31;   // 31C
// Byte[9]
//                               FanMask =       0b00001111;  // ~Byte 7 on ZJS.
const uint8_t kMitsubishiHeavy152FanAuto =  0x0;  // 0b0000
const uint8_t kMitsubishiHeavy152FanLow =   0x1;  // 0b0001
const uint8_t kMitsubishiHeavy152FanMed =   0x2;  // 0b0010
const uint8_t kMitsubishiHeavy152FanHigh =  0x3;  // 0b0011
const uint8_t kMitsubishiHeavy152FanMax =   0x4;  // 0b0100
const uint8_t kMitsubishiHeavy152FanEcono = 0x6;  // 0b0110
const uint8_t kMitsubishiHeavy152FanTurbo = 0x8;  // 0b1000
// Byte[11]
const uint8_t kMitsubishiHeavy3DMask =        0b00010010;
const uint8_t kMitsubishiHeavy152SwingVOffset = 5;
const uint8_t kMitsubishiHeavy152SwingVSize = 3;  // Bits
const uint8_t kMitsubishiHeavy152SwingVAuto =    0;  // 0b000
const uint8_t kMitsubishiHeavy152SwingVHighest = 1;  // 0b001
const uint8_t kMitsubishiHeavy152SwingVHigh =    2;  // 0b010
const uint8_t kMitsubishiHeavy152SwingVMiddle =  3;  // 0b011
const uint8_t kMitsubishiHeavy152SwingVLow =     4;  // 0b100
const uint8_t kMitsubishiHeavy152SwingVLowest =  5;  // 0b101
const uint8_t kMitsubishiHeavy152SwingVOff =     6;  // 0b110
// Byte[13]
const uint8_t kMitsubishiHeavy152SwingHAuto =      0;  // 0b0000
const uint8_t kMitsubishiHeavy152SwingHLeftMax =   1;  // 0b0001
const uint8_t kMitsubishiHeavy152SwingHLeft =      2;  // 0b0010
const uint8_t kMitsubishiHeavy152SwingHMiddle =    3;  // 0b0011
const uint8_t kMitsubishiHeavy152SwingHRight =     4;  // 0b0100
const uint8_t kMitsubishiHeavy152SwingHRightMax =  5;  // 0b0101
const uint8_t kMitsubishiHeavy152SwingHRightLeft = 6;  // 0b0110
const uint8_t kMitsubishiHeavy152SwingHLeftRight = 7;  // 0b0111
const uint8_t kMitsubishiHeavy152SwingHOff =       8;  // 0b1000
// Byte[15]
const uint8_t kMitsubishiHeavyNightOffset = 6;
const uint8_t kMitsubishiHeavySilentOffset = 7;


// ZJS (88 bit)
const uint8_t kMitsubishiHeavyZjsSig[kMitsubishiHeavySigLength] = {
    0xAD, 0x51, 0x3C, 0xD9, 0x26};
// Byte [5]
const uint8_t kMitsubishiHeavy88CleanOffset = 5;
const uint8_t kMitsubishiHeavy88SwingHOffset1 = 2;
const uint8_t kMitsubishiHeavy88SwingHOffset2 = 6;
const uint8_t kMitsubishiHeavy88SwingHSize = 2;  // Bits (per offset)
const uint8_t kMitsubishiHeavy88SwingHOff =       0b0000;
const uint8_t kMitsubishiHeavy88SwingHAuto =      0b1000;
const uint8_t kMitsubishiHeavy88SwingHLeftMax =   0b0001;
const uint8_t kMitsubishiHeavy88SwingHLeft =      0b0101;
const uint8_t kMitsubishiHeavy88SwingHMiddle =    0b1001;
const uint8_t kMitsubishiHeavy88SwingHRight =     0b1101;
const uint8_t kMitsubishiHeavy88SwingHRightMax =  0b0010;
const uint8_t kMitsubishiHeavy88SwingHRightLeft = 0b1010;
const uint8_t kMitsubishiHeavy88SwingHLeftRight = 0b0110;
const uint8_t kMitsubishiHeavy88SwingH3D =        0b1110;
// Byte[7]
const uint8_t kMitsubishiHeavy88FanOffset = 5;
const uint8_t kMitsubishiHeavy88FanSize = 3;  // Bits
const uint8_t kMitsubishiHeavy88FanAuto =  0;  // 0b000
const uint8_t kMitsubishiHeavy88FanLow =   2;  // 0b010
const uint8_t kMitsubishiHeavy88FanMed =   3;  // 0b011
const uint8_t kMitsubishiHeavy88FanHigh =  4;  // 0b100
const uint8_t kMitsubishiHeavy88FanTurbo = 6;  // 0b110
const uint8_t kMitsubishiHeavy88FanEcono = 7;  // 0b111
const uint8_t kMitsubishiHeavy88SwingVByte5Offset = 1;
const uint8_t kMitsubishiHeavy88SwingVByte5Size = 1;
const uint8_t kMitsubishiHeavy88SwingVByte7Offset = 3;
const uint8_t kMitsubishiHeavy88SwingVByte7Size = 2;

                                          // Mask 0b111
const uint8_t kMitsubishiHeavy88SwingVOff =       0b000;  // 0
const uint8_t kMitsubishiHeavy88SwingVAuto =      0b100;  // 4
const uint8_t kMitsubishiHeavy88SwingVHighest =   0b110;  // 6
const uint8_t kMitsubishiHeavy88SwingVHigh =      0b001;  // 1
const uint8_t kMitsubishiHeavy88SwingVMiddle =    0b011;  // 3
const uint8_t kMitsubishiHeavy88SwingVLow =       0b101;  // 5
const uint8_t kMitsubishiHeavy88SwingVLowest =    0b111;  // 7
// Byte[9] is Power & Mode & Temp.


// Classes
class IRMitsubishiHeavy152Ac {
 public:
  explicit IRMitsubishiHeavy152Ac(const uint16_t pin,
                                  const bool inverted = false,
                                  const bool use_modulation = true);

  void stateReset(void);
#if SEND_MITSUBISHIHEAVY
  void send(const uint16_t repeat = kMitsubishiHeavy152MinRepeat);
  uint8_t calibrate(void) { return _irsend.calibrate(); }
#endif  // SEND_MITSUBISHIHEAVY
  void begin(void);
  void on(void);
  void off(void);

  void setPower(const bool on);
  bool getPower(void);

  void setTemp(const uint8_t temp);
  uint8_t getTemp(void);

  void setFan(const uint8_t fan);
  uint8_t getFan(void);

  void setMode(const uint8_t mode);
  uint8_t getMode(void);

  void setSwingVertical(const uint8_t pos);
  uint8_t getSwingVertical(void);
  void setSwingHorizontal(const uint8_t pos);
  uint8_t getSwingHorizontal(void);

  void setNight(const bool on);
  bool getNight(void);

  void set3D(const bool on);
  bool get3D(void);

  void setSilent(const bool on);
  bool getSilent(void);

  void setFilter(const bool on);
  bool getFilter(void);

  void setClean(const bool on);
  bool getClean(void);

  void setTurbo(const bool on);
  bool getTurbo(void);

  void setEcono(const bool on);
  bool getEcono(void);

  uint8_t* getRaw(void);
  void setRaw(const uint8_t* data);

  static bool checkZmsSig(const uint8_t *state);
  static bool validChecksum(
      const uint8_t *state,
      const uint16_t length = kMitsubishiHeavy152StateLength);
  static uint8_t convertMode(const stdAc::opmode_t mode);
  static uint8_t convertFan(const stdAc::fanspeed_t speed);
  static uint8_t convertSwingV(const stdAc::swingv_t position);
  static uint8_t convertSwingH(const stdAc::swingh_t position);
  static stdAc::opmode_t toCommonMode(const uint8_t mode);
  static stdAc::fanspeed_t toCommonFanSpeed(const uint8_t speed);
  static stdAc::swingv_t toCommonSwingV(const uint8_t pos);
  static stdAc::swingh_t toCommonSwingH(const uint8_t pos);
  stdAc::state_t toCommon(void);
  String toString(void);
#ifndef UNIT_TEST

 private:
  IRsend _irsend;
#else  // UNIT_TEST
  IRsendTest _irsend;
#endif  // UNIT_TEST
  // The state of the IR remote in IR code form.
  uint8_t remote_state[kMitsubishiHeavy152StateLength];
  void checksum(void);
};

class IRMitsubishiHeavy88Ac {
 public:
  explicit IRMitsubishiHeavy88Ac(const uint16_t pin,
                                 const bool inverted = false,
                                 const bool use_modulation = true);

  void stateReset(void);
#if SEND_MITSUBISHIHEAVY
  void send(const uint16_t repeat = kMitsubishiHeavy88MinRepeat);
#endif  // SEND_MITSUBISHIHEAVY
  void begin(void);
  void on(void);
  void off(void);

  void setPower(const bool on);
  bool getPower(void);

  void setTemp(const uint8_t temp);
  uint8_t getTemp(void);

  void setFan(const uint8_t fan);
  uint8_t getFan(void);

  void setMode(const uint8_t mode);
  uint8_t getMode(void);

  void setSwingVertical(const uint8_t pos);
  uint8_t getSwingVertical(void);
  void setSwingHorizontal(const uint8_t pos);
  uint8_t getSwingHorizontal(void);

  void setTurbo(const bool on);
  bool getTurbo(void);

  void setEcono(const bool on);
  bool getEcono(void);

  void set3D(const bool on);
  bool get3D(void);

  void setClean(const bool on);
  bool getClean(void);

  uint8_t* getRaw(void);
  void setRaw(const uint8_t* data);

  static bool checkZjsSig(const uint8_t *state);
  static bool validChecksum(
      const uint8_t *state,
      const uint16_t length = kMitsubishiHeavy88StateLength);
  static uint8_t convertMode(const stdAc::opmode_t mode);
  static uint8_t convertFan(const stdAc::fanspeed_t speed);
  static uint8_t convertSwingV(const stdAc::swingv_t position);
  static uint8_t convertSwingH(const stdAc::swingh_t position);
  static stdAc::fanspeed_t toCommonFanSpeed(const uint8_t speed);
  static stdAc::swingv_t toCommonSwingV(const uint8_t pos);
  static stdAc::swingh_t toCommonSwingH(const uint8_t pos);
  stdAc::state_t toCommon(void);
  String toString(void);
#ifndef UNIT_TEST

 private:
  IRsend _irsend;
#else  // UNIT_TEST
  IRsendTest _irsend;
#endif  // UNIT_TEST
  // The state of the IR remote in IR code form.
  uint8_t remote_state[kMitsubishiHeavy152StateLength];
  void checksum(void);
};
#endif  // IR_MITSUBISHIHEAVY_H_
