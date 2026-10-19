# Word Clock Usermod V2

This usermod drives an 11x10 pixel matrix wordclock with WLED. There are 4 additional dots for the minutes.
The visualisation is described by 4 masks with LED numbers (single dots for minutes, minutes, hours and "clock"). The index of the LEDs in the masks always starts at 0, even if the ledOffset is not 0.
These parameters control the behavior (Usermod Settings page):

* `active`: enable/disable usermod
* `displayItIs`: enable/disable display of "Es ist" on the clock
* `ledOffset`: number of LEDs before the wordclock LEDs (default 0)
* `Meander wiring?`: enable for meander (serpentine) wiring, see below
* `Norddeutsch`: show "viertel nach" / "viertel vor" instead of "viertel" / "dreiviertel"

## Update for alternative wiring pattern

Based on this fantastic work I added an alternative wiring pattern.
The original used a long wire to connect DO to DI, from one line to the next line.

I wired my clock in meander style. So the first LED in the second line is on the right.
With this method, every other line was inverted and showed the wrong letter.

I added a switch in usermod called "meander wiring?" to enable/disable the alternate wiring pattern.

## Installation

Add the usermod to `custom_usermods` of your build environment, for example in `platformio_override.ini`:

```ini
[env:wordclock_esp32]
extends = env:esp32dev
custom_usermods = usermod_v2_word_clock
```

## Change Log

2022/08/18 added meander wiring pattern.

2022/03/30 initial commit
