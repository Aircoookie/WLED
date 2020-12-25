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
#ifdef USERMOD_BUZZER
#include "../usermods/buzzer/usermod_v2_buzzer.h"
#endif
// BME280 v2 usermod. Define "USERMOD_BME280" in my_config.h
#ifdef USERMOD_BME280
#include "../usermods/BME280_v2/usermod_bme280.h"
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
  #ifdef USERMOD_BUZZER
  usermods.add(new BuzzerUsermod());
  #endif
  #ifdef USERMOD_BME280
  usermods.add(new UsermodBME280());
  #endif
}