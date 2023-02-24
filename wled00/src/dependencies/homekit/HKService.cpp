#include "HKService.h"
// #include "wled.h"

#ifdef ARDUINO_ARCH_ESP32

HKService::HKService(uint16_t _iid, const char *_type, const char *_hapName) {
    iid = _iid;
    type = _type;
    hapName = _hapName;
}

void HKService::addCharacteristic(HAPCharacteristic *characteristic) {
    characteristics.push_back(characteristic);
}

const vector<HAPCharacteristic *> &HKService::getCharacteristics() {
    return (const vector<HAPCharacteristic *> &)characteristics;
}

int HKService::serialize_json(JsonObject obj) {
    obj["iid"] = getId();
    obj["type"] = getType();

    JsonArray json_characteristics = obj.createNestedArray("characteristics");

    for (int j = 0; j < characteristics.size(); j++) {
        HAPCharacteristic *characteristic = characteristics[j];
        JsonObject json_characteristic = json_characteristics.createNestedObject();

        characteristic->serialize_json(json_characteristic);
    }
    return 1;
}

#endif