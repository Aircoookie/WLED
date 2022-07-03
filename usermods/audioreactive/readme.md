# Audioreactive usermod

This usermod allows controlling LEDs using audio input. Audio input can be either microphone or analog-in (AUX) using appropriate adapter.
Supported microphones range from analog (MAX4466, MAX9814, ...) to digital (INMP441, ICS-43434, ...).

The usermod does audio processing and provides data structure that specially written effect can use.

The usermod **does not** provide effects or draws anything to LED strip/matrix.

## Installation 

Add `-D USERMOD_AUDIOREACTIVE` to your PlatformIO environment as well as `arduinoFFT` to your `lib_deps`.
If you are not using PlatformIO (which you should) try adding `#define USERMOD_AUDIOREACTIVE` to *my_config.h* and make sure you have _arduinoFFT_ library downloaded and installed.

Customised _arduinoFFT_ library for use with this usermod can be found at https://github.com/blazoncek/arduinoFFT.git

## Configuration

All parameters are runtime configurable though some may require hard boot after change (I2S microphone or selected GPIOs).

If you want to define default GPIOs during compile time use the following (default values in parentheses):

- `DMTYPE=x` : defines digital microphone type: 0=analog, 1=generic I2S, 2=ES7243 I2S, 3=SPH0645 I2S, 4=generic I2S with master clock, 5=PDM I2S
- `AUDIOPIN=x` : GPIO for analog microphone/AUX-in (36)
- `I2S_SDPIN=x` : GPIO for SD pin on digital mcrophone (32)
- `I2S_WSPIN=x` : GPIO for WS pin on digital mcrophone (15)
- `I2S_CKPIN=x` : GPIO for SCK pin on digital mcrophone (14)
- `ES7243_SDAPIN` : GPIO for I2C SDA pin on ES7243 microphone (-1)
- `ES7243_SCLPIN` : GPIO for I2C SCL pin on ES7243 microphone (-1)
- `MCLK_PIN=x` : GPIO for master clock pin on digital mcrophone (-1)

**NOTE** Due to the fact that usermod uses I2S peripherial for analog audio sampling, use of analog *buttons* (i.e. potentiometers) is disabled while running this usermod with analog microphone.

## Release notes

2022-06 Ported from [soundreactive](https://github.com/atuline/WLED) by @blazoncek (AKA Blaz Kristan)
