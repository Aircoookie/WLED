#pragma once

#include "wled.h"

//v2 usermod that allows to change brightness and color using a rotary encoder, 
//change between modes by pressing a button (many encoders have one included)
class UsermodBrightnessFollowSun : public Usermod
{
private:
  static const char _name[];
  static const char _enabled[];
  static const char _update_interval[];
  static const char _min_bri[];
  static const char _max_bri[];
  static const char _relax_hour[];

private:
  bool enabled = false; //WLEDMM
  unsigned long update_interval = 60;
  int min_bri = 0;
  int max_bri = 255;
  float relax_hour = 0;
  int relaxSec = 0;
  unsigned long lastUMRun = 0;
public:
  float mapFloat(float inputValue, float inMin, float inMax, float outMin, float outMax) {
    if (inMax == inMin) 
      return outMin;
    
    inputValue = constrain(inputValue, inMin, inMax);
    
    return ((inputValue - inMin) * (outMax - outMin) / (inMax - inMin)) + outMin;
  }

  uint16_t getId() override
  {
    return USERMOD_ID_BRIGHTNESS_FOLLOW_SUN;
  }

  void loop() override
  {
    if (!enabled || strip.isUpdating() || sunrise == 0 || sunset == 0 || localTime == 0)
      return;

    if (millis() - lastUMRun > (update_interval*1000))
      return;
    lastUMRun = millis();

    int sunriseSec = elapsedSecsToday(sunrise);
    int sunsetSec = elapsedSecsToday(sunset);
    int sunMiddleSec = sunriseSec + (sunsetSec-sunriseSec)/2;
    int curSec = elapsedSecsToday(localTime);

    int relaxSecH = sunriseSec-relaxSec;
    int relaxSecE = sunsetSec+relaxSec;

    int briSet = 0;
    if (curSec >= relaxSecH && curSec <= relaxSecE) {
      briSet = curSec < sunMiddleSec ?
                    mapFloat(curSec, sunriseSec, sunMiddleSec, min_bri, max_bri) :
                    mapFloat(curSec, sunMiddleSec, sunsetSec, max_bri, min_bri) ;
    }

    bri = briSet;
    stateUpdated(CALL_MODE_DIRECT_CHANGE);
}

  void addToConfig(JsonObject& root)
  {
      JsonObject top = root.createNestedObject(FPSTR(_name)); // usermodname

      top[FPSTR(_enabled)] = enabled;
      top[FPSTR(_update_interval)] = update_interval;
      top[FPSTR(_min_bri)] = min_bri;
      top[FPSTR(_max_bri)] = max_bri;
      top[FPSTR(_relax_hour)] = relax_hour;
  }

  bool readFromConfig(JsonObject& root)
  {
    JsonObject top = root[FPSTR(_name)];
    if (top.isNull()) {
      DEBUG_PRINTF("[%s] No config found. (Using defaults.)\n", _name);
      return false;
    }

    bool configComplete = true;

    configComplete &= getJsonValue(top[FPSTR(_enabled)], enabled, false);
    configComplete &= getJsonValue(top[FPSTR(_update_interval)], update_interval, 60);
    configComplete &= getJsonValue(top[FPSTR(_min_bri)], min_bri, 0);
    configComplete &= getJsonValue(top[FPSTR(_max_bri)], max_bri, 255);
    configComplete &= getJsonValue(top[FPSTR(_relax_hour)], relax_hour, 0);
    
    update_interval = constrain(update_interval, 1, SECS_PER_HOUR);
    _min_bri = constrain(_min_bri, 0, 255);
    _max_bri = constrain(_max_bri, 0, 255);
    _relax_hour = constrain(_relax_hour, 0, 3);

    relaxSec = SECS_PER_HOUR*_relax_hour;

    lastUMRun = millis()-(update_interval*1000);

    return configComplete;
  }
};


const char UsermodBrightnessFollowSun::_name[]                PROGMEM = "Brightness Follow Sun";
const char UsermodBrightnessFollowSun::_enabled[]             PROGMEM = "Enabled";
const char UsermodBrightnessFollowSun::_update_interval[]     PROGMEM = "Update Interval(sec)";
const char UsermodBrightnessFollowSun::_min_bri[]             PROGMEM = "Min Brightness";
const char UsermodBrightnessFollowSun::_max_bri[]             PROGMEM = "Max Brightness";
const char UsermodBrightnessFollowSun::_relax_hour[]          PROGMEM = "Relax Hour";
