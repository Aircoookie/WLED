#include "HKStore.h"

#ifdef ESP32

#include <sodium/randombytes.h>
#include <mdns.h>

#include "HKDebug.h"
#include "wled.h"
#include "my_config.h"

#define HK_NVS_STORE_NAME "hk_store"
#define HK_PAIRING_STORE_KEY "hk_pairings"
#define HK_ACCESSORY_KEYS_KEY "hk_acc_keys"
#define HK_ACCESSORY_ID_KEY "hk_acc_id_key"

HKStore& HKStore::get_instance() {
    static HKStore instance;
    return instance;
}

HKStore::HKStore() {
    //////////////////////////////
    // Start storage partition  //
    //////////////////////////////

    esp_err_t err = nvs_flash_init();

    if (!err) {
        err = nvs_open(HK_NVS_STORE_NAME, NVS_READWRITE, &hk_store_handle);
    } else {
        EHK_DEBUGLN("There was an error starting homekit storage.");
        return;
    }

    size_t val_size = sizeof(acc_id);
    accessory_keys keys;
    if (nvs_get_blob(hk_store_handle, HK_ACCESSORY_ID_KEY, acc_id, &val_size) == ESP_OK) {
        val_size = crypto_sign_ed25519_PUBLICKEYBYTES + crypto_sign_ed25519_SECRETKEYBYTES;
        nvs_get_blob(hk_store_handle, HK_ACCESSORY_KEYS_KEY, &keys, &val_size);
    } else {
        EHK_DEBUGLN("-- Generating new accessory id and public and private key.");
        esp_fill_random(acc_id, 6);
        nvs_set_blob(hk_store_handle, HK_ACCESSORY_ID_KEY, acc_id, sizeof(acc_id));

        val_size = sizeof(keys.ltpk) + sizeof(keys.ltsk);
        memset(keys.ltpk, 0, crypto_sign_ed25519_PUBLICKEYBYTES);
        memset(keys.ltsk, 0, crypto_sign_ed25519_SECRETKEYBYTES);
        crypto_sign_ed25519_keypair(keys.ltpk, keys.ltsk);
        nvs_set_blob(hk_store_handle, HK_ACCESSORY_KEYS_KEY, &keys, val_size);
        nvs_commit(hk_store_handle);
    }

    val_size = sizeof(pairings);
    if (nvs_get_blob(hk_store_handle, HK_PAIRING_STORE_KEY, pairings, &val_size) != ESP_OK) {
        EHK_DEBUGLN("-- Accessory has no pairings stored");
        memset(pairings, 0, sizeof(pairings));
        nvs_set_blob(hk_store_handle, HK_PAIRING_STORE_KEY, pairings, sizeof(pairings));
        nvs_commit(hk_store_handle);
    }

    //////////////////////////////////////
    // Create services and accessories  //
    //////////////////////////////////////

    // Create accessory info service. Must have an ID of 1.
    HKService *accessory_info_service = new HKService(1, AccessoryInfoServiceType, "AccessoryInformation");
    // Name Characteristic
    accessory_info_service->addCharacteristic(new HAPCharacteristic(2, "Name", "23", CharPerms::PR, "WLED Light"));
    // Manufacturer Characteristic
    accessory_info_service->addCharacteristic(new HAPCharacteristic(3, "Manufacturer", "20", CharPerms::PR, "WLED"));
    // SerialNumber Characteristic
    accessory_info_service->addCharacteristic(new HAPCharacteristic(4, "SerialNumber", "30", CharPerms::PR, "WLED-001"));
    // Model Characteristic
    accessory_info_service->addCharacteristic(new HAPCharacteristic(5, "Model", "21", CharPerms::PR, "WLED-Device"));
    // Identity Characteristic
    accessory_info_service->addCharacteristic(new HAPCharacteristic(6, "Identify", "14", CharPerms::PW, CharFormat::BOOL, true));
    // FirmwareRevision Characteristic
    accessory_info_service->addCharacteristic(new HAPCharacteristic(7, "FirmwareRevision", "52", CharPerms::PR, "0.0.1"));

    // Craate ProtocolInfo service
    HKService *protocol_info_service = new HKService(8, ProtocolInfoServiceType, "HAPProtocolInformation");
    // Version Characteristic
    protocol_info_service->addCharacteristic(new HAPCharacteristic(9, "Version", "37", CharPerms::PR, "1.1.0"));

    // Create LightBulb service
    HKService *light_bulb_service = new HKService(10, LightServiceType, "LightBulb");
    // On Characteristic
    light_bulb_service->addCharacteristic(new HAPCharacteristic(11, "On", LB_CHAR_ON_TYPE, CharPerms::PR + CharPerms::PW + CharPerms::EV, CharFormat::BOOL, true));
    // Brightness Characteristic
    light_bulb_service->addCharacteristic(new HAPCharacteristic(12, "Brightness", LB_CHAR_BRI_TYPE, CharPerms::PR + CharPerms::PW + CharPerms::EV, CharFormat::INT, 0, CharUnit::PERCENTAGE, 0, 100));
    // Hue Characteristic
    light_bulb_service->addCharacteristic(new HAPCharacteristic(13, "Hue", LB_CHAR_HUE_TYPE, CharPerms::PR + CharPerms::PW + CharPerms::EV, CharFormat::FLOAT, 0, CharUnit::ARCDEG, 0, 360));
    // Saturation Characteristic
    light_bulb_service->addCharacteristic(new HAPCharacteristic(14, "Saturation", LB_CHAR_SAT_TYPE, CharPerms::PR + CharPerms::PW + CharPerms::EV, CharFormat::FLOAT, 0, CharUnit::PERCENTAGE,0, 100));

    // Set Accessory services
    accessory = new HKAccessory(accessory_info_service);
    accessory->addService(protocol_info_service);
    accessory->addService(light_bulb_service);

    srp.generate_verifier(HK_PAIRING_CODE);

    pair_status = pairState_M1;
}

