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
#ifdef USERMOD_DALLASTEMPERATURE
#include "../usermods/Temperature/usermod_temperature.h"
#endif
//#include "usermod_v2_empty.h"

#ifdef WLED_ENABLE_SOUND
  #include "usermod_sr.h"
#endif

void registerUsermods()
{
  /*
   * Add your usermod class name here
   * || || ||
   * \/ \/ \/
   */
  //usermods.add(new MyExampleUsermod());
  #ifdef USERMOD_DALLASTEMPERATURE
  usermods.add(new UsermodTemperature());
  #endif
  //usermods.add(new UsermodRenameMe());

  #ifdef WLED_ENABLE_SOUND
    usermods.add(new UsermodSoundReactive());
  #endif
}