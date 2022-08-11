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

// class name. Use something descriptive and leave the ": public Usermod" part :)
class PingPongClockUsermod : public Usermod
{
private:
  // Private class members. You can declare variables and functions only accessible to your usermod here
  unsigned long lastTime = 0;
  bool colonOn = true;

  // ---- Variables modified by settings below -----
  // set your config variables to their boot default value (this can also be done in readFromConfig() or a constructor if you prefer)
  bool pingPongClockEnabled = true;
  int colorR = 0xFF;
  int colorG = 0xFF;
  int colorB = 0xFF;

  // ---- Variables for correct LED numbering below, edit only if your clock is built different ----

  int baseH = 43;
  int baseHH = 7;
  int baseM = 133;
  int baseMM = 97;
  int colon1 = 79;
  int colon2 = 80;

  // Matrix for the illumination of the numbers
  // Note: These only define the increments of the base adress. e.g. to define the second Minute you have to add the baseMM to every led position
  const int numbers[10][10] = 
    {
      {  0,  1,  4,  6, 13, 15, 18, 19, -1, -1 }, // 0: null
      { 13, 14, 15, 18, 19, -1, -1, -1, -1, -1 }, // 1: eins
      {  0,  4,  5,  6, 13, 14, 15, 19, -1, -1 }, // 2: zwei
      {  4,  5,  6, 13, 14, 15, 18, 19, -1, -1 }, // 3: drei
      {  1,  4,  5, 14, 15, 18, 19, -1, -1, -1 }, // 4: vier
      {  1,  4,  5,  6, 13, 14, 15, 18, -1, -1 }, // 5: fünf
      {  0,  5,  6, 10, 13, 14, 15, 18, -1, -1 }, // 6: sechs
      {  4,  6,  9, 13, 14, 19, -1, -1, -1, -1 }, // 7: sieben
      {  0,  1,  4,  5,  6, 13, 14, 15, 18, 19 }, // 8: acht
      {  1,  4,  5,  6,  9, 13, 14, 19, -1, -1 }  // 9: neun
    };

public:
  // Functions called by WLED

  /*
   * setup() is called once at boot. WiFi is not yet connected at this point.
   * You can use it to initialize variables, sensors or similar.
   */
  void setup()
  { }

  /*
   * connected() is called every time the WiFi is (re)connected
   * Use it to initialize network interfaces
   */
  void connected()
  { }

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
  void loop()
  {
    if (millis() - lastTime > 1000)
    {
      lastTime = millis();
      colonOn = !colonOn;
    }
  }

  /*
   * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
   * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
   * Below it is shown how this could be used for e.g. a light sensor
   */
  void addToJsonInfo(JsonObject& root)
  {
    JsonObject user = root["u"];
    if (user.isNull()) user = root.createNestedObject("u");

    JsonArray lightArr = user.createNestedArray("Uhrzeit-Anzeige"); //name
    lightArr.add(pingPongClockEnabled ? "aktiv" : "inaktiv"); //value
    lightArr.add(""); //unit
  }

  /*
   * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
   * Values in the state object may be modified by connected clients
   */
  void addToJsonState(JsonObject &root)
  { }

  /*
   * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
   * Values in the state object may be modified by connected clients
   */
  void readFromJsonState(JsonObject &root)
  { }

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
  void addToConfig(JsonObject &root)
  {
    JsonObject top = root.createNestedObject("Ping Pong Clock");
    top["enabled"] = pingPongClockEnabled;
    top["colorR"]   = colorR;
    top["colorG"]   = colorG;
    top["colorB"]   = colorB;
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
  bool readFromConfig(JsonObject &root)
  {
    JsonObject top = root["Ping Pong Clock"];

      bool configComplete = !top.isNull();

      configComplete &= getJsonValue(top["enabled"], pingPongClockEnabled);
      configComplete &= getJsonValue(top["colorR"], colorR);
      configComplete &= getJsonValue(top["colorG"], colorG);
      configComplete &= getJsonValue(top["colorB"], colorB);

      return configComplete;
  }

  void drawNumber(int base, int number)
  {
    for(int i = 0; i < 10; i++)
    {
      if(numbers[number][i] > -1)
        strip.setPixelColor(numbers[number][i] + base, RGBW32(colorR, colorG, colorB, 0));
    }
  }

  /*
   * handleOverlayDraw() is called just before every show() (LED strip update frame) after effects have set the colors.
   * Use this to blank out some LEDs or set them to a different color regardless of the set effect mode.
   * Commonly used for custom clocks (Cronixie, 7 segment)
   */
  void handleOverlayDraw()
  {
    if(pingPongClockEnabled){
      if(colonOn)
      {
        strip.setPixelColor(colon1, RGBW32(colorR, colorG, colorB, 0));
        strip.setPixelColor(colon2, RGBW32(colorR, colorG, colorB, 0));
      }
      drawNumber(baseHH, (hour(localTime) / 10) % 10);
      drawNumber(baseH, hour(localTime) % 10); 
      drawNumber(baseM, (minute(localTime) / 10) % 10);
      drawNumber(baseMM, minute(localTime) % 10);
    }
  }

  /*
   * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
   * This could be used in the future for the system to determine whether your usermod is installed.
   */
  uint16_t getId()
  {
    return USERMOD_ID_PING_PONG_CLOCK;
  }

};