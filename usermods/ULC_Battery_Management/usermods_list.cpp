#include "wled.h"
/*
 * Register your v2 usermods here!
 */

/*
 * Add/uncomment your usermod filename here (and once more below)
 * || || ||
 * \/ \/ \/
 */
//#include "usermod_v2_example.h"
#if (defined(USERMOD_ULC_BATTERYMANAGEMENT) && defined(ARDUINO_ARCH_ESP32))
#include "../usermods/ULC_Battery_Management/usermod_ulc_batterymanagement.h"
#endif

//#include "usermod_v2_empty.h"

void registerUsermods()
{
  /*
   * Add your usermod class name here
   * || || ||
   * \/ \/ \/
   */
  //usermods.add(new MyExampleUsermod());
#if (defined(USERMOD_ULC_BATTERYMANAGEMENT) && defined(ARDUINO_ARCH_ESP32))
  usermods.add(new UsermodULCBatteryManagement());
#endif

  //usermods.add(new UsermodRenameMe());
}