#pragma once

#ifdef ARDUINO_ARCH_ESP32

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "HKDebug.h"
#include "HAPStatus.h"

#include "src/dependencies/json/ArduinoJson-v6.h"

#define LB_CHAR_ON_TYPE "25"
#define LB_CHAR_BRI_TYPE "8"
#define LB_CHAR_HUE_TYPE "13"
#define LB_CHAR_SAT_TYPE "2F"

// Character Permissions

typedef enum {
    PR = 1,
    PW = 2,
    EV = 4,
    AA = 8,
    TW = 16,
    HD = 32,
    WR = 64
} CharPerms;

/**
 * HAP characteristic value formats
 */
typedef enum {
    BOOL,
    INT,
    FLOAT,
    STRING
} CharFormat;

typedef enum {
    NONE,
    CELSIUS,
    PERCENTAGE,
    ARCDEG,
    LUX,
    SECONDS
} CharUnit;

class HAPCharacteristic {
    union CharValue {
        bool BOOL;
        int INT;
        float FLOAT;
        char *STRING = NULL;
    };

   private:
    uint16_t iid;
    const char *type;  // Characteristic Type
    CharValue value;
    uint8_t perms;

    // Optional
    CharFormat format;    // Characteristic Format
    const char *hapName;  // HAP Name

    CharValue minValue;   // Characteristic minimum (not applicable for STRING)
    CharValue maxValue;   // Characteristic maximum (not applicable for STRING)
    CharValue stepValue;  // step value, not applicable for string
    CharUnit unit;        // not applicable to string

    bool event_notif = false;  // currently turned off.

    template <class T>
    T internalGet(CharValue &u) {
        switch (format) {
            case CharFormat::BOOL:
                return ((T)u.BOOL);
            case CharFormat::INT:
                return ((T)u.INT);
            case CharFormat::FLOAT:
                return ((T)u.FLOAT);
            default:
                return 0;
        }
        return 0;  // included to prevent compiler warnings
    }

    void internalSet(CharValue &dest, CharValue &src) {
        if (format == CharFormat::STRING) {
            internalSet(dest, (const char *)src.STRING);
        } else {
            dest = src;
        }
    }

    void internalSet(CharValue &dest, const char *val) {
        dest.STRING = (char *)realloc(dest.STRING, strlen(val) + 1);
        strcpy(dest.STRING, val);
    }

    template <typename T>
    void internalSet(CharValue &dest, T val) {
        switch (format) {
            case CharFormat::BOOL:
                dest.BOOL = (bool)val;
                break;
            case CharFormat::INT:
                dest.INT = (int)val;
                break;
            case CharFormat::FLOAT:
                dest.FLOAT = (float)val;
                break;
            default:
                break;
        }  // switch
    }

   public:
    bool updated_value = false;


    HAPCharacteristic(uint16_t _iid, const char *_hapName, const char *_type, uint8_t _perms, const char *_val) {
        iid = _iid;
        hapName = _hapName;
        type = _type;
        perms = _perms;
        format = CharFormat::STRING;
        unit = CharUnit::NONE;

        internalSet(value, _val);
    }

    template <typename T, typename A = bool, typename B = bool, typename C = bool>
    HAPCharacteristic(uint16_t _iid, const char *_hapName, const char *_type, uint8_t _perms, CharFormat _format, T val, CharUnit _unit = CharUnit::NONE, A min = 0, B max = 1, C step = 1) {
        iid = _iid;
        hapName = _hapName;
        type = _type;
        perms = _perms;
        format = _format;
        unit = _unit;

        internalSet(value, val);
        internalSet(minValue, min);
        internalSet(maxValue, max);
        internalSet(stepValue, step);

        if (perms & CharPerms::EV) {
            event_notif = true;
        }
    }

    const char *getType() { return type; }
    uint16_t getId() { return iid; }
    uint8_t getPermissions() { return perms; }
    const char *getFormat() {
        switch (format) {
            case CharFormat::STRING:
                return "string";
            case CharFormat::FLOAT:
                return "float";
            case CharFormat::BOOL:
                return "bool";
            case CharFormat::INT:
                return "int";
            default:
                return "none";
        }
    }

    bool can_send_ev_notif() { return event_notif; }

