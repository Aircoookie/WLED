#pragma once

#include "wled.h"
#include "time.h"

// #define YA_WEATHER_DEBUG                // Show debug message with [YandexWeatherUsermod] prefix 
// #define YA_WEATHER_ALLOW_ALL_TIMEOUT    // Allows you to set UpdateInterval to less than 30
// #define YA_WEATHER_HIDE_REMAINING       // Hide the remaining time to update in Info 
// #define YA_WEATHER_DRAW                 // Enable support of Four Line Display

#ifdef YA_WEATHER_DRAW
#ifndef USERMOD_FOUR_LINE_DISPLAY
#undef YA_WEATHER_DRAW
#endif
#endif

class YandexWeatherUsermod: public Usermod 
{
public:
    // Enums
    enum class WeatherInfoResult {
        Success = 0,    // Data fetch is Ok
        Timeout,        // The time hasn't come yet (Update interval)
        InternalError,  // ApiKey or coordinate missed
        ServerError,    // Some error from server
    };

    // Weather info
    struct WeatherInfo {
        unsigned long   date;       // Server time (Unixtime)
        int             tempReal;   // Real temperature (°C)
        int             tempFeels;  // Feels temperature (°C)
        float           windSpeed;  // Wind speed (kmp)
        String          windDir;    // Wind directions
      
      WeatherInfo(unsigned long dt, int tr, int tf, float ws, String wd): date(dt), tempReal(tr), tempFeels(tf), windSpeed(ws), windDir(wd) {}
    };

private:
    // Internal
    bool isConnected = false;
    char errorMessage[100] = "";
    WeatherInfo *_weatherInfo;

    #ifdef YA_WEATHER_DRAW
    FourLineDisplayUsermod* display;
    #endif

    // configurable parameters
    String _apiKey          = "";
    int _updateInterval     = 30;
    int _apiLanguage        = 0;
    bool _isUseCustomCoord  = false;
    bool _isShowInInfo      = false;
    bool _isPostToMQTT      = false;
    float _apiLat           = 0;
    float _apiLon           = 0;

    // Const chars
    static const char _json_enabled[];
    static const char _cfg_key_apiKey[];
    static const char _cfg_key_updateInterval[];
    static const char _cfg_key_apiLang[];
    static const char _cfg_key_custom_coord[];
    static const char _cfg_key_apiLat[];
    static const char _cfg_key_apiLon[];
    static const char _cfg_key_showInfo[];
    static const char _cfg_key_postMQTT[];
    
    // Inlines
    inline bool isValidCoordinate(float lat, float lon) { return fabs(lat) > __FLT_EPSILON__ && fabs(lon) > __FLT_EPSILON__; }
    inline bool isValidCoordinatePair(std::pair<float, float> coord) { return isValidCoordinate(coord.first, coord.second); }
    inline bool isCharEmpty(String val) { return strcmp(val.c_str(), "") == 0; }
    inline long unsigned getUpdateInterval() { return (_updateInterval * 60 * 1000); }
    inline String uppercasedString(String str) { str.toUpperCase(); return str; }

    inline const char* apiLanguageByIdx(int idx) {
        switch (idx) {
        case 1: return "ru_RU";
        case 2: return "ru_UA";
        case 3: return "uk_UA";
        case 4: return "be_BY";
        case 5: return "kk_KZ";
        case 6: return "tr_TR";
        default: return "en_US";
        }
    }

    inline std::string windCharByString(std::string wd) {
        if (wd.compare("nw") == 0)  { return "⬊"; }
        if (wd.compare("n") == 0)   { return "⬇︎"; }
        if (wd.compare("ne") == 0)  { return "⬋"; }
        if (wd.compare("e") == 0)   { return "⬅︎"; }
        if (wd.compare("se") == 0)  { return "⬉"; }
        if (wd.compare("s") == 0)   { return "⬆︎"; }
        if (wd.compare("sw") == 0)  { return "⬈"; }
        if (wd.compare("w") == 0)   { return "➡"; }
        return "●";
    }

