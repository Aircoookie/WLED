#pragma once

#ifdef ARDUINO_ARCH_ESP32

#include <stdint.h>
#include <vector>

#include "HKService.h"

#include "src/dependencies/json/ArduinoJson-v6.h"

using std::vector;

// Compromised of services and characteristics for each Accessory

class HKAccessory {
private:
    // Required: public.hap.service.accessory-information
    HKService * info_service = nullptr;

    // All other services
    vector<HKService *> services;

    // The lightbulb service.
    // HKService * lightbulb_service = nullptr;

public:
    // We will be using only one accessory so this can be set to one.
    static constexpr uint16_t aid = 1;

    HKAccessory(HKService* accessory_info_service);
    ~HKAccessory();

    HKService * getService(uint16_t iid);
    const vector<HKService *> & getServices();

    void addService(HKService *service);
    void removeService(uint64_t iid);

    int serialize_json(JsonObject doc);
};

#endif