    template <typename T>
    void setValue(T val, bool notify = true) {
        if ((perms & CharPerms::PW) == 0) {
            EHK_DEBUGF("\n*** WARNING:  Attempt to update Characteristic::%s with setVal() ignored.  No NOTIFICATION permission on this characteristic\n\n", hapName);
            return;
        }

        if (val < internalGet<T>(minValue) || val > internalGet<T>(maxValue)) {
            EHK_DEBUGF("\n*** WARNING:  Attempt to update Characteristic::%s with setVal(%.4f) is out of range [%.4f,%.4f].  This may cause device to become non-reponsive!\n\n", hapName, (float)val, internalGet<float>(minValue), internalGet<float>(maxValue));
        }

        internalSet(value, val);
        EHK_DEBUGF("Updating value for %s\n", hapName);
    }

    void setString(const char *val) {
        internalSet(value, val);
    }

    template <class T = int>
    T getMinValue() {
        return internalGet<T>(minValue);
    }

    template <class T = int>
    T getMaxValue() {
        return internalGet<T>(maxValue);
    }

    template <class T = int>
    T getStepValue() {
        return internalGet<T>(stepValue);
    }

    template <class T = int>
    T getValue() {
        return internalGet<T>(value);
    }

    const char *getStringValue() {
        if (format == CharFormat::STRING) {
            return value.STRING;
        }

        return NULL;
    }

    const char *getUnit() {
        switch (unit) {
            case CharUnit::NONE:
                return NULL;
            case CharUnit::CELSIUS:
                return "celsius";
            case CharUnit::PERCENTAGE:
                return "percentage";
            case CharUnit::ARCDEG:
                return "arcdegrees";
            case CharUnit::LUX:
                return "lux";
            case CharUnit::SECONDS:
                return "seconds";
        }
        return NULL;
    }

    int serialize_json(JsonObject obj, int aid = -1) {
        const char *format = getFormat();

        obj["iid"] = getId();

        if (aid == -1) {
            obj["type"] = getType();
            obj["format"] = format;

            // Create permissions array
            JsonArray json_permissions = obj.createNestedArray("perms");

            if (perms & CharPerms::AA) {
                json_permissions.add("aa");
            }
            if (perms & CharPerms::EV) {
                json_permissions.add("ev");
            }
            if (perms & CharPerms::HD) {
                json_permissions.add("hd");
            }
            if (perms & CharPerms::PW) {
                json_permissions.add("pw");
            }
            if (perms & CharPerms::TW) {
                json_permissions.add("tw");
            }
            if (perms & CharPerms::WR) {
                json_permissions.add("wr");
            }
            if (perms & CharPerms::PR) {
                json_permissions.add("pr");
            }
        } else {
            obj["aid"] = aid;
        }

        if (perms & CharPerms::PR) {
            // Set Value if PR is in permission
            if (strcmp(format, "string") == 0) {
                obj["value"] = getStringValue();
            } else if (strcmp(format, "bool") == 0) {
                obj["value"] = getValue<bool>();
            } else if (strcmp(format, "int") == 0) {
                obj["value"] = getValue<int>();
                if (aid == -1) {
                    obj["minValue"] = getMinValue<int>();
                    obj["maxValue"] = getMaxValue<int>();
                    obj["minStep"] = getStepValue<int>();
                    obj["unit"] = getUnit();
                }
            } else if (strcmp(format, "float") == 0) {
                obj["value"] = getValue<float>();
                if (aid == -1) {
                    obj["minValue"] = getMinValue<float>();
                    obj["maxValue"] = getMaxValue<float>();
                    obj["minStep"] = getStepValue<float>();
                    obj["unit"] = getUnit();
                }
            }
        }

        return 1;
    }

    HAPStatus deserialize_json(JsonObject obj) {
        if (obj.containsKey("ev")) {
            if (perms & CharPerms::EV) {
                event_notif = obj["ev"].as<bool>();
            } else {
                return HAPStatus::NOTIFICATION_NOT_SUPPORTED;
            }
        }

        if (obj.containsKey("value")) {
            if (perms & CharPerms::PW) {
                switch (format) {
                    case CharFormat::STRING:
                        setString(obj["value"].as<const char *>());
                        break;
                    case CharFormat::FLOAT:
                        setValue(obj["value"].as<float>());
                        break;
                    case CharFormat::BOOL:
                        setValue(obj["value"].as<bool>());
                        break;
                    case CharFormat::INT:
                        setValue(obj["value"].as<int>());
                        break;
                    default:
                        break;
                }
                if (event_notif) {
                    updated_value = true;
                }
            } else {
                // Need write permission
                return HAPStatus::READ_ONLY_CHARACTERISTIC;
            }
        }
        return HAPStatus::SUCCESS;
    }
};

#endif