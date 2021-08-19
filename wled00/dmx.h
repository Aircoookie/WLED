#ifndef DMX_H
#define DMX_H

#include <stdint.h>

/**
 * A virtual DMX fixture.
 *
 * The fixture is given a starting universe and address within that universe. A fixture can span multiple consecutive universes depending on the number of LEDs controlled by the fixture. 
 */
struct DMXFixture {
  uint16_t start_universe; // DMX universe assigned to this virtual fixture
  uint16_t start_address; // Start address (i.e. channel) within the universe
  uint16_t start_led; // Index of the first LED this fixture represents
  uint16_t led_count; // Number of LEDs in this fixture
};

#endif /* DMX_H */
