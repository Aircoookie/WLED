# 1 "C:\\Users\\gsieb\\AppData\\Local\\Temp\\tmpb4qxv53h"
#include <Arduino.h>
# 1 "C:/Users/gsieb/NextcloudLocal/Meine Bibliothek/Development/ESP8266/WLED/wled00/wled00.ino"
# 13 "C:/Users/gsieb/NextcloudLocal/Meine Bibliothek/Development/ESP8266/WLED/wled00/wled00.ino"
#include "wled.h"
void setup();
void loop();
#line 15 "C:/Users/gsieb/NextcloudLocal/Meine Bibliothek/Development/ESP8266/WLED/wled00/wled00.ino"
void setup() {
  WLED::instance().setup();
}

void loop() {
  WLED::instance().loop();
}