# Seven Segment Display Reloaded

Usermod that uses the overlay feature to create a configurable seven segment display.
Optimized for maximum configurability and use with seven segment clocks by parallyze (https://www.instructables.com/Retro-7-Segment-Clock-the-Final-Ones/)
Based on the existing usermod "seven segment display"


## Installation

Add the compile-time option `-D USERMOD_SSDR` to your `platformio.ini` (or `platformio_override.ini`) or use `#define USERMOD_SSDR` in `my_config.h`.


## Example

Example for Leds definition

      <  A  >
    /\       /\
    F        B
    \/       \/
      <  G  >
    /\       /\
    E        C
    \/       \/
      <  D  >


Leds or Range of Leds are seperated by a comma ","
Segments are seperated by a semicolon ";" and are read as A;B;C;D;E;F;G
Digits are seperated by colon ":" -> A;B;C;D;E;F;G:A;B;C;D;E;F;G
Ranges are defined as lower-higher

For example, an clock definition for the following clock (https://www.instructables.com/Lazy-7-Quick-Build-Edition/) is

hour "59,46;47-48;50-51;52-53;54-55;57-58;49,56:0,13;1-2;4-5;6-7;8-9;11-12;3,10"
minute "37-38;39-40;42-43;44,31;32-33;35-36;34,41:21-22;23-24;26-27;28,15;16-17;19-20;18,25"

or

hour "6,7;8,9;11,12;13,0;1,2;4,5;3,10:52,53;54,55;57,58;59,46;47,48;50,51;49,56"
minute "15,28;16,17;19,20;21,22;23,24;26,27;18,25:31,44;32,33;35,36;37,38;39,40;42,43;34,41"

depending on the orientation.

# The example detailed:
hour "59,46;47-48;50-51;52-53;54-55;57-58;49,56:0,13;1-2;4-5;6-7;8-9;11-12;3,10"

there are two digits seperated by ":"

- 59,46;47-48;50-51;52-53;54-55;57-58;49,56
- 0,13;1-2;4-5;6-7;8-9;11-12;3,10

In the first digit, 
the section A consists of the leds number 59 and 46., section B consists of the leds number 47, 48 and so on

The second digit starts again with segment A and leds 0 and 13, section B consists of the leds number 1 and 2 and so on

