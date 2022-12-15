#pragma once

#include "wled.h"

#include "ble_const.h"

#include "BleServiceBase.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

class BleInfoService : public BleServiceBase
{
protected:
  bool writeData(BleChunker *chunker)
  {
    DEBUG_PRINTLN("BleInfoService writeData");
    if (!requestJSONBufferLock(100))
      return false;

    JsonObject info = doc.createNestedObject("info");

    serializeInfo(info);

    bool ret = chunker->writeData(info);

    releaseJSONBufferLock();

    return ret;
  }

public:
  BleInfoService()
  {
  }

  void setupBle(BLEServer *server)
  {
    BleServiceBase::setupBle(WLED_BLE_INFO_SERVICE_ID, WLED_BLE_INFO_DATA_ID,
                             WLED_BLE_INFO_CONTROL_ID,
                             server);
  }

  void loop()
  {
    BleServiceBase::loop();
  }
};