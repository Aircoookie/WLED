#pragma once

#include <BLEUtils.h>

#define WLED_BLE_2_JSON_NAME "WLED BLE2JSON"
#define WLED_BLE_SERVICE_ID "bee3d55e-a2eb-4f7a-889b-13192c8c1819"

#define WLED_BLE_MAIN_SERVICE_ID 0x0000
#define WLED_BLE_MAIN_BLE_ON_ID 0x0001

#define WLED_BLE_STATE_SERVICE_ID 0x0100
#define WLED_BLE_STATE_DATA_ID 0x0101
#define WLED_BLE_STATE_CONTROL_ID 0x0102

#define WLED_BLE_INFO_SERVICE_ID 0x0200
#define WLED_BLE_INFO_DATA_ID 0x0201
#define WLED_BLE_INFO_CONTROL_ID 0x0202

#define WLED_BLE_CFG_SERVICE_ID 0x0300
#define WLED_BLE_CFG_DATA_ID 0x0301
#define WLED_BLE_CFG_CONTROL_ID 0x0302

#define BLE_UUID(id) createId(WLED_BLE_SERVICE_ID, id)

BLEUUID createId(std::string value, uint16_t id)
{
  BLEUUID bleUuid = BLEUUID(value);
  esp_bt_uuid_t *baseUuid = bleUuid.getNative();

  memcpy(&baseUuid->uuid.uuid128[12], &id, sizeof(id));

  return bleUuid;
}
