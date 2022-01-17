# Rotary Encoder (Brightness, Color and Color temperature)

Based on "usermod_rotary_brightness_color"

V2 usermod that allows changing brightness and color/color temperature using a rotary encoder, 
change between modes by pressing a button, change between RGB and white by double-pressing a button (many encoders have one included)


## Installation

define `USERMOD_ROTARY_ENCODER_BCT` e.g.

`#define USERMOD_ROTARY_ENCODER_BCT` in my_config.h

or add `-D USERMOD_ROTARY_ENCODER_BCT` to `build_flags` in platformio_override.ini

For the button to work a button has to be defined in `LED Preferences`. This button has to be a `Pushbutton`. For this the button included in the rotary encoder can be used. The same pin, that is used for the button has also to be set in the usermod config as `SWPin`.

### Define Your Options

Open Usermod Settings in WLED to change settings:

`fadeAmount` - how many points to fade the Neopixel with each step of the rotary encoder (default 5)  
`fadeAmountWhite` - how many points to change the color temperature (default 100)  
`CLKPin` is pin A (CLK pin) on your rotary encoder  
`DTPin` is pin B (DT pin) on your rotary encoder  
`SWPin` is the button on your rotary encoder (optional, set to -1 to disable the button and the rotary encoder will control brightness only)  
