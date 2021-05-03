# Multi Relay

This usermod-v2 modification allows the connection of multiple relays each with individual delay and on/off mode.

## Usermod installation

1. Register the usermod by adding `#include "../usermods/multi_relay/usermod_multi_relay.h"` at the top and `usermods.add(new MultiRelay());` at the bottom of `usermods_list.cpp`.
or
2. Use `#define USERMOD_MULTI_RELAY` in wled.h or `-D USERMOD_MULTI_RELAY`in your platformio.ini

You can override the default maximum number (4) of relays by defining MULTI_RELAY_MAX_RELAYS.

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
#include "../usermods/usermod_multi_relay.h"

void registerUsermods()
{
  /*
   * Add your usermod class name here
   * || || ||
   * \/ \/ \/
   */
  //usermods.add(new MyExampleUsermod());
  //usermods.add(new UsermodTemperature());
  usermods.add(new MultiRelay());

}
```

## Configuration

Usermod can be configured in Usermods settings page.

If there is no MultiRelay section, just save current configuration and re-open Usermods settings page. 

Have fun - @blazoncek

## Change log
2021-04
* First implementation.