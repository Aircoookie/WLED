#pragma once

#include "wled.h"

#include <sstream>
#include <iomanip>
#include "ble_const.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#define CHUNK_SIZE 512

class BleChunkerCallbacks {
public:
  virtual void onReadyToRead() = 0;
  virtual void onWrite(std::string value) = 0;
};

class BleChunker : public BLECharacteristicCallbacks {
private:
  std::string m_writeBuffer = "";
  std::string m_readBuffer = "";
  bool m_reading = false;
  bool m_writing = false;
  bool m_writeReady = false;
  bool m_readReady = false;
  size_t m_currentPos = 0;

  BleChunkerCallbacks *m_callbacks;
  BLECharacteristic *m_data;
  BLECharacteristic *m_control;

  BLECharacteristic *initChar(uint16_t id, BLEService *pService) {
    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
              BLE_UUID(id),
              BLECharacteristic::PROPERTY_READ   |
              BLECharacteristic::PROPERTY_WRITE  |
              BLECharacteristic::PROPERTY_NOTIFY 
            );

    pCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
    pCharacteristic->setCallbacks(this);
    pCharacteristic->setValue("");

    return pCharacteristic;
  }

  void writeChunk() {
    size_t len = m_writeBuffer.length();

    if (len <= m_currentPos) {
      return;
    }

    if (m_currentPos == 0 && len % CHUNK_SIZE == 0) {
      m_writeBuffer += " ";
      len++;
    }

    size_t toWriteLen = len - m_currentPos > CHUNK_SIZE ? CHUNK_SIZE:len - m_currentPos;

    uint8_t *toWrite = (uint8_t*)m_writeBuffer.data() + m_currentPos;

    m_data->setValue(toWrite, toWriteLen);
    // m_writeReady = false;
    m_data->notify(true);

    ESP_LOGD("BleChunker", ">> writeChunk: curr pos=%d, len=%d, to write=%d, writing=%d to write=%s", m_currentPos, len, toWriteLen, m_writing, toWrite);

    if (len - m_currentPos > CHUNK_SIZE) {
      Serial.println("updating");
      m_writing = true;
      m_currentPos = m_currentPos + toWriteLen;
    }
    else {
      m_writing = false;
      m_writeBuffer = "";
    }

    ESP_LOGD("BleChunker", ">> writeChunk (after): curr pos=%d, len=%d, to write=%d, writing=%d", m_currentPos, len, toWriteLen, m_writing);
  }

public:
  BleChunker(uint16_t dataId, uint16_t controlId, BLEService *service, BleChunkerCallbacks *callbacks) {
    m_callbacks = callbacks;

    m_data = initChar(dataId, service);
    m_control = initChar(controlId, service);
  }

  bool writeData(JsonObject data) {
    Serial.println("BleChunker writeData");

    if (!m_writing) {
      serializeJson(data, m_writeBuffer);

      m_writeReady = true;
      m_writing = true;
      m_currentPos = 0;

      while (m_writing && m_writeReady) {
        writeChunk();
      }

      return true;
    }
    return false;
  }

  void onRead(BLECharacteristic* pCharacteristic) {
    // ignore read requests... communicate through m_control
  }

	void onWrite(BLECharacteristic* pCharacteristic) {
    if (pCharacteristic == m_control) {
      std::string command = pCharacteristic->getValue();

      if (command == "r") {
        m_callbacks->onReadyToRead();
        return;
      }
    }
    else {
      m_readBuffer += pCharacteristic->getValue();
      m_reading = true;

      ESP_LOGD("BleChunker", ">> onWrite: len=%d, recv=%s", pCharacteristic->getValue().length(), 
        pCharacteristic->getValue().data());

      if (pCharacteristic->getValue().length() != CHUNK_SIZE) {
        m_callbacks->onWrite(m_readBuffer);
        m_reading = false;
        m_readBuffer = "";
      }
    }
  }
};