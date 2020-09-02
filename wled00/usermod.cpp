#include "wled.h"
#include "audio_reactive.h"
/*
 * This v1 usermod file allows you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * EEPROM bytes 2750+ are reserved for your custom use case. (if you extend #define EEPSIZE in const.h)
 * If you just need 8 bytes, use 2551-2559 (you do not need to increase EEPSIZE)
 *
 * Consider the v2 usermod API if you need a more advanced feature set!
 */

/*
 * Functions and variable delarations moved to audio_reactive.h
 * Not 100% sure this was done right. There is probably a better way to handle this...
 */


// This gets called once at boot. Do all initialization that doesn't depend on network here
void userSetup()
{
  #ifdef ESP32
    // Attempt to configure INMP441 Microphone
    esp_err_t err;
    const i2s_config_t i2s_config = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),  // Receive, not transfer
        .sample_rate = SAMPLE_RATE,                         // 10240, was 16KHz
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,       // could only get it to work with 32bits
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,        // LEFT when pin is tied to ground.
        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,           // Interrupt level 1
        .dma_buf_count = 8,                                 // number of buffers
        .dma_buf_len = BLOCK_SIZE                           // samples per buffer
    };
    const i2s_pin_config_t pin_config = {
      .bck_io_num = I2S_SCK,      // BCLK aka SCK
      .ws_io_num = I2S_WS,        // LRCL aka WS
      .data_out_num = -1,         // not used (only for speakers)
      .data_in_num = I2S_SD       // DOUT aka SD
    };
    // Configuring the I2S driver and pins.
    // This function must be called before any I2S driver read/write operations.
    err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
      Serial.printf("Failed installing driver: %d\n", err);
      while (true);
    }
    err = i2s_set_pin(I2S_PORT, &pin_config);
    if (err != ESP_OK) {
      Serial.printf("Failed setting pin: %d\n", err);
      while (true);
    }
    Serial.println("I2S driver installed.");

  delay(100);


  // Test to see if we have a digital microphone installed or not.

  float mean = 0.0;
  int32_t samples[BLOCK_SIZE];
  int num_bytes_read = i2s_read_bytes(I2S_PORT,
                                      (char *)samples,
                                      BLOCK_SIZE,     // the doc says bytes, but its elements.
                                      portMAX_DELAY); // no timeout

  int samples_read = num_bytes_read / 8;
  if (samples_read > 0) {
    for (int i = 0; i < samples_read; ++i) {
      mean += samples[i];
    }
    mean = mean/BLOCK_SIZE/16384;
    if (mean != 0.0) {
      Serial.println("Digital microphone is present.");
      digitalMic = true;
    } else {
      Serial.println("Digital microphone is NOT present.");
    }
  }
  #endif


  #ifdef ESP32
    pinMode(LED_BUILTIN, OUTPUT);

    sampling_period_us = round(1000000*(1.0/SAMPLE_RATE));

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

  if (millis()-lastTime > delayMs) {                        // I need to run this continuously because the animations are too slow
    if (!(audioSyncEnabled & (1 << 1))) {                   // Only run the sampling code IF we're not in Receive mode
      lastTime = millis();
      getSample();                                          // Sample the microphone
      agcAvg();                                             // Calculated the PI adjusted value as sampleAvg
      myVals[millis()%32] = sampleAgc;
      logAudio();
    }
    #ifdef ESP32
    if (audioSyncEnabled & (1 << 0)) {
      // Only run the transmit code IF we're in Transmit mode
//      Serial.println("Transmitting UDP Mic Packet");
      transmitAudioData();
    }
    #endif
  }

  // Begin UDP Microphone Sync
  if (audioSyncEnabled & (1 << 1)) {
    // Only run the audio listener code if we're in Receive mode
    if (millis()-lastTime > delayMs) {
      if (udpSyncConnected) {
//        Serial.println("Checking for UDP Microphone Packet");
        int packetSize = fftUdp.parsePacket();
        if (packetSize) {
          // Serial.println("Received UDP Sync Packet");
          uint8_t fftBuff[packetSize];
          fftUdp.read(fftBuff, packetSize);
          audioSyncPacket receivedPacket;
          memcpy(&receivedPacket, fftBuff, packetSize);
          for (int i = 0; i < 32; i++ ){
            myVals[i] = receivedPacket.myVals[i];
          }
          sampleAgc = receivedPacket.sampleAgc;
          sample = receivedPacket.sample;
          sampleAvg = receivedPacket.sampleAvg;
          // VERIFY THAT THIS IS A COMPATIBLE PACKET
          char packetHeader[6];
          memcpy(&receivedPacket, packetHeader, 6);
          if (!(isValidUdpSyncVersion(packetHeader))) {
            memcpy(&receivedPacket, fftBuff, packetSize);
            for (int i = 0; i < 32; i++ ){
              myVals[i] = receivedPacket.myVals[i];
            }
            sampleAgc = receivedPacket.sampleAgc;
            sample = receivedPacket.sample;
            sampleAvg = receivedPacket.sampleAvg;

            // Only change samplePeak IF it's currently false.  If it's true already, then the animation still needs to respond
            if (!samplePeak) {
              samplePeak = receivedPacket.samplePeak;
            }

            #ifdef ESP32   //These values are only available on the ESP32
              for (int i = 0; i < 16; i++) {
                fftResult[i] = receivedPacket.fftResult[i];
              }

              FFT_Magnitude = receivedPacket.FFT_Magnitude;
              FFT_MajorPeak = receivedPacket.FFT_MajorPeak;
              // Serial.println("Finished parsing UDP Sync Packet");
            #endif
          }
        }
      }
    }
  }
} // userLoop()
