# Word Clock Usermod V2

This usermod can be used to drive a wordclock with a 11x10 pixel matrix with WLED. There are also 4 additional dots for the minutes. 
The visualisation is desribed in 4 mask with LED numbers (single dots for minutes, minutes, hours and "clock/Uhr"). The index of the LEDs in the masks always starts with the index 0, even if the ledOffset is not 0.
There are 3 parameters to change the behaviour:
 
active: enable/disable usermod
diplayItIs: enable/disable display of "Es ist" on the clock
ledOffset: number of LEDs before the wordclock LEDs

### Update for alternatative wiring pattern
Based on this fantastic work I added an alternative wiring pattern.
For original you have to use a long wire to connect DO - DI from first line to the next line.

I wired my clock in meander style. So the first LED in second line is in the right.
With this problem every second line was inverted and showed the wrong letter.

I added a switch in usermod called "meander wiring?" to enable/disable alternativ wiring pattern.


## Installation

Copy and update the example `platformio_override.ini.sample` 
from the Rotary Encoder UI usermode folder to the root directory of your particular build.
This file should be placed in the same directory as `platformio.ini`.

### Define Your Options

* `USERMOD_WORDCLOCK`   - define this to have this the Auto Save usermod included wled00\usermods_list.cpp

### PlatformIO requirements

No special requirements.

## Change Log

2022/08/18 added meander wiring pattern.

2022/03/30 initial commit
