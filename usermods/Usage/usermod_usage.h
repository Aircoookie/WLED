#pragma once

#include "wled.h"
#ifdef ESP8266
#include <Hash.h>
#else
#include "esp32hash.h"
#endif

/*
 * Send usage info to WLED to help with support and development
 */

struct __attribute__((packed)) UsagePacket {
  byte header;
  uint8_t length = sizeof(UsagePacket);
  char deviceId[40];
  char version[20]; // TODO: size
  char chip[15];    // TODO: size
  uint16_t uptime;
  uint16_t totalLEDs;
  bool isMatrix;
};

class UsageUsermod : public Usermod {

private:
    bool enabled = true; // TODO: set to false to disable usermod
    bool isConnected = false;
    bool initDone = false;
    unsigned long lastTime = 0;

    // set your config variables to their boot default value (this can also be
    // done in readFromConfig() or a constructor if you prefer)
    String usageHost = "192.168.178.50";
    int port = 7001;
    WiFiUDP wifiUDP;

    // string that are used multiple time (this will save some flash memory)
    static const char _name[];
    static const char _enabled[];

    UsagePacket usagePacket;

public:

  /**
   * Enable/Disable the usermod
   */
    void enable(bool enable) { enabled = enable; }

  /**
   * Get usermod enabled/disabled state
   */
    bool isEnabled() { return enabled; }

  /*
   * setup() is called once at boot. WiFi is not yet connected at this point.
   * readFromConfig() is called prior to setup()
   * You can use it to initialize variables, sensors or similar.
   */
    void setup() override {
        initDone = true;
        usagePacket.header = 0x01;
        strncpy(usagePacket.deviceId, sha1("WLEDUSAGE" + WiFi.macAddress()).c_str(), sizeof(usagePacket.deviceId));
#ifdef ESP8266
        strncpy(usagePacket.chip, "ESP8266", sizeof(usagePacket.chip));
#else
        strncpy(usagePacket.chip, ESP.getChipModel(), sizeof(usagePacket.chip));
#endif

        strncpy(usagePacket.version, versionString, sizeof(usagePacket.version));
    }

  /*
   * connected() is called every time the WiFi is (re)connected
   * Use it to initialize network interfaces
   */
  void connected() override {
      isConnected = true;
#ifdef ESP8266
      wifiUDP.begin(port);
#else
      wifiUDP.begin(WiFi.localIP(), port);
#endif
    }


    /*
     * loop() is called continuously. Here you can check for events, read sensors, etc.
     * 
     * Tips:
     * 1. You can use "if (WLED_CONNECTED)" to check for a successful network connection.
     *    Additionally, "if (WLED_MQTT_CONNECTED)" is available to check for a connection to an MQTT broker.
     * 
     * 2. Try to avoid using the delay() function. NEVER use delays longer than 10 milliseconds.
     *    Instead, use a timer check as shown here.
     */
    void loop() override {
      // if usermod is disabled or called during strip updating just exit
      // NOTE: on very long strips strip.isUpdating() may always return true so update accordingly
        if (!enabled || strip.isUpdating()) return;
        if(!isConnected) return;

        if (millis() - lastTime > 1000) {
            lastTime = millis();
            usagePacket.uptime = (millis() / 1000l);
            usagePacket.totalLEDs = strip.getLength();
            usagePacket.isMatrix = strip.isMatrix;

            if(wifiUDP.beginPacket(IPAddress(192, 168, 178, 50), port)) {
                wifiUDP.write(reinterpret_cast<uint8_t *>(&usagePacket), sizeof(usagePacket));
                wifiUDP.endPacket();
                Serial.printf("Send usage packet to %s:%u\n", wifiUDP.remoteIP().toString().c_str(), wifiUDP.remotePort());
            }
        }
  }

