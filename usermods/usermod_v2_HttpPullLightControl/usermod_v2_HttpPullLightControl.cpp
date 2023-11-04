#include "usermod_v2_HttpPullLightControl.h"

// add more strings here to reduce flash memory usage
const char HttpPullLightControl::_name[]    PROGMEM = "HttpPullLightControl";
const char HttpPullLightControl::_enabled[] PROGMEM = "Enable";

void HttpPullLightControl::setup() {
  //Serial.begin(115200);
  // Define maximum lights and maximum segment. TODO: We only need the maxlights_per_array so we define a bit too much here.
  max_lights = strip.getLengthTotal();
  max_segments = strip.getSegmentsNum();

  // Print version number
  Serial.print(F("HttpPullLightControl version: "));
  Serial.println(HTTP_PULL_LIGHT_CONTROL_VERSION);

  // Start a nice chase so we know its booting and searching for its first http pull.
  DEBUG_PRINTLN(F("Starting a nice chase so we now it is booting."));
  segments_settings.segment_id = 0;
  segments_settings.effect_id = 28;
  segments_settings.speed = 200;
  segments_settings.intensity = 255;
  segments_settings.palette_id = 128;
  segments_settings.color_slots[0].color = 5263440;
  segments_settings.color_slots[0].isValid = true;  // Aannemende dat deze slot geldig is
  segments_settings.color_slots[1].color = 0;
  segments_settings.color_slots[1].isValid = true;  // Aannemende dat deze slot geldig is
  segments_settings.color_slots[2].color = 4605510;
  segments_settings.color_slots[2].isValid = true;  // Aannemende dat deze slot geldig is
  setEffect(segments_settings); // Call to a function that sets the effect

  // Go on with generating a unique ID, getting the URL into parts and initializing the segments
  uniqueId = generateUniqueId();  // Cache the unique ID
  parseUrl();
  initializeSegments(max_segments, max_lights);
  DEBUG_PRINTLN(F("HttpPullLightControl successfully setup"));
}

// This is the main loop function, from here we check the URL and handle the response.
// Effects are set as a result from this, but not individual LEDs, they are saved in an array and that is
// used in the Overlay function which is called by WLED just before sending it on the bus.
void HttpPullLightControl::loop() {
  if (!enabled || offMode) return; // Do nothing when not enabled or powered off
  if (millis() - lastCheck >= checkInterval * 1000) {
    DEBUG_PRINTLN(F("Calling checkUrl function"));
    checkUrl();
    lastCheck = millis();
  }

}

// Set the Segments array (and the Lights array in it) to zero and re-initialize.
void HttpPullLightControl::initializeSegments(int numSegments, int numLightsPerSegment) {
  DEBUG_PRINTLN(F("(Re)initializing Segments array"));
  // Delete arrays
  if (segments_array) {
    for (int i = 0; i < numSegments; ++i) {
      delete[] segments_array[i].lights;
    }
    delete[] segments_array;
  }
  // Create arrays
  segments_array = new mySegment[numSegments];
  for (int i = 0; i < numSegments; ++i) {
    segments_array[i].hasLights = false;      // Default set to false, which means no lights to turn on/of or colorize in this segment
    segments_array[i].lights = new myLight[numLightsPerSegment];
    for (int j = 0; j < numLightsPerSegment; ++j) {
      segments_array[i].lights[j].state = false; // Default state off
      segments_array[i].lights[j].color = 0x000000; // Default color black
    }
  }
}

