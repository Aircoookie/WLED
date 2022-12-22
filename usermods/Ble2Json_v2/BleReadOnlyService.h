#pragma once

#include "wled.h"

#include "ble_const.h"

#include "BleServiceBase.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

class BleReadOnlyService : public BleServiceBase
{
private:
  uint16_t m_serviceId;
  uint16_t m_dataId;
  uint16_t m_controlId;

protected:
  bool writeData(BleChunker *chunker, std::string page)
  {
    DEBUG_PRINTLN("BleStateInfoService writeData");
    if (!requestJSONBufferLock(100))
      return false;

    Serial.printf("read only write: %d", m_dataId);

    switch (m_dataId)
    {
    case WLED_BLE_FX_DETAILS_DATA_ID:
      serializeModeData(doc.as<JsonArray>());
      break;
    case WLED_BLE_FX_NAMES_DATA_ID:
      serializeModeNames(doc.as<JsonArray>());
      break;
    case WLED_BLE_PRESETS_DATA_ID:
      readObjectFromFile("/presets.json", nullptr, &doc);
      break;
    case WLED_BLE_PALETTE_NAME_DATA_ID:
      doc[F("palettes")] = serialized((const __FlashStringHelper *)JSON_palette_names);
      break;
    default:
      break;
    }

    bool ret = chunker->writeData(doc.as<JsonObject>(), false);

    releaseJSONBufferLock();

    return ret;
  }

  bool writeNotify(BleChunker *chunker)
  {
    // Doesn't notify
    return true;
  }

public:
  BleReadOnlyService(uint16_t serviceId, uint16_t dataId, uint16_t controlId)
  {
    m_serviceId = serviceId;
    m_dataId = dataId;
    m_controlId = controlId;
  }

  void setupBle(BLEServer *server)
  {
    BleServiceBase::setupBle(m_serviceId, m_dataId, m_controlId, 0, server);
  }
};