# Usermod PIR Staircase

This usermod makes your staircase look cool:

- When the bottom PIR detects movement, the stairs will light up bottom-to-top
  step by step in the selected color effect.
- When the top PIR detects movement, the stairs will light up top-to-bottom
  step by step in the selected color effect.
- The stairs lights will switch off in the direction of the last PIR detection, and
  will always switch on when one of the PIRs detect movement, even if an effect
  is still running. It can therewith handle multiple people on the stairs gracefully.

## WLED integration

1. Open `wled00/usermods_list.cpp`
2. add `#include "../usermods/PIR_staircase/PIR_staircase.h"` to the top
3. add `usermods.add(new PIR_staircase());` to the end of the `void registerUsermods()` function.
4. Open `usermods/PIR_staircase/PIR_staircase.h` 
5. Change the PIR pinnumbers from line 13 and 14 into whatever
   pins are supported by your board.

   Examples:

   Using D7 and D6 pin notation as used on several boards:
   ```
     const int topPIR_PIN    = D7;
     const int bottomPIR_PIN = D6;
   ```

   Using GPIO 25 and 26 pins:
   ```
     const int topPIR_PIN    = 25;
     const int bottomPIR_PIN = 26;
   ```

## Hardware installation
1. Stick the led strip under each step of the stairs.
2. Mount a PIR sensor at the bottom of the stairs and connect it to bottomPIR_PIN.
3. Mount a PIR sensor at the top of the stairs and connect it to topPIR_PIN.

You may need to use 1k pull-down resistors on the selected PIR pins, depending on the sensor.

## WLED configuration
1. In the WLED UI, confgure a segment for each step. The lowest step of the stairs is the 
   lowest segment id. 
2. Save your segments into a preset. 
3. Ideally, add the subsequent preset ìn the config > LED setup menu to the "apply 
   preset `x` at boot" setting.
