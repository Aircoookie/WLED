#pragma once

#include "wled.h"

// #define WEATHER_DEBUG

//declare weathermod global variables (always precede with weather_ (psuedo class static variables)
static uint32_t usermods_pushLoop = 0; //effect pushes loop to execute. might be interesting for audioreactive too
static uint8_t  weather_units = 1; //config var metric (celsius) is default. (Standard=Kelvin, Imperial is Fahrenheit)
static float    weather_minTemp = 0; //config var
static float    weather_maxTemp = 40; //config var
static float    weather_temps[100]; //array of temperatures
static time_t   weather_times[100]; //array of corresponding times

//effect function
uint16_t mode_2DWeather(void) { 

  usermods_pushLoop = millis(); //will be reset to 0 in usermod loop

  SEGMENT.fadeToBlackBy(10);

  float currentTemp = 0;
  // time_t currentTime = 0;

  for (int x=0; x<SEGMENT.virtualWidth(); x++) {
    CRGB color;
    currentTemp = weather_temps[0];
    // currentTime = weather_times[0];
    if (weather_times[x%100] < localTime && weather_times[(x+1)%100] >= localTime) {
      color = RED;
      currentTemp = map(localTime, weather_times[x%100], weather_times[(x+1)%100], weather_temps[x%100] * 1000, weather_temps[(x+1)%100] * 1000) / 1000.0;
      // currentTime = localTime;
    }
    else
      color = ColorFromPalette(SEGPALETTE, map((uint8_t)weather_temps[x%100], 0, 40, 0, 255), 255, LINEARBLEND);

    for (int y=0; y<SEGMENT.virtualHeight() * (weather_temps[x%100]-weather_minTemp)/(weather_maxTemp - weather_minTemp); y++) {
      SEGMENT.setPixelColorXY(x, SEGMENT.virtualHeight() - y, color);
    }
    // Serial.print(" ");
    // Serial.print(weather_temps[x%16]);
  }
  if (localTime < weather_times[0])
    currentTemp = map(localTime, weather_times[0], weather_times[1], weather_temps[0] * 1000, weather_temps[1] * 1000) / 1000.0;

  // Serial.print(" time ");

  // char timeString[64];
  // epochToString(currentTime, timeString);
  // Serial.print(timeString);

  // Serial.print(" temp ");

  char tempString[5] = "";
  sprintf(tempString, "%5.2f", currentTemp);
  // Serial.println();

  CRGB color = ColorFromPalette(SEGPALETTE, map((uint8_t)currentTemp, 0, 40, 0, 255), 255, LINEARBLEND);
  //really don't understand why this is not working if width < 16 (only works when Serial.println is uncommented ???)
  // uint16_t x = 0;
  // const uint16_t xSpace = (SEGMENT.virtualWidth()<16)?4:5;
  // SEGMENT.drawCharacter(tempString[0], x, -2, 5, 8, color);
  // // Serial.printf("%d %d", x, xSpace);
  // x += xSpace;
  // SEGMENT.drawCharacter(tempString[1], x, -2, 5, 8, color);
  // // Serial.printf("  %d %d", x, xSpace);
  // x += xSpace;
  // SEGMENT.setPixelColorXY(x, 4, color);
  // // Serial.printf("  %d %d", x, xSpace);
  // x += (SEGMENT.virtualWidth()<16)?1:2;
  // SEGMENT.drawCharacter(tempString[3], x, -2, 5, 8, color);
  // // Serial.printf("  %d %d", x, xSpace);
  // x += xSpace;
  // SEGMENT.drawCharacter(tempString[4], x, -2, 5, 8, color);
  // // Serial.printf("  %d %d\n", x, xSpace);

 if (SEGMENT.virtualWidth() < 16) {
    SEGMENT.drawCharacter(tempString[0], 0, -2, 5, 8, color);
    SEGMENT.drawCharacter(tempString[1], 4, -2, 5, 8, color);
    SEGMENT.setPixelColorXY(8, 4, color);
    SEGMENT.drawCharacter(tempString[3], 9, -2, 5, 8, color);
  }
  else {
    SEGMENT.drawCharacter(tempString[0], 0, -2, 5, 8, color);
    SEGMENT.drawCharacter(tempString[1], 5, -2, 5, 8, color);
    SEGMENT.setPixelColorXY(10, 4, color);
    SEGMENT.drawCharacter(tempString[3], 12, -2, 5, 8, color);
    SEGMENT.drawCharacter(tempString[4], 17, -2, 5, 8, color);
  }

  return FRAMETIME;
}