    inline String firstWeatherLine() {
        if (_weatherInfo == nullptr) return String(F(""));
        String res = "";
        res += _weatherInfo->tempReal;
        res += F("°C\n(like ");
        res += _weatherInfo->tempFeels;
        res += F("°C)");
        return res;
    }

    inline String secondWeatherLine() {
        if (_weatherInfo == nullptr) return String(F(""));
        String res = "";
        res += _weatherInfo->windSpeed;
        res += F(" m/s");
        if (!_weatherInfo->windDir.isEmpty()) {
            res += F(" (");
            res += String(windCharByString(_weatherInfo->windDir.c_str()).c_str());
            res += F(")");
        }
        return res;
    }

    // Helping functions
    std::pair<float, float> getCoordinates()
    {
        std::pair<float, float> res { 0, 0};
        if (_isUseCustomCoord) {
            if (isValidCoordinate(_apiLat, _apiLon)) {
                res.first = _apiLat;
                res.second = _apiLon;
            }
        } else if (isValidCoordinate(latitude, longitude)) {
            res.first = latitude;
            res.second = longitude;
        }
        return res;
    }

    String epochToStirng(time_t time) {
        tmElements_t tm;
        breakTime(time, tm);
        char buf[20];
        sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", tm.Year + 1970, tm.Month, tm.Day, tm.Hour, tm.Minute, tm.Second);
        return String(buf);
    }

    String remainingHumanString() {
        unsigned long remainingTimeSec = (getUpdateInterval() - (millis() - lastTime)) / 1000;
        int h = remainingTimeSec / 3600;
        int m = (remainingTimeSec % 3600) / 60;
        int s = remainingTimeSec % 60;
        char buf[8];
        sprintf(buf, "%02d:%02d:%02d", h, m, s);
        return String(buf);
    }

    void resetLastTime() {
        lastTime = millis() - getUpdateInterval();
    }

    // API Calls
    bool parseReponse(WiFiClient client)
    {   
        bool isSuccess = false;

        StaticJsonDocument<256> filter;
        filter["now"] = true;
        filter["fact"]["temp"] = true;
        filter["fact"]["feels_like"] = true;
        filter["fact"]["wind_speed"] = true;
        filter["fact"]["wind_dir"] = true;

        PSRAMDynamicJsonDocument weatherDoc(4096);
        DeserializationError parseError = deserializeJson(weatherDoc, client, DeserializationOption::Filter(filter));
        if (parseError) {
            strcat(errorMessage, PSTR("deserializeJson() failed: "));
            strcat(errorMessage, parseError.c_str());
        } else {
            isSuccess = true;
            
            unsigned long nowDate = 0;
            getJsonValue(weatherDoc["now"], nowDate);

            int tempReal;
            int tempFeels;
            float windSpeed;
            String windDir;

            JsonObject weatherDocObject = weatherDoc.as<JsonObject>();
            JsonObject weatherFactObject = weatherDocObject["fact"];
            getJsonValue(weatherFactObject["temp"], tempReal);
            getJsonValue(weatherFactObject["feels_like"], tempFeels);
            getJsonValue(weatherFactObject["wind_speed"], windSpeed);
            getJsonValue(weatherFactObject["wind_dir"], windDir);

            if (_weatherInfo) {
                _weatherInfo->date = nowDate;
                _weatherInfo->tempReal = tempReal;
                _weatherInfo->tempFeels = tempFeels;
                _weatherInfo->windSpeed = windSpeed;
                _weatherInfo->windDir = windDir;
            } else {
                _weatherInfo = new WeatherInfo(nowDate, tempReal, tempFeels, windSpeed, windDir);
            }
            if (_weatherInfo) {
                publishMQTT(_weatherInfo);
            } else {
                isSuccess = false;
            }
        }

        return isSuccess;
    }

