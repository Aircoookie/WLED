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

#ifdef USERMOD_DHT
#include "../usermods/DHT/usermod_dht.h"
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
#ifdef USERMOD_DHT
  usermods.add(new UsermodDht());
#endif

  //usermods.add(new UsermodRenameMe());
}