    /*
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
     * Below it is shown how this could be used for e.g. a light sensor
     */
    void addToJsonInfo(JsonObject& root) override
    {
      // if "u" object does not exist yet wee need to create it
      JsonObject user = root["u"];
      if (user.isNull()) user = root.createNestedObject("u");

    // this code adds "u":{"ExampleUsermod":[20," lux"]} to the info object
    // int reading = 20;
    // JsonArray lightArr = user.createNestedArray(FPSTR(_name))); //name
    // lightArr.add(reading); //value
    // lightArr.add(F(" lux")); //unit

    // if you are implementing a sensor usermod, you may publish sensor data
    // JsonObject sensor = root[F("sensor")];
    // if (sensor.isNull()) sensor = root.createNestedObject(F("sensor"));
    // temp = sensor.createNestedArray(F("light"));
    // temp.add(reading);
    // temp.add(F("lux"));
  }

  
    /*
     * addToConfig() can be used to add custom persistent settings to the cfg.json file in the "um" (usermod) object.
     * It will be called by WLED when settings are actually saved (for example, LED settings are saved)
     * If you want to force saving the current state, use serializeConfig() in your loop().
     * 
     * CAUTION: serializeConfig() will initiate a filesystem write operation.
     * It might cause the LEDs to stutter and will cause flash wear if called too often.
     * Use it sparingly and always in the loop, never in network callbacks!
     * 
     * addToConfig() will make your settings editable through the Usermod Settings page automatically.
     *
     * Usermod Settings Overview:
     * - Numeric values are treated as floats in the browser.
     *   - If the numeric value entered into the browser contains a decimal point, it will be parsed as a C float
     *     before being returned to the Usermod.  The float data type has only 6-7 decimal digits of precision, and
     *     doubles are not supported, numbers will be rounded to the nearest float value when being parsed.
     *     The range accepted by the input field is +/- 1.175494351e-38 to +/- 3.402823466e+38.
     *   - If the numeric value entered into the browser doesn't contain a decimal point, it will be parsed as a
     *     C int32_t (range: -2147483648 to 2147483647) before being returned to the usermod.
     *     Overflows or underflows are truncated to the max/min value for an int32_t, and again truncated to the type
     *     used in the Usermod when reading the value from ArduinoJson.
     * - Pin values can be treated differently from an integer value by using the key name "pin"
     *   - "pin" can contain a single or array of integer values
     *   - On the Usermod Settings page there is simple checking for pin conflicts and warnings for special pins
     *     - Red color indicates a conflict.  Yellow color indicates a pin with a warning (e.g. an input-only pin)
     *   - Tip: use int8_t to store the pin value in the Usermod, so a -1 value (pin not set) can be used
     *
     * See usermod_v2_auto_save.h for an example that saves Flash space by reusing ArduinoJson key name strings
     * 
     * If you need a dedicated settings page with custom layout for your Usermod, that takes a lot more work.  
     * You will have to add the setting to the HTML, xml.cpp and set.cpp manually.
     * See the WLED Soundreactive fork (code and wiki) for reference.  https://github.com/atuline/WLED
     * 
     * I highly recommend checking out the basics of ArduinoJson serialization and deserialization in order to use custom settings!
     */
    void addToConfig(JsonObject& root) override
    {
      JsonObject top = root.createNestedObject(FPSTR(_name));
      top[FPSTR(_enabled)] = enabled;
      // //save these vars persistently whenever settings are saved
      // top["great"] = userVar0;
      // top["testBool"] = testBool;
      // top["testInt"] = testInt;
      // top["testLong"] = testLong;
      // top["testULong"] = testULong;
      // top["testFloat"] = testFloat;
      // top["testString"] = testString;
      // JsonArray pinArray = top.createNestedArray("pin");
      // pinArray.add(testPins[0]);
      // pinArray.add(testPins[1]); 
    }


    /*
     * readFromConfig() can be used to read back the custom settings you added with addToConfig().
     * This is called by WLED when settings are loaded (currently this only happens immediately after boot, or after saving on the Usermod Settings page)
     * 
     * readFromConfig() is called BEFORE setup(). This means you can use your persistent values in setup() (e.g. pin assignments, buffer sizes),
     * but also that if you want to write persistent values to a dynamic buffer, you'd need to allocate it here instead of in setup.
     * If you don't know what that is, don't fret. It most likely doesn't affect your use case :)
     * 
     * Return true in case the config values returned from Usermod Settings were complete, or false if you'd like WLED to save your defaults to disk (so any missing values are editable in Usermod Settings)
     * 
     * getJsonValue() returns false if the value is missing, or copies the value into the variable provided and returns true if the value is present
     * The configComplete variable is true only if the "exampleUsermod" object and all values are present.  If any values are missing, WLED will know to call addToConfig() to save them
     * 
     * This function is guaranteed to be called on boot, but could also be called every time settings are updated
     */
    bool readFromConfig(JsonObject& root) override
    {
      // default settings values could be set here (or below using the 3-argument getJsonValue()) instead of in the class definition or constructor
      // setting them inside readFromConfig() is slightly more robust, handling the rare but plausible use case of single value being missing after boot (e.g. if the cfg.json was manually edited and a value was removed)

    JsonObject top = root[FPSTR(_name)];

    bool configComplete = !top.isNull();

    // configComplete &= getJsonValue(top["great"], userVar0);
    // configComplete &= getJsonValue(top["testBool"], testBool);
    // configComplete &= getJsonValue(top["testULong"], testULong);
    // configComplete &= getJsonValue(top["testFloat"], testFloat);
    // configComplete &= getJsonValue(top["testString"], testString);

      // // A 3-argument getJsonValue() assigns the 3rd argument as a default value if the Json value is missing
      // configComplete &= getJsonValue(top["testInt"], testInt, 42);  
      // configComplete &= getJsonValue(top["testLong"], testLong, -42424242);

      // // "pin" fields have special handling in settings page (or some_pin as well)
      // configComplete &= getJsonValue(top["pin"][0], testPins[0], -1);
      // configComplete &= getJsonValue(top["pin"][1], testPins[1], -1);

    return configComplete;
  }


    /*
     * appendConfigData() is called when user enters usermod settings page
     * it may add additional metadata for certain entry fields (adding drop down is possible)
     * be careful not to add too much as oappend() buffer is limited to 3k
     */
    void appendConfigData() override
    {
      // oappend(F("addInfo('")); oappend(String(FPSTR(_name)).c_str()); oappend(F(":great")); oappend(F("',1,'<i>(this is a great config value)</i>');"));
      // oappend(F("addInfo('")); oappend(String(FPSTR(_name)).c_str()); oappend(F(":testString")); oappend(F("',1,'enter any string you want');"));
      // oappend(F("dd=addDropdown('")); oappend(String(FPSTR(_name)).c_str()); oappend(F("','testInt');"));
      // oappend(F("addOption(dd,'Nothing',0);"));
      // oappend(F("addOption(dd,'Everything',42);"));
    }


    /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
    uint16_t getId() override
    {
      return USERMOD_ID_USAGE;
    }

   //More methods can be added in the future, this example will then be extended.
   //Your usermod will remain compatible as it does not need to implement all methods from the Usermod base class!
};

// add more strings here to reduce flash memory usage
const char UsageUsermod::_name[] PROGMEM = "Usage";
const char UsageUsermod::_enabled[] PROGMEM = "enabled";
