# PIR & Ldr sensor switch

This is an update to "PIR sensor switch" from @gegu because version 0.11.1 uses Preset instead of Macros.
This version uses a PIR sensor to activate the led strip and a Ldr to trigger between NightLight or "Party-Ligth".


_Story:_

I use the PIR sensor to activate my son's night light, the operation is as follows:
When detecting movement if there is enough light (it is daytime) a Preset from 2 to 6 is randomly activated, in this way it is very easy to choose the effect that you like the most and show off your WLED.

In case there is not enough light (my son gets up at night)
Preset1 is activated so Preset 1 is NightLight

## Webinterface

The info page in the web interface shows the items below

- the state of the sensor. By clicking on the state the sensor can be deactivated/activated. Changes persist after a reboot.
**I recommend to deactivate the sensor before an OTA update and activate it again afterwards**.
- the remaining time of the off timer. 
- the amount of light from the Ldr (0-1024), you can adjust the treshold.

## JSON API

The usermod supports the following state changes:

| JSON key   | Value range | Description                     |
|------------|-------------|---------------------------------|
| PIRenabled | bool        | Deactivdate/activate the sensor |
| PIRoffSec  | 60 to 43200 | Off timer seconds               |
| LDRadjust  | 0 to 1024   | Ldr treshold               |

 Changes also persist after a reboot.

## Sensor connection

My setup uses an AM312 sensor,I do not recommend using SR602 because it is too sensitive, the underfloor heating activated it, it almost drives me crazy.
And a LDR  with a pulldown resistor(10K) at input A0, the connection is very simple.

Gnd---<<10K>>---A0---(LDR)---Vcc (3v3)
               
               
The usermod uses A0 for the Ldr signal and GPIO13 (D1 mini pin D7) for the PIR sensor signal. 
[This example page](http://www.esp8266learning.com/wemos-mini-pir-sensor-example.php) describes how to connect the sensor.


## Usermod installation

1. Copy the file `usermod_PIR_Ldr_sensor_switch.h` to the `wled00` directory.
2. Register the usermod by adding `#include "usermod_PIR_Ldr_sensor_switch.h"` in the top and `registerUsermod(new PIRsensorSwitch());` in the bottom of `usermods_list.cpp`.

Example **usermods_list.cpp**:

```cpp
#include "wled.h"
/*
 * Register your v2 usermods here!
 *   (for v1 usermods using just usermod.cpp, you can ignore this file)
 */

/*
 * Add/uncomment your usermod filename here (and once more below)
 * || || ||
 * \/ \/ \/
 */
//#include "usermod_v2_example.h"
//#include "usermod_temperature.h"
//#include "usermod_v2_empty.h"
#include "usermod_PIR_Ldr_sensor_switch.h"

void registerUsermods()
{
  /*
   * Add your usermod class name here
   * || || ||
   * \/ \/ \/
   */
  //usermods.add(new MyExampleUsermod());
  //usermods.add(new UsermodTemperature());
  //usermods.add(new UsermodRenameMe());
  usermods.add(new PIRsensorSwitch());

}
```

## API to enable/disable the PIR sensor from outside. For example from another usermod.

The class provides the static method `PIRsensorSwitch* PIRsensorSwitch::GetInstance()` to get a pointer to the usermod object.

To query or change the PIR sensor state the methods `bool PIRsensorEnabled()` and `void EnablePIRsensor(bool enable)` are available. 

### There are two options to get access to the usermod instance:

1. Include `usermod_PIR_sensor_switch.h` **before** you include the other usermod in `usermods_list.cpp'

or

2. Use `#include "usermod_PIR_sensor_switch.h"` at the top of the `usermod.h` where you need it.

**Example usermod.h :**
```cpp
#include "wled.h"

#include "usermod_PIR_sensor_switch.h"

class MyUsermod : public Usermod {
  //...

  void togglePIRSensor() {
    if (PIRsensorSwitch::GetInstance() != nullptr) {
      PIRsensorSwitch::GetInstance()->EnablePIRsensor(!PIRsensorSwitch::GetInstance()->PIRsensorEnabled());
    }
  }
  //...
};
```

Have fun - @gegu & @dgcasana
