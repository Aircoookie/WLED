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

#ifdef USERMOD_FOUR_LINE_DISLAY
#include "../usermods/usermod_v2_four_line_display/usermod_v2_four_line_display.h"
#endif
#ifdef USERMOD_ROTARY_ENCODER_UI
#include "../usermods/usermod_v2_rotary_encoder_ui/usermod_v2_rotary_encoder_ui.h"
#endif
#ifdef USERMOD_AUTO_SAVE
#include "../usermods/usermod_v2_auto_save/usermod_v2_auto_save.h"
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
#ifdef USERMOD_SENSORSTOMQTT
  usermods.add(new UserMod_SensorsToMQTT());
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
}