static const char _data_FX_MODE_2DWEATHER[] PROGMEM = "Weather@;!;!;pal=54,2d"; //temperature palette

//utility function, move somewhere else???
void epochToString(time_t time, char *timeString) {
  tmElements_t tm;
  breakTime(time, tm);
  sprintf(timeString, "%04d-%02d-%02d %02d:%02d:%02d", tm.Year + 1970, tm.Month, tm.Day, tm.Hour, tm.Minute, tm.Second);
}

//utility function, move somewhere else???
void httpGet(WiFiClient &client, const char *url, char *errorMessage) {
  //https://arduinojson.org/v6/example/http-client/
  //is this the most compact way to do http get and put it in arduinojson object???
  //would like async response ... ???
  client.setTimeout(10000);
  if (!client.connect("api.openweathermap.org", 80)) {
    strcat(errorMessage, PSTR("Connection failed"));
  }
  else {
    // Send HTTP request
    client.println(url);
    client.println(F("Host: api.openweathermap.org"));
    client.println(F("Connection: close"));
    if (client.println() == 0) {
      strcat(errorMessage, PSTR("Failed to send request"));
    }
    else {
      // Check HTTP status
      char status[32] = {0};
      client.readBytesUntil('\r', status, sizeof(status));
      if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
        strcat(errorMessage, PSTR("Unexpected response: "));
        strcat(errorMessage, status);
      }
      else {
        // Skip HTTP headers
        char endOfHeaders[] = "\r\n\r\n";
        if (!client.find(endOfHeaders)) {
          strcat(errorMessage, PSTR("Invalid response"));
        }
      }
    }
  }
}

class WeatherUsermod : public Usermod {
  private:
    // strings to reduce flash memory usage (used more than twice)
    static const char _name[]; //usermod name
    String apiKey = ""; //config var

    unsigned long lastTime = 0; //will be used to download new forecast every hour
    char errorMessage[100] = "";

  public:

    void setup() {
      strip.addEffect(255, &mode_2DWeather, _data_FX_MODE_2DWEATHER);
    }

    void connected() {
    }

