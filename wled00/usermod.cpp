#include "wled.h"
#include "audio_reactive.h"
/*
 * This file allows you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * EEPROM bytes 2750+ are reserved for your custom use case. (if you extend #define EEPSIZE in const.h)
 * bytes 2400+ are currently ununsed, but might be used for future wled features
 */

/*
 * Functions and variable delarations moved to audio_reactive.h
 * Not 100% sure this was done right. There is probably a better way to handle this...
 */


// This gets called once at boot. Do all initialization that doesn't depend on network here
void userSetup()
{
#ifndef ESP8266
  pinMode(LED_BUILTIN, OUTPUT);

 sampling_period_us = round(1000000*(1.0/samplingFrequency));

// Define the FFT Task and lock it to core 0
xTaskCreatePinnedToCore(
      FFTcode,                          // Function to implement the task
      "FFT",                            // Name of the task
      10000,                            // Stack size in words
      NULL,                             // Task input parameter
      1,                                // Priority of the task
      &FFT_Task,                        // Task handle
      0);                               // Core where the task should run
#endif
}

// This gets called every time WiFi is (re-)connected. Initialize own network interfaces here
void userConnected()
{
}

// userLoop. You can use "if (WLED_CONNECTED)" to check for successful connection
void userLoop() {

  if (millis()-lastTime > delayMs) {                          // I need to run this continuously because the animations are too slow
    lastTime = millis();
    getSample();                                              // Sample the microphone
    agcAvg();                                                 // Calculated the PI adjusted value as sampleAvg
    myVals[millis()%32] = sampleAgc;
  }

} // userLoop()