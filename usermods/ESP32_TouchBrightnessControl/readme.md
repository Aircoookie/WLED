# ESP32 Touch Brightness Control

Toggle On/Off with a long press (800ms)
Switch through 5 brightness levels (defined in usermod_touchbrightness.h, values 0-255) with a short (100ms) touch

## Installation 

Copy 'usermod_touchbrightness.h' to the wled00 directory.  
in 'usermod_list.cpp' add this:

> #include "usermod_touchbrightness.h"
above "void registerUsermods()"

and

> usermods.add(new TouchBrightnessControl());
inside the "registerUsermods()" function


