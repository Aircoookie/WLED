#pragma once

#include "wled.h"
#include "IP5306_I2C.h"
#include <driver/adc.h>

// the frequency to check battery voltage, 5 seconds
#ifndef USERMOD_BATTERY_MEASUREMENT_INTERVAL
  #define USERMOD_BATTERY_MEASUREMENT_INTERVAL 5000
#endif

#ifndef USERMOD_BATTERY_ADC_PRECISION
  #define USERMOD_BATTERY_ADC_PRECISION 4095.0f
#endif

#ifndef USERMOD_BATTERY_ADC_REF
  #define USERMOD_BATTERY_ADC_REF 3.3f
#endif

#ifndef USERMOD_BATTERY_ADC_R1
  #define USERMOD_BATTERY_ADC_R1 8.0f
#endif

#ifndef USERMOD_BATTERY_ADC_R2
  #define USERMOD_BATTERY_ADC_R2 2.5f
#endif

#ifndef USERMOD_BATTERY_MIN_VOLTAGE
  #define USERMOD_BATTERY_MIN_VOLTAGE 2.6f
#endif

#ifndef USERMOD_BATTERY_MAX_VOLTAGE
  #define USERMOD_BATTERY_MAX_VOLTAGE 4.2f
#endif

#ifndef USERMOD_IP5306_SDA
  #define USERMOD_IP5306_SDA 32
#endif

#ifndef USERMOD_IP5306_SCL
  #define USERMOD_IP5306_SCL 33
#endif

#ifndef USERMOD_ABL_BATTERY
  #define USERMOD_ABL_BATTERY 1250
#endif

#ifndef USERMOD_ABL_EXT
  #define USERMOD_ABL_EXT 2500
#endif

class UsermodULCBatteryManagement : public Usermod {

  private:
    IP5306 *ip5306 = nullptr;

    bool initDone = false;
    bool initializing = true;
    // GPIO pin used for battery measurment (with a default compile-time fallback)
    int8_t ip5306_sda = USERMOD_IP5306_SDA;
    int8_t ip5306_scl = USERMOD_IP5306_SCL;
    // how often do we measure?
    unsigned long readingInterval = USERMOD_BATTERY_MEASUREMENT_INTERVAL;
    unsigned long nextReadTime = 0;
    unsigned long lastReadTime = 0;

    uint32_t ablBattery = USERMOD_ABL_BATTERY;
    uint32_t ablExt = USERMOD_ABL_EXT;
    
    // battery min. voltage
    float minBatteryVoltage = USERMOD_BATTERY_MIN_VOLTAGE;
    // battery max. voltage
    float maxBatteryVoltage = USERMOD_BATTERY_MAX_VOLTAGE;

    uint8_t batteryLevel = 0; 
    bool isCharging = false;
    bool isPluggedIn = false;

    // strings to reduce flash memory usage (used more than twice)
    static const char _name[];
    static const char _readInterval[];
    static const char _maxCurrentBattery[];
    static const char _maxCurrentExt[];

