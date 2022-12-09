#pragma once

#include "wled.h"
#include "ble_const.h"
#include "ble_unpair.h"
#include "BleStateService.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_err.h"


BLEServer* pServer = NULL;

class Ble2JsonConfig {
public:
  virtual bool getBleOnFlag() = 0;
  virtual void setBleOnFlag(bool bleOnFlag) = 0;
  virtual uint32_t getBlePairingPin() = 0;
  virtual bool getBleUnPairDevices() = 0;
  virtual void setBleUnPairDevice(bool bleUnPairDevices) = 0;
};


/////////////////////
//BLE Secure Server//
/////////////////////

class ServerCallback: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println(" - ServerCallback - onConnect");
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println(" - ServerCallback - onDisconnect");
    }

};

class SecurityCallback : public BLESecurityCallbacks {

  uint32_t onPassKeyRequest(){
    Serial.println("onPassKeyRequest"); 
    return 000000;
  }

  void onPassKeyNotify(uint32_t pass_key){
    Serial.println("onPassKeyNotify"); 
  }

  bool onConfirmPIN(uint32_t pass_key){
    Serial.println("confirming pin");
    return true;
  }

  bool onSecurityRequest(){
    Serial.println("onSecurityRequest");
    return true;
  }

  void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl){
    if(cmpl.success){
      Serial.println("   - SecurityCallback - Authentication Success");       
    }else{
      Serial.printf_P(PSTR("%u\n"),cmpl.fail_reason);
      Serial.println("   - SecurityCallback - Authentication Failure* ");
      pServer->removePeerDevice(pServer->getConnId(), true);
    }
    BLEDevice::startAdvertising();
  }
};

void bleSecurity(uint32_t passkey){
  esp_ble_auth_req_t auth_req = ESP_LE_AUTH_REQ_SC_MITM_BOND;
  esp_ble_io_cap_t iocap = ESP_IO_CAP_OUT;          
  uint8_t key_size = 16;     
  uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
  uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
  uint8_t auth_option = ESP_BLE_ONLY_ACCEPT_SPECIFIED_AUTH_DISABLE;
  uint8_t oob_support = ESP_BLE_OOB_DISABLE;
  esp_ble_gap_set_security_param(ESP_BLE_SM_CLEAR_STATIC_PASSKEY, &passkey, sizeof(uint32_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_SET_STATIC_PASSKEY, &passkey, sizeof(uint32_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_ONLY_ACCEPT_SPECIFIED_SEC_AUTH, &auth_option, sizeof(uint8_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_OOB_SUPPORT, &oob_support, sizeof(uint8_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(uint8_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t));
}

class MyCallbackHandler: public BLECharacteristicCallbacks {
  private: 
    Ble2JsonConfig *m_config;
    BleStateService *m_stateService;

  public: 
    MyCallbackHandler(Ble2JsonConfig *config) {
      m_config = config;
    }

	void onWrite(BLECharacteristic* pCharacteristic) {
    ESP_LOGD("main switch", ">> on write %s", pCharacteristic->getValue().data());

    if (strcmp(pCharacteristic->getValue().data(), "off") == 0) {
      m_config->setBleOnFlag(false);
    }
  }
};

class BleMainSwitch {
  private:
    Ble2JsonConfig *m_config = NULL;
    BleStateService *m_stateService = NULL;
    bool m_bleInitted = false;

    void checkBleInit(bool fromSetup) {
      Serial.println("checkBleInit");

      bool bleOnFlag = m_config->getBleOnFlag();

      if (fromSetup && bleOnFlag && !m_bleInitted) {
        Serial.println("bleInitting");
        WLED::instance().disableWiFi();
        bleInit(new MyCallbackHandler(m_config), m_config->getBlePairingPin());
        m_bleInitted = true;
        checkUnPair();
      }
      else if (!fromSetup && bleOnFlag && !m_bleInitted) {
        Serial.println("ble going on reset");
        WLED::instance().reset();
      }
      else if (!bleOnFlag && m_bleInitted) {
        Serial.println("ble going off reset");
        WLED::instance().reset();
      }
    }

    void bleInit(BLECharacteristicCallbacks *charCallbackHandler, uint32_t passkey){
      Serial.println("bleInit");
      BLEDevice::init(WLED_BLE_2_JSON_NAME);
      BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
      BLEDevice::setSecurityCallbacks(new SecurityCallback());

      pServer = BLEDevice::createServer();
      pServer->setCallbacks(new ServerCallback());

      BLEService *pService = pServer->createService(BLE_UUID(WLED_BLE_MAIN_SERVICE_ID));

      BLECharacteristic* pCharacteristic = pService->createCharacteristic(
                          BLE_UUID(WLED_BLE_MAIN_BLE_ON_ID),
                          BLECharacteristic::PROPERTY_READ   |
                          BLECharacteristic::PROPERTY_WRITE  |
                          BLECharacteristic::PROPERTY_NOTIFY 
                        );

      pCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
      pCharacteristic->setCallbacks(charCallbackHandler);
      pCharacteristic->setValue("on");

      pService->start();

      BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
      pAdvertising->addServiceUUID(BLE_UUID(WLED_BLE_MAIN_SERVICE_ID));
      pAdvertising->setScanResponse(false);
      pAdvertising->setMinPreferred(0x0);
      BLEDevice::startAdvertising();

      bleSecurity(passkey);

      m_stateService = new BleStateService();
      m_stateService->setupBle(pServer);
    }

    void checkUnPair() {
      if (m_config->getBleUnPairDevices()) {
        m_config->setBleUnPairDevice(false);
        unPairAllDevices();
      }
    }

  public: 
    BleMainSwitch(Ble2JsonConfig *config) {
      m_config = config;
    }

    void setup() {
      Serial.printf_P(PSTR("Switch PIN: %u\n"), m_config->getBlePairingPin());

      checkBleInit(true);
    }

    void loop() {
      checkBleInit(false);
      if (m_bleInitted && m_stateService != NULL) {
        m_stateService->loop();
      }
    }

    /*
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void readFromJsonState(JsonObject& root)
    {
      if (m_bleInitted && m_stateService != NULL) {
        m_stateService->readFromJsonState(root);
      }
    }
};