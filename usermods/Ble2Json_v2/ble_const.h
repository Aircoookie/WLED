#pragma once

#include <BLEUtils.h>

#define WLED_BLE_2_JSON_NAME "WLED BLE2JSON"
#define WLED_BLE_SERVICE_ID "bee3d55e-a2eb-4f7a-889b-13192c8c1819"

#define WLED_BLE_MAIN_SERVICE_ID 0x0000
#define WLED_BLE_MAIN_BLE_ON_ID 0x0001

#define WLED_BLE_DATA_SERVICE_ID 0x0100
#define WLED_BLE_STATE_INFO_DATA_ID 0x0101
#define WLED_BLE_STATE_INFO_CONTROL_ID 0x0102
#define WLED_BLE_STATE_NOTIFY_ID 0x103

#define BLE_UUID(id) createId(WLED_BLE_SERVICE_ID, id)

BLEUUID createId(std::string value, uint16_t id)
{
  BLEUUID bleUuid = BLEUUID(value);
  esp_bt_uuid_t *baseUuid = bleUuid.getNative();

  memcpy(&baseUuid->uuid.uuid128[12], &id, sizeof(id));

  return bleUuid;
}
