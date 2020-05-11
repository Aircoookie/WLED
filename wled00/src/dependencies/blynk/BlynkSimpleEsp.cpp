#include "BlynkSimpleEsp.h"

WiFiClient _blynkWifiClient;
BlynkArduinoClient _blynkTransport(_blynkWifiClient);
BlynkWifi Blynk(_blynkTransport);