    WeatherInfoResult apiGetWeather(char *errorMessage)
    {
        WiFiClient client;
        client.setTimeout(10000);

        if(!client.connect("api.weather.yandex.ru", 80)) {
            strcat(errorMessage, PSTR("Connection failed"));
            #ifdef YA_WEATHER_DEBUG
            DEBUG_PRINTLN(F("[YandexWeatherUsermod] Connection failed"));
            #endif
            return WeatherInfoResult::ServerError;
        }

        if (client.connected()) {
            char url[180];
            std::pair<float, float> coords = getCoordinates();
            sprintf(url, "GET /v2/informers?lat=%f&lon=%f&lang=%s HTTP/1.0", coords.first, coords.second, apiLanguageByIdx(_apiLanguage));

            client.println(url);
            client.println(F("Host: api.weather.yandex.ru"));
            client.printf("X-Yandex-API-Key: %s\n", _apiKey.c_str());
            client.println(F("Connection: close"));
        }

        bool isSuccess = false;
        if (client.println() == 0) {
            strcat(errorMessage, PSTR("Failed to send request"));
            #ifdef YA_WEATHER_DEBUG
            DEBUG_PRINTLN(F("[YandexWeatherUsermod] Failed to send request"));
            #endif
        } else {
            char status[32] = {0};
            client.readBytesUntil('\r', status, sizeof(status));
            if (strcmp(status, "HTTP/1.0 200 OK") != 0) {
                strcat(errorMessage, PSTR("Unexpected response: "));
                strcat(errorMessage, status);
                #ifdef YA_WEATHER_DEBUG
                DEBUG_PRINTF("[YandexWeatherUsermod] Unexpected response: %s\n", status);
                #endif
            } else {
                char endOfHeaders[] = "\r\n\r\n";
                if (!client.find(endOfHeaders)) {
                    strcat(errorMessage, PSTR("Invalid response"));
                    #ifdef YA_WEATHER_DEBUG
                    DEBUG_PRINTLN(F("[YandexWeatherUsermod] Invalid response"));
                    #endif
                } else {
                    isSuccess |= parseReponse(client);
                }
            }
        }

        client.stop();
        return isSuccess ? WeatherInfoResult::Success : WeatherInfoResult::ServerError;
    }

    // MQTT
    void publishMQTT(WeatherInfo *wi) {
        if (!wi || !_isPostToMQTT) return;

        #ifndef WLED_DISABLE_MQTT
        if (WLED_MQTT_CONNECTED) {
            String topicName = mqttDeviceTopic;
            topicName += F("/yandexWeather");

            mqtt->publish((topicName + F("/date")).c_str(), 0, false, epochToStirng(_weatherInfo->date).c_str());
            
            char buf[10];
            sprintf(buf, "%d", _weatherInfo->tempReal);
            mqtt->publish((topicName + F("/tempReal")).c_str(), 0, false, buf);
            
            sprintf(buf, "%d", _weatherInfo->tempFeels);
            mqtt->publish((topicName + F("/tempFeels")).c_str(), 0, false, buf);

            sprintf(buf, "%.2f", _weatherInfo->windSpeed);
            mqtt->publish((topicName + F("/windSpeed")).c_str(), 0, false, buf);

            mqtt->publish((topicName + F("/windDir")).c_str(), 0, false, _weatherInfo->windDir.c_str());
        }
        #endif
    }

public:
    // Class Constructor/Destructor
    YandexWeatherUsermod(const char *name, bool enabled) : Usermod(name, enabled), _weatherInfo(nullptr) {
        lastTime = 0;
    }

    ~YandexWeatherUsermod() {
        if (_weatherInfo) { delete _weatherInfo; _weatherInfo = nullptr; }
    }    
    // Public methods
    /**
     * Togle enabled for Usermod
     * @param enable New state of enabled of Usermod
    */
    void setEnable(bool enable) { enabled = enable; }
    /**
     * @return Current enabled state of Usermod
    */
    bool isEnable() { return enabled;}
    /**
     * @return Current weather struct (WeatherInfo)
    */
    WeatherInfo *currentWeather() { return _weatherInfo; };
    
