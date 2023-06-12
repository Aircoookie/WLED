#pragma once

#include "wled.h"

/*
* PhillipsWho Usermod class that reads information 
* from WLED and lights LED's as a result. 
* LED 1: wifi connection status
* LED 2: WLED activity status
* Contact: chill@chillaspect.com
*/
class PhillipsWho : public Usermod {
  private:
    static const char _name[];
    static const char _enabled[];

  public:
    //How frequently to check for wifi
    const unsigned long poll_frequency_wifi = 5000; // every 5 second

    //Time when wifi was last checked
    unsigned long last_checked_wifi = 0;

    //How long the Activity LED remains on when activity detected
    const unsigned long activity_led_on_period = 1000;
    
    //Default Pin Values
    int led_wifi_gpio = 13;
    int led_activity_gpio = 14;

    //Enabled Bool
    bool enabled = false;

    //The current time since ESP32 Started (in ms)
    unsigned long current_ms = 0L;

    //Time (in ms) when onStateChange was called
    unsigned long state_change_fired_ms = 0;

    /*
    * Check if wifi is connected
    * Light the LED appropriately
    */
    void check_wifi() {
      if(WLED_CONNECTED){
          digitalWrite(led_wifi_gpio, HIGH);
      }else{
          digitalWrite(led_wifi_gpio, LOW);
      }
    }

    /**
     * Enable/Disable the usermod
     */
    inline void enable(bool enable) { enabled = enable; }

    /**
     * Get usermod enabled/disabled state
     */
    inline bool isEnabled() { return enabled; }

    /*
    * Callback function called when any change happens in WLED
    * This includes light and configuration changes
    */
    void onStateChange(uint8_t mode){
      digitalWrite(led_activity_gpio, HIGH);
      state_change_fired_ms = millis();
    }

    /*
    * Add settings to the configuration file
    */
    void addToConfig(JsonObject& root)
    {
      JsonObject top = root.createNestedObject(FPSTR(_name));
      top["activity_led_pin"] = led_activity_gpio;
      top["wifi_status_led_pin"] = led_wifi_gpio;
      top["enabled"] = enabled;
    }

    /*
    * Read Settings from Configuration File
    */
    bool readFromConfig(JsonObject& root)
    {
      JsonObject top = root[FPSTR(_name)];
      bool configComplete = !top.isNull();
      configComplete &= getJsonValue(top["activity_led_pin"], led_activity_gpio, 14);
      configComplete &= getJsonValue(top["wifi_status_led_pin"], led_wifi_gpio, 13);
      configComplete &= getJsonValue(top["enabled"], enabled, false);

      // Turn off LED's if disabling
      if(!enabled){
          digitalWrite(led_wifi_gpio, LOW);
          digitalWrite(led_activity_gpio, LOW);
      }

      return configComplete;
    }

    /*
    * Initial Setup - Called once on boot
    */
    void setup() {
      Serial.println("PhillipsWho - Setup");
      pinMode(led_wifi_gpio, OUTPUT);
      pinMode(led_activity_gpio, OUTPUT);
      digitalWrite(led_wifi_gpio, LOW);
      digitalWrite(led_activity_gpio, LOW);
    }

    /*
    * Main Loop - Called continuously while ESP32 is running
    */
    void loop() {
      //If disabled, do nothing
      if (!enabled) return;

      //Get the current time in MS
      current_ms = millis();

      //Do we need to check for wifi yet?
      if(current_ms - last_checked_wifi >= poll_frequency_wifi){
          last_checked_wifi = current_ms;
          check_wifi();
      }

      if(current_ms - state_change_fired_ms >= activity_led_on_period){
          digitalWrite(led_activity_gpio, LOW);
      }

      //Check for millis overflow - reset back to 0
      if(current_ms < last_checked_wifi){
          last_checked_wifi = 0;
          state_change_fired_ms = 0;
      }

      //Dont waste CPU
      delay(50);
    }
};

// strings to reduce flash memory usage (used more than twice)
const char PhillipsWho::_name[]     PROGMEM = "PhillipsWho";
const char PhillipsWho::_enabled[]  PROGMEM = "enabled";
