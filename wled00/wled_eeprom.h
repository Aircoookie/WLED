#ifndef WLED_EPPROM_H
#define WLED_EPPROM_H
#include <Arduino.h>
/*
 * Methods to handle saving and loading to non-volatile memory
 * EEPROM Map: https://github.com/Aircoookie/WLED/wiki/EEPROM-Map
 */

void commit();
void clearEEPROM();
void writeStringToEEPROM(uint16_t pos, char* str, uint16_t len);
void readStringFromEEPROM(uint16_t pos, char* str, uint16_t len);
void saveSettingsToEEPROM();
void loadSettingsFromEEPROM(bool first);
void savedToPresets();
bool applyPreset(byte index, bool loadBri = true);
void savePreset(byte index, bool persist = true);
void loadMacro(byte index, char* m);
void applyMacro(byte index);
void saveMacro(byte index, String mc, bool persist = true); //only commit on single save, not in settings

#endif //WLED_EPPROM_H
