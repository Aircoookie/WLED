#ifndef WLED_CRONIXIE_H
#define WLED_CRONIXIE_H
#include <Arduino.h>
/*
 * Support for the Cronixie clock
 */

byte getSameCodeLength(char code, int index, char const cronixieDisplay[]);
void setCronixie();
void _overlayCronixie();    
void _drawOverlayCronixie();

#endif // WLED_CRONIXIE_H