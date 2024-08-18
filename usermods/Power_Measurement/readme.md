# Voltage and Current Measurement using ESP's Internal ADC

This usermod is a proof of concept that measures current and voltage using a voltage divider and a current sensor. It leverages the ESP's internal ADC to read the voltage and current, calculate power and energy consumption, and optionally publish these measurements via MQTT.

## Features

- **Voltage and Current Measurement**: Reads voltage and current using ADC pins of the ESP.
- **Power and Energy Calculation**: Calculates power (in watts) and energy consumption (in kilowatt-hours).
- **Calibration Support**: Offers calibration for more accurate measurements.
- **MQTT Publishing**: Publishes voltage, current, power, and energy measurements to an MQTT broker.
- **Debug Information**: Provides debug output via serial for monitoring raw and processed data.

## Dependencies

- **ESP32 ADC Calibration Library**: Requires `esp_adc_cal.h` for ADC calibration, which is a standard ESP-IDF library.

## Configuration

### Pins

- `VOLTAGE_PIN`: ADC pin for voltage measurement (default: `0`)
- `CURRENT_PIN`: ADC pin for current measurement (default: `1`)

### Constants

- `NUM_READINGS`: Number of readings for moving average (default: `10`)
- `NUM_READINGS_CAL`: Number of readings for calibration (default: `100`)
- `UPDATE_INTERVAL_MAIN`: Main update interval in milliseconds (default: `100`)
- `UPDATE_INTERVAL_MQTT`: MQTT update interval in milliseconds (default: `60000`)

## Installation

Add `-D USERMOD_CURRENT_MEASUREMENT` to `build_flags` in `platformio_override.ini`.

Or copy the example `platformio_override.ini` to the root directory. This file should be placed in the same directory as `platformio.ini`.

## Hardware Example

![Example Schematic](./assets/img/example%20schematic.png "Example Schematic")

## Define Your Options

- `USERMOD_POWER_MEASUREMENT`: Enable the usermod

All parameters and calibration variables can be configured at runtime via the Usermods settings page.

## Calibration

### Calibration Steps

1. Enable the `Calibration mode` checkbox.
2. Connect the controller via USB.
3. Disconnect the power supply (Vin) from the LED strip.
4. Select the option to `Calibrate Zero Points`.
5. Reconnect the power supply to the LED strip and set it to white and full brightness.
6. Measure the voltage and current and enter the values into the `Measured Voltage` and `Measured Current` fields.
7. Check the checkboxes for `Calibrate Voltage` and `Calibrate Current`.

### Advanced

![Advanced Calibration](./assets/img/screenshot%203%20-%20settings.png "Advanced Calibration")

## MQTT

If MQTT is enabled, the module will periodically publish the voltage, current, power, and energy measurements to the configured MQTT broker.

## Debugging

Enable `WLED_DEBUG` to print detailed debug information to the serial output, including raw and calculated values for voltage, current, power, and energy.

## Screenshots

Info screen                                                              | Settings page
:-----------------------------------------------------------------------:|:-------------------------------------------------------------------------------:
![Info screen](./assets/img/screenshot%201%20-%20info.jpg "Info screen") | ![Settings page](./assets/img/screenshot%202%20-%20settings.png "Settings page")

## To-Do

- [ ] Pin manager doesn't work properly.
- [ ] Implement a brightness limiter based on current.
- [ ] Make the code use less flash memory.

## Changelog

19.8.2024
- Initial PR

## License

This code was created by Tomáš Kuchta.

## Contributions

- Tomáš Kuchta (Initial idea)