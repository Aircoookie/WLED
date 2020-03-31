#ifndef WLED_LED_H
#define WLED_LED_H
#include <Arduino.h>
/*
 * LED methods
 */

void setValuesFromMainSeg();
void resetTimebase();
void toggleOnOff();
void setAllLeds();
void setLedsStandard(bool justColors = false);
bool colorChanged();
void colorUpdated(int callMode);
void updateInterfaces(uint8_t callMode);
void handleTransitions();
void handleNightlight();

#endif