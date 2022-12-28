#pragma once

#include "wled.h"

#include "ble_const.h"

#include "BleComms.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

class BleServiceBase : public BleCommsCallbacks
{
private:
  bool m_shouldNotify = false;
  BleComms *m_comms = NULL;
  BLEServer *m_server = NULL;

protected:
  virtual bool writeData(BleComms *comms, std::string subCommand) = 0;
  virtual bool writeNotify(BleComms *comms) = 0;

public:
  BleServiceBase()
  {
  }

  virtual void setupBle(uint16_t serviceId, uint16_t dataId, uint16_t controlId, uint16_t notifyId, BLEServer *server)
  {
    m_server = server;
    BLEService *pService = server->createService(BLE_UUID(serviceId));

    BLE_DEBUG_PRINTF("serviceId: %s data id: %02X control: %02X, notify: %02X",
                 BLE_UUID(serviceId).toString().data(), dataId, controlId, notifyId);

    BLE_DEBUG_PRINTLN("");
    m_comms = new BleComms(dataId, controlId, notifyId, pService, this);

    pService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLE_UUID(serviceId));
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0);
  }

  virtual void loop()
  {
    BLE_DEBUG_PRINTLN("base loop");

    if (m_shouldNotify && m_server->getPeerDevices(true).size() > 0)
    {
      if (writeNotify(m_comms))
      {
        m_shouldNotify = false;
      }
    }
  }

  virtual void setShouldNotify(boolean shouldNotify)
  {
    m_shouldNotify = shouldNotify;
  }

  virtual void onReadyToRead(std::string subCommand)
  {
    BLE_DEBUG_PRINTLN("Ready to read");
    writeData(m_comms, subCommand);
  }

  virtual void onWrite(std::string *pValue)
  {
    BLE_DEBUG_PRINTF("ServiceBase >> got write : %s\n", pValue->data());
  }
};