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
2. add `usermods.add(new PIR_staircase())` to the end of the `void registerUsermods()` function.

## Hardware installation
1. Stick the led strip under each step of the stairs
2. Mount a PIR sensor at the bottom of the stairs
3. Mount a PIR sensor at the top of the stairs

Connect the sensors to your board:

| Board           | Bottom PIR | Top PIR |
|-----------------|------------|---------|
| NodeMCU         | D5         | D6      |
| d1_mini esp32   | GPIO 15    | GPIO 16 |
| others          | GPIO 0     | GPIO 2  |

You may need to use 1k pull-down resistors on pins D5 and D6 depending on the sensor.

## WLED configuration
In the web interface, confgure  segments such that the lowest step
of the stairs is the lowest segment id.


## TODO:
- Store settings in flash (swipe speed, on-time)
