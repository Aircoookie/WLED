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
  BleChunker *m_chunker = NULL;
  BLEServer *m_server = NULL;

protected:
  virtual bool writeData(BleChunker *chunker) = 0;

public:
  BleServiceBase()
  {
  }

  virtual void setupBle(uint32_t serviceId, uint32_t dataId, uint32_t controlId, BLEServer *server)
  {
    m_server = server;
    BLEService *pService = server->createService(BLE_UUID(serviceId));

    m_chunker = new BleChunker(dataId, controlId, pService, this);

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
      if (writeData(m_chunker))
      {
        m_shouldWrite = false;
      }
    }
  }

  virtual void setShouldWrite(bool shouldWrite)
  {
    m_shouldWrite = shouldWrite;
  }

  virtual void onReadyToRead()
  {
    DEBUG_PRINTLN("Ready to read");

    m_shouldWrite = true;
  }

  virtual void onWrite(std::string value)
  {
    ESP_LOGD("Service Base", ">> got write : %s", value);
  }
};