#pragma once

#include "wled.h"

#include "ble_const.h"

#include "BleServiceBase.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

class BleCfgService : public BleServiceBase
{
protected:
  bool writeData(BleChunker *chunker)
  {
    DEBUG_PRINTLN("BleCfgService writeData");
    if (!requestJSONBufferLock(100))
      return false;

    DEBUG_PRINTLN(F("Reading settings from /cfg.json..."));

    if (!readObjectFromFile("/cfg.json", nullptr, &doc))
    {
      releaseJSONBufferLock();
      return false;
    }

    bool ret = chunker->writeData(doc.as<JsonObject>());

    releaseJSONBufferLock();

    return ret;
  }

public:
  BleCfgService()
  {
  }

  void setupBle(BLEServer *server)
  {
    BleServiceBase::setupBle(WLED_BLE_CFG_SERVICE_ID, WLED_BLE_CFG_DATA_ID,
                             WLED_BLE_CFG_CONTROL_ID,
                             server);
  }

  void loop()
  {
    BleServiceBase::loop();
  }
};