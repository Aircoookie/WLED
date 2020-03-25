#ifndef WLED_BLYNK_H
#define WLED_BLYNK_H
#include <Arduino.h>
/*
 * Remote light control with the free Blynk app
 */

void initBlynk(const char* auth);
void handleBlynk();
void updateBlynk();
// Unsure if the macro expansions need to accessed through the declaration... TODO

#endif //WLED_BLYNK_H