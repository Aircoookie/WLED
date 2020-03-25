#ifndef WLED_HUE_H
#define WLED_HUE_H
/*
 * Sync to Philips hue lights
 */
#include <Arduino.h>
#include <ESPAsyncTCP.h>

void handleHue();
void reconnectHue();
void onHueError(void* arg, AsyncClient* client, int8_t error);
void onHueConnect(void* arg, AsyncClient* client);
void sendHuePoll();
void onHueData(void* arg, AsyncClient* client, void *data, size_t len);

#endif //WLED_HUE_H