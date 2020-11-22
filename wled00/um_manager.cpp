#include "wled.h"
/*
 * Registration and management utility for v2 usermods
 */

//Usermod Manager internals
void UsermodManager::loop()      { for (byte i = 0; i < numMods; i++) ums[i]->loop();  }

void UsermodManager::setup()     { for (byte i = 0; i < numMods; i++) ums[i]->setup(); }
void UsermodManager::connected() { for (byte i = 0; i < numMods; i++) ums[i]->connected(); }

void UsermodManager::addToJsonState(JsonObject& obj)    { for (byte i = 0; i < numMods; i++) ums[i]->addToJsonState(obj); }
void UsermodManager::addToJsonInfo(JsonObject& obj)     { for (byte i = 0; i < numMods; i++) ums[i]->addToJsonInfo(obj); }
void UsermodManager::readFromJsonState(JsonObject& obj) { for (byte i = 0; i < numMods; i++) ums[i]->readFromJsonState(obj); }
void UsermodManager::addToConfig(JsonObject& obj)       { for (byte i = 0; i < numMods; i++) ums[i]->addToConfig(obj); }
void UsermodManager::readFromConfig(JsonObject& obj)    { for (byte i = 0; i < numMods; i++) ums[i]->readFromConfig(obj); }

bool UsermodManager::add(Usermod* um)
{
  if (numMods >= WLED_MAX_USERMODS || um == nullptr) return false;
  ums[numMods] = um;
  numMods++;
}

byte UsermodManager::getModCount() {return numMods;}