    /**
     * Try to fetch new weather data from server
     * @param isForce If `true` – ignore update interval
     * @return Status result of fetching data (WeatherInfoResult)
    */
    WeatherInfoResult tryGetWeather(bool isForce)
    {
        if (!isForce && millis() - lastTime < getUpdateInterval()) {
            return WeatherInfoResult::Timeout;
        }
        #ifdef YA_WEATHER_DEBUG
        DEBUG_PRINTLN(F("[YandexWeatherUsermod] Trying to get weather..."));
        #endif
        lastTime = millis();
        strcpy(errorMessage, "");

        if (isCharEmpty(_apiKey)) { 
            strcpy(errorMessage, PSTR("No API Key set"));
            #ifdef YA_WEATHER_DEBUG
            DEBUG_PRINTLN(F("[YandexWeatherUsermod] No API Key set"));
            #endif
            return WeatherInfoResult::InternalError;
        }

        std::pair<float, float> coords = getCoordinates();
        if (!isValidCoordinatePair(coords)) {
            strcpy(errorMessage, PSTR("No coordinates set"));
            #ifdef YA_WEATHER_DEBUG
            DEBUG_PRINTLN(F("[YandexWeatherUsermod] No coordinates set"));
            #endif
            return WeatherInfoResult::InternalError;
        }

        return apiGetWeather(errorMessage);
    }

    /**
     * Show weather (if exists) on Four line Display
     * @param howLong Display time in milliseconds
     * @return `true` – If displayed successfully, otherwise – `false`
    */
    inline bool drawWeatherOnDisplay(long howLong) {
        #ifdef YA_WEATHER_DRAW
        if (display != nullptr) {
            display->wakeDisplay();
            #if defined(USE_ALT_DISPLAY) || defined(USE_ALT_DISPlAY)
            if (display->canDraw()) display->overlay(firstWeatherLine().c_str(), secondWeatherLine().c_str(), howLong);   // WLEDMM bugfix
            #else
            display->overlay(firstWeatherLine().c_str(), secondWeatherLine().c_str(), howLong);
            #endif
            return true;
        } else {
            return false;
        }
        #else
        return false;
        #endif
    }

    // WLED lyfecycle

    void setup() {
        #ifdef YA_WEATHER_DRAW    
        display = (FourLineDisplayUsermod*) usermods.lookup(USERMOD_ID_FOUR_LINE_DISP);
        #endif
        initDone = true;
    }

    void connected() {
        isConnected = true;
    }
    
    void loop() {
        if (!initDone) return;
        if (!enabled || !isConnected || strip.isUpdating()) return;

        WeatherInfoResult res = tryGetWeather(false);
        switch (res) {
        case WeatherInfoResult::Success:
            lastTime = millis();
            break;

        case WeatherInfoResult::ServerError:
            lastTime = millis() - getUpdateInterval() + 10000;
            break;
        
        default:
            break;
        }
    }

    // MQTT

    void onMqttConnect(bool sessionPresent) {
        publishMQTT(_weatherInfo);
    }
    
    // Info page