// LightBulb

void HKStore::set_lightbulb_values(bool on, uint16_t hue, uint8_t sat, uint8_t bri) {
    HKService * light_bulb_service = accessory->getService(10);

    if (light_bulb_service == nullptr) {
        return;
    }

    const vector<HAPCharacteristic *>& characteristics = light_bulb_service->getCharacteristics();

    for (int i = 0; i < characteristics.size(); i++) {
        HAPCharacteristic * characteristic = characteristics.at(i);
        if (characteristic != nullptr) {
            characteristic->updated_value = true;
            if (characteristic->getType() == LB_CHAR_ON_TYPE) {
                characteristic->setValue(on);
            } else if (characteristic->getType() == LB_CHAR_HUE_TYPE) {
                characteristic->setValue(hue);
            } else if (characteristic->getType() == LB_CHAR_SAT_TYPE) {
                characteristic->setValue(sat);
            } else if (characteristic->getType() == LB_CHAR_BRI_TYPE) {
                characteristic->setValue(bri);
            } else {
                characteristic->updated_value = false;
            }
        }
    }
}

void HKStore::get_lightbulb_values(bool * on, int * hue, int * sat, int * bri) {
    HKService * light_bulb_service = accessory->getService(10);

    if (light_bulb_service == nullptr) {
        return;
    }

    const vector<HAPCharacteristic *>& characteristics = light_bulb_service->getCharacteristics();

    for (int i = 0; i < characteristics.size(); i++) {
        HAPCharacteristic * characteristic = characteristics.at(i);
        if (characteristic != nullptr) {
            if (characteristic->getType() == LB_CHAR_ON_TYPE) {
                *on = characteristic->getValue<bool>();
            } else if (characteristic->getType() == LB_CHAR_HUE_TYPE) {
                *hue = characteristic->getValue<int>();
            } else if (characteristic->getType() == LB_CHAR_SAT_TYPE) {
                *sat = characteristic->getValue<int>();
            } else if (characteristic->getType() == LB_CHAR_BRI_TYPE) {
                *bri = characteristic->getValue<int>();
            }
        }
    }
}

//////////////////////////
//      Accessory       //
//////////////////////////
void HKStore::get_accessory_id(uint8_t * data) {
    memcpy(data, acc_id, 6);
}

void HKStore::get_serialized_accessory_id(char * accessory_id) {
    uint8_t data[6];
    get_accessory_id(data);
    sprintf(accessory_id, "%02X:%02X:%02X:%02X:%02X:%02X", data[0], data[1], data[2], data[3], data[4], data[5]);    
}

int HKStore::get_accessory_keys(accessory_keys * keys) {
    size_t keys_len;

    if (nvs_get_blob(hk_store_handle, HK_ACCESSORY_KEYS_KEY, keys, &keys_len) != ESP_OK) {
        EHK_DEBUGLN("There was an error retrieving accessory keys.");
        return 0;
    } else {
        return 1;
    }
}

