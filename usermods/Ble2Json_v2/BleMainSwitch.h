#pragma once

#include "wled.h"
#include "ble_const.h"
#include "ble_unpair.h"
#include "BleStateInfoService.h"
#include "BleReadOnlyService.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_err.h"

BLEServer *pServer = NULL;
uint16_t m_gatts_if = ESP_GATT_IF_NONE;

void gattServerEventHandler(
    esp_gatts_cb_event_t event,
    esp_gatt_if_t gatts_if,
    esp_ble_gatts_cb_param_t *param)
{
  if (event == ESP_GATTS_REG_EVT)
  {
    m_gatts_if = gatts_if;
  }
}

class Ble2JsonConfig
{
public:
  virtual bool getBleOnFlag() = 0;
  virtual void setBleOnFlag(bool bleOnFlag) = 0;
  virtual uint32_t getBlePairingPin() = 0;
  virtual bool getBleUnPairDevices() = 0;
  virtual void setBleUnPairDevice(bool bleUnPairDevices) = 0;
};

/////////////////////
// BLE Secure Server//
/////////////////////

class ServerCallback : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    BLE_DEBUG_PRINTLN(F(" - ServerCallback - onConnect"));
  };

  void onDisconnect(BLEServer *pServer)
  {
    BLE_DEBUG_PRINTLN(F(" - ServerCallback - onDisconnect"));
  }
};

class SecurityCallback : public BLESecurityCallbacks
{

  uint32_t onPassKeyRequest()
  {
    BLE_DEBUG_PRINTLN(F("onPassKeyRequest"));
    return 000000;
  }

  void onPassKeyNotify(uint32_t pass_key)
  {
    BLE_DEBUG_PRINTLN(F("onPassKeyNotify"));
  }

  bool onConfirmPIN(uint32_t pass_key)
  {
    BLE_DEBUG_PRINTLN(F("confirming pin"));
    return true;
  }

  bool onSecurityRequest()
  {
    BLE_DEBUG_PRINTLN(F("onSecurityRequest"));
    return true;
  }

  void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl)
  {
    if (cmpl.success)
    {
      BLE_DEBUG_PRINTLN(F("   - SecurityCallback - Authentication Success"));
    }
    else
    {
      BLE_DEBUG_PRINTLN(F("   - SecurityCallback - Authentication Failure* "));
      pServer->removePeerDevice(pServer->getConnId(), true);
    }
    BLEDevice::startAdvertising();
  }
};

void bleSecurity(uint32_t passkey)
{
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

class BleMainSwitch
{
private:
  Ble2JsonConfig *m_config = NULL;
  BleStateInfoService *m_stateService = NULL;
  BleReadOnlyService *m_fxNamesService = NULL;
  BleReadOnlyService *m_fxDetailsService = NULL;
  BleReadOnlyService *m_presetsService = NULL;
  BleReadOnlyService *m_paletteNamesService = NULL;
  BleReadOnlyService *m_paletteDetailsDataService = NULL;

  bool m_bleInitted = false;
  unsigned long lastTime = 0;

  void checkBleInit(bool fromSetup)
  {
    BLE_DEBUG_PRINTLN(F("checkBleInit"));

    bool bleOnFlag = m_config->getBleOnFlag();

    if (fromSetup && bleOnFlag && !m_bleInitted)
    {
      BLE_DEBUG_PRINTLN(F("bleInitting"));
      WLED::instance().disableWiFi();
      bleInit(m_config->getBlePairingPin());
      m_bleInitted = true;
      checkUnPair();
    }
    else if (!fromSetup && bleOnFlag && !m_bleInitted)
    {
      BLE_DEBUG_PRINTLN(F("ble going on reset"));
      WLED::instance().reset();
    }
    else if (!bleOnFlag && m_bleInitted)
    {
      BLE_DEBUG_PRINTLN(F("ble going off reset"));
      WLED::instance().reset();
    }
  }

  BleReadOnlyService *createReadonlyService(uint16_t serviceId, uint16_t dataId, uint16_t controlId, BLEServer *pServer)
  {
    BleReadOnlyService *service = new BleReadOnlyService(serviceId, dataId, controlId);
    service->setupBle(pServer, m_gatts_if);
    return service;
  }

  void bleInit(uint32_t passkey)
  {
    BLE_DEBUG_PRINTLN(F("bleInit"));
    BLEDevice::init(WLED_BLE_2_JSON_NAME);
    BLEDevice::setCustomGattsHandler(gattServerEventHandler);
    BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
    BLEDevice::setSecurityCallbacks(new SecurityCallback());

    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallback());

    m_stateService = new BleStateInfoService();
    m_stateService->setupBle(pServer, m_gatts_if);

    m_paletteNamesService = createReadonlyService(WLED_BLE_PALETTE_NAME_SERVICE_ID,
                                                  WLED_BLE_PALETTE_NAME_DATA_ID,
                                                  WLED_BLE_PALETTE_NAME_CONTROL_ID,
                                                  pServer);

    m_fxDetailsService = createReadonlyService(WLED_BLE_FX_DETAILS_SERVICE_ID,
                                               WLED_BLE_FX_DETAILS_DATA_ID,
                                               WLED_BLE_FX_DETAILS_CONTROL_ID,
                                               pServer);

    m_fxNamesService = createReadonlyService(WLED_BLE_FX_NAMES_SERVICE_ID,
                                             WLED_BLE_FX_NAMES_DATA_ID,
                                             WLED_BLE_FX_NAMES_CONTROL_ID,
                                             pServer);

    m_presetsService = createReadonlyService(WLED_BLE_PRESETS_SERVICE_ID,
                                             WLED_BLE_PRESETS_DATA_ID,
                                             WLED_BLE_PRESETS_CONTROL_ID,
                                             pServer);

    m_paletteDetailsDataService = createReadonlyService(WLED_BLE_PALETTE_DETAILS_SERVICE_ID,
                                                        WLED_BLE_PALETTE_DETAILS_DATA_ID,
                                                        WLED_BLE_PALETTE_DETAILS_CONTROL_ID,
                                                        pServer);

    BLEDevice::startAdvertising();
    bleSecurity(passkey);
  }

  void checkUnPair()
  {
    if (m_config->getBleUnPairDevices())
    {
      m_config->setBleUnPairDevice(false);
      unPairAllDevices();
    }
  }

  void serviceLoop(BleServiceBase *service)
  {
    if (m_bleInitted && service != NULL)
    {
      service->loop();
    }
  }

public:
  BleMainSwitch(Ble2JsonConfig *config)
  {
    m_config = config;
  }

  void setup()
  {
    checkBleInit(true);
  }

  void loop()
  {
    if (millis() - lastTime > 1000)
    {
      lastTime = millis();
      checkBleInit(false);
    }

    serviceLoop(m_stateService);

    serviceLoop(m_fxNamesService);
    serviceLoop(m_fxDetailsService);
    serviceLoop(m_presetsService);
    serviceLoop(m_paletteNamesService);
    serviceLoop(m_paletteDetailsDataService);
  }

  /*
   * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
   * Values in the state object may be modified by connected clients
   */
  void readFromJsonState(JsonObject &root)
  {
    if (m_bleInitted && m_stateService != NULL)
    {
      m_stateService->readFromJsonState(root);
    }
  }
};