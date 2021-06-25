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
//#include "../usermods/EXAMPLE_v2/usermod_v2_example.h"

#ifdef USERMOD_DALLASTEMPERATURE
#include "../usermods/Temperature/usermod_temperature.h"
#endif

#ifdef USERMOD_SN_PHOTORESISTOR
#include "../usermods/SN_Photoresistor/usermod_sn_photoresistor.h"
#endif

//#include "usermod_v2_empty.h"

#ifdef USERMOD_BUZZER
#include "../usermods/buzzer/usermod_v2_buzzer.h"
#endif
#ifdef USERMOD_SENSORSTOMQTT
#include "../usermods/sensors_to_mqtt/usermod_v2_SensorsToMqtt.h"
#endif
#ifdef USERMOD_PIRSWITCH
#include "../usermods/PIR_sensor_switch/usermod_PIR_sensor_switch.h"
#endif

#ifdef USERMOD_MODE_SORT
#include "../usermods/usermod_v2_mode_sort/usermod_v2_mode_sort.h"
#endif

// BME280 v2 usermod. Define "USERMOD_BME280" in my_config.h
#ifdef USERMOD_BME280
#include "../usermods/BME280_v2/usermod_bme280.h"
#endif
#ifdef USERMOD_FOUR_LINE_DISPLAY
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

#ifdef USERMOD_ANIMATED_STAIRCASE
#include "../usermods/Animated_Staircase/Animated_Staircase.h"
#endif

#ifdef USERMOD_MULTI_RELAY
#include "../usermods/multi_relay/usermod_multi_relay.h"
#endif

#ifdef USERMOD_RTC
#include "../usermods/RTC/usermod_rtc.h"
#endif

#ifdef USERMOD_ELEKSTUBE_IPS
#include "../usermods/EleksTube_IPS/usermod_elekstube_ips.h"
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

  #ifdef USERMOD_SN_PHOTORESISTOR
  usermods.add(new Usermod_SN_Photoresistor());
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
  #ifdef USERMOD_PIRSWITCH
  usermods.add(new PIRsensorSwitch());
  #endif

  #ifdef USERMOD_MODE_SORT
  usermods.add(new ModeSortUsermod());
  #endif
  #ifdef USERMOD_FOUR_LINE_DISPLAY
  usermods.add(new FourLineDisplayUsermod());
  #endif
  #ifdef USERMOD_ROTARY_ENCODER_UI
  usermods.add(new RotaryEncoderUIUsermod()); // can use USERMOD_FOUR_LINE_DISPLAY
  #endif
  #ifdef USERMOD_AUTO_SAVE
  usermods.add(new AutoSaveUsermod());  // can use USERMOD_FOUR_LINE_DISPLAY
  #endif

  #ifdef USERMOD_DHT
  usermods.add(new UsermodDHT());
  #endif

  #ifdef USERMOD_VL53L0X_GESTURES
  usermods.add(new UsermodVL53L0XGestures());
  #endif

  #ifdef USERMOD_ANIMATED_STAIRCASE
  usermods.add(new Animated_Staircase());
  #endif

  #ifdef USERMOD_MULTI_RELAY
  usermods.add(new MultiRelay());
  #endif

  #ifdef USERMOD_RTC
  usermods.add(new RTCUsermod());
  #endif

  #ifdef USERMOD_ELEKSTUBE_IPS
  usermods.add(new ElekstubeIPSUsermod());
  #endif
}
