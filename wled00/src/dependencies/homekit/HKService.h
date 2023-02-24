#pragma once

#include <stdint.h>
#include <vector>

#include "HAPCharacteristic.h"
#include "src/dependencies/json/ArduinoJson-v6.h"

constexpr auto AccessoryInfoServiceType = "3E";   // Accessory info service
constexpr auto ProtocolInfoServiceType = "A2";    // Protocol Info Service
constexpr auto LightServiceType = "43";           // Light Service

using std::vector;

// Group functionality
class HKService {
private:
    uint16_t iid;                                       // Id Name
    const char *type;                                         // Service Type
    const char *hapName;                                      // Service Name
    bool hidden = false;                                // optional property indicating service is hidden
    bool primary = false;                               // optional property indicating service is primary
    vector<HAPCharacteristic *> characteristics;

public:
    HKService(uint16_t _iid, const char* _type, const char* _hapName);

    void addCharacteristic(HAPCharacteristic *characteristic);

    const vector<HAPCharacteristic *>& getCharacteristics();
    const char * getType() { return type; };
    uint16_t getId() { return iid; };

    int serialize_json(JsonObject obj);
};
