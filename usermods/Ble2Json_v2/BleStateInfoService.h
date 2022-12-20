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
  std::string *m_toSave = NULL;

protected:
  bool writeData(BleChunker *chunker)
  {
    DEBUG_PRINTLN("BleStateInfoService writeData");
    if (!requestJSONBufferLock(100))
      return false;
    JsonObject state = doc.createNestedObject("state");
    JsonObject info = doc.createNestedObject("info");

    serializeState(state);
    serializeInfo(info);

    bool ret = chunker->writeData(doc.as<JsonObject>(), false);

    releaseJSONBufferLock();

    return ret;
  }

  bool writeNotify(BleChunker *chunker)
  {
    DEBUG_PRINTLN("BleStateInfoService writeNotify");
    if (!requestJSONBufferLock(100))
      return false;
    JsonObject state = doc.createNestedObject("state");

    serializeState(state);

    bool ret = chunker->writeData(state, true);

    releaseJSONBufferLock();

    return ret;
  }

  void saveData()
  {
    DEBUG_PRINTLN("check save");

    if (m_toSave != NULL)
    {
      DEBUG_PRINTLN("trying to save data");
      DynamicJsonDocument localDoc(JSON_BUFFER_SIZE);

      DeserializationError error = deserializeJson(localDoc, m_toSave->data());

      if (error)
      {
        ESP_LOGD("State Server", "error : %d", error.code());
        return;
      }

      DEBUG_PRINTF("got data %s", m_toSave->data());

      JsonObject obj = localDoc.as<JsonObject>();

      serializeJson(obj, Serial);
      DEBUG_PRINTLN("");
      serializeJson(localDoc, Serial);
      DEBUG_PRINTLN("");

      deserializeState(obj, CALL_MODE_BUTTON_PRESET);

      stateUpdated(CALL_MODE_BUTTON);

      ESP_LOGD("State Server", "saving data : %s", m_toSave->data());

      delete m_toSave;
      m_toSave = NULL;
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
    DEBUG_PRINTLN("state service loop");

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

  void onWrite(std::string value)
  {
    ESP_LOGD("State Server", ">> got write : %s", value.data());

    if (m_toSave)
    {
      delete m_toSave;
      m_toSave = NULL;
    }

    m_toSave = new std::string(value);
  }
};