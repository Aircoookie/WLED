#pragma once

#include "wled.h"


class AutoPlaylistUsermod : public Usermod {

  private:
    bool silenceDetected = true;
    uint32_t lastSoundTime = 0;
    byte ambientPlaylist = 1;
    byte musicPlaylist = 2;
    int timeout = 60;
    bool autoChange = false;


    int avg_long_energy = 10000;
    int avg_long_lfc = 1000;
    int avg_long_zcr = 100;

    int avg_short_energy = 10000;
    int avg_short_lfc = 1000;
    int avg_short_zcr = 100;

    int vector_energy = 0;
    int vector_lfc = 0;
    int vector_zcr = 0;

    int squared_distance = 0;
    int lastchange = millis();

    int last_beat_interval = millis();
    int change_threshold = 10;

    int change_lockout = 100; // never change below this numnber of millis. I think of this more like a debounce, but opinions may vary.
    int ideal_change_max = 5000; // ideally change patterns no more than this number of millis
    int ideal_change_min = 2000; // ideally change patterns no less than this number of millis

    static const char _enabled[];
    static const char _ambientPlaylist[];
    static const char _musicPlaylist[];
    static const char _timeout[];
    static const char _autoChange[];

  public:

    AutoPlaylistUsermod(const char *name, bool enabled):Usermod(name, enabled) {}

    // gets called once at boot. Do all initialization that doesn't depend on
    // network here
    void setup() {
      USER_PRINTLN("AutoPlaylistUsermod");
    }

    // gets called every time WiFi is (re-)connected. Initialize own network
    // interfaces here
    void connected() {}

    void change(um_data_t *um_data) {

      int *extra = (int*) um_data->u_data[11];

      int zcr = extra[0];
      int energy = extra[1];
      int lfc = extra[2];

      // WLED-MM/TroyHacks: Calculate the long- and short-running averages
      // and the squared_distance for the vector.

      avg_long_energy = avg_long_energy * 0.99 + energy * 0.01;
      avg_long_lfc    = avg_long_lfc    * 0.99 + lfc    * 0.01;
      avg_long_zcr    = avg_long_zcr    * 0.99 + zcr    * 0.01;

      avg_short_energy = avg_short_energy * 0.9 + energy * 0.1;
      avg_short_lfc    = avg_short_lfc    * 0.9 + lfc    * 0.1;
      avg_short_zcr    = avg_short_zcr    * 0.9 + zcr    * 0.1;

      // allegedly this is faster than pow(whatever,2)
      vector_lfc = (avg_short_lfc-avg_long_lfc)*(avg_short_lfc-avg_long_lfc);
      vector_energy = (avg_short_energy-avg_long_energy)*(avg_short_energy-avg_long_energy);
      vector_zcr = (avg_short_zcr-avg_long_zcr)*(avg_short_zcr-avg_long_zcr);

      squared_distance = vector_lfc + vector_energy + vector_zcr;
      squared_distance = squared_distance * squared_distance / 10000000; // shorten just because it's a big number

      // WLED-MM/TroyHacks - Change pattern testing
      //
      int change_interval = millis()-lastchange;
      if (squared_distance <= change_threshold && change_interval > change_lockout) { 
        if (change_interval > ideal_change_max) {
          change_threshold += 1;
        } else if (change_interval < ideal_change_min) {
          change_threshold -= 1;
        }
        if (change_threshold < 0) change_threshold = 0;

        int newpreset = random(2,30);
        while (currentPreset == newpreset || newpreset == 8 || newpreset == 16 || newpreset == 27) { // 8 is a playlist for me so skip it and the other two just suck :D
          newpreset = random(2,30); // make sure we get a different preset
        }
        applyPreset(newpreset);
        USER_PRINTF("*** CHANGE! Squared distance = %d - change interval was %d ms - next change min is %d\n",squared_distance, change_interval, change_threshold);
        lastchange = millis();
      }
    }
    /*
     * Da loop.
     */
    void loop() {
      
      if(!enabled) return;

      um_data_t *um_data;
      if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
        // No Audio Reactive
        silenceDetected = true;
        return;
      }

      float volumeSmth    = *(float*)um_data->u_data[0];

      if(volumeSmth > 0.1) {
        lastSoundTime = millis();
      }

