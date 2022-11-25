# Audioreactive usermod

Enabless controlling LEDs via audio input. Audio source can be a microphone or analog-in (AUX) using an appropriate adapter.
Supported microphones range from analog (MAX4466, MAX9814, ...) to digital (INMP441, ICS-43434, ...).

Does audio processing and provides data structure that specially written effects can use.

**does not** provide effects or draw anything to an LED strip/matrix.

## Installation 

Add `-D USERMOD_AUDIOREACTIVE` to your PlatformIO environment as well as `arduinoFFT` to your `lib_deps`.
If you are not using PlatformIO (which you should) try adding `#define USERMOD_AUDIOREACTIVE` to *my_config.h* and make sure you have _arduinoFFT_ library downloaded and installed.

Customised _arduinoFFT_ library for use with this usermod can be found at https://github.com/blazoncek/arduinoFFT.git

## Configuration

All parameters are runtime configurable. Some may require a hard reset after changing them (I2S microphone or selected GPIOs).

If you want to define default GPIOs during compile time, use the following (default values in parentheses):

- `DMTYPE=x` : defines microphone type: 0=analog, 1=generic I2S, 2=ES7243 I2S, 3=SPH0645 I2S, 4=generic I2S with master clock, 5=PDM I2S
- `AUDIOPIN=x` : GPIO for analog microphone/AUX-in (36)
- `I2S_SDPIN=x` : GPIO for SD pin on digital mcrophone (32)
- `I2S_WSPIN=x` : GPIO for WS pin on digital mcrophone (15)
- `I2S_CKPIN=x` : GPIO for SCK pin on digital mcrophone (14)
- `ES7243_SDAPIN` : GPIO for I2C SDA pin on ES7243 microphone (-1)
- `ES7243_SCLPIN` : GPIO for I2C SCL pin on ES7243 microphone (-1)
- `MCLK_PIN=x` : GPIO for master clock pin on digital mcrophone (-1)

**NOTE** I2S is used for analog audio sampling. Hence, the analog *buttons* (i.e. potentiometers) are disabled when running this usermod with an analog microphone.

## Release notes

2022-06 Ported from [soundreactive](https://github.com/atuline/WLED) by @blazoncek (AKA Blaz Kristan)
