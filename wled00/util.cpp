#include "wled.h"
#include "fcn_declare.h"
#include "const.h"


//append a numeric setting to string buffer
void sappend(char stype, const char* key, int val)
{
  char ds[] = "d.Sf.";

  switch(stype)
  {
    case 'c': //checkbox
      oappend(ds);
      oappend(key);
      oappend(".checked=");
      oappendi(val);
      oappend(";");
      break;
    case 'v': //numeric
      oappend(ds);
      oappend(key);
      oappend(".value=");
      oappendi(val);
      oappend(";");
      break;
    case 'i': //selectedIndex
      oappend(ds);
      oappend(key);
      oappend(SET_F(".selectedIndex="));
      oappendi(val);
      oappend(";");
      break;
  }
}


//append a string setting to buffer
void sappends(char stype, const char* key, char* val)
{
  switch(stype)
  {
    case 's': {//string (we can interpret val as char*)
      String buf = val;
      //convert "%" to "%%" to make EspAsyncWebServer happy
      buf.replace("%","%%");
      oappend("d.Sf.");
      oappend(key);
      oappend(".value=\"");
      oappend(buf.c_str());
      oappend("\";");
      break;}
    case 'm': //message
      oappend(SET_F("d.getElementsByClassName"));
      oappend(key);
      oappend(SET_F(".innerHTML=\""));
      oappend(val);
      oappend("\";");
      break;
  }
}


bool oappendi(int i)
{
  char s[11];
  sprintf(s, "%d", i);
  return oappend(s);
}


bool oappend(const char* txt)
{
  uint16_t len = strlen(txt);
  if (olen + len >= SETTINGS_STACK_BUF_SIZE)
    return false;        // buffer full
  strcpy(obuf + olen, txt);
  olen += len;
  return true;
}


void prepareHostname(char* hostname)
{
  const char *pC = serverDescription;
  uint8_t pos = 5;

  while (*pC && pos < 24) { // while !null and not over length
    if (isalnum(*pC)) {     // if the current char is alpha-numeric append it to the hostname
      hostname[pos] = *pC;
      pos++;
    } else if (*pC == ' ' || *pC == '_' || *pC == '-' || *pC == '+' || *pC == '!' || *pC == '?' || *pC == '*') {
      hostname[pos] = '-';
      pos++;
    }
      // else do nothing - no leading hyphens and do not include hyphens for all other characters.
      pC++;
    }
    // if the hostname is left blank, use the mac address/default mdns name
    if (pos < 6) {
      sprintf(hostname + 5, "%*s", 6, escapedMac.c_str() + 6);
    } else { //last character must not be hyphen
      while (pos > 0 && hostname[pos -1] == '-') {
        hostname[pos -1] = 0;
        pos--;
      }
    }
}


void _setRandomColor(bool _sec, bool fromButton)
{
  lastRandomIndex = strip.get_random_wheel_index(lastRandomIndex);
  if (_sec){
    colorHStoRGB(lastRandomIndex*256,255,colSec);
  } else {
    colorHStoRGB(lastRandomIndex*256,255,col);
  }
  if (fromButton) colorUpdated(2);
}


bool isAsterisksOnly(const char* str, byte maxLen)
{
  for (byte i = 0; i < maxLen; i++) {
    if (str[i] == 0) break;
    if (str[i] != '*') return false;
  }
  //at this point the password contains asterisks only
  return (str[0] != 0); //false on empty string
}


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
#ifndef WLED_USE_DYNAMIC_JSON
  fileDoc = nullptr;
  jsonBufferLock = false;
#endif
}
