#include "HKAccessory.h"

#ifdef ARDUINO_ARCH_ESP32

HKAccessory::HKAccessory(HKService *accessory_info_service) {
    if (!accessory_info_service) {
        Serial.println("Accessory info service is zero. This is not supposed to happen.");
        return;
    }

    this->info_service = accessory_info_service;
    this->services.push_back(accessory_info_service);
}

void HKAccessory::addService(HKService *service) {
    services.push_back(service);
}

HKService * HKAccessory::getService(uint16_t _iid) {
    for (int i = 0; i < services.size(); i++) {
        HKService * service = services.at(i);
        if (service != nullptr && service->getId() == _iid) {
            return service;
        }
    }

    return nullptr;
}

const vector<HKService *> &HKAccessory::getServices() {
    return (const vector<HKService *> &)services;
}

int HKAccessory::serialize_json(JsonObject obj) {
    obj["aid"] = aid;

    JsonArray json_services = obj.createNestedArray("services");

    for (int i = 0; i < services.size(); i++) {
        HKService *service = services[i];
        JsonObject json_service = json_services.createNestedObject();
        service->serialize_json(json_service);
    }

    return 1;
}

#endif