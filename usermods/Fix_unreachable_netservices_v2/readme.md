# Fix unreachable net services V2

This usermod-v2 modification performs a ping request to the local IP address every 60 seconds. By this procedure the net services of WLED remains accessible in some problematic WLAN environments.

The modification works with static or DHCP IP address configuration.

**Webinterface**: The number of pings and reconnects is displayed on the info page in the web interface.

_Story:_

Unfortunately, with all ESP projects where a web server or other network services are running, I have the problem that after some time the web server is no longer accessible.  Now I found out that the connection is at least reestablished when a ping request is executed by the device.

With this modification, in the worst case, the network functions are not available for 60 seconds until the next ping request.

## Installation

1. Copy the file `usermod_Fix_unreachable_netservices.h` to the `wled00` directory.
2. Register the usermod by adding `#include "usermod_Fix_unreachable_netservices.h"` in the top and `registerUsermod(new FixUnreachableNetServices());` in the bottom of `usermods_list.cpp`.

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
#include  "usermod_Fix_unreachable_netservices.h"

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
  usermods.add(new FixUnreachableNetServices());

}
```

Hopefully I can help someone with that - @gegu
