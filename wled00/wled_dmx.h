#ifndef WLED_DMX_H
#define WLED_DMX_H
/*
 * Support for DMX via MAX485.
 * Needs the espdmx library. You might have to change the output pin within the library. Sketchy, i know.
 * https://github.com/Rickgg/ESP-Dmx
 */

void handleDMX();

#endif //WLED_DMX_H