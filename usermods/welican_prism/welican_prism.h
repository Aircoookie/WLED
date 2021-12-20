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
class WelicanPrism : public Usermod {
 private:
    //Private class members. You can declare variables and functions only accessible to your usermod here
    //PART OF EXAMPLE BELOW
	unsigned long lastTime = 0;

    // set your config variables to their boot default value (this can also be done in readFromConfig() or a constructor if you prefer)
    bool testBool = false;
    unsigned long testULong = 42424242;
    float testFloat = 42.42;
    String testString = "Forty-Two";

    // These config variables have defaults set inside readFromConfig()
    int testInt;
    long testLong;
    int8_t testPins[2];

	//WELICAN START
	int fadeAmount = 8;  // how many points to fade the Neopixel with each step
	int tempBri = 0;
	byte presetCurrent = 0;
	byte presetMax = 10;
	unsigned long currentTime;
	unsigned long loopTime;

	const int pinA1 = D1;  // DT from encoder
	const int pinB1 = D2;  // CLK from encoder
	const int pinA2 = D5;  // DT from encoder
	const int pinB2 = D6;  // CLK from encoder
	const int pinC2 = D7;  // BTN from encoder


	/*
	const int pinA1 = D5;  // DT from encoder
	const int pinB1 = D6;  // CLK from encoder
	const int pinA2 = D1;  // DT from encoder
	const int pinB2 = D2;  // CLK from encoder
	const int pinC2 = D7;  // BTN from encoder
	*/

	unsigned char Enc_A1;
	unsigned char Enc_B1;
	unsigned char Enc_A1_prev = 0;
	unsigned char Enc_A2;
	unsigned char Enc_B2;
	unsigned char Enc_A2_prev = 0;
	
  public:
    //Functions called by WLED

    /*
     * setup() is called once at boot. WiFi is not yet connected at this point.
     * You can use it to initialize variables, sensors or similar.
     */
    void setup() {
      //Serial.println("Hello from my usermod!");
	  pinMode(pinA1, INPUT_PULLUP);
	  pinMode(pinB1, INPUT_PULLUP);
	  pinMode(pinA2, INPUT_PULLUP);
	  pinMode(pinB2, INPUT_PULLUP);
	  pinMode(pinC2, INPUT_PULLUP);
	  currentTime = millis();
	  loopTime = currentTime;
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
     currentTime = millis();  // get the current elapsed time
	  //if((currentTime - loopTime) >= 2)  // 2ms since last check of encoder = 500Hz 
	  if((currentTime - loopTime) >= 1)  // 1ms since last check of encoder = 1 kHz 
	  //if (1 == 1)
	  {
		int Enc_A1 = digitalRead(pinA1);  // Read encoder pins
		int Enc_B1 = digitalRead(pinB1);   
		if((! Enc_A1) && (Enc_A1_prev)) { // A has gone from high to low
		  if(Enc_B1 == HIGH) { // B is high so clockwise
			//if(bri + fadeAmount <= 255) bri += fadeAmount;   // increase the brightness, dont go over 255
			
			if (bri < 40)
			{
			  tempBri = bri + fadeAmount / 4;
			}
			else if (bri < 80)
			{
			  tempBri = bri + fadeAmount / 2;
			}
			else if (bri < 160)
			{
			  tempBri = bri + fadeAmount;
			}
			else
			{
			  tempBri = bri + fadeAmount * 2;
			}

			if (tempBri > 255)
			{
			  tempBri = 255;
			}
			bri = tempBri;
			Serial.print("Up   - ");
			Serial.println(bri);
			colorUpdated(6);
		  } else if (Enc_B1 == LOW) { // B is low so counter-clockwise
			//if(bri - fadeAmount >= 0) bri -= fadeAmount;   // decrease the brightness, dont go below 0   
			tempBri = bri - fadeAmount;

			if (tempBri < 0)
			{
			  tempBri = 0;
			}
			bri = tempBri;
			Serial.print("Down - ");
			Serial.println(bri);  
			colorUpdated(6);     
		  }   
		}   
		Enc_A1_prev = Enc_A1;     // Store value of A for next time    
		


		/* Encoder for MODES
		int Enc_A2 = digitalRead(pinA2);  // Read encoder pins
		int Enc_B2 = digitalRead(pinB2);   
		if((! Enc_A2) && (Enc_A2_prev)) { // A has gone from high to low
		  if(Enc_B2 == HIGH) { // B is high so clockwise
			effectCurrent += 1;
			if (effectCurrent >= MODE_COUNT) effectCurrent = 0;
			colorUpdated(8);
			Serial.print("2 Up - ");
			Serial.println(effectCurrent);  
		  } else if (Enc_B2 == LOW) { // B is low so counter-clockwise
			
			if (effectCurrent <= 0)
			{
			  effectCurrent = (MODE_COUNT-1);
			}
			else
			{
			  effectCurrent -= 1;
			}
			colorUpdated(8);
			Serial.print("2 Down - ");
			Serial.println(effectCurrent);      
		  }   
		}   
		Enc_A2_prev = Enc_A2;     // Store value of A for next time   
		*/
		
		//Encoder for PRESETS
		int Enc_A2 = digitalRead(pinA2);  // Read encoder pins
		int Enc_B2 = digitalRead(pinB2);   
		if((! Enc_A2) && (Enc_A2_prev)) { // A has gone from high to low
		  if(Enc_B2 == HIGH) { // B is high so clockwise
			presetCurrent += 1;
			
			if (presetCurrent >= presetMax) presetCurrent = 0;
			
			applyPreset(presetCurrent, 0);
			
			//colorUpdated(8);
		  } else if (Enc_B2 == LOW) { // B is low so counter-clockwise
			
			if (presetCurrent <= 2)
			{
			  presetCurrent = (presetMax);
			}
			else
			{
			  presetCurrent -= 1;
			}
			applyPreset(presetCurrent, 0);
			//colorUpdated(8);    
		  }   
		}   
		Enc_A2_prev = Enc_A2;     // Store value of A for next time   


		if (digitalRead(pinC2) == LOW)
		{
		  //effectCurrent = 0;
		  applyPreset(20, 0);
		  Serial.println("CC");
		  colorUpdated(8);
		}

		loopTime = currentTime;  // Updates loopTime
		
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
    bool readFromConfig(JsonObject& root)
    {
      // default settings values could be set here (or below using the 3-argument getJsonValue()) instead of in the class definition or constructor
      // setting them inside readFromConfig() is slightly more robust, handling the rare but plausible use case of single value being missing after boot (e.g. if the cfg.json was manually edited and a value was removed)

      JsonObject top = root["exampleUsermod"];

      bool configComplete = !top.isNull();

      configComplete &= getJsonValue(top["great"], userVar0);
      configComplete &= getJsonValue(top["testBool"], testBool);
      configComplete &= getJsonValue(top["testULong"], testULong);
      configComplete &= getJsonValue(top["testFloat"], testFloat);
      configComplete &= getJsonValue(top["testString"], testString);

      // A 3-argument getJsonValue() assigns the 3rd argument as a default value if the Json value is missing
      configComplete &= getJsonValue(top["testInt"], testInt, 42);  
      configComplete &= getJsonValue(top["testLong"], testLong, -42424242);
      configComplete &= getJsonValue(top["pin"][0], testPins[0], -1);
      configComplete &= getJsonValue(top["pin"][1], testPins[1], -1);

      return configComplete;
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