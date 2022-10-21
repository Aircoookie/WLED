# Seven Segment Display

Usermod that uses the overlay feature to create a configurable seven segment display.  
This has only been tested on a single configuration. Colon support is entirely  untested. 

## Installation

Add the compile-time option `-D USERMOD_SEVEN_SEGMENT` to your `platformio.ini` (or `platformio_override.ini`) or use `#define USERMOD_SEVEN_SEGMENT` in `my_config.h`.

## Settings
Settings can be controlled through both the usermod setting page and through MQTT with a raw payload.
##### Example
 Topic ```<mqttDeviceTopic||mqttGroupTopic>/sevenSeg/perSegment/set```  
 Payload ```3```
#### perSegment -- ssLEDPerSegment
The number of individual LEDs per segment. There are 7 segments per digit.  
#### perPeriod -- ssLEDPerPeriod
The number of individual LEDs per period. A ':' has 2x periods.
#### startIdx -- ssStartLED
Index of the LED that the display starts at. Allows a seven segment display to be in the middle of a string.
#### timeEnable -- ssTimeEnabled
When true, when displayMask is configured for a time output and no message is set the time will  be displayed.
#### scrollSpd -- ssScrollSpeed
Time, in milliseconds, between message shifts when the length of displayMsg exceeds the length of the displayMask.
#### displayMask -- ssDisplayMask
This should represent the configuration of the physical display. 
<pre>
HH - 0-23. hh - 1-12, kk - 1-24 hours  
MM or mm - 0-59 minutes  
SS or ss = 0-59 seconds  
: for a colon  
All others for alpha numeric, (will be blank when displaying time)
</pre>
##### Example
```HHMMSS ```  
```hh:MM:SS ```
#### displayMsg -- ssDisplayMessage
Message to be displayed across the display. If the length exceeds the length of the displayMask the message will scroll at scrollSpd. To 'remove' a message or revert  back to time, if timeEnabled is true, set the message to '~'.
#### displayCfg -- ssDisplayConfig
The order that your LEDs are configured. All seven segments in the display need to be wired the same way.
<pre>
           -------
         /   A   /          0 - EDCGFAB
        / F     / B         1 - EDCBAFG
       /       /            2 - GCDEFAB
       -------              3 - GBAFEDC
     /   G   /              4 - FABGEDC
    / E     / C             5 - FABCDEG
   /       /
   -------
      D
</pre>

## Version
20211009 - Initial release
