#pragma once

#include "wled.h"

#include <sstream>
#include <iostream>
#include <iomanip>
#include "ble_const.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLE2904.h>

#define CHUNK_SIZE 512

class BleComms;

class BleCommsCallbacks
{
public:
  virtual void onReadyToRead(std::string subCommand, int page) = 0;
  virtual void onWrite(std::string *pValue) = 0;
};

class BleComms : public BLECharacteristicCallbacks
{
private:
  std::string m_writeBuffer = "";
  std::string m_readBuffer = "";

  File fileToWrite;

  bool m_reading = false;
  bool m_writing = false;
  bool m_writingFile = false;

  bool m_writeReady = false;

  BleCommsCallbacks *m_callbacks;
  BLECharacteristic *m_data;
  BLECharacteristic *m_control;
  BLECharacteristic *m_notify;
  BLEService *m_service;

  uint16_t m_gatts_if;

  BLECharacteristic *initChar(uint16_t id, BLEService *pService)
  {
    m_service = pService;
    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
        BLE_UUID(id),
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY);

    pCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
    pCharacteristic->setCallbacks(this);
    pCharacteristic->setValue("");
    BLE2902 *myBLE2902 = new BLE2902();
    BLE2904 *myBLE2904 = new BLE2904();
    myBLE2904->setFormat(BLE2904::FORMAT_UTF8);
    myBLE2902->setNotifications(false);

    pCharacteristic->addDescriptor(myBLE2902);
    pCharacteristic->addDescriptor(myBLE2904);

    return pCharacteristic;
  }

  void sendData(BLECharacteristic *pChar, uint8_t *value, uint16_t len)
  {
    for (auto &myPair : m_service->getServer()->getPeerDevices(false))
    {
      ::esp_ble_gatts_send_indicate(
          m_gatts_if,
          myPair.first,
          pChar->getHandle(), len, value, false); // The need_confirm = false makes this a notify.
    }
  }

public:
  BleComms(uint16_t dataId, uint16_t controlId, uint16_t notifyId,
           BLEService *service, BleCommsCallbacks *callbacks, uint16_t gatts_if)
  {
    m_gatts_if = gatts_if;
    m_callbacks = callbacks;

    m_data = initChar(dataId, service);
    m_control = initChar(controlId, service);
    if (notifyId != 0)
    {
      m_notify = initChar(notifyId, service);
    }
  }

  void writeNext(int page, BLECharacteristic *pChar)
  {
    uint32_t pos = (page - 1) * CHUNK_SIZE;

    if (m_writingFile)
    {
      uint8_t buf[CHUNK_SIZE];

      if (pos == fileToWrite.size())
      {
        buf[0] = ' ';
        sendData(pChar, buf, 1);
        m_writingFile = false;
        return;
      }

      fileToWrite.seek(pos);

      uint16_t bufsize = fileToWrite.read(buf, CHUNK_SIZE);

      BLE_DEBUG_PRINTF(F("writing chunk: %d\n"), bufsize);

      sendData(pChar, buf, bufsize);

      if (bufsize != CHUNK_SIZE)
      {
        m_writingFile = false;
        fileToWrite.close();
      }
    }
    else
    {
      size_t len = m_writeBuffer.length();

      size_t toWriteLen = len - pos > CHUNK_SIZE ? CHUNK_SIZE : len - pos;

      uint8_t *toWrite = (uint8_t *)m_writeBuffer.data() + pos;

      BLE_DEBUG_PRINTLN(F("about to write"));

      sendData(pChar, toWrite, toWriteLen);

      BLE_DEBUG_PRINTF(F("notified %d\n"), toWriteLen);

      if (toWriteLen != CHUNK_SIZE)
      {
        BLE_DEBUG_PRINTLN(F("clearing"));
        m_writing = false;
        m_writeBuffer = "";
      }
    }
  }

  bool writeData(JsonObject data, bool notify)
  {
    BLE_DEBUG_PRINTLN(F("BleComms writeData"));

    m_writing = true;

    serializeJson(data, m_writeBuffer);

    BLE_DEBUG_PRINTF(F("writing data: %s\n"), m_writeBuffer.data());

    m_writeReady = true;
    m_writing = true;

    if (notify)
    {
      uint32_t page = 1;
      while (m_writing)
      {
        writeNext(page, m_notify);
        page++;
      }
    }
    else
    {
      writeNext(1, m_data);
    }

    return true;
  }

  bool streamFile(const char *path)
  {
    fileToWrite = WLED_FS.open(path);

    if (!fileToWrite || !fileToWrite.size())
    {
      return false;
    }

    m_writingFile = true;

    writeNext(1, m_data);

    return true;
  }

  void onRead(BLECharacteristic *pCharacteristic)
  {
    // ignore read requests... communicate through m_control
  }

  void onWrite(BLECharacteristic *pCharacteristic)
  {
    BLE_DEBUG_PRINTF(F("got a write from char %s\n"), m_data->getUUID().toString().data());

    if (pCharacteristic == m_control)
    {
      // Command in format r<infoForService>:pageNumber
      std::string command = pCharacteristic->getValue();

      BLE_DEBUG_PRINTF(F("got a write from char %s\n"), pCharacteristic->getUUID().toString().data());

      if (command.at(0) == 'r')
      {
        std::string pageNumStr;

        std::string subCommand;

        std::istringstream stream(command);

        std::getline(stream, subCommand, ':');
        std::getline(stream, pageNumStr, ':');

        int pageNum = std::atoi(pageNumStr.data());

        BLE_DEBUG_PRINTF(F("after sscanf %s %d\n"), subCommand.data(), pageNum);

        m_callbacks->onReadyToRead(subCommand, pageNum);

        return;
      }
    }
    else
    {
      if (m_reading == false)
      {
        m_readBuffer = "";
      }

      m_readBuffer += pCharacteristic->getValue();
      m_reading = true;

      BLE_DEBUG_PRINTF(F("BleComms >> onWrite: len=%d, recv=%s\n"), pCharacteristic->getValue().length(),
                       pCharacteristic->getValue().data());

      if (pCharacteristic->getValue().length() != CHUNK_SIZE)
      {
        m_callbacks->onWrite(&m_readBuffer);
        m_reading = false;
      }
    }
  }

  BLECharacteristic *getDataChar()
  {
    return m_data;
  }
};