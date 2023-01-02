#pragma once

#include "wled.h"
#include "BleMainSwitch.h"

#define PASSKEY 999999

/*
 * usermod to add bluetooth low energy interface to wled
 * see readme for installation instructions
 */

class Ble2JsonUsermod : public Usermod, Ble2JsonConfig
{
private:
  // for the loop
  unsigned long lastTime = 0;

  // Settings
  bool m_bleOnFlag = false;
  uint32_t m_blePairingPin = PASSKEY;
  bool m_bleUnPairDevices = false;

  // so we can update the config in the loop
  bool m_configDirty = false;

  BleMainSwitch *m_mainSwitch;

public:
  // Functions called by WLED

  /*
   * setup() is called once at boot. WiFi is not yet connected at this point.
   * You can use it to initialize variables, sensors or similar.
   *
   * this will check to see if ble should be enabled and, if so, will disable wifi
   */
  void setup()
  {
    m_mainSwitch = new BleMainSwitch(this);

    m_mainSwitch->setup();
  }

  /*
   * loop() is called continuously. Here you can check for events, read sensors, etc.
   *
   */
  void loop()
  {
    if (millis() - lastTime > 1000)
    {
      lastTime = millis();

      if (m_configDirty)
      {
        serializeConfig();
      }
    }
    m_mainSwitch->loop();
  }

  /*
   * put in the ble.support flag to let devices know this
   * is ble capable wled instance
   */
  void addToJsonInfo(JsonObject &root)
  {
    JsonObject ble = root["ble"];
    if (ble.isNull())
      ble = root.createNestedObject("ble");

    ble["support"] = true;
  }

  /*
   * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
   * Values in the state object may be modified by connected clients
   *
   * if "bleToggle" is true then toggle between ble and wifi
   */
  void readFromJsonState(JsonObject &root)
  {
    m_mainSwitch->readFromJsonState(root);

    // toggle ble on (applies only once)
    if (root["bleToggle"])
    {
      setBleOnFlag(!m_bleOnFlag);
    }
  }

  /*
   * addToConfig()
   *
   */
  void addToConfig(JsonObject &root)
  {
    JsonObject top = root.createNestedObject("ble2jsonUsermod");
    top["bleOnFlag"] = m_bleOnFlag;
    top["blePairingPin"] = m_blePairingPin;
    top["bleUnPairDevices"] = m_bleUnPairDevices;

    BLE_DEBUG_PRINTF(F("add: %u %d\n"), m_blePairingPin, m_bleOnFlag);

    BLE_DEBUG_PRINTLN(F("Add to config"));
  }

  /*
   * readFromConfig()
   */
  bool readFromConfig(JsonObject &root)
  {
    BLE_DEBUG_PRINTLN("read from config");

    JsonObject top = root["ble2jsonUsermod"];

    bool configComplete = !top.isNull();

    configComplete &= getJsonValue(top["bleOnFlag"], m_bleOnFlag, false);
    configComplete &= getJsonValue(top["blePairingPin"], m_blePairingPin, PASSKEY);
    configComplete &= getJsonValue(top["bleUnPairDevices"], m_bleUnPairDevices, false);

    BLE_DEBUG_PRINTF(F("read: %u\n"), m_blePairingPin);

    return configComplete;
  }

  /*
   * getId()
   */
  uint16_t getId()
  {
    return USERMOD_ID_BLE_2_JSON;
  }

  bool getBleOnFlag()
  {
    return m_bleOnFlag;
  }

  void setBleOnFlag(bool bleOnFlag)
  {
    BLE_DEBUG_PRINTF(F("setting on flag: %d\n"), bleOnFlag);

    m_bleOnFlag = bleOnFlag;
    m_configDirty = true;
  }

  uint32_t getBlePairingPin()
  {
    BLE_DEBUG_PRINTF(F("returning PIN mod: %u\n"), m_blePairingPin);

    return m_blePairingPin;
  }

  bool getBleUnPairDevices()
  {
    return m_bleUnPairDevices;
  }

  void setBleUnPairDevice(bool bleUnPairDevices)
  {
    m_bleUnPairDevices = bleUnPairDevices;
    m_configDirty = true;
  }
};