    void addToJsonInfo(JsonObject &root)
    {
        if (!initDone) return;

        JsonObject user = root["u"];
        if (user.isNull()) user = root.createNestedObject("u");

        JsonArray infoArr = user.createNestedArray(FPSTR(_name));   // name
        #ifndef YA_WEATHER_HIDE_REMAINING
        if (enabled) {
            infoArr.add(remainingHumanString());
        }
        #endif

        String uiDomString = F("<button class=\"btn btn-xs\" onclick=\"requestJson({");
        uiDomString += FPSTR(_name);
        uiDomString += F(":{");
        uiDomString += FPSTR("enabled");
        uiDomString += enabled ? F(":false}});\">") : F(":true}});\">");
        uiDomString += F("<i class=\"icons ");
        uiDomString += enabled ? "on" : "off";
        uiDomString += F("\">&#xe08f;</i></button>");
        infoArr.add(uiDomString);

        if (!enabled || !_isShowInInfo) return;

        if (!isCharEmpty(errorMessage)) {
            infoArr = user.createNestedArray(F("Weather error"));
            String errorString = F("<b style=\"color:red;\">");
            errorString += FPSTR(errorMessage);
            errorString += F("</b>");
            infoArr.add(errorString);
        } else if (!_weatherInfo) {
            infoArr = user.createNestedArray(F("Weather info"));
            infoArr.add(F("no weather data"));
        } else {
            infoArr = user.createNestedArray(F("Weather date"));
            infoArr.add(epochToStirng(_weatherInfo->date));
            infoArr.add(F(" (UTC)"));
            
            infoArr = user.createNestedArray(F("Temperature"));
            infoArr.add(firstWeatherLine());

            infoArr = user.createNestedArray(F("Wind"));
            infoArr.add(secondWeatherLine());
        }
    }

    // JSON State

    void readFromJsonState(JsonObject& root)
    {
        if (!initDone) return;

        bool en = enabled;

        JsonObject um = root[FPSTR(_name)];
        if (!um.isNull()) {
            if (um[FPSTR(_json_enabled)].is<bool>()) {
                en = um[FPSTR(_json_enabled)].as<bool>();
            } else {
                String str = um[FPSTR(_json_enabled)];
                en = (bool)(str!="off");
            }
        }
        if (en != enabled) {
            enabled = en;
            if (enabled) resetLastTime();
        }
    }

    // Config

    void addToConfig(JsonObject &root)
    {
        Usermod::addToConfig(root);

        JsonObject top = root[FPSTR(_name)];
        top[FPSTR(_cfg_key_apiKey)] = _apiKey;
        top[FPSTR(_cfg_key_updateInterval)] = _updateInterval;
        top[FPSTR(_cfg_key_apiLang)] = _apiLanguage;
        top[FPSTR(_cfg_key_custom_coord)] = _isUseCustomCoord;
        if (isValidCoordinate(_apiLat, _apiLon)) {
            top[FPSTR(_cfg_key_apiLat)] = _apiLat;
            top[FPSTR(_cfg_key_apiLon)] = _apiLon;
        } else {
            top[FPSTR(_cfg_key_apiLat)] = 0;
            top[FPSTR(_cfg_key_apiLon)] = 0;
        }
        top[FPSTR(_cfg_key_showInfo)] = _isShowInInfo;
        top[FPSTR(_cfg_key_postMQTT)] = _isPostToMQTT;
    }

    void appendConfigData()
    {
        oappend(SET_F("addHB('YandexWeather');"));

        oappend(SET_F("addInfo('"));
        oappend(String(FPSTR(_name)).c_str());
        oappend((":" + String(FPSTR(_cfg_key_updateInterval))).c_str());
        #ifdef YA_WEATHER_ALLOW_ALL_TIMEOUT
        oappend(SET_F("', 1,'minutes <i>(should be ≥ 1)</i>');"));
        #else
        oappend(SET_F("', 1,'minutes <i>(should be ≥ 30)</i>');"));
        #endif

        oappend(SET_F("dd=addDropdown('"));
        oappend(String(FPSTR(_name)).c_str());
        oappend(("', '" + String(FPSTR(_cfg_key_apiLang)) + "');").c_str());
        oappend(SET_F("addOption(dd,'English',0);"));
        oappend(SET_F("addOption(dd,'Russian (Russian domain)',1);"));
        oappend(SET_F("addOption(dd,'Russian (Ukrainian domain)',2);"));
        oappend(SET_F("addOption(dd,'Ukrainian (Ukrainian domain)',3);"));
        oappend(SET_F("addOption(dd,'Belarusian',4);"));
        oappend(SET_F("addOption(dd,'Kazakh',5);"));
        oappend(SET_F("addOption(dd,'Turkish',6);"));
    }