int HKStore::add_pairing(const char * device_id, size_t dev_id_len, uint8_t * device_ltpk, bool is_admin) {
    pairing_info * slot = find_pairing(device_id);

    bool valid = false;

    if (slot) {
        if (device_ltpk != nullptr) {
            memcpy(slot->device_ltpk, device_ltpk, crypto_sign_ed25519_PUBLICKEYBYTES);
        }
        slot->is_admin = is_admin;
        EHK_DEBUGLN("\nUpdated pairing.");
        valid = true;
    } else if ((slot = get_free_pairing())) {
        if (device_ltpk == nullptr) {
            valid = false;
        } else {
            slot->allocated = true;
            memcpy(slot->device_ltpk, device_ltpk, crypto_sign_ed25519_PUBLICKEYBYTES);
            memcpy(slot->device_pair_id, device_id, dev_id_len);
            slot->is_admin = is_admin;
            valid = true;
        }
    }

    if (valid) {
        nvs_set_blob(hk_store_handle, HK_PAIRING_STORE_KEY, pairings, sizeof(pairings));
        nvs_commit(hk_store_handle);
    }

    return valid;
}

pairing_info * HKStore::get_free_pairing() {
    for (int i = 0; i < MAX_PAIRINGS; i++) {
        if (!pairings[i].allocated) {
            return pairings + i;
        }
    }

    return NULL;
}

pairing_info * HKStore::find_pairing(const char * device_id) {
    for (int i = 0; i < MAX_PAIRINGS; i++) {
        pairing_info info = pairings[i];
        if (info.allocated && !strcmp(info.device_pair_id, device_id)) {
            return pairings + i;
        }
    }

    return NULL;
}

bool HKStore::has_admin_pairings() {
    for (int i = 0; i < MAX_PAIRINGS; i++) {
        if (pairings[i].allocated && pairings[i].is_admin) {
            return true;
        }
    }

    return false;
}

void HKStore::remove_pairing(const char * pairing_id) {
    pairing_info * slot = find_pairing(pairing_id);

    if (slot) {
        slot->allocated = false;
    }

    if (!has_admin_pairings()) {
        remove_pairings();
        mdns_service_txt_item_set("_hap","_tcp","sf","1");
    }

    nvs_set_blob(hk_store_handle, HK_PAIRING_STORE_KEY, pairings, sizeof(pairings));
    nvs_commit(hk_store_handle);
}

void HKStore::remove_pairings() {
    for (int i = 0; i < MAX_PAIRINGS; i++) {
        pairings[i].allocated = false;
    }
}

HAPCharacteristic * HKStore::get_characteristic(int iid) {
    const vector<HKService *> & services = accessory->getServices();
    for (int i = 0; i < services.size(); i++) {
        HKService * service = services.at(i);
        const vector<HAPCharacteristic *> & characteristics = service->getCharacteristics();

        for (int j = 0; j < characteristics.size(); j++) {
            HAPCharacteristic * characteristic = characteristics.at(j);

            if (iid == characteristic->getId()) {
                return characteristic;
            }
        }
    }
    return NULL;
}

char * HKStore::serialize_accessory(int * json_len) {
    #ifdef WLED_USE_DYNAMIC_JSON
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    #else
    if (!requestJSONBufferLock(18)) return NULL;
    #endif

    JsonArray json_accessories = doc.createNestedArray("accessories");
    JsonObject json_main_accessory = json_accessories.createNestedObject();

    if (!accessory->serialize_json(json_main_accessory)) {
        *json_len = 0;
        releaseJSONBufferLock();
        return NULL;
    }

    int size = measureJson(doc);;
    char * output = new char[size + 1];
    serializeJson(doc, output, size);
    output[size] = '\0';

    *json_len = size;
    releaseJSONBufferLock();
    return output;
}

