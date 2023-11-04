/*
 * Usermod: HttpPullLightControl
 * Versie: 0.0.2
 * Repository: https://github.com/roelbroersma/WLED-usermodv2_HttpPullLightControl
 * Author: Roel Broersma
 * Website: https://www.roelbroersma.nl
 * Github author: github.com/roelbroersma
 * Description: This usermod for WLED will request a given URL to know which effects
 *              or individual lights it should turn on/off. So you can remote control a WLED
 *              installation without having access to it (if no port forward, vpn or public IP is available).
 * Use Case: Create a WLED 'Ring of Thought' christmass card. Sent a LED ring with 60 LEDs to 60 friends.
 *           When they turn it on and put it at their WiFi, it will contact your server. Now you can reply with a given
 *           number of lights that should turn on. Each light is a friend who did contact your server in the past 5 minutes.
 *           So on each of your friends LED rings, the number of lights will be the number of friends who have it turned on.
 * Features: It sends a unique ID (has of MAC and salt) to the URL, so you can define each client without a need to map their IP address.
 * Tested: Tested on WLED v0.14 with ESP32-S3 (M5Stack Atom S3 Lite), but should also workd for other ESPs and ESP8266.
 */

#pragma once

#include "wled.h"

#define HTTP_PULL_LIGHT_CONTROL_VERSION "0.0.2"

class HttpPullLightControl : public Usermod {
private:
  static const char _name[];
  static const char _enabled[];
  static const char _salt[];
  static const char _url[];

  bool enabled = true;

  #ifdef HTTP_PULL_LIGHT_CONTROL_INTERVAL
    uint16_t checkInterval = HTTP_PULL_LIGHT_CONTROL_INTERVAL;
  #else
    uint16_t checkInterval = 60;  // Default interval of 1 minute
  #endif

  #ifdef HTTP_PULL_LIGHT_CONTROL_URL
    String url = HTTP_PULL_LIGHT_CONTROL_URL;
  #else
    String url = "http://example.org/example.php";  // Default-URL (http only!), can also be url with IP address in it. HttpS urls are not supported (yet) because of AsyncTCP library
  #endif

  #ifdef HTTP_PULL_LIGHT_CONTROL_SALT
    String salt = HTTP_PULL_LIGHT_CONTROL_SALT;
  #else
    String salt = "1just_a_very-secret_salt2";  // Salt for generating a unique ID when requesting the URL (in this way you can give different answers based on the WLED device who does the request)
  #endif

  // THERE IS ALSO A #ifdef HTTP_PULL_LIGHT_CONTROL_HIDE_URL and a HTTP_PULL_LIGHT_CONTROL_HIDE_SALT IF YOU DO NOT WANT TO SHOW THE OPTIONS IN THE USERMOD SETTINGS

  unsigned long lastCheck = 0;  // Timestamp of last check
  int16_t ackTimeout = 10000;  // ACK timeout in milliseconds when doing the URL request
  uint16_t rxTimeout = 10000;   // RX timeout in milliseconds when doing the URL request
  String host;  // Host extracted from the URL
  String path;  // Path extracted from the URL
  String uniqueId;  // Cached unique ID
  const unsigned long FNV_offset_basis = 2166136261;
  const unsigned long FNV_prime = 16777619;
  AsyncClient *client = nullptr; // Used very often, beware of closing and freeing

  uint16_t max_lights = 0;  // Set this number during setup
  // Define Light structure
  struct myLight {
    bool state;     // State (on or off)
    uint32_t color; // Color, saved as a 32-bit number (e.g. 0x00FF00 for green, RGB)
  };

  // Define segment structure with its Light array
  uint8_t max_segments = 0; // Set this number during setup
  struct mySegment {
    bool hasLights; // Has light or effect
    myLight* lights;
  };
  mySegment* segments_array = nullptr;  // Set to zero pointer

  // Define ColorSlot structure. Colorslots are the three slots you see in the webinterface, sort of memory
  struct myColorSlot {
    uint32_t color;
    bool isValid;
  };
  // Define the SegmentSetting structure with its ColorSlots array
  struct mySegmentSetting {
    uint8_t segment_id;
    uint8_t effect_id;
    uint8_t speed;
    uint8_t intensity;
    uint8_t palette_id;
    myColorSlot color_slots[3];
  };
  mySegmentSetting segments_settings;

  void initializeSegments(int numSegments, int numLightsPerSegment);
  String generateUniqueId();
  uint32_t colorFromRgb(uint8_t r, uint8_t g, uint8_t b);  // Declare colorFromRgb function here
  void parseUrl();
  void updateSalt(String newSalt);  // Update the salt value and recalculate the unique ID
  uint16_t getAbsoluteIndex(uint8_t segmentId, uint16_t relativeIndex);
  void checkUrl();  // Check the specified URL for light control instructions
  void handleResponse(String& response);
  void onClientConnect(AsyncClient *c);
  void setEffect(mySegmentSetting& segment_settings); // Set the effect with its speed and intensity for the segment

public:
  void setup();
  void loop();
  bool readFromConfig(JsonObject& root);
  void addToConfig(JsonObject& root);
  uint16_t getId() { return USERMOD_ID_HTTP_PULL_LIGHT_CONTROL; }
  void handleOverlayDraw() override;  // This is the overlay function which is called by WLED everytime
  inline void enable(bool enable) { enabled = enable; }  // Enable or Disable the usermod
  inline bool isEnabled() { return enabled; }  // Get usermod enabled or disabled state
  virtual ~HttpPullLightControl() { 
   // Removed the cached client if needed
    if (client) {
      client->onDisconnect(nullptr);
      client->onError(nullptr);
      client->onTimeout(nullptr);
      client->onData(nullptr);
      // Now it is safe to delete the client.
      delete client; // This is safe even if client is nullptr.

      // Clean up the segments array.
      if (segments_array != nullptr) {
        for (int i = 0; i < max_segments; ++i) {
          delete[] segments_array[i].lights;
        }
        delete[] segments_array;
      }
    }
  }
};