// Generate a unique ID based on the MAC address and a SALT
String HttpPullLightControl::generateUniqueId() {
  // We use an easy to implement Fowler–Noll–Vo hash function so we dont need any Sha1.h or Crypto.h dependencies
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char macStr[18];
  sprintf(macStr, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  String input = String(macStr) + salt;
  unsigned long hashValue = FNV_offset_basis;
  for (char c : input) {
    hashValue *= FNV_prime;
    hashValue ^= c;
  }
  DEBUG_PRINT(F("Unique ID generated: "));
  DEBUG_PRINTLN(hashValue);
  return String(hashValue);
}

// This function is called when the user updates the Sald and so we need to re-calculate the unique ID
void HttpPullLightControl::updateSalt(String newSalt) {
  DEBUG_PRINTLN(F("Salt updated"));
  this->salt = newSalt;
  uniqueId = generateUniqueId();
}

// The function is used to separate the URL in a host part and a path part
void HttpPullLightControl::parseUrl() {
  int firstSlash = url.indexOf('/', 7);  // Skip http(s)://
  host = url.substring(7, firstSlash);
  path = url.substring(firstSlash);
}

// This function is called by WLED when the USERMOD config is read
bool HttpPullLightControl::readFromConfig(JsonObject& root) {
  // Attempt to retrieve the nested object for this usermod
  JsonObject top = root[FPSTR(_name)];
  bool configComplete = !top.isNull();  // check if the object exists

  // Retrieve the values using the getJsonValue function for better error handling
  configComplete &= getJsonValue(top[FPSTR(_enabled)], enabled, enabled);  // default value=enabled
  configComplete &= getJsonValue(top["checkInterval"], checkInterval, checkInterval);  // default value=60
  #ifndef HTTP_PULL_LIGHT_CONTROL_HIDE_URL
    configComplete &= getJsonValue(top["url"], url, url);  // default value="http://example.com"
  #endif
  #ifndef HTTP_PULL_LIGHT_CONTROL_HIDE_SALT
    configComplete &= getJsonValue(top["salt"], salt, salt);  // default value=your_salt_here
  #endif

  return configComplete;
}

// This function is called by WLED when the USERMOD config is saved in the frontend
void HttpPullLightControl::addToConfig(JsonObject& root) {
  // Create a nested object for this usermod
  JsonObject top = root.createNestedObject(FPSTR(_name));

  // Write the configuration parameters to the nested object
  top[FPSTR(_enabled)] = enabled;
  top["checkInterval"] = checkInterval;
  #ifndef HTTP_PULL_LIGHT_CONTROL_HIDE_URL
    top["url"] = url;
  #endif
  #ifndef HTTP_PULL_LIGHT_CONTROL_HIDE_SALT
    top["salt"] = salt;
    updateSalt(salt);  // Update the UniqueID
  #endif
  parseUrl();  // Re-parse the URL, maybe path and host is changed
}

// This function calculated the exact position of the LED on the strip when the segment and the number is the segment is given.
// Example: When you have two segments of 100 LEDs. What is the position of LED 50 of the second segment?  = position 150 on the strip.
uint16_t HttpPullLightControl::getAbsoluteIndex(uint8_t segmentId, uint16_t relativeIndex) {
  Segment& seg = strip.getSegment(segmentId);
  if (relativeIndex < seg.length()) {
    return seg.start + relativeIndex;  // Calculate absolute position
  }
  return 65535;  // If invalid, return , because 0 is probably the first led.
}

// Do the http request here. Note that we can not do https requests with the AsyncTCP library
// We do everything Asynchronous, so all callbacks are defined here
void HttpPullLightControl::checkUrl() {
  if (client != nullptr && client->connected()) {
      DEBUG_PRINTLN(F("We are still connected, do nothing"));
      // Do nothing, Client is still connected
      return;
  }
  if (client != nullptr) {
    // Delete previous client instance if exists, just to prevent any memory leaks
    DEBUG_PRINTLN(F("Delete previous instances"));
    delete client;
    client = nullptr;
  }

  DEBUG_PRINTLN(F("Creating new AsyncClient instance."));
  client = new AsyncClient();
  if(client) {
    client->onData([](void *arg, AsyncClient *c, void *data, size_t len) {
      DEBUG_PRINTLN(F("Data received."));
      // Cast arg back to the usermod class instance
      HttpPullLightControl *instance = (HttpPullLightControl *)arg;
      // Convertert to Safe-String
      char *strData = new char[len + 1];
      strncpy(strData, (char*)data, len);
      strData[len] = '\0';
      String responseData = String(strData);
      //String responseData = String((char *)data);
      // Make sure its zero-terminated String
      //responseData[len] = '\0';
      delete[] strData; // Do not forget to remove this one
      instance->handleResponse(responseData);
    }, this);
    client->onDisconnect([](void *arg, AsyncClient *c) {
      DEBUG_PRINTLN(F("Disconnected."));
      //Set the class-own client pointer to nullptr if its the current client
      HttpPullLightControl *instance = static_cast<HttpPullLightControl*>(arg);
      if (instance->client == c) {
        instance->client = nullptr;
      }
      // Do not remove client here, it is maintained by AsyncClient
    }, this);
    client->onTimeout([](void *arg, AsyncClient *c, uint32_t time) {
      DEBUG_PRINTLN(F("Timeout"));
      //Set the class-own client pointer to nullptr if its the current client
      HttpPullLightControl *instance = static_cast<HttpPullLightControl*>(arg);
      if (instance->client == c) {
        delete instance->client;
        instance->client = nullptr;
      }
      // Do not remove client here, it is maintained by AsyncClient
    }, this);
    client->onError([](void *arg, AsyncClient *c, int8_t error) {
      DEBUG_PRINTLN("Connection error occurred!");
      DEBUG_PRINT("Error code: ");
      DEBUG_PRINTLN(error);
      //Set the class-own client pointer to nullptr if its the current client
      HttpPullLightControl *instance = static_cast<HttpPullLightControl*>(arg);
      if (instance->client == c) {
        delete instance->client;
        instance->client = nullptr;
      }
      // Do not remove client here, it is maintained by AsyncClient
    }, this);
    client->onConnect([](void *arg, AsyncClient *c) {
      // Cast arg back to the usermod class instance
      HttpPullLightControl *instance = (HttpPullLightControl *)arg;
      instance->onClientConnect(c);  // Call a method on the instance when the client connects
    }, this);
    client->setAckTimeout(ackTimeout); // Just some safety measures because we do not want any memory fillup
    client->setRxTimeout(rxTimeout);
    DEBUG_PRINT(F("Connecting to: "));
    DEBUG_PRINT(host);
    DEBUG_PRINT(F(" via port "));
    DEBUG_PRINTLN((url.startsWith("https")) ? 443 : 80);
    //Try to connect
    if (!client->connect(host.c_str(), (url.startsWith("https")) ? 443 : 80)) {
      DEBUG_PRINTLN(F("Failed to initiate connection."));
      // Connection failed, so cleanup
      delete client;
      client = nullptr;
    } else {
      // Connection successfull, wait for callbacks to go on.
      DEBUG_PRINTLN(F("Connection initiated, awaiting response..."));
    }
  } else {
    DEBUG_PRINTLN(F("Failed to create AsyncClient instance."));
  }
}

// This function is called from the checkUrl function when the connection is establised
// We request the data here
void HttpPullLightControl::onClientConnect(AsyncClient *c) {
  DEBUG_PRINT(F("Client connected: "));
  DEBUG_PRINTLN(c->connected() ? F("Yes") : F("No"));

  if (c->connected()) {
    String request = "GET " + path + (path.indexOf('?') > 0 ? "&id=" : "?id=") + uniqueId + " HTTP/1.1\r\n"
                    "Host: " + host + "\r\n"
                    "Connection: close\r\n"
                    "Accept: application/json\r\n"
                    "Accept-Encoding: identity\r\n" // No compression
                    "User-Agent: ESP32 HTTP Client\r\n\r\n"; // Optional: User-Agent and end with a double rnrn !
    DEBUG_PRINT(request.c_str());
    auto bytesSent  = c->write(request.c_str());
    if (bytesSent  == 0) {
      // Connection could not be made
      DEBUG_PRINT(F("Failed to send HTTP request."));
    } else {
      DEBUG_PRINT(F("Request sent successfully, bytes sent: "));
      DEBUG_PRINTLN(bytesSent );
    }
  }
}

// This function is called when we receive data after connecting and doing our request
// We loop through the JSON data, see (per segment) if we need to set an array or light individual lights
void HttpPullLightControl::handleResponse(String& responseStr) {
  DEBUG_PRINTLN(F("Received response for handleResponse."));
  // Calculate a sufficient size for the JSON buffer
  size_t jsonSize = responseStr.length() * 3; // 3 times as much memory as the full content, that would be enough. It needs more for shifting and stuff
  DEBUG_PRINT(F("Calculated JSON buffer size: "));
  DEBUG_PRINTLN(jsonSize);
  DynamicJsonDocument doc(jsonSize);

  // Search for two linebreaks between headers and content
  int bodyPos = responseStr.indexOf("\r\n\r\n");
  if (bodyPos > 0) {
    String jsonStr = responseStr.substring(bodyPos + 4); // +4 Skip the two CRLFs
    jsonStr.trim();

    DEBUG_PRINTLN("Response: ");
    DEBUG_PRINTLN(jsonStr);

    // Attempt to deserialize the JSON response
    DeserializationError error = deserializeJson(doc, jsonStr);
    if (error) {
      // If there is an error in deserialization, exit the function
      DEBUG_PRINT(F("DeserializationError: "));
      DEBUG_PRINTLN(error.c_str());
      return;
    }
  } else {
    DEBUG_PRINTLN(F("No body found in the response"));
    return;
  }

  JsonArray segments = doc["segments"];
  if (!segments) {
    // If the 'segments' key is not found, exit the function
    DEBUG_PRINTLN(F("No 'segments' key found in JSON."));
    return;
  }

  DEBUG_PRINT(F("Processing "));
  DEBUG_PRINT(segments.size());
  DEBUG_PRINTLN(F(" segments."));

  // Clear our segments and lights array
  initializeSegments(max_segments, max_lights);

  // Loop through each segment in the array
  for (JsonObject segment : segments) {
    // Ensure 'segment_id' is present and is an integer
    if (!segment.containsKey("segment_id") || !segment["segment_id"].is<int>()) {
      DEBUG_PRINTLN(F("Segment missing 'segment_id' or it is not an int."));
      continue; // If not, skip to the next segment
    }

    // Parse the segment ID
    uint8_t segmentID = segment["segment_id"].as<uint8_t>();
    DEBUG_PRINT(F("Segment ID: "));
    DEBUG_PRINTLN(segmentID);

    // Check if the segment exists, if not, skip to the next
    if (segmentID >= strip.getSegmentsNum()) {
      DEBUG_PRINT(F("Segment ID "));
      DEBUG_PRINT(segmentID);
      DEBUG_PRINTLN(F(" exceeds the number of segments."));
      continue;
    }

    // Apply the effect settings if specified
    if (segment.containsKey("effect_id")) {
      segments_settings.segment_id = segmentID;
      segments_settings.effect_id = segment["effect_id"];
      segments_settings.speed = segment.containsKey("speed") ? segment["speed"].as<int>() : 0;
      segments_settings.intensity = segment.containsKey("intensity") ? segment["intensity"].as<int>() : 0;
      segments_settings.palette_id = segment.containsKey("palette_id") ? segment["palette_id"].as<int>() : 0;

      // Set all colorslots to NotUpdated
      for (auto& colorSlot : segments_settings.color_slots) {
          colorSlot.isValid = false;
      }
      // Check if color_slots exists
      if (!segment.containsKey("color_slots") || !segment["color_slots"].is<JsonArray>()) {
        DEBUG_PRINTLN(F("No 'color_slots' key found or not an array."));
      } else {
        JsonArray colorSlots = segment["color_slots"];
        // Loop through all color_slots in the JSON array
        for (JsonObject colorSlot : colorSlots) {
          int slotIndex = colorSlot["index"];
          if (slotIndex < 3) {  // Check if we are not running out of our array
            if (colorSlot.containsKey("color")) {
              segments_settings.color_slots[slotIndex].color = colorFromRgb(
                colorSlot["color"].containsKey("r") ? colorSlot["color"]["r"].as<uint8_t>() : 0, 
                colorSlot["color"].containsKey("g") ? colorSlot["color"]["g"].as<uint8_t>() : 0, 
                colorSlot["color"].containsKey("b") ? colorSlot["color"]["b"].as<uint8_t>() : 0
              );
            }
            segments_settings.color_slots[slotIndex].isValid = true; // Set this flag so we know we have updated this Colorslot, otherwise (we do not want to set the color to Black when it was not given in the JSON)
          } else {
            DEBUG_PRINTLN(F("Stop processing. Running out of array. Probably too much Color Slots are given in the JSON response."));
            break;
          }
        }
      }
      DEBUG_PRINTLN(F("Setting effect for segment."));
      setEffect(segments_settings); // Call to a function that sets the effect
    }


    // Apply the lights settings if specified
    if (segment.containsKey("lights")) {
      segments_array[segmentID].hasLights = true;
      DEBUG_PRINTLN(F("Setting Lights array for segment."));
      JsonArray lights = segment["lights"].as<JsonArray>();

      for (JsonObject lightObj : lights) {
        // Get relative index in segment and convert to absolute pixel on strip.
        uint16_t index = lightObj["index"].as<uint16_t>();
        uint32_t color = colorFromRgb(lightObj["color"].containsKey("r") ? lightObj["color"]["r"].as<uint8_t>() : 0, 
                                      lightObj["color"].containsKey("g") ? lightObj["color"]["g"].as<uint8_t>() : 0, 
                                      lightObj["color"].containsKey("b") ? lightObj["color"]["b"].as<uint8_t>() : 0);
        bool state = lightObj["state"].as<bool>();

        segments_array[segmentID].lights[index].state = state;
        segments_array[segmentID].lights[index].color = color;
      }
    }

    // Set the (overall?) brightness if specified
    if (segment.containsKey("brightness")) {
      uint8_t brightness = segment["brightness"].as<uint8_t>();
      DEBUG_PRINT(F("Setting brightness to "));
      DEBUG_PRINTLN(brightness);
      strip.setBrightness(brightness);
    }
  }
}

// Function to get the 32bit RGB color number from 3 separate 8bit R, G and B color values
uint32_t HttpPullLightControl::colorFromRgb(uint8_t r, uint8_t g, uint8_t b) {
  // Combine the RGB values into a single 32-bit integer color value
  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

// This function is called by WLED just before sending data on the BUS. We clear the strip here and set individual lights if needed
void HttpPullLightControl::handleOverlayDraw() {
  if (enabled && !offMode) {
    // Loop through segments
    for (uint8_t i = 0; i < strip.getSegmentsNum(); ++i) {
      Segment seg = strip.getSegment(i);
      if (!seg.isActive()) continue;
      // If this segment should have statically assigned LEDs, set the current mode to static
      if (segments_array[i].hasLights == true) {
        DEBUG_PRINTLN(F("In HandleOverLayDraw function"));
        DEBUG_PRINT(F("Current Segment mode: "));
        DEBUG_PRINTLN(seg.mode);
        if (seg.mode != FX_MODE_STATIC) {
          DEBUG_PRINT(F("Changing mode to STATIC for segment: "));
          DEBUG_PRINTLN(i);
          seg.setMode(FX_MODE_STATIC);
          DEBUG_PRINTLN(F("Segment set to Static."));
          // Set the whole segment to BLACK
          DEBUG_PRINT(F("Fill whole segment "));
          DEBUG_PRINT(i);
          DEBUG_PRINTLN(F(" to black"));
          seg.fill(BLACK);
          DEBUG_PRINTLN(F("Segment is black now"));
        }

        for (uint16_t y = 0; y < max_lights; ++y) {
          if (segments_array[i].lights[y].state && segments_array[i].lights[y].color != BLACK ) {
            DEBUG_PRINT(F("Set pixelcolor for Led: "));
            DEBUG_PRINTLN(y);
            //seg.setPixelColor(i, segments_array[i].lights[y].color);
            strip.setPixelColor(getAbsoluteIndex(i,y), segments_array[i].lights[y].color);
            DEBUG_PRINTLN(F("Succesfully set"));
          } else {
            //seg.setPixelColor(i, segments_array[i].lights[y].color);
            strip.setPixelColor(getAbsoluteIndex(i,y), BLACK);
          }
        }
      }
    }
  }
}

// THis function is used to start an effect on a given segment.
// It is called from the handlerespone function and the setup function (to do a start animation while booting)
void HttpPullLightControl::setEffect(mySegmentSetting& segment_settings) {
  DEBUG_PRINT(F("setEffect called for segment: "));
  DEBUG_PRINTLN(segment_settings.segment_id);

  // Get a reference to the segment
  Segment& seg = strip.getSegment(segment_settings.segment_id);

  // Assign the effect based on the effect_id
  DEBUG_PRINT(F("Current effectId: "));
  DEBUG_PRINTLN(seg.mode);
  if (segment_settings.effect_id < strip.getModeCount() && segment_settings.effect_id != seg.mode ) { // Check if the effect ID is within a valid range
    DEBUG_PRINT(F("Set to effectId: "));
    DEBUG_PRINTLN(segment_settings.effect_id);
    seg.setMode(segment_settings.effect_id); // Update the segment's effect
  }
  DEBUG_PRINT(F("New effectId is: "));
  DEBUG_PRINTLN(seg.mode);

  // Set the effect speed if provided
  if (seg.speed != segment_settings.speed) {
    DEBUG_PRINT(F("Set speed to: "));
    DEBUG_PRINTLN(segment_settings.speed);
    seg.speed = segment_settings.speed; // Update the speed for the effect
  }

  // Set the effect intensity if provided
  if (seg.intensity != segment_settings.intensity) {
    DEBUG_PRINT(F("Set intensity to: "));
    DEBUG_PRINTLN(segment_settings.intensity);
    seg.intensity = segment_settings.intensity; // Update the intensity for the effect
  }

    // Set the effect intensity if provided
  if (seg.palette != segment_settings.palette_id) {
    DEBUG_PRINT(F("Set paletteId to: "));
    DEBUG_PRINTLN(segment_settings.palette_id);
    seg.setPalette(segment_settings.palette_id); // Update the intensity for the effect
  }

  for (int i = 0; i < sizeof(segments_settings.color_slots) / sizeof(myColorSlot); ++i) {
    DEBUG_PRINT(F("Looping for color-slot: "));
    DEBUG_PRINTLN(i);
    if (segments_settings.color_slots[i].isValid) { // Check if we had a value in the JSON that we need to set
        DEBUG_PRINT(F("Current color: "));
        DEBUG_PRINTLN(seg.colors[i]);
      if (seg.colors[i] != segments_settings.color_slots[i].color) {
        DEBUG_PRINT(F("Set color slot "));
        DEBUG_PRINT(i);
        DEBUG_PRINT(F(" to: "));
        DEBUG_PRINTLN(segments_settings.color_slots[i].color); // Veronderstelt dat u kleuren in HEX wilt printen
        seg.setColor(i, segments_settings.color_slots[i].color); // Stel de kleurslot in
        DEBUG_PRINT(F("New color is set to: "));
        DEBUG_PRINTLN(seg.colors[i]);
      }
    }
  }

  strip.trigger();
}

