# Bus Status Usermod

v2 usermod to display bus/port status on a configured LED segment
Use instead of the builtin STATUSLED

WLED uses the term `bus` to refer to a logical port. This may be a physical collection of gpio pins or a virtual
network bus.

This usermod finds the type of each specified bus, plus the device status, and displays the result on configured LEDs.
The bus status colours are configurable and listed at the end.

The device status colours are:
* Red device element = Error value
* AP mode = Blue
* Connected to Wifi = Green
* MQTT Connected = Cyan

The status is also shown on the UI Info page.

## Define Your Options

* `USERMOD_BUSSTATUS`   - have this usermod included
* `BUSSTATUS_PINS`      - (optional) Array of GPIO pin numbers to display the status of, in order. e.g. `-D BUSSTATUS_PINS='{1,2,-1}'`
                        - `-1` displays the device status. Default: { -1, 1, 2}

## Configure

* Set up your pixel strips in `LED Preferences`. For a racitup.com v1 board the settings for the status bus are:
  - Type = WS281x
  - LED order = GRB
  - Length = 3
  - GPIO = 12
  - Skip = 0
  - `Make a segment for each output`: ticked
  - Ensure the automatic brightness limiter is set up appropriately for your number of LEDs, or disable

* Configure the associated segment. The bus type colours below are configurable:
  - Effect = Bus Status
  - Colour 1 = DMX (Blue)
  - Colour 2 = Digital (Green)
  - Colour 3 = PWM (Magenta)
