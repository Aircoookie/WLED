#ifndef WLED_OVERLAYS_H
#define WLED_OVERLAYS_H
#include <Arduino.h>
/*
 * Used to draw clock overlays over the strip
 */

void initCronixie();
void handleOverlays();
void handleOverlayDraw();
void _overlayAnalogCountdown();
void _overlayAnalogClock();

#endif // WLED_OVERLAY_H