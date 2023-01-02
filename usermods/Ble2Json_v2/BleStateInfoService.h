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
  /*
   * writeData(BleComms *comms, std::string subCommand, bool notify) called by the service base
   * in the loop if there is a data to write (either as response to read or as a notification)
   *
   * will gather the state/info data and write it back to the connected device
   *
   */
  bool writeData(BleComms *comms, std::string subCommand, bool notify)
  {
    BLE_DEBUG_PRINTLN(F("BleStateInfoService writeData"));
    if (!requestJSONBufferLock(100))
      return false;
    JsonObject state = doc.createNestedObject("state");
    JsonObject info = doc.createNestedObject("info");

    serializeState(state);
    serializeInfo(info);

    bool ret = comms->writeData(doc.as<JsonObject>(), notify);

    releaseJSONBufferLock();

    return ret;
  }

  /*
   * saveData() is called in the loop...
   *
   * if there is data to save to state, this is where that happens
   */
  void saveData()
  {
    BLE_DEBUG_PRINTF(F("trying to save data"));
    if (!requestJSONBufferLock(100))
      return;

    DeserializationError error = deserializeJson(doc, m_toSave->data());

    if (error)
    {
      BLE_DEBUG_PRINTF(F("State Server error : %d %s\n"), error.code(), m_toSave->data());
      releaseJSONBufferLock();

      return;
    }

    BLE_DEBUG_PRINTF(F("got data %s\n"), m_toSave->data());

    JsonObject obj = doc.as<JsonObject>();

    deserializeState(obj, CALL_MODE_BUTTON_PRESET);

    stateUpdated(CALL_MODE_BUTTON);

    BLE_DEBUG_PRINTF(F("State Server saving data : %s"), m_toSave->data());

    if (doc['v'])
    {
      setShouldNotify(true);
    }

    m_toSave = nullptr;
    releaseJSONBufferLock();
  }

public:
  BleStateInfoService()
  {
  }

  void setupBle(BLEServer *server, uint16_t gatts_if)
  {
    BleServiceBase::setupBle(WLED_BLE_DATA_SERVICE_ID, WLED_BLE_STATE_INFO_DATA_ID,
                             WLED_BLE_STATE_INFO_CONTROL_ID, WLED_BLE_STATE_NOTIFY_ID, server, gatts_if);
  }

  void loop()
  {
    BleServiceBase::loop();

    if (m_toSave != nullptr)
    {
      saveData();
    }
  }

  /*
   * readFromJsonState() state changed, notify all connected devices
   */
  void readFromJsonState(JsonObject &root)
  {
    setShouldNotify(true);
  }

  /*
   * onWrite(std::string *pValue) got a write from a connected device...
   * will set state in the loop
   */
  virtual void onWrite(std::string *pValue)
  {
    BLE_DEBUG_PRINTF(F("State Service>> got write : %s\n"), pValue->data());

    m_toSave = pValue;
  }
};