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
    byte lastAutoPlaylist = 0;
    int change_timer = millis();

    uint_fast32_t energy = 0;

    uint_fast32_t avg_long_energy = 250;
    uint_fast32_t avg_long_lfc = 1000;
    uint_fast32_t avg_long_zcr = 500;

    uint_fast32_t avg_short_energy = 250;
    uint_fast32_t avg_short_lfc = 1000;
    uint_fast32_t avg_short_zcr = 500;

    uint_fast32_t vector_energy = 0;
    uint_fast32_t vector_lfc = 0;
    uint_fast32_t vector_zcr = 0;

    uint_fast32_t distance = 0;
    uint_fast32_t distance_tracker = UINT_FAST32_MAX;
    // uint_fast64_t squared_distance = 0;

    int lastchange = millis();

    int_fast16_t change_threshold = 50; // arbitrary starting point.
    uint_fast16_t change_threshold_change = 0;

    int change_lockout = 1000;    // never change below this number of millis. Ideally 60000/your_average_bpm*beats_to_skip = change_lockout (1000 = skip 2 beats at 120bpm)
    int ideal_change_min = 10000; // ideally change patterns no less than this number of millis
    int ideal_change_max = 20000; // ideally change patterns no more than this number of millis

    std::vector<int> autoChangeIds;
  
    static const char _enabled[];
    static const char _ambientPlaylist[];
    static const char _musicPlaylist[];
    static const char _timeout[];
    static const char _autoChange[];
    static const char _change_lockout[];
    static const char _ideal_change_min[];
    static const char _ideal_change_max[];

  public:

    AutoPlaylistUsermod(bool enabled):Usermod("AutoPlaylist", enabled) {
      // noop
    }

    // gets called once at boot. Do all initialization that doesn't depend on
    // network here
    void setup() {
      USER_PRINTLN("AutoPlaylistUsermod");
    }

    // gets called every time WiFi is (re-)connected. Initialize own network
    // interfaces here
    void connected() {
      // noop
    }

    void change(um_data_t *um_data) {

      uint8_t *fftResult = (uint8_t*)um_data->u_data[2];

      for (int i=0; i < NUM_GEQ_CHANNELS; i++) {
        energy += fftResult[i];
      }
      
      energy *= energy;
      energy /= 10000; // scale down so we get 0 sometimes

      uint8_t lfc = fftResult[0];
      uint_fast16_t zcr = *(uint_fast16_t*)um_data->u_data[11];

      // WLED-MM/TroyHacks: Calculate the long- and short-running averages
      // and the individual vector distances.

      if (volumeSmth > 1) { 

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

      }

      // distance is linear, squared_distance is magnitude.
      // linear is easier to fine-tune, IMHO.
      distance = vector_lfc + vector_energy + vector_zcr;
      // squared_distance = distance * distance;

      int change_interval = millis()-lastchange;

      if (distance < distance_tracker && change_interval > change_lockout && volumeSmth > 1) {
        distance_tracker = distance;
      }

      // USER_PRINTF("Distance: %3lu - v_lfc: %5lu v_energy: %5lu v_zcr: %5lu\n",(unsigned long)distance,(unsigned long)vector_lfc,(unsigned long)vector_energy,(unsigned long)vector_zcr);

      if (millis() > change_timer + ideal_change_min) {

        // Make the analysis less sensitive if we miss the window.
        // Sometimes the analysis lowers the change_threshold too much for
        // the current music, especially after track changes or during 
        // sparce intros and breakdowns.

        if (change_interval > ideal_change_min && distance_tracker < 1000) {

          change_threshold_change = (distance_tracker)-change_threshold;
          change_threshold = distance_tracker;

          USER_PRINTF("--- lowest distance =%4lu - no changes done in %6ums - next change_threshold is %3u (%3u diff aprox)\n", (unsigned long)distance_tracker,change_interval,change_threshold,change_threshold_change);

          distance_tracker = UINT_FAST32_MAX;

        }

        change_timer = millis();

      }
      
      if (distance <= change_threshold && change_interval > change_lockout && volumeSmth > 1) { 

        change_threshold_change = change_threshold-(distance*0.9);

        if (change_threshold_change < 1) change_threshold_change = 1;

        if (change_interval > ideal_change_max) {
          change_threshold += change_threshold_change;    // make changes more sensitive
        } else if (change_interval < ideal_change_min) {
          change_threshold -= change_threshold_change;    // make changes less sensitive
        } else {
          change_threshold_change = 0;                    // change was within our window, no sensitivity change
        }

        if (change_threshold < 1) change_threshold = 0;   // we need change_threshold to be signed becasue otherwise this wraps to UINT_FAST16_MAX

        distance_tracker = UINT_FAST32_MAX;

        if (autoChangeIds.size() == 0) {

          USER_PRINTF("Loading presets from playlist: %3u\n", currentPlaylist);

          JsonObject playtlistOjb = doc.to<JsonObject>();
          serializePlaylist(playtlistOjb);
          JsonArray playlistArray = playtlistOjb["playlist"]["ps"];

          for(JsonVariant v : playlistArray) {
            USER_PRINTF("Adding %3u to autoChangeIds\n", v.as<int>());
            autoChangeIds.push_back(v.as<int>());
          }

        }

        uint8_t newpreset = 0;

        do {
          newpreset = autoChangeIds.at(random(0, autoChangeIds.size())); // random() is *exclusive* of the last value, so it's OK to use the full size.
        }
        while (currentPreset == newpreset); // make sure we get a different random preset.

        if (change_interval > change_lockout+3) {

          // Make sure we have a statistically significant change and we aren't
          // just bouncing off change_lockout. That's valid for changing the
          // thresholds, but might be a bit crazy for lighting changes. 
          // When the music changes quite a bit, the distance calculation can
          // go into freefall - this logic stops that from triggering right
          // after change_lockout. Better for smaller change_lockout values.

          applyPreset(newpreset);

          USER_PRINTF("*** CHANGE distance =%4lu - change_interval was %5ums - next change_threshold is %3u (%3u diff aprox)\n",(unsigned long)distance,change_interval,change_threshold,change_threshold_change);

        } else {

          USER_PRINTF("*** SKIP!! distance =%4lu - change_interval was %5ums - next change_threshold is %3u (%3u diff aprox)\n",(unsigned long)distance,change_interval,change_threshold,change_threshold_change);

        }
        
        lastchange = millis();  

      }

    }

    /*
     * Da loop.
     */
    void loop() {
      
      if (millis() < 10000) return; // Wait for device to settle

      if (lastAutoPlaylist > 0 && currentPlaylist != lastAutoPlaylist && currentPreset != 0) {
        if (currentPlaylist == musicPlaylist) {
          USER_PRINTF("AutoPlaylist: enabled due to manual change of playlist back to %u\n", currentPlaylist);
          enabled = true;
          lastAutoPlaylist = currentPlaylist;
        } else if (enabled) {
          USER_PRINTF("AutoPlaylist: disable due to manual change of playlist from %u to %d, preset:%u\n", lastAutoPlaylist, currentPlaylist, currentPreset);
          enabled = false;
        }
      }

      if (!enabled && currentPlaylist == musicPlaylist) {
          USER_PRINTF("AutoPlaylist: enabled due selecting musicPlaylist(%u)", musicPlaylist);
          enabled = true;
      }

      if (!enabled) return;

      if (bri == 0) return;

      um_data_t *um_data;

      if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
        // No Audio Reactive
        silenceDetected = true;
        return;
      }

      float volumeSmth = *(float*)um_data->u_data[0];

      if (volumeSmth > 0.5) {
        lastSoundTime = millis();
      }

      if (millis() - lastSoundTime > (timeout * 1000)) {
        if (!silenceDetected) {
          silenceDetected = true;
          USER_PRINTF("AutoPlaylist: Silence ");
          changePlaylist(ambientPlaylist);
        }
      } else {
        if (silenceDetected) {
          silenceDetected = false;
          USER_PRINTF("AutoPlaylist: End of silence ");
          changePlaylist(musicPlaylist);
        }
        if (autoChange) change(um_data);
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

      infoArr = user.createNestedArray(F(""));
      if(!enabled) {
        infoArr.add("");  
      }
      else {
        infoArr.add("Active");
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
      JsonObject um = root[FPSTR(_name)];
      if (!um.isNull()) {
        if (um[FPSTR(_enabled)].is<bool>()) {
          enabled = um[FPSTR(_enabled)].as<bool>();
        }
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
      top[FPSTR(_change_lockout)]     = change_lockout;
      top[FPSTR(_ideal_change_min)]   = ideal_change_min;
      top[FPSTR(_ideal_change_max)]   = ideal_change_max;

      lastAutoPlaylist = 0;

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
      getJsonValue(top[_change_lockout], change_lockout);
      getJsonValue(top[_ideal_change_min], ideal_change_min);
      getJsonValue(top[_ideal_change_max], ideal_change_max);

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

  private:

    void changePlaylist(byte id) {
        String name = "";
        getPresetName(id, name);
        USER_PRINTF("apply %s\n", name.c_str());
        applyPreset(id, CALL_MODE_NOTIFICATION);
        lastAutoPlaylist = id;
    }

};

const char AutoPlaylistUsermod::_enabled[]          PROGMEM = "enabled";
const char AutoPlaylistUsermod::_ambientPlaylist[]  PROGMEM = "ambientPlaylist";
const char AutoPlaylistUsermod::_musicPlaylist[]    PROGMEM = "musicPlaylist";
const char AutoPlaylistUsermod::_timeout[]          PROGMEM = "timeout";
const char AutoPlaylistUsermod::_autoChange[]       PROGMEM = "autoChange";
const char AutoPlaylistUsermod::_change_lockout[]   PROGMEM = "change_lockout";
const char AutoPlaylistUsermod::_ideal_change_min[] PROGMEM = "ideal_change_min";
const char AutoPlaylistUsermod::_ideal_change_max[] PROGMEM = "ideal_change_max";