    void loop() {
      //execute only if effect pushes it or every hour
      if (usermods_pushLoop > millis() - 1000 && (lastTime == 0 || millis() - lastTime > 3600 * 1000)) {
        lastTime = millis();

        WiFiClient client;

        char url[180];
        sprintf(url, "GET /data/2.5/forecast?lat=%f&lon=%f&appid=%s&units=%s HTTP/1.0", latitude, longitude, apiKey.c_str(), weather_units==0?"standard":weather_units==1?"metric":"imperial");
        #ifdef WEATHER_DEBUG
          Serial.println(url);
        #endif

        httpGet(client, url, errorMessage);

        if (strcmp(errorMessage, "") == 0) {
          // Allocate the JSON document
          // Use arduinojson.org/v6/assistant to compute the capacity.
          // const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
          PSRAMDynamicJsonDocument weatherDoc(24000); //in practive about 20.000

          // Parse JSON object
          DeserializationError error = deserializeJson(weatherDoc, client);
          if (error) {
            #ifdef WEATHER_DEBUG
              Serial.printf("weatherDoc %u / %u%% (%u %u %u)\n", (unsigned int)weatherDoc.memoryUsage(), 100 * weatherDoc.memoryUsage() / weatherDoc.capacity(), (unsigned int)weatherDoc.size(), weatherDoc.overflowed(), (unsigned int)weatherDoc.nesting());
            #endif
            strcat(errorMessage, PSTR("deserializeJson() failed: "));
            strcat(errorMessage, error.c_str());
          }
          else { //everything successful!!
            JsonObject weatherDocObject = weatherDoc.as<JsonObject>();
            JsonArray list = weatherDocObject[F("list")];
            JsonObject city = weatherDocObject["city"];
            strcat(errorMessage, city["name"]); //api succesfull
            strcat(errorMessage, city["country"]);

            uint8_t i = 0;
            for (JsonObject listElement: list) {
              weather_times[i%100] = listElement["dt"]; 

              JsonObject main = listElement["main"];
              weather_temps[i%100] = main["temp"]; 

              #ifdef WEATHER_DEBUG
                char timeString[64];
                epochToString(listElement["dt"], timeString);
                Serial.print(timeString);

                Serial.print(" temp ");
                Serial.print(weather_temps[i%100]);

                Serial.print(" city ");
                Serial.print(errorMessage);

                Serial.print(" sunrise ");
                char sunriseString[64];
                epochToString(city["sunrise"], sunriseString);
                Serial.print(sunriseString);

                Serial.print(" localtime ");
                char localTimeString[64];
                epochToString(localTime, localTimeString);
                Serial.print(localTimeString);

                Serial.println();
              #endif

              i++;
            }
          }
        }

        // Disconnect
        client.stop();

      }
      usermods_pushLoop = 0;
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

      JsonArray infoArr = user.createNestedArray(FPSTR(_name));
      infoArr.add(errorMessage); //value
      // infoArr.add(""); //unit
    }


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
      // userVar0 = root["user0"] | userVar0; //if "user0" key exists in JSON, update, else keep old value
      //if (root["bri"] == 255) Serial.println(F("Don't burn down your garage!"));
    }


    void addToConfig(JsonObject& root)
    {
      JsonObject top = root.createNestedObject(FPSTR(_name));
      top[F("apiKey")] = apiKey;
      top[F("units")]   = weather_units;
      top[F("minTemp")] = weather_minTemp;
      top[F("maxTemp")] = weather_maxTemp;
    }


    bool readFromConfig(JsonObject& root)
    {
      JsonObject top = root[FPSTR(_name)];

      bool configComplete = !top.isNull();

      configComplete &= getJsonValue(top[F("apiKey")], apiKey);
      configComplete &= getJsonValue(top[F("units")], weather_units);
      configComplete &= getJsonValue(top[F("minTemp")], weather_minTemp);
      configComplete &= getJsonValue(top[F("maxTemp")], weather_maxTemp);

      //  * Return true in case the config values returned from Usermod Settings were complete, or false if you'd like WLED to save your defaults to disk (so any missing values are editable in Usermod Settings)
      return configComplete;
    }

    void appendConfigData()
    {
      oappend(SET_F("dd=addDropdown('WeatherUserMod','units');"));
      oappend(SET_F("addOption(dd,'Kelvin',0);"));
      oappend(SET_F("addOption(dd,'Celcius',1);"));
      oappend(SET_F("addOption(dd,'Fahrenheit',2);"));
      oappend(SET_F("addInfo('WeatherUserMod:units',1,'<i>Set time and location in time settings</i>');"));
      oappend(SET_F("addInfo('WeatherUserMod:apiKey',1,'<i>Create acount on openweathermap.org and copy the key</i>');"));
      oappend(SET_F("addInfo('WeatherUserMod:minTemp',1,'<i>Changing values: Reboot to (re)load forecast</i>');"));
    }

    /*
     * handleOverlayDraw() is called just before every show() (LED strip update frame) after effects have set the colors.
     * Use this to blank out some LEDs or set them to a different color regardless of the set effect mode.
     * Commonly used for custom clocks (Cronixie, 7 segment)
     */
    void handleOverlayDraw()
    {
      //strip.setPixelColor(0, RGBW32(0,0,0,0)) // set the first pixel to black
    }

   
    /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
    uint16_t getId()
    {
      return USERMOD_ID_WEATHER;
    }
};

// strings to reduce flash memory usage (used more than twice)
const char WeatherUsermod::_name[]       PROGMEM = "WeatherUserMod";
