#include "wled.h"
/*
 * Register your v2 usermods here!
 */
#ifdef USERMOD_ANDON_MOD
#include "../usermods/ANDON_MOD/usermod_ANDON_MOD.h"
#endif

void registerUsermods()
{
#ifdef USERMOD_ANDON_MOD
  usermods.add(new UsermodAndon());
#endif
}