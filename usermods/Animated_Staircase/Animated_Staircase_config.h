/*
 * Animated_Staircase compiletime confguration.
 *
 * Please see README.md on how to change this file.
 */

// Please change the pin numbering below to match your board.
#define TOP_PIR_PIN D5
#define BOTTOM_PIR_PIN D6

// Or uncumment and a pir and use an ultrasound HC-SR04 sensor,
// see README.md for details
#ifndef TOP_PIR_PIN
#define TOP_TRIGGER_PIN D2
#define TOP_ECHO_PIN D3
#endif

#ifndef BOTTOM_PIR_PIN
#define BOTTOM_TRIGGER_PIN D4
#define BOTTOM_ECHO_PIN D5
#endif