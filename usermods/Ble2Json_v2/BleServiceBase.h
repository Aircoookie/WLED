#pragma once

#include "wled.h"

#include "ble_const.h"

#include "BleChunker.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>


class BleServiceBase :public BleChunkerCallbacks {
  private:
    bool m_shouldWrite = false;
    BleChunker *m_chunker = NULL;
    BLEServer *m_server = NULL;

  protected:
    virtual bool writeData(BleChunker *chunker) = 0;
    
  public: 
    BleServiceBase() {
    }

    void setupBle(uint32_t serviceId, uint32_t dataId, uint32_t controlId, BLEServer *server) {
      m_server = server;
      BLEService *pService = server->createService(BLE_UUID(serviceId));

      m_chunker = new BleChunker(dataId, controlId, pService, this);

      pService->start();

      BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
      pAdvertising->addServiceUUID(BLE_UUID(serviceId));
      pAdvertising->setScanResponse(false);
      pAdvertising->setMinPreferred(0x0);
    }

    void loop() {
      if (m_shouldWrite && m_server->getPeerDevices(true).size() > 0) {
        if (writeData(m_chunker)) {
          m_shouldWrite = false;
        }
      }

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

    void setShouldWrite(bool shouldWrite) {
      m_shouldWrite = shouldWrite;
    }    

    void onReadyToRead() {
      Serial.println("Ready to read");

      m_shouldWrite = true;
    }

    void onWrite(std::string value) {
      ESP_LOGD("Service Base", ">> got write : %s", value);
    }
};