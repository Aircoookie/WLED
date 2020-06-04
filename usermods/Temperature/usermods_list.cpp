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
#include "usermod_temperature.h"
//#include "usermod_v2_empty.h"

void registerUsermods()
{
  /*
   * Add your usermod class name here
   * || || ||
   * \/ \/ \/
   */
  //usermods.add(new MyExampleUsermod());
  usermods.add(new UsermodTemperature());
  //usermods.add(new UsermodRenameMe());
}