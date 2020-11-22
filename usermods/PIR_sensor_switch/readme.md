# PIR sensor switch

This usermod-v2 modification allows the connection of a PIR sensor to switch on the LED strip when motion is detected. The switch-off occurs ten minutes after no more motion is detected.

_Story:_

I use the PIR Sensor to automatically turn on the WLED analog clock in my home office room when I am there.
The LED strip is switched [using a relay](https://github.com/Aircoookie/WLED/wiki/Control-a-relay-with-WLED) to keep the power consumption low when it is switched off.

## Webinterface

The info page in the web interface shows the items below

- the state of the sensor. By clicking on the state the sensor can be deactivated/activated. 
**I recommend to deactivate the sensor before installing an OTA update**.
- the remaining time of the off timer. 

## JSON API

The usermod supports  the following state changes:

| JSON key   | Value range | Description                     |
|------------|-------------|---------------------------------|
| PIRenabled | bool        | Deactivdate/activate the sensor |
| PIRoffSec  | 60 to 43200 | Off timer seconds               |

## Sensor connection

My setup uses an HC-SR501 sensor, a HC-SR505 should also work.

The usermod uses GPIO13 (D1 mini pin D7) for the sensor signal. 
[This example page](http://www.esp8266learning.com/wemos-mini-pir-sensor-example.php) describes how to connect the sensor.

Use the potentiometers on the sensor to set the time-delay to the minimum and the sensitivity to about half, or slightly above.

## Usermod installation

1. Copy the file `usermod_PIR_sensor_switch.h` to the `wled00` directory.
2. Register the usermod by adding `#include "usermod_PIR_sensor_switch.h"` in the top and `registerUsermod(new PIRsensorSwitch());` in the bottom of `usermods_list.cpp`.

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
#include  "usermod_PIR_sensor_switch.h"

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

## Usermod installation (advanced mode)

In this mode IR sensor will disable PIR when light ON by remote controller and enable PIR when light OFF.

1. Copy the file `usermod_PIR_sensor_switch.h` to the `wled00` directory.
2. Register the usermod by adding `#include "usermod_PIR_sensor_switch.h"` in the top and `registerUsermod(new PIRsensorSwitch());` in the bottom of `usermods_list.cpp`.
3. Add to the line 237, on `wled.h` in the `wled00` directory:

	`WLED_GLOBAL bool m_PIRenabled _INIT(true);                        // enable PIR sensor`
  
4. On `ir.cpp` in the `wled00` directory, add to the IR controller's mapping (beyond line 200):
	
- To the off button:
		`m_PIRenabled = true;`
    
- To the on button:
		`m_PIRenabled = false;`
    
5. Edit line 40, on `usermod_PIR_sensor_switch.h` in the `wled00` directory:
    
    `\\bool m_PIRenabled = true;`

Have fun - @gegu
