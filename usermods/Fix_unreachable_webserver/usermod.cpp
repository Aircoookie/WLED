#include "wled.h"
/*
 * This file allows you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * EEPROM bytes 2750+ are reserved for your custom use case. (if you extend #define EEPSIZE in const.h)
 * bytes 2400+ are currently ununsed, but might be used for future wled features
 */

#include <ping.h>

const int PingDelayMs = 60000;
long lastCheckTime = 0;
bool connectedWiFi = false;
ping_option pingOpt;

//Use userVar0 and userVar1 (API calls &U0=,&U1=, uint16_t)

//gets called once at boot. Do all initialization that doesn't depend on network here
void userSetup()
{

}


//gets called every time WiFi is (re-)connected. Initialize own network interfaces here
void userConnected()
{
  connectedWiFi = true;
  // initialize ping_options structure
  memset(&pingOpt, 0, sizeof(struct ping_option));
  pingOpt.count = 1;
  pingOpt.ip = WiFi.localIP();
}

//loop. You can use "if (WLED_CONNECTED)" to check for successful connection
void userLoop()
{
  if (connectedWiFi && millis()-lastCheckTime > PingDelayMs)
  {
    ping_start(&pingOpt);
    lastCheckTime = millis();
  }
}
