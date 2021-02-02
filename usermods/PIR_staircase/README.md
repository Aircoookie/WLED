# Usermod PIR Staircase

This usermod makes your staircase look cool:

- When the bottom PIR detects feet, the stairs will light up bottom-to-top
  in the selected color effect.
- When the top PIR detects feet, the stairs will light up top-to-bottom
  in the selected color effect.
- The stairs will switch off in the direction of the last PIR detection, and
  will always switch on when one of the PIRs detect feet, even if an effect
  is running. It can handle multiple people on the stairs gracefully.

## WLED integration

Open 'usermods_list.cpp' and
1. add `#include "../usermods/PIR_staircase/PIR_staircase.h"` to the top
2. add `usermods.add(new PIR_staircase());` to the end of the `void registerUsermods()` function.
3. Change the PIR pinnumbers (D6 and D7) in `usermods/PIR_staircase/PIR_staircase.h` to whatever
   is supported by your board:
   ```
     const int topPIR_PIN    = D7;
     const int bottomPIR_PIN = D6;
   ```

## Hardware installation
1. Stick the led strip under each step of the stairs.
2. Mount a PIR sensor at the bottom of the stairs and connect it to bottomPIR_PIN.
3. Mount a PIR sensor at the top of the stairs and connect it to topPIR_PIN.

You may need to use 1k pull-down resistors on pins D5 and D6 depending on the sensor.

## WLED configuration
In the web interface, confgure  segments such that the lowest step
of the stairs is the lowest segment id.
