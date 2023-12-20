# Yandex Weaher Usermod
V2 Usermod for getting weather data from [Yandex Weather](https://yandex.ru/pogoda) service

## Web interface
The info page in the web interface shows:
- Remaining time until new weather request
- Current temperature (°C)
- Feels temperature (°C)
- Wind speed (m/s)
- Wind direction

## Usage
Compile the source with the buildflag `-D USERMOD_YA_WEATHER` added.

```ini
build_flags = ${env:esp32dev.build_flags}
  -D USERMOD_YA_WEATHER
```

## API for other Usermods
- `void setEnable(bool enable)` – To change enable state
- `bool isEnable()` – To query enable state
- `WeatherInfo *currentWeather()` – To query current weather struct
- `inline bool drawWeatherOnDisplay(long howLong)` – Show current weather (if available) on four line display for _howLong_ milliseconds. 
See _YA_WEATHER_DRAW_ in __Additional Build Flags__ section

### Access from other Usermod

There are two options to get access to the usermod instance:
 1. Include `yandex-weather-usermod-v2.h` before your Usermod in _usermods_list.cpp_
 1. Use `#include "yandex-weather-usermod-v2.h"` at the top of the  your Usermod

 ### Usage example
 ```cpp
#include "wled.h"
#include "yandex-weather-usermod-v2.h"

class MyUsermode: public Usermod {
  void foo() {
    #ifdef USERMOD_ID_YA_WEATHER
    YandexWeatherUsermod *weatherUserMode = (YandexWeatherUsermod *)usermods.lookup(USERMOD_ID_YA_WEATHER);
    if (weatherUserMode != nullptr) {
      weatherUserMode->setEnable(true);
      weatherUserMode->currentWeather();
    }
    #endif
  }
}
 ```

## Settings 

### Enabled:
Checkbox to enable or disable the Usermod

### ApiKey
ApiKey for access to Yandex Weather API.
You can get it [here](https://yandex.ru/dev/weather/doc/dg/concepts/about.html).

### UpdateInterval
API access frequency in minutes.

*Note:* It should be greater or equal to 30, because free APIKey has a limit of 50 requests per day.

### ApiLanguage
The language and country combinations for which weather wording data will be returned.

More info [here](https://yandex.ru/dev/weather/doc/dg/concepts/forecast-info.html)

### CustomCoordinates
Enables the use of custom coordinates (more on them below) instead of the system ones from WLED.

### CityLatitude
Custom city latitude coordinate (works only with `CustomCoordinates` enabled)

### CityLongitude
Custom city longitude coordinate (works only with `CustomCoordinates` enabled)

### ShowInInfo
Enable display of weather information in the "Info" page (on the WLED main scree)

### PostToMQTT
Enable post weather data to MQTT topic (/yandexWeather)

## MQTT Topics

| Topic                   | Unit    | Description             |
|:------------------------|:--------|:------------------------|
| yandexWeather/date      | String  | Weather received date   |
| yandexWeather/tempReal  | °C      | Real temperature        |
| yandexWeather/tempFeels | °C      | Feels like temperature  |
| yandexWeather/windSpeed | m/s     | Wind speed              |
| yandexWeather/windDir   | String  | Wind direction          |

## Additional Build Flags
| Flag                          | Description                                                                                   |
|:------------------------------|:----------------------------------------------------------------------------------------------|
| YA_WEATHER_DEBUG              | Show debug message with _[YandexWeatherUsermod]_ prefix                                       |
| YA_WEATHER_ALLOW_ALL_TIMEOUT  | Allows you to set UpdateInterval to less than 30 (Use for tests or if you have a paid ApiKey) |
| YA_WEATHER_HIDE_REMAINING     | Hide the remaining time to update in Info                                                     |
| YA_WEATHER_DRAW               | Enable support of [Four Line Display](https://mm.kno.wled.ge/usermods/4LineDisplay)           |

-----
Author:

2Grey | [Github](https://github.com/2Grey)