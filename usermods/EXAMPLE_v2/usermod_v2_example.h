#pragma once

#include "wled.h"

/*
 * Usermods allow you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * 
 * This is an example for a v2 usermod.
 * v2 usermods are class inheritance based and can (but don't have to) implement more functions, each of them is shown in this example.
 * Multiple v2 usermods can be added to one compilation easily.
 * 
 * Creating a usermod:
 * This file serves as an example. If you want to create a usermod, it is recommended to use usermod_v2_empty.h from the usermods folder as a template.
 * Please remember to rename the class and file to a descriptive name.
 * You may also use multiple .h and .cpp files.
 * 
 * Using a usermod:
 * 1. Copy the usermod into the sketch folder (same folder as wled00.ino)
 * 2. Register the usermod by adding #include "usermod_filename.h" in the top and registerUsermod(new MyUsermodClass()) in the bottom of usermods_list.cpp
 */

//class name. Use something descriptive and leave the ": public Usermod" part :)
class MyExampleUsermod : public Usermod {
  private:
    //Private class members. You can declare variables and functions only accessible to your usermod here
    unsigned long lastTime = 0;

    // set your config variables to their boot default value (this can also be done in readFromConfig() or a constructor if you prefer)
    bool testBool = 42;
    int testInt = false;
    long testLong = -42424242;
    unsigned long testULong = 42424242;
    float testFloat = 42.42;
    String testString = "Forty-Two";
    int8_t testPins[2] = {-1, -1};

  public:
    //Functions called by WLED

    /*
     * setup() is called once at boot. WiFi is not yet connected at this point.
     * You can use it to initialize variables, sensors or similar.
     */
    void setup() {
      //Serial.println("Hello from my usermod!");
    }


