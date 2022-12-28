#pragma once

#include "wled.h"

#include "ble_const.h"

#include "BleServiceBase.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

class BleStateInfoService : public BleServiceBase
{
private:
  std::string *m_toSave = nullptr;

protected:
  bool writeData(BleComms *comms, std::string subCommand)
  {
    BLE_DEBUG_PRINTLN("BleStateInfoService writeData");
    if (!requestJSONBufferLock(100))
      return false;
    JsonObject state = doc.createNestedObject("state");
    JsonObject info = doc.createNestedObject("info");

    serializeState(state);
    serializeInfo(info);

    bool ret = comms->writeData(doc.as<JsonObject>(), false);

    releaseJSONBufferLock();

    return ret;
  }

  bool writeNotify(BleComms *comms)
  {
    BLE_DEBUG_PRINTLN("BleStateInfoService writeNotify");
    if (!requestJSONBufferLock(100))
      return false;
    JsonObject state = doc.createNestedObject("state");

    serializeState(state);

    bool ret = comms->writeData(state, true);

    releaseJSONBufferLock();

    return ret;
  }

  void saveData()
  {
    BLE_DEBUG_PRINTLN("check save");

    if (m_toSave != nullptr)
    {
      BLE_DEBUG_PRINTF("trying to save data");
      if (!requestJSONBufferLock(100))
        return;

      DeserializationError error = deserializeJson(doc, m_toSave->data());

      if (error)
      {
        BLE_DEBUG_PRINTF("State Server error : %d %s\n", error.code(), m_toSave->data());
        releaseJSONBufferLock();

        return;
      }

      BLE_DEBUG_PRINTF("got data %s\n", m_toSave->data());

      JsonObject obj = doc.as<JsonObject>();

      deserializeState(obj, CALL_MODE_BUTTON_PRESET);

      stateUpdated(CALL_MODE_BUTTON);

      ESP_LOGD("State Server", "saving data : %s", m_toSave->data());

      m_toSave = nullptr;
      releaseJSONBufferLock();
    }
  }

public:
  BleStateInfoService()
  {
  }

  void setupBle(BLEServer *server)
  {
    BleServiceBase::setupBle(WLED_BLE_DATA_SERVICE_ID, WLED_BLE_STATE_INFO_DATA_ID,
                             WLED_BLE_STATE_INFO_CONTROL_ID, WLED_BLE_STATE_NOTIFY_ID, server);
  }

  void loop()
  {
    BLE_DEBUG_PRINTLN("state service loop");

    BleServiceBase::loop();

    saveData();
  }

  /*
   * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
   * Values in the state object may be modified by connected clients
   */
  void readFromJsonState(JsonObject &root)
  {
    setShouldNotify(true);
  }

  virtual void onWrite(std::string *pValue)
  {
    BLE_DEBUG_PRINTF("State Service>> got write : %s\n", pValue->data());

    m_toSave = pValue;
  }
};