#pragma once

#include "wled.h"

#include "ble_const.h"

#include "BleChunker.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

class BleServiceBase : public BleChunkerCallbacks
{
private:
  bool m_shouldWrite = false;
  bool m_shouldNotify = false;
  BleChunker *m_chunker = NULL;
  BLEServer *m_server = NULL;
  std::string m_page = "";

protected:
  virtual bool writeData(BleChunker *chunker, std::string page) = 0;
  virtual bool writeNotify(BleChunker *chunker) = 0;

public:
  BleServiceBase()
  {
  }

  virtual void setupBle(uint16_t serviceId, uint16_t dataId, uint16_t controlId, uint16_t notifyId, BLEServer *server)
  {
    m_server = server;
    BLEService *pService = server->createService(BLE_UUID(serviceId));

    Serial.printf("serviceId: %s data id: %02X control: %02X, notify: %02X",
                  BLE_UUID(serviceId).toString().data(), dataId, controlId, notifyId);

    Serial.println("");
    m_chunker = new BleChunker(dataId, controlId, notifyId, pService, this);

    pService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLE_UUID(serviceId));
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0);
  }

  virtual void loop()
  {
    DEBUG_PRINTLN("base loop");

    if (m_shouldWrite && m_server->getPeerDevices(true).size() > 0)
    {
      if (writeData(m_chunker, m_page))
      {
        m_shouldWrite = false;
      }
    }

    if (m_shouldNotify && m_server->getPeerDevices(true).size() > 0)
    {
      if (writeNotify(m_chunker))
      {
        m_shouldNotify = false;
      }
    }
  }

  virtual void setShouldWrite(bool shouldWrite)
  {
    m_shouldWrite = shouldWrite;
  }

  virtual void setShouldNotify(boolean shouldNotify)
  {
    m_shouldNotify = shouldNotify;
  }

  virtual void onReadyToRead(std::string page)
  {
    DEBUG_PRINTLN("Ready to read");

    m_page = page;
    m_shouldWrite = true;
  }

  virtual void onWrite(std::string value)
  {
    ESP_LOGD("Service Base", ">> got write : %s", value);
  }
};