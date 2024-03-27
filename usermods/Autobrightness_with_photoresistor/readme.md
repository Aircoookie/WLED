# Autobrightness usermod

Autobrightness usermod by Michal Maciola

The usermod will use an analog photoresistor for ambient light measurement. In response to reading changes, led brightness will be adjusted.


## Installation

Minimal setup: set `-D USERMOD_AUTOBRIGHTNESS_PHOTORESISTOR` to your build_flags in `platformio.ini` and compile WLED from source.

Other configuration:
*   Define adc pin with `-D USERMOD_AUTOBRIGHTNESS_PHOTORESISTOR_PIN=0`.
*   If a photoresistor is connected between ADC pin and GND use inverted flag `-D USERMOD_AUTOBRIGHTNESS_PHOTORESISTOR_INVERTED`.
*   Set your bit to lux factor `-D USERMOD_AUTOBRIGHTNESS_BIT_TO_LUX_FACTOR=9215` (if not set, default 0x23ff will be used).

For other changes, see source code.

## Algorithm

The autobrightness usermod is using an array of overlapping buckets that defines min and max lux values, along with suggested brightness level.
When the ambient light changes, a new bucket is identified. The proportional difference, between the previous brightness and new suggested brightness value, is kept.
To minimize sensitivity to short-term changes in lighting, debounce functionality has been implemented. Changes in values must remain below a certain threshold for a defined duration, specified as `USERMOD_AUTOBRIGHTNESS_DEBOUNCE_MILLIS`.

## Change Log

September 2023 - initial release