    virtual bool readFromConfig(JsonObject &root)
    {
        // Old values (for re-call api for case when something changed)
        bool oldEnabledState = enabled;
        String oldAPIKey = _apiKey;
        int oldApiLanguage = _apiLanguage;
        bool oldIsUseCustomCoord = _isUseCustomCoord;
        std::pair<float, float> oldCoords { _apiLat, _apiLon};

        // Config logic
        bool configComplete = Usermod::readFromConfig(root);
        JsonObject top = root[FPSTR(_name)];
        if (top.isNull()) {
            return false;
        }

        configComplete &= getJsonValue(top[FPSTR(_cfg_key_updateInterval)], _updateInterval);
        #ifdef YA_WEATHER_ALLOW_ALL_TIMEOUT
        _updateInterval = max(1, _updateInterval);
        #else
        _updateInterval = max(30, _updateInterval);
        #endif
        
        configComplete &= getJsonValue(top[FPSTR(_cfg_key_apiKey)], _apiKey);
        configComplete &= getJsonValue(top[FPSTR(_cfg_key_apiLang)], _apiLanguage);
        _apiLanguage = max(0, min(6, _apiLanguage));

        configComplete &= getJsonValue(top[FPSTR(_cfg_key_custom_coord)], _isUseCustomCoord, false);
        configComplete &= getJsonValue(top[FPSTR(_cfg_key_apiLat)], _apiLat);
        configComplete &= getJsonValue(top[FPSTR(_cfg_key_apiLon)], _apiLon);
        configComplete &= getJsonValue(top[FPSTR(_cfg_key_showInfo)], _isShowInInfo, false);
        configComplete &= getJsonValue(top[FPSTR(_cfg_key_postMQTT)], _isPostToMQTT, false);

        // –– 
        if (enabled) {
            bool isEnabledChanged = (oldEnabledState == false);
            bool isApiKeyChanged = (oldAPIKey.compareTo(_apiKey) != 0);
            bool isApiLanguageChanged = (oldApiLanguage != _apiLanguage);
            bool isUseCustomCoordChanged = (oldIsUseCustomCoord != _isUseCustomCoord);
            bool isCoordinateChanges = (oldCoords.first != _apiLat || oldCoords.second != _apiLon);

            if (isEnabledChanged || isApiKeyChanged || isApiLanguageChanged || isUseCustomCoordChanged) resetLastTime();
            if (isUseCustomCoordChanged && isCoordinateChanges) resetLastTime();
        } else {
            strcpy(errorMessage, "");
        }

        return configComplete;
    }

    // Usermode ID

    uint16_t getId() {
        return USERMOD_ID_YA_WEATHER;
    }
};

// strings to reduce flash memory usage (used more than twice)
const char YandexWeatherUsermod::_json_enabled[]            PROGMEM = "enabled";
const char YandexWeatherUsermod::_cfg_key_apiKey[]          PROGMEM = "apiKey";
const char YandexWeatherUsermod::_cfg_key_updateInterval[]  PROGMEM = "updateInterval";
const char YandexWeatherUsermod::_cfg_key_apiLang[]         PROGMEM = "apiLanguage";
const char YandexWeatherUsermod::_cfg_key_custom_coord[]    PROGMEM = "customCoordinates";
const char YandexWeatherUsermod::_cfg_key_apiLat[]          PROGMEM = "cityLatitude";
const char YandexWeatherUsermod::_cfg_key_apiLon[]          PROGMEM = "cityLongitude";
const char YandexWeatherUsermod::_cfg_key_showInfo[]        PROGMEM = "showInInfo";
const char YandexWeatherUsermod::_cfg_key_postMQTT[]        PROGMEM = "postToMQTT";
