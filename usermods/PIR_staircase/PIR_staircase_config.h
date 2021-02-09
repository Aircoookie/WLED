/*
 * PIR_Staircase compiletime confguration.
 *
 * Please see README.md on how to change this file.
 */

// Please change the pin numbering below to match your board.
#define topPIR_PIN D7
#define bottomPIR_PIN D6

// Or uncumment and a pir and use an ultrasound HC-SR04 sensor,
// see README.md for details
#ifndef topPIR_PIN
#define topSignalPin D2
#define topEchoPin D3
#endif

#ifndef bottomPIR_PIN
#define bottomSignalPin D0
#define bottomEchoPin D1
#endif