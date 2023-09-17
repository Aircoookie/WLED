#include "wled00.h"

class CustomUsermod : public Usermod {
private:
  String deviceID;

public:
  void setup() override {
    Serial.begin(1000000);
    delay(100);
    randomSeed(millis());
    generateDeviceID();
  }

  void loop() override {
    if (Serial.available() > 0) {
      String receivedData = Serial.readStringUntil('\n');
      if (receivedData == "ID") {
        Serial.println(deviceID);
      }
    }
  }

  void generateDeviceID() {
    String chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    deviceID = "R";
    for (int i = 0; i < 4; i++) {
      char randomChar = chars.charAt(random(0, chars.length()));
      deviceID += randomChar;
    }
    deviceID += "T";
  }
};