    // custom map function
    // https://forum.arduino.cc/t/floating-point-using-map-function/348113/2
    double mapf(double x, double in_min, double in_max, double out_min, double out_max) 
    {
      return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    float truncate(float val, byte dec) 
    {
      float x = val * pow(10, dec);
      float y = round(x);
      float z = x - y;
      if ((int)z == 5)
      {
          y++;
      }
      x = y / pow(10, dec);
      return x;
    }

  public:

    void setup() {
      DEBUG_PRINTLN(F("Allocating ip5306 pins..."));
      PinManagerPinType pins[2] = { { ip5306_sda, true }, { ip5306_scl, true } };
      if (pinManager.allocateMultiplePins(pins, 2, PinOwner::HW_I2C))
      {
        ip5306 = new IP5306(ip5306_sda, ip5306_scl);
        DEBUG_PRINTLN(F("IP5306 allocation succeeded."));
      } else {
        if (ip5306_sda >= 0 && ip5306_scl >= 0) DEBUG_PRINTLN(F("IP5306 allocation failed."));
        ip5306_sda = -1;  // allocation failed
        ip5306_scl = -1;
      }

      nextReadTime = millis() + 5000;
      lastReadTime = millis();

      if (ip5306 != nullptr) {
        //set battery voltage
        ip5306->set_battery_voltage(BATT_VOLTAGE_0);   //4.2V

        //Voltage Vout for charging
        ip5306->charger_under_voltage(VOUT_5);         //4.7V
        
        //set charging complete current
        ip5306->end_charge_current(CURRENT_200);       //400mA
        
        //set cutoff voltage
        ip5306->set_charging_stop_voltage(CUT_OFF_VOLTAGE_3);    // 4.2/4.305/4.35/4.395  V   

        //enable low battery shutdown mode
        ip5306->low_battery_shutdown(DISABLE);

        //allow boost even after removing Vin
        ip5306->boost_after_vin(ENABLE);

        //allow auto power on after load detection
        ip5306->power_on_load(DISABLE);

        //enable boost mode
        ip5306->boost_mode(ENABLE);
      }

      initDone = true;
    }

    void loop() {
      if(strip.isUpdating()) return;

      // check the battery level every USERMOD_BATTERY_MEASUREMENT_INTERVAL (ms)
      if (millis() < nextReadTime) return;

      nextReadTime = millis() + readingInterval;
      lastReadTime = millis();
      initializing = false;

      if (ip5306 != nullptr) {
        isPluggedIn = ip5306->check_charging_status();
        isCharging = !ip5306->check_battery_status();
        batteryLevel = ip5306->get_battery_charge();

        if (isPluggedIn) {
          strip.ablMilliampsMax = ablExt;
        } else {
          strip.ablMilliampsMax = ablBattery;
        }
      }
    }

    /*
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
     * Below it is shown how this could be used for e.g. a light sensor
     */
    void addToJsonInfo(JsonObject& root) {
      JsonObject user = root["u"];
      if (user.isNull()) user = root.createNestedObject("u");

      // info modal display names
      JsonArray batteryPercentage = user.createNestedArray("Battery Level");
      JsonArray batteryState = user.createNestedArray("Battery State");

      if (initializing) {
        batteryPercentage.add((nextReadTime - millis()) / 1000);
        batteryPercentage.add(" sec");
        //batteryVoltage.add((nextReadTime - millis()) / 1000);
        //batteryVoltage.add(" sec");
        batteryState.add((nextReadTime - millis()) / 1000);
        batteryState.add(" sec");
        return;
      }

      if(batteryLevel < 0) {
        batteryPercentage.add(F("invalid"));
      } else {
        batteryPercentage.add(batteryLevel);
      }
      batteryPercentage.add(F(" %"));

      if (isPluggedIn && isCharging) {
        batteryState.add("Charging");
      } else if (isPluggedIn && !isCharging) {
        batteryState.add("Charged");
      } else if (!isPluggedIn) {
        batteryState.add("Discharging");
      }
    }

    /**
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    //void addToJsonState(JsonObject &root)
    //{
    //}

    /**
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     * Read "<usermodname>_<usermodparam>" from json state and and change settings (i.e. GPIO pin) used.
     */
    //void readFromJsonState(JsonObject &root) {
    //  if (!initDone) return;  // prevent crash on boot applyPreset()
    //}

    /**
     * addToConfig() (called from set.cpp) stores persistent properties to cfg.json
     */
    void addToConfig(JsonObject &root) {
      JsonObject battery = root.createNestedObject(FPSTR(_name)); // usermodname
      //battery["minBatteryVoltage"] = minBatteryVoltage;           // usermodparam
      //battery["maxBatteryVoltage"] = maxBatteryVoltage;           // usermodparam
      battery[FPSTR(_readInterval)] = readingInterval;
      battery[FPSTR(_maxCurrentBattery)] = ablBattery;
      battery[FPSTR(_maxCurrentExt)] = ablExt;

      DEBUG_PRINTLN(F("Battery config saved."));
    }

    /**
     * readFromConfig() is called before setup() to populate properties from values stored in cfg.json
     *
     * The function should return true if configuration was successfully loaded or false if there was no configuration.
     */
    bool readFromConfig(JsonObject &root) {
      JsonObject battery = root[FPSTR(_name)];
      if (battery.isNull()) 
      {
        DEBUG_PRINT(FPSTR(_name));
        DEBUG_PRINTLN(F(": No config found. (Using defaults.)"));
        return false;
      }

      readingInterval     = battery[FPSTR(_readInterval)] | readingInterval;
      readingInterval     = max(3000, (int)readingInterval); // minimum repetition is >5000ms (5s)
      ablBattery          = battery[FPSTR(_maxCurrentBattery)] | ablBattery;
      ablExt              = battery[FPSTR(_maxCurrentExt)] | ablExt;

      DEBUG_PRINT(FPSTR(_name));

      return !battery[FPSTR(_readInterval)].isNull();
    }

    uint16_t getId()
    {
      return USERMOD_ID_ULC_BATTERYMANAGEMENT;
    }
};

// strings to reduce flash memory usage (used more than twice)
const char UsermodULCBatteryManagement::_name[]         PROGMEM = "Battery-level";
const char UsermodULCBatteryManagement::_readInterval[] PROGMEM = "Battery status update interval";
const char UsermodULCBatteryManagement::_maxCurrentBattery[] PROGMEM = "Max current when on battery";
const char UsermodULCBatteryManagement::_maxCurrentExt[] PROGMEM = "Max current when on charger";