int HKStore::deserialize_characteristics(const char * json_buf, size_t json_buf_len, char * output_buf) {
    #ifdef WLED_USE_DYNAMIC_JSON
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    #else
    if (!requestJSONBufferLock(19)) return -1;
    #endif

    DeserializationError err = deserializeJson(doc, json_buf, json_buf_len);

    if (err) {
        EHK_DEBUGF("\n\nFailed to deserialize characteristics. %s\n\n", err.c_str());
        releaseJSONBufferLock();
        return -1;
    }

    if (!doc.containsKey("characteristics")) {
        EHK_DEBUGLN("\n\nJSON does not have characteristics key.\n\n");
        releaseJSONBufferLock();
        return -1;
    }

    int error_count = 0;

    JsonArray json_char_array = doc["characteristics"];
    for (int i = 0; i < json_char_array.size(); i++) {
        JsonObject json_char_obj = json_char_array.getElement(i);

        int aid = json_char_obj["aid"].as<int>();
        int iid = json_char_obj["iid"].as<int>();

        HAPCharacteristic * characteristic = get_characteristic(iid);
        if (!characteristic) {
            // Write error in json
            json_char_obj.clear();
            json_char_obj["aid"] = aid;
            json_char_obj["iid"] = iid;
            json_char_obj["status"] = HAPStatus::RESOURCE_INEXISTENT;
            error_count++;
            continue;
        }

        int status = 0;
        if ((status = characteristic->deserialize_json(json_char_obj)) != SUCCESS) {
            // write error here.
            json_char_obj.clear();
            json_char_obj["aid"] = aid;
            json_char_obj["iid"] = iid;
            json_char_obj["status"] = status;
            error_count++;
            continue;
        }
    }

    int output_buf_len = 0;

    if (error_count > 0) {
        // There were errors, so now we need to notify the errors
        for (int i = 0; i < json_char_array.size(); i++) {
            JsonObject json_char_obj = json_char_array.getElement(i);

            int aid = json_char_obj["aid"].as<int>();
            int iid = json_char_obj["iid"].as<int>();

            bool contains_status = json_char_obj.containsKey("status");
            if (!contains_status) {
                json_char_obj.clear();
                json_char_obj["aid"] = aid;
                json_char_obj["iid"] = iid;
                json_char_obj["status"] = HAPStatus::SUCCESS;
            }
        }

        output_buf_len = measureJson(doc);
        output_buf = new char[output_buf_len + 1];
        serializeJson(doc, output_buf, output_buf_len);
        output_buf[output_buf_len] = '\0';
    }

    releaseJSONBufferLock();
    return output_buf_len;
}

char * HKStore::serialize_characteristics(int * json_len, const int * ids, size_t ids_size) {
    #ifdef WLED_USE_DYNAMIC_JSON
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    #else
    if (!requestJSONBufferLock(20)) return NULL;
    #endif

    JsonArray json_characteristics = doc.createNestedArray("characteristics");
    for (int i = 0; i < ids_size; i++) {
        int char_iid = ids[i];
        HAPCharacteristic * characteristic = get_characteristic(char_iid);
        if (!characteristic) {
            releaseJSONBufferLock();
            return NULL;
        }

        JsonObject json_char = json_characteristics.createNestedObject();
        characteristic->serialize_json(json_char, accessory->aid);
    }

    int size = measureJson(doc);;
    char * output = new char[size + 1];
    serializeJson(doc, output, size);
    output[size] = '\0';
    *json_len = size;
    releaseJSONBufferLock();
    return output;
}

char * HKStore::serialize_events(int * json_len) {
    bool should_send_ev_notif = false;
    const vector<HKService *>& services = accessory->getServices();

    // Check if it's even necessary before locking json.
    for (int i = 0; i < services.size(); i++) {
        HKService * service = services.at(i);
        const vector<HAPCharacteristic *>& characteristics = service->getCharacteristics();
        for (int j = 0; j < characteristics.size(); j++) {
            HAPCharacteristic * characteristic = characteristics.at(j);
            if (characteristic->can_send_ev_notif() && characteristic->updated_value) {
                should_send_ev_notif = true;
                break;
            }
        }
    }

    if (!should_send_ev_notif) { 
        // do not lock json buffer if not necessary
        *json_len = 0;
        return NULL; 
    }

    #ifdef WLED_USE_DYNAMIC_JSON
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    #else
    if (!requestJSONBufferLock(21)) return NULL;
    #endif

    JsonArray json_characteristics = doc.createNestedArray("characteristics");

    for (int i = 0; i < services.size(); i++) {
        HKService * service = services.at(i);
        const vector<HAPCharacteristic *>& characteristics = service->getCharacteristics();

        for (int j = 0; j < characteristics.size(); j++) {
            HAPCharacteristic * characteristic = characteristics.at(j);

            if (characteristic->can_send_ev_notif() && characteristic->updated_value) {
                JsonObject json_char = json_characteristics.createNestedObject();
                json_char["aid"] = accessory->aid;
                json_char["iid"] = characteristic->getId();
                json_char["value"] = characteristic->getValue();                
                characteristic->updated_value = false;
            }
        }
    }

    int size = measureJson(doc);
    char * json = new char[size + 1];;
    serializeJson(doc, json, size);
    json[size] = '\0';        
    *json_len = size;
    releaseJSONBufferLock();
    return json;
}
#endif