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
  bool m_shouldWrite = false;
  int m_page = 0;
  std::string m_subCommand = "";

protected:
  virtual bool writeData(BleComms *comms, std::string subCommand, bool notify) = 0;

public:
  BleServiceBase()
  {
  }

  void setAdvertisementData(BLEAdvertising *ad)
  {
    BLEAdvertisementData data = BLEAdvertisementData();

    data.setManufacturerData("WLED");
    data.setName(cmDNS);

    ad->setAdvertisementData(data);
  }

  virtual void setupBle(uint16_t serviceId, uint16_t dataId,
                        uint16_t controlId, uint16_t notifyId,
                        BLEServer *server, uint16_t gatts_if)
  {
    m_server = server;
    BLEService *pService = server->createService(BLE_UUID(serviceId));

    BLE_DEBUG_PRINTF("serviceId: %s data id: %02X control: %02X, notify: %02X",
                     BLE_UUID(serviceId).toString().data(), dataId, controlId, notifyId);

    BLE_DEBUG_PRINTLN("");
    m_comms = new BleComms(dataId, controlId, notifyId, pService, this, gatts_if);

    pService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLE_UUID(serviceId));
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0);
    setAdvertisementData(pAdvertising);
  }

  virtual void loop()
  {
    if (m_shouldNotify && m_server->getPeerDevices(true).size() > 0)
    {
      if (writeData(m_comms, "", true))
      {
        m_shouldNotify = false;
      }
    }

    if (m_shouldWrite && m_server->getPeerDevices(true).size() > 0)
    {
      m_shouldWrite = false;
      if (m_page == 1)
        writeData(m_comms, m_subCommand, false);
      else
        m_comms->writeNext(m_page, m_comms->getDataChar());
    }
  }

  virtual void setShouldNotify(boolean shouldNotify)
  {
    m_shouldNotify = shouldNotify;
  }

  virtual void onReadyToRead(std::string subCommand, int page)
  {
    BLE_DEBUG_PRINTLN(F("Ready to read"));
    m_shouldWrite = true;
    m_page = page;
    m_subCommand = subCommand;
  }

  virtual void onWrite(std::string *pValue)
  {
    BLE_DEBUG_PRINTF(F("ServiceBase >> got write : %s\n"), pValue->data());
  }
};