#pragma once

#include "wled.h"

#include "ble_const.h"

#include "BleServiceBase.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>


class BleStateService :public BleServiceBase {
  private:
    std::string *m_toSave = NULL;

  protected:
    bool writeData(BleChunker *chunker) {
      Serial.println("BleStateService writeData");
      if (!requestJSONBufferLock(100)) return false;
      JsonObject state = doc.createNestedObject("state");

      serializeState(state);

      bool ret = chunker->writeData(state);

      releaseJSONBufferLock();

      return ret;
    }

    void saveData() {
      if (m_toSave != NULL) {
        DynamicJsonDocument localDoc(JSON_BUFFER_SIZE);

        DeserializationError error = deserializeJson(localDoc, m_toSave->data());

        if (error) {
          ESP_LOGD("State Server", "error : %d", error.code());
          return;
        }

        JsonObject obj = localDoc.as<JsonObject>();

        serializeJson(obj, Serial);
        Serial.println("");
        serializeJson(localDoc, Serial);
        Serial.println("");
        
        deserializeState(obj, CALL_MODE_BUTTON_PRESET);

        stateUpdated(CALL_MODE_BUTTON);

        ESP_LOGD("State Server", "saving data : %s", m_toSave->data());

        delete m_toSave;
        m_toSave = NULL;
      }
    }
    
  public: 
    BleStateService() {
    }

    void setupBle(BLEServer *server) {
      BleServiceBase::setupBle(WLED_BLE_STATE_SERVICE_ID, WLED_BLE_STATE_DATA_ID, 
        WLED_BLE_STATE_CONTROL_ID, server);
    }

    void loop() {
      saveData();

      // bool dirty = false;
      // std::string newData = "{";

      // for (std::map<int, BleStateCharacteristicHandler *>::iterator it = m_characteristics.begin(); it != m_characteristics.end(); it++)
      // {
      //   it->second->loop();
      //   if(it->second->isServerDirty()) {
      //     Serial.println("server was dirty");
      //     dirty = true;

      //     Serial.printf_P(PSTR("setting %s to value: %s\n"), 
      //       it->second->getStateKey().c_str(), 
      //       it->second->getCurrentValue()->c_str());

      //     if (newData.length() > 1) {
      //       newData += ",";
      //     }

      //     newData += "\"" + it->second->getStateKey() + "\":" + *it->second->getCurrentValue();

      //     it->second->setServerDirty(false);
      //   }
      // }

      // if (dirty) {
      //   newData += "}";

      //   Serial.printf_P(PSTR("json string:  %s\n"), newData.c_str());

      //   if (!requestJSONBufferLock(100)) return;

      //   deserializeJson(doc, newData);
      //   JsonObject obj = doc.as<JsonObject>();

      //   serializeJson(obj, Serial);
      //   deserializeState(obj, CALL_MODE_BUTTON_PRESET);

      //   releaseJSONBufferLock();

      //   stateUpdated(CALL_MODE_BUTTON);
      // }
    }
    
    /*
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void readFromJsonState(JsonObject& root)
    {
      setShouldWrite(true);
    }

    void onWrite(std::string value) {
      ESP_LOGD("State Server", ">> got write : %s", value.data());

      if (m_toSave) {
        delete m_toSave;
        m_toSave = NULL;
      }

      m_toSave = new std::string(value);
    }
};