    /*
     * connected() is called every time the WiFi is (re)connected
     * Use it to initialize network interfaces
     */
    void connected() {
      //Serial.println("Connected to WiFi!");
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
    void loop() {
      if (millis() - lastTime > 1000) {
        //Serial.println("I'm alive!");
        lastTime = millis();
      }
    }


    /*
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
     * Below it is shown how this could be used for e.g. a light sensor
     */
    /*
    void addToJsonInfo(JsonObject& root)
    {
      int reading = 20;
      //this code adds "u":{"Light":[20," lux"]} to the info object
      JsonObject user = root["u"];
      if (user.isNull()) user = root.createNestedObject("u");

      JsonArray lightArr = user.createNestedArray("Light"); //name
      lightArr.add(reading); //value
      lightArr.add(" lux"); //unit
    }
    */


    /*
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void addToJsonState(JsonObject& root)
    {
      //root["user0"] = userVar0;
    }


    /*
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void readFromJsonState(JsonObject& root)
    {
      userVar0 = root["user0"] | userVar0; //if "user0" key exists in JSON, update, else keep old value
      //if (root["bri"] == 255) Serial.println(F("Don't burn down your garage!"));
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
    void addToConfig(JsonObject& root)
    {
      JsonObject top = root.createNestedObject("exampleUsermod");
      top["great"] = userVar0; //save these vars persistently whenever settings are saved
      top["testBool"] = testBool;
      top["testInt"] = testInt;
      top["testLong"] = testLong;
      top["testULong"] = testULong;
      top["testFloat"] = testFloat;
      top["testString"] = testString;
      JsonArray pinArray = top.createNestedArray("pin");
      pinArray.add(testPins[0]);
      pinArray.add(testPins[1]); 
    }


    /*
     * readFromConfig() can be used to read back the custom settings you added with addToConfig().
     * This is called by WLED when settings are loaded (currently this only happens once immediately after boot)
     * 
     * readFromConfig() is called BEFORE setup(). This means you can use your persistent values in setup() (e.g. pin assignments, buffer sizes),
     * but also that if you want to write persistent values to a dynamic buffer, you'd need to allocate it here instead of in setup.
     * If you don't know what that is, don't fret. It most likely doesn't affect your use case :)
     *
     * See usermode_rotary_brightness_color.h for a more robust example that handles missing config/upgrades better
     * 
     * Return true in case the config values returned from Usermod Settings were complete, or false if you'd like WLED to save your defaults to disk (so any missing values are editable in Usermod Settings)
     * 
     * This function is guaranteed to be called on boot, but could also be called every time settings are updated
     */
    bool readFromConfig(JsonObject& root)
    {
      JsonObject top = root["exampleUsermod"];

      // Option 1: AC's proposed default
#if 0
      // if the "exampleUsermod" object is missing, return false so WLED knows to add it (by calling addToConfig())
      if(top.isNull())
        return false;

      userVar0 = top["great"] | userVar0; // the ArduinoJson | operator only sets userVar0 if the value for "great" is present, otherwise it's left at the default (userVar0 = userVar0)
      testBool = top["testBool"] | testBool;
      testInt = top["testInt"] | testInt;
      testLong = top["testLong"] | testLong;
      testULong = top["testULong"] | testULong;
      testFloat = top["testFloat"] | testFloat;
      testString = top["testString"] | testString;
      testPins[0] = top["pin"][0] | testPins[0];
      testPins[1] = top["pin"][1] | testPins[1];

      // this optional check looks for any single missing key-value pair, so it can be added back (by calling addToConfig())
      if(top["great"].isNull() || top["testBool"].isNull() || top["testInt"].isNull() || 
        top["testLong"].isNull() || top["testULong"].isNull() || top["testFloat"].isNull() || 
        top["testString"].isNull() || top["pin"][0].isNull() || top["pin"][1].isNull())
        return false;

      return true;
#else
      // Option 2: Louis's proposed default (seems cleaner and robust)
#if 1
      bool configComplete = !top.isNull();

      configComplete &= getValueFromJsonKey(top["great"], userVar0);
      configComplete &= getValueFromJsonKey(top["testBool"], testBool);
      configComplete &= getValueFromJsonKey(top["testInt"], testInt);
      configComplete &= getValueFromJsonKey(top["testLong"], testLong);
      configComplete &= getValueFromJsonKey(top["testULong"], testULong);
      configComplete &= getValueFromJsonKey(top["testFloat"], testFloat);
      configComplete &= getValueFromJsonKey(top["testString"], testString);
      configComplete &= getValueFromJsonKey(top["pin"][0], testPins[0]);
      configComplete &= getValueFromJsonKey(top["pin"][1], testPins[1]);

      return configComplete;

      //Option 3: Hybrid, using wrapper function but not checking return value at same time
#else
      if(top.isNull())
        return false;

      getValueFromJsonKey(top["great"], userVar0);
      getValueFromJsonKey(top["testBool"], testBool);
      getValueFromJsonKey(top["testInt"], testInt);
      getValueFromJsonKey(top["testLong"], testLong);
      getValueFromJsonKey(top["testULong"], testULong);
      getValueFromJsonKey(top["testFloat"], testFloat);
      getValueFromJsonKey(top["testString"], testString);
      getValueFromJsonKey(top["pin"][0], testPins[0]);
      getValueFromJsonKey(top["pin"][1], testPins[1]);

      // this optional check looks for any single missing key-value pair, so it can be added back (by calling addToConfig())
      if(top["great"].isNull() || top["testBool"].isNull() || top["testInt"].isNull() || 
        top["testLong"].isNull() || top["testULong"].isNull() || top["testFloat"].isNull() || 
        top["testString"].isNull() || top["pin"][0].isNull() || top["pin"][1].isNull())
        return false;

      return true;  
#endif
#endif
    }

   
    /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
    uint16_t getId()
    {
      return USERMOD_ID_EXAMPLE;
    }

   //More methods can be added in the future, this example will then be extended.
   //Your usermod will remain compatible as it does not need to implement all methods from the Usermod base class!
};