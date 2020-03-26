#ifndef WLED_IR_H
#define WLED_IR_H
#include <Arduino.h>
/*
 * Infrared sensor support for generic 24/40/44 key RGB remotes
 */

bool decodeIRCustom(uint32_t code);
void relativeChange(byte* property, int8_t amount, byte lowerBoundary = 0, byte higherBoundary = 0xFF);
void changeEffectSpeed(int8_t amount);
void changeEffectIntensity(int8_t amount);
void decodeIR(uint32_t code);
void decodeIR24(uint32_t code);
void decodeIR24OLD(uint32_t code);
void decodeIR24CT(uint32_t code);
void decodeIR40(uint32_t code);
void decodeIR44(uint32_t code);
void decodeIR21(uint32_t code);
void decodeIR6(uint32_t code);

void initIR();
void handleIR();

#endif //WLED_IR_H