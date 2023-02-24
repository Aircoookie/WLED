#pragma once

#ifdef ARDUINO_ARCH_ESP32

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiServer.h>

#include "HKDebug.h"
#include "HKDevice.h"
#include "HKStore.h"
#include "HKClient.h"

#define SERVER_PORT 8080

#define LIGHT_CATEGORY_ID 5

// This is HAP Accessory Server.

class HKServer {
private:
  WiFiServer * server;

  HKDevice * device_callback = nullptr;

  // Any connected devices will be here.
  HKClient ** hk_clients = nullptr;

  bool connected = false;

  void setup_mdns();

  // Client
  int get_free_slot();
  void event_notify();
  void refresh_connections();

public:  
  bool begin();
  void poll();
  void reconnect();
  void add_device(HKDevice * new_device);
};

#endif