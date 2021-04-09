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
#ifdef USERMOD_SENSORSTOMQTT
#include "usermod_v2_SensorsToMqtt.h"
#endif

#ifdef USERMOD_MODE_SORT
#include "../usermods/usermod_v2_mode_sort/usermod_v2_mode_sort.h"
#endif

// BME280 v2 usermod. Define "USERMOD_BME280" in my_config.h
#ifdef USERMOD_BME280
#include "../usermods/BME280_v2/usermod_bme280.h"
#endif
#ifdef USERMOD_FOUR_LINE_DISLAY
#include "../usermods/usermod_v2_four_line_display/usermod_v2_four_line_display.h"
#endif
#ifdef USERMOD_ROTARY_ENCODER_UI
#include "../usermods/usermod_v2_rotary_encoder_ui/usermod_v2_rotary_encoder_ui.h"
#endif
#ifdef USERMOD_AUTO_SAVE
#include "../usermods/usermod_v2_auto_save/usermod_v2_auto_save.h"
#endif

#ifdef USERMOD_DHT
#include "../usermods/DHT/usermod_dht.h"
#endif

#ifdef USERMOD_VL53L0X_GESTURES
#include <Wire.h> //it's needed here to correctly resolve dependencies
#include "../usermods/VL53L0X_gestures/usermod_vl53l0x_gestures.h"
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
#ifdef USERMOD_SENSORSTOMQTT
  usermods.add(new UserMod_SensorsToMQTT());
#endif

#ifdef USERMOD_MODE_SORT
  usermods.add(new ModeSortUsermod());
#endif
#ifdef USERMOD_FOUR_LINE_DISLAY
  usermods.add(new FourLineDisplayUsermod());
#endif
#ifdef USERMOD_ROTARY_ENCODER_UI
  usermods.add(new RotaryEncoderUIUsermod());
#endif
#ifdef USERMOD_AUTO_SAVE
  usermods.add(new AutoSaveUsermod());
#endif

#ifdef USERMOD_DHT
usermods.add(new UsermodDHT());
#endif

#ifdef USERMOD_VL53L0X_GESTURES
  usermods.add(new UsermodVL53L0XGestures());
#endif
}