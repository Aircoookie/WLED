# Controlling Wiz lights

This usermod allows the control of [WiZ](https://www.wizconnected.com/en/consumer/) lights that are in the same network as the WLED controller.

The mod takes the colors from the first few pixels and sends them to the lights.

## Configuration

First, enter how often the data will be sent to the lights (in ms).

Then enter the IPs for the lights to be controlled, in order. There is currently a limit of 10 devices that can be controled, but that number
can be easily changed by updating _MAX_WIZ_LIGHTS_.

