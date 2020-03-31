#ifndef WLED_BLYNK_H
#define WLED_BLYNK_H
#include <Arduino.h>
/*
 * Remote light control with the free Blynk app
 */

void initBlynk(const char* auth);
void handleBlynk();
void updateBlynk();
// TODO: Make sure that the macro expansions are handled correctly.

#endif //WLED_BLYNK_H