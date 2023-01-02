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
  bool writeData(BleComms *comms, std::string subCommand, bool notify)
  {
    if (m_dataId == WLED_BLE_PRESETS_DATA_ID)
    {
      return comms->streamFile("/presets.json");
    }

    BLE_DEBUG_PRINTLN(F("BleReadOnlyService writeData"));
    if (!requestJSONBufferLock(100))
      return false;

    BLE_DEBUG_PRINTF(F("read only write: %d\n"), m_dataId);

    JsonArray data = doc.createNestedArray("array");

    switch (m_dataId)
    {
    case WLED_BLE_FX_DETAILS_DATA_ID:
      BLE_DEBUG_PRINTLN(F("reading mode data"));
      serializeModeData(data);
      break;
    case WLED_BLE_FX_NAMES_DATA_ID:
      BLE_DEBUG_PRINTLN(F("reading mode names"));
      serializeModeNames(data);
      break;
    case WLED_BLE_PALETTE_NAME_DATA_ID:
      doc[F("array")] = serialized((const __FlashStringHelper *)JSON_palette_names);
      break;
    default:
      break;
    }

    bool ret = comms->writeData(doc.as<JsonObject>(), false);

    releaseJSONBufferLock();

    return ret;
  }

  bool writeNotify(BleComms *comms)
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

  void setupBle(BLEServer *server, uint16_t gatts_if)
  {
    BleServiceBase::setupBle(m_serviceId, m_dataId, m_controlId, 0, server, gatts_if);
  }
};