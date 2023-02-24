#pragma once

#include <Arduino.h>

#ifdef ESPHOMEKIT_DEBUG
#pragma message "Homekit debug mode"
#define EHK_DEBUG(x)  Serial.print (x)
#define EHK_DEBUGLN(x) Serial.println (x)
#define EHK_DEBUGF(x, args...) Serial.printf(x, args)
#else
#define EHK_DEBUG(x)
#define EHK_DEBUGLN(x)
#define EHK_DEBUGF(x, args...)
#endif