      if(millis() - lastSoundTime > (timeout * 1000)) {
        if(!silenceDetected) {
          silenceDetected = true;
          String name = "";
          getPresetName(ambientPlaylist, name);
          USER_PRINTF("AutoPlaylist: Silence - apply %s\n", name.c_str());
          applyPreset(ambientPlaylist, CALL_MODE_NOTIFICATION);
        }
        if(autoChange) change(um_data);
      }
      else {
        if(silenceDetected) {
          silenceDetected = false;
          String name = "";
          getPresetName(musicPlaylist, name);
          USER_PRINTF("AutoPlaylist: End of silence - apply %s\n", name.c_str());
          applyPreset(musicPlaylist, CALL_MODE_NOTIFICATION);
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
      if (user.isNull()) {
        user = root.createNestedObject("u");
      }

      JsonArray infoArr = user.createNestedArray(FPSTR(_name));  // name
      if(!enabled) {
        infoArr.add("disabled");  
      }
      else {
        infoArr.add(lastSoundTime);
      }
    }

    /*
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    //void addToJsonState(JsonObject& root) {
    //}

    /*
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void readFromJsonState(JsonObject& root) {
      if (!initDone) return;  // prevent crash on boot applyPreset()
      bool en = enabled;
      JsonObject um = root[FPSTR(_name)];
      if (!um.isNull()) {
        if (en != enabled) enabled = en;
      }
    }

    void appendConfigData() {
      oappend(SET_F("addHB('AutoPlaylist');"));
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
     * addToConfig() will also not yet add your setting to one of the settings pages automatically.
     * To make that work you still have to add the setting to the HTML, xml.cpp and set.cpp manually.
     * 
     * I highly recommend checking out the basics of ArduinoJson serialization and deserialization in order to use custom settings!
     */
    void addToConfig(JsonObject& root) {
      JsonObject top = root.createNestedObject(FPSTR(_name)); // usermodname
      top[FPSTR(_enabled)]            = enabled;
      top[FPSTR(_timeout)]            = timeout;
      top[FPSTR(_ambientPlaylist)]    = ambientPlaylist;  // usermodparam
      top[FPSTR(_musicPlaylist)]      = musicPlaylist;    // usermodparam
      top[FPSTR(_autoChange)]         = autoChange;
      DEBUG_PRINTLN(F("AutoPlaylist config saved."));
    }

    /*
     * readFromConfig() can be used to read back the custom settings you added with addToConfig().
     * This is called by WLED when settings are loaded (currently this only happens once immediately after boot)
     * 
     * readFromConfig() is called BEFORE setup(). This means you can use your persistent values in setup() (e.g. pin assignments, buffer sizes),
     * but also that if you want to write persistent values to a dynamic buffer, you'd need to allocate it here instead of in setup.
     * If you don't know what that is, don't fret. It most likely doesn't affect your use case :)
     * 
     * The function should return true if configuration was successfully loaded or false if there was no configuration.
     */
    bool readFromConfig(JsonObject& root) {
      JsonObject top = root[FPSTR(_name)];
      if (top.isNull()) {
        DEBUG_PRINT(FPSTR(_name));
        DEBUG_PRINTLN(F(": No config found. (Using defaults.)"));
        return false;
      }

      DEBUG_PRINT(FPSTR(_name));
      getJsonValue(top[_enabled], enabled);
      getJsonValue(top[_timeout], timeout);
      getJsonValue(top[_ambientPlaylist], ambientPlaylist);
      getJsonValue(top[_musicPlaylist], musicPlaylist);
      getJsonValue(top[_autoChange], autoChange);

      DEBUG_PRINTLN(F(" config (re)loaded."));

      // use "return !top["newestParameter"].isNull();" when updating Usermod with new features
      return true;
  }

    /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
    uint16_t getId() {
      return USERMOD_ID_AUTOPLAYLIST;
    }
};

const char AutoPlaylistUsermod::_enabled[]         PROGMEM = "enabled";
const char AutoPlaylistUsermod::_ambientPlaylist[] PROGMEM = "ambientPlaylist";
const char AutoPlaylistUsermod::_musicPlaylist[]   PROGMEM = "musicPlaylist";
const char AutoPlaylistUsermod::_timeout[]         PROGMEM = "timeout";
const char AutoPlaylistUsermod::_autoChange[]      PROGMEM = "autoChange";
