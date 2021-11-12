#include "wled.h"
#include "fcn_declare.h"
#include "const.h"


bool requestJSONBufferLock()
{
  unsigned long now = millis();

  while (jsonBufferLock && millis()-now < 1000) delay(1); // wait for a second for buffer lock

  if (millis()-now >= 1000) return false; // waiting time-outed

  jsonBufferLock = true;
  fileDoc = &doc;  // used for applying presets (presets.cpp)
  doc.clear();
  return true;
}


void releaseJSONBufferLock()
{
  fileDoc = nullptr;
  jsonBufferLock = false;
}
