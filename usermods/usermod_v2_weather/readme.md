# Usermods API v2 example usermod

In this usermod file you can find the documentation on how to take advantage of the new version 2 usermods!

### adding usermod specific effects

This usermod also provides an example on how to add your own effects in a usermod.
* the new effect code is in this function, similar to other effects fx.cpp:
```c++
//effect function
uint16_t mode_2DWeather(void) { 

....

  return FRAMETIME;
}
static const char _data_FX_MODE_2DWEATHER[] PROGMEM = "Weather@;!;!;2;pal=54"; //temperature palette
```
* then activated in the usermod setup function 
```c++
class WeatherUsermod : public Usermod {
  public:
    void setup() {
      strip.addEffect(255, &mode_2DWeather, _data_FX_MODE_2DWEATHER);
    }
```

## Installation 

Copy `usermod_v2_example.h` to the wled00 directory.  
Uncomment the corresponding lines in `usermods_list.cpp` and compile!  
_(You shouldn't need to actually install this, it does nothing useful)_

